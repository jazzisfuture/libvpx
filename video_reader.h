/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_READER_H_
#define VIDEO_READER_H_

#include "./video_common.h"

// The following code is work in progress. It is going to  support transparent
// reading of IVF and Y4M formats. Right now only IVF format is supported for
// simplicity. The main goal the API is to be simple and easy to use in example
// code (and probably in vpxenc/vpxdec later). All low-level details like memory
// buffer management are hidden from API users.
struct VpxVideoReaderStruct;
typedef struct VpxVideoReaderStruct VpxVideoReader;

// Opens the input file and inspects it to determine file type. Returns an
// opaque vpx_video_t* upon success, or NULL upon failure.
VpxVideoReader *vpx_video_reader_open(const char *filename);

// Frees all resources associated with vpx_video_t returned from
// vpx_video_open_file() call
void vpx_video_reader_close(VpxVideoReader *reader);

// Reads video frame bytes from the file and stores them into internal buffer.
int vpx_video_reader_read_frame(VpxVideoReader *reader);

// Returns the pointer to internal memory buffer with frame bytes read from
// last call to vpx_video_read_frame().
const unsigned char *vpx_video_reader_get_frame(VpxVideoReader *reader,
                                                size_t *size);

void vpx_video_reader_get_info(VpxVideoReader *reader, VpxVideoInfo *info);

#endif  // VIDEO_READER_H_
