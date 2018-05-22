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
#include "vpx_ports/vpx_timer.h"

// Number of iterations used to compute median run time.
#define VPX_BENCH_ROBUST_ITER 15
#define VPX_BENCH_MEDIAN_IDX ((VPX_BENCH_ROBUST_ITER) >> 1)

static INLINE void vpx_print_median(
    std::string title, vpx_usec_timer timer[VPX_BENCH_ROBUST_ITER]) {
  int sorted_times[VPX_BENCH_ROBUST_ITER];

  for (int r = 0; r < VPX_BENCH_ROBUST_ITER; r++) {
    const int elapsed_time =
        static_cast<int>(vpx_usec_timer_elapsed(&timer[r]));

    int t;
    for (t = r; t > 0 && elapsed_time < sorted_times[t - 1]; --t) {
      sorted_times[t] = sorted_times[t - 1];
    }
    sorted_times[t] = elapsed_time;
  }

  const int median = sorted_times[VPX_BENCH_MEDIAN_IDX];
  int sse = 0;
  for (int r = 0; r < VPX_BENCH_ROBUST_ITER; r++) {
    const int d = sorted_times[r] - median;
    sse += d * d;
  }
  printf("[%10s] %s %.1f ms ( ±%.1f ms )\n", "BENCH ", title.c_str(),
         median / 1000.0, sqrt(sse / (VPX_BENCH_ROBUST_ITER * 1000000.0)));
}

#endif  // TEST_BENCH_H_
