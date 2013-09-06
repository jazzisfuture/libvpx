/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

void QUANTIZE_B(int16_t *coeff_ptr, intptr_t n_coeffs, int skip_block,
                int16_t *zbin_ptr, int16_t *round_ptr, int16_t *quant_ptr,
                int16_t *quant_shift_ptr, int16_t *qcoeff_ptr,
                int16_t *dqcoeff_ptr, int16_t *dequant_ptr,
                int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan,
                const int16_t *iscan) {
  int i, rc, eob;
  int zbins[2], nzbins[2], zbin;
  int x, y, z, sz;
  int zero_flag = n_coeffs;

  vpx_memset(qcoeff_ptr, 0, n_coeffs*sizeof(int16_t));
  vpx_memset(dqcoeff_ptr, 0, n_coeffs*sizeof(int16_t));

  eob = -1;

  // Base ZBIN
  zbins[0] = zbin_ptr[0] + zbin_oq_value;
  zbins[1] = zbin_ptr[1] + zbin_oq_value;
  nzbins[0] = zbins[0] * -1;
  nzbins[1] = zbins[1] * -1;

  if (!skip_block) {
    // Pre-scan pass
    for (i = n_coeffs - 1; i >= 0; i--) {
      rc = scan[i];
      z = coeff_ptr[rc];

      if (z < zbins[rc != 0] && z > nzbins[rc != 0]) {
        zero_flag--;
      } else {
        break;
      }
    }

    // Quantization pass: All coefficients with index >= zero_flag are
    // skippable. Note: zero_flag can be zero.
    for (i = 0; i < zero_flag; i++) {
      rc = scan[i];
      z  = coeff_ptr[rc];

      zbin = (zbins[rc != 0]);

      sz = (z >> 31);                               // sign of z
      x  = (z ^ sz) - sz;

      if (x >= zbin) {
        x += (round_ptr[rc != 0]);
#if CLAMP_FLAG
        x  = clamp(x, INT16_MIN, INT16_MAX);
#endif
        y  = (((int)(((int)(x * quant_ptr[rc != 0]) >> 16) + x)) *
              quant_shift_ptr[rc != 0]) >> 16;      // quantize (x)
        x  = (y ^ sz) - sz;                         // get the sign back
        qcoeff_ptr[rc]  = x;                        // write to destination
        dqcoeff_ptr[rc] = x * dequant_ptr[rc != 0];  // dequantized value

        if (y) {
          eob = i;                                  // last nonzero coeffs
        }
      }
    }
  }
  *eob_ptr = eob + 1;
}
