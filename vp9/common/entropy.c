/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <stdio.h>

#include "entropy.h"
#include "string.h"
#include "blockd.h"
#include "onyxc_int.h"
#include "entropymode.h"
#include "vpx_mem/vpx_mem.h"

#define uchar unsigned char     /* typedefs can clash */
#define uint  unsigned int

typedef const uchar cuchar;
typedef const uint cuint;

typedef vp9_prob Prob;

#include "coefupdateprobs.h"

const int vp9_i8x8_block[4] = {0, 2, 8, 10};

DECLARE_ALIGNED(16, const unsigned char, vp9_norm[256]) = {
  0, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

DECLARE_ALIGNED(16, const int, vp9_coef_bands[16]) = {
  0, 1, 2, 3, 6, 4, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7
};

DECLARE_ALIGNED(16, cuchar, vp9_prev_token_class[MAX_ENTROPY_TOKENS]) = {
  0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 0
};

DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d[16]) = {
  0,  1,  4,  8,
  5,  2,  3,  6,
  9, 12, 13, 10,
  7, 11, 14, 15,
};

DECLARE_ALIGNED(16, const int, vp9_col_scan[16]) = {
  0, 4,  8, 12,
  1, 5,  9, 13,
  2, 6, 10, 14,
  3, 7, 11, 15
};

DECLARE_ALIGNED(16, const int, vp9_row_scan[16]) = {
  0,   1,  2,  3,
  4,   5,  6,  7,
  8,   9, 10, 11,
  12, 13, 14, 15
};

DECLARE_ALIGNED(64, const int, vp9_coef_bands_8x8[64]) = { 0, 1, 2, 3, 5, 4, 4, 5,
  5, 3, 6, 3, 5, 4, 6, 6,
  6, 5, 5, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7
};
DECLARE_ALIGNED(64, const int, vp9_default_zig_zag1d_8x8[64]) = {
  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

// Table can be optimized.
DECLARE_ALIGNED(16, const int, vp9_coef_bands_16x16[256]) = {
  0, 1, 2, 3, 5, 4, 4, 5, 5, 3, 6, 3, 5, 4, 6, 6,
  6, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};

DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d_16x16[256]) = {
  0,   1,  16,  32,  17,   2,   3,  18,  33,  48,  64,  49,  34,  19,   4,   5,
  20,  35,  50,  65,  80,  96,  81,  66,  51,  36,  21,   6,   7,  22,  37,  52,
  67,  82,  97, 112, 128, 113,  98,  83,  68,  53,  38,  23,   8,   9,  24,  39,
  54,  69,  84,  99, 114, 129, 144, 160, 145, 130, 115, 100,  85,  70,  55,  40,
  25,  10,  11,  26,  41,  56,  71,  86, 101, 116, 131, 146, 161, 176, 192, 177,
  162, 147, 132, 117, 102,  87,  72,  57,  42,  27,  12,  13,  28,  43,  58,  73,
  88, 103, 118, 133, 148, 163, 178, 193, 208, 224, 209, 194, 179, 164, 149, 134,
  119, 104,  89,  74,  59,  44,  29,  14,  15,  30,  45,  60,  75,  90, 105, 120,
  135, 150, 165, 180, 195, 210, 225, 240, 241, 226, 211, 196, 181, 166, 151, 136,
  121, 106,  91,  76,  61,  46,  31,  47,  62,  77,  92, 107, 122, 137, 152, 167,
  182, 197, 212, 227, 242, 243, 228, 213, 198, 183, 168, 153, 138, 123, 108,  93,
  78,  63,  79,  94, 109, 124, 139, 154, 169, 184, 199, 214, 229, 244, 245, 230,
  215, 200, 185, 170, 155, 140, 125, 110,  95, 111, 126, 141, 156, 171, 186, 201,
  216, 231, 246, 247, 232, 217, 202, 187, 172, 157, 142, 127, 143, 158, 173, 188,
  203, 218, 233, 248, 249, 234, 219, 204, 189, 174, 159, 175, 190, 205, 220, 235,
  250, 251, 236, 221, 206, 191, 207, 222, 237, 252, 253, 238, 223, 239, 254, 255,
};


#if CONFIG_NEWCOEFCONTEXT
DECLARE_ALIGNED(16, const int,
                vp9_default_zig_zag1d_neighbors[16 * MAX_NEIGHBORS]) = {
  -1, -1, -1, -1, -1,
  -1,  0, -1, -1, -1,
  -1,  1, -1, -1,  5,
  -1,  2, -1, -1, -1,
   0, -1, -1,  1, -1,
   1,  4,  0, -1,  8,
   2,  5,  1,  3, -1,
   3,  6,  2, -1, 10,
   4, -1, -1, -1, -1,
   5,  8,  4,  6, -1,
   6,  9,  5, -1, 13,
   7, 10,  6, -1, -1,
   8, -1, -1,  9, -1,
   9, 12,  8, -1, -1,
   10, 13,  9, 11, -1,
   11, 14, 10, -1, -1,
};

DECLARE_ALIGNED(16, const int,
                vp9_col_scan_neighbors[16 * MAX_NEIGHBORS]) = {
  -1, -1, -1, -1, -1,
  -1,  0, -1, -1,  4,
  -1,  1, -1, -1,  5,
  -1,  2, -1, -1,  6,
  0, -1, -1, -1, -1,
  1,  4,  0, -1,  8,
  2,  5,  1, -1,  9,
  3,  6,  2, -1, 10,
  4, -1, -1, -1, -1,
  5,  8,  4, -1, 12,
  6,  9,  5, -1, 13,
  7, 10,  6, -1, 14,
  8, -1, -1, -1, -1,
  9, 12,  8, -1, -1,
  10, 13,  9, -1, -1,
  11, 14, 10, -1, -1
};

DECLARE_ALIGNED(16, const int,
                vp9_row_scan_neighbors[16 * MAX_NEIGHBORS]) = {
  -1, -1, -1, -1, -1,
  -1,  0, -1, -1, -1,
  -1,  1, -1, -1, -1,
  -1,  2, -1, -1, -1,
  0, -1, -1,  1, -1,
  1,  4,  0,  2, -1,
  2,  5,  1,  3, -1,
  3,  6,  2, -1, -1,
  4, -1, -1,  5, -1,
  5,  8,  4,  6, -1,
  6,  9,  5,  7, -1,
  7, 10,  6, -1, -1,
  8, -1, -1,  9, -1,
  9, 12,  8, 10, -1,
  10, 13,  9, 11, -1,
  11, 14, 10, -1, -1
};

DECLARE_ALIGNED(16, const int,
                vp9_default_zig_zag1d_8x8_neighbors[64 * MAX_NEIGHBORS]) = {
  -1, -1, -1, -1, -1,
  -1,  0, -1, -1, -1,
  -1,  1, -1, -1,  9,
  -1,  2, -1, -1, -1,
  -1,  3, -1, -1, 11,
  -1,  4, -1, -1, -1,
  -1,  5, -1, -1, 13,
  -1,  6, -1, -1, -1,
  0, -1, -1,  1, -1,
  1,  8,  0, -1, 16,
  2,  9,  1,  3, -1,
  3, 10,  2, -1, 18,
  4, 11,  3,  5, -1,
  5, 12,  4, -1, 20,
  6, 13,  5,  7, -1,
  7, 14,  6, -1, 22,
  8, -1, -1, -1, -1,
  9, 16,  8, 10, -1,
  10, 17,  9, -1, 25,
  11, 18, 10, 12, -1,
  12, 19, 11, -1, 27,
  13, 20, 12, 14, -1,
  14, 21, 13, -1, 29,
  15, 22, 14, -1, -1,
  16, -1, -1, 17, -1,
  17, 24, 16, -1, 32,
  18, 25, 17, 19, -1,
  19, 26, 18, -1, 34,
  20, 27, 19, 21, -1,
  21, 28, 20, -1, 36,
  22, 29, 21, 23, -1,
  23, 30, 22, -1, 38,
  24, -1, -1, -1, -1,
  25, 32, 24, 26, -1,
  26, 33, 25, -1, 41,
  27, 34, 26, 28, -1,
  28, 35, 27, -1, 43,
  29, 36, 28, 30, -1,
  30, 37, 29, -1, 45,
  31, 38, 30, -1, -1,
  32, -1, -1, 33, -1,
  33, 40, 32, -1, 48,
  34, 41, 33, 35, -1,
  35, 42, 34, -1, 50,
  36, 43, 35, 37, -1,
  37, 44, 36, -1, 52,
  38, 45, 37, 39, -1,
  39, 46, 38, -1, 54,
  40, -1, -1, -1, -1,
  41, 48, 40, 42, -1,
  42, 49, 41, -1, 57,
  43, 50, 42, 44, -1,
  44, 51, 43, -1, 59,
  45, 52, 44, 46, -1,
  46, 53, 45, -1, 61,
  47, 54, 46, -1, -1,
  48, -1, -1, 49, -1,
  49, 56, 48, -1, -1,
  50, 57, 49, 51, -1,
  51, 58, 50, -1, -1,
  52, 59, 51, 53, -1,
  53, 60, 52, -1, -1,
  54, 61, 53, 55, -1,
  55, 62, 54, -1, -1,
};

DECLARE_ALIGNED(16, const int,
                vp9_default_zig_zag1d_16x16_neighbors[256 * MAX_NEIGHBORS]) = {
  -1,  -1,  -1,  -1,  -1,
  -1,   0,  -1,  -1,  -1,
  -1,   1,  -1,  -1,  17,
  -1,   2,  -1,  -1,  -1,
  -1,   3,  -1,  -1,  19,
  -1,   4,  -1,  -1,  -1,
  -1,   5,  -1,  -1,  21,
  -1,   6,  -1,  -1,  -1,
  -1,   7,  -1,  -1,  23,
  -1,   8,  -1,  -1,  -1,
  -1,   9,  -1,  -1,  25,
  -1,  10,  -1,  -1,  -1,
  -1,  11,  -1,  -1,  27,
  -1,  12,  -1,  -1,  -1,
  -1,  13,  -1,  -1,  29,
  -1,  14,  -1,  -1,  -1,
  0,  -1,  -1,   1,  -1,
  1,  16,   0,  -1,  32,
  2,  17,   1,   3,  -1,
  3,  18,   2,  -1,  34,
  4,  19,   3,   5,  -1,
  5,  20,   4,  -1,  36,
  6,  21,   5,   7,  -1,
  7,  22,   6,  -1,  38,
  8,  23,   7,   9,  -1,
  9,  24,   8,  -1,  40,
  10,  25,   9,  11,  -1,
  11,  26,  10,  -1,  42,
  12,  27,  11,  13,  -1,
  13,  28,  12,  -1,  44,
  14,  29,  13,  15,  -1,
  15,  30,  14,  -1,  46,
  16,  -1,  -1,  -1,  -1,
  17,  32,  16,  18,  -1,
  18,  33,  17,  -1,  49,
  19,  34,  18,  20,  -1,
  20,  35,  19,  -1,  51,
  21,  36,  20,  22,  -1,
  22,  37,  21,  -1,  53,
  23,  38,  22,  24,  -1,
  24,  39,  23,  -1,  55,
  25,  40,  24,  26,  -1,
  26,  41,  25,  -1,  57,
  27,  42,  26,  28,  -1,
  28,  43,  27,  -1,  59,
  29,  44,  28,  30,  -1,
  30,  45,  29,  -1,  61,
  31,  46,  30,  -1,  -1,
  32,  -1,  -1,  33,  -1,
  33,  48,  32,  -1,  64,
  34,  49,  33,  35,  -1,
  35,  50,  34,  -1,  66,
  36,  51,  35,  37,  -1,
  37,  52,  36,  -1,  68,
  38,  53,  37,  39,  -1,
  39,  54,  38,  -1,  70,
  40,  55,  39,  41,  -1,
  41,  56,  40,  -1,  72,
  42,  57,  41,  43,  -1,
  43,  58,  42,  -1,  74,
  44,  59,  43,  45,  -1,
  45,  60,  44,  -1,  76,
  46,  61,  45,  47,  -1,
  47,  62,  46,  -1,  78,
  48,  -1,  -1,  -1,  -1,
  49,  64,  48,  50,  -1,
  50,  65,  49,  -1,  81,
  51,  66,  50,  52,  -1,
  52,  67,  51,  -1,  83,
  53,  68,  52,  54,  -1,
  54,  69,  53,  -1,  85,
  55,  70,  54,  56,  -1,
  56,  71,  55,  -1,  87,
  57,  72,  56,  58,  -1,
  58,  73,  57,  -1,  89,
  59,  74,  58,  60,  -1,
  60,  75,  59,  -1,  91,
  61,  76,  60,  62,  -1,
  62,  77,  61,  -1,  93,
  63,  78,  62,  -1,  -1,
  64,  -1,  -1,  65,  -1,
  65,  80,  64,  -1,  96,
  66,  81,  65,  67,  -1,
  67,  82,  66,  -1,  98,
  68,  83,  67,  69,  -1,
  69,  84,  68,  -1, 100,
  70,  85,  69,  71,  -1,
  71,  86,  70,  -1, 102,
  72,  87,  71,  73,  -1,
  73,  88,  72,  -1, 104,
  74,  89,  73,  75,  -1,
  75,  90,  74,  -1, 106,
  76,  91,  75,  77,  -1,
  77,  92,  76,  -1, 108,
  78,  93,  77,  79,  -1,
  79,  94,  78,  -1, 110,
  80,  -1,  -1,  -1,  -1,
  81,  96,  80,  82,  -1,
  82,  97,  81,  -1, 113,
  83,  98,  82,  84,  -1,
  84,  99,  83,  -1, 115,
  85, 100,  84,  86,  -1,
  86, 101,  85,  -1, 117,
  87, 102,  86,  88,  -1,
  88, 103,  87,  -1, 119,
  89, 104,  88,  90,  -1,
  90, 105,  89,  -1, 121,
  91, 106,  90,  92,  -1,
  92, 107,  91,  -1, 123,
  93, 108,  92,  94,  -1,
  94, 109,  93,  -1, 125,
  95, 110,  94,  -1,  -1,
  96,  -1,  -1,  97,  -1,
  97, 112,  96,  -1, 128,
  98, 113,  97,  99,  -1,
  99, 114,  98,  -1, 130,
  100, 115,  99, 101,  -1,
  101, 116, 100,  -1, 132,
  102, 117, 101, 103,  -1,
  103, 118, 102,  -1, 134,
  104, 119, 103, 105,  -1,
  105, 120, 104,  -1, 136,
  106, 121, 105, 107,  -1,
  107, 122, 106,  -1, 138,
  108, 123, 107, 109,  -1,
  109, 124, 108,  -1, 140,
  110, 125, 109, 111,  -1,
  111, 126, 110,  -1, 142,
  112,  -1,  -1,  -1,  -1,
  113, 128, 112, 114,  -1,
  114, 129, 113,  -1, 145,
  115, 130, 114, 116,  -1,
  116, 131, 115,  -1, 147,
  117, 132, 116, 118,  -1,
  118, 133, 117,  -1, 149,
  119, 134, 118, 120,  -1,
  120, 135, 119,  -1, 151,
  121, 136, 120, 122,  -1,
  122, 137, 121,  -1, 153,
  123, 138, 122, 124,  -1,
  124, 139, 123,  -1, 155,
  125, 140, 124, 126,  -1,
  126, 141, 125,  -1, 157,
  127, 142, 126,  -1,  -1,
  128,  -1,  -1, 129,  -1,
  129, 144, 128,  -1, 160,
  130, 145, 129, 131,  -1,
  131, 146, 130,  -1, 162,
  132, 147, 131, 133,  -1,
  133, 148, 132,  -1, 164,
  134, 149, 133, 135,  -1,
  135, 150, 134,  -1, 166,
  136, 151, 135, 137,  -1,
  137, 152, 136,  -1, 168,
  138, 153, 137, 139,  -1,
  139, 154, 138,  -1, 170,
  140, 155, 139, 141,  -1,
  141, 156, 140,  -1, 172,
  142, 157, 141, 143,  -1,
  143, 158, 142,  -1, 174,
  144,  -1,  -1,  -1,  -1,
  145, 160, 144, 146,  -1,
  146, 161, 145,  -1, 177,
  147, 162, 146, 148,  -1,
  148, 163, 147,  -1, 179,
  149, 164, 148, 150,  -1,
  150, 165, 149,  -1, 181,
  151, 166, 150, 152,  -1,
  152, 167, 151,  -1, 183,
  153, 168, 152, 154,  -1,
  154, 169, 153,  -1, 185,
  155, 170, 154, 156,  -1,
  156, 171, 155,  -1, 187,
  157, 172, 156, 158,  -1,
  158, 173, 157,  -1, 189,
  159, 174, 158,  -1,  -1,
  160,  -1,  -1, 161,  -1,
  161, 176, 160,  -1, 192,
  162, 177, 161, 163,  -1,
  163, 178, 162,  -1, 194,
  164, 179, 163, 165,  -1,
  165, 180, 164,  -1, 196,
  166, 181, 165, 167,  -1,
  167, 182, 166,  -1, 198,
  168, 183, 167, 169,  -1,
  169, 184, 168,  -1, 200,
  170, 185, 169, 171,  -1,
  171, 186, 170,  -1, 202,
  172, 187, 171, 173,  -1,
  173, 188, 172,  -1, 204,
  174, 189, 173, 175,  -1,
  175, 190, 174,  -1, 206,
  176,  -1,  -1,  -1,  -1,
  177, 192, 176, 178,  -1,
  178, 193, 177,  -1, 209,
  179, 194, 178, 180,  -1,
  180, 195, 179,  -1, 211,
  181, 196, 180, 182,  -1,
  182, 197, 181,  -1, 213,
  183, 198, 182, 184,  -1,
  184, 199, 183,  -1, 215,
  185, 200, 184, 186,  -1,
  186, 201, 185,  -1, 217,
  187, 202, 186, 188,  -1,
  188, 203, 187,  -1, 219,
  189, 204, 188, 190,  -1,
  190, 205, 189,  -1, 221,
  191, 206, 190,  -1,  -1,
  192,  -1,  -1, 193,  -1,
  193, 208, 192,  -1, 224,
  194, 209, 193, 195,  -1,
  195, 210, 194,  -1, 226,
  196, 211, 195, 197,  -1,
  197, 212, 196,  -1, 228,
  198, 213, 197, 199,  -1,
  199, 214, 198,  -1, 230,
  200, 215, 199, 201,  -1,
  201, 216, 200,  -1, 232,
  202, 217, 201, 203,  -1,
  203, 218, 202,  -1, 234,
  204, 219, 203, 205,  -1,
  205, 220, 204,  -1, 236,
  206, 221, 205, 207,  -1,
  207, 222, 206,  -1, 238,
  208,  -1,  -1,  -1,  -1,
  209, 224, 208, 210,  -1,
  210, 225, 209,  -1, 241,
  211, 226, 210, 212,  -1,
  212, 227, 211,  -1, 243,
  213, 228, 212, 214,  -1,
  214, 229, 213,  -1, 245,
  215, 230, 214, 216,  -1,
  216, 231, 215,  -1, 247,
  217, 232, 216, 218,  -1,
  218, 233, 217,  -1, 249,
  219, 234, 218, 220,  -1,
  220, 235, 219,  -1, 251,
  221, 236, 220, 222,  -1,
  222, 237, 221,  -1, 253,
  223, 238, 222,  -1,  -1,
  224,  -1,  -1, 225,  -1,
  225, 240, 224,  -1,  -1,
  226, 241, 225, 227,  -1,
  227, 242, 226,  -1,  -1,
  228, 243, 227, 229,  -1,
  229, 244, 228,  -1,  -1,
  230, 245, 229, 231,  -1,
  231, 246, 230,  -1,  -1,
  232, 247, 231, 233,  -1,
  233, 248, 232,  -1,  -1,
  234, 249, 233, 235,  -1,
  235, 250, 234,  -1,  -1,
  236, 251, 235, 237,  -1,
  237, 252, 236,  -1,  -1,
  238, 253, 237, 239,  -1,
  239, 254, 238,  -1,  -1,
};
#endif

/* Array indices are identical to previously-existing CONTEXT_NODE indices */

const vp9_tree_index vp9_coef_tree[ 22] =     /* corresponding _CONTEXT_NODEs */
{
  -DCT_EOB_TOKEN, 2,                             /* 0 = EOB */
  -ZERO_TOKEN, 4,                               /* 1 = ZERO */
  -ONE_TOKEN, 6,                               /* 2 = ONE */
  8, 12,                                      /* 3 = LOW_VAL */
  -TWO_TOKEN, 10,                            /* 4 = TWO */
  -THREE_TOKEN, -FOUR_TOKEN,                /* 5 = THREE */
  14, 16,                                    /* 6 = HIGH_LOW */
  -DCT_VAL_CATEGORY1, -DCT_VAL_CATEGORY2,   /* 7 = CAT_ONE */
  18, 20,                                   /* 8 = CAT_THREEFOUR */
  -DCT_VAL_CATEGORY3, -DCT_VAL_CATEGORY4,  /* 9 = CAT_THREE */
  -DCT_VAL_CATEGORY5, -DCT_VAL_CATEGORY6   /* 10 = CAT_FIVE */
};

struct vp9_token_struct vp9_coef_encodings[MAX_ENTROPY_TOKENS];

/* Trees for extra bits.  Probabilities are constant and
   do not depend on previously encoded bits */

static const Prob Pcat1[] = { 159};
static const Prob Pcat2[] = { 165, 145};
static const Prob Pcat3[] = { 173, 148, 140};
static const Prob Pcat4[] = { 176, 155, 140, 135};
static const Prob Pcat5[] = { 180, 157, 141, 134, 130};
static const Prob Pcat6[] =
{ 254, 254, 252, 249, 243, 230, 196, 177, 153, 140, 133, 130, 129};

static vp9_tree_index cat1[2], cat2[4], cat3[6], cat4[8], cat5[10], cat6[26];

static void init_bit_tree(vp9_tree_index *p, int n) {
  int i = 0;

  while (++i < n) {
    p[0] = p[1] = i << 1;
    p += 2;
  }

  p[0] = p[1] = 0;
}

static void init_bit_trees() {
  init_bit_tree(cat1, 1);
  init_bit_tree(cat2, 2);
  init_bit_tree(cat3, 3);
  init_bit_tree(cat4, 4);
  init_bit_tree(cat5, 5);
  init_bit_tree(cat6, 13);
}

vp9_extra_bit_struct vp9_extra_bits[12] = {
  { 0, 0, 0, 0},
  { 0, 0, 0, 1},
  { 0, 0, 0, 2},
  { 0, 0, 0, 3},
  { 0, 0, 0, 4},
  { cat1, Pcat1, 1, 5},
  { cat2, Pcat2, 2, 7},
  { cat3, Pcat3, 3, 11},
  { cat4, Pcat4, 4, 19},
  { cat5, Pcat5, 5, 35},
  { cat6, Pcat6, 13, 67},
  { 0, 0, 0, 0}
};

#if CONFIG_NEWCOEFCONTEXT
const int *vp9_get_coef_neighbors_handle(const int *scan) {
  if (scan == vp9_default_zig_zag1d) {
    return vp9_default_zig_zag1d_neighbors;
  } else if (scan == vp9_row_scan) {
    return vp9_row_scan_neighbors;
  } else if (scan == vp9_col_scan) {
    return vp9_col_scan_neighbors;
  } else if (scan == vp9_default_zig_zag1d_8x8) {
    return vp9_default_zig_zag1d_8x8_neighbors;
  } else if (scan == vp9_default_zig_zag1d_16x16) {
    return vp9_default_zig_zag1d_16x16_neighbors;
  }
}

int vp9_get_coef_neighbor_context(const short int *qcoeff_ptr, int nodc,
                                  const int *neigbor_handle, int rc) {
  const int *nb = neigbor_handle + rc * MAX_NEIGHBORS;
  int i, v, maxval = 0;
  for (i = 0; i < MAX_NEIGHBORS; ++i) {
    if (nb[i] == -1 || (nb[i] == 0 && nodc)) continue;
    v = abs(qcoeff_ptr[nb[i]]);
    maxval = (v > maxval ? v : maxval);
  }
  if (maxval <= 1) return maxval;
  else if (maxval < 4) return 2;
  else return 3;
}
#endif

#include "default_coef_probs.h"

void vp9_default_coef_probs(VP9_COMMON *pc) {
  vpx_memcpy(pc->fc.coef_probs, default_coef_probs,
             sizeof(pc->fc.coef_probs));
  vpx_memcpy(pc->fc.hybrid_coef_probs, default_hybrid_coef_probs,
             sizeof(pc->fc.hybrid_coef_probs));

  vpx_memcpy(pc->fc.coef_probs_8x8, default_coef_probs_8x8,
             sizeof(pc->fc.coef_probs_8x8));
  vpx_memcpy(pc->fc.hybrid_coef_probs_8x8, default_hybrid_coef_probs_8x8,
             sizeof(pc->fc.hybrid_coef_probs_8x8));

  vpx_memcpy(pc->fc.coef_probs_16x16, default_coef_probs_16x16,
             sizeof(pc->fc.coef_probs_16x16));
  vpx_memcpy(pc->fc.hybrid_coef_probs_16x16,
             default_hybrid_coef_probs_16x16,
             sizeof(pc->fc.hybrid_coef_probs_16x16));
}

void vp9_coef_tree_initialize() {
  init_bit_trees();
  vp9_tokens_from_tree(vp9_coef_encodings, vp9_coef_tree);
}

// #define COEF_COUNT_TESTING

#define COEF_COUNT_SAT 24
#define COEF_MAX_UPDATE_FACTOR 112
#define COEF_COUNT_SAT_KEY 24
#define COEF_MAX_UPDATE_FACTOR_KEY 112
#define COEF_COUNT_SAT_AFTER_KEY 24
#define COEF_MAX_UPDATE_FACTOR_AFTER_KEY 128

void vp9_adapt_coef_probs(VP9_COMMON *cm) {
  int t, i, j, k, count;
  unsigned int branch_ct[ENTROPY_NODES][2];
  vp9_prob coef_probs[ENTROPY_NODES];
  int update_factor; /* denominator 256 */
  int factor;
  int count_sat;

  // printf("Frame type: %d\n", cm->frame_type);
  if (cm->frame_type == KEY_FRAME) {
    update_factor = COEF_MAX_UPDATE_FACTOR_KEY;
    count_sat = COEF_COUNT_SAT_KEY;
  } else if (cm->last_frame_type == KEY_FRAME) {
    update_factor = COEF_MAX_UPDATE_FACTOR_AFTER_KEY;  /* adapt quickly */
    count_sat = COEF_COUNT_SAT_AFTER_KEY;
  } else {
    update_factor = COEF_MAX_UPDATE_FACTOR;
    count_sat = COEF_COUNT_SAT;
  }

#ifdef COEF_COUNT_TESTING
  {
    printf("static const unsigned int\ncoef_counts"
           "[BLOCK_TYPES] [COEF_BANDS]"
           "[PREV_COEF_CONTEXTS] [MAX_ENTROPY_TOKENS] = {\n");
    for (i = 0; i < BLOCK_TYPES; ++i) {
      printf("  {\n");
      for (j = 0; j < COEF_BANDS; ++j) {
        printf("    {\n");
        for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
          printf("      {");
          for (t = 0; t < MAX_ENTROPY_TOKENS; ++t)
            printf("%d, ", cm->fc.coef_counts[i][j][k][t]);
          printf("},\n");
        }
        printf("    },\n");
      }
      printf("  },\n");
    }
    printf("};\n");
    printf("static const unsigned int\ncoef_counts_8x8"
           "[BLOCK_TYPES_8X8] [COEF_BANDS]"
           "[PREV_COEF_CONTEXTS] [MAX_ENTROPY_TOKENS] = {\n");
    for (i = 0; i < BLOCK_TYPES_8X8; ++i) {
      printf("  {\n");
      for (j = 0; j < COEF_BANDS; ++j) {
        printf("    {\n");
        for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
          printf("      {");
          for (t = 0; t < MAX_ENTROPY_TOKENS; ++t)
            printf("%d, ", cm->fc.coef_counts_8x8[i][j][k][t]);
          printf("},\n");
        }
        printf("    },\n");
      }
      printf("  },\n");
    }
    printf("};\n");
    printf("static const unsigned int\nhybrid_coef_counts"
           "[BLOCK_TYPES] [COEF_BANDS]"
           "[PREV_COEF_CONTEXTS] [MAX_ENTROPY_TOKENS] = {\n");
    for (i = 0; i < BLOCK_TYPES; ++i) {
      printf("  {\n");
      for (j = 0; j < COEF_BANDS; ++j) {
        printf("    {\n");
        for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
          printf("      {");
          for (t = 0; t < MAX_ENTROPY_TOKENS; ++t)
            printf("%d, ", cm->fc.hybrid_coef_counts[i][j][k][t]);
          printf("},\n");
        }
        printf("    },\n");
      }
      printf("  },\n");
    }
    printf("};\n");
  }
#endif

  for (i = 0; i < BLOCK_TYPES; ++i)
    for (j = 0; j < COEF_BANDS; ++j)
      for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
        if (k >= 3 && ((i == 0 && j == 1) || (i > 0 && j == 0)))
          continue;
        vp9_tree_probs_from_distribution(
          MAX_ENTROPY_TOKENS, vp9_coef_encodings, vp9_coef_tree,
          coef_probs, branch_ct, cm->fc.coef_counts [i][j][k],
          256, 1);
        for (t = 0; t < ENTROPY_NODES; ++t) {
          int prob;
          count = branch_ct[t][0] + branch_ct[t][1];
          count = count > count_sat ? count_sat : count;
          factor = (update_factor * count / count_sat);
          prob = ((int)cm->fc.pre_coef_probs[i][j][k][t] * (256 - factor) +
                  (int)coef_probs[t] * factor + 128) >> 8;
          if (prob <= 0) cm->fc.coef_probs[i][j][k][t] = 1;
          else if (prob > 255) cm->fc.coef_probs[i][j][k][t] = 255;
          else cm->fc.coef_probs[i][j][k][t] = prob;
        }
      }

  for (i = 0; i < BLOCK_TYPES; ++i)
    for (j = 0; j < COEF_BANDS; ++j)
      for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
        if (k >= 3 && ((i == 0 && j == 1) || (i > 0 && j == 0)))
          continue;
        vp9_tree_probs_from_distribution(
          MAX_ENTROPY_TOKENS, vp9_coef_encodings, vp9_coef_tree,
          coef_probs, branch_ct, cm->fc.hybrid_coef_counts [i][j][k],
          256, 1);
        for (t = 0; t < ENTROPY_NODES; ++t) {
          int prob;
          count = branch_ct[t][0] + branch_ct[t][1];
          count = count > count_sat ? count_sat : count;
          factor = (update_factor * count / count_sat);
          prob = ((int)cm->fc.pre_hybrid_coef_probs[i][j][k][t] * (256 - factor) +
                  (int)coef_probs[t] * factor + 128) >> 8;
          if (prob <= 0) cm->fc.hybrid_coef_probs[i][j][k][t] = 1;
          else if (prob > 255) cm->fc.hybrid_coef_probs[i][j][k][t] = 255;
          else cm->fc.hybrid_coef_probs[i][j][k][t] = prob;
        }
      }

  for (i = 0; i < BLOCK_TYPES_8X8; ++i)
    for (j = 0; j < COEF_BANDS; ++j)
      for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
        if (k >= 3 && ((i == 0 && j == 1) || (i > 0 && j == 0)))
          continue;
        vp9_tree_probs_from_distribution(
          MAX_ENTROPY_TOKENS, vp9_coef_encodings, vp9_coef_tree,
          coef_probs, branch_ct, cm->fc.coef_counts_8x8 [i][j][k],
          256, 1);
        for (t = 0; t < ENTROPY_NODES; ++t) {
          int prob;
          count = branch_ct[t][0] + branch_ct[t][1];
          count = count > count_sat ? count_sat : count;
          factor = (update_factor * count / count_sat);
          prob = ((int)cm->fc.pre_coef_probs_8x8[i][j][k][t] * (256 - factor) +
                  (int)coef_probs[t] * factor + 128) >> 8;
          if (prob <= 0) cm->fc.coef_probs_8x8[i][j][k][t] = 1;
          else if (prob > 255) cm->fc.coef_probs_8x8[i][j][k][t] = 255;
          else cm->fc.coef_probs_8x8[i][j][k][t] = prob;
        }
      }

  for (i = 0; i < BLOCK_TYPES_8X8; ++i)
    for (j = 0; j < COEF_BANDS; ++j)
      for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
        if (k >= 3 && ((i == 0 && j == 1) || (i > 0 && j == 0)))
          continue;
        vp9_tree_probs_from_distribution(
          MAX_ENTROPY_TOKENS, vp9_coef_encodings, vp9_coef_tree,
          coef_probs, branch_ct, cm->fc.hybrid_coef_counts_8x8 [i][j][k],
          256, 1);
        for (t = 0; t < ENTROPY_NODES; ++t) {
          int prob;
          count = branch_ct[t][0] + branch_ct[t][1];
          count = count > count_sat ? count_sat : count;
          factor = (update_factor * count / count_sat);
          prob = ((int)cm->fc.pre_hybrid_coef_probs_8x8[i][j][k][t] *
                  (256 - factor) +
                  (int)coef_probs[t] * factor + 128) >> 8;
          if (prob <= 0) cm->fc.hybrid_coef_probs_8x8[i][j][k][t] = 1;
          else if (prob > 255) cm->fc.hybrid_coef_probs_8x8[i][j][k][t] = 255;
          else cm->fc.hybrid_coef_probs_8x8[i][j][k][t] = prob;
        }
      }

  for (i = 0; i < BLOCK_TYPES_16X16; ++i)
    for (j = 0; j < COEF_BANDS; ++j)
      for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
        if (k >= 3 && ((i == 0 && j == 1) || (i > 0 && j == 0)))
          continue;
        vp9_tree_probs_from_distribution(
          MAX_ENTROPY_TOKENS, vp9_coef_encodings, vp9_coef_tree,
          coef_probs, branch_ct, cm->fc.coef_counts_16x16[i][j][k], 256, 1);
        for (t = 0; t < ENTROPY_NODES; ++t) {
          int prob;
          count = branch_ct[t][0] + branch_ct[t][1];
          count = count > count_sat ? count_sat : count;
          factor = (update_factor * count / count_sat);
          prob = ((int)cm->fc.pre_coef_probs_16x16[i][j][k][t] *
                  (256 - factor) +
                  (int)coef_probs[t] * factor + 128) >> 8;
          if (prob <= 0) cm->fc.coef_probs_16x16[i][j][k][t] = 1;
          else if (prob > 255) cm->fc.coef_probs_16x16[i][j][k][t] = 255;
          else cm->fc.coef_probs_16x16[i][j][k][t] = prob;
        }
      }

  for (i = 0; i < BLOCK_TYPES_16X16; ++i)
    for (j = 0; j < COEF_BANDS; ++j)
      for (k = 0; k < PREV_COEF_CONTEXTS; ++k) {
        if (k >= 3 && ((i == 0 && j == 1) || (i > 0 && j == 0)))
          continue;
        vp9_tree_probs_from_distribution(
          MAX_ENTROPY_TOKENS, vp9_coef_encodings, vp9_coef_tree,
          coef_probs, branch_ct, cm->fc.hybrid_coef_counts_16x16[i][j][k], 256, 1);
        for (t = 0; t < ENTROPY_NODES; ++t) {
          int prob;
          count = branch_ct[t][0] + branch_ct[t][1];
          count = count > count_sat ? count_sat : count;
          factor = (update_factor * count / count_sat);
          prob = ((int)cm->fc.pre_hybrid_coef_probs_16x16[i][j][k][t] * (256 - factor) +
                  (int)coef_probs[t] * factor + 128) >> 8;
          if (prob <= 0) cm->fc.hybrid_coef_probs_16x16[i][j][k][t] = 1;
          else if (prob > 255) cm->fc.hybrid_coef_probs_16x16[i][j][k][t] = 255;
          else cm->fc.hybrid_coef_probs_16x16[i][j][k][t] = prob;
        }
      }
}
