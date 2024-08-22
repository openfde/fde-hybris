set(LIBBASENAME applet)

set(LIBSRC
  main.cpp
)

add_executable(${LIBBASENAME} ${LIBSRC})

target_link_libraries(${LIBBASENAME} EGL_proxy)

