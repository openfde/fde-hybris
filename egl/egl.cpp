
#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <log/log.h>
#include <system/graphics-base.h>
#include <vndk/window.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "egl-display.h"
#include "egl-misc.h"
#include "egl-proxy.h"
#include "egl-surface.h"
#include "u_gralloc/u_gralloc.h"

#define HYBRIS_GET_SYMBOL_ADDRESS(symbol) \
  ({ egl::EglProxy::Instance()->Api().symbol; })

#include "binding.h"

extern "C" {

// EGL 1.0
extern EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id);
extern EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
// HYBRIS_IMPLEMENT_FUNCTION3(EGLBoolean, eglInitialize, EGLDisplay, EGLint,
//                            EGLint);
HYBRIS_IMPLEMENT_FUNCTION4(EGLContext, eglCreateContext, EGLDisplay, EGLConfig,
                           EGLContext, const EGLint *);
HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglGetConfigs, EGLDisplay, EGLConfig,
                           EGLint, const EGLint *);
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
HYBRIS_IMPLEMENT_FUNCTION4(EGLSurface, eglCreatePixmapSurface, EGLDisplay,
                           EGLConfig, EGLNativePixmapType, const EGLint *);

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
extern __eglMustCastToProperFunctionPointerType eglGetProcAddress(
    const char *procname);
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
HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglSwapInterval, EGLDisplay, EGLint);

// EGL_KHR_fence_sync extension requires EGL 1.1
// HYBRIS_IMPLEMENT_FUNCTION3(EGLSyncKHR, eglCreateSyncKHR, EGLDisplay, EGLenum,
//                            const EGLint *);

// HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglDestroySyncKHR, EGLDisplay,
//                            EGLSyncKHR);

// HYBRIS_IMPLEMENT_FUNCTION4(EGLint, eglClientWaitSyncKHR, EGLDisplay,
// EGLSyncKHR,
//                            EGLint, EGLTimeKHR);

// HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglGetSyncAttribKHR, EGLDisplay,
//                            EGLSyncKHR, EGLint, EGLint *);

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

// EGL_KHR_swap_buffers_with_damage extension requires EGL 1.4
extern EGLBoolean eglSwapBuffersWithDamageKHR(EGLDisplay dpy,
                                              EGLSurface surface,
                                              const EGLint *rects,
                                              EGLint n_rects);

// EGL_EXT_platform_base, EGL_MESA_platform_gbm, EGL_MESA_platform_surfaceless
// extern EGLDisplay eglGetPlatformDisplayEXT(EGLenum platform,
//                                            void *native_display,
//                                            const EGLint *attrib_list);
// extern EGLSurface eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
//                                                     EGLConfig config,
//                                                     void *native_window,
//                                                     const EGLint
//                                                     *attrib_list);
// extern EGLSurface eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
//                                                     EGLConfig config,
//                                                     void *native_pixmap,
//                                                     const EGLint
//                                                     *attrib_list);

// EGL 1.5
HYBRIS_IMPLEMENT_FUNCTION3(EGLSync, eglCreateSync, EGLDisplay, EGLenum,
                           const EGLAttrib *);

HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglDestroySync, EGLDisplay, EGLSync);

HYBRIS_IMPLEMENT_FUNCTION4(EGLint, eglClientWaitSync, EGLDisplay, EGLSync,
                           EGLint, EGLTime);

HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglGetSyncAttrib, EGLDisplay, EGLSync,
                           EGLint, EGLAttrib *);

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
HYBRIS_IMPLEMENT_FUNCTION4(EGLSurface, eglCreatePlatformPixmapSurface,
                           EGLDisplay, EGLConfig, void *, const EGLAttrib *);

HYBRIS_IMPLEMENT_FUNCTION3(EGLBoolean, eglWaitSync, EGLDisplay, EGLSync,
                           EGLint);
}

namespace {

// EGL_ANDROID_create_native_client_buffer
EGLClientBuffer eglCreateNativeClientBufferANDROID(const EGLint *attrib_list) {
  (void)attrib_list;
  assert(false);
  return nullptr;
}

// EGL_ANDROID_get_native_client_buffer
EGLClientBuffer eglGetNativeClientBufferANDROID(
    const struct AHardwareBuffer *buffer) {
  (void)buffer;
  ALOGD("Not implement %s", __func__);
  return nullptr;
}

}  // namespace

HYBRIS_VISIBILITY EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) {
  if (auto manager = egl::DisplayManager::Instance(); manager) {
    if (auto display = manager->GetDisplay(display_id); display) {
      return display->GetEglDisplay();
    }
  }
  return EGL_NO_DISPLAY;
}

HYBRIS_VISIBILITY EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major,
                                           EGLint *minor) {
  if (egl::EglProxy::Instance()->Api().eglInitialize(dpy, major, minor)) {
    return EGL_TRUE;
  }
  ALOGD("eglInitialize failed : %s",
        egl::EglProxy::Instance()->StrLastError().c_str());
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLDisplay eglGetPlatformDisplay(
    EGLenum platform, void *native_display, const EGLAttrib *attrib_list) {
  if (auto display = egl::DisplayManager::Instance()->GetPlatformDisplay(
          platform, native_display, attrib_list);
      display) {
    return display->GetEglDisplay();
  }
  return EGL_NO_DISPLAY;
}

HYBRIS_VISIBILITY EGLBoolean eglChooseConfig(EGLDisplay dpy,
                                             const EGLint *attrib_list,
                                             EGLConfig *configs,
                                             EGLint config_size,
                                             EGLint *num_config) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (dpy == EGL_NO_DISPLAY || !attrib_list || attrib_list[0] == EGL_NONE) {
    return api.eglChooseConfig(dpy, attrib_list, configs, config_size,
                               num_config);
  }
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->ChooseConfig(attrib_list, configs, config_size, num_config);
  }
  // EGL_BAD_DISPLAY
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
                                            EGLSurface read, EGLContext ctx) {
  if (auto surface = reinterpret_cast<egl::Surface *>(read); surface) {
    read = surface->GetEglSurface();
  }
  if (auto surface = reinterpret_cast<egl::Surface *>(draw); surface) {
    draw = surface->GetEglSurface();
  }
  return egl::EglProxy::Instance()->Api().eglMakeCurrent(dpy, draw, read, ctx);
}

HYBRIS_VISIBILITY EGLSurface eglCreatePbufferSurface(
    EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->CreatePbufferSurface(config, attrib_list);
  }
  ALOGD("eglCreatePbufferSurface display %p", dpy);
  return EGL_NO_SURFACE;
}

HYBRIS_VISIBILITY EGLSurface eglCreateWindowSurface(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    EGLNativeWindowType win,
                                                    const EGLint *attrib_list) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (dpy == EGL_NO_DISPLAY) {
    return api.eglCreateWindowSurface(dpy, config, win, attrib_list);
  }
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->CreateWindowSurface(config, win, attrib_list);
  }

  ALOGD("eglCreateWindowSurface display %p window %p", dpy, win);
  return EGL_NO_SURFACE;
}

HYBRIS_VISIBILITY EGLBoolean eglDestroySurface(EGLDisplay dpy,
                                               EGLSurface surface) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (dpy == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
    return api.eglDestroySurface(dpy, surface);
  }
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->DestroySurface(surface);
  }

  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLBoolean eglGetConfigAttrib(EGLDisplay dpy,
                                                EGLConfig config,
                                                EGLint attribute,
                                                EGLint *value) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (dpy == EGL_NO_DISPLAY) {
    return api.eglGetConfigAttrib(dpy, config, attribute, value);
  }
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->GetConfigAttrib(config, attribute, value);
  }
  // EGL_BAD_DISPLAY
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLSurface eglGetCurrentSurface(EGLint readdraw) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (auto egl_surf = api.eglGetCurrentSurface(readdraw);
      egl_surf != EGL_NO_SURFACE) {
    auto egl_dpy = api.eglGetCurrentDisplay();
    if (auto display = egl::DisplayManager::Instance()->FindDispay(egl_dpy);
        display) {
      return display->FindSurface(egl_surf);
    }
  }
  return EGL_NO_SURFACE;
}

HYBRIS_VISIBILITY EGLint eglGetError(void) {
  return egl::EglProxy::Instance()->EglError();
}

HYBRIS_VISIBILITY __eglMustCastToProperFunctionPointerType
eglGetProcAddress(const char *procname) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (strncmp(procname, "egl", 3) != 0) {
    return api.eglGetProcAddress(procname);
  }

  if (strcmp(procname, "eglDupNativeFenceFDANDROID") == 0) {
    // not support EGL_ANDROID_native_fence_sync extension
  } else if (strcmp(procname, "eglSetBlobCacheFuncsANDROID") == 0) {
    // not support EGL_ANDROID_blob_cache extension
  } else if (strcmp(procname, "eglPresentationTimeANDROID") == 0) {
    // not support EGL_ANDROID_presentation_time extension
  } else if (strcmp(procname, "eglCreateImageKHR") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglCreateImageKHR);
  } else if (strcmp(procname, "eglDestroyImageKHR") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglDestroyImageKHR);
  } else if (strcmp(procname, "eglSwapBuffersWithDamageKHR") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglSwapBuffersWithDamageKHR);
  }

  if (auto addr = api.eglGetProcAddress(procname); addr) {
    ALOGV("eglGetProcAddress %s=%p", procname, addr);
    return addr;
  }
  if (strcmp(procname, "eglCreateNativeClientBufferANDROID") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglCreateNativeClientBufferANDROID);

  } else if (strcmp(procname, "eglGetNativeClientBufferANDROID") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglGetNativeClientBufferANDROID);
  } else if (strcmp(procname, "eglQueryStringImplementationANDROID") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        api.eglQueryString);
  }
  ALOGD("Not implement %s", procname);

  return nullptr;
}

HYBRIS_VISIBILITY const char *eglQueryString(EGLDisplay dpy, EGLint name) {
  auto proxy = egl::EglProxy::Instance();
  if (name == EGL_EXTENSIONS) {
    if (dpy == EGL_NO_DISPLAY) {
      return proxy->GetClientExtensions();
    } else {
      if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
          display) {
        return display->GetEglExtensions();
      }
      // TODO error
    }
    return nullptr;
  }
  return egl::EglProxy::Instance()->Api().eglQueryString(dpy, name);
}

HYBRIS_VISIBILITY EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                                             EGLint attribute, EGLint *value) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (auto egl_surface = reinterpret_cast<egl::Surface *>(surface);
      egl_surface) {
    return egl_surface->QuerySurface(attribute, value);
  }
  return api.eglQuerySurface(dpy, EGL_NO_SURFACE, attribute, value);
}

HYBRIS_VISIBILITY EGLBoolean eglSwapBuffers(EGLDisplay dpy,
                                            EGLSurface surface) {
  if (auto egl_surface = reinterpret_cast<egl::Surface *>(surface);
      egl_surface) {
    return egl_surface->SwapBuffers();
  }
  auto const &api = egl::EglProxy::Instance()->Api();
  return api.eglSwapBuffers(dpy, EGL_NO_SURFACE);
}

HYBRIS_VISIBILITY
EGLBoolean eglSwapBuffersWithDamageKHR(EGLDisplay dpy, EGLSurface surface,
                                       const EGLint *rects, EGLint n_rects) {
  if (auto egl_surface = reinterpret_cast<egl::Surface *>(surface);
      egl_surface) {
    return egl_surface->SwapBuffersWithDamageKHR(rects, n_rects);
  }
  auto const &api = egl::EglProxy::Instance()->Api();
  return api.eglSwapBuffersWithDamageKHR(dpy, EGL_NO_SURFACE, rects, n_rects);
}

HYBRIS_VISIBILITY EGLBoolean eglTerminate(EGLDisplay dpy) {
  return egl::DisplayManager::Instance()->Terminate(dpy);
}

HYBRIS_VISIBILITY EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface,
                                             EGLint buffer) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (auto egl_surface = reinterpret_cast<egl::Surface *>(surface);
      egl_surface) {
    surface = egl_surface->GetEglSurface();
  }
  return api.eglBindTexImage(dpy, surface, buffer);
}

HYBRIS_VISIBILITY EGLBoolean eglReleaseTexImage(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLint buffer) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (auto egl_surface = reinterpret_cast<egl::Surface *>(surface);
      egl_surface) {
    surface = egl_surface->GetEglSurface();
  }
  return api.eglReleaseTexImage(dpy, surface, buffer);
}
HYBRIS_VISIBILITY EGLBoolean eglSurfaceAttrib(EGLDisplay dpy,
                                              EGLSurface surface,
                                              EGLint attribute, EGLint value) {
  if (auto egl_surface = reinterpret_cast<egl::Surface *>(surface);
      egl_surface) {
    return egl_surface->SurfaceAttrib(attribute, value);
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLSurface eglCreatePbufferFromClientBuffer(
    EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config,
    const EGLint *attrib_list) {
  (void)dpy;
  (void)buftype;
  (void)buffer;
  (void)config;
  (void)attrib_list;
  ALOGD("eglCreatePbufferFromClientBuffer Not implement");
  return EGL_NO_SURFACE;
}

HYBRIS_VISIBILITY EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                                                EGLenum target,
                                                EGLClientBuffer buffer,
                                                const EGLint *attrib_list) {
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    if (auto image = display->CreateImageKHR(ctx, target, buffer, attrib_list);
        image) {
      return reinterpret_cast<EGLImageKHR>(image->GetEglImage());
    }
  }
  return EGL_NO_IMAGE_KHR;
}

HYBRIS_VISIBILITY EGLBoolean eglDestroyImageKHR(EGLDisplay dpy,
                                                EGLImageKHR img) {
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->DestroyImageKHR(img);
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLImage eglCreateImage(EGLDisplay dpy, EGLContext ctx,
                                          EGLenum target,
                                          EGLClientBuffer buffer,
                                          const EGLAttrib *attrib_list) {
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    if (auto image = display->CreateImage(ctx, target, buffer, attrib_list);
        image) {
      return image->GetEglImage();
    }
  }
  return EGL_NO_IMAGE;
}

HYBRIS_VISIBILITY EGLBoolean eglDestroyImage(EGLDisplay dpy, EGLImage img) {
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->DestroyImage(img);
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLSurface eglCreatePlatformWindowSurface(
    EGLDisplay dpy, EGLConfig config, void *native_window,
    const EGLAttrib *attrib_list) {
  auto const &api = egl::EglProxy::Instance()->Api();
  if (dpy == EGL_NO_DISPLAY) {
    return api.eglCreatePlatformWindowSurface(dpy, config, native_window,
                                              attrib_list);
  }
  if (auto display = egl::DisplayManager::Instance()->FindDispay(dpy);
      display) {
    return display->CreatePlatformWindowSurface(config, native_window,
                                                attrib_list);
  }
  ALOGD("eglCreatePlatformWindowSurface display %p window %p", dpy,
        native_window);
  return EGL_NO_SURFACE;
}
