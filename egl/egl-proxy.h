#pragma once

#ifndef EGL_EGL_PROTOTYPES
#define EGL_EGL_PROTOTYPES 0
#endif
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <memory>
#include <string>

namespace egl {

struct Egl10 {
  PFNEGLCHOOSECONFIGPROC eglChooseConfig = {};
  PFNEGLCOPYBUFFERSPROC eglCopyBuffers = {};
  PFNEGLCREATECONTEXTPROC eglCreateContext = {};
  PFNEGLCREATEPBUFFERSURFACEPROC eglCreatePbufferSurface = {};
  PFNEGLCREATEPIXMAPSURFACEPROC eglCreatePixmapSurface = {};
  PFNEGLCREATEWINDOWSURFACEPROC eglCreateWindowSurface = {};
  PFNEGLDESTROYCONTEXTPROC eglDestroyContext = {};
  PFNEGLDESTROYSURFACEPROC eglDestroySurface = {};
  PFNEGLGETCONFIGATTRIBPROC eglGetConfigAttrib = {};
  PFNEGLGETCONFIGSPROC eglGetConfigs = {};
  PFNEGLGETCURRENTDISPLAYPROC eglGetCurrentDisplay = {};
  PFNEGLGETCURRENTSURFACEPROC eglGetCurrentSurface = {};
  PFNEGLGETDISPLAYPROC eglGetDisplay = {};
  PFNEGLGETERRORPROC eglGetError = {};
  PFNEGLGETPROCADDRESSPROC eglGetProcAddress = {};
  PFNEGLINITIALIZEPROC eglInitialize = {};
  PFNEGLMAKECURRENTPROC eglMakeCurrent = {};
  PFNEGLQUERYCONTEXTPROC eglQueryContext = {};
  PFNEGLQUERYSTRINGPROC eglQueryString = {};
  PFNEGLQUERYSURFACEPROC eglQuerySurface = {};
  PFNEGLSWAPBUFFERSPROC eglSwapBuffers = {};
  PFNEGLTERMINATEPROC eglTerminate = {};
  PFNEGLWAITGLPROC eglWaitGL = {};
  PFNEGLWAITNATIVEPROC eglWaitNative = {};
};

struct Egl11 : public Egl10 {
  PFNEGLBINDTEXIMAGEPROC eglBindTexImage = {};
  PFNEGLRELEASETEXIMAGEPROC eglReleaseTexImage = {};
  PFNEGLSURFACEATTRIBPROC eglSurfaceAttrib = {};
  PFNEGLSWAPINTERVALPROC eglSwapInterval = {};
};

struct Egl12 : public Egl11 {
  PFNEGLBINDAPIPROC eglBindAPI = {};
  PFNEGLQUERYAPIPROC eglQueryAPI = {};
  PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC eglCreatePbufferFromClientBuffer = {};
  PFNEGLRELEASETHREADPROC eglReleaseThread = {};
  PFNEGLWAITCLIENTPROC eglWaitClient = {};
};

struct Egl14 : public Egl12 {
  PFNEGLGETCURRENTCONTEXTPROC eglGetCurrentContext = {};
};

struct Egl15 : public Egl14 {
  PFNEGLCREATESYNCPROC eglCreateSync = {};
  PFNEGLDESTROYSYNCPROC eglDestroySync = {};
  PFNEGLCLIENTWAITSYNCPROC eglClientWaitSync = {};
  PFNEGLGETSYNCATTRIBPROC eglGetSyncAttrib = {};
  PFNEGLCREATEIMAGEPROC eglCreateImage = {};
  PFNEGLDESTROYIMAGEPROC eglDestroyImage = {};
  PFNEGLGETPLATFORMDISPLAYPROC eglGetPlatformDisplay = {};
  PFNEGLCREATEPLATFORMWINDOWSURFACEPROC eglCreatePlatformWindowSurface = {};
  PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC eglCreatePlatformPixmapSurface = {};
  PFNEGLWAITSYNCPROC eglWaitSync = {};
};

struct EglApi : public Egl15 {};

class EglProxy;

using EglProxyPtr = std::shared_ptr<EglProxy>;

class EglProxy {
 public:
  static EglProxy *Instance();

  EglProxy(std::shared_ptr<void> handle) : handle_(std::move(handle)) {}

  bool Initialize();

  const EglApi &Api() const { return api_; }
  std::string StrLastError() const;
  const char *GetClientExtensions() const {
    if (client_extensions_.empty()) {
      return nullptr;
    }
    return client_extensions_.c_str();
  }

  EGLint EglError();

  static std::string StrError(EGLint errnum);
  static std::shared_ptr<void> LoadLibrary();
  static std::shared_ptr<void> LoadLibrary(const char *name);

 private:
  void InitializeApi();
  void ImplementEgl15Api();

  void ImplementByExtPlatorm();

  static EglProxyPtr Load();

  EglApi api_{};
  std::shared_ptr<void> handle_ = {};
  int32_t major_ = {};
  int32_t minor_ = {};

  std::string client_extensions_;
};

}  // namespace egl
