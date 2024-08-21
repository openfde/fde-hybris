#pragma once

#include <EGL/egl.h>
#include <system/window.h>

#include <cassert>

class EglSurface {
public:
  explicit EglSurface(EGLSurface real_surface) : real_surface_(real_surface) {}
  explicit EglSurface(EGLNativeWindowType window) : android_window_(window) {}
  EGLSurface GetSurface() { return real_surface_; }

  void SetReal(EGLSurface surface, EGLNativeWindowType window = {}) {
    real_surface_ = surface;
    real_window_ = window;
  }

  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  uint32_t GetFormat() const;
  uint32_t GetFlags() const;

  void DequeueBuffer();
  void QueueBuffer();
  void CancelBuffer();

  ANativeWindow *GetWindow() {
    return reinterpret_cast<ANativeWindow *>(android_window_);
  }

  ANativeWindow *GetWindow() const {
    return reinterpret_cast<ANativeWindow *>(android_window_);
  }

  bool IsWindow() const { return android_window_ != nullptr; }

  static void InsertSurface(EglSurface *surface);
  static void RemoveSurface(EGLSurface surface);
  static EglSurface *FindSurface(EGLSurface surface);

private:
  EGLSurface real_surface_ = EGL_NO_SURFACE;
  EGLNativeWindowType real_window_ = {};
  EGLNativeWindowType android_window_ = {};
  ANativeWindowBuffer *buffer_ = {};
};