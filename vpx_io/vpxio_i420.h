#ifndef VPXIO_I420_H
#define VPXIO_I420_H

#include "vpx/vpx_image.h"
int vpxio_read_img_raw(struct vpxio_ctx *ctx,
                        vpx_image_t * img_raw);

#endif
