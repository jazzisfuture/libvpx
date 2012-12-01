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
#include <assert.h>
#include <math.h>
#include "vpx_ports/config.h"
#include "vp9/common/vp9_systemdependent.h"

#include "vp9/common/vp9_blockd.h"

static const int cospi8sqrt2minus1 = 20091;
static const int sinpi8sqrt2      = 35468;
static const int rounding = 0;

// TODO: these transforms can be further converted into integer forms
//       for complexity optimization
static const float idct_4[16] = {
  0.500000000000000,   0.653281482438188,   0.500000000000000,   0.270598050073099,
  0.500000000000000,   0.270598050073099,  -0.500000000000000,  -0.653281482438188,
  0.500000000000000,  -0.270598050073099,  -0.500000000000000,   0.653281482438188,
  0.500000000000000,  -0.653281482438188,   0.500000000000000,  -0.270598050073099
};

static const float iadst_4[16] = {
  0.228013428883779,   0.577350269189626,   0.656538502008139,   0.428525073124360,
  0.428525073124360,   0.577350269189626,  -0.228013428883779,  -0.656538502008139,
  0.577350269189626,                   0,  -0.577350269189626,   0.577350269189626,
  0.656538502008139,  -0.577350269189626,   0.428525073124359,  -0.228013428883779
};

static const float idct_8[64] = {
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

static const float iadst_8[64] = {
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

static const int16_t idct_i4[16] = {
  8192,  10703,  8192,   4433,
  8192,   4433, -8192, -10703,
  8192,  -4433, -8192,  10703,
  8192, -10703,  8192,  -4433
};

static const int16_t iadst_i4[16] = {
   3736,  9459, 10757,   7021,
   7021,  9459, -3736, -10757,
   9459,     0, -9459,   9459,
  10757, -9459,  7021,  -3736
};

static const int16_t idct_i8[64] = {
   5793,  8035,  7568,  6811,
   5793,  4551,  3135,  1598,
   5793,  6811,  3135, -1598,
  -5793, -8035, -7568, -4551,
   5793,  4551, -3135, -8035,
  -5793,  1598,  7568,  6811,
   5793,  1598, -7568, -4551,
   5793,  6811, -3135, -8035,
   5793, -1598, -7568,  4551,
   5793, -6811, -3135,  8035,
   5793, -4551, -3135,  8035,
  -5793, -1598,  7568, -6811,
   5793, -6811,  3135,  1598,
  -5793,  8035, -7568,  4551,
   5793, -8035,  7568, -6811,
   5793, -4551,  3135, -1598
};

static const int16_t iadst_i8[64] = {
   1460,  4184,  6342,  7644,
   7914,  7114,  5354,  2871,
   2871,  7114,  7644,  4184,
  -1460, -6342, -7914, -5354,
   4184,  7914,  2871, -5354,
  -7644, -1460,  6342,  7114,
   5354,  6342, -4184, -7114,
   2871,  7644, -1460, -7914,
   6342,  2871, -7914,  1460,
   7114, -5354, -4184,  7644,
   7114, -1460, -5354,  7914,
  -4184, -2871,  7644, -6342,
   7644, -5354,  1460,  2871,
  -6342,  7914, -7114,  4184,
   7914, -7644,  7114, -6342,
   5354, -4184,  2871, -1460
};

static float idct_16[256] = {
  0.250000,  0.351851,  0.346760,  0.338330,  0.326641,  0.311806,  0.293969,  0.273300,
  0.250000,  0.224292,  0.196424,  0.166664,  0.135299,  0.102631,  0.068975,  0.034654,
  0.250000,  0.338330,  0.293969,  0.224292,  0.135299,  0.034654, -0.068975, -0.166664,
 -0.250000, -0.311806, -0.346760, -0.351851, -0.326641, -0.273300, -0.196424, -0.102631,
  0.250000,  0.311806,  0.196424,  0.034654, -0.135299, -0.273300, -0.346760, -0.338330,
 -0.250000, -0.102631,  0.068975,  0.224292,  0.326641,  0.351851,  0.293969,  0.166664,
  0.250000,  0.273300,  0.068975, -0.166664, -0.326641, -0.338330, -0.196424,  0.034654,
  0.250000,  0.351851,  0.293969,  0.102631, -0.135299, -0.311806, -0.346760, -0.224292,
  0.250000,  0.224292, -0.068975, -0.311806, -0.326641, -0.102631,  0.196424,  0.351851,
  0.250000, -0.034654, -0.293969, -0.338330, -0.135299,  0.166664,  0.346760,  0.273300,
  0.250000,  0.166664, -0.196424, -0.351851, -0.135299,  0.224292,  0.346760,  0.102631,
 -0.250000, -0.338330, -0.068975,  0.273300,  0.326641,  0.034654, -0.293969, -0.311806,
  0.250000,  0.102631, -0.293969, -0.273300,  0.135299,  0.351851,  0.068975, -0.311806,
 -0.250000,  0.166664,  0.346760,  0.034654, -0.326641, -0.224292,  0.196424,  0.338330,
  0.250000,  0.034654, -0.346760, -0.102631,  0.326641,  0.166664, -0.293969, -0.224292,
  0.250000,  0.273300, -0.196424, -0.311806,  0.135299,  0.338330, -0.068975, -0.351851,
  0.250000, -0.034654, -0.346760,  0.102631,  0.326641, -0.166664, -0.293969,  0.224292,
  0.250000, -0.273300, -0.196424,  0.311806,  0.135299, -0.338330, -0.068975,  0.351851,
  0.250000, -0.102631, -0.293969,  0.273300,  0.135299, -0.351851,  0.068975,  0.311806,
 -0.250000, -0.166664,  0.346760, -0.034654, -0.326641,  0.224292,  0.196424, -0.338330,
  0.250000, -0.166664, -0.196424,  0.351851, -0.135299, -0.224292,  0.346760, -0.102631,
 -0.250000,  0.338330, -0.068975, -0.273300,  0.326641, -0.034654, -0.293969,  0.311806,
  0.250000, -0.224292, -0.068975,  0.311806, -0.326641,  0.102631,  0.196424, -0.351851,
  0.250000,  0.034654, -0.293969,  0.338330, -0.135299, -0.166664,  0.346760, -0.273300,
  0.250000, -0.273300,  0.068975,  0.166664, -0.326641,  0.338330, -0.196424, -0.034654,
  0.250000, -0.351851,  0.293969, -0.102631, -0.135299,  0.311806, -0.346760,  0.224292,
  0.250000, -0.311806,  0.196424, -0.034654, -0.135299,  0.273300, -0.346760,  0.338330,
 -0.250000,  0.102631,  0.068975, -0.224292,  0.326641, -0.351851,  0.293969, -0.166664,
  0.250000, -0.338330,  0.293969, -0.224292,  0.135299, -0.034654, -0.068975,  0.166664,
 -0.250000,  0.311806, -0.346760,  0.351851, -0.326641,  0.273300, -0.196424,  0.102631,
  0.250000, -0.351851,  0.346760, -0.338330,  0.326641, -0.311806,  0.293969, -0.273300,
  0.250000, -0.224292,  0.196424, -0.166664,  0.135299, -0.102631,  0.068975, -0.034654
};

static float iadst_16[256] = {
  0.033094,  0.098087,  0.159534,  0.215215,  0.263118,  0.301511,  0.329007,  0.344612,
  0.347761,  0.338341,  0.316693,  0.283599,  0.240255,  0.188227,  0.129396,  0.065889,
  0.065889,  0.188227,  0.283599,  0.338341,  0.344612,  0.301511,  0.215215,  0.098087,
 -0.033094, -0.159534, -0.263118, -0.329007, -0.347761, -0.316693, -0.240255, -0.129396,
  0.098087,  0.263118,  0.344612,  0.316693,  0.188227,  0.000000, -0.188227, -0.316693,
 -0.344612, -0.263118, -0.098087,  0.098087,  0.263118,  0.344612,  0.316693,  0.188227,
  0.129396,  0.316693,  0.329007,  0.159534, -0.098087, -0.301511, -0.338341, -0.188227,
  0.065889,  0.283599,  0.344612,  0.215215, -0.033094, -0.263118, -0.347761, -0.240255,
  0.159534,  0.344612,  0.240255, -0.065889, -0.316693, -0.301511, -0.033094,  0.263118,
  0.338341,  0.129396, -0.188227, -0.347761, -0.215215,  0.098087,  0.329007,  0.283599,
  0.188227,  0.344612,  0.098087, -0.263118, -0.316693, -0.000000,  0.316693,  0.263118,
 -0.098087, -0.344612, -0.188227,  0.188227,  0.344612,  0.098087, -0.263118, -0.316693,
  0.215215,  0.316693, -0.065889, -0.347761, -0.098087,  0.301511,  0.240255, -0.188227,
 -0.329007,  0.033094,  0.344612,  0.129396, -0.283599, -0.263118,  0.159534,  0.338341,
  0.240255,  0.263118, -0.215215, -0.283599,  0.188227,  0.301511, -0.159534, -0.316693,
  0.129396,  0.329007, -0.098087, -0.338341,  0.065889,  0.344612, -0.033094, -0.347761,
  0.263118,  0.188227, -0.316693, -0.098087,  0.344612,  0.000000, -0.344612,  0.098087,
  0.316693, -0.188227, -0.263118,  0.263118,  0.188227, -0.316693, -0.098087,  0.344612,
  0.283599,  0.098087, -0.347761,  0.129396,  0.263118, -0.301511, -0.065889,  0.344612,
 -0.159534, -0.240255,  0.316693,  0.033094, -0.338341,  0.188227,  0.215215, -0.329007,
  0.301511,  0.000000, -0.301511,  0.301511,  0.000000, -0.301511,  0.301511,  0.000000,
 -0.301511,  0.301511,  0.000000, -0.301511,  0.301511,  0.000000, -0.301511,  0.301511,
  0.316693, -0.098087, -0.188227,  0.344612, -0.263118, -0.000000,  0.263118, -0.344612,
  0.188227,  0.098087, -0.316693,  0.316693, -0.098087, -0.188227,  0.344612, -0.263118,
  0.329007, -0.188227, -0.033094,  0.240255, -0.344612,  0.301511, -0.129396, -0.098087,
  0.283599, -0.347761,  0.263118, -0.065889, -0.159534,  0.316693, -0.338341,  0.215215,
  0.338341, -0.263118,  0.129396,  0.033094, -0.188227,  0.301511, -0.347761,  0.316693,
 -0.215215,  0.065889,  0.098087, -0.240255,  0.329007, -0.344612,  0.283599, -0.159534,
  0.344612, -0.316693,  0.263118, -0.188227,  0.098087,  0.000000, -0.098087,  0.188227,
 -0.263118,  0.316693, -0.344612,  0.344612, -0.316693,  0.263118, -0.188227,  0.098087,
  0.347761, -0.344612,  0.338341, -0.329007,  0.316693, -0.301511,  0.283599, -0.263118,
  0.240255, -0.215215,  0.188227, -0.159534,  0.129396, -0.098087,  0.065889, -0.033094
};

static const int16_t idct_i16[256] = {
   4096,  5765,  5681,  5543,  5352,  5109,  4816,  4478,
   4096,  3675,  3218,  2731,  2217,  1682,  1130,   568,
   4096,  5543,  4816,  3675,  2217,   568, -1130, -2731,
  -4096, -5109, -5681, -5765, -5352, -4478, -3218, -1682,
   4096,  5109,  3218,   568, -2217, -4478, -5681, -5543,
  -4096, -1682,  1130,  3675,  5352,  5765,  4816,  2731,
   4096,  4478,  1130, -2731, -5352, -5543, -3218,   568,
   4096,  5765,  4816,  1682, -2217, -5109, -5681, -3675,
   4096,  3675, -1130, -5109, -5352, -1682,  3218,  5765,
   4096,  -568, -4816, -5543, -2217,  2731,  5681,  4478,
   4096,  2731, -3218, -5765, -2217,  3675,  5681,  1682,
  -4096, -5543, -1130,  4478,  5352,   568, -4816, -5109,
   4096,  1682, -4816, -4478,  2217,  5765,  1130, -5109,
  -4096,  2731,  5681,   568, -5352, -3675,  3218,  5543,
   4096,   568, -5681, -1682,  5352,  2731, -4816, -3675,
   4096,  4478, -3218, -5109,  2217,  5543, -1130, -5765,
   4096,  -568, -5681,  1682,  5352, -2731, -4816,  3675,
   4096, -4478, -3218,  5109,  2217, -5543, -1130,  5765,
   4096, -1682, -4816,  4478,  2217, -5765,  1130,  5109,
  -4096, -2731,  5681,  -568, -5352,  3675,  3218, -5543,
   4096, -2731, -3218,  5765, -2217, -3675,  5681, -1682,
  -4096,  5543, -1130, -4478,  5352,  -568, -4816,  5109,
   4096, -3675, -1130,  5109, -5352,  1682,  3218, -5765,
   4096,   568, -4816,  5543, -2217, -2731,  5681, -4478,
   4096, -4478,  1130,  2731, -5352,  5543, -3218,  -568,
   4096, -5765,  4816, -1682, -2217,  5109, -5681,  3675,
   4096, -5109,  3218,  -568, -2217,  4478, -5681,  5543,
  -4096,  1682,  1130, -3675,  5352, -5765,  4816, -2731,
   4096, -5543,  4816, -3675,  2217,  -568, -1130,  2731,
  -4096,  5109, -5681,  5765, -5352,  4478, -3218,  1682,
   4096, -5765,  5681, -5543,  5352, -5109,  4816, -4478,
   4096, -3675,  3218, -2731,  2217, -1682,  1130,  -568
};

static const int16_t iadst_i16[256] = {
    542,  1607,  2614,  3526,  4311,  4940,  5390,  5646,
   5698,  5543,  5189,  4646,  3936,  3084,  2120,  1080,
   1080,  3084,  4646,  5543,  5646,  4940,  3526,  1607,
   -542, -2614, -4311, -5390, -5698, -5189, -3936, -2120,
   1607,  4311,  5646,  5189,  3084,     0, -3084, -5189,
  -5646, -4311, -1607,  1607,  4311,  5646,  5189,  3084,
   2120,  5189,  5390,  2614, -1607, -4940, -5543, -3084,
   1080,  4646,  5646,  3526, -542,  -4311, -5698, -3936,
   2614,  5646,  3936, -1080, -5189, -4940,  -542,  4311,
   5543,  2120, -3084, -5698, -3526,  1607,  5390,  4646,
   3084,  5646,  1607, -4311, -5189,     0,  5189,  4311,
  -1607, -5646, -3084,  3084,  5646,  1607, -4311, -5189,
   3526,  5189, -1080, -5698, -1607,  4940,  3936, -3084,
  -5390,   542,  5646,  2120, -4646, -4311,  2614,  5543,
   3936,  4311, -3526, -4646,  3084,  4940, -2614, -5189,
   2120,  5390, -1607, -5543,  1080,  5646,  -542, -5698,
   4311,  3084, -5189, -1607,  5646,     0, -5646,  1607,
   5189, -3084, -4311,  4311,  3084, -5189, -1607,  5646,
   4646,  1607, -5698,  2120,  4311, -4940, -1080,  5646,
  -2614, -3936,  5189,   542, -5543,  3084,  3526, -5390,
   4940,     0, -4940,  4940,     0, -4940,  4940,     0,
  -4940,  4940,     0, -4940,  4940,     0, -4940,  4940,
   5189, -1607, -3084,  5646, -4311,     0,  4311, -5646,
   3084,  1607, -5189,  5189, -1607, -3084,  5646, -4311,
   5390, -3084,  -542,  3936, -5646,  4940, -2120, -1607,
   4646, -5698,  4311, -1080, -2614,  5189, -5543,  3526,
   5543, -4311,  2120,   542, -3084,  4940, -5698,  5189,
  -3526,  1080,  1607, -3936,  5390, -5646,  4646, -2614,
   5646, -5189,  4311, -3084,  1607,     0, -1607,  3084,
  -4311,  5189, -5646,  5646, -5189,  4311, -3084,  1607,
   5698, -5646,  5543, -5390,  5189, -4940,  4646, -4311,
   3936, -3526,  3084, -2614,  2120, -1607,  1080,  -542
};

void vp9_ihtllm_float_c(const int16_t *input, int16_t *output, int pitch,
                  TX_TYPE tx_type, int tx_dim) {
  vp9_clear_system_state();  // Make it simd safe : __asm emms;
  {
    int i, j, k;
    float bufa[256], bufb[256];  // buffers are for floating-point test purpose
                                 // the implementation could be simplified in
                                 // conjunction with integer transform
    const int16_t *ip = input;
    int16_t *op = output;
    int shortpitch = pitch >> 1;

    float *pfa = &bufa[0];
    float *pfb = &bufb[0];

    // pointers to vertical and horizontal transforms
    const float *ptv, *pth;

    assert(tx_type != DCT_DCT);
    // load and convert residual array into floating-point
    for(j = 0; j < tx_dim; j++) {
      for(i = 0; i < tx_dim; i++) {
        pfa[i] = (float)ip[i];
      }
      pfa += tx_dim;
      ip  += tx_dim;
    }

    // vertical transformation
    pfa = &bufa[0];
    pfb = &bufb[0];

    switch(tx_type) {
      case ADST_ADST :
      case ADST_DCT  :
        ptv = (tx_dim == 4) ? &iadst_4[0] :
                              ((tx_dim == 8) ? &iadst_8[0] : &iadst_16[0]);
        break;

      default :
        ptv = (tx_dim == 4) ? &idct_4[0] :
                              ((tx_dim == 8) ? &idct_8[0] : &idct_16[0]);
        break;
    }

    for(j = 0; j < tx_dim; j++) {
      for(i = 0; i < tx_dim; i++) {
        pfb[i] = 0 ;
        for(k = 0; k < tx_dim; k++) {
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

    switch(tx_type) {
      case ADST_ADST :
      case  DCT_ADST :
        pth = (tx_dim == 4) ? &iadst_4[0] :
                              ((tx_dim == 8) ? &iadst_8[0] : &iadst_16[0]);
        break;

      default :
        pth = (tx_dim == 4) ? &idct_4[0] :
                              ((tx_dim == 8) ? &idct_8[0] : &idct_16[0]);
        break;
    }

    for(j = 0; j < tx_dim; j++) {
      for(i = 0; i < tx_dim; i++) {
        pfa[i] = 0;
        for(k = 0; k < tx_dim; k++) {
          pfa[i] += pfb[k] * pth[k];
        }
        pth += tx_dim;
       }

      pfa += tx_dim;
      pfb += tx_dim;

      switch(tx_type) {
        case ADST_ADST :
        case  DCT_ADST :
          pth = (tx_dim == 4) ? &iadst_4[0] :
                                ((tx_dim == 8) ? &iadst_8[0] : &iadst_16[0]);
          break;

        default :
          pth = (tx_dim == 4) ? &idct_4[0] :
                                ((tx_dim == 8) ? &idct_8[0] : &idct_16[0]);
          break;
      }
    }

    // convert to short integer format and load BLOCKD buffer
    op  = output;
    pfa = &bufa[0];

    for(j = 0; j < tx_dim; j++) {
      for(i = 0; i < tx_dim; i++) {
        op[i] = (pfa[i] > 0 ) ? (int16_t)( pfa[i] / 8 + 0.49) :
                               -(int16_t)( - pfa[i] / 8 + 0.49);
      }

      op += shortpitch;
      pfa += tx_dim;
    }
  }
  vp9_clear_system_state(); // Make it simd safe : __asm emms;
}

/* Converted the transforms to integer form. */
#define VERTICAL_SHIFT 14  // 16
#define VERTICAL_ROUNDING ((1 << (VERTICAL_SHIFT - 1)) - 1)
#define HORIZONTAL_SHIFT 17  // 15
#define HORIZONTAL_ROUNDING ((1 << (HORIZONTAL_SHIFT - 1)) - 1)
void vp9_ihtllm_c(const int16_t *input, int16_t *output, int pitch,
                      TX_TYPE tx_type, int tx_dim) {
  int i, j, k;
  int16_t imbuf[256];

  const int16_t *ip = input;
  int16_t *op = output;
  int16_t *im = &imbuf[0];

  /* pointers to vertical and horizontal transforms. */
  const int16_t *ptv = NULL, *pth = NULL;
  int shortpitch = pitch >> 1;

  switch (tx_type) {
    case ADST_ADST :
      ptv = pth = (tx_dim == 4) ? &iadst_i4[0]
                                  : ((tx_dim == 8) ? &iadst_i8[0]
                                                     : &iadst_i16[0]);
      break;
    case ADST_DCT  :
      ptv = (tx_dim == 4) ? &iadst_i4[0]
                            : ((tx_dim == 8) ? &iadst_i8[0] : &iadst_i16[0]);
      pth = (tx_dim == 4) ? &idct_i4[0]
                            : ((tx_dim == 8) ? &idct_i8[0] : &idct_i16[0]);
      break;
    case  DCT_ADST :
      ptv = (tx_dim == 4) ? &idct_i4[0]
                            : ((tx_dim == 8) ? &idct_i8[0] : &idct_i16[0]);
      pth = (tx_dim == 4) ? &iadst_i4[0]
                            : ((tx_dim == 8) ? &iadst_i8[0] : &iadst_i16[0]);
      break;
    case  DCT_DCT :
      ptv = pth = (tx_dim == 4) ? &idct_i4[0]
                                  : ((tx_dim == 8) ? &idct_i8[0]
                                                     : &idct_i16[0]);
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
        temp += ptv[k] * ip[(k * tx_dim)];
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
    op += shortpitch;
  }
}

void vp9_short_idct4x4llm_c(short *input, short *output, int pitch) {
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

void vp9_short_idct4x4llm_1_c(short *input, short *output, int pitch) {
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

void vp9_dc_only_idct_add_c(short input_dc, unsigned char *pred_ptr,
                            unsigned char *dst_ptr, int pitch, int stride) {
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

void vp9_short_inv_walsh4x4_c(short *input, short *output) {
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

void vp9_short_inv_walsh4x4_1_c(short *in, short *out) {
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
void vp9_short_inv_walsh4x4_lossless_c(short *input, short *output) {
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

void vp9_short_inv_walsh4x4_1_lossless_c(short *in, short *out) {
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

void vp9_short_inv_walsh4x4_x8_c(short *input, short *output, int pitch) {
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

void vp9_short_inv_walsh4x4_1_x8_c(short *in, short *out, int pitch) {
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

void vp9_dc_only_inv_walsh_add_c(short input_dc, unsigned char *pred_ptr,
                                 unsigned char *dst_ptr,
                                 int pitch, int stride) {
  int r, c;
  short tmp[16];
  vp9_short_inv_walsh4x4_1_x8_c(&input_dc, tmp, 4 << 1);

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

void vp9_dc_only_idct_add_8x8_c(short input_dc,
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
void vp9_short_idct8x8_c(short *coefs, short *block, int pitch) {
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

/* Row IDCT when only first 4 coefficients are non-zero. */
static void idctrow10(int *blk) {
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
  x5 = W7 * x4;
  x4 = W1 * x4;
  x6 = W3 * x7;
  x7 = -W5 * x7;

  /* second stage */
  x2 = W6 * x3;
  x3 = W2 * x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;

  /* third stage */
  x7 = x0 + x3;
  x8 = x0 - x3;
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

/* Column (vertical) IDCT when only first 4 coefficients are non-zero. */
static void idctcol10(int *blk) {
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
  x5 = (W7 * x4 + 4) >> 3;
  x4 = (W1 * x4 + 4) >> 3;
  x6 = (W3 * x7 + 4) >> 3;
  x7 = (-W5 * x7 + 4) >> 3;

  /* second stage */
  x2 = (W6 * x3 + 4) >> 3;
  x3 = (W2 * x3 + 4) >> 3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;

  /* third stage */
  x7 = x0 + x3;
  x8 = x0 - x3;
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

void vp9_short_idct10_8x8_c(short *coefs, short *block, int pitch) {
  int X[TX_DIM * TX_DIM];
  int i, j;
  int shortpitch = pitch >> 1;

  for (i = 0; i < TX_DIM; i++) {
    for (j = 0; j < TX_DIM; j++) {
      X[i * TX_DIM + j] = (int)(coefs[i * TX_DIM + j] + 1
                                + (coefs[i * TX_DIM + j] < 0)) >> 2;
    }
  }

  /* Do first 4 row idct only since non-zero dct coefficients are all in
   *  upper-left 4x4 area. */
  for (i = 0; i < 4; i++)
    idctrow10(X + 8 * i);

  for (i = 0; i < 8; i++)
    idctcol10(X + i);

  for (i = 0; i < TX_DIM; i++) {
    for (j = 0; j < TX_DIM; j++) {
      block[i * shortpitch + j]  = X[i * TX_DIM + j] >> 1;
    }
  }
}

void vp9_short_ihaar2x2_c(short *input, short *output, int pitch) {
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


#if 0
// Keep a really bad float version as reference for now.
void vp9_short_idct16x16_c(short *input, short *output, int pitch) {

  vp9_clear_system_state(); // Make it simd safe : __asm emms;
  {
    double x;
    const int short_pitch = pitch >> 1;
    int i, j, k, l;
    for (l = 0; l < 16; ++l) {
      for (k = 0; k < 16; ++k) {
        double s = 0;
        for (i = 0; i < 16; ++i) {
          for (j = 0; j < 16; ++j) {
            x=cos(PI*j*(l+0.5)/16.0)*cos(PI*i*(k+0.5)/16.0)*input[i*16+j]/32;
            if (i != 0)
              x *= sqrt(2.0);
            if (j != 0)
              x *= sqrt(2.0);
            s += x;
          }
        }
        output[k*short_pitch+l] = (short)round(s);
      }
    }
  }
  vp9_clear_system_state(); // Make it simd safe : __asm emms;
}
#endif

#define TEST_INT_16x16_IDCT 1
#if !TEST_INT_16x16_IDCT
static const double C1 = 0.995184726672197;
static const double C2 = 0.98078528040323;
static const double C3 = 0.956940335732209;
static const double C4 = 0.923879532511287;
static const double C5 = 0.881921264348355;
static const double C6 = 0.831469612302545;
static const double C7 = 0.773010453362737;
static const double C8 = 0.707106781186548;
static const double C9 = 0.634393284163646;
static const double C10 = 0.555570233019602;
static const double C11 = 0.471396736825998;
static const double C12 = 0.38268343236509;
static const double C13 = 0.290284677254462;
static const double C14 = 0.195090322016128;
static const double C15 = 0.098017140329561;


static void butterfly_16x16_idct_1d(double input[16], double output[16]) {

  vp9_clear_system_state(); // Make it simd safe : __asm emms;
  {
    double step[16];
    double intermediate[16];
    double temp1, temp2;


    // step 1 and 2
    step[ 0] = input[0] + input[8];
    step[ 1] = input[0] - input[8];

    temp1 = input[4]*C12;
    temp2 = input[12]*C4;

    temp1 -= temp2;
    temp1 *= C8;

    step[ 2] = 2*(temp1);

    temp1 = input[4]*C4;
    temp2 = input[12]*C12;
    temp1 += temp2;
    temp1 = (temp1);
    temp1 *= C8;
    step[ 3] = 2*(temp1);

    temp1 = input[2]*C8;
    temp1 = 2*(temp1);
    temp2 = input[6] + input[10];

    step[ 4] = temp1 + temp2;
    step[ 5] = temp1 - temp2;

    temp1 = input[14]*C8;
    temp1 = 2*(temp1);
    temp2 = input[6] - input[10];

    step[ 6] = temp2 - temp1;
    step[ 7] = temp2 + temp1;

    // for odd input
    temp1 = input[3]*C12;
    temp2 = input[13]*C4;
    temp1 += temp2;
    temp1 = (temp1);
    temp1 *= C8;
    intermediate[ 8] = 2*(temp1);

    temp1 = input[3]*C4;
    temp2 = input[13]*C12;
    temp2 -= temp1;
    temp2 = (temp2);
    temp2 *= C8;
    intermediate[ 9] = 2*(temp2);

    intermediate[10] = 2*(input[9]*C8);
    intermediate[11] = input[15] - input[1];
    intermediate[12] = input[15] + input[1];
    intermediate[13] = 2*((input[7]*C8));

    temp1 = input[11]*C12;
    temp2 = input[5]*C4;
    temp2 -= temp1;
    temp2 = (temp2);
    temp2 *= C8;
    intermediate[14] = 2*(temp2);

    temp1 = input[11]*C4;
    temp2 = input[5]*C12;
    temp1 += temp2;
    temp1 = (temp1);
    temp1 *= C8;
    intermediate[15] = 2*(temp1);

    step[ 8] = intermediate[ 8] + intermediate[14];
    step[ 9] = intermediate[ 9] + intermediate[15];
    step[10] = intermediate[10] + intermediate[11];
    step[11] = intermediate[10] - intermediate[11];
    step[12] = intermediate[12] + intermediate[13];
    step[13] = intermediate[12] - intermediate[13];
    step[14] = intermediate[ 8] - intermediate[14];
    step[15] = intermediate[ 9] - intermediate[15];

    // step 3
    output[0] = step[ 0] + step[ 3];
    output[1] = step[ 1] + step[ 2];
    output[2] = step[ 1] - step[ 2];
    output[3] = step[ 0] - step[ 3];

    temp1 = step[ 4]*C14;
    temp2 = step[ 7]*C2;
    temp1 -= temp2;
    output[4] =  (temp1);

    temp1 = step[ 4]*C2;
    temp2 = step[ 7]*C14;
    temp1 += temp2;
    output[7] =  (temp1);

    temp1 = step[ 5]*C10;
    temp2 = step[ 6]*C6;
    temp1 -= temp2;
    output[5] =  (temp1);

    temp1 = step[ 5]*C6;
    temp2 = step[ 6]*C10;
    temp1 += temp2;
    output[6] =  (temp1);

    output[8] = step[ 8] + step[11];
    output[9] = step[ 9] + step[10];
    output[10] = step[ 9] - step[10];
    output[11] = step[ 8] - step[11];
    output[12] = step[12] + step[15];
    output[13] = step[13] + step[14];
    output[14] = step[13] - step[14];
    output[15] = step[12] - step[15];

    // output 4
    step[ 0] = output[0] + output[7];
    step[ 1] = output[1] + output[6];
    step[ 2] = output[2] + output[5];
    step[ 3] = output[3] + output[4];
    step[ 4] = output[3] - output[4];
    step[ 5] = output[2] - output[5];
    step[ 6] = output[1] - output[6];
    step[ 7] = output[0] - output[7];

    temp1 = output[8]*C7;
    temp2 = output[15]*C9;
    temp1 -= temp2;
    step[ 8] = (temp1);

    temp1 = output[9]*C11;
    temp2 = output[14]*C5;
    temp1 += temp2;
    step[ 9] = (temp1);

    temp1 = output[10]*C3;
    temp2 = output[13]*C13;
    temp1 -= temp2;
    step[10] = (temp1);

    temp1 = output[11]*C15;
    temp2 = output[12]*C1;
    temp1 += temp2;
    step[11] = (temp1);

    temp1 = output[11]*C1;
    temp2 = output[12]*C15;
    temp2 -= temp1;
    step[12] = (temp2);

    temp1 = output[10]*C13;
    temp2 = output[13]*C3;
    temp1 += temp2;
    step[13] = (temp1);

    temp1 = output[9]*C5;
    temp2 = output[14]*C11;
    temp2 -= temp1;
    step[14] = (temp2);

    temp1 = output[8]*C9;
    temp2 = output[15]*C7;
    temp1 += temp2;
    step[15] = (temp1);

    // step 5
    output[0] = (step[0] + step[15]);
    output[1] = (step[1] + step[14]);
    output[2] = (step[2] + step[13]);
    output[3] = (step[3] + step[12]);
    output[4] = (step[4] + step[11]);
    output[5] = (step[5] + step[10]);
    output[6] = (step[6] + step[ 9]);
    output[7] = (step[7] + step[ 8]);

    output[15] = (step[0] - step[15]);
    output[14] = (step[1] - step[14]);
    output[13] = (step[2] - step[13]);
    output[12] = (step[3] - step[12]);
    output[11] = (step[4] - step[11]);
    output[10] = (step[5] - step[10]);
    output[9] = (step[6] - step[ 9]);
    output[8] = (step[7] - step[ 8]);
  }
  vp9_clear_system_state(); // Make it simd safe : __asm emms;
}

// Remove once an int version of iDCT is written
#if 0
void reference_16x16_idct_1d(double input[16], double output[16]) {

  vp9_clear_system_state(); // Make it simd safe : __asm emms;
  {
    const double kPi = 3.141592653589793238462643383279502884;
    const double kSqrt2 = 1.414213562373095048801688724209698;
    for (int k = 0; k < 16; k++) {
      output[k] = 0.0;
      for (int n = 0; n < 16; n++) {
        output[k] += input[n]*cos(kPi*(2*k+1)*n/32.0);
        if (n == 0)
          output[k] = output[k]/kSqrt2;
      }
    }
  }
  vp9_clear_system_state(); // Make it simd safe : __asm emms;
}
#endif

void vp9_short_idct16x16_c(short *input, short *output, int pitch) {

  vp9_clear_system_state(); // Make it simd safe : __asm emms;
  {
    double out[16*16], out2[16*16];
    const int short_pitch = pitch >> 1;
    int i, j;
      // First transform rows
    for (i = 0; i < 16; ++i) {
      double temp_in[16], temp_out[16];
      for (j = 0; j < 16; ++j)
        temp_in[j] = input[j + i*short_pitch];
      butterfly_16x16_idct_1d(temp_in, temp_out);
      for (j = 0; j < 16; ++j)
        out[j + i*16] = temp_out[j];
    }
    // Then transform columns
    for (i = 0; i < 16; ++i) {
      double temp_in[16], temp_out[16];
      for (j = 0; j < 16; ++j)
        temp_in[j] = out[j*16 + i];
      butterfly_16x16_idct_1d(temp_in, temp_out);
      for (j = 0; j < 16; ++j)
        out2[j*16 + i] = temp_out[j];
    }
    for (i = 0; i < 16*16; ++i)
      output[i] = round(out2[i]/128);
  }
  vp9_clear_system_state(); // Make it simd safe : __asm emms;
}

#else
static const int16_t C1 = 16305;
static const int16_t C2 = 16069;
static const int16_t C3 = 15679;
static const int16_t C4 = 15137;
static const int16_t C5 = 14449;
static const int16_t C6 = 13623;
static const int16_t C7 = 12665;
static const int16_t C8 = 11585;
static const int16_t C9 = 10394;
static const int16_t C10 = 9102;
static const int16_t C11 = 7723;
static const int16_t C12 = 6270;
static const int16_t C13 = 4756;
static const int16_t C14 = 3196;
static const int16_t C15 = 1606;

#define INITIAL_SHIFT 2
#define INITIAL_ROUNDING (1 << (INITIAL_SHIFT - 1))
#define RIGHT_SHIFT 14
#define RIGHT_ROUNDING (1 << (RIGHT_SHIFT - 1))

static void butterfly_16x16_idct_1d(int16_t input[16], int16_t output[16],
                                    int last_shift_bits) {
    int16_t step[16];
    int intermediate[16];
    int temp1, temp2;

    int step1_shift = RIGHT_SHIFT + INITIAL_SHIFT;
    int step1_rounding = 1 << (step1_shift - 1);
    int last_rounding = 0;

    if (last_shift_bits > 0)
      last_rounding = 1 << (last_shift_bits - 1);

    // step 1 and 2
    step[ 0] = (input[0] + input[8] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[ 1] = (input[0] - input[8] + INITIAL_ROUNDING) >> INITIAL_SHIFT;

    temp1 = input[4] * C12;
    temp2 = input[12] * C4;
    temp1 = (temp1 - temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp1  *= C8;
    step[ 2] = (2 * (temp1) + step1_rounding) >> step1_shift;

    temp1 = input[4] * C4;
    temp2 = input[12] * C12;
    temp1 = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp1 *= C8;
    step[ 3] = (2 * (temp1) + step1_rounding) >> step1_shift;

    temp1 = input[2] * C8;
    temp1 = (2 * (temp1) +   RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp2 = input[6] + input[10];
    step[ 4] = (temp1 + temp2 + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[ 5] = (temp1 - temp2 + INITIAL_ROUNDING) >> INITIAL_SHIFT;

    temp1 = input[14] * C8;
    temp1 = (2 * (temp1) +   RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp2 = input[6] - input[10];
    step[ 6] = (temp2 - temp1 + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[ 7] = (temp2 + temp1 + INITIAL_ROUNDING) >> INITIAL_SHIFT;

    // for odd input
    temp1 = input[3] * C12;
    temp2 = input[13] * C4;
    temp1 = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp1 *= C8;
    intermediate[ 8] = (2 * (temp1) +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = input[3] * C4;
    temp2 = input[13] * C12;
    temp2 = (temp2 - temp1 + RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp2 *= C8;
    intermediate[ 9] = (2 * (temp2) +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    intermediate[10] = (2 * (input[9] * C8) + RIGHT_ROUNDING) >> RIGHT_SHIFT;
    intermediate[11] = input[15] - input[1];
    intermediate[12] = input[15] + input[1];
    intermediate[13] = (2 * (input[7] * C8) + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = input[11] * C12;
    temp2 = input[5] * C4;
    temp2 = (temp2 - temp1 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp2 *= C8;
    intermediate[14] = (2 * (temp2) +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = input[11] * C4;
    temp2 = input[5] * C12;
    temp1 = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp1 *= C8;
    intermediate[15] = (2 * (temp1) +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    step[ 8] = (intermediate[ 8] + intermediate[14] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;
    step[ 9] = (intermediate[ 9] + intermediate[15] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;
    step[10] = (intermediate[10] + intermediate[11] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;
    step[11] = (intermediate[10] - intermediate[11] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;
    step[12] = (intermediate[12] + intermediate[13] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;
    step[13] = (intermediate[12] - intermediate[13] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;
    step[14] = (intermediate[ 8] - intermediate[14] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;
    step[15] = (intermediate[ 9] - intermediate[15] + INITIAL_ROUNDING)
        >> INITIAL_SHIFT;

    // step 3
    output[0] = step[ 0] + step[ 3];
    output[1] = step[ 1] + step[ 2];
    output[2] = step[ 1] - step[ 2];
    output[3] = step[ 0] - step[ 3];

    temp1 = step[ 4] * C14;
    temp2 = step[ 7] * C2;
    output[4] =  (temp1 - temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = step[ 4] * C2;
    temp2 = step[ 7] * C14;
    output[7] =  (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = step[ 5] * C10;
    temp2 = step[ 6] * C6;
    output[5] =  (temp1 - temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = step[ 5] * C6;
    temp2 = step[ 6] * C10;
    output[6] =  (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    output[8] = step[ 8] + step[11];
    output[9] = step[ 9] + step[10];
    output[10] = step[ 9] - step[10];
    output[11] = step[ 8] - step[11];
    output[12] = step[12] + step[15];
    output[13] = step[13] + step[14];
    output[14] = step[13] - step[14];
    output[15] = step[12] - step[15];

    // output 4
    step[ 0] = output[0] + output[7];
    step[ 1] = output[1] + output[6];
    step[ 2] = output[2] + output[5];
    step[ 3] = output[3] + output[4];
    step[ 4] = output[3] - output[4];
    step[ 5] = output[2] - output[5];
    step[ 6] = output[1] - output[6];
    step[ 7] = output[0] - output[7];

    temp1 = output[8] * C7;
    temp2 = output[15] * C9;
    step[ 8] = (temp1 - temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[9] * C11;
    temp2 = output[14] * C5;
    step[ 9] = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[10] * C3;
    temp2 = output[13] * C13;
    step[10] = (temp1 - temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[11] * C15;
    temp2 = output[12] * C1;
    step[11] = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[11] * C1;
    temp2 = output[12] * C15;
    step[12] = (temp2 - temp1 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[10] * C13;
    temp2 = output[13] * C3;
    step[13] = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[9] * C5;
    temp2 = output[14] * C11;
    step[14] = (temp2 - temp1 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[8] * C9;
    temp2 = output[15] * C7;
    step[15] = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    // step 5
    output[0] = (step[0] + step[15] + last_rounding) >> last_shift_bits;
    output[1] = (step[1] + step[14] + last_rounding) >> last_shift_bits;
    output[2] = (step[2] + step[13] + last_rounding) >> last_shift_bits;
    output[3] = (step[3] + step[12] + last_rounding) >> last_shift_bits;
    output[4] = (step[4] + step[11] + last_rounding) >> last_shift_bits;
    output[5] = (step[5] + step[10] + last_rounding) >> last_shift_bits;
    output[6] = (step[6] + step[ 9] + last_rounding) >> last_shift_bits;
    output[7] = (step[7] + step[ 8] + last_rounding) >> last_shift_bits;

    output[15] = (step[0] - step[15] + last_rounding) >> last_shift_bits;
    output[14] = (step[1] - step[14] + last_rounding) >> last_shift_bits;
    output[13] = (step[2] - step[13] + last_rounding) >> last_shift_bits;
    output[12] = (step[3] - step[12] + last_rounding) >> last_shift_bits;
    output[11] = (step[4] - step[11] + last_rounding) >> last_shift_bits;
    output[10] = (step[5] - step[10] + last_rounding) >> last_shift_bits;
    output[9] = (step[6] - step[ 9] + last_rounding) >> last_shift_bits;
    output[8] = (step[7] - step[ 8] + last_rounding) >> last_shift_bits;
}

void vp9_short_idct16x16_c(int16_t *input, int16_t *output, int pitch) {
    int16_t out[16 * 16];
    int16_t *outptr = &out[0];
    const int short_pitch = pitch >> 1;
    int i, j;
    int16_t temp_in[16], temp_out[16];

    // First transform rows
    for (i = 0; i < 16; ++i) {
      butterfly_16x16_idct_1d(input, outptr, 0);
      input += short_pitch;
      outptr += 16;
    }

    // Then transform columns
    for (i = 0; i < 16; ++i) {
      for (j = 0; j < 16; ++j)
        temp_in[j] = out[j * 16 + i];
      butterfly_16x16_idct_1d(temp_in, temp_out, 3);
      for (j = 0; j < 16; ++j)
        output[j * 16 + i] = temp_out[j];
    }
}

/* The following function is called when we know the maximum number of non-zero
 * dct coefficients is less or equal 10.
 */
static void butterfly_16x16_idct10_1d(int16_t input[16], int16_t output[16],
                                      int last_shift_bits) {
    int16_t step[16] = {0};
    int intermediate[16] = {0};
    int temp1, temp2;
    int last_rounding = 0;

    if (last_shift_bits > 0)
      last_rounding = 1 << (last_shift_bits - 1);

    // step 1 and 2
    step[ 0] = (input[0] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[ 1] = (input[0] + INITIAL_ROUNDING) >> INITIAL_SHIFT;

    temp1 = (2 * (input[2] * C8) + RIGHT_ROUNDING) >> RIGHT_SHIFT;
    step[ 4] = (temp1 + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[ 5] = (temp1 + INITIAL_ROUNDING) >> INITIAL_SHIFT;

    // for odd input
    temp1 = (input[3] * C12 + RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp1 *= C8;
    intermediate[ 8] = (2 * (temp1) + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = (-input[3] * C4 + RIGHT_ROUNDING) >> RIGHT_SHIFT;
    temp1 *= C8;
    intermediate[ 9] = (2 * (temp1) + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    step[ 8] = (intermediate[ 8] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[ 9] = (intermediate[ 9] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[10] = (-input[1] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[11] = (input[1] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[12] = (input[1] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[13] = (input[1] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[14] = (intermediate[ 8] + INITIAL_ROUNDING) >> INITIAL_SHIFT;
    step[15] = (intermediate[ 9] + INITIAL_ROUNDING) >> INITIAL_SHIFT;

    // step 3
    output[0] = step[ 0];
    output[1] = step[ 1];
    output[2] = step[ 1];
    output[3] = step[ 0];

    temp1 = step[ 4] * C14;
    output[4] =  (temp1 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = step[ 4] * C2;
    output[7] =  (temp1 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = step[ 5] * C10;
    output[5] =  (temp1 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = step[ 5] * C6;
    output[6] =  (temp1 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    output[8] = step[ 8] + step[11];
    output[9] = step[ 9] + step[10];
    output[10] = step[ 9] - step[10];
    output[11] = step[ 8] - step[11];
    output[12] = step[12] + step[15];
    output[13] = step[13] + step[14];
    output[14] = step[13] - step[14];
    output[15] = step[12] - step[15];

    // output 4
    step[ 0] = output[0] + output[7];
    step[ 1] = output[1] + output[6];
    step[ 2] = output[2] + output[5];
    step[ 3] = output[3] + output[4];
    step[ 4] = output[3] - output[4];
    step[ 5] = output[2] - output[5];
    step[ 6] = output[1] - output[6];
    step[ 7] = output[0] - output[7];

    temp1 = output[8] * C7;
    temp2 = output[15] * C9;
    step[ 8] = (temp1 - temp2 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[9] * C11;
    temp2 = output[14] * C5;
    step[ 9] = (temp1 + temp2 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[10] * C3;
    temp2 = output[13] * C13;
    step[10] = (temp1 - temp2 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[11] * C15;
    temp2 = output[12] * C1;
    step[11] = (temp1 + temp2 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[11] * C1;
    temp2 = output[12] * C15;
    step[12] = (temp2 - temp1 + RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[10] * C13;
    temp2 = output[13] * C3;
    step[13] = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[9] * C5;
    temp2 = output[14] * C11;
    step[14] = (temp2 - temp1 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    temp1 = output[8] * C9;
    temp2 = output[15] * C7;
    step[15] = (temp1 + temp2 +   RIGHT_ROUNDING) >> RIGHT_SHIFT;

    // step 5
    output[0] = (step[0] + step[15] + last_rounding) >> last_shift_bits;
    output[1] = (step[1] + step[14] + last_rounding) >> last_shift_bits;
    output[2] = (step[2] + step[13] + last_rounding) >> last_shift_bits;
    output[3] = (step[3] + step[12] + last_rounding) >> last_shift_bits;
    output[4] = (step[4] + step[11] + last_rounding) >> last_shift_bits;
    output[5] = (step[5] + step[10] + last_rounding) >> last_shift_bits;
    output[6] = (step[6] + step[ 9] + last_rounding) >> last_shift_bits;
    output[7] = (step[7] + step[ 8] + last_rounding) >> last_shift_bits;

    output[15] = (step[0] - step[15] + last_rounding) >> last_shift_bits;
    output[14] = (step[1] - step[14] + last_rounding) >> last_shift_bits;
    output[13] = (step[2] - step[13] + last_rounding) >> last_shift_bits;
    output[12] = (step[3] - step[12] + last_rounding) >> last_shift_bits;
    output[11] = (step[4] - step[11] + last_rounding) >> last_shift_bits;
    output[10] = (step[5] - step[10] + last_rounding) >> last_shift_bits;
    output[9] = (step[6] - step[ 9] + last_rounding) >> last_shift_bits;
    output[8] = (step[7] - step[ 8] + last_rounding) >> last_shift_bits;
}

void vp9_short_idct10_16x16_c(int16_t *input, int16_t *output, int pitch) {
    int16_t out[16 * 16];
    int16_t *outptr = &out[0];
    const int short_pitch = pitch >> 1;
    int i, j;
    int16_t temp_in[16], temp_out[16];

    /* First transform rows. Since all non-zero dct coefficients are in
     * upper-left 4x4 area, we only need to calculate first 4 rows here.
     */
    vpx_memset(out, 0, sizeof(out));
    for (i = 0; i < 4; ++i) {
      butterfly_16x16_idct10_1d(input, outptr, 0);
      input += short_pitch;
      outptr += 16;
    }

    // Then transform columns
    for (i = 0; i < 16; ++i) {
      for (j = 0; j < 16; ++j)
        temp_in[j] = out[j*16 + i];
      butterfly_16x16_idct10_1d(temp_in, temp_out, 3);
      for (j = 0; j < 16; ++j)
        output[j*16 + i] = temp_out[j];
    }
}
#undef INITIAL_SHIFT
#undef INITIAL_ROUNDING
#undef RIGHT_SHIFT
#undef RIGHT_ROUNDING
#endif

#if CONFIG_TX32X32
#define USE_DWT 0
#if USE_DWT == 0
#define DownshiftMultiplyBy2(x) x * 2
#define DownshiftMultiply(x) x
static void idct16(double *input, double *output, int stride) {
  static const double C1 = 0.995184726672197;
  static const double C2 = 0.98078528040323;
  static const double C3 = 0.956940335732209;
  static const double C4 = 0.923879532511287;
  static const double C5 = 0.881921264348355;
  static const double C6 = 0.831469612302545;
  static const double C7 = 0.773010453362737;
  static const double C8 = 0.707106781186548;
  static const double C9 = 0.634393284163646;
  static const double C10 = 0.555570233019602;
  static const double C11 = 0.471396736825998;
  static const double C12 = 0.38268343236509;
  static const double C13 = 0.290284677254462;
  static const double C14 = 0.195090322016128;
  static const double C15 = 0.098017140329561;
  
  double step[16];
  double intermediate[16];
  double temp1, temp2;
  
  // step 1 and 2
  step[ 0] = input[stride*0] + input[stride*8];
  step[ 1] = input[stride*0] - input[stride*8];
  
  temp1 = input[stride*4]*C12;
  temp2 = input[stride*12]*C4;
  
  temp1 -= temp2;
  temp1 = DownshiftMultiply(temp1);
  temp1 *= C8;
  
  step[ 2] = DownshiftMultiplyBy2(temp1);
  
  temp1 = input[stride*4]*C4;
  temp2 = input[stride*12]*C12;
  temp1 += temp2;
  temp1 = DownshiftMultiply(temp1);
  temp1 *= C8;
  step[ 3] = DownshiftMultiplyBy2(temp1);
  
  temp1 = input[stride*2]*C8;
  temp1 = DownshiftMultiplyBy2(temp1);
  temp2 = input[stride*6] + input[stride*10];
  
  step[ 4] = temp1 + temp2;
  step[ 5] = temp1 - temp2;
  
  temp1 = input[stride*14]*C8;
  temp1 = DownshiftMultiplyBy2(temp1);
  temp2 = input[stride*6] - input[stride*10];
  
  step[ 6] = temp2 - temp1;
  step[ 7] = temp2 + temp1;
  
  // for odd input
  temp1 = input[stride*3]*C12;
  temp2 = input[stride*13]*C4;
  temp1 += temp2;
  temp1 = DownshiftMultiply(temp1);
  temp1 *= C8;
  intermediate[ 8] = DownshiftMultiplyBy2(temp1);
  
  temp1 = input[stride*3]*C4;
  temp2 = input[stride*13]*C12;
  temp2 -= temp1;
  temp2 = DownshiftMultiply(temp2);
  temp2 *= C8;
  intermediate[ 9] = DownshiftMultiplyBy2(temp2);
  
  intermediate[10] = DownshiftMultiplyBy2(input[stride*9]*C8);
  intermediate[11] = input[stride*15] - input[stride*1];
  intermediate[12] = input[stride*15] + input[stride*1];
  intermediate[13] = DownshiftMultiplyBy2((input[stride*7]*C8));
  
  temp1 = input[stride*11]*C12;
  temp2 = input[stride*5]*C4;
  temp2 -= temp1;
  temp2 = DownshiftMultiply(temp2);
  temp2 *= C8;
  intermediate[14] = DownshiftMultiplyBy2(temp2);
  
  temp1 = input[stride*11]*C4;
  temp2 = input[stride*5]*C12;
  temp1 += temp2;
  temp1 = DownshiftMultiply(temp1);
  temp1 *= C8;
  intermediate[15] = DownshiftMultiplyBy2(temp1);
  
  step[ 8] = intermediate[ 8] + intermediate[14];
  step[ 9] = intermediate[ 9] + intermediate[15];
  step[10] = intermediate[10] + intermediate[11];
  step[11] = intermediate[10] - intermediate[11];
  step[12] = intermediate[12] + intermediate[13];
  step[13] = intermediate[12] - intermediate[13];
  step[14] = intermediate[ 8] - intermediate[14];
  step[15] = intermediate[ 9] - intermediate[15];
  
  // step 3
  output[stride*0] = step[ 0] + step[ 3];
  output[stride*1] = step[ 1] + step[ 2];
  output[stride*2] = step[ 1] - step[ 2];
  output[stride*3] = step[ 0] - step[ 3];
  
  temp1 = step[ 4]*C14;
  temp2 = step[ 7]*C2;
  temp1 -= temp2;
  output[stride*4] =  DownshiftMultiply(temp1);
  
  temp1 = step[ 4]*C2;
  temp2 = step[ 7]*C14;
  temp1 += temp2;
  output[stride*7] =  DownshiftMultiply(temp1);
  
  temp1 = step[ 5]*C10;
  temp2 = step[ 6]*C6;
  temp1 -= temp2;
  output[stride*5] =  DownshiftMultiply(temp1);
  
  temp1 = step[ 5]*C6;
  temp2 = step[ 6]*C10;
  temp1 += temp2;
  output[stride*6] =  DownshiftMultiply(temp1);
  
  output[stride*8] = step[ 8] + step[11];
  output[stride*9] = step[ 9] + step[10];
  output[stride*10] = step[ 9] - step[10];
  output[stride*11] = step[ 8] - step[11];
  output[stride*12] = step[12] + step[15];
  output[stride*13] = step[13] + step[14];
  output[stride*14] = step[13] - step[14];
  output[stride*15] = step[12] - step[15];
  
  // output 4
  step[ 0] = output[stride*0] + output[stride*7];
  step[ 1] = output[stride*1] + output[stride*6];
  step[ 2] = output[stride*2] + output[stride*5];
  step[ 3] = output[stride*3] + output[stride*4];
  step[ 4] = output[stride*3] - output[stride*4];
  step[ 5] = output[stride*2] - output[stride*5];
  step[ 6] = output[stride*1] - output[stride*6];
  step[ 7] = output[stride*0] - output[stride*7];
  
  temp1 = output[stride*8]*C7;
  temp2 = output[stride*15]*C9;
  temp1 -= temp2;
  step[ 8] = DownshiftMultiply(temp1);
  
  temp1 = output[stride*9]*C11;
  temp2 = output[stride*14]*C5;
  temp1 += temp2;
  step[ 9] = DownshiftMultiply(temp1);
  
  temp1 = output[stride*10]*C3;
  temp2 = output[stride*13]*C13;
  temp1 -= temp2;
  step[10] = DownshiftMultiply(temp1);
  
  temp1 = output[stride*11]*C15;
  temp2 = output[stride*12]*C1;
  temp1 += temp2;
  step[11] = DownshiftMultiply(temp1);
  
  temp1 = output[stride*11]*C1;
  temp2 = output[stride*12]*C15;
  temp2 -= temp1;
  step[12] = DownshiftMultiply(temp2);
  
  temp1 = output[stride*10]*C13;
  temp2 = output[stride*13]*C3;
  temp1 += temp2;
  step[13] = DownshiftMultiply(temp1);
  
  temp1 = output[stride*9]*C5;
  temp2 = output[stride*14]*C11;
  temp2 -= temp1;
  step[14] = DownshiftMultiply(temp2);
  
  temp1 = output[stride*8]*C9;
  temp2 = output[stride*15]*C7;
  temp1 += temp2;
  step[15] = DownshiftMultiply(temp1);
  
  // step 5
  output[stride*0] = step[0] + step[15];
  output[stride*1] = step[1] + step[14];
  output[stride*2] = step[2] + step[13];
  output[stride*3] = step[3] + step[12];
  output[stride*4] = step[4] + step[11];
  output[stride*5] = step[5] + step[10];
  output[stride*6] = step[6] + step[ 9];
  output[stride*7] = step[7] + step[ 8];
  
  output[stride*15] = step[0] - step[15];
  output[stride*14] = step[1] - step[14];
  output[stride*13] = step[2] - step[13];
  output[stride*12] = step[3] - step[12];
  output[stride*11] = step[4] - step[11];
  output[stride*10] = step[5] - step[10];
  output[stride*9] = step[6] - step[ 9];
  output[stride*8] = step[7] - step[ 8];
}
static void butterfly_32_idct_1d(double *input, double *output, int stride) {
  static const double C1 = 0.998795456205; // cos(pi * 1 / 64)
  static const double C2 = 0.995184726672; // cos(pi * 2 / 64)
  static const double C3 = 0.989176509965; // cos(pi * 3 / 64)
  static const double C4 = 0.980785280403; // cos(pi * 4 / 64)
  static const double C5 = 0.970031253195; // cos(pi * 5 / 64)
  static const double C6 = 0.956940335732; // cos(pi * 6 / 64)
  static const double C7 = 0.941544065183; // cos(pi * 7 / 64)
  static const double C8 = 0.923879532511; // cos(pi * 8 / 64)
  static const double C9 = 0.903989293123; // cos(pi * 9 / 64)
  static const double C10 = 0.881921264348; // cos(pi * 10 / 64)
  static const double C11 = 0.857728610000; // cos(pi * 11 / 64)
  static const double C12 = 0.831469612303; // cos(pi * 12 / 64)
  static const double C13 = 0.803207531481; // cos(pi * 13 / 64)
  static const double C14 = 0.773010453363; // cos(pi * 14 / 64)
  static const double C15 = 0.740951125355; // cos(pi * 15 / 64)
  static const double C16 = 0.707106781187; // cos(pi * 16 / 64)
  static const double C17 = 0.671558954847; // cos(pi * 17 / 64)
  static const double C18 = 0.634393284164; // cos(pi * 18 / 64)
  static const double C19 = 0.595699304492; // cos(pi * 19 / 64)
  static const double C20 = 0.555570233020; // cos(pi * 20 / 64)
  static const double C21 = 0.514102744193; // cos(pi * 21 / 64)
  static const double C22 = 0.471396736826; // cos(pi * 22 / 64)
  static const double C23 = 0.427555093430; // cos(pi * 23 / 64)
  static const double C24 = 0.382683432365; // cos(pi * 24 / 64)
  static const double C25 = 0.336889853392; // cos(pi * 25 / 64)
  static const double C26 = 0.290284677254; // cos(pi * 26 / 64)
  static const double C27 = 0.242980179903; // cos(pi * 27 / 64)
  static const double C28 = 0.195090322016; // cos(pi * 28 / 64)
  static const double C29 = 0.146730474455; // cos(pi * 29 / 64)
  static const double C30 = 0.098017140330; // cos(pi * 30 / 64)
  static const double C31 = 0.049067674327; // cos(pi * 31 / 64)
  
  double step1[32];
  double step2[32];
  
  step1[ 0] = input[stride*0];
  step1[ 1] = input[stride*2];
  step1[ 2] = input[stride*4];
  step1[ 3] = input[stride*6];
  step1[ 4] = input[stride*8];
  step1[ 5] = input[stride*10];
  step1[ 6] = input[stride*12];
  step1[ 7] = input[stride*14];
  step1[ 8] = input[stride*16];
  step1[ 9] = input[stride*18];
  step1[10] = input[stride*20];
  step1[11] = input[stride*22];
  step1[12] = input[stride*24];
  step1[13] = input[stride*26];
  step1[14] = input[stride*28];
  step1[15] = input[stride*30];
  
  step1[16] = DownshiftMultiplyBy2(input[stride*1]*C16);
  step1[17] = (input[stride*3] + input[stride*1]);
  step1[18] = (input[stride*5] + input[stride*3]);
  step1[19] = (input[stride*7] + input[stride*5]);
  step1[20] = (input[stride*9] + input[stride*7]);
  step1[21] = (input[stride*11] + input[stride*9]);
  step1[22] = (input[stride*13] + input[stride*11]);
  step1[23] = (input[stride*15] + input[stride*13]);
  step1[24] = (input[stride*17] + input[stride*15]);
  step1[25] = (input[stride*19] + input[stride*17]);
  step1[26] = (input[stride*21] + input[stride*19]);
  step1[27] = (input[stride*23] + input[stride*21]);
  step1[28] = (input[stride*25] + input[stride*23]);
  step1[29] = (input[stride*27] + input[stride*25]);
  step1[30] = (input[stride*29] + input[stride*27]);
  step1[31] = (input[stride*31] + input[stride*29]);
  
  idct16(step1, step2, 1);
  idct16(step1 + 16, step2 + 16, 1);
  
  step2[16] = DownshiftMultiply(step2[16] / (2*C1));
  step2[17] = DownshiftMultiply(step2[17] / (2*C3));
  step2[18] = DownshiftMultiply(step2[18] / (2*C5));
  step2[19] = DownshiftMultiply(step2[19] / (2*C7));
  step2[20] = DownshiftMultiply(step2[20] / (2*C9));
  step2[21] = DownshiftMultiply(step2[21] / (2*C11));
  step2[22] = DownshiftMultiply(step2[22] / (2*C13));
  step2[23] = DownshiftMultiply(step2[23] / (2*C15));
  step2[24] = DownshiftMultiply(step2[24] / (2*C17));
  step2[25] = DownshiftMultiply(step2[25] / (2*C19));
  step2[26] = DownshiftMultiply(step2[26] / (2*C21));
  step2[27] = DownshiftMultiply(step2[27] / (2*C23));
  step2[28] = DownshiftMultiply(step2[28] / (2*C25));
  step2[29] = DownshiftMultiply(step2[29] / (2*C27));
  step2[30] = DownshiftMultiply(step2[30] / (2*C29));
  step2[31] = DownshiftMultiply(step2[31] / (2*C31));
  
  output[stride* 0] = step2[ 0] + step2[16];
  output[stride* 1] = step2[ 1] + step2[17];
  output[stride* 2] = step2[ 2] + step2[18];
  output[stride* 3] = step2[ 3] + step2[19];
  output[stride* 4] = step2[ 4] + step2[20];
  output[stride* 5] = step2[ 5] + step2[21];
  output[stride* 6] = step2[ 6] + step2[22];
  output[stride* 7] = step2[ 7] + step2[23];
  output[stride* 8] = step2[ 8] + step2[24];
  output[stride* 9] = step2[ 9] + step2[25];
  output[stride*10] = step2[10] + step2[26];
  output[stride*11] = step2[11] + step2[27];
  output[stride*12] = step2[12] + step2[28];
  output[stride*13] = step2[13] + step2[29];
  output[stride*14] = step2[14] + step2[30];
  output[stride*15] = step2[15] + step2[31];
  output[stride*16] = step2[15] - step2[(31 - 0)];
  output[stride*17] = step2[14] - step2[(31 - 1)];
  output[stride*18] = step2[13] - step2[(31 - 2)];
  output[stride*19] = step2[12] - step2[(31 - 3)];
  output[stride*20] = step2[11] - step2[(31 - 4)];
  output[stride*21] = step2[10] - step2[(31 - 5)];
  output[stride*22] = step2[ 9] - step2[(31 - 6)];
  output[stride*23] = step2[ 8] - step2[(31 - 7)];
  output[stride*24] = step2[ 7] - step2[(31 - 8)];
  output[stride*25] = step2[ 6] - step2[(31 - 9)];
  output[stride*26] = step2[ 5] - step2[(31 - 10)];
  output[stride*27] = step2[ 4] - step2[(31 - 11)];
  output[stride*28] = step2[ 3] - step2[(31 - 12)];
  output[stride*29] = step2[ 2] - step2[(31 - 13)];
  output[stride*30] = step2[ 1] - step2[(31 - 14)];
  output[stride*31] = step2[ 0] - step2[(31 - 15)];
}

void vp9_short_idct32x32_c(short *input, short *output, int pitch) {
  vp9_clear_system_state(); // Make it simd safe : __asm emms;
  {
    double out[32*32], out2[32*32];
    const int short_pitch = pitch >> 1;
    int i, j;
    // First transform rows
    for (i = 0; i < 32; ++i) {
      double temp_in[32], temp_out[32];
      for (j = 0; j < 32; ++j)
        temp_in[j] = input[j + i*short_pitch];
      butterfly_32_idct_1d(temp_in, temp_out, 1);
      for (j = 0; j < 32; ++j)
        out[j + i*32] = temp_out[j];
    }
    // Then transform columns
    for (i = 0; i < 32; ++i) {
      double temp_in[32], temp_out[32];
      for (j = 0; j < 32; ++j)
        temp_in[j] = out[j*32 + i];
      butterfly_32_idct_1d(temp_in, temp_out, 1);
      for (j = 0; j < 32; ++j)
        out2[j*32 + i] = temp_out[j];
    }
    for (i = 0; i < 32*32; ++i)
      output[i] = round(out2[i]/128);
  }
  vp9_clear_system_state(); // Make it simd safe : __asm emms;
}
#else // USE_DWT

#define MAX_BLOCK_LENGTH   64
#define ENH_PRECISION_BITS 1
#define ENH_PRECISION_RND ((1 << ENH_PRECISION_BITS) / 2)

// Note: block length must be even for this implementation
static void synthesis_53_row(int length, short *lowpass, short *highpass,
                             short *x) {
  short r, * a, * b;
  int n;
  
  n = length >> 1;
  b = highpass;
  a = lowpass;
  r = *highpass;
  while (n--) {
    *a++ -= (r + (*b) + 1) >> 1;
    r = *b++;
  }
  
  n = length >> 1;
  b = highpass;
  a = lowpass;
  while (--n) {
    *x++ = ((r = *a++) + 1) >> 1;
    *x++ = *b++ + ((r + (*a) + 2) >> 2);
  }
  *x++ = ((r = *a) + 1)>>1;
  *x++ = *b + ((r+1)>>1);
}

static void synthesis_53_col(int length, short *lowpass, short *highpass,
                             short *x) {
  short r, * a, * b;
  int n;
  
  n = length >> 1;
  b = highpass;
  a = lowpass;
  r = *highpass;
  while (n--) {
    *a++ -= (r + (*b) + 1) >> 1;
    r = *b++;
  }
  
  n = length >> 1;
  b = highpass;
  a = lowpass;
  while (--n) {
    *x++ = r = *a++;
    *x++ = ((*b++) << 1) + ((r + (*a) + 1) >> 1);
  }
  *x++ = r = *a;
  *x++ = ((*b) << 1) + r;
}

// NOTE: Using a 5/3 integer wavelet for now. Explore using a wavelet
// with a better response later
void dyadic_synthesize(int levels, int width, int height, short *c, int pitch_c,
                       short *x, int pitch_x) {
  int th[16], tw[16], lv, i, j, nh, nw, hh = height, hw = width;
  short buffer[2 * MAX_BLOCK_LENGTH];
  
  th[0] = hh;
  tw[0] = hw;
  for (i = 1; i <= levels; i++) {
    th[i] = (th[i - 1] + 1) >> 1;
    tw[i] = (tw[i - 1] + 1) >> 1;
  }
  for (lv = levels - 1; lv >= 0; lv--) {
    nh = th[lv];
    nw = tw[lv];
    hh = th[lv + 1];
    hw = tw[lv + 1];
    if ((nh < 2) || (nw < 2)) continue;
    for (j = 0; j < nw; j++) {
      for (i = 0; i < nh; i++)
        buffer[i] = c[i * pitch_c + j];
      synthesis_53_col(nh, buffer, buffer + hh, buffer + nh);
      for (i = 0; i < nh; i++)
        c[i * pitch_c + j] = buffer[i + nh];
    }
    for (i = 0; i < nh; i++) {
      memcpy(buffer, &c[i * pitch_c], nw * sizeof(short));
      synthesis_53_row(nw, buffer, buffer + hw, &c[i * pitch_c]);
    }
  }
  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++)
      x[i * pitch_x + j] = (c[i * pitch_c + j] + ENH_PRECISION_RND) >>
      ENH_PRECISION_BITS;
}

void vp9_short_idct32x32_c(short *input, short *output, int pitch) {
  // assume out is a 32x32 buffer
  short buffer[16 * 16];
  short buffer2[32 * 32];
  const int short_pitch = pitch >> 1;
  int i;
  // TODO(debargha): Implement more efficiently by adding output pitch
  // argument to the idct16x16 function
  vp9_short_idct16x16_c(input, buffer, pitch);
  for (i = 0; i < 16; ++i) {
    vpx_memcpy(buffer2 + i * 32, buffer + i * 16, sizeof(short) * 16);
    vpx_memcpy(buffer2 + i * 32 + 16, input + i * short_pitch + 16,
               sizeof(short) * 16);
  }
  for (; i < 32; ++i) {
    vpx_memcpy(buffer2 + i * 32, input + i * short_pitch,
               sizeof(short) * 32);
  }
  dyadic_synthesize(1, 32, 32, buffer2, 32, output, 32);
}
#endif // USE_DWT
#endif // CONFIG_TX32X32
