#include "test/bench.h"

#include <algorithm>

void AbstractBench::runNTimes(int n) {
  for (int r = 0; r < VPX_BENCH_ROBUST_ITER; r++) {
    vpx_usec_timer timer;
    vpx_usec_timer_start(&timer);
    for (int j = 0; j < n; ++j) {
      run();
    }
    vpx_usec_timer_mark(&timer);
    times[r] = static_cast<int>(vpx_usec_timer_elapsed(&timer));
  }
}

void AbstractBench::printMedian(const char *title) {
  std::sort(times, times + VPX_BENCH_ROBUST_ITER);
  const int med = times[VPX_BENCH_ROBUST_ITER >> 1];
  int sad = 0;
  for (int t = 0; t < VPX_BENCH_ROBUST_ITER; t++) {
    sad += abs(times[t] - med);
  }
  printf("[%10s] %s %.1f ms ( ±%.1f ms )\n", "BENCH ", title, med / 1000.0,
         sad / (VPX_BENCH_ROBUST_ITER * 1000.0));
}
