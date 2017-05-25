/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp8/encoder/skin_detection.h"

#define MODEL_MODE 1

// Fixed point implementation of a skin color classifier. Skin color
// is model by a Gaussian distribution in the CbCr color space.
// See ../../test/skin_color_detector_test.cc where the reference
// skin color classifier is defined.

// Fixed-point skin color model parameters.
static const int skin_mean[5][2] = { { 7463, 9614 },
                                     { 6400, 10240 },
                                     { 7040, 10240 },
                                     { 8320, 9280 },
                                     { 6800, 9614 } };
static const int skin_inv_cov[4] = { 4107, 1663, 1663, 2157 };  // q16
static const int skin_threshold[6] = { 1570636, 1400000, 800000,
                                       800000,  800000,  800000 };  // q18

// Evaluates the Mahalanobis distance measure for the input CbCr values.
static int evaluate_skin_color_difference(int cb, int cr, int idx) {
  const int cb_q6 = cb << 6;
  const int cr_q6 = cr << 6;
  const int cb_diff_q12 =
      (cb_q6 - skin_mean[idx][0]) * (cb_q6 - skin_mean[idx][0]);
  const int cbcr_diff_q12 =
      (cb_q6 - skin_mean[idx][0]) * (cr_q6 - skin_mean[idx][1]);
  const int cr_diff_q12 =
      (cr_q6 - skin_mean[idx][1]) * (cr_q6 - skin_mean[idx][1]);
  const int cb_diff_q2 = (cb_diff_q12 + (1 << 9)) >> 10;
  const int cbcr_diff_q2 = (cbcr_diff_q12 + (1 << 9)) >> 10;
  const int cr_diff_q2 = (cr_diff_q12 + (1 << 9)) >> 10;
  const int skin_diff =
      skin_inv_cov[0] * cb_diff_q2 + skin_inv_cov[1] * cbcr_diff_q2 +
      skin_inv_cov[2] * cbcr_diff_q2 + skin_inv_cov[3] * cr_diff_q2;
  return skin_diff;
}

// Checks if the input yCbCr values corresponds to skin color.
int is_skin_color(int y, int cb, int cr, int consec_zeromv) {
  if (y < 40 || y > 220) {
    return 0;
  } else {
    if (MODEL_MODE == 0) {
      return (evaluate_skin_color_difference(cb, cr, 0) < skin_threshold[0]);
    } else {
      int i = 0;
      // No skin if block has been zero motion for long consecutive time.
      if (consec_zeromv > 60) return 0;
      // Exit on grey.
      if (cb == 128 && cr == 128) return 0;
      // Exit on very strong cb.
      if (cb > 150 && cr < 110) return 0;
      for (; i < 5; ++i) {
        int skin_color_diff = evaluate_skin_color_difference(cb, cr, i);
        if (skin_color_diff < skin_threshold[i + 1]) {
          if (y < 60 && skin_color_diff > 3 * (skin_threshold[i + 1] >> 2)) {
            return 0;
          } else if (consec_zeromv > 25 &&
                     skin_color_diff > (skin_threshold[i + 1] >> 1)) {
            return 0;
          } else {
            return 1;
          }
        }
        // Exit if difference is much large than the threshold.
        if (skin_color_diff > (skin_threshold[i + 1] << 3)) {
          return 0;
        }
      }
      return 0;
    }
  }
}
