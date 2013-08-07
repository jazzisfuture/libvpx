/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_common_data.h"

// Log 2 conversion lookup tables for block width and height
const int b_width_log2_lookup[BLOCK_SIZE_TYPES] =
  {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4};
const int b_height_log2_lookup[BLOCK_SIZE_TYPES] =
  {0, 1, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4};
const int num_4x4_blocks_wide_lookup[BLOCK_SIZE_TYPES] =
  {1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16};
const int num_4x4_blocks_high_lookup[BLOCK_SIZE_TYPES] =
  {1, 2, 1, 2, 4, 2, 4, 8, 4, 8, 16, 8, 16};
// Log 2 conversion lookup tables for modeinfo width and height
const int mi_width_log2_lookup[BLOCK_SIZE_TYPES] =
  {0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3};
const int num_8x8_blocks_wide_lookup[BLOCK_SIZE_TYPES] =
  {1, 1, 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8};
const int mi_height_log2_lookup[BLOCK_SIZE_TYPES] =
  {0, 0, 0, 0, 1, 0, 1, 2, 1, 2, 3, 2, 3};
const int num_8x8_blocks_high_lookup[BLOCK_SIZE_TYPES] =
  {1, 1, 1, 1, 2, 1, 2, 4, 2, 4, 8, 4, 8};

// MIN(3, MIN(b_width_log2(bsize), b_height_log2(bsize)))
const int size_group_lookup[BLOCK_SIZE_TYPES] =
  {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3};

const int num_pels_log2_lookup[BLOCK_SIZE_TYPES] =
  {4, 5, 5, 6, 7, 7, 8, 9, 9, 10, 11, 11, 12};


const PARTITION_TYPE partition_lookup[][BLOCK_SIZE_TYPES] = {
  {  // 4X4
    // 4X4, 4X8,8X4,8X8,8X16,16X8,16X16,16X32,32X16,32X32,32X64,64X32,64X64
    PARTITION_NONE, PARTITION_INVALID, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID, PARTITION_INVALID,
    PARTITION_INVALID
  }, {  // 8X8
    // 4X4, 4X8,8X4,8X8,8X16,16X8,16X16,16X32,32X16,32X32,32X64,64X32,64X64
    PARTITION_SPLIT, PARTITION_VERT, PARTITION_HORZ, PARTITION_NONE,
    PARTITION_INVALID, PARTITION_INVALID, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID, PARTITION_INVALID
  }, {  // 16X16
    // 4X4, 4X8,8X4,8X8,8X16,16X8,16X16,16X32,32X16,32X32,32X64,64X32,64X64
    PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT,
    PARTITION_VERT, PARTITION_HORZ, PARTITION_NONE, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID
  }, {  // 32X32
    // 4X4, 4X8,8X4,8X8,8X16,16X8,16X16,16X32,32X16,32X32,32X64,64X32,64X64
    PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT,
    PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_VERT,
    PARTITION_HORZ, PARTITION_NONE, PARTITION_INVALID,
    PARTITION_INVALID, PARTITION_INVALID
  }, {  // 64X64
    // 4X4, 4X8,8X4,8X8,8X16,16X8,16X16,16X32,32X16,32X32,32X64,64X32,64X64
    PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT,
    PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_SPLIT,
    PARTITION_SPLIT, PARTITION_SPLIT, PARTITION_VERT, PARTITION_HORZ,
    PARTITION_NONE
  }
};

const BLOCK_SIZE_TYPE subsize_lookup[PARTITION_TYPES][BLOCK_SIZE_TYPES] = {
  {     // PARTITION_NONE
    BLOCK_4X4,   BLOCK_4X8,   BLOCK_8X4,
    BLOCK_8X8,   BLOCK_8X16,  BLOCK_16X8,
    BLOCK_16X16, BLOCK_16X32, BLOCK_32X16,
    BLOCK_32X32, BLOCK_32X64, BLOCK_64X32,
    BLOCK_64X64,
  }, {  // PARTITION_HORZ
    BLOCK_INVALID, BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_8X4,     BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_16X8,    BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_32X16,   BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_64X32,
  }, {  // PARTITION_VERT
    BLOCK_INVALID, BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_4X8,     BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_8X16,    BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_16X32,   BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_32X64,
  }, {  // PARTITION_SPLIT
    BLOCK_INVALID, BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_4X4,     BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_8X8,     BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_16X16,   BLOCK_INVALID, BLOCK_INVALID,
    BLOCK_32X32,
  }
};

const TX_SIZE max_txsize_lookup[BLOCK_SIZE_TYPES] = {
  TX_4X4,   TX_4X4,   TX_4X4,
  TX_8X8,   TX_8X8,   TX_8X8,
  TX_16X16, TX_16X16, TX_16X16,
  TX_32X32, TX_32X32, TX_32X32, TX_32X32
};
const TX_SIZE max_uv_txsize_lookup[BLOCK_SIZE_TYPES] = {
  TX_4X4,   TX_4X4,   TX_4X4,
  TX_4X4,   TX_4X4,   TX_4X4,
  TX_8X8,   TX_8X8,   TX_8X8,
  TX_16X16, TX_16X16, TX_16X16, TX_32X32
};

const int block_pixels_lookup[BLOCK_SIZE_TYPES] = {
  16,   32,   32,
  64,   128,  128,
  256,  512,  512,
  1024, 2048, 2048,
  4096
};

const BLOCK_SIZE_TYPE ss_size_lookup[BLOCK_SIZE_TYPES][2][2] = {
//  ss_x == 0    ss_x == 0        ss_x == 1      ss_x == 1
//  ss_y == 0    ss_y == 1        ss_y == 0      ss_y == 1
  {{BLOCK_4X4,   BLOCK_INVALID}, {BLOCK_INVALID, BLOCK_INVALID}},
  {{BLOCK_4X8,   BLOCK_4X4},     {BLOCK_INVALID, BLOCK_INVALID}},
  {{BLOCK_8X4,   BLOCK_INVALID}, {BLOCK_4X4,     BLOCK_INVALID}},
  {{BLOCK_8X8,   BLOCK_8X4},     {BLOCK_4X8,     BLOCK_4X4}},
  {{BLOCK_8X16,  BLOCK_8X8},     {BLOCK_INVALID, BLOCK_4X8}},
  {{BLOCK_16X8,  BLOCK_INVALID}, {BLOCK_8X8,     BLOCK_8X4}},
  {{BLOCK_16X16, BLOCK_16X8},    {BLOCK_8X16,    BLOCK_8X8}},
  {{BLOCK_16X32, BLOCK_16X16},   {BLOCK_INVALID, BLOCK_8X16}},
  {{BLOCK_32X16, BLOCK_INVALID}, {BLOCK_16X16,   BLOCK_16X8}},
  {{BLOCK_32X32, BLOCK_32X16},   {BLOCK_16X32,   BLOCK_16X16}},
  {{BLOCK_32X64, BLOCK_32X32},   {BLOCK_INVALID, BLOCK_16X32}},
  {{BLOCK_64X32, BLOCK_INVALID}, {BLOCK_32X32,   BLOCK_32X16}},
  {{BLOCK_64X64, BLOCK_64X32},   {BLOCK_32X64,   BLOCK_32X32}},
};

