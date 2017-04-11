#include <stdint.h>
#include <stdio.h>

#include "vpx_timer.h"

int main(int argc, char **argv) {
  uint8_t src[32 * 32], dst[32 * 32];
  // border of 8 pixels. offset in:
  uint8_t *src_ptr = src + 8 + 8 * 32, *dst_ptr = dst + 8 + 8 * 32;

  int i;
  int elapsed_time;
  struct vpx_usec_timer timer;

  vpx_usec_timer_start(&timer);

  for (i = 0; i < 100000; ++i) {
    convolve(src_ptr, 32, dst_ptr, 32);
  }

  vpx_usec_timer_mark(&timer);
  elapsed_time = (int)(vpx_usec_timer_elapsed(&timer) / 1000);
  printf("two pass 8 tap time: %d\n", elapsed_time);


  vpx_usec_timer_start(&timer);

  for (i = 0; i < 100000; ++i) {
    box_filter(src_ptr, 32, dst_ptr, 32);
  }

  vpx_usec_timer_mark(&timer);
  elapsed_time = (int)(vpx_usec_timer_elapsed(&timer) / 1000);
  printf("box time: %d\n", elapsed_time);
}
