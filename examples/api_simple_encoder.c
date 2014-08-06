#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpx/api_vpx_encoder.h"

#include "./tools_common.h"
#include "./video_writer.h"

/*
  vpx_encoder_set_timebase(encoder, &time_base);
  vpx_encoder_set_mode(encoder, VPX_CQ);
  vpx_encoder_set_target_bitrate(encoder, 250);
  vpx_encoder_set_cq_level(10);
  vpx_encoder_set_speed_type(encoder, VPX_GOOD);
  vpx_encoder_set_speed(encoder, 10);

  vpx_encoder_set_tiles(encoder, &tiles);
  vpx_encoder_set_adaptive_quantization_mode(encoder, &aq);
  vpx_encoder_set_error_resilient(encoder, 1);
  vpx_encoder_set_calculate_psnr(encoder, 1);
 */

static const char *exec_name;

void usage_exit() {
  fprintf(stderr,
          "Usage: %s <codec> <width> <height> <infile> <outfile> "
              "<keyframe-interval> [<error-resilient>]\nSee comments in "
              "simple_encoder.c for more information.\n",
          exec_name);
  exit(EXIT_FAILURE);
}

static void write_frame(VpxVideoWriter *writer, vpx_encoded_frame *frame) {
   vpx_buffer *buf = vpx_encoded_frame_get_buffer(frame);
   const int pts = vpx_encoded_frame_get_pts(frame);
   const int duration = vpx_encoded_frame_get_duration(frame);

   vpx_video_writer_write_frame(writer, buf->buf, buf->size, pts);

   double psnr_y, psnr_u, psnr_v;
   vpx_encoded_frame_get_psnr(frame, &psnr_y, &psnr_u, &psnr_v);

   printf(vpx_encoded_frame_is_keyframe(frame) ? "K" : ".");
   fflush(stdout);
}

int main(int argc, char **argv) {
  FILE *infile = NULL;
  int frame_count = 0;
  vpx_image_t raw;
  VpxVideoInfo info = {0};
  VpxVideoWriter *writer = NULL;
  const int fps = 30;        // TODO(dkovalev) add command line argument
  const int bitrate = 200;   // kbit/s TODO(dkovalev) add command line argument
  int keyframe_interval = 0;

  // TODO(dkovalev): Add some simple command line parsing code to make the
  // command line more flexible.
  const char *codec_arg = NULL;
  const char *width_arg = NULL;
  const char *height_arg = NULL;
  const char *infile_arg = NULL;
  const char *outfile_arg = NULL;
  const char *keyframe_interval_arg = NULL;

  exec_name = argv[0];

  if (argc < 7)
    die("Invalid number of arguments");

  codec_arg = argv[1];
  width_arg = argv[2];
  height_arg = argv[3];
  infile_arg = argv[4];
  outfile_arg = argv[5];
  keyframe_interval_arg = argv[6];

  info.codec_fourcc = VP9_FOURCC;
  info.frame_width = strtol(width_arg, NULL, 0);
  info.frame_height = strtol(height_arg, NULL, 0);
  info.time_base.numerator = 1;
  info.time_base.denominator = fps;

  if (info.frame_width <= 0 ||
      info.frame_height <= 0 ||
      (info.frame_width % 2) != 0 ||
      (info.frame_height % 2) != 0) {
    die("Invalid frame size: %dx%d", info.frame_width, info.frame_height);
  }

  if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, info.frame_width,
                                             info.frame_height, 1)) {
    die("Failed to allocate image.");
  }

  keyframe_interval = strtol(keyframe_interval_arg, NULL, 0);
  if (keyframe_interval < 0)
    die("Invalid keyframe interval value.");

  vpx_encoder *encoder = vpx_encoder_new(VPX_CODEC_VP9);
  if (!encoder)
    die("Unsupported codec.");

  vpx_encoder_set_frame_size(encoder, info.frame_width, info.frame_height);
  vpx_encoder_set_time_base(encoder, info.time_base.numerator, info.time_base.denominator);
  vpx_encoder_set_bitrate(encoder, bitrate);
  //vpx_encoder_set_error_ressilient(encoder, argc > 7 ? strtol(argv[7], NULL, 0) : 0);

  writer = vpx_video_writer_open(outfile_arg, kContainerIVF, &info);
  if (!writer)
    die("Failed to open %s for writing.", outfile_arg);

  if (!(infile = fopen(infile_arg, "rb")))
    die("Failed to open %s for reading.", infile_arg);

  vpx_encoder_initialize(encoder);

  while (vpx_img_read(&raw, infile)) {
    int flags = 0;
    //if (keyframe_interval > 0 && frame_count % keyframe_interval == 0)
    //  flags |= VPX_EFLAG_FORCE_KF;
    vpx_encoded_frame *frame = NULL;
    const vpx_status status = vpx_encoder_encode(encoder, &raw, frame_count++, 1, flags, &frame);
    if (status != VPX_STATUS_OK)
      break;

    if (frame)
      write_frame(writer, frame);
  }

  // flush
  while (1) {
    vpx_encoded_frame *frame = NULL;
    if (vpx_encoder_encode(encoder, &raw, 0, 0, 0, &frame) != VPX_STATUS_OK)
      break;

    if (frame)
      write_frame(writer, frame);
  }

  printf("\n");
  fclose(infile);
  printf("Processed %d frames.\n", frame_count);

  vpx_img_free(&raw);
  vpx_encoder_delete(encoder);
  vpx_video_writer_close(writer);

  return EXIT_SUCCESS;
}
