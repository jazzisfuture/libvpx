#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./tools_common.h"
#include "./video_reader.h"
#include "./vpx_config.h"

#include "vpx/api_vpx_decoder.h"

static const char *exec_name;

typedef struct DecoderContext {
  FILE *file;
  int frames;
} DecoderContext;


static void frame_decoded(vpx_decoder *decoder, vpx_decoded_frame *frame) {
  DecoderContext *ctx = (DecoderContext *)vpx_decoder_get_priv_data(decoder);

  vpx_img_write(vpx_decoded_frame_get_image(frame), ctx->file);
  vpx_decoded_frame_done(frame);
  ++ctx->frames;
}

void usage_exit() {
  fprintf(stderr, "Usage: %s <infile> <outfile>\n", exec_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  DecoderContext ctx = {NULL, 0};
  VpxVideoReader *reader = NULL;
  const VpxVideoInfo *info = NULL;

  exec_name = argv[0];

  if (argc != 3)
    die("Invalid number of arguments.");

  reader = vpx_video_reader_open(argv[1]);
  if (!reader)
    die("Failed to open %s for reading.", argv[1]);

  if (!(ctx.file = fopen(argv[2], "wb")))
    die("Failed to open %s for writing.", argv[2]);

  info = vpx_video_reader_get_info(reader);

  vpx_decoder *decoder = vpx_decoder_alloc(VPX_CODEC_VP9);
  vpx_decoder_set_callback(decoder, frame_decoded);
  vpx_decoder_set_priv_data(decoder, &ctx);
  //vpx_decoder_set_parallel_decode(decoder, 1);

  while (vpx_video_reader_read_frame(reader)) {
    vpx_const_buffer buf = {NULL, 0};
    buf.buf = vpx_video_reader_get_frame(reader, &buf.size);

    if (vpx_decoder_push_data(decoder, &buf, 0))
      die("Can not push data");
  }

  vpx_decoder_flush(decoder);

  printf("Decoded %d frames.\n", ctx.frames);

  vpx_decoder_free(decoder);

  fclose(ctx.file);
  vpx_video_reader_close(reader);

  printf("Play: ffplay -f rawvideo -pix_fmt yuv420p -s %dx%d %s\n",
         info->frame_width, info->frame_height, argv[2]);

  return EXIT_SUCCESS;
}
