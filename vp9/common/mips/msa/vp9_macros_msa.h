/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_MIPS_MSA_VP9_MACROS_MSA_H_
#define VP9_COMMON_MIPS_MSA_VP9_MACROS_MSA_H_

#include <stdint.h>
#include <msa.h>

#include "./vpx_config.h"

#if HAVE_MSA
#define ALIGNMENT           16
#define ALIGNMENT_MINUS_1   (ALIGNMENT - 1)

#define LOAD_UB(psrc) ({              \
  v16u8 out_m;                        \
  out_m = *((const v16u8 *) (psrc));  \
  out_m;                              \
})

#define LOAD_SB(psrc) ({              \
  v16i8 out_m;                        \
  out_m = *((const v16i8 *) (psrc));  \
  out_m;                              \
})

#define LOAD_UH(psrc) ({              \
  v8u16 out_m;                        \
  out_m = *((const v8u16 *) (psrc));  \
  out_m;                              \
})

#define LOAD_SH(psrc) ({              \
  v8i16 out_m;                        \
  out_m = *((const v8i16 *) (psrc));  \
  out_m;                              \
})

#define LOAD_UW(psrc) ({              \
  v4u32 out_m;                        \
  out_m = *((const v4u32 *) (psrc));  \
  out_m;                              \
})

#define LOAD_SW(psrc) ({              \
  v4i32 out_m;                        \
  out_m = *((const v4i32 *) (psrc));  \
  out_m;                              \
})

#define LOAD_UD(psrc) ({              \
  v2u64 out_m;                        \
  out_m = *((const v2u64 *) (psrc));  \
  out_m;                              \
})

#define LOAD_D(psrc) ({               \
  v2i64 out_m;                        \
  out_m = *((const v2i64 *) (psrc));  \
  out_m;                              \
})

#define STORE_UB(vec, pdest) {   \
  *((v16u8 *) (pdest)) = (vec);  \
}

#define STORE_SB(vec, pdest) {   \
  *((v16i8 *) (pdest)) = (vec);  \
}

#define STORE_UH(vec, pdest) {   \
  *((v8u16 *) (pdest)) = (vec);  \
}

#define STORE_SH(vec, pdest) {   \
  *((v8i16 *) (pdest)) = (vec);  \
}

#define STORE_UW(vec, pdest) {   \
  *((v4u32 *) (pdest)) = (vec);  \
}

#define STORE_SW(vec, pdest) {   \
  *((v4i32 *) (pdest)) = (vec);  \
}

#define STORE_UD(vec, pdest) {   \
  *((v2u64 *) (pdest)) = (vec);  \
}

#define STORE_SD(vec, pdest) {   \
  *((v2i64 *) (pdest)) = (vec);  \
}

#define ALLOC_ALIGNED(align) __attribute__ ((aligned((align) << 1)))

#if (__mips_isa_rev >= 6)
  #define LOAD_HWORD(psrc) ({                         \
    const uint8_t *src_m = (const uint8_t *) (psrc);  \
    uint16_t val_m;                                   \
                                                      \
    __asm__ __volatile__ (                            \
        "lh  %[val_m],  %[src_m]  \n\t"               \
                                                      \
        : [val_m] "=r" (val_m)                        \
        : [src_m] "m" (*src_m)                        \
    );                                                \
                                                      \
    val_m;                                            \
  })

  #define LOAD_WORD(psrc) ({                          \
    const uint8_t *src_m = (const uint8_t *) (psrc);  \
    uint32_t val_m;                                   \
                                                      \
    __asm__ __volatile__ (                            \
        "lw  %[val_m],  %[src_m]  \n\t"               \
                                                      \
        : [val_m] "=r" (val_m)                        \
        : [src_m] "m" (*src_m)                        \
    );                                                \
                                                      \
    val_m;                                            \
  })

  #if (__mips == 64)
    #define LOAD_DWORD(psrc) ({                         \
      const uint8_t *src_m = (const uint8_t *) (psrc);  \
      uint64_t val_m = 0;                               \
                                                        \
      __asm__ __volatile__ (                            \
          "ld  %[val_m],  %[src_m]  \n\t"               \
                                                        \
          : [val_m] "=r" (val_m)                        \
          : [src_m] "m" (*src_m)                        \
      );                                                \
                                                        \
      val_m;                                            \
    })
  #else
    #define LOAD_DWORD(psrc) ({                                       \
      const uint8_t *src1_m = (const uint8_t *) (psrc);               \
      const uint8_t *src2_m = ((const uint8_t *) (psrc)) + 4;         \
      uint32_t val0_m, val1_m;                                        \
      uint64_t genval_m = 0;                                          \
                                                                      \
      __asm__ __volatile__ (                                          \
          "lw  %[val0_m],  %[src1_m]  \n\t"                           \
                                                                      \
          : [val0_m] "=r" (val0_m)                                    \
          : [src1_m] "m" (*src1_m)                                    \
      );                                                              \
                                                                      \
      __asm__ __volatile__ (                                          \
          "lw  %[val1_m],  %[src2_m]  \n\t"                           \
                                                                      \
          : [val1_m] "=r" (val1_m)                                    \
          : [src2_m] "m" (*src2_m)                                    \
      );                                                              \
                                                                      \
      genval_m = (uint64_t) (val1_m);                                 \
      genval_m = (uint64_t) ((genval_m << 32) & 0xFFFFFFFF00000000);  \
      genval_m = (uint64_t) (genval_m | (uint64_t) val0_m);           \
                                                                      \
      genval_m;                                                       \
    })
  #endif

  #define STORE_WORD_WITH_OFFSET_1(pdst, val) {     \
    uint8_t *dst_ptr_m = ((uint8_t *) (pdst)) + 1;  \
    uint32_t val_m = (val);                         \
                                                    \
    __asm__ __volatile__ (                          \
        "sw  %[val_m],  %[dst_ptr_m]  \n\t"         \
                                                    \
        : [dst_ptr_m] "=m" (*dst_ptr_m)             \
        : [val_m] "r" (val_m)                       \
    );                                              \
  }

  #define STORE_WORD(pdst, val) {             \
    uint8_t *dst_ptr_m = (uint8_t *) (pdst);  \
    uint32_t val_m = (val);                   \
                                              \
    __asm__ __volatile__ (                    \
        "sw  %[val_m],  %[dst_ptr_m]  \n\t"   \
                                              \
        : [dst_ptr_m] "=m" (*dst_ptr_m)       \
        : [val_m] "r" (val_m)                 \
    );                                        \
  }

  #define STORE_DWORD(pdst, val) {            \
    uint8_t *dst_ptr_m = (uint8_t *) (pdst);  \
    uint64_t val_m = (val);                   \
                                              \
    __asm__ __volatile__ (                    \
        "sd  %[val_m],  %[dst_ptr_m]  \n\t"   \
                                              \
        : [dst_ptr_m] "=m" (*dst_ptr_m)       \
        : [val_m] "r" (val_m)                 \
    );                                        \
  }

  #define STORE_HWORD(pdst, val) {            \
    uint8_t *dst_ptr_m = (uint8_t *) (pdst);  \
    uint16_t val_m = (val);                   \
                                              \
    __asm__ __volatile__ (                    \
        "sh  %[val_m],  %[dst_ptr_m]  \n\t"   \
                                              \
        : [dst_ptr_m] "=m" (*dst_ptr_m)       \
        : [val_m] "r" (val_m)                 \
    );                                        \
  }

  #define STORE_HWORD_WITH_OFFSET_5(pdst, val) {    \
    uint8_t *dst_ptr_m = ((uint8_t *) (pdst)) + 5;  \
    uint16_t val_m = (val);                         \
                                                    \
    __asm__ __volatile__ (                          \
        "sh  %[val_m],  %[dst_ptr_m]  \n\t"         \
                                                    \
        : [dst_ptr_m] "=m" (*dst_ptr_m)             \
        : [val_m] "r" (val_m)                       \
    );                                              \
  }
#else
  #define LOAD_HWORD(psrc) ({                         \
    const uint8_t *src_m = (const uint8_t *) (psrc);  \
    uint16_t val_m;                                   \
                                                      \
    __asm__ __volatile__ (                            \
        "ulh  %[val_m],  %[src_m]  \n\t"              \
                                                      \
        : [val_m] "=r" (val_m)                        \
        : [src_m] "m" (*src_m)                        \
    );                                                \
                                                      \
    val_m;                                            \
  })

  #define LOAD_WORD(psrc) ({                          \
    const uint8_t *src_m = (const uint8_t *) (psrc);  \
    uint32_t val_m;                                   \
                                                      \
    __asm__ __volatile__ (                            \
        "ulw  %[val_m],  %[src_m]  \n\t"              \
                                                      \
        : [val_m] "=r" (val_m)                        \
        : [src_m] "m" (*src_m)                        \
    );                                                \
                                                      \
    val_m;                                            \
  })

  #if (__mips == 64)
    #define LOAD_DWORD(psrc) ({                         \
      const uint8_t *src_m = (const uint8_t *) (psrc);  \
      uint64_t val_m = 0;                               \
                                                        \
      __asm__ __volatile__ (                            \
          "uld  %[val_m],  %[src_m]  \n\t"              \
                                                        \
          : [val_m] "=r" (val_m)                        \
          : [src_m] "m" (*src_m)                        \
      );                                                \
                                                        \
      val_m;                                            \
    })
  #else
    #define LOAD_DWORD(psrc) ({                                       \
      const uint8_t *src1_m = (const uint8_t *) (psrc);               \
      const uint8_t *src2_m = ((const uint8_t *) (psrc)) + 4;         \
      uint32_t val0_m, val1_m;                                        \
      uint64_t genval_m = 0;                                          \
                                                                      \
      __asm__ __volatile__ (                                          \
          "ulw  %[val0_m],  %[src1_m]  \n\t"                          \
                                                                      \
          : [val0_m] "=r" (val0_m)                                    \
          : [src1_m] "m" (*src1_m)                                    \
      );                                                              \
                                                                      \
      __asm__ __volatile__ (                                          \
          "ulw  %[val1_m],  %[src2_m]  \n\t"                          \
                                                                      \
          : [val1_m] "=r" (val1_m)                                    \
          : [src2_m] "m" (*src2_m)                                    \
      );                                                              \
                                                                      \
      genval_m = (uint64_t) (val1_m);                                 \
      genval_m = (uint64_t) ((genval_m << 32) & 0xFFFFFFFF00000000);  \
      genval_m = (uint64_t) (genval_m | (uint64_t) val0_m);           \
                                                                      \
      genval_m;                                                       \
    })
  #endif

  #define STORE_WORD_WITH_OFFSET_1(pdst, val) {     \
    uint8_t *dst_ptr_m = ((uint8_t *) (pdst)) + 1;  \
    uint32_t val_m = (val);                         \
                                                    \
    __asm__ __volatile__ (                          \
        "usw  %[val_m],  %[dst_ptr_m]  \n\t"        \
                                                    \
        : [dst_ptr_m] "=m" (*dst_ptr_m)             \
        : [val_m] "r" (val_m)                       \
    );                                              \
  }

  #define STORE_WORD(pdst, val) {             \
    uint8_t *dst_ptr_m = (uint8_t *) (pdst);  \
    uint32_t val_m = (val);                   \
                                              \
    __asm__ __volatile__ (                    \
        "usw  %[val_m],  %[dst_ptr_m]  \n\t"  \
                                              \
        : [dst_ptr_m] "=m" (*dst_ptr_m)       \
        : [val_m] "r" (val_m)                 \
    );                                        \
  }

  #define STORE_DWORD(pdst, val) {                             \
    uint8_t *dst1_m = (uint8_t *) (pdst);                      \
    uint8_t *dst2_m = ((uint8_t *) (pdst)) + 4;                \
    uint32_t val0_m, val1_m;                                   \
                                                               \
    val0_m = (uint32_t) ((val) & 0x00000000FFFFFFFF);          \
    val1_m = (uint32_t) (((val) >> 32) & 0x00000000FFFFFFFF);  \
                                                               \
    __asm__ __volatile__ (                                     \
        "usw  %[val0_m],  %[dst1_m]  \n\t"                     \
        "usw  %[val1_m],  %[dst2_m]  \n\t"                     \
                                                               \
        : [dst1_m] "=m" (*dst1_m), [dst2_m] "=m" (*dst2_m)     \
        : [val0_m] "r" (val0_m), [val1_m] "r" (val1_m)         \
    );                                                         \
  }

  #define STORE_HWORD(pdst, val) {            \
    uint8_t *dst_ptr_m = (uint8_t *) (pdst);  \
    uint16_t val_m = (val);                   \
                                              \
    __asm__ __volatile__ (                    \
        "ush  %[val_m],  %[dst_ptr_m]  \n\t"  \
                                              \
        : [dst_ptr_m] "=m" (*dst_ptr_m)       \
        : [val_m] "r" (val_m)                 \
    );                                        \
  }

  #define STORE_HWORD_WITH_OFFSET_5(pdst, val) {    \
    uint8_t *dst_ptr_m = ((uint8_t *) (pdst)) + 5;  \
    uint16_t val_m = (val);                         \
                                                    \
    __asm__ __volatile__ (                          \
        "ush  %[val_m],  %[dst_ptr_m]  \n\t"        \
                                                    \
        : [dst_ptr_m] "=m" (*dst_ptr_m)             \
        : [val_m] "r" (val_m)                       \
    );                                              \
  }
#endif

#define LOAD_4WORDS_WITH_STRIDE(psrc, src_stride,          \
                                src0, src1, src2, src3) {  \
  src0 = LOAD_WORD(psrc + 0 * src_stride);                 \
  src1 = LOAD_WORD(psrc + 1 * src_stride);                 \
  src2 = LOAD_WORD(psrc + 2 * src_stride);                 \
  src3 = LOAD_WORD(psrc + 3 * src_stride);                 \
}

#define LOAD_2DWORDS_WITH_STRIDE(psrc, src_stride,  \
                                 src0, src1) {      \
  src0 = LOAD_DWORD(psrc + 0 * src_stride);         \
  src1 = LOAD_DWORD(psrc + 1 * src_stride);         \
}

#define LOAD_2VECS_UB(psrc, stride,   \
                      val0, val1) {   \
  val0 = LOAD_UB(psrc + 0 * stride);  \
  val1 = LOAD_UB(psrc + 1 * stride);  \
}

#define LOAD_4VECS_UB(psrc, stride,              \
                      val0, val1, val2, val3) {  \
  val0 = LOAD_UB(psrc + 0 * stride);             \
  val1 = LOAD_UB(psrc + 1 * stride);             \
  val2 = LOAD_UB(psrc + 2 * stride);             \
  val3 = LOAD_UB(psrc + 3 * stride);             \
}

#define LOAD_4VECS_SB(psrc, stride,              \
                      val0, val1, val2, val3) {  \
  val0 = LOAD_SB(psrc + 0 * stride);             \
  val1 = LOAD_SB(psrc + 1 * stride);             \
  val2 = LOAD_SB(psrc + 2 * stride);             \
  val3 = LOAD_SB(psrc + 3 * stride);             \
}

#define LOAD_5VECS_UB(psrc, stride,                    \
                      out0, out1, out2, out3, out4) {  \
  LOAD_4VECS_UB((psrc), (stride),                      \
                (out0), (out1), (out2), (out3));       \
  out4 = LOAD_UB(psrc + 4 * stride);                   \
}

#define LOAD_5VECS_SB(psrc, stride,                    \
                      out0, out1, out2, out3, out4) {  \
  LOAD_4VECS_SB((psrc), (stride),                      \
                (out0), (out1), (out2), (out3));       \
  out4 = LOAD_SB(psrc + 4 * stride);                   \
}

#define LOAD_7VECS_SB(psrc, stride,            \
                      val0, val1, val2, val3,  \
                      val4, val5, val6) {      \
  val0 = LOAD_SB((psrc) + 0 * (stride));       \
  val1 = LOAD_SB((psrc) + 1 * (stride));       \
  val2 = LOAD_SB((psrc) + 2 * (stride));       \
  val3 = LOAD_SB((psrc) + 3 * (stride));       \
  val4 = LOAD_SB((psrc) + 4 * (stride));       \
  val5 = LOAD_SB((psrc) + 5 * (stride));       \
  val6 = LOAD_SB((psrc) + 6 * (stride));       \
}

#define LOAD_8VECS_UB(psrc, stride,               \
                      out0, out1, out2, out3,     \
                      out4, out5, out6, out7) {   \
  LOAD_4VECS_UB((psrc), (stride),                 \
                (out0), (out1), (out2), (out3));  \
  LOAD_4VECS_UB((psrc + 4 * stride), (stride),    \
                (out4), (out5), (out6), (out7));  \
}

#define LOAD_8VECS_SB(psrc, stride,               \
                      out0, out1, out2, out3,     \
                      out4, out5, out6, out7) {   \
  LOAD_4VECS_SB((psrc), (stride),                 \
                (out0), (out1), (out2), (out3));  \
  LOAD_4VECS_SB((psrc + 4 * stride), (stride),    \
                (out4), (out5), (out6), (out7));  \
}

#define LOAD_4VECS_INC_UB(psrc, stride,              \
                          out0, out1, out2, out3) {  \
  out0 = LOAD_UB(psrc);                              \
  psrc += (stride);                                  \
  out1 = LOAD_UB(psrc);                              \
  psrc += (stride);                                  \
  out2 = LOAD_UB(psrc);                              \
  psrc += (stride);                                  \
  out3 = LOAD_UB(psrc);                              \
}

#define LOAD_8VECS_INC_UB(psrc, stride,               \
                          out0, out1, out2, out3,     \
                          out4, out5, out6, out7) {   \
  LOAD_4VECS_INC_UB((psrc), (stride),                 \
                    (out0), (out1), (out2), (out3));  \
  (psrc) += (stride);                                 \
                                                      \
  LOAD_4VECS_INC_UB((psrc), (stride),                 \
                    (out4), (out5), (out6), (out7));  \
}

#define LOAD_2VECS_SH(psrc, stride,       \
                      val0, val1) {       \
  val0 = LOAD_SH((psrc) + 0 * (stride));  \
  val1 = LOAD_SH((psrc) + 1 * (stride));  \
}

#define LOAD_4VECS_SH(psrc, stride,                          \
                      val0, val1, val2, val3) {              \
  LOAD_2VECS_SH((psrc), (stride), val0, val1);               \
  LOAD_2VECS_SH((psrc + 2 * stride), (stride), val2, val3);  \
}

#define LOAD_8VECS_SH(psrc, stride,              \
                      val0, val1, val2, val3,    \
                      val4, val5, val6, val7) {  \
  LOAD_4VECS_SH((psrc), (stride),                \
                val0, val1, val2, val3);         \
  LOAD_4VECS_SH((psrc + 4 * stride), (stride),   \
                val4, val5, val6, val7);         \
}

#define LOAD_16VECS_SH(psrc, stride,                  \
                       val0, val1, val2, val3,        \
                       val4, val5, val6, val7,        \
                       val8, val9, val10, val11,      \
                       val12, val13, val14, val15) {  \
  LOAD_8VECS_SH((psrc), (stride),                     \
                val0, val1, val2, val3,               \
                val4, val5, val6, val7);              \
  LOAD_8VECS_SH((psrc + 8 * (stride)), (stride),      \
                val8, val9, val10, val11,             \
                val12, val13, val14, val15);          \
}

#define STORE_4VECS_UB(dst_out, pitch,         \
                       in0, in1, in2, in3) {   \
  STORE_UB((in0), (dst_out));                  \
  STORE_UB((in1), ((dst_out) + (pitch)));      \
  STORE_UB((in2), ((dst_out) + 2 * (pitch)));  \
  STORE_UB((in3), ((dst_out) + 3 * (pitch)));  \
}

#define STORE_8VECS_UB(dst_out, pitch_in,               \
                       in0, in1, in2, in3,              \
                       in4, in5, in6, in7) {            \
  STORE_4VECS_UB(dst_out, pitch_in,                     \
                 in0, in1, in2, in3);                   \
  STORE_4VECS_UB((dst_out + 4 * (pitch_in)), pitch_in,  \
                 in4, in5, in6, in7);                   \
}

#define STORE_4VECS_SH(ptr, stride,           \
                       in0, in1, in2, in3) {  \
  STORE_SH(in0, ((ptr) + 0 * stride));        \
  STORE_SH(in1, ((ptr) + 1 * stride));        \
  STORE_SH(in2, ((ptr) + 2 * stride));        \
  STORE_SH(in3, ((ptr) + 3 * stride));        \
}

#define STORE_8VECS_SH(ptr, stride,           \
                       in0, in1, in2, in3,    \
                       in4, in5, in6, in7) {  \
  STORE_SH(in0, ((ptr) + 0 * stride));        \
  STORE_SH(in1, ((ptr) + 1 * stride));        \
  STORE_SH(in2, ((ptr) + 2 * stride));        \
  STORE_SH(in3, ((ptr) + 3 * stride));        \
  STORE_SH(in4, ((ptr) + 4 * stride));        \
  STORE_SH(in5, ((ptr) + 5 * stride));        \
  STORE_SH(in6, ((ptr) + 6 * stride));        \
  STORE_SH(in7, ((ptr) + 7 * stride));        \
}

#define CLIP_UNSIGNED_CHAR_H(in) ({                     \
  v8i16 max_m = __msa_ldi_h(255);                       \
  v8i16 out_m;                                          \
                                                        \
  out_m = __msa_maxi_s_h((v8i16) (in), 0);              \
  out_m = __msa_min_s_h((v8i16) max_m, (v8i16) out_m);  \
  out_m;                                                \
})

#define CALC_ADDITIVE_SUM(result) ({                              \
  v2i64 result_m, result_dup_m;                                   \
  int32_t sum_m;                                                  \
                                                                  \
  result_m = __msa_hadd_s_d((v4i32) (result), (v4i32) (result));  \
  result_dup_m = __msa_splati_d(result_m, 1);                     \
  result_m = result_m + result_dup_m;                             \
  sum_m = __msa_copy_s_w((v4i32) result_m, 0);                    \
  sum_m;                                                          \
})

#define CALC_ADDITIVE_SUM_H(sad) ({                      \
  v4u32 sad_m;                                           \
  uint32_t sad_out_m;                                    \
                                                         \
  sad_m = __msa_hadd_u_w((v8u16) (sad), (v8u16) (sad));  \
  sad_out_m = (uint32_t) CALC_ADDITIVE_SUM(sad_m);       \
  sad_out_m;                                             \
})

#define CALC_MSE_B(src, ref, var) {                                    \
  v16u8 src_l0_m, src_l1_m;                                            \
  v8i16 res_l0_m, res_l1_m;                                            \
                                                                       \
  src_l0_m = (v16u8) __msa_ilvr_b((v16i8) (src), (v16i8) (ref));       \
  src_l1_m = (v16u8) __msa_ilvl_b((v16i8) (src), (v16i8) (ref));       \
                                                                       \
  res_l0_m = __msa_hsub_u_h(src_l0_m, src_l0_m);                       \
  res_l1_m = __msa_hsub_u_h(src_l1_m, src_l1_m);                       \
                                                                       \
  (var) = (v4i32) __msa_dpadd_s_w((v4i32) (var), res_l0_m, res_l0_m);  \
  (var) = (v4i32) __msa_dpadd_s_w((v4i32) (var), res_l1_m, res_l1_m);  \
}

#define CALC_MSE_AVG_B(src, ref, var, sub) {                           \
  v16u8 src_l0_m, src_l1_m;                                            \
  v8i16 res_l0_m, res_l1_m;                                            \
                                                                       \
  src_l0_m = (v16u8) __msa_ilvr_b((v16i8) (src), (v16i8) (ref));       \
  src_l1_m = (v16u8) __msa_ilvl_b((v16i8) (src), (v16i8) (ref));       \
                                                                       \
  res_l0_m = __msa_hsub_u_h(src_l0_m, src_l0_m);                       \
  res_l1_m = __msa_hsub_u_h(src_l1_m, src_l1_m);                       \
                                                                       \
  (var) = (v4i32) __msa_dpadd_s_w((v4i32) (var), res_l0_m, res_l0_m);  \
  (var) = (v4i32) __msa_dpadd_s_w((v4i32) (var), res_l1_m, res_l1_m);  \
                                                                       \
  (sub) += res_l0_m + res_l1_m;                                        \
}

#define VARIANCE_WxH(sse, diff, shift) ({                     \
  uint32_t var_m;                                             \
                                                              \
  var_m = (sse) - (((uint32_t) (diff) * (diff)) >> (shift));  \
                                                              \
  var_m;                                                      \
})

#define VARIANCE_LARGE_WxH(sse, diff, shift) ({              \
  uint32_t var_m;                                            \
                                                             \
  var_m = (sse) - (((int64_t) (diff) * (diff)) >> (shift));  \
                                                             \
  var_m;                                                     \
})

#define VEC_INSERT_4W_UB(src,                              \
                         src0, src1, src2, src3) {         \
  src = (v16u8) __msa_insert_w((v4i32) (src), 0, (src0));  \
  src = (v16u8) __msa_insert_w((v4i32) (src), 1, (src1));  \
  src = (v16u8) __msa_insert_w((v4i32) (src), 2, (src2));  \
  src = (v16u8) __msa_insert_w((v4i32) (src), 3, (src3));  \
}

#define VEC_INSERT_2DW_UB(src, src0, src1) {               \
  src = (v16u8) __msa_insert_d((v2i64) (src), 0, (src0));  \
  src = (v16u8) __msa_insert_d((v2i64) (src), 1, (src1));  \
}

#define TRANSPOSE8x8_B_UB(in0, in1, in2, in3,                   \
                          in4, in5, in6, in7,                   \
                          out0, out1, out2, out3,               \
                          out4, out5, out6, out7) {             \
  v16i8 tmp0_m, tmp1_m, tmp2_m, tmp3_m;                         \
  v16i8 tmp4_m, tmp5_m, tmp6_m, tmp7_m;                         \
  v16i8 zero_m = { 0 };                                         \
                                                                \
  tmp0_m = __msa_ilvr_b((v16i8) (in2), (v16i8) (in0));          \
  tmp1_m = __msa_ilvr_b((v16i8) (in3), (v16i8) (in1));          \
  tmp2_m = __msa_ilvr_b((v16i8) (in6), (v16i8) (in4));          \
  tmp3_m = __msa_ilvr_b((v16i8) (in7), (v16i8) (in5));          \
                                                                \
  tmp4_m = __msa_ilvr_b((v16i8) tmp1_m, (v16i8) tmp0_m);        \
  tmp5_m = __msa_ilvl_b((v16i8) tmp1_m, (v16i8) tmp0_m);        \
  tmp6_m = __msa_ilvr_b((v16i8) tmp3_m, (v16i8) tmp2_m);        \
  tmp7_m = __msa_ilvl_b((v16i8) tmp3_m, (v16i8) tmp2_m);        \
                                                                \
  out0 = (v16u8) __msa_ilvr_w((v4i32) tmp6_m, (v4i32) tmp4_m);  \
  out2 = (v16u8) __msa_ilvl_w((v4i32) tmp6_m, (v4i32) tmp4_m);  \
  out4 = (v16u8) __msa_ilvr_w((v4i32) tmp7_m, (v4i32) tmp5_m);  \
  out6 = (v16u8) __msa_ilvl_w((v4i32) tmp7_m, (v4i32) tmp5_m);  \
                                                                \
  out1 = (v16u8) __msa_sldi_b(zero_m, (v16i8) out0, 8);         \
  out3 = (v16u8) __msa_sldi_b(zero_m, (v16i8) out2, 8);         \
  out5 = (v16u8) __msa_sldi_b(zero_m, (v16i8) out4, 8);         \
  out7 = (v16u8) __msa_sldi_b(zero_m, (v16i8) out6, 8);         \
}

/* transpose 16x8 matrix into 8x16 */
#define TRANSPOSE16x8_B_UB(in0, in1, in2, in3,                     \
                           in4, in5, in6, in7,                     \
                           in8, in9, in10, in11,                   \
                           in12, in13, in14, in15,                 \
                           out0, out1, out2, out3,                 \
                           out4, out5, out6, out7) {               \
  v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m;                            \
  v16u8 tmp4_m, tmp5_m, tmp6_m, tmp7_m;                            \
                                                                   \
  (out7) = (v16u8) __msa_ilvev_d((v2i64) (in8), (v2i64) (in0));    \
  (out6) = (v16u8) __msa_ilvev_d((v2i64) (in9), (v2i64) (in1));    \
  (out5) = (v16u8) __msa_ilvev_d((v2i64) (in10), (v2i64) (in2));   \
  (out4) = (v16u8) __msa_ilvev_d((v2i64) (in11), (v2i64) (in3));   \
  (out3) = (v16u8) __msa_ilvev_d((v2i64) (in12), (v2i64) (in4));   \
  (out2) = (v16u8) __msa_ilvev_d((v2i64) (in13), (v2i64) (in5));   \
  (out1) = (v16u8) __msa_ilvev_d((v2i64) (in14), (v2i64) (in6));   \
  (out0) = (v16u8) __msa_ilvev_d((v2i64) (in15), (v2i64) (in7));   \
                                                                   \
  tmp0_m = (v16u8) __msa_ilvev_b((v16i8) (out6), (v16i8) (out7));  \
  tmp4_m = (v16u8) __msa_ilvod_b((v16i8) (out6), (v16i8) (out7));  \
  tmp1_m = (v16u8) __msa_ilvev_b((v16i8) (out4), (v16i8) (out5));  \
  tmp5_m = (v16u8) __msa_ilvod_b((v16i8) (out4), (v16i8) (out5));  \
  (out5) = (v16u8) __msa_ilvev_b((v16i8) (out2), (v16i8) (out3));  \
  tmp6_m = (v16u8) __msa_ilvod_b((v16i8) (out2), (v16i8) (out3));  \
  (out7) = (v16u8) __msa_ilvev_b((v16i8) (out0), (v16i8) (out1));  \
  tmp7_m = (v16u8) __msa_ilvod_b((v16i8) (out0), (v16i8) (out1));  \
                                                                   \
  tmp2_m = (v16u8) __msa_ilvev_h((v8i16) tmp1_m, (v8i16) tmp0_m);  \
  tmp3_m = (v16u8) __msa_ilvev_h((v8i16) (out7), (v8i16) (out5));  \
  (out0) = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
  (out4) = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
                                                                   \
  tmp2_m = (v16u8) __msa_ilvod_h((v8i16) tmp1_m, (v8i16) tmp0_m);  \
  tmp3_m = (v16u8) __msa_ilvod_h((v8i16) (out7), (v8i16) (out5));  \
  (out2) = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
  (out6) = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
                                                                   \
  tmp2_m = (v16u8) __msa_ilvev_h((v8i16) tmp5_m, (v8i16) tmp4_m);  \
  tmp3_m = (v16u8) __msa_ilvev_h((v8i16) tmp7_m, (v8i16) tmp6_m);  \
  (out1) = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
  (out5) = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
                                                                   \
  tmp2_m = (v16u8) __msa_ilvod_h((v8i16) tmp5_m, (v8i16) tmp4_m);  \
  tmp2_m = (v16u8) __msa_ilvod_h((v8i16) tmp5_m, (v8i16) tmp4_m);  \
  tmp3_m = (v16u8) __msa_ilvod_h((v8i16) tmp7_m, (v8i16) tmp6_m);  \
  tmp3_m = (v16u8) __msa_ilvod_h((v8i16) tmp7_m, (v8i16) tmp6_m);  \
  (out3) = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
  (out7) = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m);  \
}

/* halfword transpose macro */
#define TRANSPOSE4x4_H(in0, in1, in2, in3,                  \
                       out0, out1, out2, out3) {            \
  v8i16 s0_m, s1_m;                                         \
                                                            \
  s0_m = __msa_ilvr_h((v8i16) (in1), (v8i16) (in0));        \
  s1_m = __msa_ilvr_h((v8i16) (in3), (v8i16) (in2));        \
                                                            \
  out0 = (v8i16) __msa_ilvr_w((v4i32) s1_m, (v4i32) s0_m);  \
  out1 = (v8i16) __msa_ilvl_d((v2i64) out0, (v2i64) out0);  \
  out2 = (v8i16) __msa_ilvl_w((v4i32) s1_m, (v4i32) s0_m);  \
  out3 = (v8i16) __msa_ilvl_d((v2i64) out0, (v2i64) out2);  \
}

#define TRANSPOSE4X8_H(in0, in1, in2, in3,                      \
                       in4, in5, in6, in7,                      \
                       out0, out1, out2, out3,                  \
                       out4, out5, out6, out7) {                \
  v8i16 tmp0_m, tmp1_m, tmp2_m, tmp3_m;                         \
  v8i16 tmp0_n, tmp1_n, tmp2_n, tmp3_n;                         \
  v8i16 zero_m = { 0 };                                         \
                                                                \
  tmp0_n = __msa_ilvr_h((v8i16) (in1), (v8i16) (in0));          \
  tmp1_n = __msa_ilvr_h((v8i16) (in3), (v8i16) (in2));          \
  tmp2_n = __msa_ilvr_h((v8i16) (in5), (v8i16) (in4));          \
  tmp3_n = __msa_ilvr_h((v8i16) (in7), (v8i16) (in6));          \
                                                                \
  ILV_W_LRLR_SH((tmp0_n), (tmp1_n), (tmp2_n), (tmp3_n),         \
                tmp2_m, tmp0_m, tmp3_m, tmp1_m);                \
                                                                \
  out1 = (v8i16) __msa_ilvl_d((v2i64) tmp1_m, (v2i64) tmp0_m);  \
  out0 = (v8i16) __msa_ilvr_d((v2i64) tmp1_m, (v2i64) tmp0_m);  \
  out3 = (v8i16) __msa_ilvl_d((v2i64) tmp3_m, (v2i64) tmp2_m);  \
  out2 = (v8i16) __msa_ilvr_d((v2i64) tmp3_m, (v2i64) tmp2_m);  \
                                                                \
  out4 = zero_m;                                                \
  out5 = zero_m;                                                \
  out6 = zero_m;                                                \
  out7 = zero_m;                                                \
}

#define TRANSPOSE8X4_H(in0, in1, in2, in3,        \
                       out0, out1, out2, out3) {  \
  v8i16 tmp0_m, tmp1_m, tmp2_m, tmp3_m;           \
                                                  \
  ILV_H_LRLR_SH((in0), (in1), (in2), (in3),       \
                tmp2_m, tmp0_m, tmp3_m, tmp1_m);  \
                                                  \
  ILV_W_LRLR_SH(tmp0_m, tmp1_m, tmp2_m, tmp3_m,   \
                out1, out0, out3, out2);          \
}

/* halfword 8x8 transpose macro */
#define TRANSPOSE8x8_H_SH(in0, in1, in2, in3,                    \
                          in4, in5, in6, in7,                    \
                          out0, out1, out2, out3,                \
                          out4, out5, out6, out7) {              \
  v8i16 s0_m, s1_m;                                              \
  v8i16 tmp0_m, tmp1_m, tmp2_m, tmp3_m;                          \
  v8i16 tmp4_m, tmp5_m, tmp6_m, tmp7_m;                          \
                                                                 \
  s0_m = __msa_ilvr_h((v8i16) (in6), (v8i16) (in4));             \
  s1_m = __msa_ilvr_h((v8i16) (in7), (v8i16) (in5));             \
  tmp0_m = __msa_ilvr_h((v8i16) s1_m, (v8i16) s0_m);             \
  tmp1_m = __msa_ilvl_h((v8i16) s1_m, (v8i16) s0_m);             \
                                                                 \
  s0_m = __msa_ilvl_h((v8i16) (in6), (v8i16) (in4));             \
  s1_m = __msa_ilvl_h((v8i16) (in7), (v8i16) (in5));             \
  tmp2_m = __msa_ilvr_h((v8i16) s1_m, (v8i16) s0_m);             \
  tmp3_m = __msa_ilvl_h((v8i16) s1_m, (v8i16) s0_m);             \
                                                                 \
  s0_m = __msa_ilvr_h((v8i16) (in2), (v8i16) (in0));             \
  s1_m = __msa_ilvr_h((v8i16) (in3), (v8i16) (in1));             \
  tmp4_m = __msa_ilvr_h((v8i16) s1_m, (v8i16) s0_m);             \
  tmp5_m = __msa_ilvl_h((v8i16) s1_m, (v8i16) s0_m);             \
                                                                 \
  s0_m = __msa_ilvl_h((v8i16) (in2), (v8i16) (in0));             \
  s1_m = __msa_ilvl_h((v8i16) (in3), (v8i16) (in1));             \
  tmp6_m = __msa_ilvr_h((v8i16) s1_m, (v8i16) s0_m);             \
  tmp7_m = __msa_ilvl_h((v8i16) s1_m, (v8i16) s0_m);             \
                                                                 \
  out0 = (v8i16) __msa_pckev_d((v2i64) tmp0_m, (v2i64) tmp4_m);  \
  out1 = (v8i16) __msa_pckod_d((v2i64) tmp0_m, (v2i64) tmp4_m);  \
  out2 = (v8i16) __msa_pckev_d((v2i64) tmp1_m, (v2i64) tmp5_m);  \
  out3 = (v8i16) __msa_pckod_d((v2i64) tmp1_m, (v2i64) tmp5_m);  \
  out4 = (v8i16) __msa_pckev_d((v2i64) tmp2_m, (v2i64) tmp6_m);  \
  out5 = (v8i16) __msa_pckod_d((v2i64) tmp2_m, (v2i64) tmp6_m);  \
  out6 = (v8i16) __msa_pckev_d((v2i64) tmp3_m, (v2i64) tmp7_m);  \
  out7 = (v8i16) __msa_pckod_d((v2i64) tmp3_m, (v2i64) tmp7_m);  \
}

/* word transpose macro */
#define TRANSPOSE4x4_W(in0, in1, in2, in3,                  \
                       out0, out1, out2, out3) {            \
  v4i32 s0_m, s1_m, s2_m, s3_m;                             \
                                                            \
  s0_m = __msa_ilvr_w((v4i32) (in1), (v4i32) (in0));        \
  s1_m = __msa_ilvl_w((v4i32) (in1), (v4i32) (in0));        \
  s2_m = __msa_ilvr_w((v4i32) (in3), (v4i32) (in2));        \
  s3_m = __msa_ilvl_w((v4i32) (in3), (v4i32) (in2));        \
                                                            \
  out0 = (v4i32) __msa_ilvr_d((v2i64) s2_m, (v2i64) s0_m);  \
  out1 = (v4i32) __msa_ilvl_d((v2i64) s2_m, (v2i64) s0_m);  \
  out2 = (v4i32) __msa_ilvr_d((v2i64) s3_m, (v2i64) s1_m);  \
  out3 = (v4i32) __msa_ilvl_d((v2i64) s3_m, (v2i64) s1_m);  \
}

#define LPF_MASK_HEV(p3_in, p2_in, p1_in, p0_in,                 \
                     q0_in, q1_in, q2_in, q3_in,                 \
                     limit_in, b_limit_in, thresh_in,            \
                     hev_out, mask_out, flat_out) {              \
  v16u8 p3_asub_p2_m, p2_asub_p1_m, p1_asub_p0_m, q1_asub_q0_m;  \
  v16u8 p1_asub_q1_m, p0_asub_q0_m, q3_asub_q2_m, q2_asub_q1_m;  \
                                                                 \
  /* absolute subtraction of pixel values */                     \
  p3_asub_p2_m = __msa_asub_u_b((p3_in), (p2_in));               \
  p2_asub_p1_m = __msa_asub_u_b((p2_in), (p1_in));               \
  p1_asub_p0_m = __msa_asub_u_b((p1_in), (p0_in));               \
  q1_asub_q0_m = __msa_asub_u_b((q1_in), (q0_in));               \
  q2_asub_q1_m = __msa_asub_u_b((q2_in), (q1_in));               \
  q3_asub_q2_m = __msa_asub_u_b((q3_in), (q2_in));               \
  p0_asub_q0_m = __msa_asub_u_b((p0_in), (q0_in));               \
  p1_asub_q1_m = __msa_asub_u_b((p1_in), (q1_in));               \
                                                                 \
  /* calculation of hev */                                       \
  flat_out = __msa_max_u_b(p1_asub_p0_m, q1_asub_q0_m);          \
  hev_out = (thresh_in) < (v16u8) flat_out;                      \
                                                                 \
  /* calculation of mask */                                      \
  p0_asub_q0_m = __msa_adds_u_b(p0_asub_q0_m, p0_asub_q0_m);     \
  p1_asub_q1_m >>= 1;                                            \
  p0_asub_q0_m = __msa_adds_u_b(p0_asub_q0_m, p1_asub_q1_m);     \
                                                                 \
  mask_out = (b_limit_in) < p0_asub_q0_m;                        \
  mask_out = __msa_max_u_b(flat_out, mask_out);                  \
  p3_asub_p2_m = __msa_max_u_b(p3_asub_p2_m, p2_asub_p1_m);      \
  mask_out = __msa_max_u_b(p3_asub_p2_m, mask_out);              \
  q2_asub_q1_m = __msa_max_u_b(q2_asub_q1_m, q3_asub_q2_m);      \
  mask_out = __msa_max_u_b(q2_asub_q1_m, mask_out);              \
                                                                 \
  mask_out = (limit_in) < (v16u8) mask_out;                      \
  mask_out = __msa_xori_b(mask_out, 0xff);                       \
}

#define LPF_FILTER4(p1_in_out, p0_in_out, q0_in_out, q1_in_out,             \
                    mask_in, hev_in) {                                      \
  v8i16 q0_sub_p0_r_m, q0_sub_p0_l_m;                                       \
  v8i16 filter_val_l_m, filter_val_r_m, short3_m;                           \
  v16i8 filt_val_sign_bit_m;                                                \
  v16i8 q0_sub_p0_sign_bit_m, q0_sub_p0_m;                                  \
  v16i8 byte4_m, byte3_m;                                                   \
  v16i8 filt_val_m, filt1_m, filt2_m;                                       \
  v16i8 p1_m, p0_m, q0_m, q1_m;                                             \
                                                                            \
  p1_m = (v16i8) __msa_xori_b((p1_in_out), 0x80);                           \
  p0_m = (v16i8) __msa_xori_b((p0_in_out), 0x80);                           \
  q0_m = (v16i8) __msa_xori_b((q0_in_out), 0x80);                           \
  q1_m = (v16i8) __msa_xori_b((q1_in_out), 0x80);                           \
                                                                            \
  filt_val_m = __msa_subs_s_b(p1_m, q1_m);                                  \
                                                                            \
  filt_val_m = filt_val_m & (v16i8) (hev_in);                               \
                                                                            \
  q0_sub_p0_m = q0_m - p0_m;                                                \
  q0_sub_p0_sign_bit_m = __msa_clti_s_b(q0_sub_p0_m, 0);                    \
  filt_val_sign_bit_m = __msa_clti_s_b(filt_val_m, 0);                      \
                                                                            \
  short3_m = __msa_ldi_h(3);                                                \
  q0_sub_p0_r_m = (v8i16) __msa_ilvr_b(q0_sub_p0_sign_bit_m, q0_sub_p0_m);  \
  q0_sub_p0_r_m *= short3_m;                                                \
  filter_val_r_m = (v8i16) __msa_ilvr_b(filt_val_sign_bit_m, filt_val_m);   \
  filter_val_r_m += q0_sub_p0_r_m;                                          \
  filter_val_r_m = __msa_sat_s_h(filter_val_r_m, 7);                        \
                                                                            \
  q0_sub_p0_l_m = (v8i16) __msa_ilvl_b(q0_sub_p0_sign_bit_m, q0_sub_p0_m);  \
  q0_sub_p0_l_m *= short3_m;                                                \
  filter_val_l_m = (v8i16) __msa_ilvl_b(filt_val_sign_bit_m, filt_val_m);   \
  filter_val_l_m += q0_sub_p0_l_m;                                          \
  filter_val_l_m = __msa_sat_s_h(filter_val_l_m, 7);                        \
                                                                            \
  filt_val_m = __msa_pckev_b((v16i8) filter_val_l_m,                        \
                             (v16i8) filter_val_r_m);                       \
                                                                            \
  filt_val_m = filt_val_m & (v16i8) (mask_in);                              \
                                                                            \
  byte4_m = __msa_ldi_b(4);                                                 \
  filt1_m = __msa_adds_s_b(filt_val_m, byte4_m);                            \
  filt1_m >>= 3;                                                            \
                                                                            \
  byte3_m = __msa_ldi_b(3);                                                 \
  filt2_m = __msa_adds_s_b(filt_val_m, byte3_m);                            \
  filt2_m >>= 3;                                                            \
                                                                            \
  q0_m = __msa_subs_s_b(q0_m, filt1_m);                                     \
                                                                            \
  (q0_in_out) = __msa_xori_b((v16u8) q0_m, 0x80);                           \
                                                                            \
  p0_m = __msa_adds_s_b(p0_m, filt2_m);                                     \
  (p0_in_out) = __msa_xori_b((v16u8) p0_m, 0x80);                           \
                                                                            \
  filt_val_m = __msa_srari_b(filt1_m, 1);                                   \
                                                                            \
  (hev_in) = __msa_xori_b((v16u8) (hev_in), 0xff);                          \
  filt_val_m = filt_val_m & (v16i8) (hev_in);                               \
                                                                            \
  q1_m = __msa_subs_s_b(q1_m, filt_val_m);                                  \
  (q1_in_out) = __msa_xori_b((v16u8) q1_m, 0x80);                           \
                                                                            \
  p1_m = __msa_adds_s_b(p1_m, filt_val_m);                                  \
  (p1_in_out) = __msa_xori_b((v16u8) p1_m, 0x80);                           \
}

/* interleave macros */
/* no in-place support */
#define ILV_B_LRLR_UB(in0, in1, in2, in3,                     \
                      out0, out1, out2, out3) {               \
  out0 = (v16u8) __msa_ilvl_b((v16i8) (in1), (v16i8) (in0));  \
  out1 = (v16u8) __msa_ilvr_b((v16i8) (in1), (v16i8) (in0));  \
  out2 = (v16u8) __msa_ilvl_b((v16i8) (in3), (v16i8) (in2));  \
  out3 = (v16u8) __msa_ilvr_b((v16i8) (in3), (v16i8) (in2));  \
}

#define ILV_B_LRLR_UH(in0, in1, in2, in3,                     \
                      out0, out1, out2, out3) {               \
  out0 = (v8u16) __msa_ilvl_b((v16i8) (in1), (v16i8) (in0));  \
  out1 = (v8u16) __msa_ilvr_b((v16i8) (in1), (v16i8) (in0));  \
  out2 = (v8u16) __msa_ilvl_b((v16i8) (in3), (v16i8) (in2));  \
  out3 = (v8u16) __msa_ilvr_b((v16i8) (in3), (v16i8) (in2));  \
}

#define ILV_H_LRLR_SH(in0, in1, in2, in3,             \
                      out0, out1, out2, out3) {       \
  out0 = __msa_ilvl_h((v8i16) (in1), (v8i16) (in0));  \
  out1 = __msa_ilvr_h((v8i16) (in1), (v8i16) (in0));  \
  out2 = __msa_ilvl_h((v8i16) (in3), (v8i16) (in2));  \
  out3 = __msa_ilvr_h((v8i16) (in3), (v8i16) (in2));  \
}

#define ILV_W_LRLR_SH(in0, in1, in2, in3,                     \
                      out0, out1, out2, out3) {               \
  out0 = (v8i16) __msa_ilvl_w((v4i32) (in1), (v4i32) (in0));  \
  out1 = (v8i16) __msa_ilvr_w((v4i32) (in1), (v4i32) (in0));  \
  out2 = (v8i16) __msa_ilvl_w((v4i32) (in3), (v4i32) (in2));  \
  out3 = (v8i16) __msa_ilvr_w((v4i32) (in3), (v4i32) (in2));  \
}

/* no in-place support */
#define ILV_H_LR_SH(in0, in1, out0, out1) {           \
  out0 = __msa_ilvl_h((v8i16) (in1), (v8i16) (in0));  \
  out1 = __msa_ilvr_h((v8i16) (in1), (v8i16) (in0));  \
}

#define ILV_H_LR_SW(in0, in1, out0, out1) {                   \
  out0 = (v4i32) __msa_ilvl_h((v8i16) (in1), (v8i16) (in0));  \
  out1 = (v4i32) __msa_ilvr_h((v8i16) (in1), (v8i16) (in0));  \
}

#define ILVR_B_2VECS_UB(in0_r, in1_r, in0_l, in1_l,               \
                        out0, out1) {                             \
  out0 = (v16u8) __msa_ilvr_b((v16i8) (in0_l), (v16i8) (in0_r));  \
  out1 = (v16u8) __msa_ilvr_b((v16i8) (in1_l), (v16i8) (in1_r));  \
}

#define ILVR_B_2VECS_SB(in0_r, in1_r, in0_l, in1_l,       \
                        out0, out1) {                     \
  out0 = __msa_ilvr_b((v16i8) (in0_l), (v16i8) (in0_r));  \
  out1 = __msa_ilvr_b((v16i8) (in1_l), (v16i8) (in1_r));  \
}

#define ILVR_B_4VECS_UB(in0_r, in1_r, in2_r, in3_r,  \
                        in0_l, in1_l, in2_l, in3_l,  \
                        out0, out1, out2, out3) {    \
  ILVR_B_2VECS_UB(in0_r, in1_r, in0_l, in1_l,        \
                  out0, out1);                       \
  ILVR_B_2VECS_UB(in2_r, in3_r, in2_l, in3_l,        \
                  out2, out3);                       \
}

#define ILVR_B_4VECS_SB(in0_r, in1_r, in2_r, in3_r,  \
                        in0_l, in1_l, in2_l, in3_l,  \
                        out0, out1, out2, out3) {    \
  ILVR_B_2VECS_SB(in0_r, in1_r, in0_l, in1_l,        \
                  out0, out1);                       \
  ILVR_B_2VECS_SB(in2_r, in3_r, in2_l, in3_l,        \
                  out2, out3);                       \
}

#define ILVR_B_6VECS_SB(in0_r, in1_r, in2_r,     \
                        in3_r, in4_r, in5_r,     \
                        in0_l, in1_l, in2_l,     \
                        in3_l, in4_l, in5_l,     \
                        out0, out1, out2,        \
                        out3, out4, out5) {      \
  ILVR_B_2VECS_SB(in0_r, in1_r, in0_l, in1_l,    \
                  out0, out1);                   \
  ILVR_B_2VECS_SB(in2_r, in3_r, in2_l, in3_l,    \
                  out2, out3);                   \
  ILVR_B_2VECS_SB(in4_r, in5_r, in4_l, in5_l,    \
                  out4, out5);                   \
}

#define ILVR_B_8VECS_SB(in0_r, in1_r, in2_r, in3_r,  \
                        in4_r, in5_r, in6_r, in7_r,  \
                        in0_l, in1_l, in2_l, in3_l,  \
                        in4_l, in5_l, in6_l, in7_l,  \
                        out0, out1, out2, out3,      \
                        out4, out5, out6, out7) {    \
  ILVR_B_2VECS_SB(in0_r, in1_r, in0_l, in1_l,        \
                  out0, out1);                       \
  ILVR_B_2VECS_SB(in2_r, in3_r, in2_l, in3_l,        \
                  out2, out3);                       \
  ILVR_B_2VECS_SB(in4_r, in5_r, in4_l, in5_l,        \
                  out4, out5);                       \
  ILVR_B_2VECS_SB(in6_r, in7_r, in6_l, in7_l,        \
                  out6, out7);                       \
}

#define ILVR_B_2VECS_UH(in0_r, in1_r, in0_l, in1_l,               \
                        out0, out1) {                             \
  out0 = (v8u16) __msa_ilvr_b((v16i8) (in0_l), (v16i8) (in0_r));  \
  out1 = (v8u16) __msa_ilvr_b((v16i8) (in1_l), (v16i8) (in1_r));  \
}

#define ILVR_B_4VECS_UH(in0_r, in1_r, in2_r, in3_r,  \
                        in0_l, in1_l, in2_l, in3_l,  \
                        out0, out1, out2, out3) {    \
  ILVR_B_2VECS_UH(in0_r, in1_r, in0_l, in1_l,        \
                  out0, out1);                       \
  ILVR_B_2VECS_UH(in2_r, in3_r, in2_l, in3_l,        \
                  out2, out3);                       \
}

#define ILVR_B_8VECS_UH(in0_r, in1_r, in2_r, in3_r,  \
                        in4_r, in5_r, in6_r, in7_r,  \
                        in0_l, in1_l, in2_l, in3_l,  \
                        in4_l, in5_l, in6_l, in7_l,  \
                        out0, out1, out2, out3,      \
                        out4, out5, out6, out7) {    \
  ILVR_B_2VECS_UH(in0_r, in1_r, in0_l, in1_l,        \
                  out0, out1);                       \
  ILVR_B_2VECS_UH(in2_r, in3_r, in2_l, in3_l,        \
                  out2, out3);                       \
  ILVR_B_2VECS_UH(in4_r, in5_r, in4_l, in5_l,        \
                  out4, out5);                       \
  ILVR_B_2VECS_UH(in6_r, in7_r, in6_l, in7_l,        \
                  out6, out7);                       \
}

#define ILVL_B_2VECS_SB(in0_r, in1_r, in0_l, in1_l,       \
                        out0, out1) {                     \
  out0 = __msa_ilvl_b((v16i8) (in0_l), (v16i8) (in0_r));  \
  out1 = __msa_ilvl_b((v16i8) (in1_l), (v16i8) (in1_r));  \
}

#define ILVL_B_4VECS_SB(in0_r, in1_r, in2_r, in3_r,  \
                        in0_l, in1_l, in2_l, in3_l,  \
                        out0, out1, out2, out3) {    \
  ILVL_B_2VECS_SB(in0_r, in1_r, in0_l, in1_l,        \
                  out0, out1);                       \
  ILVL_B_2VECS_SB(in2_r, in3_r, in2_l, in3_l,        \
                  out2, out3);                       \
}

#define ILVL_B_6VECS_SB(in0_r, in1_r, in2_r,   \
                        in3_r, in4_r, in5_r,   \
                        in0_l, in1_l, in2_l,   \
                        in3_l, in4_l, in5_l,   \
                        out0, out1, out2,      \
                        out3, out4, out5) {    \
  ILVL_B_2VECS_SB(in0_r, in1_r, in0_l, in1_l,  \
                  out0, out1);                 \
  ILVL_B_2VECS_SB(in2_r, in3_r, in2_l, in3_l,  \
                  out2, out3);                 \
  ILVL_B_2VECS_SB(in4_r, in5_r, in4_l, in5_l,  \
                  out4, out5);                 \
}

#define ILVL_B_2VECS_UH(in0_r, in1_r, in0_l, in1_l,               \
                        out0, out1) {                             \
  out0 = (v8u16) __msa_ilvl_b((v16i8) (in0_l), (v16i8) (in0_r));  \
  out1 = (v8u16) __msa_ilvl_b((v16i8) (in1_l), (v16i8) (in1_r));  \
}

#define ILVL_B_8VECS_UH(in0_r, in1_r, in2_r, in3_r,  \
                        in4_r, in5_r, in6_r, in7_r,  \
                        in0_l, in1_l, in2_l, in3_l,  \
                        in4_l, in5_l, in6_l, in7_l,  \
                        out0, out1, out2, out3,      \
                        out4, out5, out6, out7) {    \
  ILVL_B_2VECS_UH(in0_r, in1_r, in0_l, in1_l,        \
                  out0, out1);                       \
  ILVL_B_2VECS_UH(in2_r, in3_r, in2_l, in3_l,        \
                  out2, out3);                       \
  ILVL_B_2VECS_UH(in4_r, in5_r, in4_l, in5_l,        \
                  out4, out5);                       \
  ILVL_B_2VECS_UH(in6_r, in7_r, in6_l, in7_l,        \
                  out6, out7);                       \
}

#define ILVR_D_2VECS_UB(out0, in0_l, in0_r,                       \
                        out1, in1_l, in1_r) {                     \
  out0 = (v16u8) __msa_ilvr_d((v2i64) (in0_l), (v2i64) (in0_r));  \
  out1 = (v16u8) __msa_ilvr_d((v2i64) (in1_l), (v2i64) (in1_r));  \
}

#define ILVR_D_2VECS_SB(out0, in0_l, in0_r,                       \
                        out1, in1_l, in1_r) {                     \
  out0 = (v16i8) __msa_ilvr_d((v2i64) (in0_l), (v2i64) (in0_r));  \
  out1 = (v16i8) __msa_ilvr_d((v2i64) (in1_l), (v2i64) (in1_r));  \
}

#define ILVR_D_3VECS_SB(out0, in0_l, in0_r,                       \
                        out1, in1_l, in1_r,                       \
                        out2, in2_l, in2_r) {                     \
  ILVR_D_2VECS_SB(out0, in0_l, in0_r,                             \
                  out1, in1_l, in1_r);                            \
  out2 = (v16i8) __msa_ilvr_d((v2i64) (in2_l), (v2i64) (in2_r));  \
}

#define ILVR_D_4VECS_SB(out0, in0_l, in0_r,    \
                        out1, in1_l, in1_r,    \
                        out2, in2_l, in2_r,    \
                        out3, in3_l, in3_r) {  \
  ILVR_D_2VECS_SB(out0, in0_l, in0_r,          \
                  out1, in1_l, in1_r);         \
  ILVR_D_2VECS_SB(out2, in2_l, in2_r,          \
                  out3, in3_l, in3_r);         \
}

#define DOTP_S_W_4VECS_SW(m0, c0, m1, c1,             \
                          m2, c2, m3, c3,             \
                          out0, out1, out2, out3) {   \
  out0 = __msa_dotp_s_w((v8i16) (m0), (v8i16) (c0));  \
  out1 = __msa_dotp_s_w((v8i16) (m1), (v8i16) (c1));  \
  out2 = __msa_dotp_s_w((v8i16) (m2), (v8i16) (c2));  \
  out3 = __msa_dotp_s_w((v8i16) (m3), (v8i16) (c3));  \
}

#define SPLATI_H_4VECS_SH(coeff,                     \
                          val0, val1, val2, val3,    \
                          out0, out1, out2, out3) {  \
  out0 = __msa_splati_h((v8i16) (coeff), (val0));    \
  out1 = __msa_splati_h((v8i16) (coeff), (val1));    \
  out2 = __msa_splati_h((v8i16) (coeff), (val2));    \
  out3 = __msa_splati_h((v8i16) (coeff), (val3));    \
}

#define PCKEV_H_2VECS_SH(in0_l, in0_r, in1_l, in1_r,       \
                         out0, out1) {                     \
  out0 = __msa_pckev_h((v8i16) (in0_l), (v8i16) (in0_r));  \
  out1 = __msa_pckev_h((v8i16) (in1_l), (v8i16) (in1_r));  \
}

#define PCKEV_H_2VECS_SW(in0_l, in0_r, in1_l, in1_r,               \
                         out0, out1) {                             \
  out0 = (v4i32) __msa_pckev_h((v8i16) (in0_l), (v8i16) (in0_r));  \
  out1 = (v4i32) __msa_pckev_h((v8i16) (in1_l), (v8i16) (in1_r));  \
}

#define PCKEV_H_4VECS_SH(in0_l, in0_r, in1_l, in1_r,       \
                         in2_l, in2_r, in3_l, in3_r,       \
                         out0, out1, out2, out3) {         \
  out0 = __msa_pckev_h((v8i16) (in0_l), (v8i16) (in0_r));  \
  out1 = __msa_pckev_h((v8i16) (in1_l), (v8i16) (in1_r));  \
  out2 = __msa_pckev_h((v8i16) (in2_l), (v8i16) (in2_r));  \
  out3 = __msa_pckev_h((v8i16) (in3_l), (v8i16) (in3_r));  \
}

#define PCKEV_D_4VECS_UB(in0_l, in0_r, in1_l, in1_r,               \
                         in2_l, in2_r, in3_l, in3_r,               \
                         out0, out1, out2, out3) {                 \
  out0 = (v16u8) __msa_pckev_d((v2i64) (in0_l), (v2i64) (in0_r));  \
  out1 = (v16u8) __msa_pckev_d((v2i64) (in1_l), (v2i64) (in1_r));  \
  out2 = (v16u8) __msa_pckev_d((v2i64) (in2_l), (v2i64) (in2_r));  \
  out3 = (v16u8) __msa_pckev_d((v2i64) (in3_l), (v2i64) (in3_r));  \
}

#define XORI_B_2VECS_UB(val0, val1,                \
                        out0, out1, xor_val) {     \
  out0 = __msa_xori_b((v16u8) (val0), (xor_val));  \
  out1 = __msa_xori_b((v16u8) (val1), (xor_val));  \
}

#define XORI_B_2VECS_SB(val0, val1,                        \
                        out0, out1, xor_val) {             \
  out0 = (v16i8) __msa_xori_b((v16u8) (val0), (xor_val));  \
  out1 = (v16i8) __msa_xori_b((v16u8) (val1), (xor_val));  \
}

#define XORI_B_3VECS_SB(val0, val1, val2,                   \
                        out0, out1, out2,                   \
                        xor_val) {                          \
  XORI_B_2VECS_SB(val0, val1,                               \
                  out0, out1, xor_val);                     \
  out2 = (v16i8) __msa_xori_b((v16u8) (val2), (xor_val));   \
}

#define XORI_B_4VECS_UB(val0, val1, val2, val3,  \
                        out0, out1, out2, out3,  \
                        xor_val) {               \
  XORI_B_2VECS_UB(val0, val1,                    \
                  out0, out1, xor_val);          \
  XORI_B_2VECS_UB(val2, val3,                    \
                  out2, out3, xor_val);          \
}

#define XORI_B_4VECS_SB(val0, val1, val2, val3,  \
                        out0, out1, out2, out3,  \
                        xor_val) {               \
  XORI_B_2VECS_SB(val0, val1,                    \
                  out0, out1, xor_val);          \
  XORI_B_2VECS_SB(val2, val3,                    \
                  out2, out3, xor_val);          \
}

#define XORI_B_7VECS_SB(val0, val1, val2, val3,      \
                        val4, val5, val6,            \
                        out0, out1, out2, out3,      \
                        out4, out5, out6,            \
                        xor_val) {                   \
  XORI_B_4VECS_SB(val0, val1, val2, val3,            \
                  out0, out1, out2, out3, xor_val);  \
  XORI_B_3VECS_SB(val4, val5, val6,                  \
                  out4, out5, out6, xor_val);        \
}

#define AVE_S_H_4VECS_SH(in0, in1, in2, in3,           \
                         in4, in5, in6, in7,           \
                         out0, out1, out2, out3) {     \
  out0 = __msa_ave_s_h((v8i16) (in0), (v8i16) (in1));  \
  out1 = __msa_ave_s_h((v8i16) (in2), (v8i16) (in3));  \
  out2 = __msa_ave_s_h((v8i16) (in4), (v8i16) (in5));  \
  out3 = __msa_ave_s_h((v8i16) (in6), (v8i16) (in7));  \
}

#define SLLI_4VECS(val0, val1, val2, val3,  \
                   out0, out1, out2, out3,  \
                   shift_left_val) {        \
  out0 = (val0) << (shift_left_val);        \
  out1 = (val1) << (shift_left_val);        \
  out2 = (val2) << (shift_left_val);        \
  out3 = (val3) << (shift_left_val);        \
}

#define SRAI_4VECS(val0, val1, val2, val3,  \
                   out0, out1, out2, out3,  \
                   shift_right_val) {       \
  out0 = (val0) >> (shift_right_val);       \
  out1 = (val1) >> (shift_right_val);       \
  out2 = (val2) >> (shift_right_val);       \
  out3 = (val3) >> (shift_right_val);       \
}

#define SRARI_H_4VECS_UH(val0, val1, val2, val3,                    \
                         out0, out1, out2, out3,                    \
                         shift_right_val) {                         \
  out0 = (v8u16) __msa_srari_h((v8i16) (val0), (shift_right_val));  \
  out1 = (v8u16) __msa_srari_h((v8i16) (val1), (shift_right_val));  \
  out2 = (v8u16) __msa_srari_h((v8i16) (val2), (shift_right_val));  \
  out3 = (v8u16) __msa_srari_h((v8i16) (val3), (shift_right_val));  \
}

#define SRARI_H_4VECS_SH(val0, val1, val2, val3,            \
                         out0, out1, out2, out3,            \
                         shift_right_val) {                 \
  out0 = __msa_srari_h((v8i16) (val0), (shift_right_val));  \
  out1 = __msa_srari_h((v8i16) (val1), (shift_right_val));  \
  out2 = __msa_srari_h((v8i16) (val2), (shift_right_val));  \
  out3 = __msa_srari_h((v8i16) (val3), (shift_right_val));  \
}

#define SRARI_W_4VECS_SH(val0, val1, val2, val3,                    \
                         out0, out1, out2, out3,                    \
                         shift_right_val) {                         \
  out0 = (v8i16) __msa_srari_w((v4i32) (val0), (shift_right_val));  \
  out1 = (v8i16) __msa_srari_w((v4i32) (val1), (shift_right_val));  \
  out2 = (v8i16) __msa_srari_w((v4i32) (val2), (shift_right_val));  \
  out3 = (v8i16) __msa_srari_w((v4i32) (val3), (shift_right_val));  \
}

#define SRARI_W_4VECS_SW(val0, val1, val2, val3,            \
                         out0, out1, out2, out3,            \
                         shift_right_val) {                 \
  out0 = __msa_srari_w((v4i32) (val0), (shift_right_val));  \
  out1 = __msa_srari_w((v4i32) (val1), (shift_right_val));  \
  out2 = __msa_srari_w((v4i32) (val2), (shift_right_val));  \
  out3 = __msa_srari_w((v4i32) (val3), (shift_right_val));  \
}

#define SRLI_H_4VECS_SH(in0, in1, in2, in3,               \
                        out0, out1, out2, out3,           \
                        shift_right_val) {                \
  out0 = __msa_srli_h((v8i16) (in0), (shift_right_val));  \
  out1 = __msa_srli_h((v8i16) (in1), (shift_right_val));  \
  out2 = __msa_srli_h((v8i16) (in2), (shift_right_val));  \
  out3 = __msa_srli_h((v8i16) (in3), (shift_right_val));  \
}

#define SRARI_SATURATE_UNSIGNED_H(input, right_shift_val, sat_val) ({  \
  v8u16 out_m;                                                         \
                                                                       \
  out_m = (v8u16) __msa_srari_h((v8i16) (input), (right_shift_val));   \
  out_m = __msa_sat_u_h(out_m, (sat_val));                             \
  out_m;                                                               \
})

#define SRARI_SATURATE_SIGNED_H(input, right_shift_val, sat_val) ({  \
  v8i16 out_m;                                                       \
                                                                     \
  out_m = __msa_srari_h((v8i16) (input), (right_shift_val));         \
  out_m = __msa_sat_s_h(out_m, (sat_val));                           \
  out_m;                                                             \
})

#define ILVR_SIGNED_H_TO_W_SW(in, out1) {             \
  v8i16 sign_m;                                       \
                                                      \
  sign_m = __msa_clti_s_h((v8i16) (in), 0);           \
  out1 = (v4i32) __msa_ilvr_h(sign_m, (v8i16) (in));  \
}

#define PCKEV_2B_XORI128_STORE_4_BYTES_4(in1, in2,        \
                                         pdst, stride) {  \
  uint32_t out0_m, out1_m, out2_m, out3_m;                \
  v16i8 tmp0_m;                                           \
  uint8_t *dst_m = (uint8_t *) (pdst);                    \
                                                          \
  tmp0_m = __msa_pckev_b((v16i8) (in2), (v16i8) (in1));   \
  tmp0_m = (v16i8) __msa_xori_b((v16u8) tmp0_m, 128);     \
                                                          \
  out0_m = __msa_copy_u_w((v4i32) tmp0_m, 0);             \
  out1_m = __msa_copy_u_w((v4i32) tmp0_m, 1);             \
  out2_m = __msa_copy_u_w((v4i32) tmp0_m, 2);             \
  out3_m = __msa_copy_u_w((v4i32) tmp0_m, 3);             \
                                                          \
  STORE_WORD(dst_m, out0_m);                              \
  dst_m += stride;                                        \
  STORE_WORD(dst_m, out1_m);                              \
  dst_m += stride;                                        \
  STORE_WORD(dst_m, out2_m);                              \
  dst_m += stride;                                        \
  STORE_WORD(dst_m, out3_m);                              \
}

#define PCKEV_B_4_XORI128_STORE_8_BYTES_4(in1, in2,        \
                                          in3, in4,        \
                                          pdst, stride) {  \
  uint64_t out0_m, out1_m, out2_m, out3_m;                 \
  v16i8 tmp0_m, tmp1_m;                                    \
  uint8_t *dst_m = (uint8_t *) (pdst);                     \
                                                           \
  tmp0_m = __msa_pckev_b((v16i8) (in2), (v16i8) (in1));    \
  tmp1_m = __msa_pckev_b((v16i8) (in4), (v16i8) (in3));    \
                                                           \
  tmp0_m = (v16i8) __msa_xori_b((v16u8) tmp0_m, 128);      \
  tmp1_m = (v16i8) __msa_xori_b((v16u8) tmp1_m, 128);      \
                                                           \
  out0_m = __msa_copy_u_d((v2i64) tmp0_m, 0);              \
  out1_m = __msa_copy_u_d((v2i64) tmp0_m, 1);              \
  out2_m = __msa_copy_u_d((v2i64) tmp1_m, 0);              \
  out3_m = __msa_copy_u_d((v2i64) tmp1_m, 1);              \
                                                           \
  STORE_DWORD(dst_m, out0_m);                              \
  dst_m += stride;                                         \
  STORE_DWORD(dst_m, out1_m);                              \
  dst_m += stride;                                         \
  STORE_DWORD(dst_m, out2_m);                              \
  dst_m += stride;                                         \
  STORE_DWORD(dst_m, out3_m);                              \
}

/* Only for signed vecs */
#define PCKEV_B_XORI128_STORE_VEC(in1, in2, pdest) {    \
  v16i8 tmp_m;                                          \
                                                        \
  tmp_m = __msa_pckev_b((v16i8) (in1), (v16i8) (in2));  \
  tmp_m = (v16i8) __msa_xori_b((v16u8) tmp_m, 128);     \
  STORE_SB(tmp_m, (pdest));                             \
}

/* Only for signed vecs */
#define PCKEV_B_4_XORI128_AVG_STORE_8_BYTES_4(in1, dst0,          \
                                              in2, dst1,          \
                                              in3, dst2,          \
                                              in4, dst3,          \
                                              pdst, stride) {     \
  uint64_t out0_m, out1_m, out2_m, out3_m;                        \
  v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m;                           \
  uint8_t *dst_m = (uint8_t *) (pdst);                            \
                                                                  \
  tmp0_m = (v16u8) __msa_pckev_b((v16i8) (in2), (v16i8) (in1));   \
  tmp1_m = (v16u8) __msa_pckev_b((v16i8) (in4), (v16i8) (in3));   \
                                                                  \
  tmp2_m = (v16u8) __msa_ilvr_d((v2i64) (dst1), (v2i64) (dst0));  \
  tmp3_m = (v16u8) __msa_ilvr_d((v2i64) (dst3), (v2i64) (dst2));  \
                                                                  \
  tmp0_m = __msa_xori_b(tmp0_m, 128);                             \
  tmp1_m = __msa_xori_b(tmp1_m, 128);                             \
                                                                  \
  tmp0_m = __msa_aver_u_b(tmp0_m, tmp2_m);                        \
  tmp1_m = __msa_aver_u_b(tmp1_m, tmp3_m);                        \
                                                                  \
  out0_m = __msa_copy_u_d((v2i64) tmp0_m, 0);                     \
  out1_m = __msa_copy_u_d((v2i64) tmp0_m, 1);                     \
  out2_m = __msa_copy_u_d((v2i64) tmp1_m, 0);                     \
  out3_m = __msa_copy_u_d((v2i64) tmp1_m, 1);                     \
                                                                  \
  STORE_DWORD(dst_m, out0_m);                                     \
  dst_m += stride;                                                \
  STORE_DWORD(dst_m, out1_m);                                     \
  dst_m += stride;                                                \
  STORE_DWORD(dst_m, out2_m);                                     \
  dst_m += stride;                                                \
  STORE_DWORD(dst_m, out3_m);                                     \
}

/* Only for signed vecs */
#define PCKEV_B_XORI128_AVG_STORE_VEC(in1, in2, dst, pdest) {   \
  v16u8 tmp_m;                                                  \
                                                                \
  tmp_m = (v16u8) __msa_pckev_b((v16i8) (in1), (v16i8) (in2));  \
  tmp_m = __msa_xori_b(tmp_m, 128);                             \
  tmp_m = __msa_aver_u_b(tmp_m, (v16u8) (dst));                 \
  STORE_UB(tmp_m, (pdest));                                     \
}

#define PCKEV_B_STORE_8_BYTES_4(in1, in2, in3, in4,      \
                                pdst, stride) {          \
  uint64_t out0_m, out1_m, out2_m, out3_m;               \
  v16i8 tmp0_m, tmp1_m;                                  \
  uint8_t *dst_m = (uint8_t *) (pdst);                   \
                                                         \
  tmp0_m = __msa_pckev_b((v16i8) (in2), (v16i8) (in1));  \
  tmp1_m = __msa_pckev_b((v16i8) (in4), (v16i8) (in3));  \
                                                         \
  out0_m = __msa_copy_u_d((v2i64) tmp0_m, 0);            \
  out1_m = __msa_copy_u_d((v2i64) tmp0_m, 1);            \
  out2_m = __msa_copy_u_d((v2i64) tmp1_m, 0);            \
  out3_m = __msa_copy_u_d((v2i64) tmp1_m, 1);            \
                                                         \
  STORE_DWORD(dst_m, out0_m);                            \
  dst_m += stride;                                       \
  STORE_DWORD(dst_m, out1_m);                            \
  dst_m += stride;                                       \
  STORE_DWORD(dst_m, out2_m);                            \
  dst_m += stride;                                       \
  STORE_DWORD(dst_m, out3_m);                            \
}

/* Only for unsigned vecs */
#define PCKEV_B_STORE_VEC(in1, in2, pdest) {            \
  v16i8 tmp_m;                                          \
                                                        \
  tmp_m = __msa_pckev_b((v16i8) (in1), (v16i8) (in2));  \
  STORE_SB(tmp_m, (pdest));                             \
}

#define PCKEV_B_AVG_STORE_8_BYTES_4(in1, dst0, in2, dst1,          \
                                    in3, dst2, in4, dst3,          \
                                    pdst, stride) {                \
  uint64_t out0_m, out1_m, out2_m, out3_m;                         \
  v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m;                            \
  uint8_t *dst_m = (uint8_t *) (pdst);                             \
                                                                   \
  tmp0_m = (v16u8) __msa_pckev_b((v16i8) (in2), (v16i8) (in1));    \
  tmp1_m = (v16u8) __msa_pckev_b((v16i8) (in4), (v16i8) (in3));    \
                                                                   \
  tmp2_m = (v16u8) __msa_pckev_d((v2i64) (dst1), (v2i64) (dst0));  \
  tmp3_m = (v16u8) __msa_pckev_d((v2i64) (dst3), (v2i64) (dst2));  \
                                                                   \
  tmp0_m = __msa_aver_u_b(tmp0_m, tmp2_m);                         \
  tmp1_m = __msa_aver_u_b(tmp1_m, tmp3_m);                         \
                                                                   \
  out0_m = __msa_copy_u_d((v2i64) tmp0_m, 0);                      \
  out1_m = __msa_copy_u_d((v2i64) tmp0_m, 1);                      \
  out2_m = __msa_copy_u_d((v2i64) tmp1_m, 0);                      \
  out3_m = __msa_copy_u_d((v2i64) tmp1_m, 1);                      \
                                                                   \
  STORE_DWORD(dst_m, out0_m);                                      \
  dst_m += stride;                                                 \
  STORE_DWORD(dst_m, out1_m);                                      \
  dst_m += stride;                                                 \
  STORE_DWORD(dst_m, out2_m);                                      \
  dst_m += stride;                                                 \
  STORE_DWORD(dst_m, out3_m);                                      \
}

/* Only for unsigned vecs */
#define PCKEV_B_AVG_STORE_VEC(in1, in2, dst, pdest) {           \
  v16u8 tmp_m;                                                  \
                                                                \
  tmp_m = (v16u8) __msa_pckev_b((v16i8) (in1), (v16i8) (in2));  \
  tmp_m = __msa_aver_u_b(tmp_m, (v16u8) (dst));                 \
  STORE_UB(tmp_m, (pdest));                                     \
}

#define UNPCK_SIGNED_H_TO_W(in, out1, out2) {        \
  v8i16 tmp_m;                                       \
                                                     \
  tmp_m = __msa_clti_s_h((v8i16) (in), 0);           \
  out1 = (v4i32) __msa_ilvr_h(tmp_m, (v8i16) (in));  \
  out2 = (v4i32) __msa_ilvl_h(tmp_m, (v8i16) (in));  \
}

/* Generic for Vector types and GP operations */
#define BUTTERFLY_4(in0, in1, in2, in3,        \
                    out0, out1, out2, out3) {  \
  out0 = (in0) + (in3);                        \
  out1 = (in1) + (in2);                        \
                                               \
  out2 = (in1) - (in2);                        \
  out3 = (in0) - (in3);                        \
}

/* Generic for Vector types and GP operations */
#define BUTTERFLY_8(in0, in1, in2, in3,        \
                    in4, in5, in6, in7,        \
                    out0, out1, out2, out3,    \
                    out4, out5, out6, out7) {  \
  out0 = (in0) + (in7);                        \
  out1 = (in1) + (in6);                        \
  out2 = (in2) + (in5);                        \
  out3 = (in3) + (in4);                        \
                                               \
  out4 = (in3) - (in4);                        \
  out5 = (in2) - (in5);                        \
  out6 = (in1) - (in6);                        \
  out7 = (in0) - (in7);                        \
}

/* Generic for Vector types and GP operations */
#define BUTTERFLY_16(in0, in1, in2, in3,            \
                     in4, in5, in6, in7,            \
                     in8, in9,  in10, in11,         \
                     in12, in13, in14, in15,        \
                     out0, out1, out2, out3,        \
                     out4, out5, out6, out7,        \
                     out8, out9, out10, out11,      \
                     out12, out13, out14, out15) {  \
  out0 = (in0) + (in15);                            \
  out1 = (in1) + (in14);                            \
  out2 = (in2) + (in13);                            \
  out3 = (in3) + (in12);                            \
  out4 = (in4) + (in11);                            \
  out5 = (in5) + (in10);                            \
  out6 = (in6) + (in9);                             \
  out7 = (in7) + (in8);                             \
                                                    \
  out8 = (in7) - (in8);                             \
  out9 = (in6) - (in9);                             \
  out10 = (in5) - (in10);                           \
  out11 = (in4) - (in11);                           \
  out12 = (in3) - (in12);                           \
  out13 = (in2) - (in13);                           \
  out14 = (in1) - (in14);                           \
  out15 = (in0) - (in15);                           \
}

#define ADD_4ELE_PAIRS(in0, in1, in2, in3,        \
                       in4, in5, in6, in7,        \
                       out0, out1, out2, out3) {  \
  out0 = (in0) + (in4);                           \
  out1 = (in1) + (in5);                           \
  out2 = (in2) + (in6);                           \
  out3 = (in3) + (in7);                           \
}

#define SUB_4ELE_PAIRS(in0, in1, in2, in3,        \
                       in4, in5, in6, in7,        \
                       out0, out1, out2, out3) {  \
  out0 = (in0) - (in4);                           \
  out1 = (in1) - (in5);                           \
  out2 = (in2) - (in6);                           \
  out3 = (in3) - (in7);                           \
}

#define ADD_RESIDUE_PRED_CLIP_AND_STORE_4(dest, dst_stride,        \
                                          in0, in1, in2, in3) {    \
  uint32_t src0_m, src1_m, src2_m, src3_m;                         \
  uint32_t out0_m, out1_m, out2_m, out3_m;                         \
  v8i16 inp0_m, inp1_m;                                            \
  v8i16 res0_m, res1_m;                                            \
  v16i8 dest0_m = { 0 };                                           \
  v16i8 dest1_m = { 0 };                                           \
  v16i8 zero_m = { 0 };                                            \
                                                                   \
  inp0_m = (v8i16) __msa_ilvr_d((v2i64) (in1), (v2i64) (in0));     \
  inp1_m = (v8i16) __msa_ilvr_d((v2i64) (in3), (v2i64) (in2));     \
                                                                   \
  LOAD_4WORDS_WITH_STRIDE(dest, dst_stride,                        \
                          src0_m, src1_m, src2_m, src3_m);         \
  dest0_m = (v16i8) __msa_insert_w((v4i32) dest0_m, 0, src0_m);    \
  dest0_m = (v16i8) __msa_insert_w((v4i32) dest0_m, 1, src1_m);    \
  dest1_m = (v16i8) __msa_insert_w((v4i32) dest1_m, 0, src2_m);    \
  dest1_m = (v16i8) __msa_insert_w((v4i32) dest1_m, 1, src3_m);    \
                                                                   \
  res0_m = (v8i16) __msa_ilvr_b(zero_m, dest0_m);                  \
  res1_m = (v8i16) __msa_ilvr_b(zero_m, dest1_m);                  \
                                                                   \
  res0_m += inp0_m;                                                \
  res1_m += inp1_m;                                                \
                                                                   \
  res0_m = CLIP_UNSIGNED_CHAR_H(res0_m);                           \
  res1_m = CLIP_UNSIGNED_CHAR_H(res1_m);                           \
                                                                   \
  dest0_m = __msa_pckev_b((v16i8) res0_m, (v16i8) res0_m);         \
  dest1_m = __msa_pckev_b((v16i8) res1_m, (v16i8) res1_m);         \
                                                                   \
  out0_m = __msa_copy_u_w((v4i32) dest0_m, 0);                     \
  out1_m = __msa_copy_u_w((v4i32) dest0_m, 1);                     \
  out2_m = __msa_copy_u_w((v4i32) dest1_m, 0);                     \
  out3_m = __msa_copy_u_w((v4i32) dest1_m, 1);                     \
                                                                   \
  STORE_WORD(dest, out0_m);                                        \
  dest += dst_stride;                                              \
  STORE_WORD(dest, out1_m);                                        \
  dest += dst_stride;                                              \
  STORE_WORD(dest, out2_m);                                        \
  dest += dst_stride;                                              \
  STORE_WORD(dest, out3_m);                                        \
}
#endif  /* HAVE_MSA */
#endif  /* VP9_COMMON_MIPS_MSA_VP9_MACROS_MSA_H_ */
