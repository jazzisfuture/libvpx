int write_ivf_file_header(struct vpxio_ctx *output_ctx);
int write_ivf_frame_header(FILE *outfile, const vpx_codec_cx_pkt_t *pkt);
int write_ivf_frame_header_dec(FILE *outfile,
                               uint64_t timeStamp,
                               uint32_t frameSize);
int vpxio_read_open_raw_file_ivf(struct vpxio_ctx *input_ctx);
int vpxio_read_img_ivf(struct vpxio_ctx *input_ctx, vpx_image_t *img_raw);
int vpxio_write_pkt_ivf(struct vpxio_ctx *output_ctx,
                        const vpx_codec_cx_pkt_t *pkt);
int vpxio_write_close_enc_file_ivf(struct vpxio_ctx *output_ctx);
int vpxio_read_pkt_ivf(struct vpxio_ctx *input_ctx,
                       uint8_t **buf,
                       size_t *buf_sz,
                       size_t *buf_alloc_sz);
int vpxio_write_img_ivf(struct vpxio_ctx *output_ctx,
                        vpx_image_t *img,
                        uint8_t *buf);