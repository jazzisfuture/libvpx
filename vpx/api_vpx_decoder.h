#ifndef API_VPX_DECODER_H_
#define API_VPX_DECODER_H_

#include "./vpx_image.h"
#include "./api_vpx_common.h"

struct vpx_decoded_frame;
typedef struct vpx_decoded_frame vpx_decoded_frame;

vpx_image_t *vpx_decoded_frame_get_image(vpx_decoded_frame *frame);
void vpx_decoded_frame_done(vpx_decoded_frame *frame);

struct vpx_decoder;
typedef struct vpx_decoder vpx_decoder;

typedef void (*vpx_frame_decoded_callback)(vpx_decoder *decoder, vpx_decoded_frame *frame);

// alloc/free
vpx_decoder *vpx_decoder_alloc(vpx_codec_id codec_id);
void vpx_decoder_free(vpx_decoder *decoder);

// status
vpx_status vpx_decoder_get_status(vpx_decoder *status);

// priv_data
vpx_status vpx_decoder_set_priv_data(vpx_decoder *decoder, void *data);
void *vpx_decoder_get_priv_data(vpx_decoder *decoder);

// callback
vpx_status vpx_decoder_set_callback(vpx_decoder *decoder, vpx_frame_decoded_callback callback);
vpx_frame_decoded_callback vpx_decoder_get_callback(vpx_decoder *decoder);

// push
// must be asynchronous if parallel mode is enabled
// must be synchronous if parallel mode is disabled

#define VPX_BLOCK_IF_BUSY (1 << 0)

// VPX_STATUS_OK
// VPX_STATUS_BROKEN
// VPX_STATUS_BUSY (only in parallel mode)
vpx_status vpx_decoder_push_data(vpx_decoder *decoder, const vpx_const_buffer *buf, int flags);

// flush
vpx_status vpx_decoder_flush(vpx_decoder *decoder);


// Example: push frame and get frame

static void frame_decoded(vpx_decoder *decoder, vpx_decoded_frame *frame) {
  vpx_decoder_set_priv_data(decoder, frame);
}

static vpx_decoded_frame *sync_decode(vpx_decoder *decoder, vpx_const_buffer *buf) {
  vpx_decoded_frame *frame = NULL;
  vpx_decoder_push_data(decoder, buf);
  vpx_decoder_flush(decoder);
  frame = (vpx_decoded_frame *)vpx_decoder_get_priv_data(decoder);
  vpx_decoder_set_priv_data(decoder, NULL);
  return frame;
}

void main() {
  vpx_decoder *decoder = vpx_decoder_alloc(VPX_CODEC_VP9);
  vpx_decoder_set_callback(frame_decoded);
  vpx_decoder_set_priv_data(decoder, &s);

  while (1) {
    vpx_decoded_frame *frame = sync_decode(decoder, buf);
    if (frame) {
      // process frame
      vpx_decoded_frame_done(frame);
    }
  }

  vpx_decoder_free(decoder);
}


#endif  // API_VPX_DECODER_H_
