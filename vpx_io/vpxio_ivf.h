#ifndef VPXIO_IVF_H
#define VPXIO_IVF_H

int file_is_ivf(struct vpxio_ctx *ctx);
int write_ivf_file_header(struct vpxio_ctx *ctx);
int write_ivf_frame_header_pkt(FILE *outfile, const vpx_codec_cx_pkt_t *pkt);
int write_ivf_frame_header_img(FILE *outfile,
                               uint64_t timeStamp,
                               uint32_t frameSize);
int vpxio_read_img_ivf(struct vpxio_ctx *ctx, vpx_image_t *img_raw);
int vpxio_write_pkt_ivf(struct vpxio_ctx *ctx,
                        const vpx_codec_cx_pkt_t *pkt);
int vpxio_write_close_enc_file_ivf(struct vpxio_ctx *ctx);
int vpxio_read_pkt_ivf(struct vpxio_ctx *ctx,
                       uint8_t **buf,
                       size_t *buf_sz,
                       size_t *buf_alloc_sz);
#endif
