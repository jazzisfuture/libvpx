/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <stdio.h>

#include "./bitwriter.h"

int in_stop_encode = 0;

void vpx_start_encode(vpx_writer *br, uint8_t *source) {
  br->lowvalue = 0;
  br->range    = 255;
  br->count    = -24;
  br->buffer   = source;
  br->pos      = 0;
  vpx_write_bit(br, 0);
}

void vpx_stop_encode(vpx_writer *br) {
  int i;

  in_stop_encode = 1;
  for (i = 0; i < 32; i++)
    vpx_write_bit(br, 0);

  in_stop_encode = 0;
  // Ensure there's no ambigous collision with any index marker bytes
  if ((br->buffer[br->pos - 1] & 0xe0) == 0xc0)
    br->buffer[br->pos++] = 0;
}

#define CABAC_CHECK_BUF_SIZE  (1024*1024)

unsigned int cabac_check_stop_at = 0;
unsigned int cabac_check_wcount = 0;
int cabac_check_ridx = 0;
int cabac_check_widx = 0;
int cabac_check_prob[CABAC_CHECK_BUF_SIZE];
int cabac_check_range[CABAC_CHECK_BUF_SIZE];
int cabac_check_bit[CABAC_CHECK_BUF_SIZE];

void log_write_arith(int probability, unsigned int range, int bit)
{
  if (cabac_check_wcount == cabac_check_stop_at && cabac_check_stop_at) {
    printf("CABAC write stop point. Sequence number: %d\n",
           cabac_check_wcount);
    printf("  probability:  %3d\n", probability);
    printf("        range:  %3d\n", range);
    printf("          bit:  %3d\n", bit);
  }

  cabac_check_prob[cabac_check_widx] = probability;
  cabac_check_range[cabac_check_widx] = range;
  cabac_check_bit[cabac_check_widx] = bit;

  cabac_check_widx = (cabac_check_widx + 1) % CABAC_CHECK_BUF_SIZE;

  assert(cabac_check_widx != cabac_check_ridx &&
         "Increase CABAC_CHECK_BUF_SIZE");

  cabac_check_wcount++;
}
