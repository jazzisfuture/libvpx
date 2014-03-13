/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_ENCODER_VP9_SVC_LAYERCONTEXT_H_
#define VP9_ENCODER_VP9_SVC_LAYERCONTEXT_H_

#include "vp9/encoder/vp9_onyx_int.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize layer context data from init_config().
void vp9_init_layer_context(VP9_COMP *const cpi);

// Update the layer context from a change_config() call.
void vp9_update_layer_context_change_config(VP9_COMP *const cpi,
                                            const int target_bandwidth);

// Prior to encoding the frame, update framerate-related quantities
// for the current layer.
void vp9_update_layer_framerate(VP9_COMP *const cpi);

// Prior to encoding the frame, set the layer context, for the current layer
// to be encoded, to the cpi struct.
void vp9_restore_layer_context(VP9_COMP *const cpi);

// Save the layer context after encoding the frame.
void vp9_save_layer_context(VP9_COMP *const cpi);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_SVC_LAYERCONTEXT_
