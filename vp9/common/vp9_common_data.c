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
// Log 2 conversion lookup tables for modeinfo width and height
const int mi_width_log2_lookup[BLOCK_SIZE_TYPES] =
  {0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3};
const int mi_height_log2_lookup[BLOCK_SIZE_TYPES] =
  {0, 0, 0, 0, 1, 0, 1, 2, 1, 2, 3, 2, 3};

const BLOCK_SIZE_TYPE subsize_lookup[PARTITION_TYPES][BLOCK_SIZE_TYPES] = {
  {     // PARTITION_NONE
    BLOCK_SIZE_AB4X4, BLOCK_SIZE_SB4X8, BLOCK_SIZE_SB8X4,
    BLOCK_SIZE_SB8X8, BLOCK_SIZE_SB8X16, BLOCK_SIZE_SB16X8,
    BLOCK_SIZE_MB16X16, BLOCK_SIZE_SB16X32, BLOCK_SIZE_SB32X16,
    BLOCK_SIZE_SB32X32, BLOCK_SIZE_SB32X64, BLOCK_SIZE_SB64X32,
    BLOCK_SIZE_SB64X64,
  }, {  // PARTITION_HORZ
    BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB8X4, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB16X8, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB32X16, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB64X32,
  }, {  // PARTITION_VERT
    BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB4X8, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB8X16, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB16X32, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB32X64,
  }, {  // PARTITION_SPLIT
    BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_AB4X4, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB8X8, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_MB16X16, BLOCK_SIZE_TYPES, BLOCK_SIZE_TYPES,
    BLOCK_SIZE_SB32X32,
  }
};

const TX_SIZE max_txsize_lookup[BLOCK_SIZE_TYPES] = {
  TX_4X4, TX_4X4, TX_4X4,
  TX_8X8, TX_8X8, TX_8X8,
  TX_16X16, TX_16X16, TX_16X16,
  TX_32X32, TX_32X32, TX_32X32, TX_32X32
};
const TX_SIZE max_uv_txsize_lookup[BLOCK_SIZE_TYPES] = {
  TX_4X4, TX_4X4, TX_4X4,
  TX_4X4, TX_4X4, TX_4X4,
  TX_8X8, TX_8X8, TX_8X8,
  TX_16X16, TX_16X16, TX_16X16, TX_32X32
};

const BLOCK_SIZE_TYPE bsize_from_dim_lookup[5][5] = {
  {BLOCK_SIZE_AB4X4,   BLOCK_SIZE_SB4X8,   BLOCK_SIZE_SB4X8,
    BLOCK_SIZE_SB4X8,   BLOCK_SIZE_SB4X8},
  {BLOCK_SIZE_SB8X4,   BLOCK_SIZE_SB8X8,   BLOCK_SIZE_SB8X16,
    BLOCK_SIZE_SB8X16,  BLOCK_SIZE_SB8X16},
  {BLOCK_SIZE_SB16X8,  BLOCK_SIZE_SB16X8,  BLOCK_SIZE_MB16X16,
    BLOCK_SIZE_SB16X32, BLOCK_SIZE_SB16X32},
  {BLOCK_SIZE_SB32X16, BLOCK_SIZE_SB32X16, BLOCK_SIZE_SB32X16,
    BLOCK_SIZE_SB32X32, BLOCK_SIZE_SB32X64},
  {BLOCK_SIZE_SB64X32, BLOCK_SIZE_SB64X32, BLOCK_SIZE_SB64X32,
    BLOCK_SIZE_SB64X32, BLOCK_SIZE_SB64X64}
};

int sb_index_offset_lookup[BLOCK_SIZE_TYPES] = {
  0, 0, 0
};

void vp9_fill_lookups() {
  MACROBLOCKD xd;
  int sb_index_offset = (char*)&xd.sb_index - (char*)&xd;
  int mb_index_offset = (char*)&xd.mb_index - (char*)&xd;
  int ab_index_offset = (char*)&xd.ab_index - (char*)&xd;
  int b_index_offset = (char*)&xd.b_index - (char*)&xd;

  sb_index_offset_lookup[BLOCK_SIZE_AB4X4] =
  sb_index_offset_lookup[BLOCK_SIZE_SB4X8] =
  sb_index_offset_lookup[BLOCK_SIZE_SB8X4] = ab_index_offset;
  sb_index_offset_lookup[BLOCK_SIZE_SB16X8] =
  sb_index_offset_lookup[BLOCK_SIZE_SB8X16] =
  sb_index_offset_lookup[BLOCK_SIZE_SB8X8] = b_index_offset;
  sb_index_offset_lookup[BLOCK_SIZE_SB32X16] =
  sb_index_offset_lookup[BLOCK_SIZE_SB16X32] =
  sb_index_offset_lookup[BLOCK_SIZE_MB16X16] = mb_index_offset;
  sb_index_offset_lookup[BLOCK_SIZE_SB32X32] =
  sb_index_offset_lookup[BLOCK_SIZE_SB32X64] =
  sb_index_offset_lookup[BLOCK_SIZE_SB64X32] =
  sb_index_offset_lookup[BLOCK_SIZE_SB64X64] = sb_index_offset;
}
