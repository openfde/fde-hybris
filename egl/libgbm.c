#include <assert.h>
#include <dlfcn.h>
#include <gbm.h>
#include <stdio.h>
#include <stdlib.h>

static const char *kLibEnvName = "LIBGBM";
static const char *kLibName = "libgbm.so";

static void *open_library() {
  static void *lib_handle = NULL;
  if (!lib_handle) {
    // the symbols of getenv/dlopen are not had parsed when call it.
    const char *name = getenv(kLibEnvName);
    if (!name) {
      name = kLibName;
    }
    lib_handle = dlopen(name, RTLD_GLOBAL);
    if (!lib_handle) {
      printf("dlopen %s %s\n", name, dlerror());
    }
  }
  assert(lib_handle != NULL);
  return lib_handle;
}

#define IMPLEMENT_FUNCTION1(symbol, return_type, t1) \
  return_type symbol(t1 v1) {                        \
    static return_type (*f)(t1) = NULL;              \
    if (!f) {                                        \
      void *handle = open_library();                 \
      f = (void *)dlsym(handle, #symbol);            \
    }                                                \
    return f(v1);                                    \
  }

#define IMPLEMENT_FUNCTION1(symbol, return_type, t1) \
  return_type symbol(t1 v1) {                        \
    static return_type (*f)(t1) = NULL;              \
    if (!f) {                                        \
      void *handle = open_library();                 \
      f = (void *)dlsym(handle, #symbol);            \
    }                                                \
    return f(v1);                                    \
  }

#define IMPLEMENT_FUNCTION2(symbol, return_type, t1, t2) \
  return_type symbol(t1 v1, t2 v2) {                     \
    static return_type (*f)(t1, t2) = NULL;              \
    if (!f) {                                            \
      void *handle = open_library();                     \
      f = (void *)dlsym(handle, #symbol);                \
    }                                                    \
    return f(v1, v2);                                    \
  }

#define IMPLEMENT_FUNCTION3(symbol, return_type, t1, t2, t3) \
  return_type symbol(t1 v1, t2 v2, t3 v3) {                  \
    static return_type (*f)(t1, t2, t3) = NULL;              \
    if (!f) {                                                \
      void *handle = open_library();                         \
      f = (void *)dlsym(handle, #symbol);                    \
    }                                                        \
    return f(v1, v2, v3);                                    \
  }

#define IMPLEMENT_FUNCTION4(symbol, return_type, t1, t2, t3, t4) \
  return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4) {               \
    static return_type (*f)(t1, t2, t3, t4) = NULL;              \
    if (!f) {                                                    \
      void *handle = open_library();                             \
      f = (void *)dlsym(handle, #symbol);                        \
    }                                                            \
    return f(v1, v2, v3, v4);                                    \
  }

#define IMPLEMENT_FUNCTION5(symbol, return_type, t1, t2, t3, t4, t5) \
  return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5) {            \
    static return_type (*f)(t1, t2, t3, t4, t5) = NULL;              \
    if (!f) {                                                        \
      void *handle = open_library();                                 \
      f = (void *)dlsym(handle, #symbol);                            \
    }                                                                \
    return f(v1, v2, v3, v4, v5);                                    \
  }

#define IMPLEMENT_FUNCTION6(symbol, return_type, t1, t2, t3, t4, t5, t6) \
  return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6) {         \
    static return_type (*f)(t1, t2, t3, t4, t5, t6) = NULL;              \
    if (!f) {                                                            \
      void *handle = open_library();                                     \
      f = (void *)dlsym(handle, #symbol);                                \
    }                                                                    \
    return f(v1, v2, v3, v4, v5, v6);                                    \
  }

IMPLEMENT_FUNCTION1(gbm_create_device, struct gbm_device *, int);
IMPLEMENT_FUNCTION1(gbm_device_destroy, void, struct gbm_device *);
IMPLEMENT_FUNCTION5(gbm_surface_create, struct gbm_surface *,
                    struct gbm_device *, uint32_t, uint32_t, uint32_t,
                    uint32_t);
IMPLEMENT_FUNCTION1(gbm_surface_destroy, void, struct gbm_surface *);

IMPLEMENT_FUNCTION4(gbm_bo_import, struct gbm_bo *, struct gbm_device *,
                    uint32_t, void *, uint32_t);
IMPLEMENT_FUNCTION1(gbm_bo_destroy, void, struct gbm_bo *);

IMPLEMENT_FUNCTION1(gbm_surface_lock_front_buffer, struct gbm_bo *,
                    struct gbm_surface *);
IMPLEMENT_FUNCTION2(gbm_surface_release_buffer, void, struct gbm_surface *,
                    struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_surface_has_free_buffers, int, struct gbm_surface *);

IMPLEMENT_FUNCTION1(gbm_bo_get_fd, int, struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_bo_get_width, uint32_t, struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_bo_get_height, uint32_t, struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_bo_get_stride, uint32_t, struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_bo_get_handle, union gbm_bo_handle, struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_bo_get_format, uint32_t, struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_bo_get_plane_count, int, struct gbm_bo *);
IMPLEMENT_FUNCTION2(gbm_bo_get_offset, uint32_t, struct gbm_bo *, int);

IMPLEMENT_FUNCTION1(gbm_bo_get_device, struct gbm_device *, struct gbm_bo *);
IMPLEMENT_FUNCTION1(gbm_device_get_fd, int, struct gbm_device *);
IMPLEMENT_FUNCTION3(drmPrimeFDToHandle, int, int, int, uint32_t *);
IMPLEMENT_FUNCTION1(gbm_bo_get_user_data, void *, struct gbm_bo *);
typedef void (*set_user_cb)(struct gbm_bo *, void *);
IMPLEMENT_FUNCTION3(gbm_bo_set_user_data, void, struct gbm_bo *, void *,
                    set_user_cb);
