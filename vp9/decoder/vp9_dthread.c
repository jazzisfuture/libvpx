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

#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_reconinter.h"

#include "vp9/decoder/vp9_dthread.h"
#include "vp9/decoder/vp9_decoder.h"

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
    pthread_mutex_t *const mutex = &lf_sync->mutex_[r - 1];
    mutex_lock(mutex);

    while (c > lf_sync->cur_sb_col[r - 1] - nsync) {
      pthread_cond_wait(&lf_sync->cond_[r - 1], mutex);
    }
    pthread_mutex_unlock(mutex);
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
                                VP9_COMMON *const cm,
                                struct macroblockd_plane planes[MAX_MB_PLANE],
                                int start, int stop, int y_only,
                                VP9LfSync *const lf_sync, int num_lf_workers) {
  const int num_planes = y_only ? 1 : MAX_MB_PLANE;
  int r, c;  // SB row and col
  const int sb_cols = mi_cols_aligned_to_sb(cm->mi_cols) >> MI_BLOCK_SIZE_LOG2;

  for (r = start; r < stop; r += num_lf_workers) {
    const int mi_row = r << MI_BLOCK_SIZE_LOG2;
    MODE_INFO **const mi = cm->mi_grid_visible + mi_row * cm->mi_stride;

    for (c = 0; c < sb_cols; ++c) {
      const int mi_col = c << MI_BLOCK_SIZE_LOG2;
      LOOP_FILTER_MASK lfm;
      int plane;

      sync_read(lf_sync, r, c);

      vp9_setup_dst_planes(planes, frame_buffer, mi_row, mi_col);
      vp9_setup_mask(cm, mi_row, mi_col, mi + mi_col, cm->mi_stride, &lfm);

      for (plane = 0; plane < num_planes; ++plane) {
        vp9_filter_block_plane(cm, &planes[plane], mi_row, &lfm);
      }

      sync_write(lf_sync, r, c, sb_cols);
    }
  }
}

// Row-based multi-threaded loopfilter hook
static int loop_filter_row_worker(void *arg1, void *arg2) {
  TileWorkerData *const tile_data = (TileWorkerData*)arg1;
  LFWorkerData *const lf_data = &tile_data->lfdata;
  (void) arg2;
  loop_filter_rows_mt(lf_data->frame_buffer, lf_data->cm, lf_data->planes,
                      lf_data->start, lf_data->stop, lf_data->y_only,
                      lf_data->lf_sync, lf_data->num_lf_workers);
  return 1;
}

// VP9 decoder: Implement multi-threaded loopfilter that uses the tile
// threads.
void vp9_loop_filter_frame_mt(YV12_BUFFER_CONFIG *frame,
                              VP9Decoder *pbi, VP9_COMMON *cm,
                              int frame_filter_level,
                              int y_only) {
  VP9LfSync *const lf_sync = &pbi->lf_row_sync;
  const VP9WorkerInterface *const winterface = vp9_get_worker_interface();
  // Number of superblock rows and cols
  const int sb_rows = mi_cols_aligned_to_sb(cm->mi_rows) >> MI_BLOCK_SIZE_LOG2;
  const int tile_cols = 1 << cm->log2_tile_cols;
  const int num_workers = MIN(pbi->max_threads & ~1, tile_cols);
  int i;

  // Allocate memory used in thread synchronization.
  // This always needs to be done even if frame_filter_level is 0.
  if (!cm->current_video_frame || cm->last_height != cm->height) {
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
  vpx_memset(lf_sync->cur_sb_col, -1, sizeof(*lf_sync->cur_sb_col) * sb_rows);

  // Set up loopfilter thread data.
  // The decoder is using num_workers instead of pbi->num_tile_workers
  // because it has been observed that using more threads on the
  // loopfilter, than there are tile columns in the frame will hurt
  // performance on Android. This is because the system will only
  // schedule the tile decode workers on cores equal to the number
  // of tile columns. Then if the decoder tries to use more threads for the
  // loopfilter, it will hurt performance because of contention. If the
  // multithreading code changes in the future then the number of workers
  // used by the loopfilter should be revisited.
  for (i = 0; i < num_workers; ++i) {
    VP9Worker *const worker = &pbi->tile_workers[i];
    TileWorkerData *const tile_data = (TileWorkerData*)worker->data1;
    LFWorkerData *const lf_data = &tile_data->lfdata;

    worker->hook = (VP9WorkerHook)loop_filter_row_worker;

    // Loopfilter data
    lf_data->frame_buffer = frame;
    lf_data->cm = cm;
    vp9_copy(lf_data->planes, pbi->mb.plane);
    lf_data->start = i;
    lf_data->stop = sb_rows;
    lf_data->y_only = y_only;   // always do all planes in decoder

    lf_data->lf_sync = lf_sync;
    lf_data->num_lf_workers = num_workers;

    // Start loopfiltering
    if (i == num_workers - 1) {
      winterface->execute(worker);
    } else {
      winterface->launch(worker);
    }
  }

  // Wait till all rows are finished
  for (i = 0; i < num_workers; ++i) {
    winterface->sync(&pbi->tile_workers[i]);
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
#if !CONFIG_MULTITHREAD
  (void)rows;
#endif  // !CONFIG_MULTITHREAD

  if (lf_sync != NULL) {
#if CONFIG_MULTITHREAD
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
#endif  // CONFIG_MULTITHREAD
    vpx_free(lf_sync->cur_sb_col);
    // clear the structure as the source of this call may be a resize in which
    // case this call will be followed by an _alloc() which may fail.
    vp9_zero(*lf_sync);
  }
}

void vp9_frameworker_wait(VP9Worker* const worker, int row, int col,
                          RefCntBuffer *ref_buf) {
#if CONFIG_MULTITHREAD
  FrameWorkerData *const worker_data = (FrameWorkerData *)worker->data1;
  const VP9Decoder *const pbi = worker_data->pbi;
  const RefCntBuffer *const cur_buf = pbi->cur_buf;

  // Check if worker already release the ref_buf.
  if (!worker || !ref_buf || ref_buf->owner_frame_worker == NULL) return;

  pthread_mutex_lock(&worker_data->stats_mutex);
  while (!(cur_buf->row >= row && cur_buf->col >= col)
         && pbi->cur_buf == ref_buf && ref_buf->owner_frame_worker != NULL) {
    pthread_cond_wait(&worker_data->stats_cond, &worker_data->stats_mutex);
  }
  pthread_mutex_unlock(&worker_data->stats_mutex);
#else
  (void)worker;
  (void)row;
  (void)col;
  (void)ref_buf;
#endif  // CONFIG_MULTITHREAD
}

void vp9_frameworker_broadcast(VP9Worker* const worker, int row, int col) {
#if CONFIG_MULTITHREAD
  FrameWorkerData *const worker_data = (FrameWorkerData *)worker->data1;
  const VP9Decoder *const pbi = worker_data->pbi;
  RefCntBuffer *const cur_buf = pbi->cur_buf;

  pthread_mutex_lock(&worker_data->stats_mutex);
  cur_buf->row = row;
  cur_buf->col = col;
  pthread_cond_signal(&worker_data->stats_cond);
  pthread_mutex_unlock(&worker_data->stats_mutex);
#else
  (void)worker;
  (void)row;
  (void)col;
#endif  // CONFIG_MULTITHREAD
}

void vp9_frameworker_copy_context(VP9Worker * const dst_worker,
                                  const VP9Worker * const src_worker) {
#if CONFIG_MULTITHREAD
  FrameWorkerData *const src_worker_data =
  (FrameWorkerData *)src_worker->data1;
  FrameWorkerData *const dst_worker_data =
  (FrameWorkerData *)dst_worker->data1;
  const VP9_COMMON *const src_cm = &src_worker_data->pbi->common;
  VP9_COMMON *const dst_cm = &dst_worker_data->pbi->common;
  int i;

  // Wait until source frame's context is ready.
  pthread_mutex_lock(&src_worker_data->stats_mutex);
  while (!src_worker_data->frame_context_ready) {
    pthread_cond_wait(&src_worker_data->stats_cond,
        &src_worker_data->stats_mutex);
  }
  pthread_mutex_unlock(&src_worker_data->stats_mutex);

  dst_worker_data->pbi->prev_buf = src_worker_data->pbi->cur_buf;
  dst_cm->last_width = src_cm->width;
  dst_cm->last_height = src_cm->height;
  dst_cm->display_width = src_cm->display_width;
  dst_cm->display_height = src_cm->display_height;
  dst_cm->subsampling_x = src_cm->subsampling_x;
  dst_cm->subsampling_y = src_cm->subsampling_y;

  dst_cm->last_show_frame = src_cm->show_frame;

  dst_cm-> new_fb_idx = src_cm->new_fb_idx;
  dst_cm-> last_frame_type = src_cm->last_frame_type;
  dst_cm-> frame_type = src_cm->frame_type;

  for (i = 0; i < REF_FRAMES; ++i)
    dst_cm->ref_frame_map[i] = src_cm->next_ref_frame_map[i];

  if (src_worker_data->decoding_done) {
    dst_cm->prev_mip = src_cm->prev_mip;
    dst_cm->prev_mi = src_cm->prev_mi;
    dst_cm->prev_mi_grid_base = src_cm->prev_mi_grid_base;
    dst_cm->prev_mi_grid_visible = src_cm->prev_mi_grid_visible;
    dst_cm->last_frame_seg_map = src_cm->last_frame_seg_map;
  } else {
    dst_cm->prev_mip = src_cm->mip;
    dst_cm->prev_mi = src_cm->mi;
    dst_cm->prev_mi_grid_base = src_cm->mi_grid_base;
    dst_cm->prev_mi_grid_visible = src_cm->mi_grid_visible;
    dst_cm->last_frame_seg_map = src_cm->current_frame_seg_map;
  }

  dst_cm->lf.last_sharpness_level = src_cm->lf.sharpness_level;

  for (i = 0; i < MAX_REF_LF_DELTAS; ++i) {
    dst_cm->lf.last_ref_deltas[i] = src_cm->lf.ref_deltas[i];
    dst_cm->lf.ref_deltas[i] = src_cm->lf.ref_deltas[i];
  }

  for (i = 0; i < MAX_MODE_LF_DELTAS; ++i)
    dst_cm-> lf.last_mode_deltas[i] = src_cm->lf.mode_deltas[i];

  for (i = 0; i < FRAME_CONTEXTS; ++i)
    dst_cm-> frame_contexts[i] = src_cm->frame_contexts[i];

#else
  (void) dst_worker;
  (void) src_worker;
#endif  // CONFIG_MULTITHREAD
}
