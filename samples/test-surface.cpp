#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "triangle.h"

namespace {

const std::map<EGLint, std::string> kEglStrErrors{
#define STRERROR(error) {error, #error}
    STRERROR(EGL_SUCCESS),
    STRERROR(EGL_NOT_INITIALIZED),
    STRERROR(EGL_BAD_ACCESS),
    STRERROR(EGL_BAD_ALLOC),
    STRERROR(EGL_BAD_ATTRIBUTE),
    STRERROR(EGL_BAD_CONFIG),
    STRERROR(EGL_BAD_CONTEXT),
    STRERROR(EGL_BAD_CURRENT_SURFACE),
    STRERROR(EGL_BAD_DISPLAY),
    STRERROR(EGL_BAD_MATCH),
    STRERROR(EGL_BAD_NATIVE_PIXMAP),
    STRERROR(EGL_BAD_NATIVE_WINDOW),
    STRERROR(EGL_BAD_PARAMETER),
    STRERROR(EGL_BAD_SURFACE),
    STRERROR(EGL_CONTEXT_LOST),
    STRERROR(EGL_BAD_OUTPUT_LAYER_EXT),
    STRERROR(EGL_BAD_OUTPUT_PORT_EXT),
#undef STRERROR
};

std::string StrError(EGLint errnum) {
  if (auto it = kEglStrErrors.find(errnum); it != kEglStrErrors.end()) {
    return it->second.c_str();
  }

  char buf[8]{};
  snprintf(buf, sizeof(buf), " : 0x%04X", errnum);
  return buf;
}

std::string StrLastError() { return StrError(eglGetError()); }

struct Color {
  Color() = default;
  Color(float r, float g, float b, float a)
      : red(r), green(g), blue(b), alpha(a) {}
  float red = {};
  float green = {};
  float blue = {};
  float alpha = 1.0;
};

}  // namespace

using namespace android;

int32_t main(int32_t argc, char *argv[]) {
  (void)argc;
  (void)argv;

  int32_t width = 480;
  int32_t height = 480;

  // HAL_PIXEL_FORMAT_RGB_565  or HAL_PIXEL_FORMAT_RGBA_8888
  int32_t native_format = HAL_PIXEL_FORMAT_RGB_565;

  sp<SurfaceComposerClient> client = new SurfaceComposerClient();

  sp<SurfaceControl> surface_control =
      client->createSurface(String8("resize"), width, height, native_format, 0);

  auto surface = surface_control->getSurface();

  auto egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (egl_dpy == EGL_NO_DISPLAY) {
    fprintf(stderr, "eglGetDisplay  : %s\n", StrLastError().c_str());
    return -1;
  }

  if (!eglInitialize(egl_dpy, nullptr, nullptr)) {
    fprintf(stderr, "eglInitialize  : %s\n", StrLastError().c_str());
    return -1;
  }

  EGLint num_config{};
  if (auto ret = eglGetConfigs(egl_dpy, nullptr, 0, &num_config); !ret) {
    fprintf(stderr, "eglGetConfigs  : %s\n", StrLastError().c_str());
    return -1;
  }

  std::vector<EGLConfig> configs(num_config, EGL_NO_CONFIG_KHR);
  num_config = 0;
  if (auto ret =
          eglGetConfigs(egl_dpy, configs.data(), configs.size(), &num_config);
      !ret) {
    fprintf(stderr, "eglGetConfigs  : %s\n", StrLastError().c_str());
    return -1;
  }
  configs.resize(num_config);

  EGLint renderableType = EGL_OPENGL_ES3_BIT;
  const EGLint tmpAttribs[] = {
      EGL_RENDERABLE_TYPE,
      renderableType,
      // EGL_RECORDABLE_ANDROID,
      // EGL_TRUE,
      EGL_SURFACE_TYPE,
      EGL_WINDOW_BIT,
      // EGL_FRAMEBUFFER_TARGET_ANDROID,
      // EGL_TRUE,
      EGL_RED_SIZE,
      1,
      EGL_GREEN_SIZE,
      1,
      EGL_BLUE_SIZE,
      1,
      EGL_ALPHA_SIZE,
      EGL_DONT_CARE,
      EGL_NONE,
  };

  num_config = 0;
  if (auto ret = eglChooseConfig(egl_dpy, tmpAttribs, configs.data(),
                                 configs.size(), &num_config);
      !ret) {
    fprintf(stderr, "eglChooseConfig  : %s\n", StrLastError().c_str());
  }
  configs.resize(num_config);

  EGLConfig egl_conf = EGL_NO_CONFIG_KHR;
  for (auto config : configs) {
    EGLint attribute = EGL_NATIVE_VISUAL_ID;
    EGLint value = {};
    if (eglGetConfigAttrib(egl_dpy, config, attribute, &value)) {
      if (value == native_format) {
        fprintf(stderr, "Found 0x%08X config : %p\n", value, config);
        egl_conf = config;
        break;
      }
    }
  }

  // must set config attributes to eglCreateContext, or maybe cause
  // glCreateShader segment fault
  EGLint config_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  auto egl_cntxt =
      eglCreateContext(egl_dpy, egl_conf, EGL_NO_CONTEXT, config_attribs);
  if (egl_cntxt == EGL_NO_CONTEXT) {
    fprintf(stderr, "eglCreateContext  : %s\n", StrLastError().c_str());
    return -1;
  }

  {
    auto version = eglQueryString(egl_dpy, EGL_VERSION);
    auto vendor = eglQueryString(egl_dpy, EGL_VENDOR);
    auto apis = eglQueryString(egl_dpy, EGL_CLIENT_APIS);
    printf("EGL vendor : %s, version : %s, apis : %s\n", vendor, version, apis);

    if (auto extension = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
        extension) {
      printf("Client egl extensions : %s\n", extension);
    }
    if (auto extension = eglQueryString(egl_dpy, EGL_EXTENSIONS); extension) {
      printf("Display egl extensions : %s\n", extension);
    }
  }
  if (auto ret = eglBindAPI(EGL_OPENGL_ES_API); !ret) {
    fprintf(stderr, "eglBindAPI  : %s\n", StrLastError().c_str());
    return -1;
  }

  auto egl_surface = eglCreateWindowSurface(
      egl_dpy, egl_conf, (EGLNativeWindowType)surface.get(), nullptr);

  if (egl_surface == EGL_NO_SURFACE) {
    fprintf(stderr, "eglCreateWindowSurface  : %s\n", StrLastError().c_str());
    return -1;
  }

  if (!eglMakeCurrent(egl_dpy, egl_surface, egl_surface, egl_cntxt)) {
    fprintf(stderr, "eglMakeCurrent  : %s\n", StrLastError().c_str());
    return -1;
  }

#define TEST_SHADER 1
#ifndef TEST_SHADER
  std::vector<Color> colors{
      {0.0, 0.0, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0},
      {0.0, 0.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.5, 0.0, 0.0, 0.5},
      {0.0, 0.5, 0.0, 0.5}, {0.0, 0.0, 0.5, 0.5}, {0.5, 0.5, 0.5, 0.5},
  };
  for (auto const &color : colors) {
    glViewport(0, 0, width, height);
    glClearColor(color.red, color.green, color.blue, color.alpha);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    if (!eglSwapBuffers(egl_dpy, egl_surface)) {
      fprintf(stderr, "eglSwapBuffers  : %s\n", StrLastError().c_str());
      return -1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

#else
  auto drawer = std::make_shared<Triangle>(width, height);

  drawer->BuildCompileLinkShaders();
  drawer->SetupResources();

  drawer->RenderData();

  if (!eglSwapBuffers(egl_dpy, egl_surface)) {
    fprintf(stderr, "eglSwapBuffers  : %s\n", StrLastError().c_str());
    return -1;
  }

  drawer->TerminalProgram();
#endif

#if 0
  ANativeWindow_Buffer out_buffer;
  surface->lock(&out_buffer, nullptr);

  size_t bpr = out_buffer.stride * bytesPerPixel(out_buffer.format);
  constexpr uint16_t kPixelValue = 0x07E0;
  std::fill_n(reinterpret_cast<uint16_t *>(out_buffer.bits),
              bpr * out_buffer.height, kPixelValue);
  surface->unlockAndPost();
#endif

  eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_cntxt);
  eglDestroySurface(egl_dpy, egl_surface);

  if (!eglTerminate(egl_dpy)) {
    fprintf(stderr, "eglTerminate  : %s\n", StrLastError().c_str());
  }
  return 0;
}