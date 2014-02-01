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
  FILE *file;
};

VpxVideoWriter *vpx_video_writer_open(FILE *file, VpxContainerFormat format,
                                      const VpxVideoInfo *info) {
  if (format == IVF_CONTAINER) {
    VpxVideoWriter *writer = malloc(sizeof(*writer));
    writer->file = file;

    struct vpx_codec_enc_cfg cfg;
    cfg.g_w = info->frame_width;
    cfg.g_h = info->frame_height;
    cfg.g_timebase.den = 1;
    cfg.g_timebase.num = 1;

    ivf_write_file_header(file, &cfg, info->codec_fourcc, info->frame_count);
    return writer;
  }

  return NULL;
}

void vpx_video_writer_write_frame(VpxVideoWriter *writer,
                                  const unsigned char *buffer, size_t size) {
  ivf_write_frame_header(writer->file, 0, size);
  fwrite(writer->file, buffer, 1, size);
}

void vpx_video_writer_close(VpxVideoWriter *writer) {
  if (writer) {
    fclose(writer->file);
    free(writer);
  }
}
