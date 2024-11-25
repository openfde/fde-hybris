#pragma once

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef HYBRIS_GET_SYMBOL_ADDRESS
// Should dedine HYBRIS_LIBNAME macro before include this header file
// You can define a environment variable by HYBRIS_ENVNAME macro
static void *hybris_open_library() {
  static void *lib_handle = NULL;
  static int loaded = 0;
  if (loaded == 0) {
#ifndef HYBRIS_LIBNAME
#error No specify library name by HYBRIS_LIBNAME macro
#else
    const char *name = HYBRIS_LIBNAME;
#ifdef HYBRIS_ENVNAME
    const char *env_name = getenv(HYBRIS_ENVNAME);
    name = env_name ? env_name : name;
#endif
    lib_handle = dlopen(name, RTLD_LOCAL);
    if (!lib_handle) {
      fprintf(stderr, "dlopen %s failed: %s", name, dlerror());
    }
    loaded = 1;
#endif
  }
  return lib_handle;
}

#define HYBRIS_GET_SYMBOL_ADDRESS(symbol) \
  ({                                      \
    void *addr = NULL;                    \
    void *handle = hybris_open_library(); \
    if (handle) {                         \
      addr = dlsym(handle, #symbol);      \
    }                                     \
    addr;                                 \
  })
#endif

#define HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol) \
  ({                                                      \
    static func_type f = NULL;                            \
    if (!f) {                                             \
      f = (func_type)HYBRIS_GET_SYMBOL_ADDRESS(symbol);   \
    }                                                     \
    f;                                                    \
  })

#ifndef HYBRIS_VISIBILITY
#define HYBRIS_VISIBILITY __attribute__((visibility("default")))
#endif

#define HYBRIS_IMPLEMENT_FUNCTION0(return_type, symbol)              \
  HYBRIS_VISIBILITY return_type symbol(void) {                       \
    typedef return_type (*func_type)(void);                          \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol); \
    return f();                                                      \
  }

#define HYBRIS_IMPLEMENT_FUNCTION1(return_type, symbol, t1)          \
  HYBRIS_VISIBILITY return_type symbol(t1 v1) {                      \
    typedef return_type (*func_type)(t1);                            \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol); \
    return f(v1);                                                    \
  }

#define HYBRIS_IMPLEMENT_FUNCTION2(return_type, symbol, t1, t2)      \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2) {               \
    typedef return_type (*func_type)(t1, t2);                        \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol); \
    return f(v1, v2);                                                \
  }

#define HYBRIS_IMPLEMENT_FUNCTION3(return_type, symbol, t1, t2, t3)  \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3) {        \
    typedef return_type (*func_type)(t1, t2, t3);                    \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol); \
    return f(v1, v2, v3);                                            \
  }

#define HYBRIS_IMPLEMENT_FUNCTION4(return_type, symbol, t1, t2, t3, t4) \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4) {    \
    typedef return_type (*func_type)(t1, t2, t3, t4);                   \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);    \
    return f(v1, v2, v3, v4);                                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION5(return_type, symbol, t1, t2, t3, t4, t5) \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5) { \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5);                   \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);        \
    return f(v1, v2, v3, v4, v5);                                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION6(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6)                                      \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6) {                             \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6);               \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);        \
    return f(v1, v2, v3, v4, v5, v6);                                       \
  }

#define HYBRIS_IMPLEMENT_FUNCTION7(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6, t7)                                  \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6, t7 v7) {                      \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7);           \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);        \
    return f(v1, v2, v3, v4, v5, v6, v7);                                   \
  }

#define HYBRIS_IMPLEMENT_FUNCTION8(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6, t7, t8)                              \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6, t7 v7, t8 v8) {               \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8);       \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);        \
    return f(v1, v2, v3, v4, v5, v6, v7, v8);                               \
  }

#define HYBRIS_IMPLEMENT_FUNCTION9(return_type, symbol, t1, t2, t3, t4, t5, \
                                   t6, t7, t8, t9)                          \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,   \
                                       t6 v6, t7 v7, t8 v8, t9 v9) {        \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9);   \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);        \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9);                           \
  }

#define HYBRIS_IMPLEMENT_FUNCTION10(return_type, symbol, t1, t2, t3, t4, t5,   \
                                    t6, t7, t8, t9, t10)                       \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,      \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10) {  \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10); \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);           \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);                         \
  }

#define HYBRIS_IMPLEMENT_FUNCTION11(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11)                 \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11) {                             \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11);                                    \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);          \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11);                   \
  }

#define HYBRIS_IMPLEMENT_FUNCTION12(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11, t12)            \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11, t12 v12) {                    \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11, t12);                               \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);          \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);              \
  }

#define HYBRIS_IMPLEMENT_FUNCTION13(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11, t12, t13)       \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11, t12 v12, t13 v13) {           \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11, t12, t13);                          \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);          \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);         \
  }

#define HYBRIS_IMPLEMENT_FUNCTION14(return_type, symbol, t1, t2, t3, t4, t5,  \
                                    t6, t7, t8, t9, t10, t11, t12, t13, t14)  \
  HYBRIS_VISIBILITY return_type symbol(t1 v1, t2 v2, t3 v3, t4 v4, t5 v5,     \
                                       t6 v6, t7 v7, t8 v8, t9 v9, t10 v10,   \
                                       t11 v11, t12 v12, t13 v13, t14 v14) {  \
    typedef return_type (*func_type)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, \
                                     t11, t12, t13, t14);                     \
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);          \
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
    func_type f = HYBRIS_GET_SYMBOL_ADDRESS_ONCE(func_type, symbol);          \
    return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14,     \
             v15);                                                            \
  }
