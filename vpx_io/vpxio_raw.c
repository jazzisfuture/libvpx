#include <string.h>
#include <stdlib.h>
#include "vpx_io/vpxio.h"
#include "vpx_io/vpxio_intrn.h"
#include "vpx_ports/mem_ops.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"

#define VP8_FOURCC (0x00385056)

static const struct
{
    char const *name;
    const vpx_codec_iface_t *iface;
    unsigned int             fourcc;
    unsigned int             fourcc_mask;
} ifaces[] =
{
#if CONFIG_VP8_DECODER
    {"vp8",  &vpx_codec_vp8_dx_algo,   VP8_FOURCC, 0x00FFFFFF},
#endif
};

int file_is_raw(struct vpxio_ctx *ctx)
{
    unsigned char buf[32];
    int is_raw = 0;
    vpx_codec_stream_info_t si;
    si.sz = sizeof(si);

    if (fread(buf, 1, 32, ctx->file) == 32)
    {
        int i;

        if (mem_get_le32(buf) < 256 * 1024 * 1024)
            for (i = 0; i < sizeof(ifaces) / sizeof(ifaces[0]); i++)
                if (!vpx_codec_peek_stream_info(ifaces[i].iface,
                                                buf + 4, 32 - 4, &si))
                {
                    is_raw = 1;
                    ctx->fourcc = ifaces[i].fourcc;
                    ctx->width = si.w;
                    ctx->height = si.h;
                    ctx->framerate.num = 30;
                    ctx->framerate.den = 1;
                    break;
                }
    }

    rewind(ctx->file);
    return is_raw;
}
int vpxio_read_pkt_raw(struct vpxio_ctx *ctx,
                       uint8_t **buf,
                       size_t *buf_sz,
                       size_t *buf_alloc_sz)
{
    char     raw_hdr[IVF_FRAME_HDR_SZ];
    size_t          new_buf_sz;

    if (fread(raw_hdr, RAW_FRAME_HDR_SZ, 1,
              ctx->decompression_ctx.webm_ctx.infile) != 1)
    {
        if (!feof(ctx->decompression_ctx.webm_ctx.infile))
            fprintf(stderr, "Failed to read frame size\n");

        new_buf_sz = 0;
    }
    else
    {
        new_buf_sz = mem_get_le32(raw_hdr);

        if (new_buf_sz > 256 * 1024 * 1024)
        {
            fprintf(stderr, "Error: Read invalid frame size (%u)\n",
                    new_buf_sz);
            new_buf_sz = 0;
        }

        if (ctx->decompression_ctx.webm_ctx.kind == FILE_TYPE_RAW_PKT &&
                new_buf_sz > 256 * 1024)
            fprintf(stderr, "Warning: Read invalid frame size (%u)"
                    " - not a raw file?\n", new_buf_sz);

        if (new_buf_sz > *buf_alloc_sz)
        {
            uint8_t *new_buf = (uint8_t *)realloc(*buf, 2 * new_buf_sz);

            if (new_buf)
            {
                *buf = new_buf;
                *buf_alloc_sz = 2 * new_buf_sz;
            }
            else
            {
                fprintf(stderr, "Failed to allocate compressed data buffer\n");
                new_buf_sz = 0;
            }
        }
    }

    *buf_sz = new_buf_sz;

    if (*buf_sz)
    {
        if (fread(*buf, 1, *buf_sz, ctx->decompression_ctx.webm_ctx.infile)
                != *buf_sz)
        {
            fprintf(stderr, "Failed to read full frame\n");
            return 1;
        }

        return 0;
    }

    return 1;
}
