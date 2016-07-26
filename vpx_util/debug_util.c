#include <assert.h>
#include <stdio.h>
#include "vpx_util/debug_util.h"
#if CONFIG_BITSTREAM_DEBUG
static int result_queue[2000000];
static int prob_queue[2000000];
static int queue_max_size = 2000000;
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
  if(!skip_r) {
    if (queue_w == queue_r) {
      printf("buffer underflow queue_w %d queue_r %d\n", queue_w, queue_r);
      assert(0);
    }
    *result = result_queue[queue_r];
    *prob = prob_queue[queue_r];
    queue_r = (queue_r + 1) % queue_max_size;
  }
}

void bitstream_queue_push(int result, int prob) {
  if(!skip_w) {
    result_queue[queue_w] = result;
    prob_queue[queue_w] = prob;
    queue_w = (queue_w + 1) % queue_max_size;
    if (queue_w == queue_r) {
      printf("buffer overflow queue_w %d queue_r %d\n", queue_w, queue_r);
      assert(0);
    }
  }
}
#endif
