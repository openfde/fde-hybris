#pragma once
#include <EGL/egl.h>
#include <android/native_window.h>

#include <memory>

#include "egl-image.h"
#include "egl-proxy.h"
#include "texture-flip.h"

namespace egl {

class Texture2D;
using Texture2DPtr = std::shared_ptr<Texture2D>;

class BlitFramebuffer {
 public:
  explicit BlitFramebuffer(EglProxyPtr proxy, EGLDisplay egl_dpy)
      : proxy_(proxy), egl_dpy_(egl_dpy) {}

  void Blit(ANativeWindowBuffer *native_buffer);

 private:
  bool Resize(int32_t width, int32_t height, int32_t native_format);

  TextureFlipPtr &GetDraw();

  EglProxyPtr proxy_{};
  EGLDisplay egl_dpy_{};
  int32_t width_{};
  int32_t height_{};
  int32_t native_format_{};
  Texture2DPtr texture_{};
  TextureFlipPtr flip_ = {};
};

using BlitFramebufferPtr = std::shared_ptr<BlitFramebuffer>;

class Texture2D {
 public:
  Texture2D() = default;
  Texture2D(GLuint tex) : tex_(tex){};
  ~Texture2D() { Delete(); }

  EGLImage EglImage();
  GLuint Id();

  void Delete();

  Texture2D(const Texture2D &) = delete;
  Texture2D &operator=(const Texture2D &) = delete;

  static Texture2DPtr CreateTexture(GLenum internal_format, int32_t width,
                                    int32_t height);

 private:
  std::shared_ptr<void> egl_img_{};
  GLuint tex_{};
};

}  // namespace egl