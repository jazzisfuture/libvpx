#include "atomic.h"

int vpx_nobarrier_load_32(atomic_int *x);
void vpx_nobarrier_store_32(atomic_int *x, int v);
int vpx_acquire_load_32(atomic_int *x);
void vpx_release_store_32(atomic_int *x, int v);
