/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

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

// This shortcuts a conditional / case statement to convert a combination
// of partition and block type to a subsize.
const BLOCK_SIZE_TYPE partition_subsize[PARTITION_TYPES][BLOCK_SIZE_TYPES] =
  {{0,0,0,BLOCK_SIZE_SB8X8,0,0,BLOCK_SIZE_MB16X16,      // PARTITION_NONE
    0,0,BLOCK_SIZE_SB32X32,0,0,BLOCK_SIZE_SB64X64},
   {0,0,0,BLOCK_SIZE_SB8X4,0,0,BLOCK_SIZE_SB16X8,       // PARTITION_HORZ
    0,0,BLOCK_SIZE_SB32X16,0,0,BLOCK_SIZE_SB64X32},
   {0,0,0,BLOCK_SIZE_SB4X8,0,0,BLOCK_SIZE_SB8X16,       // PARTITION_VERT
    0,0,BLOCK_SIZE_SB16X32,0,0,BLOCK_SIZE_SB32X64},
   {0,0,0,BLOCK_SIZE_AB4X4,0,0,BLOCK_SIZE_SB8X8,        // PARTITION_SPLIT
    0,0,BLOCK_SIZE_MB16X16,0,0,BLOCK_SIZE_SB32X32}};
