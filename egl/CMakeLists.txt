set(LIBBASENAME EGL_proxy)

set(LIBSRC
  egl.cpp
  # display.cpp
  # display.h

  egl-surface.cpp
  egl-surface.h

  egl-proxy.cpp
  egl-proxy.h
  egl-display.cpp
  egl-display.h
  egl-image.cpp
  egl-image.h

  egl-misc.cpp
  egl-misc.h

  libgbm.c
)

add_library(${LIBBASENAME} SHARED ${LIBSRC})

target_link_libraries(${LIBBASENAME} GLESv2_proxy hybris_utils)

set(ANDROID_INCLUDES
  ../../../frameworks/native/libs/nativewindow/include
  ../../../frameworks/native/libs/nativebase/include
  ../../../system/core/include
  ../../../system/core/libsystem/include
)

target_include_directories(${LIBBASENAME} AFTER PRIVATE ${ANDROID_INCLUDES})
