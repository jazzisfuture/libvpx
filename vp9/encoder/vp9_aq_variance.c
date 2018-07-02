/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>

#include "vpx_ports/mem.h"
#include "vpx_ports/system_state.h"
#include "vp9/encoder/vp9_aq_variance.h"

#include "vp9/common/vp9_seg_common.h"

#include "vp9/encoder/vp9_ratectrl.h"
#include "vp9/encoder/vp9_rd.h"
#include "vp9/encoder/vp9_encodeframe.h"
#include "vp9/encoder/vp9_segmentation.h"

static const double rate_ratio[MAX_SEGMENTS] = { 2.5,  1.8,  1.1, 1.0,
                                                 0.85, 0.75, 0.6, 0.5 };

#define SEGMENT_ID(i) i

DECLARE_ALIGNED(16, static const uint8_t, vp9_64_zeros[64]) = { 0 };
#if CONFIG_VP9_HIGHBITDEPTH
DECLARE_ALIGNED(16, static const uint16_t, vp9_highbd_64_zeros[64]) = { 0 };
#endif

unsigned int vp9_vaq_segment_id(int energy) { return energy; }

void vp9_vaq_frame_setup(VP9_COMP *cpi) {
  VP9_COMMON *cm = &cpi->common;
  struct segmentation *seg = &cm->seg;
  int i;
  // mb_av_eng - 2 because 16x16 variance is on the order of 2x typical 4x4
  // variance.
  int avg_energy = (int)(cpi->twopass.mb_av_energy - 2);
  double avg_ratio;

  if (avg_energy > 7) avg_energy = 7;
  if (avg_energy < 0) avg_energy = 0;
  avg_ratio = rate_ratio[avg_energy];

  if (frame_is_intra_only(cm) || cm->error_resilient_mode ||
      cpi->refresh_alt_ref_frame || cpi->force_update_segmentation ||
      (cpi->refresh_golden_frame && !cpi->rc.is_src_frame_alt_ref)) {
    vp9_enable_segmentation(seg);
    vp9_clearall_segfeatures(seg);

    seg->abs_delta = SEGMENT_DELTADATA;

    vpx_clear_system_state();

    for (i = 0; i < MAX_SEGMENTS; ++i) {
      // Set up avg segment id to be 1.0 and adjust the other segments around
      // it.
      int qindex_delta =
          vp9_compute_qdelta_by_rate(&cpi->rc, cm->frame_type, cm->base_qindex,
                                     rate_ratio[i] / avg_ratio, cm->bit_depth);

      // We don't allow qindex 0 in a segment if the base value is not 0.
      // Q index 0 (lossless) implies 4x4 encoding only and in AQ mode a segment
      // Q delta is sometimes applied without going back around the rd loop.
      // This could lead to an illegal combination of partition size and q.
      if ((cm->base_qindex != 0) && ((cm->base_qindex + qindex_delta) == 0)) {
        qindex_delta = -cm->base_qindex + 1;
      }

      // No need to enable SEG_LVL_ALT_Q for this segment.
      if (rate_ratio[i] == 1.0) {
        continue;
      }

      vp9_set_segdata(seg, i, SEG_LVL_ALT_Q, qindex_delta);
      vp9_enable_segfeature(seg, i, SEG_LVL_ALT_Q);
    }
  }
}

static unsigned int block_lvariance(const VP9_COMP *const cpi, MACROBLOCK *x,
                                    BLOCK_SIZE bs) {
  // This functions returns a score for the blocks local variance as calculated
  // by: sum of the log of the (4x4 variances) of each subblock to the current
  // block (x,bs)
  // * 32 / number of pixels in the block_size.
  // This is used for segmentation because to avoid situations in which a large
  // block with a gentle gradient gets marked high variance even though each
  // subblock has a low variance.   This allows us to assign the same segment
  // number for the same sorts of area regardless of how the partitioning goes.

  MACROBLOCKD *xd = &x->e_mbd;
  double var = 0;
  unsigned int sse;
  int i, j;

  int right_overflow =
      (xd->mb_to_right_edge < 0) ? ((-xd->mb_to_right_edge) >> 3) : 0;
  int bottom_overflow =
      (xd->mb_to_bottom_edge < 0) ? ((-xd->mb_to_bottom_edge) >> 3) : 0;

  const int bw = 8 * num_8x8_blocks_wide_lookup[bs] - right_overflow;
  const int bh = 8 * num_8x8_blocks_high_lookup[bs] - bottom_overflow;

  for (i = 0; i < bh; i += 4) {
    for (j = 0; j < bw; j += 4) {
#if CONFIG_VP9_HIGHBITDEPTH
      if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) {
        var +=
            log(1.0 + cpi->fn_ptr[BLOCK_4X4].vf(
                          x->plane[0].src.buf + i * x->plane[0].src.stride + j,
                          x->plane[0].src.stride,
                          CONVERT_TO_BYTEPTR(vp9_highbd_64_zeros), 0, &sse) /
                          16);
      } else {
#endif
        var +=
            log(1.0 + cpi->fn_ptr[BLOCK_4X4].vf(
                          x->plane[0].src.buf + i * x->plane[0].src.stride + j,
                          x->plane[0].src.stride, vp9_64_zeros, 0, &sse) /
                          16);
#if CONFIG_VP9_HIGHBITDEPTH
      }
#endif
    }
  }
  // Use average of 4x4 log variance. The range for 8 bit 0 - 9.704121561.
  var /= (bw / 4 * bh / 4);
  if (var > 7) var = 7;

  // Convert to integer.
  return (unsigned int)var;
}

double vp9_log_block_var(VP9_COMP *cpi, MACROBLOCK *x, BLOCK_SIZE bs) {
  unsigned int var = block_lvariance(cpi, x, bs);
  vpx_clear_system_state();
  return var;
}

// Get the range of sub block energy values;
void vp9_get_sub_block_energy(VP9_COMP *cpi, MACROBLOCK *mb, int mi_row,
                              int mi_col, BLOCK_SIZE bsize, int *min_e,
                              int *max_e) {
  VP9_COMMON *const cm = &cpi->common;
  const int bw = num_8x8_blocks_wide_lookup[bsize];
  const int bh = num_8x8_blocks_high_lookup[bsize];
  const int xmis = VPXMIN(cm->mi_cols - mi_col, bw);
  const int ymis = VPXMIN(cm->mi_rows - mi_row, bh);
  int x, y;

  if (xmis < bw || ymis < bh) {
    vp9_setup_src_planes(mb, cpi->Source, mi_row, mi_col);
    *min_e = vp9_block_energy(cpi, mb, bsize);
    *max_e = *min_e;
  } else {
    int energy;
    *min_e = ENERGY_MAX;
    *max_e = ENERGY_MIN;

    for (y = 0; y < ymis; ++y) {
      for (x = 0; x < xmis; ++x) {
        vp9_setup_src_planes(mb, cpi->Source, mi_row + y, mi_col + x);
        energy = vp9_block_energy(cpi, mb, BLOCK_8X8);
        *min_e = VPXMIN(*min_e, energy);
        *max_e = VPXMAX(*max_e, energy);
      }
    }
  }

  // Re-instate source pointers back to what they should have been on entry.
  vp9_setup_src_planes(mb, cpi->Source, mi_row, mi_col);
}

#define DEFAULT_E_MIDPOINT 10.0
int vp9_block_energy(VP9_COMP *cpi, MACROBLOCK *x, BLOCK_SIZE bs) {
  vpx_clear_system_state();
  return block_lvariance(cpi, x, bs);
}
