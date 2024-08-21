set(LIBBASENAME GLESv2_proxy)

set(LIBSRC
  glesv2.c
)

add_library(${LIBBASENAME} SHARED ${LIBSRC})

set(ANDROID_INCLUDES
  ../../../frameworks/native/libs/nativewindow/include
  ../../../frameworks/native/libs/nativebase/include
  ../../../system/core/include
  ../../../system/core/libsystem/include
)

target_include_directories(${LIBBASENAME} AFTER PRIVATE ${ANDROID_INCLUDES})
