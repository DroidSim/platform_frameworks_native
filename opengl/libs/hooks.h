/*
 ** Copyright 2007, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef ANDROID_GLES_CM_HOOKS_H
#define ANDROID_GLES_CM_HOOKS_H

#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

// set to 1 for debugging
#define USE_SLOW_BINDING    0

#undef NELEM
#define NELEM(x)            (sizeof(x)/sizeof(*(x)))

// maximum number of GL extensions that can be used simultaneously in
// a given process. this limitation exists because we need to have
// a static function for each extension and currently these static functions
// are generated at compile time.
#define MAX_NUMBER_OF_GL_EXTENSIONS 256


#ifdef ANDROID_GNU_LINUX
/* clayc: copied from bionic/libc/private/__get_tls.h */
#if defined(__aarch64__)
# define __get_tls() ({ void** __val; __asm__("mrs %0, tpidr_el0" : "=r"(__val)); __val; })
#elif defined(__arm__)
# define __get_tls() ({ void** __val; __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); __val; })
#elif defined(__mips__)
# define __get_tls() \
    /* On mips32r1, this goes via a kernel illegal instruction trap that's optimized for v1. */ \
    ({ register void** __val asm("v1"); \
       __asm__(".set    push\n" \
               ".set    mips32r2\n" \
               "rdhwr   %0,$29\n" \
               ".set    pop\n" : "=r"(__val)); \
       __val; })
#elif defined(__i386__)
# define __get_tls() ({ void** __val; __asm__("movl %%gs:0, %0" : "=r"(__val)); __val; })
#elif defined(__x86_64__)
# define __get_tls() ({ void** __val; __asm__("mov %%fs:0, %0" : "=r"(__val)); __val; })
#else
#error unsupported architecture
#endif
/* __get_tls.h */
/* clayc: copied from bionic/libc/private/bionic_tls.h */
/* Well-known TLS slots. What data goes in which slot is arbitrary unless otherwise noted. */
enum {
  TLS_SLOT_SELF = 0, /* The kernel requires this specific slot for x86. */
  TLS_SLOT_THREAD_ID,
  TLS_SLOT_ERRNO,

  /* This slot in the child's TLS is used to synchronize the parent and child
   * during thread initialization. The child finishes with this mutex before
   * running any code that can set errno, so we can reuse the errno slot. */
  TLS_SLOT_START_MUTEX = TLS_SLOT_ERRNO,

  /* These two aren't used by bionic itself, but allow the graphics code to
   * access TLS directly rather than using the pthread API. */
  TLS_SLOT_OPENGL_API = 3,
  TLS_SLOT_OPENGL = 4,

  /* This slot is only used to pass information from the dynamic linker to
   * libc.so when the C library is loaded in to memory. The C runtime init
   * function will then clear it. Since its use is extremely temporary,
   * we reuse an existing location that isn't needed during libc startup. */
  TLS_SLOT_BIONIC_PREINIT = TLS_SLOT_OPENGL_API,

  TLS_SLOT_STACK_GUARD = 5, /* GCC requires this specific slot for x86. */
  TLS_SLOT_DLERROR,

  TLS_SLOT_FIRST_USER_SLOT /* Must come last! */
};
/* bionic_tls.h */
#else
#include <bionic_tls.h>  /* special private C library header */
#endif

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

// GL / EGL hooks

#undef GL_ENTRY
#undef EGL_ENTRY
#define GL_ENTRY(_r, _api, ...) _r (*_api)(__VA_ARGS__);
#define EGL_ENTRY(_r, _api, ...) _r (*_api)(__VA_ARGS__);

struct egl_t {
    #include "EGL/egl_entries.in"
};

struct gl_hooks_t {
    struct gl_t {
        #include "entries.in"
    } gl;
    struct gl_ext_t {
        __eglMustCastToProperFunctionPointerType extensions[MAX_NUMBER_OF_GL_EXTENSIONS];
    } ext;
};
#undef GL_ENTRY
#undef EGL_ENTRY

EGLAPI void setGlThreadSpecific(gl_hooks_t const *value);

// We have a dedicated TLS slot in bionic
inline gl_hooks_t const * volatile * get_tls_hooks() {
    volatile void *tls_base = __get_tls();
    gl_hooks_t const * volatile * tls_hooks =
            reinterpret_cast<gl_hooks_t const * volatile *>(tls_base);
    return tls_hooks;
}

inline EGLAPI gl_hooks_t const* getGlThreadSpecific() {
    gl_hooks_t const * volatile * tls_hooks = get_tls_hooks();
    gl_hooks_t const* hooks = tls_hooks[TLS_SLOT_OPENGL_API];
    return hooks;
}

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif /* ANDROID_GLES_CM_HOOKS_H */
