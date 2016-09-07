#ifndef VP8_COMMON_ATOMIC_H_
#define VP8_COMMON_ATOMIC_H_

#include "vpx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || \
    (defined(__cplusplus) && __cplusplus >= 201112L)
#define USE_STD_ATOMIC 1
#else
#undef USE_STD_ATOMIC
#endif

#undef USE_TSAN_ATOMICS
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define USE_TSAN_ATOMICS 1
#endif
#endif

#ifdef USE_STD_ATOMIC
#include <stdatomic.h>

static inline int vpx_nobarrier_load_32(atomic_int *x) {
  return atomic_load_explicit(x, memory_order_relaxed);
}

static inline void vpx_nobarrier_store_32(atomic_int *x, int v) {
  atomic_store_explicit(x, v, memory_order_relaxed);
}

static inline int vpx_acquire_load_32(atomic_int *x) {
  return atomic_load_explicit(x, memory_order_acquire);
}

static inline void vpx_release_store_32(atomic_int *x, int v) {
  atomic_store_explicit(x, v, memory_order_release);
}

#elif defined(USE_TSAN_ATOMICS)
// We're building under ThreadSanitizer, but without stdatomic.h.
// Use TSan-provided atomics implementations.

#include <sanitizer/tsan_interface_atomic.h>

typedef volatile int atomic_int;

static inline int vpx_nobarrier_load_32(atomic_int *x) {
  return __tsan_atomic32_load(x, __tsan_memory_order_relaxed);
}

static inline void vpx_nobarrier_store_32(atomic_int *x, int v) {
  __tsan_atomic32_store(x, v, __tsan_memory_order_relaxed);
}

static inline int vpx_acquire_load_32(atomic_int *x) {
  return __tsan_atomic32_load(x, __tsan_memory_order_acquire);
}

static inline void vpx_release_store_32(atomic_int *x, int v) {
  __tsan_atomic32_store(x, v, __tsan_memory_order_release);
}
#else

// stdatomic.h is unavailable, emulate atomics using volatile integer accesses
// and memory barriers.
// Note: from the C11/C++11 point of view these operations are still non-atomic,
// and using them causes undefined behavior. We however assume that that older
// compilers are unlikely to exploit this behavior and break the code using
// these operations. Newer compilers are supposed to use stdatomic.h instead.

typedef volatile int atomic_int;

#if defined(_WIN32)
#include <winnt.h>
#define vpx_memory_barrier() MemoryBarrier()
#else
#if ARCH_ARM
#define vpx_memory_barrier()                      \
  do {                                            \
    __asm__ __volatile__("dmb ish" ::: "memory"); \
  } while (0)
#elif ARCH_MIPS
#define vpx_memory_barrier()                     \
  do {                                           \
    __asm__ __volatile__("sync" : : : "memory"); \
  } while (0)
#else
// Assuming strong memory model.
#define vpx_memory_barrier()
#endif  // !ARCH_ARM && !ARCH_MIPS
#endif

static inline int vpx_nobarrier_load_32(atomic_int *x) { return *x; }

static inline void vpx_nobarrier_store_32(atomic_int *x, int v) { *x = v; }

static inline int vpx_acquire_load_32(atomic_int *x) {
  int v = *x;
  vpx_memory_barrier();
  return v;
}

static inline void vpx_release_store_32(atomic_int *x, int v) {
  vpx_memory_barrier();
  *x = v;
}

#endif  // USE_STD_ATOMIC

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP8_COMMON_ATOMIC_H_
