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

WindowSurface::WindowSurface(EGLDisplay egl_dpy, EglProxyPtr proxy,
                             ANativeWindow *window)
    : Surface(egl_dpy, proxy), native_window_(window) {
  blit_ = std::make_shared<BlitFramebuffer>(proxy_, egl_dpy_);
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

  SyncWait(in_fence_fd_);

  blit_->Blit(native_buffer_);

  // clear GL errors, because its possible that the fbo format does not match
  // the format of the read buffer, in the case of OpenGL ES 3.1 and integer
  // RGBA formats.
  glGetError();

  glFinish();

  QueueBuffer();
  DequeueBuffer();
  return MaybeResize();
}

void WindowSurface::DequeueBuffer() {
  if (!native_buffer_) {
    CloseFenceFd(in_fence_fd_);
    ANativeWindow_dequeueBuffer(native_window_, &native_buffer_, &in_fence_fd_);
  }
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

EGLBoolean WindowSurface::MaybeResize() {
  auto created_width = created_state_->width;
  auto created_height = created_state_->height;
  GLsizei width = ANativeWindow_getWidth(native_window_);
  GLsizei height = ANativeWindow_getHeight(native_window_);
  if (created_width == width && created_height == height) {
    return EGL_TRUE;
  }
  ALOGD("Window resized : %dx%d -> %dx%d", created_width, created_height, width,
        height);
  auto &api = proxy_->Api();
  auto prev_context = api.eglGetCurrentContext();
  auto prev_read_surf = api.eglGetCurrentSurface(EGL_READ);
  auto prev_draw_surf = api.eglGetCurrentSurface(EGL_DRAW);
  auto prev_surf = egl_surf_.get();
  bool need_rebind = (prev_surf && (prev_read_surf == prev_surf ||
                                    prev_draw_surf == prev_surf));
  if (need_rebind) {
    api.eglMakeCurrent(egl_dpy_, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);
  }
  DestroySurface();
  auto egl_surf =
      CreateSurface(created_state_->config, created_state_->attribs.data());
  if (need_rebind) {
    auto read_surf = (prev_read_surf == prev_surf) ? egl_surf : prev_read_surf;
    auto draw_surf = (prev_draw_surf == prev_surf) ? egl_surf : prev_draw_surf;
    api.eglMakeCurrent(egl_dpy_, read_surf, draw_surf, prev_context);
  }
  return (egl_surf != EGL_NO_SURFACE);
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
