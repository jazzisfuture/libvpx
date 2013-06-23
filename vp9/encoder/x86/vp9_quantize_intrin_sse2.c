/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>
#include "vpx_mem/vpx_mem.h"

#include "vp9/encoder/vp9_onyx_int.h"
#include "vp9/encoder/vp9_quantize.h"
#include "vp9/common/vp9_quant_common.h"
#include "vp9/common/vp9_seg_common.h"

#include <emmintrin.h> /* SSE2 */

union simd {
  __m128i m;
  int16_t s[8];
};

void vp9_quantize_sparse_sse2(int16_t *zbin_boost_orig_ptr,
                            int16_t *coeff_ptr, int n_coeffs, int skip_block,
                            int16_t *zbin_ptr, int16_t *round_ptr,
                            int16_t *quant_ptr, uint8_t *quant_shift_ptr,
                            int16_t *qcoeff_ptr, int16_t *dqcoeff_ptr,
                            int16_t *dequant_ptr, int zbin_oq_value,
                            uint16_t *eob_ptr, const int *scan, int mul,
                            int *idx_arr) {
  int i, j, rc, eob;
  int zbins[2], pzbins[2], nzbins[2];
  int z;
  int zero_run = 0;
  int16_t *zbin_boost_ptr = zbin_boost_orig_ptr;
  int idx = 0;
  int pre_idx = 0;
  DECLARE_ALIGNED_ARRAY(16, int16_t, selected_coeff, 256);

  assert(mul == 1);

  vpx_memset(qcoeff_ptr, 0, n_coeffs*sizeof(int16_t));
  vpx_memset(dqcoeff_ptr, 0, n_coeffs*sizeof(int16_t));

  eob = -1;

  // Base ZBIN
  zbins[0] = zbin_ptr[0] + zbin_oq_value;
  zbins[1] = zbin_ptr[1] + zbin_oq_value;
  // Positive and negative ZBIN
  pzbins[0] = zbins[0];
  pzbins[1] = zbins[1];
  nzbins[0] = pzbins[0] * -1;
  nzbins[1] = pzbins[1] * -1;

  if (!skip_block) {
    // Pre-scan pass
    for (i = 0; i < n_coeffs; i++) {
      rc = scan[i];
      z = coeff_ptr[rc];

      // If the coefficient is out of the base ZBIN range, keep it for
      // quantization.
      if (z >= pzbins[rc != 0] || z <= nzbins[rc != 0]) {
        idx_arr[idx] = i;
        selected_coeff[idx] = z;
        idx++;
      }
    }

    if (!idx)
      goto done;

    // SIMD: do 8 coefficients at one time.
    {
      const __m128i round0 = _mm_load_si128((__m128i *)(round_ptr));
      const __m128i round1 = _mm_load_si128((__m128i *)(round_ptr + 8));
      const __m128i quant0 = _mm_load_si128((__m128i *)(quant_ptr));
      const __m128i quant1 = _mm_load_si128((__m128i *)(quant_ptr + 8));
      __m128i zbin0 = _mm_load_si128((__m128i *)(zbin_ptr));
      __m128i zbin1 = _mm_load_si128((__m128i *)(zbin_ptr + 8));
      __m128i zbin_extra = _mm_cvtsi32_si128(zbin_oq_value);
      __m128i quant_shift0;
      __m128i quant_shift1;
      __m128i dequant0;
      __m128i dequant1;
      __m128i sign, x, x_mul, y, z, x_diff;

      // convert quant_shift to multiplication
      int qs0 = 16 - quant_shift_ptr[0];
      int qs1 = 16 - quant_shift_ptr[1];
      union simd x_zbin, qcoeff, dqcoeff, temp;
      int start = 0;
      int end;

      // get quant_shift
      qs0 = 1 << qs0;
      qs1 = 1 << qs1;

      for (i=0; i< 8; i++)
        temp.s[i] = qs1;
      quant_shift1 = temp.m;
      temp.s[0] = qs0;
      quant_shift0 = temp.m;

      // get dequant
      for (i=0; i< 8; i++)
        temp.s[i] = dequant_ptr[1];
      dequant1 = temp.m;
      temp.s[0] = dequant_ptr[0];
      dequant0 = temp.m;

      // calculate fixed zbin part
      zbin_extra = _mm_shufflelo_epi16(zbin_extra, 0);
      zbin_extra = _mm_unpacklo_epi16(zbin_extra, zbin_extra);
      zbin0 = _mm_add_epi16(zbin0, zbin_extra);
      zbin1 = _mm_add_epi16(zbin1, zbin_extra);

      // If first saved coeff is the DC, do the first 8 separately.
      if (idx_arr[0] == 0) {
        i = 0;
        start = 8;

        z = _mm_load_si128((__m128i *)selected_coeff);
        selected_coeff += 8;

        // find the sign
        sign = _mm_srai_epi16(z, 15);

        // get the absolute values.
        x = _mm_xor_si128(z, sign);
        x = _mm_sub_epi16(x, sign);

        // subtract fixed zbin from x_abs
        x_diff = _mm_sub_epi16(x, zbin0);

        x = _mm_add_epi16(x, round0);
        x_mul = _mm_mulhi_epi16(x, quant0);
        x = _mm_add_epi16(x_mul, x);
        x = _mm_mulhi_epi16(x, quant_shift0);

        // get the sign back
        x = _mm_xor_si128(x, sign);
        x = _mm_sub_epi16(x, sign);

        // get dequantized coefficients
        y = _mm_mullo_epi16(x, dequant0);

        // check to see which to keep or discard
        end = i + 8;
        if (i + 8 > idx)
          end = idx;

        x_zbin.m = x_diff;
        qcoeff.m = x;
        dqcoeff.m = y;

        for (j = i; j < end; j++) {
          rc = scan[idx_arr[j]];
          zero_run += idx_arr[j] - pre_idx;
          if (zero_run > 15)
            zero_run = 15;
          pre_idx = idx_arr[j];

          if ((x_zbin.s[j] >= zbin_boost_ptr[zero_run]) && (qcoeff.s[j] != 0)) {
            qcoeff_ptr[rc] = qcoeff.s[j];
            dqcoeff_ptr[rc] = dqcoeff.s[j];
            eob = idx_arr[j];
            zero_run = -1;
          }
        }
      }

      for (i = start; i < idx; i += 8) {
        z = _mm_load_si128((__m128i *)selected_coeff);
        selected_coeff += 8;

        // find the sign
        sign = _mm_srai_epi16(z, 15);

        // get the absolute values
        x = _mm_xor_si128(z, sign);
        x = _mm_sub_epi16(x, sign);

        // subtract fixed zbin from x_abs
        x_diff = _mm_sub_epi16(x, zbin1);

        x = _mm_add_epi16(x, round1);
        x_mul = _mm_mulhi_epi16(x, quant1);
        x = _mm_add_epi16(x_mul, x);
        x = _mm_mulhi_epi16(x, quant_shift1);

        // get the sign back
        x = _mm_xor_si128(x, sign);
        x = _mm_sub_epi16(x, sign);

        // dequantized coefficients
        y = _mm_mullo_epi16(x, dequant1);

        // check to see which to keep or discard
        end = i + 8;
        if (i + 8 > idx)
          end = idx;

        x_zbin.m = x_diff;
        qcoeff.m = x;
        dqcoeff.m = y;

        for (j = i; j < end; j++) {
          int k = j % 8;

          rc = scan[idx_arr[j]];
          zero_run += idx_arr[j] - pre_idx;
          if (zero_run > 15)
            zero_run = 15;
          pre_idx = idx_arr[j];

          if ((x_zbin.s[k] >= zbin_boost_ptr[zero_run]) && (qcoeff.s[k] != 0)) {
            qcoeff_ptr[rc] = qcoeff.s[k];
            dqcoeff_ptr[rc] = dqcoeff.s[k];
            eob = idx_arr[j];
            zero_run = -1;
          }
        }
      }
    }
  }
done:
  *eob_ptr = eob + 1;
}
