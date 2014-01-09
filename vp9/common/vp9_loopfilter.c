/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "vp9/common/vp9_loopfilter.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_reconinter.h"
#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_seg_common.h"
//#include "vp9/common/cuda_loopfilter.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)
//typedef unsigned char uchar;
//#include "cuda_loopfilter.h"
typedef struct {
  cl_uchar mblim;
  cl_uchar lim;
  cl_uchar hev_thr;
} cuda_loop_filter_thresh;

typedef struct {
  cl_ulong left_y[TX_SIZES];
  cl_ulong above_y[TX_SIZES];
  cl_ulong int_4x4_y;
  cl_ushort left_uv[TX_SIZES];
  cl_ushort above_uv[TX_SIZES];
  cl_ushort int_4x4_uv;
  cl_uchar lfl_y[64];
  cl_uchar lfl_uv[16];
} LOOP_FILTER_MASK;

void disable_filter(LOOP_FILTER_MASK* lfm, uint32_t mi_col, uint32_t mi_row, uint32_t tx, uint32_t ty) {
 /* if (mi_col * 8 <= tx && (mi_col + 8) *8 >= tx) {
    printf("c %d %d %d\n", mi_row * 64, (mi_row + 1) * 64, ty);
  }
  if (mi_row * 8 <= ty && (mi_row + 8) * 8 >= ty) {
    printf("r %d %d %d\n", mi_col * 64, (mi_col + 1) * 64, tx);
  }*/
 // printf("col %d %d row %d %d tx %d ty %d\n", mi_col*8, mi_row*8, (mi_col+1) * 8, (mi_row +1)*8, tx, ty);
 /* if (mi_col * 8 <= tx
      && (mi_col + 8) * 8 >= tx
      && mi_row * 8 <= ty
      && (mi_row + 8) * 8 >= ty) {
    lfm->above_y[TX_4X4] = 0;
    lfm->above_y[TX_8X8] = 0;
    lfm->above_y[TX_16X16] = 0;
    lfm->above_y[TX_32X32] = 0;
    lfm->int_4x4_y = 0;
    lfm->left_y[TX_4X4] = 0;
    lfm->left_y[TX_8X8] = 0;
    //lfm->left_y[TX_16X16] = 0;
    lfm->left_y[TX_32X32] = 0;
    print_mask(lfm, mi_col, mi_row);

  } else {*/
    //lfm->above_y[TX_4X4] = 0;
   // lfm->above_y[TX_8X8] = 0;
   // lfm->above_y[TX_16X16] = 0;
  //  lfm->above_y[TX_32X32] = 0;
   // lfm->int_4x4_y = 0;
  //lfm->left_y[TX_4X4] = 0;
  //lfm->left_y[TX_8X8] = 0;
  // lfm->left_y[TX_16X16] = 0;
  // lfm->left_y[TX_32X32] = 0;
    //lfm->above_uv[TX_4X4] = 0;
    //lfm->above_uv[TX_8X8] = 0;
    //lfm->above_uv[TX_16X16] = 0;
    //lfm->above_uv[TX_32X32] = 0;
   // lfm->int_4x4_uv = 0;
  //lfm->left_uv[TX_4X4] = 0;
  //lfm->left_uv[TX_8X8] = 0;
   //lfm->left_uv[TX_16X16] = 0;
   //lfm->left_uv[TX_32X32] = 0;
  //}
  //}

}

//test
#include <sys/time.h>
typedef struct {
#if defined(_WIN32)
  LARGE_INTEGER  begin, end;
#else
  struct timeval begin, end;
#endif
} vpx_usec_timer;


static void
vpx_usec_timer_start( vpx_usec_timer *t) {
#if defined(_WIN32)
  QueryPerformanceCounter(&t->begin);
#else
  gettimeofday(&t->begin, NULL);
#endif
}


static void
vpx_usec_timer_mark( vpx_usec_timer *t) {
#if defined(_WIN32)
  QueryPerformanceCounter(&t->end);
#else
  gettimeofday(&t->end, NULL);
#endif
}


static int64_t
vpx_usec_timer_elapsed( vpx_usec_timer *t) {
#if defined(_WIN32)
  LARGE_INTEGER freq, diff;

  diff.QuadPart = t->end.QuadPart - t->begin.QuadPart;

  QueryPerformanceFrequency(&freq);
  return diff.QuadPart * 1000000 / freq.QuadPart;
#else
  struct timeval diff;

  timersub(&t->end, &t->begin, &diff);
  return diff.tv_sec * 1000000 + diff.tv_usec;
#endif
}
//end test

// This structure holds bit masks for all 8x8 blocks in a 64x64 region.
// Each 1 bit represents a position in which we want to apply the loop filter.
// Left_ entries refer to whether we apply a filter on the border to the
// left of the block.   Above_ entries refer to whether or not to apply a
// filter on the above border.   Int_ entries refer to whether or not to
// apply borders on the 4x4 edges within the 8x8 block that each bit
// represents.
// Since each transform is accompanied by a potentially different type of
// loop filter there is a different entry in the array for each transform size.
/*
typedef struct {
  uint64_t left_y[TX_SIZES];
  uint64_t above_y[TX_SIZES];
  uint64_t int_4x4_y;
  uint16_t left_uv[TX_SIZES];
  uint16_t above_uv[TX_SIZES];
  uint16_t int_4x4_uv;
  uint8_t lfl_y[64];
  uint8_t lfl_uv[16];
} LOOP_FILTER_MASK;
*/
// 64 bit masks for left transform size.  Each 1 represents a position where
// we should apply a loop filter across the left border of an 8x8 block
// boundary.
//
// In the case of TX_16X16->  ( in low order byte first we end up with
// a mask that looks like this
//
//    10101010
//    10101010
//    10101010
//    10101010
//    10101010
//    10101010
//    10101010
//    10101010
//
// A loopfilter should be applied to every other 8x8 horizontally.
static const uint64_t left_64x64_txform_mask[TX_SIZES]= {
    0xffffffffffffffff,  // TX_4X4
    0xffffffffffffffff,  // TX_8x8
    0x5555555555555555,  // TX_16x16
    0x1111111111111111,  // TX_32x32
};

// 64 bit masks for above transform size.  Each 1 represents a position where
// we should apply a loop filter across the top border of an 8x8 block
// boundary.
//
// In the case of TX_32x32 ->  ( in low order byte first we end up with
// a mask that looks like this
//
//    11111111
//    00000000
//    00000000
//    00000000
//    11111111
//    00000000
//    00000000
//    00000000
//
// A loopfilter should be applied to every other 4 the row vertically.
static const uint64_t above_64x64_txform_mask[TX_SIZES]= {
    0xffffffffffffffff,  // TX_4X4
    0xffffffffffffffff,  // TX_8x8
    0x00ff00ff00ff00ff,  // TX_16x16
    0x000000ff000000ff,  // TX_32x32
};

// 64 bit masks for prediction sizes (left).  Each 1 represents a position
// where left border of an 8x8 block.  These are aligned to the right most
// appropriate bit,  and then shifted into place.
//
// In the case of TX_16x32 ->  ( low order byte first ) we end up with
// a mask that looks like this :
//
//  10000000
//  10000000
//  10000000
//  10000000
//  00000000
//  00000000
//  00000000
//  00000000
static const uint64_t left_prediction_mask[BLOCK_SIZES] = {
    0x0000000000000001,  // BLOCK_4X4,
    0x0000000000000001,  // BLOCK_4X8,
    0x0000000000000001,  // BLOCK_8X4,
    0x0000000000000001,  // BLOCK_8X8,
    0x0000000000000101,  // BLOCK_8X16,
    0x0000000000000001,  // BLOCK_16X8,
    0x0000000000000101,  // BLOCK_16X16,
    0x0000000001010101,  // BLOCK_16X32,
    0x0000000000000101,  // BLOCK_32X16,
    0x0000000001010101,  // BLOCK_32X32,
    0x0101010101010101,  // BLOCK_32X64,
    0x0000000001010101,  // BLOCK_64X32,
    0x0101010101010101,  // BLOCK_64X64
};

// 64 bit mask to shift and set for each prediction size.
static const uint64_t above_prediction_mask[BLOCK_SIZES] = {
    0x0000000000000001,  // BLOCK_4X4
    0x0000000000000001,  // BLOCK_4X8
    0x0000000000000001,  // BLOCK_8X4
    0x0000000000000001,  // BLOCK_8X8
    0x0000000000000001,  // BLOCK_8X16,
    0x0000000000000003,  // BLOCK_16X8
    0x0000000000000003,  // BLOCK_16X16
    0x0000000000000003,  // BLOCK_16X32,
    0x000000000000000f,  // BLOCK_32X16,
    0x000000000000000f,  // BLOCK_32X32,
    0x000000000000000f,  // BLOCK_32X64,
    0x00000000000000ff,  // BLOCK_64X32,
    0x00000000000000ff,  // BLOCK_64X64
};
// 64 bit mask to shift and set for each prediction size.  A bit is set for
// each 8x8 block that would be in the left most block of the given block
// size in the 64x64 block.
static const uint64_t size_mask[BLOCK_SIZES] = {
    0x0000000000000001,  // BLOCK_4X4
    0x0000000000000001,  // BLOCK_4X8
    0x0000000000000001,  // BLOCK_8X4
    0x0000000000000001,  // BLOCK_8X8
    0x0000000000000101,  // BLOCK_8X16,
    0x0000000000000003,  // BLOCK_16X8
    0x0000000000000303,  // BLOCK_16X16
    0x0000000003030303,  // BLOCK_16X32,
    0x0000000000000f0f,  // BLOCK_32X16,
    0x000000000f0f0f0f,  // BLOCK_32X32,
    0x0f0f0f0f0f0f0f0f,  // BLOCK_32X64,
    0x00000000ffffffff,  // BLOCK_64X32,
    0xffffffffffffffff,  // BLOCK_64X64
};

// These are used for masking the left and above borders.
static const uint64_t left_border =  0x1111111111111111;
static const uint64_t above_border = 0x000000ff000000ff;

// 16 bit masks for uv transform sizes.
static const uint16_t left_64x64_txform_mask_uv[TX_SIZES]= {
    0xffff,  // TX_4X4
    0xffff,  // TX_8x8
    0x5555,  // TX_16x16
    0x1111,  // TX_32x32
};

static const uint16_t above_64x64_txform_mask_uv[TX_SIZES]= {
    0xffff,  // TX_4X4
    0xffff,  // TX_8x8
    0x0f0f,  // TX_16x16
    0x000f,  // TX_32x32
};

// 16 bit left mask to shift and set for each uv prediction size.
static const uint16_t left_prediction_mask_uv[BLOCK_SIZES] = {
    0x0001,  // BLOCK_4X4,
    0x0001,  // BLOCK_4X8,
    0x0001,  // BLOCK_8X4,
    0x0001,  // BLOCK_8X8,
    0x0001,  // BLOCK_8X16,
    0x0001,  // BLOCK_16X8,
    0x0001,  // BLOCK_16X16,
    0x0011,  // BLOCK_16X32,
    0x0001,  // BLOCK_32X16,
    0x0011,  // BLOCK_32X32,
    0x1111,  // BLOCK_32X64
    0x0011,  // BLOCK_64X32,
    0x1111,  // BLOCK_64X64
};
// 16 bit above mask to shift and set for uv each prediction size.
static const uint16_t above_prediction_mask_uv[BLOCK_SIZES] = {
    0x0001,  // BLOCK_4X4
    0x0001,  // BLOCK_4X8
    0x0001,  // BLOCK_8X4
    0x0001,  // BLOCK_8X8
    0x0001,  // BLOCK_8X16,
    0x0001,  // BLOCK_16X8
    0x0001,  // BLOCK_16X16
    0x0001,  // BLOCK_16X32,
    0x0003,  // BLOCK_32X16,
    0x0003,  // BLOCK_32X32,
    0x0003,  // BLOCK_32X64,
    0x000f,  // BLOCK_64X32,
    0x000f,  // BLOCK_64X64
};

// 64 bit mask to shift and set for each uv prediction size
static const uint16_t size_mask_uv[BLOCK_SIZES] = {
    0x0001,  // BLOCK_4X4
    0x0001,  // BLOCK_4X8
    0x0001,  // BLOCK_8X4
    0x0001,  // BLOCK_8X8
    0x0001,  // BLOCK_8X16,
    0x0001,  // BLOCK_16X8
    0x0001,  // BLOCK_16X16
    0x0011,  // BLOCK_16X32,
    0x0003,  // BLOCK_32X16,
    0x0033,  // BLOCK_32X32,
    0x3333,  // BLOCK_32X64,
    0x00ff,  // BLOCK_64X32,
    0xffff,  // BLOCK_64X64
};
static const uint16_t left_border_uv =  0x1111;
static const uint16_t above_border_uv = 0x000f;

static const int mode_lf_lut[MB_MODE_COUNT] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // INTRA_MODES
  1, 1, 0, 1                     // INTER_MODES (ZEROMV == 0)
};

static void update_sharpness(loop_filter_info_n *lfi, int sharpness_lvl) {
  int lvl;

  // For each possible value for the loop filter fill out limits
  for (lvl = 0; lvl <= MAX_LOOP_FILTER; lvl++) {
    // Set loop filter paramaeters that control sharpness.
    int block_inside_limit = lvl >> ((sharpness_lvl > 0) + (sharpness_lvl > 4));

    if (sharpness_lvl > 0) {
      if (block_inside_limit > (9 - sharpness_lvl))
        block_inside_limit = (9 - sharpness_lvl);
    }

    if (block_inside_limit < 1)
      block_inside_limit = 1;

    vpx_memset(lfi->lfthr[lvl].lim, block_inside_limit, SIMD_WIDTH);
    vpx_memset(lfi->lfthr[lvl].mblim, (2 * (lvl + 2) + block_inside_limit),
               SIMD_WIDTH);
  }
}

void vp9_loop_filter_init(VP9_COMMON *cm) {
  loop_filter_info_n *lfi = &cm->lf_info;
  struct loopfilter *lf = &cm->lf;
  int lvl;

  // init limits for given sharpness
  update_sharpness(lfi, lf->sharpness_level);
  lf->last_sharpness_level = lf->sharpness_level;

  // init hev threshold const vectors
  for (lvl = 0; lvl <= MAX_LOOP_FILTER; lvl++)
    vpx_memset(lfi->lfthr[lvl].hev_thr, (lvl >> 4), SIMD_WIDTH);
}

void vp9_loop_filter_frame_init(VP9_COMMON *cm, int default_filt_lvl) {
  int seg_id;
  // n_shift is the a multiplier for lf_deltas
  // the multiplier is 1 for when filter_lvl is between 0 and 31;
  // 2 when filter_lvl is between 32 and 63
  const int scale = 1 << (default_filt_lvl >> 5);
  loop_filter_info_n *const lfi = &cm->lf_info;
  struct loopfilter *const lf = &cm->lf;
  const struct segmentation *const seg = &cm->seg;

  // update limits if sharpness has changed
  if (lf->last_sharpness_level != lf->sharpness_level) {
    update_sharpness(lfi, lf->sharpness_level);
    lf->last_sharpness_level = lf->sharpness_level;
  }

  for (seg_id = 0; seg_id < MAX_SEGMENTS; seg_id++) {
    int lvl_seg = default_filt_lvl;
    if (vp9_segfeature_active(seg, seg_id, SEG_LVL_ALT_LF)) {
      const int data = vp9_get_segdata(seg, seg_id, SEG_LVL_ALT_LF);
      lvl_seg = seg->abs_delta == SEGMENT_ABSDATA
                  ? data
                  : clamp(default_filt_lvl + data, 0, MAX_LOOP_FILTER);
    }

    if (!lf->mode_ref_delta_enabled) {
      // we could get rid of this if we assume that deltas are set to
      // zero when not in use; encoder always uses deltas
      vpx_memset(lfi->lvl[seg_id], lvl_seg, sizeof(lfi->lvl[seg_id]));
    } else {
      int ref, mode;
      const int intra_lvl = lvl_seg + lf->ref_deltas[INTRA_FRAME] * scale;
      lfi->lvl[seg_id][INTRA_FRAME][0] = clamp(intra_lvl, 0, MAX_LOOP_FILTER);

      for (ref = LAST_FRAME; ref < MAX_REF_FRAMES; ++ref) {
        for (mode = 0; mode < MAX_MODE_LF_DELTAS; ++mode) {
          const int inter_lvl = lvl_seg + lf->ref_deltas[ref] * scale
                                        + lf->mode_deltas[mode] * scale;
          lfi->lvl[seg_id][ref][mode] = clamp(inter_lvl, 0, MAX_LOOP_FILTER);
        }
      }
    }
  }
}

static void filter_selectively_vert_row2(PLANE_TYPE plane_type,
                                         uint8_t *s, int pitch,
                                         unsigned int mask_16x16_l,
                                         unsigned int mask_8x8_l,
                                         unsigned int mask_4x4_l,
                                         unsigned int mask_4x4_int_l,
                                         const loop_filter_info_n *lfi_n,
                                         const uint8_t *lfl) {
  const int mask_shift = plane_type ? 4 : 8;
  const int mask_cutoff = plane_type ? 0xf : 0xff;
  const int lfl_forward = plane_type ? 4 : 8;

  unsigned int mask_16x16_0 = mask_16x16_l & mask_cutoff;
  unsigned int mask_8x8_0 = mask_8x8_l & mask_cutoff;
  unsigned int mask_4x4_0 = mask_4x4_l & mask_cutoff;
  unsigned int mask_4x4_int_0 = mask_4x4_int_l & mask_cutoff;
  unsigned int mask_16x16_1 = (mask_16x16_l >> mask_shift) & mask_cutoff;
  unsigned int mask_8x8_1 = (mask_8x8_l >> mask_shift) & mask_cutoff;
  unsigned int mask_4x4_1 = (mask_4x4_l >> mask_shift) & mask_cutoff;
  unsigned int mask_4x4_int_1 = (mask_4x4_int_l >> mask_shift) & mask_cutoff;
  unsigned int mask;

  for (mask = mask_16x16_0 | mask_8x8_0 | mask_4x4_0 | mask_4x4_int_0 |
      mask_16x16_1 | mask_8x8_1 | mask_4x4_1 | mask_4x4_int_1;
      mask; mask >>= 1) {
    const loop_filter_thresh *lfi0 = lfi_n->lfthr + *lfl;
    const loop_filter_thresh *lfi1 = lfi_n->lfthr + *(lfl + lfl_forward);

    // TODO(yunqingwang): count in loopfilter functions should be removed.
    if (mask & 1) {
      if ((mask_16x16_0 | mask_16x16_1) & 1) {
        if ((mask_16x16_0 & mask_16x16_1) & 1) {
          vp9_lpf_vertical_16_dual(s, pitch, lfi0->mblim, lfi0->lim,
                                   lfi0->hev_thr);
        } else if (mask_16x16_0 & 1) {
          vp9_lpf_vertical_16(s, pitch, lfi0->mblim, lfi0->lim,
                              lfi0->hev_thr);
        } else {
          vp9_lpf_vertical_16(s + 8 *pitch, pitch, lfi1->mblim,
                              lfi1->lim, lfi1->hev_thr);
        }
      }

      if ((mask_8x8_0 | mask_8x8_1) & 1) {
        if ((mask_8x8_0 & mask_8x8_1) & 1) {
          vp9_lpf_vertical_8_dual(s, pitch, lfi0->mblim, lfi0->lim,
                                  lfi0->hev_thr, lfi1->mblim, lfi1->lim,
                                  lfi1->hev_thr);
        } else if (mask_8x8_0 & 1) {
          vp9_lpf_vertical_8(s, pitch, lfi0->mblim, lfi0->lim, lfi0->hev_thr,
                             1);
        } else {
          vp9_lpf_vertical_8(s + 8 * pitch, pitch, lfi1->mblim, lfi1->lim,
                             lfi1->hev_thr, 1);
        }
      }

      if ((mask_4x4_0 | mask_4x4_1) & 1) {
        if ((mask_4x4_0 & mask_4x4_1) & 1) {
          vp9_lpf_vertical_4_dual(s, pitch, lfi0->mblim, lfi0->lim,
                                  lfi0->hev_thr, lfi1->mblim, lfi1->lim,
                                  lfi1->hev_thr);
        } else if (mask_4x4_0 & 1) {
          vp9_lpf_vertical_4(s, pitch, lfi0->mblim, lfi0->lim, lfi0->hev_thr,
                             1);
        } else {
          vp9_lpf_vertical_4(s + 8 * pitch, pitch, lfi1->mblim, lfi1->lim,
                             lfi1->hev_thr, 1);
        }
      }

      if ((mask_4x4_int_0 | mask_4x4_int_1) & 1) {
        if ((mask_4x4_int_0 & mask_4x4_int_1) & 1) {
          vp9_lpf_vertical_4_dual(s + 4, pitch, lfi0->mblim, lfi0->lim,
                                  lfi0->hev_thr, lfi1->mblim, lfi1->lim,
                                  lfi1->hev_thr);
        } else if (mask_4x4_int_0 & 1) {
          vp9_lpf_vertical_4(s + 4, pitch, lfi0->mblim, lfi0->lim,
                             lfi0->hev_thr, 1);
        } else {
          vp9_lpf_vertical_4(s + 8 * pitch + 4, pitch, lfi1->mblim, lfi1->lim,
                             lfi1->hev_thr, 1);
        }
      }
    }

    s += 8;
    lfl += 1;
    mask_16x16_0 >>= 1;
    mask_8x8_0 >>= 1;
    mask_4x4_0 >>= 1;
    mask_4x4_int_0 >>= 1;
    mask_16x16_1 >>= 1;
    mask_8x8_1 >>= 1;
    mask_4x4_1 >>= 1;
    mask_4x4_int_1 >>= 1;
  }
}

static void filter_selectively_horiz(uint8_t *s, int pitch,
                                     unsigned int mask_16x16,
                                     unsigned int mask_8x8,
                                     unsigned int mask_4x4,
                                     unsigned int mask_4x4_int,
                                     const loop_filter_info_n *lfi_n,
                                     const uint8_t *lfl) {
  unsigned int mask;
  int count;

  for (mask = mask_16x16 | mask_8x8 | mask_4x4 | mask_4x4_int;
       mask; mask >>= count) {
    const loop_filter_thresh *lfi = lfi_n->lfthr + *lfl;

    count = 1;
    if (mask & 1) {
      if (mask_16x16 & 1) {
        if ((mask_16x16 & 3) == 3) {
          vp9_lpf_horizontal_16(s, pitch, lfi->mblim, lfi->lim,
                                lfi->hev_thr, 2);
          count = 2;
        } else {
          vp9_lpf_horizontal_16(s, pitch, lfi->mblim, lfi->lim,
                                lfi->hev_thr, 1);
        }
      } else if (mask_8x8 & 1) {
        if ((mask_8x8 & 3) == 3) {
          // Next block's thresholds
          const loop_filter_thresh *lfin = lfi_n->lfthr + *(lfl + 1);

          vp9_lpf_horizontal_8_dual(s, pitch, lfi->mblim, lfi->lim,
                                    lfi->hev_thr, lfin->mblim, lfin->lim,
                                    lfin->hev_thr);

          if ((mask_4x4_int & 3) == 3) {
            vp9_lpf_horizontal_4_dual(s + 4 * pitch, pitch, lfi->mblim,
                                      lfi->lim, lfi->hev_thr, lfin->mblim,
                                      lfin->lim, lfin->hev_thr);
          } else {
            if (mask_4x4_int & 1)
              vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim, lfi->lim,
                                   lfi->hev_thr, 1);
            else if (mask_4x4_int & 2)
              vp9_lpf_horizontal_4(s + 8 + 4 * pitch, pitch, lfin->mblim,
                                   lfin->lim, lfin->hev_thr, 1);
          }
          count = 2;
        } else {
          vp9_lpf_horizontal_8(s, pitch, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

          if (mask_4x4_int & 1)
            vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim, lfi->lim,
                                 lfi->hev_thr, 1);
        }
      } else if (mask_4x4 & 1) {
        if ((mask_4x4 & 3) == 3) {
          // Next block's thresholds
          const loop_filter_thresh *lfin = lfi_n->lfthr + *(lfl + 1);

          vp9_lpf_horizontal_4_dual(s, pitch, lfi->mblim, lfi->lim,
                                    lfi->hev_thr, lfin->mblim, lfin->lim,
                                    lfin->hev_thr);
          if ((mask_4x4_int & 3) == 3) {
            vp9_lpf_horizontal_4_dual(s + 4 * pitch, pitch, lfi->mblim,
                                      lfi->lim, lfi->hev_thr, lfin->mblim,
                                      lfin->lim, lfin->hev_thr);
          } else {
            if (mask_4x4_int & 1)
              vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim, lfi->lim,
                                   lfi->hev_thr, 1);
            else if (mask_4x4_int & 2)
              vp9_lpf_horizontal_4(s + 8 + 4 * pitch, pitch, lfin->mblim,
                                   lfin->lim, lfin->hev_thr, 1);
          }
          count = 2;
        } else {
          vp9_lpf_horizontal_4(s, pitch, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

          if (mask_4x4_int & 1)
            vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim, lfi->lim,
                                 lfi->hev_thr, 1);
        }
      } else if (mask_4x4_int & 1) {
        vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim, lfi->lim,
                             lfi->hev_thr, 1);
      }
    }
    s += 8 * count;
    lfl += count;
    mask_16x16 >>= count;
    mask_8x8 >>= count;
    mask_4x4 >>= count;
    mask_4x4_int >>= count;
  }
}

// This function ors into the current lfm structure, where to do loop
// filters for the specific mi we are looking at.   It uses information
// including the block_size_type (32x16, 32x32, etc),  the transform size,
// whether there were any coefficients encoded, and the loop filter strength
// block we are currently looking at. Shift is used to position the
// 1's we produce.
// TODO(JBB) Need another function for different resolution color..
static void build_masks(const loop_filter_info_n *const lfi_n,
                        const MODE_INFO *mi, const int shift_y,
                        const int shift_uv,
                        LOOP_FILTER_MASK *lfm) {
  const BLOCK_SIZE block_size = mi->mbmi.sb_type;
  const TX_SIZE tx_size_y = mi->mbmi.tx_size;
  const TX_SIZE tx_size_uv = get_uv_tx_size(&mi->mbmi);
  const int skip = mi->mbmi.skip_coeff;
  const int seg = mi->mbmi.segment_id;
  const int ref = mi->mbmi.ref_frame[0];
  const int filter_level = lfi_n->lvl[seg][ref][mode_lf_lut[mi->mbmi.mode]];
  uint64_t *left_y = &lfm->left_y[tx_size_y];
  uint64_t *above_y = &lfm->above_y[tx_size_y];
  uint64_t *int_4x4_y = &lfm->int_4x4_y;
  uint16_t *left_uv = &lfm->left_uv[tx_size_uv];
  uint16_t *above_uv = &lfm->above_uv[tx_size_uv];
  uint16_t *int_4x4_uv = &lfm->int_4x4_uv;
  int i;
  int w = num_8x8_blocks_wide_lookup[block_size];
  int h = num_8x8_blocks_high_lookup[block_size];

  // If filter level is 0 we don't loop filter.
  if (!filter_level) {
    return;
  } else {
    int index = shift_y;
    for (i = 0; i < h; i++) {
      vpx_memset(&lfm->lfl_y[index], filter_level, w);
      index += 8;
    }
  }

  // These set 1 in the current block size for the block size edges.
  // For instance if the block size is 32x16,   we'll set :
  //    above =   1111
  //              0000
  //    and
  //    left  =   1000
  //          =   1000
  // NOTE : In this example the low bit is left most ( 1000 ) is stored as
  //        1,  not 8...
  //
  // U and v set things on a 16 bit scale.
  //
  *above_y |= above_prediction_mask[block_size] << shift_y;
  *above_uv |= above_prediction_mask_uv[block_size] << shift_uv;
  *left_y |= left_prediction_mask[block_size] << shift_y;
  *left_uv |= left_prediction_mask_uv[block_size] << shift_uv;

  // If the block has no coefficients and is not intra we skip applying
  // the loop filter on block edges.
  if (skip && ref > INTRA_FRAME)
    return;

  // Here we are adding a mask for the transform size.  The transform
  // size mask is set to be correct for a 64x64 prediction block size. We
  // mask to match the size of the block we are working on and then shift it
  // into place..
  *above_y |= (size_mask[block_size] &
               above_64x64_txform_mask[tx_size_y]) << shift_y;
  *above_uv |= (size_mask_uv[block_size] &
                above_64x64_txform_mask_uv[tx_size_uv]) << shift_uv;

  *left_y |= (size_mask[block_size] &
              left_64x64_txform_mask[tx_size_y]) << shift_y;
  *left_uv |= (size_mask_uv[block_size] &
               left_64x64_txform_mask_uv[tx_size_uv]) << shift_uv;

  // Here we are trying to determine what to do with the internal 4x4 block
  // boundaries.  These differ from the 4x4 boundaries on the outside edge of
  // an 8x8 in that the internal ones can be skipped and don't depend on
  // the prediction block size.
  if (tx_size_y == TX_4X4) {
    *int_4x4_y |= (size_mask[block_size] & 0xffffffffffffffff) << shift_y;
  }
  if (tx_size_uv == TX_4X4) {
    *int_4x4_uv |= (size_mask_uv[block_size] & 0xffff) << shift_uv;
  }
}

// This function does the same thing as the one above with the exception that
// it only affects the y masks.   It exists because for blocks < 16x16 in size,
// we only update u and v masks on the first block.
static void build_y_mask(const loop_filter_info_n *const lfi_n,
                         const MODE_INFO *mi, const int shift_y,
                         LOOP_FILTER_MASK *lfm) {
  const BLOCK_SIZE block_size = mi->mbmi.sb_type;
  const TX_SIZE tx_size_y = mi->mbmi.tx_size;
  const int skip = mi->mbmi.skip_coeff;
  const int seg = mi->mbmi.segment_id;
  const int ref = mi->mbmi.ref_frame[0];
  const int filter_level = lfi_n->lvl[seg][ref][mode_lf_lut[mi->mbmi.mode]];
  uint64_t *left_y = &lfm->left_y[tx_size_y];
  uint64_t *above_y = &lfm->above_y[tx_size_y];
  uint64_t *int_4x4_y = &lfm->int_4x4_y;
  int i;
  int w = num_8x8_blocks_wide_lookup[block_size];
  int h = num_8x8_blocks_high_lookup[block_size];

  if (!filter_level) {
    return;
  } else {
    int index = shift_y;
    for (i = 0; i < h; i++) {
      vpx_memset(&lfm->lfl_y[index], filter_level, w);
      index += 8;
    }
  }

  *above_y |= above_prediction_mask[block_size] << shift_y;
  *left_y |= left_prediction_mask[block_size] << shift_y;

  if (skip && ref > INTRA_FRAME)
    return;

  *above_y |= (size_mask[block_size] &
               above_64x64_txform_mask[tx_size_y]) << shift_y;

  *left_y |= (size_mask[block_size] &
              left_64x64_txform_mask[tx_size_y]) << shift_y;

  if (tx_size_y == TX_4X4) {
    *int_4x4_y |= (size_mask[block_size] & 0xffffffffffffffff) << shift_y;
  }
}

// This function sets up the bit masks for the entire 64x64 region represented
// by mi_row, mi_col.
// TODO(JBB): This function only works for yv12.
static void setup_mask(VP9_COMMON *const cm, const int mi_row, const int mi_col,
                       MODE_INFO **mi_8x8, const int mode_info_stride,
                       LOOP_FILTER_MASK *lfm) {
  int idx_32, idx_16, idx_8;
  const loop_filter_info_n *const lfi_n = &cm->lf_info;
  MODE_INFO **mip = mi_8x8;
  MODE_INFO **mip2 = mi_8x8;

  // These are offsets to the next mi in the 64x64 block. It is what gets
  // added to the mi ptr as we go through each loop.  It helps us to avoids
  // setting up special row and column counters for each index.  The last step
  // brings us out back to the starting position.
  const int offset_32[] = {4, (mode_info_stride << 2) - 4, 4,
                           -(mode_info_stride << 2) - 4};
  const int offset_16[] = {2, (mode_info_stride << 1) - 2, 2,
                           -(mode_info_stride << 1) - 2};
  const int offset[] = {1, mode_info_stride - 1, 1, -mode_info_stride - 1};

  // Following variables represent shifts to position the current block
  // mask over the appropriate block.   A shift of 36 to the left will move
  // the bits for the final 32 by 32 block in the 64x64 up 4 rows and left
  // 4 rows to the appropriate spot.
  const int shift_32_y[] = {0, 4, 32, 36};
  const int shift_16_y[] = {0, 2, 16, 18};
  const int shift_8_y[] = {0, 1, 8, 9};
  const int shift_32_uv[] = {0, 2, 8, 10};
  const int shift_16_uv[] = {0, 1, 4, 5};
  int i;
  const int max_rows = (mi_row + MI_BLOCK_SIZE > cm->mi_rows ?
                        cm->mi_rows - mi_row : MI_BLOCK_SIZE);
  const int max_cols = (mi_col + MI_BLOCK_SIZE > cm->mi_cols ?
                        cm->mi_cols - mi_col : MI_BLOCK_SIZE);

  vp9_zero(*lfm);

  // TODO(jimbankoski): Try moving most of the following code into decode
  // loop and storing lfm in the mbmi structure so that we don't have to go
  // through the recursive loop structure multiple times.
  switch (mip[0]->mbmi.sb_type) {
    case BLOCK_64X64:
      build_masks(lfi_n, mip[0] , 0, 0, lfm);
      break;
    case BLOCK_64X32:
      build_masks(lfi_n, mip[0], 0, 0, lfm);
      mip2 = mip + mode_info_stride * 4;
      if (4 >= max_rows)
        break;
      build_masks(lfi_n, mip2[0], 32, 8, lfm);
      break;
    case BLOCK_32X64:
      build_masks(lfi_n, mip[0], 0, 0, lfm);
      mip2 = mip + 4;
      if (4 >= max_cols)
        break;
      build_masks(lfi_n, mip2[0], 4, 2, lfm);
      break;
    default:
      for (idx_32 = 0; idx_32 < 4; mip += offset_32[idx_32], ++idx_32) {
        const int shift_y = shift_32_y[idx_32];
        const int shift_uv = shift_32_uv[idx_32];
        const int mi_32_col_offset = ((idx_32 & 1) << 2);
        const int mi_32_row_offset = ((idx_32 >> 1) << 2);
        if (mi_32_col_offset >= max_cols || mi_32_row_offset >= max_rows)
          continue;
        switch (mip[0]->mbmi.sb_type) {
          case BLOCK_32X32:
            build_masks(lfi_n, mip[0], shift_y, shift_uv, lfm);
            break;
          case BLOCK_32X16:
            build_masks(lfi_n, mip[0], shift_y, shift_uv, lfm);
            if (mi_32_row_offset + 2 >= max_rows)
              continue;
            mip2 = mip + mode_info_stride * 2;
            build_masks(lfi_n, mip2[0], shift_y + 16, shift_uv + 4, lfm);
            break;
          case BLOCK_16X32:
            build_masks(lfi_n, mip[0], shift_y, shift_uv, lfm);
            if (mi_32_col_offset + 2 >= max_cols)
              continue;
            mip2 = mip + 2;
            build_masks(lfi_n, mip2[0], shift_y + 2, shift_uv + 1, lfm);
            break;
          default:
            for (idx_16 = 0; idx_16 < 4; mip += offset_16[idx_16], ++idx_16) {
              const int shift_y = shift_32_y[idx_32] + shift_16_y[idx_16];
              const int shift_uv = shift_32_uv[idx_32] + shift_16_uv[idx_16];
              const int mi_16_col_offset = mi_32_col_offset +
                  ((idx_16 & 1) << 1);
              const int mi_16_row_offset = mi_32_row_offset +
                  ((idx_16 >> 1) << 1);

              if (mi_16_col_offset >= max_cols || mi_16_row_offset >= max_rows)
                continue;

              switch (mip[0]->mbmi.sb_type) {
                case BLOCK_16X16:
                  build_masks(lfi_n, mip[0], shift_y, shift_uv, lfm);
                  break;
                case BLOCK_16X8:
                  build_masks(lfi_n, mip[0], shift_y, shift_uv, lfm);
                  if (mi_16_row_offset + 1 >= max_rows)
                    continue;
                  mip2 = mip + mode_info_stride;
                  build_y_mask(lfi_n, mip2[0], shift_y+8, lfm);
                  break;
                case BLOCK_8X16:
                  build_masks(lfi_n, mip[0], shift_y, shift_uv, lfm);
                  if (mi_16_col_offset +1 >= max_cols)
                    continue;
                  mip2 = mip + 1;
                  build_y_mask(lfi_n, mip2[0], shift_y+1, lfm);
                  break;
                default: {
                  const int shift_y = shift_32_y[idx_32] +
                                      shift_16_y[idx_16] +
                                      shift_8_y[0];
                  build_masks(lfi_n, mip[0], shift_y, shift_uv, lfm);
                  mip += offset[0];
                  for (idx_8 = 1; idx_8 < 4; mip += offset[idx_8], ++idx_8) {
                    const int shift_y = shift_32_y[idx_32] +
                                        shift_16_y[idx_16] +
                                        shift_8_y[idx_8];
                    const int mi_8_col_offset = mi_16_col_offset +
                        ((idx_8 & 1));
                    const int mi_8_row_offset = mi_16_row_offset +
                        ((idx_8 >> 1));

                    if (mi_8_col_offset >= max_cols ||
                        mi_8_row_offset >= max_rows)
                      continue;
                    build_y_mask(lfi_n, mip[0], shift_y, lfm);
                  }
                  break;
                }
              }
            }
            break;
        }
      }
      break;
  }
  // The largest loopfilter we have is 16x16 so we use the 16x16 mask
  // for 32x32 transforms also also.
  lfm->left_y[TX_16X16] |= lfm->left_y[TX_32X32];
  lfm->above_y[TX_16X16] |= lfm->above_y[TX_32X32];
  lfm->left_uv[TX_16X16] |= lfm->left_uv[TX_32X32];
  lfm->above_uv[TX_16X16] |= lfm->above_uv[TX_32X32];

  // We do at least 8 tap filter on every 32x32 even if the transform size
  // is 4x4.  So if the 4x4 is set on a border pixel add it to the 8x8 and
  // remove it from the 4x4.
  lfm->left_y[TX_8X8] |= lfm->left_y[TX_4X4] & left_border;
  lfm->left_y[TX_4X4] &= ~left_border;
  lfm->above_y[TX_8X8] |= lfm->above_y[TX_4X4] & above_border;
  lfm->above_y[TX_4X4] &= ~above_border;
  lfm->left_uv[TX_8X8] |= lfm->left_uv[TX_4X4] & left_border_uv;
  lfm->left_uv[TX_4X4] &= ~left_border_uv;
  lfm->above_uv[TX_8X8] |= lfm->above_uv[TX_4X4] & above_border_uv;
  lfm->above_uv[TX_4X4] &= ~above_border_uv;

  // We do some special edge handling.
  if (mi_row + MI_BLOCK_SIZE > cm->mi_rows) {
    const uint64_t rows = cm->mi_rows - mi_row;

    // Each pixel inside the border gets a 1,
    const uint64_t mask_y = (((uint64_t) 1 << (rows << 3)) - 1);
    const uint16_t mask_uv = (((uint16_t) 1 << (((rows + 1) >> 1) << 2)) - 1);

    // Remove values completely outside our border.
    for (i = 0; i < TX_32X32; i++) {
      lfm->left_y[i] &= mask_y;
      lfm->above_y[i] &= mask_y;
      lfm->left_uv[i] &= mask_uv;
      lfm->above_uv[i] &= mask_uv;
    }
    lfm->int_4x4_y &= mask_y;
    lfm->int_4x4_uv &= mask_uv;

    // We don't apply a wide loop filter on the last uv block row.  If set
    // apply the shorter one instead.
    if (rows == 1) {
      lfm->above_uv[TX_8X8] |= lfm->above_uv[TX_16X16];
      lfm->above_uv[TX_16X16] = 0;
    }
    if (rows == 5) {
      lfm->above_uv[TX_8X8] |= lfm->above_uv[TX_16X16] & 0xff00;
      lfm->above_uv[TX_16X16] &= ~(lfm->above_uv[TX_16X16] & 0xff00);
    }
  }

  if (mi_col + MI_BLOCK_SIZE > cm->mi_cols) {
    const uint64_t columns = cm->mi_cols - mi_col;

    // Each pixel inside the border gets a 1, the multiply copies the border
    // to where we need it.
    const uint64_t mask_y  = (((1 << columns) - 1)) * 0x0101010101010101;
    const uint16_t mask_uv = ((1 << ((columns + 1) >> 1)) - 1) * 0x1111;

    // Internal edges are not applied on the last column of the image so
    // we mask 1 more for the internal edges
    const uint16_t mask_uv_int = ((1 << (columns >> 1)) - 1) * 0x1111;

    // Remove the bits outside the image edge.
    for (i = 0; i < TX_32X32; i++) {
      lfm->left_y[i] &= mask_y;
      lfm->above_y[i] &= mask_y;
      lfm->left_uv[i] &= mask_uv;
      lfm->above_uv[i] &= mask_uv;
    }
    lfm->int_4x4_y &= mask_y;
    lfm->int_4x4_uv &= mask_uv_int;

    // We don't apply a wide loop filter on the last uv column.  If set
    // apply the shorter one instead.
    if (columns == 1) {
      lfm->left_uv[TX_8X8] |= lfm->left_uv[TX_16X16];
      lfm->left_uv[TX_16X16] = 0;
    }
    if (columns == 5) {
      lfm->left_uv[TX_8X8] |= (lfm->left_uv[TX_16X16] & 0xcccc);
      lfm->left_uv[TX_16X16] &= ~(lfm->left_uv[TX_16X16] & 0xcccc);
    }
  }
  // We don't a loop filter on the first column in the image.  Mask that out.
  if (mi_col == 0) {
    for (i = 0; i < TX_32X32; i++) {
      lfm->left_y[i] &= 0xfefefefefefefefe;
      lfm->left_uv[i] &= 0xeeee;
    }
  }

  // Assert if we try to apply 2 different loop filters at the same position.
  assert(!(lfm->left_y[TX_16X16] & lfm->left_y[TX_8X8]));
  assert(!(lfm->left_y[TX_16X16] & lfm->left_y[TX_4X4]));
  assert(!(lfm->left_y[TX_8X8] & lfm->left_y[TX_4X4]));
  assert(!(lfm->int_4x4_y & lfm->left_y[TX_16X16]));
  assert(!(lfm->left_uv[TX_16X16]&lfm->left_uv[TX_8X8]));
  assert(!(lfm->left_uv[TX_16X16] & lfm->left_uv[TX_4X4]));
  assert(!(lfm->left_uv[TX_8X8] & lfm->left_uv[TX_4X4]));
  assert(!(lfm->int_4x4_uv & lfm->left_uv[TX_16X16]));
  assert(!(lfm->above_y[TX_16X16] & lfm->above_y[TX_8X8]));
  assert(!(lfm->above_y[TX_16X16] & lfm->above_y[TX_4X4]));
  assert(!(lfm->above_y[TX_8X8] & lfm->above_y[TX_4X4]));
  assert(!(lfm->int_4x4_y & lfm->above_y[TX_16X16]));
  assert(!(lfm->above_uv[TX_16X16] & lfm->above_uv[TX_8X8]));
  assert(!(lfm->above_uv[TX_16X16] & lfm->above_uv[TX_4X4]));
  assert(!(lfm->above_uv[TX_8X8] & lfm->above_uv[TX_4X4]));
  assert(!(lfm->int_4x4_uv & lfm->above_uv[TX_16X16]));
}

#if CONFIG_NON420
static uint8_t build_lfi(const loop_filter_info_n *lfi_n,
                     const MB_MODE_INFO *mbmi) {
  const int seg = mbmi->segment_id;
  const int ref = mbmi->ref_frame[0];
  return lfi_n->lvl[seg][ref][mode_lf_lut[mbmi->mode]];
}

static void filter_selectively_vert(uint8_t *s, int pitch,
                                    unsigned int mask_16x16,
                                    unsigned int mask_8x8,
                                    unsigned int mask_4x4,
                                    unsigned int mask_4x4_int,
                                    const loop_filter_info_n *lfi_n,
                                    const uint8_t *lfl) {
  unsigned int mask;

  for (mask = mask_16x16 | mask_8x8 | mask_4x4 | mask_4x4_int;
       mask; mask >>= 1) {
    const loop_filter_thresh *lfi = lfi_n->lfthr + *lfl;

    if (mask & 1) {
      if (mask_16x16 & 1) {
        vp9_lpf_vertical_16(s, pitch, lfi->mblim, lfi->lim, lfi->hev_thr);
      } else if (mask_8x8 & 1) {
        vp9_lpf_vertical_8(s, pitch, lfi->mblim, lfi->lim, lfi->hev_thr, 1);
      } else if (mask_4x4 & 1) {
        vp9_lpf_vertical_4(s, pitch, lfi->mblim, lfi->lim, lfi->hev_thr, 1);
      }
    }
    if (mask_4x4_int & 1)
      vp9_lpf_vertical_4(s + 4, pitch, lfi->mblim, lfi->lim, lfi->hev_thr, 1);
    s += 8;
    lfl += 1;
    mask_16x16 >>= 1;
    mask_8x8 >>= 1;
    mask_4x4 >>= 1;
    mask_4x4_int >>= 1;
  }
}

static void filter_block_plane_non420(VP9_COMMON *cm,
                                      struct macroblockd_plane *plane,
                                      MODE_INFO **mi_8x8,
                                      int mi_row, int mi_col) {
  const int ss_x = plane->subsampling_x;
  const int ss_y = plane->subsampling_y;
  const int row_step = 1 << ss_x;
  const int col_step = 1 << ss_y;
  const int row_step_stride = cm->mode_info_stride * row_step;
  struct buf_2d *const dst = &plane->dst;
  uint8_t* const dst0 = dst->buf;
  unsigned int mask_16x16[MI_BLOCK_SIZE] = {0};
  unsigned int mask_8x8[MI_BLOCK_SIZE] = {0};
  unsigned int mask_4x4[MI_BLOCK_SIZE] = {0};
  unsigned int mask_4x4_int[MI_BLOCK_SIZE] = {0};
  uint8_t lfl[MI_BLOCK_SIZE * MI_BLOCK_SIZE];
  int r, c;

  for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < cm->mi_rows; r += row_step) {
    unsigned int mask_16x16_c = 0;
    unsigned int mask_8x8_c = 0;
    unsigned int mask_4x4_c = 0;
    unsigned int border_mask;

    // Determine the vertical edges that need filtering
    for (c = 0; c < MI_BLOCK_SIZE && mi_col + c < cm->mi_cols; c += col_step) {
      const MODE_INFO *mi = mi_8x8[c];
      const BLOCK_SIZE sb_type = mi[0].mbmi.sb_type;
      const int skip_this = mi[0].mbmi.skip_coeff
                            && is_inter_block(&mi[0].mbmi);
      // left edge of current unit is block/partition edge -> no skip
      const int block_edge_left = (num_4x4_blocks_wide_lookup[sb_type] > 1) ?
          !(c & (num_8x8_blocks_wide_lookup[sb_type] - 1)) : 1;
      const int skip_this_c = skip_this && !block_edge_left;
      // top edge of current unit is block/partition edge -> no skip
      const int block_edge_above = (num_4x4_blocks_high_lookup[sb_type] > 1) ?
          !(r & (num_8x8_blocks_high_lookup[sb_type] - 1)) : 1;
      const int skip_this_r = skip_this && !block_edge_above;
      const TX_SIZE tx_size = (plane->plane_type == PLANE_TYPE_UV)
                            ? get_uv_tx_size(&mi[0].mbmi)
                            : mi[0].mbmi.tx_size;
      const int skip_border_4x4_c = ss_x && mi_col + c == cm->mi_cols - 1;
      const int skip_border_4x4_r = ss_y && mi_row + r == cm->mi_rows - 1;

      // Filter level can vary per MI
      if (!(lfl[(r << 3) + (c >> ss_x)] =
          build_lfi(&cm->lf_info, &mi[0].mbmi)))
        continue;

      // Build masks based on the transform size of each block
      if (tx_size == TX_32X32) {
        if (!skip_this_c && ((c >> ss_x) & 3) == 0) {
          if (!skip_border_4x4_c)
            mask_16x16_c |= 1 << (c >> ss_x);
          else
            mask_8x8_c |= 1 << (c >> ss_x);
        }
        if (!skip_this_r && ((r >> ss_y) & 3) == 0) {
          if (!skip_border_4x4_r)
            mask_16x16[r] |= 1 << (c >> ss_x);
          else
            mask_8x8[r] |= 1 << (c >> ss_x);
        }
      } else if (tx_size == TX_16X16) {
        if (!skip_this_c && ((c >> ss_x) & 1) == 0) {
          if (!skip_border_4x4_c)
            mask_16x16_c |= 1 << (c >> ss_x);
          else
            mask_8x8_c |= 1 << (c >> ss_x);
        }
        if (!skip_this_r && ((r >> ss_y) & 1) == 0) {
          if (!skip_border_4x4_r)
            mask_16x16[r] |= 1 << (c >> ss_x);
          else
            mask_8x8[r] |= 1 << (c >> ss_x);
        }
      } else {
        // force 8x8 filtering on 32x32 boundaries
        if (!skip_this_c) {
          if (tx_size == TX_8X8 || ((c >> ss_x) & 3) == 0)
            mask_8x8_c |= 1 << (c >> ss_x);
          else
            mask_4x4_c |= 1 << (c >> ss_x);
        }

        if (!skip_this_r) {
          if (tx_size == TX_8X8 || ((r >> ss_y) & 3) == 0)
            mask_8x8[r] |= 1 << (c >> ss_x);
          else
            mask_4x4[r] |= 1 << (c >> ss_x);
        }

        if (!skip_this && tx_size < TX_8X8 && !skip_border_4x4_c)
          mask_4x4_int[r] |= 1 << (c >> ss_x);
      }
    }

    // Disable filtering on the leftmost column
    border_mask = ~(mi_col == 0);
    filter_selectively_vert(dst->buf, dst->stride,
                            mask_16x16_c & border_mask,
                            mask_8x8_c & border_mask,
                            mask_4x4_c & border_mask,
                            mask_4x4_int[r],
                            &cm->lf_info, &lfl[r << 3]);
    dst->buf += 8 * dst->stride;
    mi_8x8 += row_step_stride;
  }

  // Now do horizontal pass
  dst->buf = dst0;
  for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < cm->mi_rows; r += row_step) {
    const int skip_border_4x4_r = ss_y && mi_row + r == cm->mi_rows - 1;
    const unsigned int mask_4x4_int_r = skip_border_4x4_r ? 0 : mask_4x4_int[r];

    unsigned int mask_16x16_r;
    unsigned int mask_8x8_r;
    unsigned int mask_4x4_r;

    if (mi_row + r == 0) {
      mask_16x16_r = 0;
      mask_8x8_r = 0;
      mask_4x4_r = 0;
    } else {
      mask_16x16_r = mask_16x16[r];
      mask_8x8_r = mask_8x8[r];
      mask_4x4_r = mask_4x4[r];
    }

    filter_selectively_horiz(dst->buf, dst->stride,
                             mask_16x16_r,
                             mask_8x8_r,
                             mask_4x4_r,
                             mask_4x4_int_r,
                             &cm->lf_info, &lfl[r << 3]);
    dst->buf += 8 * dst->stride;
  }
}
#endif

static void filter_block_plane(VP9_COMMON *const cm,
                               struct macroblockd_plane *const plane,
                               int mi_row,
                               LOOP_FILTER_MASK *lfm) {
  struct buf_2d *const dst = &plane->dst;
  uint8_t* const dst0 = dst->buf;
  int r, c;

  if (!plane->plane_type) {
    uint64_t mask_16x16 = lfm->left_y[TX_16X16];
    uint64_t mask_8x8 = lfm->left_y[TX_8X8];
    uint64_t mask_4x4 = lfm->left_y[TX_4X4];
    uint64_t mask_4x4_int = lfm->int_4x4_y;

    // Vertical pass: do 2 rows at one time
    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < cm->mi_rows; r += 2) {
      unsigned int mask_16x16_l = mask_16x16 & 0xffff;
      unsigned int mask_8x8_l = mask_8x8 & 0xffff;
      unsigned int mask_4x4_l = mask_4x4 & 0xffff;
      unsigned int mask_4x4_int_l = mask_4x4_int & 0xffff;

      // Disable filtering on the leftmost column
      filter_selectively_vert_row2(plane->plane_type,
                                   dst->buf, dst->stride,
                                   mask_16x16_l,
                                   mask_8x8_l,
                                   mask_4x4_l,
                                   mask_4x4_int_l,
                                   &cm->lf_info, &lfm->lfl_y[r << 3]);

      dst->buf += 16 * dst->stride;
      mask_16x16 >>= 16;
      mask_8x8 >>= 16;
      mask_4x4 >>= 16;
      mask_4x4_int >>= 16;
    }

    // Horizontal pass
    dst->buf = dst0;
    mask_16x16 = lfm->above_y[TX_16X16];
    mask_8x8 = lfm->above_y[TX_8X8];
    mask_4x4 = lfm->above_y[TX_4X4];
    mask_4x4_int = lfm->int_4x4_y;

    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < cm->mi_rows; r++) {
      unsigned int mask_16x16_r;
      unsigned int mask_8x8_r;
      unsigned int mask_4x4_r;

      if (mi_row + r == 0) {
        mask_16x16_r = 0;
        mask_8x8_r = 0;
        mask_4x4_r = 0;
      } else {
        mask_16x16_r = mask_16x16 & 0xff;
        mask_8x8_r = mask_8x8 & 0xff;
        mask_4x4_r = mask_4x4 & 0xff;
      }

      filter_selectively_horiz(dst->buf, dst->stride,
                               mask_16x16_r,
                               mask_8x8_r,
                               mask_4x4_r,
                               mask_4x4_int & 0xff,
                               &cm->lf_info, &lfm->lfl_y[r << 3]);

      dst->buf += 8 * dst->stride;
      mask_16x16 >>= 8;
      mask_8x8 >>= 8;
      mask_4x4 >>= 8;
      mask_4x4_int >>= 8;
    }
  } else {
    uint16_t mask_16x16 = lfm->left_uv[TX_16X16];
    uint16_t mask_8x8 = lfm->left_uv[TX_8X8];
    uint16_t mask_4x4 = lfm->left_uv[TX_4X4];
    uint16_t mask_4x4_int = lfm->int_4x4_uv;

    // Vertical pass: do 2 rows at one time
    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < cm->mi_rows; r += 4) {
      if (plane->plane_type == 1) {
        for (c = 0; c < (MI_BLOCK_SIZE >> 1); c++) {
          lfm->lfl_uv[(r << 1) + c] = lfm->lfl_y[(r << 3) + (c << 1)];
          lfm->lfl_uv[((r + 2) << 1) + c] = lfm->lfl_y[((r + 2) << 3) +
                                                       (c << 1)];
        }
      }

      {
        unsigned int mask_16x16_l = mask_16x16 & 0xff;
        unsigned int mask_8x8_l = mask_8x8 & 0xff;
        unsigned int mask_4x4_l = mask_4x4 & 0xff;
        unsigned int mask_4x4_int_l = mask_4x4_int & 0xff;

        // Disable filtering on the leftmost column
        filter_selectively_vert_row2(plane->plane_type,
                                     dst->buf, dst->stride,
                                     mask_16x16_l,
                                     mask_8x8_l,
                                     mask_4x4_l,
                                     mask_4x4_int_l,
                                     &cm->lf_info, &lfm->lfl_uv[r << 1]);

        dst->buf += 16 * dst->stride;
        mask_16x16 >>= 8;
        mask_8x8 >>= 8;
        mask_4x4 >>= 8;
        mask_4x4_int >>= 8;
      }
    }

    // Horizontal pass
    dst->buf = dst0;
    mask_16x16 = lfm->above_uv[TX_16X16];
    mask_8x8 = lfm->above_uv[TX_8X8];
    mask_4x4 = lfm->above_uv[TX_4X4];
    mask_4x4_int = lfm->int_4x4_uv;

    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < cm->mi_rows; r += 2) {
      const int skip_border_4x4_r = mi_row + r == cm->mi_rows - 1;
      const unsigned int mask_4x4_int_r = skip_border_4x4_r ?
          0 : (mask_4x4_int & 0xf);
      unsigned int mask_16x16_r;
      unsigned int mask_8x8_r;
      unsigned int mask_4x4_r;

      if (mi_row + r == 0) {
        mask_16x16_r = 0;
        mask_8x8_r = 0;
        mask_4x4_r = 0;
      } else {
        mask_16x16_r = mask_16x16 & 0xf;
        mask_8x8_r = mask_8x8 & 0xf;
        mask_4x4_r = mask_4x4 & 0xf;
      }

      filter_selectively_horiz(dst->buf, dst->stride,
                               mask_16x16_r,
                               mask_8x8_r,
                               mask_4x4_r,
                               mask_4x4_int_r,
                               &cm->lf_info, &lfm->lfl_uv[r << 1]);

      dst->buf += 8 * dst->stride;
      mask_16x16 >>= 4;
      mask_8x8 >>= 4;
      mask_4x4 >>= 4;
      mask_4x4_int >>= 4;
    }
  }
}

void vp9_loop_filter_rows(const YV12_BUFFER_CONFIG *frame_buffer,
                          VP9_COMMON *cm, MACROBLOCKD *xd,
                          int start, int stop, int y_only) {
  const int num_planes = y_only ? 1 : MAX_MB_PLANE;
  int mi_row, mi_col;
  LOOP_FILTER_MASK lfm;
#if CONFIG_NON420
  int use_420 = y_only || (xd->plane[1].subsampling_y == 1 &&
      xd->plane[1].subsampling_x == 1);
#endif

  for (mi_row = start; mi_row < stop; mi_row += MI_BLOCK_SIZE) {
    MODE_INFO **mi_8x8 = cm->mi_grid_visible + mi_row * cm->mode_info_stride;

    for (mi_col = 0; mi_col < cm->mi_cols; mi_col += MI_BLOCK_SIZE) {
      int plane;

      setup_dst_planes(xd, frame_buffer, mi_row, mi_col);

      // TODO(JBB): Make setup_mask work for non 420.
#if CONFIG_NON420
      if (use_420)
#endif
        setup_mask(cm, mi_row, mi_col, mi_8x8 + mi_col, cm->mode_info_stride,
                   &lfm);

      disable_filter(&lfm, mi_col, mi_row, 0, 0);
      for (plane = 0; plane < num_planes; ++plane) {
#if CONFIG_NON420
        if (use_420)
#endif
          filter_block_plane(cm, &xd->plane[plane], mi_row, &lfm);
#if CONFIG_NON420
        else
          filter_block_plane_non420(cm, &xd->plane[plane], mi_8x8 + mi_col,
                                    mi_row, mi_col);
#endif
      }
    }
  }
}

#define kUsecsInSec 1000.0
//I MUST USE STRIDE, TODO
typedef struct {
  uint8_t * y_buf;
  uint8_t * u_buf;
  uint8_t * v_buf;
} cuda_buf;

void opencl_loopfilter(LOOP_FILTER_MASK* lfm,
                     cuda_loop_filter_thresh* lft,
                     uint32_t y_rows,
                     uint32_t y_cols,
                     uint8_t * y_buf,
                     uint32_t uv_rows,
                     uint32_t uv_cols,
                     uint8_t * u_buf,
                     uint8_t * v_buf);

cuda_buf vp9_cuda_loop_filter_frame(const YV12_BUFFER_CONFIG *frame_buffer,
                          VP9_COMMON *cm, MACROBLOCKD *xd,
                          int start, int stop, int y_only) {
 // const int num_planes = y_only ? 1 : MAX_MB_PLANE;
  cuda_buf c_b;
  uint32_t size, s, r, c,ic, ir,stride;
  int mi_row, mi_col;//, plane;
  uint32_t y_rows, y_cols, uv_rows, uv_cols;
  uint32_t lfm_rows = (stop + MI_SIZE - 1) / MI_SIZE;
  uint32_t lfm_cols = (cm->mi_cols + MI_SIZE - 1) / MI_SIZE;
  uint32_t lfm_size = lfm_rows*lfm_cols*sizeof(LOOP_FILTER_MASK);
  uint32_t lft_size = (MAX_LOOP_FILTER + 1) * sizeof(cuda_loop_filter_thresh);
  cuda_loop_filter_thresh *lft = malloc(lft_size);
  LOOP_FILTER_MASK* lfm = malloc(lfm_size);
  uint8_t* y_buf;
  uint8_t* u_buf;
  uint8_t* v_buf;
  FILE * pFile;
  //printf("r %d c %d so %d lfm %d\n", lfm_rows, lfm_cols, sizeof(LOOP_FILTER_MASK), lfm_size);

  memset((void*)lfm, 0, lfm_size);

  y_cols = frame_buffer->y_width;
  y_rows = frame_buffer->y_height;
  uv_cols = frame_buffer->uv_width;
  uv_rows = frame_buffer->uv_height;
 // printf("rows %d cols %d\n", rows, cols);
  y_buf = malloc(y_rows * y_cols);
  memset(y_buf, 0, y_rows * y_cols);

  u_buf = malloc(uv_rows * uv_cols);
  memset(u_buf, 0, uv_rows * uv_cols);

  v_buf = malloc(uv_rows * uv_cols);
  memset(v_buf, 0, uv_rows * uv_cols);

  //stride = xd->plane[0].dst.stride;

  for (r = 0; r < y_rows; r++) {
    for (c = 0; c < y_cols; c++ ) {
      y_buf[r * y_cols + c] = frame_buffer->y_buffer[r * frame_buffer->y_stride + c];
    }
  }
  for (r = 0; r < uv_rows; r++) {
    for (c = 0; c < uv_cols; c++ ) {
      u_buf[r * uv_cols + c] = frame_buffer->u_buffer[r * frame_buffer->uv_stride + c];
    }
  }
  for (r = 0; r < uv_rows; r++) {
    for (c = 0; c < uv_cols; c++ ) {
      v_buf[r * uv_cols + c] = frame_buffer->v_buffer[r * frame_buffer->uv_stride + c];
    }
  }

  printf("%dx%d %dx%d\n", y_rows, y_cols, uv_rows, uv_cols);

  for (mi_row = start, r = 0; mi_row < stop; mi_row += MI_BLOCK_SIZE, r++) {
    MODE_INFO **mi_8x8 = cm->mi_grid_visible + mi_row * cm->mode_info_stride;

    for (mi_col = 0, c = 0; mi_col < cm->mi_cols; mi_col += MI_BLOCK_SIZE, c++) {
     // plane = 0;

     // setup_dst_planes(xd, frame_buffer, mi_row, mi_col);
    // printf("offset-%d\n", r * lfm_cols + c);
      setup_mask(cm, mi_row, mi_col, mi_8x8 + mi_col, cm->mode_info_stride,
                 &lfm[r * lfm_cols + c]);
      LOOP_FILTER_MASK* mask = &lfm[r * lfm_cols + c];
      if (mi_row == start) {

        mask->above_y[TX_16X16] = (mask->above_y[TX_16X16] >> MI_SIZE) << MI_SIZE;
        mask->above_y[TX_8X8] = (mask->above_y[TX_8X8] >> MI_SIZE) << MI_SIZE;
        mask->above_y[TX_4X4] = (mask->above_y[TX_4X4] >> MI_SIZE) << MI_SIZE;

        mask->above_uv[TX_16X16] = (mask->above_uv[TX_16X16] >> (MI_SIZE / 2)) << (MI_SIZE / 2);
        mask->above_uv[TX_8X8] = (mask->above_uv[TX_8X8] >> (MI_SIZE / 2)) << (MI_SIZE / 2);
        mask->above_uv[TX_4X4] = (mask->above_uv[TX_4X4] >> (MI_SIZE / 2)) << (MI_SIZE / 2);
      }
   //   printf("%04x\n", mask->int_4x4_uv);
      if (mi_row + MI_BLOCK_SIZE >= stop) {
        mask->int_4x4_uv = (mask->int_4x4_uv << (MI_SIZE / 2)) >> (MI_SIZE / 2);
      }
      for (ir = 0; ir < MI_BLOCK_SIZE && mi_row + ir < cm->mi_rows; ir += 2) {
        for (ic = 0; ic < (MI_BLOCK_SIZE >> 1); ic++)
          mask->lfl_uv[(ir << 1) + ic] = mask->lfl_y[(ir << 3) + (ic << 1)];
      }
      disable_filter(&lfm[r*lfm_cols + c], mi_col, mi_row, 0, 0);

    }
  }

  memset(lft, 0, lft_size);





 // memcpy(buf, xd->plane[0].dst.buf, rows * cols);
  for (size = 0; size < MAX_LOOP_FILTER + 1; size++) {
      lft[size].mblim = cm->lf_info.lfthr[size].mblim[0];
      lft[size].lim = cm->lf_info.lfthr[size].lim[0];
      lft[size].hev_thr = cm->lf_info.lfthr[size].hev_thr[0];
  }
  double elapsed_secs;
  vpx_usec_timer g;
  vpx_usec_timer_start(&g);


  ocl_loopfilter(lfm, lft, y_rows, y_cols, y_buf, uv_rows, uv_cols, u_buf, v_buf);
  vpx_usec_timer_mark(&g);
  elapsed_secs = (double)(vpx_usec_timer_elapsed(&g))
                              / kUsecsInSec;

  printf("GPU - %f\n", elapsed_secs);

 // dump_buf(buf, NULL, rows, cols, cols, "post-gpu.txt");
  //saveRedImg(buf, rows, cols, cols, "post-gpu.ppm");

  free(lfm);
  //free(u_buf);
  //free(v_buf);

  c_b.y_buf = y_buf;
  c_b.u_buf = u_buf;
  c_b.v_buf = v_buf;
  return c_b;

}
void compareBuf(uint8_t* y_buf, uint8_t* u_buf, uint8_t* v_buf, const YV12_BUFFER_CONFIG *frame_buffer) {
  uint32_t r, c,rows, cols;
  int mismatch = 0;
  cols = frame_buffer->y_width;
  rows = frame_buffer->y_height;
  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++ ) {
      if (y_buf[r * cols + c] != frame_buffer->y_buffer[r * frame_buffer->y_stride + c]) {
        printf ("Y(%03d,%03d) %03u %03u\n", c, r, y_buf[r*cols + c], frame_buffer->y_buffer[r * frame_buffer->y_stride + c]);
        mismatch = 1;
      }
    }
  }
/*
  cols = frame_buffer->uv_width;
  rows = frame_buffer->uv_height;
  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++ ) {
      if (u_buf[r * cols + c] != frame_buffer->u_buffer[r * frame_buffer->uv_stride + c]) {
        printf ("U(%03d,%03d) %03u %03u\n", c, r, u_buf[r*cols + c], frame_buffer->u_buffer[r * frame_buffer->uv_stride + c]);
        mismatch = 1;
      }
    }
  }

  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++ ) {
      if (v_buf[r * cols + c] != frame_buffer->v_buffer[r * frame_buffer->uv_stride + c]) {
        printf ("V(%03d,%03d) %03u %03u\n", c, r, v_buf[r*cols + c], frame_buffer->v_buffer[r * frame_buffer->uv_stride + c]);
        mismatch = 1;
      }
    }
  }*/

  assert(mismatch == 0);
  printf("CRC OK\n");
}


void vp9_loop_filter_frame(VP9_COMMON *cm, MACROBLOCKD *xd,
                           int frame_filter_level,
                           int y_only, int partial) {
  int start_mi_row, end_mi_row, mi_rows_to_filter;
  vpx_usec_timer g, c;
  cuda_buf c_b;
  double elapsed_secs = 0;
  if (!frame_filter_level) return;
  start_mi_row = 0;
  mi_rows_to_filter = cm->mi_rows;
  if (partial && cm->mi_rows > 8) {
    start_mi_row = cm->mi_rows >> 1;
    start_mi_row &= 0xfffffff8;
    mi_rows_to_filter = MAX(cm->mi_rows / 8, 8);
  }
  end_mi_row = start_mi_row + mi_rows_to_filter;
  vp9_loop_filter_frame_init(cm, frame_filter_level);
  printf("----NEXT_FRAME----%d\n", cm->current_video_frame);
  vpx_usec_timer_start(&g);

  c_b = vp9_cuda_loop_filter_frame(cm->frame_to_show, cm, xd,
                       start_mi_row, end_mi_row,
                       y_only);
  vpx_usec_timer_mark(&g);
  elapsed_secs = (double)(vpx_usec_timer_elapsed(&g))
                              / kUsecsInSec;

  //printf("GPU - %f\n", elapsed_secs);

  printf("___CPU___\n");
//  vpx_usec_timer_mark(&s);
  vpx_usec_timer_start(&c);
  vp9_loop_filter_rows(cm->frame_to_show, cm, xd,
                       start_mi_row, end_mi_row,
                       y_only);
  vpx_usec_timer_mark(&c);
 // vpx_usec_timer_start(&s);
  elapsed_secs = (double)(vpx_usec_timer_elapsed(&c))
                              / kUsecsInSec;


  printf("CPU - %f\n", elapsed_secs);
  compareBuf(c_b.y_buf, c_b.u_buf, c_b.v_buf, cm->frame_to_show);
  free(c_b.y_buf);
  free(c_b.u_buf);
  free(c_b.v_buf);
}

int vp9_loop_filter_worker(void *arg1, void *arg2) {
  LFWorkerData *const lf_data = (LFWorkerData*)arg1;
  (void)arg2;
  vp9_loop_filter_rows(lf_data->frame_buffer, lf_data->cm, &lf_data->xd,
                       lf_data->start, lf_data->stop, lf_data->y_only);
  return 1;
}

// TESTING

#define MAX_LOOP_FILTER 63
#define MI_SIZE 8
#define Y_LFL_SHIFT 3
#define UV_LFL_SHIFT 1
#define SUPER_BLOCK_DIM 64

typedef struct {
  cl_context context;
  cl_command_queue y_command_queue;
  cl_command_queue u_command_queue;
  cl_command_queue v_command_queue;
  cl_program program;
  cl_kernel clear_kernel;
  cl_kernel filter_kernel;
} ocl_context;

void ocl_init_frame(ocl_context *oc) {

  static cl_context context = NULL;
  static cl_kernel clear_kernel = NULL;
  static cl_kernel filter_kernel = NULL;
  static cl_command_queue y_command_queue = NULL;
  static cl_command_queue u_command_queue = NULL;
  static cl_command_queue v_command_queue = NULL;
  static cl_program program = NULL;
  static cl_kernel kernel = NULL;

  if (context == NULL) {
    cl_device_id device_id = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    char string[MEM_SIZE];

    FILE *fp;
    char fileName[] = "./vp9/common/cuda_loopfilter.cl";
    char *source_str;
    size_t source_size;

    /* Load the source code containing the kernel*/
    fp = fopen(fileName, "r");
    if (!fp) {
      fprintf(stderr, "Failed to load kernel.\n");
      exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    /* Get Platform and Device Info */
    //cl_platform_id pid[2];
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    //clGetPlatformIDs(2, pid, &ret_num_platforms);
    //platform_id = pid[1];
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);

    // TEST CODE
    cl_device_id devices[100];
      cl_uint devices_n = 0;
      int i = 0,g = 0;
      clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 100, devices, &devices_n);
      //clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 100, devices, &devices_n);
      for (i=0; i<devices_n; i++)
      {
        char buffer[10240];
        cl_uint buf_uint;
        size_t gs;
        size_t item_sizes[1024];
        cl_ulong buf_ulong;
        printf("  -- %d --\n", i);
        ret = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &gs, NULL);
        printf("  Max work group size=%d\n", gs);
        ret = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &buf_uint, NULL);
        ret = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(item_sizes), item_sizes, NULL);
        for(g = 0; g < buf_uint;g++) {
          printf("  %d Max work item size=%d\n", g, item_sizes[g]);
        }
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
        printf("  DEVICE_NAME = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL);
        printf("  DEVICE_VENDOR = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL);
        printf("  DEVICE_VERSION = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL);
        printf("  DRIVER_VERSION = %s\n", buffer);
        clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL);
        printf("  DEVICE_MAX_COMPUTE_UNITS = %u\n", (unsigned int)buf_uint);
        clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL);
        printf("  DEVICE_MAX_CLOCK_FREQUENCY = %u\n", (unsigned int)buf_uint);
        clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
        printf("  DEVICE_GLOBAL_MEM_SIZE = %llu\n", (unsigned long long)buf_ulong);
      }



    //end test
    /* Create OpenCL context */
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

    /* Create Command Queue */
    y_command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    u_command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    v_command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    /* Create Kernel Program from the source */
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
      (const size_t *)&source_size, &ret);

    /* Build Kernel Program */
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (ret == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
    }
    clear_kernel = clCreateKernel(program, "fill_pattern", &ret);
    filter_kernel = clCreateKernel(program, "filter_all", &ret);
    free(source_str);
  }
  oc->context = context;
  oc->y_command_queue = y_command_queue;
  oc->u_command_queue = u_command_queue;
  oc->v_command_queue = v_command_queue;
  oc->program = program;
  oc->clear_kernel = clear_kernel;
  oc->filter_kernel = filter_kernel;
}

typedef struct {
  uint8_t * buf;
  cl_mem d_buf;
  cl_int rows;
  cl_int cols;
  cl_mem d_col_col_filtered;
  cl_mem d_col_row_filtered;
  cl_uint step_max;
  cl_uint lvl_shift;
  cl_int sb_rows;
  cl_int sb_cols;
  cl_uint row_mult;
  cl_int is_y;
  size_t local_ws;
  size_t global_ws;
  cl_mem d_masks;
  cl_mem d_lft;
  cl_command_queue q;

} ocl_filter_info;

void ocl_setup_mutex(ocl_context *oc, ocl_filter_info *f) {
  cl_int ret;
  size_t local_ws = f->sb_rows;
  size_t global_ws = f->sb_rows;
  size_t mutex_size = f->sb_rows * sizeof(int32_t);

  f->d_col_col_filtered =
    clCreateBuffer(oc->context, CL_MEM_READ_WRITE,
      mutex_size, 0, &ret);

  f->d_col_row_filtered =
    clCreateBuffer(oc->context, CL_MEM_READ_WRITE,
      mutex_size, 0, &ret);

  uint32_t fill_pattern = 0xFFFFFFFF;

  // clear first mutex
  clSetKernelArg(oc->clear_kernel, 0, sizeof(cl_mem), &f->d_col_col_filtered);
  clSetKernelArg(oc->clear_kernel, 1, sizeof(cl_int), &fill_pattern);
  clEnqueueNDRangeKernel(f->q, oc->clear_kernel, 1, NULL,
    &global_ws, &local_ws, 0, NULL, NULL);

  // clear second mutex
  clSetKernelArg(oc->clear_kernel, 0, sizeof(cl_mem), &f->d_col_row_filtered);
  clSetKernelArg(oc->clear_kernel, 1, sizeof(cl_int), &fill_pattern);
  clEnqueueNDRangeKernel(f->q, oc->clear_kernel, 1, NULL,
    &global_ws, &local_ws, 0, NULL, NULL);

}


void ocl_queue_filter(ocl_context *oc, ocl_filter_info *f){

  cl_int ret;
  uint32_t buf_size = f->rows * f->cols * sizeof(uint8_t);
  f->d_buf = clCreateBuffer(oc->context, CL_MEM_READ_WRITE |
    CL_MEM_COPY_HOST_PTR, buf_size, f->buf, &ret);

  clSetKernelArg(oc->filter_kernel, 0, sizeof(cl_mem), &f->d_masks);
  clSetKernelArg(oc->filter_kernel, 1, sizeof(cl_mem), &f->d_lft);
  clSetKernelArg(oc->filter_kernel, 2, sizeof(cl_mem), &f->d_col_row_filtered);
  clSetKernelArg(oc->filter_kernel, 3, sizeof(cl_mem), &f->d_col_col_filtered);
  clSetKernelArg(oc->filter_kernel, 4, sizeof(cl_int), &f->sb_rows);
  clSetKernelArg(oc->filter_kernel, 5, sizeof(cl_int), &f->sb_cols);
  clSetKernelArg(oc->filter_kernel, 6, sizeof(cl_int), &f->rows);
  clSetKernelArg(oc->filter_kernel, 7, sizeof(cl_int), &f->cols);
  clSetKernelArg(oc->filter_kernel, 8, sizeof(cl_mem), &f->d_buf);
  clSetKernelArg(oc->filter_kernel, 9, sizeof(cl_int), &f->is_y);
  clSetKernelArg(oc->filter_kernel, 10, sizeof(cl_uint), &f->step_max);
  clSetKernelArg(oc->filter_kernel, 11, sizeof(cl_uint), &f->lvl_shift);
  clSetKernelArg(oc->filter_kernel, 12, sizeof(cl_uint), &f->row_mult);
  clEnqueueNDRangeKernel(f->q, oc->filter_kernel, 1, NULL,
    &f->global_ws, &f->local_ws, 0, NULL, NULL);

  clEnqueueReadBuffer(f->q, f->d_buf, CL_TRUE, 0,
    buf_size, f->buf, 0, NULL, NULL);
}

void ocl_cleanup_plane(ocl_filter_info *f) {
  clFinish(f->q);
  clReleaseMemObject(f->d_buf);
  clReleaseMemObject(f->d_col_col_filtered);
  clReleaseMemObject(f->d_col_row_filtered);
}

void ocl_loopfilter(LOOP_FILTER_MASK* lfm,
                     cuda_loop_filter_thresh* lft,
                     uint32_t y_rows,
                     uint32_t y_cols,
                     uint8_t * y_buf,
                     uint32_t uv_rows,
                     uint32_t uv_cols,
                     uint8_t * u_buf,
                     uint8_t * v_buf) {
  ocl_context oc;
  cl_int ret;
  ocl_init_frame(&oc);

  /* Set OpenCL Kernel Parameters */
  cl_mem d_lft = clCreateBuffer(oc.context, CL_MEM_READ_ONLY |
      CL_MEM_COPY_HOST_PTR, (MAX_LOOP_FILTER + 1)
      * sizeof(cuda_loop_filter_thresh), lft, &ret);

  uint32_t sb_rows = (y_rows + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_cols = (y_cols + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_count = sb_rows * sb_cols;
  uint32_t mask_bytes = sb_count * sizeof(LOOP_FILTER_MASK);
  cl_mem d_masks = clCreateBuffer(oc.context, CL_MEM_READ_ONLY |
      CL_MEM_COPY_HOST_PTR, mask_bytes, lfm, &ret);

  // setup y plane
  ocl_filter_info y, u, v;
  y.sb_rows = sb_rows;
  y.sb_cols = sb_cols;
  y.rows = y_rows;
  y.cols = y_cols;
  y.buf = y_buf;
  y.d_masks = d_masks;
  y.d_lft = d_lft;
  y.is_y = 1;
  y.step_max = MI_SIZE;
  y.lvl_shift = Y_LFL_SHIFT;
  y.row_mult = 1;
  y.local_ws = SUPER_BLOCK_DIM;
  y.global_ws = sb_rows * SUPER_BLOCK_DIM;
  y.q = oc.y_command_queue;

  // setup u plane
  u.sb_rows = sb_rows;
  u.sb_cols = sb_cols;
  u.rows = uv_rows;
  u.cols = uv_cols;
  u.d_masks = d_masks;
  u.d_lft = d_lft;
  u.buf = u_buf;
  u.is_y = 0;
  u.step_max = MI_SIZE / 2;
  u.lvl_shift = UV_LFL_SHIFT;
  u.row_mult = 2;
  u.local_ws = SUPER_BLOCK_DIM / 2;
  u.global_ws = sb_rows * (SUPER_BLOCK_DIM / 2);
  u.q = oc.u_command_queue;

  //setup v plane
  v.sb_rows = sb_rows;
  v.sb_cols = sb_cols;
  v.rows = uv_rows;
  v.cols = uv_cols;
  v.d_masks = d_masks;
  v.d_lft = d_lft;
  v.buf = v_buf;
  v.is_y = 0;
  v.step_max = MI_SIZE / 2;
  v.lvl_shift = UV_LFL_SHIFT;
  v.row_mult = 2;
  v.local_ws = SUPER_BLOCK_DIM / 2;
  v.global_ws = sb_rows * (SUPER_BLOCK_DIM / 2);
  v.q = oc.v_command_queue;

  //setup mutexes
  ocl_setup_mutex(&oc, &y);
  ocl_setup_mutex(&oc, &u);
  ocl_setup_mutex(&oc, &v);

  // run filters on each plane
  ocl_queue_filter(&oc, &y);
  ocl_queue_filter(&oc, &u);
  ocl_queue_filter(&oc, &v);

  /* Finalization */
  clFlush(y.q);
  clFlush(u.q);
  clFlush(v.q);

  ocl_cleanup_plane(&u);
  ocl_cleanup_plane(&v);
  ocl_cleanup_plane(&y);

 // ret = clReleaseKernel(kernel);
 // ret = clReleaseProgram(program);
  clReleaseMemObject(d_lft);
  clReleaseMemObject(d_masks);
//  ret = clReleaseContext(context);

//  free(source_str);
}
