/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SCHED_ATOMIC_H_
#define SCHED_ATOMIC_H_

#include "vp9/sched/thread.h"

typedef struct atomic {
  volatile int counter;
#ifdef _WIN32
  pthread_mutex_t mutex;
#endif
} atomic_t;

#ifndef _WIN32
static INLINE void atomic_init(atomic_t *d, int v) {
  d->counter = v;
}

static INLINE void atomic_fini(atomic_t *d) {
}

static INLINE int atomic_add(atomic_t *d, int v) {
  return __sync_add_and_fetch(&d->counter, v);
}

static INLINE int atomic_sub(atomic_t *d, int v) {
  return __sync_sub_and_fetch(&d->counter, v);
}

#else
static INLINE void atomic_init(atomic_t *d, int v) {
  d->counter = v;
  pthread_mutex_init(&d->mutex, NULL);
}

static INLINE void atomic_fini(atomic_t *d) {
  pthread_mutex_destroy(&d->mutex);
}

static INLINE int atomic_add(atomic_t *d, int v) {
  int rv;
  pthread_mutex_lock(&d->mutex);
  rv = ++d->counter;
  pthread_mutex_unlock(&d->mutex);
  return rv;
}

static INLINE int atomic_sub(atomic_t *d, int v) {
  int rv;
  pthread_mutex_lock(&d->mutex);
  rv = --d->counter;
  pthread_mutex_unlock(&d->mutex);
  return rv;
}

#endif

static INLINE int atomic_inc(atomic_t *d) {
  return atomic_add(d, 1);
}

static INLINE int atomic_dec(atomic_t *d) {
  return atomic_sub(d, 1);
}

static INLINE int atomic_get(atomic_t *d) {
  return d->counter;
}

#endif  // SCHED_ATOMIC_H_
