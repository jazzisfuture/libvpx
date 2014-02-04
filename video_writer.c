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
  cfg.g_timebase.den = 1;
  cfg.g_timebase.num = 1;

  ivf_write_file_header(file, &cfg, info->codec_fourcc, frame_count);
}

VpxVideoWriter *vpx_video_writer_open(const char *filename,
                                      VpxContainerFormat format,
                                      const VpxVideoInfo *info) {
  if (format == IVF_CONTAINER) {
    VpxVideoWriter *const writer = malloc(sizeof(*writer));
    writer->info = *info;
    writer->frame_count = 0;
    writer->file = fopen(filename, "wb");
    if (!writer->file)
      return NULL;

    write_header(writer->file, info, 0);

    return writer;
  }

  return NULL;
}

int vpx_video_writer_write_frame(VpxVideoWriter *writer,
                                 const unsigned char *buffer, size_t size) {
  ivf_write_frame_header(writer->file, 0, size);
  if (fwrite(buffer, 1, size, writer->file) != size)
    return 0;

  ++writer->frame_count;

  return 1;
}

void vpx_video_writer_close(VpxVideoWriter *writer) {
  if (writer) {
    rewind(writer->file);
    write_header(writer->file, &writer->info, writer->frame_count);

    fclose(writer->file);
    free(writer);
  }
}
