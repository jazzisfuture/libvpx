/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_BENCH_H_
#define TEST_BENCH_H_

#include <math.h>
#include <string>
#include "vpx_ports/vpx_timer.h"

// Number of iterations used to compute median run time.
#define VPX_BENCH_ROBUST_ITER 15

static INLINE void vpx_print_median(
    const char *title, vpx_usec_timer timer[VPX_BENCH_ROBUST_ITER]) {
  int times[VPX_BENCH_ROBUST_ITER];
  for (int r = 0; r < VPX_BENCH_ROBUST_ITER; r++) {
    int time = static_cast<int>(vpx_usec_timer_elapsed(&timer[r]));
    int t;
    for (t = r; t > 0 && time < times[t - 1]; --t) {
      times[t] = times[t - 1];
    }
    times[t] = time;
  }
  const int med = times[VPX_BENCH_ROBUST_ITER >> 1];
  int sad = 0;
  for (int t = 0; t < VPX_BENCH_ROBUST_ITER; t++) {
    sad += abs(times[t] - med);
  }
  printf("[%10s] %s %.1f ms ( ±%.1f ms )\n", "BENCH ", title, med / 1000.0,
         sad / (VPX_BENCH_ROBUST_ITER * 1000.0));
}

#endif  // TEST_BENCH_H_
