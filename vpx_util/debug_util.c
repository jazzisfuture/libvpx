/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <stdio.h>
#include "vpx_util/debug_util.h"
#if CONFIG_BITSTREAM_DEBUG
#define QUEUE_MAX_SIZE 2000000;
static int result_queue[QUEUE_MAX_SIZE];
static int prob_queue[QUEUE_MAX_SIZE];
static int queue_r = 0;
static int queue_w = 0;
static int queue_prev_w = -1;
static int skip_r = 0;
static int skip_w = 0;

void bitstream_queue_skip_write_start() {
  skip_w = 1;
}
void bitstream_queue_skip_write_end() {
  skip_w = 0;
}

void bitstream_queue_skip_read_start() {
  skip_r = 1;
}

void bitstream_queue_skip_read_end() {
  skip_r = 0;
}

void bitstream_queue_record_write() {
  queue_prev_w = queue_w;
}

void bitstream_queue_reset_write() {
  queue_w = queue_prev_w;
}

int bitstream_queue_get_write() {
  return queue_w;
}

int bitstream_queue_get_read() {
  return queue_r;
}

void bitstream_queue_pop(int* result, int* prob) {
  if (!skip_r) {
    if (queue_w == queue_r) {
      printf("buffer underflow queue_w %d queue_r %d\n", queue_w, queue_r);
      assert(0);
    }
    *result = result_queue[queue_r];
    *prob = prob_queue[queue_r];
    queue_r = (queue_r + 1) % QUEUE_MAX_SIZE;
  }
}

void bitstream_queue_push(int result, int prob) {
  if (!skip_w) {
    result_queue[queue_w] = result;
    prob_queue[queue_w] = prob;
    queue_w = (queue_w + 1) % QUEUE_MAX_SIZE;
    if (queue_w == queue_r) {
      printf("buffer overflow queue_w %d queue_r %d\n", queue_w, queue_r);
      assert(0);
    }
  }
}
#endif
