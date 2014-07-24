#ifndef API_VPX_COMMON_H_
#define API_VPX_COMMON_H_

#include "vpx/vpx_integer.h"

typedef enum {
  VPX_CODEC_VP8,
  VPX_CODEC_VP9
} vpx_codec_id;

typedef struct vpx_buffer {
  uint8_t *buf;
  size_t size;
} vpx_buffer;


typedef struct vpx_const_buffer {
  const uint8_t *buf;
  size_t size;
} vpx_const_buffer;

typedef int vpx_status;

#define VPX_STATUS_OK 0
#define VPX_STATUS_ERROR 1

#endif  // API_VPX_COMMON_H_
