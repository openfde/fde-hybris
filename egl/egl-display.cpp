#include "egl-display.h"

#include <android/native_window.h>
#include <fcntl.h>
#include <log/log.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vndk/window.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <iomanip>
#include <set>
#include <sstream>

#include "egl-misc.h"
#include "egl-proxy.h"
#include "pixel-format.h"

namespace {

std::map<EGLint, EGLint> kAndroidSpecialAttributes{
    // EGL_ANDROID_framebuffer_target
    {EGL_FRAMEBUFFER_TARGET_ANDROID, EGL_TRUE},
    // EGL_ANDROID_recordable
    {EGL_RECORDABLE_ANDROID, EGL_TRUE},
    // EGL_NV_coverage_sample
    {EGL_COVERAGE_SAMPLES_NV, 0},
    {EGL_COVERAGE_BUFFERS_NV, 0},
    // EGL_NV_depth_nonlinear
    {EGL_DEPTH_ENCODING_NV, EGL_DEPTH_ENCODING_NONE_NV},
    // EGL_EXT_pixel_format_float
    {EGL_COLOR_COMPONENT_TYPE_EXT, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT},

    {EGL_NATIVE_VISUAL_ID, EGL_DONT_CARE},
    {EGL_NATIVE_VISUAL_TYPE, EGL_DONT_CARE},
};

const std::set<std::string> kExcludeForAndroidExtensions{
    "EGL_ANDROID_blob_cache",
    "EGL_ANDROID_get_native_client_buffer",
    "EGL_ANDROID_create_native_client_buffer",
    "EGL_ANDROID_presentation_time",
    "EGL_ANDROID_get_frame_timestamps",
};

}  // namespace

namespace egl {

DisplayManagerPtr &DisplayManager::Instance() {
  static bool inited = false;
  static DisplayManagerPtr manager;
  static std::mutex mtx;
  if (!inited) {
    std::lock_guard<std::mutex> guard{mtx};
    if (!inited) {
      manager = std::make_shared<DisplayManager>();
      std::atomic_thread_fence(std::memory_order::memory_order_release);
      inited = true;
    }
  }
  return manager;
}

Display *DisplayManager::GetDisplay(EGLNativeDisplayType display_id) {
  if (display_id == EGL_DEFAULT_DISPLAY) {
    return GetPlatformDisplay(EGL_PLATFORM_ANDROID_KHR, EGL_DEFAULT_DISPLAY,
                              nullptr);
  }
  ALOGD("eglGetDisplay failed: not support display id %p", display_id);
  return nullptr;
}

Display *DisplayManager::GetPlatformDisplay(EGLenum platform,
                                            void *native_display,
                                            const EGLAttrib *attrib_list) {
  auto proxy = EglProxy::Instance();
  if (!proxy) {
    return nullptr;
  }
  {
    std::lock_guard<std::mutex> lock{mtx_};
    if (auto it =
            FindDisplayPosByParameters(platform, native_display, attrib_list);
        it != displays_.end()) {
      ALOGD("eglGetPlatformDisplay(0x%04X) found display %p", platform,
            it->get());
      return it->get();
    }
  }

  DisplayPtr display = {};
  if (platform == EGL_PLATFORM_ANDROID_KHR) {
    display = std::make_shared<AndroidDisplay>(proxy);
  } else if (platform == EGL_PLATFORM_GBM_KHR) {
    display = std::make_shared<GbmDisplay>(proxy);
  } else {
    ALOGD("eglGetPlatformDisplay Not support platform 0x%04X, display %p",
          platform, native_display);
  }
  if (display) {
    display->SetParameters(platform, native_display, attrib_list);
    if (auto egl_dpy = display->GetPlatformDisplay(native_display, attrib_list);
        egl_dpy != EGL_NO_DISPLAY) {
      std::lock_guard<std::mutex> lock{mtx_};
      if (auto it = FindIdleSlot(); it != displays_.end()) {
        *it = display;
        return display.get();
      } else {
        ALOGD("eglGetPlatformDisplay failed: too many displays");
      }
    } else {
      ALOGD("eglGetPlatformDisplay failed : %s", proxy->StrLastError().c_str());
    }
  }

  return nullptr;
}

Display *DisplayManager::GetPlatformDisplayEXT(EGLenum platform,
                                               void *native_display,
                                               const EGLint *attrib_list) {
  std::vector<EGLAttrib> attribs;
  auto list = misc::ConvertAttributes(attrib_list, attribs);
  return GetPlatformDisplay(platform, native_display, list);
}

EGLBoolean DisplayManager::Terminate(EGLDisplay egl_dpy) {
  std::lock_guard<std::mutex> guard{mtx_};
  if (auto it = FindDisplayPos(egl_dpy); it != displays_.end()) {
    it->reset();
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

Display *DisplayManager::FindDispay(EGLDisplay egl_dpy) {
  if (auto it = FindDisplayPos(egl_dpy); it != displays_.end()) {
    return it->get();
  }
  return nullptr;
}

DisplayManager::DisplayIterator DisplayManager::FindDisplayPos(
    EGLDisplay egl_dpy) {
  return std::find_if(displays_.begin(), displays_.end(),
                      [egl_dpy](auto &display) {
                        return display && display->GetEglDisplay() == egl_dpy;
                      });
}

DisplayManager::DisplayIterator DisplayManager::FindDisplayPosByParameters(
    EGLenum platform, void *native_display, const EGLAttrib *attrib_list) {
  return std::find_if(displays_.begin(), displays_.end(),
                      [platform, native_display, attrib_list](auto &display) {
                        return display &&
                               display->SameAs(platform, native_display,
                                               attrib_list);
                      });
}

DisplayManager::DisplayIterator DisplayManager::FindIdleSlot() {
  return std::find_if(displays_.begin(), displays_.end(),
                      [](auto &display) { return !display; });
}

Display::ParameterT::ParameterT(EGLenum platform, void *native_display,
                                const EGLAttrib *attrib_list)
    : plt(platform),
      native_dpy(native_display),
      attribs(misc::DupAttributes(attrib_list)) {}

Display::~Display() {
  surfaces_.clear();
  image_manager_.ClearImages();
  egl_dpy_.reset();
}

const char *Display::GetEglExtensions() {
  constexpr char kNativeBufferExtensions[] = "EGL_ANDROID_image_native_buffer";
  if (!inited_extensions_) {
    std::lock_guard<std::mutex> guard{mtx_};
    if (!inited_extensions_) {
      inited_extensions_ = true;
      auto strs = proxy_->Api().eglQueryString(egl_dpy_.get(), EGL_EXTENSIONS);
      if (!strs) {
        return nullptr;
      }
      auto platform_extensions = misc::SplitBySpace(strs);
      // EGL_ANDROID_framebuffer_target ???
      if (std::find(platform_extensions.cbegin(), platform_extensions.cend(),
                    "EGL_EXT_image_dma_buf_import") !=
          platform_extensions.cend()) {
        // not support EGL_ANDROID_create_native_client_buffer and
        // EGL_ANDROID_get_native_client_buffer
        platform_extensions.push_back(kNativeBufferExtensions);
      }

      // EGL_KHR_fence_sync, EGL_KHR_image_base and EGL_KHR_gl_texture_2d_image
      // extensions
      extensions_ = misc::SerializeExtensions(platform_extensions,
                                              kExcludeForAndroidExtensions);
    }
  }
  return !extensions_.empty() ? extensions_.c_str() : nullptr;
}

Surface *Display::CreateWindowSurface(EGLConfig config, EGLNativeWindowType win,
                                      const EGLint *attrib_list) {
  std::vector<EGLAttrib> attribs;
  auto list = misc::ConvertAttributes(attrib_list, attribs);
  return CreatePlatformWindowSurface(config, win, list);
}

Surface *Display::CreatePlatformWindowSurfaceEXT(EGLConfig config,
                                                 void *native_window,
                                                 const EGLint *attrib_list) {
  std::vector<EGLAttrib> attribs;
  auto list = misc::ConvertAttributes(attrib_list, attribs);
  return CreatePlatformWindowSurface(config, native_window, list);
}

EGLSurface Display::CreatePbufferSurface(EGLConfig config,
                                         const EGLint *attrib_list) {
  auto surface =
      std::make_shared<PassthroughSurface>(GetEglDisplay(), proxy_, nullptr);
  std::vector<EGLAttrib> attribs;
  auto list = misc::ConvertAttributes(attrib_list, attribs);
  if (auto egl_surf = surface->CreateSurface(config, list);
      egl_surf != EGL_NO_SURFACE && AddSurface(surface)) {
    return surface.get();
  }
  return EGL_NO_SURFACE;
}

void Display::SetParameters(EGLenum platform, void *native_display,
                            const EGLAttrib *attrib_list) {
  parameters_ = ParameterT{platform, native_display, attrib_list};
}

bool Display::SameAs(EGLenum platform, void *native_display,
                     const EGLAttrib *attrib_list) {
  if (platform != parameters_.plt || native_display != parameters_.native_dpy) {
    return false;
  }
  if (!attrib_list && (parameters_.attribs.empty() ||
                       parameters_.attribs.front() == EGL_NONE)) {
    return true;
  }
  if (!attrib_list || parameters_.attribs.empty()) {
    return false;
  }
  size_t pos = {};
  for (auto end_pos = parameters_.attribs.size() - 1;
       pos < end_pos && attrib_list[pos] != EGL_NONE; pos++) {
    if (parameters_.attribs[pos] != attrib_list[pos]) {
      return false;
    }
  }
  if (attrib_list[pos] != parameters_.attribs[pos]) {
    return false;
  }
  return true;
}

void Display::SetEglDisplay(EGLDisplay egl_dpy) {
  if (egl_dpy) {
    egl_dpy_ = std::shared_ptr<void>(egl_dpy, [this](EGLDisplay dpy) {
      if (proxy_) {
        proxy_->Api().eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
                                     EGL_NO_CONTEXT);
        proxy_->Api().eglTerminate(dpy);
      }
    });
  }
}

bool Display::AddSurface(SurfacePtr surface) {
  if (surface) {
    std::lock_guard<std::mutex> guard{mtx_};
    assert(std::find(surfaces_.cbegin(), surfaces_.cend(), surface) ==
           surfaces_.cend());
    surfaces_.emplace_back(std::move(surface));
    return true;
  }
  return false;
}
void Display::DeleteSurface(SurfacePtr surface) {
  if (surface) {
    std::lock_guard<std::mutex> guard{mtx_};
    if (auto it = std::find(surfaces_.cbegin(), surfaces_.cend(), surface);
        it != surfaces_.cend()) {
      surfaces_.erase(it);
    }
  }
}

const SurfacePtr &Display::FindSurfaceByEgl(EGLSurface egl_surf) {
  static SurfacePtr kNotFoundSurface;
  std::lock_guard<std::mutex> guard{mtx_};
  if (auto it = std::find_if(surfaces_.begin(), surfaces_.end(),
                             [egl_surf](const SurfacePtr &surface) {
                               return surface.get() == egl_surf;
                             });
      it != surfaces_.end()) {
    return *it;
  }
  return kNotFoundSurface;
}

Surface *Display::FindSurface(EGLSurface egl_surf) {
  return FindSurfaceByEgl(egl_surf).get();
}

Image *Display::CreateImage(EGLContext ctx, EGLenum target,
                            EGLClientBuffer buffer,
                            const EGLAttrib *attrib_list) {
  const std::set<EGLenum> kGlImageTargets{
      // EGL_KHR_gl_texture_2D_image
      EGL_GL_TEXTURE_2D_KHR,
      // EGL_KHR_gl_texture_3D_image
      EGL_GL_TEXTURE_3D_KHR,
      // EGL_KHR_gl_renderbuffer_image
      EGL_GL_RENDERBUFFER_KHR,
      // EGL_KHR_gl_texture_cubemap_image
      EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR,
      EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR,
      EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR,
      EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR,
      EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR,
      EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR,
  };

  ImagePtr image;
  auto dpy = egl_dpy_.get();
  if (target == EGL_NATIVE_BUFFER_ANDROID) {
    // EGL_ANDROID_image_native_buffer
    auto native_buffer = reinterpret_cast<ANativeWindowBuffer *>(buffer);
    image = std::make_shared<AndroidBufferImage>(dpy, proxy_, native_buffer,
                                                 attrib_list);
  } else if (kGlImageTargets.find(target) != kGlImageTargets.end()) {
    auto name = static_cast<GLuint>(reinterpret_cast<uintptr_t>(buffer));
    image = std::make_shared<GlBufferImage>(dpy, proxy_, target, name, ctx,
                                            attrib_list);
  } else {
    // EGL_LINUX_DMA_BUF_EXT or EGL_WAYLAND_BUFFER_WL
    image = std::make_shared<PassthroughImage>(dpy, proxy_, target, buffer, ctx,
                                               attrib_list);
  }
  if (auto egl_image = image->CreateImage(); egl_image) {
    image_manager_.AddImage(image);
    return image.get();
  }
  return nullptr;
}

EGLBoolean Display::DestroyImage(EGLImageKHR img) {
  if (auto image = image_manager_.FindImage(img); image) {
    if (image->DestroyImage()) {
      image_manager_.DeleteImage(img);
      return EGL_TRUE;
    }
  }
  return EGL_FALSE;
}

Image *Display::CreateImageKHR(EGLContext ctx, EGLenum target,
                               EGLClientBuffer buffer,
                               const EGLint *attrib_list) {
  auto attribs = misc::ConvertIntToAttrib(attrib_list);
  return CreateImage(ctx, target, buffer, attribs.data());
}
EGLBoolean Display::DestroyImageKHR(EGLImageKHR img) {
  return DestroyImage(img);
}

EGLBoolean Display::DestroySurface(EGLSurface egl_surf) {
  if (auto const &surface = FindSurfaceByEgl(egl_surf); surface) {
    if (surface->DestroySurface()) {
      DeleteSurface(surface);
      return EGL_TRUE;
    }
  }
  return EGL_FALSE;
}

EGLBoolean Display::QuerySurface(EGLSurface egl_surf, EGLint attribute,
                                 EGLint *value) {
  if (auto surface = FindSurface(egl_surf); surface) {
    return surface->QuerySurface(attribute, value);
  }
  return EGL_FALSE;
}

EGLBoolean Display::SwapBuffers(EGLSurface egl_surf) {
  if (egl_surf == EGL_NO_SURFACE) {
    return proxy_->Api().eglSwapBuffers(egl_dpy_.get(), EGL_NO_SURFACE);
  }
  if (auto surface = FindSurface(egl_surf); surface) {
    return surface->SwapBuffers();
  }
  return EGL_FALSE;
}

EGLBoolean Display::ChooseConfig(const EGLint *attrib_list, EGLConfig *configs,
                                 EGLint config_size, EGLint *num_config) {
  auto const &api = proxy_->Api();
  auto dpy = egl_dpy_.get();
  return api.eglChooseConfig(dpy, attrib_list, configs, config_size,
                             num_config);
}

EGLBoolean Display::GetConfigAttrib(EGLConfig config, EGLint attribute,
                                    EGLint *value) {
  auto const &api = proxy_->Api();
  auto dpy = egl_dpy_.get();
  return api.eglGetConfigAttrib(dpy, config, attribute, value);
}

EGLDisplay AndroidDisplay::GetPlatformDisplay(void *native_display,
                                              const EGLAttrib *attrib_list) {
  assert(proxy_);
  if (native_display != EGL_DEFAULT_DISPLAY ||
      (attrib_list && attrib_list[0] != EGL_NONE)) {
    return EGL_NO_DISPLAY;
  }
  auto egl_dpy = proxy_->Api().eglGetPlatformDisplay(
      EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
  SetEglDisplay(egl_dpy);
  return egl_dpy;
}

Surface *AndroidDisplay::CreatePlatformWindowSurface(
    EGLConfig config, void *native_window, const EGLAttrib *attrib_list) {
  if (auto window = reinterpret_cast<ANativeWindow *>(native_window); window) {
    auto surface =
        std::make_shared<WindowSurface>(GetEglDisplay(), proxy_, window);
    if (auto egl_surf = surface->CreateSurface(config, attrib_list);
        egl_surf != EGL_NO_SURFACE && AddSurface(surface)) {
      return surface.get();
    }
  }
  return nullptr;
}

EGLBoolean AndroidDisplay::ChooseConfig(const EGLint *attrib_list,
                                        EGLConfig *configs, EGLint config_size,
                                        EGLint *num_config) {
  auto const &api = proxy_->Api();
  auto dpy = egl_dpy_.get();
  if (!attrib_list || attrib_list[0] == EGL_NONE) {
    return api.eglChooseConfig(dpy, attrib_list, configs, config_size,
                               num_config);
  }
  auto attribs = egl::misc::DupAttributes(attrib_list);

  EGLint native_visual_id = -1;
  bool has_surface_type = false;
  for (auto it = attribs.begin(); *it != EGL_NONE; it += 2) {
    if (*it == EGL_SURFACE_TYPE) {
      auto &type = *std::next(it);
      if ((type & EGL_WINDOW_BIT) != 0) {
        type &= ~EGL_WINDOW_BIT;
        type &= ~EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
        type |= EGL_PBUFFER_BIT;
        has_surface_type = true;
      }
    } else if (*it == EGL_NATIVE_VISUAL_ID) {
      native_visual_id = *std::next(it);
    }
  }

  if (auto mismatch = misc::CheckAndFilterSpecialAttributes(
          attribs.data(), kAndroidSpecialAttributes);
      mismatch) {
    auto supported_value = kAndroidSpecialAttributes.find(*mismatch)->second;
    ALOGD("eglChooseConfig attributes 0x%04X is 0x%08X, only support 0x%08X",
          *mismatch, *(mismatch + 1), supported_value);
    return EGL_FALSE;
  }

  if (native_visual_id != -1) {
    HalPixelFormat pixel_format{native_visual_id};
    std::map<EGLint, EGLint> checked_attributes{
        {EGL_RED_SIZE, pixel_format.RedSize()},
        {EGL_GREEN_SIZE, pixel_format.GreenSize()},
        {EGL_BLUE_SIZE, pixel_format.BlueSize()},
        {EGL_ALPHA_SIZE, pixel_format.AlphaSize()},
    };
    if (auto mismatch = misc::CheckAndFilterSpecialAttributes(
            attribs.data(), checked_attributes);
        mismatch) {
      // EGL_BAD_MATCH
      return EGL_FALSE;
    }

    std::vector<GLint> size_config{
        EGL_RED_SIZE,   pixel_format.RedSize(),
        EGL_GREEN_SIZE, pixel_format.GreenSize(),
        EGL_BLUE_SIZE,  pixel_format.BlueSize(),
        EGL_ALPHA_SIZE, pixel_format.AlphaSize(),
    };
    misc::ApendAttributes(attribs.data(), size_config);
    std::swap(attribs, size_config);
  }

  if (!has_surface_type) {
    const EGLint surface_config[] = {
        EGL_SURFACE_TYPE,
        EGL_PBUFFER_BIT,
        EGL_NONE,
    };
    misc::ApendAttributes(surface_config, attribs);
  }

  auto ret = api.eglChooseConfig(dpy, attribs.data(), configs, config_size,
                                 num_config);
  if (!ret) {
    ALOGD("eglChooseConfig %s from attributes : %s",
          proxy_->StrLastError().c_str(),
          misc::StringifyAttributes(attrib_list).c_str());
  }
  return ret;
}

EGLBoolean AndroidDisplay::GetConfigAttrib(EGLConfig config, EGLint attribute,
                                           EGLint *value) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (attribute == EGL_NATIVE_VISUAL_TYPE ||
      attribute == EGL_NATIVE_VISUAL_ID) {
    HalPixelFormat pixel_format;
    if (!GetFormatSizeFromConfig(config, pixel_format)) {
      // EGL_BAD_ATTRIBUTE
      return EGL_FALSE;
    }
    *value = pixel_format.PixelFormat();
    return EGL_TRUE;
  } else if (attribute == EGL_SURFACE_TYPE) {
    *value = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;
  }
  if (auto ret = Display::GetConfigAttrib(config, attribute, value); ret) {
    return EGL_TRUE;
  } else if (!ret && api.eglGetError() == EGL_BAD_ATTRIBUTE) {
    if (auto it = kAndroidSpecialAttributes.find(attribute);
        it != kAndroidSpecialAttributes.end()) {
      *value = it->second;
      return EGL_TRUE;
    }
  }
  ALOGD("eglGetConfigAttrib failed from attribute : 0x%04X", attribute);
  return EGL_FALSE;
}

EGLBoolean AndroidDisplay::GetFormatSizeFromConfig(
    EGLConfig config, HalPixelFormat &pixel_format) {
  GLint red_size;
  GLint green_size;
  GLint blue_size;
  GLint alpha_size;

  if (!(Display::GetConfigAttrib(config, EGL_RED_SIZE, &red_size) &&
        Display::GetConfigAttrib(config, EGL_GREEN_SIZE, &green_size) &&
        Display::GetConfigAttrib(config, EGL_BLUE_SIZE, &blue_size) &&
        Display::GetConfigAttrib(config, EGL_ALPHA_SIZE, &alpha_size))) {
    return EGL_FALSE;
  }
  if (!pixel_format.BuildFormat(red_size, green_size, blue_size, alpha_size)) {
    return EGL_FALSE;
  }
  return EGL_TRUE;
}

EGLDisplay GbmDisplay::GetPlatformDisplay(void *native_display,
                                          const EGLAttrib *attrib_list) {
  assert(proxy_);
  auto egl_dpy = proxy_->Api().eglGetPlatformDisplay(
      EGL_PLATFORM_GBM_KHR, native_display, attrib_list);
  SetEglDisplay(egl_dpy);
  return egl_dpy;
}

Surface *GbmDisplay::CreatePlatformWindowSurface(EGLConfig config,
                                                 void *native_window,
                                                 const EGLAttrib *attrib_list) {
  auto surface = std::make_shared<PassthroughSurface>(GetEglDisplay(), proxy_,
                                                      native_window);
  if (auto egl_surf = surface->CreateSurface(config, attrib_list);
      egl_surf != EGL_NO_SURFACE) {
    if (AddSurface(surface)) {
      return surface.get();
    }
  }
  return nullptr;
}

}  // namespace egl