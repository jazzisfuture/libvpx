/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * Fuzzer for libvpx decoders
 * ==========================
 * Requirements
 * --------------
 * Requires Clang 6.0 or above as -fsanitize=fuzzer is used as a linker
 * option.

 * Steps to build
 * --------------
 * Clone libvpx repository
   $git clone https://chromium.googlesource.com/webm/libvpx

 * Create a directory in parallel to libvpx and change directory
   $mkdir vpx_dec_fuzzer
   $cd vpx_dec_fuzzer/

 * Configure libvpx.
 * Note --size-limit and VPX_MAX_ALLOCABLE_MEMORY are defined to avoid
 * Out of memory errors when running generated fuzzer binary
    $../libvpx/configure --disable-unit-tests --size-limit=12288x12288 \
   --extra-cflags="-DVPX_MAX_ALLOCABLE_MEMORY=1073741824" \
   --disable-webm-io --enable-debug

 * Build libvpx
   $make -j32

 * Build vp9 threaded fuzzer
   $clang++ -std=c++11 -DDECODE_MODE_threaded -DENABLE_vp9 \
   -fsanitize=fuzzer -I../libvpx -I. -Wl,--start-group \
   ../libvpx/examples/vpx_dec_fuzzer.cc -o ./vpx_dec_fuzzer_threaded_vp9 \
   ./libvpx.a ./tools_common.c.o -Wl,--end-group

 * ENABLE_vp9 or ENABLE_vp8 needs to be defined to enable vp9/vp8
 * DECODE_MODE_threaded or DECODE_MODE_ needs to be defined to test
 * multi-threaded or single core implementation
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(DECODE_MODE_threaded)
#include <algorithm>
#endif
#include <memory>
#include "vpx_config.h"
#include "ivfdec.h"
#include "vpx/vpx_decoder.h"
#include "vpx_ports/mem_ops.h"
#include "tools_common.h"
#if CONFIG_VP8_DECODER || CONFIG_VP9_DECODER
#include "vpx/vp8dx.h"
#endif

static void close_file(FILE *file) { fclose(file); }

static int read_frame(FILE *infile, uint8_t **buffer, size_t *bytes_read,
                   size_t *buffer_size) {
  char raw_header[IVF_FRAME_HDR_SZ] = { 0 };
  size_t frame_size = 0;

  if (fread(raw_header, IVF_FRAME_HDR_SZ, 1, infile) == 1) {
    frame_size = mem_get_le32(raw_header);

    if (frame_size > 256 * 1024 * 1024) {
      frame_size = 0;
    }

    if (frame_size > *buffer_size) {
      uint8_t *new_buffer = (uint8_t *)realloc(*buffer, 2 * frame_size);

      if (new_buffer) {
        *buffer = new_buffer;
        *buffer_size = 2 * frame_size;
      } else {
        frame_size = 0;
      }
    }
  }

  if (!feof(infile)) {
    *bytes_read = fread(*buffer, 1, frame_size, infile);
    return 0;
  }

  return 1;
}
extern "C" void usage_exit(void) { exit(EXIT_FAILURE); }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::unique_ptr<FILE, decltype(&close_file)> file(
      fmemopen((void *)data, size, "rb"), &close_file);
  if (file == nullptr) {
    return 0;
  }
  // Ensure input contains at least one file header and one frame header
  if (size < IVF_FILE_HDR_SZ + IVF_FRAME_HDR_SZ) {
      return 0;
  }
  char header[IVF_FILE_HDR_SZ];
  if (fread(header, 1, IVF_FILE_HDR_SZ, file.get()) != IVF_FILE_HDR_SZ) {
    return 0;
  }
#ifdef ENABLE_vp8
  const VpxInterface *decoder = get_vpx_decoder_by_name("vp8");
#else
  const VpxInterface *decoder = get_vpx_decoder_by_name("vp9");
#endif
  if (decoder == nullptr) {
    return 0;
  }

  vpx_codec_ctx_t codec;
#if defined(DECODE_MODE)
  const unsigned int threads = 1;
#elif defined(DECODE_MODE_threaded)
  // Set thread count in the range [2, 64].
  const unsigned int threads = std::max((data[IVF_FILE_HDR_SZ] & 0x3f) + 1, 2);
#else
#error define one of DECODE_MODE or DECODE_MODE_threaded
#endif
  vpx_codec_dec_cfg_t cfg = {threads, 0, 0};
  if (vpx_codec_dec_init(&codec, decoder->codec_interface(), &cfg, 0)) {
    return 0;
  }

  uint8_t *buffer = nullptr;
  size_t buffer_size = 0;
  size_t frame_size = 0;

  while (!read_frame(file.get(), &buffer, &frame_size, &buffer_size)) {
    const vpx_codec_err_t err =
        vpx_codec_decode(&codec, buffer, frame_size, nullptr, 0);
    static_cast<void>(err);
    vpx_codec_iter_t iter = nullptr;
    vpx_image_t *img = nullptr;
    while ((img = vpx_codec_get_frame(&codec, &iter)) != nullptr) {
    }
  }
  vpx_codec_destroy(&codec);
  free(buffer);
  return 0;
}
