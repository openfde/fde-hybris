#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>
#include <system/window.h>

#include <cassert>
#include <memory>
#include <vector>

#include "blit-framebuffer.h"
#include "egl-proxy.h"

namespace egl {

class Surface;
using SurfacePtr = std::shared_ptr<Surface>;

class Surface {
 public:
  Surface(EGLDisplay egl_dpy, EglProxyPtr proxy)
      : egl_dpy_(egl_dpy), proxy_(proxy) {}
  virtual EGLSurface CreateSurface(EGLConfig config,
                                   const EGLAttrib *attrib_list) = 0;
  virtual EGLBoolean DestroySurface() = 0;
  virtual EGLBoolean QuerySurface(EGLint attribute, EGLint *value);
  virtual EGLBoolean SurfaceAttrib(EGLint attribute, EGLint value);

  virtual EGLBoolean SwapBuffers();
  virtual EGLBoolean SwapBuffersWithDamageKHR(const EGLint *rects,
                                              EGLint n_rects);

  EGLSurface GetEglSurface() { return egl_surf_.get(); }

  virtual ~Surface() = default;

 protected:
  void SetEglSurface(EGLSurface egl_surf);

  EGLDisplay egl_dpy_{};
  EglProxyPtr proxy_{};
  std::shared_ptr<void> egl_surf_{};
};

class WindowSurface : public Surface {
 public:
  WindowSurface(EGLDisplay egl_dpy, EglProxyPtr proxy, ANativeWindow *window);

  ~WindowSurface();

  EGLSurface CreateSurface(EGLConfig config,
                           const EGLAttrib *attrib_list) override;
  EGLBoolean DestroySurface() override;
  EGLBoolean QuerySurface(EGLint attribute, EGLint *value) override;
  EGLBoolean SurfaceAttrib(EGLint attribute, EGLint value) override;

  EGLBoolean SwapBuffers() override;
  EGLBoolean SwapBuffersWithDamageKHR(const EGLint *rects,
                                      EGLint n_rects) override;

 private:
  struct CreatedStateT {
    CreatedStateT(int32_t w, int32_t h, EGLConfig c, std::vector<EGLAttrib> a)
        : width(w), height(h), config(c), attribs(std::move(a)) {}
    int32_t width{};
    int32_t height{};
    EGLConfig config{};
    std::vector<EGLAttrib> attribs;
  };
  using CreateStatePtr = std::shared_ptr<CreatedStateT>;

  EGLSurface CreateNewSurface(const CreatedStateT &created_state);

  void DequeueBuffer();
  void QueueBuffer();
  void CancelBuffer();

  EGLBoolean MaybeResize();

 private:
  ANativeWindow *native_window_{};
  ANativeWindowBuffer *native_buffer_{};
  CreateStatePtr created_state_{};
  BlitFramebufferPtr blit_{};

  int32_t in_fence_fd_ = -1;
  EGLint swap_behavior_ = EGL_BUFFER_DESTROYED;
};

class PassthroughSurface : public Surface {
 public:
  PassthroughSurface(EGLDisplay egl_dpy, EglProxyPtr proxy, void *window)
      : Surface(egl_dpy, proxy), native_window_(window) {}

  EGLSurface CreateSurface(EGLConfig config,
                           const EGLAttrib *attrib_list) override;
  EGLBoolean DestroySurface() override;
  EGLBoolean QuerySurface(EGLint attribute, EGLint *value) override;

 private:
  void *native_window_{};
};

}  // namespace egl