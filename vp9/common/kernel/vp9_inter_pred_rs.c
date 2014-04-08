/*
 * Copyright (C) 2013 MultiCoreWare, Inc. All rights reserved.
 * XinSu <xin@multicorewareinc.com>
 */

#include <assert.h>

#include "./vpx_scale_rtcd.h"
#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_filter.h"
#include "vp9/common/vp9_reconinter.h"
#include "vp9/common/vp9_reconintra.h"
#include "vp9/common/vp9_scale.h"
#include "vp9/common/kernel/vp9_inter_pred_rs.h"
#include "vp9/common/kernel/vp9_convolve_rs_c.h"

#include <stdio.h>
#include <dlfcn.h>
#define LOGI(...) fprintf(stdout, __VA_ARGS__)
#define LOGE(...) fprintf(stderr, __VA_ARGS__)

#define STABLE_THREAD_COUNT_RS 18375
#define STABLE_BUFFER_SIZE_RS  4704000
#define MAX_TILE 4

static void build_mc_border(const uint8_t *src, int src_stride,
                            uint8_t *dst, int dst_stride,
                            int x, int y, int b_w, int b_h, int w, int h) {
  // Get a pointer to the start of the real data for this row.
  const uint8_t *ref_row = src - x - y * src_stride;
  if (y >= h)
    ref_row += (h - 1) * src_stride;
  else if (y > 0)
    ref_row += y * src_stride;

  do {
    int right = 0, copy;
    int left = x < 0 ? -x : 0;

    if (left > b_w)
      left = b_w;

    if (x + b_w > w)
      right = x + b_w - w;

    if (right > b_w)
      right = b_w;

    copy = b_w - left - right;

    if (left)
      memset(dst, ref_row[0], left);

    if (copy)
      memcpy(dst + left, ref_row + x + left, copy);

    if (right)
      memset(dst + left + copy, ref_row[w - 1], right);

    dst += dst_stride;
    ++y;

    if (y > 0 && y < h)
      ref_row += src_stride;
  } while (--b_h);
}


typedef struct {
  void *lib_handle;
  int (*init_rs)(int pred_param_size, int param_index_size, int buff_size,
                 int pool_size, int global_size, int tile_index,
                 uint8_t *block_param, uint8_t *index_param,
                 uint8_t *dst_buf, uint8_t *mid_buf);
  void (*release_rs)(int tile_count);
  void (*update_pool_rs)(unsigned char *per_buf, int frame_num);
  void (*get_rs_res)(unsigned char *dst_buf, int frame_num);
  void (*invoke_inter_rs)(unsigned char *dst_buf, unsigned char *block_param,
                          unsigned char *index_param, int tile_index,
                          int counts_8x8, int block_param_size,
                          int index_param_size);
  void (*create_buffer_rs)(int pred_param_size, int param_index_size,
                           int buf_size, int pool_size, int global_size,
                           int tile_count, uint8_t **block, uint8_t **index,
                           uint8_t **buf, uint8_t **mid_buf);
  void (*update_buffer_size_rs)(int pred_param_size, int param_index_size,
                               int buf_size, int pool_size);

  void (*release_buffer_rs)(int tile_count);

} LIB_CONTEXT;
LIB_CONTEXT inter_rs_context;
int rs_init = 1;
INTER_RS_OBJ inter_rs_obj = { 0 };

static const int16_t *inter_filter_rs[4] = {
                                  vp9_sub_pel_filters_8[0],
                                  vp9_sub_pel_filters_8lp[0],
                                  vp9_sub_pel_filters_8s[0],
                                  vp9_bilinear_filters[0]};

int vp9_setup_interp_filters_rs(MACROBLOCKD *xd,
    INTERPOLATION_TYPE mcomp_filter_type,
    VP9_COMMON *cm) {
  int ret_filter_num = 0;

  switch (mcomp_filter_type) {
    case EIGHTTAP:
    case SWITCHABLE:
      ret_filter_num = 0;
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8;
      break;
    case EIGHTTAP_SMOOTH:
      ret_filter_num = 1;
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8lp;
      break;
    case EIGHTTAP_SHARP:
      ret_filter_num = 2;
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8s;
      break;
    case BILINEAR:
      ret_filter_num = 3;
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_bilinear_filters;
      break;
  }
  assert(((intptr_t)xd->subpix.filter_x & 0xff) == 0);

  return ret_filter_num;
}

static INLINE int round_mv_comp_q4(int value) {
  return (value < 0 ? value - 2 : value + 2) / 4;
}

static MV mi_mv_pred_q4(const MODE_INFO *mi, int idx) {
  MV res = { round_mv_comp_q4(mi->bmi[0].as_mv[idx].as_mv.row +
      mi->bmi[1].as_mv[idx].as_mv.row +
      mi->bmi[2].as_mv[idx].as_mv.row +
      mi->bmi[3].as_mv[idx].as_mv.row),
  round_mv_comp_q4(mi->bmi[0].as_mv[idx].as_mv.col +
      mi->bmi[1].as_mv[idx].as_mv.col +
      mi->bmi[2].as_mv[idx].as_mv.col +
      mi->bmi[3].as_mv[idx].as_mv.col) };
  return res;
}

static MV clamp_mv_to_umv_border_sb(const MACROBLOCKD *xd, const MV *src_mv,
    int bw, int bh, int ss_x, int ss_y) {
  const int spel_left = (VP9_INTERP_EXTEND + bw) << SUBPEL_BITS;
  const int spel_right = spel_left - SUBPEL_SHIFTS;
  const int spel_top = (VP9_INTERP_EXTEND + bh) << SUBPEL_BITS;
  const int spel_bottom = spel_top - SUBPEL_SHIFTS;
  MV clamped_mv = {
    src_mv->row * (1 << (1 - ss_y)),
    src_mv->col * (1 << (1 - ss_x))
  };
  assert(ss_x <= 1);
  assert(ss_y <= 1);

  clamp_mv(&clamped_mv,
      xd->mb_to_left_edge * (1 << (1 - ss_x)) - spel_left,
      xd->mb_to_right_edge * (1 << (1 - ss_x)) + spel_right,
      xd->mb_to_top_edge * (1 << (1 - ss_y)) - spel_top,
      xd->mb_to_bottom_edge * (1 << (1 - ss_y)) + spel_bottom);

  return clamped_mv;
}

static void update_buffer(int tile_count, int param_count_cpu,
                          int param_count_gpu) {
  uint8_t *pblock[MAX_TILE];
  uint8_t *pindex[MAX_TILE];
  uint8_t *pbuf[MAX_TILE];
  uint8_t *pmid[MAX_TILE];
  int i = 0;
  for (i = 0; i < tile_count; ++i) {
    inter_rs_obj.pred_param_cpu_fri[i] =
        MALLOC_INTER_RS(INTER_PRED_PARAM_CPU_RS, param_count_cpu);
    inter_rs_obj.pred_param_cpu_sec[i] =
        inter_rs_obj.pred_param_cpu_fri[i] + (param_count_cpu >> 1);
    inter_rs_obj.pred_param_gpu[i] =
        MALLOC_INTER_RS(INTER_PRED_PARAM_GPU_RS, param_count_gpu);
    inter_rs_obj.pred_param_index_gpu[i] =
        MALLOC_INTER_RS(INTER_PARAM_INDEX_GPU_RS, param_count_gpu);
    inter_rs_obj.pred_block_size_gpu[i] =
        MALLOC_INTER_RS(INTER_BLOCK_SIZE_GPU_RS, param_count_gpu);

    inter_rs_obj.ref_buffer[i] = (uint8_t *)malloc(inter_rs_obj.buffer_size);
    inter_rs_obj.pref[i] = inter_rs_obj.ref_buffer[i];

    inter_rs_obj.new_buffer[i] = (uint8_t *)malloc(inter_rs_obj.buffer_size);
    inter_rs_obj.mid_buffer[i] = (uint8_t *)malloc(inter_rs_obj.buffer_size);
    assert(inter_rs_obj.pred_param_gpu[i] != NULL);
    assert(inter_rs_obj.pred_param_cpu_fri[i] != NULL);
    assert(inter_rs_obj.pred_block_size_gpu[i] != NULL);
    assert(inter_rs_obj.pred_param_index_gpu[i] != NULL);

    inter_rs_obj.pred_param_cpu_fri_pre[i] =
        inter_rs_obj.pred_param_cpu_fri[i];
    inter_rs_obj.pred_param_cpu_sec_pre[i] =
        inter_rs_obj.pred_param_cpu_sec[i];
    inter_rs_obj.pred_param_gpu_pre[i] =
        inter_rs_obj.pred_param_gpu[i];
    inter_rs_obj.pred_param_index_gpu_pre[i] =
        inter_rs_obj.pred_param_index_gpu[i];
    inter_rs_obj.pred_block_size_gpu_pre[i] =
        inter_rs_obj.pred_block_size_gpu[i];
    inter_rs_obj.cpu_fri_count[i] = 0;
    inter_rs_obj.cpu_sec_count[i] = 0;
    inter_rs_obj.gpu_block_count[i] = 0;
    inter_rs_obj.gpu_index_count[i] = 0;
  }
  for (i = 0; i < tile_count; i++) {
    pblock[i] = (uint8_t *)inter_rs_obj.pred_param_gpu[i];
    pindex[i] = (uint8_t *)inter_rs_obj.pred_param_index_gpu[i];
    pbuf[i]   = inter_rs_obj.new_buffer[i];
    pmid[i]   = inter_rs_obj.mid_buffer[i];
  }
  inter_rs_context.create_buffer_rs(
            inter_rs_obj.pred_param_size,
            inter_rs_obj.param_index_size,
            inter_rs_obj.buffer_size,
            inter_rs_obj.buffer_pool_size,
            inter_rs_obj.globalThreads, tile_count,
            (uint8_t **)&pblock, (uint8_t **)&pindex, (uint8_t **)&pbuf,
            (uint8_t **)&pmid);
}

void build_inter_pred_param_sec_ref_rs(const int plane,
                                        const int block,
                                        BLOCK_SIZE bsize, void *argv,
                                        VP9_COMMON *const cm,
                                        const int src_num,
                                        const int filter_num,
                                        const int tile_num) {
  int xs, ys;
  int filter_radix;
  int subpel_x, subpel_y;
  const int16_t *filter;
  const uint8_t *src_fri, *dst_fri;
  const uint8_t *dst;

  MV32 scaled_mv;
  MV mv, mv_q4;
  struct scale_factors *sf;
  struct subpix_fn_table *subpix;
  struct buf_2d *pre_buf, *dst_buf;
  const YV12_BUFFER_CONFIG *cfg_src;
  const YV12_BUFFER_CONFIG *cfg_dst;
  int x0, y0, x0_16, y0_16, x1, y1, frame_width,
      frame_height, buf_stride;
  uint8_t *ref_frame, *buf_ptr;
  int w, h;
  const INTER_PRED_ARGS_RS * const arg = argv;
  MACROBLOCKD *const xd = arg->xd;
  int mi_x = arg->x;
  int mi_y = arg->y;
  struct macroblockd_plane *const pd = &xd->plane[plane];
  const YV12_BUFFER_CONFIG *ref_buf = xd->block_refs[1]->buf;

  const BLOCK_SIZE plane_bsize = get_plane_block_size(bsize, pd);
  const int bwl = b_width_log2(plane_bsize);
  const int bw = 4 << bwl;
  const int bh = 4 * num_4x4_blocks_high_lookup[plane_bsize];
  const int x = 4 * (block & ((1 << bwl) - 1));
  const int y = 4 * (block >> bwl);
  const MODE_INFO *mi = xd->mi_8x8[0];

  int pred_w = b_width_log2(bsize) - xd->plane[plane].subsampling_x;
  int pred_h = b_height_log2(bsize) - xd->plane[plane].subsampling_y;

  if (mi->mbmi.sb_type < BLOCK_8X8 && !plane) {
    pred_w = 0;
    pred_h = 0;
  }
  sf = &xd->block_refs[1]->sf;

  pre_buf = &pd->pre[1];
  dst_buf = &pd->dst;

  dst = dst_buf->buf + dst_buf->stride * y + x;

  mv = mi->mbmi.sb_type < BLOCK_8X8
       ? (plane == 0 ? mi->bmi[block].as_mv[1].as_mv
                       : mi_mv_pred_q4(mi, 1))
       : mi->mbmi.mv[1].as_mv;

  mv_q4 = clamp_mv_to_umv_border_sb(xd, &mv, bw, bh,
                                    pd->subsampling_x,
                                    pd->subsampling_y);

  w = 4 << pred_w;
  h = 4 << pred_h;
  // Get reference frame pointer, width and height.
  if (plane == 0) {
    frame_width = ref_buf->y_crop_width;
    frame_height = ref_buf->y_crop_height;
    ref_frame = ref_buf->y_buffer;
  } else {
    frame_width = ref_buf->uv_crop_width;
    frame_height = ref_buf->uv_crop_height;
    ref_frame = plane == 1 ? ref_buf->u_buffer : ref_buf->v_buffer;
  }
  x0 = (-xd->mb_to_left_edge >> (3 + pd->subsampling_x)) + x;
  y0 = (-xd->mb_to_top_edge >> (3 + pd->subsampling_y)) + y;
  x0_16 = x0 << SUBPEL_BITS;
  y0_16 = y0 << SUBPEL_BITS;

  if (vp9_is_scaled(sf)) {
    scaled_mv = vp9_scale_mv(&mv_q4, mi_x + x, mi_y + y, sf);
    xs = sf->x_step_q4;
    ys = sf->y_step_q4;
    x0 = sf->scale_value_x(x0, sf);
    y0 = sf->scale_value_y(y0, sf);
    x0_16 = sf->scale_value_x(x0_16, sf);
    y0_16 = sf->scale_value_y(y0_16, sf);
  } else {
    scaled_mv.row = mv_q4.row;
    scaled_mv.col = mv_q4.col;
    xs = ys = 16;
  }

  subpix = &xd->subpix;
  subpel_x = scaled_mv.col & SUBPEL_MASK;
  subpel_y = scaled_mv.row & SUBPEL_MASK;

  x0 += scaled_mv.col >> SUBPEL_BITS;
  y0 += scaled_mv.row >> SUBPEL_BITS;
  x0_16 += scaled_mv.col;
  y0_16 += scaled_mv.row;

  x1 = ((x0_16 + (w - 1) * xs) >> SUBPEL_BITS) + 1;
  y1 = ((y0_16 + (h - 1) * ys) >> SUBPEL_BITS) + 1;
  buf_ptr = ref_frame + y0 * pre_buf->stride + x0;
  buf_stride = pre_buf->stride;
  if (scaled_mv.col || scaled_mv.row ||
      (frame_width & 0x7) || (frame_height & 0x7)) {
    int x_pad = 0, y_pad = 0;

    if (subpel_x || (sf->x_step_q4 & SUBPEL_MASK)) {
      x0 -= VP9_INTERP_EXTEND - 1;
      x1 += VP9_INTERP_EXTEND;
      x_pad = 1;
    }

    if (subpel_y || (sf->y_step_q4 & SUBPEL_MASK)) {
      y0 -= VP9_INTERP_EXTEND - 1;
      y1 += VP9_INTERP_EXTEND;
      y_pad = 1;
    }
    if (x0 < 0 || x0 > frame_width - 1 || x1 < 0 || x1 > frame_width ||
          y0 < 0 || y0 > frame_height - 1 || y1 < 0 || y1 > frame_height - 1) {
      uint8_t *buf_ptr1 = ref_frame + y0 * pre_buf->stride + x0;
        // Extend the border.
      build_mc_border(buf_ptr1, pre_buf->stride, inter_rs_obj.pref[tile_num], x1 - x0,
                     x0, y0, x1 - x0, y1 - y0, frame_width, frame_height);
      buf_stride = x1 - x0;
      buf_ptr = inter_rs_obj.pref[tile_num] + y_pad * 3 * buf_stride + x_pad * 3;
      inter_rs_obj.pref[tile_num] += (x1 - x0) * (y1 - y0);
    }
  }

  cfg_src = &cm->yv12_fb[src_num];
  cfg_dst = &cm->yv12_fb[cm->new_fb_idx];
  src_fri = cfg_src->buffer_alloc;
  dst_fri = cfg_dst->buffer_alloc;

  filter = inter_filter_rs[filter_num];
  filter_radix = filter_num << 7;

  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->pred_mode =
      ((subpel_x != 0) << 2) + ((subpel_y != 0) << 1);

  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->src_num = src_num;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->src_mv = buf_ptr - src_fri;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->psrc = buf_ptr;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->src_stride = buf_stride;

  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->dst_mv = dst - dst_fri;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->dst_stride = dst_buf->stride;

  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->filter_x_mv =
      filter_radix + subpix->filter_x[subpel_x] - filter;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->x_step_q4 = xs;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->filter_y_mv =
      filter_radix + subpix->filter_y[subpel_y] - filter;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->y_step_q4 = ys;

  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->w = 4 << pred_w;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]->h = 4 << pred_h;

  inter_rs_obj.cpu_sec_count[tile_num]++;
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num]++;
}

void build_inter_pred_param_fri_ref_rs(const int plane,
                                        const int block,
                                        BLOCK_SIZE bsize, void *argv,
                                        VP9_COMMON *const cm,
                                        const int ref_idx,
                                        const int src_num,
                                        const int filter_num,
                                        const int tile_num) {

  int i, j;
  int xs, ys;
  int filter_radix;
  int pred_mode;
  int subpel_x, subpel_y;

  const int16_t *filter;
  const uint8_t *src_fri, *dst_fri;
  const uint8_t *dst;

  MV32 scaled_mv;
  MV mv, mv_q4;
  struct scale_factors *sf;
  struct subpix_fn_table *subpix;
  struct buf_2d *pre_buf, *dst_buf;
  const YV12_BUFFER_CONFIG *cfg_src;
  const YV12_BUFFER_CONFIG *cfg_dst;
  int w, h;
  int x0, y0, x0_16, y0_16, x1, y1, frame_width,
      frame_height, buf_stride;
  uint8_t *ref_frame, *buf_ptr;

  const INTER_PRED_ARGS_RS * const arg = argv;
  MACROBLOCKD *const xd = arg->xd;
  const YV12_BUFFER_CONFIG *ref_buf = xd->block_refs[0]->buf;
  int mi_x = arg->x;
  int mi_y = arg->y;
  struct macroblockd_plane *const pd = &xd->plane[plane];
  int changed = 0;
  const BLOCK_SIZE plane_bsize = get_plane_block_size(bsize, pd);

  const int bwl = b_width_log2(plane_bsize);
  const int bw = 4 << bwl;
  const int bh = 4 * num_4x4_blocks_high_lookup[plane_bsize];
  const int x = 4 * (block & ((1 << bwl) - 1));
  const int y = 4 * (block >> bwl);
  const MODE_INFO *mi = xd->mi_8x8[0];

  int pred_w = b_width_log2(bsize) - xd->plane[plane].subsampling_x;
  int pred_h = b_height_log2(bsize) - xd->plane[plane].subsampling_y;

  if (mi->mbmi.sb_type < BLOCK_8X8 && !plane) {
    pred_w = 0;
    pred_h = 0;
  }

  sf = &xd->block_refs[0]->sf;
  pre_buf = &pd->pre[0];
  dst_buf = &pd->dst;

  dst = dst_buf->buf + dst_buf->stride * y + x;

  mv = mi->mbmi.sb_type < BLOCK_8X8
       ? (plane == 0 ? mi->bmi[block].as_mv[0].as_mv
                       : mi_mv_pred_q4(mi, 0))
       : mi->mbmi.mv[0].as_mv;

  mv_q4 = clamp_mv_to_umv_border_sb(xd, &mv, bw, bh,
                                    pd->subsampling_x,
                                    pd->subsampling_y);
  w = 4 << pred_w;
  h = 4 << pred_h;


  // Get reference frame pointer, width and height.
  if (plane == 0) {
    frame_width = ref_buf->y_crop_width;
    frame_height = ref_buf->y_crop_height;
    ref_frame = ref_buf->y_buffer;
  } else {
    frame_width = ref_buf->uv_crop_width;
    frame_height = ref_buf->uv_crop_height;
    ref_frame = plane == 1 ? ref_buf->u_buffer : ref_buf->v_buffer;
  }

  x0 = (-xd->mb_to_left_edge >> (3 + pd->subsampling_x)) + x;
  y0 = (-xd->mb_to_top_edge >> (3 + pd->subsampling_y)) + y;
  x0_16 = x0 << SUBPEL_BITS;
  y0_16 = y0 << SUBPEL_BITS;

  if (vp9_is_scaled(sf)) {
    scaled_mv = vp9_scale_mv(&mv_q4, mi_x + x, mi_y + y, sf);
    xs = sf->x_step_q4;
    ys = sf->y_step_q4;
    x0 = sf->scale_value_x(x0, sf);
    y0 = sf->scale_value_y(y0, sf);
    x0_16 = sf->scale_value_x(x0_16, sf);
    y0_16 = sf->scale_value_y(y0_16, sf);
  } else {
    scaled_mv.row = mv_q4.row;
    scaled_mv.col = mv_q4.col;
    xs = ys = 16;
  }

  subpix = &xd->subpix;
  subpel_x = scaled_mv.col & SUBPEL_MASK;
  subpel_y = scaled_mv.row & SUBPEL_MASK;

  x0 += scaled_mv.col >> SUBPEL_BITS;
  y0 += scaled_mv.row >> SUBPEL_BITS;
  x0_16 += scaled_mv.col;
  y0_16 += scaled_mv.row;

  x1 = ((x0_16 + (w - 1) * xs) >> SUBPEL_BITS) + 1;
  y1 = ((y0_16 + (h - 1) * ys) >> SUBPEL_BITS) + 1;
  buf_ptr = ref_frame + y0 * pre_buf->stride + x0;
  buf_stride = pre_buf->stride;
  if (scaled_mv.col || scaled_mv.row ||
      (frame_width & 0x7) || (frame_height & 0x7)) {
    int x_pad = 0, y_pad = 0;

    if (subpel_x || (sf->x_step_q4 & SUBPEL_MASK)) {
      x0 -= VP9_INTERP_EXTEND - 1;
      x1 += VP9_INTERP_EXTEND;
      x_pad = 1;
    }

    if (subpel_y || (sf->y_step_q4 & SUBPEL_MASK)) {
      y0 -= VP9_INTERP_EXTEND - 1;
      y1 += VP9_INTERP_EXTEND;
      y_pad = 1;
    }

    if (x0 < 0 || x0 > frame_width - 1 || x1 < 0 || x1 > frame_width ||
          y0 < 0 || y0 > frame_height - 1 || y1 < 0 || y1 > frame_height - 1) {
      uint8_t *buf_ptr1 = ref_frame + y0 * pre_buf->stride + x0;
        // Extend the border.
      build_mc_border(buf_ptr1, pre_buf->stride, inter_rs_obj.pref[tile_num], x1 - x0,
                     x0, y0, x1 - x0, y1 - y0, frame_width, frame_height);
      buf_stride = x1 - x0;
      buf_ptr = inter_rs_obj.pref[tile_num] + y_pad * 3 * buf_stride + x_pad * 3;
      inter_rs_obj.pref[tile_num] += (x1 - x0) * (y1 - y0);
      changed = 1;
    }
  }

  cfg_src = &cm->yv12_fb[src_num];
  cfg_dst = &cm->yv12_fb[cm->new_fb_idx];
  src_fri = cfg_src->buffer_alloc;
  dst_fri = cfg_dst->buffer_alloc;

  filter = inter_filter_rs[filter_num];
  filter_radix = filter_num << 7;

  pred_mode = ((subpel_x != 0) << 2) + ((subpel_y != 0) << 1);

  if (pred_mode == 6 && !ref_idx && w > 4 && h > 4 && xs == 16 && ys == 16 &&
      !changed) {
    inter_rs_obj.pred_param_gpu_pre[tile_num]->src_num = src_num;
    inter_rs_obj.pred_param_gpu_pre[tile_num]->src_mv = buf_ptr - src_fri;
    inter_rs_obj.pred_param_gpu_pre[tile_num]->src_stride = pre_buf->stride;

    inter_rs_obj.pred_param_gpu_pre[tile_num]->dst_mv = dst - dst_fri;
    inter_rs_obj.pred_param_gpu_pre[tile_num]->dst_stride = dst_buf->stride;

    inter_rs_obj.pred_param_gpu_pre[tile_num]->filter_x_mv =
        filter_radix + subpix->filter_x[subpel_x] - filter;
    inter_rs_obj.pred_param_gpu_pre[tile_num]->filter_y_mv =
        filter_radix + subpix->filter_y[subpel_y] - filter;

    inter_rs_obj.pred_block_size_gpu_pre[tile_num]->w = w;
    inter_rs_obj.pred_block_size_gpu_pre[tile_num]->h = h;

    for (j = 0; j < h >> 3; ++j) {
      for (i = 0; i < w >> 3; ++i) {
        inter_rs_obj.pred_param_index_gpu_pre[tile_num]->param_num =
          inter_rs_obj.gpu_block_count[tile_num];
        inter_rs_obj.pred_param_index_gpu_pre[tile_num]->x_mv = i;
        inter_rs_obj.pred_param_index_gpu_pre[tile_num]->y_mv = j;

        inter_rs_obj.gpu_index_count[tile_num]++;
        inter_rs_obj.pred_param_index_gpu_pre[tile_num]++;
      }
    }

    inter_rs_obj.gpu_block_count[tile_num]++;
    inter_rs_obj.pred_param_gpu_pre[tile_num]++;
    inter_rs_obj.pred_block_size_gpu_pre[tile_num]++;
  } else {
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->pred_mode = pred_mode;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->psrc = buf_ptr;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->src_stride = buf_stride;

    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->dst_mv = dst - dst_fri;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->dst_stride = dst_buf->stride;

    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->filter_x_mv =
        filter_radix + subpix->filter_x[subpel_x] - filter;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->x_step_q4 = xs;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->filter_y_mv =
        filter_radix + subpix->filter_y[subpel_y] - filter;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->y_step_q4 = ys;

    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->w = w;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]->h = h;

    inter_rs_obj.cpu_fri_count[tile_num]++;
    inter_rs_obj.pred_param_cpu_fri_pre[tile_num]++;
  }
}

static inline void * load_rs_inter_library() {
  void * lib_handle;
  lib_handle = dlopen("vp9_rs_packing.so", RTLD_NOW);
  if (lib_handle == NULL) {
    printf("%s\n", dlerror());
  }
  return lib_handle;
}

int vp9_check_buff_size(VP9_COMMON *const cm, int tile_count) {
  const YV12_BUFFER_CONFIG *cfg_source;
  int param_count_cpu, param_count_gpu;
  cfg_source = &cm->yv12_fb[cm->new_fb_idx];
  param_count_cpu = (cfg_source->buffer_alloc_sz >> 2) / tile_count;
  param_count_gpu = param_count_cpu >> 3;

  inter_rs_obj.tile_count = tile_count;
  inter_rs_obj.globalThreads = param_count_gpu > STABLE_THREAD_COUNT_RS ?
                                param_count_gpu : STABLE_THREAD_COUNT_RS;
  if (cfg_source->buffer_alloc_sz > STABLE_BUFFER_SIZE_RS) {
    inter_rs_obj.buffer_size = cfg_source->buffer_alloc_sz;
    inter_rs_obj.buffer_pool_size =
      inter_rs_obj.buffer_size * FRAME_BUFFERS;

    inter_rs_obj.pred_param_size =
      param_count_gpu * sizeof(INTER_PRED_PARAM_GPU_RS);
    inter_rs_obj.param_index_size =
      param_count_gpu * sizeof(INTER_PARAM_INDEX_GPU_RS);

    vp9_release_rs();
    update_buffer(tile_count, param_count_cpu, param_count_gpu);

  } else {
    inter_rs_obj.buffer_size = cfg_source->buffer_alloc_sz;
    inter_rs_obj.buffer_pool_size =
      inter_rs_obj.buffer_size * FRAME_BUFFERS;
    inter_rs_obj.pred_param_size =
      param_count_gpu * sizeof(INTER_PRED_PARAM_GPU_RS);
    inter_rs_obj.param_index_size =
      param_count_gpu * sizeof(INTER_PARAM_INDEX_GPU_RS);

    inter_rs_context.update_buffer_size_rs(inter_rs_obj.pred_param_size,
                                           inter_rs_obj.param_index_size,
                                           inter_rs_obj.buffer_size,
                                           inter_rs_obj.buffer_pool_size);

  }
  inter_rs_obj.previous_f = cm->new_fb_idx;
  rs_init = 2;
  return 0;
}

static void vp9_init_convolve_t() {
  inter_rs_obj.switch_convolve_t[0] = vp9_convolve_copy;
  inter_rs_obj.switch_convolve_t[1] = vp9_convolve_avg;
  inter_rs_obj.switch_convolve_t[2] = vp9_convolve8_vert;
  inter_rs_obj.switch_convolve_t[3] = vp9_convolve8_avg_vert;
  inter_rs_obj.switch_convolve_t[4] = vp9_convolve8_horiz;
  inter_rs_obj.switch_convolve_t[5] = vp9_convolve8_avg_horiz;
  inter_rs_obj.switch_convolve_t[6] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[7] = vp9_convolve8_avg;

  inter_rs_obj.switch_convolve_t[8] = vp9_convolve8_vert;
  inter_rs_obj.switch_convolve_t[9] = vp9_convolve8_avg_vert;
  inter_rs_obj.switch_convolve_t[10] = vp9_convolve8_vert;
  inter_rs_obj.switch_convolve_t[11] = vp9_convolve8_avg_vert;
  inter_rs_obj.switch_convolve_t[12] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[13] = vp9_convolve8_avg;
  inter_rs_obj.switch_convolve_t[14] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[15] = vp9_convolve8_avg;

  inter_rs_obj.switch_convolve_t[16] = vp9_convolve8_horiz;
  inter_rs_obj.switch_convolve_t[17] = vp9_convolve8_avg_horiz;
  inter_rs_obj.switch_convolve_t[18] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[19] = vp9_convolve8_avg;
  inter_rs_obj.switch_convolve_t[20] = vp9_convolve8_horiz;
  inter_rs_obj.switch_convolve_t[21] = vp9_convolve8_avg_horiz;
  inter_rs_obj.switch_convolve_t[22] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[23] = vp9_convolve8_avg;

  inter_rs_obj.switch_convolve_t[24] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[25] = vp9_convolve8_avg;
  inter_rs_obj.switch_convolve_t[26] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[27] = vp9_convolve8_avg;
  inter_rs_obj.switch_convolve_t[28] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[29] = vp9_convolve8_avg;
  inter_rs_obj.switch_convolve_t[30] = vp9_convolve8;
  inter_rs_obj.switch_convolve_t[31] = vp9_convolve8_avg;
}

int vp9_init_rs() {
  int i;
  int param_count_cpu;
  int param_count_gpu;
  int tile_count = MAX_TILE;
  inter_rs_context.lib_handle = load_rs_inter_library();
  if (inter_rs_context.lib_handle == NULL) {
    return -1;
  }
  inter_rs_context.init_rs = dlsym(inter_rs_context.lib_handle,
                                   "init_rs");
  if (inter_rs_context.init_rs == NULL) {
    printf("get init_rs failed %s\n", dlerror());
  }
  inter_rs_context.update_pool_rs = dlsym(inter_rs_context.lib_handle,
                                          "update_pool_rs");
  if (inter_rs_context.update_pool_rs == NULL) {
    printf("get update_pool_rs %s\n", dlerror());
  }
  inter_rs_context.get_rs_res = dlsym(inter_rs_context.lib_handle,
                                      "get_rs_res");
  if (inter_rs_context.get_rs_res == NULL) {
    printf("get get_rs_res %s\n", dlerror());
  }
  inter_rs_context.invoke_inter_rs = dlsym(inter_rs_context.lib_handle,
                                           "invoke_inter_rs");
  if (inter_rs_context.invoke_inter_rs == NULL) {
    printf("get invoke_inter_rs %s\n", dlerror());
  }
  inter_rs_context.release_rs = dlsym(inter_rs_context.lib_handle,
                                      "release_rs");
  if (inter_rs_context.release_rs == NULL) {
    printf("get release_rs %s\n", dlerror());
  }
  inter_rs_context.release_buffer_rs = dlsym(inter_rs_context.lib_handle,
                                      "release_buffer_rs");
  if (inter_rs_context.release_buffer_rs == NULL) {
    printf("get release_buffer_rs %s\n", dlerror());
  }
  inter_rs_context.create_buffer_rs = dlsym(inter_rs_context.lib_handle,
                                      "create_buffer_rs");
  if (inter_rs_context.create_buffer_rs == NULL) {
    printf("get create_buffer_rs %s\n", dlerror());
  }
  inter_rs_context.update_buffer_size_rs = dlsym(inter_rs_context.lib_handle,
                                      "update_buffer_size_rs");
  if (inter_rs_context.update_buffer_size_rs == NULL) {
    printf("get update_buffer_size_rs %s\n", dlerror());
  }

  vp9_init_convolve_t();

  param_count_cpu = (STABLE_BUFFER_SIZE_RS >> 2) / tile_count;
  param_count_gpu = param_count_cpu >> 3;

  inter_rs_obj.tile_count = tile_count;
  inter_rs_obj.globalThreads = param_count_gpu > STABLE_THREAD_COUNT_RS ?
                                param_count_gpu : STABLE_THREAD_COUNT_RS;

  inter_rs_obj.buffer_size = STABLE_BUFFER_SIZE_RS;
  inter_rs_obj.buffer_pool_size =
      inter_rs_obj.buffer_size * FRAME_BUFFERS;

  inter_rs_obj.pred_param_size =
      param_count_gpu * sizeof(INTER_PRED_PARAM_GPU_RS);
  inter_rs_obj.param_index_size =
      param_count_gpu * sizeof(INTER_PARAM_INDEX_GPU_RS);

  for (i = 0; i < tile_count; ++i) {
    inter_rs_obj.pred_param_cpu_fri[i] =
        MALLOC_INTER_RS(INTER_PRED_PARAM_CPU_RS, param_count_cpu);
    inter_rs_obj.pred_param_cpu_sec[i] =
        inter_rs_obj.pred_param_cpu_fri[i] + (param_count_cpu >> 1);
    inter_rs_obj.pred_param_gpu[i] =
        MALLOC_INTER_RS(INTER_PRED_PARAM_GPU_RS, param_count_gpu);
    inter_rs_obj.pred_param_index_gpu[i] =
        MALLOC_INTER_RS(INTER_PARAM_INDEX_GPU_RS, param_count_gpu);
    inter_rs_obj.pred_block_size_gpu[i] =
        MALLOC_INTER_RS(INTER_BLOCK_SIZE_GPU_RS, param_count_gpu);

    inter_rs_obj.ref_buffer[i] = (uint8_t *)malloc(inter_rs_obj.buffer_size);
    inter_rs_obj.pref[i] = inter_rs_obj.ref_buffer[i];

    inter_rs_obj.new_buffer[i] = (uint8_t *)malloc(inter_rs_obj.buffer_size);
    inter_rs_obj.mid_buffer[i] = (uint8_t *)malloc(inter_rs_obj.buffer_size);
    assert(inter_rs_obj.pred_param_gpu[i] != NULL);
    assert(inter_rs_obj.pred_param_cpu_fri[i] != NULL);
    assert(inter_rs_obj.pred_block_size_gpu[i] != NULL);
    assert(inter_rs_obj.pred_param_index_gpu[i] != NULL);

    inter_rs_context.init_rs(inter_rs_obj.pred_param_size,
            inter_rs_obj.param_index_size,
            inter_rs_obj.buffer_size, inter_rs_obj.buffer_pool_size,
            inter_rs_obj.globalThreads, i,
            (uint8_t *)inter_rs_obj.pred_param_gpu[i],
            (uint8_t *)inter_rs_obj.pred_param_index_gpu[i],
            (uint8_t *)inter_rs_obj.new_buffer[i],
            (uint8_t *)inter_rs_obj.mid_buffer[i]);

    inter_rs_obj.pred_param_cpu_fri_pre[i] =
        inter_rs_obj.pred_param_cpu_fri[i];
    inter_rs_obj.pred_param_cpu_sec_pre[i] =
        inter_rs_obj.pred_param_cpu_sec[i];
    inter_rs_obj.pred_param_gpu_pre[i] =
        inter_rs_obj.pred_param_gpu[i];
    inter_rs_obj.pred_param_index_gpu_pre[i] =
        inter_rs_obj.pred_param_index_gpu[i];
    inter_rs_obj.pred_block_size_gpu_pre[i] =
        inter_rs_obj.pred_block_size_gpu[i];
    inter_rs_obj.cpu_fri_count[i] = 0;
    inter_rs_obj.cpu_sec_count[i] = 0;
    inter_rs_obj.gpu_block_count[i] = 0;
    inter_rs_obj.gpu_index_count[i] = 0;
  }

  return 0;
}

int vp9_release_rs() {
  int i;
  for (i = 0; i < inter_rs_obj.tile_count; ++i) {
    if (inter_rs_obj.new_buffer[i] != NULL) {
      free(inter_rs_obj.new_buffer[i]);
      inter_rs_obj.new_buffer[i] = NULL;
    }
    if (inter_rs_obj.pred_param_cpu_fri[i] != NULL) {
      free(inter_rs_obj.pred_param_cpu_fri[i]);
      inter_rs_obj.pred_param_cpu_fri[i] = NULL;
      inter_rs_obj.pred_param_cpu_fri_pre[i] = NULL;
      inter_rs_obj.pred_param_cpu_sec[i] = NULL;
      inter_rs_obj.pred_param_cpu_sec_pre[i] = NULL;
    }
    if (inter_rs_obj.pred_param_gpu[i] != NULL) {
      free(inter_rs_obj.pred_param_gpu[i]);
      inter_rs_obj.pred_param_gpu[i] = NULL;
      inter_rs_obj.pred_param_gpu_pre[i] = NULL;
    }
    if (inter_rs_obj.pred_param_index_gpu[i] != NULL) {
      free(inter_rs_obj.pred_param_index_gpu[i]);
      inter_rs_obj.pred_param_index_gpu[i] = NULL;
      inter_rs_obj.pred_param_index_gpu_pre[i] = NULL;
    }
    if (inter_rs_obj.pred_block_size_gpu[i]) {
      free(inter_rs_obj.pred_block_size_gpu[i]);
      inter_rs_obj.pred_block_size_gpu[i] = NULL;
      inter_rs_obj.pred_block_size_gpu_pre[i] = NULL;
    }
    free(inter_rs_obj.ref_buffer[i]);
    free(inter_rs_obj.mid_buffer[i]);
  }
  inter_rs_context.release_rs(inter_rs_obj.tile_count);
  return 0;
}

int vp9_mem_cpu_to_gpu_rs(VP9_COMMON *const cm) {
  const YV12_BUFFER_CONFIG *cfg_source =
            cfg_source = &cm->yv12_fb[inter_rs_obj.previous_f];
  inter_rs_context.update_pool_rs(cfg_source->buffer_alloc,
                                  inter_rs_obj.previous_f);
  inter_rs_obj.previous_f = cm->new_fb_idx;
  return 0;
}

int inter_pred_calcu_rs(VP9_COMMON *const cm, const int tile_num) {
  int i, y;
  INTER_PRED_PARAM_GPU_RS *pred_param;
  INTER_BLOCK_SIZE_GPU_RS *pred_block_size;
  const YV12_BUFFER_CONFIG *cfg_source = &cm->yv12_fb[cm->new_fb_idx];

  if (inter_rs_obj.gpu_index_count[tile_num] > 0) {
    inter_rs_context.invoke_inter_rs(inter_rs_obj.new_buffer[tile_num],
                   (uint8_t *)inter_rs_obj.pred_param_gpu[tile_num],
                   (uint8_t *)inter_rs_obj.pred_param_index_gpu[tile_num],
                   tile_num, inter_rs_obj.gpu_index_count[tile_num],
                   inter_rs_obj.gpu_block_count[tile_num]
                   * sizeof(INTER_PRED_PARAM_GPU_RS),
                   inter_rs_obj.gpu_index_count[tile_num]
                   * sizeof(INTER_PARAM_INDEX_GPU_RS)
                   );
    // Executive inter prediction CPU part
    build_inter_pred_calcu_rs_c(cm, cfg_source->buffer_alloc,
                                 inter_rs_obj.cpu_fri_count[tile_num],
                                 inter_rs_obj.cpu_sec_count[tile_num],
                                 inter_rs_obj.pred_param_cpu_fri[tile_num],
                                 inter_rs_obj.pred_param_cpu_sec[tile_num],
                                 inter_rs_obj.switch_convolve_t);
    inter_rs_context.get_rs_res(inter_rs_obj.new_buffer[tile_num], tile_num);

    pred_param = inter_rs_obj.pred_param_gpu[tile_num];
    pred_block_size = inter_rs_obj.pred_block_size_gpu[tile_num];

    for (i = 0; i < inter_rs_obj.gpu_block_count[tile_num]; ++i) {
      const uint8_t *src = inter_rs_obj.new_buffer[tile_num] + pred_param[i].dst_mv;
      uint8_t *dst = cfg_source->buffer_alloc + pred_param[i].dst_mv;
      for (y = 0; y < pred_block_size[i].h; ++y) {
        memcpy(dst, src, sizeof(uint8_t) * pred_block_size[i].w);
        src += pred_param[i].dst_stride;
        dst += pred_param[i].dst_stride;
      }
    }
  } else {
    build_inter_pred_calcu_rs_c(cm, cfg_source->buffer_alloc,
                                 inter_rs_obj.cpu_fri_count[tile_num],
                                 inter_rs_obj.cpu_sec_count[tile_num],
                                 inter_rs_obj.pred_param_cpu_fri[tile_num],
                                 inter_rs_obj.pred_param_cpu_sec[tile_num],
                                 inter_rs_obj.switch_convolve_t);
  }

  inter_rs_obj.pred_param_cpu_fri_pre[tile_num] =
      inter_rs_obj.pred_param_cpu_fri[tile_num];
  inter_rs_obj.pred_param_cpu_sec_pre[tile_num] =
      inter_rs_obj.pred_param_cpu_sec[tile_num];
  inter_rs_obj.pred_param_gpu_pre[tile_num] =
      inter_rs_obj.pred_param_gpu[tile_num];
  inter_rs_obj.pred_param_index_gpu_pre[tile_num] =
      inter_rs_obj.pred_param_index_gpu[tile_num];
  inter_rs_obj.pred_block_size_gpu_pre[tile_num] =
      inter_rs_obj.pred_block_size_gpu[tile_num];
  inter_rs_obj.pref[tile_num] = inter_rs_obj.ref_buffer[tile_num];
  inter_rs_obj.cpu_fri_count[tile_num] = 0;
  inter_rs_obj.cpu_sec_count[tile_num] = 0;
  inter_rs_obj.gpu_block_count[tile_num] = 0;
  inter_rs_obj.gpu_index_count[tile_num] = 0;

  return 0;
}
