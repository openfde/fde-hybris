#pragma once

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES/gl.h>

#include <cassert>
#include <system/window.h>
#include <vector>

class EglSurface {
public:
  explicit EglSurface(EGLDisplay display, EGLSurface real_surface)
      : display_(display), real_surface_(real_surface) {}
  explicit EglSurface(EGLDisplay display, EGLNativeWindowType window)
      : display_(display), android_window_(window) {}
  EGLSurface GetSurface() { return real_surface_; }

  void SetReal(EGLSurface surface, EGLNativeWindowType window = {}) {
    real_surface_ = surface;
    real_window_ = window;
  }

  void SetConfigAndAttribs(EGLConfig config, std::vector<EGLint> attribs) {
    config_ = config;
    attribs_ = std::move(attribs);
  }

  uint32_t GetSurfaceWidth() const;
  uint32_t GetSurfaceHeight() const;

  uint32_t GetWindowWidth() const;
  uint32_t GetWindowHeight() const;
  uint32_t GetFormat() const;
  uint64_t GetUsage() const;

  uint32_t GetNativeBufferWidth() const;
  uint32_t GetNativeBufferHeight() const;

  ANativeWindowBuffer *GetNativeBuffer() { return buffer_; }

  EGLConfig GetEglConfig() const { return config_; }

  const EGLint *GetAttribs() const { return attribs_.data(); }

  void DequeueBuffer();
  void QueueBuffer();
  void CancelBuffer();

  void UpdateSurface();

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

  static EglSurface *From(EGLSurface surface) {
    return reinterpret_cast<EglSurface *>(surface);
  }

private:
  void UpdateSurfaceSize(uint32_t width, uint32_t height);

  EGLDisplay display_ = {};
  EGLSurface real_surface_ = EGL_NO_SURFACE;
  EGLNativeWindowType real_window_ = {};
  EGLNativeWindowType android_window_ = {};
  EGLConfig config_ = {};
  std::vector<EGLint> attribs_ = {};
  ANativeWindowBuffer *buffer_ = {};
};