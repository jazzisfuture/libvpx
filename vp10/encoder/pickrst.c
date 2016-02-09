/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <limits.h>

#include "./vpx_scale_rtcd.h"

#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"

#include "vp10/common/onyxc_int.h"
#include "vp10/common/quant_common.h"

#include "vp10/encoder/encoder.h"
#include "vp10/encoder/quantize.h"
#include "vp10/encoder/picklpf.h"
#include "vp10/encoder/pickrst.h"

static int try_restoration_frame(const YV12_BUFFER_CONFIG *sd,
                                 VP10_COMP *const cpi,
                                 int restoration_level,
                                 int partial_frame) {
  VP10_COMMON *const cm = &cpi->common;
  int filt_err;
  vp10_loop_restoration_frame(cm->frame_to_show, cm,
                              restoration_level, 1, partial_frame);
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    filt_err = vp10_highbd_get_y_sse(sd, cm->frame_to_show);
  } else {
    filt_err = vp10_get_y_sse(sd, cm->frame_to_show);
  }
#else
  filt_err = vp10_get_y_sse(sd, cm->frame_to_show);
#endif  // CONFIG_VP9_HIGHBITDEPTH

  // Re-instate the unfiltered frame
  vpx_yv12_copy_y(&cpi->last_frame_db, cm->frame_to_show);
  return filt_err;
}

int search_restoration_level(const YV12_BUFFER_CONFIG *sd,
                             VP10_COMP *cpi,
                             int filter_level, int partial_frame,
                             double *best_cost_ret) {
  VP10_COMMON *const cm = &cpi->common;
  int i, restoration_best, err;
  double best_cost;
  double cost;
  const int restoration_level_bits = vp10_restoration_level_bits(&cpi->common);
  const int restoration_levels = 1 << restoration_level_bits;
  MACROBLOCK *x = &cpi->td.mb;
  int bits;

  //  Make a copy of the unfiltered / processed recon buffer
  vpx_yv12_copy_y(cm->frame_to_show, &cpi->last_frame_uf);
  vp10_loop_filter_frame(cm->frame_to_show, cm, &cpi->td.mb.e_mbd, filter_level,
                         1, partial_frame);
  vpx_yv12_copy_y(cm->frame_to_show, &cpi->last_frame_db);

  restoration_best = -1;
  err = try_restoration_frame(sd, cpi, restoration_best, partial_frame);
  bits = 0;
  best_cost = RDCOST_DBL(x->rdmult, x->rddiv, (bits << 2), err);
  for (i = 0; i < restoration_levels; ++i) {
    err = try_restoration_frame(sd, cpi, i, partial_frame);
    // Normally the rate is rate in bits * 256 and dist is sum sq err * 64
    // when RDCOST is used.  However below we just scale both in the correct
    // ratios appropriately but not exactly by these values.
    bits = restoration_level_bits;
    cost = RDCOST_DBL(x->rdmult, x->rddiv, (bits << 2), err);
    if (cost < best_cost) {
      restoration_best = i;
      best_cost = cost;
    }
  }
  if (best_cost_ret) *best_cost_ret = best_cost;
  vpx_yv12_copy_y(&cpi->last_frame_uf, cm->frame_to_show);
  return restoration_best;
}

int search_filter_restoration_level(const YV12_BUFFER_CONFIG *sd,
                                    VP10_COMP *cpi,
                                    int partial_frame,
                                    int *restoration_level,
                                    double *best_cost_ret) {
  const VP10_COMMON *const cm = &cpi->common;
  const struct loopfilter *const lf = &cm->lf;
  const int min_filter_level = 0;
  const int max_filter_level = vp10_get_max_filter_level(cpi);
  int filt_direction = 0;
  int filt_best, restoration_best;
  double best_err;
  int i;
  int rest_lev;

  // Start the search at the previous frame filter level unless it is now out of
  // range.
  int filt_mid = clamp(lf->filter_level, min_filter_level, max_filter_level);
  int filter_step = filt_mid < 16 ? 4 : filt_mid / 4;
  double ss_err[MAX_LOOP_FILTER + 1];

  // Set each entry to -1
  for (i = 0; i <= MAX_LOOP_FILTER; ++i)
    ss_err[i] = -1.0;

  rest_lev = search_restoration_level(sd, cpi, filt_mid,
                                      partial_frame, &best_err);
  filt_best = filt_mid;
  restoration_best = rest_lev;
  ss_err[filt_mid] = best_err;

  while (filter_step > 0) {
    const int filt_high = VPXMIN(filt_mid + filter_step, max_filter_level);
    const int filt_low = VPXMAX(filt_mid - filter_step, min_filter_level);

    // Bias against raising loop filter in favor of lowering it.
    double bias = (best_err / (1 << (15 - (filt_mid / 8)))) * filter_step;

    if ((cpi->oxcf.pass == 2) && (cpi->twopass.section_intra_rating < 20))
      bias = (bias * cpi->twopass.section_intra_rating) / 20;

    // yx, bias less for large block size
    if (cm->tx_mode != ONLY_4X4)
      bias /= 2;

    if (filt_direction <= 0 && filt_low != filt_mid) {
      // Get Low filter error score
      if (ss_err[filt_low] < 0) {
        rest_lev = search_restoration_level(sd, cpi, filt_low,
                                            partial_frame,
                                            &ss_err[filt_low]);
      }
      // If value is close to the best so far then bias towards a lower loop
      // filter value.
      if ((ss_err[filt_low] - bias) < best_err) {
        // Was it actually better than the previous best?
        if (ss_err[filt_low] < best_err) {
          best_err = ss_err[filt_low];
        }

        filt_best = filt_low;
        restoration_best = rest_lev;
      }
    }

    // Now look at filt_high
    if (filt_direction >= 0 && filt_high != filt_mid) {
      if (ss_err[filt_high] < 0) {
        rest_lev = search_restoration_level(
            sd, cpi, filt_high, partial_frame, &ss_err[filt_high]);
      }
      // Was it better than the previous best?
      if (ss_err[filt_high] < (best_err - bias)) {
        best_err = ss_err[filt_high];
        filt_best = filt_high;
        restoration_best = rest_lev;
      }
    }

    // Half the step distance if the best filter value was the same as last time
    if (filt_best == filt_mid) {
      filter_step /= 2;
      filt_direction = 0;
    } else {
      filt_direction = (filt_best < filt_mid) ? -1 : 1;
      filt_mid = filt_best;
    }
  }
  *restoration_level = restoration_best;
  if (best_cost_ret) *best_cost_ret = best_err;
  return filt_best;
}

void vp10_pick_filter_restoration_level(
    const YV12_BUFFER_CONFIG *sd, VP10_COMP *cpi, LPF_PICK_METHOD method) {
  VP10_COMMON *const cm = &cpi->common;
  struct loopfilter *const lf = &cm->lf;

  lf->sharpness_level = cm->frame_type == KEY_FRAME ? 0
                                                    : cpi->oxcf.sharpness;

  if (method == LPF_PICK_MINIMAL_LPF && lf->filter_level) {
      lf->filter_level = 0;
  } else if (method >= LPF_PICK_FROM_Q) {
    const int min_filter_level = 0;
    const int max_filter_level = vp10_get_max_filter_level(cpi);
    const int q = vp10_ac_quant(cm->base_qindex, 0, cm->bit_depth);
    // These values were determined by linear fitting the result of the
    // searched level, filt_guess = q * 0.316206 + 3.87252
#if CONFIG_VP9_HIGHBITDEPTH
    int filt_guess;
    switch (cm->bit_depth) {
      case VPX_BITS_8:
        filt_guess = ROUND_POWER_OF_TWO(q * 20723 + 1015158, 18);
        break;
      case VPX_BITS_10:
        filt_guess = ROUND_POWER_OF_TWO(q * 20723 + 4060632, 20);
        break;
      case VPX_BITS_12:
        filt_guess = ROUND_POWER_OF_TWO(q * 20723 + 16242526, 22);
        break;
      default:
        assert(0 && "bit_depth should be VPX_BITS_8, VPX_BITS_10 "
                    "or VPX_BITS_12");
        return;
    }
#else
    int filt_guess = ROUND_POWER_OF_TWO(q * 20723 + 1015158, 18);
#endif  // CONFIG_VP9_HIGHBITDEPTH
    if (cm->frame_type == KEY_FRAME)
      filt_guess -= 4;
    lf->filter_level = clamp(filt_guess, min_filter_level, max_filter_level);
    cm->rst_info.restoration_level = search_restoration_level(
        sd, cpi, lf->filter_level, method == LPF_PICK_FROM_SUBIMAGE, NULL);
  } else {
    lf->filter_level = search_filter_restoration_level(
        sd, cpi, method == LPF_PICK_FROM_SUBIMAGE,
        &cm->rst_info.restoration_level, NULL);
    /*  Separate search (less efficient but faster)
        lf->filter_level = vp10_search_filter_level(
            sd, cpi, method == LPF_PICK_FROM_SUBIMAGE);
        cm->rst_info.restoration_level = search_restoration_level(
            sd, cpi, lf->filter_level, method == LPF_PICK_FROM_SUBIMAGE, NULL);
    */
  }
  if (cm->rst_info.restoration_level != -1)
    cm->rst_info.restoration_type = RESTORE_BILATERAL;
  else
    cm->rst_info.restoration_type = RESTORE_NONE;
}
