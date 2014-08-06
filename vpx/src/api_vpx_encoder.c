#include "vpx/api_vpx_encoder.h"

#include "vpx/internal/api_vpx_common_internal.h"

#include "vp9/encoder/vp9_encoder.h"
#include "vp9/vp9_iface_common.h"

#include <stdlib.h>
#include <assert.h>

struct vpx_encoded_frame {
  int pts;
  int duration;
  int keyframe;
};

vpx_buffer *vpx_encoded_frame_get_buffer(vpx_encoded_frame *frame) {
  return NULL;
}

int vpx_encoded_frame_get_pts(vpx_encoded_frame *frame) {
  return 0;
}

int vpx_encoded_frame_get_duration(vpx_encoded_frame *frame) {
  return 0;
}

int vpx_encoded_frame_is_keyframe(vpx_encoded_frame *frame) {
  return 0;
}

void vpx_encoded_frame_get_psnr(vpx_encoded_frame *frame, double *psnr_y, double *psnr_u, double *psnr_v) {

}

struct vpx_encoder {
  vpx_codec_id id;
  VP9EncoderConfig cfg;
};

//
// New/delete
//
vpx_encoder *vpx_encoder_new(vpx_codec_id codec_id) {
  vpx_encoder *encoder = (vpx_encoder *)malloc(sizeof(*encoder));


  return encoder;
}

void vpx_encoder_delete(vpx_encoder *encoder) {
  if (encoder) {
    free(encoder);
  }
}

// Id
vpx_codec_id vpx_encoder_get_id(vpx_encoder *encoder) {
  assert(encoder);

  return encoder->id;
}

void vpx_encoder_set_frame_size(vpx_encoder *encoder, int width, int height) {
  assert(encoder);
}

//
// Time Base
//
void vpx_encoder_set_time_base(vpx_encoder *encoder, int num, int den) {
  assert(encoder);

}


//
// Bitrate
//
void vpx_encoder_set_bitrate(vpx_encoder *encoder, int bitrate) {
  assert(encoder);
}

int vpx_encoder_get_bitrate(vpx_encoder *encoder) {
  assert(encoder);
  return 0;
}

// Initialize
vpx_status vpx_encoder_initialize(vpx_encoder *encoder) {
  assert(encoder);
  return VPX_STATUS_OK;
}

// Encode
vpx_status vpx_encoder_encode(vpx_encoder *encoder, const vpx_image_t *img, int pts, int duration, int flags, vpx_encoded_frame **frame) {
  assert(encoder);
  return VPX_STATUS_OK;
}

