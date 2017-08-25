/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_UTIL_VPX_ATOMICS_H_
#define VPX_UTIL_VPX_ATOMICS_H_

#include "./vpx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_OS_SUPPORT && CONFIG_MULTITHREAD

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || \
    (defined(__cplusplus) && __cplusplus >= 201112L)
// Where available, use <stdatomic.h>
#define VPX_USE_STD_ATOMIC 1
#include <stdatomic.h>
#else
#define VPX_USE_STD_ATOMIC 0
#if (defined(__has_builtin) && __has_builtin(__atomic_load_n)) || \
    (defined(__GNUC__) &&             \
     (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40700))
// For GCC >= 4.7 and Clang that support __atomic builtins, use those.
#define VPX_USE_ATOMIC_BUILTINS 1
#else
#define VPX_USE_ATOMIC_BUILTINS 0
#if defined(_WIN32)
// TODO(pbos): Is _WIN32 + not supporting stdatomic + not supporting __atomic
// builtins enough of a check for MSVC and volatile int being acquire/release?

// MSVC volatile int requires no barrier, as volatile ints already have
// acquire/release semantics (assuming early MSVC versions or /volatile:ms,
// which is the default). It's assumed that later versions of MSVC support
// stdatomic.h.
#define vpx_atomics_memory_barrier() do {} while (0)
#else
#if ARCH_X86 || ARCH_X86_64
// Use a compiler barrier on x86, no runtime penalty.
#define vpx_atomics_memory_barrier() \
  __asm__ __volatile__("" ::: "memory")
#elif ARCH_ARM
#define vpx_atomics_memory_barrier() \
  __asm__ __volatile__("dmb ish" ::: "memory")
#elif ARCH_MIPS
#define vpx_atomics_memory_barrier() \
  __asm__ __volatile__("sync" : : : "memory")
#else
#error Unsupported architecture!
#endif
#endif  // !defined(_WIN32)
#endif  // atomic builtin check
#endif  // atomic method

// These are wrapped in a struct so that they are not easily accessed directly.
// This encourages using vpx_atomic_ functions to access them, which is often
// desirable. 
typedef struct vpx_atomic_int {
#if VPX_USE_STD_ATOMIC
volatile atomic_int value;
#else
volatile int value;
#endif
} vpx_atomic_int;

// Initialization of an atomic int, not thread safe.
static INLINE void vpx_atomic_init(vpx_atomic_int* atomic, int value) {
#if VPX_USE_STD_ATOMIC
  atomic_init(&atomic->value, value);
#else
  atomic->value = value;
#endif
};

// Atomic write with release semantics.
static INLINE void vpx_atomic_write(vpx_atomic_int* atomic, int value) {
#if VPX_USE_STD_ATOMIC
  atomic_store_explicit(&atomic->value, value, memory_order_release);
#elif VPX_USE_ATOMIC_BUILTINS
  __atomic_store_n(&atomic->value, value, __ATOMIC_RELEASE);
#else
  vpx_atomics_memory_barrier();
  atomic->value = value;
#endif
};

// TODO(pbos): See if vpx_atomic_int can be made const here without const_cast.
// This is not trivial as atomic_load_explicit(&atomic->value, ...) fails
// (address argument to atomic operation must be a pointer to non-const _Atomic
// type).
// Atomic read with acquire semantics.
static INLINE int vpx_atomic_read(vpx_atomic_int* atomic) {
#if VPX_USE_STD_ATOMIC
  return atomic_load_explicit(&atomic->value, memory_order_acquire);
#elif VPX_USE_ATOMIC_BUILTINS
  return __atomic_load_n(&atomic->value, __ATOMIC_ACQUIRE);
#else
  int v = atomic->value;
  vpx_atomics_memory_barrier();
  return v;
#endif
};

#undef VPX_USE_STD_ATOMIC
#undef VPX_USE_ATOMIC_BUILTINS
#if defined(vpx_atomics_memory_barrier)
#undef vpx_atomics_memory_barrier
#endif  // defined(vpx_atomics_memory_barrier)


#endif /* CONFIG_OS_SUPPORT && CONFIG_MULTITHREAD */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_UTIL_VPX_ATOMICS_H_
