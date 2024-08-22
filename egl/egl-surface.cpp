#include "egl-surface.h"

#include <android/native_window.h>
#include <vndk/window.h>

#include <GLES2/gl2.h>

#include <cassert>
#include <map>
#include <mutex>

namespace {

std::map<EGLSurface, EglSurface *> egl_surfaces;
std::mutex surface_mtx{};

} // namespace

uint32_t EglSurface::GetSurfaceWidth() const {
  GLint width = {};
  if (auto ret = eglQuerySurface(display_, real_surface_, EGL_WIDTH, &width);
      ret) {
    return width;
  }
  return 0;
}

uint32_t EglSurface::GetSurfaceHeight() const {
  GLint height = {};
  if (auto ret = eglQuerySurface(display_, real_surface_, EGL_HEIGHT, &height);
      ret) {
    return height;
  }
  return 0;
}

uint32_t EglSurface::GetWindowWidth() const {
  if (auto window = GetWindow(); window) {
    return ANativeWindow_getWidth(window);
  }
  return 0;
}

uint32_t EglSurface::GetWindowHeight() const {
  if (auto window = GetWindow(); window) {
    return ANativeWindow_getHeight(window);
  }
  return 0;
}

uint32_t EglSurface::GetFormat() const {
  if (auto window = GetWindow(); window) {
    return ANativeWindow_getFormat(window);
  }
  return 0;
}

uint64_t EglSurface::GetUsage() const { return 0; }

uint32_t EglSurface::GetNativeBufferWidth() const {
  if (buffer_) {
    return buffer_->width;
  }
  return 0;
}

uint32_t EglSurface::GetNativeBufferHeight() const {
  if (buffer_) {
    return buffer_->height;
  }
  return 0;
}

void EglSurface::DequeueBuffer() {
  assert(!buffer_);
  if (auto window = GetWindow(); window) {
    ANativeWindow_dequeueBuffer(window, &buffer_, nullptr);
  }
}

void EglSurface::QueueBuffer() {
  if (buffer_) {
    assert(GetWindow());
    auto window = GetWindow();

    ANativeWindow_queueBuffer(window, buffer_, 0);
    buffer_ = nullptr;
  }
}

void EglSurface::CancelBuffer() {
  if (GetWindow() && buffer_) {
    auto window = GetWindow();
    ANativeWindow_cancelBuffer(window, buffer_, 0);
    buffer_ = nullptr;
  }
}

void EglSurface::UpdateSurface() {
  assert(real_surface_);
  auto width = GetWindowWidth();
  auto height = GetWindowHeight();
  if (GetSurfaceWidth() != width || GetSurfaceHeight() != height) {
    eglDestroySurface(display_, real_surface_);
    UpdateSurfaceSize(width, height);
    real_surface_ = eglCreatePbufferSurface(display_, config_, attribs_.data());
    assert(real_surface_);
  }
}

void EglSurface::InsertSurface(EglSurface *surface) {
  assert(surface);
  assert(surface->GetSurface());
  std::lock_guard<std::mutex> guard{surface_mtx};
  egl_surfaces.emplace(surface->GetSurface(), surface);
}

void EglSurface::RemoveSurface(EGLSurface surface) {
  if (surface != EGL_NO_SURFACE) {
    std::lock_guard<std::mutex> guard{surface_mtx};
    egl_surfaces.erase(surface);
  }
}

EglSurface *EglSurface::FindSurface(EGLSurface surface) {
  if (surface == EGL_NO_SURFACE) {
    return nullptr;
  }
  std::lock_guard<std::mutex> guard{surface_mtx};
  if (auto it = egl_surfaces.find(surface); it != egl_surfaces.end()) {
    return it->second;
  }
  return nullptr;
}

void EglSurface::UpdateSurfaceSize(uint32_t width, uint32_t height) {
  assert(!attribs_.empty());
  for (auto it = attribs_.begin(); *it != EGL_NONE; it += 2) {
    if (*it == EGL_WIDTH) {
      *std::next(it) = width;
    } else if (*it == EGL_HEIGHT) {
      *std::next(it) = height;
    }
  }
}
