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
 * This is an example demonstrating how to implement a multi-layer VP8
 * encoding scheme based on temporal scalability for video applications
 * that benefit from a scalable bitstream.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include "vpx_config.h"

#define interface (vpx_codec_vp8_cx())
#define fourcc    0x30385056

#define IVF_FILE_HDR_SZ  (32)
#define IVF_FRAME_HDR_SZ (12)

#if CONFIG_REUSE_PARTITION
#define NUM_ENCODES 2
#define MODE_INFO_SIZE 112
#define COEFF_PROB_SIZE 1056
#define MV_DATA_SIZE 2000
#define Q_DATA_SIZE 50
#define FP_SIZE 5
#else
#define NUM_ENCODES 1
#endif

static void mem_put_le16(char *mem, unsigned int val) {
    mem[0] = val;
    mem[1] = val>>8;
}

static void mem_put_le32(char *mem, unsigned int val) {
    mem[0] = val;
    mem[1] = val>>8;
    mem[2] = val>>16;
    mem[3] = val>>24;
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
#if CONFIG_REUSE_PARTITION
void * alloc_mode_mv_buffer(int width, int height)
{
   void *ri;
   int MBs,mb_rows,mb_cols;

    mb_rows = height >> 4;
    mb_cols = width >> 4;
    MBs = mb_rows * mb_cols;
    ri = calloc(MBs*MODE_INFO_SIZE, sizeof(unsigned char));
    if (!ri)
    {
      free(ri);
    }
    return ri;
}
#else
static int mode_to_num_layers[7] = {2, 2, 3, 3, 3, 3, 5};
#endif

int main(int argc, char **argv) {
    FILE                *infile;
    vpx_codec_ctx_t      codec[NUM_ENCODES];
    vpx_codec_enc_cfg_t  cfg[NUM_ENCODES];
    int                  frame_cnt[NUM_ENCODES]={0};
    vpx_image_t          raw;
    vpx_codec_err_t      res[NUM_ENCODES];
    unsigned int         width;
    unsigned int         height;
    int                  frame_avail;
    int                  got_data;
    int                  flags = 0;
    int                  i;

    int                  layering_mode = 0;
#if CONFIG_REUSE_PARTITION
    FILE                *outfile[NUM_ENCODES];
    FILE                *speedfile;
    //FILE                *datafile;
    long int             *fp_size;
    unsigned char        *q_data,*mv_data;
    unsigned char        *reuse_map;
    static int            add_size = 32;
    long int              datasize = 0;
#else
    FILE                 *outfile[MAX_LAYERS];
    int                  j;
    int                  frames_in_layer[MAX_LAYERS] = {0};
    int                  layer_flags[MAX_PERIODICITY] = {0};
#endif
    // Check usage and arguments
    if (argc < 7)
        die("Usage: %s <infile> <outfile> <width> <height> <mode> "
            "<Rate_0> ... <Rate_nlayers-1>\n", argv[0]);

    width  = strtol (argv[3], NULL, 0);
    height = strtol (argv[4], NULL, 0);
    if (width < 16 || width%2 || height <16 || height%2)
        die ("Invalid resolution: %d x %d", width, height);

    if (!sscanf(argv[5], "%d", &layering_mode))
        die ("Invalid mode %s", argv[5]);
    if (layering_mode<0 || layering_mode>6)
        die ("Invalid mode (0..6) %s", argv[5]);

#if CONFIG_REUSE_PARTITION
    if (argc != 6+NUM_ENCODES)
#else
    if (argc != 6+mode_to_num_layers[layering_mode])
#endif
        die ("Invalid number of arguments");

    if (!vpx_img_alloc (&raw, VPX_IMG_FMT_I420, width, height, 1))
        die ("Failed to allocate image", width, height);

    printf("Using %s\n",vpx_codec_iface_name(interface));

    for (i=0; i< NUM_ENCODES; i++)
    {
        // Populate encoder configuration
        res[i] = vpx_codec_enc_config_default(interface, &cfg[i], 0);
        if(res[i]) {
            printf("Failed to get config: %s\n", vpx_codec_err_to_string(res[i]));
            return EXIT_FAILURE;
        }
    }

    // Update the default configuration with our settings
    cfg[0].g_w = width;
    cfg[0].g_h = height;

#if CONFIG_REUSE_PARTITION

   if(!(speedfile = fopen("speed.stt","a")))
        die("Failed to open speed.stt for writing");

   //if(!(datafile = fopen("datafile.txt", "wb")))
       // die("Failed to open datafile for writing");

   width = (width + 15) & ~15;
   height = (height + 15) & ~15;

    cfg[0].reuse_info = alloc_mode_mv_buffer(width, height);
    cfg[0].reuse_prob = calloc(COEFF_PROB_SIZE, sizeof(unsigned char));
    mv_data = calloc(MV_DATA_SIZE, sizeof(unsigned char));                  //
    q_data = calloc(Q_DATA_SIZE, sizeof(unsigned char));
    reuse_map = calloc((height >> 4) * (width >> 4), 1);
    cfg[0].copy_flag = 1;
    fp_size = calloc(FP_SIZE, sizeof(long int));
#else
    for (i=6; i<6+mode_to_num_layers[layering_mode]; i++)
        if (!sscanf(argv[i], "%d", &cfg[0].ts_target_bitrate[i-6]))
            die ("Invalid data rate %s", argv[i]);
#endif
    // Real time parameters
    cfg[0].rc_dropframe_thresh = 0;
    cfg[0].rc_end_usage        = VPX_CBR;
    cfg[0].rc_resize_allowed   = 0;
    cfg[0].rc_min_quantizer    = 4;
    cfg[0].rc_max_quantizer    = 63;
    cfg[0].rc_undershoot_pct   = 98;
    cfg[0].rc_overshoot_pct    = 100;
    cfg[0].rc_buf_initial_sz   = 500;
    cfg[0].rc_buf_optimal_sz   = 600;
    cfg[0].rc_buf_sz           = 1000;

    // Enable error resilient mode
    cfg[0].g_error_resilient = 1;
    cfg[0].g_lag_in_frames   = 0;
    cfg[0].kf_mode           = VPX_KF_DISABLED;

    // Disable automatic keyframe placement
    cfg[0].kf_min_dist = cfg[0].kf_max_dist = 1000;

    // Temporal scaling parameters:
    // NOTE: The 3 prediction frames cannot be used interchangebly due to
    // differences in the way they are handled throughout the code. The
    // frames should be allocated to layers in the order LAST, GF, ARF.
    // Other combinations work, but may produce slightly inferior results.

#if !CONFIG_REUSE_PARTITION
    switch (layering_mode)
    {

    case 0:
    {
        // 2-layers, 2-frame period
        int ids[2] = {0,1};
        cfg[0].ts_number_layers     = 2;
        cfg[0].ts_periodicity       = 2;
        cfg[0].ts_rate_decimator[0] = 2;
        cfg[0].ts_rate_decimator[1] = 1;
        memcpy(cfg[0].ts_layer_id, ids, sizeof(ids));

        // 0=L, 1=GF, Intra-layer prediction enabled
        layer_flags[0] = VPX_EFLAG_FORCE_KF  |
                         VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF |
                         VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF;
        layer_flags[1] = VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_LAST |
                         VP8_EFLAG_NO_REF_ARF;
#if 0
        // 0=L, 1=GF, Intra-layer 1 prediction disabled
        layer_flags[0] = VPX_EFLAG_FORCE_KF  |
                         VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF |
                         VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF;
        layer_flags[1] = VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_NO_UPD_LAST |
                         VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_REF_LAST;
#endif
        break;
    }

    case 1:
    {
        // 2-layers, 3-frame period
        int ids[3] = {0,1,1};
        cfg[0].ts_number_layers     = 2;
        cfg[0].ts_periodicity       = 3;
        cfg[0].ts_rate_decimator[0] = 3;
        cfg[0].ts_rate_decimator[1] = 1;
        memcpy(cfg[0].ts_layer_id, ids, sizeof(ids));

        // 0=L, 1=GF, Intra-layer prediction enabled
        layer_flags[0] = VPX_EFLAG_FORCE_KF  |
                         VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF;
        layer_flags[1] =
        layer_flags[2] = VP8_EFLAG_NO_REF_GF  |
                         VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_UPD_ARF |
                                                VP8_EFLAG_NO_UPD_LAST;
        break;
    }

    case 2:
    {
        // 3-layers, 6-frame period
        int ids[6] = {0,2,2,1,2,2};
        cfg[0].ts_number_layers     = 3;
        cfg[0].ts_periodicity       = 6;
        cfg[0].ts_rate_decimator[0] = 6;
        cfg[0].ts_rate_decimator[1] = 3;
        cfg[0].ts_rate_decimator[2] = 1;
        memcpy(cfg[0].ts_layer_id, ids, sizeof(ids));

        // 0=L, 1=GF, 2=ARF, Intra-layer prediction enabled
        layer_flags[0] = VPX_EFLAG_FORCE_KF  |
                         VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF;
        layer_flags[3] = VP8_EFLAG_NO_REF_ARF | VP8_EFLAG_NO_UPD_ARF |
                                                VP8_EFLAG_NO_UPD_LAST;
        layer_flags[1] =
        layer_flags[2] =
        layer_flags[4] =
        layer_flags[5] = VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_LAST;
        break;
    }

    case 3:
    {
        // 3-layers, 4-frame period
        int ids[6] = {0,2,1,2};
        cfg[0].ts_number_layers     = 3;
        cfg[0].ts_periodicity       = 4;
        cfg[0].ts_rate_decimator[0] = 4;
        cfg[0].ts_rate_decimator[1] = 2;
        cfg[0].ts_rate_decimator[2] = 1;
        memcpy(cfg[0].ts_layer_id, ids, sizeof(ids));

        // 0=L, 1=GF, 2=ARF, Intra-layer prediction disabled
        layer_flags[0] = VPX_EFLAG_FORCE_KF  |
                         VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF;
        layer_flags[2] = VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_ARF |
                         VP8_EFLAG_NO_UPD_LAST;
        layer_flags[1] =
        layer_flags[3] = VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_GF |
                         VP8_EFLAG_NO_UPD_ARF;
        break;
        cfg[0].ts_rate_decimator[2] = 1;
    }

    case 4:
    {
        // 3-layers, 4-frame period
        int ids[6] = {0,2,1,2};
        cfg[0].ts_number_layers     = 3;
        cfg[0].ts_periodicity       = 4;
        cfg[0].ts_rate_decimator[0] = 4;
        cfg[0].ts_rate_decimator[1] = 2;
        cfg[0].ts_rate_decimator[2] = 1;
        memcpy(cfg[0].ts_layer_id, ids, sizeof(ids));

        // 0=L, 1=GF, 2=ARF, Intra-layer prediction enabled in layer 1,
        // disabled in layer 2
        layer_flags[0] = VPX_EFLAG_FORCE_KF  |
                         VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF;
        layer_flags[2] = VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_ARF;
        layer_flags[1] =
        layer_flags[3] = VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_GF |
                         VP8_EFLAG_NO_UPD_ARF;
        break;
    }

    case 5:
    {
        // 3-layers, 4-frame period
        int ids[6] = {0,2,1,2};
        cfg[0].ts_number_layers     = 3;
        cfg[0].ts_periodicity       = 4;
        cfg[0].ts_rate_decimator[0] = 4;
        cfg[0].ts_rate_decimator[1] = 2;
        cfg[0].ts_rate_decimator[2] = 1;
        memcpy(cfg[0].ts_layer_id, ids, sizeof(ids));

        // 0=L, 1=GF, 2=ARF, Intra-layer prediction enabled
        layer_flags[0] = VPX_EFLAG_FORCE_KF  |
                         VP8_EFLAG_NO_REF_GF | VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_GF | VP8_EFLAG_NO_UPD_ARF;
        layer_flags[2] = VP8_EFLAG_NO_REF_ARF |
                         VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_ARF;
        layer_flags[1] =
        layer_flags[3] = VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_GF;
        break;
    }

    case 6:
    {
        // NOTE: Probably of academic interest only

        // 5-layers, 16-frame period
        int ids[16] = {0,4,3,4,2,4,3,4,1,4,3,4,2,4,3,4};
        cfg[0].ts_number_layers     = 5;
        cfg[0].ts_periodicity       = 16;
        cfg[0].ts_rate_decimator[0] = 16;
        cfg[0].ts_rate_decimator[1] = 8;
        cfg[0].ts_rate_decimator[2] = 4;
        cfg[0].ts_rate_decimator[3] = 2;
        cfg[0].ts_rate_decimator[4] = 1;
        memcpy(cfg[0].ts_layer_id, ids, sizeof(ids));

        layer_flags[0]  = VPX_EFLAG_FORCE_KF;
        layer_flags[1]  =
        layer_flags[3]  =
        layer_flags[5]  =
        layer_flags[7]  =
        layer_flags[9]  =
        layer_flags[11] =
        layer_flags[13] =
        layer_flags[15] = VP8_EFLAG_NO_UPD_LAST |
                          VP8_EFLAG_NO_UPD_GF   |
                          VP8_EFLAG_NO_UPD_ARF  |
                          VP8_EFLAG_NO_UPD_ENTROPY;
        layer_flags[2]  =
        layer_flags[6]  =
        layer_flags[10] =
        layer_flags[14] = 0;
        layer_flags[4]  =
        layer_flags[12] = VP8_EFLAG_NO_REF_LAST;
        layer_flags[8]  = VP8_EFLAG_NO_REF_LAST | VP8_EFLAG_NO_REF_GF |
                          VP8_EFLAG_NO_UPD_ENTROPY;
        break;
    }

    default:
        break;
    }
#endif

    // Open input file
    if(!(infile = fopen(argv[1], "rb")))
        die("Failed to open %s for reading", argv[1]);

    // Open an output file for each stream
#if CONFIG_REUSE_PARTITION
    for (i=0; i<NUM_ENCODES; i++)
#else
    for (i=0; i<cfg[0].ts_number_layers; i++)
#endif
    {
        char file_name[512];
        sprintf (file_name, "%s_%d.ivf", argv[2], i);
        if (!(outfile[i] = fopen(file_name, "wb")))
            die("Failed to open %s for writing", file_name);
    }

#if CONFIG_REUSE_PARTITION
    for (i=1; i<NUM_ENCODES; i++)
    {
        memcpy(&cfg[i], &cfg[0], sizeof(vpx_codec_enc_cfg_t));
        cfg[i].copy_flag = 0;
    }
#endif

    for (i=0; i<NUM_ENCODES; i++)
    {
#if CONFIG_REUSE_PARTITION
        if (!sscanf(argv[6+i], "%d", &cfg[i].rc_target_bitrate))
            die ("Invalid data rate %s", argv[6+i]);
        cfg[i].fp_size = fp_size;
        cfg[i].q_data = q_data;
        cfg[i].reuse_map = reuse_map;
#endif
        write_ivf_file_header(outfile[i], &cfg[i], 0);
        // Initialize codec
        if (vpx_codec_enc_init (&codec[i], interface, &cfg[i], 0))
            die_codec (&codec[i], "Failed to initialize encoder");

        // Cap CPU & first I-frame size
        vpx_codec_control (&codec[i], VP8E_SET_CPUUSED, -6);
        vpx_codec_control (&codec[i], VP8E_SET_MAX_INTRA_BITRATE_PCT, 600);
    }

    frame_avail = 1;
    while (frame_avail || got_data) {
        vpx_codec_iter_t iter[NUM_ENCODES] = {NULL};
        const vpx_codec_cx_pkt_t *pkt[NUM_ENCODES];
#if !CONFIG_REUSE_PARTITION
        flags = layer_flags[frame_cnt[0] % cfg[0].ts_periodicity];
#endif
        frame_avail = read_frame(infile, &raw);
        for (i=0; i< NUM_ENCODES; i++)
        {
            if (vpx_codec_encode(&codec[i], frame_avail? &raw : NULL, frame_cnt[i],
                                1, flags, VPX_DL_REALTIME))
                die_codec(&codec[i], "Failed to encode frame");

#if !CONFIG_REUSE_PARTITION
            // Reset KF flag
            layer_flags[0] &= ~VPX_EFLAG_FORCE_KF;
#endif
            got_data = 0;
            while ( (pkt[i] = vpx_codec_get_cx_data(&codec[i], &iter[i])) ) {
                got_data = 1;
                switch (pkt[i]->kind) {
                case VPX_CODEC_CX_FRAME_PKT:
#if CONFIG_REUSE_PARTITION
                        if(cfg[i].copy_flag==1)
                        {
                            write_ivf_frame_header(outfile[i], pkt[i]);
                            memcpy(mv_data,pkt[i]->data.frame.buf,*fp_size);
                            datasize +=pkt[i]->data.raw.sz;
                        }
                        else
                        {
                            int frame_size;
                            write_ivf_frame_header(outfile[i], pkt[i]);
                            datasize +=pkt[i]->data.raw.sz;
                            frame_size = pkt[i]->data.frame.sz + *fp_size;
                            if(!fseek(outfile[i], add_size , SEEK_SET));
                            if(fwrite(&frame_size, 1, 4, outfile[i]));
                            if(!fseek(outfile[i], add_size + 12 , SEEK_SET));
                            memcpy(mv_data+3+fp_size[2],q_data+3+fp_size[2],fp_size[4]);
                            if(fwrite(mv_data, 1, *fp_size,outfile[i]));
                            add_size += 12 + frame_size;
                            //fwrite(q_data+3+fp_size[2],1,fp_size[4],datafile);

                        }
                        //printf("datasize %ld\n",datasize);
                        if(fwrite(pkt[i]->data.frame.buf, 1, pkt[i]->data.frame.sz,
                                  outfile[i]));
#else
                    for (j=cfg[i].ts_layer_id[frame_cnt[i] % cfg[i].ts_periodicity];
                                                  j<cfg[i].ts_number_layers; j++)
                    {
                        write_ivf_frame_header(outfile[j], pkt[i]);
                        if (fwrite(pkt[i]->data.frame.buf, 1, pkt[i]->data.frame.sz,
                                  outfile[j]));
                        frames_in_layer[j]++;
                    }
#endif
                    break;
                default:
                    break;
                }
                printf (pkt[i]->kind == VPX_CODEC_CX_FRAME_PKT
                        && (pkt[i]->data.frame.flags & VPX_FRAME_IS_KEY)? "K":".");
                fflush (stdout);
            }
            frame_cnt[i]++;
        }
    }
    printf ("\n");
    fclose (infile);

#if CONFIG_REUSE_PARTITION
    free(cfg[0].reuse_info);
    cfg[0].reuse_info = NULL;
    free(cfg[0].reuse_prob);
    cfg[0].reuse_prob = NULL;
    free(cfg[0].fp_size);
    cfg[0].fp_size = NULL;
    free(mv_data);
    free(cfg[0].q_data);
    cfg[0].q_data = NULL;
    free(cfg[0].reuse_map);
    cfg[0].reuse_map = NULL;
    fprintf(speedfile,"%ld\n", datasize);
    fclose(speedfile);
#endif

    for (i=0; i< NUM_ENCODES; i++)
    {

        printf ("Processed %d frames.\n",frame_cnt[i]-1);
        if (vpx_codec_destroy(&codec[i]))
                die_codec (&codec[i], "Failed to destroy codec");

        // Try to rewrite the output file headers with the actual frame count
#if CONFIG_REUSE_PARTITION
        if (!fseek(outfile[i], 0, SEEK_SET))
                write_ivf_file_header (outfile[i], &cfg[i], frame_cnt[i]-1);
            fclose (outfile[i]);
#else
        for (j=0; j<cfg[0].ts_number_layers; j++)
        {
            if (!fseek(outfile[j], 0, SEEK_SET))
                write_ivf_file_header (outfile[j], &cfg[i], frames_in_layer[j]);
            fclose (outfile[j]);
        }
#endif
    }

    return EXIT_SUCCESS;
}

