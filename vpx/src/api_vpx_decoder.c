#include "vpx/api_vpx_decoder.h"

#include "vpx/internal/api_vpx_common_internal.h"

#include "vp9/decoder/vp9_decoder.h"
#include "vp9/vp9_iface_common.h"

static INLINE uint8_t read_marker(const uint8_t *data) {
  return *data;
}

static vpx_codec_err_t parse_superframe_index(const uint8_t *data,
                                              size_t data_sz,
                                              uint32_t sizes[8], int *count) {
  // A chunk ending with a byte matching 0xc0 is an invalid chunk unless
  // it is a super frame index. If the last byte of real video compression
  // data is 0xc0 the encoder must add a 0 byte. If we have the marker but
  // not the associated matching marker byte at the front of the index we have
  // an invalid bitstream and need to return an error.

  uint8_t marker;

  assert(data_sz);
  marker = read_marker(data + data_sz - 1);
  *count = 0;

  if ((marker & 0xe0) == 0xc0) {
    const uint32_t frames = (marker & 0x7) + 1;
    const uint32_t mag = ((marker >> 3) & 0x3) + 1;
    const size_t index_sz = 2 + mag * frames;

    // This chunk is marked as having a superframe index but doesn't have
    // enough data for it, thus it's an invalid superframe index.
    if (data_sz < index_sz)
      return VPX_CODEC_CORRUPT_FRAME;

    {
      const uint8_t marker2 = read_marker(data + data_sz - index_sz);

      // This chunk is marked as having a superframe index but doesn't have
      // the matching marker byte at the front of the index therefore it's an
      // invalid chunk.
      if (marker != marker2)
        return VPX_CODEC_CORRUPT_FRAME;
    }

    {
      // Found a valid superframe index.
      uint32_t i, j;
      const uint8_t *x = &data[data_sz - index_sz + 1];

      for (i = 0; i < frames; ++i) {
        uint32_t this_sz = 0;

        for (j = 0; j < mag; ++j)
          this_sz |= (*x++) << (j * 8);
        sizes[i] = this_sz;
      }
      *count = frames;
    }
  }
  return VPX_CODEC_OK;
}

// =================
// vpx_decoded_frame
// =================

struct vpx_decoded_frame {
  vpx_image_t *img;
};

vpx_image_t *vpx_decoded_frame_get_image(vpx_decoded_frame *frame) {
  return frame->img;
}

// ===========
// vpx_decoder
// ===========

struct vpx_decoder {
  vpx_codec_id id;
  void *priv;
  VP9Decoder *dec;
  vpx_frame_decoded_callback callback;
  vpx_image_t img;
  vpx_decoder_state state;
  vpx_error error;
};

//
// New/delete
//
static void init_buffer_callbacks(VP9Decoder *decoder) {
  VP9_COMMON *const cm = &decoder->common;

  cm->new_fb_idx = -1;

  cm->get_fb_cb = vp9_get_frame_buffer;
  cm->release_fb_cb = vp9_release_frame_buffer;

  if (vp9_alloc_internal_frame_buffers(&cm->int_frame_buffers))
    vpx_internal_error(&cm->error, VPX_CODEC_MEM_ERROR,
                       "Failed to initialize internal frame buffers");

  cm->cb_priv = &cm->int_frame_buffers;
}

vpx_decoder *vpx_decoder_new(vpx_codec_id codec_id) {
  vpx_decoder *decoder = (vpx_decoder *)malloc(sizeof(*decoder));
  if (!decoder)
    return NULL;

  decoder->id = codec_id;
  decoder->priv = NULL;
  decoder->callback = NULL;
  decoder->state = VPX_DECODER_STATE_NEW;
  decoder->error.message = NULL;
  decoder->error.code = 0;
  decoder->dec = vp9_decoder_create();
  decoder->dec->max_threads = 1;
  decoder->dec->inv_tile_order = 0;
  decoder->dec->frame_parallel_decode = 0;

  init_buffer_callbacks(decoder->dec);

  return decoder;
}

void vpx_decoder_delete(vpx_decoder *decoder) {
  if (decoder) {
    assert(decoder->state == VPX_DECODER_STATE_DONE);

    vp9_decoder_remove(decoder->dec);
    free(decoder);
  }
}

//
// State
//
vpx_decoder_state vpx_decoder_get_state(vpx_decoder *decoder) {
  assert(decoder);

  return decoder->state;
}

vpx_status vpx_decoder_initialize(vpx_decoder *decoder) {
  assert(decoder);

  if (decoder->state != VPX_DECODER_STATE_NEW)
    return VPX_STATUS_DECODER_INVALID_STATE;

  // Check all required options
  if (!decoder->callback)
    return VPX_STATUS_ERROR;

  decoder->state = VPX_DECODER_STATE_READY;

  return VPX_STATUS_OK;
}

vpx_status vpx_decoder_finalize(vpx_decoder *decoder) {
  assert(decoder);

  if (decoder->state != VPX_DECODER_STATE_READY)
    return VPX_STATUS_DECODER_INVALID_STATE;

  decoder->state = VPX_DECODER_STATE_DONE;

  return VPX_STATUS_OK;
}

static void decoder_finalize_with_error(vpx_decoder *decoder, const char *error) {
  //decoder->error = error;
  decoder->state = VPX_DECODER_STATE_DONE;
}

//
// Private data
//
void vpx_decoder_set_priv_data(vpx_decoder *decoder, void *data) {
  assert(decoder);

  decoder->priv = data;
}


void *vpx_decoder_get_priv_data(vpx_decoder *decoder) {
  assert(decoder);

  return decoder->priv;
}

//
// Callback
//
vpx_status vpx_decoder_set_callback(vpx_decoder *decoder, vpx_frame_decoded_callback callback) {
  assert(decoder);
  assert(callback);

  if (decoder->state != VPX_DECODER_STATE_NEW) {
    return VPX_STATUS_DECODER_INVALID_STATE;
  }

  decoder->callback = callback;

  return VPX_STATUS_OK;
}

vpx_frame_decoded_callback vpx_decoder_get_callback(vpx_decoder *decoder) {
  assert(decoder);

  return decoder->callback;
}

//
// Parallel Mode
//
vpx_status vpx_decoder_set_parallel_mode(vpx_decoder *decoder, int enabled) {
  return VPX_STATUS_DECODER_UNSUPPORTED_OPTION;
}

vpx_status vpx_decoder_get_parallel_mode(vpx_decoder *decoder, int *enabled) {
  return VPX_STATUS_DECODER_UNSUPPORTED_OPTION;
}

//
// Push
//
static int decode_one(vpx_decoder *decoder, const uint8_t **data, size_t size) {
  vp9_ppflags_t flags = {0, 0, 0};
  YV12_BUFFER_CONFIG sd;


  if (vp9_receive_compressed_data(decoder->dec, size, data))
    return VPX_STATUS_OK;

  if (vp9_get_raw_frame(decoder->dec, &sd, &flags))
    return VPX_STATUS_OK;

  yuvconfig2image(&decoder->img, &sd, NULL);

  vpx_decoded_frame frame = {&decoder->img};
  return decoder->callback(decoder, &frame);
}

vpx_status vpx_decoder_push_data(vpx_decoder *decoder, const vpx_buffer *buf, int flags) {
  assert(decoder);

  if (decoder->state != VPX_DECODER_STATE_READY)
    return VPX_STATUS_DECODER_INVALID_STATE;


  uint32_t frame_sizes[8];
  int frame_count;

  parse_superframe_index(buf->buf, buf->size, frame_sizes, &frame_count);

  const uint8_t *data_start = buf->buf;
  const uint8_t * const data_end = buf->buf + buf->size;

  // Decode in serial mode.
  if (frame_count > 0) {
    int i;

    for (i = 0; i < frame_count; ++i) {
      const uint8_t *data_start_copy = data_start;
      const uint32_t frame_size = frame_sizes[i];
      vpx_codec_err_t res;
      if (data_start < buf->buf
          || frame_size > (uint32_t) (data_end - data_start)) {
        return VPX_STATUS_ERROR;
      }

      res = decode_one(decoder, &data_start_copy, frame_size);
      if (res != VPX_STATUS_OK) {
        decoder_finalize_with_error(decoder, "Callback failed");
        return res;
      }

      data_start += frame_size;
    }
  } else {
    while (data_start < data_end) {
      const uint32_t frame_size = (uint32_t) (data_end - data_start);
      const vpx_codec_err_t res = decode_one(decoder, &data_start, frame_size);
      if (res != VPX_STATUS_OK) {
        decoder_finalize_with_error(decoder, "Callback failed");
        return res;
      }

      // Account for suboptimal termination by the encoder.
      while (data_start < data_end) {
        const uint8_t marker = read_marker(data_start);
        if (marker)
          break;
        ++data_start;
      }
    }
  }

  return VPX_STATUS_OK;
}

//
// Error
//
vpx_error *vpx_decoder_get_error(vpx_decoder *decoder) {
  assert(decoder);
  if (decoder->state != VPX_DECODER_STATE_DONE)
    return NULL;

  if (decoder->error.code == 0)
    return NULL;

  return &decoder->error;
}

//
// Flush
//
void vpx_decoder_flush(vpx_decoder *decoder) {
 // Nothing to do for synchronous decoder
}
