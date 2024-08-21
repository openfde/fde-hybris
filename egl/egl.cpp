
#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include <vndk/window.h>

#include "display.h"
#include "egl-surface.h"
#include "gbm.h"

#define HYBRIS_LIBNAME "libEGL.so"
#define HYBRIS_ENVNAME "HYBRIS-EGL"
#include "binding.h"

// static void *hybris_initialize() {
//   const char *name = HYBRIS_LIBNAME;
// #ifdef HYBRIS_ENVNAME
//   const char *env_name = getenv(HYBRIS_ENVNAME);
//   name = env_name ? env_name : name;
// #endif
//   return dlopen(name, RTLD_GLOBAL);
// }

static void *hybris_dlsym(void **fptr, const char *sym) {
  if (*fptr == NULL) {
    void *handle = hybris_open_library();
    *fptr = dlsym(handle, sym);
  }
  return *fptr;
}

#define HYBRIS_DLSYM(sym) hybris_dlsym((void **)&s_##sym, #sym)

extern "C" {

// EGL 1.0
extern EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id);
extern EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
HYBRIS_IMPLEMENT_FUNCTION4(EGLContext, eglCreateContext, EGLDisplay, EGLConfig,
                           EGLContext, const EGLint *);
extern EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
                                EGLint config_size, EGLint *num_config);
extern EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
                                  EGLConfig *configs, EGLint config_size,
                                  EGLint *num_config);
extern EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
                                 EGLSurface read, EGLContext ctx);

extern EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                         EGLNativeWindowType win,
                                         const EGLint *attrib_list);
extern EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
                                          const EGLint *attrib_list);
extern EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config,
                                         EGLNativePixmapType pixmap,
                                         const EGLint *attrib_list);

HYBRIS_IMPLEMENT_FUNCTION3(EGLBoolean, eglCopyBuffers, EGLDisplay, EGLSurface,
                           EGLNativePixmapType);

HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglDestroyContext, EGLDisplay,
                           EGLContext);
extern EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
extern EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
                                     EGLint attribute, EGLint *value);
HYBRIS_IMPLEMENT_FUNCTION0(EGLDisplay, eglGetCurrentDisplay);

extern EGLSurface eglGetCurrentSurface(EGLint readdraw);

extern EGLint eglGetError(void);
extern __eglMustCastToProperFunctionPointerType
eglGetProcAddress(const char *procname);
HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglQueryContext, EGLDisplay, EGLContext,
                           EGLint, EGLint *);
extern const char *eglQueryString(EGLDisplay dpy, EGLint name);
extern EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                                  EGLint attribute, EGLint *value);
extern EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
extern EGLBoolean eglTerminate(EGLDisplay dpy);

HYBRIS_IMPLEMENT_FUNCTION0(EGLBoolean, eglWaitGL);
HYBRIS_IMPLEMENT_FUNCTION1(EGLBoolean, eglWaitNative, EGLint);

// EGL 1.1
extern EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface,
                                  EGLint buffer);
extern EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface,
                                     EGLint buffer);
extern EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface,
                                   EGLint attribute, EGLint value);
extern EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval);

// EGL_KHR_fence_sync extension requires EGL 1.1
extern EGLSyncKHR eglCreateSyncKHR(EGLDisplay dpy, EGLenum type,
                                   const EGLint *attrib_list);

extern EGLBoolean eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync);

extern EGLint eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync,
                                   EGLint flags, EGLTimeKHR timeout);

extern EGLBoolean eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR sync,
                                      EGLint attribute, EGLint *value);

// EGL 1.2
HYBRIS_IMPLEMENT_FUNCTION1(EGLBoolean, eglBindAPI, EGLenum);
HYBRIS_IMPLEMENT_FUNCTION0(EGLenum, eglQueryAPI);
extern EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy,
                                                   EGLenum buftype,
                                                   EGLClientBuffer buffer,
                                                   EGLConfig config,
                                                   const EGLint *attrib_list);

HYBRIS_IMPLEMENT_FUNCTION0(EGLBoolean, eglReleaseThread);
HYBRIS_IMPLEMENT_FUNCTION0(EGLBoolean, eglWaitClient);

// EGL_KHR_image_base extension requires EGL 1.2
extern EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                                     EGLenum target, EGLClientBuffer buffer,
                                     const EGLint *attrib_list);
extern EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR img);

// EGL 1.4
HYBRIS_IMPLEMENT_FUNCTION0(EGLContext, eglGetCurrentContext);

// EGL 1.5
extern EGLSync eglCreateSync(EGLDisplay dpy, EGLenum type,
                             const EGLAttrib *attrib_list);
extern EGLBoolean eglDestroySync(EGLDisplay dpy, EGLSync sync);
extern EGLint eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags,
                                EGLTime timeout);
extern EGLBoolean eglGetSyncAttrib(EGLDisplay dpy, EGLSync sync,
                                   EGLint attribute, EGLAttrib *value);
extern EGLImage eglCreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                               EGLClientBuffer buffer,
                               const EGLAttrib *attrib_list);
extern EGLBoolean eglDestroyImage(EGLDisplay dpy, EGLImage image);
extern EGLDisplay eglGetPlatformDisplay(EGLenum platform, void *native_display,
                                        const EGLAttrib *attrib_list);
extern EGLSurface eglCreatePlatformWindowSurface(EGLDisplay dpy,
                                                 EGLConfig config,
                                                 void *native_window,
                                                 const EGLAttrib *attrib_list);
extern EGLSurface eglCreatePlatformPixmapSurface(EGLDisplay dpy,
                                                 EGLConfig config,
                                                 void *native_pixmap,
                                                 const EGLAttrib *attrib_list);
extern EGLBoolean eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags);
}

namespace {

EGLDisplay eglGetPlatformDisplayAdapter(EGLenum platform, void *native_display,
                                        const EGLAttrib *attrib_list) {
  static EGLDisplay (*s_eglGetPlatformDisplay)(EGLenum, void *,
                                               const EGLAttrib *) = {};
  HYBRIS_DLSYM(eglGetPlatformDisplay);
  if (!s_eglGetPlatformDisplay) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  auto real_platform = platform;
  auto real_native_display = native_display;
  if (platform == EGL_PLATFORM_ANDROID_KHR) {
    assert(native_display == EGL_DEFAULT_DISPLAY);
    real_native_display = display::NewGbmDevice();
    real_platform = EGL_PLATFORM_GBM_KHR;
  }
  if (auto egl_display = s_eglGetPlatformDisplay(
          real_platform, real_native_display, attrib_list);
      egl_display != EGL_NO_DISPLAY) {
    display::InsertDisplayInfo(egl_display, real_native_display);
  } else if (real_native_display != native_display && real_native_display) {
    gbm_device_destroy(reinterpret_cast<gbm_device *>(real_native_display));
  }

  return EGL_NO_DISPLAY;
}

// int32_t EglAttrbCount(const EGLint *attrib_list) {
//   if (!attrib_list) {
//     return 0;
//   }
//   auto begin = attrib_list;
//   while (*attrib_list != EGL_NONE) {
//     attrib_list += 2;
//   }
//   return std::distance(begin, attrib_list) / 2;
// }

void EGLAppendAttribs(const EGLint *attrib_list, std::vector<EGLint> &attribs) {
  if (!attribs.empty() && attribs.back() == EGL_NONE) {
    attribs.pop_back();
  }
  if (attrib_list) {
    while (*attrib_list != EGL_NONE) {
      attribs.push_back(attrib_list[0]);
      attribs.push_back(attrib_list[1]);
      attrib_list += 2;
    }
  }
  attribs.push_back(EGL_NONE);
}

std::vector<EGLint> EglCopyAttribs(const EGLint *attrib_list) {
  std::vector<EGLint> attribs;
  EGLAppendAttribs(attrib_list, attribs);
  return attribs;
}

EGLClientBuffer eglCreateNativeClientBufferANDROID(const EGLint *attrib_list) {
  (void)attrib_list;
  assert(false);
  return nullptr;
}

EGLClientBuffer
eglGetNativeClientBufferANDROID(const struct AHardwareBuffer *buffer) {
  (void)buffer;
  assert(false);
  return nullptr;
}

} // namespace

HYBRIS_VISIBILITY EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) {
  if (display_id != EGL_DEFAULT_DISPLAY) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  return eglGetPlatformDisplayAdapter(EGL_PLATFORM_GBM_KHR, EGL_DEFAULT_DISPLAY,
                                      nullptr);
}

HYBRIS_VISIBILITY EGLDisplay eglGetPlatformDisplay(
    EGLenum platform, void *native_display, const EGLAttrib *attrib_list) {
  if (platform == EGL_NONE) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  if (platform == EGL_PLATFORM_ANDROID_KHR &&
      native_display != EGL_DEFAULT_DISPLAY) {
    display::SetDisplayError(EGL_BAD_MATCH);
    return EGL_NO_DISPLAY;
  }

  return eglGetPlatformDisplayAdapter(platform, native_display, attrib_list);
}

HYBRIS_VISIBILITY EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major,
                                           EGLint *minor) {
  static EGLBoolean (*s_eglInitialize)(EGLDisplay, EGLint *, EGLint *) = {};
  HYBRIS_DLSYM(eglInitialize);
  if (!s_eglInitialize) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_FALSE;
  }
  if (auto ret = eglInitialize(dpy, major, minor); ret) {
    constexpr EGLint kSupportMaxMinor = 2;
    if (minor && *minor > kSupportMaxMinor) {
      *minor = kSupportMaxMinor;
    }
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
                                           EGLint config_size,
                                           EGLint *num_config) {
  static PFNEGLGETCONFIGSPROC s_eglGetConfigs = {};
  HYBRIS_DLSYM(eglGetConfigs);
  if (!s_eglGetConfigs) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_FALSE;
  }
  return s_eglGetConfigs(dpy, configs, config_size, num_config);
}

HYBRIS_VISIBILITY EGLBoolean eglChooseConfig(EGLDisplay dpy,
                                             const EGLint *attrib_list,
                                             EGLConfig *configs,
                                             EGLint config_size,
                                             EGLint *num_config) {
  static PFNEGLCHOOSECONFIGPROC s_eglChooseConfig = {};
  HYBRIS_DLSYM(eglChooseConfig);
  assert(s_eglChooseConfig);
  if (!attrib_list || attrib_list[0] == EGL_NONE) {
    return s_eglChooseConfig(dpy, attrib_list, configs, config_size,
                             num_config);
  }
  auto attribs = EglCopyAttribs(attrib_list);

  for (auto it = attribs.begin(); *it != EGL_NONE; it += 2) {
    if (*it == EGL_SURFACE_TYPE) {
      auto &type = *std::next(it);
      type &= ~EGL_WINDOW_BIT;
      type |= EGL_PBUFFER_BIT;
    }
  }
  return s_eglChooseConfig(dpy, attribs.data(), configs, config_size,
                           num_config);
}

HYBRIS_VISIBILITY EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
                                            EGLSurface read, EGLContext ctx) {
  static EGLBoolean (*s_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface,
                                        EGLContext) = {};
  EGLSurface real_draw = EGL_NO_SURFACE;
  EGLSurface real_read = EGL_NO_SURFACE;
  HYBRIS_DLSYM(eglMakeCurrent);
  assert(s_eglMakeCurrent);
  if (draw) {
    real_draw = reinterpret_cast<EglSurface *>(draw)->GetSurface();
  }
  if (read) {
    real_read = reinterpret_cast<EglSurface *>(read)->GetSurface();
  }
  return s_eglMakeCurrent(dpy, real_draw, real_read, ctx);
}

static EGLSurface (*s_eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config,
                                               const EGLint *attrib_list) = {};
HYBRIS_VISIBILITY EGLSurface eglCreatePbufferSurface(
    EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
  HYBRIS_DLSYM(eglCreatePbufferSurface);
  assert(s_eglCreatePbufferSurface);

  if (auto surface = s_eglCreatePbufferSurface(dpy, config, attrib_list);
      surface != EGL_NO_SURFACE) {
    auto egl_surface = new EglSurface{surface};
    EglSurface::InsertSurface(egl_surface);
    return egl_surface;
  }
  return EGL_NO_SURFACE;
}

static PFNEGLCREATEWINDOWSURFACEPROC s_eglCreateWindowSurface = {};

HYBRIS_VISIBILITY EGLSurface eglCreateWindowSurface(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    EGLNativeWindowType win,
                                                    const EGLint *attrib_list) {
  HYBRIS_DLSYM(eglCreateWindowSurface);
  assert(s_eglCreateWindowSurface);

  auto display_info = display::FindDispayInfo(dpy);
  if (!display_info) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_NO_SURFACE;
  }

  if (!win) {
    display::SetDisplayError(dpy, EGL_BAD_NATIVE_WINDOW);
    return EGL_NO_SURFACE;
  }

  auto egl_surface = new EglSurface{win};
  EGLint width = egl_surface->GetWidth(); // get from ANativeWindow
  EGLint height = egl_surface->GetHeight();
  // EGLint format = egl_surface->GetFormat();
  // EGLint flags = egl_surface->GetFlags();
  std::vector<EGLint> pbuf_attribs = {EGL_WIDTH, width, EGL_HEIGHT, height};
  EGLAppendAttribs(attrib_list, pbuf_attribs);

  if (auto surface = s_eglCreatePbufferSurface(dpy, config, &pbuf_attribs[0]);
      surface == EGL_NO_SURFACE) {
    delete egl_surface;
    return EGL_NO_SURFACE;
  } else {
    egl_surface->SetReal(surface);
    EglSurface::InsertSurface(egl_surface);
    egl_surface->DequeueBuffer();
  }

  // auto gbm_dev = reinterpret_cast<gbm_device *>(display_info->real_display);
  // auto real_surface = gbm_surface_create(nullptr, width, height, format,
  // flags); if (!real_surface) {
  //   delete egl_surface;
  //   display::SetDisplayError(EGL_BAD_PARAMETER);
  //   return EGL_NO_SURFACE;
  // }
  // assert(egl_surface);

  // if (auto surface = s_eglCreateWindowSurface(
  //         dpy, config, (EGLNativeWindowType)real_surface, attrib_list);
  //     surface == EGL_NO_SURFACE) {
  //   gbm_surface_destroy(real_surface);
  //   delete real_surface;
  //   return EGL_NO_SURFACE;
  // }
  // else{
  //   egl_surface->SetReal(surface, (EGLNativeWindowType)real_surface);
  // }

  return egl_surface;
}

HYBRIS_VISIBILITY EGLSurface eglCreatePlatformPixmapSurface(
    EGLDisplay dpy, EGLConfig config, void *native_pixmap,
    const EGLAttrib *attrib_list) {
  display::SetDisplayError(EGL_NOT_INITIALIZED);
  assert(false);
  return EGL_NO_SURFACE;
}

static EGLBoolean (*s_eglDestroySurface)(EGLDisplay, EGLSurface) = {};
HYBRIS_VISIBILITY EGLBoolean eglDestroySurface(EGLDisplay dpy,
                                               EGLSurface surface) {
  HYBRIS_DLSYM(eglDestroySurface);
  assert(s_eglDestroySurface);

  auto egl_surface = reinterpret_cast<EglSurface *>(surface);
  if (auto ret = s_eglDestroySurface(dpy, egl_surface->GetSurface());
      ret == EGL_TRUE) {
    egl_surface->CancelBuffer();
    EglSurface::RemoveSurface(egl_surface->GetSurface());
    delete egl_surface;
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

static EGLBoolean (*s_eglGetConfigAttrib)(EGLDisplay, EGLConfig, EGLint,
                                          EGLint *);
HYBRIS_VISIBILITY EGLBoolean eglGetConfigAttrib(EGLDisplay dpy,
                                                EGLConfig config,
                                                EGLint attribute,
                                                EGLint *value) {
  if (attribute == EGL_FRAMEBUFFER_TARGET_ANDROID) {
    *value = EGL_TRUE;
    return EGL_TRUE;
  }
  if (attribute == EGL_COVERAGE_SAMPLES_NV ||
      attribute == EGL_COVERAGE_BUFFERS_NV) {
    *value = 0;
    return EGL_TRUE;
  }
  if (attribute == EGL_DEPTH_ENCODING_NV) {
    *value = EGL_DEPTH_ENCODING_NONE_NV;
    return EGL_TRUE;
  }
  if (attribute == EGL_COLOR_COMPONENT_TYPE_EXT) {
    *value = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
    return EGL_TRUE;
  }
  HYBRIS_DLSYM(eglGetConfigAttrib);
  assert(s_eglGetConfigAttrib);
  return s_eglGetConfigAttrib(dpy, config, attribute, value);
}

static EGLSurface (*s_eglGetCurrentSurface)(EGLint) = {};
HYBRIS_VISIBILITY EGLSurface eglGetCurrentSurface(EGLint readdraw) {
  HYBRIS_DLSYM(eglGetCurrentSurface);
  assert(s_eglGetCurrentSurface);
  auto real_surface = s_eglGetCurrentSurface(readdraw);
  return EglSurface::FindSurface(real_surface);
}

HYBRIS_VISIBILITY EGLint eglGetError(void) {
  static EGLint (*s_eglGetError)();
  HYBRIS_DLSYM(eglGetError);
  assert(s_eglGetError);
  if (auto error = s_eglGetError(); error != EGL_SUCCESS) {
    return error;
  }
  return display::GetDisplayError();
}

HYBRIS_VISIBILITY __eglMustCastToProperFunctionPointerType
eglGetProcAddress(const char *procname) {
  static PFNEGLGETPROCADDRESSPROC s_eglGetProcAddress = {};
  HYBRIS_DLSYM(eglGetProcAddress);
  assert(s_eglGetProcAddress);

  if (strcmp(procname, "eglDupNativeFenceFDANDROID") == 0) {
    // not support EGL_ANDROID_native_fence_sync extension
    return nullptr;
  }

  if (strcmp(procname, "eglSetBlobCacheFuncsANDROID") == 0) {
    // not support EGL_ANDROID_blob_cache extension
    return nullptr;
  }

  if (strcmp(procname, "eglPresentationTimeANDROID") == 0) {
    // not support EGL_ANDROID_presentation_time extension
    return nullptr;
  }

  if (strcmp(procname, "eglCreateNativeClientBufferANDROID") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglCreateNativeClientBufferANDROID);
  }

  if (strcmp(procname, "eglGetNativeClientBufferANDROID") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglGetNativeClientBufferANDROID);
  }

  return s_eglGetProcAddress(procname);
}

HYBRIS_VISIBILITY const char *eglQueryString(EGLDisplay dpy, EGLint name) {
  static PFNEGLQUERYSTRINGPROC s_eglQueryString = {};
  HYBRIS_DLSYM(eglQueryString);
  assert(s_eglQueryString);

  auto strs = s_eglQueryString(dpy, name);
  if (name == EGL_EXTENSIONS) {
    thread_local std::string egl_string;
    if (strstr(strs, "EGL_EXT_image_dma_buf_import")) {
      static const char kNativeBufferExtensions[] =
          "EGL_ANDROID_image_native_buffer "
          "EGL_ANDROID_get_native_client_buffer "
          "EGL_ANDROID_create_native_client_buffer ";
      egl_string += kNativeBufferExtensions;
    }
    if (strs) {
      // EGL_KHR_fence_sync, EGL_KHR_image_base and EGL_KHR_gl_texture_2d_image
      // extensions
      egl_string += strs;
    }
    return egl_string.c_str();
  }

  return strs;
}

HYBRIS_VISIBILITY EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                                             EGLint attribute, EGLint *value) {
  static PFNEGLQUERYSURFACEPROC s_eglQuerySurface = {};
  HYBRIS_DLSYM(eglQuerySurface);
  assert(s_eglQuerySurface);
  auto real_surface = surface;
  if (surface == EGL_NO_SURFACE) {
    return s_eglQuerySurface(dpy, real_surface, attribute, value);
  }
  auto egl_surface = reinterpret_cast<EglSurface *>(surface);
  real_surface = egl_surface->GetSurface();
  auto ret = s_eglQuerySurface(dpy, real_surface, attribute, value);
  if (ret && egl_surface->IsWindow() && attribute == EGL_SURFACE_TYPE) {
    *value |= EGL_WINDOW_BIT;
  }
  return ret;
}

HYBRIS_VISIBILITY EGLBoolean eglSwapBuffers(EGLDisplay dpy,
                                            EGLSurface surface) {
  static PFNEGLSWAPBUFFERSPROC s_eglSwapBuffers = {};
  HYBRIS_DLSYM(eglSwapBuffers);
  assert(s_eglSwapBuffers);
  if (surface == EGL_NO_SURFACE) {
    return s_eglSwapBuffers(dpy, surface);
  }
  auto egl_surface = reinterpret_cast<EglSurface *>(surface);
  auto real_surface = egl_surface->GetSurface();
  if (!egl_surface->IsWindow()) {
    return s_eglSwapBuffers(dpy, real_surface);
  }

  // TODO: copy

  // TODO: sync
  egl_surface->QueueBuffer();

  //
  egl_surface->DequeueBuffer();

  return EGL_TRUE;
}

static EGLBoolean (*s_eglTerminate)(EGLDisplay) = {};
HYBRIS_VISIBILITY EGLBoolean eglTerminate(EGLDisplay dpy) {
  HYBRIS_DLSYM(eglTerminate);
  assert(s_eglTerminate);

  if (auto ret = s_eglTerminate(dpy); ret) {
    auto info = display::FindDispayInfo(dpy);
    display::RemoveDisplay(dpy);
    delete info;
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

static EGLBoolean (*s_eglBindTexImage)(EGLDisplay, EGLSurface, EGLint);
HYBRIS_VISIBILITY EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface,
                                             EGLint buffer) {
  HYBRIS_DLSYM(eglBindTexImage);
  assert(s_eglBindTexImage);

  if (auto ret = s_eglBindTexImage(dpy, surface, buffer); ret) {
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLBoolean eglReleaseTexImage(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLint buffer);
HYBRIS_VISIBILITY EGLBoolean eglSurfaceAttrib(EGLDisplay dpy,
                                              EGLSurface surface,
                                              EGLint attribute, EGLint value);
HYBRIS_VISIBILITY EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval);

HYBRIS_VISIBILITY EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                                                EGLenum target,
                                                EGLClientBuffer buffer,
                                                const EGLint *attrib_list);

HYBRIS_VISIBILITY EGLBoolean eglDestroyImageKHR(EGLDisplay dpy,
                                                EGLImageKHR img);
