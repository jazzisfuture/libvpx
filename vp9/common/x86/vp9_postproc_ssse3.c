/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <tmmintrin.h>
#include "vpx_ports/mem.h"
#include "vpx_ports/emmintrin_compat.h"


#include "./vpx_config.h"
#include "./vpx_scale_rtcd.h"
#include "./vp9_rtcd.h"

#include "vpx_scale/vpx_scale.h"
#include "vpx_scale/yv12config.h"

#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_postproc.h"
#include "vp9/common/vp9_systemdependent.h"
#include "vp9/common/vp9_textblit.h"

DECLARE_ALIGNED(32, static const uint16_t, round_pattern_x86[64]) = {
   0, 48, 12, 60,  3, 51, 15, 63,
  32, 16, 44, 28, 35, 19, 47, 31,
   8, 56,  4, 52, 11, 59,  7, 55,
  40, 24, 36, 20, 43, 27, 39, 23,
   2, 50, 14, 62,  1, 49, 13, 61,
  34, 18, 46, 30, 33, 17, 45, 29,
  10, 58,  6, 54,  9, 57,  5, 53,
  42, 26, 38, 22, 41, 25, 37, 21
};

static INLINE void pattern_round(uint16_t* src_in, uint8_t* dst_out,
                                 int src_stride, int dst_stride,
                                 int height, int width) {
  int i, j;
  uint16_t* s = (uint16_t*) src_in;
  uint8_t* d = (uint8_t*) dst_out;
  for (i = 0; i < height; ++i) {
    const __m128i v_pattern =
        _mm_load_si128((const __m128i *)&round_pattern_x86[(i & 7) * 8]);
    for (j = 0; j < width; j += 16) {
      const __m128i v_src1 = _mm_loadu_si128((__m128i *)&s[j + 0]);
      const __m128i v_src2 = _mm_loadu_si128((__m128i *)&s[j + 8]);
      const __m128i v_pat1 = _mm_add_epi16(v_src1, v_pattern);
      const __m128i v_pat2 = _mm_add_epi16(v_src2, v_pattern);
      const __m128i v_sr1 = _mm_srli_epi16(v_pat1, 6);
      const __m128i v_sr2 = _mm_srli_epi16(v_pat2, 6);
      _mm_store_si128((__m128i *)&d[j + 0],
                      _mm_packus_epi16(v_sr1, v_sr2));
//      d[j] = ((uint8_t) ((s[j] + vp9_round_pattern_x86[index3]) >> 6));
    }
    s += src_stride;
    d += dst_stride;
  }
}

static INLINE void convert_to_14(const uint8_t* src_in, uint16_t* dst_out,
                                 int src_stride, int dst_stride,
                                 int height, int width) {
  int i, j;
  const uint8_t* s = src_in;
  uint16_t* d = (uint16_t*) dst_out;
  const __m128i zero = _mm_set1_epi16(0);
  for (i = 0; i < height; ++i) {
    for (j = 0; j < width; j += 16) {
      const __m128i v_src = _mm_loadu_si128((const __m128i *)&s[j]);
      const __m128i v_unpack_lo = _mm_unpacklo_epi8(zero, v_src);
      const __m128i v_unpack_hi = _mm_unpackhi_epi8(zero, v_src);
      _mm_store_si128((__m128i *)&d[j + 0],
                      _mm_srli_epi16(v_unpack_lo, 2));
      _mm_store_si128((__m128i *)&d[j + 8],
                      _mm_srli_epi16(v_unpack_hi, 2));
//      dst_out[index1] = ((uint16_t) src_in[index2]) << 6;
    }
    s += src_stride;
    d += dst_stride;
  }
}

// number of mean, max, and min values calculated per inner loop
#define INNER_W 4

// number of value1 and value2 calculated per inner loop
#define INNER_VAL_W 8

DECLARE_ALIGNED(32, static const uint8_t, index_shuf_8[2][8]) = {
  {0, 0, 1, 1, 2, 2, 3, 3},
  {0, 1, 1, 2, 2, 3, 3, 4}
};

DECLARE_ALIGNED(32, static const uint8_t, index_shuf_16[2][16]) = {
  {0, 1, 0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7},
  {0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 8, 9}
};

static void BitGen_recur_ssse3(const uint8_t* src_max,
                               const uint8_t* src_min,
                               uint16_t* src_dst_mean,
                               uint8_t* downMax,
                               uint8_t* downMin,
                               uint16_t* downMean,
                               int max_stride,
                               int min_stride,
                               int mean_stride,
                               int depth, int height, int width,
                               int thresh, int hshift, int rcount, int rlimit) {
  int i, j, k, l, i1, j1, index1, index2, index3, index5;
  int downWidth, downHeight, thisSize;

  if(rcount > rlimit) {
//    printf("BitGen_recur_ssse3(): width %d height %d rcount %d rlimit %d \n",
  //         width, height, rcount, rlimit);
    return;
  }

  downWidth = (width >> 1) + 1;
  downHeight = (height >> 1) + 1;

  for (l = 1; l >= 0; --l) {
    for (k = 0; k < depth; ++k) {
      for (i = 0; i < downHeight; ++i) {

        i1 = (i << 1) - l;

        if ((i1 < 0) || ((i1 + 1) >= height)) {
          index5 = ((l * depth + k) * downHeight + i) * downWidth;
          for (j = 0; j < downWidth; ++j) {
            downMean[index5 + j] = 0;
            downMax[index5 + j] = 255;
            downMin[index5 + j] = 0;
          }
        } else {
          for (j = 0; j + (INNER_W - 1) < downWidth; j += INNER_W) {
            index5 = ((l * depth + k) * downHeight + i) * downWidth + j;

            j1 = (j << 1) - ((l == 0)?hshift:(1 - hshift));

            index1 = (k * height + i1) * mean_stride + j1;
            index3 = index1 + mean_stride;
            {
              // calculate 4 mean values
              const __m128i two = _mm_set1_epi16(2);
              const __m128i v_a =
                  _mm_loadu_si128((__m128i *)&src_dst_mean[index1]);
              const __m128i v_b =
                  _mm_loadu_si128((__m128i *)&src_dst_mean[index3]);
              const __m128i v_hab = _mm_hadd_epi16(v_a, v_b);
              const __m128i v_hab_h = _mm_srli_si128(v_hab, 8);
              const __m128i v_d = _mm_add_epi16(v_hab, v_hab_h);
              const __m128i v_d2 = _mm_add_epi16(v_d, two);
              const __m128i v_final = _mm_srli_epi16(v_d2, 2);
              _mm_storel_epi64((__m128i *)&downMean[index5], v_final);
            }
            index1 = (k * height + i1) * max_stride + j1;
            index3 = index1 + max_stride;
            {
              // calculate 4 max values
              uint32_t *d = (uint32_t *)(&downMax[index5]);
              const __m128i v_a =
                  _mm_loadl_epi64((const __m128i *)&src_max[index1]);
              const __m128i v_b =
                  _mm_loadl_epi64((const __m128i *)&src_max[index3]);
              const __m128i v_c = _mm_max_epu8(v_a, v_b);
              const __m128i v_d = _mm_max_epu8(v_c, _mm_slli_epi16(v_c, 8));
              const __m128i v_e = _mm_srli_epi16(v_d, 8);
              const __m128i v_f = _mm_packus_epi16(v_e, v_e);
              d[0] = _mm_cvtsi128_si32(v_f);
            }
            index1 = (k * height + i1) * min_stride + j1;
            index3 = index1 + max_stride;
            {
              // calculate 4 min values
              uint32_t *d = (uint32_t *)(&downMin[index5]);
              const __m128i v_a =
                  _mm_loadl_epi64((const __m128i *)&src_min[index1]);
              const __m128i v_b =
                  _mm_loadl_epi64((const __m128i *)&src_min[index3]);
              const __m128i v_c = _mm_min_epu8(v_a, v_b);
              const __m128i v_d = _mm_min_epu8(v_c, _mm_slli_epi16(v_c, 8));
              const __m128i v_e = _mm_srli_epi16(v_d, 8);
              const __m128i v_f = _mm_packus_epi16(v_e, v_e);
              d[0] = _mm_cvtsi128_si32(v_f);
            }
            // after SIMD ops, go back and fix border case
            if (j1 < 0) {
              int z;
              const int num = 0 - j1;
              for(z = 0; z < num; z++) {
                downMean[index5 + z] = 0;
                downMax[index5 + z] = 255;
                downMin[index5 + z] = 0;
              }
            }
          } // j

          // scalar code for remaining
          for (; j < downWidth; ++j) {
            int index4;
            index5 = ((l * depth + k) * downHeight + i) * downWidth + j;
            i1 = (i << 1) - l;
            j1 = (j << 1) - ((l == 0)?hshift:(1 - hshift));
            if ((i1 >= 0) && ((i1 + 1) < height) &&
                (j1 >= 0) && ((j1 + 1) < width)){
              index1 = (k * height + i1) * mean_stride + j1;
              index2 = index1 + 1;
              index3 = index1 + mean_stride;
              index4 = index3 + 1;
              downMean[index5] = (src_dst_mean[index1] +
                                  src_dst_mean[index2] +
                                  src_dst_mean[index3] +
                                  src_dst_mean[index4] + 2) >> 2;

              index1 = (k * height + i1) * max_stride + j1;
              index2 = index1 + 1;
              index3 = index1 + max_stride;
              index4 = index3 + 1;
              downMax[index5] = MAX(MAX(src_max[index1],
                                        src_max[index2]),
                                    MAX(src_max[index3],
                                        src_max[index4]));

              index1 = (k * height + i1) * min_stride + j1;
              index2 = index1 + 1;
              index3 = index1 + min_stride;
              index4 = index3 + 1;
              downMin[index5] = MIN(MIN(src_min[index1],
                                        src_min[index2]),
                                    MIN(src_min[index3],
                                        src_min[index4]));
            }else{
              downMean[index5] = 0;
              downMax[index5] = 255;
              downMin[index5] = 0;
            }
          } // scalar j
        }
      } // i
    } // k
  } // l

  thisSize = downWidth * downHeight * depth * 2;

  BitGen_recur_ssse3(downMax, downMin, downMean,
                     downMax + thisSize,
                     downMin + thisSize,
                     downMean + thisSize,
                     downWidth, downWidth, downWidth,
                     depth * 2, downHeight, downWidth,
                     thresh, 1 - hshift, rcount + 1, rlimit);

  {
    const __m128i v_shuf8_i2 =
        _mm_loadl_epi64((const __m128i *)index_shuf_8[hshift]);
    const __m128i v_shuf8_i3 =
        _mm_loadl_epi64((const __m128i *)index_shuf_8[1 - hshift]);
    const __m128i v_shuf16_i2 =
        _mm_load_si128((const __m128i *)index_shuf_16[hshift]);
    const __m128i v_shuf16_i3 =
        _mm_load_si128((const __m128i *)index_shuf_16[1 - hshift]);
    const __m128i zero = _mm_set1_epi8(0);
    const __m128i v_thresh = _mm_cvtsi32_si128(thresh);
    const __m128i v_thresh_8 = _mm_shuffle_epi8(v_thresh, zero);

    for (k = 0; k < depth; ++k) {
      for (i = 0; i < height; ++i) {
        for (j = 0; j + (INNER_VAL_W - 1) < width; j += INNER_VAL_W) {
          const int src_dst_index1 = (k * height + i) * mean_stride + j;
          const __m128i v_orig =
              _mm_loadu_si128((__m128i *)&src_dst_mean[src_dst_index1]);
          __m128i v_val1;
          __m128i v_val2;

          index2 = (k * downHeight + (i >> 1)) * downWidth +
              ((j) >> 1);
          {
            const __m128i v_max_i2 =
                _mm_loadl_epi64((__m128i *)&downMax[index2]);
            const __m128i v_min_i2 =
                _mm_loadl_epi64((__m128i *)&downMin[index2]);
            const __m128i v_t1 = _mm_subs_epu8(v_max_i2, v_min_i2);
            const __m128i v_t1_shuf = _mm_shuffle_epi8(v_t1, v_shuf8_i2);
            const __m128i v_mean_i2 =
                _mm_loadu_si128((__m128i *)&downMean[index2]);
            const __m128i v_mean_i2_shuf =
                _mm_shuffle_epi8(v_mean_i2, v_shuf16_i2);
            const __m128i v_sub_thresh = _mm_subs_epu8(v_t1_shuf, v_thresh_8);
            const __m128i v_mask = _mm_cmpeq_epi8(v_sub_thresh, zero);
            // convert to 16bit mask
            const __m128i v_mask_16 = _mm_unpacklo_epi8(v_mask, v_mask);
            const __m128i v_orig_masked = _mm_andnot_si128(v_mask_16, v_orig);
            const __m128i v_mean_masked =
                _mm_and_si128(v_mask_16, v_mean_i2_shuf);
            v_val1 = _mm_or_si128(v_orig_masked, v_mean_masked);
          }

          index3 = ((k + depth) * downHeight + ((i + 1) >> 1)) * downWidth +
              ((j) >> 1);
          {
            const __m128i v_max_i3 =
                _mm_loadl_epi64((__m128i *)&downMax[index3]);
            const __m128i v_min_i3 =
                _mm_loadl_epi64((__m128i *)&downMin[index3]);
            const __m128i v_t1 = _mm_subs_epu8(v_max_i3, v_min_i3);
            const __m128i v_t1_shuf = _mm_shuffle_epi8(v_t1, v_shuf8_i3);
            const __m128i v_mean_i3 =
                _mm_loadu_si128((__m128i *)&downMean[index3]);
            const __m128i v_mean_i3_shuf =
                _mm_shuffle_epi8(v_mean_i3, v_shuf16_i3);
            const __m128i v_sub_thresh = _mm_subs_epu8(v_t1_shuf, v_thresh_8);
            const __m128i v_mask = _mm_cmpeq_epi8(v_sub_thresh, zero);
            // convert to 16bit mask
            const __m128i v_mask_16 = _mm_unpacklo_epi8(v_mask, v_mask);
            const __m128i v_orig_masked = _mm_andnot_si128(v_mask_16, v_orig);
            const __m128i v_mean_masked =
                _mm_and_si128(v_mask_16, v_mean_i3_shuf);
            v_val2 = _mm_or_si128(v_orig_masked, v_mean_masked);
          }
          _mm_storeu_si128((__m128i *)&src_dst_mean[src_dst_index1],
                           _mm_avg_epu16(v_val1, v_val2));
        } // j

        // scalar code for remaining
        for (; j < width; ++j) {
          int originalValue, value1, value2;
          index1 = (k * height + i) * mean_stride + j;
          originalValue = src_dst_mean[index1];

          index2 = (k * downHeight + (i >> 1)) * downWidth +
              ((j + hshift) >> 1);
          if ((downMax[index2] - downMin[index2]) <= thresh){
            value1 = downMean[index2];
          }else{
            value1 = originalValue;
          }

          index2 = ((k + depth) * downHeight + ((i + 1) >> 1)) * downWidth +
              ((j + (1 - hshift)) >> 1);
          if ((downMax[index2] - downMin[index2]) <= thresh){
            value2 = downMean[index2];
          }else{
            value2 = originalValue;
          }

          src_dst_mean[index1] = (value1 + value2 + 1) >> 1;
        }
      }
    }
  }
}

static INLINE void unround(const struct postproc_deband *deband,
                           const uint8_t* src_in, uint16_t* dst_out,
                           int src_stride, int dst_stride,
                           int height, int width,
                           int thresh, int rlimit) {
  uint8_t *downMax = deband->downMax;
  uint8_t *downMin = deband->downMin;
  uint16_t *downMean = deband->downMean;

  convert_to_14(src_in, dst_out, src_stride, dst_stride, height, width);

  BitGen_recur_ssse3(src_in, src_in, dst_out,
                     downMax, downMin, downMean,
                     src_stride, src_stride, dst_stride,
                     1, height, width, thresh, 0, 1, rlimit);
}

void vp9_unround_then_pattern_round_ssse3(const struct postproc_deband *deband,
                                          const uint8_t *src_ptr,
                                          uint8_t *dst_ptr,
                                          int src_stride,
                                          int dst_stride,
                                          int height,
                                          int width,
                                          int thresh,
                                          int rlimit) {
  uint16_t *high = deband->high;
  int high_stride = width;

  unround(deband, src_ptr, high, src_stride, high_stride, height, width, thresh,
          rlimit);
  pattern_round(high, dst_ptr, high_stride, dst_stride, height, width);
}
