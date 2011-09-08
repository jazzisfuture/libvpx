/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


/* This is a simple program that reads ivf files and decodes them
 * using the new interface. Decoded frames are output as YV12 raw.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include "vpx_io/vpxio.h"
struct vpxio_ctx_t;

#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx_config.h"
#include "vpx/vpx_decoder.h"
#include "vpx_ports/vpx_timer.h"
#if CONFIG_VP8_DECODER
#include "vpx/vp8dx.h"
#endif

#if CONFIG_OS_SUPPORT
#if defined(_WIN32)
#include <io.h>
#define snprintf _snprintf
#define isatty   _isatty
#define fileno   _fileno
#else
#include <unistd.h>
#endif
#endif

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

static const char *exec_name;

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

#include "args.h"
static const arg_def_t codecarg = ARG_DEF(NULL, "codec", 1,
                                  "Codec to use");
static const arg_def_t use_yv12 = ARG_DEF(NULL, "yv12", 0,
                                  "Output raw YV12 frames");
static const arg_def_t use_i420 = ARG_DEF(NULL, "i420", 0,
                                  "Output raw I420 frames");
static const arg_def_t flipuvarg = ARG_DEF(NULL, "flipuv", 0,
                                   "Flip the chroma planes in the output");
static const arg_def_t noblitarg = ARG_DEF(NULL, "noblit", 0,
                                   "Don't process the decoded frames");
static const arg_def_t progressarg = ARG_DEF(NULL, "progress", 0,
                                     "Show progress after each frame decodes");
static const arg_def_t limitarg = ARG_DEF(NULL, "limit", 1,
                                  "Stop decoding after n frames");
static const arg_def_t postprocarg = ARG_DEF(NULL, "postproc", 0,
                                     "Postprocess decoded frames");
static const arg_def_t summaryarg = ARG_DEF(NULL, "summary", 0,
                                    "Show timing summary");
static const arg_def_t outputfile = ARG_DEF("o", "output", 1,
                                    "Output file name pattern (see below)");
static const arg_def_t threadsarg = ARG_DEF("t", "threads", 1,
                                    "Max threads to use");
static const arg_def_t verbosearg = ARG_DEF("v", "verbose", 0,
                                  "Show version string");
static const arg_def_t error_concealment = ARG_DEF(NULL, "error-concealment", 0,
                                       "Enable decoder error-concealment");


#if CONFIG_MD5
static const arg_def_t md5arg = ARG_DEF(NULL, "md5", 0,
                                        "Compute the MD5 sum of the decoded frame");
#endif
static const arg_def_t *all_args[] =
{
    &codecarg, &use_yv12, &use_i420, &flipuvarg, &noblitarg,
    &progressarg, &limitarg, &postprocarg, &summaryarg, &outputfile,
    &threadsarg, &verbosearg,
#if CONFIG_MD5
    &md5arg,
#endif
    &error_concealment,
    NULL
};

#if CONFIG_VP8_DECODER
static const arg_def_t addnoise_level = ARG_DEF(NULL, "noise-level", 1,
                                        "Enable VP8 postproc add noise");
static const arg_def_t deblock = ARG_DEF(NULL, "deblock", 0,
                                 "Enable VP8 deblocking");
static const arg_def_t demacroblock_level = ARG_DEF(NULL, "demacroblock-level", 1,
        "Enable VP8 demacroblocking, w/ level");
static const arg_def_t pp_debug_info = ARG_DEF(NULL, "pp-debug-info", 1,
                                       "Enable VP8 visible debug info");
static const arg_def_t pp_disp_ref_frame = ARG_DEF(NULL, "pp-dbg-ref-frame", 1,
                                       "Display only selected reference frame per macro block");
static const arg_def_t pp_disp_mb_modes = ARG_DEF(NULL, "pp-dbg-mb-modes", 1,
                                       "Display only selected macro block modes");
static const arg_def_t pp_disp_b_modes = ARG_DEF(NULL, "pp-dbg-b-modes", 1,
                                       "Display only selected block modes");
static const arg_def_t pp_disp_mvs = ARG_DEF(NULL, "pp-dbg-mvs", 1,
                                       "Draw only selected motion vectors");

static const arg_def_t *vp8_pp_args[] =
{
    &addnoise_level, &deblock, &demacroblock_level, &pp_debug_info,
    &pp_disp_ref_frame, &pp_disp_mb_modes, &pp_disp_b_modes, &pp_disp_mvs,
    NULL
};
#endif

static void usage_exit()
{
    int i;

    fprintf(stderr, "Usage: %s <options> filename\n\n"
            "Options:\n", exec_name);
    arg_show_usage(stderr, all_args);
#if CONFIG_VP8_DECODER
    fprintf(stderr, "\nVP8 Postprocessing Options:\n");
    arg_show_usage(stderr, vp8_pp_args);
#endif
    fprintf(stderr,
            "\nOutput File Patterns:\n\n"
            "  The -o argument specifies the name of the file(s) to "
            "write to. If the\n  argument does not include any escape "
            "characters, the output will be\n  written to a single file. "
            "Otherwise, the filename will be calculated by\n  expanding "
            "the following escape characters:\n"
            "\n\t%%w   - Frame width"
            "\n\t%%h   - Frame height"
            "\n\t%%<n> - Frame number, zero padded to <n> places (1..9)"
            "\n\n  Pattern arguments are only supported in conjunction "
            "with the --yv12 and\n  --i420 options. If the -o option is "
            "not specified, the output will be\n  directed to stdout.\n"
            );
    fprintf(stderr, "\nIncluded decoders:\n\n");

    for (i = 0; i < sizeof(ifaces) / sizeof(ifaces[0]); i++)
        fprintf(stderr, "    %-6s - %s\n",
                ifaces[i].name,
                vpx_codec_iface_name(ifaces[i].iface));

    exit(EXIT_FAILURE);
}

void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    usage_exit();
}


void show_progress(int frame_in, int frame_out, unsigned long dx_time)
{
    fprintf(stderr, "%d decoded frames/%d showed frames in %lu us (%.2f fps)\r",
            frame_in, frame_out, dx_time,
            (float)frame_out * 1000000.0 / (float)dx_time);
}

int main(int argc, const char **argv_)
{
    struct vpxio_ctx* input_ctx = NULL;
    struct vpxio_ctx* output_ctx = NULL;

    vpx_codec_ctx_t          decoder;
    char                  *fn = NULL;
    int                    i;
    uint8_t               *buf = NULL;
    size_t                 buf_sz = 0, buf_alloc_sz = 0;
    int                    frame_in = 0, frame_out = 0, flipuv = 0, noblit = 0;
    int                    do_md5 = 0, progress = 0;
    int                    stop_after = 0, postproc = 0, summary = 0, quiet = 1;
    int                    ec_enabled = 0;
    vpx_codec_iface_t       *iface = NULL;
    unsigned int           fourcc = 0;
    unsigned long          dx_time = 0;
    struct arg               arg;
    char                   **argv, **argi, **argj;
    const char             *outfile_pattern = 0;
    int                     use_y4m = 1;
    vpx_codec_dec_cfg_t     cfg = {0};
#if CONFIG_VP8_DECODER
    vp8_postproc_cfg_t      vp8_pp_cfg = {0};
    int                     vp8_dbg_color_ref_frame = 0;
    int                     vp8_dbg_color_mb_modes = 0;
    int                     vp8_dbg_color_b_modes = 0;
    int                     vp8_dbg_display_mv = 0;
#endif
    int                     frames_corrupted = 0;
    int                     dec_flags = 0;

    /* Parse command line */
    exec_name = argv_[0];
    argv = argv_dup(argc - 1, argv_ + 1);

    for (argi = argj = argv; (*argj = *argi); argi += arg.argv_step)
    {
        memset(&arg, 0, sizeof(arg));
        arg.argv_step = 1;

        if (arg_match(&arg, &codecarg, argi))
        {
            int j, k = -1;

            for (j = 0; j < sizeof(ifaces) / sizeof(ifaces[0]); j++)
                if (!strcmp(ifaces[j].name, arg.val))
                    k = j;

            if (k >= 0)
                iface = ifaces[k].iface;
            else
                die("Error: Unrecognized argument (%s) to --codec\n",
                    arg.val);
        }
        else if (arg_match(&arg, &outputfile, argi))
            outfile_pattern = arg.val;
        else if (arg_match(&arg, &use_yv12, argi))
        {
            use_y4m = 0;
            flipuv = 1;
        }
        else if (arg_match(&arg, &use_i420, argi))
        {
            use_y4m = 0;
            flipuv = 0;
        }
        else if (arg_match(&arg, &flipuvarg, argi))
            flipuv = 1;
        else if (arg_match(&arg, &noblitarg, argi))
            noblit = 1;
        else if (arg_match(&arg, &progressarg, argi))
            progress = 1;
        else if (arg_match(&arg, &limitarg, argi))
            stop_after = arg_parse_uint(&arg);
        else if (arg_match(&arg, &postprocarg, argi))
            postproc = 1;
        else if (arg_match(&arg, &md5arg, argi))
            do_md5 = 1;
        else if (arg_match(&arg, &summaryarg, argi))
            summary = 1;
        else if (arg_match(&arg, &threadsarg, argi))
            cfg.threads = arg_parse_uint(&arg);
        else if (arg_match(&arg, &verbosearg, argi))
            quiet = 0;

#if CONFIG_VP8_DECODER
        else if (arg_match(&arg, &addnoise_level, argi))
        {
            postproc = 1;
            vp8_pp_cfg.post_proc_flag |= VP8_ADDNOISE;
            vp8_pp_cfg.noise_level = arg_parse_uint(&arg);
        }
        else if (arg_match(&arg, &demacroblock_level, argi))
        {
            postproc = 1;
            vp8_pp_cfg.post_proc_flag |= VP8_DEMACROBLOCK;
            vp8_pp_cfg.deblocking_level = arg_parse_uint(&arg);
        }
        else if (arg_match(&arg, &deblock, argi))
        {
            postproc = 1;
            vp8_pp_cfg.post_proc_flag |= VP8_DEBLOCK;
        }
        else if (arg_match(&arg, &pp_debug_info, argi))
        {
            unsigned int level = arg_parse_uint(&arg);

            postproc = 1;
            vp8_pp_cfg.post_proc_flag &= ~0x7;

            if (level)
                vp8_pp_cfg.post_proc_flag |= level;
        }
        else if (arg_match(&arg, &pp_disp_ref_frame, argi))
        {
            unsigned int flags = arg_parse_int(&arg);
            if (flags)
            {
                postproc = 1;
                vp8_dbg_color_ref_frame = flags;
            }
        }
        else if (arg_match(&arg, &pp_disp_mb_modes, argi))
        {
            unsigned int flags = arg_parse_int(&arg);
            if (flags)
            {
                postproc = 1;
                vp8_dbg_color_mb_modes = flags;
            }
        }
        else if (arg_match(&arg, &pp_disp_b_modes, argi))
        {
            unsigned int flags = arg_parse_int(&arg);
            if (flags)
            {
                postproc = 1;
                vp8_dbg_color_b_modes = flags;
            }
        }
        else if (arg_match(&arg, &pp_disp_mvs, argi))
        {
            unsigned int flags = arg_parse_int(&arg);
            if (flags)
            {
                postproc = 1;
                vp8_dbg_display_mv = flags;
            }
        }
        else if (arg_match(&arg, &error_concealment, argi))
        {
            ec_enabled = 1;
        }

#endif
        else
            argj++;
    }

    /* Check for unrecognized options */
    for (argi = argv; *argi; argi++)
        if (argi[0][0] == '-' && strlen(argi[0]) > 1)
            die("Error: Unrecognized option %s\n", *argi);

    /* Handle non-option arguments */
    fn = argv[0];

    if (!fn)
        usage_exit();

    /* Open input file */
    if ((input_ctx = vpxio_open_src(input_ctx, argv[0])) == NULL)
    {
        printf("Failed to open input file: %s", argv[0]);
        return EXIT_FAILURE;
    }

    /* Open output file */
    if (use_y4m)
        output_ctx = vpxio_open_y4m_dst(output_ctx, outfile_pattern,
                                        vpxio_get_width(input_ctx),
                                        vpxio_get_height(input_ctx), 
                                        vpxio_get_framerate(input_ctx));
    else
        output_ctx = vpxio_open_raw_dst(output_ctx, 
                                        outfile_pattern, 
                                        vpxio_get_width(input_ctx), 
                                        vpxio_get_height(input_ctx), 
                                        flipuv?VPX_IMG_FMT_YV12:VPX_IMG_FMT_I420);

    /* Try to determine the codec from the fourcc. */
    for (i = 0; i < sizeof(ifaces) / sizeof(ifaces[0]); i++)
        if ((fourcc & ifaces[i].fourcc_mask) == ifaces[i].fourcc)
        {
            vpx_codec_iface_t  *ivf_iface = ifaces[i].iface;

            if (iface && iface != ivf_iface)
                fprintf(stderr, "Notice -- IVF header indicates codec: %s\n",
                        ifaces[i].name);
            else
                iface = ivf_iface;

            break;
        }

    dec_flags = (postproc ? VPX_CODEC_USE_POSTPROC : 0) |
                (ec_enabled ? VPX_CODEC_USE_ERROR_CONCEALMENT : 0);
    if (vpx_codec_dec_init(&decoder, iface ? iface :  ifaces[0].iface, &cfg,
                           dec_flags))
    {
        fprintf(stderr, "Failed to initialize decoder: %s\n", vpx_codec_error(&decoder));
        return EXIT_FAILURE;
    }

    if (!quiet)
        fprintf(stderr, "%s\n", decoder.name);

#if CONFIG_VP8_DECODER

    if (vp8_pp_cfg.post_proc_flag
        && vpx_codec_control(&decoder, VP8_SET_POSTPROC, &vp8_pp_cfg))
    {
        fprintf(stderr, "Failed to configure postproc: %s\n", vpx_codec_error(&decoder));
        return EXIT_FAILURE;
    }

    if (vp8_dbg_color_ref_frame
        && vpx_codec_control(&decoder, VP8_SET_DBG_COLOR_REF_FRAME, vp8_dbg_color_ref_frame))
    {
        fprintf(stderr, "Failed to configure reference block visualizer: %s\n", vpx_codec_error(&decoder));
        return EXIT_FAILURE;
    }

    if (vp8_dbg_color_mb_modes
        && vpx_codec_control(&decoder, VP8_SET_DBG_COLOR_MB_MODES, vp8_dbg_color_mb_modes))
    {
        fprintf(stderr, "Failed to configure macro block visualizer: %s\n", vpx_codec_error(&decoder));
        return EXIT_FAILURE;
    }

    if (vp8_dbg_color_b_modes
        && vpx_codec_control(&decoder, VP8_SET_DBG_COLOR_B_MODES, vp8_dbg_color_b_modes))
    {
        fprintf(stderr, "Failed to configure block visualizer: %s\n", vpx_codec_error(&decoder));
        return EXIT_FAILURE;
    }

    if (vp8_dbg_display_mv
        && vpx_codec_control(&decoder, VP8_SET_DBG_DISPLAY_MV, vp8_dbg_display_mv))
    {
        fprintf(stderr, "Failed to configure motion vector visualizer: %s\n", vpx_codec_error(&decoder));
        return EXIT_FAILURE;
    }
#endif

    /* Decode file */
    while (!vpxio_read_pkt(input_ctx, &buf, &buf_sz, &buf_alloc_sz))
    {
        vpx_codec_iter_t  iter = NULL;
        vpx_image_t    *img;
        struct vpx_usec_timer timer;
        int                   corrupted;

        vpx_usec_timer_start(&timer);

        if (vpx_codec_decode(&decoder, buf, buf_sz, NULL, 0))
        {
            const char *detail = vpx_codec_error_detail(&decoder);
            fprintf(stderr, "Failed to decode frame: %s\n", vpx_codec_error(&decoder));

            if (detail)
                fprintf(stderr, "  Additional information: %s\n", detail);

            goto fail;
        }

        vpx_usec_timer_mark(&timer);
        dx_time += vpx_usec_timer_elapsed(&timer);

        ++frame_in;

        if (vpx_codec_control(&decoder, VP8D_GET_FRAME_CORRUPTED, &corrupted))
        {
            fprintf(stderr, "Failed VP8_GET_FRAME_CORRUPTED: %s\n",
                    vpx_codec_error(&decoder));
            goto fail;
        }
        frames_corrupted += corrupted;

        if ((img = vpx_codec_get_frame(&decoder, &iter)))
            ++frame_out;

        if (progress)
            show_progress(frame_in, frame_out, dx_time);

        if (!noblit)
        {
            if (img)
                vpxio_write_img(output_ctx, img);
        }

        if (stop_after && frame_in >= stop_after)
            break;
    }

    if (summary || progress)
    {
        show_progress(frame_in, frame_out, dx_time);
        fprintf(stderr, "\n");
    }

    if (frames_corrupted)
        fprintf(stderr, "WARNING: %d frames corrupted.\n",frames_corrupted);

fail:

    if (vpx_codec_destroy(&decoder))
    {
        fprintf(stderr, "Failed to destroy decoder: %s\n", vpx_codec_error(&decoder));
        return EXIT_FAILURE;
    }

    vpxio_close(input_ctx);
    vpxio_close(output_ctx);
    free(argv);

    return frames_corrupted ? EXIT_FAILURE : EXIT_SUCCESS;
}
