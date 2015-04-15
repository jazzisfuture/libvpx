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

#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_loopfilter.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_quant_common.h"

#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_picklpf.h"
#include "vp9/encoder/vp9_quantize.h"

static int get_max_filter_level(const VP9_COMP *cpi) {
  if (cpi->oxcf.pass == 2) {
    return cpi->twopass.section_intra_rating > 8 ? MAX_LOOP_FILTER * 3 / 4
                                                 : MAX_LOOP_FILTER;
  } else {
    return MAX_LOOP_FILTER;
  }
}

static int try_filter_frame(const YV12_BUFFER_CONFIG *sd, VP9_COMP *const cpi,
                            int filt_level,
                            int partial_frame) {
  VP9_COMMON *const cm = &cpi->common;
  int filt_err;

  vp9_loop_filter_frame(cm->frame_to_show, cm, &cpi->mb.e_mbd, filt_level,
                        1, partial_frame);
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    filt_err = vp9_highbd_get_y_sse(sd, cm->frame_to_show, cm->bit_depth);
  } else {
    filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
  }
#else
  filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
#endif  // CONFIG_VP9_HIGHBITDEPTH

  // Re-instate the unfiltered frame
  vpx_yv12_copy_y(&cpi->last_frame_uf, cm->frame_to_show);

  return filt_err;
}

#if CONFIG_LOOP_POSTFILTER
#define JOINT_FILTER_BILATERAL_SEARCH
static int try_bilateral_frame(const YV12_BUFFER_CONFIG *sd,
                               VP9_COMP *const cpi,
                               int bilateral_level,
                               int partial_frame) {
  VP9_COMMON *const cm = &cpi->common;
  int filt_err;
  vp9_loop_bilateral_frame(cm->frame_to_show, cm,
                           bilateral_level, 1, partial_frame);
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    filt_err = vp9_highbd_get_y_sse(sd, cm->frame_to_show, cm->bit_depth);
  } else {
    filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
  }
#else
  filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
#endif  // CONFIG_VP9_HIGHBITDEPTH

  // Re-instate the unfiltered frame
  vpx_yv12_copy_y(&cpi->last_frame_db, cm->frame_to_show);

  return filt_err;
}

static int try_filter_bilateral_frame(const YV12_BUFFER_CONFIG *sd,
                                      VP9_COMP *const cpi,
                                      int filt_level,
                                      int bilateral_level,
                                      int partial_frame) {
  VP9_COMMON *const cm = &cpi->common;
  int filt_err;

  vp9_loop_filter_gen_frame(cm->frame_to_show, cm, &cpi->mb.e_mbd, filt_level,
                            bilateral_level, 0, 1, partial_frame);
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    filt_err = vp9_highbd_get_y_sse(sd, cm->frame_to_show, cm->bit_depth);
  } else {
    filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
  }
#else
  filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
#endif  // CONFIG_VP9_HIGHBITDEPTH

  // Re-instate the unfiltered frame
  vpx_yv12_copy_y(&cpi->last_frame_uf, cm->frame_to_show);

  return filt_err;
}

#define USE_RD_LOOP_POSTFILTER_SEARCH
static int64_t search_bilateral_level(const YV12_BUFFER_CONFIG *sd,
                                      VP9_COMP *cpi,
                                      int filter_level, int partial_frame,
                                      int64_t *best_cost_ret) {
  VP9_COMMON *const cm = &cpi->common;
  int i, bilateral_best, err;
  int64_t best_cost;
  int64_t cost[BILATERAL_LEVELS_KF];
  const int bilateral_level_bits = vp9_bilateral_level_bits(&cpi->common);
  const int bilateral_levels = 1 << bilateral_level_bits;
#ifdef USE_RD_LOOP_POSTFILTER_SEARCH
  MACROBLOCK *x = &cpi->mb;
#endif

  //  Make a copy of the unfiltered / processed recon buffer
  vpx_yv12_copy_y(cm->frame_to_show, &cpi->last_frame_uf);
  vp9_loop_filter_frame(cm->frame_to_show, cm, &cpi->mb.e_mbd, filter_level,
                        1, partial_frame);
  vpx_yv12_copy_y(cm->frame_to_show, &cpi->last_frame_db);

  bilateral_best = 0;
  err = try_bilateral_frame(sd, cpi, 0, partial_frame);
#ifdef USE_RD_LOOP_POSTFILTER_SEARCH
  cost[0] = RDCOST(x->rdmult, x->rddiv, 0, err);
#else
  cost[0] = err;
#endif
  best_cost = cost[0];
  for (i = 1; i <= bilateral_levels; ++i) {
    err = try_bilateral_frame(sd, cpi, i, partial_frame);
#ifdef USE_RD_LOOP_POSTFILTER_SEARCH
    // Normally the rate is rate in bits * 256 and dist is sum sq err * 64
    // when RDCOST is used.  However below we just scale both in the correct
    // ratios appropriately but not exactly by these values.
    cost[i] = RDCOST(x->rdmult, x->rddiv,
                     bilateral_level_bits << 2, err);
#else
    cost[i] = err;
#endif
    if (cost[i] < best_cost) {
      bilateral_best = i;
      best_cost = cost[i];
    }
  }
  if (best_cost_ret) *best_cost_ret = best_cost;
  vpx_yv12_copy_y(&cpi->last_frame_uf, cm->frame_to_show);
  return bilateral_best;
}

#ifdef JOINT_FILTER_BILATERAL_SEARCH
static int search_filter_bilateral_level(const YV12_BUFFER_CONFIG *sd,
                                         VP9_COMP *cpi,
                                         int partial_frame,
                                         int *bilateral_level) {
  const VP9_COMMON *const cm = &cpi->common;
  const struct loopfilter *const lf = &cm->lf;
  const int min_filter_level = 0;
  const int max_filter_level = get_max_filter_level(cpi);
  int filt_direction = 0;
  int filt_best, bilateral_best;
  int64_t best_err;

  // Start the search at the previous frame filter level unless it is now out of
  // range.
  int filt_mid = clamp(lf->filter_level, min_filter_level, max_filter_level);
  int filter_step = filt_mid < 16 ? 4 : filt_mid / 4;
  // Sum squared error at each filter level
  int64_t ss_err[MAX_LOOP_FILTER + 1];
  int bilateral;

  // Set each entry to -1
  vpx_memset(ss_err, 0xFF, sizeof(ss_err));

  bilateral = search_bilateral_level(sd, cpi, filt_mid,
                                     partial_frame, &best_err);
  filt_best = filt_mid;
  bilateral_best = bilateral;
  ss_err[filt_mid] = best_err;

  while (filter_step > 0) {
    const int filt_high = MIN(filt_mid + filter_step, max_filter_level);
    const int filt_low = MAX(filt_mid - filter_step, min_filter_level);

    // Bias against raising loop filter in favor of lowering it.
    int64_t bias = (best_err >> (15 - (filt_mid / 8))) * filter_step;

    if ((cpi->oxcf.pass == 2) && (cpi->twopass.section_intra_rating < 20))
      bias = (bias * cpi->twopass.section_intra_rating) / 20;

    // yx, bias less for large block size
    if (cm->tx_mode != ONLY_4X4)
      bias >>= 1;

    if (filt_direction <= 0 && filt_low != filt_mid) {
      // Get Low filter error score
      if (ss_err[filt_low] < 0) {
        bilateral = search_bilateral_level(sd, cpi, filt_low,
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
        bilateral_best = bilateral;
      }
    }

    // Now look at filt_high
    if (filt_direction >= 0 && filt_high != filt_mid) {
      if (ss_err[filt_high] < 0) {
        bilateral = search_bilateral_level(sd, cpi, filt_high, partial_frame,
                                           &ss_err[filt_high]);
      }
      // Was it better than the previous best?
      if (ss_err[filt_high] < (best_err - bias)) {
        best_err = ss_err[filt_high];
        filt_best = filt_high;
        bilateral_best = bilateral;
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
  *bilateral_level = bilateral_best;
  return filt_best;
}
#endif

// Returns used (1) or not used (0)
static int search_filter5tap(const YV12_BUFFER_CONFIG *sd, VP9_COMP *cpi,
                             int filter_level, int bilateral_level,
                             int partial_frame) {
  VP9_COMMON *const cm = &cpi->common;
  int filt_err, filt_err5, used;
#ifdef USE_RD_LOOP_POSTFILTER_SEARCH
  MACROBLOCK *x = &cpi->mb;
#endif
  //  Make a copy of the unfiltered / processed recon buffer
  vpx_yv12_copy_y(cm->frame_to_show, &cpi->last_frame_uf);

  vp9_loop_filter_gen_frame(cm->frame_to_show, cm, &cpi->mb.e_mbd, filter_level,
                            bilateral_level, 0, 1, partial_frame);
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    filt_err = vp9_highbd_get_y_sse(sd, cm->frame_to_show, cm->bit_depth);
  } else {
    filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
  }
#else
  filt_err = vp9_get_y_sse(sd, cm->frame_to_show);
#endif  // CONFIG_VP9_HIGHBITDEPTH

  if (vp9_estimate_filter5tap(cm->frame_to_show, sd, cm->lf.filter5tap)) {
    vp9_loop_filter5tap_frame(cm->frame_to_show, cm, 1, 1, partial_frame);
#if CONFIG_VP9_HIGHBITDEPTH
    if (cm->use_highbitdepth) {
      filt_err5 = vp9_highbd_get_y_sse(sd, cm->frame_to_show, cm->bit_depth);
    } else {
      filt_err5 = vp9_get_y_sse(sd, cm->frame_to_show);
    }
#else
    filt_err5 = vp9_get_y_sse(sd, cm->frame_to_show);
#endif  // CONFIG_VP9_HIGHBITDEPTH
#ifdef USE_RD_LOOP_POSTFILTER_SEARCH
    used = (RDCOST(x->rdmult, x->rddiv, FILTER5TAP_LEVEL_BITS << 4, filt_err5) <
            RDCOST(x->rdmult, x->rddiv, 0, filt_err));
#else
    used = (filt_err5 < filt_err);
#endif
  } else {
    used = 0;
  }

  // Re-instate the unfiltered frame
  vpx_yv12_copy_y(&cpi->last_frame_uf, cm->frame_to_show);
  return used;
}
#endif  // CONFIG_LOOP_POSTFILTER

static int search_filter_level(const YV12_BUFFER_CONFIG *sd, VP9_COMP *cpi,
                               int partial_frame) {
  const VP9_COMMON *const cm = &cpi->common;
  const struct loopfilter *const lf = &cm->lf;
  const int min_filter_level = 0;
  const int max_filter_level = get_max_filter_level(cpi);
  int filt_direction = 0;
  int best_err, filt_best;

  // Start the search at the previous frame filter level unless it is now out of
  // range.
  int filt_mid = clamp(lf->filter_level, min_filter_level, max_filter_level);
  int filter_step = filt_mid < 16 ? 4 : filt_mid / 4;
  // Sum squared error at each filter level
  int ss_err[MAX_LOOP_FILTER + 1];

  // Set each entry to -1
  vpx_memset(ss_err, 0xFF, sizeof(ss_err));

  //  Make a copy of the unfiltered / processed recon buffer
  vpx_yv12_copy_y(cm->frame_to_show, &cpi->last_frame_uf);

  best_err = try_filter_frame(sd, cpi, filt_mid,
                              partial_frame);
  filt_best = filt_mid;
  ss_err[filt_mid] = best_err;

  while (filter_step > 0) {
    const int filt_high = MIN(filt_mid + filter_step, max_filter_level);
    const int filt_low = MAX(filt_mid - filter_step, min_filter_level);

    // Bias against raising loop filter in favor of lowering it.
    int bias = (best_err >> (15 - (filt_mid / 8))) * filter_step;

    if ((cpi->oxcf.pass == 2) && (cpi->twopass.section_intra_rating < 20))
      bias = (bias * cpi->twopass.section_intra_rating) / 20;

    // yx, bias less for large block size
    if (cm->tx_mode != ONLY_4X4)
      bias >>= 1;

    if (filt_direction <= 0 && filt_low != filt_mid) {
      // Get Low filter error score
      if (ss_err[filt_low] < 0) {
        ss_err[filt_low] = try_filter_frame(sd, cpi, filt_low,
                                            partial_frame);
      }
      // If value is close to the best so far then bias towards a lower loop
      // filter value.
      if ((ss_err[filt_low] - bias) < best_err) {
        // Was it actually better than the previous best?
        if (ss_err[filt_low] < best_err)
          best_err = ss_err[filt_low];

        filt_best = filt_low;
      }
    }

    // Now look at filt_high
    if (filt_direction >= 0 && filt_high != filt_mid) {
      if (ss_err[filt_high] < 0) {
        ss_err[filt_high] = try_filter_frame(sd, cpi, filt_high,
                                             partial_frame);
      }
      // Was it better than the previous best?
      if (ss_err[filt_high] < (best_err - bias)) {
        best_err = ss_err[filt_high];
        filt_best = filt_high;
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
  return filt_best;
}

void vp9_pick_filter_level(const YV12_BUFFER_CONFIG *sd, VP9_COMP *cpi,
                           LPF_PICK_METHOD method) {
  VP9_COMMON *const cm = &cpi->common;
  struct loopfilter *const lf = &cm->lf;

  lf->sharpness_level = cm->frame_type == KEY_FRAME ? 0
                                                    : cpi->oxcf.sharpness;

  if (method == LPF_PICK_MINIMAL_LPF && lf->filter_level) {
      lf->filter_level = 0;
  } else if (method >= LPF_PICK_FROM_Q) {
    const int min_filter_level = 0;
    const int max_filter_level = get_max_filter_level(cpi);
    const int q = vp9_ac_quant(cm->base_qindex, 0, cm->bit_depth);
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
#if CONFIG_LOOP_POSTFILTER
    lf->bilateral_level = search_bilateral_level(
        sd, cpi, lf->filter_level, method == LPF_PICK_FROM_SUBIMAGE, NULL);
#endif  // CONFIG_LOOP_POSTFILTER
  } else {
#if CONFIG_LOOP_POSTFILTER
#ifdef JOINT_FILTER_BILATERAL_SEARCH
    lf->filter_level = search_filter_bilateral_level(
        sd, cpi, method == LPF_PICK_FROM_SUBIMAGE, &lf->bilateral_level);
#else
    lf->filter_level = search_filter_level(
        sd, cpi, method == LPF_PICK_FROM_SUBIMAGE);
    lf->bilateral_level = search_bilateral_level(
        sd, cpi, lf->filter_level, method == LPF_PICK_FROM_SUBIMAGE, NULL);
#endif  // JOINT_FILTER_BILATERAL_SEARCH
#ifdef USE_FILTER5TAP
    lf->filter5tap_used = search_filter5tap(sd, cpi,
                                            lf->filter_level,
                                            lf->bilateral_level,
                                            method == LPF_PICK_FROM_SUBIMAGE);
#else
    lf->filter5tap_used = 0;
#endif  // USE_FILTER5TAP
    /*
    if (lf->filter5tap_used)
      printf("Frame %d: %d %d %d %d %d\n", cm->current_video_frame,
             lf->filter5tap[0], lf->filter5tap[1], lf->filter5tap[2],
             lf->filter5tap[3], lf->filter5tap[4]);
             */
#else
    lf->filter_level = search_filter_level(
        sd, cpi, method == LPF_PICK_FROM_SUBIMAGE);
#endif  // CONFIG_LOOP_POSTFILTER
  }
}
