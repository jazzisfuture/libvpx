#ifndef API_VPX_DECODER_H_
#define API_VPX_DECODER_H_

#include "./vpx_image.h"
#include "./api_vpx_common.h"

#define VPX_STATUS_DECODER_INVALID_STATE        1000
#define VPX_STATUS_DECODER_UNSUPPORTED_OPTION   1001
#define VPX_STATUS_DECODER_BUSY                 1002

// =================
// vpx_decoded_frame
// =================

struct vpx_decoded_frame;
typedef struct vpx_decoded_frame vpx_decoded_frame;

void vpx_decoded_frame_retain(vpx_decoded_frame *frame);
void vpx_decoder_frame_release(vpx_decoded_frame *frame);

vpx_image_t *vpx_decoded_frame_get_image(vpx_decoded_frame *frame);

// ===========
// vpx_decoder
// ===========

struct vpx_decoder;
typedef struct vpx_decoder vpx_decoder;

//
// New/delete
//
vpx_decoder *vpx_decoder_new(vpx_codec_id codec_id);
void vpx_decoder_delete(vpx_decoder *decoder);

// Id
vpx_codec_id vpx_decoder_get_id(vpx_decoder *decoder);


// Initialize
vpx_status vpx_decoder_initialize(vpx_decoder *decoder);

//
// Parallel Mode
//
// Returns
//   VPX_STATUS_OK
//   VPX_STATUS_ERROR
//   VPX_STATUS_DECODER_UNSUPPORTED
//   VPX_STATUS_DECODER_INVALID_STATE
vpx_status vpx_decoder_set_parallel_mode(vpx_decoder *decoder, int enabled);

// Returns
//   VPX_STATUS_OK
//   VPX_STATUS_ERROR
//   VPX_STATUS_DECODER_UNSUPPORTED
vpx_status vpx_decoder_get_parallel_mode(vpx_decoder *decoder, int *enabled);

//
// Decode
//
// Returns:
//   VPX_STATUS_OK
//   VPX_STATUS_ERROR
//   VPX_STATUS_DECODER_INVALID_STATE
//   VPX_STATUS_DECODER_BUSY
vpx_status vpx_decoder_decode(vpx_decoder *decoder, const vpx_buffer *buf, vpx_decoded_frame **frame, int flags);

#endif  // API_VPX_DECODER_H_
