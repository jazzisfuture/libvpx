#include "vpx/internal/api_vpx_common_internal.h"

const char *vpx_error_get_message(vpx_error *error) {
  return error->message;
}

int vpx_error_get_code(vpx_error *error) {
  return error->code;
}
