//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int write_y4m_file_header(struct vpxio_ctx *output_ctx);
int vpxio_read_open_raw_file_y4m(struct vpxio_ctx *input_ctx);
int vpxio_read_img_y4m(struct vpxio_ctx *input_ctx, vpx_image_t *img_raw);
int vpxio_write_img_y4m(struct vpxio_ctx *output_ctx, vpx_image_t *img,
                        uint8_t *buf);