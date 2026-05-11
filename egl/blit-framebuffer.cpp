#include "blit-framebuffer.h"

#ifndef GL_GLES_PROTOTYPES
#define GL_GLES_PROTOTYPES 0
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <log/log.h>

#include <cstring>

#include "egl-display.h"
#include "pixel-format.h"
#include <hardware/gralloc.h>

#define HYBRIS_GET_SYMBOL_ADDRESS(symbol) \
  ({ egl::EglProxy::Instance()->Api().eglGetProcAddress(#symbol); })

#define HYBRIS_VISIBILITY __attribute__((visibility("hidden")))
#include "binding.h"

extern "C" {
HYBRIS_IMPLEMENT_FUNCTION2(void, glEGLImageTargetTexture2DOES, GLenum,
                           GLeglImageOES);

HYBRIS_IMPLEMENT_FUNCTION2(void, glGetIntegerv, GLenum, GLint *);

HYBRIS_IMPLEMENT_FUNCTION2(void, glGenTextures, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteTextures, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindTexture, GLenum, GLuint);

HYBRIS_IMPLEMENT_FUNCTION8(void, glCopyTexSubImage2D, GLenum, GLint, GLint,
                           GLint, GLint, GLint, GLsizei, GLsizei);

HYBRIS_IMPLEMENT_FUNCTION9(void, glTexImage2D, GLenum, GLint, GLint, GLsizei,
                           GLsizei, GLint, GLenum, GLenum, const void *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameteri, GLenum, GLenum, GLint);

HYBRIS_IMPLEMENT_FUNCTION2(void, glGenFramebuffers, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteFramebuffers, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindFramebuffer, GLenum, GLuint);

HYBRIS_IMPLEMENT_FUNCTION5(void, glFramebufferTexture2D, GLenum, GLenum, GLenum,
                           GLuint, GLint);
HYBRIS_IMPLEMENT_FUNCTION1(GLenum, glCheckFramebufferStatus, GLenum);

HYBRIS_IMPLEMENT_FUNCTION0(GLenum, glGetError);
HYBRIS_IMPLEMENT_FUNCTION0(void, glFinish);

HYBRIS_IMPLEMENT_FUNCTION4(void, glViewport, GLint, GLint, GLsizei, GLsizei);

HYBRIS_IMPLEMENT_FUNCTION10(void, glBlitFramebuffer, GLint, GLint, GLint, GLint,
                            GLint, GLint, GLint, GLint, GLbitfield, GLenum);
}

namespace {

const char* eglStrError(EGLint err)
{
    switch (err){
        case EGL_SUCCESS:           return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:   return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:        return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:         return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:     return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG:        return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT:       return "EGL_BAD_CONTEXT";
        case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:       return "EGL_BAD_DISPLAY";
        case EGL_BAD_MATCH:         return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER:     return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE:       return "EGL_BAD_SURFACE";
        case EGL_CONTEXT_LOST:      return "EGL_CONTEXT_LOST";
        case 0x502:                 return "GL_INVALID_OPERATION";
        default: return "UNKNOWN";
    }
}
void CopyFromFramebuffer(EGLImage egl_image, int32_t width, int32_t height, int32_t flip,
                         GLint fbo = 0) {
  GLuint tmp_tex = {};
  GLint curr_tex_bind = {};
  GLint prev_read_fbo = {}, prev_draw_fbo = {};
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex_bind);
  glGenTextures(1, &tmp_tex);
  glBindTexture(GL_TEXTURE_2D, tmp_tex);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, egl_image);
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
      ALOGE("glEGLImageTargetTexture2DOES fail %dx%d err:%s(0x%x)",
        width, height, eglStrError(err), err);
  }

  // 保存当前帧缓冲状态
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_read_fbo);
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_draw_fbo);

  if (prev_read_fbo != fbo) {
    // 绑定源帧缓冲（读缓冲）
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  }

  // 创建临时帧缓冲，将纹理作为颜色附件
  GLuint temp_fbo;
  glGenFramebuffers(1, &temp_fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, temp_fbo);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tmp_tex, 0);
  err = glGetError();
  if (err != GL_NO_ERROR) {
      ALOGE("glFramebufferTexture2D fail %dx%d err:%s(0x%x)",
        width, height, eglStrError(err), err);
  }

  // glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
  if (flip) {
    // 垂直翻转拷贝：源矩形 (0,0,width,height)，目标矩形 (0,height,width,0)
    glBlitFramebuffer(0, 0, width, height,
                    0, height, width, 0,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);
  } else {
    glBlitFramebuffer(0, 0, width, height,
                    0, 0, width, height,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }
  err = glGetError();
  if (err != GL_NO_ERROR) {
      ALOGE("glBlitFramebuffer fail %dx%d err:%s(0x%x)",
        width, height, eglStrError(err), err);
  }

  glFinish();
  if (prev_read_fbo != fbo) {
    // 恢复状态
    glBindFramebuffer(GL_READ_FRAMEBUFFER, prev_read_fbo);
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_draw_fbo);
  glDeleteFramebuffers(1, &temp_fbo);

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

void BlitFramebuffer::Blit(ANativeWindowBuffer *native_buffer) {
  if (!native_buffer) {
    return;
  }
  auto width = native_buffer->width;
  auto height = native_buffer->height;

  GLint vport[4] = {};
  glGetIntegerv(GL_VIEWPORT, vport);
  glViewport(0, 0, width, height);

  auto image =
      std::make_shared<AndroidBufferImage>(dpy_, proxy_, native_buffer);
  if (auto egl_img = image->CreateImage(); egl_img != EGL_NO_IMAGE) {
    int flip = (native_buffer->usage & GRALLOC_USAGE_PRIVATE_1);
    CopyFromFramebuffer(egl_img, width, height, flip);

    image->DestroyImage();
  }

  // Restore previous viewport.
  glViewport(vport[0], vport[1], vport[2], vport[3]);

  // clear GL errors, because its possible that the fbo format does not match
  // the format of the read buffer, in the case of OpenGL ES 3.1 and integer
  // RGBA formats.
  glGetError();

  glFinish();
}

}  // namespace egl