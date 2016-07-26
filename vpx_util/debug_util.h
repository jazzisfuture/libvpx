/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#ifndef VPX_UTIL_DEBUG_UTIL_H_
#define VPX_UTIL_DEBUG_UTIL_H_

#include <stdio.h>
#include "./vpx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_BITSTREAM_DEBUG
/* This is a debug tool used to detect bitstream error. On encoder side, it push
 * each bit and probability into a queue before the bit is written into the
 * Arithmetic coder.  On decoder side, whenever a bit is read out from the
 * Arithmetic coder, it pop the reference bit and probability from the queue as
 * well. If the two results do not match, this debug tood will report an error.
 * This tool can be pin down the bitstream error precisely.  By combining gdb's
 * backtrace method, we can detect which module causes the bitstream error. */
int bitstream_queue_get_write();
int bitstream_queue_get_read();
void bitstream_queue_record_write();
void bitstream_queue_reset_write();
void bitstream_queue_pop(int* result, int* prob);
void bitstream_queue_push(int result, int prob);
void bitstream_queue_skip_write_start();
void bitstream_queue_skip_write_end();
void bitstream_queue_skip_read_start();
void bitstream_queue_skip_read_end();
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_UTIL_DEBUG_UTIL_H_
