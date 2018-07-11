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
#include <algorithm>

#include "test/bench.h"
#include "vpx_ports/vpx_timer.h"

void AbstractBench::RunNTimes(int n) {
  use_ref = 0;
  for (int r = 0; r < VPX_BENCH_ROBUST_ITER; r++) {
    vpx_usec_timer timer;
    vpx_usec_timer_start(&timer);
    for (int j = 0; j < n; ++j) {
      Run();
    }
    vpx_usec_timer_mark(&timer);
    times_[r] = static_cast<int>(vpx_usec_timer_elapsed(&timer));
  }
}

void AbstractBench::RunNTimesWithRef(int n) {
  use_ref = 1;
  for (int r = 0; r < VPX_BENCH_ROBUST_ITER; r++) {
    vpx_usec_timer timer;
    vpx_usec_timer_start(&timer);
    for (int j = 0; j < n; ++j) {
      RunRef();
    }
    vpx_usec_timer_mark(&timer);
    ref_times_[r] = static_cast<int>(vpx_usec_timer_elapsed(&timer));

    vpx_usec_timer_start(&timer);
    for (int j = 0; j < n; ++j) {
      Run();
    }
    vpx_usec_timer_mark(&timer);
    times_[r] = static_cast<int>(vpx_usec_timer_elapsed(&timer));
  }
}

static INLINE int median(int *times) {
  std::sort(times, times + VPX_BENCH_ROBUST_ITER);
  return times[VPX_BENCH_ROBUST_ITER >> 1];
}

static INLINE int sumAbsDiff(int *times, int med) {
  int sad = 0;
  for (int t = 0; t < VPX_BENCH_ROBUST_ITER; t++) {
    sad += abs(times[t] - med);
  }
  return sad;
}

static INLINE void print(const char *title, int med, int sad) {
  printf("[%10s] %s %.1f ms ( ±%.1f ms )\n", "BENCH ", title, med / 1000.0,
         sad / (VPX_BENCH_ROBUST_ITER * 1000.0));
}

static INLINE void printWithRef(const char *title, int med, int sad,
                                int ref_med, int ref_sad) {
  printf("[%10s] Ref %s %.1f ms ( ±%.1f ms ) %.1f ms ( ±%.1f ms ) %.1fx\n",
         "BENCH ", title, ref_med / 1000.0,
         ref_sad / (VPX_BENCH_ROBUST_ITER * 1000.0), med / 1000.0,
         sad / (VPX_BENCH_ROBUST_ITER * 1000.0), ref_med / (double)med);
}

void AbstractBench::PrintMedian(const char *title) {
  const int med = median(times_);
  const int sad = sumAbsDiff(times_, med);

  if (use_ref) {
    const int ref_med = median(ref_times_);
    const int ref_sad = sumAbsDiff(ref_times_, ref_med);
    printWithRef(title, med, sad, ref_med, ref_sad);
  } else {
    print(title, med, sad);
  }
}
