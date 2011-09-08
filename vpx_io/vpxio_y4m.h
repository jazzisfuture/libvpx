#ifndef VPXIO_Y4M_H
#define VPXIO_Y4M_H
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int file_is_y4m(struct vpxio_ctx *ctx);
int write_y4m_file_header(struct vpxio_ctx *ctx);
int vpxio_read_open_raw_file_y4m(struct vpxio_ctx *ctx);
int vpxio_read_img_y4m(struct vpxio_ctx *ctx, vpx_image_t *img_raw);

#endif
