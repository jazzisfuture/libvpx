/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_PORTS_VPX_TIMER_H_
#define VPX_PORTS_VPX_TIMER_H_

#include <stdint.h>

/*
 * POSIX specific includes
 */
#include <sys/time.h>

/* timersub is not provided by msys at this time. */
#ifndef timersub
#define timersub(a, b, result)                       \
  do {                                               \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) {                     \
      --(result)->tv_sec;                            \
      (result)->tv_usec += 1000000;                  \
    }                                                \
  } while (0)
#endif

struct vpx_usec_timer {
  struct timeval begin, end;
};

static void vpx_usec_timer_start(struct vpx_usec_timer *t) {
  gettimeofday(&t->begin, 0);
}

static void vpx_usec_timer_mark(struct vpx_usec_timer *t) {
  gettimeofday(&t->end, 0);
}

static int64_t vpx_usec_timer_elapsed(struct vpx_usec_timer *t) {
  struct timeval diff;

  timersub(&t->end, &t->begin, &diff);
  return (int64_t)diff.tv_sec * 1000000 + diff.tv_usec;
}

#endif  // VPX_PORTS_VPX_TIMER_H_
