#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>

#include "egl/egl-proxy.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

int32_t main(int32_t argc, char *argv[]) {
  if (auto val = getenv("EGL_DISPLAY")) {
    fprintf(stderr, "EGL_DISPLAY=%s\n", val);
  }

  if (auto val = getenv("EGL_PLATFORM")) {
    fprintf(stderr, "EGL_PLATFORM=%s\n", val);
  }

  // auto lib_handle = egl::EglProxy::LoadLibrary();
  // auto proxy = std::make_shared<egl::EglProxy>(lib_handle);
  // proxy->Initialize();
  egl::EglProxy::Instance();

  auto egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  if (!egl_display) {
    fprintf(stderr, "eglGetDisplay 0x%04X\n", eglGetError());
    return -1;
  }

  EGLint major{};
  EGLint minor{};
  auto ret = eglInitialize(egl_display, &major, &minor);
  assert(ret);
  {
    auto version = eglQueryString(egl_display, EGL_VERSION);
    auto vendor = eglQueryString(egl_display, EGL_VENDOR);
    auto apis = eglQueryString(egl_display, EGL_CLIENT_APIS);
    printf("EGL vendor : %s, version : %s, apis : %s\n", vendor, version, apis);
  }
  ret = eglBindAPI(EGL_OPENGL_ES_API);
  assert(ret);

  std::vector<EGLint> client_versions{3};

  for (auto client_version : client_versions) {
    EGLint config_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, client_version,
                               EGL_NONE};
    auto egl_context = eglCreateContext(egl_display, EGL_NO_CONFIG_KHR,
                                        EGL_NO_CONTEXT, config_attribs);
    assert(egl_context);

    ret = eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                         egl_context);
    assert(ret);

    auto renderer = glGetString(GL_RENDERER);
    assert(renderer);
    auto extensions = glGetString(GL_EXTENSIONS);
    assert(extensions);
    auto version = glGetString(GL_VERSION);
    assert(version);
    printf("Client version : %s, Renderer : %s, GL extension : %s\n",
           (const char *)version, (const char *)renderer, extensions);
    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(egl_display, egl_context);
  }

  auto create_image = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(
      eglGetProcAddress("eglCreateImageKHR"));

  assert(create_image);
  create_image(egl_display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, nullptr,
               nullptr);

  eglTerminate(egl_display);
  return 0;
}
