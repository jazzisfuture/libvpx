/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_COMMON_VP9_BLOCKD_H_
#define VP9_COMMON_VP9_BLOCKD_H_

#include "./vpx_config.h"

#include "vpx_ports/mem.h"
#include "vpx_scale/yv12config.h"

#include "vp9/common/vp9_common_data.h"
#include "vp9/common/vp9_filter.h"
#include "vp9/common/vp9_mv.h"
#include "vp9/common/vp9_scale.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE_GROUPS 4
#define SKIP_CONTEXTS 3
#define INTER_MODE_CONTEXTS 7

/* Segment Feature Masks */
#define MAX_MV_REF_CANDIDATES 2

#define INTRA_INTER_CONTEXTS 4
#define COMP_INTER_CONTEXTS 5
#define REF_CONTEXTS 5

typedef enum {
  PLANE_TYPE_Y  = 0,
  PLANE_TYPE_UV = 1,
  PLANE_TYPES
} PLANE_TYPE;

#define MAX_MB_PLANE 3

typedef char ENTROPY_CONTEXT;

static INLINE int combine_entropy_contexts(ENTROPY_CONTEXT a,
                                           ENTROPY_CONTEXT b) {
  return (a != 0) + (b != 0);
}

typedef enum {
  KEY_FRAME = 0,
  INTER_FRAME = 1,
  FRAME_TYPES,
} FRAME_TYPE;

typedef enum {
  DC_PRED,         // Average of above and left pixels
  V_PRED,          // Vertical
  H_PRED,          // Horizontal
  D45_PRED,        // Directional 45  deg = round(arctan(1/1) * 180/pi)
  D135_PRED,       // Directional 135 deg = 180 - 45
  D117_PRED,       // Directional 117 deg = 180 - 63
  D153_PRED,       // Directional 153 deg = 180 - 27
  D207_PRED,       // Directional 207 deg = 180 + 27
  D63_PRED,        // Directional 63  deg = round(arctan(2/1) * 180/pi)
  TM_PRED,         // True-motion
  NEARESTMV,
  NEARMV,
  ZEROMV,
  NEWMV,
  MB_MODE_COUNT
} PREDICTION_MODE;

static INLINE int is_inter_mode(PREDICTION_MODE mode) {
  return mode >= NEARESTMV && mode <= NEWMV;
}

<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
#if CONFIG_FILTERINTRA
static INLINE int is_filter_allowed(MB_PREDICTION_MODE mode) {
  return mode != DC_PRED &&
         mode != D45_PRED &&
         mode != D27_PRED &&
         mode != D63_PRED;
}
#endif

#define VP9_INTRA_MODES (TM_PRED + 1)
=======
#define INTRA_MODES (TM_PRED + 1)
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")

#define INTER_MODES (1 + NEWMV - NEARESTMV)

#define INTER_OFFSET(mode) ((mode) - NEARESTMV)

/* For keyframes, intra block modes are predicted by the (already decoded)
   modes for the Y blocks to the left and above us; for interframes, there
   is a single probability table. */

typedef struct {
  PREDICTION_MODE as_mode;
  int_mv as_mv[2];  // first, second inter predictor motion vectors
} b_mode_info;

// Note that the rate-distortion optimization loop, bit-stream writer, and
// decoder implementation modules critically rely on the enum entry values
// specified herein. They should be refactored concurrently.
typedef enum {
  NONE = -1,
  INTRA_FRAME = 0,
  LAST_FRAME = 1,
  GOLDEN_FRAME = 2,
  ALTREF_FRAME = 3,
  MAX_REF_FRAMES = 4
} MV_REFERENCE_FRAME;

<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
static INLINE int b_width_log2(BLOCK_SIZE_TYPE sb_type) {
  return b_width_log2_lookup[sb_type];
}
static INLINE int b_height_log2(BLOCK_SIZE_TYPE sb_type) {
  return b_height_log2_lookup[sb_type];
}

static INLINE int mi_width_log2(BLOCK_SIZE_TYPE sb_type) {
  return mi_width_log2_lookup[sb_type];
}

static INLINE int mi_height_log2(BLOCK_SIZE_TYPE sb_type) {
  return mi_height_log2_lookup[sb_type];
}

#if CONFIG_INTERINTRA
static INLINE TX_SIZE intra_size_log2_for_interintra(int bs) {
  switch (bs) {
    case 4:
      return TX_4X4;
      break;
    case 8:
      return TX_8X8;
      break;
    case 16:
      return TX_16X16;
      break;
    case 32:
      return TX_32X32;
      break;
    default:
      return TX_32X32;
      break;
  }
}

static INLINE int is_interintra_allowed(BLOCK_SIZE_TYPE sb_type) {
  return ((sb_type >= BLOCK_8X8) && (sb_type < BLOCK_64X64));
}

#if CONFIG_MASKED_INTERINTRA
#define MASK_BITS_SML_INTERINTRA   3
#define MASK_BITS_MED_INTERINTRA   4
#define MASK_BITS_BIG_INTERINTRA   5
#define MASK_NONE_INTERINTRA      -1
static INLINE int get_mask_bits_interintra(BLOCK_SIZE_TYPE sb_type) {
  if (sb_type == BLOCK_4X4)
     return 0;
  if (sb_type <= BLOCK_8X8)
    return MASK_BITS_SML_INTERINTRA;
  else if (sb_type <= BLOCK_32X32)
    return MASK_BITS_MED_INTERINTRA;
  else
    return MASK_BITS_BIG_INTERINTRA;
}
#endif
#endif

#if CONFIG_MASKED_INTERINTER
#define MASK_BITS_SML   3
#define MASK_BITS_MED   4
#define MASK_BITS_BIG   5
#define MASK_NONE      -1

static inline int get_mask_bits(BLOCK_SIZE_TYPE sb_type) {
  if (sb_type == BLOCK_4X4)
     return 0;
  if (sb_type <= BLOCK_8X8)
    return MASK_BITS_SML;
  else if (sb_type <= BLOCK_32X32)
    return MASK_BITS_MED;
  else
    return MASK_BITS_BIG;
}
#endif

=======
// This structure now relates to 8x8 block regions.
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")
typedef struct {
<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
  MB_PREDICTION_MODE mode, uv_mode;
#if CONFIG_INTERINTRA
  MB_PREDICTION_MODE interintra_mode, interintra_uv_mode;
#if CONFIG_MASKED_INTERINTRA
  int interintra_mask_index;
  int interintra_uv_mask_index;
  int use_masked_interintra;
#endif
#endif
#if CONFIG_FILTERINTRA
  int filterbit, uv_filterbit;
#endif
=======
  // Common for both INTER and INTRA blocks
  BLOCK_SIZE sb_type;
  PREDICTION_MODE mode;
  TX_SIZE tx_size;
  int8_t skip;
  int8_t segment_id;
  int8_t seg_id_predicted;  // valid only when temporal_update is enabled

  // Only for INTRA blocks
  PREDICTION_MODE uv_mode;

  // Only for INTER blocks
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")
  MV_REFERENCE_FRAME ref_frame[2];
  int_mv mv[2];
  int_mv ref_mvs[MAX_REF_FRAMES][MAX_MV_REF_CANDIDATES];
<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
  int_mv best_mv, best_second_mv;

  uint8_t mb_mode_context[MAX_REF_FRAMES];

  unsigned char mb_skip_coeff;                                /* does this mb has coefficients at all, 1=no coefficients, 0=need decode tokens */
  unsigned char segment_id;           // Segment id for current frame

  // Flags used for prediction status of various bistream signals
  unsigned char seg_id_predicted;

  // Indicates if the mb is part of the image (1) vs border (0)
  // This can be useful in determining whether the MB provides
  // a valid predictor
  unsigned char mb_in_image;

  INTERPOLATIONFILTERTYPE interp_filter;

  BLOCK_SIZE_TYPE sb_type;

#if CONFIG_MASKED_INTERINTER
  int use_masked_compound;
  int mask_index;
#endif
=======
  uint8_t mode_context[MAX_REF_FRAMES];
  INTERP_FILTER interp_filter;
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")
} MB_MODE_INFO;

typedef struct MODE_INFO {
  struct MODE_INFO *src_mi;
  MB_MODE_INFO mbmi;
<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
#if CONFIG_FILTERINTRA
  int b_filter_info[4];
#endif
  union b_mode_info bmi[4];
=======
  b_mode_info bmi[4];
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")
} MODE_INFO;

static INLINE PREDICTION_MODE get_y_mode(const MODE_INFO *mi, int block) {
  return mi->mbmi.sb_type < BLOCK_8X8 ? mi->bmi[block].as_mode
                                      : mi->mbmi.mode;
}

static INLINE int is_inter_block(const MB_MODE_INFO *mbmi) {
  return mbmi->ref_frame[0] > INTRA_FRAME;
}

static INLINE int has_second_ref(const MB_MODE_INFO *mbmi) {
  return mbmi->ref_frame[1] > INTRA_FRAME;
}

PREDICTION_MODE vp9_left_block_mode(const MODE_INFO *cur_mi,
                                    const MODE_INFO *left_mi, int b);

PREDICTION_MODE vp9_above_block_mode(const MODE_INFO *cur_mi,
                                     const MODE_INFO *above_mi, int b);

enum mv_precision {
  MV_PRECISION_Q3,
  MV_PRECISION_Q4
};

struct buf_2d {
  uint8_t *buf;
  int stride;
};

struct macroblockd_plane {
  tran_low_t *dqcoeff;
  PLANE_TYPE plane_type;
  int subsampling_x;
  int subsampling_y;
  struct buf_2d dst;
  struct buf_2d pre[2];
  const int16_t *dequant;
  ENTROPY_CONTEXT *above_context;
  ENTROPY_CONTEXT *left_context;
};

#define BLOCK_OFFSET(x, i) ((x) + (i) * 16)

typedef struct RefBuffer {
  // TODO(dkovalev): idx is not really required and should be removed, now it
  // is used in vp9_onyxd_if.c
  int idx;
  YV12_BUFFER_CONFIG *buf;
  struct scale_factors sf;
} RefBuffer;

typedef struct macroblockd {
  struct macroblockd_plane plane[MAX_MB_PLANE];

  int mi_stride;

  MODE_INFO *mi;

  int up_available;
  int left_available;

  /* Distance of MB away from frame edges */
  int mb_to_left_edge;
  int mb_to_right_edge;
  int mb_to_top_edge;
  int mb_to_bottom_edge;

  /* pointers to reference frames */
  RefBuffer *block_refs[2];

  /* pointer to current frame */
  const YV12_BUFFER_CONFIG *cur_buf;

  /* mc buffer */
  DECLARE_ALIGNED(16, uint8_t, mc_buf[80 * 2 * 80 * 2]);

#if CONFIG_VP9_HIGHBITDEPTH
  /* Bit depth: 8, 10, 12 */
  int bd;
  DECLARE_ALIGNED(16, uint16_t, mc_buf_high[80 * 2 * 80 * 2]);
#endif

  int lossless;

  int corrupted;

  DECLARE_ALIGNED(16, tran_low_t, dqcoeff[MAX_MB_PLANE][64 * 64]);

  ENTROPY_CONTEXT *above_context[MAX_MB_PLANE];
  ENTROPY_CONTEXT left_context[MAX_MB_PLANE][16];

  PARTITION_CONTEXT *above_seg_context;
  PARTITION_CONTEXT left_seg_context[8];
} MACROBLOCKD;

static INLINE BLOCK_SIZE get_subsize(BLOCK_SIZE bsize,
                                     PARTITION_TYPE partition) {
  return subsize_lookup[partition][bsize];
}

extern const TX_TYPE intra_mode_to_tx_type_lookup[INTRA_MODES];

static INLINE TX_TYPE get_tx_type(PLANE_TYPE plane_type,
                                  const MACROBLOCKD *xd) {
  const MB_MODE_INFO *const mbmi = &xd->mi[0].src_mi->mbmi;

  if (plane_type != PLANE_TYPE_Y || is_inter_block(mbmi))
    return DCT_DCT;
  return intra_mode_to_tx_type_lookup[mbmi->mode];
}

static INLINE TX_TYPE get_tx_type_4x4(PLANE_TYPE plane_type,
                                      const MACROBLOCKD *xd, int ib) {
  const MODE_INFO *const mi = xd->mi[0].src_mi;

  if (plane_type != PLANE_TYPE_Y || xd->lossless || is_inter_block(&mi->mbmi))
    return DCT_DCT;

  return intra_mode_to_tx_type_lookup[get_y_mode(mi, ib)];
}

void vp9_setup_block_planes(MACROBLOCKD *xd, int ss_x, int ss_y);

static INLINE TX_SIZE get_uv_tx_size_impl(TX_SIZE y_tx_size, BLOCK_SIZE bsize,
                                          int xss, int yss) {
  if (bsize < BLOCK_8X8) {
    return TX_4X4;
  } else {
    const BLOCK_SIZE plane_bsize = ss_size_lookup[bsize][xss][yss];
    return MIN(y_tx_size, max_txsize_lookup[plane_bsize]);
  }
}

static INLINE TX_SIZE get_uv_tx_size(const MB_MODE_INFO *mbmi,
                                     const struct macroblockd_plane *pd) {
  return get_uv_tx_size_impl(mbmi->tx_size, mbmi->sb_type, pd->subsampling_x,
                             pd->subsampling_y);
}

static INLINE BLOCK_SIZE get_plane_block_size(BLOCK_SIZE bsize,
    const struct macroblockd_plane *pd) {
  return ss_size_lookup[bsize][pd->subsampling_x][pd->subsampling_y];
}

typedef void (*foreach_transformed_block_visitor)(int plane, int block,
                                                  BLOCK_SIZE plane_bsize,
                                                  TX_SIZE tx_size,
                                                  void *arg);

void vp9_foreach_transformed_block_in_plane(
    const MACROBLOCKD *const xd, BLOCK_SIZE bsize, int plane,
    foreach_transformed_block_visitor visit, void *arg);


void vp9_foreach_transformed_block(
    const MACROBLOCKD* const xd, BLOCK_SIZE bsize,
    foreach_transformed_block_visitor visit, void *arg);

static INLINE void txfrm_block_to_raster_xy(BLOCK_SIZE plane_bsize,
                                            TX_SIZE tx_size, int block,
                                            int *x, int *y) {
  const int bwl = b_width_log2_lookup[plane_bsize];
  const int tx_cols_log2 = bwl - tx_size;
  const int tx_cols = 1 << tx_cols_log2;
  const int raster_mb = block >> (tx_size << 1);
  *x = (raster_mb & (tx_cols - 1)) << tx_size;
  *y = (raster_mb >> tx_cols_log2) << tx_size;
}

void vp9_set_contexts(const MACROBLOCKD *xd, struct macroblockd_plane *pd,
                      BLOCK_SIZE plane_bsize, TX_SIZE tx_size, int has_eob,
                      int aoff, int loff);

<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
#if CONFIG_INTERINTRA
static void extend_for_interintra(MACROBLOCKD* const xd,
                                  BLOCK_SIZE_TYPE bsize) {
  int bh = 4 << b_height_log2(bsize), bw = 4 << b_width_log2(bsize);
  int ystride = xd->plane[0].dst.stride, uvstride = xd->plane[1].dst.stride;
  uint8_t *pixel_y, *pixel_u, *pixel_v;
  int ymargin, uvmargin;
  if (xd->mb_to_bottom_edge < 0) {
    int r;
    ymargin = 0 - xd->mb_to_bottom_edge / 8;
    uvmargin = 0 - xd->mb_to_bottom_edge / 16;
    pixel_y = xd->plane[0].dst.buf - 1 + (bh - ymargin -1) * ystride;
    pixel_u = xd->plane[1].dst.buf - 1 + (bh / 2 - uvmargin - 1) * uvstride;
    pixel_v = xd->plane[2].dst.buf - 1 + (bh / 2 - uvmargin - 1) * uvstride;
    for (r = 0; r < ymargin; r++)
      xd->plane[0].dst.buf[-1 + (bh - r -1) * ystride] = *pixel_y;
    for (r = 0; r < uvmargin; r++) {
      xd->plane[1].dst.buf[-1 + (bh / 2 - r -1) * uvstride] = *pixel_u;
      xd->plane[2].dst.buf[-1 + (bh / 2 - r -1) * uvstride] = *pixel_v;
    }
  }
  if (xd->mb_to_right_edge < 0) {
    ymargin = 0 - xd->mb_to_right_edge / 8;
    uvmargin = 0 - xd->mb_to_right_edge / 16;
    pixel_y = xd->plane[0].dst.buf + bw - ymargin - 1 - ystride;
    pixel_u = xd->plane[1].dst.buf + bw / 2 - uvmargin - 1 - uvstride;
    pixel_v = xd->plane[2].dst.buf + bw / 2 - uvmargin - 1 - uvstride;
    vpx_memset(xd->plane[0].dst.buf + bw - ymargin - ystride,
               *pixel_y, ymargin);
    vpx_memset(xd->plane[1].dst.buf + bw / 2 - uvmargin - uvstride,
               *pixel_u, uvmargin);
    vpx_memset(xd->plane[2].dst.buf + bw / 2 - uvmargin - uvstride,
               *pixel_v, uvmargin);
  }
}
#endif

static void extend_for_intra(MACROBLOCKD* const xd, int plane, int block,
                             BLOCK_SIZE_TYPE bsize, int ss_txfrm_size) {
  const int bw = plane_block_width(bsize, &xd->plane[plane]);
  const int bh = plane_block_height(bsize, &xd->plane[plane]);
  int x, y;
  txfrm_block_to_raster_xy(xd, bsize, plane, block, ss_txfrm_size, &x, &y);
  x = x * 4 - 1;
  y = y * 4 - 1;
  // Copy a pixel into the umv if we are in a situation where the block size
  // extends into the UMV.
  // TODO(JBB): Should be able to do the full extend in place so we don't have
  // to do this multiple times.
  if (xd->mb_to_right_edge < 0) {
    int umv_border_start = bw
        + (xd->mb_to_right_edge >> (3 + xd->plane[plane].subsampling_x));

    if (x + bw > umv_border_start)
      vpx_memset(
          xd->plane[plane].dst.buf + y * xd->plane[plane].dst.stride
              + umv_border_start,
          *(xd->plane[plane].dst.buf + y * xd->plane[plane].dst.stride
              + umv_border_start - 1),
          bw);
  }
  if (xd->mb_to_bottom_edge < 0) {
    int umv_border_start = bh
        + (xd->mb_to_bottom_edge >> (3 + xd->plane[plane].subsampling_y));
    int i;
    uint8_t c = *(xd->plane[plane].dst.buf
        + (umv_border_start - 1) * xd->plane[plane].dst.stride + x);

    uint8_t *d = xd->plane[plane].dst.buf
        + umv_border_start * xd->plane[plane].dst.stride + x;

    if (y + bh > umv_border_start)
      for (i = 0; i < bh; i++, d += xd->plane[plane].dst.stride)
        *d = c;
  }
}
static void set_contexts_on_border(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize,
                                   int plane, int tx_size_in_blocks,
                                   int eob, int aoff, int loff,
                                   ENTROPY_CONTEXT *A, ENTROPY_CONTEXT *L) {
  struct macroblockd_plane *pd = &xd->plane[plane];
  int above_contexts = tx_size_in_blocks;
  int left_contexts = tx_size_in_blocks;
  int mi_blocks_wide = 1 << plane_block_width_log2by4(bsize, pd);
  int mi_blocks_high = 1 << plane_block_height_log2by4(bsize, pd);
  int pt;

  // xd->mb_to_right_edge is in units of pixels * 8.  This converts
  // it to 4x4 block sizes.
  if (xd->mb_to_right_edge < 0)
    mi_blocks_wide += (xd->mb_to_right_edge >> (5 + pd->subsampling_x));

  // this code attempts to avoid copying into contexts that are outside
  // our border.  Any blocks that do are set to 0...
  if (above_contexts + aoff > mi_blocks_wide)
    above_contexts = mi_blocks_wide - aoff;

  if (xd->mb_to_bottom_edge < 0)
    mi_blocks_high += (xd->mb_to_bottom_edge >> (5 + pd->subsampling_y));

  if (left_contexts + loff > mi_blocks_high)
    left_contexts = mi_blocks_high - loff;

  for (pt = 0; pt < above_contexts; pt++)
    A[pt] = eob > 0;
  for (pt = above_contexts; pt < tx_size_in_blocks; pt++)
    A[pt] = 0;
  for (pt = 0; pt < left_contexts; pt++)
    L[pt] = eob > 0;
  for (pt = left_contexts; pt < tx_size_in_blocks; pt++)
    L[pt] = 0;
}

=======
#ifdef __cplusplus
}  // extern "C"
#endif
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")

#endif  // VP9_COMMON_VP9_BLOCKD_H_
