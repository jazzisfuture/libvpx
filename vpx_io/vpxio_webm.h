int write_webm_file_header(struct vpxio_ctx *output_ctx);
int vpxio_write_pkt_webm(struct vpxio_ctx *output_ctx, const vpx_codec_cx_pkt_t *pkt);
int vpxio_write_close_enc_file_webm(struct vpxio_ctx *output_ctx);
int vpxio_read_pkt_webm(struct vpxio_ctx *input_ctx, uint8_t **buf, size_t *buf_sz, size_t *buf_alloc_sz);