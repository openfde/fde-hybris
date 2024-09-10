#include "egl-proxy.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <gbm.h>
#include <log/log.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "egl-misc.h"

namespace {

constexpr char kEglLibName[] = "libEGL.so.1";  // soname
constexpr char kEglLibEnvName[] = "HYBRIS-EGL";

constexpr char kRenderDevPath[] = "/dev/dri/card0";

constexpr char kEglPlatformAndroid[] = "EGL_KHR_platform_android";

constexpr char kExtensionDelimiter = ' ';

constexpr int32_t kEglMajorVersion = 1;
constexpr int32_t kEglMinorMinVersion = 4;
constexpr int32_t kEglMinorCurrentVersion = 5;

const std::map<EGLint, std::string> kEglStrErrors{
#define STRERROR(error) {error, #error}
    STRERROR(EGL_SUCCESS),
    STRERROR(EGL_NOT_INITIALIZED),
    STRERROR(EGL_BAD_ACCESS),
    STRERROR(EGL_BAD_ALLOC),
    STRERROR(EGL_BAD_ATTRIBUTE),
    STRERROR(EGL_BAD_CONFIG),
    STRERROR(EGL_BAD_CONTEXT),
    STRERROR(EGL_BAD_CURRENT_SURFACE),
    STRERROR(EGL_BAD_DISPLAY),
    STRERROR(EGL_BAD_MATCH),
    STRERROR(EGL_BAD_NATIVE_PIXMAP),
    STRERROR(EGL_BAD_NATIVE_WINDOW),
    STRERROR(EGL_BAD_PARAMETER),
    STRERROR(EGL_BAD_SURFACE),
    STRERROR(EGL_CONTEXT_LOST),
    STRERROR(EGL_BAD_OUTPUT_LAYER_EXT),
    STRERROR(EGL_BAD_OUTPUT_PORT_EXT),
#undef STRERROR
};

// clang-format off
const std::set<std::string> kClientExtensions{
  "EGL_EXT_client_extensions",
  "EGL_KHR_client_get_all_proc_addresses",

  "EGL_EXT_device_base",
  "EGL_EXT_device_query", // for device and display attributes query functions
  "EGL_EXT_device_enumeration",
  "EGL_EXT_device_query_name",

  "EGL_EXT_platform_base", // for display and surface extensions functions.
  "EGL_EXT_platform_device",
  "EGL_EXT_platform_wayland",
  "EGL_EXT_platform_x11",
  "EGL_EXT_platform_xcb",
  "EGL_MESA_platform_xcb",
  "EGL_MESA_platform_gbm",
  "EGL_MESA_platform_surfaceless",

  "EGL_KHR_platform_wayland",
  "EGL_KHR_platform_x11",
  "EGL_KHR_platform_gbm",
  "EGL_KHR_platform_android",
};

const std::set<std::string> kExcludeForAndroidExtensions{
  "EGL_EXT_platform_base", // for display and surface extensions functions.
  "EGL_EXT_platform_device",
  "EGL_EXT_platform_wayland",
  "EGL_EXT_platform_x11",
  "EGL_EXT_platform_xcb",
  "EGL_MESA_platform_gbm",
  "EGL_MESA_platform_surfaceless",

  "EGL_KHR_platform_wayland",
  "EGL_KHR_platform_x11",
  "EGL_KHR_platform_gbm",
  "EGL_KHR_platform_android",
};

const std::set<std::string> kIncludeForAndroidExtensions{
  "EGL_EXT_client_extensions",
  "EGL_KHR_client_get_all_proc_addresses",

  "EGL_EXT_device_base",
  "EGL_EXT_device_query", // for device and display attributes query functions
  "EGL_EXT_device_enumeration",
};

// clang-format on

EGLDisplay GetPlatformDisplay(EGLenum platform, void *native_display,
                              const EGLAttrib *attrib_list) {
  if (auto get_display_addr =
          egl::EglProxy::Instance()->Api().eglGetProcAddress(
              "eglGetPlatformDisplayEXT");
      get_display_addr) {
    auto GetDisplay =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(get_display_addr);
    auto attribs = egl::misc::ConvertAttribToInt(attrib_list);
    return GetDisplay(platform, native_display, attribs.data());
  }
  return EGL_NO_DISPLAY;
}

}  // namespace

namespace egl {
bool EglProxy::Initialize() {
  if (!handle_) {
    return false;
  }
  InitializeApi();

  auto native_client_extensions =
      api_.eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  if (!native_client_extensions) {
    // not have EGL1.4 with EGL_EXT_client_extensions extension.
    ALOGE("eglQueryString with EGL_NO_DISPLAY failed: %s",
          StrLastError().c_str());
    return false;
  }

  auto HasExtensions = [](const char *extensions,
                          const std::vector<std::string> &exts) {
    for (auto const &ext : exts) {
      if (strstr(extensions, ext.c_str()) == nullptr) {
        ALOGW("%s extension not found", ext.c_str());
        return false;
      }
    }
    return true;
  };

  const std::vector<std::string> kEgl14Extensions{
      // "EGL_EXT_client_extensions",
      // "EGL_KHR_client_get_all_proc_addresses",

      "EGL_EXT_platform_base",
      "EGL_MESA_platform_gbm",
  };
  auto ok_14 = HasExtensions(native_client_extensions, kEgl14Extensions);

  const std::vector<std::string> kEgl15Extensions{
      // "EGL_EXT_client_extensions",
      // "EGL_KHR_get_all_proc_addresses",

      "EGL_KHR_platform_gbm",
  };
  auto ok_15 = HasExtensions(native_client_extensions, kEgl15Extensions);
  if (!ok_14 && !ok_15) {
    ALOGD(
        "The version of native EGL must be egl 1.4 with "
        "EGL_MESA_platform_gbm or egl 1.5 with EGL_KHR_platform_gbm");
    return false;
  }

  auto fd = open(kRenderDevPath, O_RDWR | O_CLOEXEC);
  if (fd < 0) {
    ALOGE("open %s failed: %s", kRenderDevPath, strerror(errno));
    return false;
  }
  auto gbm_dpy =
      std::shared_ptr<gbm_device>(gbm_create_device(fd), [fd](gbm_device *dev) {
        gbm_device_destroy(dev);
        close(fd);
      });
  if (!gbm_dpy) {
    ALOGE("Create Native Display failed");
    close(fd);
    return false;
  }
  auto target = EGL_PLATFORM_GBM_KHR;
  EGLDisplay egl_dpy = EGL_NO_DISPLAY;

  int32_t egl_major = kEglMajorVersion;
  int32_t egl_minor = 0;
  if (ok_15 && api_.eglGetPlatformDisplay) {
    auto native_dpy = reinterpret_cast<EGLNativeDisplayType>(gbm_dpy.get());
    egl_dpy = api_.eglGetPlatformDisplay(target, native_dpy, nullptr);
    egl_minor = kEglMinorCurrentVersion;
  } else if (ok_14 && api_.eglGetProcAddress) {
    if (auto get_display_addr =
            api_.eglGetProcAddress("eglGetPlatformDisplayEXT");
        get_display_addr) {
      auto GetDisplay =
          reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(get_display_addr);
      egl_dpy = GetDisplay(target, gbm_dpy.get(), nullptr);
      egl_minor = kEglMinorMinVersion;
    }
  }

  if (egl_dpy == EGL_NO_DISPLAY) {
    ALOGE("eglGetPlatformDisplay failed %s", StrLastError().c_str());
    return false;
  }

  auto reclaim_dpy = std::shared_ptr<void>(egl_dpy, [this](EGLDisplay dpy) {
    if (dpy != EGL_NO_DISPLAY) {
      api_.eglTerminate(dpy);
    }
  });

  if (!api_.eglInitialize(egl_dpy, nullptr, nullptr)) {
    ALOGE("eglInitialize failed : %s", StrLastError().c_str());
    return false;
  }

  auto apis = api_.eglQueryString(egl_dpy, EGL_CLIENT_APIS);
  if (!apis || strstr(apis, "OpenGL_ES") == nullptr) {
    ALOGE("Not implement OpenGL_ES client API");
    return false;
  }

  if (auto version = api_.eglQueryString(egl_dpy, EGL_VERSION); version) {
    auto GetEglVersion = [](const std::string &version_str, int32_t &major,
                            int32_t &minor) {
      if (auto pos = version_str.find('.'); pos != std::string::npos) {
        major = std::stoi(version_str.substr(0, pos));
        minor = std::stoi(version_str.substr(pos + 1));
      }
    };
    egl_minor = 0;
    GetEglVersion(version, egl_major, egl_minor);
  }

  if (egl_major != kEglMajorVersion || egl_minor < kEglMinorMinVersion) {
    ALOGE("EGL API version (%d.%d) is too low", egl_major, egl_minor);
    return false;
  }

  const std::vector<std::string> kExtensionsForDisplay{
      "EGL_EXT_image_dma_buf_import",
      // "EGL_EXT_image_dma_buf_import_modifiers",
  };

  auto display_extensions = api_.eglQueryString(egl_dpy, EGL_EXTENSIONS);
  auto ok_display = HasExtensions(display_extensions, kExtensionsForDisplay);

  auto client_extensions = misc::SplitBySpace(native_client_extensions);
  client_extensions_ = misc::SerializeExtensions(client_extensions,
                                                 kExcludeForAndroidExtensions);

  if (egl_major == kEglMajorVersion && egl_minor == kEglMinorCurrentVersion) {
    client_extensions_ += kEglPlatformAndroid;
    client_extensions_ += kExtensionDelimiter;
  }
  ALOGV("EGL client extension : %s", client_extensions_.c_str());

  ImplementEgl15Api();

  major_ = egl_major;
  minor_ = egl_minor;
  return ok_display;
}

EglProxy *EglProxy::Instance() {
  static bool inited = false;
  static EglProxyPtr egl{};
  static std::mutex mtx{};
  if (!inited) {
    std::lock_guard<std::mutex> guard{mtx};
    if (!inited) {
      egl = Load();
      std::atomic_thread_fence(std::memory_order::memory_order_release);
      inited = true;
    }
  }
  if (!egl) {
    ALOGE("Load and initialize egl proxy failed");
    abort();
  }
  return egl.get();
}

std::shared_ptr<void> EglProxy::LoadLibrary() {
  const char *env_name = getenv(kEglLibEnvName);
  auto name = env_name ? env_name : kEglLibName;
  return LoadLibrary(name);
}

std::shared_ptr<void> EglProxy::LoadLibrary(const char *name) {
  if (auto handle = dlopen(name, RTLD_LOCAL); handle) {
    return std::shared_ptr<void>(handle, [](void *h) {
      if (h) {
        dlclose(h);
      }
    });
  }
  ALOGE("dlopen %s failed: %s", name, dlerror());
  return nullptr;
}

void EglProxy::InitializeApi() {
  assert(handle_);
  // init api symbols
  auto handle = handle_.get();
  auto &api = api_;

#define GETSYMBOLADDR(symbol)                                           \
  do {                                                                  \
    api.symbol =                                                        \
        reinterpret_cast<decltype(api.symbol)>(dlsym(handle, #symbol)); \
  } while (0)

  // egl1.0
  GETSYMBOLADDR(eglChooseConfig);
  GETSYMBOLADDR(eglCopyBuffers);
  GETSYMBOLADDR(eglCreateContext);
  GETSYMBOLADDR(eglCreatePbufferSurface);
  GETSYMBOLADDR(eglCreatePixmapSurface);
  GETSYMBOLADDR(eglCreateWindowSurface);
  GETSYMBOLADDR(eglDestroyContext);
  GETSYMBOLADDR(eglDestroySurface);
  GETSYMBOLADDR(eglGetConfigAttrib);
  GETSYMBOLADDR(eglGetConfigs);
  GETSYMBOLADDR(eglGetCurrentDisplay);
  GETSYMBOLADDR(eglGetCurrentSurface);
  GETSYMBOLADDR(eglGetDisplay);
  GETSYMBOLADDR(eglGetError);
  GETSYMBOLADDR(eglGetProcAddress);
  GETSYMBOLADDR(eglInitialize);
  GETSYMBOLADDR(eglMakeCurrent);
  GETSYMBOLADDR(eglQueryContext);
  GETSYMBOLADDR(eglQueryString);
  GETSYMBOLADDR(eglQuerySurface);
  GETSYMBOLADDR(eglSwapBuffers);
  GETSYMBOLADDR(eglTerminate);
  GETSYMBOLADDR(eglWaitGL);
  GETSYMBOLADDR(eglWaitNative);

  // egl1.1
  GETSYMBOLADDR(eglBindTexImage);
  GETSYMBOLADDR(eglReleaseTexImage);
  GETSYMBOLADDR(eglSurfaceAttrib);
  GETSYMBOLADDR(eglSwapInterval);

  // egl1.2
  GETSYMBOLADDR(eglBindAPI);
  GETSYMBOLADDR(eglQueryAPI);
  GETSYMBOLADDR(eglCreatePbufferFromClientBuffer);
  GETSYMBOLADDR(eglReleaseThread);
  GETSYMBOLADDR(eglWaitClient);

  // egl1.4
  GETSYMBOLADDR(eglGetCurrentContext);

  // egl1.5
  GETSYMBOLADDR(eglCreateSync);
  GETSYMBOLADDR(eglDestroySync);
  GETSYMBOLADDR(eglClientWaitSync);
  GETSYMBOLADDR(eglGetSyncAttrib);
  GETSYMBOLADDR(eglCreateImage);
  GETSYMBOLADDR(eglDestroyImage);
  GETSYMBOLADDR(eglGetPlatformDisplay);
  GETSYMBOLADDR(eglCreatePlatformWindowSurface);
  GETSYMBOLADDR(eglCreatePlatformPixmapSurface);
  GETSYMBOLADDR(eglWaitSync);

#undef GETSYMBOLADDR
}

void EglProxy::ImplementEgl15Api() { ImplementByExtPlatorm(); }

void EglProxy::ImplementByExtPlatorm() {
  // EGL_EXT_platform_base

  api_.eglGetPlatformDisplay = GetPlatformDisplay;
}

EglProxyPtr EglProxy::Load() {
  if (auto egl_handle = LoadLibrary(); egl_handle) {
    if (auto egl = std::make_shared<EglProxy>(egl_handle); egl->Initialize()) {
      return egl;
    }
  }
  return nullptr;
}

EGLint EglProxy::EglError() {
  if (api_.eglGetError) {
    return api_.eglGetError();
  }
  return EGL_BAD_ALLOC;
}

std::string EglProxy::StrLastError() const {
  if (api_.eglGetError) {
    return StrError(api_.eglGetError());
  }
  return "Unknown";
}

std::string EglProxy::StrError(EGLint errnum) {
  if (auto it = kEglStrErrors.find(errnum); it != kEglStrErrors.end()) {
    return it->second.c_str();
  }

  char buf[8]{};
  snprintf(buf, sizeof(buf), "0x%04X", errnum);
  return buf;
}

}  // namespace egl
