#pragma once

#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>

// Should dedine HYBRIS_LIBNAME macro before include this header file
// You can define a environment variable by HYBRIS_ENVNAME macro
static void *hybris_open_library() {
  static void *lib_handle = NULL;
  if (!lib_handle) {
#ifndef HYBRIS_LIBNAME
#error No specify library name by HYBRIS_LIBNAME macro
#else
    const char *name = HYBRIS_LIBNAME;
#ifdef HYBRIS_ENVNAME
    const char *env_name = getenv(HYBRIS_ENVNAME);
    name = env_name ? env_name : name;
#endif
    lib_handle = dlopen(name, RTLD_GLOBAL);
#endif
  }
  assert(lib_handle != NULL);
  return lib_handle;
}

#ifndef HYBRIS_VISIBILITY
#define HYBRIS_VISIBILITY __attribute__((visibility("default")))
#endif

#define HYBRIS_IMPLEMENT_FUNCTION0(return_type, symbol) \
  HYBRIS_VISIBILITY return_type symbol(void) {          \
    typedef return_type (*func_type)(void);             \
    static func_type f = NULL;                          \
    if (!f) {                                           \
      void *handle = hybris_open_library();             \
      f = (func_type)dlsym(handle, #symbol);            \
    }                                                   \
    return f();                                         \
  }

#define HYBRIS_IMPLEMENT_FUNCTION1(return_type, symbol, t1) \
  HYBRIS_VISIBILITY return_type symbol(t1 v1) {             \
    typedef return_type (*func_type)(t1);                   \
    static func_type f = NULL;                              \
    if (!f) {                                               \
      void *handle = hybris_open_library();                 \
      f = (func_type)dlsym(handle, #symbol);                \
    }                                                       \
    return f(v1);                                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION2(return_type, symbol, t1, t2) \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2) {          \
    typedef return_type (*func_type)(t1, t2);                   \
    static func_type f = NULL;                                  \
    if (!f) {                                                   \
      void *handle = hybris_open_library();                     \
      f = (func_type)dlsym(handle, #symbol);                    \
    }                                                           \
    return f(v1, v2);                                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION3(return_type, symbol, t1, t2, t3) \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3) {       \
    typedef return_type (*func_type)(t1, t2, t3);                   \
    static func_type f = NULL;                                      \
    if (!f) {                                                       \
      void *handle = hybris_open_library();                         \
      f = (func_type)dlsym(handle, #symbol);                        \
    }                                                               \
    return f(v1, v2, v3);                                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION4(return_type, symbol, t1, t2, t3, t4) \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4) {    \
    typedef return_type (*func_type)(t1, t2, t3, t4);                   \
    static func_type f = NULL;                                          \
    if (!f) {                                                           \
      void *handle = hybris_open_library();                             \
      f = (func_type)dlsym(handle, #symbol);                            \
    }                                                                   \
    return f(v1, v2, v3, v4);                                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION5(return_type, symbol, t1, t2, t3, t4, t5) \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5) { \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5);                   \
    static func_type f = NULL;                                              \
    if (!f) {                                                               \
      void *handle = hybris_open_library();                                 \
      f = (func_type)dlsym(handle, #symbol);                                \
    }                                                                       \
    return f(v1, v2, v3, v4, v5);                                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION6(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6)                                      \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6) {                             \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6);               \
    static func_type f = NULL;                                              \
    if (!f) {                                                               \
      void *handle = hybris_open_library();                                 \
      f = (func_type)dlsym(handle, #symbol);                                \
    }                                                                       \
    return f(v1, v2, v3, v4, v5, v6);                                       \
  }

#define HYBRIS_IMPLEMENT_FUNCTION7(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6, t7)                                  \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6, t7 v7) {                      \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7);           \
    static func_type f = NULL;                                              \
    if (!f) {                                                               \
      void *handle = hybris_open_library();                                 \
      f = (func_type)dlsym(handle, #symbol);                                \
    }                                                                       \
    return f(v1, v2, v3, v4, v5, v6, v7);                                   \
  }

#define HYBRIS_IMPLEMENT_FUNCTION8(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6, t7, t8)                              \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6, t7 v7, t8 v8) {               \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8);       \
    static func_type f = NULL;                                              \
    if (!f) {                                                               \
      void *handle = hybris_open_library();                                 \
      f = (func_type)dlsym(handle, #symbol);                                \
    }                                                                       \
    return f(v1, v2, v3, v4, v5, v6, v7, v8);                               \
  }

#define HYBRIS_IMPLEMENT_FUNCTION9(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6, t7, t8, t9)                          \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6, t7 v7, t8 v8, t9 v9) {        \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9);   \
    static func_type f = NULL;                                              \
    if (!f) {                                                               \
      void *handle = hybris_open_library();                                 \
      f = (func_type)dlsym(handle, #symbol);                                \
    }                                                                       \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9);                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION10(return_type, symbol, t1, t2, t3, t4, t5,   \
                                    t6, t7, t8, t9, t10)                       \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,      \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10) {  \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10); \
    static func_type f = NULL;                                                 \
    if (!f) {                                                                  \
      void *handle = hybris_open_library();                                    \
      f = (func_type)dlsym(handle, #symbol);                                   \
    }                                                                          \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);                         \
  }

#define HYBRIS_IMPLEMENT_FUNCTION11(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11)                 \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11) {                             \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11);                                    \
    static func_type f = NULL;                                                \
    if (!f) {                                                                 \
      void *handle = hybris_open_library();                                   \
      f = (func_type)dlsym(handle, #symbol);                                  \
    }                                                                         \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11);                   \
  }

#define HYBRIS_IMPLEMENT_FUNCTION12(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11, t12)            \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11, t12 v12) {                    \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11, t12);                               \
    static func_type f = NULL;                                                \
    if (!f) {                                                                 \
      void *handle = hybris_open_library();                                   \
      f = (func_type)dlsym(handle, #symbol);                                  \
    }                                                                         \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);              \
  }

#define HYBRIS_IMPLEMENT_FUNCTION13(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11, t12, t13)       \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11, t12 v12, t13 v13) {           \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11, t12, t13);                          \
    static func_type f = NULL;                                                \
    if (!f) {                                                                 \
      void *handle = hybris_open_library();                                   \
      f = (func_type)dlsym(handle, #symbol);                                  \
    }                                                                         \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);         \
  }

#define HYBRIS_IMPLEMENT_FUNCTION14(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11, t12, t13, t14)  \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11, t12 v12, t13 v13, t14 v14) {  \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11, t12, t13, t14);                     \
    static func_type f = NULL;                                                \
    if (!f) {                                                                 \
      void *handle = hybris_open_library();                                   \
      f = (func_type)dlsym(handle, #symbol);                                  \
    }                                                                         \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14);    \
  }

#define HYBRIS_IMPLEMENT_FUNCTION15(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11, t12, t13, t14,  \
                                    t15)                                      \
  HYBRIS_VISIBILITY return_type symbol(                                       \
      t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6, t7 v7, t8 v8, t9 v9, t10 v10, \
      t11 v11, t12 v12, t13 v13, t14 v14, t15 v15) {                          \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11, t12, t13, t14, t15);                \
    static func_type f = NULL;                                                \
    if (!f) {                                                                 \
      void *handle = hybris_open_library();                                   \
      f = (func_type)dlsym(handle, #symbol);                                  \
    }                                                                         \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14,     \
             v15);                                                            \
  }
