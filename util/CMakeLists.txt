set(LIBBASENAME hybris_utils)

set(UGRALLOCSRC
  u_gralloc/u_gralloc.c
  u_gralloc/u_gralloc_fallback.c
  u_gralloc/u_gralloc.h
  u_gralloc/u_gralloc_internal.c
  u_gralloc/u_gralloc_internal.h
  u_gralloc/u_gralloc_libdrm.c
  u_gralloc/u_gralloc_libdrm.h
)

set(LIBSRC

  ${UGRALLOCSRC}
)

add_library(${LIBBASENAME} STATIC ${LIBSRC})

set(ANDROID_INCLUDES
  ../../../frameworks/native/libs/nativewindow/include
  ../../../frameworks/native/libs/nativebase/include
  ../../../system/core/include
  ../../../system/core/libsystem/include
  ../../../hardware/libhardware/include/
)

target_include_directories(${LIBBASENAME} AFTER PRIVATE ${ANDROID_INCLUDES})
