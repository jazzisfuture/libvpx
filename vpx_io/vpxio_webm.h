#ifndef VPXIO_WEBM_H
#define VPXIO_WEBM_H

struct vpxio_ctx;
int webm_guess_framerate(struct vpxio_ctx *ctx);
int nestegg_read_cb(void *buffer, size_t length, void *userdata);
int nestegg_seek_cb(int64_t offset, int whence, void *userdata);
int64_t nestegg_tell_cb(void *userdata);
int file_is_webm(struct vpxio_ctx *ctx);
int write_webm_file_header(struct vpxio_ctx *ctx);
int vpxio_write_pkt_webm(struct vpxio_ctx *ctx,
                         const vpx_codec_cx_pkt_t *pkt);
int vpxio_write_close_enc_file_webm(struct vpxio_ctx *ctx);
int vpxio_read_pkt_webm(struct vpxio_ctx *ctx,
                        uint8_t **buf,
                        size_t *buf_sz,
                        size_t *buf_alloc_sz);
#endif
