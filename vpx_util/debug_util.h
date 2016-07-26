/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
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
 * well. If the two results do not match, this debug tool will report an error.
 * This tool can be used to pin down the bitstream error precisely. By combining
 * gdb's backtrace method, we can detect which module causes the bitstream error. */
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
