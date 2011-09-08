#include <string.h>
#include <stdlib.h>
#include "vpx_io/vpxio.h"
#include "vpx_ports/mem_ops.h"

int write_ivf_file_header(struct vpxio_ctx *output_ctx)
{
    char header[32];

    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header + 4,  0);                             /* version */
    mem_put_le16(header + 6,  32);                            /* headersize */
    mem_put_le32(header + 8,  output_ctx->fourcc);            /* headersize */
    mem_put_le16(header + 12, output_ctx->width);             /* width */
    mem_put_le16(header + 14, output_ctx->height);            /* height */
    mem_put_le32(header + 16, output_ctx->framerate.num);     /* rate */
    mem_put_le32(header + 20, output_ctx->framerate.den);     /* scale */
    mem_put_le32(header + 24, output_ctx->frame_cnt);         /* length */
    mem_put_le32(header + 28, 0);                             /* unused */

    if (fwrite(header, 1, 32, output_ctx->file))
        return 0;
    else
        return 1;
}
int write_ivf_frame_header(FILE *outfile, const vpx_codec_cx_pkt_t *pkt)
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
int write_ivf_frame_header_dec(FILE *outfile,
                               uint64_t timeStamp,
                               uint32_t frameSize)
{
    char             header[12];

    mem_put_le32(header, frameSize);
    mem_put_le32(header + 4, 0);
    mem_put_le32(header + 8, 0);

    return 0;
}
int vpxio_read_open_raw_file_ivf(struct vpxio_ctx *input_ctx)
{
    input_ctx->mode = SRC;
    input_ctx->file_type = FILE_TYPE_IVF;

    switch (input_ctx->fourcc)
    {
    case 0x32315659:
        break;
    case 0x30323449:
        break;
    default:
        fprintf(stderr, "Unsupported fourcc (%08x) in IVF\n", input_ctx->fourcc);
    }

    return 0;
}

int vpxio_read_img_ivf(struct vpxio_ctx *input_ctx, vpx_image_t *img_raw)
{
    int plane = 0;
    int shortread = 0;

    char junk[IVF_FRAME_HDR_SZ];

    /* Skip the frame header. We know how big the frame should be. See
    * write_ivf_frame_header() for documentation on the frame header
    * layout.
    */
    if (fread(junk, 1, IVF_FRAME_HDR_SZ, input_ctx->file));

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
            const size_t left = input_ctx->compression_ctx.detect.buf_read -
                                input_ctx->compression_ctx.detect.position;

            if (left > 0)
            {
                const size_t more = (left < needed) ? left : needed;
                memcpy(ptr, input_ctx->compression_ctx.detect.buf +
                       input_ctx->compression_ctx.detect.position, more);
                buf_position = more;
                needed -= more;
                input_ctx->compression_ctx.detect.position += more;
            }

            if (needed > 0)
            {
                shortread |= (fread(ptr + buf_position, 1,
                                    needed, input_ctx->file) < needed);
            }

            ptr += img_raw->stride[plane];
        }
    }

    return !shortread;
}
int vpxio_write_pkt_ivf(struct vpxio_ctx *output_ctx,
                        const vpx_codec_cx_pkt_t *pkt)
{
    write_ivf_frame_header(output_ctx->file, pkt);

    if (fwrite(pkt->data.frame.buf, 1,
               pkt->data.frame.sz, output_ctx->file));

    return 0;
}
int vpxio_write_close_enc_file_ivf(struct vpxio_ctx *output_ctx)
{
    if (!fseek(output_ctx->file, 0, SEEK_SET))
        write_ivf_file_header(output_ctx);

    return 0;
}
int vpxio_read_pkt_ivf(struct vpxio_ctx *input_ctx,
                       uint8_t **buf,
                       size_t *buf_sz,
                       size_t *buf_alloc_sz)
{
    char     raw_hdr[IVF_FRAME_HDR_SZ];
    size_t          new_buf_sz;

    if (fread(raw_hdr, IVF_FRAME_HDR_SZ, 1,
              input_ctx->decompression_ctx.webm_ctx.infile) != 1)
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

        if (input_ctx->decompression_ctx.webm_ctx.kind ==
                RAW_FILE && new_buf_sz > 256 * 1024)
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
        if (fread(*buf, 1, *buf_sz, input_ctx->decompression_ctx.webm_ctx.infile)
                != *buf_sz)
        {
            fprintf(stderr, "Failed to read full frame\n");
            return 1;
        }

        input_ctx->decompression_ctx.buf_ptr = *buf;

        return 0;
    }

    return 1;
}
int vpxio_write_img_ivf(struct vpxio_ctx *output_ctx,
                        vpx_image_t *img,
                        uint8_t *buf)
{
    unsigned int y;

    write_ivf_frame_header_dec((FILE *)output_ctx->file, 0,
                               (3 * img->d_h * img->d_w) / 2);

    buf = img->planes[VPX_PLANE_Y];

    for (y = 0; y < img->d_h; y++)
    {
        out_put(output_ctx->file, buf, img->d_w,
                output_ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_Y];
    }

    buf = img->planes[output_ctx->decompression_ctx.flipuv?VPX_PLANE_V:
                      VPX_PLANE_U];

    for (y = 0; y < (1 + img->d_h) / 2; y++)
    {
        out_put(output_ctx->file, buf, (1 + img->d_w) / 2,
                output_ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_U];
    }

    buf = img->planes[output_ctx->decompression_ctx.flipuv?VPX_PLANE_U:
                      VPX_PLANE_V];

    for (y = 0; y < (1 + img->d_h) / 2; y++)
    {
        out_put(output_ctx->file, buf, (1 + img->d_w) / 2,
                output_ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_V];
    }

    return 0;
}