/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_HEADER_H_
#define VP9_COMMON_VP9_HEADER_H_

/* 24 bits total */
typedef struct {
  unsigned int type: 1;
  unsigned int version: 3;
  unsigned int show_frame: 1;

  /* Allow 2^20 bytes = 8 megabits for first partition */

  unsigned int first_partition_length_in_bytes: 19;

#ifdef PACKET_TESTING
  unsigned int frame_number;
  unsigned int update_gold: 1;
  unsigned int uses_gold: 1;
  unsigned int update_last: 1;
  unsigned int uses_last: 1;
#endif

} VP9_HEADER;

#ifdef PACKET_TESTING
#define VP9_HEADER_SIZE 8
#else
#define VP9_HEADER_SIZE 3
#endif

#endif  // VP9_COMMON_VP9_HEADER_H_
