int vpxio_read_open_file_i420(struct vpxio_ctx *input_ctx, unsigned int * width, unsigned int * height);
int vpxio_read_img_i420(struct vpxio_ctx *input_ctx, vpx_image_t * img_raw);
int vpxio_write_img_i420(struct vpxio_ctx *output_ctx, vpx_image_t *img, uint8_t *buf);