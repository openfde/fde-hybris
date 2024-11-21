#include "egl-image.h"

#include <log/log.h>
#include <u_gralloc/u_gralloc.h>

#include <atomic>
#include <cassert>
#include <set>
#include <vector>

#include "egl-misc.h"

namespace {}  // namespace

namespace egl {

bool ImageManager::AddImage(ImagePtr image) {
  assert(image);
  std::lock_guard<std::mutex> guard{mtx_};
  auto added = images_.emplace(image->GetEglImage(), std::move(image));
  return added.second;
}
void ImageManager::DeleteImage(EGLImage egl_image) {
  std::lock_guard<std::mutex> guard{mtx_};
  images_.erase(egl_image);
}

Image *ImageManager::FindImage(EGLImage egl_image) {
  if (egl_image == EGL_NO_IMAGE) {
    return nullptr;
  }
  std::lock_guard<std::mutex> guard{mtx_};
  if (auto it = images_.find(egl_image); it != images_.end()) {
    return it->second.get();
  }
  return nullptr;
}

void ImageManager::ClearImages() {
  std::lock_guard<std::mutex> guard{mtx_};
  images_.clear();
}

Image::~Image() {
  if (egl_image_ != EGL_NO_IMAGE) {
    proxy_->Api().eglDestroyImage(egl_dpy_, egl_image_);
  }
}

EGLBoolean Image::DestroyImage() {
  auto const &api = proxy_->Api();
  if (egl_image_ != EGL_NO_IMAGE) {
    auto ret = api.eglDestroyImage(egl_dpy_, egl_image_);
    egl_image_ = EGL_NO_IMAGE;
    return ret;
  }
  return EGL_FALSE;
}

AndroidBufferImage::AndroidBufferImage(EGLDisplay egl_dpy, EglProxyPtr proxy,
                                       ANativeWindowBuffer *buffer,
                                       const EGLAttrib *attrib_list)
    : Image(egl_dpy, proxy) {
  attribs_ = misc::DupAttributes(attrib_list);
  if (buffer) {
    auto hardware_buffer = ANativeWindowBuffer_getHardwareBuffer(buffer);
    AHardwareBuffer_acquire(hardware_buffer);
    buffer_.reset(buffer, [](ANativeWindowBuffer *native_buffer) {
      auto hardware_buffer =
          ANativeWindowBuffer_getHardwareBuffer(native_buffer);
      AHardwareBuffer_release(hardware_buffer);
    });
  }
}

EGLImage AndroidBufferImage::CreateImage() {
  auto native_buffer = buffer_.get();
  assert(native_buffer);
  if (!native_buffer ||
      native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC ||
      native_buffer->common.version != sizeof(*native_buffer)) {
    // display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
    if (native_buffer) {
      ALOGD("Not valid ANativeWindowBuffer: magic = 0x%04X, version = %d",
            native_buffer->common.magic, native_buffer->common.magic);
    } else {
      ALOGD("Not valid ANativeWindowBuffer, native buffer is null");
    }
    return EGL_NO_IMAGE;
  }

  std::vector<EGLAttrib> attribs(47);
  if (!FillAttribs(native_buffer, attribs.size(), attribs.data())) {
    // display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
    ALOGD("Cannot get native buffer info %p", native_buffer);
    return EGL_NO_IMAGE;
  }

  attribs.resize(misc::EglAttrbCount(attribs.data()) * 2 + 1);
  misc::ApendAttributes(attribs_.data(), attribs);
  egl_image_ = proxy_->Api().eglCreateImage(
      egl_dpy_, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attribs.data());

  return egl_image_;
}

GrallocPtr &AndroidBufferImage::GetGralloc() {
  static bool inited_gralloc = false;
  static GrallocPtr gralloc;
  static std::mutex mtx;
  if (!inited_gralloc) {
    std::lock_guard<std::mutex> guard{mtx};
    if (!inited_gralloc) {
      if (auto gr = u_gralloc_create(U_GRALLOC_TYPE_LIBDRM); gr) {
        gralloc.reset(gr, [](u_gralloc *gr) { u_gralloc_destroy(&gr); });
      }
      std::atomic_thread_fence(std::memory_order::memory_order_release);
      inited_gralloc = true;
    }
  }
  return gralloc;
}

EGLBoolean AndroidBufferImage::FillAttribs(
    const ANativeWindowBuffer *native_buffer, size_t attribs_size,
    EGLAttrib *attribs) {
  int32_t atti = 0;
  u_gralloc_buffer_handle buffer_handle = {
      native_buffer->handle, native_buffer->format, native_buffer->stride};

  u_gralloc_buffer_basic_info buffer_basic_info{};
  u_gralloc_buffer_color_info buffer_color_info{};
  auto gralloc = GetGralloc().get();
  if (!gralloc || u_gralloc_get_buffer_basic_info(gralloc, &buffer_handle,
                                                  &buffer_basic_info)) {
    return EGL_FALSE;
  }
  u_gralloc_get_buffer_color_info(gralloc, &buffer_handle, &buffer_color_info);

  attribs[atti++] = EGL_WIDTH;
  attribs[atti++] = native_buffer->width;
  attribs[atti++] = EGL_HEIGHT;
  attribs[atti++] = native_buffer->height;
  attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
  attribs[atti++] = buffer_basic_info.drm_fourcc;

  // egl mybe return bad attribute when uncomment below statements.
  // attribs[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
  // attribs[atti++] = buffer_color_info.sample_range;
  // attribs[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
  // attribs[atti++] = buffer_color_info.yuv_color_space;
  // attribs[atti++] = EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT;
  // attribs[atti++] = buffer_color_info.horizontal_siting;
  // attribs[atti++] = EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT;
  // attribs[atti++] = buffer_color_info.vertical_siting;

  auto n_planes = buffer_basic_info.num_planes;
  auto fds = buffer_basic_info.fds;
  auto offsets = buffer_basic_info.offsets;
  auto strides = buffer_basic_info.strides;
  // some vendor egl library not support EGL_EXT_image_dma_buf_import_modifiers
  // auto modifier = buffer_basic_info.modifier;
  if (n_planes > 0) {
    attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
    attribs[atti++] = fds[0];
    attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
    attribs[atti++] = offsets[0];
    attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
    attribs[atti++] = strides[0];

    // attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
    // attribs[atti++] = modifier & 0xFFFFFFFF;
    // attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
    // attribs[atti++] = modifier >> 32;
  }

  if (n_planes > 1) {
    attribs[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
    attribs[atti++] = fds[1];
    attribs[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
    attribs[atti++] = offsets[1];
    attribs[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
    attribs[atti++] = strides[1];

    // attribs[atti++] = EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT;
    // attribs[atti++] = modifier & 0xFFFFFFFF;
    // attribs[atti++] = EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT;
    // attribs[atti++] = modifier >> 32;
  }

  if (n_planes > 2) {
    attribs[atti++] = EGL_DMA_BUF_PLANE2_FD_EXT;
    attribs[atti++] = fds[2];
    attribs[atti++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
    attribs[atti++] = offsets[2];
    attribs[atti++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
    attribs[atti++] = strides[2];

    // attribs[atti++] = EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT;
    // attribs[atti++] = modifier & 0xFFFFFFFF;
    // attribs[atti++] = EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT;
    // attribs[atti++] = modifier >> 32;
  }

  attribs[atti++] = EGL_NONE;

  assert(static_cast<size_t>(atti) <= attribs_size);
  return EGL_TRUE;
}

GlBufferImage::GlBufferImage(EGLDisplay egl_dpy, EglProxyPtr proxy,
                             EGLenum target, GLuint buffer, EGLContext ctx,
                             const EGLAttrib *attrib_list)
    : Image(egl_dpy, proxy), target_(target), buffer_(buffer), ctx_(ctx) {
  attribs_ = misc::DupAttributes(attrib_list);
}

EGLImage GlBufferImage::CreateImage() {
  auto buffer = reinterpret_cast<EGLClientBuffer>(buffer_);
  egl_image_ = proxy_->Api().eglCreateImage(egl_dpy_, ctx_, target_, buffer,
                                            attribs_.data());
  return egl_image_;
}

PassthroughImage::PassthroughImage(EGLDisplay egl_dpy, EglProxyPtr proxy,
                                   EGLenum target, EGLClientBuffer buffer,
                                   EGLContext ctx, const EGLAttrib *attrib_list)
    : Image(egl_dpy, proxy), target_(target), buffer_(buffer), ctx_(ctx) {
  attribs_ = misc::DupAttributes(attrib_list);
}

EGLImage PassthroughImage::CreateImage() {
  egl_image_ = proxy_->Api().eglCreateImage(egl_dpy_, ctx_, target_, buffer_,
                                            attribs_.data());
  return egl_image_;
}

}  // namespace egl