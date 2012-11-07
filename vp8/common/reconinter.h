<<<<<<< HEAD   (82b1a3 Merge other top-level C code)
=======
/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef __INC_RECONINTER_H
#define __INC_RECONINTER_H

extern void vp8_build_inter_predictors_mb(MACROBLOCKD *x);
extern void vp8_build_inter16x16_predictors_mb(MACROBLOCKD *x,
                                               unsigned char *dst_y,
                                               unsigned char *dst_u,
                                               unsigned char *dst_v,
                                               int dst_ystride,
                                               int dst_uvstride);


extern void vp8_build_inter16x16_predictors_mby(MACROBLOCKD *x,
                                                unsigned char *dst_y,
                                                int dst_ystride);
extern void vp8_build_inter_predictors_b(BLOCKD *d, int pitch,
                                         unsigned char *base_pre,
                                         int pre_stride,
                                         vp8_subpix_fn_t sppf);

extern void vp8_build_inter16x16_predictors_mbuv(MACROBLOCKD *x);
extern void vp8_build_inter4x4_predictors_mbuv(MACROBLOCKD *x);

#endif
>>>>>>> BRANCH (3c8007 Merge "ads2gas.pl: various enhancements to work with flash.")
