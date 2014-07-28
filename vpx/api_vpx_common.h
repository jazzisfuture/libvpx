#ifndef API_VPX_COMMON_H_
#define API_VPX_COMMON_H_

#include "vpx/vpx_integer.h"

//
// vpx_codec_id
//
enum vpx_codec_id {
  VPX_CODEC_VP8,
  VPX_CODEC_VP9
};

typedef enum vpx_codec_id vpx_codec_id;

//
// vpx_buffer
//
struct vpx_buffer {
  const uint8_t *buf;
  size_t size;
};

typedef struct vpx_buffer vpx_buffer;

//
// vpx_status
//
typedef int vpx_status;

#define VPX_STATUS_OK 0
#define VPX_STATUS_ERROR 1

//
// vpx_error
//
struct vpx_error;
typedef struct vpx_error vpx_error;

const char *vpx_error_get_message(vpx_error *error);
int vpx_error_get_code(vpx_error *error);

#endif  // API_VPX_COMMON_H_
