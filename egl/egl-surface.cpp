#include "egl-surface.h"

#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <android/sync.h>
#include <log/log.h>
#include <system/window.h>
#include <unistd.h>
#include <vndk/window.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES/glext.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include <cassert>
#include <map>
#include <mutex>

#include "egl-image.h"
#include "egl-misc.h"

namespace {

void CloseFenceFd(int32_t &fd) {
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
}

void SyncWait(int32_t fd) {
  if (fd >= 0) {
    sync_wait(fd, -1);
  }
}

}  // namespace

namespace egl {

EGLBoolean Surface::QuerySurface(EGLint attribute, EGLint *value) {
  return proxy_->Api().eglQuerySurface(egl_dpy_, egl_surf_.get(), attribute,
                                       value);
}

EGLBoolean Surface::SwapBuffers() {
  return proxy_->Api().eglSwapBuffers(egl_dpy_, egl_surf_.get());
}

void Surface::SetEglSurface(EGLSurface egl_surf) {
  if (egl_surf != EGL_NO_SURFACE) {
    egl_surf_ = std::shared_ptr<void>(egl_surf, [this](EGLSurface surf) {
      proxy_->Api().eglDestroySurface(egl_dpy_, surf);
    });
  }
}

WindowSurface::~WindowSurface() {
  DestroySurface();
  CloseFenceFd(in_fence_fd_);
}

EGLSurface WindowSurface::CreateSurface(EGLConfig config,
                                        const EGLAttrib *attrib_list) {
  assert(native_window_);
  DequeueBuffer();

  auto width = ANativeWindow_getWidth(native_window_);
  auto height = ANativeWindow_getHeight(native_window_);
  std::vector<EGLint> pbuf_attribs = {EGL_WIDTH, width, EGL_HEIGHT, height};
  auto attribs = misc::ApendAttributes(attrib_list, pbuf_attribs);
  if (auto egl_surf =
          proxy_->Api().eglCreatePbufferSurface(egl_dpy_, config, attribs);
      egl_surf != EGL_NO_SURFACE) {
    SetEglSurface(egl_surf);
    if (created_state_) {
      created_state_->width = width;
      created_state_->height = height;
    } else {
      created_state_ = std::make_shared<CreatedStateT>(
          width, height, config, misc::DupAttributes(attrib_list));
    }

    return egl_surf;
  }
  ALOGD("eglCreateWindowSurface display %p failed : %s", egl_dpy_,
        proxy_->StrLastError().c_str());
  return EGL_NO_SURFACE;
}

EGLBoolean WindowSurface::DestroySurface() {
  CancelBuffer();
  egl_surf_.reset();
  return EGL_TRUE;
}

EGLBoolean WindowSurface::QuerySurface(EGLint attribute, EGLint *value) {
  if (attribute == EGL_NATIVE_VISUAL_TYPE) {
    attribute = EGL_NATIVE_VISUAL_ID;
  }
  if (attribute == EGL_SURFACE_TYPE) {
    *value = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;
    return EGL_TRUE;
  } else if (attribute == EGL_NATIVE_VISUAL_ID) {
    *value = ANativeWindow_getFormat(native_window_);
    return EGL_TRUE;
  }
  if (Surface::QuerySurface(attribute, value)) {
    return EGL_TRUE;
  }
  ALOGD("eglQuerySurface display %p surface %p attribute 0x%04X failed : %s",
        egl_dpy_, this, attribute, proxy_->StrLastError().c_str());
  return EGL_FALSE;
}

EGLBoolean WindowSurface::SwapBuffers() {
  if (!Surface::SwapBuffers()) {
    ALOGD("eglSwapBuffers display %p surface %p failed : %s", egl_dpy_,
          egl_surf_.get(), proxy_->StrLastError().c_str());
    return EGL_FALSE;
  }

  auto image =
      std::make_shared<AndroidBufferImage>(egl_dpy_, proxy_, native_buffer_);

  auto created_width = created_state_->width;
  auto created_height = created_state_->height;

  SyncWait(in_fence_fd_);

  auto egl_image = image->CreateImage(EGL_NO_CONTEXT, nullptr);
  if (egl_image == EGL_NO_IMAGE) {
    ALOGD("eglSwapBuffers display %p surface %p failed : %s", egl_dpy_, this,
          proxy_->StrLastError().c_str());
    return false;
  }

  GLuint tmp_tex = {};
  GLint curr_tex_bind = {};
  GLint prev_read_fbo = {};
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex_bind);
  glGenTextures(1, &tmp_tex);
  glBindTexture(GL_TEXTURE_2D, tmp_tex);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, egl_image);

  // gles3
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_read_fbo);
  if (prev_read_fbo != 0) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }

  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, created_width,
                      created_height);

  if (prev_read_fbo != 0) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prev_read_fbo);
  }
  glDeleteTextures(1, &tmp_tex);
  glBindTexture(GL_TEXTURE_2D, curr_tex_bind);

  proxy_->Api().eglDestroyImage(egl_dpy_, egl_image);

  // clear GL errors, because its possible that the fbo format does not match
  // the format of the read buffer, in the case of OpenGL ES 3.1 and integer
  // RGBA formats.
  glGetError();

  glFinish();
  image.reset();
  QueueBuffer();

  GLsizei width = ANativeWindow_getWidth(native_window_);
  GLsizei height = ANativeWindow_getHeight(native_window_);

  if (created_width != width || created_height != height) {
    DestroySurface();
    if (auto egl_surf = CreateSurface(created_state_->config,
                                      created_state_->attribs.data());
        egl_surf == EGL_NO_SURFACE) {
      return EGL_FALSE;
    }
  } else {
    DequeueBuffer();
  }

  return EGL_TRUE;
}

void WindowSurface::DequeueBuffer() {
  assert(!native_buffer_);
  CloseFenceFd(in_fence_fd_);
  ANativeWindow_dequeueBuffer(native_window_, &native_buffer_, &in_fence_fd_);
}

void WindowSurface::QueueBuffer() {
  if (native_buffer_) {
    ANativeWindow_queueBuffer(native_window_, native_buffer_, -1);
    native_buffer_ = nullptr;
  }
}

void WindowSurface::CancelBuffer() {
  if (native_window_ && native_buffer_) {
    ANativeWindow_cancelBuffer(native_window_, native_buffer_, -1);
    native_buffer_ = nullptr;
  }
}

EGLSurface PassthroughSurface::CreateSurface(EGLConfig config,
                                             const EGLAttrib *attrib_list) {
  if (auto egl_surf = proxy_->Api().eglCreatePlatformWindowSurface(
          egl_dpy_, config, native_window_, attrib_list);
      egl_surf != EGL_NO_SURFACE) {
    SetEglSurface(egl_surf);
    return egl_surf;
  }
  return EGL_NO_SURFACE;
}

EGLBoolean PassthroughSurface::DestroySurface() {
  egl_surf_.reset();
  return EGL_TRUE;
}

EGLBoolean PassthroughSurface::QuerySurface(EGLint attribute, EGLint *value) {
  if (attribute == EGL_NATIVE_VISUAL_TYPE) {
    attribute = EGL_NATIVE_VISUAL_ID;
  }
  if (Surface::QuerySurface(attribute, value)) {
    return EGL_TRUE;
  }
  ALOGD("eglQuerySurface display %p surface %p attribute 0x%04X failed : %s",
        egl_dpy_, this, attribute, proxy_->StrLastError().c_str());
  return EGL_FALSE;
}

}  // namespace egl
