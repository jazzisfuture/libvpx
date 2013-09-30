#include <stdint.h>

unsigned int vp9_satd_c(const uint8_t *src_ptr,
                        int  src_stride,
                        const uint8_t *ref_ptr,
                        int  ref_stride,
                        int width,
                        int height);
