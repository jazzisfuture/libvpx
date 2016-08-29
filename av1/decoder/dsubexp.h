/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef AV1_DECODER_DSUBEXP_H_
#define AV1_DECODER_DSUBEXP_H_

#include "av1/decoder/bitreader.h"

#ifdef __cplusplus
extern "C" {
#endif

void av1_diff_update_prob(av1_reader *r, aom_prob *p);

#ifdef __cplusplus
}  // extern "C"
#endif

// mag_bits is number of bits for magnitude. The alphabet is of size
// 2 * 2^mag_bits + 1, symmetric around 0, where one bit is used to
// indicate 0 or non-zero, mag_bits bits are used to indicate magnitide
// and 1 more bit for the sign if non-zero.
int av1_read_primitive_symmetric(av1_reader *r, unsigned int mag_bits);
#endif  // AV1_DECODER_DSUBEXP_H_
