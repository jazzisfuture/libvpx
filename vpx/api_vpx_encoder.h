#ifndef API_VPX_ENCODER_H_
#define API_VPX_ENCODER_H_

#include "./vpx_image.h"
#include "./api_vpx_common.h"

// =================
// vpx_encoded_frame
// =================
struct vpx_encoded_frame;
typedef struct vpx_encoded_frame vpx_encoded_frame;

vpx_buffer *vpx_encoded_frame_get_buffer(vpx_encoded_frame *frame);
int vpx_encoded_frame_get_pts(vpx_encoded_frame *frame);
int vpx_encoded_frame_get_duration(vpx_encoded_frame *frame);
int vpx_encoded_frame_is_keyframe(vpx_encoded_frame *frame);
void vpx_encoded_frame_get_psnr(vpx_encoded_frame *frame, double *psnr_y, double *psnr_u, double *psnr_v);

// ===========
// vpx_encoder
// ===========
struct vpx_encoder;
typedef struct vpx_encoder vpx_encoder;

//
// New/delete
//
vpx_encoder *vpx_encoder_new(vpx_codec_id codec_id);
void vpx_encoder_delete(vpx_encoder *encoder);

// Id
vpx_codec_id vpx_encoder_get_id(vpx_encoder *encoder);

void vpx_encoder_set_frame_size(vpx_encoder *encoder, int width, int height);

//
// Time Base
//
void vpx_encoder_set_time_base(vpx_encoder *encoder, int num, int den);

//
// Bitrate
//
void vpx_encoder_set_bitrate(vpx_encoder *encoder, int bitrate);
int vpx_encoder_get_bitrate(vpx_encoder *encoder);

// Initialize
vpx_status vpx_encoder_initialize(vpx_encoder *encoder);

// Encode
vpx_status vpx_encoder_encode(vpx_encoder *encoder, const vpx_image_t *img, int pts, int duration, int flags, vpx_encoded_frame **frame);




/*

int main(int argc, char *argv[]) {
  int frame = 0;
  const vpx_rational time_base = {1 / 30};
  FILE *infile = NULL;
  vpx_param_aq aq = {};
  vpx_param_tiles tiles = {0, 2}; // 1 x 4 tiles

  VpxVideoInfo info = {0};
  VpxVideoWriter *writer = NULL;
  vpx_image_t raw;

  info.codec_fourcc = 0;
  info.frame_width = 640;
  info.frame_height = 480;
  info.time_base.numerator = time_base.num;
  info.time_base.denominator = time_base.den;

  if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, info.frame_width,
                                             info.frame_height, 1))
    return 1;

  vpx_encoder *encoder = vpx_encoder_alloc(VPX_CODEC_VP9, frame_encoded);
  if (!encoder)
    return 1;

  writer = vpx_video_writer_open("output.ivf", kContainerIVF, &info);
  if (!writer) {
    vpx_encoder_free(encoder);
    return 1;
  }
  vpx_encoder_set_pass(VPX_PASS_SIGNLE);

  vpx_encoder_set_pass(VPX_PASS_MULTI_FIRST);
  vpx_encoder_set_pass(VPX_PASS_MULTI_SECOND);

  void write_stats(vpx_encoder *encoder, const vpx_buf *buf) {

  }

  vpx_encoder_set_stats_reader();
  vpx_encoder_set_stats_writer();

  vpx_encoder_set_stats(encoder, &buf)

  while (vpx_img_read(&raw, infile)) {
    vpx_encoder_push_frame(encoder, writer, &raw, frame++, 1, 0);
  }

  vpx_endoer_flush();

  vpx_encoder_free(encoder);

  vpx_video_writer_close(writer);

  return 0;
}
*/

#endif // API_VPX_ENCODER_H_
