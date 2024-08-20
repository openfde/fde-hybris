#include "display.h"

#include <EGL/egl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <array>
#include <mutex>

namespace {
std::mutex display_mtx;

constexpr int32_t kMaxDisplays = 128;
std::array<DisplayInfoPtr, kMaxDisplays> displays{};

auto FindInfoByDisplay(EGLDisplay display) {
  return std::find_if(
      displays.begin(), displays.end(),
      [display](auto const &info) { return info && info->display == display; });
}

auto FindIdleSlot() {
  return std::find_if(displays.begin(), displays.end(),
                      [](auto const &info) { return !info; });
}

}  // namespace

namespace display {
EGLint GetDisplayError() {
  if (EGLDisplay display = eglGetCurrentDisplay(); display != EGL_NO_DISPLAY) {
    return GetDisplayError(display);
  }
  return EGL_BAD_DISPLAY;
}

EGLint GetDisplayError(EGLDisplay display) {
  auto display_info = FindDispayInfo(display);
  if (!display_info) {
    return EGL_BAD_DISPLAY;
  }
  return display_info->error;
}

void SetDisplayError(EGLint error) {
  if (EGLDisplay display = eglGetCurrentDisplay(); display != EGL_NO_DISPLAY) {
    return SetDisplayError(display, error);
  }
}

void SetDisplayError(EGLDisplay display, EGLint error) {
  if (auto info = FindDispayInfo(display); info) {
    info->error = error;
  }
}

InfoT *FindDispayInfo(EGLDisplay display) {
  if (auto it = FindInfoByDisplay(display); it != displays.end()) {
    return it->get();
  }
  return nullptr;
}

bool InsertDisplayInfo(EGLDisplay display, void *native_display) {
  std::lock_guard<std::mutex> lock{display_mtx};
  if (auto display_info = FindDispayInfo(display); display_info) {
    display_info->error = EGL_BAD_MATCH;
    return display_info->real_display == native_display;
  }

  if (auto it = FindIdleSlot(); it != displays.end()) {
    *it = std::make_shared<InfoT>(display, native_display);
    return true;
  }
  SetDisplayError(EGL_BAD_ALLOC);
  return false;
}

void RemoveDisplay(EGLDisplay display) {
  std::lock_guard<std::mutex> lock{display_mtx};
  if (auto it = FindInfoByDisplay(display); it != displays.end()) {
    *it = nullptr;
  }
}

gbm_device *NewGbmDevice() {
  static int32_t gbm_fd = 0;
  if (gbm_fd == 0) {
    std::lock_guard<std::mutex> guard{display_mtx};
    if (gbm_fd == 0) {
      auto gbm_path = "/dev/dri/RenderD128";
      gbm_fd = open(gbm_path, O_RDWR | O_CLOEXEC);
    }
  }
  if (gbm_fd < 0) {
    return nullptr;
  }
  return gbm_create_device(gbm_fd);
}

}  // namespace display