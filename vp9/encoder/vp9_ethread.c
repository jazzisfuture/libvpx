/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/encoder/vp9_encodeframe.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_ethread.h"
#include "vp9/encoder/vp9_firstpass.h"
#include "vpx_dsp/vpx_dsp_common.h"

static void accumulate_rd_opt(ThreadData *td, ThreadData *td_t) {
  int i, j, k, l, m, n;

  for (i = 0; i < REFERENCE_MODES; i++)
    td->rd_counts.comp_pred_diff[i] += td_t->rd_counts.comp_pred_diff[i];

  for (i = 0; i < SWITCHABLE_FILTER_CONTEXTS; i++)
    td->rd_counts.filter_diff[i] += td_t->rd_counts.filter_diff[i];

  for (i = 0; i < TX_SIZES; i++)
    for (j = 0; j < PLANE_TYPES; j++)
      for (k = 0; k < REF_TYPES; k++)
        for (l = 0; l < COEF_BANDS; l++)
          for (m = 0; m < COEFF_CONTEXTS; m++)
            for (n = 0; n < ENTROPY_TOKENS; n++)
              td->rd_counts.coef_counts[i][j][k][l][m][n] +=
                  td_t->rd_counts.coef_counts[i][j][k][l][m][n];
}

static int enc_worker_hook(EncWorkerData *const thread_data, void *unused) {
  VP9_COMP *const cpi = thread_data->cpi;
  const VP9_COMMON *const cm = &cpi->common;
  const int tile_cols = 1 << cm->log2_tile_cols;
  const int tile_rows = 1 << cm->log2_tile_rows;
  int t;

  (void)unused;

  for (t = thread_data->start; t < tile_rows * tile_cols;
       t += cpi->num_workers) {
    int tile_row = t / tile_cols;
    int tile_col = t % tile_cols;

    vp9_encode_tile(cpi, thread_data->td, tile_row, tile_col);
  }

  return 0;
}

static int get_max_tile_cols(VP9_COMP *cpi) {
  const int aligned_width = ALIGN_POWER_OF_TWO(cpi->oxcf.width, MI_SIZE_LOG2);
  int mi_cols = aligned_width >> MI_SIZE_LOG2;
  int min_log2_tile_cols, max_log2_tile_cols;
  int log2_tile_cols;

  vp9_get_tile_n_bits(mi_cols, &min_log2_tile_cols, &max_log2_tile_cols);
  log2_tile_cols =
      clamp(cpi->oxcf.tile_columns, min_log2_tile_cols, max_log2_tile_cols);
  return (1 << log2_tile_cols);
}

static void create_enc_workers(VP9_COMP *cpi, int num_workers) {
  VP9_COMMON *const cm = &cpi->common;
  const VPxWorkerInterface *const winterface = vpx_get_worker_interface();
  int i;

  // Only run once to create threads and allocate thread data.
  if (cpi->num_workers == 0) {
    int allocated_workers = num_workers;

    // While using SVC, we need to allocate threads according to the highest
    // resolution.
    if (cpi->use_svc) {
      int max_tile_cols = get_max_tile_cols(cpi);
      allocated_workers = VPXMIN(cpi->oxcf.max_threads, max_tile_cols);
    }

    CHECK_MEM_ERROR(cm, cpi->workers,
                    vpx_malloc(allocated_workers * sizeof(*cpi->workers)));

    CHECK_MEM_ERROR(cm, cpi->tile_thr_data,
                    vpx_calloc(allocated_workers, sizeof(*cpi->tile_thr_data)));

    for (i = 0; i < allocated_workers; i++) {
      VPxWorker *const worker = &cpi->workers[i];
      EncWorkerData *thread_data = &cpi->tile_thr_data[i];

      ++cpi->num_workers;
      winterface->init(worker);

      if (i < allocated_workers - 1) {
        thread_data->cpi = cpi;

        // Allocate thread data.
        CHECK_MEM_ERROR(cm, thread_data->td,
                        vpx_memalign(32, sizeof(*thread_data->td)));
        vp9_zero(*thread_data->td);

        // Set up pc_tree.
        thread_data->td->leaf_tree = NULL;
        thread_data->td->pc_tree = NULL;
        vp9_setup_pc_tree(cm, thread_data->td);

        // Allocate frame counters in thread data.
        CHECK_MEM_ERROR(cm, thread_data->td->counts,
                        vpx_calloc(1, sizeof(*thread_data->td->counts)));

        // Create threads
        if (!winterface->reset(worker))
          vpx_internal_error(&cm->error, VPX_CODEC_ERROR,
                             "Tile encoder thread creation failed");
      } else {
        // Main thread acts as a worker and uses the thread data in cpi.
        thread_data->cpi = cpi;
        thread_data->td = &cpi->td;
      }

      winterface->sync(worker);
    }
  }
}

static void launch_enc_workers(VP9_COMP *cpi, VPxWorkerHook hook, void *data1,
                               void *data2, int num_workers) {
  const VPxWorkerInterface *const winterface = vpx_get_worker_interface();
  int i;

  (void)data1;

  for (i = 0; i < num_workers; i++) {
    VPxWorker *const worker = &cpi->workers[i];
    worker->hook = (VPxWorkerHook)hook;
    worker->data1 = &cpi->tile_thr_data[i];
    worker->data2 = data2;
  }

  // Encode a frame
  for (i = 0; i < num_workers; i++) {
    VPxWorker *const worker = &cpi->workers[i];
    EncWorkerData *const thread_data = (EncWorkerData *)worker->data1;

    // Set the starting tile for each thread.
    thread_data->start = i;

    if (i == cpi->num_workers - 1)
      winterface->execute(worker);
    else
      winterface->launch(worker);
  }

  // Encoding ends.
  for (i = 0; i < num_workers; i++) {
    VPxWorker *const worker = &cpi->workers[i];
    winterface->sync(worker);
  }
}

void vp9_encode_tiles_mt(VP9_COMP *cpi) {
  VP9_COMMON *const cm = &cpi->common;
  const int tile_cols = 1 << cm->log2_tile_cols;
  const int num_workers = VPXMIN(cpi->oxcf.max_threads, tile_cols);
  int i;

  vp9_init_tile_data(cpi);

  create_enc_workers(cpi, num_workers);

  for (i = 0; i < num_workers; i++) {
    EncWorkerData *thread_data;
    thread_data = &cpi->tile_thr_data[i];

    // Before encoding a frame, copy the thread data from cpi.
    if (thread_data->td != &cpi->td) {
      thread_data->td->mb = cpi->td.mb;
      thread_data->td->rd_counts = cpi->td.rd_counts;
    }
    if (thread_data->td->counts != &cpi->common.counts) {
      memcpy(thread_data->td->counts, &cpi->common.counts,
             sizeof(cpi->common.counts));
    }

    // Handle use_nonrd_pick_mode case.
    if (cpi->sf.use_nonrd_pick_mode) {
      MACROBLOCK *const x = &thread_data->td->mb;
      MACROBLOCKD *const xd = &x->e_mbd;
      struct macroblock_plane *const p = x->plane;
      struct macroblockd_plane *const pd = xd->plane;
      PICK_MODE_CONTEXT *ctx = &thread_data->td->pc_root->none;
      int j;

      for (j = 0; j < MAX_MB_PLANE; ++j) {
        p[j].coeff = ctx->coeff_pbuf[j][0];
        p[j].qcoeff = ctx->qcoeff_pbuf[j][0];
        pd[j].dqcoeff = ctx->dqcoeff_pbuf[j][0];
        p[j].eobs = ctx->eobs_pbuf[j][0];
      }
    }
  }

  launch_enc_workers(cpi, (VPxWorkerHook)enc_worker_hook,
                     &cpi->tile_thr_data[0], NULL, num_workers);

  for (i = 0; i < num_workers; i++) {
    VPxWorker *const worker = &cpi->workers[i];
    EncWorkerData *const thread_data = (EncWorkerData *)worker->data1;

    // Accumulate counters.
    if (i < cpi->num_workers - 1) {
      vp9_accumulate_frame_counts(&cm->counts, thread_data->td->counts, 0);
      accumulate_rd_opt(&cpi->td, thread_data->td);
    }
  }
}

static void accumulate_fp_tile_stat(TileDataEnc *tile_data,
                                    TileDataEnc *tile_data_t) {
  tile_data->fp_data.intra_factor += tile_data_t->fp_data.intra_factor;
  tile_data->fp_data.brightness_factor +=
      tile_data_t->fp_data.brightness_factor;
  tile_data->fp_data.coded_error += tile_data_t->fp_data.coded_error;
  tile_data->fp_data.sr_coded_error += tile_data_t->fp_data.sr_coded_error;
  tile_data->fp_data.frame_noise_energy += tile_data_t->fp_data.frame_noise_energy;
  tile_data->fp_data.intra_error += tile_data_t->fp_data.intra_error;
  tile_data->fp_data.intercount += tile_data_t->fp_data.intercount;
  tile_data->fp_data.second_ref_count += tile_data_t->fp_data.second_ref_count;
  tile_data->fp_data.neutral_count += tile_data_t->fp_data.neutral_count;
  tile_data->fp_data.intra_skip_count += tile_data_t->fp_data.intra_skip_count;
  tile_data->fp_data.mvcount += tile_data_t->fp_data.mvcount;
  tile_data->fp_data.sum_mvr += tile_data_t->fp_data.sum_mvr;
  tile_data->fp_data.sum_mvr_abs += tile_data_t->fp_data.sum_mvr_abs;
  tile_data->fp_data.sum_mvc += tile_data_t->fp_data.sum_mvc;
  tile_data->fp_data.sum_mvc_abs += tile_data_t->fp_data.sum_mvc_abs;
  tile_data->fp_data.sum_mvrs += tile_data_t->fp_data.sum_mvrs;
  tile_data->fp_data.sum_mvcs += tile_data_t->fp_data.sum_mvcs;
  tile_data->fp_data.sum_in_vectors += tile_data_t->fp_data.sum_in_vectors;
  tile_data->fp_data.new_mv_count += tile_data_t->fp_data.new_mv_count;
  tile_data->fp_data.intra_smooth_count +=
      tile_data_t->fp_data.intra_smooth_count;
  tile_data->fp_data.image_data_start_row =
      VPXMIN(tile_data->fp_data.image_data_start_row,
             tile_data_t->fp_data.image_data_start_row) == INVALID_ROW
          ? VPXMAX(tile_data->fp_data.image_data_start_row,
                   tile_data_t->fp_data.image_data_start_row)
          : VPXMIN(tile_data->fp_data.image_data_start_row,
                   tile_data_t->fp_data.image_data_start_row);
}

static void accumulate_fp_mb_row_stat(TileDataEnc *this_tile,
                               FIRSTPASS_DATA fp_acc_data) {
  this_tile->fp_data.intra_factor += fp_acc_data.intra_factor;
  this_tile->fp_data.brightness_factor += fp_acc_data.brightness_factor;
  this_tile->fp_data.coded_error += fp_acc_data.coded_error;
  this_tile->fp_data.sr_coded_error += fp_acc_data.sr_coded_error;
  this_tile->fp_data.frame_noise_energy += fp_acc_data.frame_noise_energy;
  this_tile->fp_data.intra_error += fp_acc_data.intra_error;
  this_tile->fp_data.intercount += fp_acc_data.intercount;
  this_tile->fp_data.second_ref_count += fp_acc_data.second_ref_count;
  this_tile->fp_data.neutral_count += fp_acc_data.neutral_count;
  this_tile->fp_data.intra_skip_count += fp_acc_data.intra_skip_count;
  this_tile->fp_data.mvcount += fp_acc_data.mvcount;
  this_tile->fp_data.sum_mvr += fp_acc_data.sum_mvr;
  this_tile->fp_data.sum_mvr_abs += fp_acc_data.sum_mvr_abs;
  this_tile->fp_data.sum_mvc += fp_acc_data.sum_mvc;
  this_tile->fp_data.sum_mvc_abs += fp_acc_data.sum_mvc_abs;
  this_tile->fp_data.sum_mvrs += fp_acc_data.sum_mvrs;
  this_tile->fp_data.sum_mvcs += fp_acc_data.sum_mvcs;
  this_tile->fp_data.sum_in_vectors += fp_acc_data.sum_in_vectors;
  this_tile->fp_data.new_mv_count += fp_acc_data.new_mv_count;
  this_tile->fp_data.intra_smooth_count += fp_acc_data.intra_smooth_count;
  this_tile->fp_data.image_data_start_row =
      VPXMIN(this_tile->fp_data.image_data_start_row,
             fp_acc_data.image_data_start_row) == INVALID_ROW
          ? VPXMAX(this_tile->fp_data.image_data_start_row,
                   fp_acc_data.image_data_start_row)
          : VPXMIN(this_tile->fp_data.image_data_start_row,
                   fp_acc_data.image_data_start_row);
}

static int first_pass_worker_hook(EncWorkerData *const thread_data,
                                  void *unused) {
  VP9_COMP *const cpi = thread_data->cpi;
  const VP9_COMMON *const cm = &cpi->common;
  const int tile_cols = 1 << cm->log2_tile_cols;
  const int tile_rows = 1 << cm->log2_tile_rows;
  FIRSTPASS_DATA fp_acc_data;
  MV zero_mv = {0, 0};
  MV best_ref_mv;
  MV lastmv;
  int mb_row;
  int t;

  (void)unused;

  for (t = thread_data->start; t < tile_rows * tile_cols;
       t += cpi->num_workers) {
    int tile_row = t / tile_cols;
    int tile_col = t % tile_cols;
    TileDataEnc *this_tile = &cpi->tile_data[tile_row * tile_cols + tile_col];
    this_tile->tile_start_lastmv = zero_mv;
    this_tile->tile_start_best_ref_mv = zero_mv;
    for (mb_row = 0; mb_row < cm->mb_rows; ++mb_row) {
      lastmv = zero_mv;
      best_ref_mv = zero_mv;
      vp9_zero(fp_acc_data);
      fp_acc_data.image_data_start_row = INVALID_ROW;
      vp9_first_pass_encode_tile_mb_row(cpi, thread_data->td, &fp_acc_data,
                                        this_tile, &lastmv, &best_ref_mv,
                                        mb_row);
      accumulate_fp_mb_row_stat(this_tile, fp_acc_data);
    }
  }
  return 0;
}

void vp9_encode_fp_tiles_mt(VP9_COMP *cpi) {
  VP9_COMMON *const cm = &cpi->common;
  const int tile_cols = 1 << cm->log2_tile_cols;
  // const int num_workers = VPXMIN(cpi->oxcf.max_threads, tile_cols);
  const int num_workers = VPXMAX(VPXMIN(cpi->oxcf.max_threads, tile_cols), 1);
  TileDataEnc *first_tile_col;
  int i;

  vp9_init_tile_data(cpi);

  create_enc_workers(cpi, num_workers);

  for (i = 0; i < num_workers; i++) {
    EncWorkerData *thread_data;
    thread_data = &cpi->tile_thr_data[i];

    // Before encoding a frame, copy the thread data from cpi.
    if (thread_data->td != &cpi->td) {
      thread_data->td->mb = cpi->td.mb;
    }
  }

  for (i = 0; i < tile_cols; i++) {
    TileDataEnc *this_tile = &cpi->tile_data[i];
    vp9_zero(this_tile->fp_data);
    this_tile->fp_data.image_data_start_row = INVALID_ROW;
  }

  launch_enc_workers(cpi, (VPxWorkerHook)first_pass_worker_hook,
                     &cpi->tile_thr_data[0], NULL, num_workers);

  first_tile_col = &cpi->tile_data[0];
  for (i = 1; i < tile_cols; i++) {
    TileDataEnc *this_tile = &cpi->tile_data[i];
    accumulate_fp_tile_stat(first_tile_col, this_tile);
  }
}
