/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"

#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_entropy.h"
#if CONFIG_COEFFICIENT_RANGE_CHECKING
#include "vp9/common/vp9_idct.h"
#endif

#include "vp9/decoder/vp9_detokenize.h"

#define EOB_CONTEXT_NODE 0
#define ZERO_CONTEXT_NODE 1
#define ONE_CONTEXT_NODE 2

#define INCREMENT_COUNT(token)                   \
  do {                                           \
    if (counts) ++coef_counts[band][ctx][token]; \
  } while (0)

#define READ_COEFF(v, probs, n, r) \
  val = 0;                         \
  for (i = 0; i < n; ++i) {        \
    VPX_READ_BIT(r, probs[i]);     \
    val = val << 1;                \
    if (VPX_READ_SET) {            \
      val |= 1;                    \
      VPX_READ_ADJUST_FOR_ONE(r);  \
    } else {                       \
      VPX_READ_ADJUST_FOR_ZERO     \
    }                              \
  }                                \
  val += v;

static int decode_coefs(const MACROBLOCKD *xd, PLANE_TYPE type,
                        tran_low_t *dqcoeff, TX_SIZE tx_size, const int16_t *dq,
                        int ctx, const int16_t *scan, const int16_t *nb,
                        vpx_reader *r) {
  FRAME_COUNTS *counts = xd->counts;
  const int max_eob = 16 << (tx_size << 1);
  const FRAME_CONTEXT *const fc = xd->fc;
  const int ref = is_inter_block(xd->mi[0]);
  int band, c = 0;
  const vpx_prob(*coef_probs)[COEFF_CONTEXTS][UNCONSTRAINED_NODES] =
      fc->coef_probs[tx_size][type][ref];
  const vpx_prob *prob;
  unsigned int(*coef_counts)[COEFF_CONTEXTS][UNCONSTRAINED_NODES + 1];
  unsigned int(*eob_branch_count)[COEFF_CONTEXTS];
  uint8_t token_cache[32 * 32];
  const uint8_t *band_translate = get_band_translate(tx_size);
  const int dq_shift = (tx_size == TX_32X32);
  int v, i;
  int16_t dqv = dq[0];
  const uint8_t *const cat6_prob =
#if CONFIG_VP9_HIGHBITDEPTH
      (xd->bd == VPX_BITS_12)
          ? vp9_cat6_prob_high12
          : (xd->bd == VPX_BITS_10) ? vp9_cat6_prob_high12 + 2 :
#endif  // CONFIG_VP9_HIGHBITDEPTH
                                    vp9_cat6_prob;
  const int cat6_bits =
#if CONFIG_VP9_HIGHBITDEPTH
      (xd->bd == VPX_BITS_12) ? 18 : (xd->bd == VPX_BITS_10) ? 16 :
#endif  // CONFIG_VP9_HIGHBITDEPTH
                                                             14;

  VPX_READ_VARS(r)

  if (counts) {
    coef_counts = counts->coef[tx_size][type][ref];
    eob_branch_count = counts->eob_branch[tx_size][type][ref];
  }

  while (c < max_eob) {
    int val = -1;
    band = *band_translate++;
    prob = coef_probs[band][ctx];
    if (counts) ++eob_branch_count[band][ctx];

    VPX_READ_BIT(r, prob[EOB_CONTEXT_NODE]);
    if (!VPX_READ_SET) {
      VPX_READ_ADJUST_FOR_ZERO
      INCREMENT_COUNT(EOB_MODEL_TOKEN);
      break;
    }
    VPX_READ_ADJUST_FOR_ONE(r)
    while (1) {
      VPX_READ_BIT(r, prob[ZERO_CONTEXT_NODE])
      if (VPX_READ_SET) {
        VPX_READ_ADJUST_FOR_ONE(r)
        break;
      }
      VPX_READ_ADJUST_FOR_ZERO
      INCREMENT_COUNT(ZERO_TOKEN);
      dqv = dq[1];
      token_cache[scan[c]] = 0;
      ++c;
      if (c >= max_eob) {
        VPX_READ_STORE(r)
        return c;  // zero tokens at the end (no eob token)
      }
      ctx = get_coef_context(nb, token_cache, c);
      band = *band_translate++;
      prob = coef_probs[band][ctx];
    }
    VPX_READ_BIT(r, prob[ONE_CONTEXT_NODE])
    if (VPX_READ_SET) {
      const vpx_prob *p = vp9_pareto8_full[prob[PIVOT_NODE] - 1];
      VPX_READ_ADJUST_FOR_ONE(r)
      INCREMENT_COUNT(TWO_TOKEN);
      VPX_READ_BIT(r, p[0]);
      if (VPX_READ_SET) {
        VPX_READ_ADJUST_FOR_ONE(r);
        VPX_READ_BIT(r, p[3]);
        if (VPX_READ_SET) {
          token_cache[scan[c]] = 5;
          VPX_READ_ADJUST_FOR_ONE(r);
          VPX_READ_BIT(r, p[5]);
          if (VPX_READ_SET) {
            VPX_READ_ADJUST_FOR_ONE(r);
            VPX_READ_BIT(r, p[7]);
            if (VPX_READ_SET) {
              VPX_READ_ADJUST_FOR_ONE(r);
              READ_COEFF(CAT6_MIN_VAL, cat6_prob, cat6_bits, r);
            } else {
              VPX_READ_ADJUST_FOR_ZERO
              READ_COEFF(CAT5_MIN_VAL, vp9_cat5_prob, 5, r);
            }
          } else {
            VPX_READ_ADJUST_FOR_ZERO
            VPX_READ_BIT(r, p[6]);
            if (VPX_READ_SET) {
              VPX_READ_ADJUST_FOR_ONE(r);
              READ_COEFF(CAT4_MIN_VAL, vp9_cat4_prob, 4, r);
            } else {
              VPX_READ_ADJUST_FOR_ZERO
              READ_COEFF(CAT3_MIN_VAL, vp9_cat3_prob, 3, r);
            }
          }
        } else {
          token_cache[scan[c]] = 4;
          VPX_READ_ADJUST_FOR_ZERO
          VPX_READ_BIT(r, p[4]);
          if (VPX_READ_SET) {
            VPX_READ_ADJUST_FOR_ONE(r);
            READ_COEFF(CAT2_MIN_VAL, vp9_cat2_prob, 2, r);
          } else {
            VPX_READ_ADJUST_FOR_ZERO
            READ_COEFF(CAT1_MIN_VAL, vp9_cat1_prob, 1, r);
          }
        }
      } else {
        VPX_READ_ADJUST_FOR_ZERO
        VPX_READ_BIT(r, p[1]);
        if (VPX_READ_SET) {
          token_cache[scan[c]] = 3;
          VPX_READ_ADJUST_FOR_ONE(r);
          VPX_READ_BIT(r, p[2]);
          if (VPX_READ_SET) {
            VPX_READ_ADJUST_FOR_ONE(r);
            val = 4;
          } else {
            VPX_READ_ADJUST_FOR_ZERO
            val = 3;
          }
        } else {
          VPX_READ_ADJUST_FOR_ZERO
          token_cache[scan[c]] = 2;
          val = 2;
        }
      }
      v = (val * dqv) >> dq_shift;
    } else {
      VPX_READ_ADJUST_FOR_ZERO
      token_cache[scan[c]] = 1;
      INCREMENT_COUNT(ONE_TOKEN);
      v = dqv >> dq_shift;
    }
    VPX_READ_BIT(r, 128)
#if CONFIG_COEFFICIENT_RANGE_CHECKING
#if CONFIG_VP9_HIGHBITDEPTH
    if (VPX_READ_SET) {
      VPX_READ_ADJUST_FOR_ONE(r)
      dqcoeff[scan[c]] = highbd_check_range(-v, cm->bit_depth);
    } else {
      VPX_READ_ADJUST_FOR_ZERO
      dqcoeff[scan[c]] = highbd_check_range(v, cm->bit_depth);
    }
#else
    if (VPX_READ_SET) {
      VPX_READ_ADJUST_FOR_ONE(r)
      dqcoeff[scan[c]] = check_range(-v, cm->bit_depth);
    } else {
      VPX_READ_ADJUST_FOR_ZERO
      dqcoeff[scan[c]] = check_range(v, cm->bit_depth);
    }
#endif  // CONFIG_VP9_HIGHBITDEPTH
#else
    if (VPX_READ_SET) {
      VPX_READ_ADJUST_FOR_ONE(r)
      dqcoeff[scan[c]] = -v;
    } else {
      VPX_READ_ADJUST_FOR_ZERO
      dqcoeff[scan[c]] = v;
    }
#endif  // CONFIG_COEFFICIENT_RANGE_CHECKING
    ++c;
    ctx = get_coef_context(nb, token_cache, c);
    dqv = dq[1];
  }
  VPX_READ_STORE(r)

  return c;
}

static void get_ctx_shift(MACROBLOCKD *xd, int *ctx_shift_a, int *ctx_shift_l,
                          int x, int y, unsigned int tx_size_in_blocks) {
  if (xd->max_blocks_wide) {
    if (tx_size_in_blocks + x > xd->max_blocks_wide)
      *ctx_shift_a = (tx_size_in_blocks - (xd->max_blocks_wide - x)) * 8;
  }
  if (xd->max_blocks_high) {
    if (tx_size_in_blocks + y > xd->max_blocks_high)
      *ctx_shift_l = (tx_size_in_blocks - (xd->max_blocks_high - y)) * 8;
  }
}

int vp9_decode_block_tokens(TileWorkerData *twd, int plane,
                            const scan_order *sc, int x, int y, TX_SIZE tx_size,
                            int seg_id) {
  vpx_reader *r = &twd->bit_reader;
  MACROBLOCKD *xd = &twd->xd;
  struct macroblockd_plane *const pd = &xd->plane[plane];
  const int16_t *const dequant = pd->seg_dequant[seg_id];
  int eob;
  ENTROPY_CONTEXT *a = pd->above_context + x;
  ENTROPY_CONTEXT *l = pd->left_context + y;
  int ctx;
  int ctx_shift_a = 0;
  int ctx_shift_l = 0;

  switch (tx_size) {
    case TX_4X4:
      ctx = a[0] != 0;
      ctx += l[0] != 0;
      eob = decode_coefs(xd, get_plane_type(plane), pd->dqcoeff, tx_size,
                         dequant, ctx, sc->scan, sc->neighbors, r);
      a[0] = l[0] = (eob > 0);
      break;
    case TX_8X8:
      get_ctx_shift(xd, &ctx_shift_a, &ctx_shift_l, x, y, 1 << TX_8X8);
      ctx = !!*(const uint16_t *)a;
      ctx += !!*(const uint16_t *)l;
      eob = decode_coefs(xd, get_plane_type(plane), pd->dqcoeff, tx_size,
                         dequant, ctx, sc->scan, sc->neighbors, r);
      *(uint16_t *)a = ((eob > 0) * 0x0101) >> ctx_shift_a;
      *(uint16_t *)l = ((eob > 0) * 0x0101) >> ctx_shift_l;
      break;
    case TX_16X16:
      get_ctx_shift(xd, &ctx_shift_a, &ctx_shift_l, x, y, 1 << TX_16X16);
      ctx = !!*(const uint32_t *)a;
      ctx += !!*(const uint32_t *)l;
      eob = decode_coefs(xd, get_plane_type(plane), pd->dqcoeff, tx_size,
                         dequant, ctx, sc->scan, sc->neighbors, r);
      *(uint32_t *)a = ((eob > 0) * 0x01010101) >> ctx_shift_a;
      *(uint32_t *)l = ((eob > 0) * 0x01010101) >> ctx_shift_l;
      break;
    case TX_32X32:
      get_ctx_shift(xd, &ctx_shift_a, &ctx_shift_l, x, y, 1 << TX_32X32);
      // NOTE: casting to uint64_t here is safe because the default memory
      // alignment is at least 8 bytes and the TX_32X32 is aligned on 8 byte
      // boundaries.
      ctx = !!*(const uint64_t *)a;
      ctx += !!*(const uint64_t *)l;
      eob = decode_coefs(xd, get_plane_type(plane), pd->dqcoeff, tx_size,
                         dequant, ctx, sc->scan, sc->neighbors, r);
      *(uint64_t *)a = ((eob > 0) * 0x0101010101010101ULL) >> ctx_shift_a;
      *(uint64_t *)l = ((eob > 0) * 0x0101010101010101ULL) >> ctx_shift_l;
      break;
    default:
      assert(0 && "Invalid transform size.");
      eob = 0;
      break;
  }

  return eob;
}
