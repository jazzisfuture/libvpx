/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Simple Encoder
// ==============
//
// This is an example of a simple encoder loop. It takes an input file in
// YV12 format, passes it through the encoder, and writes the compressed
// frames to disk in IVF format. Other decoder examples build upon this
// one.
//
// The details of the IVF format have been elided from this example for
// simplicity of presentation, as IVF files will not generally be used by
// your application. In general, an IVF file consists of a file header,
// followed by a variable number of frames. Each frame consists of a frame
// header followed by a variable length payload. The length of the payload
// is specified in the first four bytes of the frame header. The payload is
// the raw compressed data.
//
// Standard Includes
// -----------------
// For encoders, you only have to include `vpx_encoder.h` and then any
// header files for the specific codecs you use. In this case, we're using
// vp8. The `VPX_CODEC_DISABLE_COMPAT` macro can be defined to ensure
// strict compliance with the latest SDK by disabling some backwards
// compatibility features. Defining this macro is encouraged.
//
// Getting The Default Configuration
// ---------------------------------
// Encoders have the notion of "usage profiles." For example, an encoder
// may want to publish default configurations for both a video
// conferencing appliction and a best quality offline encoder. These
// obviously have very different default settings. Consult the
// documentation for your codec to see if it provides any default
// configurations. All codecs provide a default configuration, number 0,
// which is valid for material in the vacinity of QCIF/QVGA.
//
// Updating The Configuration
// ---------------------------------
// Almost all applications will want to update the default configuration
// with settings specific to their usage. Here we set the width and height
// of the video file to that specified on the command line. We also scale
// the default bitrate based on the ratio between the default resolution
// and the resolution specified on the command line.
//
// Initializing The Codec
// ----------------------
// The encoder is initialized by the following code.
//
// Encoding A Frame
// ----------------
// The frame is read as a continuous block (size width * height * 3 / 2)
// from the input file. If a frame was read (the input file has not hit
// EOF) then the frame is passed to the encoder. Otherwise, a NULL
// is passed, indicating the End-Of-Stream condition to the encoder. The
// `frame_cnt` is reused as the presentation time stamp (PTS) and each
// frame is shown for one frame-time in duration. The flags parameter is
// unused in this example. The deadline is set to VPX_DL_REALTIME to
// make the example run as quickly as possible.
//
// Processing The Encoded Data
// ---------------------------
// Each packet of type `VPX_CODEC_CX_FRAME_PKT` contains the encoded data
// for this frame. We write a IVF frame header, followed by the raw data.
//
// Cleanup
// -------
// The `vpx_codec_destroy` call frees any memory allocated by the codec.
//
// Error Handling
// --------------
// This example does not special case any error return codes. If there was
// an error, a descriptive message is printed and the program exits. With
// few exeptions, vpx_codec functions return an enumerated error status,
// with the value `0` indicating success.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VPX_CODEC_DISABLE_COMPAT 1

#include "vpx/vp8cx.h"
#include "vpx/vpx_encoder.h"

#include "./tools_common.h"
#include "./video_writer.h"

#define interface (vpx_codec_vp8_cx())

static const char *exec_name;

void usage_exit() {
  fprintf(stderr, "Usage: %s <width> <height> <infile> <outfile>\n", exec_name);
  exit(EXIT_FAILURE);
}

static int read_frame(FILE *f, vpx_image_t *img) {
  int res = 1;
  size_t to_read = img->w * img->h * 3 / 2;
  size_t nbytes = fread(img->planes[0], 1, to_read, f);
  if (nbytes != to_read) {
    res = 0;
    if (nbytes > 0)
      printf("Warning: Read partial frame. Check your width & height!\n");
  }
  return res;
}

int main(int argc, char **argv) {
  FILE *infile;
  vpx_codec_ctx_t codec;
  vpx_codec_enc_cfg_t cfg;
  int frame_cnt = 0;
  vpx_image_t raw;
  vpx_codec_err_t res;
  int width, height;
  int frame_avail;
  int got_data;
  VpxVideoInfo info;
  VpxVideoWriter *writer;
  exec_name = argv[0];

  if (argc != 5)
    die("Invalid number of arguments");

  width = strtol(argv[1], NULL, 0);
  height = strtol(argv[2], NULL, 0);
  if (width < 16 || width % 2 || height < 16 || height % 2)
    die("Invalid resolution: %dx%d", width, height);

  if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, width, height, 1))
    die("Failed to allocate image", width, height);

  printf("Using %s\n", vpx_codec_iface_name(interface));

  // Populate encoder configuration
  res = vpx_codec_enc_config_default(interface, &cfg, 0);
  if (res) {
    printf("Failed to get config: %s\n", vpx_codec_err_to_string(res));
    return EXIT_FAILURE;
  }

  // Update the default configuration with our settings
  cfg.rc_target_bitrate = width * height * cfg.rc_target_bitrate
                          / cfg.g_w / cfg.g_h;
  cfg.g_w = width;
  cfg.g_h = height;

  info.frame_width = width;
  info.frame_height = height;
  info.frame_count = 0;
  info.codec_fourcc = VP8_FOURCC;
  writer = vpx_video_writer_open(argv[4], IVF_CONTAINER, &info);
  if (!writer)
    die("Failed to open %s for writing", argv[4]);

  // Open input file for this encoding pass
  if (!(infile = fopen(argv[3], "rb")))
    die("Failed to open %s for reading", argv[3]);

  // Initialize codec
  if (vpx_codec_enc_init(&codec, interface, &cfg, 0))
    die_codec(&codec, "Failed to initialize encoder");

  frame_avail = 1;
  got_data = 0;
  while (frame_avail || got_data) {
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t *pkt;

    frame_avail = read_frame(infile, &raw);
    if (vpx_codec_encode(&codec, frame_avail? &raw : NULL, frame_cnt,
                         1, 0, VPX_DL_REALTIME))
      die_codec(&codec, "Failed to encode frame");
    got_data = 0;
    while ((pkt = vpx_codec_get_cx_data(&codec, &iter)) != NULL) {
      got_data = 1;
      if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
        vpx_video_writer_write_frame(writer, pkt->data.frame.buf,
                                     pkt->data.frame.sz);
        printf((pkt->data.frame.flags & VPX_FRAME_IS_KEY) ? "K" : ".");
      }
    }
    frame_cnt++;
  }
  printf("\n");

  fclose(infile);

  printf("Processed %d frames.\n", frame_cnt - 1);
  vpx_img_free(&raw);
  if (vpx_codec_destroy(&codec))
    die_codec(&codec, "Failed to destroy codec.");

  vpx_video_writer_close(writer, frame_cnt - 1);

  return EXIT_SUCCESS;
}
