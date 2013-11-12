/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "tools_common.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__OS2__)
#include <io.h>
#include <fcntl.h>

#ifdef __OS2__
#define _setmode    setmode
#define _fileno     fileno
#define _O_BINARY   O_BINARY
#endif
#endif

#define LOG_ERROR(label) do {\
  const char *l = label;\
  va_list ap;\
  va_start(ap, fmt);\
  if (l)\
    fprintf(stderr, "%s: ", l);\
  vfprintf(stderr, fmt, ap);\
  fprintf(stderr, "\n");\
  va_end(ap);\
} while (0)


FILE *set_binary_mode(FILE *stream) {
  (void)stream;
#if defined(_WIN32) || defined(__OS2__)
  _setmode(_fileno(stream), _O_BINARY);
#endif
  return stream;
}

void die(const char *fmt, ...) {
  LOG_ERROR(NULL);
  usage_exit();
}

void fatal(const char *fmt, ...) {
  LOG_ERROR("Fatal");
  exit(EXIT_FAILURE);
}

void warn(const char *fmt, ...) {
  LOG_ERROR("Warning");
}

unsigned int mem_get_le16(const void *data) {
  unsigned int  val;
  const unsigned char *mem = (const unsigned char *)data;

  val = mem[1] << 8;
  val |= mem[0];
  return val;
}

unsigned int mem_get_le32(const void *data) {
  unsigned int val;
  const unsigned char *mem = (const unsigned char *)data;

  val = mem[3] << 24;
  val |= mem[2] << 16;
  val |= mem[1] << 8;
  val |= mem[0];
  return val;
}

int read_yuv_frame(struct VpxInputContext *input_ctx, vpx_image_t *yuv_frame) {
  FILE *f = input_ctx->file;
  struct FileTypeDetectionBuffer *detect = &input_ctx->detect;
  int plane = 0;
  int shortread = 0;
  const enum VideoFileType file_type = input_ctx->file_type;

  if (file_type == FILE_TYPE_IVF) {
    char junk[IVF_FRAME_HDR_SZ];

    /* Skip the frame header. We know how big the frame should be. See
     * write_ivf_frame_header() for documentation on the frame header
     * layout.
     */
    if ((fread(junk, 1, IVF_FRAME_HDR_SZ, f) != 1)) {
      fatal("Unable to consume IVF header while reading YUV frame.");
    }
  }

  for (plane = 0; plane < 3; plane++) {
    unsigned char *ptr;
    int w = (plane ? (1 + yuv_frame->d_w) / 2 : yuv_frame->d_w);
    int h = (plane ? (1 + yuv_frame->d_h) / 2 : yuv_frame->d_h);
    int r;

    /* Determine the correct plane based on the image format. The for-loop
     * always counts in Y,U,V order, but this may not match the order of
     * the data on disk.
     */
    switch (plane) {
      case 1:
        ptr = yuv_frame->planes[
            yuv_frame->fmt == VPX_IMG_FMT_YV12 ? VPX_PLANE_V : VPX_PLANE_U];
        break;
      case 2:
        ptr = yuv_frame->planes[
            yuv_frame->fmt == VPX_IMG_FMT_YV12 ? VPX_PLANE_U : VPX_PLANE_V];
        break;
      default:
        ptr = yuv_frame->planes[plane];
    }

    for (r = 0; r < h; r++) {
      size_t needed = w;
      size_t buf_position = 0;
      const size_t left = detect->buf_read - detect->position;
      if (left > 0) {
        const size_t more = (left < needed) ? left : needed;
        memcpy(ptr, detect->buf + detect->position, more);
        buf_position = more;
        needed -= more;
        detect->position += more;
      }
      if (needed > 0) {
        shortread |= (fread(ptr + buf_position, 1, needed, f) < needed);
      }

      ptr += yuv_frame->stride[plane];
    }
  }

  return shortread;
}
