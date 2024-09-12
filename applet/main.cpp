#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <fcntl.h>
#include <gbm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

#include "egl/egl-proxy.h"

// system/core/libsystem/include/system/graphics-base-v1.0.h
enum android_pixel_format_t {
  HAL_PIXEL_FORMAT_RGBA_8888 = 1,
  HAL_PIXEL_FORMAT_RGBX_8888 = 2,
  HAL_PIXEL_FORMAT_RGB_888 = 3,
  HAL_PIXEL_FORMAT_RGB_565 = 4,
  HAL_PIXEL_FORMAT_BGRA_8888 = 5,
  HAL_PIXEL_FORMAT_YCBCR_422_SP = 16,
  HAL_PIXEL_FORMAT_YCRCB_420_SP = 17,
  HAL_PIXEL_FORMAT_YCBCR_422_I = 20,
  HAL_PIXEL_FORMAT_RGBA_FP16 = 22,
  HAL_PIXEL_FORMAT_RAW16 = 32,
  HAL_PIXEL_FORMAT_BLOB = 33,
  HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED = 34,
  HAL_PIXEL_FORMAT_YCBCR_420_888 = 35,
  HAL_PIXEL_FORMAT_RAW_OPAQUE = 36,
  HAL_PIXEL_FORMAT_RAW10 = 37,
  HAL_PIXEL_FORMAT_RAW12 = 38,
  HAL_PIXEL_FORMAT_RGBA_1010102 = 43,
  HAL_PIXEL_FORMAT_Y8 = 538982489,
  HAL_PIXEL_FORMAT_Y16 = 540422489,
  HAL_PIXEL_FORMAT_YV12 = 842094169,
};

enum android_pixel_format_v1_1_t {
  HAL_PIXEL_FORMAT_DEPTH_16 = 48,
  HAL_PIXEL_FORMAT_DEPTH_24 = 49,
  HAL_PIXEL_FORMAT_DEPTH_24_STENCIL_8 = 50,
  HAL_PIXEL_FORMAT_DEPTH_32F = 51,
  HAL_PIXEL_FORMAT_DEPTH_32F_STENCIL_8 = 52,
  HAL_PIXEL_FORMAT_STENCIL_8 = 53,
  HAL_PIXEL_FORMAT_YCBCR_P010 = 54,
};

enum android_pixel_format_v1_2_t {
  HAL_PIXEL_FORMAT_HSV_888 = 55 /* 0x37 */,
};

void OutputNativeVisual(EGLDisplay egl_display, const EGLConfig *configs,
                        int32_t num_configs) {
  std::set<int32_t> visuals;
  for (int32_t i = 0; i < num_configs; i++) {
    auto config = configs[i];
    int32_t visual_id = {};
    if (eglGetConfigAttrib(egl_display, config, EGL_NATIVE_VISUAL_ID,
                           &visual_id)) {
      if (visual_id > 0) {
        visuals.emplace(visual_id);
      }
    }
  }
  fprintf(stderr, "%lu native visuals : ", visuals.size());
  for (auto id : visuals) {
    auto to_string = [](uint32_t fourcc) -> std::string {
      char str[5] = {};
      sprintf(str, "%c%c%c%c", (fourcc & 0x7f), ((fourcc >> 8) & 0x7f),
              ((fourcc >> 16) & 0x7f), ((fourcc >> 24) & 0x7f));
      return str;
    };
    fprintf(stderr, "%s, ", to_string(id).c_str());
  }
  fprintf(stderr, "\n");
}

void OutputNativeVisual() {
  auto fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
  auto gbm = gbm_create_device(fd);
  auto egl_display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, nullptr);
  assert(egl_display != EGL_NO_DISPLAY);
  auto ret = eglInitialize(egl_display, nullptr, nullptr);
  assert(ret);
  EGLint num_config = {};
  ret = eglGetConfigs(egl_display, nullptr, 0, &num_config);
  assert(ret);
  assert(num_config > 0);
  std::vector<EGLConfig> configs(num_config);
  ret = eglGetConfigs(egl_display, configs.data(), configs.size(), &num_config);
  assert(ret);
  OutputNativeVisual(egl_display, configs.data(), num_config);
  eglTerminate(egl_display);
  gbm_device_destroy(gbm);
  close(fd);
}

int32_t main(int32_t argc, char *argv[]) {
  OutputNativeVisual();
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
    EGLint renderableType = EGL_OPENGL_ES3_BIT;
    bool is1010102 = false;
    const EGLint tmpAttribs[] = {
        EGL_RENDERABLE_TYPE,
        renderableType,
        EGL_RECORDABLE_ANDROID,
        EGL_TRUE,
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
        EGL_FRAMEBUFFER_TARGET_ANDROID,
        EGL_TRUE,
        EGL_RED_SIZE,
        is1010102 ? 10 : 8,
        EGL_GREEN_SIZE,
        is1010102 ? 10 : 8,
        EGL_BLUE_SIZE,
        is1010102 ? 10 : 8,
        EGL_ALPHA_SIZE,
        is1010102 ? 2 : 8,
        EGL_NONE,
    };

    auto PrintAttibutes = [](const EGLint *attrib_list) -> std::string {
      std::ostringstream oss;
      oss.setf(std::ios::showbase | std::ios::uppercase);
      if (attrib_list) {
        for (auto attirbs = attrib_list; *attirbs != EGL_NONE; attirbs += 2) {
          oss << std::hex << std::setw(4) << std::setfill('0') << attirbs[0]
              << " = " << attirbs[1] << ", ";
        }
        oss << std::hex << std::setw(4) << std::setfill('0') << EGL_NONE;
      }
      return oss.str();
    };
    fprintf(stderr, "attributes : %s\n", PrintAttibutes(tmpAttribs).c_str());

    EGLint num_config{};
    ret = eglGetConfigs(egl_display, nullptr, 0, &num_config);
    assert(ret);

    std::vector<EGLConfig> configs(num_config, EGL_NO_CONFIG_KHR);
    num_config = 0;
    ret =
        eglGetConfigs(egl_display, configs.data(), configs.size(), &num_config);
    configs.resize(num_config);
    for (auto config : configs) {
      EGLint attribute = EGL_NATIVE_VISUAL_ID;
      EGLint format = HAL_PIXEL_FORMAT_RGBA_8888;
      EGLint value = {};
      if (eglGetConfigAttrib(egl_display, config, attribute, &value)) {
        if (value == format) {
          fprintf(stderr, "Found HAL_PIXEL_FORMAT_RGBA_8888 config : %p\n",
                  config);
          break;
        }
      }
    }

    num_config = 0;
    ret = eglChooseConfig(egl_display, tmpAttribs, configs.data(),
                          configs.size(), &num_config);
    assert(ret);
    configs.resize(num_config);

    std::set<EGLint> visuals;
    std::set<EGLint> caveats;
    for (auto config : configs) {
      EGLint value = {};
      if (eglGetConfigAttrib(egl_display, config, EGL_NATIVE_VISUAL_ID,
                             &value)) {
        // assert(value == HAL_PIXEL_FORMAT_RGBA_8888);
        visuals.emplace(value);
      }
      if (eglGetConfigAttrib(egl_display, config, EGL_CONFIG_CAVEAT, &value)) {
        caveats.emplace(value);
      }
    }
    fprintf(stderr, "%lu native visuals : ", visuals.size());
    std::copy(visuals.begin(), visuals.end(),
              std::ostream_iterator<EGLint>(std::cerr, ", "));
    fprintf(stderr, "\n");

    fprintf(stderr, "%lu caveats : ", caveats.size());
    // std::cerr.setf(std::ios::hex | std::ios::showbase | std::ios::uppercase);
    std::cerr << std::hex << std::showbase << std::uppercase << std::setw(4)
              << std::setfill('0');
    std::copy(caveats.begin(), caveats.end(),
              std::ostream_iterator<EGLint>(std::cerr, ", "));
    fprintf(stderr, "\n");
  }

  {
    auto version = eglQueryString(egl_display, EGL_VERSION);
    auto vendor = eglQueryString(egl_display, EGL_VENDOR);
    auto apis = eglQueryString(egl_display, EGL_CLIENT_APIS);
    printf("EGL vendor : %s, version : %s, apis : %s\n", vendor, version, apis);

    if (auto extension = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
        extension) {
      printf("Client egl extensions : %s\n", extension);
    }
    if (auto extension = eglQueryString(egl_display, EGL_EXTENSIONS);
        extension) {
      printf("Display egl extensions : %s\n", extension);
    }
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
