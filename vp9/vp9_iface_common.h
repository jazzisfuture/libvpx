/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef VP9_VP9_IFACE_COMMON_H_
#define VP9_VP9_IFACE_COMMON_H_

#include "vpx/vpx_codec.h"
#include "vpx/vpx_image.h"
#include "vpx_scale/yv12config.h"

void vp9_yuvconfig2image(vpx_image_t *img, const YV12_BUFFER_CONFIG *yv12,
                         void *user_priv);

vpx_codec_err_t vp9_image2yuvconfig(const vpx_image_t *img,
                                    YV12_BUFFER_CONFIG *yv12);

#endif  // VP9_VP9_IFACE_COMMON_H_
