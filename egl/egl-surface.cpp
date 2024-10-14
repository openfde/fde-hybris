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
#include <gbm.h>

#include <cassert>
#include <map>
#include <mutex>

#include "egl-image.h"
#include "egl-misc.h"

namespace {

using GbmBoPtr = std::shared_ptr<gbm_bo>;
class GbmBufferImage : public egl::Image {
 public:
  GbmBufferImage(EGLDisplay egl_dpy, egl::EglProxy *proxy, GbmBoPtr buffer)
      : Image(egl_dpy, proxy), buffer_(std::move(buffer)) {}

  EGLImage CreateImage(EGLContext ctx, const EGLAttrib *attrib_list) override {
    auto native_buffer = buffer_.get();
    assert(native_buffer);
    if (!native_buffer) {
      // display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
      ALOGD("Native buffer is null");
      return EGL_NO_IMAGE;
    }

    EGLAttrib attribs[47];
    if (!FillAttribs(native_buffer, sizeof(attribs) / sizeof(*attribs),
                     attribs)) {
      // display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
      ALOGD("Cannot get native buffer info %p", native_buffer);
      return EGL_NO_IMAGE;
    }

    egl_image_ = proxy_->Api().eglCreateImage(
        egl_dpy_, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attribs);

    return egl_image_;
  }

 private:
  EGLBoolean FillAttribs(gbm_bo *native_buffer, size_t attribs_size,
                         EGLAttrib *attribs) {
    int32_t atti = 0;
    attribs[atti++] = EGL_WIDTH;
    attribs[atti++] = gbm_bo_get_width(native_buffer);
    attribs[atti++] = EGL_HEIGHT;
    attribs[atti++] = gbm_bo_get_height(native_buffer);
    attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
    attribs[atti++] = gbm_bo_get_format(native_buffer);

    auto n_planes = gbm_bo_get_plane_count(native_buffer);
    auto fd = gbm_bo_get_fd(native_buffer);

#define FILL_ATTRIBS(ith)                                              \
  {                                                                    \
    attribs[atti++] = EGL_DMA_BUF_PLANE##ith##_FD_EXT;                 \
    attribs[atti++] = fd;                                              \
    attribs[atti++] = EGL_DMA_BUF_PLANE##ith##_OFFSET_EXT;             \
    attribs[atti++] = gbm_bo_get_offset(native_buffer, ith);           \
    attribs[atti++] = EGL_DMA_BUF_PLANE##ith##_PITCH_EXT;              \
    attribs[atti++] = gbm_bo_get_stride_for_plane(native_buffer, ith); \
  }

    if (n_planes > 0) {
      FILL_ATTRIBS(0);
    }
    if (n_planes > 1) {
      FILL_ATTRIBS(1);
    }
    if (n_planes > 2) {
      FILL_ATTRIBS(2);
    }

#undef FILL_ATTRIBS

    attribs[atti++] = EGL_NONE;
    assert(static_cast<size_t>(atti) <= attribs_size);
    return EGL_TRUE;
  }

  GbmBoPtr buffer_;
};

class FrameBufferBinder {
 public:
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
      ALOGE("ColorBuffer::bindFbo: FBO not complete: 0x%#x", status);
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

EGLBoolean CopyFramebuffer(EGLImage src_image, EGLImage dest_image,
                           int32_t x_offset, int32_t y_offset, int32_t width,
                           int32_t height) {
  FrameBufferBinder read_fb(GL_READ_FRAMEBUFFER, src_image);
  FrameBufferBinder draw_fb(GL_DRAW_FRAMEBUFFER, dest_image);

  // Clear GL errors so that they don't interfere with subsequent operations
  glGetError();

  glBlitFramebuffer(x_offset, y_offset, width, height, x_offset, y_offset,
                    width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  if (auto err = glGetError(); err != GL_NO_ERROR) {
    ALOGE("glBlitFramebuffer : 0x%04X\n", err);
    return EGL_FALSE;
  }
  return EGL_TRUE;
}

EGLBoolean CopyFramebuffer(EGLDisplay egl_dpy, egl::Image *src_image,
                           egl::Image *dest_image, int32_t x_offset,
                           int32_t y_offset, int32_t width, int32_t height) {
  auto proxy = egl::EglProxy::Instance();
  auto src_egl_image = src_image->CreateImage(EGL_NO_CONTEXT, nullptr);
  auto dest_egl_image = dest_image->CreateImage(EGL_NO_CONTEXT, nullptr);
  if (src_egl_image == EGL_NO_IMAGE || dest_egl_image == EGL_NO_IMAGE) {
    ALOGE("eglCreateImage failed : %s", proxy->StrLastError().c_str());
    return EGL_FALSE;
  }

  auto AutoRecycle = [proxy, egl_dpy](EGLImage img) {
    return std::shared_ptr<void>(img, [proxy, egl_dpy](EGLImage image) {
      if (image != EGL_NO_IMAGE) {
        proxy->Api().eglDestroyImage(egl_dpy, image);
      }
    });
  };

  auto auto_src_egl_image = AutoRecycle(src_egl_image);
  auto auto_dest_egl_image = AutoRecycle(dest_egl_image);
  return CopyFramebuffer(src_egl_image, dest_egl_image, x_offset, y_offset,
                         width, height);
}

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
  auto format =
      misc::GetGbmFormatFromHalFormat(ANativeWindow_getFormat(native_window_));
  uint32_t flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
  if (auto gbm_surf = gbm_surface_create(gbm_, width, height, format, flags);
      gbm_surf) {
    gbm_surf_.reset(gbm_surf,
                    [](gbm_surface *surf) { gbm_surface_destroy(surf); });
    if (auto egl_surf = proxy_->Api().eglCreatePlatformWindowSurface(
            egl_dpy_, config, reinterpret_cast<EGLNativeWindowType>(gbm_surf),
            attrib_list);
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
  }
  ALOGD("eglCreateWindowSurface display %p failed : %s", egl_dpy_,
        proxy_->StrLastError().c_str());
  return EGL_NO_SURFACE;
}

EGLBoolean WindowSurface::DestroySurface() {
  CancelBuffer();
  egl_surf_.reset();
  gbm_surf_.reset();
  return EGL_TRUE;
}

EGLBoolean WindowSurface::QuerySurface(EGLint attribute, EGLint *value) {
  if (attribute == EGL_NATIVE_VISUAL_TYPE) {
    attribute = EGL_NATIVE_VISUAL_ID;
  }
  if (Surface::QuerySurface(attribute, value)) {
    if (attribute == EGL_SURFACE_TYPE) {
    } else if (attribute == EGL_NATIVE_VISUAL_ID) {
      *value = misc::GetHalFromFromGbmFormat(*value);
    }
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

  auto gbm_surf = gbm_surf_.get();
  auto bo = gbm_surface_lock_front_buffer(gbm_surf);
  if (!bo) {
    return EGL_FALSE;
  }

  std::shared_ptr<gbm_bo> auto_bo(
      bo, [gbm_surf](gbm_bo *b) { gbm_surface_release_buffer(gbm_surf, b); });

  auto src_image = std::make_shared<GbmBufferImage>(egl_dpy_, proxy_, auto_bo);
  auto dest_image =
      std::make_shared<AndroidBufferImage>(egl_dpy_, proxy_, native_buffer_);

  auto created_width = created_state_->width;
  auto created_height = created_state_->height;

  SyncWait(in_fence_fd_);

  if (!CopyFramebuffer(egl_dpy_, src_image.get(), dest_image.get(), 0, 0,
                       created_width, created_height)) {
    ALOGD("SwapBuffers failed");
    return EGL_FALSE;
  }

  // clear GL errors, because its possible that the fbo format does not match
  // the format of the read buffer, in the case of OpenGL ES 3.1 and integer
  // RGBA formats.
  glGetError();

  glFinish();
  src_image.reset();
  dest_image.reset();
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
