/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "vp9/common/vp9_thread_common.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_multi_thread.h"

void *vp9_enc_grp_get_next_job(MultiThreadHandle *multi_thread_ctxt,
                               int job_type, int tile_id) {
  TileRowMTInfo *tile_row_mt_info;
  JobQueueHandle *job_queue_hdl = NULL;
  void *next = NULL;
  pthread_mutex_t *mutex_handle = NULL;

  tile_row_mt_info =
      (TileRowMTInfo *)(&multi_thread_ctxt->tile_row_mt_info[tile_id]);

  if (ENCODE_JOB == job_type) {
    job_queue_hdl = (JobQueueHandle *)&tile_row_mt_info->job_que_enc_hdl;
    mutex_handle = &tile_row_mt_info->tile_job_mutex;
  } else if (ARNR_JOB == job_type) {
    job_queue_hdl = (JobQueueHandle *)&tile_row_mt_info->job_que_arnr_hdl;
    mutex_handle = &tile_row_mt_info->arnr_job_mutex;
  }

  // lock the mutex for Q access
  pthread_mutex_lock(mutex_handle);
  // Get the next
  next = job_queue_hdl->next;
  if (NULL != next) {
    job_queue_t *job_queue = (job_queue_t *)next;
    // update the next job in the queue
    job_queue_hdl->next = job_queue->next;
    job_queue_hdl->num_jobs_acquired++;
  }
  // unlock the mutex
  pthread_mutex_unlock(mutex_handle);

  return (next);
}

void vp9_row_mt_mem_alloc(VP9_COMP *cpi) {
  struct VP9Common *cm = &cpi->common;
  MultiThreadHandle *multi_thread_ctxt = &cpi->multi_thread_ctxt;
  int tile_row, tile_col, shift;
  const int tile_cols = 1 << cm->log2_tile_cols;
  const int tile_rows = 1 << cm->log2_tile_rows;
  int num_enc_row_jobs, num_enc_jobs = 0;

  num_enc_row_jobs = cpi->oxcf.pass == 1 ? multi_thread_ctxt->num_mb_rows
                                         : multi_thread_ctxt->num_sb_rows;

  // Calculate total number of jobs
  num_enc_jobs = num_enc_row_jobs * tile_cols;

  multi_thread_ctxt->num_enc_jobs = num_enc_jobs;
  multi_thread_ctxt->enc_job_queue =
      (job_queue_t *)vpx_memalign(32, num_enc_jobs * sizeof(job_queue_t));

  multi_thread_ctxt->max_num_tile_cols = tile_cols;
  multi_thread_ctxt->max_num_tile_rows = tile_rows;

  // Create mutex for each tile
  for (tile_col = 0; tile_col < tile_cols; tile_col++) {
    TileRowMTInfo *tile_row_mt_info =
        &multi_thread_ctxt->tile_row_mt_info[tile_col];
    pthread_mutex_init(&tile_row_mt_info->tile_job_mutex, NULL);
  }

  // Allocate memory for row based multi-threading
  for (tile_col = 0; tile_col < tile_cols; tile_col++) {
    TileDataEnc *this_tile = &cpi->tile_data[tile_col];
    vp9_row_mt_sync_mem_alloc(&this_tile->row_mt_sync, cm, num_enc_row_jobs,
                              multi_thread_ctxt->num_row_mt_workers);

    if (cpi->oxcf.pass == 1)
      vp9_top_row_sync_mem_alloc(&this_tile->fp_top_row_sync, cm,
                                 num_enc_row_jobs);
  }

  // Assign the sync pointer of tile row zero for every tile row > 0
  for (tile_row = 1; tile_row < tile_rows; tile_row++) {
    for (tile_col = 0; tile_col < tile_cols; tile_col++) {
      TileDataEnc *this_tile = &cpi->tile_data[tile_row * tile_cols + tile_col];
      TileDataEnc *this_col_tile = &cpi->tile_data[tile_col];
      this_tile->row_mt_sync = this_col_tile->row_mt_sync;
    }
  }

  if (cpi->oxcf.pass == 1) {
    shift = 1;
  } else {
    shift = MI_BLOCK_SIZE_LOG2;
  }

  // Calculate the number of vertical units in the given tile row
  for (tile_row = 0; tile_row < tile_rows; tile_row++) {
    TileDataEnc *this_tile = &cpi->tile_data[tile_row * tile_cols];
    TileInfo *tile_info = &this_tile->tile_info;
    multi_thread_ctxt->num_tile_vert_units[tile_row] =
        get_num_vert_units(*tile_info, shift);
  }
}

void vp9_row_mt_mem_dealloc(VP9_COMP *cpi) {
  MultiThreadHandle *multi_thread_ctxt = &cpi->multi_thread_ctxt;
  int tile_col;

  // Deallocate memory for job queue
  vpx_free(multi_thread_ctxt->enc_job_queue);

  // Destroy mutex for each tile
  for (tile_col = 0; tile_col < multi_thread_ctxt->max_num_tile_cols;
       tile_col++) {
    TileRowMTInfo *tile_row_mt_info =
        &multi_thread_ctxt->tile_row_mt_info[tile_col];
    pthread_mutex_destroy(&tile_row_mt_info->tile_job_mutex);
  }

  // Free row based mult-threading sync memory
  for (tile_col = 0; tile_col < multi_thread_ctxt->max_num_tile_cols;
       tile_col++) {
    TileDataEnc *this_tile = &cpi->tile_data[tile_col];
    vp9_row_mt_sync_mem_dealloc(&this_tile->row_mt_sync);
    if (cpi->oxcf.pass == 1)
      vp9_top_row_sync_mem_dealloc(&this_tile->fp_top_row_sync);
  }
}

void vp9_multi_thread_init(VP9_COMP *cpi, YV12_BUFFER_CONFIG *Source) {
  MultiThreadHandle *multi_thread_ctxt = &cpi->multi_thread_ctxt;

  // Initialize frame level parameters
  // shift by 6 for SB
  multi_thread_ctxt->num_sb_cols = (Source->y_width + (1 << 6) - 1) >> 6;
  multi_thread_ctxt->num_sb_rows = (Source->y_height + (1 << 6) - 1) >> 6;

  multi_thread_ctxt->num_mb_cols = (Source->y_width + (1 << 4) - 1) >> 4;
  multi_thread_ctxt->num_mb_rows = (Source->y_height + (1 << 4) - 1) >> 4;

  multi_thread_ctxt->num_row_mt_workers = VPXMAX(cpi->oxcf.max_threads, 1);
}

void vp9_multi_thread_tile_init(VP9_COMP *cpi) {
  VP9_COMMON *const cm = &cpi->common;
  MultiThreadHandle *multi_thread_ctxt = &cpi->multi_thread_ctxt;
  const int tile_cols = 1 << cm->log2_tile_cols;
  MV zero_mv = {0, 0};
  int i;

  for (i = 0; i < tile_cols; i++) {
    TileDataEnc *this_tile = &cpi->tile_data[i];
    int num_job_rows = cpi->oxcf.pass == 1 ? multi_thread_ctxt->num_mb_rows
                                           : multi_thread_ctxt->num_sb_rows;

    // Initialize cur_col to -1 for all rows.
    memset(this_tile->row_mt_sync.cur_col, -1,
           sizeof(*this_tile->row_mt_sync.cur_col) * num_job_rows);
    memset(this_tile->fp_top_row_sync.cur_mb_row, -1,
           sizeof(*this_tile->fp_top_row_sync.cur_mb_row));
    vp9_zero(this_tile->fp_data);
    this_tile->fp_data.image_data_start_row = INVALID_ROW;
    this_tile->tile_start_best_ref_mv = zero_mv;
    this_tile->tile_start_lastmv = zero_mv;
  }
}

void vp9_assign_tile_to_thread(MultiThreadHandle *multi_thread_ctxt,
                               int tile_cols) {
  int tile_id = 0;
  int i;

  // Allocating the threads for the tiles
  for (i = 0; i < multi_thread_ctxt->num_row_mt_workers; i++) {
    multi_thread_ctxt->thread_id_to_tile_id[i] = tile_id++;
    if (tile_id == tile_cols) tile_id = 0;
  }
}

int vp9_get_job_queue_status(MultiThreadHandle *multi_thread_ctxt,
                             int cur_tile_id, ENC_JOB_TYPES_T job_type) {
  TileRowMTInfo *tile_row_mt_info;
  pthread_mutex_t *mutex;
  JobQueueHandle *job_que_hndl;
  int ret_percent_proc;

  tile_row_mt_info = &multi_thread_ctxt->tile_row_mt_info[cur_tile_id];

  if (ENCODE_JOB == job_type) {
    mutex = &tile_row_mt_info->tile_job_mutex;
    job_que_hndl = &tile_row_mt_info->job_que_enc_hdl;
  } else if (ARNR_JOB == job_type) {
    mutex = &tile_row_mt_info->arnr_job_mutex;
    job_que_hndl = &tile_row_mt_info->job_que_arnr_hdl;
  } else {
    job_que_hndl = NULL;
    mutex = NULL;
    assert(0);
  }

  // Critical section
  pthread_mutex_lock(mutex);
  // calculate the % completion
  ret_percent_proc =
      (job_que_hndl->num_jobs_acquired * 10000 / job_que_hndl->total_jobs);
  pthread_mutex_unlock(mutex);

  return (ret_percent_proc);
}

void vp9_prepare_job_queue(MultiThreadHandle *multi_thread_ctxt, int tile_cols,
                           ENC_JOB_TYPES_T task_type, int num_vert_units,
                           void *v_job_queue, int num_of_jobs) {
  job_queue_t *job_queue;
  int num_jobs, num_tile_row_jobs;
  int tile_col;

  job_queue = (job_queue_t *)v_job_queue;

  // memset the entire job queue buffer to zero
  memset(job_queue, 0, (num_of_jobs) * sizeof(job_queue_t));

  // Job queue preparation
  for (tile_col = 0; tile_col < tile_cols; tile_col++) {
    TileRowMTInfo *tile_ctxt = &multi_thread_ctxt->tile_row_mt_info[tile_col];
    job_queue_t *job_queue_curr;
    job_queue_t *job_queue_temp;
    int tile_row_cnt = 0;

    if (task_type == ENCODE_JOB) {
      tile_ctxt->job_que_enc_hdl.next = (void *)job_queue;
      tile_ctxt->job_que_enc_hdl.num_jobs_acquired = 0;
      tile_ctxt->job_que_enc_hdl.total_jobs = num_vert_units;
      tile_ctxt->job_tile = job_queue;
    } else if (task_type == ARNR_JOB) {
      tile_ctxt->job_que_arnr_hdl.next = (void *)job_queue;
      tile_ctxt->job_que_arnr_hdl.num_jobs_acquired = 0;
      tile_ctxt->job_que_arnr_hdl.total_jobs = num_vert_units;
    }

    job_queue_curr = job_queue;
    job_queue_temp = job_queue;

    // loop over all the vertical rows
    for (num_jobs = 0, num_tile_row_jobs = 0; num_jobs < num_vert_units;
         num_jobs++, num_tile_row_jobs++) {
      if (task_type == ENCODE_JOB) {
        job_queue_curr->job_info.encode_job_info.vert_unit_row_num = num_jobs;
        job_queue_curr->job_info.encode_job_info.tile_col_id = tile_col;
        job_queue_curr->job_info.encode_job_info.tile_row_id = tile_row_cnt;
      } else if (task_type == ARNR_JOB) {
        job_queue_curr->job_info.arf_job_info.vert_unit_row_num = num_jobs;
        job_queue_curr->job_info.arf_job_info.tile_col_id = tile_col;
        job_queue_curr->job_info.arf_job_info.tile_row_id = tile_row_cnt;
      }
      job_queue_curr->task_type = task_type;
      job_queue_curr->next = (void *)(job_queue_temp + 1);
      job_queue_curr = ++job_queue_temp;

      if (ENCODE_JOB == task_type) {
        if (num_tile_row_jobs >=
            multi_thread_ctxt->num_tile_vert_units[tile_row_cnt] - 1) {
          tile_row_cnt++;
          num_tile_row_jobs = -1;
        }
      }
    }

    // set the last pointer to NULL
    job_queue_curr += -1;
    job_queue_curr->next = (void *)NULL;

    // Move to the next speed tile
    job_queue += num_vert_units;
  }
  return;
}

int vp9_get_tiles_proc_status(MultiThreadHandle *multi_thread_ctxt,
                              int *cur_tile_id, ENC_JOB_TYPES_T job_type,
                              int tile_cols) {
  int i;
  int min_tile_id = -1;  // Store the tile ID with minimum proc done
  int min_pc_proc = 10000;
  int temp_pc_proc;

  // Check for the status of all the tiles
  for (i = 0; i < tile_cols; i++) {
    temp_pc_proc = vp9_get_job_queue_status(multi_thread_ctxt, i, job_type);
    if (temp_pc_proc < min_pc_proc) {
      min_pc_proc = temp_pc_proc;
      min_tile_id = i;
    }
  }

  if (-1 == min_tile_id) {
    return 1;
  } else {
    // Update the cur ID to the next tile ID that will be processed,
    // which will be the least proc tile
    *cur_tile_id = min_tile_id;
    return 0;
  }
}
