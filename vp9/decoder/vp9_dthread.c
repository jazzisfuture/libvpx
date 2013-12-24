/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "vp9/common/vp9_reconinter.h"
#include "vp9/decoder/vp9_dthread.h"
#include "vp9/decoder/vp9_onyxd_int.h"
#include "vpx_mem/vpx_mem.h"

#if CONFIG_MULTITHREAD
static INLINE void mutex_lock(pthread_mutex_t *const mutex) {
  const int kMaxTryLocks = 4000;
  int locked = 0;
  int i;

  for (i = 0; i < kMaxTryLocks; ++i) {
    if (!pthread_mutex_trylock(mutex)) {
      locked = 1;
      break;
    }
  }

  if (!locked)
    pthread_mutex_lock(mutex);
}
#endif  // CONFIG_MULTITHREAD

static INLINE void sync_read(VP9LfSync *const lf_sync, int r, int c) {
#if CONFIG_MULTITHREAD
  const int nsync = lf_sync->sync_range;

  if (r && !(c & (nsync - 1))) {
    mutex_lock(&lf_sync->mutex_[r - 1]);

    while (c > lf_sync->cur_sb_col[r - 1] - nsync) {
      pthread_cond_wait(&lf_sync->cond_[r - 1],
                        &lf_sync->mutex_[r - 1]);
    }
    pthread_mutex_unlock(&lf_sync->mutex_[r - 1]);
  }
#else
  (void)lf_sync;
  (void)r;
  (void)c;
#endif  // CONFIG_MULTITHREAD
}

static INLINE void sync_write(VP9LfSync *const lf_sync, int r, int c,
                              const int sb_cols) {
#if CONFIG_MULTITHREAD
  const int nsync = lf_sync->sync_range;
  int cur;
  // Only signal when there are enough filtered SB for next row to run.
  int sig = 1;

  if (c < sb_cols - 1) {
    cur = c;
    if (c % nsync)
      sig = 0;
  } else {
    cur = sb_cols + nsync;
  }

  if (sig) {
    mutex_lock(&lf_sync->mutex_[r]);

    lf_sync->cur_sb_col[r] = cur;

    pthread_cond_signal(&lf_sync->cond_[r]);
    pthread_mutex_unlock(&lf_sync->mutex_[r]);
  }
#else
  (void)lf_sync;
  (void)r;
  (void)c;
  (void)sb_cols;
#endif  // CONFIG_MULTITHREAD
}

// Implement row loopfiltering for each thread.
static void loop_filter_rows_mt(const YV12_BUFFER_CONFIG *const frame_buffer,
                                VP9_COMMON *const cm, MACROBLOCKD *const xd,
                                int start, int stop, int y_only,
                                VP9LfSync *const lf_sync, int num_lf_workers) {
  const int num_planes = y_only ? 1 : MAX_MB_PLANE;
  int r, c;  // SB row and col
  LOOP_FILTER_MASK lfm;
  const int sb_cols = mi_cols_aligned_to_sb(cm->mi_cols) >> MI_BLOCK_SIZE_LOG2;

  for (r = start; r < stop; r += num_lf_workers) {
    const int mi_row = r << MI_BLOCK_SIZE_LOG2;
    MODE_INFO **mi_8x8 = cm->mi_grid_visible + mi_row * cm->mode_info_stride;

    for (c = 0; c < sb_cols; ++c) {
      const int mi_col = c << MI_BLOCK_SIZE_LOG2;
      int plane;

      sync_read(lf_sync, r, c);

      setup_dst_planes(xd, frame_buffer, mi_row, mi_col);
      vp9_setup_mask(cm, mi_row, mi_col, mi_8x8 + mi_col, cm->mode_info_stride,
                     &lfm);

      for (plane = 0; plane < num_planes; ++plane) {
        vp9_filter_block_plane(cm, &xd->plane[plane], mi_row, &lfm);
      }

      sync_write(lf_sync, r, c, sb_cols);
    }
  }
}

// Row-based multi-threaded loopfilter hook
static int loop_filter_row_worker(void *arg1, void *arg2) {
  TileWorkerData *const tile_data = (TileWorkerData*)arg1;
  LFWorkerData *const lf_data = &tile_data->lfdata;
  const int y_only = (lf_data->filter_planes == 1);

  loop_filter_rows_mt(lf_data->frame_buffer, lf_data->cm, &lf_data->xd,
                      lf_data->start, lf_data->stop, y_only,
                      lf_data->lf_sync, lf_data->num_lf_workers);
  return 1;
}

// VP9 decoder: Implement multi-threaded loopfilter that uses the tile
// threads.
void vp9_loop_filter_frame_mt(VP9D_COMP *pbi,
                              VP9_COMMON *cm,
                              MACROBLOCKD *xd,
                              int frame_filter_level,
                              int y_only, int partial_frame) {
  // Number of superblock rows and cols
  const int sb_rows = mi_cols_aligned_to_sb(cm->mi_rows) >> MI_BLOCK_SIZE_LOG2;
  const uint32_t filter_planes = y_only ? 1 : ~0;
  int i;

  // Allocate memory used in thread synchronization.
  // This always needs to be done even if frame_filter_level is 0.
  if (!cm->current_video_frame || cm->last_height != cm->height) {
    VP9LfSync *const lf_sync = &pbi->lf_row_sync;

    if (cm->last_height != cm->height) {
      const int aligned_last_height =
          ALIGN_POWER_OF_TWO(cm->last_height, MI_SIZE_LOG2);
      const int last_sb_rows =
          mi_cols_aligned_to_sb(aligned_last_height >> MI_SIZE_LOG2) >>
          MI_BLOCK_SIZE_LOG2;

      vp9_loop_filter_dealloc(lf_sync, last_sb_rows);
    }

    vp9_loop_filter_alloc(cm, lf_sync, sb_rows, cm->width);
  }

  if (!frame_filter_level) return;

  vp9_loop_filter_frame_init(cm, frame_filter_level);

  // Initialize cur_sb_col to -1 for all SB rows.
  vpx_memset(pbi->lf_row_sync.cur_sb_col, -1,
             sizeof(*pbi->lf_row_sync.cur_sb_col) * sb_rows);

  // Set up loopfilter thread data.
  for (i = 0; i < pbi->num_tile_workers; ++i) {
    VP9Worker *const worker = &pbi->tile_workers[i];
    TileWorkerData *const tile_data = (TileWorkerData*)worker->data1;
    LFWorkerData *const lf_data = &tile_data->lfdata;

    worker->hook = (VP9WorkerHook)loop_filter_row_worker;

    // Loopfilter data
    lf_data->frame_buffer = get_frame_new_buffer(cm);
    lf_data->cm = cm;
    lf_data->xd = pbi->mb;
    lf_data->start = i;
    lf_data->stop = sb_rows;
    lf_data->filter_planes = filter_planes;

    lf_data->lf_sync = &pbi->lf_row_sync;
    lf_data->num_lf_workers = pbi->num_tile_workers;

    // Start loopfiltering
    if (i == pbi->num_tile_workers - 1) {
      vp9_worker_execute(worker);
    } else {
      vp9_worker_launch(worker);
    }
  }

  // Wait till all rows are finished
  for (i = 0; i < pbi->num_tile_workers; ++i) {
    vp9_worker_sync(&pbi->tile_workers[i]);
  }
}

// Set up nsync by width.
static int get_sync_range(int width) {
  // nsync numbers are picked by testing. For example, for 4k
  // video, using 4 gives best performance.
  if (width < 640)
    return 1;
  else if (width <= 1280)
    return 2;
  else if (width <= 4096)
    return 4;
  else
    return 8;
}

// Allocate memory for lf row synchronization
void vp9_loop_filter_alloc(VP9_COMMON *cm, VP9LfSync *lf_sync, int rows,
                           int width) {
#if CONFIG_MULTITHREAD
  int i;

  CHECK_MEM_ERROR(cm, lf_sync->mutex_,
                  vpx_malloc(sizeof(*lf_sync->mutex_) * rows));
  for (i = 0; i < rows; ++i) {
    pthread_mutex_init(&lf_sync->mutex_[i], NULL);
  }

  CHECK_MEM_ERROR(cm, lf_sync->cond_,
                  vpx_malloc(sizeof(*lf_sync->cond_) * rows));
  for (i = 0; i < rows; ++i) {
    pthread_cond_init(&lf_sync->cond_[i], NULL);
  }
#endif  // CONFIG_MULTITHREAD

  CHECK_MEM_ERROR(cm, lf_sync->cur_sb_col,
                  vpx_malloc(sizeof(*lf_sync->cur_sb_col) * rows));

  // Set up nsync.
  lf_sync->sync_range = get_sync_range(width);
}

// Deallocate lf synchronization related mutex and data
void vp9_loop_filter_dealloc(VP9LfSync *lf_sync, int rows) {
#if CONFIG_MULTITHREAD
  if (lf_sync != NULL) {
    int i;

    if (lf_sync->mutex_ != NULL) {
      for (i = 0; i < rows; ++i) {
        pthread_mutex_destroy(&lf_sync->mutex_[i]);
      }
      vpx_free(lf_sync->mutex_);
    }
    if (lf_sync->cond_ != NULL) {
      for (i = 0; i < rows; ++i) {
        pthread_cond_destroy(&lf_sync->cond_[i]);
      }
      vpx_free(lf_sync->cond_);
    }

    vpx_free(lf_sync->cur_sb_col);
    // clear the structure as the source of this call may be a resize in which
    // case this call will be followed by an _alloc() which may fail.
    vpx_memset(lf_sync, 0, sizeof(*lf_sync));
  }
#else
  (void)rows;
  if (lf_sync != NULL) {
    vpx_free(lf_sync->cur_sb_col);
    vpx_memset(lf_sync, 0, sizeof(*lf_sync));
  }
#endif  // CONFIG_MULTITHREAD
}
