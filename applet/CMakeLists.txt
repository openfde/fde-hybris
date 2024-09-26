set(LIBBASENAME applet)

set(LIBSRC
  main.cpp
  ../egl/libgbm.c
)

add_executable(${LIBBASENAME} ${LIBSRC})

target_link_libraries(${LIBBASENAME} EGL GLESv2 log)

