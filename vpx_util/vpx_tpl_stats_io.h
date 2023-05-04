/*
 *  Copyright (c) 2023 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_UTIL_VPX_WRITE_TPL_STATS_H_
#define VPX_VPX_UTIL_VPX_WRITE_TPL_STATS_H_

#include <stdio.h>
#include <stdlib.h>
#include "vpx/vpx_encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

vpx_codec_err_t vpx_write_tpl_stats(FILE *tpl_file,
                                    VpxTplGopStats *tpl_gop_stats);

vpx_codec_err_t vpx_read_tpl_stats(FILE *tpl_file,
                                   VpxTplGopStats *tpl_gop_stats);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VPX_UTIL_VPX_WRITE_TPL_STATS_H_
