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
@*INTRODUCTION
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include "vpx_io/vpxio.h"
#define interface (vpx_codec_vp8_cx())
#define fourcc    0x30385056
@EXTRA_INCLUDES

static void die(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    if(fmt[strlen(fmt)-1] != '\n')
        printf("\n");
    exit(EXIT_FAILURE);
}

@DIE_CODEC

int main(int argc, char **argv) {
    vpxio_ctx_t input_ctx, output_ctx;
    vpx_codec_ctx_t      codec;
    vpx_codec_enc_cfg_t  cfg;
    int                  frame_cnt = 0;
    vpx_image_t          raw;
    vpx_codec_err_t      res;
    long                 width;
    long                 height;
    int                  frame_avail;
    int                  got_data;
    int                  flags = 0;
@@@@TWOPASS_VARS

    /* Open files */
@@@@USAGE
    width = strtol(argv[1], NULL, 0);
    height = strtol(argv[2], NULL, 0);
    if(width < 16 || width%2 || height <16 || height%2)
        die("Invalid resolution: %ldx%ld", width, height);

    printf("Using %s\n",vpx_codec_iface_name(interface));

@@@@ENC_DEF_CFG

/* open output file */
    if(!vpxio_open_webm_dst(&output_ctx, argv[4], width, height, 1, 30)){
					printf("Failed to open output file: %s", vpxio_get_file_name(&output_ctx));
					return EXIT_FAILURE;
	}

@@@@ENC_SET_CFG
@@@@ENC_SET_CFG2

@@@@TWOPASS_LOOP_BEGIN

    /* open input file */
	if(!vpxio_open_src(&input_ctx, argv[3]))                                              
			if(!vpxio_open_i420_src(&input_ctx, argv[3], width, height)){            
				printf("Failed to open input file: %s", vpxio_get_file_name(&input_ctx));
				return EXIT_FAILURE;
			}
			
	cfg.g_w = vpxio_get_width(&input_ctx);
	cfg.g_h = vpxio_get_height(&input_ctx);  
	cfg.g_timebase.den = 1000;

@@@@@@@@ENC_INIT

        frame_avail = 1;
        got_data = 0;
        while(frame_avail || got_data) {
            vpx_codec_iter_t iter = NULL;
            const vpx_codec_cx_pkt_t *pkt;

@@@@@@@@@@@@PER_FRAME_CFG
@@@@@@@@@@@@ENCODE_FRAME
            got_data = 0;
            while( (pkt = vpx_codec_get_cx_data(&codec, &iter)) ) {
                got_data = 1;
                switch(pkt->kind) {
@@@@@@@@@@@@@@@@PROCESS_FRAME
@@@@@@@@@@@@@@@@PROCESS_STATS
                default:
                    break;
                }
                printf(pkt->kind == VPX_CODEC_CX_FRAME_PKT
                       && (pkt->data.frame.flags & VPX_FRAME_IS_KEY)? "K":".");
                fflush(stdout);
            }
            frame_cnt++;
        }
        printf("\n");
        vpxio_close(&input_ctx);
@@@@TWOPASS_LOOP_END

    printf("Processed %d frames.\n",frame_cnt-1);
@@@@DESTROY

    vpxio_close(&output_ctx);
    return EXIT_SUCCESS;
}
