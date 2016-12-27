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
  ENCODE_JOB = 0,
  ARNR_JOB = 1,
  NUM_ENC_JOBS_QUEUES,
} ENC_JOB_TYPES_T;

// Encode job parameters
typedef struct {
  int vert_unit_row_num;  // Index of the vertical unit row
  int tile_col_id;        // bitstream tile col id within a speed tile
  int tile_row_id;        // bitstream tile col id within a speed tile
} encode_job_node_t;

typedef struct {
  int vert_unit_row_num;  // Index of the vertical unit row
  int tile_col_id;        // bitstream tile col id within a speed tile
  int tile_row_id;        // bitstream tile row id within a speed tile
} arf_job_node_t;

typedef union {
  encode_job_node_t encode_job_info;
  arf_job_node_t arf_job_info;
} job_info_t;

// Job queue element parameters
typedef struct {
  // Pointer to the next link in the job queue
  void *next;

  // Job information context of the module
  job_info_t job_info;

  // indicates what type of task is to be executed
  ENC_JOB_TYPES_T task_type;
} job_queue_t;

// Job queue handle
typedef struct {
  // Pointer to the next link in the job queue
  void *next;

  // counter to store the number of jobs picked up for processing
  int num_jobs_acquired;

  // total number of jobs in the job queue
  int total_jobs;
} JobQueueHandle;

#endif  // VP9_ENCODER_VP9_JOB_QUEUE_H_
