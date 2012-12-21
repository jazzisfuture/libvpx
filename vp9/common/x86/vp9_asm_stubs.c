/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vpx_ports/config.h"
#include "vpx_ports/mem.h"
#include "vp9/common/vp9_subpixel.h"

extern const short vp9_six_tap_mmx[16][6 * 8];

extern const short vp9_bilinear_filters_8x_mmx[16][2 * 8];

extern void vp9_filter_block1d_h6_mmx(unsigned char   *src_ptr,
                                      unsigned short  *output_ptr,
                                      unsigned int     src_pixels_per_line,
                                      unsigned int     pixel_step,
                                      unsigned int     output_height,
                                      unsigned int     output_width,
                                      const short     *vp9_filter);

extern void vp9_filter_block1dc_v6_mmx(unsigned short *src_ptr,
                                       unsigned char  *output_ptr,
                                       int             output_pitch,
                                       unsigned int    pixels_per_line,
                                       unsigned int    pixel_step,
                                       unsigned int    output_height,
                                       unsigned int    output_width,
                                       const short    *vp9_filter);

extern void vp9_filter_block1d8_h6_sse2(unsigned char  *src_ptr,
                                        unsigned short *output_ptr,
                                        unsigned int    src_pixels_per_line,
                                        unsigned int    pixel_step,
                                        unsigned int    output_height,
                                        unsigned int    output_width,
                                        const short    *vp9_filter);

extern void vp9_filter_block1d16_h6_sse2(unsigned char  *src_ptr,
                                         unsigned short *output_ptr,
                                         unsigned int    src_pixels_per_line,
                                         unsigned int    pixel_step,
                                         unsigned int    output_height,
                                         unsigned int    output_width,
                                         const short    *vp9_filter);

extern void vp9_filter_block1d8_v6_sse2(unsigned short *src_ptr,
                                        unsigned char *output_ptr,
                                        int dst_ptich,
                                        unsigned int pixels_per_line,
                                        unsigned int pixel_step,
                                        unsigned int output_height,
                                        unsigned int output_width,
                                        const short    *vp9_filter);

extern void vp9_filter_block1d16_v6_sse2(unsigned short *src_ptr,
                                         unsigned char *output_ptr,
                                         int dst_ptich,
                                         unsigned int pixels_per_line,
                                         unsigned int pixel_step,
                                         unsigned int output_height,
                                         unsigned int output_width,
                                         const short    *vp9_filter);

extern void vp9_unpack_block1d16_h6_sse2(unsigned char  *src_ptr,
                                         unsigned short *output_ptr,
                                         unsigned int    src_pixels_per_line,
                                         unsigned int    output_height,
                                         unsigned int    output_width);

extern void vp9_filter_block1d8_h6_only_sse2(unsigned char *src_ptr,
                                             unsigned int   src_pixels_per_line,
                                             unsigned char *output_ptr,
                                             int            dst_pitch,
                                             unsigned int   output_height,
                                             const short   *vp9_filter);

extern void vp9_filter_block1d16_h6_only_sse2(unsigned char *src_ptr,
                                              unsigned int   src_pixels_per_lin,
                                              unsigned char *output_ptr,
                                              int            dst_pitch,
                                              unsigned int   output_height,
                                              const short   *vp9_filter);

extern void vp9_filter_block1d8_v6_only_sse2(unsigned char *src_ptr,
                                             unsigned int   src_pixels_per_line,
                                             unsigned char *output_ptr,
                                             int            dst_pitch,
                                             unsigned int   output_height,
                                             const short   *vp9_filter);

extern prototype_subpixel_predict(vp9_bilinear_predict8x8_mmx);

///////////////////////////////////////////////////////////////////////////
// the mmx function that does the bilinear filtering and var calculation //
// int one pass                                                          //
///////////////////////////////////////////////////////////////////////////
DECLARE_ALIGNED(16, const short, vp9_bilinear_filters_mmx[16][8]) = {
  { 128, 128, 128, 128,  0,  0,  0,  0 },
  { 120, 120, 120, 120,  8,  8,  8,  8 },
  { 112, 112, 112, 112, 16, 16, 16, 16 },
  { 104, 104, 104, 104, 24, 24, 24, 24 },
  {  96, 96, 96, 96, 32, 32, 32, 32 },
  {  88, 88, 88, 88, 40, 40, 40, 40 },
  {  80, 80, 80, 80, 48, 48, 48, 48 },
  {  72, 72, 72, 72, 56, 56, 56, 56 },
  {  64, 64, 64, 64, 64, 64, 64, 64 },
  {  56, 56, 56, 56, 72, 72, 72, 72 },
  {  48, 48, 48, 48, 80, 80, 80, 80 },
  {  40, 40, 40, 40, 88, 88, 88, 88 },
  {  32, 32, 32, 32, 96, 96, 96, 96 },
  {  24, 24, 24, 24, 104, 104, 104, 104 },
  {  16, 16, 16, 16, 112, 112, 112, 112 },
  {   8,  8,  8,  8, 120, 120, 120, 120 }
};

#if HAVE_MMX

void vp9_bilinear_predict16x16_mmx(unsigned char  *src_ptr,
                                   int  src_pixels_per_line,
                                   int  xoffset,
                                   int  yoffset,
                                   unsigned char *dst_ptr,
                                   int  dst_pitch) {
  vp9_bilinear_predict8x8_mmx(src_ptr,
                              src_pixels_per_line, xoffset, yoffset,
                              dst_ptr, dst_pitch);
  vp9_bilinear_predict8x8_mmx(src_ptr + 8,
                              src_pixels_per_line, xoffset, yoffset,
                              dst_ptr + 8, dst_pitch);
  vp9_bilinear_predict8x8_mmx(src_ptr + 8 * src_pixels_per_line,
                              src_pixels_per_line, xoffset, yoffset,
                              dst_ptr + dst_pitch * 8, dst_pitch);
  vp9_bilinear_predict8x8_mmx(src_ptr + 8 * src_pixels_per_line + 8,
                              src_pixels_per_line, xoffset, yoffset,
                              dst_ptr + dst_pitch * 8 + 8, dst_pitch);
}
#endif

#if HAVE_SSSE3
extern void vp9_filter_block1d8_h6_ssse3(unsigned char  *src_ptr,
                                         unsigned int    src_pixels_per_line,
                                         unsigned char  *output_ptr,
                                         unsigned int    output_pitch,
                                         unsigned int    output_height,
                                         unsigned int    vp9_filter_index);

extern void vp9_filter_block1d16_h6_ssse3(unsigned char  *src_ptr,
                                          unsigned int    src_pixels_per_line,
                                          unsigned char  *output_ptr,
                                          unsigned int    output_pitch,
                                          unsigned int    output_height,
                                          unsigned int    vp9_filter_index);

extern void vp9_filter_block1d16_v6_ssse3(unsigned char *src_ptr,
                                          unsigned int   src_pitch,
                                          unsigned char *output_ptr,
                                          unsigned int   out_pitch,
                                          unsigned int   output_height,
                                          unsigned int   vp9_filter_index);

extern void vp9_filter_block1d8_v6_ssse3(unsigned char *src_ptr,
                                         unsigned int   src_pitch,
                                         unsigned char *output_ptr,
                                         unsigned int   out_pitch,
                                         unsigned int   output_height,
                                         unsigned int   vp9_filter_index);

extern void vp9_filter_block1d4_h6_ssse3(unsigned char  *src_ptr,
                                         unsigned int    src_pixels_per_line,
                                         unsigned char  *output_ptr,
                                         unsigned int    output_pitch,
                                         unsigned int    output_height,
                                         unsigned int    vp9_filter_index);

extern void vp9_filter_block1d4_v6_ssse3(unsigned char *src_ptr,
                                         unsigned int   src_pitch,
                                         unsigned char *output_ptr,
                                         unsigned int   out_pitch,
                                         unsigned int   output_height,
                                         unsigned int   vp9_filter_index);

void vp9_filter_block1d16_v8_ssse3(const unsigned char *src_ptr,
                                   const unsigned int src_pitch,
                                   unsigned char *output_ptr,
                                   unsigned int out_pitch,
                                   unsigned int output_height,
                                   const short *filter);

void vp9_filter_block1d16_h8_ssse3(const unsigned char *src_ptr,
                                   const unsigned int src_pitch,
                                   unsigned char *output_ptr,
                                   unsigned int out_pitch,
                                   unsigned int output_height,
                                   const short *filter);

void vp9_filter_block2d_16x16_8_ssse3(const unsigned char *src_ptr,
                                      const unsigned int src_stride,
                                      const short *hfilter_aligned16,
                                      const short *vfilter_aligned16,
                                      unsigned char *dst_ptr,
                                      unsigned int dst_stride) {
  if (hfilter_aligned16[3] != 128 && vfilter_aligned16[3] != 128) {
    DECLARE_ALIGNED_ARRAY(16, unsigned char, fdata2, 23 * 16);

    vp9_filter_block1d16_h8_ssse3(src_ptr - (3 * src_stride), src_stride,
                                  fdata2, 16, 23, hfilter_aligned16);
    vp9_filter_block1d16_v8_ssse3(fdata2, 16, dst_ptr, dst_stride, 16,
                                  vfilter_aligned16);
  } else {
    if (hfilter_aligned16[3] != 128) {
      vp9_filter_block1d16_h8_ssse3(src_ptr, src_stride, dst_ptr, dst_stride,
                                    16, hfilter_aligned16);
    } else {
      vp9_filter_block1d16_v8_ssse3(src_ptr - (3 * src_stride), src_stride,
                                    dst_ptr, dst_stride, 16, vfilter_aligned16);
    }
  }
}

void vp9_filter_block1d8_v8_ssse3(const unsigned char *src_ptr,
                                   const unsigned int src_pitch,
                                   unsigned char *output_ptr,
                                   unsigned int out_pitch,
                                   unsigned int output_height,
                                   const short *filter);

void vp9_filter_block1d8_h8_ssse3(const unsigned char *src_ptr,
                                   const unsigned int src_pitch,
                                   unsigned char *output_ptr,
                                   unsigned int out_pitch,
                                   unsigned int output_height,
                                   const short *filter);

void vp9_filter_block2d_8x8_8_ssse3(const unsigned char *src_ptr,
                                    const unsigned int src_stride,
                                    const short *hfilter_aligned16,
                                    const short *vfilter_aligned16,
                                    unsigned char *dst_ptr,
                                    unsigned int dst_stride) {
  if (hfilter_aligned16[3] != 128 && vfilter_aligned16[3] != 128) {
    DECLARE_ALIGNED_ARRAY(16, unsigned char, fdata2, 23 * 16);

    vp9_filter_block1d8_h8_ssse3(src_ptr - (3 * src_stride), src_stride,
                                 fdata2, 16, 15, hfilter_aligned16);
    vp9_filter_block1d8_v8_ssse3(fdata2, 16, dst_ptr, dst_stride, 8,
                                 vfilter_aligned16);
  } else {
    if (hfilter_aligned16[3] != 128) {
      vp9_filter_block1d8_h8_ssse3(src_ptr, src_stride, dst_ptr, dst_stride, 8,
                                   hfilter_aligned16);
    } else {
      vp9_filter_block1d8_v8_ssse3(src_ptr - (3 * src_stride), src_stride,
                                   dst_ptr, dst_stride, 8, vfilter_aligned16);
    }
  }
}

void vp9_filter_block2d_8x4_8_ssse3(const unsigned char *src_ptr,
                                    const unsigned int src_stride,
                                    const short *hfilter_aligned16,
                                    const short *vfilter_aligned16,
                                    unsigned char *dst_ptr,
                                    unsigned int dst_stride) {
  if (hfilter_aligned16[3] !=128 && vfilter_aligned16[3] != 128) {
      DECLARE_ALIGNED_ARRAY(16, unsigned char, fdata2, 23 * 16);

      vp9_filter_block1d8_h8_ssse3(src_ptr - (3 * src_stride), src_stride,
                                   fdata2, 16, 11, hfilter_aligned16);
      vp9_filter_block1d8_v8_ssse3(fdata2, 16, dst_ptr, dst_stride, 4,
                                   vfilter_aligned16);
  } else {
    if (hfilter_aligned16[3] != 128) {
      vp9_filter_block1d8_h8_ssse3(src_ptr, src_stride, dst_ptr, dst_stride, 4,
                                   hfilter_aligned16);
    } else {
      vp9_filter_block1d8_v8_ssse3(src_ptr - (3 * src_stride), src_stride,
                                   dst_ptr, dst_stride, 4, vfilter_aligned16);
    }
  }
}
#endif
