/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_ENCODER_VP9_JOB_QUEUE_H_
#define VP9_ENCODER_VP9_JOB_QUEUE_H_

typedef enum {
  FIRST_PASS_JOB,
  ENCODE_JOB,
  ARNR_JOB,
  NUM_JOB_TYPES,
} ENC_JOB_TYPES_T;

// Encode job parameters
typedef struct {
  int vert_unit_row_num;  // Index of the vertical unit row
  int tile_col_id;        // tile col id within a tile
  int tile_row_id;        // tile col id within a tile
} job_node_t;

// Job queue element parameters
typedef struct {
  // Pointer to the next link in the job queue
  void *next;

  // Job information context of the module
  job_node_t job_info;
} job_queue_t;

// Job queue handle
typedef struct {
  // Pointer to the next link in the job queue
  void *next;

  // Counter to store the number of jobs picked up for processing
  int num_jobs_acquired;
} JobQueueHandle;

#endif  // VP9_ENCODER_VP9_JOB_QUEUE_H_
