#ifndef VP8_COMMON_ATOMIC_H_
#define VP8_COMMON_ATOMIC_H_

#define USE_STD_ATOMIC 1
#undef USE_STD_ATOMIC

#ifdef USE_STD_ATOMIC
#include <stdatomic.h>

inline int vpx_nobarrier_load_32(atomic_int *x) {
  return atomic_load_explicit(x, memory_order_relaxed);
}

inline void vpx_nobarrier_store_32(atomic_int *x, int v) {
  atomic_store_explicit(x, v, memory_order_relaxed);
}

inline int vpx_acquire_load_32(atomic_int *x) {
  return atomic_load_explicit(x, memory_order_acquire);
}

inline void vpx_release_store_32(atomic_int *x, int v) {
  atomic_store_explicit(x, v, memory_order_release);
}

#else
typedef volatile int atomic_int;

#ifndef ARCH_ARM
#define vpx_memory_barrier()
#else
#define vpx_memory_barrier() do { \
  __asm__ __volatile__ ("dmb ish" ::: "memory"); \
  } while (0)
#endif

inline int vpx_nobarrier_load_32(atomic_int *x) {
  return *x;
}

inline void vpx_nobarrier_store_32(atomic_int *x, int v) {
  *x = v;
}

inline int vpx_acquire_load_32(atomic_int *x) {
  int v = *x;
  vpx_memory_barrier();
  return v;
}

inline void vpx_release_store_32(atomic_int *x, int v) {
  vpx_memory_barrier();
  *x = v;
}

#endif  // USE_STD_ATOMIC

#endif  // VP8_COMMON_ATOMIC_H_
