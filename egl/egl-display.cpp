#include "egl-display.h"

#include <android/native_window.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vndk/window.h>

#include <algorithm>
#include <atomic>
#include <cassert>

#include "egl-misc.h"
#include "egl-proxy.h"
#include "gbm.h"

namespace {
constexpr char kGbmDevicePath[] = "/dev/dri/renderD128";
}

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
    return GetPlatformDisplay(EGL_PLATFORM_ANDROID_KHR, EGL_NO_DISPLAY,
                              nullptr);
  }

  return nullptr;
}

Display *DisplayManager::GetPlatformDisplay(EGLenum platform,
                                            void *native_display,
                                            const EGLAttrib *attrib_list) {
  auto proxy = EglProxy::Instance();
  if (!proxy) {
    return nullptr;
  }
  Display *display = {};
  if (platform == EGL_PLATFORM_ANDROID_KHR) {
    if (auto gbm = NewGbmDevice(); gbm) {
      display = new AndroidDisplay{proxy, gbm};
    }
  } else if (platform == EGL_PLATFORM_GBM_KHR) {
    display = new GbmDisplay{proxy};
  }
  if (display) {
    if (auto egl_dpy = display->GetPlatformDisplay(native_display, attrib_list);
        egl_dpy != EGL_NO_DISPLAY) {
      std::lock_guard<std::mutex> lock{mtx_};
      if (auto it = FindIdleSlot(); it != displays_.end()) {
        *it = display;
        return display;
      }
    }
    delete display;
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
    auto dpy = *it;
    *it = nullptr;
    delete dpy;
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

Display *DisplayManager::FindDispay(EGLDisplay egl_dpy) {
  if (auto it = FindDisplayPos(egl_dpy); it != displays_.end()) {
    return *it;
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

GbmDevicePtr DisplayManager::NewGbmDevice() {
  if (auto fd = open(kGbmDevicePath, O_RDWR | O_CLOEXEC); fd >= 0) {
    if (auto gbm = gbm_create_device(fd); gbm) {
      return GbmDevicePtr{gbm, [](auto dev) { gbm_device_destroy(dev); }};
    }
  }
  return nullptr;
}

DisplayManager::DisplayIterator DisplayManager::FindIdleSlot() {
  return std::find_if(displays_.begin(), displays_.end(),
                      [](auto &display) { return !display; });
}

const char *Display::GetEglExtensions() {
  constexpr char kNativeBufferExtensions[] = "EGL_ANDROID_image_native_buffer";
  if (!inited_extensions_) {
    std::lock_guard<std::mutex> guard{mtx_};
    if (!inited_extensions_) {
      inited_extensions_ = true;
      auto strs = proxy_->Api().eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
      if (!strs) {
        return nullptr;
      }
      // EGL_ANDROID_framebuffer_target ???
      if (strstr(strs, "EGL_EXT_image_dma_buf_import")) {
        // not support EGL_ANDROID_create_native_client_buffer and
        // EGL_ANDROID_get_native_client_buffer
        extensions_ += kNativeBufferExtensions;
        extensions_ += ' ';
      }

      // EGL_KHR_fence_sync, EGL_KHR_image_base and EGL_KHR_gl_texture_2d_image
      // extensions
      extensions_ += strs;
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

void Display::SetEglDisplay(EGLDisplay egl_dpy) {
  if (egl_dpy) {
    egl_dpy_ = std::shared_ptr<void>(egl_dpy, [this](EGLDisplay dpy) {
      if (proxy_) {
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

Surface *Display::FindSurface(EGLSurface egl_surf) {
  std::lock_guard<std::mutex> guard{mtx_};
  if (auto it = std::find_if(surfaces_.begin(), surfaces_.end(),
                             [egl_surf](const SurfacePtr &surface) {
                               return surface->GetEglSurface() == egl_surf;
                             });
      it != surfaces_.end()) {
    return it->get();
  }
  return nullptr;
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
    image = std::make_shared<AndroidBufferImage>(dpy, proxy_, native_buffer);
  } else if (kGlImageTargets.find(target) != kGlImageTargets.end()) {
    auto name = static_cast<GLuint>(reinterpret_cast<uintptr_t>(buffer));
    image = std::make_shared<GlBufferImage>(dpy, proxy_, target, name);
  } else {
    // EGL_LINUX_DMA_BUF_EXT or EGL_WAYLAND_BUFFER_WL
    image = std::make_shared<PassthroughImage>(dpy, proxy_, target, buffer);
  }
  if (auto egl_image = image->CreateImage(ctx, attrib_list); egl_image) {
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
  if (auto surface = FindSurface(egl_surf); surface) {
    return surface->DestroySurface();
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

EGLDisplay AndroidDisplay::GetPlatformDisplay(void *native_display,
                                              const EGLAttrib *attrib_list) {
  assert(proxy_);
  assert(gbm_);
  if (native_display || (attrib_list && attrib_list[0] != EGL_NONE)) {
    return EGL_NO_DISPLAY;
  }
  auto egl_dpy = proxy_->Api().eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR,
                                                     gbm_.get(), nullptr);
  SetEglDisplay(egl_dpy);
  return egl_dpy;
}

Surface *AndroidDisplay::CreatePlatformWindowSurface(
    EGLConfig config, void *native_window, const EGLAttrib *attrib_list) {
  if (auto window = reinterpret_cast<ANativeWindow *>(native_window); window) {
    auto surface = std::make_shared<WindowSurface>(GetEglDisplay(), proxy_,
                                                   window, gbm_.get());
    if (auto egl_surf = surface->CreateSurface(config, attrib_list);
        egl_surf != EGL_NO_SURFACE && AddSurface(surface)) {
      return surface.get();
    }
  }
  return nullptr;
}

EGLBoolean AndroidDisplay::eglChooseConfig(const EGLint *attrib_list,
                                           EGLConfig *configs,
                                           EGLint config_size,
                                           EGLint *num_config) {
  auto const &api = proxy_->Api();
  auto dpy = egl_dpy_.get();
  if (!attrib_list || attrib_list[0] == EGL_NONE) {
    return api.eglChooseConfig(dpy, attrib_list, configs, config_size,
                               num_config);
  }
  auto attribs = egl::misc::DupAttributes(attrib_list);

  for (auto it = attribs.begin(); *it != EGL_NONE; it += 2) {
    if (*it == EGL_SURFACE_TYPE) {
      // auto &type = *std::next(it);
      // type &= ~EGL_WINDOW_BIT;
      // type |= EGL_PBUFFER_BIT;
    } else if (*it == EGL_NATIVE_VISUAL_ID) {
      *std::next(it) = egl::misc::GetGbmFormatFromHalFormat(*std::next(it));
    } else if (*it == EGL_NATIVE_VISUAL_TYPE) {
      *std::next(it) = EGL_DONT_CARE;
    }
  }
  return api.eglChooseConfig(dpy, attribs.data(), configs, config_size,
                             num_config);
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

EGLBoolean GbmDisplay::eglChooseConfig(const EGLint *attrib_list,
                                       EGLConfig *configs, EGLint config_size,
                                       EGLint *num_config) {
  auto const &api = proxy_->Api();
  auto dpy = egl_dpy_.get();
  return api.eglChooseConfig(dpy, attrib_list, configs, config_size,
                             num_config);
}

}  // namespace egl