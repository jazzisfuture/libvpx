/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_DECODER_VP9_DTHREAD_H_
#define VP9_DECODER_VP9_DTHREAD_H_

#include "vp9/common/vp9_loopfilter.h"
#include "vp9/decoder/vp9_dboolhuff.h"

typedef struct TileWorkerData {
  struct VP9Common *cm;
  vp9_reader bit_reader;
  DECLARE_ALIGNED(16, struct macroblockd, xd);
  DECLARE_ALIGNED(16, int16_t, dqcoeff[MAX_MB_PLANE][64 * 64]);

  // Row-based parallel loopfilter data
  LFWorkerData lfdata;
} TileWorkerData;

// Loopfilter row synchronization
typedef struct VP9LfSyncData {
#if CONFIG_MULTITHREAD
  pthread_mutex_t *mutex_;
  pthread_cond_t  *cond_;
#endif
  // Allocate memory to store the loop-filtered superblock index in each row.
  int *cur_sb_col;
  int sync_range;
} VP9LfSync;

// Allocate memory for lf row synchronization.
int vp9_lf_start(VP9LfSync *lf_sync, int rows, int width);
// Deallocate lf synchronization related mutex and data.
void vp9_lf_end(VP9LfSync *lf_sync, int rows);

// Row-based multi-threaded loopfilter hook
int vp9_loop_filter_row_worker(void *arg1, void *arg2);

// VP9 decoder: Implement multi-threaded loopfilter that uses the tile
// threads.
void vp9_loop_filter_frame_mt(struct VP9Decompressor *pbi,
                              struct VP9Common *cm,
                              struct macroblockd *xd,
                              int frame_filter_level,
                              int y_only, int partial);

#endif  // VP9_DECODER_VP9_DTHREAD_H_
