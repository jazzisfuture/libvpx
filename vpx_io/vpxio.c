#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "y4minput.h"
#include "tools_common.h"
#include "vpx/vpx_encoder.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/mem_ops.h"
#include "vpx_version.h"
#include "nestegg/include/nestegg/nestegg.h"
#include "vpx/vpx_decoder.h"

#include "vpx_io/vpxio.h"
#include "vpx_io/vpxio_i420.h"
#include "vpx_io/vpxio_ivf.h"
#include "vpx_io/vpxio_raw.h"
#include "vpx_io/vpxio_webm.h"
#include "vpx_io/vpxio_y4m.h"

#include "vpx/vp8dx.h"
#include "md5_utils.h"
#include "vpx/vp8dx.h"
#include "vpx_config.h"

#define VP8_FOURCC (0x00385056)
#define IVF_FILE_HDR_SZ (32)

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

int generate_filename(const char *pattern, char *out, size_t q_len, unsigned int d_w, unsigned int d_h,unsigned int frame_in)
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

            switch (p[1])
            {
            case 'w':
                snprintf(q, q_len - 1, "%d", d_w);
                break;
            case 'h':
                snprintf(q, q_len - 1, "%d", d_h);
                break;
            case '1':
                snprintf(q, q_len - 1, "%d", frame_in);
                break;
            case '2':
                snprintf(q, q_len - 1, "%02d", frame_in);
                break;
            case '3':
                snprintf(q, q_len - 1, "%03d", frame_in);
                break;
            case '4':
                snprintf(q, q_len - 1, "%04d", frame_in);
                break;
            case '5':
                snprintf(q, q_len - 1, "%05d", frame_in);
                break;
            case '6':
                snprintf(q, q_len - 1, "%06d", frame_in);
                break;
            case '7':
                snprintf(q, q_len - 1, "%07d", frame_in);
                break;
            case '8':
                snprintf(q, q_len - 1, "%08d", frame_in);
                break;
            case '9':
                snprintf(q, q_len - 1, "%09d", frame_in);
                break;
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
        (void)out_fn;
        MD5Init(md5_ctx);
#endif
    }
    else
    {
        FILE *outfile = out = strcmp("-", out_fn) ? fopen(out_fn, "wb")
                              : set_binary_mode(stdout);

        if (!outfile)
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
int vpxio_open_img_file(struct vpxio_ctx *output_ctx)
{
    char out_fn[PATH_MAX];

    if (output_ctx->file != NULL)
        fclose(output_ctx->file);

    output_ctx->decompression_ctx.outfile_pattern = output_ctx->decompression_ctx.outfile_pattern ? output_ctx->decompression_ctx.outfile_pattern : "-";
    output_ctx->decompression_ctx.single_file = 1;
    {
        const char *p = output_ctx->decompression_ctx.outfile_pattern;

        do
        {
            p = strchr(p, '%');

            if (p && p[1] >= '1' && p[1] <= '9')
            {
                output_ctx->decompression_ctx.single_file = 0;
                break;
            }

            if (p)
                p++;
        }
        while (p);
    }

    if (!output_ctx->decompression_ctx.noblit)
    {
        if (output_ctx->decompression_ctx.single_file)
        {
            generate_filename(output_ctx->decompression_ctx.outfile_pattern, out_fn, PATH_MAX - 1,
                              output_ctx->width, output_ctx->height, 0);

            output_ctx->file_name = out_fn;

            if (output_ctx->file_name)
                output_ctx->file = out_open(output_ctx->file_name, output_ctx->decompression_ctx.do_md5);

            if (!output_ctx->file)
            {
                printf("Failed to open output file: %s", output_ctx->file_name);
                return -1;
            }
        }
    }

    return 0;
}
static int webm_guess_framerate(struct vpxio_ctx *input_ctx)
{
    unsigned int i;
    uint64_t     tstamp = 0;

    for (i = 0; tstamp < 1000000000 && i < 50;)
    {
        nestegg_packet *pkt;
        unsigned int track;

        if (nestegg_read_packet(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx, &pkt) <= 0)
            break;

        nestegg_packet_track(pkt, &track);

        if (track == input_ctx->decompression_ctx.webm_ctx.video_track)
        {
            nestegg_packet_tstamp(pkt, &tstamp);
            i++;
        }

        nestegg_free_packet(pkt);
    }

    if (nestegg_track_seek(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx, input_ctx->decompression_ctx.webm_ctx.video_track, 0))
        goto fail;

    input_ctx->framerate.num = (i - 1) * 1000000;
    input_ctx->framerate.den = tstamp / 1000;
    return 0;
fail:
    nestegg_destroy(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx);
    input_ctx->decompression_ctx.webm_ctx.nestegg_ctx = NULL;
    rewind(input_ctx->decompression_ctx.webm_ctx.infile);
    return 1;
}
static int nestegg_read_cb(void *buffer, size_t length, void *userdata)
{
    FILE *f = (FILE *)userdata;

    if (fread(buffer, 1, length, f) < length)
    {
        if (ferror(f))
            return -1;

        if (feof(f))
            return 0;
    }

    return 1;
}
static int nestegg_seek_cb(int64_t offset, int whence, void *userdata)
{
    switch (whence)
    {
    case NESTEGG_SEEK_SET:
        whence = SEEK_SET;
        break;
    case NESTEGG_SEEK_CUR:
        whence = SEEK_CUR;
        break;
    case NESTEGG_SEEK_END:
        whence = SEEK_END;
        break;
    };

    return fseek((FILE *)userdata, offset, whence) ? -1 : 0;
}
static int64_t nestegg_tell_cb(void *userdata)
{
    return ftell((FILE *)userdata);
}
int vpxio_get_width(struct vpxio_ctx *input_ctx)
{
    return input_ctx->width;
}
int vpxio_get_height(struct vpxio_ctx *input_ctx)
{
    return input_ctx->height;
}
const char *vpxio_get_file_name(struct vpxio_ctx *input_ctx)
{
    return input_ctx->file_name;
}
video_file_type_t vpxio_get_file_type(struct vpxio_ctx *input_ctx)
{
    return input_ctx->file_type;
}
unsigned int vpxio_get_fourcc(struct vpxio_ctx *input_ctx)
{
    return input_ctx->fourcc;
}
file_kind_t vpxio_get_webm_ctx_kind(struct vpxio_ctx *input_ctx)
{
    return input_ctx->decompression_ctx.webm_ctx.kind;
}
int vpxio_get_use_i420(struct vpxio_ctx *input_ctx)
{
    int arg_use_i420 = 0;

    switch (input_ctx->fourcc)
    {
    case 0x32315659:
        arg_use_i420 = 0;
        break;
    case 0x30323449:
        arg_use_i420 = 1;
        break;
    default:
        fprintf(stderr, "Unsupported fourcc (%08x) in IVF\n", input_ctx->fourcc);
    }

    return arg_use_i420;
}
struct vpx_rational vpxio_get_framerate(struct vpxio_ctx *input_ctx)
{
    return input_ctx->framerate;
}
int vpxio_set_outfile_pattern(struct vpxio_ctx *input_ctx, const char *outfile_pattern)
{
    input_ctx->decompression_ctx.outfile_pattern = outfile_pattern;
    return 0;
}
int vpxio_set_file_name(struct vpxio_ctx *input_ctx, char *file_name)
{
    input_ctx->file_name = file_name;
    return 0;
}
int vpxio_set_file_type(struct vpxio_ctx *input_ctx, video_file_type_t file_type)
{
    input_ctx->file_type = file_type;
    return 0;
}
int vpxio_set_debug(struct vpxio_ctx *input_ctx, int debug)
{
    input_ctx->compression_ctx.ebml.debug = debug;
    return 0;
}
int vpxio_set_md5(struct vpxio_ctx *input_ctx, do_md5_t md5_value)
{
    input_ctx->decompression_ctx.do_md5 = md5_value;
    return 0;
}
int vpxio_write_file_header(struct vpxio_ctx *output_ctx)
{
    if (output_ctx->file_type == FILE_TYPE_IVF)
        return write_ivf_file_header(output_ctx);

    if (output_ctx->file_type == FILE_TYPE_WEBM)
        return write_webm_file_header(output_ctx);

    if (output_ctx->file_type == FILE_TYPE_Y4M)
        return write_y4m_file_header(output_ctx);

    return 0;
}
int vpxio_init_ctx(struct vpxio_ctx *input_ctx)
{
    input_ctx->file = NULL;
    input_ctx->file_name = NULL;
    input_ctx->file_type = 0;
    input_ctx->fourcc = 0;
    input_ctx->width = 0;
    input_ctx->height = 0;
    input_ctx->frame_cnt = 0;
    input_ctx->mode = NONE;
    input_ctx->framerate.den = 1;
    input_ctx->framerate.num = 30;
    input_ctx->timebase.den = 1000;
    input_ctx->timebase.num = 1;

    input_ctx->compression_ctx.ebml.last_pts_ms = -1;
    input_ctx->compression_ctx.img_raw_ptr = NULL;
    memset(&input_ctx->compression_ctx.ebml, 0, sizeof input_ctx->compression_ctx.ebml);
    input_ctx->compression_ctx.hash = 0;
    input_ctx->file_type = FILE_TYPE_Y4M;

    input_ctx->decompression_ctx.do_md5 = NO_MD5;
    input_ctx->decompression_ctx.flipuv = 0;
    input_ctx->decompression_ctx.noblit = 0;
    input_ctx->decompression_ctx.outfile_pattern = 0;
    input_ctx->decompression_ctx.single_file = 0;
    input_ctx->decompression_ctx.webm_ctx.chunk = 0;
    input_ctx->decompression_ctx.webm_ctx.chunks = 0;
    input_ctx->decompression_ctx.webm_ctx.infile = NULL;
    input_ctx->decompression_ctx.webm_ctx.kind = NO_FILE;
    input_ctx->decompression_ctx.webm_ctx.nestegg_ctx = 0;
    input_ctx->decompression_ctx.webm_ctx.pkt = 0;
    input_ctx->decompression_ctx.webm_ctx.video_track = 0;
    input_ctx->decompression_ctx.buf_ptr = NULL;

    return 0;
}
int file_is_y4m(struct vpxio_ctx *input_ctx)
{
    input_ctx->compression_ctx.detect.buf_read = fread(input_ctx->compression_ctx.detect.buf, 1, 4, input_ctx->file);
    input_ctx->compression_ctx.detect.position = 0;

    input_ctx->timebase.den = 1000;
    input_ctx->timebase.num = 1;

    if (memcmp(&input_ctx->compression_ctx.detect, "YUV4", 4) == 0)
        return 1;
    else
        rewind(input_ctx->file);

    return 0;
}
int file_is_ivf(struct vpxio_ctx *input_ctx)
{
    char raw_hdr[IVF_FILE_HDR_SZ];
    int is_ivf = 0;

    input_ctx->compression_ctx.detect.buf_read = fread(input_ctx->compression_ctx.detect.buf, 1, 4, input_ctx->file);
    input_ctx->compression_ctx.detect.position = 0;

    if (memcmp(input_ctx->compression_ctx.detect.buf, "DKIF", 4) != 0)
    {
        rewind(input_ctx->file);
        return 0;
    }

    if (fread(raw_hdr + 4, 1, IVF_FILE_HDR_SZ - 4, input_ctx->file)
        == IVF_FILE_HDR_SZ - 4)
    {
        is_ivf = 1;

        if (mem_get_le16(raw_hdr + 4) != 0)
            fprintf(stderr, "Error: Unrecognized IVF version! This file may not"
                    " decode properly.");

        input_ctx->fourcc = mem_get_le32(raw_hdr + 8);
        input_ctx->width = mem_get_le16(raw_hdr + 12);
        input_ctx->height = mem_get_le16(raw_hdr + 14);
        input_ctx->framerate.num = mem_get_le32(raw_hdr + 16);
        input_ctx->framerate.den = mem_get_le32(raw_hdr + 20);
    }

    if (!is_ivf)
        rewind(input_ctx->file);

    return is_ivf;
}
int file_is_raw(struct vpxio_ctx *input_ctx)
{
    unsigned char buf[32];
    int is_raw = 0;
    vpx_codec_stream_info_t si;
    si.sz = sizeof(si);

    if (fread(buf, 1, 32, input_ctx->file) == 32)
    {
        int i;

        if (mem_get_le32(buf) < 256 * 1024 * 1024)
            for (i = 0; i < sizeof(ifaces) / sizeof(ifaces[0]); i++)
                if (!vpx_codec_peek_stream_info(ifaces[i].iface,
                                                buf + 4, 32 - 4, &si))
                {
                    is_raw = 1;
                    input_ctx->fourcc = ifaces[i].fourcc;
                    input_ctx->width = si.w;
                    input_ctx->height = si.h;
                    input_ctx->framerate.num = 30;
                    input_ctx->framerate.den = 1;
                    break;
                }
    }

    rewind(input_ctx->file);
    return is_raw;
}
int file_is_webm(struct vpxio_ctx *input_ctx)
{
    unsigned int i, n;
    int          track_type = -1;
    nestegg_io io = {nestegg_read_cb, nestegg_seek_cb, nestegg_tell_cb,
                     input_ctx->decompression_ctx.webm_ctx.infile
                    };
    nestegg_video_params params;

    if (nestegg_init(&input_ctx->decompression_ctx.webm_ctx.nestegg_ctx, io, NULL))
        goto fail;

    if (nestegg_track_count(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx, &n))
        goto fail;

    for (i = 0; i < n; i++)
    {
        track_type = nestegg_track_type(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx, i);

        if (track_type == NESTEGG_TRACK_VIDEO)
            break;
        else if (track_type < 0)
            goto fail;
    }

    if (nestegg_track_codec_id(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx, i) != NESTEGG_CODEC_VP8)
    {
        fprintf(stderr, "Not VP8 video, quitting.\n");
        exit(1);
    }

    input_ctx->decompression_ctx.webm_ctx.video_track = i;

    if (nestegg_track_video_params(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx, i, &params))
        goto fail;

    input_ctx->framerate.den = 0;
    input_ctx->framerate.num = 0;
    input_ctx->fourcc = VP8_FOURCC;
    input_ctx->width  = params.width;
    input_ctx->height = params.height;

    return 1;
fail:
    input_ctx->decompression_ctx.webm_ctx.nestegg_ctx = NULL;
    rewind(input_ctx->decompression_ctx.webm_ctx.infile);
    return 0;
}
int vpxio_open_src(struct vpxio_ctx *input_ctx, const char *input_file_name)
{
    //opens source files for reading currently handles
    //y4m, ivf, and webm, and raw(pkt) formatts
    //return 1 on success
    //return 0 on fail

    vpxio_init_ctx(input_ctx);
    input_ctx->file_name = input_file_name;

    input_ctx->file = strcmp(input_ctx->file_name, "-") ? fopen(input_ctx->file_name, "rb") : set_binary_mode(stdin);
    input_ctx->decompression_ctx.webm_ctx.infile = input_ctx->file;

    if (!input_ctx->file)
    {
        return 0;
    }

    input_ctx->mode = SRC;

    if (file_is_ivf(input_ctx))
    {
        input_ctx->file_type = FILE_TYPE_IVF;
        input_ctx->decompression_ctx.webm_ctx.kind = IVF_FILE;
        input_ctx->compression_ctx.detect.position = 4;
        return 1;
    }
    else if (file_is_webm(input_ctx))
    {
        input_ctx->file_type = FILE_TYPE_WEBM;
        input_ctx->decompression_ctx.webm_ctx.kind = WEBM_FILE;

        if (webm_guess_framerate(input_ctx))
        {
            fprintf(stderr, "Failed to guess framerate -- error parsing "
                    "webm file?\n");
            return 1;
        }

        return 1;
    }
    else if (file_is_raw(input_ctx))
    {
        input_ctx->file_type = FILE_TYPE_RAW;
        input_ctx->decompression_ctx.webm_ctx.kind = RAW_FILE;
        return 1;
    }
    else if (file_is_y4m(input_ctx))
    {
        vpxio_read_open_raw_file_y4m(input_ctx);
        return 1;
    }

    fclose(input_ctx->file);

    return 0;
}
int vpxio_open_i420_src(struct vpxio_ctx *input_ctx, const char *input_file_name, int width, int height)
{
    //opens source yuv file for reading
    //return 1 on success
    //return 0 on fail

    vpxio_init_ctx(input_ctx);
    input_ctx->file_name = input_file_name;

    input_ctx->file = strcmp(input_ctx->file_name, "-") ? fopen(input_ctx->file_name, "rb") : set_binary_mode(stdin);
    input_ctx->decompression_ctx.webm_ctx.infile = input_ctx->file;
    input_ctx->mode = SRC;
    input_ctx->width = width;
    input_ctx->height = height;

    input_ctx->fourcc = MAKEFOURCC('I', '4', '2', '0');

    if (!input_ctx->file)
        return 0;

    vpxio_read_open_file_i420(input_ctx, &input_ctx->width, &input_ctx->height);

    return 1;
}
int vpxio_open_ivf_dst(struct vpxio_ctx *output_ctx, const char *file_name, int width, int height, int framerate_den, int framerate_num)
{
    vpxio_init_ctx(output_ctx);
    output_ctx->file_name = file_name;
    output_ctx->decompression_ctx.outfile_pattern = file_name;

    output_ctx->file = strcmp(output_ctx->file_name, "-") ? fopen(output_ctx->file_name, "wb") : set_binary_mode(stdout);

    if (!output_ctx->file)
    {
        printf("Failed to open output file: %s", output_ctx->file_name);
        return 0;
    }

    output_ctx->mode = DST;
    output_ctx->file_name = file_name;
    output_ctx->file_type = FILE_TYPE_IVF;
    output_ctx->width = width;
    output_ctx->height = height;
    output_ctx->framerate.den = framerate_den;
    output_ctx->framerate.num = framerate_num;

    return 1;
}
int vpxio_open_webm_dst(struct vpxio_ctx *output_ctx, const char *file_name, int width, int height, int framerate_den, int framerate_num)
{
    vpxio_init_ctx(output_ctx);

    output_ctx->file_name = file_name;
    output_ctx->file = strcmp(output_ctx->file_name, "-") ? fopen(output_ctx->file_name, "wb") : set_binary_mode(stdout);

    if (!output_ctx->file)
    {
        printf("Failed to open output file: %s", output_ctx->file_name);
        return 0;
    }

    output_ctx->mode = DST;
    output_ctx->fourcc = MAKEFOURCC('V', 'P', '8', '0');
    output_ctx->file_type = FILE_TYPE_WEBM;
    output_ctx->width = width;
    output_ctx->height = height;
    output_ctx->framerate.den = framerate_den;
    output_ctx->framerate.num = framerate_num;

    return 1;
}
int vpxio_open_i420_dst(struct vpxio_ctx *output_ctx, const char *outfile_pattern, int width, int height)
{
    vpxio_init_ctx(output_ctx);

    output_ctx->width = width;
    output_ctx->height = height;
    output_ctx->decompression_ctx.outfile_pattern = outfile_pattern;
    output_ctx->file_type = FILE_TYPE_RAW;
    output_ctx->mode = DST;

    return 1;
}
int vpxio_open_y4m_dst(struct vpxio_ctx *output_ctx, const char *outfile_pattern, int width, int height, int framerate_den, int framerate_num)
{
    vpxio_init_ctx(output_ctx);

    output_ctx->width = width;
    output_ctx->height = height;
    output_ctx->decompression_ctx.outfile_pattern = outfile_pattern;
    output_ctx->framerate.den = framerate_den;
    output_ctx->framerate.num = framerate_num;
    output_ctx->file_type = FILE_TYPE_Y4M;
    output_ctx->mode = DST;

    return 1;
}
int vpxio_close(struct vpxio_ctx *input_ctx)
{
    if (input_ctx->file)
    {
        if (input_ctx->mode == SRC)
        {

            if (input_ctx->compression_ctx.img_raw_ptr != NULL)
                vpx_img_free(input_ctx->compression_ctx.img_raw_ptr);

            if (input_ctx->decompression_ctx.webm_ctx.nestegg_ctx)
                nestegg_destroy(input_ctx->decompression_ctx.webm_ctx.nestegg_ctx);

            if (input_ctx->file_type == FILE_TYPE_Y4M)
                y4m_input_close(&input_ctx->compression_ctx.y4m_in);

            if (input_ctx->decompression_ctx.webm_ctx.kind == IVF_FILE)
                free(input_ctx->decompression_ctx.buf_ptr);
            return fclose(input_ctx->file);
        }
        else
        {

            if (input_ctx->file_type == FILE_TYPE_WEBM)
            {
                vpxio_write_close_enc_file_webm(input_ctx);
                return fclose(input_ctx->file);
            }

            if (input_ctx->file_type == FILE_TYPE_IVF)
            {
                vpxio_write_close_enc_file_ivf(input_ctx);
                return fclose(input_ctx->file);
            }

            if (input_ctx->decompression_ctx.single_file && !input_ctx->decompression_ctx.noblit)
            {
                out_close(input_ctx->file, input_ctx->file_name, input_ctx->decompression_ctx.do_md5);
                return 0;
            }
        }

        return 1;

    }
    else
        return 1;
}
int vpxio_read_pkt(struct vpxio_ctx *input_ctx, uint8_t **buf, size_t *buf_sz, size_t *buf_alloc_sz)
{
    input_ctx->frame_cnt = input_ctx->frame_cnt + 1;

    if (input_ctx->decompression_ctx.webm_ctx.kind == WEBM_FILE)
        return vpxio_read_pkt_webm(input_ctx, buf, buf_sz, buf_alloc_sz);
    /* For both the raw and ivf formats, the frame size is the first 4 bytes
    * of the frame header. We just need to special case on the header
    * size.
    */
    else if (input_ctx->decompression_ctx.webm_ctx.kind == IVF_FILE)
        return vpxio_read_pkt_ivf(input_ctx, buf, buf_sz, buf_alloc_sz);
    else if (input_ctx->decompression_ctx.webm_ctx.kind == RAW_FILE)
        return vpxio_read_pkt_raw(input_ctx, buf, buf_sz, buf_alloc_sz);

    return 1;
}

int vpxio_read_img(struct vpxio_ctx *input_ctx, vpx_image_t *img_raw)
{
    if (input_ctx->frame_cnt == 0)
    {

        if (input_ctx->file_type == FILE_TYPE_Y4M)
            /*The Y4M reader does its own allocation.
            Just initialize this here to avoid problems if we never read any
            frames.*/
            memset(img_raw, 0, sizeof(img_raw));
        else
        {
            input_ctx->compression_ctx.img_raw_ptr = img_raw;
            vpx_img_alloc(img_raw, vpxio_get_use_i420(input_ctx) ? VPX_IMG_FMT_I420 : VPX_IMG_FMT_YV12, input_ctx->width, input_ctx->height, 1);
        }
    }

    input_ctx->frame_cnt = input_ctx->frame_cnt + 1;

    if (input_ctx->file_type == FILE_TYPE_Y4M)
        return vpxio_read_img_y4m(input_ctx, img_raw);
    else if (input_ctx->file_type == FILE_TYPE_IVF)
        return vpxio_read_img_ivf(input_ctx, img_raw);
    else if (input_ctx->file_type == FILE_TYPE_I420)
        return vpxio_read_img_i420(input_ctx, img_raw);

    return 1;
}
int vpxio_write_pkt(struct vpxio_ctx *output_ctx, const vpx_codec_cx_pkt_t *pkt)
{
    if (output_ctx->frame_cnt == 0)
    {
        output_ctx->fourcc = MAKEFOURCC('V', 'P', '8', '0');
        vpxio_write_file_header(output_ctx);
    }

    output_ctx->frame_cnt = output_ctx->frame_cnt + 1;

    if (output_ctx->file_type == FILE_TYPE_WEBM)
        vpxio_write_pkt_webm(output_ctx, pkt);
    else
        vpxio_write_pkt_ivf(output_ctx, pkt);

    return pkt->data.raw.sz;
}
int vpxio_write_img(struct vpxio_ctx *output_ctx, vpx_image_t *img)
{
    char out_fn[PATH_MAX];
    uint8_t *buf = NULL;

    if (output_ctx->frame_cnt == 0)
        vpxio_open_img_file(output_ctx);

    if (!output_ctx->decompression_ctx.single_file)
    {
        size_t len = sizeof(out_fn) - 1;
        out_fn[len] = '\0';
        generate_filename(output_ctx->decompression_ctx.outfile_pattern, out_fn, len - 1,
                          img->d_w, img->d_h, output_ctx->frame_cnt);

        output_ctx->file_name = out_fn;

        if (output_ctx->file_name)
            output_ctx->file = out_open(output_ctx->file_name, output_ctx->decompression_ctx.do_md5);

        if (!output_ctx->file)
        {
            printf("Failed to open output file: %s", output_ctx->file_name);
            return -1;
        }
    }

    if (output_ctx->decompression_ctx.do_md5 == FRAME_MD5 || output_ctx->decompression_ctx.do_md5 == FRAME_FILE_MD5 || output_ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
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
            if (output_ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
                fprintf(output_ctx->file, "%02x", md5_sum[i]);

            if (output_ctx->decompression_ctx.do_md5 == FRAME_MD5 || output_ctx->decompression_ctx.do_md5 == FRAME_FILE_MD5)
                printf("%02x", md5_sum[i]);
        }

        if (output_ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
            fprintf(output_ctx->file, "  img-%dx%d-%04d.i420\n", img->d_w, img->d_h,   //
                    output_ctx->frame_cnt);

        if (output_ctx->decompression_ctx.do_md5 == FRAME_MD5 || output_ctx->decompression_ctx.do_md5 == FRAME_FILE_MD5)
            printf("  img-%dx%d-%04d.i420\n", img->d_w, img->d_h,   //
                   output_ctx->frame_cnt);

        if (output_ctx->decompression_ctx.do_md5 == FRAME_OUT_MD5)
        {
            output_ctx->frame_cnt = output_ctx->frame_cnt + 1;
            return 0;
        }
    }

    if (output_ctx->frame_cnt == 0)
    {
        output_ctx->fourcc = MAKEFOURCC('I', '4', '2', '0');

        if (output_ctx->decompression_ctx.do_md5 < FILE_MD5)
            vpxio_write_file_header(output_ctx);
    }

    if (output_ctx->file_type == FILE_TYPE_RAW)
        vpxio_write_img_i420(output_ctx, img, buf);

    if (output_ctx->file_type == FILE_TYPE_Y4M)
        vpxio_write_img_y4m(output_ctx, img, buf);

    if (output_ctx->file_type == FILE_TYPE_IVF)
        vpxio_write_img_ivf(output_ctx, img, buf);


    if (!output_ctx->decompression_ctx.single_file)
        out_close(output_ctx->file, out_fn, output_ctx->decompression_ctx.do_md5);

    output_ctx->frame_cnt = output_ctx->frame_cnt + 1;

    return 0;
}
