#include "./vpx_config.h"
#include "./vpx_scale_rtcd.h"
#include "./vp9_rtcd.h"
#include "./vp8_rtcd.h"

#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_postproc.h"

void vp9_filter_by_weight32x32(uint8_t *src, int src_stride, uint8_t *dst,
                               int dst_stride, int weight) {
  vp8_filter_by_weight16x16(src, src_stride, dst, dst_stride, weight);
  vp8_filter_by_weight16x16(src + 16, src_stride, dst + 16,
                            dst_stride, weight);
  vp8_filter_by_weight16x16(src + src_stride * 16, src_stride,
                            dst + dst_stride * 16, dst_stride, weight);
  vp8_filter_by_weight16x16(src + src_stride * 16 + 16, src_stride,
                            dst + dst_stride * 16 + 16, dst_stride, weight);
}

void vp9_filter_by_weight64x64(uint8_t *src, int src_stride, uint8_t *dst,
                               int dst_stride, int weight) {
  vp9_filter_by_weight32x32(src, src_stride, dst, dst_stride, weight);
  vp9_filter_by_weight32x32(src + 32, src_stride, dst + 32,
                            dst_stride, weight);
  vp9_filter_by_weight32x32(src + src_stride * 32, src_stride,
                            dst + dst_stride * 32, dst_stride, weight);
  vp9_filter_by_weight32x32(src + src_stride * 32 + 32, src_stride,
                            dst + dst_stride * 32 + 32, dst_stride, weight);
}

static void vp9_apply_ifactor(uint8_t *y, int y_stride, uint8_t *yd,
                              int yd_stride, uint8_t *u, uint8_t *v,
                              int uv_stride, uint8_t *ud, uint8_t *vd,
                              int uvd_stride, BLOCK_SIZE block_size,
                              int weight) {
  if (block_size == BLOCK_16X16) {
    vp8_filter_by_weight16x16(y, y_stride, yd, yd_stride, weight);
    vp8_filter_by_weight8x8(u, uv_stride, ud, uvd_stride, weight);
    vp8_filter_by_weight8x8(v, uv_stride, vd, uvd_stride, weight);
  } else if (block_size == BLOCK_32X32) {
    vp9_filter_by_weight32x32(y, y_stride, yd, yd_stride, weight);
    vp8_filter_by_weight16x16(u, uv_stride, ud, uvd_stride, weight);
    vp8_filter_by_weight16x16(v, uv_stride, vd, uvd_stride, weight);
  } else if (block_size == BLOCK_64X64) {
    vp9_filter_by_weight64x64(y, y_stride, yd, yd_stride, weight);
    vp9_filter_by_weight32x32(u, uv_stride, ud, uvd_stride, weight);
    vp9_filter_by_weight32x32(v, uv_stride, vd, uvd_stride, weight);
  }
}

void vp9_copy_mem16x16(uint8_t *src, int src_stride,
                       uint8_t *dst, int dst_stride) {
  vp8_copy_mem16x16(src, src_stride, dst, dst_stride);
}

void vp9_copy_mem32x32(uint8_t *src, int src_stride,
                       uint8_t *dst, int dst_stride) {
  vp8_copy_mem16x16(src, src_stride, dst, dst_stride);
  vp8_copy_mem16x16(src + 16, src_stride, dst + 16, dst_stride);
  vp8_copy_mem16x16(src + src_stride * 16, src_stride,
                    dst + dst_stride * 16, dst_stride);
  vp8_copy_mem16x16(src + src_stride * 16 + 16, src_stride,
                    dst + dst_stride * 16 + 16, dst_stride);
}

void vp9_copy_mem64x64(uint8_t *src, int src_stride,
                       uint8_t *dst, int dst_stride) {
  vp9_copy_mem32x32(src, src_stride, dst, dst_stride);
  vp9_copy_mem32x32(src + 32, src_stride, dst + 32, dst_stride);
  vp9_copy_mem32x32(src + src_stride * 32, src_stride,
                    dst + src_stride * 32, dst_stride);
  vp9_copy_mem32x32(src + src_stride * 32 + 32, src_stride,
                    dst + src_stride * 32 + 32, dst_stride);
}

static void vp9_memcpy_block(uint8_t *y, uint8_t *u, uint8_t *v,
                             int y_stride, int uv_stride, uint8_t *yd,
                             uint8_t *ud, uint8_t *vd, int yd_stride,
                             int uvd_stride, BLOCK_SIZE bs) {
  if (bs == BLOCK_16X16) {
    vp9_copy_mem16x16(y, y_stride, yd, yd_stride);
    vp8_copy_mem8x8(u, uv_stride, ud, uvd_stride);
    vp8_copy_mem8x8(v, uv_stride, vd, uvd_stride);
  } else if (bs == BLOCK_32X32) {
    vp9_copy_mem32x32(y, y_stride, yd, yd_stride);
    vp8_copy_mem16x16(u, uv_stride, ud, uvd_stride);
    vp8_copy_mem16x16(v, uv_stride, vd, uvd_stride);
  } else {
    vp9_copy_mem64x64(y, y_stride, yd, yd_stride);
    vp9_copy_mem32x32(u, uv_stride, ud, uvd_stride);
    vp9_copy_mem32x32(v, uv_stride, vd, uvd_stride);
  }
}

int max_vdiff = 0;
void vp9_mfqe_block(BLOCK_SIZE bs, uint8_t *y, uint8_t *u,
                    uint8_t *v, int y_stride, int uv_stride,
                    uint8_t *yd, uint8_t *ud, uint8_t *vd,
                    int yd_stride, int uvd_stride) {
  int sad, vdiff, sad_thr = 8;
  uint32_t sse;

  if (bs == BLOCK_16X16) {
    vdiff = (vp9_variance16x16(y, y_stride, yd, yd_stride, &sse) + 128) >> 8;
    sad = (vp9_sad16x16(y, y_stride, yd, yd_stride) + 128) >> 8;
  } else if (bs == BLOCK_32X32) {
    vdiff = (vp9_variance32x32(y, y_stride, yd, yd_stride, &sse) + 512) >> 10;
    sad = (vp9_sad32x32(y, y_stride, yd, yd_stride) + 512) >> 10;
  } else /* if (bs == BLOCK_64X64) */ {
    vdiff = (vp9_variance64x64(y, y_stride, yd, yd_stride, &sse) + 2048) >> 12;
    sad = (vp9_sad64x64(y, y_stride, yd, yd_stride) + 2048) >> 12;
  }

  if (bs == BLOCK_16X16) {
    sad_thr = 8;
  } else if (bs == BLOCK_32X32) {
    sad_thr = 7;
  } else { // BLOCK_64X64
    sad_thr = 6;
  }

  // TODO(jackychen): More experiments and remove magic numbers.
  if (sad > 1 && sad < sad_thr && vdiff > sad * 3 && vdiff < 150) {
    // TODO(jackychen): Add weighted average in the calculation.
    // Currently, the data is copied from last frame without averaging.
    vp9_apply_ifactor(y, y_stride, yd, yd_stride, u, v, uv_stride,
                      ud, vd, uvd_stride, bs, 0);
  } else {
    // Copy the block from current frame.
    vp9_memcpy_block(y, u, v, y_stride, uv_stride, yd, ud, vd,
                     yd_stride, uvd_stride, bs);
  }
}

static int mv_threshold() {
  return 100;
}

// Process each partiton in a super block, recursively.
void vp9_mfqe_partition(VP9_COMMON *cm, MODE_INFO *mi, BLOCK_SIZE bs,
                        uint8_t *y, uint8_t *u, uint8_t *v, int y_stride,
                        int uv_stride, uint8_t *yd, uint8_t *ud, uint8_t *vd,
                        int yd_stride, int uvd_stride) {
  BLOCK_SIZE cur_bs = mi->mbmi.sb_type;
  // TODO(jackychen): Consider how and whether to use qdiff in MFQE .
  //int qdiff = cm->base_qindex - cm->postproc_state.last_base_qindex;
  int mv_len_square = 0;

  if (cur_bs == 0) {
    // If there are 4x4 blocks, it must be on the boundary.
    return;
  } else if (bs - cur_bs > 0 && bs - cur_bs <= 2) {
    // If current block size is not square, copy the block from current frame.
    // TODO(jackychen): Rectangle blocks should also be taken into account.
    vp9_memcpy_block(y, u, v, y_stride, uv_stride, yd, ud, vd,
                     yd_stride, uvd_stride, bs);
  } else if (cur_bs == bs || bs == BLOCK_16X16) {
    // Check the motion in current block(for inter frame),
    // or check the motion in the correlated block in last frame(for keyframe).
    mv_len_square = mi->mbmi.mv[0].as_mv.row * mi->mbmi.mv[0].as_mv.row +
                    mi->mbmi.mv[0].as_mv.col * mi->mbmi.mv[0].as_mv.col;
    if (mi->mbmi.mode >= NEARESTMV && cur_bs >= BLOCK_16X16
        && mv_len_square <= mv_threshold()) {
      // Do mfqe on this partition.
      vp9_mfqe_block(cur_bs, y, u, v, y_stride, uv_stride,
                     yd, ud, vd, yd_stride, uvd_stride);
    } else {
      // Copy the block from current frame.
      vp9_memcpy_block(y, u, v, y_stride, uv_stride, yd, ud, vd,
                       yd_stride, uvd_stride, bs);
    }
  } else if(bs - cur_bs > 2) {
    // There are sub-partition in current block.
    int mi_offset, y_offset, uv_offset;
    if (bs == BLOCK_64X64) {
      mi_offset = 4;
      y_offset = 32;
      uv_offset = 16;
    } else {
      mi_offset = 2;
      y_offset = 16;
      uv_offset = 8;
    }
    // Recursion on four square partitions, e.g. if bs is 64X64,
    // then look into four 32X32 blocks in it.
    vp9_mfqe_partition(cm, mi, bs - 3, y, u, v, y_stride, uv_stride,
                       yd, ud, vd, yd_stride, uvd_stride);
    vp9_mfqe_partition(cm, mi + mi_offset, bs - 3, y + y_offset,
                       u + uv_offset, v + uv_offset,
                       y_stride, uv_stride, yd + y_offset,
                       ud + uv_offset, vd + uv_offset,
                       yd_stride, uvd_stride);
    vp9_mfqe_partition(cm, mi + mi_offset * cm->mi_stride, bs - 3,
                       y + y_offset * y_stride, u + uv_offset * uv_stride,
                       v + uv_offset * uv_stride, y_stride, uv_stride,
                       yd + y_offset * yd_stride, ud + uv_offset * uvd_stride,
                       vd + uv_offset * uvd_stride, yd_stride, uvd_stride);
    vp9_mfqe_partition(cm, mi + mi_offset * cm->mi_stride + mi_offset,
                       bs - 3, y + y_offset * y_stride + y_offset,
                       u + uv_offset * uv_stride + uv_offset,
                       v + uv_offset * uv_stride + uv_offset, y_stride,
                       uv_stride, yd + y_offset * yd_stride + y_offset,
                       ud + uv_offset * uvd_stride + uv_offset,
                       vd + uv_offset * uvd_stride + uv_offset,
                       yd_stride, uvd_stride);
  }
}

void vp9_mfqe(VP9_COMMON *cm) {
  int mi_row, mi_col;
  // Current frame.
  YV12_BUFFER_CONFIG *show = cm->frame_to_show;
  // Last frame and the result of the processing.
  YV12_BUFFER_CONFIG *dest = &cm->post_proc_buffer;
  // Loop through each super block.
  for (mi_row = 0; mi_row < (cm->height >> 3); mi_row += MI_BLOCK_SIZE) {
    for (mi_col = 0; mi_col < (cm->width >> 3); mi_col += MI_BLOCK_SIZE) {
      MODE_INFO *mi, *mi_local = cm->mi + (mi_row * cm->mi_stride + mi_col);
      // Motion Info in last frame.
      MODE_INFO *prev_mi = cm->postproc_state.prev_mi +
                           (mi_row * cm->mi_stride + mi_col);
      uint32_t y_stride = show->y_stride;
      uint32_t uv_stride = show->uv_stride;
      uint32_t yd_stride = dest->y_stride;
      uint32_t uvd_stride = dest->uv_stride;
      uint8_t *y = show->y_buffer + (mi_row << 3) * y_stride + (mi_col << 3);
      uint8_t *u = show->u_buffer + (mi_row << 2) * uv_stride + (mi_col << 2);
      uint8_t *v = show->v_buffer + (mi_row << 2) * uv_stride + (mi_col << 2);
      uint8_t *yd = dest->y_buffer + (mi_row << 3) * yd_stride + (mi_col << 3);
      uint8_t *ud = dest->u_buffer + (mi_row << 2) * uvd_stride +
                    (mi_col << 2);
      uint8_t *vd = dest->v_buffer + (mi_row << 2) * uvd_stride +
                    (mi_col << 2);
      if (cm->frame_type == KEY_FRAME || cm->intra_only) {
        mi = prev_mi;
      } else {
        mi = mi_local;
      }
      vp9_mfqe_partition(cm, mi, BLOCK_64X64, y, u, v, y_stride, uv_stride,
                         yd, ud, vd, yd_stride, uvd_stride);
    }
  }
}
