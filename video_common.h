/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_COMMON_H_
#define VIDEO_COMMON_H_

typedef struct {
  unsigned int codec_fourcc;
  int frame_width;
  int frame_height;
  int frame_count;
} VpxVideoInfo;

#endif  // VIDEO_COMMON_H_

