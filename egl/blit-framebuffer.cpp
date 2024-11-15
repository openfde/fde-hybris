#include "blit-framebuffer.h"

#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <log/log.h>

#include <cstring>

#define GL_GLEXT_PROTOTYPES
#include <GLES/glext.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include "pixel-format.h"

namespace {

class FrameBufferBinder {
 public:
  GLint GetFbo() const { return fbo_; }

  explicit FrameBufferBinder(GLenum fb_target, EGLImage image)
      : fb_target_(fb_target) {
    GLenum fb_bound = (fb_target_ == GL_READ_FRAMEBUFFER)
                          ? GL_READ_FRAMEBUFFER_BINDING
                          : GL_DRAW_FRAMEBUFFER_BINDING;
    glGetIntegerv(fb_bound, &prev_fbo_);
    if (prev_fbo_ != 0) {
      glBindFramebuffer(fb_target_, 0);
    }

    GLint curr_tex_bind = {};
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex_bind);

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    glBindTexture(GL_TEXTURE_2D, curr_tex_bind);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(fb_target_, fbo_);
    glFramebufferTexture2D(fb_target_, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D,
                           tex_, 0);
    if (auto status = glCheckFramebufferStatus(fb_target_);
        status != GL_FRAMEBUFFER_COMPLETE_OES) {
      ALOGE("glFramebufferTexture2D: FBO not complete: 0x%04X", status);
    }
  }

  ~FrameBufferBinder() {
    if (fbo_ != 0) {
      glBindFramebuffer(fb_target_, 0);
      glDeleteFramebuffers(1, &fbo_);
    }
    if (tex_ != 0) {
      glDeleteTextures(1, &tex_);
    }
    if (prev_fbo_ != 0) {
      glBindFramebuffer(fb_target_, prev_fbo_);
    }
  }

 private:
  GLenum fb_target_{};
  GLint prev_fbo_{};
  GLuint tex_{};
  GLuint fbo_{};
};

class Texture2DBinder {
 public:
  Texture2DBinder(GLuint tex) {
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex_);
    glBindTexture(GL_TEXTURE_2D, tex);
  }
  ~Texture2DBinder() { glBindTexture(GL_TEXTURE_2D, prev_tex_); }

 private:
  GLint prev_tex_{};
};

void CopyFromFramebuffer(EGLImage egl_image, int32_t width, int32_t height,
                         GLint fbo = 0) {
  GLuint tmp_tex = {};
  GLint curr_tex_bind = {};
  GLint prev_read_fbo = {};
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex_bind);
  glGenTextures(1, &tmp_tex);
  glBindTexture(GL_TEXTURE_2D, tmp_tex);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, egl_image);

  // gles3
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_read_fbo);
  if (prev_read_fbo != fbo) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  }

  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

  if (prev_read_fbo != fbo) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prev_read_fbo);
  }

  glBindTexture(GL_TEXTURE_2D, curr_tex_bind);
  glDeleteTextures(1, &tmp_tex);
}

bool GetTextureFormatParameters(GLint internal_format, GLenum *tex_format,
                                GLenum *pixel_type,
                                GLint *sized_internal_format) {
  switch (internal_format) {
    case GL_RGB:
    case GL_RGB8:
      *tex_format = GL_RGB;
      *pixel_type = GL_UNSIGNED_BYTE;
      *sized_internal_format = GL_RGB8;
      return true;
    case GL_RGB565_OES:
      *tex_format = GL_RGB;
      *pixel_type = GL_UNSIGNED_SHORT_5_6_5;
      *sized_internal_format = GL_RGB565;
      return true;
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGB5_A1_OES:
    case GL_RGBA4_OES:
      *tex_format = GL_RGBA;
      *pixel_type = GL_UNSIGNED_BYTE;
      *sized_internal_format = GL_RGBA8;
      return true;
    case GL_UNSIGNED_INT_10_10_10_2_OES:
      *tex_format = GL_RGBA;
      *pixel_type = GL_UNSIGNED_SHORT;
      *sized_internal_format = GL_UNSIGNED_INT_10_10_10_2_OES;
      return true;
    case GL_RGB10_A2:
      *tex_format = GL_RGBA;
      *pixel_type = GL_UNSIGNED_INT_2_10_10_10_REV;
      *sized_internal_format = GL_RGB10_A2;
      return true;
    case GL_RGB16F:
      *tex_format = GL_RGB;
      *pixel_type = GL_HALF_FLOAT;
      *sized_internal_format = GL_RGB16F;
      return true;
    case GL_RGBA16F:
      *tex_format = GL_RGBA;
      *pixel_type = GL_HALF_FLOAT;
      *sized_internal_format = GL_RGBA16F;
      return true;
    case GL_LUMINANCE:
      *tex_format = GL_LUMINANCE;
      *pixel_type = GL_UNSIGNED_SHORT;
      *sized_internal_format = GL_R8;
      return true;
    case GL_BGRA_EXT:
      *tex_format = GL_BGRA_EXT;
      *pixel_type = GL_UNSIGNED_BYTE;
      *sized_internal_format = GL_BGRA8_EXT;
      return true;
    case GL_R8:
    case GL_RED:
      *tex_format = GL_RED;
      *pixel_type = GL_UNSIGNED_BYTE;
      *sized_internal_format = GL_R8;
      return true;
    case GL_RG8:
    case GL_RG:
      *tex_format = GL_RG;
      *pixel_type = GL_UNSIGNED_BYTE;
      *sized_internal_format = GL_RG8;
      return true;
    default:
      fprintf(stderr, "%s: Unknown format 0x%x\n", __func__, internal_format);
      return false;
  }
}

}  // namespace

namespace egl {

EGLImage Texture2D::EglImage() {
  if (!egl_img_) {
    auto &api = egl::EglProxy::Instance()->Api();
    auto dpy = api.eglGetCurrentDisplay();
    auto context = api.eglGetCurrentContext();
    auto tex = Id();
    if (auto egl_img =
            api.eglCreateImage(dpy, context, EGL_GL_TEXTURE_2D_KHR,
                               (EGLClientBuffer)(uintptr_t)tex, nullptr);
        egl_img) {
      auto destroy = api.eglDestroyImage;
      egl_img_.reset(egl_img,
                     [destroy, dpy](EGLImage img) { destroy(dpy, img); });
    }
  }
  return egl_img_.get();
}

GLuint Texture2D::Id() { return tex_; }

void Texture2D::Delete() {
  egl_img_.reset();
  if (tex_ != 0) {
    glDeleteTextures(1, &tex_);
    tex_ = 0;
  }
}

Texture2DPtr Texture2D::CreateTexture(GLenum internal_format, int32_t width,
                                      int32_t height) {
  GLenum tex_format;
  GLenum pixel_type;
  GLint sized_internal_format;
  if (!GetTextureFormatParameters(internal_format, &tex_format, &pixel_type,
                                  &sized_internal_format)) {
    ALOGE("Not support texture internal format 0x%04X", internal_format);
    return nullptr;
  }

  GLuint tex = {};
  glGenTextures(1, &tex);
  Texture2DBinder tex2d(tex);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, tex_format,
               pixel_type, nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return std::make_shared<Texture2D>(tex);
}

void BlitFramebuffer::Blit(ANativeWindowBuffer *native_buffer) {
  if (!native_buffer) {
    return;
  }
  auto width = native_buffer->width;
  auto height = native_buffer->height;
  auto native_format = native_buffer->format;
  if (!Resize(width, height, native_format)) {
    return;
  }
  auto image =
      std::make_shared<AndroidBufferImage>(egl_dpy_, proxy_, native_buffer);
  if (auto egl_img = image->CreateImage(); egl_img != EGL_NO_IMAGE) {
    CopyFromFramebuffer(texture_->EglImage(), width, height);
    FrameBufferBinder draw_fb{GL_DRAW_FRAMEBUFFER, egl_img};
    auto draw = GetDraw();
    draw->Flip(texture_->Id());
    image->DestroyImage();
  }
}

bool BlitFramebuffer::Resize(int32_t width, int32_t height,
                             int32_t native_format) {
  if (width != width_ || height != height_ || native_format != native_format_) {
    HalPixelFormat pixel_format{native_format};
    texture_ = Texture2D::CreateTexture(pixel_format.TextureInternalFormat(),
                                        width, height);
  }
  return texture_ != nullptr;
}

TextureFlipPtr &BlitFramebuffer::GetDraw() { return TextureFlip::Instance(); }

}  // namespace egl