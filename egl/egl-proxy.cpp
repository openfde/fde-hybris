#include "egl-proxy.h"

#include <dlfcn.h>
#include <fcntl.h>
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

auto HasExtensions = [](const char *extensions,
                        const std::vector<std::string> &exts,
                        EGLenum target) {
  for (auto const &ext : exts) {
    if (strstr(extensions, ext.c_str()) == nullptr) {
      ALOGW("target(0x%X) : %s extension not found", target, ext.c_str());
      return false;
    }
  }
  return true;
};

auto HasExtension = [](const char *extensions, const char *ext) {
  return (strstr(extensions, ext) != nullptr);
};

EGLDisplay GetDisplayEarly(const egl::EglApi &api, EGLenum target) {
  EGLDisplay egl_dpy = EGL_NO_DISPLAY;
  if (api.eglGetPlatformDisplay) {
    egl_dpy = api.eglGetPlatformDisplay(target, EGL_DEFAULT_DISPLAY, nullptr);
  } else if (api.eglGetProcAddress) {
    auto native_client_extensions =
        api.eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!native_client_extensions ||
        !HasExtension(native_client_extensions, "EGL_EXT_platform_base")) {
      return EGL_NO_DISPLAY;
    }
    if (auto get_display_addr =
            api.eglGetProcAddress("eglGetPlatformDisplayEXT");
        get_display_addr) {
      auto GetDisplay =
          reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(get_display_addr);
      egl_dpy = GetDisplay(target, EGL_DEFAULT_DISPLAY, nullptr);
    }
  }
  return egl_dpy;
}

std::shared_ptr<void> GetPlatformDisplayExt(
    const egl::EglApi &api, EGLenum target,
    const std::vector<std::string> &needed_extensions) {
  auto egl_dpy = GetDisplayEarly(api, target);
  if (egl_dpy == EGL_NO_DISPLAY) {
    return nullptr;
  }
  auto terminate = api.eglTerminate;
  std::shared_ptr<void> dpy{egl_dpy,
                            [terminate](EGLDisplay d) { terminate(d); }};
  if (!api.eglInitialize(egl_dpy, nullptr, nullptr)) {
    return nullptr;
  }
  const EGLint neededAttribs[] = {
      EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
      EGL_RED_SIZE,     1,
      EGL_GREEN_SIZE,   1,
      EGL_BLUE_SIZE,    1,
      EGL_ALPHA_SIZE,   EGL_DONT_CARE,
      EGL_NONE,
  };
  EGLConfig selected_config;
  EGLint num_config = 0;
  if (auto ret = api.eglChooseConfig(egl_dpy, neededAttribs, &selected_config,
                                     1, &num_config);
      !ret || num_config == 0) {
    ALOGE("target(0x%X) : eglChooseConfig fail, ret:%d, num_config:%d",
      target, ret, num_config);
    return nullptr;
  }
  auto display_extensions = api.eglQueryString(egl_dpy, EGL_EXTENSIONS);
  if (display_extensions &&
      HasExtensions(display_extensions, needed_extensions, target)) {
    return dpy;
  }
  ALOGE("target(0x%X) : GetPlatformDisplayExt fail, extensions no found:%s",
    target, display_extensions);
  return nullptr;
}

std::shared_ptr<void> SelectPlatformDisplay(
    const egl::EglApi &api, const std::vector<EGLenum> &platforms,
    const std::vector<std::string> &needed_extensions,
    EGLenum &selected_target) {
  for (auto target : platforms) {
    if (auto dpy = GetPlatformDisplayExt(api, target, needed_extensions); dpy) {
      selected_target = target;
      return dpy;
    }
  }
  return nullptr;
}

}  // namespace

namespace egl {
bool EglProxy::Initialize() {
  if (!handle_) {
    return false;
  }
  InitializeApi();
  InitializeApiExtensions();

  int32_t egl_major = kEglMajorVersion;
  int32_t egl_minor = 0;

  const std::vector<std::string> kExtensionsForDisplay{
      "EGL_EXT_image_dma_buf_import",
      // "EGL_EXT_image_dma_buf_import_modifiers",
  };

  std::vector<EGLenum> platforms{EGL_PLATFORM_SURFACELESS_MESA,
                                 EGL_PLATFORM_GBM_KHR};
  EGLenum target;
  auto dpy =
      SelectPlatformDisplay(api_, platforms, kExtensionsForDisplay, target);
  if (!dpy) {
    ALOGE("Can not select suitable platform display for android");
    return false;
  }

  auto egl_dpy = dpy.get();
  has_surfaceless_ = (target == EGL_PLATFORM_SURFACELESS_MESA);

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
    GetEglVersion(version, egl_major, egl_minor);
  }

  if (egl_major != kEglMajorVersion || egl_minor <= kEglMinorMinVersion) {
    ALOGE("EGL API version (%d.%d) is too low", egl_major, egl_minor);
    return false;
  }

  auto native_client_extensions =
      api_.eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  auto vendor = api_.eglQueryString(egl_dpy, EGL_VENDOR);
  auto display_extensions = api_.eglQueryString(egl_dpy, EGL_EXTENSIONS);
  ALOGD("EGL host vendor : %s, version : %d.%d, apis : %s", vendor, egl_major,
        egl_minor, apis);
  ALOGD("EGL host client extensions : %s",
        native_client_extensions ? native_client_extensions : "NONE");
  ALOGD("EGL host display extensions : %s",
        display_extensions ? display_extensions : "NONE");
  ALOGD("Select EGL host platform : 0x%04X, surfaceless : %d", target, has_surfaceless_);

  auto client_extensions = misc::SplitBySpace(native_client_extensions);
  client_extensions_ = misc::SerializeExtensions(client_extensions,
                                                 kExcludeForAndroidExtensions);

  if (egl_major == kEglMajorVersion && egl_minor == kEglMinorCurrentVersion) {
    client_extensions_ += kEglPlatformAndroid;
    client_extensions_ += kExtensionDelimiter;
  }
  ALOGD("EGL client extensions : %s", client_extensions_.c_str());

  ImplementEgl15Api();

  major_ = egl_major;
  minor_ = egl_minor;
  return true;
}

EglProxyPtr &EglProxy::Instance() {
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
    // abort();
  }
  return egl;
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

void EglProxy::InitializeApiExtensions() {
  auto &api = api_;

  if (!api.eglGetProcAddress) {
    return;
  }
#define GETSYMBOLADDR(symbol)                                  \
  do {                                                         \
    auto addr = api.eglGetProcAddress(#symbol);                \
    api.symbol = reinterpret_cast<decltype(api.symbol)>(addr); \
  } while (0)

  GETSYMBOLADDR(eglSwapBuffersWithDamageKHR);
  GETSYMBOLADDR(eglQueryDmaBufFormatsEXT);
  GETSYMBOLADDR(eglQueryDmaBufModifiersEXT);

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
