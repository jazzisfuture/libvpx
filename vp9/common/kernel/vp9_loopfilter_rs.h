/*
 * Copyright (C) 2013 MultiCoreWare, Inc. All rights reserved.
 * Guanlin Liang <guanlin@multicorewareinc.com>
 */

#ifndef VP9_LOOP_FILTER_RS_H_
#define VP9_LOOP_FILTER_RS_H_

#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_loopfilter.h"

struct LOOP_FILTER_MASK;

int vp9_loop_filter_rs_init(void **handle);
void vp9_loop_filter_rs_fini(void *handle);

typedef int (*vp9_loop_filter_rs_create_buffer_fun)(void *handle_,
                                                    int idx,
                                                    int size,
                                                    uint8_t *pointer);
extern vp9_loop_filter_rs_create_buffer_fun vp9_loop_filter_rs_create_buffer;

typedef void (*vp9_loop_filter_rows_work_rs_fun)(void *handle_,
                                                 int start, int stop,
                                                 int num_planes,
                                                 int mi_rows,
                                                 int mi_cols,
                                                 int frame_buffer_idx,
                                                 int y_offset,
                                                 int u_offset,
                                                 int v_offset,
                                                 int y_stride,
                                                 int uv_stride,
                                                 loop_filter_info_n *lf_info,
                                                 LOOP_FILTER_MASK *lfms);
extern vp9_loop_filter_rows_work_rs_fun vp9_loop_filter_rows_work_rs;

#endif // VP9_LOOP_FILTER_RS_H_
