#include "gbm_proxy.h"

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

constexpr char kGbmLibName[] = "libgbm_mesa_proxy.so.1";  // soname
constexpr char kGbmLibEnvName[] = "HYBRIS-EGL";

bool GbmProxy::Initialize() {
  if (!handle_) {
    return false;
  }
  InitializeApi();
  return true;
}

GbmProxyPtr &GbmProxy::Instance() {
  static bool inited = false;
  static GbmProxyPtr gbm{};
  static std::mutex mtx{};
  if (!inited) {
    std::lock_guard<std::mutex> guard{mtx};
    if (!inited) {
      gbm = Load();
      std::atomic_thread_fence(std::memory_order::memory_order_release);
      inited = true;
    }
  }
  if (!gbm) {
    ALOGE("Load and initialize gbm proxy failed");
    // abort();
  }
  return gbm;
}

GbmProxyPtr GbmProxy::Load() {
  if (auto gbm_handle = LoadLibrary(); gbm_handle) {
    if (auto gbm = std::make_shared<GbmProxy>(gbm_handle); gbm->Initialize()) {
      return gbm;
    }
  }
  return nullptr;
}

std::shared_ptr<void> GbmProxy::LoadLibrary() {
  const char *env_name = getenv(kGbmLibEnvName);
  auto name = env_name ? env_name : kGbmLibName;
  return LoadLibrary(name);
}

std::shared_ptr<void> GbmProxy::LoadLibrary(const char *name) {
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

void GbmProxy::InitializeApi() {
  assert(handle_);
  // init api symbols
  auto handle = handle_.get();
  auto &api = api_;

#define GETSYMBOLADDR(symbol)                                           \
  do {                                                                  \
    api.symbol =                                                        \
        reinterpret_cast<decltype(api.symbol)>(dlsym(handle, #symbol)); \
  } while (0)

  GETSYMBOLADDR(gbm_device_get_fd);
  GETSYMBOLADDR(gbm_create_device);
  GETSYMBOLADDR(gbm_device_get_backend_name);
  GETSYMBOLADDR(gbm_device_is_format_supported);
  GETSYMBOLADDR(gbm_device_get_format_modifier_plane_count);
  GETSYMBOLADDR(gbm_device_destroy);

  GETSYMBOLADDR(gbm_bo_create);
  GETSYMBOLADDR(gbm_bo_create_with_modifiers);
  GETSYMBOLADDR(gbm_bo_create_with_modifiers2);
  GETSYMBOLADDR(gbm_bo_import);

  GETSYMBOLADDR(gbm_bo_map);
  GETSYMBOLADDR(gbm_bo_unmap);

  GETSYMBOLADDR(gbm_bo_get_width);
  GETSYMBOLADDR(gbm_bo_get_height);
  GETSYMBOLADDR(gbm_bo_get_stride);
  GETSYMBOLADDR(gbm_bo_get_stride_for_plane);
  GETSYMBOLADDR(gbm_bo_get_format);
  GETSYMBOLADDR(gbm_bo_get_bpp);
  GETSYMBOLADDR(gbm_bo_get_offset);
  GETSYMBOLADDR(gbm_bo_get_device);

  GETSYMBOLADDR(gbm_bo_get_handle);
  GETSYMBOLADDR(gbm_bo_get_fd);
  GETSYMBOLADDR(gbm_bo_get_modifier);
  GETSYMBOLADDR(gbm_bo_get_plane_count);
  GETSYMBOLADDR(gbm_bo_get_handle_for_plane);
  GETSYMBOLADDR(gbm_bo_get_fd_for_plane);

  GETSYMBOLADDR(gbm_bo_write);
  GETSYMBOLADDR(gbm_bo_set_user_data);
  GETSYMBOLADDR(gbm_bo_get_user_data);
  GETSYMBOLADDR(gbm_bo_destroy);

  GETSYMBOLADDR(gbm_surface_create);
  GETSYMBOLADDR(gbm_surface_create_with_modifiers);
  GETSYMBOLADDR(gbm_surface_create_with_modifiers2);
  GETSYMBOLADDR(gbm_surface_lock_front_buffer);
  GETSYMBOLADDR(gbm_surface_release_buffer);
  GETSYMBOLADDR(gbm_surface_has_free_buffers);
  GETSYMBOLADDR(gbm_surface_destroy);

  GETSYMBOLADDR(gbm_format_get_name);

#undef GETSYMBOLADDR
}

