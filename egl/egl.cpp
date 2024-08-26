
#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include <log/log.h>
#include <system/graphics-base.h>
#include <vndk/window.h>

#include "u_gralloc/u_gralloc.h"

#include "display.h"
#include "egl-surface.h"
#include "gbm.h"

#define HYBRIS_LIBNAME "libEGL.so.1" // soname
#define HYBRIS_ENVNAME "HYBRIS-EGL"
#include "binding.h"

// static void *hybris_initialize() {
//   const char *name = HYBRIS_LIBNAME;
// #ifdef HYBRIS_ENVNAME
//   const char *env_name = getenv(HYBRIS_ENVNAME);
//   name = env_name ? env_name : name;
// #endif
//   return dlopen(name, RTLD_GLOBAL);
// }

static void *hybris_dlsym(void **fptr, const char *sym) {
  if (*fptr == NULL) {
    void *handle = hybris_open_library();
    *fptr = dlsym(handle, sym);
  }
  return *fptr;
}

#define HYBRIS_DLSYM(sym) hybris_dlsym((void **)&s_##sym, #sym)

extern "C" {

// EGL 1.0
extern EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id);
extern EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
HYBRIS_IMPLEMENT_FUNCTION4(EGLContext, eglCreateContext, EGLDisplay, EGLConfig,
                           EGLContext, const EGLint *);
extern EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
                                EGLint config_size, EGLint *num_config);
extern EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
                                  EGLConfig *configs, EGLint config_size,
                                  EGLint *num_config);
extern EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
                                 EGLSurface read, EGLContext ctx);

extern EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                         EGLNativeWindowType win,
                                         const EGLint *attrib_list);
extern EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
                                          const EGLint *attrib_list);
HYBRIS_IMPLEMENT_FUNCTION4(EGLSurface, eglCreatePixmapSurface, EGLDisplay,
                           EGLConfig, EGLNativePixmapType, const EGLint *);

HYBRIS_IMPLEMENT_FUNCTION3(EGLBoolean, eglCopyBuffers, EGLDisplay, EGLSurface,
                           EGLNativePixmapType);

HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglDestroyContext, EGLDisplay,
                           EGLContext);
extern EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
extern EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
                                     EGLint attribute, EGLint *value);
HYBRIS_IMPLEMENT_FUNCTION0(EGLDisplay, eglGetCurrentDisplay);

extern EGLSurface eglGetCurrentSurface(EGLint readdraw);

extern EGLint eglGetError(void);
extern __eglMustCastToProperFunctionPointerType
eglGetProcAddress(const char *procname);
HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglQueryContext, EGLDisplay, EGLContext,
                           EGLint, EGLint *);
extern const char *eglQueryString(EGLDisplay dpy, EGLint name);
extern EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                                  EGLint attribute, EGLint *value);
extern EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
extern EGLBoolean eglTerminate(EGLDisplay dpy);

HYBRIS_IMPLEMENT_FUNCTION0(EGLBoolean, eglWaitGL);
HYBRIS_IMPLEMENT_FUNCTION1(EGLBoolean, eglWaitNative, EGLint);

// EGL 1.1
extern EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface,
                                  EGLint buffer);
extern EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface,
                                     EGLint buffer);
extern EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface,
                                   EGLint attribute, EGLint value);
HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglSwapInterval, EGLDisplay, EGLint);

// EGL_KHR_fence_sync extension requires EGL 1.1
HYBRIS_IMPLEMENT_FUNCTION3(EGLSyncKHR, eglCreateSyncKHR, EGLDisplay, EGLenum,
                           const EGLint *);

HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglDestroySyncKHR, EGLDisplay,
                           EGLSyncKHR);

HYBRIS_IMPLEMENT_FUNCTION4(EGLint, eglClientWaitSyncKHR, EGLDisplay, EGLSyncKHR,
                           EGLint, EGLTimeKHR);

HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglGetSyncAttribKHR, EGLDisplay,
                           EGLSyncKHR, EGLint, EGLint *);

// EGL 1.2
HYBRIS_IMPLEMENT_FUNCTION1(EGLBoolean, eglBindAPI, EGLenum);
HYBRIS_IMPLEMENT_FUNCTION0(EGLenum, eglQueryAPI);
extern EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy,
                                                   EGLenum buftype,
                                                   EGLClientBuffer buffer,
                                                   EGLConfig config,
                                                   const EGLint *attrib_list);

HYBRIS_IMPLEMENT_FUNCTION0(EGLBoolean, eglReleaseThread);
HYBRIS_IMPLEMENT_FUNCTION0(EGLBoolean, eglWaitClient);

// EGL_KHR_image_base extension requires EGL 1.2
extern EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                                     EGLenum target, EGLClientBuffer buffer,
                                     const EGLint *attrib_list);
extern EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR img);

// EGL 1.4
HYBRIS_IMPLEMENT_FUNCTION0(EGLContext, eglGetCurrentContext);

// EGL_EXT_platform_base, EGL_MESA_platform_gbm, EGL_MESA_platform_surfaceless
extern EGLDisplay eglGetPlatformDisplayEXT(EGLenum platform,
                                           void *native_display,
                                           const EGLint *attrib_list);
extern EGLSurface eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    void *native_window,
                                                    const EGLint *attrib_list);
extern EGLSurface eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    void *native_pixmap,
                                                    const EGLint *attrib_list);

// EGL 1.5
HYBRIS_IMPLEMENT_FUNCTION3(EGLSync, eglCreateSync, EGLDisplay, EGLenum,
                           const EGLAttrib *);

HYBRIS_IMPLEMENT_FUNCTION2(EGLBoolean, eglDestroySync, EGLDisplay, EGLSync);

HYBRIS_IMPLEMENT_FUNCTION4(EGLint, eglClientWaitSync, EGLDisplay, EGLSync,
                           EGLint, EGLTime);

HYBRIS_IMPLEMENT_FUNCTION4(EGLBoolean, eglGetSyncAttrib, EGLDisplay, EGLSync,
                           EGLint, EGLAttrib *);

extern EGLImage eglCreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                               EGLClientBuffer buffer,
                               const EGLAttrib *attrib_list);
extern EGLBoolean eglDestroyImage(EGLDisplay dpy, EGLImage image);
extern EGLDisplay eglGetPlatformDisplay(EGLenum platform, void *native_display,
                                        const EGLAttrib *attrib_list);
extern EGLSurface eglCreatePlatformWindowSurface(EGLDisplay dpy,
                                                 EGLConfig config,
                                                 void *native_window,
                                                 const EGLAttrib *attrib_list);
HYBRIS_IMPLEMENT_FUNCTION4(EGLSurface, eglCreatePlatformPixmapSurface,
                           EGLDisplay, EGLConfig, void *, const EGLAttrib *);

HYBRIS_IMPLEMENT_FUNCTION3(EGLBoolean, eglWaitSync, EGLDisplay, EGLSync,
                           EGLint);
}

namespace {

auto gralloc = u_gralloc_create(U_GRALLOC_TYPE_LIBDRM);

EGLDisplay eglGetPlatformDisplayAdapter(EGLenum platform, void *native_display,
                                        const EGLAttrib *attrib_list) {
  static EGLDisplay (*s_eglGetPlatformDisplay)(EGLenum, void *,
                                               const EGLAttrib *) = {};
  HYBRIS_DLSYM(eglGetPlatformDisplay);
  if (!s_eglGetPlatformDisplay) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  auto real_platform = platform;
  auto real_native_display = native_display;
  if (platform == EGL_PLATFORM_ANDROID_KHR) {
    assert(native_display == EGL_DEFAULT_DISPLAY);
    real_native_display = display::NewGbmDevice();
    real_platform = EGL_PLATFORM_GBM_KHR;
  }
  if (auto egl_display = s_eglGetPlatformDisplay(
          real_platform, real_native_display, attrib_list);
      egl_display != EGL_NO_DISPLAY) {
    display::InsertDisplayInfo(egl_display, real_native_display);
  } else if (real_native_display != native_display && real_native_display) {
    gbm_device_destroy(reinterpret_cast<gbm_device *>(real_native_display));
  }

  return EGL_NO_DISPLAY;
}

// int32_t EglAttrbCount(const EGLint *attrib_list) {
//   if (!attrib_list) {
//     return 0;
//   }
//   auto begin = attrib_list;
//   while (*attrib_list != EGL_NONE) {
//     attrib_list += 2;
//   }
//   return std::distance(begin, attrib_list) / 2;
// }

void EGLAppendAttribs(const EGLint *attrib_list, std::vector<EGLint> &attribs) {
  if (!attribs.empty() && attribs.back() == EGL_NONE) {
    attribs.pop_back();
  }
  if (attrib_list) {
    while (*attrib_list != EGL_NONE) {
      attribs.push_back(attrib_list[0]);
      attribs.push_back(attrib_list[1]);
      attrib_list += 2;
    }
  }
  attribs.push_back(EGL_NONE);
}

std::vector<EGLint> EglCopyAttribs(const EGLint *attrib_list) {
  std::vector<EGLint> attribs;
  EGLAppendAttribs(attrib_list, attribs);
  return attribs;
}

// EGL_ANDROID_create_native_client_buffer
EGLClientBuffer eglCreateNativeClientBufferANDROID(const EGLint *attrib_list) {
  (void)attrib_list;
  assert(false);
  return nullptr;
}

// EGL_ANDROID_get_native_client_buffer
EGLClientBuffer
eglGetNativeClientBufferANDROID(const struct AHardwareBuffer *buffer) {
  (void)buffer;
  assert(false);
  return nullptr;
}

EGLBoolean EglDestroyImage(EGLDisplay dpy, EGLImageKHR img,
                           PFNEGLDESTROYIMAGEKHRPROC destroy_image) {
  // auto native_buffer = find by img;
  // auto hardware_buffer =
  // ANativeWindowBuffer_getHardwareBuffer(native_buffer);
  // AHardwareBuffer_release(hardware_buffer);
  return destroy_image(dpy, img);
}

template <typename AttribType>
EGLBoolean FillAttribs(const ANativeWindowBuffer *native_buffer,
                       size_t attribs_size, AttribType *attribs) {
  int32_t atti = 0;
  u_gralloc_buffer_handle buffer_handle = {
      native_buffer->handle, native_buffer->format, native_buffer->stride};

  u_gralloc_buffer_basic_info buffer_basic_info{};
  u_gralloc_buffer_color_info buffer_color_info{};
  assert(gralloc);
  if (u_gralloc_get_buffer_basic_info(gralloc, &buffer_handle,
                                      &buffer_basic_info)) {
    return EGL_FALSE;
  }
  u_gralloc_get_buffer_color_info(gralloc, &buffer_handle, &buffer_color_info);

  attribs[atti++] = EGL_WIDTH;
  attribs[atti++] = native_buffer->width;
  attribs[atti++] = EGL_HEIGHT;
  attribs[atti++] = native_buffer->height;
  attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
  attribs[atti++] = buffer_basic_info.drm_fourcc;
  attribs[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
  attribs[atti++] = buffer_color_info.yuv_color_space;
  attribs[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
  attribs[atti++] = buffer_color_info.sample_range;
  attribs[atti++] = EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT;
  attribs[atti++] = buffer_color_info.horizontal_siting;
  attribs[atti++] = EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT;
  attribs[atti++] = buffer_color_info.vertical_siting;

  auto n_planes = buffer_basic_info.num_planes;
  auto fds = buffer_basic_info.fds;
  auto offsets = buffer_basic_info.offsets;
  auto strides = buffer_basic_info.strides;
  auto modifier = buffer_basic_info.modifier;
  if (n_planes > 0) {
    attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
    attribs[atti++] = fds[0];
    attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
    attribs[atti++] = offsets[0];
    attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
    attribs[atti++] = strides[0];

    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
    attribs[atti++] = modifier & 0xFFFFFFFF;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
    attribs[atti++] = modifier >> 32;
  }

  if (n_planes > 1) {
    attribs[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
    attribs[atti++] = fds[1];
    attribs[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
    attribs[atti++] = offsets[1];
    attribs[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
    attribs[atti++] = strides[1];

    attribs[atti++] = EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT;
    attribs[atti++] = modifier & 0xFFFFFFFF;
    attribs[atti++] = EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT;
    attribs[atti++] = modifier >> 32;
  }

  if (n_planes > 2) {
    attribs[atti++] = EGL_DMA_BUF_PLANE2_FD_EXT;
    attribs[atti++] = fds[2];
    attribs[atti++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
    attribs[atti++] = offsets[2];
    attribs[atti++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
    attribs[atti++] = strides[2];

    attribs[atti++] = EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT;
    attribs[atti++] = modifier & 0xFFFFFFFF;
    attribs[atti++] = EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT;
    attribs[atti++] = modifier >> 32;
  }

  attribs[atti++] = EGL_NONE;

  assert(static_cast<size_t>(atti) <= attribs_size);
  return EGL_TRUE;
}

int32_t get_gbm_from_hal_format(int32_t hal_format) {
  switch (hal_format) {
  case HAL_PIXEL_FORMAT_RGB_565:
    return GBM_FORMAT_RGB565;
  case HAL_PIXEL_FORMAT_BGRA_8888:
    return GBM_FORMAT_ARGB8888;
  case HAL_PIXEL_FORMAT_RGBA_8888:
    return GBM_FORMAT_ABGR8888;
  case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
    /*
     * HACK: Hardcode this to RGBX_8888 as per cros_gralloc hack.
     * TODO: Remove this once https://issuetracker.google.com/32077885 is
     * fixed.
     */
  case HAL_PIXEL_FORMAT_RGBX_8888:
    return GBM_FORMAT_XBGR8888;
  case HAL_PIXEL_FORMAT_RGBA_FP16:
    return GBM_FORMAT_ABGR16161616F;
  case HAL_PIXEL_FORMAT_RGBA_1010102:
    return GBM_FORMAT_ABGR2101010;
  default:
    break;
  }
  return EGL_DONT_CARE;
}

int32_t get_hal_from_gbm_format(int32_t gbm_format) {
  switch (gbm_format) {
  // case GBM_FORMAT_R8:
  // case GBM_FORMAT_R16:
  // case GBM_FORMAT_GR88:
  // case GBM_FORMAT_GR1616:
  // case GBM_FORMAT_ARGB1555:
  case GBM_FORMAT_RGB565:
    return HAL_PIXEL_FORMAT_RGB_565;
  case GBM_FORMAT_XRGB8888:
    return HAL_PIXEL_FORMAT_RGB_888;
  case GBM_FORMAT_ARGB8888:
    return HAL_PIXEL_FORMAT_BGRA_8888;
  case GBM_FORMAT_ABGR8888:
    return HAL_PIXEL_FORMAT_RGBA_8888;
  case GBM_FORMAT_XBGR8888:
    return HAL_PIXEL_FORMAT_RGBX_8888;
  // case GBM_FORMAT_XBGR16161616:
  case GBM_FORMAT_XBGR16161616F:
  case GBM_FORMAT_ABGR16161616F:
    return HAL_PIXEL_FORMAT_RGBA_FP16;
  // case GBM_FORMAT_XRGB2101010:
  // case GBM_FORMAT_ARGB2101010:
  case GBM_FORMAT_XBGR2101010:
  case GBM_FORMAT_ABGR2101010:
    return HAL_PIXEL_FORMAT_RGBA_1010102;
  default:
    ALOGW("unsupported gbm buffer format 0x%08X", gbm_format);
  }
  return EGL_DONT_CARE;
}

} // namespace

HYBRIS_VISIBILITY EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) {
  if (display_id != EGL_DEFAULT_DISPLAY) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  return eglGetPlatformDisplayAdapter(EGL_PLATFORM_GBM_KHR, EGL_DEFAULT_DISPLAY,
                                      nullptr);
}

HYBRIS_VISIBILITY EGLDisplay eglGetPlatformDisplay(
    EGLenum platform, void *native_display, const EGLAttrib *attrib_list) {
  if (platform == EGL_NONE) {
    display::SetDisplayError(EGL_BAD_PARAMETER);
    return EGL_NO_DISPLAY;
  }

  if (platform == EGL_PLATFORM_ANDROID_KHR &&
      native_display != EGL_DEFAULT_DISPLAY) {
    display::SetDisplayError(EGL_BAD_MATCH);
    return EGL_NO_DISPLAY;
  }

  return eglGetPlatformDisplayAdapter(platform, native_display, attrib_list);
}

HYBRIS_VISIBILITY EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major,
                                           EGLint *minor) {
  static EGLBoolean (*s_eglInitialize)(EGLDisplay, EGLint *, EGLint *) = {};
  HYBRIS_DLSYM(eglInitialize);
  if (!s_eglInitialize) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_FALSE;
  }
  if (auto ret = eglInitialize(dpy, major, minor); ret) {
    constexpr EGLint kSupportMaxMinor = 2;
    if (minor && *minor > kSupportMaxMinor) {
      *minor = kSupportMaxMinor;
    }
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
                                           EGLint config_size,
                                           EGLint *num_config) {
  static PFNEGLGETCONFIGSPROC s_eglGetConfigs = {};
  HYBRIS_DLSYM(eglGetConfigs);
  if (!s_eglGetConfigs) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_FALSE;
  }
  return s_eglGetConfigs(dpy, configs, config_size, num_config);
}

HYBRIS_VISIBILITY EGLBoolean eglChooseConfig(EGLDisplay dpy,
                                             const EGLint *attrib_list,
                                             EGLConfig *configs,
                                             EGLint config_size,
                                             EGLint *num_config) {
  static PFNEGLCHOOSECONFIGPROC s_eglChooseConfig = {};
  HYBRIS_DLSYM(eglChooseConfig);
  assert(s_eglChooseConfig);
  if (!attrib_list || attrib_list[0] == EGL_NONE) {
    return s_eglChooseConfig(dpy, attrib_list, configs, config_size,
                             num_config);
  }
  auto attribs = EglCopyAttribs(attrib_list);

  for (auto it = attribs.begin(); *it != EGL_NONE; it += 2) {
    if (*it == EGL_SURFACE_TYPE) {
      auto &type = *std::next(it);
      type &= ~EGL_WINDOW_BIT;
      type |= EGL_PBUFFER_BIT;
    } else if (*it == EGL_NATIVE_VISUAL_ID) {
      *std::next(it) = get_gbm_from_hal_format(*std::next(it));
    } else if (*it == EGL_NATIVE_VISUAL_TYPE) {
      *std::next(it) = EGL_DONT_CARE;
    }
  }
  return s_eglChooseConfig(dpy, attribs.data(), configs, config_size,
                           num_config);
}

HYBRIS_VISIBILITY EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
                                            EGLSurface read, EGLContext ctx) {
  static EGLBoolean (*s_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface,
                                        EGLContext) = {};
  EGLSurface real_draw = EGL_NO_SURFACE;
  EGLSurface real_read = EGL_NO_SURFACE;
  HYBRIS_DLSYM(eglMakeCurrent);
  assert(s_eglMakeCurrent);
  if (draw) {
    real_draw = reinterpret_cast<EglSurface *>(draw)->GetSurface();
  }
  if (read) {
    real_read = reinterpret_cast<EglSurface *>(read)->GetSurface();
  }
  return s_eglMakeCurrent(dpy, real_draw, real_read, ctx);
}

static EGLSurface (*s_eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config,
                                               const EGLint *attrib_list) = {};
HYBRIS_VISIBILITY EGLSurface eglCreatePbufferSurface(
    EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
  HYBRIS_DLSYM(eglCreatePbufferSurface);
  assert(s_eglCreatePbufferSurface);

  if (auto surface = s_eglCreatePbufferSurface(dpy, config, attrib_list);
      surface != EGL_NO_SURFACE) {
    auto egl_surface = new EglSurface{dpy, surface};
    EglSurface::InsertSurface(egl_surface);
    return egl_surface;
  }
  return EGL_NO_SURFACE;
}

static PFNEGLCREATEWINDOWSURFACEPROC s_eglCreateWindowSurface = {};

HYBRIS_VISIBILITY EGLSurface eglCreateWindowSurface(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    EGLNativeWindowType win,
                                                    const EGLint *attrib_list) {
  HYBRIS_DLSYM(eglCreateWindowSurface);
  assert(s_eglCreateWindowSurface);

  auto display_info = display::FindDispayInfo(dpy);
  if (!display_info) {
    display::SetDisplayError(EGL_NOT_INITIALIZED);
    return EGL_NO_SURFACE;
  }

  if (!win) {
    display::SetDisplayError(dpy, EGL_BAD_NATIVE_WINDOW);
    return EGL_NO_SURFACE;
  }

  auto egl_surface = new EglSurface{dpy, win};
  EGLint width = egl_surface->GetWindowWidth(); // get from ANativeWindow
  EGLint height = egl_surface->GetWindowHeight();
  // EGLint format = egl_surface->GetFormat();
  // EGLint flags = egl_surface->GetFlags();
  std::vector<EGLint> pbuf_attribs = {EGL_WIDTH, width, EGL_HEIGHT, height};
  EGLAppendAttribs(attrib_list, pbuf_attribs);

  if (auto surface = s_eglCreatePbufferSurface(dpy, config, &pbuf_attribs[0]);
      surface == EGL_NO_SURFACE) {
    delete egl_surface;
    return EGL_NO_SURFACE;
  } else {
    egl_surface->SetReal(surface);
    egl_surface->SetConfigAndAttribs(config, std::move(pbuf_attribs));
    EglSurface::InsertSurface(egl_surface);
    egl_surface->DequeueBuffer();
  }

  // auto gbm_dev = reinterpret_cast<gbm_device *>(display_info->real_display);
  // auto real_surface = gbm_surface_create(gbm_dev, width, height, format,
  // flags); if (!real_surface) {
  //   delete egl_surface;
  //   display::SetDisplayError(EGL_BAD_PARAMETER);
  //   return EGL_NO_SURFACE;
  // }
  // assert(egl_surface);

  // if (auto surface = s_eglCreateWindowSurface(
  //         dpy, config, (EGLNativeWindowType)real_surface, attrib_list);
  //     surface == EGL_NO_SURFACE) {
  //   gbm_surface_destroy(real_surface);
  //   delete real_surface;
  //   return EGL_NO_SURFACE;
  // }
  // else{
  //   egl_surface->SetReal(surface, (EGLNativeWindowType)real_surface);
  // }

  return egl_surface;
}

HYBRIS_VISIBILITY EGLBoolean eglDestroySurface(EGLDisplay dpy,
                                               EGLSurface surface) {
  static PFNEGLDESTROYSURFACEPROC s_eglDestroySurface = {};
  HYBRIS_DLSYM(eglDestroySurface);
  assert(s_eglDestroySurface);

  auto egl_surface = EglSurface::From(surface);
  if (auto ret = s_eglDestroySurface(dpy, egl_surface->GetSurface());
      ret == EGL_TRUE) {
    egl_surface->CancelBuffer();
    EglSurface::RemoveSurface(egl_surface->GetSurface());
    delete egl_surface;
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

static EGLBoolean (*s_eglGetConfigAttrib)(EGLDisplay, EGLConfig, EGLint,
                                          EGLint *);
HYBRIS_VISIBILITY EGLBoolean eglGetConfigAttrib(EGLDisplay dpy,
                                                EGLConfig config,
                                                EGLint attribute,
                                                EGLint *value) {
  HYBRIS_DLSYM(eglGetConfigAttrib);
  assert(s_eglGetConfigAttrib);

  auto ret = s_eglGetConfigAttrib(dpy, config, attribute, value);
  if (!ret && eglGetError() == EGL_BAD_ATTRIBUTE) {
    // EGL_ANDROID_framebuffer_target
    if (attribute == EGL_FRAMEBUFFER_TARGET_ANDROID) {
      *value = EGL_TRUE;
      return EGL_TRUE;
    }
    if (attribute == EGL_COVERAGE_SAMPLES_NV ||
        attribute == EGL_COVERAGE_BUFFERS_NV) {
      *value = 0;
      return EGL_TRUE;
    }
    if (attribute == EGL_DEPTH_ENCODING_NV) {
      *value = EGL_DEPTH_ENCODING_NONE_NV;
      return EGL_TRUE;
    }
    if (attribute == EGL_COLOR_COMPONENT_TYPE_EXT) {
      *value = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
      return EGL_TRUE;
    }
  }
  return ret;
}

static EGLSurface (*s_eglGetCurrentSurface)(EGLint) = {};
HYBRIS_VISIBILITY EGLSurface eglGetCurrentSurface(EGLint readdraw) {
  HYBRIS_DLSYM(eglGetCurrentSurface);
  assert(s_eglGetCurrentSurface);
  auto real_surface = s_eglGetCurrentSurface(readdraw);
  return EglSurface::FindSurface(real_surface);
}

HYBRIS_VISIBILITY EGLint eglGetError(void) {
  static EGLint (*s_eglGetError)();
  HYBRIS_DLSYM(eglGetError);
  assert(s_eglGetError);
  if (auto error = s_eglGetError(); error != EGL_SUCCESS) {
    return error;
  }
  return display::GetDisplayError();
}

HYBRIS_VISIBILITY __eglMustCastToProperFunctionPointerType
eglGetProcAddress(const char *procname) {
  static PFNEGLGETPROCADDRESSPROC s_eglGetProcAddress = {};
  HYBRIS_DLSYM(eglGetProcAddress);
  assert(s_eglGetProcAddress);

  if (strcmp(procname, "eglDupNativeFenceFDANDROID") == 0) {
    // not support EGL_ANDROID_native_fence_sync extension
    return nullptr;
  }

  if (strcmp(procname, "eglSetBlobCacheFuncsANDROID") == 0) {
    // not support EGL_ANDROID_blob_cache extension
    return nullptr;
  }

  if (strcmp(procname, "eglPresentationTimeANDROID") == 0) {
    // not support EGL_ANDROID_presentation_time extension
    return nullptr;
  }

  if (strcmp(procname, "eglCreateNativeClientBufferANDROID") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglCreateNativeClientBufferANDROID);
  }

  if (strcmp(procname, "eglGetNativeClientBufferANDROID") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        eglGetNativeClientBufferANDROID);
  }

  return s_eglGetProcAddress(procname);
}

HYBRIS_VISIBILITY const char *eglQueryString(EGLDisplay dpy, EGLint name) {
  static PFNEGLQUERYSTRINGPROC s_eglQueryString = {};
  HYBRIS_DLSYM(eglQueryString);
  assert(s_eglQueryString);

  auto strs = s_eglQueryString(dpy, name);
  if (name == EGL_EXTENSIONS) {
    thread_local std::string egl_string;
    // EGL_ANDROID_framebuffer_target ???
    if (strstr(strs, "EGL_EXT_image_dma_buf_import")) {
      static const char kNativeBufferExtensions[] =
          "EGL_ANDROID_image_native_buffer ";
      // not support EGL_ANDROID_create_native_client_buffer and
      // EGL_ANDROID_get_native_client_buffer
      egl_string += kNativeBufferExtensions;
    }
    if (strs) {
      // EGL_KHR_fence_sync, EGL_KHR_image_base and EGL_KHR_gl_texture_2d_image
      // extensions
      egl_string += strs;
    }
    return egl_string.c_str();
  }

  return strs;
}

HYBRIS_VISIBILITY EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                                             EGLint attribute, EGLint *value) {
  static PFNEGLQUERYSURFACEPROC s_eglQuerySurface = {};
  HYBRIS_DLSYM(eglQuerySurface);
  assert(s_eglQuerySurface);
  if (surface == EGL_NO_SURFACE) {
    return s_eglQuerySurface(dpy, EGL_NO_SURFACE, attribute, value);
  }
  auto egl_surface = EglSurface::From(surface);
  surface = egl_surface->GetSurface();
  if (attribute == EGL_NATIVE_VISUAL_TYPE) {
    attribute = EGL_NATIVE_VISUAL_ID;
  }
  if (auto ret = s_eglQuerySurface(dpy, surface, attribute, value); ret) {
    if (attribute == EGL_SURFACE_TYPE) {
      *value |= egl_surface->IsWindow() ? EGL_WINDOW_BIT : 0;
    } else if (attribute == EGL_NATIVE_VISUAL_ID) {
      *value = get_hal_from_gbm_format(*value);
    }
    return ret;
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLBoolean eglSwapBuffers(EGLDisplay dpy,
                                            EGLSurface surface) {
  static PFNEGLSWAPBUFFERSPROC s_eglSwapBuffers = {};
  HYBRIS_DLSYM(eglSwapBuffers);
  assert(s_eglSwapBuffers);
  if (surface == EGL_NO_SURFACE) {
    return s_eglSwapBuffers(dpy, EGL_NO_SURFACE);
  }
  auto egl_surface = EglSurface::From(surface);

  if (auto real_surface = egl_surface->GetSurface(); !egl_surface->IsWindow()) {
    return s_eglSwapBuffers(dpy, real_surface);
  }

  // copy
  auto native_buffer =
      reinterpret_cast<EGLClientBuffer>(egl_surface->GetNativeBuffer());
  auto image = eglCreateImage(dpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                              native_buffer, nullptr);

  GLuint tmp_tex = {};
  GLint curr_tex_bind = {};
  GLint prev_read_fbo = {};
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex_bind);
  glGenTextures(1, &tmp_tex);
  glBindTexture(GL_TEXTURE_2D, tmp_tex);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

  // gles3
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_read_fbo);
  if (prev_read_fbo != 0) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }

  GLint samples = {};
  glGetIntegerv(GL_SAMPLE_BUFFERS, &samples);
  assert(samples == 0);
  GLsizei width = egl_surface->GetWindowWidth();
  GLsizei height = egl_surface->GetWindowHeight();
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

  if (prev_read_fbo != 0) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prev_read_fbo);
  }

  glDeleteTextures(1, &tmp_tex);
  glBindTexture(GL_TEXTURE_2D, curr_tex_bind);

  // clear GL errors, because its possible that the fbo format does not match
  // the format of the read buffer, in the case of OpenGL ES 3.1 and integer
  // RGBA formats.
  glGetError();

  eglDestroyImage(dpy, image);
  // sync
  glFinish();
  egl_surface->QueueBuffer();

  egl_surface->DequeueBuffer();
  // maybe resize window
  egl_surface->UpdateSurface();

  return EGL_TRUE;
}

static EGLBoolean (*s_eglTerminate)(EGLDisplay) = {};
HYBRIS_VISIBILITY EGLBoolean eglTerminate(EGLDisplay dpy) {
  HYBRIS_DLSYM(eglTerminate);
  assert(s_eglTerminate);

  if (auto ret = s_eglTerminate(dpy); ret) {
    auto info = display::FindDispayInfo(dpy);
    display::RemoveDisplay(dpy);
    delete info;
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

HYBRIS_VISIBILITY EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface,
                                             EGLint buffer) {
  static PFNEGLBINDTEXIMAGEPROC s_eglBindTexImage = {};
  HYBRIS_DLSYM(eglBindTexImage);
  assert(s_eglBindTexImage);
  if (surface != EGL_NO_SURFACE) {
    auto egl_surface = EglSurface::From(surface);
    surface = egl_surface->GetSurface();
  }
  return s_eglBindTexImage(dpy, surface, buffer);
}

HYBRIS_VISIBILITY EGLBoolean eglReleaseTexImage(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLint buffer) {
  static PFNEGLRELEASETEXIMAGEPROC s_eglReleaseTexImage = {};
  HYBRIS_DLSYM(eglReleaseTexImage);
  assert(s_eglReleaseTexImage);
  if (surface != EGL_NO_SURFACE) {
    auto egl_surface = EglSurface::From(surface);
    surface = egl_surface->GetSurface();
  }
  return s_eglReleaseTexImage(dpy, surface, buffer);
}
HYBRIS_VISIBILITY EGLBoolean eglSurfaceAttrib(EGLDisplay dpy,
                                              EGLSurface surface,
                                              EGLint attribute, EGLint value) {
  static PFNEGLSURFACEATTRIBPROC s_eglSurfaceAttrib = {};
  HYBRIS_DLSYM(eglSurfaceAttrib);
  assert(s_eglSurfaceAttrib);
  if (surface != EGL_NO_SURFACE) {
    auto egl_surface = EglSurface::From(surface);
    surface = egl_surface->GetSurface();
  }
  return s_eglSurfaceAttrib(dpy, surface, attribute, value);
}

HYBRIS_VISIBILITY EGLSurface eglCreatePbufferFromClientBuffer(
    EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config,
    const EGLint *attrib_list) {
  static PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC
      s_eglCreatePbufferFromClientBuffer = {};
  HYBRIS_DLSYM(eglCreatePbufferFromClientBuffer);
  assert(s_eglCreatePbufferFromClientBuffer);

  // TODO: set egl error
  return EGL_NO_SURFACE;
}

HYBRIS_VISIBILITY EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                                                EGLenum target,
                                                EGLClientBuffer buffer,
                                                const EGLint *attrib_list) {
  static PFNEGLCREATEIMAGEKHRPROC s_eglCreateImageKHR = {};
  HYBRIS_DLSYM(eglCreateImageKHR);
  assert(s_eglCreateImageKHR);

  if (target != EGL_NATIVE_BUFFER_ANDROID) {
    return s_eglCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
  }
  auto native_buffer = reinterpret_cast<ANativeWindowBuffer *>(buffer);
  if (!native_buffer ||
      native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC ||
      native_buffer->common.version != sizeof(*native_buffer)) {
    display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
    return EGL_NO_IMAGE;
  }

  EGLint attribs[47];
  if (!FillAttribs(native_buffer, sizeof(attribs) / sizeof(*attribs),
                   attribs)) {
    display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
    return EGL_NO_IMAGE_KHR;
  }

  if (auto image = s_eglCreateImageKHR(dpy, EGL_NO_CONTEXT,
                                       EGL_LINUX_DMA_BUF_EXT, nullptr, attribs);
      image != EGL_NO_IMAGE_KHR) {
    auto hardware_buffer = ANativeWindowBuffer_getHardwareBuffer(native_buffer);
    AHardwareBuffer_acquire(hardware_buffer);
    return image;
  }
  return EGL_NO_IMAGE_KHR;
}

HYBRIS_VISIBILITY EGLBoolean eglDestroyImageKHR(EGLDisplay dpy,
                                                EGLImageKHR img) {
  PFNEGLDESTROYIMAGEKHRPROC s_eglDestroyImageKHR = {};
  HYBRIS_DLSYM(eglDestroyImageKHR);
  assert(s_eglDestroyImageKHR);
  return EglDestroyImage(dpy, img, s_eglDestroyImageKHR);
}

HYBRIS_VISIBILITY EGLImage eglCreateImage(EGLDisplay dpy, EGLContext ctx,
                                          EGLenum target,
                                          EGLClientBuffer buffer,
                                          const EGLAttrib *attrib_list) {
  static PFNEGLCREATEIMAGEPROC s_eglCreateImage = {};
  HYBRIS_DLSYM(eglCreateImage);
  assert(s_eglCreateImage);

  if (target != EGL_NATIVE_BUFFER_ANDROID) {
    return s_eglCreateImage(dpy, ctx, target, buffer, attrib_list);
  }
  auto native_buffer = reinterpret_cast<ANativeWindowBuffer *>(buffer);
  if (!native_buffer ||
      native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC ||
      native_buffer->common.version != sizeof(*native_buffer)) {
    display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
    return EGL_NO_IMAGE;
  }

  EGLAttrib attribs[47];
  if (!FillAttribs(native_buffer, sizeof(attribs) / sizeof(*attribs),
                   attribs)) {
    display::SetDisplayError(dpy, EGL_BAD_PARAMETER);
    return EGL_NO_IMAGE;
  }

  if (auto image = s_eglCreateImage(dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                    nullptr, attribs);
      image != EGL_NO_IMAGE) {
    auto hardware_buffer = ANativeWindowBuffer_getHardwareBuffer(native_buffer);
    AHardwareBuffer_acquire(hardware_buffer);
    return image;
  }
  return EGL_NO_IMAGE;
}

HYBRIS_VISIBILITY EGLBoolean eglDestroyImage(EGLDisplay dpy, EGLImageKHR img) {
  PFNEGLDESTROYIMAGEPROC s_eglDestroyImage = {};
  HYBRIS_DLSYM(eglDestroyImage);
  assert(s_eglDestroyImage);
  return EglDestroyImage(dpy, img, s_eglDestroyImage);
}

HYBRIS_VISIBILITY EGLSurface eglCreatePlatformWindowSurface(
    EGLDisplay dpy, EGLConfig config, void *native_window,
    const EGLAttrib *attrib_list) {
  static PFNEGLCREATEPLATFORMWINDOWSURFACEPROC
      s_eglCreatePlatformWindowSurface = {};
  HYBRIS_DLSYM(eglCreatePlatformWindowSurface);
  assert(s_eglCreatePlatformWindowSurface);
  // TODO: adapt
  return s_eglCreatePlatformWindowSurface(dpy, config, native_window,
                                          attrib_list);
}
