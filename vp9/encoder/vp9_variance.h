/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_ENCODER_VP9_VARIANCE_H_
#define VP9_ENCODER_VP9_VARIANCE_H_

#include "vpx/vpx_integer.h"

#ifdef __cplusplus
extern "C" {
#endif

void variance(const uint8_t *a, int a_stride,
              const uint8_t *b, int b_stride,
              int  w, int  h,
              unsigned int *sse, int *sum);

#if CONFIG_VP9_HIGHBITDEPTH
void highbd_variance(const uint8_t *a8, int a_stride,
                     const uint8_t *b8, int b_stride,
                     int w, int h,
                     unsigned int *sse, int *sum);

void highbd_10_variance(const uint8_t *a8, int a_stride,
                        const uint8_t *b8, int b_stride,
                        int w, int h,
                        unsigned int *sse, int *sum);

void highbd_12_variance(const uint8_t *a8, int a_stride,
                        const uint8_t *b8, int b_stride,
                        int w, int h,
                        unsigned int *sse, int *sum);
#endif

typedef unsigned int(*vp9_sad_fn_t)(const uint8_t *src_ptr,
                                    int source_stride,
                                    const uint8_t *ref_ptr,
                                    int ref_stride);

typedef unsigned int(*vp9_sad_avg_fn_t)(const uint8_t *src_ptr,
                                        int source_stride,
                                        const uint8_t *ref_ptr,
                                        int ref_stride,
                                        const uint8_t *second_pred);

typedef void (*vp9_sad_multi_fn_t)(const uint8_t *src_ptr,
                                   int source_stride,
                                   const uint8_t *ref_ptr,
                                   int  ref_stride,
                                   unsigned int *sad_array);

typedef void (*vp9_sad_multi_d_fn_t)(const uint8_t *src_ptr,
                                     int source_stride,
                                     const uint8_t* const ref_ptr[],
                                     int  ref_stride, unsigned int *sad_array);

typedef unsigned int (*vp9_variance_fn_t)(const uint8_t *src_ptr,
                                          int source_stride,
                                          const uint8_t *ref_ptr,
                                          int ref_stride,
                                          unsigned int *sse);

typedef unsigned int (*vp9_subpixvariance_fn_t)(const uint8_t *src_ptr,
                                                int source_stride,
                                                int xoffset,
                                                int yoffset,
                                                const uint8_t *ref_ptr,
                                                int Refstride,
                                                unsigned int *sse);

typedef unsigned int (*vp9_subp_avg_variance_fn_t)(const uint8_t *src_ptr,
                                                   int source_stride,
                                                   int xoffset,
                                                   int yoffset,
                                                   const uint8_t *ref_ptr,
                                                   int Refstride,
                                                   unsigned int *sse,
                                                   const uint8_t *second_pred);

<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
#if CONFIG_MASKED_INTERINTRA || CONFIG_MASKED_INTERINTER
typedef unsigned int(*vp9_masked_sad_fn_t)(const uint8_t *src_ptr,
                                         int source_stride,
                                         const uint8_t *ref_ptr,
                                         int ref_stride,
                                         const uint8_t *msk_ptr,
                                         int msk_stride,
                                         unsigned int max_sad);
typedef unsigned int (*vp9_masked_variance_fn_t)(const uint8_t *src_ptr,
                                               int source_stride,
                                               const uint8_t *ref_ptr,
                                               int ref_stride,
                                               const uint8_t *msk_ptr,
                                               int msk_stride,
                                               unsigned int *sse);
typedef unsigned int (*vp9_masked_subpixvariance_fn_t)(const uint8_t *src_ptr,
                                                     int source_stride,
                                                     int xoffset,
                                                     int yoffset,
                                                     const uint8_t *ref_ptr,
                                                     int Refstride,
                                                     const uint8_t *msk_ptr,
                                                     int msk_stride,
                                                     unsigned int *sse);
#endif

typedef void (*vp9_ssimpf_fn_t)(uint8_t *s, int sp, uint8_t *r,
                                int rp, unsigned long *sum_s,
                                unsigned long *sum_r, unsigned long *sum_sq_s,
                                unsigned long *sum_sq_r,
                                unsigned long *sum_sxr);

typedef unsigned int (*vp9_getmbss_fn_t)(const short *);

typedef unsigned int (*vp9_get16x16prederror_fn_t)(const uint8_t *src_ptr,
                                                   int source_stride,
                                                   const uint8_t *ref_ptr,
                                                   int  ref_stride);

=======
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")
typedef struct vp9_variance_vtable {
  vp9_sad_fn_t               sdf;
  vp9_sad_avg_fn_t           sdaf;
  vp9_variance_fn_t          vf;
  vp9_subpixvariance_fn_t    svf;
  vp9_subp_avg_variance_fn_t svaf;
  vp9_sad_multi_fn_t         sdx3f;
  vp9_sad_multi_fn_t         sdx8f;
  vp9_sad_multi_d_fn_t       sdx4df;
#if ((CONFIG_MASKED_INTERINTRA && CONFIG_INTERINTRA) || \
    CONFIG_MASKED_INTERINTER)
  vp9_masked_sad_fn_t            msdf;
  vp9_masked_variance_fn_t       mvf;
  vp9_masked_subpixvariance_fn_t msvf;
#endif
} vp9_variance_fn_ptr_t;

void vp9_comp_avg_pred(uint8_t *comp_pred, const uint8_t *pred, int width,
                       int height, const uint8_t *ref, int ref_stride);

#if CONFIG_VP9_HIGHBITDEPTH
void vp9_highbd_comp_avg_pred(uint16_t *comp_pred, const uint8_t *pred,
                              int width, int height,
                              const uint8_t *ref, int ref_stride);
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_VARIANCE_H_
