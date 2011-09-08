#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "md5_utils.h"
#include "tools_common.h"
#include "vpx_config.h"
#include "vpx_version.h"
#include "y4minput.h"
#include "vpx/vpx_encoder.h"
#include "vpx_io/vpxio.h"
#include "vpx_io/vpxio_intrn.h"
#include "vpx_io/vpxio_i420.h"
#include "vpx_io/vpxio_ivf.h"
#include "vpx_io/vpxio_raw.h"
#include "vpx_io/vpxio_webm.h"
#include "vpx_io/vpxio_y4m.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/mem_ops.h"

int generate_filename(const char *pattern,
                      char *out,
                      size_t q_len,
                      unsigned int d_w,
                      unsigned int d_h,
                      unsigned int frame_in)
{
    const char *p = pattern;
    char *q = out;

    do
    {
        char *next_pat = strchr(p, '%');

        if (p == next_pat)
        {
            size_t pat_len;

            // parse the pattern
            q[q_len - 1] = '\0';

            switch(p[1])
            {
            case 'w': snprintf(q, q_len - 1, "%d", d_w); break;
            case 'h': snprintf(q, q_len - 1, "%d", d_h); break;
            case '1': snprintf(q, q_len - 1, "%d", frame_in); break;
            case '2': snprintf(q, q_len - 1, "%02d", frame_in); break;
            case '3': snprintf(q, q_len - 1, "%03d", frame_in); break;
            case '4': snprintf(q, q_len - 1, "%04d", frame_in); break;
            case '5': snprintf(q, q_len - 1, "%05d", frame_in); break;
            case '6': snprintf(q, q_len - 1, "%06d", frame_in); break;
            case '7': snprintf(q, q_len - 1, "%07d", frame_in); break;
            case '8': snprintf(q, q_len - 1, "%08d", frame_in); break;
            case '9': snprintf(q, q_len - 1, "%09d", frame_in); break;
            default:
                printf("Unrecognized pattern %%%c\n", p[1]);
                return 0;
            }

            pat_len = strlen(q);

            if (pat_len >= q_len - 1)
            {
                printf("Output filename too long.\n");
                return 0;
            }

            q += pat_len;
            p += 2;
            q_len -= pat_len;
        }
        else
        {
            size_t copy_len;

            // copy the next segment
            if (!next_pat)
                copy_len = strlen(p);
            else
                copy_len = next_pat - p;

            if (copy_len >= q_len - 1)
            {
                printf("Output filename too long.\n");
                return 0;
            }

            memcpy(q, p, copy_len);
            q[copy_len] = '\0';
            q += copy_len;
            p += copy_len;
            q_len -= copy_len;
        }
    }
    while (*p);

    return 1;
}

void *out_open(const char *out_fn, int do_md5)
{
    void *out = NULL;

    if (do_md5 != FRAME_MD5 && do_md5 != NO_MD5 && do_md5 != FRAME_OUT_MD5)
    {
#if CONFIG_MD5
        MD5Context *md5_ctx = out = malloc(sizeof(MD5Context));
        //(void)out_fn;
        MD5Init(md5_ctx);
#endif
    }
    else
    {
        out = strcmp("-", out_fn) ? fopen(out_fn, "wb")
                     : set_binary_mode(stdout);

        if (!out)
        {
            fprintf(stderr, "Failed to output file");
            exit(EXIT_FAILURE);
        }
    }

    return out;
}
void out_put(void *out, const uint8_t *buf, unsigned int len, int do_md5)
{
    if (do_md5 != FRAME_MD5 && do_md5 != NO_MD5 && do_md5 != FRAME_OUT_MD5)
    {
#if CONFIG_MD5
        MD5Update((MD5Context *) out, buf, len);
#endif
    }
    else
    {
        fwrite(buf, 1, len, (FILE *)out);
    }
}
void out_close(void *out, const char *out_fn, int do_md5)
{
    if (do_md5 != FRAME_MD5 && do_md5 != NO_MD5 && do_md5 != FRAME_OUT_MD5)
    {
#if CONFIG_MD5
        uint8_t md5[16];
        int i;

        MD5Final(md5, out);
        free(out);

        for (i = 0; i < 16; i++)
            printf("%02x", md5[i]);

        printf("  %s\n", out_fn);
#endif
    }
    else
    {
        fclose(out);
    }
}
int vpxio_open_img_file(struct vpxio_ctx *ctx)
{
    char out_fn[PATH_MAX];

    if (ctx->file != NULL)
        fclose(ctx->file);

    ctx->decompression_ctx.outfile_pattern =
        ctx->decompression_ctx.outfile_pattern ?
        ctx->decompression_ctx.outfile_pattern : "-";
    ctx->decompression_ctx.single_file = 1;
    {
        const char *p = ctx->decompression_ctx.outfile_pattern;

        do
        {
            p = strchr(p, '%');

            if (p && p[1] >= '1' && p[1] <= '9')
            {
                ctx->decompression_ctx.single_file = 0;
                break;
            }

            if (p)
                p++;
        }
        while (p);
    }

    if (!ctx->decompression_ctx.noblit)
    {
        if (ctx->decompression_ctx.single_file)
        {
            generate_filename(ctx->decompression_ctx.outfile_pattern,
                              out_fn, PATH_MAX - 1, ctx->width,
                              ctx->height, 0);

            ctx->file_name = out_fn;

            if (ctx->file_name)
                ctx->file = out_open(ctx->file_name,
                                            ctx->decompression_ctx.do_md5);

            if (!ctx->file)
            {
                printf("Failed to open output file: %s", ctx->file_name);
                return -1;
            }
        }
    }

    return 0;
}
int vpxio_get_width(struct vpxio_ctx *ctx)
{
    return ctx->width;
}
int vpxio_get_height(struct vpxio_ctx *ctx)
{
    return ctx->height;
}
const char *vpxio_get_file_name(struct vpxio_ctx *ctx)
{
    return ctx->file_name;
}
video_file_type_t vpxio_get_file_type(struct vpxio_ctx *ctx)
{
    return ctx->file_type;
}
unsigned int vpxio_get_fourcc(struct vpxio_ctx *ctx)
{
    return ctx->fourcc;
}
int vpxio_get_use_i420(struct vpxio_ctx *ctx)
{
    int arg_use_i420 = 0;

    switch (ctx->fourcc)
    {
    case 0x32315659:
        arg_use_i420 = 0;
        break;
    case 0x30323449:
        arg_use_i420 = 1;
        break;
    default:
        fprintf(stderr, "Unsupported fourcc (%08x) in IVF\n", ctx->fourcc);
    }

    return arg_use_i420;
}
struct vpx_rational vpxio_get_framerate(struct vpxio_ctx *ctx)
{
    return ctx->framerate;
}
int vpxio_set_outfile_pattern(struct vpxio_ctx *ctx,
                              const char *outfile_pattern)
{
    ctx->decompression_ctx.outfile_pattern = outfile_pattern;
    return 0;
}
int vpxio_set_file_name(struct vpxio_ctx *ctx, char *file_name)
{
    ctx->file_name = file_name;
    return 0;
}
int vpxio_set_debug(struct vpxio_ctx *ctx, int debug)
{
    ctx->compression_ctx.ebml.debug = debug;
    return 0;
}
int vpxio_set_md5(struct vpxio_ctx *ctx, do_md5_t md5_value)
{
    ctx->decompression_ctx.do_md5 = md5_value;
    return 0;
}
int vpxio_write_file_header(struct vpxio_ctx *ctx)
{
    if (ctx->file_type == FILE_TYPE_IVF)
        return write_ivf_file_header(ctx);

    if (ctx->file_type == FILE_TYPE_WEBM)
        return write_webm_file_header(ctx);

    if (ctx->file_type == FILE_TYPE_Y4M)
        return write_y4m_file_header(ctx);

    return 0;
}
struct vpxio_ctx* vpxio_init_ctx(struct vpxio_ctx *ctx)
{
    ctx = malloc(sizeof(struct vpxio_ctx));

    ctx->file = NULL;
    ctx->file_name = NULL;
    ctx->file_type = 0;
    ctx->fourcc = 0;
    ctx->width = 0;
    ctx->height = 0;
    ctx->frame_cnt = 0;
    ctx->mode = NONE;
    ctx->framerate.den = 1;
    ctx->framerate.num = 30;
    ctx->timebase.den = 1000;
    ctx->timebase.num = 1;

    ctx->compression_ctx.ebml.last_pts_ms = -1;
    ctx->compression_ctx.img_raw_ptr = NULL;
    memset(&ctx->compression_ctx.ebml, 0,
           sizeof ctx->compression_ctx.ebml);
    ctx->compression_ctx.hash = 0;
    ctx->file_type = FILE_TYPE_Y4M;

    ctx->decompression_ctx.do_md5 = NO_MD5;
    ctx->decompression_ctx.noblit = 0;
    ctx->decompression_ctx.outfile_pattern = 0;
    ctx->decompression_ctx.single_file = 0;
    ctx->decompression_ctx.webm_ctx.chunk = 0;
    ctx->decompression_ctx.webm_ctx.chunks = 0;
    ctx->decompression_ctx.webm_ctx.infile = NULL;
    ctx->decompression_ctx.webm_ctx.kind = NO_FILE;
    ctx->decompression_ctx.webm_ctx.nestegg_ctx = 0;
    ctx->decompression_ctx.webm_ctx.pkt = 0;
    ctx->decompression_ctx.webm_ctx.video_track = 0;
    ctx->decompression_ctx.buf_ptr = NULL;

    return ctx;
}
struct vpxio_ctx* vpxio_open_src(struct vpxio_ctx *ctx, 
                                 const char *input_file_name)
{
    ctx = vpxio_init_ctx(ctx);
    ctx->file_name = input_file_name;

    ctx->file = strcmp(ctx->file_name, "-") ?
                      fopen(ctx->file_name, "rb") : set_binary_mode(stdin);
    ctx->decompression_ctx.webm_ctx.infile = ctx->file;

    if (!ctx->file)
    {
        return NULL;
    }

    ctx->mode = SRC;

    if (file_is_ivf(ctx))
    {
        ctx->file_type = FILE_TYPE_IVF;
        ctx->decompression_ctx.webm_ctx.kind = FILE_TYPE_IVF;
        ctx->compression_ctx.detect.position = 4;
        return ctx;
    }
    else if (file_is_webm(ctx))
    {
        ctx->file_type = FILE_TYPE_WEBM;
        ctx->decompression_ctx.webm_ctx.kind = FILE_TYPE_WEBM;

        if (webm_guess_framerate(ctx))
        {
            fprintf(stderr, "Failed to guess framerate -- error parsing "
                    "webm file?\n");
            return ctx;
        }

        return ctx;
    }
    else if (file_is_raw(ctx))
    {
        ctx->file_type = FILE_TYPE_RAW_PKT;
        ctx->decompression_ctx.webm_ctx.kind = FILE_TYPE_RAW_PKT;
        return ctx;
    }
    else if (file_is_y4m(ctx))
    {
        vpxio_read_open_raw_file_y4m(ctx);
        return ctx;
    }

    fclose(ctx->file);
    free(ctx);
    return NULL;
}
int vpxio_close(struct vpxio_ctx *ctx)
{
    if (ctx->file)
    {
        if (ctx->mode == SRC)
        {

            if (ctx->compression_ctx.img_raw_ptr != NULL)
                vpx_img_free(ctx->compression_ctx.img_raw_ptr);

            if (ctx->decompression_ctx.webm_ctx.nestegg_ctx)
                nestegg_destroy(ctx->decompression_ctx.webm_ctx.nestegg_ctx);

            if (ctx->file_type == FILE_TYPE_Y4M)
                y4m_input_close(&ctx->compression_ctx.y4m_in);

            if (ctx->decompression_ctx.webm_ctx.kind == FILE_TYPE_IVF)
                free(ctx->decompression_ctx.buf_ptr);
                fclose(ctx->file);
                free(ctx);
                return 0;
        }
        else
        {

            if (ctx->file_type == FILE_TYPE_WEBM)
            {
                vpxio_write_close_enc_file_webm(ctx);
                fclose(ctx->file);
                free(ctx);
                return 0;
            }

            if (ctx->file_type == FILE_TYPE_IVF)
            {
                vpxio_write_close_enc_file_ivf(ctx);
                fclose(ctx->file);
                free(ctx);
                return 0;
            }

            if (ctx->decompression_ctx.single_file &&
                    !ctx->decompression_ctx.noblit)
            {
                out_close(ctx->file, ctx->file_name,
                          ctx->decompression_ctx.do_md5);
                free(ctx);
                return 0;
            }

            free(ctx);
        }

        return 1;

    }
    else
        return 1;
}
int vpxio_read_pkt(struct vpxio_ctx *ctx,
                   uint8_t **buf,
                   size_t *buf_sz,
                   size_t *buf_alloc_sz)
{
    ctx->frame_cnt = ctx->frame_cnt + 1;

    if (ctx->decompression_ctx.webm_ctx.kind == FILE_TYPE_WEBM)
        return vpxio_read_pkt_webm(ctx, buf, buf_sz, buf_alloc_sz);
    /* For both the raw and ivf formats, the frame size is the first 4 bytes
    * of the frame header. We just need to special case on the header
    * size.
    */
    else if (ctx->decompression_ctx.webm_ctx.kind == FILE_TYPE_IVF)
        return vpxio_read_pkt_ivf(ctx, buf, buf_sz, buf_alloc_sz);
    else if (ctx->decompression_ctx.webm_ctx.kind == FILE_TYPE_RAW_PKT)
        return vpxio_read_pkt_raw(ctx, buf, buf_sz, buf_alloc_sz);

    return 1;
}

int vpxio_read_img(struct vpxio_ctx *ctx, vpx_image_t *img_raw)
{
    if (ctx->frame_cnt == 0)
    {

        if (ctx->file_type == FILE_TYPE_Y4M)
            /*The Y4M reader does its own allocation.
            Just initialize this here to avoid problems if we never read any
            frames.*/
            memset(img_raw, 0, sizeof(img_raw));
        else
        {
            ctx->compression_ctx.img_raw_ptr = img_raw;
            vpx_img_alloc(img_raw, vpxio_get_use_i420(ctx) ?
                          VPX_IMG_FMT_I420 : VPX_IMG_FMT_YV12, ctx->width,
                          ctx->height, 1);
        }
    }

    ctx->frame_cnt = ctx->frame_cnt + 1;

    if (ctx->file_type == FILE_TYPE_Y4M)
        return vpxio_read_img_y4m(ctx, img_raw);
    else if (ctx->file_type == FILE_TYPE_IVF)
        return vpxio_read_img_ivf(ctx, img_raw);
    else if (ctx->file_type == FILE_TYPE_I420)
        return vpxio_read_img_raw(ctx, img_raw);

    return 1;
}
int vpxio_write_pkt(struct vpxio_ctx *ctx, const vpx_codec_cx_pkt_t *pkt)
{
    if (ctx->frame_cnt == 0)
    {
        ctx->fourcc = MAKEFOURCC('V', 'P', '8', '0');
        vpxio_write_file_header(ctx);
    }

    ctx->frame_cnt = ctx->frame_cnt + 1;

    if (ctx->file_type == FILE_TYPE_WEBM)
        vpxio_write_pkt_webm(ctx, pkt);
    else
        vpxio_write_pkt_ivf(ctx, pkt);

    return pkt->data.raw.sz;
}
int vpxio_write_img(struct vpxio_ctx *ctx, vpx_image_t *img)
{
    char out_fn[PATH_MAX];
    uint8_t *buf = NULL;
    int y;
    int flipuv = 0;

    if(ctx->file_type == FILE_TYPE_YV12)
        flipuv = 1;

    if (ctx->frame_cnt == 0)
        vpxio_open_img_file(ctx);

    if (!ctx->decompression_ctx.single_file)
    {
        size_t len = sizeof(out_fn) - 1;
        out_fn[len] = '\0';
        generate_filename(ctx->decompression_ctx.outfile_pattern,
                          out_fn, len - 1, img->d_w, img->d_h,
                          ctx->frame_cnt);

        ctx->file_name = out_fn;

        if (ctx->file_name)
            ctx->file = out_open(ctx->file_name,
                                        ctx->decompression_ctx.do_md5);

        if (!ctx->file)
        {
            printf("Failed to open output file: %s", ctx->file_name);
            return -1;
        }
    }

    if (ctx->decompression_ctx.do_md5 == FRAME_MD5 ||
        ctx->decompression_ctx.do_md5 == FRAME_FILE_MD5 ||
        ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
    {
        unsigned int plane, y;
        unsigned char  md5_sum[16];
        MD5Context     md5;
        int            i;

        MD5Init(&md5);

        for (plane = 0; plane < 3; plane++)
        {
            unsigned char *buf = img->planes[plane];

            for (y = 0; y < img->d_h >> (plane ? 1 : 0); y++)
            {
                MD5Update(&md5, buf, img->d_w >> (plane ? 1 : 0));
                buf += img->stride[plane];
            }
        }

        MD5Final(md5_sum, &md5);

        for (i = 0; i < 16; i++)
        {
            if (ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
                fprintf(ctx->file, "%02x", md5_sum[i]);

            if (ctx->decompression_ctx.do_md5 == FRAME_MD5 ||
                    ctx->decompression_ctx.do_md5 == FRAME_FILE_MD5)
                printf("%02x", md5_sum[i]);
        }

        if (ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
            fprintf(ctx->file, "  img-%dx%d-%04d.i420\n", img->d_w,
                    img->d_h,   ctx->frame_cnt);

        if (ctx->decompression_ctx.do_md5 == FRAME_MD5 ||
                ctx->decompression_ctx.do_md5 == FRAME_FILE_MD5)
            printf("  img-%dx%d-%04d.i420\n", img->d_w, img->d_h,
                   ctx->frame_cnt);

        if (ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
        {
            ctx->frame_cnt = ctx->frame_cnt + 1;
            return 0;
        }
    }

    if (ctx->frame_cnt == 0)
    {
        ctx->fourcc = MAKEFOURCC('I', '4', '2', '0');

        if (ctx->decompression_ctx.do_md5 < FILE_MD5)
            vpxio_write_file_header(ctx);
    }

    if (ctx->file_type == FILE_TYPE_Y4M)
        out_put(ctx->file, (unsigned char *)"FRAME\n", 6,
                ctx->decompression_ctx.do_md5);

    if (ctx->file_type == FILE_TYPE_IVF)
        write_ivf_frame_header_img(ctx->file, 0,
                                  (3 * img->d_h * img->d_w) / 2);

    buf = img->planes[VPX_PLANE_Y];

    for (y = 0; y < img->d_h; y++)
    {
        out_put(ctx->file, buf, img->d_w,
                ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_Y];
    }

    buf = img->planes[flipuv ? VPX_PLANE_V : VPX_PLANE_U];

    for (y = 0; y < (1 + img->d_h) / 2; y++)
    {
        out_put(ctx->file, buf, (1 + img->d_w) / 2,
                ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_U];
    }

    buf = img->planes[flipuv ? VPX_PLANE_U : VPX_PLANE_V];

    for (y = 0; y < (1 + img->d_h) / 2; y++)
    {
        out_put(ctx->file, buf, (1 + img->d_w) / 2,
                ctx->decompression_ctx.do_md5);
        buf += img->stride[VPX_PLANE_V];
    }

    if (!ctx->decompression_ctx.single_file)
        out_close(ctx->file, out_fn,
                  ctx->decompression_ctx.do_md5);

    ctx->frame_cnt = ctx->frame_cnt + 1;

    return 0;
}
