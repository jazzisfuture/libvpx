/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef __INC_RECONINTRA_H
#define __INC_RECONINTRA_H

#include "blockd.h"

extern void vp9_recon_intra_mbuv(MACROBLOCKD *xd);
#if CONFIG_COMP_INTERINTRA_PRED
extern void vp9_build_interintra_16x16_predictors_mby(MACROBLOCKD *xd);
extern void vp9_build_interintra_16x16_predictors_mbuv(MACROBLOCKD *xd);
#endif

#endif  // __INC_RECONINTRA_H
