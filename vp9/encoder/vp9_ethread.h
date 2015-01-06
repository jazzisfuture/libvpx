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

struct VP9_COMP;
struct ThreadData;

typedef struct EncWorkerData {
  struct VP9_COMP *cpi;
  struct ThreadData *td;
  int start;
} EncWorkerData;

void vp9_encode_tiles_mt(struct VP9_COMP *cpi);

void vp9e_loop_filter_frame(YV12_BUFFER_CONFIG *frame,
                            VP9_COMMON *cm, MACROBLOCKD *xd,
                            int frame_filter_level,
                            int y_only, int partial_frame,
                            VP9Worker *workers, int num_workers,
                            VP9LfSync *lf_sync);

#endif  // VP9_ENCODER_VP9_ETHREAD_H_
