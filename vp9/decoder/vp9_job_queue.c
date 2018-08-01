/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "vp9_job_queue.h"

JOB_QUEUE_STATUS vp9_jobq_lock(jobq_t *ps_jobq) {
#if CONFIG_MULTITHREAD
  int32_t retval;
  retval = pthread_mutex_lock(&ps_jobq->jobq_mutex);
  if (retval) {
    return JOB_Q_FAIL;
  }
#else
  (void)ps_jobq;
#endif
  return JOB_Q_SUCCESS;
}

int32_t vp9_jobq_unlock(jobq_t *ps_jobq) {
#if CONFIG_MULTITHREAD
  int32_t retval;
  retval = pthread_mutex_unlock(&ps_jobq->jobq_mutex);
  if (retval) {
    return JOB_Q_FAIL;
  }
#else
  (void)ps_jobq;
#endif
  return JOB_Q_SUCCESS;
}

int32_t vp9_jobq_yield(jobq_t *ps_jobq) {
  JOB_QUEUE_STATUS ret = JOB_Q_SUCCESS;
#if CONFIG_MULTITHREAD
  JOB_QUEUE_STATUS rettmp;
  rettmp = vp9_jobq_unlock(ps_jobq);
  RETURN_IF((rettmp != JOB_Q_SUCCESS), rettmp);

  // NOP(1024 * 8);
  sched_yield();

  rettmp = vp9_jobq_lock(ps_jobq);
  RETURN_IF((rettmp != JOB_Q_SUCCESS), rettmp);
#else
  (void)ps_jobq;
#endif
  return ret;
}

JOB_QUEUE_STATUS vp9_jobq_free(jobq_t *ps_jobq) {
  JOB_QUEUE_STATUS ret = JOB_Q_SUCCESS;
#if CONFIG_MULTITHREAD
  ret = pthread_mutex_destroy(&ps_jobq->jobq_mutex);
  ret += pthread_cond_destroy(&ps_jobq->cond);

  if (0 != ret) ret = JOB_Q_FAIL;
#else
  (void)ps_jobq;
#endif
  return ret;
}

void vp9_jobq_init(void *pv_buf, int32_t buf_size, jobq_t *ps_jobq) {
#if CONFIG_MULTITHREAD
  uint8_t *pu1_buf;
  pu1_buf = (uint8_t *)pv_buf;

  if (buf_size > 0) {
    pthread_mutex_init(&ps_jobq->jobq_mutex, NULL);
    pthread_cond_init(&ps_jobq->cond, NULL);
    ps_jobq->pv_buf_base = pu1_buf;
    ps_jobq->pv_buf_wr = pu1_buf;
    ps_jobq->pv_buf_rd = pu1_buf;
    ps_jobq->pv_buf_end = pu1_buf + buf_size;
    ps_jobq->i4_terminate = 0;
  }
#else
  (void)pv_buf;
  (void)buf_size;
  (void)ps_jobq;
#endif
}

/**
 *  Resets the jobq context by initializing job queue context elements
 */
JOB_QUEUE_STATUS vp9_jobq_reset(jobq_t *ps_jobq) {
  JOB_QUEUE_STATUS ret = JOB_Q_SUCCESS;
#if CONFIG_MULTITHREAD
  ret = vp9_jobq_lock(ps_jobq);
  RETURN_IF((ret != JOB_Q_SUCCESS), ret);

  ps_jobq->pv_buf_wr = ps_jobq->pv_buf_base;
  ps_jobq->pv_buf_rd = ps_jobq->pv_buf_base;
  ps_jobq->i4_terminate = 0;
  ret = vp9_jobq_unlock(ps_jobq);
  RETURN_IF((ret != JOB_Q_SUCCESS), ret);
#else
  (void)ps_jobq;
#endif
  return ret;
}

/**
 *   De-initializes the jobq context by calling vp9_jobq_reset()
 * and then destroying the mutex created
 */
JOB_QUEUE_STATUS vp9_jobq_deinit(jobq_t *ps_jobq) {
#if CONFIG_MULTITHREAD
  JOB_QUEUE_STATUS retval;
  JOB_QUEUE_STATUS ret = JOB_Q_SUCCESS;

  ret = vp9_jobq_reset(ps_jobq);
  RETURN_IF((ret != JOB_Q_SUCCESS), ret);

  retval = pthread_mutex_destroy(&ps_jobq->jobq_mutex);
  retval += pthread_cond_destroy(&ps_jobq->cond);
  if (retval) {
    return JOB_Q_FAIL;
  }
#else
  (void)ps_jobq;
#endif
  return JOB_Q_SUCCESS;
}

/*
 * Terminates the jobq by setting a flag in context.
 */

JOB_QUEUE_STATUS vp9_jobq_terminate(jobq_t *ps_jobq) {
  JOB_QUEUE_STATUS ret = JOB_Q_SUCCESS;
#if CONFIG_MULTITHREAD
  ret = vp9_jobq_lock(ps_jobq);
  RETURN_IF((ret != JOB_Q_SUCCESS), ret);

  ps_jobq->i4_terminate = 1;
  pthread_cond_broadcast(&ps_jobq->cond);

  ret = vp9_jobq_unlock(ps_jobq);
  RETURN_IF((ret != JOB_Q_SUCCESS), ret);
#else
  (void)ps_jobq;
#endif
  return ret;
}

JOB_QUEUE_STATUS vp9_jobq_queue(jobq_t *ps_jobq, void *pv_job, int32_t job_size,
                                int32_t blocking) {
  JOB_QUEUE_STATUS ret = JOB_Q_SUCCESS;
#if CONFIG_MULTITHREAD
  JOB_QUEUE_STATUS rettmp;
  uint8_t *pu1_buf;
  UNUSED(blocking);

  rettmp = vp9_jobq_lock(ps_jobq);
  RETURN_IF((rettmp != JOB_Q_SUCCESS), rettmp);

  pu1_buf = (uint8_t *)ps_jobq->pv_buf_wr;
  if ((uint8_t *)ps_jobq->pv_buf_end >= (pu1_buf + job_size)) {
    memcpy(ps_jobq->pv_buf_wr, pv_job, job_size);
    ps_jobq->pv_buf_wr = (uint8_t *)ps_jobq->pv_buf_wr + job_size;
    pthread_cond_signal(&ps_jobq->cond);
    ret = JOB_Q_SUCCESS;
  } else {
    /* Handle wrap around case */
    /* Wait for pv_buf_rd to consume first job_size number of bytes
     * from the beginning of job queue
     */
    ret = JOB_Q_FAIL;
  }

  ps_jobq->i4_terminate = 0;

  rettmp = vp9_jobq_unlock(ps_jobq);
  RETURN_IF((rettmp != JOB_Q_SUCCESS), rettmp);
#else
  (void)ps_jobq;
  (void)pv_job;
  (void)job_size;
  (void)blocking;
#endif
  return ret;
}

JOB_QUEUE_STATUS vp9_jobq_dequeue(jobq_t *ps_jobq, void *pv_job,
                                  int32_t job_size, int32_t blocking) {
  JOB_QUEUE_STATUS ret = JOB_Q_SUCCESS;
#if CONFIG_MULTITHREAD
  JOB_QUEUE_STATUS rettmp;
  volatile uint8_t *pu1_buf;

  rettmp = vp9_jobq_lock(ps_jobq);
  RETURN_IF((rettmp != JOB_Q_SUCCESS), rettmp);
  pu1_buf = (uint8_t *)ps_jobq->pv_buf_rd;

  if ((uint8_t *)ps_jobq->pv_buf_end >= (pu1_buf + job_size)) {
    while (1) {
      pu1_buf = (uint8_t *)ps_jobq->pv_buf_rd;
      if ((uint8_t *)ps_jobq->pv_buf_wr >= (pu1_buf + job_size)) {
        memcpy(pv_job, ps_jobq->pv_buf_rd, job_size);
        ps_jobq->pv_buf_rd = (uint8_t *)ps_jobq->pv_buf_rd + job_size;
        ret = JOB_Q_SUCCESS;
        break;
      } else {
        /* If all the entries have been dequeued, then break and return */
        if (1 == ps_jobq->i4_terminate) {
          ret = JOB_Q_FAIL;
          break;
        }

        if (1 == blocking) {
          pthread_cond_wait(&ps_jobq->cond, &ps_jobq->jobq_mutex);

        } else {
          /* If there is no job available,
           * and this is non blocking call then return fail */
          ret = JOB_Q_FAIL;
        }
      }
    }
  } else {
    /* Handle wrap around case */
    /* Wait for pv_buf_rd to consume first job_size number of bytes
     * from the beginning of job queue
     */
    ret = JOB_Q_FAIL;
  }
  rettmp = vp9_jobq_unlock(ps_jobq);
  RETURN_IF((rettmp != JOB_Q_SUCCESS), rettmp);
#else
  (void)ps_jobq;
  (void)pv_job;
  (void)job_size;
  (void)blocking;
#endif
  return ret;
}
