#include <assert.h>
#include <dlfcn.h>
#include <gbm.h>
#include <stdio.h>
#include <stdlib.h>

#define HYBRIS_LIBNAME "libgbm.so"
#define HYBRIS_ENVNAME "HYBRIS-GBM"
#include "binding.h"

HYBRIS_IMPLEMENT_FUNCTION1(struct gbm_device *, gbm_create_device, int);
HYBRIS_IMPLEMENT_FUNCTION1(void, gbm_device_destroy, struct gbm_device *);
HYBRIS_IMPLEMENT_FUNCTION5(struct gbm_surface *, gbm_surface_create,
                           struct gbm_device *, uint32_t, uint32_t, uint32_t,
                           uint32_t);
HYBRIS_IMPLEMENT_FUNCTION1(void, gbm_surface_destroy, struct gbm_surface *);

HYBRIS_IMPLEMENT_FUNCTION4(struct gbm_bo *, gbm_bo_import, struct gbm_device *,
                           uint32_t, void *, uint32_t);
HYBRIS_IMPLEMENT_FUNCTION1(void, gbm_bo_destroy, struct gbm_bo *);

HYBRIS_IMPLEMENT_FUNCTION1(struct gbm_bo *, gbm_surface_lock_front_buffer,
                           struct gbm_surface *);
HYBRIS_IMPLEMENT_FUNCTION2(void, gbm_surface_release_buffer,
                           struct gbm_surface *, struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(int, gbm_surface_has_free_buffers,
                           struct gbm_surface *);

HYBRIS_IMPLEMENT_FUNCTION1(int, gbm_bo_get_fd, struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(uint32_t, gbm_bo_get_width, struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(uint32_t, gbm_bo_get_height, struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(uint32_t, gbm_bo_get_stride, struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(union gbm_bo_handle, gbm_bo_get_handle,
                           struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(uint32_t, gbm_bo_get_format, struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(int, gbm_bo_get_plane_count, struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION2(uint32_t, gbm_bo_get_offset, struct gbm_bo *, int);

HYBRIS_IMPLEMENT_FUNCTION1(struct gbm_device *, gbm_bo_get_device,
                           struct gbm_bo *);
HYBRIS_IMPLEMENT_FUNCTION1(int, gbm_device_get_fd, struct gbm_device *);
HYBRIS_IMPLEMENT_FUNCTION3(int, drmPrimeFDToHandle, int, int, uint32_t *);
HYBRIS_IMPLEMENT_FUNCTION1(void *, gbm_bo_get_user_data, struct gbm_bo *);
typedef void (*set_user_cb)(struct gbm_bo *, void *);
HYBRIS_IMPLEMENT_FUNCTION3(void, gbm_bo_set_user_data, struct gbm_bo *, void *,
                           set_user_cb);
