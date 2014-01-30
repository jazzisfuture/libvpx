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

struct VpxVideoWriterStruct {
  FILE *file;
};

VpxVideoWriter *vpx_video_writer_open(FILE *file, VpxContainerFormat format,
                                      const VpxVideoInfo *info) {
  if (format == IVF_CONTAINER) {
    VpxVideoWriter writer = malloc(sizeof(*writer));
    writer->file = file;

    vpx_codec_enc_cfg cfg;

    ivf_write_file_header(file, &cfg, info->codec_fourcc, info->frame_count);
    return writer;
  }


  return NULL;
}

void vpx_video_writer_write_frame(VpxVideoWriter *writer,
                                  const unsigned char *buffer, size_t size) {
  vpx_codec_cx_pkt pkt;
  ivf_write_frame_header(writer->file, &pkt);
}

void vpx_video_writer_close(VpxVideoWriter *writer) {
  if (writer) {
    fclose(writer->file);
    free(writer);
  }
}
