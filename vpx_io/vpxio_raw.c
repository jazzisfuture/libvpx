#include <string.h>
#include <stdlib.h>
#include "vpx_io/vpxio.h"
#include "vpx_ports/mem_ops.h"

struct vpxio_ctx_t;

int vpxio_read_pkt_raw(struct vpxio_ctx *input_ctx, uint8_t **buf, size_t *buf_sz, size_t *buf_alloc_sz)
{
    char     raw_hdr[IVF_FRAME_HDR_SZ];
    size_t          new_buf_sz;

    if (fread(raw_hdr, RAW_FRAME_HDR_SZ, 1, input_ctx->decompression_ctx.webm_ctx.infile) != 1)
    {
        if (!feof(input_ctx->decompression_ctx.webm_ctx.infile))
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

        if (input_ctx->decompression_ctx.webm_ctx.kind == RAW_FILE && new_buf_sz > 256 * 1024)
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
        if (fread(*buf, 1, *buf_sz, input_ctx->decompression_ctx.webm_ctx.infile) != *buf_sz)
        {
            fprintf(stderr, "Failed to read full frame\n");
            return 1;
        }

        return 0;
    }

    return 1;
}
