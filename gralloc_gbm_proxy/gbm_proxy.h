#ifndef _GBM_PROXY_H_
#define _GBM_PROXY_H_

#include "gbm.h"

#include <memory>
#include <string>

typedef int (*gbm_device_get_fd_t)(struct gbm_device *);
typedef struct gbm_device *(*gbm_create_device_t)(int);

typedef void *(*gbm_bo_get_user_data_t)(struct gbm_bo *);
typedef const char *(*gbm_device_get_backend_name_t)(struct gbm_device *);
typedef int (*gbm_device_is_format_supported_t)(struct gbm_device *, uint32_t, uint32_t);
typedef int (*gbm_device_get_format_modifier_plane_count_t)(struct gbm_device *, uint32_t, uint64_t);
typedef void (*gbm_device_destroy_t)(struct gbm_device *);

typedef struct gbm_bo *(*gbm_bo_create_t)(struct gbm_device *, uint32_t, uint32_t, uint32_t, uint32_t);
typedef struct gbm_bo *(*gbm_bo_create_with_modifiers_t)(struct gbm_device *, uint32_t, uint32_t, uint32_t, const uint64_t *, const unsigned int);
typedef struct gbm_bo *(*gbm_bo_create_with_modifiers2_t)(struct gbm_device *, uint32_t, uint32_t, uint32_t, const uint64_t *, const unsigned int, uint32_t);
typedef struct gbm_bo *(*gbm_bo_import_t)(struct gbm_device *, uint32_t, void *, uint32_t);

typedef void *(*gbm_bo_map_t)(struct gbm_bo *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *, void **);
typedef void (*gbm_bo_unmap_t)(struct gbm_bo *, void *);

typedef uint32_t (*gbm_bo_get_width_t)(struct gbm_bo *);
typedef uint32_t (*gbm_bo_get_height_t)(struct gbm_bo *);
typedef uint32_t (*gbm_bo_get_stride_t)(struct gbm_bo *);
typedef uint32_t (*gbm_bo_get_stride_for_plane_t)(struct gbm_bo *, int);
typedef uint32_t (*gbm_bo_get_format_t)(struct gbm_bo *);
typedef uint32_t (*gbm_bo_get_bpp_t)(struct gbm_bo *);
typedef uint32_t (*gbm_bo_get_offset_t)(struct gbm_bo *, int);
typedef struct gbm_device *(*gbm_bo_get_device_t)(struct gbm_bo *);

typedef union gbm_bo_handle (*gbm_bo_get_handle_t)(struct gbm_bo *);
typedef int (*gbm_bo_get_fd_t)(struct gbm_bo *);
typedef uint64_t (*gbm_bo_get_modifier_t)(struct gbm_bo *);
typedef int (*gbm_bo_get_plane_count_t)(struct gbm_bo *);
typedef union gbm_bo_handle (*gbm_bo_get_handle_for_plane_t)(struct gbm_bo *, int);
typedef int (*gbm_bo_get_fd_for_plane_t)(struct gbm_bo *, int);
typedef int (*gbm_bo_write_t)(struct gbm_bo *, const void *, size_t);
typedef void (*gbm_bo_set_user_data_t)(struct gbm_bo *, void *, void (*)(struct gbm_bo *, void *));
typedef void (*gbm_bo_destroy_t)(struct gbm_bo *);

typedef struct gbm_surface *(*gbm_surface_create_t)(struct gbm_device *, uint32_t, uint32_t, uint32_t, uint32_t);
typedef struct gbm_surface *(*gbm_surface_create_with_modifiers_t)(struct gbm_device *, uint32_t, uint32_t, uint32_t, const uint64_t *, const unsigned int);
typedef struct gbm_surface *(*gbm_surface_create_with_modifiers2_t)(struct gbm_device *, uint32_t, uint32_t, uint32_t, const uint64_t *, const unsigned int, uint32_t);
typedef struct gbm_bo *(*gbm_surface_lock_front_buffer_t)(struct gbm_surface *);
typedef void (*gbm_surface_release_buffer_t)(struct gbm_surface *, struct gbm_bo *);
typedef int (*gbm_surface_has_free_buffers_t)(struct gbm_surface *);
typedef void (*gbm_surface_destroy_t)(struct gbm_surface *);

typedef char *(*gbm_format_get_name_t)(uint32_t, struct gbm_format_name_desc *);

struct GbmApi {
    gbm_device_get_fd_t                     gbm_device_get_fd = nullptr;
    gbm_create_device_t                     gbm_create_device = nullptr;

    gbm_bo_get_user_data_t                  gbm_bo_get_user_data = nullptr;
    gbm_device_get_backend_name_t           gbm_device_get_backend_name = nullptr;
    gbm_device_is_format_supported_t        gbm_device_is_format_supported = nullptr;
    gbm_device_get_format_modifier_plane_count_t gbm_device_get_format_modifier_plane_count = nullptr;
    gbm_device_destroy_t                    gbm_device_destroy = nullptr;

    gbm_bo_create_t                         gbm_bo_create = nullptr;
    gbm_bo_create_with_modifiers_t          gbm_bo_create_with_modifiers = nullptr;
    gbm_bo_create_with_modifiers2_t         gbm_bo_create_with_modifiers2 = nullptr;
    gbm_bo_import_t                         gbm_bo_import = nullptr;

    gbm_bo_map_t                            gbm_bo_map = nullptr;
    gbm_bo_unmap_t                          gbm_bo_unmap = nullptr;

    gbm_bo_get_width_t                      gbm_bo_get_width = nullptr;
    gbm_bo_get_height_t                     gbm_bo_get_height = nullptr;
    gbm_bo_get_stride_t                     gbm_bo_get_stride = nullptr;
    gbm_bo_get_stride_for_plane_t           gbm_bo_get_stride_for_plane = nullptr;
    gbm_bo_get_format_t                     gbm_bo_get_format = nullptr;
    gbm_bo_get_bpp_t                        gbm_bo_get_bpp = nullptr;
    gbm_bo_get_offset_t                     gbm_bo_get_offset = nullptr;
    gbm_bo_get_device_t                     gbm_bo_get_device = nullptr;

    gbm_bo_get_handle_t                     gbm_bo_get_handle = nullptr;
    gbm_bo_get_fd_t                         gbm_bo_get_fd = nullptr;
    gbm_bo_get_modifier_t                   gbm_bo_get_modifier = nullptr;
    gbm_bo_get_plane_count_t                gbm_bo_get_plane_count = nullptr;
    gbm_bo_get_handle_for_plane_t           gbm_bo_get_handle_for_plane = nullptr;
    gbm_bo_get_fd_for_plane_t               gbm_bo_get_fd_for_plane = nullptr;
    gbm_bo_write_t                          gbm_bo_write = nullptr;
    gbm_bo_set_user_data_t                  gbm_bo_set_user_data = nullptr;
    gbm_bo_destroy_t                        gbm_bo_destroy = nullptr;

    gbm_surface_create_t                    gbm_surface_create = nullptr;
    gbm_surface_create_with_modifiers_t     gbm_surface_create_with_modifiers = nullptr;
    gbm_surface_create_with_modifiers2_t    gbm_surface_create_with_modifiers2 = nullptr;
    gbm_surface_lock_front_buffer_t         gbm_surface_lock_front_buffer = nullptr;
    gbm_surface_release_buffer_t            gbm_surface_release_buffer = nullptr;
    gbm_surface_has_free_buffers_t          gbm_surface_has_free_buffers = nullptr;
    gbm_surface_destroy_t                   gbm_surface_destroy = nullptr;

    gbm_format_get_name_t                   gbm_format_get_name = nullptr;
};

class GbmProxy;
using GbmProxyPtr = std::shared_ptr<GbmProxy>;

class GbmProxy {
  
public:
  static GbmProxyPtr &Instance();

  GbmProxy(std::shared_ptr<void> handle) : handle_(std::move(handle)) {}

  bool Initialize();

  const GbmApi &Api() const { return api_; }

  static std::shared_ptr<void> LoadLibrary();
  static std::shared_ptr<void> LoadLibrary(const char *name);

 private:
  void InitializeApi();

  static GbmProxyPtr Load();

  GbmApi api_{};
  std::shared_ptr<void> handle_ = {};
};

#endif
