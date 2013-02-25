/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <assert.h>
#include <math.h>
#include "./vpx_config.h"
#include "vp9/common/vp9_systemdependent.h"

#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_idct.h"

// TODO: these transforms can be converted into integer forms to reduce
//       the complexity
static const float dct_4[16] = {
  0.500000000000000,  0.500000000000000,  0.500000000000000,  0.500000000000000,
  0.653281482438188,  0.270598050073099, -0.270598050073099, -0.653281482438188,
  0.500000000000000, -0.500000000000000, -0.500000000000000,  0.500000000000000,
  0.270598050073099, -0.653281482438188,  0.653281482438188, -0.270598050073099
};

static const float adst_4[16] = {
  0.228013428883779,  0.428525073124360,  0.577350269189626,  0.656538502008139,
  0.577350269189626,  0.577350269189626,  0.000000000000000, -0.577350269189626,
  0.656538502008139, -0.228013428883779, -0.577350269189626,  0.428525073124359,
  0.428525073124360, -0.656538502008139,  0.577350269189626, -0.228013428883779
};

static const float dct_8[64] = {
  0.353553390593274,   0.353553390593274,   0.353553390593274,   0.353553390593274,
  0.353553390593274,   0.353553390593274,   0.353553390593274,   0.353553390593274,
  0.490392640201615,   0.415734806151273,   0.277785116509801,   0.097545161008064,
 -0.097545161008064,  -0.277785116509801,  -0.415734806151273,  -0.490392640201615,
  0.461939766255643,   0.191341716182545,  -0.191341716182545,  -0.461939766255643,
 -0.461939766255643,  -0.191341716182545,   0.191341716182545,   0.461939766255643,
  0.415734806151273,  -0.097545161008064,  -0.490392640201615,  -0.277785116509801,
  0.277785116509801,   0.490392640201615,   0.097545161008064,  -0.415734806151273,
  0.353553390593274,  -0.353553390593274,  -0.353553390593274,   0.353553390593274,
  0.353553390593274,  -0.353553390593274,  -0.353553390593274,   0.353553390593274,
  0.277785116509801,  -0.490392640201615,   0.097545161008064,   0.415734806151273,
 -0.415734806151273,  -0.097545161008064,   0.490392640201615,  -0.277785116509801,
  0.191341716182545,  -0.461939766255643,   0.461939766255643,  -0.191341716182545,
 -0.191341716182545,   0.461939766255643,  -0.461939766255643,   0.191341716182545,
  0.097545161008064,  -0.277785116509801,   0.415734806151273,  -0.490392640201615,
  0.490392640201615,  -0.415734806151273,   0.277785116509801,  -0.097545161008064
};

static const float adst_8[64] = {
  0.089131608307533,   0.175227946595735,   0.255357107325376,   0.326790388032145,
  0.387095214016349,   0.434217976756762,   0.466553967085785,   0.483002021635509,
  0.255357107325376,   0.434217976756762,   0.483002021635509,   0.387095214016349,
  0.175227946595735,  -0.089131608307533,  -0.326790388032145,  -0.466553967085785,
  0.387095214016349,   0.466553967085785,   0.175227946595735,  -0.255357107325376,
 -0.483002021635509,  -0.326790388032145,   0.089131608307533,   0.434217976756762,
  0.466553967085785,   0.255357107325376,  -0.326790388032145,  -0.434217976756762,
  0.089131608307533,   0.483002021635509,   0.175227946595735,  -0.387095214016348,
  0.483002021635509,  -0.089131608307533,  -0.466553967085785,   0.175227946595735,
  0.434217976756762,  -0.255357107325376,  -0.387095214016348,   0.326790388032145,
  0.434217976756762,  -0.387095214016348,  -0.089131608307533,   0.466553967085786,
 -0.326790388032145,  -0.175227946595735,   0.483002021635509,  -0.255357107325375,
  0.326790388032145,  -0.483002021635509,   0.387095214016349,  -0.089131608307534,
 -0.255357107325377,   0.466553967085785,  -0.434217976756762,   0.175227946595736,
  0.175227946595735,  -0.326790388032145,   0.434217976756762,  -0.483002021635509,
  0.466553967085785,  -0.387095214016348,   0.255357107325376,  -0.089131608307532
};

/* Converted the transforms to integers. */
static const int16_t dct_i4[16] = {
  16384,  16384,  16384,  16384,
  21407,   8867,  -8867, -21407,
  16384, -16384, -16384,  16384,
   8867, -21407,  21407,  -8867
};

static const int16_t adst_i4[16] = {
   7472,  14042,  18919,  21513,
  18919,  18919,      0, -18919,
  21513,  -7472, -18919,  14042,
  14042, -21513,  18919,  -7472
};

static const int16_t dct_i8[64] = {
   11585,  11585,  11585,  11585,
   11585,  11585,  11585,  11585,
   16069,  13623,   9102,   3196,
   -3196,  -9102, -13623, -16069,
   15137,   6270,  -6270, -15137,
  -15137,  -6270,   6270,  15137,
   13623,  -3196, -16069,  -9102,
    9102,  16069,   3196, -13623,
   11585, -11585, -11585,  11585,
   11585, -11585, -11585,  11585,
    9102, -16069,   3196,  13623,
  -13623,  -3196,  16069,  -9102,
    6270, -15137,  15137,  -6270,
   -6270,  15137, -15137,   6270,
    3196,  -9102,  13623, -16069,
   16069, -13623,   9102,  -3196
};

#if CONFIG_INTHT
static const int16_t adst_i8[64] = {
   1606,    4756,     7723,    10394,
  12665,   14449,    15678,    16305,
   4756,   12665,    16305,    14449,
   7723,   -1606,   -10394,   -15678,
   7723,   16305,    10394,    -4756,
 -15678,  -12665,     1606,    14449,
  10394,   14449,    -4756,   -16305,
  -1606,   15678,     7723,   -12665,
  12665,    7723,   -15678,    -1606,
  16305,   -4756,   -14449,    10394,
  14449,   -1606,   -12665,    15678,
  -4756,  -10394,    16305,    -7723,
  15678,  -10394,     1606,     7723,
 -14449,   16305,   -12665,     4756,
  16305,  -15678,    14449,   -12665,
  10394,   -7723,     4756,    -1606
};
#else
static const int16_t adst_i8[64] = {
    2921,   5742,   8368,  10708,
   12684,  14228,  15288,  15827,
    8368,  14228,  15827,  12684,
    5742,  -2921, -10708, -15288,
   12684,  15288,   5742,  -8368,
  -15827, -10708,   2921,  14228,
   15288,   8368, -10708, -14228,
    2921,  15827,   5742, -12684,
   15827,  -2921, -15288,   5742,
   14228,  -8368, -12684,  10708,
   14228, -12684,  -2921,  15288,
  -10708,  -5742,  15827,  -8368,
   10708, -15827,  12684,  -2921,
   -8368,  15288, -14228,   5742,
    5742, -10708,  14228, -15827,
   15288, -12684,   8368,  -2921
};
#endif

static const float dct_16[256] = {
  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,
  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,  0.250000,
  0.351851,  0.338330,  0.311806,  0.273300,  0.224292,  0.166664,  0.102631,  0.034654,
 -0.034654, -0.102631, -0.166664, -0.224292, -0.273300, -0.311806, -0.338330, -0.351851,
  0.346760,  0.293969,  0.196424,  0.068975, -0.068975, -0.196424, -0.293969, -0.346760,
 -0.346760, -0.293969, -0.196424, -0.068975,  0.068975,  0.196424,  0.293969,  0.346760,
  0.338330,  0.224292,  0.034654, -0.166664, -0.311806, -0.351851, -0.273300, -0.102631,
  0.102631,  0.273300,  0.351851,  0.311806,  0.166664, -0.034654, -0.224292, -0.338330,
  0.326641,  0.135299, -0.135299, -0.326641, -0.326641, -0.135299,  0.135299,  0.326641,
  0.326641,  0.135299, -0.135299, -0.326641, -0.326641, -0.135299,  0.135299,  0.326641,
  0.311806,  0.034654, -0.273300, -0.338330, -0.102631,  0.224292,  0.351851,  0.166664,
 -0.166664, -0.351851, -0.224292,  0.102631,  0.338330,  0.273300, -0.034654, -0.311806,
  0.293969, -0.068975, -0.346760, -0.196424,  0.196424,  0.346760,  0.068975, -0.293969,
 -0.293969,  0.068975,  0.346760,  0.196424, -0.196424, -0.346760, -0.068975,  0.293969,
  0.273300, -0.166664, -0.338330,  0.034654,  0.351851,  0.102631, -0.311806, -0.224292,
  0.224292,  0.311806, -0.102631, -0.351851, -0.034654,  0.338330,  0.166664, -0.273300,
  0.250000, -0.250000, -0.250000,  0.250000,  0.250000, -0.250000, -0.250000,  0.250000,
  0.250000, -0.250000, -0.250000,  0.250000,  0.250000, -0.250000, -0.250000,  0.250000,
  0.224292, -0.311806, -0.102631,  0.351851, -0.034654, -0.338330,  0.166664,  0.273300,
 -0.273300, -0.166664,  0.338330,  0.034654, -0.351851,  0.102631,  0.311806, -0.224292,
  0.196424, -0.346760,  0.068975,  0.293969, -0.293969, -0.068975,  0.346760, -0.196424,
 -0.196424,  0.346760, -0.068975, -0.293969,  0.293969,  0.068975, -0.346760,  0.196424,
  0.166664, -0.351851,  0.224292,  0.102631, -0.338330,  0.273300,  0.034654, -0.311806,
  0.311806, -0.034654, -0.273300,  0.338330, -0.102631, -0.224292,  0.351851, -0.166664,
  0.135299, -0.326641,  0.326641, -0.135299, -0.135299,  0.326641, -0.326641,  0.135299,
  0.135299, -0.326641,  0.326641, -0.135299, -0.135299,  0.326641, -0.326641,  0.135299,
  0.102631, -0.273300,  0.351851, -0.311806,  0.166664,  0.034654, -0.224292,  0.338330,
 -0.338330,  0.224292, -0.034654, -0.166664,  0.311806, -0.351851,  0.273300, -0.102631,
  0.068975, -0.196424,  0.293969, -0.346760,  0.346760, -0.293969,  0.196424, -0.068975,
 -0.068975,  0.196424, -0.293969,  0.346760, -0.346760,  0.293969, -0.196424,  0.068975,
  0.034654, -0.102631,  0.166664, -0.224292,  0.273300, -0.311806,  0.338330, -0.351851,
  0.351851, -0.338330,  0.311806, -0.273300,  0.224292, -0.166664,  0.102631, -0.034654
};

static const float adst_16[256] = {
  0.033094,  0.065889,  0.098087,  0.129396,  0.159534,  0.188227,  0.215215,  0.240255,
  0.263118,  0.283599,  0.301511,  0.316693,  0.329007,  0.338341,  0.344612,  0.347761,
  0.098087,  0.188227,  0.263118,  0.316693,  0.344612,  0.344612,  0.316693,  0.263118,
  0.188227,  0.098087,  0.000000, -0.098087, -0.188227, -0.263118, -0.316693, -0.344612,
  0.159534,  0.283599,  0.344612,  0.329007,  0.240255,  0.098087, -0.065889, -0.215215,
 -0.316693, -0.347761, -0.301511, -0.188227, -0.033094,  0.129396,  0.263118,  0.338341,
  0.215215,  0.338341,  0.316693,  0.159534, -0.065889, -0.263118, -0.347761, -0.283599,
 -0.098087,  0.129396,  0.301511,  0.344612,  0.240255,  0.033094, -0.188227, -0.329007,
  0.263118,  0.344612,  0.188227, -0.098087, -0.316693, -0.316693, -0.098087,  0.188227,
  0.344612,  0.263118,  0.000000, -0.263118, -0.344612, -0.188227,  0.098087,  0.316693,
  0.301511,  0.301511,  0.000000, -0.301511, -0.301511, -0.000000,  0.301511,  0.301511,
  0.000000, -0.301511, -0.301511, -0.000000,  0.301511,  0.301511,  0.000000, -0.301511,
  0.329007,  0.215215, -0.188227, -0.338341, -0.033094,  0.316693,  0.240255, -0.159534,
 -0.344612, -0.065889,  0.301511,  0.263118, -0.129396, -0.347761, -0.098087,  0.283599,
  0.344612,  0.098087, -0.316693, -0.188227,  0.263118,  0.263118, -0.188227, -0.316693,
  0.098087,  0.344612,  0.000000, -0.344612, -0.098087,  0.316693,  0.188227, -0.263118,
  0.347761, -0.033094, -0.344612,  0.065889,  0.338341, -0.098087, -0.329007,  0.129396,
  0.316693, -0.159534, -0.301511,  0.188227,  0.283599, -0.215215, -0.263118,  0.240255,
  0.338341, -0.159534, -0.263118,  0.283599,  0.129396, -0.344612,  0.033094,  0.329007,
 -0.188227, -0.240255,  0.301511,  0.098087, -0.347761,  0.065889,  0.316693, -0.215215,
  0.316693, -0.263118, -0.098087,  0.344612, -0.188227, -0.188227,  0.344612, -0.098087,
 -0.263118,  0.316693,  0.000000, -0.316693,  0.263118,  0.098087, -0.344612,  0.188227,
  0.283599, -0.329007,  0.098087,  0.215215, -0.347761,  0.188227,  0.129396, -0.338341,
  0.263118,  0.033094, -0.301511,  0.316693, -0.065889, -0.240255,  0.344612, -0.159534,
  0.240255, -0.347761,  0.263118, -0.033094, -0.215215,  0.344612, -0.283599,  0.065889,
  0.188227, -0.338341,  0.301511, -0.098087, -0.159534,  0.329007, -0.316693,  0.129396,
  0.188227, -0.316693,  0.344612, -0.263118,  0.098087,  0.098087, -0.263118,  0.344612,
 -0.316693,  0.188227,  0.000000, -0.188227,  0.316693, -0.344612,  0.263118, -0.098087,
  0.129396, -0.240255,  0.316693, -0.347761,  0.329007, -0.263118,  0.159534, -0.033094,
 -0.098087,  0.215215, -0.301511,  0.344612, -0.338341,  0.283599, -0.188227,  0.065889,
  0.065889, -0.129396,  0.188227, -0.240255,  0.283599, -0.316693,  0.338341, -0.347761,
  0.344612, -0.329007,  0.301511, -0.263118,  0.215215, -0.159534,  0.098087, -0.033094
};

/* Converted the transforms to integers. */
static const int16_t dct_i16[256] = {
    8192,   8192,   8192,   8192,   8192,   8192,   8192,   8192,
    8192,   8192,   8192,   8192,   8192,   8192,   8192,   8192,
   11529,  11086,  10217,   8955,   7350,   5461,   3363,   1136,
   -1136,  -3363,  -5461,  -7350,  -8955, -10217, -11086, -11529,
   11363,   9633,   6436,   2260,  -2260,  -6436,  -9633, -11363,
  -11363,  -9633,  -6436,  -2260,   2260,   6436,   9633,  11363,
   11086,   7350,   1136,  -5461, -10217, -11529,  -8955,  -3363,
    3363,   8955,  11529,  10217,   5461,  -1136,  -7350, -11086,
   10703,   4433,  -4433, -10703, -10703,  -4433,   4433,  10703,
   10703,   4433,  -4433, -10703, -10703,  -4433,   4433,  10703,
   10217,   1136,  -8955, -11086,  -3363,   7350,  11529,   5461,
   -5461, -11529,  -7350,   3363,  11086,   8955,  -1136, -10217,
    9633,  -2260, -11363,  -6436,   6436,  11363,   2260,  -9633,
   -9633,   2260,  11363,   6436,  -6436, -11363,  -2260,   9633,
    8955,  -5461, -11086,   1136,  11529,   3363, -10217,  -7350,
    7350,  10217,  -3363, -11529,  -1136,  11086,   5461,  -8955,
    8192,  -8192,  -8192,   8192,   8192,  -8192,  -8192,   8192,
    8192,  -8192,  -8192,   8192,   8192,  -8192,  -8192,   8192,
    7350, -10217,  -3363,  11529,  -1136, -11086,   5461,   8955,
   -8955,  -5461,  11086,   1136, -11529,   3363,  10217,  -7350,
    6436, -11363,   2260,   9633,  -9633,  -2260,  11363,  -6436,
   -6436,  11363,  -2260,  -9633,   9633,   2260, -11363,   6436,
    5461, -11529,   7350,   3363, -11086,   8955,   1136, -10217,
   10217,  -1136,  -8955,  11086,  -3363,  -7350,  11529,  -5461,
    4433, -10703,  10703,  -4433,  -4433,  10703, -10703,   4433,
    4433, -10703,  10703,  -4433,  -4433,  10703, -10703,   4433,
    3363,  -8955,  11529, -10217,   5461,   1136,  -7350,  11086,
  -11086,   7350,  -1136,  -5461,  10217, -11529,   8955,  -3363,
    2260,  -6436,   9633, -11363,  11363,  -9633,   6436,  -2260,
   -2260,   6436,  -9633,  11363, -11363,   9633,  -6436,   2260,
    1136,  -3363,   5461,  -7350,   8955, -10217,  11086, -11529,
   11529, -11086,  10217,  -8955,   7350,  -5461,   3363,  -1136
};

#if CONFIG_INTHT
static const int16_t adst_i16[256] = {
     568,    1700,    2815,    3903,    4953,    5956,    6901,    7780,
    8584,    9305,    9937,   10473,   10908,   11238,   11459,   11571,
    1700,    4953,    7780,    9937,   11238,   11571,   10908,    9305,
    6901,    3903,     568,   -2815,   -5956,   -8584,  -10473,  -11459,
    2815,    7780,   10908,   11459,    9305,    4953,    -568,   -5956,
   -9937,  -11571,  -10473,   -6901,   -1700,    3903,    8584,   11238,
    3903,    9937,   11459,    7780,     568,   -6901,  -11238,  -10473,
   -4953,    2815,    9305,   11571,    8584,    1700,   -5956,  -10908,
    4953,   11238,    9305,     568,   -8584,  -11459,   -5956,    3903,
   10908,    9937,    1700,   -7780,  -11571,   -6901,    2815,   10473,
    5956,   11571,    4953,   -6901,  -11459,   -3903,    7780,   11238,
    2815,   -8584,  -10908,   -1700,    9305,   10473,     568,   -9937,
    6901,   10908,    -568,  -11238,   -5956,    7780,   10473,   -1700,
  -11459,   -4953,    8584,    9937,   -2815,  -11571,   -3903,    9305,
    7780,    9305,   -5956,  -10473,    3903,   11238,   -1700,  -11571,
    -568,   11459,    2815,  -10908,   -4953,    9937,    6901,   -8584,
    8584,    6901,   -9937,   -4953,   10908,    2815,  -11459,    -568,
   11571,   -1700,  -11238,    3903,   10473,   -5956,   -9305,    7780,
    9305,    3903,  -11571,    2815,    9937,   -8584,   -4953,   11459,
   -1700,  -10473,    7780,    5956,  -11238,     568,   10908,   -6901,
    9937,     568,  -10473,    9305,    1700,  -10908,    8584,    2815,
  -11238,    7780,    3903,  -11459,    6901,    4953,  -11571,    5956,
   10473,   -2815,   -6901,   11571,   -7780,   -1700,    9937,  -10908,
    3903,    5956,  -11459,    8584,     568,   -9305,   11238,   -4953,
   10908,   -5956,   -1700,    8584,  -11571,    9305,   -2815,   -4953,
   10473,  -11238,    6901,     568,   -7780,   11459,   -9937,    3903,
   11238,   -8584,    3903,    1700,   -6901,   10473,  -11571,    9937,
   -5956,     568,    4953,   -9305,   11459,  -10908,    7780,   -2815,
   11459,  -10473,    8584,   -5956,    2815,     568,   -3903,    6901,
   -9305,   10908,  -11571,   11238,   -9937,    7780,   -4953,    1700,
   11571,  -11459,   11238,  -10908,   10473,   -9937,    9305,   -8584,
    7780,   -6901,    5956,   -4953,    3903,   -2815,    1700,    -568
};
#else
static const int16_t adst_i16[256] = {
    1084,   2159,   3214,   4240,   5228,   6168,   7052,   7873,
    8622,   9293,   9880,  10377,  10781,  11087,  11292,  11395,
    3214,   6168,   8622,  10377,  11292,  11292,  10377,   8622,
    6168,   3214,      0,  -3214,  -6168,  -8622, -10377, -11292,
    5228,   9293,  11292,  10781,   7873,   3214,  -2159,  -7052,
  -10377, -11395,  -9880,  -6168,  -1084,   4240,   8622,  11087,
    7052,  11087,  10377,   5228,  -2159,  -8622, -11395,  -9293,
   -3214,   4240,   9880,  11292,   7873,   1084,  -6168, -10781,
    8622,  11292,   6168,  -3214, -10377, -10377,  -3214,   6168,
   11292,   8622,      0,  -8622, -11292,  -6168,   3214,  10377,
    9880,   9880,      0,  -9880,  -9880,      0,   9880,   9880,
       0,  -9880,  -9880,      0,   9880,   9880,      0,  -9880,
   10781,   7052,  -6168, -11087,  -1084,  10377,   7873,  -5228,
  -11292,  -2159,   9880,   8622,  -4240, -11395,  -3214,   9293,
   11292,   3214, -10377,  -6168,   8622,   8622,  -6168, -10377,
    3214,  11292,      0, -11292,  -3214,  10377,   6168,  -8622,
   11395,  -1084, -11292,   2159,  11087,  -3214, -10781,   4240,
   10377,  -5228,  -9880,   6168,   9293,  -7052,  -8622,   7873,
   11087,  -5228,  -8622,   9293,   4240, -11292,   1084,  10781,
   -6168,  -7873,   9880,   3214, -11395,   2159,  10377,  -7052,
   10377,  -8622,  -3214,  11292,  -6168,  -6168,  11292,  -3214,
   -8622,  10377,      0, -10377,   8622,   3214, -11292,   6168,
    9293, -10781,   3214,   7052, -11395,   6168,   4240, -11087,
    8622,   1084,  -9880,  10377,  -2159,  -7873,  11292,  -5228,
    7873, -11395,   8622,  -1084,  -7052,  11292,  -9293,   2159,
    6168, -11087,   9880,  -3214,  -5228,  10781, -10377,   4240,
    6168, -10377,  11292,  -8622,   3214,   3214,  -8622,  11292,
  -10377,   6168,      0,  -6168,  10377, -11292,   8622,  -3214,
    4240,  -7873,  10377, -11395,  10781,  -8622,   5228,  -1084,
   -3214,   7052,  -9880,  11292, -11087,   9293,  -6168,   2159,
    2159,  -4240,   6168,  -7873,   9293, -10377,  11087, -11395,
   11292, -10781,   9880,  -8622,   7052,  -5228,   3214,  -1084
};
#endif

/* For test */
#define TEST_INT 1
#if TEST_INT
#define vp9_fht_int_c vp9_fht_c
#else
#define vp9_fht_float_c vp9_fht_c
#endif

void vp9_fht_float_c(const int16_t *input, int pitch, int16_t *output,
               TX_TYPE tx_type, int tx_dim) {
  vp9_clear_system_state();  // Make it simd safe : __asm emms;
  {
    int i, j, k;
    float bufa[256], bufb[256];  // buffers are for floating-point test purpose
                                 // the implementation could be simplified in
                                 // conjunction with integer transform
    const int16_t *ip = input;
    int16_t *op = output;

    float *pfa = &bufa[0];
    float *pfb = &bufb[0];

    // pointers to vertical and horizontal transforms
    const float *ptv, *pth;

    assert(tx_type != DCT_DCT);
    // load and convert residual array into floating-point
    for (j = 0; j < tx_dim; j++) {
      for (i = 0; i < tx_dim; i++) {
        pfa[i] = (float)ip[i];
      }
      pfa += tx_dim;
      ip  += pitch / 2;
    }

    // vertical transformation
    pfa = &bufa[0];
    pfb = &bufb[0];

    switch (tx_type) {
      case ADST_ADST :
      case ADST_DCT  :
        ptv = (tx_dim == 4) ? &adst_4[0] :
                              ((tx_dim == 8) ? &adst_8[0] : &adst_16[0]);
        break;

      default :
        ptv = (tx_dim == 4) ? &dct_4[0] :
                              ((tx_dim == 8) ? &dct_8[0] : &dct_16[0]);
        break;
    }

    for (j = 0; j < tx_dim; j++) {
      for (i = 0; i < tx_dim; i++) {
        pfb[i] = 0;
        for (k = 0; k < tx_dim; k++) {
          pfb[i] += ptv[k] * pfa[(k * tx_dim)];
        }
        pfa += 1;
      }
      pfb += tx_dim;
      ptv += tx_dim;
      pfa = &bufa[0];
    }

    // horizontal transformation
    pfa = &bufa[0];
    pfb = &bufb[0];

    switch (tx_type) {
      case ADST_ADST :
      case  DCT_ADST :
        pth = (tx_dim == 4) ? &adst_4[0] :
                              ((tx_dim == 8) ? &adst_8[0] : &adst_16[0]);
        break;

      default :
        pth = (tx_dim == 4) ? &dct_4[0] :
                              ((tx_dim == 8) ? &dct_8[0] : &dct_16[0]);
        break;
    }

    for (j = 0; j < tx_dim; j++) {
      for (i = 0; i < tx_dim; i++) {
        pfa[i] = 0;
        for (k = 0; k < tx_dim; k++) {
          pfa[i] += pfb[k] * pth[k];
        }
        pth += tx_dim;
      }

      pfa += tx_dim;
      pfb += tx_dim;
      // pth -= tx_dim * tx_dim;

      switch (tx_type) {
        case ADST_ADST :
        case  DCT_ADST :
          pth = (tx_dim == 4) ? &adst_4[0] :
                                ((tx_dim == 8) ? &adst_8[0] : &adst_16[0]);
          break;

        default :
          pth = (tx_dim == 4) ? &dct_4[0] :
                                ((tx_dim == 8) ? &dct_8[0] : &dct_16[0]);
          break;
      }
    }

    // convert to short integer format and load BLOCKD buffer
    op = output;
    pfa = &bufa[0];

    for (j = 0; j < tx_dim; j++) {
      for (i = 0; i < tx_dim; i++) {
        op[i] = (pfa[i] > 0 ) ? (int16_t)( 8 * pfa[i] + 0.49) :
                                     -(int16_t)(- 8 * pfa[i] + 0.49);
      }
      op  += tx_dim;
      pfa += tx_dim;
    }
  }
  vp9_clear_system_state();  // Make it simd safe : __asm emms;
}

/* Converted the transforms to integer form. */
#define VERTICAL_SHIFT 11
#define VERTICAL_ROUNDING ((1 << (VERTICAL_SHIFT - 1)) - 1)
#define HORIZONTAL_SHIFT 16
#define HORIZONTAL_ROUNDING ((1 << (HORIZONTAL_SHIFT - 1)) - 1)
void vp9_fht_int_c(const int16_t *input, int pitch, int16_t *output,
                   TX_TYPE tx_type, int tx_dim) {
  int i, j, k;
  int16_t imbuf[256];

  const int16_t *ip = input;
  int16_t *op = output;
  int16_t *im = &imbuf[0];

  /* pointers to vertical and horizontal transforms. */
  const int16_t *ptv = NULL, *pth = NULL;

  switch (tx_type) {
    case ADST_ADST :
      ptv = pth = (tx_dim == 4) ? &adst_i4[0]
                                  : ((tx_dim == 8) ? &adst_i8[0]
                                                     : &adst_i16[0]);
      break;
    case ADST_DCT  :
      ptv = (tx_dim == 4) ? &adst_i4[0]
                            : ((tx_dim == 8) ? &adst_i8[0] : &adst_i16[0]);
      pth = (tx_dim == 4) ? &dct_i4[0]
                            : ((tx_dim == 8) ? &dct_i8[0] : &dct_i16[0]);
      break;
    case  DCT_ADST :
      ptv = (tx_dim == 4) ? &dct_i4[0]
                            : ((tx_dim == 8) ? &dct_i8[0] : &dct_i16[0]);
      pth = (tx_dim == 4) ? &adst_i4[0]
                            : ((tx_dim == 8) ? &adst_i8[0] : &adst_i16[0]);
      break;
    case  DCT_DCT :
      ptv = pth = (tx_dim == 4) ? &dct_i4[0]
                                  : ((tx_dim == 8) ? &dct_i8[0] : &dct_i16[0]);
      break;
    default:
      assert(0);
      break;
  }

  /* vertical transformation */
  for (j = 0; j < tx_dim; j++) {
    for (i = 0; i < tx_dim; i++) {
      int temp = 0;

      for (k = 0; k < tx_dim; k++) {
        temp += ptv[k] * ip[(k * (pitch >> 1))];
      }

      im[i] = (int16_t)((temp + VERTICAL_ROUNDING) >> VERTICAL_SHIFT);
      ip++;
    }
    im += tx_dim;  // 16
    ptv += tx_dim;
    ip = input;
  }

  /* horizontal transformation */
  im = &imbuf[0];

  for (j = 0; j < tx_dim; j++) {
    const int16_t *pthc = pth;

    for (i = 0; i < tx_dim; i++) {
      int temp = 0;

      for (k = 0; k < tx_dim; k++) {
        temp += im[k] * pthc[k];
      }

      op[i] = (int16_t)((temp + HORIZONTAL_ROUNDING) >> HORIZONTAL_SHIFT);
      pthc += tx_dim;
    }

    im += tx_dim;  // 16
    op += tx_dim;
  }
}

static void fdct4_1d(int16_t *input, int16_t *output) {
  int16_t step[4];
  int temp1, temp2;

  step[0] = input[0] + input[3];
  step[1] = input[1] + input[2];
  step[2] = input[1] - input[2];
  step[3] = input[0] - input[3];

  temp1 = (step[0] + step[1]) * cospi_16_64;
  temp2 = (step[0] - step[1]) * cospi_16_64;
  output[0] = dct_const_round_shift(temp1);
  output[2] = dct_const_round_shift(temp2);
  temp1 = step[2] * cospi_24_64 + step[3] * cospi_8_64;
  temp2 = -step[2] * cospi_8_64 + step[3] * cospi_24_64;
  output[1] = dct_const_round_shift(temp1);
  output[3] = dct_const_round_shift(temp2);
}

void vp9_short_fdct4x4_c(short *input, short *output, int pitch) {
  int16_t out[4 * 4];
  int16_t *outptr = &out[0];
  const int short_pitch = pitch >> 1;
  int i, j;
  int16_t temp_in[4], temp_out[4];
  // First transform cols
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j)
      temp_in[j] = input[j * short_pitch + i] << 4;
    if (i == 0 && temp_in[0])
      temp_in[0] += 1;
    fdct4_1d(temp_in, temp_out);
    for (j = 0; j < 4; ++j)
      outptr[j * 4 + i] = temp_out[j];
  }
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j)
      temp_in[j] = out[j + i * 4];
    fdct4_1d(temp_in, temp_out);
    for (j = 0; j < 4; ++j)
        output[j + i * 4] = (temp_out[j] + 1) >> 2;
  }
}

#if CONFIG_INTHT4X4
static void fadst4_1d(int16_t *input, int16_t *output) {
  int x0, x1, x2, x3;
  int s0, s1, s2, s3, s4, s5, s6, s7;

  x0 = input[0];
  x1 = input[1];
  x2 = input[2];
  x3 = input[3];

  if (!(x0 | x1 | x2 | x3)) {
    output[0] = output[1] = output[2] = output[3] = 0;
    return;
  }

  s0 = sinpi_1_9 * x0;
  s1 = sinpi_4_9 * x0;
  s2 = sinpi_2_9 * x1;
  s3 = sinpi_1_9 * x1;
  s4 = sinpi_3_9 * x2;
  s5 = sinpi_4_9 * x3;
  s6 = sinpi_2_9 * x3;
  s7 = x0 + x1 - x3;

  x0 = s0 + s2 + s5;
  x1 = sinpi_3_9 * s7;
  x2 = s1 - s3 + s6;
  x3 = s4;

  s0 = x0 + x3;
  s1 = x1;
  s2 = x2 - x3;
  s3 = x2 - x0 + x3;

  // 1-D transform scaling factor is sqrt(2).
  output[0] = dct_const_round_shift(s0);
  output[1] = dct_const_round_shift(s1);
  output[2] = dct_const_round_shift(s2);
  output[3] = dct_const_round_shift(s3);
}

void vp9_short_fht4x4_c(int16_t *input, int16_t *output,
                        int pitch, TX_TYPE tx_type) {
  int16_t out[4 * 4];
  int16_t *outptr = &out[0];
  const int short_pitch = pitch >> 1;
  int i, j;
  int16_t temp_in[4], temp_out[4];

  void (*fwdr)(int16_t*, int16_t*);
  void (*fwdc)(int16_t*, int16_t*);

  switch (tx_type) {
    case ADST_ADST:
      fwdc = &fadst4_1d;
      fwdr = &fadst4_1d;
      break;
    case ADST_DCT:
      fwdc = &fadst4_1d;
      fwdr = &fdct4_1d;
      break;
    case DCT_ADST:
      fwdc = &fdct4_1d;
      fwdr = &fadst4_1d;
      break;
    case DCT_DCT:
      fwdc = &fdct4_1d;
      fwdr = &fdct4_1d;
      break;
    default:
      assert(0);
  }


  // column transform
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j)
      temp_in[j] = input[j * short_pitch + i] << 4;
    if (i == 0 && temp_in[0])
      temp_in[0] += 1;
    fwdc(temp_in, temp_out);
    for (j = 0; j < 4; ++j)
      outptr[j * 4 + i] = temp_out[j];
  }

  // row transform
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j)
      temp_in[j] = out[j + i * 4];
    fwdr(temp_in, temp_out);
    for (j = 0; j < 4; ++j)
      output[j + i * 4] = (temp_out[j] + 1) >> 2;
  }
}
#endif

void vp9_short_fdct8x4_c(short *input, short *output, int pitch)
{
    vp9_short_fdct4x4_c(input,   output,    pitch);
    vp9_short_fdct4x4_c(input + 4, output + 16, pitch);
}

static void fdct8_1d(int16_t *input, int16_t *output) {
  int16_t step[8];
  int temp1, temp2;

  // stage 1
  step[0] = input[0] + input[7];
  step[1] = input[1] + input[6];
  step[2] = input[2] + input[5];
  step[3] = input[3] + input[4];
  step[4] = input[3] - input[4];
  step[5] = input[2] - input[5];
  step[6] = input[1] - input[6];
  step[7] = input[0] - input[7];

  fdct4_1d(step, step);

  // Stage 2
  output[4] = step[4];
  temp1 = (-step[5] + step[6]) * cospi_16_64;
  temp2 = (step[6] + step[5]) * cospi_16_64;
  output[5] = dct_const_round_shift(temp1);
  output[6] = dct_const_round_shift(temp2);
  output[7] = step[7];

  // Stage 3
  step[4] = output[4] + output[5];
  step[5] = -output[5] + output[4];
  step[6] = -output[6] + output[7];
  step[7] = output[7] + output[6];

  // Stage 4
  output[0] = step[0];
  output[4] = step[2];
  output[2] = step[1];
  output[6] = step[3];

  temp1 = step[4] * cospi_28_64 + step[7] * cospi_4_64;
  temp2 = step[5] * cospi_12_64 + step[6] * cospi_20_64;
  output[1] = dct_const_round_shift(temp1);
  output[5] = dct_const_round_shift(temp2);
  temp1 = step[6] * cospi_12_64 + step[5] * -cospi_20_64;
  temp2 = step[7] * cospi_28_64 + step[4] * -cospi_4_64;
  output[3] = dct_const_round_shift(temp1);
  output[7] = dct_const_round_shift(temp2);
}

void vp9_short_fdct8x8_c(int16_t *input, int16_t *output, int pitch) {
  int shortpitch = pitch >> 1;
  int i, j;
  int16_t out[64];
  int16_t temp_in[8], temp_out[8];

  // First transform columns
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++)
      temp_in[j] = input[j * shortpitch + i] << 2;
    fdct8_1d(temp_in, temp_out);
    for (j = 0; j < 8; j++)
      out[j * 8 + i] = temp_out[j];
  }

  // Then transform rows
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j)
      temp_in[j] = out[j + i * 8];
    fdct8_1d(temp_in, temp_out);
    for (j = 0; j < 8; ++j)
      output[j + i * 8] = temp_out[j] / 2;
  }
}

#if CONFIG_INTHT
static void fadst8_1d(int16_t *input, int16_t *output) {
  int x0, x1, x2, x3, x4, x5, x6, x7;
  int s0, s1, s2, s3, s4, s5, s6, s7;

  x0 = input[7];
  x1 = input[0];
  x2 = input[5];
  x3 = input[2];
  x4 = input[3];
  x5 = input[4];
  x6 = input[1];
  x7 = input[6];

  // stage 1
  s0 = cospi_2_64  * x0 + cospi_30_64 * x1;
  s1 = cospi_30_64 * x0 - cospi_2_64  * x1;
  s2 = cospi_10_64 * x2 + cospi_22_64 * x3;
  s3 = cospi_22_64 * x2 - cospi_10_64 * x3;
  s4 = cospi_18_64 * x4 + cospi_14_64 * x5;
  s5 = cospi_14_64 * x4 - cospi_18_64 * x5;
  s6 = cospi_26_64 * x6 + cospi_6_64  * x7;
  s7 = cospi_6_64  * x6 - cospi_26_64 * x7;

  x0 = dct_const_round_shift(s0 + s4);
  x1 = dct_const_round_shift(s1 + s5);
  x2 = dct_const_round_shift(s2 + s6);
  x3 = dct_const_round_shift(s3 + s7);
  x4 = dct_const_round_shift(s0 - s4);
  x5 = dct_const_round_shift(s1 - s5);
  x6 = dct_const_round_shift(s2 - s6);
  x7 = dct_const_round_shift(s3 - s7);

  // stage 2
  s0 = x0;
  s1 = x1;
  s2 = x2;
  s3 = x3;
  s4 = cospi_8_64  * x4 + cospi_24_64 * x5;
  s5 = cospi_24_64 * x4 - cospi_8_64  * x5;
  s6 = - cospi_24_64 * x6 + cospi_8_64  * x7;
  s7 =   cospi_8_64  * x6 + cospi_24_64 * x7;

  x0 = s0 + s2;
  x1 = s1 + s3;
  x2 = s0 - s2;
  x3 = s1 - s3;
  x4 = dct_const_round_shift(s4 + s6);
  x5 = dct_const_round_shift(s5 + s7);
  x6 = dct_const_round_shift(s4 - s6);
  x7 = dct_const_round_shift(s5 - s7);

  // stage 3
  s2 = cospi_16_64 * (x2 + x3);
  s3 = cospi_16_64 * (x2 - x3);
  s6 = cospi_16_64 * (x6 + x7);
  s7 = cospi_16_64 * (x6 - x7);

  x2 = dct_const_round_shift(s2);
  x3 = dct_const_round_shift(s3);
  x6 = dct_const_round_shift(s6);
  x7 = dct_const_round_shift(s7);

  output[0] =   x0;
  output[1] = - x4;
  output[2] =   x6;
  output[3] = - x2;
  output[4] =   x3;
  output[5] = - x7;
  output[6] =   x5;
  output[7] = - x1;
}

void vp9_short_fht8x8_c(int16_t *input, int16_t *output,
                        int pitch, TX_TYPE tx_type) {
  int16_t out[64];
  int16_t *outptr = &out[0];
  const int short_pitch = pitch >> 1;
  int i, j;
  int16_t temp_in[8], temp_out[8];

  void (*fwdr)(int16_t*, int16_t*);
  void (*fwdc)(int16_t*, int16_t*);

  switch (tx_type) {
    case ADST_ADST:
      fwdc = &fadst8_1d;
      fwdr = &fadst8_1d;
      break;
    case ADST_DCT:
      fwdc = &fadst8_1d;
      fwdr = &fdct8_1d;
      break;
    case DCT_ADST:
      fwdc = &fdct8_1d;
      fwdr = &fadst8_1d;
      break;
    case DCT_DCT:
      fwdc = &fdct8_1d;
      fwdr = &fdct8_1d;
      break;
    default:
      assert(0);
  }

  // column transform
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j)
      temp_in[j] = input[j * short_pitch + i] << 2;
    fwdc(temp_in, temp_out);
    for (j = 0; j < 8; ++j)
      outptr[j * 8 + i] = temp_out[j];
  }

  // row transform
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j)
      temp_in[j] = out[j + i * 8];
    fwdr(temp_in, temp_out);
    for (j = 0; j < 8; ++j)
      output[j + i * 8] = temp_out[j] >> 1;
  }
}
#endif

void vp9_short_walsh4x4_x8_c(short *input, short *output, int pitch) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;
  int pitch_short = pitch >> 1;

  for (i = 0; i < 4; i++) {
    a1 = ip[0 * pitch_short] + ip[3 * pitch_short];
    b1 = ip[1 * pitch_short] + ip[2 * pitch_short];
    c1 = ip[1 * pitch_short] - ip[2 * pitch_short];
    d1 = ip[0 * pitch_short] - ip[3 * pitch_short];

    op[0] = (a1 + b1 + 1) >> 1;
    op[4] = (c1 + d1) >> 1;
    op[8] = (a1 - b1) >> 1;
    op[12] = (d1 - c1) >> 1;

    ip++;
    op++;
  }
  ip = output;
  op = output;

  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[3];
    b1 = ip[1] + ip[2];
    c1 = ip[1] - ip[2];
    d1 = ip[0] - ip[3];

    op[0] = ((a1 + b1 + 1) >> 1) << WHT_UPSCALE_FACTOR;
    op[1] = ((c1 + d1) >> 1) << WHT_UPSCALE_FACTOR;
    op[2] = ((a1 - b1) >> 1) << WHT_UPSCALE_FACTOR;
    op[3] = ((d1 - c1) >> 1) << WHT_UPSCALE_FACTOR;

    ip += 4;
    op += 4;
  }
}

void vp9_short_walsh8x4_x8_c(short *input, short *output, int pitch) {
  vp9_short_walsh4x4_x8_c(input,   output,    pitch);
  vp9_short_walsh4x4_x8_c(input + 4, output + 16, pitch);
}


// Rewrote to use same algorithm as others.
static void fdct16_1d(int16_t input[16], int16_t output[16]) {
  int16_t step[16];
  int temp1, temp2;

  // step 1
  step[ 0] = input[0] + input[15];
  step[ 1] = input[1] + input[14];
  step[ 2] = input[2] + input[13];
  step[ 3] = input[3] + input[12];
  step[ 4] = input[4] + input[11];
  step[ 5] = input[5] + input[10];
  step[ 6] = input[6] + input[ 9];
  step[ 7] = input[7] + input[ 8];
  step[ 8] = input[7] - input[ 8];
  step[ 9] = input[6] - input[ 9];
  step[10] = input[5] - input[10];
  step[11] = input[4] - input[11];
  step[12] = input[3] - input[12];
  step[13] = input[2] - input[13];
  step[14] = input[1] - input[14];
  step[15] = input[0] - input[15];

  fdct8_1d(step, step);

  // step 2
  output[8] = step[8];
  output[9] = step[9];
  temp1 = (-step[10] + step[13]) * cospi_16_64;
  temp2 = (-step[11] + step[12]) * cospi_16_64;
  output[10] = dct_const_round_shift(temp1);
  output[11] = dct_const_round_shift(temp2);
  temp1 = (step[11] + step[12]) * cospi_16_64;
  temp2 = (step[10] + step[13]) * cospi_16_64;
  output[12] = dct_const_round_shift(temp1);
  output[13] = dct_const_round_shift(temp2);
  output[14] = step[14];
  output[15] = step[15];

  // step 3
  step[ 8] = output[8] + output[11];
  step[ 9] = output[9] + output[10];
  step[ 10] = output[9] - output[10];
  step[ 11] = output[8] - output[11];
  step[ 12] = -output[12] + output[15];
  step[ 13] = -output[13] + output[14];
  step[ 14] = output[13] + output[14];
  step[ 15] = output[12] + output[15];

  // step 4
  output[8] = step[8];
  temp1 = -step[9] * cospi_8_64 + step[14] * cospi_24_64;
  temp2 = -step[10] * cospi_24_64 - step[13] * cospi_8_64;
  output[9] = dct_const_round_shift(temp1);
  output[10] = dct_const_round_shift(temp2);
  output[11] = step[11];
  output[12] = step[12];
  temp1 = -step[10] * cospi_8_64 + step[13] * cospi_24_64;
  temp2 = step[9] * cospi_24_64 + step[14] * cospi_8_64;
  output[13] = dct_const_round_shift(temp1);
  output[14] = dct_const_round_shift(temp2);
  output[15] = step[15];

  // step 5
  step[8] = output[8] + output[9];
  step[9] = output[8] - output[9];
  step[10] = -output[10] + output[11];
  step[11] = output[10] + output[11];
  step[12] = output[12] + output[13];
  step[13] = output[12] - output[13];
  step[14] = -output[14] + output[15];
  step[15] = output[14] + output[15];

  // step 6
  output[0] = step[0];
  output[8] = step[4];
  output[4] = step[2];
  output[12] = step[6];
  output[2] = step[1];
  output[10] = step[5];
  output[6] = step[3];
  output[14] = step[7];

  temp1 = step[8] * cospi_30_64 + step[15] * cospi_2_64;
  temp2 = step[9] * cospi_14_64 + step[14] * cospi_18_64;
  output[1] = dct_const_round_shift(temp1);
  output[9] = dct_const_round_shift(temp2);

  temp1 = step[10] * cospi_22_64 + step[13] * cospi_10_64;
  temp2 = step[11] * cospi_6_64 + step[12] * cospi_26_64;
  output[5] = dct_const_round_shift(temp1);
  output[13] = dct_const_round_shift(temp2);

  temp1 = -step[11] * cospi_26_64 + step[12] * cospi_6_64;
  temp2 = -step[10] * cospi_10_64 + step[13] * cospi_22_64;
  output[3] = dct_const_round_shift(temp1);
  output[11] = dct_const_round_shift(temp2);

  temp1 = -step[9] * cospi_18_64 + step[14] * cospi_14_64;
  temp2 = -step[8] * cospi_2_64 + step[15] * cospi_30_64;
  output[7] = dct_const_round_shift(temp1);
  output[15] = dct_const_round_shift(temp2);
}

void vp9_short_fdct16x16_c(int16_t *input, int16_t *out, int pitch) {
  int shortpitch = pitch >> 1;
  int i, j;
  int16_t output[256];
  int16_t temp_in[16], temp_out[16];

  // First transform columns
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++)
      temp_in[j] = input[j * shortpitch + i] << 2;
    fdct16_1d(temp_in, temp_out);
    for (j = 0; j < 16; j++)
      output[j * 16 + i] = (temp_out[j] + 1) >> 2;
  }

  // Then transform rows
  for (i = 0; i < 16; ++i) {
    for (j = 0; j < 16; ++j)
      temp_in[j] = output[j + i * 16];
    fdct16_1d(temp_in, temp_out);
    for (j = 0; j < 16; ++j)
      out[j + i * 16] = temp_out[j];
  }
}

#if CONFIG_INTHT16X16
void fadst16_1d(int16_t *input, int16_t *output) {
  int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
  int s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15;

  x0 = input[15];
  x1 = input[0];
  x2 = input[13];
  x3 = input[2];
  x4 = input[11];
  x5 = input[4];
  x6 = input[9];
  x7 = input[6];
  x8 = input[7];
  x9 = input[8];
  x10 = input[5];
  x11 = input[10];
  x12 = input[3];
  x13 = input[12];
  x14 = input[1];
  x15 = input[14];

  // stage 1
  s0 = x0 * cospi_1_64  + x1 * cospi_31_64;
  s1 = x0 * cospi_31_64 - x1 * cospi_1_64;
  s2 = x2 * cospi_5_64  + x3 * cospi_27_64;
  s3 = x2 * cospi_27_64 - x3 * cospi_5_64;
  s4 = x4 * cospi_9_64  + x5 * cospi_23_64;
  s5 = x4 * cospi_23_64 - x5 * cospi_9_64;
  s6 = x6 * cospi_13_64 + x7 * cospi_19_64;
  s7 = x6 * cospi_19_64 - x7 * cospi_13_64;
  s8 = x8 * cospi_17_64 + x9 * cospi_15_64;
  s9 = x8 * cospi_15_64 - x9 * cospi_17_64;
  s10 = x10 * cospi_21_64 + x11 * cospi_11_64;
  s11 = x10 * cospi_11_64 - x11 * cospi_21_64;
  s12 = x12 * cospi_25_64 + x13 * cospi_7_64;
  s13 = x12 * cospi_7_64  - x13 * cospi_25_64;
  s14 = x14 * cospi_29_64 + x15 * cospi_3_64;
  s15 = x14 * cospi_3_64  - x15 * cospi_29_64;

  x0 = dct_const_round_shift(s0 + s8);
  x1 = dct_const_round_shift(s1 + s9);
  x2 = dct_const_round_shift(s2 + s10);
  x3 = dct_const_round_shift(s3 + s11);
  x4 = dct_const_round_shift(s4 + s12);
  x5 = dct_const_round_shift(s5 + s13);
  x6 = dct_const_round_shift(s6 + s14);
  x7 = dct_const_round_shift(s7 + s15);
  x8  = dct_const_round_shift(s0 - s8);
  x9  = dct_const_round_shift(s1 - s9);
  x10 = dct_const_round_shift(s2 - s10);
  x11 = dct_const_round_shift(s3 - s11);
  x12 = dct_const_round_shift(s4 - s12);
  x13 = dct_const_round_shift(s5 - s13);
  x14 = dct_const_round_shift(s6 - s14);
  x15 = dct_const_round_shift(s7 - s15);

  // stage 2
  s0 = x0;
  s1 = x1;
  s2 = x2;
  s3 = x3;
  s4 = x4;
  s5 = x5;
  s6 = x6;
  s7 = x7;
  s8 =    x8 * cospi_4_64   + x9 * cospi_28_64;
  s9 =    x8 * cospi_28_64  - x9 * cospi_4_64;
  s10 =   x10 * cospi_20_64 + x11 * cospi_12_64;
  s11 =   x10 * cospi_12_64 - x11 * cospi_20_64;
  s12 = - x12 * cospi_28_64 + x13 * cospi_4_64;
  s13 =   x12 * cospi_4_64  + x13 * cospi_28_64;
  s14 = - x14 * cospi_12_64 + x15 * cospi_20_64;
  s15 =   x14 * cospi_20_64 + x15 * cospi_12_64;

  x0 = s0 + s4;
  x1 = s1 + s5;
  x2 = s2 + s6;
  x3 = s3 + s7;
  x4 = s0 - s4;
  x5 = s1 - s5;
  x6 = s2 - s6;
  x7 = s3 - s7;
  x8 = dct_const_round_shift(s8 + s12);
  x9 = dct_const_round_shift(s9 + s13);
  x10 = dct_const_round_shift(s10 + s14);
  x11 = dct_const_round_shift(s11 + s15);
  x12 = dct_const_round_shift(s8 - s12);
  x13 = dct_const_round_shift(s9 - s13);
  x14 = dct_const_round_shift(s10 - s14);
  x15 = dct_const_round_shift(s11 - s15);

  // stage 3
  s0 = x0;
  s1 = x1;
  s2 = x2;
  s3 = x3;
  s4 = x4 * cospi_8_64  + x5 * cospi_24_64;
  s5 = x4 * cospi_24_64 - x5 * cospi_8_64;
  s6 = - x6 * cospi_24_64 + x7 * cospi_8_64;
  s7 =   x6 * cospi_8_64  + x7 * cospi_24_64;
  s8 = x8;
  s9 = x9;
  s10 = x10;
  s11 = x11;
  s12 = x12 * cospi_8_64  + x13 * cospi_24_64;
  s13 = x12 * cospi_24_64 - x13 * cospi_8_64;
  s14 = - x14 * cospi_24_64 + x15 * cospi_8_64;
  s15 =   x14 * cospi_8_64  + x15 * cospi_24_64;

  x0 = s0 + s2;
  x1 = s1 + s3;
  x2 = s0 - s2;
  x3 = s1 - s3;
  x4 = dct_const_round_shift(s4 + s6);
  x5 = dct_const_round_shift(s5 + s7);
  x6 = dct_const_round_shift(s4 - s6);
  x7 = dct_const_round_shift(s5 - s7);
  x8 = s8 + s10;
  x9 = s9 + s11;
  x10 = s8 - s10;
  x11 = s9 - s11;
  x12 = dct_const_round_shift(s12 + s14);
  x13 = dct_const_round_shift(s13 + s15);
  x14 = dct_const_round_shift(s12 - s14);
  x15 = dct_const_round_shift(s13 - s15);

  // stage 4
  s2 = (- cospi_16_64) * (x2 + x3);
  s3 = cospi_16_64 * (x2 - x3);
  s6 = cospi_16_64 * (x6 + x7);
  s7 = cospi_16_64 * (- x6 + x7);
  s10 = cospi_16_64 * (x10 + x11);
  s11 = cospi_16_64 * (- x10 + x11);
  s14 = (- cospi_16_64) * (x14 + x15);
  s15 = cospi_16_64 * (x14 - x15);

  x2 = dct_const_round_shift(s2);
  x3 = dct_const_round_shift(s3);
  x6 = dct_const_round_shift(s6);
  x7 = dct_const_round_shift(s7);
  x10 = dct_const_round_shift(s10);
  x11 = dct_const_round_shift(s11);
  x14 = dct_const_round_shift(s14);
  x15 = dct_const_round_shift(s15);

  output[0] = x0;
  output[1] = - x8;
  output[2] = x12;
  output[3] = - x4;
  output[4] = x6;
  output[5] = x14;
  output[6] = x10;
  output[7] = x2;
  output[8] = x3;
  output[9] =  x11;
  output[10] = x15;
  output[11] = x7;
  output[12] = x5;
  output[13] = - x13;
  output[14] = x9;
  output[15] = - x1;
}

void vp9_short_fht16x16_c(int16_t *input, int16_t *output,
                          int pitch, TX_TYPE tx_type) {
  int16_t out[256];
  int16_t *outptr = &out[0];
  const int short_pitch = pitch >> 1;
  int i, j;
  int16_t temp_in[16], temp_out[16];

  void (*fwdr)(int16_t*, int16_t*);
  void (*fwdc)(int16_t*, int16_t*);

  switch (tx_type) {
    case ADST_ADST:
      fwdc = &fadst16_1d;
      fwdr = &fadst16_1d;
      break;
    case ADST_DCT:
      fwdc = &fadst16_1d;
      fwdr = &fdct16_1d;
      break;
    case DCT_ADST:
      fwdc = &fdct16_1d;
      fwdr = &fadst16_1d;
      break;
    case DCT_DCT:
      fwdc = &fdct16_1d;
      fwdr = &fdct16_1d;
      break;
    default:
      assert(0);
  }

  // column transform
  for (i = 0; i < 16; ++i) {
    for (j = 0; j < 16; ++j)
      temp_in[j] = input[j * short_pitch + i];
    fwdc(temp_in, temp_out);
    for (j = 0; j < 16; ++j)
      outptr[j * 16 + i] = temp_out[j];
  }

  // row transform
  for (i = 0; i < 16; ++i) {
    for (j = 0; j < 16; ++j)
      temp_in[j] = out[j + i * 16];
    fwdr(temp_in, temp_out);
    for (j = 0; j < 16; ++j)
      output[j + i * 16] = temp_out[j];
  }
}
#endif


static void dct32_1d(int *input, int *output) {
  int step[32];
  // Stage 1
  step[0] = input[0] + input[(32 - 1)];
  step[1] = input[1] + input[(32 - 2)];
  step[2] = input[2] + input[(32 - 3)];
  step[3] = input[3] + input[(32 - 4)];
  step[4] = input[4] + input[(32 - 5)];
  step[5] = input[5] + input[(32 - 6)];
  step[6] = input[6] + input[(32 - 7)];
  step[7] = input[7] + input[(32 - 8)];
  step[8] = input[8] + input[(32 - 9)];
  step[9] = input[9] + input[(32 - 10)];
  step[10] = input[10] + input[(32 - 11)];
  step[11] = input[11] + input[(32 - 12)];
  step[12] = input[12] + input[(32 - 13)];
  step[13] = input[13] + input[(32 - 14)];
  step[14] = input[14] + input[(32 - 15)];
  step[15] = input[15] + input[(32 - 16)];
  step[16] = -input[16] + input[(32 - 17)];
  step[17] = -input[17] + input[(32 - 18)];
  step[18] = -input[18] + input[(32 - 19)];
  step[19] = -input[19] + input[(32 - 20)];
  step[20] = -input[20] + input[(32 - 21)];
  step[21] = -input[21] + input[(32 - 22)];
  step[22] = -input[22] + input[(32 - 23)];
  step[23] = -input[23] + input[(32 - 24)];
  step[24] = -input[24] + input[(32 - 25)];
  step[25] = -input[25] + input[(32 - 26)];
  step[26] = -input[26] + input[(32 - 27)];
  step[27] = -input[27] + input[(32 - 28)];
  step[28] = -input[28] + input[(32 - 29)];
  step[29] = -input[29] + input[(32 - 30)];
  step[30] = -input[30] + input[(32 - 31)];
  step[31] = -input[31] + input[(32 - 32)];

  // Stage 2
  output[0] = step[0] + step[16 - 1];
  output[1] = step[1] + step[16 - 2];
  output[2] = step[2] + step[16 - 3];
  output[3] = step[3] + step[16 - 4];
  output[4] = step[4] + step[16 - 5];
  output[5] = step[5] + step[16 - 6];
  output[6] = step[6] + step[16 - 7];
  output[7] = step[7] + step[16 - 8];
  output[8] = -step[8] + step[16 - 9];
  output[9] = -step[9] + step[16 - 10];
  output[10] = -step[10] + step[16 - 11];
  output[11] = -step[11] + step[16 - 12];
  output[12] = -step[12] + step[16 - 13];
  output[13] = -step[13] + step[16 - 14];
  output[14] = -step[14] + step[16 - 15];
  output[15] = -step[15] + step[16 - 16];

  output[16] = step[16];
  output[17] = step[17];
  output[18] = step[18];
  output[19] = step[19];

  output[20] = dct_32_round_shift((-step[20] + step[27]) * cospi_16_64);
  output[21] = dct_32_round_shift((-step[21] + step[26]) * cospi_16_64);
  output[22] = dct_32_round_shift((-step[22] + step[25]) * cospi_16_64);
  output[23] = dct_32_round_shift((-step[23] + step[24]) * cospi_16_64);

  output[24] = dct_32_round_shift((step[24] + step[23]) * cospi_16_64);
  output[25] = dct_32_round_shift((step[25] + step[22]) * cospi_16_64);
  output[26] = dct_32_round_shift((step[26] + step[21]) * cospi_16_64);
  output[27] = dct_32_round_shift((step[27] + step[20]) * cospi_16_64);

  output[28] = step[28];
  output[29] = step[29];
  output[30] = step[30];
  output[31] = step[31];

  // Stage 3
  step[0] = output[0] + output[(8 - 1)];
  step[1] = output[1] + output[(8 - 2)];
  step[2] = output[2] + output[(8 - 3)];
  step[3] = output[3] + output[(8 - 4)];
  step[4] = -output[4] + output[(8 - 5)];
  step[5] = -output[5] + output[(8 - 6)];
  step[6] = -output[6] + output[(8 - 7)];
  step[7] = -output[7] + output[(8 - 8)];
  step[8] = output[8];
  step[9] = output[9];
  step[10] = dct_32_round_shift((-output[10] + output[13]) * cospi_16_64);
  step[11] = dct_32_round_shift((-output[11] + output[12]) * cospi_16_64);
  step[12] = dct_32_round_shift((output[12] + output[11]) * cospi_16_64);
  step[13] = dct_32_round_shift((output[13] + output[10]) * cospi_16_64);
  step[14] = output[14];
  step[15] = output[15];

  step[16] = output[16] + output[23];
  step[17] = output[17] + output[22];
  step[18] = output[18] + output[21];
  step[19] = output[19] + output[20];
  step[20] = -output[20] + output[19];
  step[21] = -output[21] + output[18];
  step[22] = -output[22] + output[17];
  step[23] = -output[23] + output[16];
  step[24] = -output[24] + output[31];
  step[25] = -output[25] + output[30];
  step[26] = -output[26] + output[29];
  step[27] = -output[27] + output[28];
  step[28] = output[28] + output[27];
  step[29] = output[29] + output[26];
  step[30] = output[30] + output[25];
  step[31] = output[31] + output[24];

  // Stage 4
  output[0] = step[0] + step[3];
  output[1] = step[1] + step[2];
  output[2] = -step[2] + step[1];
  output[3] = -step[3] + step[0];
  output[4] = step[4];
  output[5] = dct_32_round_shift((-step[5] + step[6]) * cospi_16_64);
  output[6] = dct_32_round_shift((step[6] + step[5]) * cospi_16_64);
  output[7] = step[7];
  output[8] = step[8] + step[11];
  output[9] = step[9] + step[10];
  output[10] = -step[10] + step[9];
  output[11] = -step[11] + step[8];
  output[12] = -step[12] + step[15];
  output[13] = -step[13] + step[14];
  output[14] = step[14] + step[13];
  output[15] = step[15] + step[12];

  output[16] = step[16];
  output[17] = step[17];
  output[18] = dct_32_round_shift(step[18] * -cospi_8_64 + step[29] * cospi_24_64);
  output[19] = dct_32_round_shift(step[19] * -cospi_8_64 + step[28] * cospi_24_64);
  output[20] = dct_32_round_shift(step[20] * -cospi_24_64 + step[27] * -cospi_8_64);
  output[21] = dct_32_round_shift(step[21] * -cospi_24_64 + step[26] * -cospi_8_64);
  output[22] = step[22];
  output[23] = step[23];
  output[24] = step[24];
  output[25] = step[25];
  output[26] = dct_32_round_shift(step[26] * cospi_24_64 + step[21] * -cospi_8_64);
  output[27] = dct_32_round_shift(step[27] * cospi_24_64 + step[20] * -cospi_8_64);
  output[28] = dct_32_round_shift(step[28] * cospi_8_64 + step[19] * cospi_24_64);
  output[29] = dct_32_round_shift(step[29] * cospi_8_64 + step[18] * cospi_24_64);
  output[30] = step[30];
  output[31] = step[31];

  // Stage 5
  step[0] = dct_32_round_shift((output[0] + output[1]) * cospi_16_64);
  step[1] = dct_32_round_shift((-output[1] + output[0]) * cospi_16_64);
  step[2] = dct_32_round_shift(output[2] * cospi_24_64 + output[3] * cospi_8_64);
  step[3] = dct_32_round_shift(output[3] * cospi_24_64 - output[2] * cospi_8_64);
  step[4] = output[4] + output[5];
  step[5] = -output[5] + output[4];
  step[6] = -output[6] + output[7];
  step[7] = output[7] + output[6];
  step[8] = output[8];
  step[9] = dct_32_round_shift(output[9] * -cospi_8_64 + output[14] * cospi_24_64);
  step[10] = dct_32_round_shift(output[10] * -cospi_24_64 + output[13] * -cospi_8_64);
  step[11] = output[11];
  step[12] = output[12];
  step[13] = dct_32_round_shift(output[13] * cospi_24_64 + output[10] * -cospi_8_64);
  step[14] = dct_32_round_shift(output[14] * cospi_8_64 + output[9] * cospi_24_64);
  step[15] = output[15];

  step[16] = output[16] + output[19];
  step[17] = output[17] + output[18];
  step[18] = -output[18] + output[17];
  step[19] = -output[19] + output[16];
  step[20] = -output[20] + output[23];
  step[21] = -output[21] + output[22];
  step[22] = output[22] + output[21];
  step[23] = output[23] + output[20];
  step[24] = output[24] + output[27];
  step[25] = output[25] + output[26];
  step[26] = -output[26] + output[25];
  step[27] = -output[27] + output[24];
  step[28] = -output[28] + output[31];
  step[29] = -output[29] + output[30];
  step[30] = output[30] + output[29];
  step[31] = output[31] + output[28];

  // Stage 6
  output[0] = step[0];
  output[1] = step[1];
  output[2] = step[2];
  output[3] = step[3];
  output[4] = dct_32_round_shift(step[4] * cospi_28_64 + step[7] * cospi_4_64);
  output[5] = dct_32_round_shift(step[5] * cospi_12_64 + step[6] * cospi_20_64);
  output[6] = dct_32_round_shift(step[6] * cospi_12_64 + step[5] * -cospi_20_64);
  output[7] = dct_32_round_shift(step[7] * cospi_28_64 + step[4] * -cospi_4_64);
  output[8] = step[8] + step[9];
  output[9] = -step[9] + step[8];
  output[10] = -step[10] + step[11];
  output[11] = step[11] + step[10];
  output[12] = step[12] + step[13];
  output[13] = -step[13] + step[12];
  output[14] = -step[14] + step[15];
  output[15] = step[15] + step[14];

  output[16] = step[16];
  output[17] = dct_32_round_shift(step[17] * -cospi_4_64 + step[30] * cospi_28_64);
  output[18] = dct_32_round_shift(step[18] * -cospi_28_64 + step[29] * -cospi_4_64);
  output[19] = step[19];
  output[20] = step[20];
  output[21] = dct_32_round_shift(step[21] * -cospi_20_64 + step[26] * cospi_12_64);
  output[22] = dct_32_round_shift(step[22] * -cospi_12_64 + step[25] * -cospi_20_64);
  output[23] = step[23];
  output[24] = step[24];
  output[25] = dct_32_round_shift(step[25] * cospi_12_64 + step[22] * -cospi_20_64);
  output[26] = dct_32_round_shift(step[26] * cospi_20_64 + step[21] * cospi_12_64);
  output[27] = step[27];
  output[28] = step[28];
  output[29] = dct_32_round_shift(step[29] * cospi_28_64 + step[18] * -cospi_4_64);
  output[30] = dct_32_round_shift(step[30] * cospi_4_64 + step[17] * cospi_28_64);
  output[31] = step[31];

  // Stage 7
  step[0] = output[0];
  step[1] = output[1];
  step[2] = output[2];
  step[3] = output[3];
  step[4] = output[4];
  step[5] = output[5];
  step[6] = output[6];
  step[7] = output[7];
  step[8] = dct_32_round_shift(output[8] * cospi_30_64 + output[15] * cospi_2_64);
  step[9] = dct_32_round_shift(output[9] * cospi_14_64 + output[14] * cospi_18_64);
  step[10] = dct_32_round_shift(output[10] * cospi_22_64 + output[13] * cospi_10_64);
  step[11] = dct_32_round_shift(output[11] * cospi_6_64 + output[12] * cospi_26_64);
  step[12] = dct_32_round_shift(output[12] * cospi_6_64 + output[11] * -cospi_26_64);
  step[13] = dct_32_round_shift(output[13] * cospi_22_64 + output[10] * -cospi_10_64);
  step[14] = dct_32_round_shift(output[14] * cospi_14_64 + output[9] * -cospi_18_64);
  step[15] = dct_32_round_shift(output[15] * cospi_30_64 + output[8] * -cospi_2_64);

  step[16] = output[16] + output[17];
  step[17] = -output[17] + output[16];
  step[18] = -output[18] + output[19];
  step[19] = output[19] + output[18];
  step[20] = output[20] + output[21];
  step[21] = -output[21] + output[20];
  step[22] = -output[22] + output[23];
  step[23] = output[23] + output[22];
  step[24] = output[24] + output[25];
  step[25] = -output[25] + output[24];
  step[26] = -output[26] + output[27];
  step[27] = output[27] + output[26];
  step[28] = output[28] + output[29];
  step[29] = -output[29] + output[28];
  step[30] = -output[30] + output[31];
  step[31] = output[31] + output[30];

  // Final stage --- outputs indices are bit-reversed.
  output[0]  = step[0];
  output[16] = step[1];
  output[8]  = step[2];
  output[24] = step[3];
  output[4]  = step[4];
  output[20] = step[5];
  output[12] = step[6];
  output[28] = step[7];
  output[2]  = step[8];
  output[18] = step[9];
  output[10] = step[10];
  output[26] = step[11];
  output[6]  = step[12];
  output[22] = step[13];
  output[14] = step[14];
  output[30] = step[15];

  output[1]  = dct_32_round_shift(step[16] * cospi_31_64 + step[31] * cospi_1_64);
  output[17] = dct_32_round_shift(step[17] * cospi_15_64 + step[30] * cospi_17_64);
  output[9]  = dct_32_round_shift(step[18] * cospi_23_64 + step[29] * cospi_9_64);
  output[25] = dct_32_round_shift(step[19] * cospi_7_64 + step[28] * cospi_25_64);
  output[5]  = dct_32_round_shift(step[20] * cospi_27_64 + step[27] * cospi_5_64);
  output[21] = dct_32_round_shift(step[21] * cospi_11_64 + step[26] * cospi_21_64);
  output[13] = dct_32_round_shift(step[22] * cospi_19_64 + step[25] * cospi_13_64);
  output[29] = dct_32_round_shift(step[23] * cospi_3_64 + step[24] * cospi_29_64);
  output[3]  = dct_32_round_shift(step[24] * cospi_3_64 + step[23] * -cospi_29_64);
  output[19] = dct_32_round_shift(step[25] * cospi_19_64 + step[22] * -cospi_13_64);
  output[11] = dct_32_round_shift(step[26] * cospi_11_64 + step[21] * -cospi_21_64);
  output[27] = dct_32_round_shift(step[27] * cospi_27_64 + step[20] * -cospi_5_64);
  output[7]  = dct_32_round_shift(step[28] * cospi_7_64 + step[19] * -cospi_25_64);
  output[23] = dct_32_round_shift(step[29] * cospi_23_64 + step[18] * -cospi_9_64);
  output[15] = dct_32_round_shift(step[30] * cospi_15_64 + step[17] * -cospi_17_64);
  output[31] = dct_32_round_shift(step[31] * cospi_31_64 + step[16] * -cospi_1_64);
}

void vp9_short_fdct32x32_c(int16_t *input, int16_t *out, int pitch) {
  int shortpitch = pitch >> 1;
  int i, j;
  int output[1024];
  // First transform columns
  for (i = 0; i < 32; i++) {
    int temp_in[32], temp_out[32];
    for (j = 0; j < 32; j++)
      temp_in[j] = input[j * shortpitch + i] << 2;
    dct32_1d(temp_in, temp_out);
    for (j = 0; j < 32; j++)
      output[j * 32 + i] = (temp_out[j] + 1 + (temp_out[j]<0)) >> 2;
  }

  // Then transform rows
  for (i = 0; i < 32; ++i) {
    int temp_in[32], temp_out[32];
    for (j = 0; j < 32; ++j)
      temp_in[j] = output[j + i * 32];
    dct32_1d(temp_in, temp_out);
    for (j = 0; j < 32; ++j)
      out[j + i * 32] = (temp_out[j] + 1 + (temp_out[j]<0)) >> 2;
  }
}

