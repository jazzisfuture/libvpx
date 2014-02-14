/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Decode With Partial Drops Example
// =========================
//
// This is an example utility which drops a series of frames (or parts of
// frames), as specified on the command line. This is useful for observing the
// error recovery features of the codec.
//
// Usage
// -----
// This example adds a single argument to the `simple_decoder` example,
// which specifies the range or pattern of frames to drop. The parameter is
// parsed as follows.
//
// Dropping A Range Of Frames
// --------------------------
// To drop a range of frames, specify the starting frame and the ending
// frame to drop, separated by a dash. The following command will drop
// frames 5 through 10 (base 1).
//
//  $ ./decode_with_partial_drops in.ivf out.i420 5-10
//
//
// Dropping A Pattern Of Frames
// ----------------------------
// To drop a pattern of frames, specify the number of frames to drop and
// the number of frames after which to repeat the pattern, separated by
// a forward-slash. The following command will drop 3 of 7 frames.
// Specifically, it will decode 4 frames, then drop 3 frames, and then
// repeat.
//
//  $ ./decode_with_partial_drops in.ivf out.i420 3/7
//
// Dropping Random Parts Of Frames
// -------------------------------
// A third argument tuple is available to split the frame into 1500 bytes pieces
// and randomly drop pieces rather than frames. The frame will be split at
// partition boundaries where possible. The following example will seed the RNG
// with the seed 123 and drop approximately 5% of the pieces. Pieces which
// are depending on an already dropped piece will also be dropped.
//
//  $ ./decode_with_partial_drops in.ivf out.i420 5,123
//
// Extra Variables
// ---------------
// This example maintains the pattern passed on the command line in the
// `n`, `m`, and `is_range` variables:
//
// Making The Drop Decision
// ------------------------
// The example decides whether to drop the frame based on the current
// frame number, immediately before decoding the frame.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VPX_CODEC_DISABLE_COMPAT 1
#include "./vpx_config.h"
#include "vpx/vp8dx.h"
#include "vpx/vpx_decoder.h"
#include "./tools_common.h"
#include "./video_reader.h"

static const char *exec_name;

void usage_exit() {
  fprintf(stderr, "Usage: %s <infile> <outfile> <num-threads> <N-M|N/M|L,S>\n",
          exec_name);
  exit(EXIT_FAILURE);
}

enum drop_mode {
  DROP_FRAME_RANGE,
  DROP_FRAME_PATTERN,
  DROP_RANDOM_PACKETS
};

struct parsed_header {
  char key_frame;
  int version;
  char show_frame;
  int first_partition_size;
};

static int next_packet(const struct parsed_header *header, int pos, int length,
                       int mtu) {
  // Uncompressed part is 3 bytes for P frames and 10 bytes for I frames
  const int uncompressed_size = header->key_frame ? 10 : 3;
  // Number of bytes yet to send from the whole frame
  const int remaining = length - pos;
  // Number of bytes yet to send from header and the first partition
  const int remaining_first = uncompressed_size + header->first_partition_size -
                                  pos;
  if (remaining_first > 0)
    return remaining_first <= mtu ? remaining_first : mtu;

  return remaining <= mtu ? remaining : mtu;
}

size_t throw_packets(uint8_t *frame, size_t frame_size, int loss_rate) {
  struct parsed_header header;
  const int mtu = 1500;
  const unsigned int tmp = (frame[2] << 16) | (frame[1] << 8) | frame[0];

  if (frame_size < 3)
    return 0;

  putc('|', stdout);

  // Parse uncompressed 3 bytes
  header.key_frame = !(tmp & 0x1);
  header.version = (tmp >> 1) & 0x7;
  header.show_frame = (tmp >> 4) & 0x1;
  header.first_partition_size = (tmp >> 5) & 0x7FFFF;

  // Don't drop key frames
  if (header.key_frame) {
    int i;
    const int kept = frame_size / mtu + ((frame_size % mtu) > 0 ? 1 : 0);
    for (i = 0; i < kept; ++i)
      putc('.', stdout);  // per packet
    return 0;
  } else {
    unsigned char loss_frame[256 * 1024];
    int thrown = 0;
    int pkg_size = 0;
    int pos = 0;
    int loss_pos = 0;
    while ((pkg_size = next_packet(&header, pos, frame_size, mtu)) > 0) {
      const int loss = (rand() + 1.0) / (RAND_MAX + 1.0) < loss_rate / 100.0;
      if (thrown == 0 && !loss) {
        memcpy(loss_frame + loss_pos, frame + pos, pkg_size);
        loss_pos += pkg_size;
        putc('.', stdout);
      } else {
        ++thrown;
        putc('X', stdout);
      }
      pos += pkg_size;
    }
    memcpy(frame, loss_frame, loss_pos);
    memset(frame + loss_pos, 0, frame_size - loss_pos);

    return loss_pos;
  }
}

static int is_delimeter(char p) {
  return p == '-' || p  == '/' || p == ',';
}

static enum drop_mode get_mode(char p) {
  switch (p) {
    case '-': return DROP_FRAME_RANGE;
    case '/': return DROP_FRAME_PATTERN;
    case ',': return DROP_RANDOM_PACKETS;
  }
  return -1;
}

int main(int argc, char **argv) {
  FILE *outfile = NULL;
  VpxVideoReader *reader = NULL;
  vpx_codec_ctx_t codec = {0};
  int frame_cnt = 0;
  vpx_codec_err_t res;
  int n, m, mode;
  unsigned int seed;
  vpx_codec_dec_cfg_t dec_cfg = {0};
  const VpxVideoInfo *info = NULL;
  const VpxInterface *decoder = NULL;
  char *p = NULL;

  exec_name = argv[0];

  if (argc != 5)
    die("Invalid number of arguments.");

  reader = vpx_video_reader_open(argv[1]);
  if (!reader)
    die("Failed to open %s for reading", argv[1]);

  if (!(outfile = fopen(argv[2], "wb")))
    die("Failed to open %s for writing", argv[2]);

  dec_cfg.threads = strtol(argv[3], NULL, 0);
  if (!dec_cfg.threads)
    die("Couldn't parse number of threads\n", argv[3]);
  printf("Threads: %d\n", dec_cfg.threads);

  n = strtol(argv[4], &p, 0);
  if (n == 0 || *p == '\0' || !is_delimeter(*p))
    die("Couldn't parse pattern %s\n", argv[4]);
  mode = get_mode(*p);
  m = strtol(p + 1, NULL, 0);
  if (m == 0)
    die("Couldn't parse pattern %s\n", argv[4]);

  seed = (m > 0) ? m : (unsigned int)time(NULL);
  srand(seed);
  printf("Seed: %u\n", seed);

  info = vpx_video_reader_get_info(reader);
  decoder = get_vpx_decoder_by_fourcc(info->codec_fourcc);
  printf("Using %s\n", vpx_codec_iface_name(decoder->interface()));

  res = vpx_codec_dec_init(&codec, decoder->interface(), &dec_cfg,
                           VPX_CODEC_USE_ERROR_CONCEALMENT);
  if (res)
    die_codec(&codec, "Failed to initialize decoder");

  while (vpx_video_reader_read_frame(reader)) {
    vpx_codec_iter_t iter = NULL;
    vpx_image_t *img = NULL;
    size_t frame_size = 0;
    const uint8_t *frame = vpx_video_reader_get_frame(reader, &frame_size);
    int drop_frame = 0;

    frame_cnt++;

    switch (mode) {
      case DROP_FRAME_PATTERN:
        printf("DROP_FRAME_PATTERN\n");
        drop_frame = !((m - (frame_cnt - 1) % m <= n));
        putc(drop_frame ? 'X' : '.', stdout);
        if (drop_frame)
          frame_size = 0;
        break;
      case DROP_FRAME_RANGE:
        printf("DROP_FRAME_RANGE\n");
        drop_frame = !(frame_cnt >= n && frame_cnt <= m);
        putc(drop_frame ? 'X' : '.', stdout);
        //if (drop_frame)
          //frame_size = 0;
        break;
      case DROP_RANDOM_PACKETS:
        printf("DROP_RANDOM_PACKETS\n");
        throw_packets((uint8_t *)frame, frame_size, n);
        break;
      default:
        break;
    }

    fflush(stdout);

    if (vpx_codec_decode(&codec, frame, frame_size, NULL, 0))
      die_codec(&codec, "Failed to decode frame");

    while ((img = vpx_codec_get_frame(&codec, &iter)) != NULL)
      vpx_img_write(img, outfile);
  }

  printf("Processed %d frames.\n", frame_cnt);
  if (vpx_codec_destroy(&codec))
    die_codec(&codec, "Failed to destroy codec");

  vpx_video_reader_close(reader);
  fclose(outfile);

  return EXIT_SUCCESS;
}
