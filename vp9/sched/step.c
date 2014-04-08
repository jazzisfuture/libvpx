/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include "vpx_mem/vpx_mem.h"
#include "vp9/sched/step.h"

// for ffs
#if  __ANDROID__ || _SVID_SOURCE || _BSD_SOURCE || _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
#include <strings.h>
#else
typedef unsigned char uint8_t;
static INLINE uint8_t get_byte(int i, int nr) {
  return (i & (0xFF << (nr << 3)));
}

static int ffs(int i) {
  int n;
  int b;

  for (b = 0; b < sizeof(i); b++) {
    uint8_t byte = get_byte(i, b);
    if (byte) {
      for (n = 0; n < 8; n++) {
        if (byte & (1 << n))
          return n + (b << 3);
      }
    }
  }

  return 0;
}
#endif

struct task_steps_pool *task_steps_pool_create(struct task_step *steps,
                                               int count) {
  int i;
  struct task_steps_pool *pool;
  int size = sizeof(*pool) + sizeof(struct task_step) * count;

  pool = (struct task_steps_pool *)vpx_calloc(1, size);
  if (!pool) {
    return NULL;
  }

  pool->steps = (struct task_step *)(pool + 1);

  pool->steps_count = count;

  // Copy steps
  for (i = 0; i < count; i++) {
    pool->steps[i] = steps[i];
    pool->steps[i].pool = pool;
  }

  return pool;
}

void task_steps_pool_delete(struct task_steps_pool *pool) {
  vpx_free(pool);
}

void task_step_for_each_prev(struct task_step *step,
                             int (*fn)(struct task_step *step, void *args),
                             void *args) {
  int i;
  int map = step->prev_steps_map;
  for (i = 0; i < step->prev_count; i++) {
    int nr = ffs(map);
    if (nr == 0)
      return;
    map &= ~(1 << (nr - 1));
    fn(step->pool->steps + nr - 1, args);
  }
}

void task_step_for_each_next(struct task_step *step,
                             int (*fn)(struct task_step *step, void *args),
                             void *args) {
  int i;
  int map = step->next_steps_map;

  for (i = 0; i < step->next_count; i++) {
    int nr = ffs(map);
    if (nr == 0)
      return;
    map &= ~(1 << (nr - 1));
    fn(step->pool->steps + nr - 1, args);
  }
}
