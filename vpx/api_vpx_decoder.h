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
// Creates the decoder in NEW state
vpx_decoder *vpx_decoder_new(vpx_codec_id codec_id);

// Deletes the decoder in DONE state
void vpx_decoder_delete(vpx_decoder *decoder);

//
// State
//
typedef enum vpx_decoder_state {
  VPX_DECODER_STATE_NEW,
  VPX_DECODER_STATE_READY,
  VPX_DECODER_STATE_DONE
} vpx_decoder_state;

// State: any
vpx_decoder_state vpx_decoder_get_state(vpx_decoder *decoder);

// State: NEW
// Changes state from NEW to READY
vpx_status vpx_decoder_initialize(vpx_decoder *decoder);

// State: READY
// Changes state from READY to DONE
vpx_status vpx_decoder_finalize(vpx_decoder *decoder);

//
// Error
//
// State: any
// Returns error (NULL if no error) when STATE is DONE, otherwise returns NULL
vpx_error *vpx_decoder_get_error(vpx_decoder *decoder);

//
// Private data
//
// State: NEW, READY
void vpx_decoder_set_priv_data(vpx_decoder *decoder, void *data);

// State: any
void *vpx_decoder_get_priv_data(vpx_decoder *decoder);

//
// Callback
//
typedef vpx_status (*vpx_frame_decoded_callback)(vpx_decoder *decoder, vpx_decoded_frame *frame);

// State: NEW
// Returns
//   VPX_STATUS_OK
//   VPX_STATUS_ERROR
//   VPX_STATUS_DECODER_INVALID_STATE
vpx_status vpx_decoder_set_callback(vpx_decoder *decoder, vpx_frame_decoded_callback callback);

// State: any
vpx_frame_decoded_callback vpx_decoder_get_callback(vpx_decoder *decoder);

//
// Parallel Mode
//
// State: NEW
// Returns
//   VPX_STATUS_OK
//   VPX_STATUS_ERROR
//   VPX_STATUS_DECODER_UNSUPPORTED
//   VPX_STATUS_DECODER_INVALID_STATE
vpx_status vpx_decoder_set_parallel_mode(vpx_decoder *decoder, int enabled);

// State: any
// Returns
//   VPX_STATUS_OK
//   VPX_STATUS_ERROR
//   VPX_STATUS_DECODER_UNSUPPORTED
vpx_status vpx_decoder_get_parallel_mode(vpx_decoder *decoder, int *enabled);


//
// Push
//
// Asynchronous if parallel mode is enabled, synchronous if parallel mode is disabled.
// States: READY
// Returns:
//   VPX_STATUS_OK
//   VPX_STATUS_ERROR
//   VPX_STATUS_DECODER_INVALID_STATE
//   VPX_STATUS_DECODER_BUSY
vpx_status vpx_decoder_push_data(vpx_decoder *decoder, const vpx_buffer *buf, int flags);

//
// Flush
//
// State: any
void vpx_decoder_flush(vpx_decoder *decoder);

#endif  // API_VPX_DECODER_H_
