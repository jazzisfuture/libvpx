#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "vpx_io/vpxio.h"
#include "vpx_io/vpxio_intrn.h"
#include "vpx_ports/mem_ops.h"
#include "tools_common.h"

struct vpxio_ctx* vpxio_open_raw_src(struct vpxio_ctx *ctx,
                        const char *input_file_name,
                        int width,
                        int height,
                        vpx_img_fmt_t img_fmt)
{
    ctx = vpxio_init_ctx(ctx);
    ctx->file_name = input_file_name;

    ctx->file = strcmp(ctx->file_name, "-") ?
                      fopen(ctx->file_name, "rb") : set_binary_mode(stdin);
    ctx->decompression_ctx.webm_ctx.infile = ctx->file;
    ctx->mode = SRC;
    ctx->width = width;
    ctx->height = height;

    if(img_fmt == VPX_IMG_FMT_YV12){
        ctx->fourcc = MAKEFOURCC('Y', 'V', '1', '2');
        ctx->file_type = FILE_TYPE_I420;
    }
    if(img_fmt == VPX_IMG_FMT_I420){
        ctx->fourcc = MAKEFOURCC('I', '4', '2', '0');
        ctx->file_type = FILE_TYPE_YV12;
    }

    if (!ctx->file || !width || !height || (img_fmt != VPX_IMG_FMT_YV12 && 
        img_fmt != VPX_IMG_FMT_I420))
    {

        if (!width || !height)
        {
            fprintf(stderr, "Specify stream dimensions with --width (-w) "
                " and --height (-h).\n");
        }
        if(img_fmt != VPX_IMG_FMT_YV12 && img_fmt != VPX_IMG_FMT_I420)
        {
            fprintf(stderr, "Specify stream format YV12, I420.\n");
        }

        free(ctx);
        return NULL;
    }

    return ctx;
}
struct vpxio_ctx* vpxio_open_raw_dst(struct vpxio_ctx *ctx,
                        const char *outfile_pattern,
                        int width,
                        int height,
                        vpx_img_fmt_t img_fmt)
{
    ctx = vpxio_init_ctx(ctx);

    ctx->width = width;
    ctx->height = height;
    ctx->decompression_ctx.outfile_pattern = outfile_pattern;
    if(img_fmt == VPX_IMG_FMT_YV12){
        ctx->file_type = FILE_TYPE_YV12;
    }
    else
        ctx->file_type = FILE_TYPE_I420;
    ctx->mode = DST;

    return ctx;
}
int vpxio_read_img_raw(struct vpxio_ctx *ctx, vpx_image_t *img_raw)
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
                shortread |= (fread(ptr + buf_position, 1, needed,
                                    ctx->file) < needed);
            }

            ptr += img_raw->stride[plane];
        }
    }

    return !shortread;
}
