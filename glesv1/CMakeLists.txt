set(LIBBASENAME GLESv1_CM_proxy)

set(LIBSRC
  glesv1_cm.c
)

add_library(${LIBBASENAME} SHARED ${LIBSRC})

set(ANDROID_INCLUDES
  ../../../frameworks/native/libs/nativewindow/include
  ../../../frameworks/native/libs/nativebase/include
  ../../../system/core/include
  ../../../system/core/libsystem/include
)

target_include_directories(${LIBBASENAME} AFTER PRIVATE ${ANDROID_INCLUDES})
