#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "vpx_io/vpxio.h"
#include "vpx_io/vpxio_intrn.h"
#include "vpx_ports/mem_ops.h"
#include "tools_common.h"

#define IVF_FILE_HDR_SZ (32)
struct vpxio_ctx* vpxio_open_ivf_dst(struct vpxio_ctx *ctx,
                       const char *file_name,
                       int width, int height,
                       int framerate_den,
                       int framerate_num)
{
    ctx = vpxio_init_ctx(ctx);
    ctx->file_name = file_name;
    ctx->decompression_ctx.outfile_pattern = file_name;

    ctx->file = strcmp(ctx->file_name, "-") ?
                       fopen(ctx->file_name, "wb") : 
                       set_binary_mode(stdout);

    if (!ctx->file)
    {
        printf("Failed to open output file: %s", ctx->file_name);
        free(ctx);
        return NULL;
    }

    ctx->mode = DST;
    ctx->file_name = file_name;
    ctx->file_type = FILE_TYPE_IVF;
    ctx->width = width;
    ctx->height = height;
    ctx->framerate.den = framerate_den;
    ctx->framerate.num = framerate_num;

    return ctx;
}
int file_is_ivf(struct vpxio_ctx *ctx)
{
    char raw_hdr[IVF_FILE_HDR_SZ];
    int is_ivf = 0;

    ctx->compression_ctx.detect.buf_read =
        fread(ctx->compression_ctx.detect.buf, 1, 4, ctx->file);
    ctx->compression_ctx.detect.position = 0;

    if (memcmp(ctx->compression_ctx.detect.buf, "DKIF", 4) != 0)
    {
        rewind(ctx->file);
        return 0;
    }

    if (fread(raw_hdr + 4, 1, IVF_FILE_HDR_SZ - 4, ctx->file)
            == IVF_FILE_HDR_SZ - 4)
    {
        is_ivf = 1;

        if (mem_get_le16(raw_hdr + 4) != 0)
            fprintf(stderr, "Error: Unrecognized IVF version! This file may not"
                    " decode properly.");

        ctx->fourcc = mem_get_le32(raw_hdr + 8);
        ctx->width = mem_get_le16(raw_hdr + 12);
        ctx->height = mem_get_le16(raw_hdr + 14);
        ctx->framerate.num = mem_get_le32(raw_hdr + 16);
        ctx->framerate.den = mem_get_le32(raw_hdr + 20);
    }

    if (!is_ivf)
        rewind(ctx->file);

    return is_ivf;
}
int write_ivf_file_header(struct vpxio_ctx *ctx)
{
    char header[32];

    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header + 4,  0);                             /* version */
    mem_put_le16(header + 6,  32);                            /* headersize */
    mem_put_le32(header + 8,  ctx->fourcc);            /* headersize */
    mem_put_le16(header + 12, ctx->width);             /* width */
    mem_put_le16(header + 14, ctx->height);            /* height */
    mem_put_le32(header + 16, ctx->framerate.num);     /* rate */
    mem_put_le32(header + 20, ctx->framerate.den);     /* scale */
    mem_put_le32(header + 24, ctx->frame_cnt);         /* length */
    mem_put_le32(header + 28, 0);                             /* unused */

    if (fwrite(header, 1, 32, ctx->file))
        return 0;
    else
        return 1;
}
int write_ivf_frame_header_pkt(FILE *outfile, const vpx_codec_cx_pkt_t *pkt)
{
    char             header[12];
    vpx_codec_pts_t  pts;

    if (pkt->kind != VPX_CODEC_CX_FRAME_PKT)
        return 0;

    pts = pkt->data.frame.pts;
    mem_put_le32(header, pkt->data.frame.sz);
    mem_put_le32(header + 4, pts & 0xFFFFFFFF);
    mem_put_le32(header + 8, pts >> 32);

    if (fwrite(header, 1, 12, outfile))
        return 0;
    else
        return 1;
}
int write_ivf_frame_header_img(FILE *outfile,
                               uint64_t timeStamp,
                               uint32_t frameSize)
{
    char             header[12];

    mem_put_le32(header, frameSize);
    mem_put_le32(header + 4, 0);
    mem_put_le32(header + 8, 0);

    return 0;
}
int vpxio_read_img_ivf(struct vpxio_ctx *ctx, vpx_image_t *img_raw)
{
    int plane = 0;
    int shortread = 0;

    char junk[IVF_FRAME_HDR_SZ];

    /* Skip the frame header. We know how big the frame should be. See
    * write_ivf_frame_header() for documentation on the frame header
    * layout.
    */
    if (fread(junk, 1, IVF_FRAME_HDR_SZ, ctx->file));

    for (plane = 0; plane < 3; plane++)
    {
        unsigned char *ptr;
        int w = (plane ? (1 + img_raw->d_w) / 2 : img_raw->d_w);
        int h = (plane ? (1 + img_raw->d_h) / 2 : img_raw->d_h);
        int r;

        /* Determine the correct plane based on the image format. The for-loop
        * always counts in Y,U,V order, but this may not match the order of
        * the data on disk.
        */
        switch (plane)
        {
        case 1:
            ptr = img_raw->planes[img_raw->fmt==IMG_FMT_YV12? PLANE_V : PLANE_U];
            break;
        case 2:
            ptr = img_raw->planes[img_raw->fmt==IMG_FMT_YV12?PLANE_U : PLANE_V];
            break;
        default:
            ptr = img_raw->planes[plane];
        }

        for (r = 0; r < h; r++)
        {
            size_t needed = w;
            size_t buf_position = 0;
            const size_t left = ctx->compression_ctx.detect.buf_read -
                                ctx->compression_ctx.detect.position;

            if (left > 0)
            {
                const size_t more = (left < needed) ? left : needed;
                memcpy(ptr, ctx->compression_ctx.detect.buf +
                       ctx->compression_ctx.detect.position, more);
                buf_position = more;
                needed -= more;
                ctx->compression_ctx.detect.position += more;
            }

            if (needed > 0)
            {
                shortread |= (fread(ptr + buf_position, 1,
                                    needed, ctx->file) < needed);
            }

            ptr += img_raw->stride[plane];
        }
    }

    return !shortread;
}
int vpxio_write_pkt_ivf(struct vpxio_ctx *ctx,
                        const vpx_codec_cx_pkt_t *pkt)
{
    write_ivf_frame_header_pkt(ctx->file, pkt);

    if (fwrite(pkt->data.frame.buf, 1,
               pkt->data.frame.sz, ctx->file));

    return 0;
}
int vpxio_write_close_enc_file_ivf(struct vpxio_ctx *ctx)
{
    if (!fseek(ctx->file, 0, SEEK_SET))
        write_ivf_file_header(ctx);

    return 0;
}
int vpxio_read_pkt_ivf(struct vpxio_ctx *ctx,
                       uint8_t **buf,
                       size_t *buf_sz,
                       size_t *buf_alloc_sz)
{
    char     raw_hdr[IVF_FRAME_HDR_SZ];
    size_t          new_buf_sz;

    if (fread(raw_hdr, IVF_FRAME_HDR_SZ, 1,
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

        if (ctx->decompression_ctx.webm_ctx.kind ==
                FILE_TYPE_RAW_PKT && new_buf_sz > 256 * 1024)
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

        ctx->decompression_ctx.buf_ptr = *buf;

        return 0;
    }

    return 1;
}
