/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP8_ENCODER_SKIN_DETECTION_H_
#define VP8_ENCODER_SKIN_DETECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

int is_skin_color(int y, int cb, int cr, int consec_zeromv);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP8_ENCODER_SKIN_DETECTION_H_