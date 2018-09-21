/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef _VP9_JOB_QUEUE_H_
#define _VP9_JOB_QUEUE_H_

#include "vpx_util/vpx_thread.h"

typedef struct {
  /* Pointer to buffer base which contains the jobs */
  uint8_t *buf_base;

  /* Pointer to current address where new job can be added */
  uint8_t *volatile buf_wr;

  /* Pointer to current address from where next job can be obtained */
  uint8_t *volatile buf_rd;

  /* Pointer to end of job buffer */
  uint8_t *buf_end;

  /* Flag to indicate jobq has to be terminated */
  int terminate;

#if CONFIG_MULTITHREAD
  /* Mutex used to keep the functions thread-safe */
  pthread_mutex_t mutex;

  /* Declaration of thread condition variable */
  pthread_cond_t cond;
#endif
} jobq_t;

void vp9_jobq_init(jobq_t *jobq, uint8_t *buf, int buf_size);
void vp9_jobq_reset(jobq_t *jobq);
void vp9_jobq_deinit(jobq_t *jobq);
void vp9_jobq_terminate(jobq_t *jobq);
int vp9_jobq_queue(jobq_t *jobq, void *job, int job_size);
int vp9_jobq_dequeue(jobq_t *jobq, void *job, int job_size, int blocking);

#endif /* _VP9_JOB_QUEUE_H_ */
