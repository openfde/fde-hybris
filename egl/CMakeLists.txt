set(LIBBASENAME EGL_proxy)

set(LIBSRC
  egl.cpp
  display.cpp
  display.h

  egl-surface.cpp
  egl-surface.h
)

add_library(${LIBBASENAME} SHARED ${LIBSRC})

set(ANDROID_INCLUDES
  ../../../frameworks/native/libs/nativewindow/include
  ../../../frameworks/native/libs/nativebase/include
  ../../../system/core/include
  ../../../system/core/libsystem/include
)

target_include_directories(${LIBBASENAME} AFTER PRIVATE ${ANDROID_INCLUDES})
