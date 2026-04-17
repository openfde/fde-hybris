#pragma once
#include <EGL/egl.h>
#include <android/native_window.h>

#include <memory>

#include "egl-image.h"
#include "egl-proxy.h"

namespace egl {

class Display;

class BlitFramebuffer {
 public:
  explicit BlitFramebuffer(EglProxyPtr proxy, Display *dpy)
      : proxy_(proxy), dpy_(dpy) {}

  void Blit(ANativeWindowBuffer *native_buffer);

 private:
  EglProxyPtr proxy_{};
  Display *dpy_{};
};

using BlitFramebufferPtr = std::shared_ptr<BlitFramebuffer>;

}  // namespace egl