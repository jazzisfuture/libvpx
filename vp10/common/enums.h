/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP10_COMMON_ENUMS_H_
#define VP10_COMMON_ENUMS_H_

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MI_SIZE_LOG2 3
#define MI_BLOCK_SIZE_LOG2 (6 - MI_SIZE_LOG2)  // 64 = 2^6

#define MI_SIZE (1 << MI_SIZE_LOG2)  // pixels per mi-unit
#define MI_BLOCK_SIZE (1 << MI_BLOCK_SIZE_LOG2)  // mi-units per max block

#define MI_MASK (MI_BLOCK_SIZE - 1)

// Bitstream profiles indicated by 2-3 bits in the uncompressed header.
// 00: Profile 0.  8-bit 4:2:0 only.
// 10: Profile 1.  8-bit 4:4:4, 4:2:2, and 4:4:0.
// 01: Profile 2.  10-bit and 12-bit color only, with 4:2:0 sampling.
// 110: Profile 3. 10-bit and 12-bit color only, with 4:2:2/4:4:4/4:4:0
//                 sampling.
// 111: Undefined profile.
typedef enum BITSTREAM_PROFILE {
  PROFILE_0,
  PROFILE_1,
  PROFILE_2,
  PROFILE_3,
  MAX_PROFILES
} BITSTREAM_PROFILE;

#define BLOCK_4X4      0
#define BLOCK_4X8      1
#define BLOCK_8X4      2
#define BLOCK_8X8      3
#define BLOCK_8X16     4
#define BLOCK_16X8     5
#define BLOCK_16X16    6
#define BLOCK_16X32    7
#define BLOCK_32X16    8
#define BLOCK_32X32    9
#define BLOCK_32X64   10
#define BLOCK_64X32   11
#define BLOCK_64X64   12

#if CONFIG_EXT_PARTITION
#define BLOCK_64X128  13
#define BLOCK_128X64  14
#define BLOCK_128X128 15
#define BLOCK_SIZES   16
#else
#define BLOCK_SIZES   13
#endif  // CONFIG_EXT_PARTITION

#define BLOCK_INVALID (BLOCK_SIZES)
#define BLOCK_LARGEST (BLOCK_SIZES - 1)

typedef uint8_t BLOCK_SIZE;

typedef enum PARTITION_TYPE {
  PARTITION_NONE,
  PARTITION_HORZ,
  PARTITION_VERT,
  PARTITION_SPLIT,
  PARTITION_TYPES,
  PARTITION_INVALID = PARTITION_TYPES
} PARTITION_TYPE;

typedef char PARTITION_CONTEXT;
#define PARTITION_PLOFFSET   4  // number of probability models per block size
#define PARTITION_CONTEXTS (4 * PARTITION_PLOFFSET)

// block transform size
typedef uint8_t TX_SIZE;
#define TX_4X4   ((TX_SIZE)0)   // 4x4 transform
#define TX_8X8   ((TX_SIZE)1)   // 8x8 transform
#define TX_16X16 ((TX_SIZE)2)   // 16x16 transform
#define TX_32X32 ((TX_SIZE)3)   // 32x32 transform
#define TX_SIZES ((TX_SIZE)4)

// frame transform mode
typedef enum {
  ONLY_4X4            = 0,        // only 4x4 transform used
  ALLOW_8X8           = 1,        // allow block transform size up to 8x8
  ALLOW_16X16         = 2,        // allow block transform size up to 16x16
  ALLOW_32X32         = 3,        // allow block transform size up to 32x32
  TX_MODE_SELECT      = 4,        // transform specified for each block
  TX_MODES            = 5,
} TX_MODE;

typedef enum {
  DCT_DCT   = 0,                      // DCT  in both horizontal and vertical
  ADST_DCT  = 1,                      // ADST in vertical, DCT in horizontal
  DCT_ADST  = 2,                      // DCT  in vertical, ADST in horizontal
  ADST_ADST = 3,                      // ADST in both directions
#if CONFIG_EXT_TX
  FLIPADST_DCT = 4,
  DCT_FLIPADST = 5,
  FLIPADST_FLIPADST = 6,
  ADST_FLIPADST = 7,
  FLIPADST_ADST = 8,
  DST_DCT = 9,
  DCT_DST = 10,
  DST_ADST = 11,
  ADST_DST = 12,
  DST_FLIPADST = 13,
  FLIPADST_DST = 14,
  DST_DST = 15,
  IDTX = 16,
  V_DCT = 17,
  H_DCT = 18,
  V_ADST = 19,
  H_ADST = 20,
  V_FLIPADST = 21,
  H_FLIPADST = 22,
  V_DST = 23,
  H_DST = 24,
#endif  // CONFIG_EXT_TX
  TX_TYPES,
} TX_TYPE;


#if CONFIG_EXT_TX
#define EXT_TX_SIZES       4  // number of sizes that use extended transforms
#define EXT_TX_SETS_INTER  4  // Sets of transform selections for INTER
#define EXT_TX_SETS_INTRA  3  // Sets of transform selections for INTRA
#else
#define EXT_TX_SIZES       3  // number of sizes that use extended transforms
#endif  // CONFIG_EXT_TX

typedef enum {
  VP9_LAST_FLAG = 1 << 0,
#if CONFIG_EXT_REFS
  VP9_LAST2_FLAG = 1 << 1,
  VP9_LAST3_FLAG = 1 << 2,
  VP9_LAST4_FLAG = 1 << 3,
  VP9_GOLD_FLAG = 1 << 4,
  VP9_ALT_FLAG = 1 << 5,
#else
  VP9_GOLD_FLAG = 1 << 1,
  VP9_ALT_FLAG = 1 << 2,
#endif  // CONFIG_EXT_REFS
} VP9_REFFRAME;

typedef enum {
  PLANE_TYPE_Y  = 0,
  PLANE_TYPE_UV = 1,
  PLANE_TYPES
} PLANE_TYPE;

typedef enum {
  TWO_COLORS,
  THREE_COLORS,
  FOUR_COLORS,
  FIVE_COLORS,
  SIX_COLORS,
  SEVEN_COLORS,
  EIGHT_COLORS,
  PALETTE_SIZES
} PALETTE_SIZE;

typedef enum {
  PALETTE_COLOR_ONE,
  PALETTE_COLOR_TWO,
  PALETTE_COLOR_THREE,
  PALETTE_COLOR_FOUR,
  PALETTE_COLOR_FIVE,
  PALETTE_COLOR_SIX,
  PALETTE_COLOR_SEVEN,
  PALETTE_COLOR_EIGHT,
  PALETTE_COLORS
} PALETTE_COLOR;

#define DC_PRED    0       // Average of above and left pixels
#define V_PRED     1       // Vertical
#define H_PRED     2       // Horizontal
#define D45_PRED   3       // Directional 45  deg = round(arctan(1/1) * 180/pi)
#define D135_PRED  4       // Directional 135 deg = 180 - 45
#define D117_PRED  5       // Directional 117 deg = 180 - 63
#define D153_PRED  6       // Directional 153 deg = 180 - 27
#define D207_PRED  7       // Directional 207 deg = 180 + 27
#define D63_PRED   8       // Directional 63  deg = round(arctan(2/1) * 180/pi)
#define TM_PRED    9       // True-motion
#define NEARESTMV 10
#define NEARMV    11
#define ZEROMV    12
#define NEWMV     13
#if CONFIG_EXT_INTER
#define NEWFROMNEARMV     14
#define NEAREST_NEARESTMV 15
#define NEAREST_NEARMV    16
#define NEAR_NEARESTMV    17
#define NEAREST_NEWMV     18
#define NEW_NEARESTMV     19
#define NEAR_NEWMV        20
#define NEW_NEARMV        21
#define ZERO_ZEROMV       22
#define NEW_NEWMV         23
#define MB_MODE_COUNT     24
#else
#define MB_MODE_COUNT 14
#endif  // CONFIG_EXT_INTER
typedef uint8_t PREDICTION_MODE;

#define INTRA_MODES (TM_PRED + 1)

#if CONFIG_EXT_INTRA
typedef enum {
  FILTER_DC_PRED,
  FILTER_V_PRED,
  FILTER_H_PRED,
  FILTER_D45_PRED,
  FILTER_D135_PRED,
  FILTER_D117_PRED,
  FILTER_D153_PRED,
  FILTER_D207_PRED,
  FILTER_D63_PRED,
  FILTER_TM_PRED,
  EXT_INTRA_MODES,
} EXT_INTRA_MODE;

#define FILTER_INTRA_MODES (FILTER_TM_PRED + 1)
#define DIRECTIONAL_MODES (INTRA_MODES - 2)
#endif  // CONFIG_EXT_INTRA

#if CONFIG_EXT_INTER
#define INTER_MODES (1 + NEWFROMNEARMV - NEARESTMV)
#else
#define INTER_MODES (1 + NEWMV - NEARESTMV)
#endif  // CONFIG_EXT_INTER

#if CONFIG_EXT_INTER
#define INTER_COMPOUND_MODES (1 + NEW_NEWMV - NEAREST_NEARESTMV)
#endif  // CONFIG_EXT_INTER

#define SKIP_CONTEXTS 3

#if CONFIG_REF_MV
#define NMV_CONTEXTS 3

#define NEWMV_MODE_CONTEXTS  7
#define ZEROMV_MODE_CONTEXTS 2
#define REFMV_MODE_CONTEXTS  9
#define DRL_MODE_CONTEXTS    3

#define ZEROMV_OFFSET 3
#define REFMV_OFFSET  4

#define NEWMV_CTX_MASK ((1 << ZEROMV_OFFSET) - 1)
#define ZEROMV_CTX_MASK ((1 << (REFMV_OFFSET - ZEROMV_OFFSET)) - 1)
#define REFMV_CTX_MASK ((1 << (8 - REFMV_OFFSET)) - 1)

#define ALL_ZERO_FLAG_OFFSET   8
#define SKIP_NEARESTMV_OFFSET  9
#define SKIP_NEARMV_OFFSET    10
#define SKIP_NEARESTMV_SUB8X8_OFFSET 11
#endif

#define INTER_MODE_CONTEXTS 7

/* Segment Feature Masks */
#define MAX_MV_REF_CANDIDATES 2
#if CONFIG_REF_MV
#define MAX_REF_MV_STACK_SIZE 16
#define REF_CAT_LEVEL  160
#endif

#define INTRA_INTER_CONTEXTS 4
#define COMP_INTER_CONTEXTS 5
#define REF_CONTEXTS 5

#if CONFIG_VAR_TX
#define TXFM_PARTITION_CONTEXTS 9
typedef TX_SIZE TXFM_CONTEXT;
#endif

#if CONFIG_EXT_REFS
#define SINGLE_REFS 6
#define COMP_REFS 5
#else
#define SINGLE_REFS 3
#define COMP_REFS 2
#endif  // CONFIG_EXT_REFS

#if CONFIG_SUPERTX
#define PARTITION_SUPERTX_CONTEXTS 2
#define MAX_SUPERTX_BLOCK_SIZE BLOCK_32X32
#endif  // CONFIG_SUPERTX

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP10_COMMON_ENUMS_H_
