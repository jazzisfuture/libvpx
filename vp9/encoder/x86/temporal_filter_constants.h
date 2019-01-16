/*
 *  Copyright (c) 2019 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"

// Division using multiplication and shifting. The C implementation does:
// modifier *= 3;
// modifier /= index;
// where 'modifier' is a set of summed values and 'index' is the number of
// summed values.
//
// This equation works out to (m * 3) / i which reduces to:
// m * 3/4
// m * 1/2
// m * 1/3
//
// By pairing the multiply with a down shift by 16 (_mm_mulhi_epu16):
// m * C / 65536
// we can create a C to replicate the division.
//
// m * 49152 / 65536 = m * 3/4
// m * 32758 / 65536 = m * 1/2
// m * 21846 / 65536 = m * 0.3333
//
// These are loaded using an instruction expecting int16_t values but are used
// with _mm_mulhi_epu16(), which treats them as unsigned.
#define NEIGHBOR_CONSTANT_4 (int16_t)49152
#define NEIGHBOR_CONSTANT_5 (int16_t)39322
#define NEIGHBOR_CONSTANT_6 (int16_t)32768
#define NEIGHBOR_CONSTANT_7 (int16_t)28087
#define NEIGHBOR_CONSTANT_8 (int16_t)24576
#define NEIGHBOR_CONSTANT_9 (int16_t)21846
#define NEIGHBOR_CONSTANT_10 (int16_t)19661
#define NEIGHBOR_CONSTANT_11 (int16_t)17874
#define NEIGHBOR_CONSTANT_13 (int16_t)15124

DECLARE_ALIGNED(16, static const int16_t, LEFT_CORNER_NEIGHBORS_PLUS_1[8]) = {
  NEIGHBOR_CONSTANT_5, NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7
};

DECLARE_ALIGNED(16, static const int16_t, RIGHT_CORNER_NEIGHBORS_PLUS_1[8]) = {
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_5
};

DECLARE_ALIGNED(16, static const int16_t, LEFT_EDGE_NEIGHBORS_PLUS_1[8]) = {
  NEIGHBOR_CONSTANT_7,  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10
};

DECLARE_ALIGNED(16, static const int16_t, RIGHT_EDGE_NEIGHBORS_PLUS_1[8]) = {
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_7
};

DECLARE_ALIGNED(16, static const int16_t, MIDDLE_EDGE_NEIGHBORS_PLUS_1[8]) = {
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7
};

DECLARE_ALIGNED(16, static const int16_t, MIDDLE_CENTER_NEIGHBORS_PLUS_1[8]) = {
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10
};

DECLARE_ALIGNED(16, static const int16_t, LEFT_CORNER_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8
};

DECLARE_ALIGNED(16, static const int16_t, RIGHT_CORNER_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6
};

DECLARE_ALIGNED(16, static const int16_t, LEFT_EDGE_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_8,  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11
};

DECLARE_ALIGNED(16, static const int16_t, RIGHT_EDGE_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_8
};

DECLARE_ALIGNED(16, static const int16_t, MIDDLE_EDGE_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8
};

DECLARE_ALIGNED(16, static const int16_t, MIDDLE_CENTER_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11
};

DECLARE_ALIGNED(16, static const int16_t, TWO_CORNER_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6
};

DECLARE_ALIGNED(16, static const int16_t, TWO_EDGE_NEIGHBORS_PLUS_2[8]) = {
  NEIGHBOR_CONSTANT_8,  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_8
};

DECLARE_ALIGNED(16, static const int16_t, LEFT_CORNER_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_8,  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10
};

DECLARE_ALIGNED(16, static const int16_t, RIGHT_CORNER_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_8
};

DECLARE_ALIGNED(16, static const int16_t, LEFT_EDGE_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13
};

DECLARE_ALIGNED(16, static const int16_t, RIGHT_EDGE_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_10
};

DECLARE_ALIGNED(16, static const int16_t, MIDDLE_EDGE_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10
};

DECLARE_ALIGNED(16, static const int16_t, MIDDLE_CENTER_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13
};

DECLARE_ALIGNED(16, static const int16_t, TWO_CORNER_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_8,  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_8
};

DECLARE_ALIGNED(16, static const int16_t, TWO_EDGE_NEIGHBORS_PLUS_4[8]) = {
  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_10
};

static const int16_t *const LUMA_LEFT_COLUMN_NEIGHBORS[2] = {
  LEFT_CORNER_NEIGHBORS_PLUS_2, LEFT_EDGE_NEIGHBORS_PLUS_2
};

static const int16_t *const LUMA_MIDDLE_COLUMN_NEIGHBORS[2] = {
  MIDDLE_EDGE_NEIGHBORS_PLUS_2, MIDDLE_CENTER_NEIGHBORS_PLUS_2
};

static const int16_t *const LUMA_RIGHT_COLUMN_NEIGHBORS[2] = {
  RIGHT_CORNER_NEIGHBORS_PLUS_2, RIGHT_EDGE_NEIGHBORS_PLUS_2
};

static const int16_t *const CHROMA_NO_SS_LEFT_COLUMN_NEIGHBORS[2] = {
  LEFT_CORNER_NEIGHBORS_PLUS_1, LEFT_EDGE_NEIGHBORS_PLUS_1
};

static const int16_t *const CHROMA_NO_SS_MIDDLE_COLUMN_NEIGHBORS[2] = {
  MIDDLE_EDGE_NEIGHBORS_PLUS_1, MIDDLE_CENTER_NEIGHBORS_PLUS_1
};

static const int16_t *const CHROMA_NO_SS_RIGHT_COLUMN_NEIGHBORS[2] = {
  RIGHT_CORNER_NEIGHBORS_PLUS_1, RIGHT_EDGE_NEIGHBORS_PLUS_1
};

static const int16_t *const CHROMA_SINGLE_SS_LEFT_COLUMN_NEIGHBORS[2] = {
  LEFT_CORNER_NEIGHBORS_PLUS_2, LEFT_EDGE_NEIGHBORS_PLUS_2
};

static const int16_t *const CHROMA_SINGLE_SS_MIDDLE_COLUMN_NEIGHBORS[2] = {
  MIDDLE_EDGE_NEIGHBORS_PLUS_2, MIDDLE_CENTER_NEIGHBORS_PLUS_2
};

static const int16_t *const CHROMA_SINGLE_SS_RIGHT_COLUMN_NEIGHBORS[2] = {
  RIGHT_CORNER_NEIGHBORS_PLUS_2, RIGHT_EDGE_NEIGHBORS_PLUS_2
};

static const int16_t *const CHROMA_SINGLE_SS_SINGLE_COLUMN_NEIGHBORS[2] = {
  TWO_CORNER_NEIGHBORS_PLUS_2, TWO_EDGE_NEIGHBORS_PLUS_2
};

static const int16_t *const CHROMA_DOUBLE_SS_LEFT_COLUMN_NEIGHBORS[2] = {
  LEFT_CORNER_NEIGHBORS_PLUS_4, LEFT_EDGE_NEIGHBORS_PLUS_4
};

static const int16_t *const CHROMA_DOUBLE_SS_MIDDLE_COLUMN_NEIGHBORS[2] = {
  MIDDLE_EDGE_NEIGHBORS_PLUS_4, MIDDLE_CENTER_NEIGHBORS_PLUS_4
};

static const int16_t *const CHROMA_DOUBLE_SS_RIGHT_COLUMN_NEIGHBORS[2] = {
  RIGHT_CORNER_NEIGHBORS_PLUS_4, RIGHT_EDGE_NEIGHBORS_PLUS_4
};

static const int16_t *const CHROMA_DOUBLE_SS_SINGLE_COLUMN_NEIGHBORS[2] = {
  TWO_CORNER_NEIGHBORS_PLUS_4, TWO_EDGE_NEIGHBORS_PLUS_4
};

#define DIST_STRIDE ((BW) + 2)
