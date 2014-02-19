/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

#include "./ivfenc.h"
#include "./video_writer.h"
#include "vpx/vpx_encoder.h"

struct VpxVideoWriterStruct {
  VpxVideoInfo info;
  FILE *file;
  int frame_count;
};

static void write_header(FILE *file, const VpxVideoInfo *info,
                         int frame_count) {
  struct vpx_codec_enc_cfg cfg;
  cfg.g_w = info->frame_width;
  cfg.g_h = info->frame_height;
  cfg.g_timebase.num = info->time_base.numerator;
  cfg.g_timebase.den = info->time_base.denominator;

  ivf_write_file_header(file, &cfg, info->codec_fourcc, frame_count);
}

VpxVideoWriter *vpx_video_writer_open(const char *filename,
                                      VpxContainer container,
                                      const VpxVideoInfo *info) {
  if (container == kContainerIVF) {
    VpxVideoWriter *writer = NULL;
    FILE *const file = fopen(filename, "wb");
    if (!file)
      return NULL;

    writer = malloc(sizeof(*writer));
    if (!writer)
      return NULL;

    writer->frame_count = 0;
    writer->info = *info;
    writer->file = file;

    write_header(writer->file, info, 0);

    return writer;
  }

  return NULL;
}

void vpx_video_writer_close(VpxVideoWriter *writer) {
  if (writer) {
    // Rewriting frame header with real frame count
    rewind(writer->file);
    write_header(writer->file, &writer->info, writer->frame_count);

    fclose(writer->file);
    free(writer);
  }
}

int vpx_video_writer_write_frame(VpxVideoWriter *writer,
                                 const uint8_t *buffer, size_t size,
                                 int64_t pts) {
  ivf_write_frame_header(writer->file, pts, size);
  if (fwrite(buffer, 1, size, writer->file) != size)
    return 0;

  ++writer->frame_count;

  return 1;
}

#if CONFIG_ENCODERS
void vpx_video_writer_encode_frame(VpxVideoWriter *writer,
                                   vpx_codec_ctx_t *codec_context,
                                   const vpx_image_t *image,
                                   int64_t timestamp,
                                   int64_t duration,
                                   vpx_enc_frame_flags_t flags,
                                   unsigned int deadline) {
  vpx_codec_iter_t iter = NULL;
  const vpx_codec_cx_pkt_t *packet = NULL;
  const vpx_codec_err_t res = vpx_codec_encode(codec_context,
                                               image,
                                               timestamp,
                                               (unsigned int)duration,
                                               flags,
                                               deadline);
  if (res != VPX_CODEC_OK)
    die_codec(codec_context, "Failed to encode frame");

  while ((packet = vpx_codec_get_cx_data(codec_context, &iter)) != NULL) {
    if (packet->kind == VPX_CODEC_CX_FRAME_PKT) {
      if (!vpx_video_writer_write_frame(writer,
                                        packet->data.frame.buf,
                                        packet->data.frame.sz,
                                        packet->data.frame.pts)) {
        die_codec(codec_context, "Failed to write compressed frame");
      }
    }
  }
}
#endif
