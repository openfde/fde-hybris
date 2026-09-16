
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <errno.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <log/log.h>


/* 辅助函数：打印一个整型属性 */
static void print_attrib(EGLDisplay dpy, EGLConfig config, EGLint attrib, const char *name) {
    EGLint value;
    if (eglGetConfigAttrib(dpy, config, attrib, &value)) {
        printf("  %-24s: 0x%x\n", name, value);
    } else {
        printf("  %-24s: <error>\n", name);
    }
}

/* 辅助函数：打印一个布尔属性（0/1 转为 Yes/No） */
static void print_bool_attrib(EGLDisplay dpy, EGLConfig config, EGLint attrib, const char *name) {
    EGLint value;
    if (eglGetConfigAttrib(dpy, config, attrib, &value)) {
        printf("  %-24s: %s\n", name, value ? "Yes" : "No");
    } else {
        printf("  %-24s: <error>\n", name);
    }
}

int printEGLConfig(EGLDisplay dpy) {
    EGLint num_configs;
    EGLConfig *configs;
    int i;

    /* 3. 获取所有配置数量 */
    if (!eglGetConfigs(dpy, NULL, 0, &num_configs) || num_configs == 0) {
        fprintf(stderr, "No EGLConfigs found\n");
        eglTerminate(dpy);
        return 1;
    }
    printf("Number of EGLConfigs: %d\n\n", num_configs);

    /* 4. 分配数组并获取所有配置 */
    configs = (EGLConfig *)malloc(num_configs * sizeof(EGLConfig));
    if (!configs) {
        fprintf(stderr, "Memory allocation failed\n");
        eglTerminate(dpy);
        return 1;
    }
    if (!eglGetConfigs(dpy, configs, num_configs, &num_configs)) {
        fprintf(stderr, "eglGetConfigs failed\n");
        free(configs);
        eglTerminate(dpy);
        return 1;
    }

    int cnt_pbuffer = 0;
    int cnt_wbuffer = 0;
    /* 5. 遍历并打印每个配置的属性 */
    for (i = 0; i < num_configs; i++) {
        EGLConfig cfg = configs[i];
        EGLint id;

        EGLint value;
        if (eglGetConfigAttrib(dpy, cfg, EGL_SURFACE_TYPE, &value)) {
            if (value | EGL_PBUFFER_BIT) {
                cnt_pbuffer++;
            } else if (value | EGL_WINDOW_BIT) {
                cnt_wbuffer++;
            }
        }
        
#if 0
        eglGetConfigAttrib(dpy, cfg, EGL_CONFIG_ID, &id);
        printf("==================================================\n");
        printf("Config #%d (ID: %d)\n", i, id);
        /* 表面类型 (位掩码) */
        print_attrib(dpy, cfg, EGL_SURFACE_TYPE,      "EGL_SURFACE_TYPE");
        /* 可渲染类型 (位掩码) */
        print_attrib(dpy, cfg, EGL_RENDERABLE_TYPE,   "EGL_RENDERABLE_TYPE");
        /* 基本颜色缓冲 */
        print_attrib(dpy, cfg, EGL_BUFFER_SIZE,       "EGL_BUFFER_SIZE");
        print_attrib(dpy, cfg, EGL_RED_SIZE,          "EGL_RED_SIZE");
        print_attrib(dpy, cfg, EGL_GREEN_SIZE,        "EGL_GREEN_SIZE");
        print_attrib(dpy, cfg, EGL_BLUE_SIZE,         "EGL_BLUE_SIZE");
        print_attrib(dpy, cfg, EGL_ALPHA_SIZE,        "EGL_ALPHA_SIZE");
        /* 深度与模板 */
        print_attrib(dpy, cfg, EGL_DEPTH_SIZE,        "EGL_DEPTH_SIZE");
        print_attrib(dpy, cfg, EGL_STENCIL_SIZE,      "EGL_STENCIL_SIZE");
        /* 配置属性 */
        print_attrib(dpy, cfg, EGL_CONFIG_CAVEAT,     "EGL_CONFIG_CAVEAT");
        print_attrib(dpy, cfg, EGL_NATIVE_VISUAL_ID,  "EGL_NATIVE_VISUAL_ID");
        print_attrib(dpy, cfg, EGL_NATIVE_VISUAL_TYPE,"EGL_NATIVE_VISUAL_TYPE");
        printf("\n");
#endif
    }

    printf("hybris --- get EGL_PBUFFER_BIT Cnt:%2d/%2d\n", cnt_pbuffer, num_configs);
    printf("hybris --- get EGL_WINDOW_BIT  Cnt:%2d/%2d\n\n", cnt_wbuffer, num_configs);
    /* 清理 */
    free(configs);
    return 0;
}

int main(int argc, char* argv[]) {
    printf("\n eglGetPlatformDisplay EGL\n");
    EGLDisplay egl_display = EGL_NO_DISPLAY;//eglGetDisplay(EGL_DEFAULT_DISPLAY);
    // EGLDisplay egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_display == EGL_NO_DISPLAY) {
        // 尝试使用GBM平台扩展
        // PFNEGLGETPLATFORMDISPLAYPROC eglGetPlatformDisplay =
        //     (PFNEGLGETPLATFORMDISPLAYPROC)eglGetProcAddress("eglGetPlatformDisplay");
        egl_display = eglGetPlatformDisplay(EGL_PLATFORM_ANDROID_KHR, EGL_DEFAULT_DISPLAY, NULL);
        // for linux platform
        //egl_display = eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL);
        //egl_display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, EGL_DEFAULT_DISPLAY, NULL);
        if (egl_display == EGL_NO_DISPLAY)
        {
            ALOGD("hybris - Failed to get EGL display: 0x%x\n", eglGetError());
            return -1;
        }
    }

    printf("\n eglInitialize EGL\n");
    if (!eglInitialize(egl_display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        eglTerminate(egl_display);
        return -1;
    }
    
    printEGLConfig(egl_display);
    
    printf("EGL initialized: %s, EXTENSIONS:%s\n", eglQueryString(egl_display, EGL_VERSION), eglQueryString(egl_display, EGL_EXTENSIONS));
    
    if (strstr(eglQueryString(egl_display, EGL_EXTENSIONS), "EGL_EXT_image_dma_buf_import") != NULL) {
        printf("\nThe EGL_EXTENSIONS have EGL_EXT_image_dma_buf_import\n\n");
    } else {
        printf("\nThe EGL_EXTENSIONS Not have EGL_EXT_image_dma_buf_import, Fail!!\n\n");
    }

    
#if 1
    const EGLint neededAttribs[] = {
      EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
    };
    EGLConfig egl_config_test = 0;
    EGLint num_configs_test;
    if (!eglChooseConfig(egl_display, neededAttribs, &egl_config_test, 1, &num_configs_test)) {
        printf("Failed to choose EGL config\n");
    }
    printf("Test . EGL eglChooseConfig num:%d, err:0x%x\n", num_configs_test, eglGetError());
    
    
    EGLint eglSwapBehavior = EGL_SWAP_BEHAVIOR_PRESERVED_BIT;

    EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                        EGL_OPENGL_ES2_BIT,
                        EGL_RED_SIZE,
                        8,
                        EGL_GREEN_SIZE,
                        8,
                        EGL_BLUE_SIZE,
                        8,
                        EGL_ALPHA_SIZE,
                        8,
                        EGL_DEPTH_SIZE,
                        0,
                        EGL_CONFIG_CAVEAT,
                        EGL_NONE,
                        //EGL_STENCIL_SIZE,
                        //STENCIL_BUFFER_SIZE,
                        EGL_SURFACE_TYPE,
                        EGL_PBUFFER_BIT,//EGL_WINDOW_BIT,//
                        EGL_NONE};
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EGLint numConfigs = 1;
    if (!eglChooseConfig(egl_display, attribs, &config, numConfigs, &numConfigs)) {
        printf("eglChooseConfig fail\n");
    } else {
        printf("eglChooseConfig suc numConfigs:%d\n", numConfigs);
    }
#endif

    // 8. 选择EGL配置
    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    
    EGLConfig egl_config;
    EGLint num_configs;
    if (!eglChooseConfig(egl_display, config_attribs, &egl_config, 1, &num_configs)) {
        fprintf(stderr, "Failed to choose EGL config\n");
        eglTerminate(egl_display);
        return -1;
    }
    printf("EGL eglChooseConfig success\n");
    
    // 9. 创建EGL上下文
    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLContext egl_context = eglCreateContext(egl_display, egl_config,
                                             EGL_NO_CONTEXT, context_attribs);
    if (egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        eglTerminate(egl_display);
        return -1;
    }
    printf("EGL eglCreateContext success\n");


    eglDestroyContext(egl_display, egl_context);
    eglTerminate(egl_display);
    
    return 0;
}