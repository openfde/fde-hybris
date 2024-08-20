
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <cassert>
#include <string>

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

extern "C" {

// EGL 1.0
extern EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id);
extern EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
HYBRIS_IMPLEMENT_FUNCTION4(EGLContext, eglCreateContext, EGLDisplay, EGLConfig,
                           EGLContext, const EGLint *);
extern EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
                                EGLint config_size, EGLint *num_config);
HYBRIS_IMPLEMENT_FUNCTION5(EGLBoolean, eglChooseConfig, EGLDisplay,
                           const EGLint *, EGLConfig *, EGLint, EGLint *);
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

EGLDisplay (*s_eglGetPlatformDisplay)(EGLenum, void *, const EGLAttrib *);
static EGLDisplay eglGetPlatformDisplayAdapter(EGLenum platform,
                                               void *native_display,
                                               const EGLAttrib *attrib_list) {
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
  }

  if (real_native_display != native_display && real_native_display) {
    gbm_device_destroy(reinterpret_cast<gbm_device *>(real_native_display));
  }

  return EGL_NO_DISPLAY;
}
} // namespace

EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) {
  if (display_id != EGL_DEFAULT_DISPLAY) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  return eglGetPlatformDisplayAdapter(EGL_PLATFORM_GBM_KHR, EGL_DEFAULT_DISPLAY,
                                      nullptr);
}

EGLDisplay eglGetPlatformDisplay(EGLenum platform, void *native_display,
                                 const EGLAttrib *attrib_list) {
  if (platform == EGL_NONE) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  if (platform == EGL_PLATFORM_ANDROID_KHR &&
      native_display != EGL_DEFAULT_DISPLAY) {
    display::SetDisplayError(EGL_BAD_MATCH);
    return EGL_NO_DISPLAY;
  }

  EGLenum real_platform =
      (platform == EGL_PLATFORM_ANDROID_KHR) ? EGL_PLATFORM_GBM_KHR : platform;

  return eglGetPlatformDisplayAdapter(real_platform, native_display,
                                      attrib_list);
}

static EGLBoolean (*s_eglInitialize)(EGLDisplay, EGLint *, EGLint *) = {};
EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
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

EGLBoolean (*s_eglGetConfigs)(EGLDisplay dpy, EGLConfig *configs,
                              EGLint config_size, EGLint *num_config) = {};
EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size,
                         EGLint *num_config) {
  HYBRIS_DLSYM(eglGetConfigs);
  if (!s_eglGetConfigs) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_FALSE;
  }
  if (auto ret = s_eglGetConfigs(dpy, configs, config_size, num_config);
      ret && config_size > 0) {
    for (intptr_t config = 0; config < *num_config; config++) {
      // TODO: HAL_PIXEL_FORMAT_RGBA_8888 / HAL_PIXEL_FORMAT_RGBX_8888 /
      // HAL_PIXEL_FORMAT_RGB_565
      //  uint32_t format; if
      // (getConfigNativePixelFormat(config, &format)) {
      //   setConfigAttrib(config, EGL_NATIVE_VISUAL_ID, format);
      // }
    }
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

EGLBoolean (*s_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface,
                               EGLContext) = {};
EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read,
                          EGLContext ctx) {
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
EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
                                   const EGLint *attrib_list) {
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

static EGLSurface (*s_eglCreateWindowSurface)(EGLDisplay, EGLConfig,
                                              EGLNativeWindowType,
                                              const EGLint *) = {};
EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                  EGLNativeWindowType win,
                                  const EGLint *attrib_list) {
  HYBRIS_DLSYM(eglCreateWindowSurface);
  assert(s_eglCreateWindowSurface);

  auto display_info = display::FindDispayInfo(dpy);
  if (display_info) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_NO_SURFACE;
  }
  auto egl_surface = new EglSurface{win};
  // TODO:
  EGLint width = egl_surface->GetWidth(); // get from ANativeWindow
  EGLint height = egl_surface->GetHeight();
  // EGLint format = egl_surface->GetFormat();
  // EGLint flags = egl_surface->GetFlags();
  const EGLint pbuf_attribs[] = {EGL_WIDTH, width, EGL_HEIGHT, height,
                                 EGL_NONE};
  if (auto surface = s_eglCreatePbufferSurface(dpy, config, pbuf_attribs);
      surface == EGL_NO_SURFACE) {
    delete egl_surface;
    return EGL_NO_SURFACE;
  } else {
    egl_surface->SetReal(surface);
    EglSurface::InsertSurface(egl_surface);
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

EGLSurface eglCreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config,
                                          void *native_pixmap,
                                          const EGLAttrib *attrib_list) {
  display::SetDisplayError(EGL_NOT_INITIALIZED);
  assert(false);
  return EGL_NO_SURFACE;
}

static EGLBoolean (*s_eglDestroySurface)(EGLDisplay, EGLSurface) = {};
EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
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
EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
                              EGLint attribute, EGLint *value) {
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
EGLSurface eglGetCurrentSurface(EGLint readdraw) {
  HYBRIS_DLSYM(eglGetCurrentSurface);
  assert(s_eglGetCurrentSurface);
  auto real_surface = s_eglGetCurrentSurface(readdraw);
  return EglSurface::FindSurface(real_surface);
}

static EGLint (*s_eglGetError)();
EGLint eglGetError(void) {
  HYBRIS_DLSYM(eglGetError);
  assert(s_eglGetError);
  if (auto error = s_eglGetError(); error != EGL_SUCCESS) {
    return error;
  }
  return display::GetDisplayError();
}

static __eglMustCastToProperFunctionPointerType (*s_eglGetProcAddress)(
    const char *) = {};

__eglMustCastToProperFunctionPointerType
eglGetProcAddress(const char *procname) {
  HYBRIS_DLSYM(eglGetProcAddress);
  assert(s_eglGetProcAddress);

  if (strcmp(procname, "eglDupNativeFenceFDANDROID") == 0) {
    // TODO: EGL_ANDROID_native_fence_sync extension
    return nullptr;
  }

  return s_eglGetProcAddress(procname);
}

static const char *(*s_eglQueryString)(EGLDisplay, EGLint);
const char *eglQueryString(EGLDisplay dpy, EGLint name) {
  HYBRIS_DLSYM(eglQueryString);
  assert(s_eglQueryString);
  // list of extensions supported by this EGL implementation
  //  NOTE that each extension name should be suffixed with space
  static const char kStaticEglExtensions[] = "EGL_ANDROID_image_native_buffer "
                                             "EGL_KHR_fence_sync "
                                             "EGL_KHR_wait_sync "
                                             "EGL_KHR_image_base "
                                             "EGL_KHR_gl_texture_2d_image ";
  auto strs = s_eglQueryString(dpy, name);
  if (name != EGL_EXTENSIONS) {
    static char eglextensionsbuf[2048];
    snprintf(eglextensionsbuf, 2046, "%s%s", kStaticEglExtensions,
             strs ? strs : "");
    strs = eglextensionsbuf;
  }

  // extensions to add dynamically depending on host-side support
  // static const char kDynamicEglExtNativeSync[] =
  //     "EGL_ANDROID_native_fence_sync ";
  // TODO:

  return strs;
}

static EGLBoolean (*s_eglQuerySurface)(EGLDisplay, EGLSurface, EGLint,
                                       EGLint *) = nullptr;
EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute,
                           EGLint *value) {
  HYBRIS_DLSYM(eglQuerySurface);
  assert(s_eglQuerySurface);
  auto real_surface = surface;
  if (surface != EGL_NO_SURFACE) {
    auto egl_surface = reinterpret_cast<EglSurface *>(surface);
    real_surface = egl_surface->GetSurface();
  }
  return s_eglQuerySurface(dpy, real_surface, attribute, value);
}

static EGLBoolean (*s_eglSwapBuffers)(EGLDisplay, EGLSurface) = {};
EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
  HYBRIS_DLSYM(eglSwapBuffers);
  assert(s_eglSwapBuffers);
  if (surface == EGL_NO_SURFACE) {
    return s_eglSwapBuffers(dpy, surface);
  }
  auto egl_surface = reinterpret_cast<EglSurface *>(surface);
  auto real_surface = egl_surface->GetSurface();
  if (auto ret = s_eglSwapBuffers(dpy, real_surface); ret) {
    // TODO: copy

    // TODO: sync
    egl_surface->QueueBuffer();

    //
    egl_surface->DequeueBuffer();
  }
  return EGL_FALSE;
}

static EGLBoolean (*s_eglTerminate)(EGLDisplay) = {};
EGLBoolean eglTerminate(EGLDisplay dpy) {
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
EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
  HYBRIS_DLSYM(eglBindTexImage);
  assert(s_eglBindTexImage);

  if (auto ret = s_eglBindTexImage(dpy, surface, buffer); ret) {
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface,
                              EGLint buffer);
EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface,
                            EGLint attribute, EGLint value);
EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval);
