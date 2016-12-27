/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_THREAD_COMMON_H_
#define VP9_COMMON_VP9_THREAD_COMMON_H_
#include "./vpx_config.h"
#include "vp9/common/vp9_loopfilter.h"
#include "vpx_util/vpx_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct VP9Common;
struct FRAME_COUNTS;

// Loopfilter row synchronization
typedef struct VP9LfSyncData {
#if CONFIG_MULTITHREAD
  pthread_mutex_t *mutex_;
  pthread_cond_t *cond_;
#endif
  // Allocate memory to store the loop-filtered superblock index in each row.
  int *cur_sb_col;
  // The optimal sync_range for different resolution and platform should be
  // determined by testing. Currently, it is chosen to be a power-of-2 number.
  int sync_range;
  int rows;

  // Row-based parallel loopfilter data
  LFWorkerData *lfdata;
  int num_workers;
} VP9LfSync;

// Encoder row synchronization
typedef struct VP9RowMTSyncData {
#if CONFIG_MULTITHREAD
  pthread_mutex_t *mutex_;
  pthread_cond_t *cond_;
#endif
  // Allocate memory to store the sb/mb block index in each row.
  int *cur_col;
  int sync_range;
  int rows;
  int num_workers;
} VP9RowMTSync;

typedef struct VP9TopRowSyncData {
#if CONFIG_MULTITHREAD
  pthread_mutex_t *mutex_;
  pthread_cond_t *cond_;
#endif
  // Allocate memory to store the completed row in the
  // first pass stats accumulation
  int *cur_mb_row;
  int rows;
} VP9TopRowSync;

// Allocate memory for loopfilter row synchronization.
void vp9_loop_filter_alloc(VP9LfSync *lf_sync, struct VP9Common *cm, int rows,
                           int width, int num_workers);

// Deallocate loopfilter synchronization related mutex and data.
void vp9_loop_filter_dealloc(VP9LfSync *lf_sync);

// Multi-threaded loopfilter that uses the tile threads.
void vp9_loop_filter_frame_mt(YV12_BUFFER_CONFIG *frame,
                              struct VP9Common *cm,
                              struct macroblockd_plane planes[MAX_MB_PLANE],
                              int frame_filter_level,
                              int y_only, int partial_frame,
                              VPxWorker *workers, int num_workers,
                              VP9LfSync *lf_sync);

void vp9_accumulate_frame_counts(struct FRAME_COUNTS *accum,
                                 const struct FRAME_COUNTS *counts, int is_dec);

// Allocate memory for row based multi-threading synchronization.
void vp9_row_mt_sync_mem_alloc(VP9RowMTSync *row_mt_sync, struct VP9Common *cm,
                               int rows, int num_workers);

// Allocate memory for top row synchronization in first pass stats accumulation.
// This is done to maintain the order of addition of floating point variables,
// thereby generating identical first pass stats file in multithreaded case.
void vp9_top_row_sync_mem_alloc(VP9TopRowSync *top_row_sync,
                                struct VP9Common *cm, int rows);

// Deallocate row based multi-threading synchronization related mutex and data.
void vp9_row_mt_sync_mem_dealloc(VP9RowMTSync *row_mt_sync);

// Deallocate top row synchronization related mutex and data.
void vp9_top_row_sync_mem_dealloc(VP9TopRowSync *top_row_sync);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_THREAD_COMMON_H_
