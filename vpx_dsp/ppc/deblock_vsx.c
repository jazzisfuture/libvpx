/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/ppc/types_vsx.h"

extern const int16_t vpx_rv[];

static const uint8x16_t load_merge = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A,
                                       0x0C, 0x0E, 0x18, 0x19, 0x1A, 0x1B,
                                       0x1C, 0x1D, 0x1E, 0x1F };

static const uint8x16_t mask_merge = { 0x00, 0x01, 0x10, 0x11, 0x04, 0x05,
                                       0x14, 0x15, 0x08, 0x09, 0x18, 0x19,
                                       0x0C, 0x0D, 0x1C, 0x1D };

void vpx_mbpost_proc_down_vsx(uint8_t *dst, int pitch, int rows, int cols,
                              int flimit) {
  int col, row, i;
  int16x8_t win[16];
  int32x4_t winsq_even[16];
  int32x4_t winsq_odd[16];
  const int32x4_t lim = vec_splats(flimit);

  // 8 columns are processed at a time.
  assert(cols % 8 == 0);
  // If rows is less than 8 the bottom border extension fails.
  assert(rows >= 8);

  for (col = 0; col < cols; col += 8) {
    // The sum is signed and requires at most 13 bits.
    // (8 bits + sign) * 15 (4 bits)
    int16x8_t r1, sum;
    // The sum of squares requires at most 20 bits.
    // (16 bits + sign) * 15 (4 bits)
    int32x4_t r1sq_even, r1sq_odd, sumsq_even, sumsq_odd;

    r1 = unpack_to_s16_h(vec_vsx_ld(0, dst));
    r1sq_even = vec_mule(r1, r1);
    r1sq_odd = vec_mulo(r1, r1);
    // Fill sliding window with first row.
    for (i = 0; i <= 8; i++) {
      win[i] = r1;
      winsq_even[i] = r1sq_even;
      winsq_odd[i] = r1sq_odd;
    }
    // First 9 rows of the sliding window are the same.
    // sum = r1 * 9
    sum = vec_mladd(r1, vec_splats((int16_t)9), vec_zeros_s16);

    // sumsq = r1 * r1 * 9
    sumsq_even = vec_mule(sum, r1);
    sumsq_odd = vec_mulo(sum, r1);

    // Fill the next 6 rows of the sliding window with rows 2 to 7.
    for (i = 1; i <= 6; ++i) {
      const int16x8_t next_row = unpack_to_s16_h(vec_vsx_ld(i * pitch, dst));
      win[i + 8] = next_row;
      winsq_even[i + 8] = vec_mule(next_row, next_row);
      winsq_odd[i + 8] = vec_mulo(next_row, next_row);
      sum = vec_add(sum, next_row);
      sumsq_even = vec_add(sumsq_even, winsq_even[i + 8]);
      sumsq_odd = vec_add(sumsq_odd, winsq_odd[i + 8]);
    }

    for (row = 0; row < rows; row++) {
      int32x4_t sumsq_odd_scaled, sumsq_even_scaled;
      int32x4_t thres_odd, thres_even;
      bool32x4_t mask_odd, mask_even;
      bool16x8_t mask;
      int16x8_t filtered, masked;
      uint8x16_t out;

      const int16x8_t rv = vec_vsx_ld(0, vpx_rv + (row & 127));

      // Move the sliding window
      if (row + 7 < rows) {
        const int16x8_t next_row =
            unpack_to_s16_h(vec_vsx_ld((row + 7) * pitch, dst));
        win[15] = next_row;
        winsq_even[15] = vec_mule(next_row, next_row);
        winsq_odd[15] = vec_mulo(next_row, next_row);
      } else {
        win[15] = win[14];
        winsq_even[15] = winsq_even[14];
        winsq_odd[15] = winsq_odd[14];
      }

      // C: sum += s[7 * pitch] - s[-8 * pitch];
      sum = vec_add(sum, vec_sub(win[15], win[0]));

      // TODO(ltrudeau) Replace with squared Sliding Window
      // C: sumsq += s[7 * pitch] * s[7 * pitch] - s[-8 * pitch] * s[-8 *
      // pitch];
      sumsq_odd = vec_add(sumsq_odd, vec_sub(winsq_odd[15], winsq_odd[0]));
      sumsq_even = vec_add(sumsq_even, vec_sub(winsq_even[15], winsq_even[0]));

      // C: sumsq * 15 - sum * sum
      sumsq_odd_scaled = vec_mul(sumsq_odd, vec_splats((int32_t)15));
      sumsq_even_scaled = vec_mul(sumsq_even, vec_splats((int32_t)15));
      thres_odd = vec_sub(sumsq_odd_scaled, vec_mulo(sum, sum));
      thres_even = vec_sub(sumsq_even_scaled, vec_mule(sum, sum));

      // C: (vpx_rv[(r & 127) + (c & 7)] + sum + s[0]) >> 4
      filtered = vec_add(vec_add(rv, sum), win[8]);
      filtered = vec_sra(filtered, vec_splats((uint16_t)4));

      mask_odd = vec_cmplt(thres_odd, lim);
      mask_even = vec_cmplt(thres_even, lim);
      mask = vec_perm((bool16x8_t)mask_even, (bool16x8_t)mask_odd, mask_merge);
      masked = vec_sel(win[8], filtered, mask);

      // TODO(ltrudeau) If cols % 16 == 0, we could just process 16 per
      // iteration
      out = vec_perm((uint8x16_t)masked, vec_vsx_ld(0, dst + row * pitch),
                     load_merge);
      vec_vsx_st(out, 0, dst + row * pitch);

      for (i = 1; i < 16; i++) {
        win[i - 1] = win[i];
        winsq_even[i - 1] = winsq_even[i];
        winsq_odd[i - 1] = winsq_odd[i];
      }
    }
    dst += 8;
  }
}
