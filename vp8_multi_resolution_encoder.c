/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * This is an example demonstrating multi-resolution encoding in VP8.
 * High-resolution input video is down-sampled to lower-resolutions. The
 * encoder then encodes the video and outputs multiple bitstreams with
 * different resolutions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "math.h"
#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include "vpx_ports/mem_ops.h"
#include "vpx_mem/vpx_mem.h"
#define interface (vpx_codec_vp8_cx())
#define fourcc    0x30385056

#define IVF_FILE_HDR_SZ  (32)
#define IVF_FRAME_HDR_SZ (12)

/* define how many encoders in total. For example, if input video is 1280x720
 * and NUM_ENCODES=3, output bit streams with resolutions: 1280x720, 640x360,
 * and 320x180.
 */
#define NUM_ENCODES 3

#if ENABLE_MULTI_RESOLUTION_ENCODING
#include "third_party/libyuv/include/libyuv/basic_types.h"
#include "third_party/libyuv/include/libyuv/scale.h"
#include "third_party/libyuv/include/libyuv/cpu_id.h"
extern int I420Scale(const uint8* src_y, int src_stride_y,
                     const uint8* src_u, int src_stride_u,
                     const uint8* src_v, int src_stride_v,
                     int src_width, int src_height,
                     uint8* dst_y, int dst_stride_y,
                     uint8* dst_u, int dst_stride_u,
                     uint8* dst_v, int dst_stride_v,
                     int dst_width, int dst_height,
                     FilterMode filtering);
#endif

static double vp8_mse2psnr(double Samples, double Peak, double Mse)
{
    double psnr;

    if ((double)Mse > 0.0)
        psnr = 10.0 * log10(Peak * Peak * Samples / Mse);
    else
        psnr = 60;      // Limit to prevent / 0

    if (psnr > 60)
        psnr = 60;

    return psnr;
}

static void die(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    if(fmt[strlen(fmt)-1] != '\n')
        printf("\n");
    exit(EXIT_FAILURE);
}

static void die_codec(vpx_codec_ctx_t *ctx, const char *s) {
    const char *detail = vpx_codec_error_detail(ctx);

    printf("%s: %s\n", s, vpx_codec_error(ctx));
    if(detail)
        printf("    %s\n",detail);
    exit(EXIT_FAILURE);
}

static int read_frame(FILE *f, vpx_image_t *img) {
    size_t nbytes, to_read;
    int    res = 1;

    to_read = img->w*img->h*3/2;
    nbytes = fread(img->planes[0], 1, to_read, f);
    if(nbytes != to_read) {
        res = 0;
        if(nbytes > 0)
            printf("Warning: Read partial frame. Check your width & height!\n");
    }
    return res;
}

static void write_ivf_file_header(FILE *outfile,
                                  const vpx_codec_enc_cfg_t *cfg,
                                  int frame_cnt) {
    char header[32];

    if(cfg->g_pass != VPX_RC_ONE_PASS && cfg->g_pass != VPX_RC_LAST_PASS)
        return;
    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header+4,  0);                   /* version */
    mem_put_le16(header+6,  32);                  /* headersize */
    mem_put_le32(header+8,  fourcc);              /* headersize */
    mem_put_le16(header+12, cfg->g_w);            /* width */
    mem_put_le16(header+14, cfg->g_h);            /* height */
    mem_put_le32(header+16, cfg->g_timebase.den); /* rate */
    mem_put_le32(header+20, cfg->g_timebase.num); /* scale */
    mem_put_le32(header+24, frame_cnt);           /* length */
    mem_put_le32(header+28, 0);                   /* unused */

    if(fwrite(header, 1, 32, outfile));
}

static void write_ivf_frame_header(FILE *outfile,
                                   const vpx_codec_cx_pkt_t *pkt)
{
    char             header[12];
    vpx_codec_pts_t  pts;

    if(pkt->kind != VPX_CODEC_CX_FRAME_PKT)
        return;

    pts = pkt->data.frame.pts;
    mem_put_le32(header, pkt->data.frame.sz);
    mem_put_le32(header+4, pts&0xFFFFFFFF);
    mem_put_le32(header+8, pts >> 32);

    if(fwrite(header, 1, 12, outfile));
}

int main(int argc, char **argv)
{
    FILE                *infile, *outfile[NUM_ENCODES];
    vpx_codec_ctx_t      codec[NUM_ENCODES];
    vpx_codec_enc_cfg_t  cfg[NUM_ENCODES];
    int                  frame_cnt[NUM_ENCODES];
    vpx_image_t          raw[NUM_ENCODES];
    vpx_codec_err_t      res[NUM_ENCODES];
    int                  i;
    long                 width;
    long                 height;
    int                  frame_avail;
    int                  got_data;
    int                  flags = 0;
    int                  arg_deadline = VPX_DL_REALTIME;

    int                  show_psnr = 0;
    uint64_t             psnr_sse_total[NUM_ENCODES] = {0};
    uint64_t             psnr_samples_total[NUM_ENCODES] = {0};
    double               psnr_totals[NUM_ENCODES][4] = {{0,0}};
    int                  psnr_count[NUM_ENCODES] = {0};

    // Set the required target bitrates for 3 resolutions.(0: 1280x720, 1: 640x360, 2: 320x180)
    unsigned int         target_bitrate[3]={1400, 500, 100};

    // Enter the frame rate of the input video
    int                  framerate = 30;

    /* Open files */
    if(argc!= (5+NUM_ENCODES))
        die("Usage: %s <width> <height> <infile> <outfile(s)> <output psnr?> \n", argv[0]);
    width = strtol(argv[1], NULL, 0);
    height = strtol(argv[2], NULL, 0);
    if(width < 16 || width%2 || height <16 || height%2)
        die("Invalid resolution: %ldx%ld", width, height);

#if ENABLE_MULTI_RESOLUTION_ENCODING
    printf("Using %s\n",vpx_codec_iface_name(interface));

    for (i=0; i< NUM_ENCODES; i++)
    {
        frame_cnt[i] = 0;
        if(!(outfile[i] = fopen(argv[i+4], "wb")))
            die("Failed to open %s for writing", argv[i+4]);
    }

    show_psnr = strtol(argv[NUM_ENCODES + 4], NULL, 0);

    /* Populate encoder configuration */
    for (i=0; i< NUM_ENCODES; i++)
    {
        res[i] = vpx_codec_enc_config_default(interface, &cfg[i], 0);
        if(res[i]) {
            printf("Failed to get config: %s\n", vpx_codec_err_to_string(res[i]));
            return EXIT_FAILURE;
        }
    }

    /* Update the default configuration with our settings */
    cfg[0].g_w = width;
    cfg[0].g_h = height;
    cfg[0].mr_down_sampling_factor = 2;

    /* Real time parameters */
    cfg[0].rc_dropframe_thresh = 0;
    cfg[0].rc_end_usage = VPX_CBR;
    cfg[0].rc_resize_allowed = 0;
    cfg[0].rc_min_quantizer = 4;
    cfg[0].rc_max_quantizer = 56;
    cfg[0].rc_undershoot_pct = 98;
    cfg[0].rc_overshoot_pct = 100;
    cfg[0].rc_buf_initial_sz = 500;
    cfg[0].rc_buf_optimal_sz = 600;
    cfg[0].rc_buf_sz = 1000;
    /* The higher, the more drop happens. Highest value=100 that means droping frame if buffer < 100% target rate.*/
    //cfg[0].rc_dropframe_thresh = 10;

    /* Enable error resilient mode */
    cfg[0].g_error_resilient = 1;
    cfg[0].g_lag_in_frames   = 0;

    //cfg[0].kf_mode           = VPX_KF_DISABLED;

    /* Disable automatic keyframe placement */
    cfg[0].kf_min_dist = cfg[0].kf_max_dist = 1000;

    cfg[0].rc_target_bitrate = target_bitrate[0];

    /* set fps
     * Note: In vpxenc, use different fps and g_timebase.den (default:1000).
     * For comparison test, use fps = 1/time_base in vpxenc to get same output.
     */
    cfg[0].g_timebase.num = 1;
    cfg[0].g_timebase.den = framerate;   // 50fps

    if(!vpx_img_alloc(&raw[0], VPX_IMG_FMT_I420, width, height, 1))
        die("Failed to allocate image", width, height);

    /* Allocate memory for storing mode info for next encoder to use */
    {
        int mb_rows = (cfg[0].g_w >>4) + ((cfg[0].g_w & 15)?1:0);
        int mb_cols = (cfg[0].g_h >>4) + ((cfg[0].g_h & 15)?1:0);

        cfg[0].mr_low_res_mode_info = (int *)vpx_calloc(mb_rows*mb_cols, 100);  //sizeof(LOWER_RES_INFO) = 80 (on 32bit or 64 bit)
        if(!cfg[0].mr_low_res_mode_info)
        {
            vpx_free(cfg[0].mr_low_res_mode_info);
            die("Failed to allocate memory for mr_low_res_mode_info\n");
        }

        cfg[0].mr_total_resoutions = NUM_ENCODES;
        cfg[0].mr_encoder_id = NUM_ENCODES-1;
    }

    for (i=1; i< NUM_ENCODES; i++)
    {
        memcpy(&cfg[i], &cfg[0], sizeof(vpx_codec_enc_cfg_t));

        // Setting for each spatial layer
        cfg[i].g_w = width>>i;
        cfg[i].g_h = height>>i;

        cfg[i].rc_target_bitrate = target_bitrate[i];

        cfg[i].mr_encoder_id = NUM_ENCODES-1-i;
        cfg[i].rc_max_quantizer = 56;
        cfg[i].mr_down_sampling_factor = 2;

        if(!vpx_img_alloc(&raw[i], VPX_IMG_FMT_I420, cfg[i].g_w, cfg[i].g_h, 1))
            die("Failed to allocate image", cfg[i].g_w, cfg[i].g_h);

        cfg[i].mr_low_res_mode_info = cfg[0].mr_low_res_mode_info;
    }

    /* Open input file for this encoding pass */
    if(!(infile = fopen(argv[3], "rb")))
        die("Failed to open %s for reading", argv[3]);

    for (i=0; i< NUM_ENCODES; i++)
    {
        write_ivf_file_header(outfile[i], &cfg[i], 0);
        /* Initialize codec */
        if(vpx_codec_enc_init(&codec[i], interface, &cfg[i], show_psnr ? VPX_CODEC_USE_PSNR : 0))
            die_codec(&codec[i], "Failed to initialize encoder");
    }

    frame_avail = 1;
    got_data = 0;
    {
    int speed = -6;
    if(vpx_codec_control(&codec[0], VP8E_SET_CPUUSED, speed))        //
        die_codec(&codec[0], "Failed to set cpu_used");
    }
    {
    unsigned int static_thresh = 1000;
    if(vpx_codec_control(&codec[0], VP8E_SET_STATIC_THRESHOLD, static_thresh))        //
        die_codec(&codec[0], "Failed to set static threshold");
    }

    for ( i=1; i<NUM_ENCODES; i++)
    {
      {
        int speed = -6;
        if(vpx_codec_control(&codec[i], VP8E_SET_CPUUSED, speed))        //
            die_codec(&codec[i], "Failed to set cpu_used");
      }

      {
      unsigned int static_thresh = 0;
      if(vpx_codec_control(&codec[i], VP8E_SET_STATIC_THRESHOLD, static_thresh))        //
          die_codec(&codec[i], "Failed to set static threshold");
      }
    }
    while(frame_avail || got_data)
    {
        vpx_codec_iter_t iter[NUM_ENCODES]={NULL};
        const vpx_codec_cx_pkt_t *pkt[NUM_ENCODES];

        flags = 0;

        frame_avail = read_frame(infile, &raw[0]);

        for ( i=1; i<NUM_ENCODES; i++)
        {
            if(frame_avail)
            {
                int src_uvwidth = (raw[i-1].d_w + 1) >> 1;
                int src_uvheight = (raw[i-1].d_h + 1) >> 1;
                const unsigned char* src_y = raw[i-1].planes[VPX_PLANE_Y];
                const unsigned char* src_u = raw[i-1].planes[VPX_PLANE_Y] + raw[i-1].d_w*raw[i-1].d_h;
                const unsigned char* src_v = raw[i-1].planes[VPX_PLANE_Y] + raw[i-1].d_w*raw[i-1].d_h + src_uvwidth*src_uvheight;
                int dst_uvwidth = (raw[i].d_w + 1) >> 1;
                int dst_uvheight = (raw[i].d_h + 1) >> 1;
                unsigned char* dst_y = raw[i].planes[VPX_PLANE_Y];
                unsigned char* dst_u = raw[i].planes[VPX_PLANE_Y] + raw[i].d_w*raw[i].d_h;
                unsigned char* dst_v = raw[i].planes[VPX_PLANE_Y] + raw[i].d_w*raw[i].d_h + dst_uvwidth*dst_uvheight;

                //FilterMode = 0 results in worse psnr, FilterMode = 1 or 2 give better/similar psnr. 3 modes have similar result performance wise.
                I420Scale(src_y, raw[i-1].d_w, src_u, src_uvwidth, src_v, src_uvwidth, raw[i-1].d_w, raw[i-1].d_h,
                          dst_y, raw[i].d_w, dst_u, dst_uvwidth, dst_v, dst_uvwidth, raw[i].d_w, raw[i].d_h, 1);
            }
        }

        for (i=NUM_ENCODES-1; i>=0 ; i--)
        {
            if(vpx_codec_encode(&codec[i], frame_avail? &raw[i] : NULL, frame_cnt[i],
                1, flags, arg_deadline))
                die_codec(&codec[i], "Failed to encode frame");

            got_data = 0;

            while( (pkt[i] = vpx_codec_get_cx_data(&codec[i], &iter[i])) )
            {
                got_data = 1;
                switch(pkt[i]->kind) {
                    case VPX_CODEC_CX_FRAME_PKT:
                        write_ivf_frame_header(outfile[i], pkt[i]);
                        if(fwrite(pkt[i]->data.frame.buf, 1, pkt[i]->data.frame.sz,
                                  outfile[i]));
                    break;
                    case VPX_CODEC_PSNR_PKT:
                        if (show_psnr)
                        {
                            int j;

                            psnr_sse_total[i] += pkt[i]->data.psnr.sse[0];
                            psnr_samples_total[i] += pkt[i]->data.psnr.samples[0];
                            for (j = 0; j < 4; j++)
                            {
                                //fprintf(stderr, "%.3lf ", pkt[i]->data.psnr.psnr[j]);
                                psnr_totals[i][j] += pkt[i]->data.psnr.psnr[j];
                            }
                            psnr_count[i]++;
                        }

                        break;
                    default:
                        break;
                }
                printf(pkt[i]->kind == VPX_CODEC_CX_FRAME_PKT
                       && (pkt[i]->data.frame.flags & VPX_FRAME_IS_KEY)? "K":".");
                fflush(stdout);
            }
            frame_cnt[i]++;
        }
    }
    printf("\n");

    fclose(infile);

    for (i=0; i< NUM_ENCODES; i++)
    {
        printf("Processed %d frames.\n",frame_cnt[i]-1);

        /* Calculate PSNR and print it out */
        if ( (show_psnr) && (psnr_count[i]>0) )
        {
            int j;
            double ovpsnr = vp8_mse2psnr(psnr_samples_total[i], 255.0,
                                         psnr_sse_total[i]);

            fprintf(stderr, "\n ENC%d PSNR (Overall/Avg/Y/U/V)", i);

            fprintf(stderr, " %.3lf", ovpsnr);
            for (j = 0; j < 4; j++)
            {
                fprintf(stderr, " %.3lf", psnr_totals[i][j]/psnr_count[i]);
            }
        }

        if(vpx_codec_destroy(&codec[i]))
            die_codec(&codec[i], "Failed to destroy codec");

        /* Try to rewrite the file header with the actual frame count */
        if(!fseek(outfile[i], 0, SEEK_SET))
            write_ivf_file_header(outfile[i], &cfg[i], frame_cnt[i]-1);
        fclose(outfile[i]);

        vpx_img_free(&raw[i]);
    }

    vpx_free(cfg[0].mr_low_res_mode_info);
#endif

    return EXIT_SUCCESS;
}
