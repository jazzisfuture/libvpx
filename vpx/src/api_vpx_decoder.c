#include "vpx/api_vpx_decoder.h"

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

      // Frames has a maximum of 8 and mag has a maximum of 4.
      assert(sizeof(clear_buffer) >= frames * mag);


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


struct vpx_decoded_frame {
  vpx_image_t *img;
};

void vpx_decoded_frame_done(vpx_decoded_frame *frame) {
  // Nothing to do
}

vpx_image_t *vpx_decoded_frame_get_image(vpx_decoded_frame *frame) {
  return frame->img;
}

struct vpx_decoder {
  vpx_codec_id id;
  void *priv;
  VP9Decoder *dec;
  vpx_frame_decoded_callback callback;
  vpx_image_t img;
};

// alloc/free

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

vpx_decoder *vpx_decoder_alloc(vpx_codec_id codec_id) {
  vpx_decoder *decoder = (vpx_decoder *)malloc(sizeof(*decoder));
  if (!decoder)
    return NULL;

  decoder->id = codec_id;
  decoder->priv = NULL;
  decoder->dec = vp9_decoder_create();
  decoder->dec->max_threads = 1;
  decoder->dec->inv_tile_order = 0;
  decoder->dec->frame_parallel_decode = 0;
  init_buffer_callbacks(decoder->dec);


  decoder->callback = NULL;

  return decoder;
}

void vpx_decoder_free(vpx_decoder *decoder) {
  if (decoder) {
    vp9_decoder_remove(decoder->dec);
    free(decoder);
  }
}

// priv_data
vpx_status vpx_decoder_set_priv_data(vpx_decoder *decoder, void *data) {
  decoder->priv = data;
  return VPX_STATUS_OK;
}

void *vpx_decoder_get_priv_data(vpx_decoder *decoder) {
  return decoder->priv;
}

// callback
vpx_status vpx_decoder_set_callback(vpx_decoder *decoder, vpx_frame_decoded_callback callback) {
  decoder->callback = callback;
  return VPX_STATUS_OK;
}

vpx_frame_decoded_callback vpx_decoder_get_callback(vpx_decoder *decoder) {
 return decoder->callback;
}

// push
static int decode_one(vpx_decoder *decoder, const uint8_t **data, size_t size) {
  vp9_ppflags_t flags = {0, 0, 0};
  YV12_BUFFER_CONFIG sd;


  if (vp9_receive_compressed_data(decoder->dec, size, data)) {
    return VPX_STATUS_OK;
  }

  if (vp9_get_raw_frame(decoder->dec, &sd, &flags)) {
    return VPX_STATUS_OK;
  }

  yuvconfig2image(&decoder->img, &sd, NULL);

  vpx_decoded_frame frame = {&decoder->img};
  decoder->callback(decoder, &frame);
  return VPX_STATUS_OK;
}

vpx_status vpx_decoder_push_data(vpx_decoder *decoder, const vpx_const_buffer *buf, int flags) {
  if (!decoder)
    return VPX_STATUS_ERROR;

  if (!decoder->callback)
    return VPX_STATUS_ERROR;

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
      if (res)
        return res;

      data_start += frame_size;
    }
  } else {
    while (data_start < data_end) {
      const uint32_t frame_size = (uint32_t) (data_end - data_start);
      const vpx_codec_err_t res = decode_one(decoder, &data_start, frame_size);
      if (res)
        return res;

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

// flush
vpx_status vpx_decoder_flush(vpx_decoder *decoder) {
  return VPX_STATUS_OK;
}
