#pragma once

#include <EGL/egl.h>

#include <memory>

#include "gbm.h"

namespace display {

struct InfoT {
  InfoT() = default;
  explicit InfoT(EGLDisplay display, void *real_display)
      : display(display), real_display(real_display) {}
  EGLDisplay display{};
  void *real_display{};
  EGLint error = EGL_SUCCESS;
};

EGLint GetDisplayError();
EGLint GetDisplayError(EGLDisplay display);

void SetDisplayError(EGLint error);
void SetDisplayError(EGLDisplay display, EGLint error);

InfoT *FindDispayInfo(EGLDisplay display);

bool InsertDisplayInfo(EGLDisplay display, void *native_display);
void RemoveDisplay(EGLDisplay display);

gbm_device *NewGbmDevice();

}  // namespace display

using DisplayInfoT = display::InfoT;
using DisplayInfoPtr = std::shared_ptr<DisplayInfoT>;
