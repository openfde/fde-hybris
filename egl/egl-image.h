#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <system/window.h>

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include "egl-proxy.h"

#ifdef __cplusplus
extern "C" {
#endif

struct u_gralloc;

#ifdef __cplusplus
}
#endif

namespace egl {

class Image;
using ImagePtr = std::shared_ptr<Image>;
using GrallocPtr = std::shared_ptr<u_gralloc>;

class ImageManager;
using ImageManagerPtr = std::shared_ptr<ImageManager>;

class ImageManager {
 public:
  bool AddImage(ImagePtr image);
  void DeleteImage(EGLImage egl_image);
  Image *FindImage(EGLImage egl_image);
  void ClearImages();

 private:
  std::map<EGLImage, ImagePtr> images_;
  std::mutex mtx_;
};

class Image {
 public:
  Image(EGLDisplay egl_dpy, EglProxy *proxy)
      : egl_dpy_(egl_dpy), proxy_(proxy) {}
  virtual EGLImage CreateImage(EGLContext ctx,
                               const EGLAttrib *attrib_list) = 0;

  EGLBoolean DestroyImage();

  EGLImage GetEglImage() { return egl_image_; }

  virtual ~Image();

 protected:
  EGLDisplay egl_dpy_{};
  EglProxy *proxy_{};
  EGLImage egl_image_;
};

class AndroidBufferImage : public Image {
 public:
  AndroidBufferImage(EGLDisplay egl_dpy, EglProxy *proxy,
                     ANativeWindowBuffer *buffer);

  EGLImage CreateImage(EGLContext ctx, const EGLAttrib *attrib_list) override;

 private:
  static GrallocPtr &GetGralloc();

  EGLBoolean FillAttribs(const ANativeWindowBuffer *native_buffer,
                         size_t attribs_size, EGLAttrib *attribs);

  std::shared_ptr<ANativeWindowBuffer> buffer_;
};

class GlBufferImage : public Image {
 public:
  GlBufferImage(EGLDisplay egl_dpy, EglProxy *proxy, EGLenum target,
                GLuint buffer)
      : Image(egl_dpy, proxy), target_(target), buffer_(buffer) {}

  EGLImage CreateImage(EGLContext ctx, const EGLAttrib *attrib_list) override;

 private:
  EGLenum target_{};
  GLuint buffer_{};
};

class PassthroughImage : public Image {
 public:
  PassthroughImage(EGLDisplay egl_dpy, EglProxy *proxy, EGLenum target,
                   EGLClientBuffer buffer)
      : Image(egl_dpy, proxy), target_(target), buffer_(buffer) {}

  EGLImage CreateImage(EGLContext ctx, const EGLAttrib *attrib_list) override;

 private:
  EGLenum target_{};
  EGLClientBuffer buffer_{};
};

}  // namespace egl