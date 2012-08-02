/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


/****************************************************************************
 * Notes:
 *
 * This implementation makes use of 16 bit fixed point verio of two multiply
 * constants:
 *         1.   sqrt(2) * cos (pi/8)
 *         2.   sqrt(2) * sin (pi/8)
 * Becuase the first constant is bigger than 1, to maintain the same 16 bit
 * fixed point precision as the second one, we use a trick of
 *         x * a = x + x*(a-1)
 * so
 *         x * sqrt(2) * cos (pi/8) = x + x * (sqrt(2) *cos(pi/8)-1).
 **************************************************************************/
#include "vpx_ports/config.h"
#include "vp8/common/idct.h"

#if CONFIG_HYBRIDTRANSFORM
#include "vp8/common/blockd.h"
#endif

#include <math.h>

static const int cospi8sqrt2minus1 = 20091;
static const int sinpi8sqrt2      = 35468;
static const int rounding = 0;

#if CONFIG_HYBRIDTRANSFORM
float idct_4[16] = {
  0.500000000000000,   0.653281482438188,   0.500000000000000,   0.270598050073099,
  0.500000000000000,   0.270598050073099,  -0.500000000000000,  -0.653281482438188,
  0.500000000000000,  -0.270598050073099,  -0.500000000000000,   0.653281482438188,
  0.500000000000000,  -0.653281482438188,   0.500000000000000,  -0.270598050073099
};

float iadst_4[16] = {
  0.228013428883779,   0.577350269189626,   0.656538502008139,   0.428525073124360,
  0.428525073124360,   0.577350269189626,  -0.228013428883779,  -0.656538502008139,
  0.577350269189626,                   0,  -0.577350269189626,   0.577350269189626,
  0.656538502008139,  -0.577350269189626,   0.428525073124359,  -0.228013428883779
};
#endif

#if CONFIG_HTRANS8X8
float idct_8[64] = {
  0.353553390593274,   0.490392640201615,   0.461939766255643,   0.415734806151273,
  0.353553390593274,   0.277785116509801,   0.191341716182545,   0.097545161008064,
  0.353553390593274,   0.415734806151273,   0.191341716182545,  -0.097545161008064,
 -0.353553390593274,  -0.490392640201615,  -0.461939766255643,  -0.277785116509801,
  0.353553390593274,   0.277785116509801,  -0.191341716182545,  -0.490392640201615,
 -0.353553390593274,   0.097545161008064,   0.461939766255643,   0.415734806151273,
  0.353553390593274,   0.097545161008064,  -0.461939766255643,  -0.277785116509801,
  0.353553390593274,   0.415734806151273,  -0.191341716182545,  -0.490392640201615,
  0.353553390593274,  -0.097545161008064,  -0.461939766255643,   0.277785116509801,
  0.353553390593274,  -0.415734806151273,  -0.191341716182545,   0.490392640201615,
  0.353553390593274,  -0.277785116509801,  -0.191341716182545,   0.490392640201615,
 -0.353553390593274,  -0.097545161008064,   0.461939766255643,  -0.415734806151273,
  0.353553390593274,  -0.415734806151273,   0.191341716182545,   0.097545161008064,
 -0.353553390593274,   0.490392640201615,  -0.461939766255643,   0.277785116509801,
  0.353553390593274,  -0.490392640201615,   0.461939766255643,  -0.415734806151273,
  0.353553390593274,  -0.277785116509801,   0.191341716182545,  -0.097545161008064
};

float iadst_8[64] = {
  0.089131608307533,   0.255357107325376,   0.387095214016349,   0.466553967085785,
  0.483002021635509,   0.434217976756762,   0.326790388032145,   0.175227946595735,
  0.175227946595735,   0.434217976756762,   0.466553967085785,   0.255357107325376,
 -0.089131608307533,  -0.387095214016348,  -0.483002021635509,  -0.326790388032145,
  0.255357107325376,   0.483002021635509,   0.175227946595735,  -0.326790388032145,
 -0.466553967085785,  -0.089131608307533,   0.387095214016349,   0.434217976756762,
  0.326790388032145,   0.387095214016349,  -0.255357107325376,  -0.434217976756762,
  0.175227946595735,   0.466553967085786,  -0.089131608307534,  -0.483002021635509,
  0.387095214016349,   0.175227946595735,  -0.483002021635509,   0.089131608307533,
  0.434217976756762,  -0.326790388032145,  -0.255357107325377,   0.466553967085785,
  0.434217976756762,  -0.089131608307533,  -0.326790388032145,   0.483002021635509,
 -0.255357107325376,  -0.175227946595735,   0.466553967085785,  -0.387095214016348,
  0.466553967085785,  -0.326790388032145,   0.089131608307533,   0.175227946595735,
 -0.387095214016348,   0.483002021635509,  -0.434217976756762,   0.255357107325376,
  0.483002021635509,  -0.466553967085785,   0.434217976756762,  -0.387095214016348,
  0.326790388032145,  -0.255357107325375,   0.175227946595736,  -0.089131608307532
};
#endif

#if CONFIG_HYBRIDTRANSFORM
void vp8_iht4x4llm_c(short *input, short *output, int pitch, TX_TYPE tx_type) {
  int i, j, k;
  float bufa[16], bufb[16]; // buffers are for floating-point test purpose
                            // the implementation could be simplified in conjunction with integer transform
  short *ip = input;
  short *op = output;
  int shortpitch = pitch >> 1;

  float *pfa = &bufa[0];
  float *pfb = &bufb[0];

  // pointers to vertical and horizontal transforms
  float *ptv, *pth;

  // load and convert residual array into floating-point
  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      pfa[i] = (float)ip[i];
    }
    pfa += 4;
    ip  += 4;
  }

  // vertical transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case ADST_DCT  :
      ptv = &iadst_4[0];
      break;

    default :
      ptv = &idct_4[0];
      break;
  }

  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      pfb[i] = 0 ;
      for(k = 0; k < 4; k++) {
        pfb[i] += ptv[k] * pfa[(k<<2)];
      }
      pfa += 1;
    }

    pfb += 4;
    ptv += 4;
    pfa = &bufa[0];
  }

  // horizontal transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case  DCT_ADST :
      pth = &iadst_4[0];
      break;

    default :
      pth = &idct_4[0];
      break;
  }

  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      pfa[i] = 0;
      for(k = 0; k < 4; k++) {
        pfa[i] += pfb[k] * pth[k];
      }
      pth += 4;
     }

    pfa += 4;
    pfb += 4;

    switch(tx_type) {
      case ADST_ADST :
      case  DCT_ADST :
        pth = &iadst_4[0];
        break;

      default :
        pth = &idct_4[0];
        break;
    }
  }

  // convert to short integer format and load BLOCKD buffer
  op  = output;
  pfa = &bufa[0];

  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      op[i] = (pfa[i] > 0 ) ? (short)( pfa[i] / 8 + 0.49) :
                             -(short)( - pfa[i] / 8 + 0.49);
    }
    op  += shortpitch;
    pfa += 4;
  }
}
#endif

#if CONFIG_HTRANS8X8
void vp8_iht8x8llm_c(short *input, short *output, int pitch, TX_TYPE tx_type) {
  int i, j, k;
  float bufa[64], bufb[64]; // buffers are for floating-point test purpose
                            // the implementation could be simplified in conjunction with integer transform
  short *ip = input;
  short *op = output;
  int shortpitch = pitch >> 1;

  float *pfa = &bufa[0];
  float *pfb = &bufb[0];

  // pointers to vertical and horizontal transforms
  float *ptv, *pth;

  // load and convert residual array into floating-point
  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      pfa[i] = (float)ip[i];
    }
    pfa += 8;
    ip  += 8;
  }

  // vertical transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case ADST_DCT  :
      ptv = &iadst_8[0];
      break;

    default :
      ptv = &idct_8[0];
      break;
  }

  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      pfb[i] = 0 ;
      for(k = 0; k < 8; k++) {
        pfb[i] += ptv[k] * pfa[(k<<3)];
      }
      pfa += 1;
    }

    pfb += 8;
    ptv += 8;
    pfa = &bufa[0];
  }

  // horizontal transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case  DCT_ADST :
      pth = &iadst_8[0];
      break;

    default :
      pth = &idct_8[0];
      break;
  }

  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      pfa[i] = 0;
      for(k = 0; k < 8; k++) {
        pfa[i] += pfb[k] * pth[k];
      }
      pth += 8;
     }

    pfa += 8;
    pfb += 8;

    switch(tx_type) {
      case ADST_ADST :
      case  DCT_ADST :
        pth = &iadst_8[0];
        break;

      default :
        pth = &idct_8[0];
        break;
    }
  }

  // convert to short integer format and load BLOCKD buffer
  op  = output;
  pfa = &bufa[0];

  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      op[i] = (pfa[i] > 0 ) ? (short)( pfa[i] / 8 + 0.49) :
                             -(short)( - pfa[i] / 8 + 0.49);
    }
    op  += shortpitch;
    pfa += 8;
  }
}
#endif

void vp8_short_idct4x4llm_c(short *input, short *output, int pitch) {
  int i;
  int a1, b1, c1, d1;

  short *ip = input;
  short *op = output;
  int temp1, temp2;
  int shortpitch = pitch >> 1;

  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[8];
    b1 = ip[0] - ip[8];

    temp1 = (ip[4] * sinpi8sqrt2 + rounding) >> 16;
    temp2 = ip[12] + ((ip[12] * cospi8sqrt2minus1 + rounding) >> 16);
    c1 = temp1 - temp2;

    temp1 = ip[4] + ((ip[4] * cospi8sqrt2minus1 + rounding) >> 16);
    temp2 = (ip[12] * sinpi8sqrt2 + rounding) >> 16;
    d1 = temp1 + temp2;

    op[shortpitch * 0] = a1 + d1;
    op[shortpitch * 3] = a1 - d1;

    op[shortpitch * 1] = b1 + c1;
    op[shortpitch * 2] = b1 - c1;

    ip++;
    op++;
  }

  ip = output;
  op = output;

  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[2];
    b1 = ip[0] - ip[2];

    temp1 = (ip[1] * sinpi8sqrt2 + rounding) >> 16;
    temp2 = ip[3] + ((ip[3] * cospi8sqrt2minus1 + rounding) >> 16);
    c1 = temp1 - temp2;

    temp1 = ip[1] + ((ip[1] * cospi8sqrt2minus1 + rounding) >> 16);
    temp2 = (ip[3] * sinpi8sqrt2 + rounding) >> 16;
    d1 = temp1 + temp2;

    op[0] = (a1 + d1 + 16) >> 5;
    op[3] = (a1 - d1 + 16) >> 5;

    op[1] = (b1 + c1 + 16) >> 5;
    op[2] = (b1 - c1 + 16) >> 5;

    ip += shortpitch;
    op += shortpitch;
  }
}

void vp8_short_idct4x4llm_1_c(short *input, short *output, int pitch) {
  int i;
  int a1;
  short *op = output;
  int shortpitch = pitch >> 1;
  a1 = ((input[0] + 16) >> 5);
  for (i = 0; i < 4; i++) {
    op[0] = a1;
    op[1] = a1;
    op[2] = a1;
    op[3] = a1;
    op += shortpitch;
  }
}

void vp8_dc_only_idct_add_c(short input_dc, unsigned char *pred_ptr, unsigned char *dst_ptr, int pitch, int stride) {
  int a1 = ((input_dc + 16) >> 5);
  int r, c;

  for (r = 0; r < 4; r++) {
    for (c = 0; c < 4; c++) {
      int a = a1 + pred_ptr[c];

      if (a < 0)
        a = 0;

      if (a > 255)
        a = 255;

      dst_ptr[c] = (unsigned char) a;
    }

    dst_ptr += stride;
    pred_ptr += pitch;
  }

}

void vp8_short_inv_walsh4x4_c(short *input, short *output) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;

  for (i = 0; i < 4; i++) {
    a1 = ((ip[0] + ip[3]));
    b1 = ((ip[1] + ip[2]));
    c1 = ((ip[1] - ip[2]));
    d1 = ((ip[0] - ip[3]));

    op[0] = (a1 + b1 + 1) >> 1;
    op[1] = (c1 + d1) >> 1;
    op[2] = (a1 - b1) >> 1;
    op[3] = (d1 - c1) >> 1;

    ip += 4;
    op += 4;
  }

  ip = output;
  op = output;
  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[12];
    b1 = ip[4] + ip[8];
    c1 = ip[4] - ip[8];
    d1 = ip[0] - ip[12];
    op[0] = (a1 + b1 + 1) >> 1;
    op[4] = (c1 + d1) >> 1;
    op[8] = (a1 - b1) >> 1;
    op[12] = (d1 - c1) >> 1;
    ip++;
    op++;
  }
}

void vp8_short_inv_walsh4x4_1_c(short *in, short *out) {
  int i;
  short tmp[4];
  short *ip = in;
  short *op = tmp;

  op[0] = (ip[0] + 1) >> 1;
  op[1] = op[2] = op[3] = (ip[0] >> 1);

  ip = tmp;
  op = out;
  for (i = 0; i < 4; i++) {
    op[0] = (ip[0] + 1) >> 1;
    op[4] = op[8] = op[12] = (ip[0] >> 1);
    ip++;
    op++;
  }
}

#if CONFIG_LOSSLESS
void vp8_short_inv_walsh4x4_lossless_c(short *input, short *output) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;

  for (i = 0; i < 4; i++) {
    a1 = ((ip[0] + ip[3])) >> Y2_WHT_UPSCALE_FACTOR;
    b1 = ((ip[1] + ip[2])) >> Y2_WHT_UPSCALE_FACTOR;
    c1 = ((ip[1] - ip[2])) >> Y2_WHT_UPSCALE_FACTOR;
    d1 = ((ip[0] - ip[3])) >> Y2_WHT_UPSCALE_FACTOR;

    op[0] = (a1 + b1 + 1) >> 1;
    op[1] = (c1 + d1) >> 1;
    op[2] = (a1 - b1) >> 1;
    op[3] = (d1 - c1) >> 1;

    ip += 4;
    op += 4;
  }

  ip = output;
  op = output;
  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[12];
    b1 = ip[4] + ip[8];
    c1 = ip[4] - ip[8];
    d1 = ip[0] - ip[12];


    op[0] = ((a1 + b1 + 1) >> 1) << Y2_WHT_UPSCALE_FACTOR;
    op[4] = ((c1 + d1) >> 1) << Y2_WHT_UPSCALE_FACTOR;
    op[8] = ((a1 - b1) >> 1) << Y2_WHT_UPSCALE_FACTOR;
    op[12] = ((d1 - c1) >> 1) << Y2_WHT_UPSCALE_FACTOR;

    ip++;
    op++;
  }
}

void vp8_short_inv_walsh4x4_1_lossless_c(short *in, short *out) {
  int i;
  short tmp[4];
  short *ip = in;
  short *op = tmp;

  op[0] = ((ip[0] >> Y2_WHT_UPSCALE_FACTOR) + 1) >> 1;
  op[1] = op[2] = op[3] = ((ip[0] >> Y2_WHT_UPSCALE_FACTOR) >> 1);

  ip = tmp;
  op = out;
  for (i = 0; i < 4; i++) {
    op[0] = ((ip[0] + 1) >> 1) << Y2_WHT_UPSCALE_FACTOR;
    op[4] = op[8] = op[12] = ((ip[0] >> 1)) << Y2_WHT_UPSCALE_FACTOR;
    ip++;
    op++;
  }
}

void vp8_short_inv_walsh4x4_x8_c(short *input, short *output, int pitch) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;
  int shortpitch = pitch >> 1;

  for (i = 0; i < 4; i++) {
    a1 = ((ip[0] + ip[3])) >> WHT_UPSCALE_FACTOR;
    b1 = ((ip[1] + ip[2])) >> WHT_UPSCALE_FACTOR;
    c1 = ((ip[1] - ip[2])) >> WHT_UPSCALE_FACTOR;
    d1 = ((ip[0] - ip[3])) >> WHT_UPSCALE_FACTOR;

    op[0] = (a1 + b1 + 1) >> 1;
    op[1] = (c1 + d1) >> 1;
    op[2] = (a1 - b1) >> 1;
    op[3] = (d1 - c1) >> 1;

    ip += 4;
    op += shortpitch;
  }

  ip = output;
  op = output;
  for (i = 0; i < 4; i++) {
    a1 = ip[shortpitch * 0] + ip[shortpitch * 3];
    b1 = ip[shortpitch * 1] + ip[shortpitch * 2];
    c1 = ip[shortpitch * 1] - ip[shortpitch * 2];
    d1 = ip[shortpitch * 0] - ip[shortpitch * 3];


    op[shortpitch * 0] = (a1 + b1 + 1) >> 1;
    op[shortpitch * 1] = (c1 + d1) >> 1;
    op[shortpitch * 2] = (a1 - b1) >> 1;
    op[shortpitch * 3] = (d1 - c1) >> 1;

    ip++;
    op++;
  }
}

void vp8_short_inv_walsh4x4_1_x8_c(short *in, short *out, int pitch) {
  int i;
  short tmp[4];
  short *ip = in;
  short *op = tmp;
  int shortpitch = pitch >> 1;

  op[0] = ((ip[0] >> WHT_UPSCALE_FACTOR) + 1) >> 1;
  op[1] = op[2] = op[3] = ((ip[0] >> WHT_UPSCALE_FACTOR) >> 1);


  ip = tmp;
  op = out;
  for (i = 0; i < 4; i++) {
    op[shortpitch * 0] = (ip[0] + 1) >> 1;
    op[shortpitch * 1] = op[shortpitch * 2] = op[shortpitch * 3] = ip[0] >> 1;
    ip++;
    op++;
  }
}

void vp8_dc_only_inv_walsh_add_c(short input_dc, unsigned char *pred_ptr, unsigned char *dst_ptr, int pitch, int stride) {
  int r, c;
  short tmp[16];
  vp8_short_inv_walsh4x4_1_x8_c(&input_dc, tmp, 4 << 1);

  for (r = 0; r < 4; r++) {
    for (c = 0; c < 4; c++) {
      int a = tmp[r * 4 + c] + pred_ptr[c];
      if (a < 0)
        a = 0;

      if (a > 255)
        a = 255;

      dst_ptr[c] = (unsigned char) a;
    }

    dst_ptr += stride;
    pred_ptr += pitch;
  }
}
#endif

void vp8_dc_only_idct_add_8x8_c(short input_dc,
                                unsigned char *pred_ptr,
                                unsigned char *dst_ptr,
                                int pitch, int stride) {
  int a1 = ((input_dc + 16) >> 5);
  int r, c, b;
  unsigned char *orig_pred = pred_ptr;
  unsigned char *orig_dst = dst_ptr;
  for (b = 0; b < 4; b++) {
    for (r = 0; r < 4; r++) {
      for (c = 0; c < 4; c++) {
        int a = a1 + pred_ptr[c];

        if (a < 0)
          a = 0;

        if (a > 255)
          a = 255;

        dst_ptr[c] = (unsigned char) a;
      }

      dst_ptr += stride;
      pred_ptr += pitch;
    }
    dst_ptr = orig_dst + (b + 1) % 2 * 4 + (b + 1) / 2 * 4 * stride;
    pred_ptr = orig_pred + (b + 1) % 2 * 4 + (b + 1) / 2 * 4 * pitch;
  }
}

#define W1 2841                 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676                 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408                 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609                 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108                 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565                  /* 2048*sqrt(2)*cos(7*pi/16) */

/* row (horizontal) IDCT
 *
 * 7                       pi         1 dst[k] = sum c[l] * src[l] * cos( -- *
 * ( k + - ) * l ) l=0                      8          2
 *
 * where: c[0]    = 128 c[1..7] = 128*sqrt(2) */

static void idctrow(int *blk) {
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;
  /* shortcut */
  if (!((x1 = blk[4] << 11) | (x2 = blk[6]) | (x3 = blk[2]) |
        (x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3]))) {
    blk[0] = blk[1] = blk[2] = blk[3] = blk[4]
                                        = blk[5] = blk[6] = blk[7] = blk[0] << 3;
    return;
  }

  x0 = (blk[0] << 11) + 128;    /* for proper rounding in the fourth stage */
  /* first stage */
  x8 = W7 * (x4 + x5);
  x4 = x8 + (W1 - W7) * x4;
  x5 = x8 - (W1 + W7) * x5;
  x8 = W3 * (x6 + x7);
  x6 = x8 - (W3 - W5) * x6;
  x7 = x8 - (W3 + W5) * x7;

  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6 * (x3 + x2);
  x2 = x1 - (W2 + W6) * x2;
  x3 = x1 + (W2 - W6) * x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;

  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181 * (x4 + x5) + 128) >> 8;
  x4 = (181 * (x4 - x5) + 128) >> 8;

  /* fourth stage */
  blk[0] = (x7 + x1) >> 8;
  blk[1] = (x3 + x2) >> 8;
  blk[2] = (x0 + x4) >> 8;
  blk[3] = (x8 + x6) >> 8;
  blk[4] = (x8 - x6) >> 8;
  blk[5] = (x0 - x4) >> 8;
  blk[6] = (x3 - x2) >> 8;
  blk[7] = (x7 - x1) >> 8;
}

/* column (vertical) IDCT
 *
 * 7                         pi         1 dst[8*k] = sum c[l] * src[8*l] *
 * cos( -- * ( k + - ) * l ) l=0                        8          2
 *
 * where: c[0]    = 1/1024 c[1..7] = (1/1024)*sqrt(2) */
static void idctcol(int *blk) {
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  /* shortcut */
  if (!((x1 = (blk[8 * 4] << 8)) | (x2 = blk[8 * 6]) | (x3 = blk[8 * 2]) |
        (x4 = blk[8 * 1]) | (x5 = blk[8 * 7]) | (x6 = blk[8 * 5]) |
        (x7 = blk[8 * 3]))) {
    blk[8 * 0] = blk[8 * 1] = blk[8 * 2] = blk[8 * 3]
                                           = blk[8 * 4] = blk[8 * 5] = blk[8 * 6]
                                                                       = blk[8 * 7] = ((blk[8 * 0] + 32) >> 6);
    return;
  }

  x0 = (blk[8 * 0] << 8) + 16384;

  /* first stage */
  x8 = W7 * (x4 + x5) + 4;
  x4 = (x8 + (W1 - W7) * x4) >> 3;
  x5 = (x8 - (W1 + W7) * x5) >> 3;
  x8 = W3 * (x6 + x7) + 4;
  x6 = (x8 - (W3 - W5) * x6) >> 3;
  x7 = (x8 - (W3 + W5) * x7) >> 3;

  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6 * (x3 + x2) + 4;
  x2 = (x1 - (W2 + W6) * x2) >> 3;
  x3 = (x1 + (W2 - W6) * x3) >> 3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;

  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181 * (x4 + x5) + 128) >> 8;
  x4 = (181 * (x4 - x5) + 128) >> 8;

  /* fourth stage */
  blk[8 * 0] = (x7 + x1) >> 14;
  blk[8 * 1] = (x3 + x2) >> 14;
  blk[8 * 2] = (x0 + x4) >> 14;
  blk[8 * 3] = (x8 + x6) >> 14;
  blk[8 * 4] = (x8 - x6) >> 14;
  blk[8 * 5] = (x0 - x4) >> 14;
  blk[8 * 6] = (x3 - x2) >> 14;
  blk[8 * 7] = (x7 - x1) >> 14;
}

#define TX_DIM 8
void vp8_short_idct8x8_c(short *coefs, short *block, int pitch) {
  int X[TX_DIM * TX_DIM];
  int i, j;
  int shortpitch = pitch >> 1;

  for (i = 0; i < TX_DIM; i++) {
    for (j = 0; j < TX_DIM; j++) {
      X[i * TX_DIM + j] = (int)(coefs[i * TX_DIM + j] + 1
                                + (coefs[i * TX_DIM + j] < 0)) >> 2;
    }
  }
  for (i = 0; i < 8; i++)
    idctrow(X + 8 * i);

  for (i = 0; i < 8; i++)
    idctcol(X + i);

  for (i = 0; i < TX_DIM; i++) {
    for (j = 0; j < TX_DIM; j++) {
      block[i * shortpitch + j]  = X[i * TX_DIM + j] >> 1;
    }
  }
}


void vp8_short_ihaar2x2_c(short *input, short *output, int pitch) {
  int i;
  short *ip = input; // 0,1, 4, 8
  short *op = output;
  for (i = 0; i < 16; i++) {
    op[i] = 0;
  }

  op[0] = (ip[0] + ip[1] + ip[4] + ip[8] + 1) >> 1;
  op[1] = (ip[0] - ip[1] + ip[4] - ip[8]) >> 1;
  op[4] = (ip[0] + ip[1] - ip[4] - ip[8]) >> 1;
  op[8] = (ip[0] - ip[1] - ip[4] + ip[8]) >> 1;
}

