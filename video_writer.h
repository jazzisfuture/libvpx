/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_WRITER_H_
#define VIDEO_WRITER_H_

#include "./video_common.h"

#if CONFIG_ENCODERS
#include "vpx/vpx_encoder.h"
#endif

typedef enum {
  kContainerIVF
} VpxContainer;

struct VpxVideoWriterStruct;
typedef struct VpxVideoWriterStruct VpxVideoWriter;

#ifdef __cplusplus
extern "C" {
#endif

// Finds and opens writer for specified container format.
// Returns an opaque VpxVideoWriter* upon success, or NULL upon failure.
// Right now only IVF format is supported.
VpxVideoWriter *vpx_video_writer_open(const char *filename,
                                      VpxContainer container,
                                      const VpxVideoInfo *info);

// Frees all resources associated with VpxVideoWriter* returned from
// vpx_video_writer_open() call.
void vpx_video_writer_close(VpxVideoWriter *writer);

// Writes frame bytes to the file.
int vpx_video_writer_write_frame(VpxVideoWriter *writer,
                                 const uint8_t *buffer, size_t size,
                                 int64_t pts);

#if CONFIG_ENCODERS
// Passes |image| to libvpx for encoding, extracts compressed frames from the
// library, and passes them to vpx_video_writer_write_frame(). Calls
// die_codec() upon failure.
void vpx_video_writer_encode_frame(VpxVideoWriter *writer,
                                   vpx_codec_ctx_t *codec_context,
                                   const vpx_image_t *image,
                                   int64_t timestamp,
                                   int64_t duration,
                                   vpx_enc_frame_flags_t flags,
                                   unsigned int deadline);
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VIDEO_WRITER_H_
