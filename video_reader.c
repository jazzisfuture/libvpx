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
#include <string.h>

#include "./ivfdec.h"
#include "./video_reader.h"

static const char *const IVF_SIGNATURE = "DKIF";

struct VpxVideoReaderStruct {
  FILE *file;
  uint8_t *buffer;
  size_t buffer_size;
  size_t frame_size;
  uint32_t codec_fourcc;
  int width;
  int height;
};

VpxVideoReader *vpx_video_reader_open(const char *filename) {
  char header[32];
  VpxVideoReader *reader;
  FILE *const file = fopen(filename, "rb");
  if (!file)
    return NULL;  // Can't open file

  if (fread(header, 1, 32, file) != 32)
    return NULL;  // Can't read file header

  if (memcmp(IVF_SIGNATURE, header, 4) != 0)
    return NULL;  // Wrong IVF signature

  if (mem_get_le16(header + 4) != 0)
    return NULL;  // Wrong IVF version

  reader = malloc(sizeof(*reader));
  if (!reader)
    return NULL;  // Can't allocate VpxVideoReader

  reader->file = file;
  reader->buffer = NULL;
  reader->buffer_size = 0;
  reader->frame_size = 0;
  reader->codec_fourcc = mem_get_le32(header + 8);
  reader->width = mem_get_le16(header + 12);
  reader->height = mem_get_le16(header + 14);

  return reader;
}

void vpx_video_reader_close(VpxVideoReader *reader) {
  if (reader) {
    fclose(reader->file);
    free(reader->buffer);
    free(reader);
  }
}

int vpx_video_reader_read_frame(VpxVideoReader *reader) {
  return !ivf_read_frame(reader->file, &reader->buffer, &reader->frame_size,
                         &reader->buffer_size);
}

const uint8_t *vpx_video_reader_get_frame(VpxVideoReader *reader,
                                          size_t *size) {
  if (size)
    *size = reader->frame_size;

  return reader->buffer;
}

void vpx_video_reader_get_info(VpxVideoReader *reader, VpxVideoInfo *info) {
  info->frame_width = reader->width;
  info->frame_height = reader->height;
  info->codec_fourcc = reader->codec_fourcc;
}
