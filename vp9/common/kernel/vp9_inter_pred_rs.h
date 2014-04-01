/*
 * Copyright (C) 2013 MultiCoreWare, Inc. All rights reserved.
 * XinSu <xin@multicorewareinc.com>
 */

#ifndef VP9_INTER_PRED_RS_H_
#define VP9_INTER_PRED_RS_H_

#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_onyxc_int.h"

#define MAX_TILE_COUNT_RS 4

#define MALLOC_INTER_RS(type, n) (type *)malloc((n)*sizeof(type))

typedef struct inter_pred_param_gpu_rs {
  int src_num;
  int src_mv;
  int src_stride;

  int dst_mv;
  int dst_stride;

  int filter_x_mv;
  int filter_y_mv;
}INTER_PRED_PARAM_GPU_RS;

typedef struct inter_pred_param_cpu_rs {
  int pred_mode;

  int src_num;
  int src_mv;
  int src_stride;
  uint8_t *psrc;
  int dst_mv;
  int dst_stride;

  int filter_x_mv;
  int x_step_q4;
  int filter_y_mv;
  int y_step_q4;

  int w;
  int h;
}INTER_PRED_PARAM_CPU_RS;

typedef struct inter_block_size_gpu_rs {
  int w;
  int h;
}INTER_BLOCK_SIZE_GPU_RS;

typedef struct inter_param_index_rs {
  int param_num;
  int x_mv;
  int y_mv;
}INTER_PARAM_INDEX_GPU_RS;

typedef struct inter_pred_args_rs {
  MACROBLOCKD *xd;
  int x, y;
}INTER_PRED_ARGS_RS;

typedef struct inter_rs_obj {
  int previous_f;
  int tile_count;
  int buffer_size;
  int pred_param_size;
  int param_index_size;
  int buffer_pool_size;

  int globalThreads;

  convolve_fn_t switch_convolve_t[32];

  int gpu_block_count[MAX_TILE_COUNT_RS];
  int gpu_index_count[MAX_TILE_COUNT_RS];

  int cpu_fri_count[MAX_TILE_COUNT_RS];
  int cpu_sec_count[MAX_TILE_COUNT_RS];

  INTER_PRED_PARAM_GPU_RS *pred_param_gpu[MAX_TILE_COUNT_RS];
  INTER_PRED_PARAM_GPU_RS *pred_param_gpu_pre[MAX_TILE_COUNT_RS];

  INTER_BLOCK_SIZE_GPU_RS *pred_block_size_gpu[MAX_TILE_COUNT_RS];
  INTER_BLOCK_SIZE_GPU_RS *pred_block_size_gpu_pre[MAX_TILE_COUNT_RS];

  INTER_PARAM_INDEX_GPU_RS *pred_param_index_gpu[MAX_TILE_COUNT_RS];
  INTER_PARAM_INDEX_GPU_RS *pred_param_index_gpu_pre[MAX_TILE_COUNT_RS];

  INTER_PRED_PARAM_CPU_RS *pred_param_cpu_fri[MAX_TILE_COUNT_RS];
  INTER_PRED_PARAM_CPU_RS *pred_param_cpu_sec[MAX_TILE_COUNT_RS];
  INTER_PRED_PARAM_CPU_RS *pred_param_cpu_fri_pre[MAX_TILE_COUNT_RS];
  INTER_PRED_PARAM_CPU_RS *pred_param_cpu_sec_pre[MAX_TILE_COUNT_RS];
  uint8_t *ref_buffer[MAX_TILE_COUNT_RS];
  uint8_t *pref[MAX_TILE_COUNT_RS];
  uint8_t *new_buffer[MAX_TILE_COUNT_RS];
  uint8_t *mid_buffer[MAX_TILE_COUNT_RS];
}INTER_RS_OBJ;

int vp9_setup_interp_filters_rs(MACROBLOCKD *xd,
                                 INTERPOLATION_TYPE mcomp_filter_type,
                                 VP9_COMMON *cm);

void build_inter_pred_param_sec_ref_rs(const int plane,
                                        const int block,
                                        BLOCK_SIZE bsize, void *argv,
                                        VP9_COMMON *const cm,
                                        const int src_num,
                                        const int filter_num,
                                        const int tile_num);

void build_inter_pred_param_fri_ref_rs(const int plane,
                                        const int block,
                                        BLOCK_SIZE bsize, void *argv,
                                        VP9_COMMON *const cm,
                                        const int ref_idx,
                                        const int src_num,
                                        const int filter_num,
                                        const int tile_num);

int vp9_init_rs();
int vp9_check_buff_size(VP9_COMMON *const cm, int tile_count);
int vp9_release_rs();

int vp9_mem_cpu_to_gpu_rs(VP9_COMMON *const cm);

int inter_pred_calcu_rs(VP9_COMMON *const cm, const int tile_num);

extern INTER_RS_OBJ inter_rs_obj;
extern int rs_init;

#endif // VP9_INTER_PRED_RS_H_
