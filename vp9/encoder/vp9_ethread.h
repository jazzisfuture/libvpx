/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_ENCODER_VP9_ETHREAD_H_
#define VP9_ENCODER_VP9_ETHREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

struct VP9_COMP;
struct ThreadData;

typedef struct EncWorkerData {
  struct VP9_COMP *cpi;
  struct ThreadData *td;
  int start;
  int thread_id;
} EncWorkerData;

void vp9_encode_tiles_mt(struct VP9_COMP *cpi);

void vp9_encode_fp_tiles_row_mt(struct VP9_COMP *cpi);

void vp9_row_mt_sync_read(VP9RowMTSync *const row_mt_sync, int r, int c);
void vp9_row_mt_sync_write(VP9RowMTSync *const row_mt_sync, int r, int c,
                           const int cols);

void vp9_row_mt_sync_read_dummy(VP9RowMTSync *const row_mt_sync, int r, int c);
void vp9_row_mt_sync_write_dummy(VP9RowMTSync *const row_mt_sync, int r, int c,
                                 const int cols);

void vp9_top_row_sync_read(VP9TopRowSync *const top_row_sync, int r);
void vp9_top_row_sync_write(VP9TopRowSync *const top_row_sync, int r);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_ETHREAD_H_
