#include "vpx_io/vpxio.h"
#include "vpx_io/vpxio_intrn.h"
#include <string.h>
#include "vpx_ports/mem_ops.h"

int file_is_y4m(struct vpxio_ctx *ctx)
{
    ctx->compression_ctx.detect.buf_read =
        fread(ctx->compression_ctx.detect.buf, 1, 4, ctx->file);
    ctx->compression_ctx.detect.position = 0;

    ctx->timebase.den = 1000;
    ctx->timebase.num = 1;

    if (memcmp(&ctx->compression_ctx.detect, "YUV4", 4) == 0)
        return 1;
    else
        rewind(ctx->file);

    return 0;
}
int write_y4m_file_header(struct vpxio_ctx *ctx)
{
    char buffer[128];

    if (!ctx->decompression_ctx.single_file)
    {
        fprintf(stderr, "YUV4MPEG2 not supported with output patterns,"
                " try --i420 or --yv12.\n");
        return 1;
    }

    /*Note: We can't output an aspect ratio here because IVF doesn't
    store one, and neither does VP8.
    That will have to wait until these tools support WebM natively.*/

    sprintf(buffer, "YUV4MPEG2 C%s W%u H%u F%u:%u I%c\n",
            "420jpeg", ctx->width, ctx->height,
            ctx->framerate.num, ctx->framerate.den, 'p');
    out_put(ctx->file, (unsigned char *)buffer, strlen(buffer),
            ctx->decompression_ctx.do_md5);
    return 0;
}
int vpxio_read_open_raw_file_y4m(struct vpxio_ctx *ctx)
{
    ctx->mode = SRC;

    if (y4m_input_open(&ctx->compression_ctx.y4m_in, ctx->file,
                       ctx->compression_ctx.detect.buf, 4) >= 0)
    {
        ctx->file_type = FILE_TYPE_Y4M;
        ctx->width = ctx->compression_ctx.y4m_in.pic_w;
        ctx->height = ctx->compression_ctx.y4m_in.pic_h;
        ctx->framerate.num = ctx->compression_ctx.y4m_in.fps_n;
        ctx->framerate.den = ctx->compression_ctx.y4m_in.fps_d;
        ctx->fourcc = 0x30323449;
    }
    else
        fprintf(stderr, "Unsupported Y4M stream.\n");

    return 0;
}
struct vpxio_ctx* vpxio_open_y4m_dst(struct vpxio_ctx *ctx,
                       const char *outfile_pattern,
                       int width, int height,
                       struct vpx_rational framerate)
{
    ctx = vpxio_init_ctx(ctx);

    ctx->width = width;
    ctx->height = height;
    ctx->decompression_ctx.outfile_pattern = outfile_pattern;
    ctx->framerate.den = framerate.den;
    ctx->framerate.num = framerate.num;
    ctx->file_type = FILE_TYPE_Y4M;
    ctx->mode = DST;

    return ctx;
}
int vpxio_read_img_y4m(struct vpxio_ctx *ctx, vpx_image_t *img_raw)
{
    if (y4m_input_fetch_frame(&ctx->compression_ctx.y4m_in,
                              ctx->file, img_raw) < 1)
        return 0;
    else
        return 1;
}