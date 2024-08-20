#include "egl-surface.h"

#include <android/native_window.h>

#include <cassert>
#include <map>
#include <mutex>

namespace {

std::map<EGLSurface, EglSurface *> egl_surfaces;
std::mutex surface_mtx{};

} // namespace

uint32_t EglSurface::GetWidth() const {
  if (auto window = GetWindow(); window) {
    return ANativeWindow_getWidth(window);
  }
  return 0;
}

uint32_t EglSurface::GetHeight() const {
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

uint32_t EglSurface::GetFlags() const {
  assert(false);
  return 0;
}

void EglSurface::DequeueBuffer() {
  assert(!buffer_);
  if (auto window = GetWindow(); window) {
    window->dequeueBuffer(window, &buffer_, nullptr);
  }
}

void EglSurface::QueueBuffer() {
  if (buffer_) {
    assert(GetWindow());
    auto window = GetWindow();
    window->queueBuffer(window, buffer_, 0);
    buffer_ = nullptr;
  }
}

void EglSurface::CancelBuffer() {
  if (GetWindow() && buffer_) {
    auto window = GetWindow();
    window->cancelBuffer(window, buffer_, 0);
    buffer_ = nullptr;
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
