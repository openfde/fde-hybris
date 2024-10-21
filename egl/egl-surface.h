#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>
#include <system/window.h>

#include <cassert>
#include <memory>
#include <vector>

#include "egl-proxy.h"

namespace egl {

class Surface;
using SurfacePtr = std::shared_ptr<Surface>;

class Surface {
 public:
  Surface(EGLDisplay egl_dpy, EglProxy *proxy)
      : egl_dpy_(egl_dpy), proxy_(proxy) {}
  virtual EGLSurface CreateSurface(EGLConfig config,
                                   const EGLAttrib *attrib_list) = 0;
  virtual EGLBoolean DestroySurface() = 0;
  virtual EGLBoolean QuerySurface(EGLint attribute, EGLint *value);

  virtual EGLBoolean SwapBuffers();

  EGLSurface GetEglSurface() { return egl_surf_.get(); }

  virtual ~Surface() = default;

 protected:
  void SetEglSurface(EGLSurface egl_surf);

  EGLDisplay egl_dpy_{};
  EglProxy *proxy_{};
  std::shared_ptr<void> egl_surf_{};
};

class WindowSurface : public Surface {
 public:
  WindowSurface(EGLDisplay egl_dpy, EglProxy *proxy, ANativeWindow *window)
      : Surface(egl_dpy, proxy), native_window_(window) {}

  ~WindowSurface();

  EGLSurface CreateSurface(EGLConfig config,
                           const EGLAttrib *attrib_list) override;
  EGLBoolean DestroySurface() override;
  EGLBoolean QuerySurface(EGLint attribute, EGLint *value) override;

  EGLBoolean SwapBuffers() override;

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

  void DequeueBuffer();
  void QueueBuffer();
  void CancelBuffer();

 private:
  ANativeWindow *native_window_{};
  ANativeWindowBuffer *native_buffer_{};
  CreateStatePtr created_state_{};

  int32_t in_fence_fd_ = -1;
};

class PassthroughSurface : public Surface {
 public:
  PassthroughSurface(EGLDisplay egl_dpy, EglProxy *proxy, void *window)
      : Surface(egl_dpy, proxy), native_window_(window) {}

  EGLSurface CreateSurface(EGLConfig config,
                           const EGLAttrib *attrib_list) override;
  EGLBoolean DestroySurface() override;
  EGLBoolean QuerySurface(EGLint attribute, EGLint *value) override;

 private:
  void *native_window_{};
};

}  // namespace egl