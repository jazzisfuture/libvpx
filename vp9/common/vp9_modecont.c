/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vp9/common/vp9_entropy.h"

const int vp9_default_mode_contexts[BLOCK_SIZE_SIZE][INTER_MODE_CONTEXTS][4] = {
  {
    {1,       223,   1,    237},  // 0,0 best: Only candidate
    {87,      166,   26,   219},  // 0,0 best: non zero candidates
    {89,      67,    18,   125},  // 0,0 best: non zero candidates, split
    {16,      141,   69,   226},  // strong nz candidate(s), no split
    {35,      122,   14,   227},  // weak nz candidate(s), no split
    {14,      122,   22,   164},  // strong nz candidate(s), split
    {16,      70,    9,    183},  // weak nz candidate(s), split
  },
  {
    {128,     1,     1,   255},
    {  5,   163,   114,   255},
    { 73,   205,   255,   128},
    { 10,   169,    99,   255},
    {171,    23,     1,   255},
    { 23,   128,     1,   255},
    {128,   128,   128,   128},
  },
  {
    {224,     1,     1,   255},
    { 95,    95,    75,   255},
    {  1,     1,     1,   255},
    {  4,   179,    59,   255},
    { 51,     1,     1,   255},
    {255,   128,   128,   128},
    {128,   128,   128,   128},
  },
};
