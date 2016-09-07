#ifndef VP8_COMMON_ATOMIC_H_
#define VP8_COMMON_ATOMIC_H_

#include "vpx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) ||              \
    (defined(__cplusplus) && __cplusplus >= 201112L)
// Where available, use <stdatomic.h>
#define USE_STD_ATOMIC 1

#else

#define USE_STD_ATOMIC 0

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__atomic_load_n) ||                                          \
    (defined(__GNUC__) &&                                                      \
     (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40700))
// For GCC >= 4.7 and Clang that support __atomic builtins, use those.
#define USE_ATOMIC_BUILTINS 1
#else
#define USE_ATOMIC_BUILTINS 0
#endif

#endif // no stdatomic.h

#if USE_STD_ATOMIC
#include <stdatomic.h>

static inline int vpx_relaxed_load_32(volatile atomic_int *x) {
  return atomic_load_explicit(x, memory_order_relaxed);
}

static inline void vpx_relaxed_store_32(volatile atomic_int *x, int v) {
  atomic_store_explicit(x, v, memory_order_relaxed);
}

static inline int vpx_acquire_load_32(volatile atomic_int *x) {
  return atomic_load_explicit(x, memory_order_acquire);
}

static inline void vpx_release_store_32(volatile atomic_int *x, int v) {
  atomic_store_explicit(x, v, memory_order_release);
}

#elif defined(USE_ATOMIC_BUILTINS)
// Use the __atomic builtins provided by GCC and Clang.

typedef int atomic_int;

static inline int vpx_relaxed_load_32(volatile atomic_int *x) {
  return __atomic_load_n(x, __ATOMIC_RELAXED);
}

static inline void vpx_relaxed_store_32(volatile atomic_int *x, int v) {
  __atomic_store_n(x, v, __ATOMIC_RELAXED);
}

static inline int vpx_acquire_load_32(volatile atomic_int *x) {
  return __atomic_load_n(x, __ATOMIC_ACQUIRE);
}

static inline void vpx_release_store_32(volatile atomic_int *x, int v) {
  __atomic_store_n(x, v, __ATOMIC_RELEASE);
}
#else

// stdatomic.h is unavailable, emulate atomics using volatile integer accesses
// and memory barriers.
// Note: from the C11/C++11 point of view these operations are still non-atomic,
// and using them causes undefined behavior. We however assume that older
// compilers are unlikely to exploit this behavior and break the code using
// these operations. Newer compilers are supposed to use stdatomic.h instead.

typedef int atomic_int;

#if defined(_WIN32)
#if ARCH_X86 || ARCH_X86_64
// Use a compiler barrier on x86, no runtime penalty.
#include <intrin.h>
#define vpx_memory_barrier() _ReadWriteBarrier()
#else
// Use a real memory barrier on architectures with weaker memory model.
#include <winnt.h>
#define vpx_memory_barrier() MemoryBarrier()
#endif

#else // defined(_WIN32)

#if ARCH_X86 || ARCH_X86_64
// Use a compiler barrier on x86, no runtime penalty.
#define vpx_memory_barrier()                                                   \
  do {                                                                         \
    __asm__ __volatile__("" ::: "memory");                                     \
  } while (0)
#elif ARCH_ARM
#define vpx_memory_barrier()                                                   \
  do {                                                                         \
    __asm__ __volatile__("dmb ish" ::: "memory");                              \
  } while (0)
#elif ARCH_MIPS
#define vpx_memory_barrier()                                                   \
  do {                                                                         \
    __asm__ __volatile__("sync" : : : "memory");                               \
  } while (0)
#else
#error Unsupported architecture!
#endif
#endif // defined(_WIN32)

static inline int vpx_relaxed_load_32(volatile atomic_int *x) { return *x; }

static inline void vpx_relaxed_store_32(volatile atomic_int *x, int v) {
  *x = v;
}

static inline int vpx_acquire_load_32(volatile atomic_int *x) {
  int v = *x;
  vpx_memory_barrier();
  return v;
}

static inline void vpx_release_store_32(volatile atomic_int *x, int v) {
  vpx_memory_barrier();
  *x = v;
}

#undef vpx_memory_barrier

#endif // USE_STD_ATOMIC

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VP8_COMMON_ATOMIC_H_
