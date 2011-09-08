#include "vpx_io/vpxio.h"
#include <string.h>
#include "vpx_ports/mem_ops.h"

int write_y4m_file_header(struct vpxio_ctx *output_ctx)
{
    char buffer[128];

    if (!output_ctx->decompression_ctx.single_file)
    {
        fprintf(stderr, "YUV4MPEG2 not supported with output patterns,"
                " try --i420 or --yv12.\n");
        return 1;
    }

    /*Note: We can't output an aspect ratio here because IVF doesn't
    store one, and neither does VP8.
    That will have to wait until these tools support WebM natively.*/

    sprintf(buffer, "YUV4MPEG2 C%s W%u H%u F%u:%u I%c\n",
            "420jpeg", output_ctx->width, output_ctx->height,
            output_ctx->framerate.num, output_ctx->framerate.den, 'p');
    out_put(output_ctx->file, (unsigned char *)buffer, strlen(buffer),
            output_ctx->decompression_ctx.do_md5);
    return 0;
}
int vpxio_read_open_raw_file_y4m(struct vpxio_ctx *input_ctx)
{
    input_ctx->mode = SRC;

    if (y4m_input_open(&input_ctx->compression_ctx.y4m_in, input_ctx->file,
                       input_ctx->compression_ctx.detect.buf, 4) >= 0)
    {
        input_ctx->file_type = FILE_TYPE_Y4M;
        input_ctx->width = input_ctx->compression_ctx.y4m_in.pic_w;
        input_ctx->height = input_ctx->compression_ctx.y4m_in.pic_h;
        input_ctx->framerate.num = input_ctx->compression_ctx.y4m_in.fps_n;
        input_ctx->framerate.den = input_ctx->compression_ctx.y4m_in.fps_d;
        input_ctx->fourcc = 0x30323449;
    }
    else
        fprintf(stderr, "Unsupported Y4M stream.\n");

    return 0;
}
int vpxio_read_img_y4m(struct vpxio_ctx *input_ctx, vpx_image_t *img_raw)
{
    if (y4m_input_fetch_frame(&input_ctx->compression_ctx.y4m_in,
                              input_ctx->file, img_raw) < 1)
        return 0;
    else
        return 1;
}
int vpxio_write_img_y4m(struct vpxio_ctx *output_ctx, vpx_image_t *img,
                        uint8_t *buf)
{
    unsigned int y;

    out_put(output_ctx->file, (unsigned char *)"FRAME\n", 6,
            output_ctx->decompression_ctx.do_md5);

    buf = img->planes[VPX_PLANE_Y];

    for (y = 0; y < img->d_h; y++)
    {
        out_put(output_ctx->file, buf, img->d_w,
                output_ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_Y];
    }

    buf = img->planes[output_ctx->decompression_ctx.flipuv?VPX_PLANE_V:VPX_PLANE_U];

    for (y = 0; y < (1 + img->d_h) / 2; y++)
    {
        out_put(output_ctx->file, buf, (1 + img->d_w) / 2,
                output_ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_U];
    }

    buf = img->planes[output_ctx->decompression_ctx.flipuv?VPX_PLANE_U:VPX_PLANE_V];

    for (y = 0; y < (1 + img->d_h) / 2; y++)
    {
        out_put(output_ctx->file, buf, (1 + img->d_w) / 2,
                output_ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_V];
    }

    return 0;
}