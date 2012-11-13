/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "entropy.h"

const int vp9_default_mode_contexts[INTER_MODE_CONTEXTS][4] = {
  {223,     1,     1,    240},  // 0,0 best: Only candidate
  {90,      175,   40,   226},  // 0,0 best: non zero candidates
  {102,     99,    23,   139},  // 0,0 best: non zero candidates, split
  {21,      185,   60,   231},  // strong nz candidate(s), no split
  {45,      91,    3,    227},  // weak nz candidate(s), no split
  {18,      149,   15,   191},  // strong nz candidate(s), split
  {26,      93,    3,    215},  // weak nz candidate(s), split
};
const int vp9_default_mode_contexts_a[INTER_MODE_CONTEXTS][4] = {
  {202,     1,     1,    231},  // 0,0 best: Only candidate
  {110,     156,   22,   219},  // 0,0 best: non zero candidates
  {97,      68,    19,   129},  // 0,0 best: non zero candidates, split
  {14,      172,   55,   221},  // strong nz candidate(s), no split
  {20,      104,   12,   209},  // weak nz candidate(s), no split
  {12,      99,    52,   164},  // strong nz candidate(s), split
  {12,      58,    9,    165},  // weak nz candidate(s), split
};
