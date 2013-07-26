/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "./vpx_config.h"
#include "vpx_scale/yv12config.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_scale/vpx_scale.h"

#if HAVE_DSPR2
static void extend_plane(uint8_t *s,       /* source */
                         int sp,           /* source pitch */
                         int w,            /* width */
                         int h,            /* height */
                         int et,           /* extend top border */
                         int el,           /* extend left border */
                         int eb,           /* extend bottom border */
                         int er) {         /* extend right border */
  int i,j;
  uint8_t *src_ptr1, *src_ptr2;
  uint8_t *dest_ptr1, *dest_ptr2;
  uint8_t *dest_ptr1_tmp, *dest_ptr2_tmp;
  uint32_t tmp;
  uint32_t tmp2;
  uint32_t linesize;

  /* copy the left and right most columns out */
  src_ptr1 = s;
  src_ptr2 = s + w - 1;
  dest_ptr1 = s - el;
  dest_ptr2 = s + w;

  for (i = h; i--; ) {
    dest_ptr1_tmp = dest_ptr1;
    dest_ptr2_tmp = dest_ptr2;

    __asm__ __volatile__ (
    "lb        %[tmp], 0(%[src_ptr1])             \n\t"
    "lb        %[tmp2], 0(%[src_ptr2])            \n\t"
    "replv.qb  %[tmp], %[tmp]                     \n\t"
    "replv.qb  %[tmp2], %[tmp2]                   \n\t"

    :[tmp] "=&r" (tmp), [tmp2] "=&r" (tmp2)
    :[src_ptr1] "r" (src_ptr1), [src_ptr2] "r" (src_ptr2)
    );

    for (j = el/4; j--; ) {
      __asm__ __volatile__ (
      "sw    %[tmp], 0(%[dest_ptr1_tmp])           \n\t"
      :
      :[dest_ptr1_tmp] "r" (dest_ptr1_tmp), [tmp] "r" (tmp)

      );
      dest_ptr1_tmp +=4;
    }

    for (j = el%4; j--; ) {
      __asm__ __volatile__ (
      "sb    %[tmp], 0(%[dest_ptr1_tmp])           \n\t"
      :
      :[dest_ptr1_tmp] "r" (dest_ptr1_tmp), [tmp] "r" (tmp)

      );
      dest_ptr1_tmp +=1;
    }

    for (j = er/4; j--; ) {
      __asm__ __volatile__ (
      "sw    %[tmp2], 0(%[dest_ptr2_tmp])           \n\t"
      :
      :[dest_ptr2_tmp] "r" (dest_ptr2_tmp), [tmp2] "r" (tmp2)

      );
      dest_ptr2_tmp +=4;
    }

    for (j = er%4; j--; ) {
      __asm__ __volatile__ (
      "sb    %[tmp2], 0(%[dest_ptr2_tmp])           \n\t"
      :
      :[dest_ptr2_tmp] "r" (dest_ptr2_tmp), [tmp2] "r" (tmp2)

      );
      dest_ptr2_tmp +=1;
    }

    src_ptr1  += sp;
    src_ptr2  += sp;
    dest_ptr1 += sp;
    dest_ptr2 += sp;
  }

  /* Now copy the top and bottom lines into each line of the respective
   * borders
   */
  src_ptr1 = s - el;
  src_ptr2 = s + sp * (h - 1) - el;
  dest_ptr1 = s + sp * (-et) - el;
  dest_ptr2 = s + sp * (h) - el;
  linesize = el + er + w;

  for (i = 0; i < et; i++) {
    vpx_memcpy(dest_ptr1, src_ptr1, linesize);
    dest_ptr1 += sp;
  }

  for (i = 0; i < eb; i++) {
    vpx_memcpy(dest_ptr2, src_ptr2, linesize);
    dest_ptr2 += sp;
  }
}

void vp9_extend_frame_borders_dspr2(YV12_BUFFER_CONFIG *ybf,
                                    int subsampling_x, int subsampling_y) {
  const int c_w = (ybf->y_crop_width + subsampling_x) >> subsampling_x;
  const int c_h = (ybf->y_crop_height + subsampling_y) >> subsampling_y;
  const int c_et = ybf->border >> subsampling_y;
  const int c_el = ybf->border >> subsampling_x;
  const int c_eb = (ybf->border + ybf->y_height - ybf->y_crop_height +
                    subsampling_y) >> subsampling_y;
  const int c_er = (ybf->border + ybf->y_width - ybf->y_crop_width +
                    subsampling_x) >> subsampling_x;

  assert(ybf->y_height - ybf->y_crop_height < 16);
  assert(ybf->y_width - ybf->y_crop_width < 16);
  assert(ybf->y_height - ybf->y_crop_height >= 0);
  assert(ybf->y_width - ybf->y_crop_width >= 0);

  extend_plane(ybf->y_buffer, ybf->y_stride,
               ybf->y_crop_width, ybf->y_crop_height,
               ybf->border, ybf->border,
               ybf->border + ybf->y_height - ybf->y_crop_height,
               ybf->border + ybf->y_width - ybf->y_crop_width);

  extend_plane(ybf->u_buffer, ybf->uv_stride,
               c_w, c_h, c_et, c_el, c_eb, c_er);

  extend_plane(ybf->v_buffer, ybf->uv_stride,
               c_w, c_h, c_et, c_el, c_eb, c_er);
}
#endif
