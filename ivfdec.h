/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef IVFDEC_H_
#define IVFDEC_H_

#include "./tools_common.h"

#ifdef __cplusplus
extern "C" {
#endif

int file_is_ivf(struct VpxInputContext *input);

int ivf_read_frame(FILE *infile, uint8_t **buffer,
                   size_t *bytes_read, size_t *buffer_size);

struct vpx_video;
typedef struct vpx_video vpx_video_t;

vpx_video_t *vpx_video_open_file(FILE *file);
void vpx_video_close(vpx_video_t *video);

int vpx_video_get_width(vpx_video_t *video);
int vpx_video_get_height(vpx_video_t *video);
unsigned int vpx_video_get_fourcc(vpx_video_t *video);

int vpx_video_read_frame(vpx_video_t *);
const unsigned char *vpx_video_get_frame(vpx_video_t *video, size_t *size);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  // IVFDEC_H_
