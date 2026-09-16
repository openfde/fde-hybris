#include "gbm.h"
#include "gbm_proxy.h"

int
gbm_device_get_fd(struct gbm_device *gbm)
{
    return GbmProxy::Instance()->Api().gbm_device_get_fd(gbm);
}

const char *
gbm_device_get_backend_name(struct gbm_device *gbm)
{
    return GbmProxy::Instance()->Api().gbm_device_get_backend_name(gbm);
}

int
gbm_device_is_format_supported(struct gbm_device *gbm,
                               uint32_t format, uint32_t flags)
{
    return GbmProxy::Instance()->Api().gbm_device_is_format_supported(gbm, format, flags);
}

int
gbm_device_get_format_modifier_plane_count(struct gbm_device *gbm,
                                           uint32_t format,
                                           uint64_t modifier)
{
    return GbmProxy::Instance()->Api().gbm_device_get_format_modifier_plane_count(gbm, format, modifier);
}

void
gbm_device_destroy(struct gbm_device *gbm)
{
    GbmProxy::Instance()->Api().gbm_device_destroy(gbm);
}

struct gbm_device *
gbm_create_device(int fd)
{
    return GbmProxy::Instance()->Api().gbm_create_device(fd);
}

struct gbm_bo *
gbm_bo_create(struct gbm_device *gbm,
              uint32_t width, uint32_t height,
              uint32_t format, uint32_t flags)
{
    return GbmProxy::Instance()->Api().gbm_bo_create(gbm, width, height, format, flags);
}

struct gbm_bo *
gbm_bo_create_with_modifiers(struct gbm_device *gbm,
                             uint32_t width, uint32_t height,
                             uint32_t format,
                             const uint64_t *modifiers,
                             const unsigned int count)
{
    return GbmProxy::Instance()->Api().gbm_bo_create_with_modifiers(gbm, width, height, format, modifiers, count);
}

struct gbm_bo *
gbm_bo_create_with_modifiers2(struct gbm_device *gbm,
                              uint32_t width, uint32_t height,
                              uint32_t format,
                              const uint64_t *modifiers,
                              const unsigned int count,
                              uint32_t flags)
{
    return GbmProxy::Instance()->Api().gbm_bo_create_with_modifiers2(gbm, width, height, format, modifiers, count, flags);
}

struct gbm_bo *
gbm_bo_import(struct gbm_device *gbm, uint32_t type,
              void *buffer, uint32_t flags)
{
    return GbmProxy::Instance()->Api().gbm_bo_import(gbm, type, buffer, flags);
}

void *
gbm_bo_map(struct gbm_bo *bo,
           uint32_t x, uint32_t y, uint32_t width, uint32_t height,
           uint32_t flags, uint32_t *stride, void **map_data)
{
    return GbmProxy::Instance()->Api().gbm_bo_map(bo, x, y, width, height, flags, stride, map_data);
}

void
gbm_bo_unmap(struct gbm_bo *bo, void *map_data)
{
    GbmProxy::Instance()->Api().gbm_bo_unmap(bo, map_data);
}

uint32_t
gbm_bo_get_width(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_width(bo);
}

uint32_t
gbm_bo_get_height(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_height(bo);
}

uint32_t
gbm_bo_get_stride(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_stride(bo);
}

uint32_t
gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_stride_for_plane(bo, plane);
}

uint32_t
gbm_bo_get_format(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_format(bo);
}

uint32_t
gbm_bo_get_bpp(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_bpp(bo);
}

uint32_t
gbm_bo_get_offset(struct gbm_bo *bo, int plane)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_offset(bo, plane);
}

struct gbm_device *
gbm_bo_get_device(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_device(bo);
}

union gbm_bo_handle
gbm_bo_get_handle(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_handle(bo);
}

int
gbm_bo_get_fd(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_fd(bo);
}

uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_modifier(bo);
}

int
gbm_bo_get_plane_count(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_plane_count(bo);
}

union gbm_bo_handle
gbm_bo_get_handle_for_plane(struct gbm_bo *bo, int plane)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_handle_for_plane(bo, plane);
}

int
gbm_bo_get_fd_for_plane(struct gbm_bo *bo, int plane)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_fd_for_plane(bo, plane);
}

int
gbm_bo_write(struct gbm_bo *bo, const void *buf, size_t count)
{
    return GbmProxy::Instance()->Api().gbm_bo_write(bo, buf, count);
}

void
gbm_bo_set_user_data(struct gbm_bo *bo, void *data,
                     void (*destroy_user_data)(struct gbm_bo *, void *))
{
    GbmProxy::Instance()->Api().gbm_bo_set_user_data(bo, data, destroy_user_data);
}

void *
gbm_bo_get_user_data(struct gbm_bo *bo)
{
    return GbmProxy::Instance()->Api().gbm_bo_get_user_data(bo);
}

void
gbm_bo_destroy(struct gbm_bo *bo)
{
    GbmProxy::Instance()->Api().gbm_bo_destroy(bo);
}

struct gbm_surface *
gbm_surface_create(struct gbm_device *gbm,
                   uint32_t width, uint32_t height,
                   uint32_t format, uint32_t flags)
{
    return GbmProxy::Instance()->Api().gbm_surface_create(gbm, width, height, format, flags);
}

struct gbm_surface *
gbm_surface_create_with_modifiers(struct gbm_device *gbm,
                                  uint32_t width, uint32_t height,
                                  uint32_t format,
                                  const uint64_t *modifiers,
                                  const unsigned int count)
{
    return GbmProxy::Instance()->Api().gbm_surface_create_with_modifiers(gbm, width, height, format, modifiers, count);
}

struct gbm_surface *
gbm_surface_create_with_modifiers2(struct gbm_device *gbm,
                                   uint32_t width, uint32_t height,
                                   uint32_t format,
                                   const uint64_t *modifiers,
                                   const unsigned int count,
                                   uint32_t flags)
{
    return GbmProxy::Instance()->Api().gbm_surface_create_with_modifiers2(gbm, width, height, format, modifiers, count, flags);
}

struct gbm_bo *
gbm_surface_lock_front_buffer(struct gbm_surface *surface)
{
    return GbmProxy::Instance()->Api().gbm_surface_lock_front_buffer(surface);
}

void
gbm_surface_release_buffer(struct gbm_surface *surface, struct gbm_bo *bo)
{
    GbmProxy::Instance()->Api().gbm_surface_release_buffer(surface, bo);
}

int
gbm_surface_has_free_buffers(struct gbm_surface *surface)
{
    return GbmProxy::Instance()->Api().gbm_surface_has_free_buffers(surface);
}

void
gbm_surface_destroy(struct gbm_surface *surface)
{
    GbmProxy::Instance()->Api().gbm_surface_destroy(surface);
}

char *
gbm_format_get_name(uint32_t gbm_format, struct gbm_format_name_desc *desc)
{
    return GbmProxy::Instance()->Api().gbm_format_get_name(gbm_format, desc);
}