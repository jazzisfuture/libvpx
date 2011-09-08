#include "vpx_io/vpxio.h"
#include <string.h>
#include "vpx_ports/mem_ops.h"

struct vpxio_ctx_t;

int vpxio_read_open_file_i420(struct vpxio_ctx *input_ctx,
                              unsigned int *width,
                              unsigned int *height)
{
    input_ctx->file_type = FILE_TYPE_I420;

    if (!width || !height)
    {
        fprintf(stderr, "Specify stream dimensions with --width (-w) "
                " and --height (-h).\n");
    }

    return 0;
}
int vpxio_read_img_i420(struct vpxio_ctx *input_ctx, vpx_image_t *img_raw)
{
    int plane = 0;
    int shortread = 0;

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
                shortread |= (fread(ptr + buf_position, 1, needed,
                                    input_ctx->file) < needed);
            }

            ptr += img_raw->stride[plane];
        }
    }

    return !shortread;
}
int vpxio_write_img_i420(struct vpxio_ctx *output_ctx,
                         vpx_image_t *img,
                         uint8_t *buf)
{
    unsigned int y;

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