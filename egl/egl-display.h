#pragma once

#ifndef EGL_EGL_PROTOTYPES
#define EGL_EGL_PROTOTYPES 0
#endif
#include <EGL/egl.h>

#include <array>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "egl-image.h"
#include "egl-proxy.h"
#include "egl-surface.h"
#include "gbm.h"

extern "C" {
struct gbm_device;
}

namespace egl {

class Display;
using DisplayPtr = std::shared_ptr<Display>;

using GbmDevicePtr = std::shared_ptr<gbm_device>;

class DisplayManager;
using DisplayManagerPtr = std::shared_ptr<DisplayManager>;

class DisplayManager {
 public:
  static DisplayManagerPtr &Instance();

  // get android display from gbm platform
  Display *GetDisplay(EGLNativeDisplayType display_id);
  Display *GetPlatformDisplay(EGLenum platform, void *native_display,
                              const EGLAttrib *attrib_list);
  Display *GetPlatformDisplayEXT(EGLenum platform, void *native_display,
                                 const EGLint *attrib_list);

  EGLBoolean Terminate(EGLDisplay egl_dpy);
  Display *FindDispay(EGLDisplay egl_dpy);

 private:
  static constexpr int32_t kMaxDisplays = 128;
  using DisplayIterator = std::array<DisplayPtr, kMaxDisplays>::iterator;

  DisplayIterator FindDisplayPos(EGLDisplay egl_dpy);
  DisplayIterator FindDisplayPosByParameters(EGLenum platform,
                                             void *native_display,
                                             const EGLAttrib *attrib_list);

  GbmDevicePtr NewGbmDevice();
  DisplayIterator FindIdleSlot();

  std::array<DisplayPtr, kMaxDisplays> displays_{};
  std::mutex mtx_;
};

class Display {
 private:
  struct ParameterT {
    ParameterT() = default;
    ParameterT(EGLenum platform, void *native_display,
               const EGLAttrib *attrib_list);
    EGLenum plt{};
    void *native_dpy{};
    std::vector<EGLAttrib> attribs;
  };

 public:
  Display(EglProxy *proxy) : proxy_(proxy) {}
  virtual ~Display();

  EGLDisplay GetEglDisplay() { return egl_dpy_.get(); }

  const char *GetEglExtensions();

  virtual EGLDisplay GetPlatformDisplay(void *native_display,
                                        const EGLAttrib *attrib_list) = 0;

  virtual Surface *CreatePlatformWindowSurface(
      EGLConfig config, void *native_window, const EGLAttrib *attrib_list) = 0;

  Surface *CreateWindowSurface(EGLConfig config, EGLNativeWindowType win,
                               const EGLint *attrib_list);

  Surface *CreatePlatformWindowSurfaceEXT(EGLConfig config, void *native_window,
                                          const EGLint *attrib_list);

  EGLBoolean DestroySurface(EGLSurface egl_surf);

  EGLBoolean QuerySurface(EGLSurface egl_surf, EGLint attribute, EGLint *value);
  EGLBoolean SwapBuffers(EGLSurface egl_surf);

  Surface *FindSurface(EGLSurface egl_surf);

  Image *CreateImage(EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
                     const EGLAttrib *attrib_list);
  EGLBoolean DestroyImage(EGLImageKHR img);

  Image *CreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
                        const EGLint *attrib_list);
  EGLBoolean DestroyImageKHR(EGLImageKHR img);

  virtual EGLBoolean ChooseConfig(const EGLint *attrib_list, EGLConfig *configs,
                                  EGLint config_size, EGLint *num_config);
  virtual EGLBoolean GetConfigAttrib(EGLConfig config, EGLint attribute,
                                     EGLint *value);

  void SetParameters(EGLenum platform, void *native_display,
                     const EGLAttrib *attrib_list);
  bool SameAs(EGLenum platform, void *native_display,
              const EGLAttrib *attrib_list);

 protected:
  void SetEglDisplay(EGLDisplay egl_dpy);
  bool AddSurface(SurfacePtr surface);
  void DeleteSurface(SurfacePtr surface);
  const SurfacePtr &FindSurfaceByEgl(EGLSurface egl_surf);

  std::shared_ptr<void> egl_dpy_;
  EglProxy *proxy_{};
  std::string extensions_;
  std::vector<SurfacePtr> surfaces_;
  std::mutex mtx_;
  bool inited_extensions_ = false;
  ImageManager image_manager_;
  ParameterT parameters_;
};

class AndroidDisplay : public Display {
 public:
  AndroidDisplay(EglProxy *proxy, GbmDevicePtr gbm)
      : Display(proxy), gbm_(std::move(gbm)) {}

  EGLDisplay GetPlatformDisplay(void *native_display,
                                const EGLAttrib *attrib_list) override;

  Surface *CreatePlatformWindowSurface(EGLConfig config, void *native_window,
                                       const EGLAttrib *attrib_list) override;

  EGLBoolean ChooseConfig(const EGLint *attrib_list, EGLConfig *configs,
                          EGLint config_size, EGLint *num_config) override;
  EGLBoolean GetConfigAttrib(EGLConfig config, EGLint attribute,
                             EGLint *value) override;

 private:
  GbmDevicePtr gbm_;
};

class GbmDisplay : public Display {
 public:
  GbmDisplay(EglProxy *proxy) : Display(proxy) {}

  EGLDisplay GetPlatformDisplay(void *native_display,
                                const EGLAttrib *attrib_list) override;

  Surface *CreatePlatformWindowSurface(EGLConfig config, void *native_window,
                                       const EGLAttrib *attrib_list) override;
};

}  // namespace egl