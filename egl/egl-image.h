#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <system/window.h>

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

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
  Image(EGLDisplay egl_dpy, EglProxyPtr proxy)
      : egl_dpy_(egl_dpy), proxy_(std::move(proxy)) {}
  virtual EGLImage CreateImage() = 0;

  EGLBoolean DestroyImage();

  EGLImage GetEglImage() { return egl_image_; }

  virtual ~Image();

 protected:
  EGLDisplay egl_dpy_{};
  EglProxyPtr proxy_{};
  EGLImage egl_image_;
};

class AndroidBufferImage : public Image {
 public:
  AndroidBufferImage(EGLDisplay egl_dpy, EglProxyPtr proxy,
                     ANativeWindowBuffer *buffer,
                     const EGLAttrib *attrib_list = nullptr);

  EGLImage CreateImage() override;

 private:
  static GrallocPtr &GetGralloc();

  EGLBoolean FillAttribs(const ANativeWindowBuffer *native_buffer,
                         size_t attribs_size, EGLAttrib *attribs);

  std::shared_ptr<ANativeWindowBuffer> buffer_;
  std::vector<EGLAttrib> attribs_;
};

class GlBufferImage : public Image {
 public:
  GlBufferImage(EGLDisplay egl_dpy, EglProxyPtr proxy, EGLenum target,
                GLuint buffer, EGLContext ctx, const EGLAttrib *attrib_list);

  EGLImage CreateImage() override;

 private:
  EGLenum target_{};
  GLuint buffer_{};
  EGLContext ctx_{};
  std::vector<EGLAttrib> attribs_;
};

class PassthroughImage : public Image {
 public:
  PassthroughImage(EGLDisplay egl_dpy, EglProxyPtr proxy, EGLenum target,
                   EGLClientBuffer buffer, EGLContext ctx,
                   const EGLAttrib *attrib_list);

  EGLImage CreateImage() override;

 private:
  EGLenum target_{};
  EGLClientBuffer buffer_{};
  EGLContext ctx_{};
  std::vector<EGLAttrib> attribs_;
};

}  // namespace egl