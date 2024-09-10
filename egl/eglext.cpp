#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#include <EGL/eglext.h>

extern "C" {
// EGL_KHR_fence_sync extension requires EGL 1.1
HYBRIS_IMPLEMENT_FUNCTION3(EGLSyncKHR, eglCreateSyncKHR, EGLDisplay, EGLenum,
                           const EGLint *);

HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglDestroySyncKHR, EGLDisplay,
                           EGLSyncKHR);

HYBRIS_IMPLEMENT_FUNCTION4(EGLint, eglClientWaitSyncKHR, EGLDisplay, EGLSyncKHR,
                           EGLint, EGLTimeKHR);

HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglGetSyncAttribKHR, EGLDisplay,
                           EGLSyncKHR, EGLint, EGLint *);

// EGL_KHR_image_base extension requires EGL 1.2
extern EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                                     EGLenum target, EGLClientBuffer buffer,
                                     const EGLint *attrib_list);
extern EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR img);

// EGL_EXT_platform_base, EGL_MESA_platform_gbm, EGL_MESA_platform_surfaceless
extern EGLDisplay eglGetPlatformDisplayEXT(EGLenum platform,
                                           void *native_display,
                                           const EGLint *attrib_list);
extern EGLSurface eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    void *native_window,
                                                    const EGLint *attrib_list);
extern EGLSurface eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    void *native_pixmap,
                                                    const EGLint *attrib_list);
}

namespace egl {

EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                              EGLClientBuffer buffer,
                              const EGLint *attrib_list) {
  auto image =
      CreateImageAdapter(dpy, ctx, target, buffer, attrib_list, CreateImageKHR);
  return reinterpret_cast<EGLImageKHR>(image);
}

EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR img) {
  PFNEGLDESTROYIMAGEKHRPROC s_eglDestroyImageKHR = {};
  assert(s_eglGetProcAddress);
  s_eglDestroyImageKHR = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(
      s_eglGetProcAddress("eglDestroyImageKHR"));
  assert(s_eglDestroyImageKHR);
  return EglDestroyImage(dpy, img, s_eglDestroyImageKHR);
}

__eglMustCastToProperFunctionPointerType
eglGetProcAddress(const char *procname) {
  if (strcmp(procname, "eglDupNativeFenceFDANDROID") == 0) {
    // not support EGL_ANDROID_native_fence_sync extension
    ALOGD("Not implement eglDupNativeFenceFDANDROID");
    return nullptr;
  }

  if (strcmp(procname, "eglSetBlobCacheFuncsANDROID") == 0) {
    // not support EGL_ANDROID_blob_cache extension
    ALOGD("Not implement eglSetBlobCacheFuncsANDROID");
    return nullptr;
  }

  if (strcmp(procname, "eglPresentationTimeANDROID") == 0) {
    // not support EGL_ANDROID_presentation_time extension
    ALOGD("Not implement eglPresentationTimeANDROID");
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

  if (strcmp(procname, "eglCreateImageKHR") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglCreateImageKHR);
  }

  if (strcmp(procname, "eglDestroyImageKHR") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglDestroyImageKHR);
  }
}

} // namespace egl