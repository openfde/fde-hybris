/*
 * Copyright (C) 2010-2011 Chia-I Wu <olvaffe@gmail.com>
 * Copyright (C) 2010-2011 LunarG Inc.
 * Copyright (C) 2016 Linaro, Ltd., Rob Herring <robh@kernel.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define LOG_TAG "GRALLOC-GBM-PROXY"
//#define LOG_NDEBUG 0

#include <log/log.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <hardware/gralloc.h>
#include <system/graphics.h>

#include "gbm.h"
#include <drm/drm_fourcc.h>

#include "gralloc_gbm_priv.h"
#include <android/gralloc_handle.h>

#include <unordered_map>
#include <sstream>
#include <vector>
#include <cmath>

#include <utils/CallStack.h>

#include <dlfcn.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
constexpr char kEglLibName[] = "libEGL.so.1";  // soname

#define GRALLOC_ALIGN(value, base) (((value) + ((base)-1)) & ~((base)-1))
int align_x = 16;
int is_nvidia = 0;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define unlikely(x) __builtin_expect(!!(x), 0)

static std::unordered_map<buffer_handle_t, struct gbm_bo *> gbm_bo_handle_map;
static std::unordered_map<uint32_t, std::vector<uint64_t>> gbm_format_modifiers_map;

static bool is_rgba_family(int format);
static bool is_cursor_buffer(uint32_t format, int gbm_usage, uint32_t w, uint32_t h);
static bool gralloc_need_sync(struct gralloc_handle_t *gbm_handle);

/* Serializes ALL GL work: the readback (sync_gpu_to_cpu), the writeback
 * (sync_cpu_to_gpu) and the release-fence creation. The EGL state is not
 * thread-safe, so concurrent users (scrcpy + local recording, or a producer
 * and a consumer in the same process) must not interleave GL calls. Each thread
 * gets its OWN EGL context (see g_thread_blits / ensure_thread_context), so
 * there is no cross-thread context stealing -- the root cause of the earlier
 * "FBO incomplete" when both ran at once. */
static std::mutex g_blit_mtx;
/* One-time context init guard. Kept separate from g_blit_mtx so callers can
 * take g_blit_mtx around their own GL ops without re-entering it (deadlock). */
static std::once_flag g_blit_once;

typedef EGLBoolean (EGLAPIENTRY *PFNEGLDESTROYIMAGEKHRPROC)(EGLDisplay, EGLImageKHR);
static PFNEGLGETPROCADDRESSPROC g_blit_getproc = nullptr;

static void *g_blit_egl = NULL;
static EGLDisplay g_blit_dpy = EGL_NO_DISPLAY;
static EGLConfig  g_blit_cfg = NULL;
static EGLBoolean (*g_blit_makecurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext) = nullptr;
static EGLContext (*g_blit_createctx)(EGLDisplay, EGLConfig, EGLContext, const EGLint *) = nullptr;

/* One surfaceless EGL context PER calling thread (keyed by pthread_t). An EGL
 * context may be current on exactly one thread at a time; the previous single
 * shared context forced scrcpy and the local recorder to fight over
 * eglMakeCurrent, and the loser's dma-buf texture/FBO was left half-built --
 * the "FBO incomplete on create" seen when both ran. With a private context per
 * thread each stays bound to its owner and is never stolen. The display/config
 * are process-wide and shared. Readback/writeback GL objects are transient
 * (created + destroyed within the call) so nothing is tied to a dead bo and
 * there is no per-buffer GPU-memory leak. */
struct thread_blit {
	EGLContext ctx;
};
static std::unordered_map<uintptr_t, thread_blit> g_thread_blits;

struct bo_data_t {
	void *map_data;
	int lock_count;
	int locked_for;
	/* CPU-accessible pixel pointer. For a dual (RENDERING|LINEAR) RGBA buffer
	 * that needs GPU<->CPU sync we hand the framework a CACHEABLE heap buffer
	 * (cpu_cache) instead of any GBM bo: a GBM linear mirror is mapped WC
	 * (uncached) by the driver and is pathologically slow for the software
	 * AVC encoder's sustained CPU reads (~7s/frame stall). For generic
	 * (non-sync) CPU-accessible buffers gbm_map() sets this to the mapped
	 * primary bo. */
	void *cpu_ptr;
	/* CACHEABLE heap buffer used for GPU<->CPU sync on dual RGBA buffers:
	 *  - GPU->CPU (read): glReadPixels writes the TILED bo here, the
	 *    framework reads cached DRAM at normal speed.
	 *  - CPU->GPU (write): the framework writes here at handle->stride, then
	 *    sync_cpu_to_gpu re-packs and uploads it into the TILED bo.
	 * Allocated lazily (alloc_cpu_cache) and reused across locks. */
	void *cpu_cache;
	size_t cpu_cache_size;
	/* True when cpu_ptr == cpu_cache, i.e. this bo was locked for the
	 * malloc-backed sync path. Lets unlock() tell a sync write buffer apart
	 * from a generic mapped primary bo (which must NOT be blitted). */
	bool cpu_sync;
};

void gralloc_gbm_destroy_user_data(struct gbm_bo *bo, void *data)
{
	struct bo_data_t *bo_data = (struct bo_data_t *)data;
	/* Readback/writeback GL objects are transient (created + destroyed inside
	 * each sync call), so there is no per-bo GL state to tear down here. The
	 * sync path uses a plain malloc buffer (cpu_cache), freed below. */
	if (bo_data && bo_data->cpu_cache)
		free(bo_data->cpu_cache);
	delete bo_data;

	(void)bo;
}

static struct bo_data_t *gbm_bo_data(struct gbm_bo *bo) {
	return (struct bo_data_t *)gbm_bo_get_user_data(bo);
}


static std::vector<uint64_t> get_supported_modifiers(struct gbm_device *gbm, uint32_t format) {
	if (gbm_format_modifiers_map.find(format) != gbm_format_modifiers_map.end()) {
		return gbm_format_modifiers_map[format];
	}

	// Create empty default so we can match it next time
	std::vector<uint64_t> &modifiers = gbm_format_modifiers_map[format];

	std::stringstream prop_name_stream;
	prop_name_stream << "openfde.modifiers." << std::hex << format << ".";
	std::string prop_name_base = prop_name_stream.str();
	char modifier_prop[PROPERTY_VALUE_MAX];
	int i = 0;
	while (true) {
		std::string prop_name = (std::stringstream() << prop_name_base << i).str();
		if (property_get(prop_name.c_str(), modifier_prop, NULL) < 1)
			break;
		std::stringstream ss(modifier_prop);
		uint64_t mod;
		ss >> std::hex >> mod;

		// Filter out multiplanar format-modifier combos
		if (gbm_device_get_format_modifier_plane_count(gbm, format, mod) < 2)
			modifiers.push_back(mod);
		i++;
	}

	return modifiers;
}

static uint32_t get_gbm_format(int format)
{
	uint32_t fmt;

	switch (format) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
		if (property_get_bool("persist.openfde.invert_colors", false))
			fmt = GBM_FORMAT_ARGB8888;
		else
			fmt = GBM_FORMAT_ABGR8888;
		break;
	case HAL_PIXEL_FORMAT_RGBX_8888:
		fmt = GBM_FORMAT_XBGR8888;
		break;
	case HAL_PIXEL_FORMAT_RGB_888:
		fmt = GBM_FORMAT_RGB888;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
		fmt = GBM_FORMAT_RGB565;
		break;
	case HAL_PIXEL_FORMAT_BGRA_8888:
		if (property_get_bool("persist.openfde.invert_colors", false))
			fmt = GBM_FORMAT_ABGR8888;
		else
			fmt = GBM_FORMAT_ARGB8888;
		break;
	case HAL_PIXEL_FORMAT_YV12:
	case HAL_PIXEL_FORMAT_YCbCr_420_888:
		/* YV12 is planar, but must be a single buffer so ask for RGB565 */
		if (is_nvidia)
			fmt = GBM_FORMAT_GR88;
		else
			fmt = GBM_FORMAT_RGB565;
		break;
	case HAL_PIXEL_FORMAT_RGBA_FP16:
		fmt = GBM_FORMAT_ABGR16161616F;
		break;
	case HAL_PIXEL_FORMAT_RGBA_1010102:
		fmt = GBM_FORMAT_ABGR2101010;
		break;
	case HAL_PIXEL_FORMAT_BLOB:
		fmt = GBM_FORMAT_RGB565;
		break;
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
	case HAL_PIXEL_FORMAT_YCrCb_420_SP:
	default:
		fmt = 0;
		break;
	}

	return fmt;
}

static int gralloc_gbm_get_bpp(int format)
{
	int bpp;

	switch (format) {
	case HAL_PIXEL_FORMAT_RGBA_FP16:
		bpp = 8;
		break;
	case HAL_PIXEL_FORMAT_RGBA_8888:
	case HAL_PIXEL_FORMAT_RGBX_8888:
	case HAL_PIXEL_FORMAT_BGRA_8888:
	case HAL_PIXEL_FORMAT_RGBA_1010102:
		bpp = 4;
		break;
	case HAL_PIXEL_FORMAT_RGB_888:
		bpp = 3;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
	case HAL_PIXEL_FORMAT_YCbCr_422_I:
		bpp = 2;
		break;
	/* planar; only Y is considered */
	case HAL_PIXEL_FORMAT_YV12:
	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
	case HAL_PIXEL_FORMAT_YCrCb_420_SP:
	case HAL_PIXEL_FORMAT_YCbCr_420_888:
		bpp = 1;
		break;
	default:
		bpp = 0;
		break;
	}

	return bpp;
}

static unsigned int get_pipe_bind(int usage)
{
	unsigned int bind = 0;
	if (usage & (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN))
		bind |= GBM_BO_USE_LINEAR;
	if (usage & GRALLOC_USAGE_CURSOR)
		bind = GBM_BO_USE_CURSOR;
	if (usage & (GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_TEXTURE))
		bind |= GBM_BO_USE_RENDERING;
	if (usage & GRALLOC_USAGE_HW_FB)
		bind |= GBM_BO_USE_RENDERING;
	if (usage & GRALLOC_USAGE_HW_COMPOSER)
		bind |= GBM_BO_USE_RENDERING;
	return bind;
}

static std::pair<int, int> find_closest_size(int blob) {
    if (blob <= 0) {
        return {0, 0};
    }

    int width = static_cast<int>(std::sqrt(blob));
    int height = width = ((width + 7) / 8) * 8;

    while (true) {
        if (width * height == blob) {
            return {width, height};
        } else if (width * height < blob) {
            return {width, height + 1};
        }
        height--;
    }
}

static struct gbm_bo *gbm_import(struct gbm_device *gbm,
		buffer_handle_t _handle)
{
	struct gbm_bo *bo;
	struct gralloc_handle_t *handle = gralloc_handle(_handle);
	#ifdef GBM_BO_IMPORT_FD_MODIFIER
	struct gbm_import_fd_modifier_data data;
	#else
	struct gbm_import_fd_data data;
	#endif

	int format = get_gbm_format(handle->format);
	if (handle->prime_fd < 0)
		return NULL;

	memset(&data, 0, sizeof(data));
	data.width = handle->width;
	data.height = handle->height;
	data.format = format;
	/* Adjust the width and height for a GBM GR88 buffer */
	if (handle->format == HAL_PIXEL_FORMAT_YV12 || handle->format == HAL_PIXEL_FORMAT_YCbCr_420_888) {
		data.width = GRALLOC_ALIGN(data.width/2, align_x);
		data.height += handle->height / 2;
	}
	if (handle->format == HAL_PIXEL_FORMAT_BLOB) {
		std::pair<int, int> size = find_closest_size(data.width);
		data.width = size.first;
		data.height = size.second;
	}
	#ifdef GBM_BO_IMPORT_FD_MODIFIER
	data.num_fds = 1;
	data.fds[0] = handle->prime_fd;
	data.strides[0] = handle->stride;
	data.modifier = handle->modifier;
	bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD_MODIFIER, &data, 0);
	#else
	data.fd = handle->prime_fd;
	data.stride = handle->stride;
	bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD, &data, 0);
	#endif

	if (!bo) {
		ALOGE("failed to import BO, size=%dx%d, fmt=%d",
		      handle->width, handle->height, handle->format);
	}

	return bo;
}

static bool is_rgba_family(int format)
{
	switch (format) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
	case HAL_PIXEL_FORMAT_RGBX_8888:
	case HAL_PIXEL_FORMAT_BGRA_8888:
	case HAL_PIXEL_FORMAT_RGBA_FP16:
	case HAL_PIXEL_FORMAT_RGBA_1010102:
	case HAL_PIXEL_FORMAT_RGB_888:
	case HAL_PIXEL_FORMAT_RGB_565:
		return true;
	default:
		return false;
	}
}

/* A Wayland/KMS hardware cursor (wl_pointer_set_cursor) must be a LINEAR dma-buf:
 * it is scanned out directly by the cursor plane, which rejects tiled buffers, so
 * the cursor stays invisible. Our dual-bo path makes prime_fd TILED, which is
 * exactly what breaks the cursor. Identify a cursor buffer either by the explicit
 * GBM_BO_USE_CURSOR flag, or -- when the framework omits it (as Android's
 * SpriteController does) -- by the practical signature: a small RGBA buffer
 * carrying both RENDERING and LINEAR usage (CPU-written, GPU-readable). */
static bool is_cursor_buffer(uint32_t format, int usage, uint32_t w, uint32_t h)
{
	if (usage & GBM_BO_USE_CURSOR)
		return true;
	if (is_rgba_family(format)
		&& ((usage&GBM_BO_USE_LINEAR) && (usage&GBM_BO_USE_RENDERING))
		&& (w < 64 && h < 64 && w == h))
		return true;
	return false;
}


static bool gralloc_need_sync(struct gralloc_handle_t *gbm_handle)
{
	int usage = get_pipe_bind(gbm_handle->usage);
	if (!is_nvidia)
		return false;
	if (is_cursor_buffer(gbm_handle->format, usage, gbm_handle->width, gbm_handle->height))
		return false;
	if (!((usage&GBM_BO_USE_LINEAR) && (usage&GBM_BO_USE_RENDERING)))
		return false;
	if (!is_rgba_family(gbm_handle->format))
		return false;

	return true;
}

/* Map a HAL pixel format to the GL (format, type, bytes-per-pixel) needed to
 * upload its CPU-side *linear* data with glTexImage2D. Returns false for the
 * HDR/wide formats (FP16, 1010102) we don't blit yet; the caller then skips
 * the SW->GPU sync and leaves the tiled bo as-is. */
static bool gralloc_to_gl_upload(int hal_fmt, GLenum *gl_fmt, GLenum *gl_type, uint32_t *bpp)
{
	switch (hal_fmt) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
	case HAL_PIXEL_FORMAT_RGBX_8888:
		*gl_fmt = GL_RGBA; *gl_type = GL_UNSIGNED_BYTE; *bpp = 4; return true;
	case HAL_PIXEL_FORMAT_BGRA_8888:
#ifdef GL_BGRA_EXT
		*gl_fmt = GL_BGRA_EXT; *gl_type = GL_UNSIGNED_BYTE; *bpp = 4; return true;
#else
		*gl_fmt = GL_RGBA; *gl_type = GL_UNSIGNED_BYTE; *bpp = 4; return true;
#endif
	case HAL_PIXEL_FORMAT_RGB_888:
		*gl_fmt = GL_RGB; *gl_type = GL_UNSIGNED_BYTE; *bpp = 3; return true;
	case HAL_PIXEL_FORMAT_RGB_565:
		*gl_fmt = GL_RGB; *gl_type = GL_UNSIGNED_SHORT_5_6_5; *bpp = 2; return true;
	default:
		return false;
	}
}

/* Lazily (re)allocate a CACHEABLE heap buffer in bo_data large enough to hold
 * `need` bytes. Cached (not WC) so the software encoder's CPU-side RGB->YUV
 * conversion reads it at normal DRAM speed instead of stalling on every access
 * to the WC GBM mirror (the ~7s/frame screen-record stall). Reused across locks
 * as long as the size suffices. Returns NULL on allocation failure. */
static void *alloc_cpu_cache(struct bo_data_t *bo_data, size_t need)
{
	if (!bo_data)
		return NULL;
	if (bo_data->cpu_cache && bo_data->cpu_cache_size >= need)
		return bo_data->cpu_cache;
	free(bo_data->cpu_cache);
	bo_data->cpu_cache = malloc(need);
	bo_data->cpu_cache_size = bo_data->cpu_cache ? need : 0;
	return bo_data->cpu_cache;
}

static struct gbm_bo *gbm_alloc(struct gbm_device *gbm,
		buffer_handle_t _handle)
{
	struct gbm_bo *bo = NULL;
	struct gralloc_handle_t *handle = gralloc_handle(_handle);
	int format = get_gbm_format(handle->format);
	int usage = get_pipe_bind(handle->usage);
	int width, height;

	width = handle->width;
	height = handle->height;
	if (usage & GBM_BO_USE_CURSOR) {
		if (handle->width < 64)
			width = 64;
		if (handle->height < 64)
			height = 64;
	}

	if (is_cursor_buffer(handle->format, usage, handle->width, handle->height) && is_nvidia) {
		usage = GBM_BO_USE_CURSOR;
	}

	/*
	 * For YV12, we request GR88, so halve the width since we're getting
	 * 16bpp. Then increase the height by 1.5 for the U and V planes.
	 */
	if (handle->format == HAL_PIXEL_FORMAT_YV12 || handle->format == HAL_PIXEL_FORMAT_YCbCr_420_888) {
		width = GRALLOC_ALIGN(width/2, align_x);
		height += handle->height / 2;
	}

	if (handle->format == HAL_PIXEL_FORMAT_BLOB) {
		std::pair<int, int> size = find_closest_size(width);
		width = size.first;
		height = size.second;
	}
	ALOGV("create BO, size=%dx%d, fmt=%d, usage=%x",
		handle->width, handle->height, handle->format, usage);

	std::vector<uint64_t> modifiers = get_supported_modifiers(gbm, format);
	if (format == GBM_FORMAT_GR88 && is_nvidia) {
		uint64_t modifier[] = { 0x0 };
		bo = gbm_bo_create_with_modifiers(gbm, width, height, format, modifier, 1);
	} else if (modifiers.size() > 0) {
		bo = gbm_bo_create_with_modifiers2(gbm, width, height, format,
			                                   modifiers.data(), modifiers.size(), usage);
		if (!bo && (usage&GBM_BO_USE_RENDERING)) {
			ALOGD("fallback to gbm_bo_create_with_modifiers %dx%d, format:%d useage:0x%x",
						handle->width, handle->height, handle->format, usage);
			bo = gbm_bo_create_with_modifiers(gbm, width, height, format,
				                                  modifiers.data(), modifiers.size());
		}
	}

	if (!bo) {
		ALOGV("fallback to gbm_bo_create %dx%d, format:%d, usage:0x%x",
				handle->width, handle->height, handle->format, usage);
		bo = gbm_bo_create(gbm, width, height, format, usage);
	}

	if (!bo) {
		ALOGE("failed to create BO, size=%dx%d, fmt=%d, usage=0x%x",
		      handle->width, handle->height, handle->format, usage);
		return NULL;
	}

	handle->prime_fd = gbm_bo_get_fd(bo);
	handle->stride = gbm_bo_get_stride(bo);
	#ifdef GBM_BO_IMPORT_FD_MODIFIER
	handle->modifier = gbm_bo_get_modifier(bo);
	#endif

	return bo;
}

void gbm_free(buffer_handle_t handle)
{
	struct gbm_bo *bo = gralloc_gbm_bo_from_handle(handle);

	if (!bo)
		return;

	gbm_bo_handle_map.erase(handle);
	gbm_bo_destroy(bo);
}

/*
 * Return the bo of a registered handle.
 */
struct gbm_bo *gralloc_gbm_bo_from_handle(buffer_handle_t handle)
{
	return gbm_bo_handle_map[handle];
}

static int gbm_map(buffer_handle_t handle, int enable_write, void **addr)
{
	int err = 0;
	int flags = GBM_BO_TRANSFER_READ;
	struct gbm_bo *bo = gralloc_gbm_bo_from_handle(handle);
	struct bo_data_t *bo_data = gbm_bo_data(bo);
	uint32_t stride;

	if (bo_data->map_data) {
		ALOGE("gbm_map fail, buffer already mapped");
		return -EINVAL;
	}

	if (enable_write)
		flags |= GBM_BO_TRANSFER_WRITE;

	*addr = gbm_bo_map(bo, 0, 0, gbm_bo_get_width(bo), gbm_bo_get_height(bo),
			   flags, &stride, &bo_data->map_data);
	ALOGV("mapped bo %p at %p", bo, *addr);
	if (*addr == NULL)
		return -ENOMEM;

	assert(stride == gbm_bo_get_stride(bo));

	return err;
}

static void gbm_unmap(struct gbm_bo *bo)
{
	struct bo_data_t *bo_data = gbm_bo_data(bo);

	gbm_bo_unmap(bo, bo_data->map_data);
	bo_data->map_data = NULL;
}

static void blit_init_impl()
{
	g_blit_egl = dlopen(kEglLibName, RTLD_NOW | RTLD_LOCAL);
	if (!g_blit_egl) {
		ALOGE("sync_blit: dlopen %s failed: %s", kEglLibName, dlerror());
		return;
	}
	g_blit_getproc = (PFNEGLGETPROCADDRESSPROC)dlsym(g_blit_egl, "eglGetProcAddress");
	if (!g_blit_getproc) {
		ALOGE("sync_blit: dlsym eglGetProcAddress failed: %s", dlerror());
		return;
	}
	auto eglGetDisplay = (EGLDisplay (*)(EGLNativeDisplayType))dlsym(g_blit_egl, "eglGetDisplay");
	auto eglInitialize = (EGLBoolean (*)(EGLDisplay, EGLint *, EGLint *))dlsym(g_blit_egl, "eglInitialize");
	auto eglChooseConfig = (EGLBoolean (*)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *))dlsym(g_blit_egl, "eglChooseConfig");
	g_blit_createctx = (EGLContext (*)(EGLDisplay, EGLConfig, EGLContext, const EGLint *))dlsym(g_blit_egl, "eglCreateContext");
	auto eglMakeCurrent = (EGLBoolean (*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))dlsym(g_blit_egl, "eglMakeCurrent");
	if (!eglGetDisplay || !eglInitialize || !eglChooseConfig || !g_blit_createctx || !eglMakeCurrent) {
		ALOGE("sync_blit: missing EGL symbols %d-%d-%d-%d-%d",
			 !eglGetDisplay, !eglInitialize, !eglChooseConfig, !g_blit_createctx, !eglMakeCurrent);
		return;
	}
	g_blit_makecurrent = eglMakeCurrent;
	g_blit_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (g_blit_dpy == EGL_NO_DISPLAY) {
		ALOGE("sync_blit: eglGetDisplay failed");
		return;
	}
	if (!eglInitialize(g_blit_dpy, NULL, NULL)) {
		ALOGE("sync_blit: eglInitialize failed");
		return;
	}
	EGLint cfg_attr[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
	                      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_NONE };
	EGLint ncfg = 0;
	if (!eglChooseConfig(g_blit_dpy, cfg_attr, &g_blit_cfg, 1, &ncfg) || ncfg < 1) {
		ALOGE("sync_blit: eglChooseConfig failed");
		return;
	}
	ALOGD("sync_blit: surfaceless nvidia EGL display ready");
}

static bool blit_ensure_context()
{
	std::call_once(g_blit_once, blit_init_impl);
	return g_blit_dpy != EGL_NO_DISPLAY;
}

/* Ensure THIS thread has its OWN surfaceless EGL context, current on the
 * calling thread. Called under g_blit_mtx. The first time a thread touches the
 * blit path we create a fresh context for it and bind it; afterwards we just
 * re-bind it (which also self-heals after any transient move). Because each
 * thread keeps a private context bound to itself, scrcpy and the local
 * recorder never fight over eglMakeCurrent the way they did with the old single
 * shared context -- which is what produced "FBO incomplete on create" when
 * both ran. Returns EGL_NO_CONTEXT on failure. */
static EGLContext ensure_thread_context(void)
{
	uintptr_t tid = (uintptr_t)pthread_self();
	auto it = g_thread_blits.find(tid);
	if (it != g_thread_blits.end() && it->second.ctx != EGL_NO_CONTEXT) {
		g_blit_makecurrent(g_blit_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
		                   it->second.ctx);
		return it->second.ctx;
	}
	if (!g_blit_createctx) {
		ALOGE("sync_blit: eglCreateContext symbol missing");
		return EGL_NO_CONTEXT;
	}
	EGLint ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
	EGLContext ctx = g_blit_createctx(g_blit_dpy, g_blit_cfg,
	                                  EGL_NO_CONTEXT, ctx_attr);
	if (ctx == EGL_NO_CONTEXT) {
		ALOGE("sync_blit: eglCreateContext (per-thread) failed");
		return EGL_NO_CONTEXT;
	}
	if (!g_blit_makecurrent(g_blit_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		ALOGE("sync_blit: eglMakeCurrent (per-thread) failed");
		return EGL_NO_CONTEXT;
	}
	thread_blit tb; tb.ctx = ctx;
	g_thread_blits[tid] = tb;
	return ctx;
}

static void sync_cpu_to_gpu(struct gbm_bo *gpu_bo, void *cpu_ptr,
                             uint32_t src_pitch, uint32_t valid_w,
                             uint32_t valid_h, int hal_format)
{
	if (!blit_ensure_context())
		return;

	/* Serialize with the readback (sync_gpu_to) and the release-fence: the EGL
	 * state is not thread-safe, and a producer writeback racing a consumer
	 * readback in the same process would corrupt GL state. */
	std::lock_guard<std::mutex> lk(g_blit_mtx);
	/* Bind THIS thread's own EGL context (one per thread; never stolen by
	 * another consumer such as scrcpy, which is what previously caused the
	 * readback's "FBO incomplete" when both ran at once). */
	EGLContext ctx = ensure_thread_context();
	if (ctx == EGL_NO_CONTEXT) {
		ALOGE("sync_cpu_to: no per-thread EGL context");
		return;
	}
	(void)ctx;

	auto eglCreateImage = (PFNEGLCREATEIMAGEPROC)dlsym(g_blit_egl, "eglCreateImage");
	auto eglDestroyImage = (PFNEGLDESTROYIMAGEPROC)dlsym(g_blit_egl, "eglDestroyImage");
	auto glEGLImageTargetTexture2DOES = (void (*)(GLenum, GLeglImageOES))g_blit_getproc("glEGLImageTargetTexture2DOES");
	auto glTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *))g_blit_getproc("glTexSubImage2D");
	auto glGenTextures = (void (*)(GLsizei, GLuint *))g_blit_getproc("glGenTextures");
	auto glBindTexture = (void (*)(GLenum, GLuint))g_blit_getproc("glBindTexture");
	auto glDeleteTextures = (void (*)(GLsizei, const GLuint *))g_blit_getproc("glDeleteTextures");
	auto glTexParameteri = (void (*)(GLenum, GLenum, GLint))g_blit_getproc("glTexParameteri");
	auto glPixelStorei = (void (*)(GLenum, GLint))g_blit_getproc("glPixelStorei");
	auto glGetError = (GLenum (*)(void))g_blit_getproc("glGetError");
	if (!eglCreateImage || !glEGLImageTargetTexture2DOES || !glTexSubImage2D ||
			!glGenTextures || !glBindTexture || !glDeleteTextures ||
			!glTexParameteri || !glPixelStorei) {
		ALOGE("sync_cpu_to: missing GL/EGL image symbols %d-%d-%d-%d-%d-%d-%d-%d",
			!eglCreateImage, !glEGLImageTargetTexture2DOES, !glTexSubImage2D,
			!glGenTextures, !glBindTexture, !glDeleteTextures,
			!glTexParameteri, !glPixelStorei);
		return;
	}

	auto make_image = [&](struct gbm_bo *bo) -> EGLImageKHR {
		int fd = gbm_bo_get_fd(bo);
		int w = gbm_bo_get_width(bo), h = gbm_bo_get_height(bo);
		int stride = gbm_bo_get_stride(bo);
		uint64_t mod = gbm_bo_get_modifier(bo);
		uint32_t fourcc = gbm_bo_get_format(bo);
		EGLAttrib attrs[] = {
			EGL_WIDTH, w,
			EGL_HEIGHT, h,
			EGL_LINUX_DRM_FOURCC_EXT, static_cast<EGLAttrib>(fourcc),
			EGL_DMA_BUF_PLANE0_FD_EXT, fd,
			EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
			EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
			EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, static_cast<EGLAttrib>(mod & 0xFFFFFFFF),
			EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, static_cast<EGLAttrib>(mod >> 32),
			EGL_NONE,
		};
		return eglCreateImage(g_blit_dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attrs);
	};

	/* Pick the GL upload format for this SW buffer; bail if unsupported. */
	GLenum ul_fmt, ul_type;
	uint32_t bpp = 4;
	if (!gralloc_to_gl_upload(hal_format, &ul_fmt, &ul_type, &bpp)) {
		ALOGW("sync_cpu_to: SW format %d not blittable; tiled bo left stale",
		      hal_format);
		return;
	}
	uint32_t w = valid_w, h = valid_h;
	uint32_t cpu_stride = src_pitch;   /* client's actual row pitch */

	/* The client wrote cpu_ptr (a malloc buffer) using handle->stride (the
	 * primary TILED bo's pitch) as the row pitch; that pitch is >= the tight
	 * w*bpp due to row alignment. Re-pack the padded source into a
	 * tightly-packed w x h buffer. nvidia ignores GL_UNPACK_ROW_LENGTH for
	 * EGLImage-backed textures, so re-packing (rather than relying on it) is
	 * the robust fix for the previous sheared/garbled (花屏) output. */
	const void *upload_ptr = cpu_ptr;
	std::vector<unsigned char> tight;
	if (cpu_stride != w * bpp) {
		tight.resize((size_t)w * h * bpp);
		const unsigned char *src = (const unsigned char *)cpu_ptr;
		unsigned char *dst = tight.data();
		uint32_t row_bytes = w * bpp;
		for (uint32_t y = 0; y < h; y++) {
			memcpy(dst, src, row_bytes);
			src += cpu_stride;
			dst += row_bytes;
		}
		upload_ptr = tight.data();
	}

	/* Upload the LINEAR CPU data directly into the shared TILED bo's EGLImage
	 * texture. We deliberately avoid two broken paths on nvidia:
	 *  - texturing the LINEAR dma-buf itself (glEGLImageTargetTexture2DOES
	 *    returns 0x502 for LINEAR RGBA); and
	 *  - glCopyImageSubData into an EGLImage-backed dma-buf (nvidia rejects an
	 *    EGLImage texture as a copy target -> 0x502, the exact failure seen in
	 *    testing: "unlock sync_blit: CPU->GPU copy done glerr=0x502").
	 * glTexSubImage2D into the EGLImage-backed texture is the EGL_KHR_gl_image
	 * sanctioned way to push CPU pixels into a tiled dma-buf. */
	EGLImageKHR dst = make_image(gpu_bo);
	if (dst == EGL_NO_IMAGE_KHR) {
		ALOGE("sync_cpu_to: eglCreateImage(dst) failed");
		return;
	}
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, dst);   /* tiled bo = tex storage */

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/* upload_ptr is already tightly packed (w x h); no UNPACK_ROW_LENGTH */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)w, (GLsizei)h,
	                ul_fmt, ul_type, upload_ptr);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	if (glGetError && glGetError() != GL_NO_ERROR) {
		ALOGE("unlock sync_cpu_to: glTexSubImage2D failed, %dx%d gpu_bo:%p, cpu_ptr:%p",
		      w, h, gpu_bo, cpu_ptr);
		glDeleteTextures(1, &tex);
		eglDestroyImage(g_blit_dpy, dst);
		return;
	}
	ALOGV("unlock sync_blit: CPU->GPU writback %ux%u fmt=%d done. gpu_bo:%p, cpu_ptr:%p",
				w, h, hal_format, gpu_bo, cpu_ptr);

	glDeleteTextures(1, &tex);
	eglDestroyImage(g_blit_dpy, dst);
}

static void sync_gpu_to_cpu(struct gbm_bo *gpu_bo, void *cpu_ptr,
                            uint32_t cpu_stride, int hal_format)
{
	if (!blit_ensure_context())
		return;
	if (!cpu_ptr)
		return;

	GLenum ul_fmt, ul_type;
	uint32_t bpp = 4;
	if (!gralloc_to_gl_upload(hal_format, &ul_fmt, &ul_type, &bpp)) {
		ALOGW("sync_gpu_to: SW format %d not blittable; cpu buffer left stale",
		      hal_format);
		return;
	}

	auto eglCreateImage = (PFNEGLCREATEIMAGEPROC)dlsym(g_blit_egl, "eglCreateImage");
	auto eglDestroyImage = (PFNEGLDESTROYIMAGEPROC)dlsym(g_blit_egl, "eglDestroyImage");
	auto glEGLImageTargetTexture2DOES = (void (*)(GLenum, GLeglImageOES))g_blit_getproc("glEGLImageTargetTexture2DOES");
	auto glGenTextures = (void (*)(GLsizei, GLuint *))g_blit_getproc("glGenTextures");
	auto glBindTexture = (void (*)(GLenum, GLuint))g_blit_getproc("glBindTexture");
	auto glDeleteTextures = (void (*)(GLsizei, const GLuint *))g_blit_getproc("glDeleteTextures");
	auto glGenFramebuffers = (void (*)(GLsizei, GLuint *))g_blit_getproc("glGenFramebuffers");
	auto glDeleteFramebuffers = (void (*)(GLsizei, const GLuint *))g_blit_getproc("glDeleteFramebuffers");
	auto glBindFramebuffer = (void (*)(GLenum, GLuint))g_blit_getproc("glBindFramebuffer");
	auto glFramebufferTexture2D = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint))g_blit_getproc("glFramebufferTexture2D");
	auto glCheckFramebufferStatus = (GLenum (*)(GLenum))g_blit_getproc("glCheckFramebufferStatus");
	auto glReadPixels = (void (*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *))g_blit_getproc("glReadPixels");
	auto glPixelStorei = (void (*)(GLenum, GLint))g_blit_getproc("glPixelStorei");
	auto glFinish = (void (*)(void))g_blit_getproc("glFinish");
	auto glGetError = (GLenum (*)(void))g_blit_getproc("glGetError");
	if (!eglCreateImage || !glEGLImageTargetTexture2DOES || !glGenTextures ||
	    !glBindTexture || !glDeleteTextures || !glGenFramebuffers ||
	    !glBindFramebuffer || !glFramebufferTexture2D || !glCheckFramebufferStatus ||
	    !glDeleteFramebuffers || !glReadPixels || !glPixelStorei) {
		ALOGE("sync_gpu_to: missing GL symbols for GPU->CPU readback %d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
			!eglCreateImage, !glEGLImageTargetTexture2DOES, !glGenTextures,
			!glBindTexture, !glDeleteTextures, !glGenFramebuffers,
			!glBindFramebuffer, !glFramebufferTexture2D, !glCheckFramebufferStatus,
			!glDeleteFramebuffers, !glReadPixels, !glPixelStorei);
		return;
	}

	int w = gbm_bo_get_width(gpu_bo), h = gbm_bo_get_height(gpu_bo);
	int fd = gbm_bo_get_fd(gpu_bo);
	int stride = gbm_bo_get_stride(gpu_bo);
	uint64_t mod = gbm_bo_get_modifier(gpu_bo);
	uint32_t fourcc = gbm_bo_get_format(gpu_bo);

	struct bo_data_t *bo_data = gbm_bo_data(gpu_bo);
	if (!bo_data) {
		ALOGE("sync_gpu_to: no bo_data for gpu_bo %p", gpu_bo);
		return;
	}

	/* Serialize ALL GL work. The EGL state is not thread-safe, so concurrent
	 * users (scrcpy + local recording) or a producer and consumer in the same
	 * process must not interleave GL calls. Each thread has its OWN context
	 * (ensure_thread_context), so there is no cross-thread context stealing --
	 * the former source of "FBO incomplete". */
	std::lock_guard<std::mutex> lk(g_blit_mtx);
	/* Bind THIS thread's own EGL context (never stolen by another consumer). */
	EGLContext ctx = ensure_thread_context();
	if (ctx == EGL_NO_CONTEXT) {
		ALOGE("sync_gpu_to: no per-thread EGL context");
		return;
	}
	(void)ctx;

	/* Create the readback GL objects transiently for this call and destroy
	 * them before returning. Transient (rather than cached per bo) avoids any
	 * cross-thread / cross-context cache invalidation and leaves nothing tied
	 * to the bo's lifetime, so buffer destroy needs no GL teardown. Each
	 * thread builds its own texture/FBO in its OWN context, so scrcpy and the
	 * local recorder never share (or corrupt) each other's GL objects. */
	EGLImageKHR img = EGL_NO_IMAGE_KHR;
	GLuint tex = 0, fbo = 0;
	{
		EGLAttrib attrs[] = {
			EGL_WIDTH, w,
			EGL_HEIGHT, h,
			EGL_LINUX_DRM_FOURCC_EXT, (EGLAttrib)fourcc,
			EGL_DMA_BUF_PLANE0_FD_EXT, fd,
			EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
			EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
			EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, (EGLAttrib)(mod & 0xFFFFFFFF),
			EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, (EGLAttrib)(mod >> 32),
			EGL_NONE,
		};
		img = eglCreateImage(g_blit_dpy, EGL_NO_CONTEXT,
		                     EGL_LINUX_DMA_BUF_EXT, NULL, attrs);
		if (img == EGL_NO_IMAGE_KHR) {
			ALOGE("sync_gpu_to: eglCreateImage(src tiled) failed");
			return;
		}
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, img);
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		                      GL_TEXTURE_2D, tex, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			ALOGE("sync_gpu_to: FBO incomplete on create");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &fbo);
			glDeleteTextures(1, &tex);
			eglDestroyImage(g_blit_dpy, img);
			return;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		ALOGE("sync_gpu_to: FBO incomplete on use");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &tex);
		eglDestroyImage(g_blit_dpy, img);
		return;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glFinish();
	glReadPixels(0, 0, (GLsizei)w, (GLsizei)h, ul_fmt, ul_type, cpu_ptr);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLenum rb_err = glGetError ? glGetError() : GL_NO_ERROR;

	/* Transient: free the readback GL objects now. Nothing is cached on the
	 * bo, so buffer destroy needs no GL teardown and there is no per-buffer
	 * GPU-memory leak. */
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &tex);
	eglDestroyImage(g_blit_dpy, img);

	if (rb_err != GL_NO_ERROR) {
		ALOGE("sync_gpu_to: glReadPixels failed (0x%x)", rb_err);
		return;
	}

	ALOGV("  lock sync_blit: GPU->CPU readback %ux%u fmt=%d done. gpu_bo:%p, cpu_ptr:%p",
		w, h, hal_format, gpu_bo, cpu_ptr);
}

void gbm_dev_destroy(struct gbm_device *gbm)
{
	int fd = gbm_device_get_fd(gbm);

	gbm_device_destroy(gbm);
	close(fd);
}

struct gbm_device *gbm_dev_create(void)
{
	struct gbm_device *gbm;
	char path[PROPERTY_VALUE_MAX];
	int fd;

	property_get("gralloc.gbm.device", path, "/dev/dri/renderD128");
	fd = open(path, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		ALOGE("failed to open %s", path);
		return NULL;
	}

	gbm = gbm_create_device(fd);
	if (!gbm) {
		ALOGE("failed to create gbm device");
		close(fd);
	}

	char egl_type[PROPERTY_VALUE_MAX];
	property_get("ro.hardware.egl", egl_type, "none");
	if (strcmp(egl_type, "mesa") == 0) {
		align_x = 256;
	} else if (strcmp(egl_type, "FTG340") == 0) {
		align_x = 32;
	}

	char gpu_model[PROPERTY_VALUE_MAX];
	property_get("ro.hardware.gpu.model", gpu_model, "none");
	if (strcmp(gpu_model, "nvidia") == 0) {
		is_nvidia = 1;
		align_x = 16;
	}

	return gbm;
}

/*
 * Register a buffer handle.
 */
int gralloc_gbm_handle_register(buffer_handle_t _handle, struct gbm_device *gbm)
{
	struct gbm_bo *bo;

	if (!_handle)
		return -EINVAL;

	if (gbm_bo_handle_map.count(_handle))
		return -EINVAL;

	bo = gbm_import(gbm, _handle);
	if (!bo)
		return -EINVAL;

	gbm_bo_handle_map.emplace(_handle, bo);

	return 0;
}

/*
 * Unregister a buffer handle.  It is no-op for handles created locally.
 */
int gralloc_gbm_handle_unregister(buffer_handle_t handle)
{
	gbm_free(handle);

	return 0;
}

/*
 * Create a bo.
 */
buffer_handle_t gralloc_gbm_bo_create(struct gbm_device *gbm,
		int width, int height, int format, int usage, int *stride)
{
	struct gbm_bo *bo;
	native_handle_t *handle;

	handle = gralloc_handle_create(width, height, format, usage);
	if (!handle)
		return NULL;

	bo = gbm_alloc(gbm, handle);
	if (!bo) {
		native_handle_delete(handle);
		return NULL;
	}

	gbm_bo_handle_map.emplace(handle, bo);

	/* in pixels */
	*stride = gralloc_handle(handle)->stride / gralloc_gbm_get_bpp(format);

	return handle;
}

/*
 * Lock a bo.  XXX thread-safety?
 */
int gralloc_gbm_bo_lock(buffer_handle_t handle,
		int usage, int /*x*/, int /*y*/, int /*w*/, int /*h*/,
		void **addr)
{
	struct gralloc_handle_t *gbm_handle = gralloc_handle(handle);
	struct gbm_bo *bo = gralloc_gbm_bo_from_handle(handle);
	struct bo_data_t *bo_data;

	if (!bo)
		return -EINVAL;

	if ((gbm_handle->usage & usage) != (uint32_t)usage) {
		/* make FB special for testing software renderer with */

		if (!(gbm_handle->usage & (
				GRALLOC_USAGE_SW_READ_OFTEN |
				GRALLOC_USAGE_HW_FB |
				GRALLOC_USAGE_HW_TEXTURE |
				GRALLOC_USAGE_HW_VIDEO_ENCODER))) {

			ALOGE("bo.usage:x%X/usage:x%X is not GRALLOC_USAGE_HW_{FB,TEXTURE,VIDEO_ENCODER}",
				gbm_handle->usage, usage);
			return -EINVAL;
		}
	}

	bo_data = gbm_bo_data(bo);
	if (!bo_data) {
		bo_data = new struct bo_data_t();
		gbm_bo_set_user_data(bo, bo_data, gralloc_gbm_destroy_user_data);
	}

	ALOGV("lock bo %p, cnt=%d, usage=%x", bo, bo_data->lock_count, usage);

	/* allow multiple locks with compatible usages */
	if (bo_data->lock_count && (bo_data->locked_for & usage) != usage)
		return -EINVAL;

	usage |= bo_data->locked_for;

	if (usage & (GRALLOC_USAGE_SW_WRITE_MASK |
		     GRALLOC_USAGE_SW_READ_MASK)) {
		int write = !!(usage & GRALLOC_USAGE_SW_WRITE_MASK);
		bool need_sync = gralloc_need_sync(gbm_handle);
		if (need_sync) {
			size_t rb_bytes = (size_t)gbm_handle->stride *
			                 gbm_handle->height;
			void *cache = alloc_cpu_cache(bo_data, rb_bytes);
			if (!cache) {
				ALOGE("lock: failed to alloc cpu_cache (%zu bytes) for "
				      "dual buffer %dx%d fmt=%d",
				      rb_bytes, gbm_handle->width, gbm_handle->height,
				      gbm_handle->format);
				return -ENOMEM;
			}
			int is_encoder = !!(gbm_handle->usage & GRALLOC_USAGE_HW_VIDEO_ENCODER);
			int is_hw_compose = !!(gbm_handle->usage & GRALLOC_USAGE_HW_COMPOSER);
			if (!write && !is_encoder && !is_hw_compose) {
				sync_gpu_to_cpu(bo, cache,
							gbm_handle->stride,
							gbm_handle->format);
			}
			*addr = cache;
			bo_data->cpu_ptr = cache;
			bo_data->cpu_sync = true;
		} else {
			/* Non-sync CPU access: just map the primary bo; no GPU
			 * readback / writeback needed. */
			int err = gbm_map(handle, write, addr);
			if (err) {
				ALOGE("gralloc_gbm_bo_lock fail, gbm_map is invalid, write:%d", write);
				return err;
			}
		}
	} else {
		/* kernel handles the synchronization here */
	}

	bo_data->lock_count++;
	bo_data->locked_for |= usage;

	return 0;
}

/*
 * Unlock a bo.
 */
int gralloc_gbm_bo_unlock(buffer_handle_t handle)
{
	struct gralloc_handle_t *gbm_handle = gralloc_handle(handle);

	struct gbm_bo *bo = gralloc_gbm_bo_from_handle(handle);
	struct bo_data_t *bo_data;
	if (!bo)
		return -EINVAL;

	bo_data = gbm_bo_data(bo);

	int mapped = bo_data->locked_for &
		(GRALLOC_USAGE_SW_WRITE_MASK | GRALLOC_USAGE_SW_READ_MASK);

	if (!bo_data->lock_count)
		return 0;

	if (mapped) {
		bool need_sync = bo_data->cpu_sync && (bo_data->locked_for & GRALLOC_USAGE_SW_WRITE_MASK);
		if (need_sync) {
			sync_cpu_to_gpu(bo, bo_data->cpu_ptr, gbm_handle->stride,
				                gbm_handle->width, gbm_handle->height, gbm_handle->format);
		}
		gbm_unmap(bo);
	}

	bo_data->lock_count--;
	if (!bo_data->lock_count)
		bo_data->locked_for = 0;

	return 0;
}

int gralloc_gbm_bo_lock_ycbcr(buffer_handle_t handle,
		int usage, int x, int y, int w, int h,
		struct android_ycbcr *ycbcr)
{
	struct gralloc_handle_t *hnd = gralloc_handle(handle);
	int ystride, cstride;
	void *addr = 0;
	int err;

	ALOGV("handle %p, hnd %p, usage 0x%x", handle, hnd, usage);

	err = gralloc_gbm_bo_lock(handle, usage, x, y, w, h, &addr);
	if (err)
		return err;

	memset(ycbcr->reserved, 0, sizeof(ycbcr->reserved));

	switch (hnd->format) {
	case HAL_PIXEL_FORMAT_YCrCb_420_SP:
		ystride = cstride = hnd->stride;
		ycbcr->y = addr;
		ycbcr->cr = (unsigned char *)addr + ystride * hnd->height;
		ycbcr->cb = (unsigned char *)addr + ystride * hnd->height + 1;
		ycbcr->ystride = ystride;
		ycbcr->cstride = cstride;
		ycbcr->chroma_step = 2;
		break;
	case HAL_PIXEL_FORMAT_YCbCr_420_888:
		ystride = cstride = hnd->stride;
		ycbcr->y = addr;
		ycbcr->cb = (unsigned char *)addr + ystride * hnd->height;
		ycbcr->cr = (unsigned char *)addr + ystride * hnd->height + 1;
		ycbcr->ystride = ystride;
		ycbcr->cstride = cstride;
		ycbcr->chroma_step = 2;
		break;
	case HAL_PIXEL_FORMAT_YV12:
		ystride = hnd->stride;
		cstride = hnd->stride / 2;
		ycbcr->y = addr;
		ycbcr->cr = (unsigned char *)addr + ystride * hnd->height;
		ycbcr->cb = (unsigned char *)addr + ystride * hnd->height + cstride * hnd->height / 2;
		ycbcr->ystride = ystride;
		ycbcr->cstride = cstride;
		ycbcr->chroma_step = 1;
		break;
	default:
		ALOGE("Can not lock buffer, invalid format: 0x%x", hnd->format);
		return -EINVAL;
	}

	return 0;
}
