#ifndef VPXIO_RAW_H
#define VPXIO_RAW_H

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int file_is_raw(struct vpxio_ctx *ctx);
int vpxio_read_pkt_raw(struct vpxio_ctx *ctx,
                       uint8_t **buf,
                       size_t *buf_sz,
                       size_t *buf_alloc_sz);
#endif