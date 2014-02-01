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

typedef enum {
  IVF_CONTAINER
} VpxContainerFormat;

struct VpxVideoWriterStruct;
typedef struct VpxVideoWriterStruct VpxVideoWriter;

VpxVideoWriter *vpx_video_writer_open(FILE *file, VpxContainerFormat format,
                                      const VpxVideoInfo *info);

void vpx_video_writer_write_frame(VpxVideoWriter *writer,
                                  const unsigned char *buffer, size_t size);

void vpx_video_writer_close(VpxVideoWriter *writer);

#endif  // VIDEO_WRITER_H_

