/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_EXTERNAL_FRAME_BUFFER_H
#define VPX_EXTERNAL_FRAME_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vpx/vpx_integer.h"

/*!\brief Stream properties
 *
 * This structure is used to hold external frame buffers passed into the
 * decoder by the application.
 */
typedef struct vpx_codec_frame_buffer {
  unsigned char* data;    /**< Pointer to the data buffer */
  unsigned int size;      /**< Size of data in bytes */
} vpx_codec_frame_buffer_t;

/*!\brief realloc frame buffer callback prototype
 *
 * This callback is invoked by the decoder to notify the application one
 * of the external frame buffers must increase the size, in order for the
 * decode call to complete. This is usually triggered by a frame size
 * change.
 *
 * \param[in] user_priv    User's private data
 * \param[in] new_size     Size in bytes needed by the buffer.
 * \param[in/out] fb       Pointer to frame buffer to increase size.
 */
typedef int (*vpx_realloc_frame_buffer_cb_fn_t)(
    void *user_priv,
    int new_size,
    vpx_codec_frame_buffer_t *fb);

#ifdef __cplusplus
}
#endif

#endif  // VPX_EXTERNAL_FRAME_BUFFER_H
