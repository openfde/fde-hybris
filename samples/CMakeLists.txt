
set(EXECNAME run-samples)

set(LIBSRC
  test-surface.cpp
)

add_executable(${EXECNAME} ${LIBSRC})

set(ANDROID_INCLUDES
  ../../../frameworks/native/libs/nativewindow/include
  ../../../frameworks/native/libs/nativebase/include
  ../../../system/core/include/
  ../../../system/core/libsystem/include
  ../../../frameworks/native/include/
  ../../../system/core/base/include
  ../../../frameworks/native/libs/ui/include/
  ../../../hardware/interfaces/common/
)

target_include_directories(${EXECNAME} AFTER PRIVATE ${ANDROID_INCLUDES})

target_link_libraries(${EXECNAME} cutils utils ui gui binder EGL)
