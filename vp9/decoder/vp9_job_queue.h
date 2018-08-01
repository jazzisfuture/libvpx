/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef _IVP9D_JOB_QUEUE_H_
#define _IVP9D_JOB_QUEUE_H_

#include "vpx_util/vpx_thread.h"

#define RETURN_IF(cond, retval) \
  if (cond) {                   \
    return (retval);            \
  }
#define UNUSED(x) (void)(x);

typedef enum {
  JOB_Q_SUCCESS = 0,
  JOB_Q_FAIL = 1,
} JOB_QUEUE_STATUS;

typedef struct {
  /** Pointer to buffer base which contains the jobs */
  void *pv_buf_base;

  /** Pointer to current address where new job can be added */
  void *pv_buf_wr;

  /** Pointer to current address from where next job can be obtained */
  void *pv_buf_rd;

  /** Pointer to end of job buffer */
  void *pv_buf_end;

  /** Flag to indicate jobq has to be terminated */
  int32_t i4_terminate;

#if CONFIG_MULTITHREAD
  /** Mutex used to keep the functions thread-safe */
  pthread_mutex_t jobq_mutex;

  /** Declaration of thread condition variable */
  pthread_cond_t cond;
#endif
} jobq_t;

int32_t vp9_jobq_ctxt_size(void);
void vp9_jobq_init(void *pv_buf, int32_t buf_size, jobq_t *ps_jobq);
JOB_QUEUE_STATUS vp9_jobq_free(jobq_t *ps_jobq);
JOB_QUEUE_STATUS vp9_jobq_reset(jobq_t *ps_jobq);
JOB_QUEUE_STATUS vp9_jobq_deinit(jobq_t *ps_jobq);
JOB_QUEUE_STATUS vp9_jobq_terminate(jobq_t *ps_jobq);
JOB_QUEUE_STATUS vp9_jobq_queue(jobq_t *ps_jobq, void *pv_job, int32_t job_size,
                                int32_t blocking);
JOB_QUEUE_STATUS vp9_jobq_dequeue(jobq_t *ps_jobq, void *pv_job,
                                  int32_t job_size, int32_t blocking);

#endif /* _VP9_JOB_QUEUE_H_ */
