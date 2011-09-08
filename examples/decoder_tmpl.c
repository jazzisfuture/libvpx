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
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#include "vpx_io/vpxio.h"
#define interface (vpx_codec_vp8_dx())
@EXTRA_INCLUDES

static unsigned int mem_get_le32(const unsigned char *mem) {
    return (mem[3] << 24)|(mem[2] << 16)|(mem[1] << 8)|(mem[0]);
}

static void die(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    if(fmt[strlen(fmt)-1] != '\n')
        printf("\n");
    exit(EXIT_FAILURE);
}

@DIE_CODEC

@HELPERS

int main(int argc, char **argv) {
    vpxio_ctx_t input_ctx, output_ctx;
    vpx_codec_ctx_t  codec;
    int              flags = 0, frame_cnt = 0;
    uint8_t               *buf = NULL;
    size_t                 buf_sz = 0, buf_alloc_sz = 0;
    vpx_codec_err_t  res;
@@@@EXTRA_VARS

    (void)res;
    /* Open files */
@@@@USAGE
    /* Open input file */
    if(!vpxio_open_src(&input_ctx, argv[1])){
        printf("Failed to open input file: %s", vpxio_get_file_name(&input_ctx));
        return 0;
    }
    /* Open output file */
    vpxio_open_y4m_dst(&output_ctx, argv[2], vpxio_get_width(&input_ctx), vpxio_get_height(&input_ctx), vpxio_get_framerate(&input_ctx).den, vpxio_get_framerate(&input_ctx).num);

    printf("Using %s\n",vpx_codec_iface_name(interface));
@@@@DEC_INIT

    /* Read each frame */
    while(!vpxio_read_pkt(&input_ctx, &buf, (size_t *)&buf_sz, (size_t *)&buf_alloc_sz)){
        vpx_codec_iter_t  iter = NULL;
        vpx_image_t      *img;

        frame_cnt++;
@@@@@@@@PRE_DECODE
@@@@@@@@DECODE

        /* Write decoded data to disk */
@@@@@@@@GET_FRAME

@@@@@@@@@@@@PROCESS_DX
        }
    }
    printf("Processed %d frames.\n",frame_cnt);
@@@@DESTROY

    vpxio_close(&input_ctx);
    vpxio_close(&output_ctx);
    return EXIT_SUCCESS;
}
