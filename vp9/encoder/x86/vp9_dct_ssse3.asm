;
;  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;
%include "third_party/x86inc/x86inc.asm"

; This file provides SSSE3 version of the forward transformation. Part
; of the macro definitions are originally derived from the ffmpeg project.
; The current version applies to x86 64-bit only.

SECTION_RODATA

pw_11585x2: times 8 dw 23170
pd_8192:    times 4 dd 8192
pw_1:       times 8 dw 1

%macro TRANSFORM_COEFFS 2
pw_%1_%2:   dw  %1,  %2,  %1,  %2,  %1,  %2,  %1,  %2
pw_%2_m%1:  dw  %2, -%1,  %2, -%1,  %2, -%1,  %2, -%1
%endmacro

TRANSFORM_COEFFS 15137,   6270
TRANSFORM_COEFFS  6270,  15137
TRANSFORM_COEFFS 16069,   3196
TRANSFORM_COEFFS  9102,  13623
TRANSFORM_COEFFS 16305,   1606
TRANSFORM_COEFFS 10394,  12665
TRANSFORM_COEFFS 14449,   7723
TRANSFORM_COEFFS  4756,  15679

SECTION .text

%if ARCH_X86_64
%macro SUM_SUB 3
  psubw  m%3, m%1, m%2
  paddw  m%1, m%2
  SWAP    %2, %3
%endmacro

; butterfly operation
%macro MUL_ADD_2X 6 ; dst1, dst2, src, round, coefs1, coefs2
  pmaddwd            m%1, m%3, %5
  pmaddwd            m%2, m%3, %6
  paddd              m%1,  %4
  paddd              m%2,  %4
  psrad              m%1,  14
  psrad              m%2,  14
%endmacro

%macro BUTTERFLY_4X 7 ; dst1, dst2, coef1, coef2, round, tmp1, tmp2
  punpckhwd          m%6, m%2, m%1
  MUL_ADD_2X         %7,  %6,  %6,  %5, [pw_%4_%3], [pw_%3_m%4]
  punpcklwd          m%2, m%1
  MUL_ADD_2X         %1,  %2,  %2,  %5, [pw_%4_%3], [pw_%3_m%4]
  packssdw           m%1, m%7
  packssdw           m%2, m%6
%endmacro

; matrix transpose
%macro INTERLEAVE_2X 4
  punpckh%1          m%4, m%2, m%3
  punpckl%1          m%2, m%3
  SWAP               %3,  %4
%endmacro

%macro TRANSPOSE8X8 9
  INTERLEAVE_2X  wd, %1, %2, %9
  INTERLEAVE_2X  wd, %3, %4, %9
  INTERLEAVE_2X  wd, %5, %6, %9
  INTERLEAVE_2X  wd, %7, %8, %9

  INTERLEAVE_2X  dq, %1, %3, %9
  INTERLEAVE_2X  dq, %2, %4, %9
  INTERLEAVE_2X  dq, %5, %7, %9
  INTERLEAVE_2X  dq, %6, %8, %9

  INTERLEAVE_2X  qdq, %1, %5, %9
  INTERLEAVE_2X  qdq, %3, %7, %9
  INTERLEAVE_2X  qdq, %2, %6, %9
  INTERLEAVE_2X  qdq, %4, %8, %9

  SWAP  %2, %5
  SWAP  %4, %7
%endmacro

; 1D forward 8x8 DCT transform
%macro FDCT8_1D 0
  SUM_SUB            0,  7,  9
  SUM_SUB            1,  6,  9
  SUM_SUB            2,  5,  9
  SUM_SUB            3,  4,  9

  SUM_SUB            0,  3,  9
  SUM_SUB            1,  2,  9
  SUM_SUB            6,  5,  9
  SUM_SUB            0,  1,  9

  BUTTERFLY_4X       2,  3,  6270,  15137,  m8,  9,  10

  pmulhrsw           m6, m12
  pmulhrsw           m5, m12
  pmulhrsw           m0, m12
  pmulhrsw           m1, m12

  SUM_SUB            4,  5,  9
  SUM_SUB            7,  6,  9
  BUTTERFLY_4X       4,  7,  3196,  16069,  m8,  9,  10
  BUTTERFLY_4X       5,  6,  13623,  9102,  m8,  9,  10
  SWAP               1,  4
  SWAP               3,  6
%endmacro

%macro DIVIDE_ROUND_2X 4 ; dst1, dst2, tmp1, tmp2
  psraw              m%3, m%1, 15
  psraw              m%4, m%2, 15
  psubw              m%1, m%3
  psubw              m%2, m%4
  psraw              m%1, 1
  psraw              m%2, 1
%endmacro

INIT_XMM ssse3
cglobal fdct8x8, 3, 5, 13, input, output, stride

  mova               m8, [pd_8192]
  mova              m12, [pw_11585x2]
  pxor              m11, m11

  lea                r3, [2 * strideq]
  lea                r4, [4 * strideq]
  mova               m0, [inputq]
  mova               m1, [inputq + r3]
  lea                inputq, [inputq + r4]
  mova               m2, [inputq]
  mova               m3, [inputq + r3]
  lea                inputq, [inputq + r4]
  mova               m4, [inputq]
  mova               m5, [inputq + r3]
  lea                inputq, [inputq + r4]
  mova               m6, [inputq]
  mova               m7, [inputq + r3]

  ; left shift by 2 to increase forward transformation precision
  psllw              m0, 2
  psllw              m1, 2
  psllw              m2, 2
  psllw              m3, 2
  psllw              m4, 2
  psllw              m5, 2
  psllw              m6, 2
  psllw              m7, 2

  ; column transform
  FDCT8_1D
  TRANSPOSE8X8 0, 1, 2, 3, 4, 5, 6, 7, 9

  FDCT8_1D
  TRANSPOSE8X8 0, 1, 2, 3, 4, 5, 6, 7, 9

  DIVIDE_ROUND_2X   0, 1, 9, 10
  DIVIDE_ROUND_2X   2, 3, 9, 10
  DIVIDE_ROUND_2X   4, 5, 9, 10
  DIVIDE_ROUND_2X   6, 7, 9, 10

  mova              [outputq +   0], m0
  mova              [outputq +  16], m1
  mova              [outputq +  32], m2
  mova              [outputq +  48], m3
  mova              [outputq +  64], m4
  mova              [outputq +  80], m5
  mova              [outputq +  96], m6
  mova              [outputq + 112], m7

  RET

; handle 8 columns
%macro FDCT16_1D 2 ; inputq, index
  mova               m0,  [%1]
  mova               m1,  [%1 + r3]
  lea                %1,  [%1 + r4]
  mova               m2,  [%1]
  mova               m3,  [%1 + r3]
  lea                %1,  [%1 + r4]
  mova               m4,  [%1]
  mova               m5,  [%1 + r3]
  lea                %1,  [%1 + r4]
  mova               m6,  [%1]
  mova               m7,  [%1 + r3]
  lea                %1,  [%1 + r4]
  mova               m8,  [%1]
  mova               m9,  [%1 + r3]
  lea                %1,  [%1 + r4]
  mova               m10, [%1]
  mova               m11, [%1 + r3]
  lea                %1,  [%1 + r4]
  mova               m12, [%1]
  mova               m13, [%1 + r3]

%if %2 == 0 ; multiply by 4 in the first pass
  psllw              m0,  2
  psllw              m1,  2
  psllw              m2,  2
  psllw              m3,  2
  psllw              m4,  2
  psllw              m5,  2
  psllw              m6,  2
  psllw              m7,  2
  psllw              m8,  2
  psllw              m9,  2
  psllw              m10, 2
  psllw              m11, 2
  psllw              m12, 2
  psllw              m13, 2
%else ; %2 == 1, (input + 1) >> 2
  mova               m14, [pw_1]
  paddw              m0,  m14
  paddw              m1,  m14
  paddw              m2,  m14
  paddw              m3,  m14
  paddw              m4,  m14
  paddw              m5,  m14
  paddw              m6,  m14
  paddw              m7,  m14
  paddw              m8,  m14
  paddw              m9,  m14
  paddw              m10, m14
  paddw              m11, m14
  paddw              m12, m14
  paddw              m13, m14

  psraw              m0,  2
  psraw              m1,  2
  psraw              m2,  2
  psraw              m3,  2
  psraw              m4,  2
  psraw              m5,  2
  psraw              m6,  2
  psraw              m7,  2
  psraw              m8,  2
  psraw              m9,  2
  psraw              m10, 2
  psraw              m11, 2
  psraw              m12, 2
  psraw              m13, 2
%endif

  SUM_SUB            2, 13, 15
  SUM_SUB            3, 12, 15
  SUM_SUB            4, 11, 15
  SUM_SUB            5, 10, 15
  SUM_SUB            6,  9, 15
  SUM_SUB            7,  8, 15

  ; step2[2], [3], [4], [5]
  ;      m10, m11, m12, m13
  SUM_SUB           12, 11, 15
  SUM_SUB           13, 10, 15

  mova              m14, [pw_11585x2]
  pmulhrsw          m12, m14
  pmulhrsw          m11, m14
  pmulhrsw          m13, m14
  pmulhrsw          m10, m14

  ; step2[4],        [5]
  ;      [rsp + 32], [rsp + 64]
  mova              [rsp + 32], m12
  mova              [rsp + 64], m13

  lea               %1,  [%1 + r4]
  mova              m12, [%1]
  mova              m13, [%1 + r3]

%if %2 == 0 ; multiply by 4 in the first pass
  psllw             m12, 2
  psllw             m13, 2
%else ; %2 == 1, (input + 1) >> 2
  paddw             m12, [pw_1]
  paddw             m13, [pw_1]
  psraw             m12, 2
  psraw             m13, 2
%endif
  SUM_SUB           0, 13, 15
  SUM_SUB           1, 12, 15
  ; input[0], [1], [2], [3], [4], [5], [6], [7]
  ;       m0,  m1,  m2,  m3,  m4,  m5,  m6,  m7

  SUM_SUB           0, 7, 15
  SUM_SUB           1, 6, 15
  SUM_SUB           2, 5, 15
  SUM_SUB           3, 4, 15
  ; s0, s1, s2, s3, s4, s5, s6, s7
  ; m0, m1, m2, m3, m4, m5, m6, m7

  SUM_SUB           0, 3, 15
  SUM_SUB           1, 2, 15

  SUM_SUB           0, 1, 15
  SUM_SUB           6, 5, 15
  pmulhrsw          m0, m14
  pmulhrsw          m1, m14
  pmulhrsw          m6, m14
  pmulhrsw          m5, m14
  mova              [rsp], m0
  ; out[0] -> [rsp]
  ; out[8] -> m1
  ; t2, t3
  ; m5, m6

  mova              m0, [pd_8192]
  BUTTERFLY_4X      2, 3, 6270, 15137, m0, 14, 15
  ; out[4], out[12]
  ;    m2,     m3

  ; MOVE next two instructions 2 steps above
  SUM_SUB           4, 5, 15
  SUM_SUB           7, 6, 15
  BUTTERFLY_4X      4, 7,  3196, 16069, m0, 14, 15
  BUTTERFLY_4X      5, 6, 13623,  9102, m0, 14, 15
  mova              [rsp +  96], m4
  mova              [rsp + 128], m5
  ; out[2],        out[10]
  ;    [rsp + 96],    [rsp + 128]
  ; out[6], [14]
  ;     m6,  m7

  mova             m4, [rsp + 32]
  mova             m5, [rsp + 64]
  ; step2[2], [3], [4], [5]
  ;      m10, m11,  m4,  m5

  SUM_SUB           8, 11, 15
  SUM_SUB           9, 10, 15
  SUM_SUB          12,  5, 15
  SUM_SUB          13,  4, 15
  ; step3[0], [1], [2], [3], [4], [5], [6], [7]
  ;       m8,  m9, m10, m11,  m4,  m5, m12, m13

  BUTTERFLY_4X      9, 12,  6270, 15137, m0, 14, 15
  BUTTERFLY_4X      5, 10, 15137,  6270, m0, 14, 15

  SUM_SUB           8, 12, 15
  SUM_SUB          11,  5, 15
  SUM_SUB           4, 10, 15
  SUM_SUB          13,  9, 15

  ; step1[0], [1], [2], [3], [4], [5], [6], [7]
  ;       m8, m12, m11,  m5, m10,  m4,  m9, m13
  BUTTERFLY_4X      8, 13,  1606, 16305, m0, 14, 15
  BUTTERFLY_4X     12,  9, 12665, 10394, m0, 14, 15
  BUTTERFLY_4X     11,  4,  7723, 14449, m0, 14, 15
  BUTTERFLY_4X      5, 10, 15679,  4756, m0, 14, 15

  mova              m0, [rsp]
  mova             m14, [rsp + 96]
  ; transpose
  TRANSPOSE8X8      0, 8, 14, 10, 2, 11, 6, 9, 15
%if %2 == 0
  mova              [rsp],       m0
  mova              [rsp +  32], m8
  mova              [rsp +  64], m14
  mova              [rsp +  96], m10
  mova              m14, [rsp + 128]
  mova              [rsp + 128], m2
  mova              [rsp + 160], m11
  mova              [rsp + 192], m6
  mova              [rsp + 224], m9
%else ; %2 == 1
  mova              [outputq],       m0
  mova              [outputq +  32], m8
  mova              [outputq +  64], m14
  mova              [outputq +  96], m10
  mova              [outputq + 128], m2
  mova              [outputq + 160], m11
  mova              [outputq + 192], m6
  mova              [outputq + 224], m9
  mova              m14, [rsp + 128]
%endif

  TRANSPOSE8X8      1, 12, 14, 4, 3, 5, 7, 13, 15
%if %2 == 0
  mova              [rsp +  16], m1
  mova              [rsp +  48], m12
  mova              [rsp +  80], m14
  mova              [rsp + 112], m4
  mova              [rsp + 144], m3
  mova              [rsp + 176], m5
  mova              [rsp + 208], m7
  mova              [rsp + 240], m13
%else ; %2 == 1
  mova              [outputq +  16], m1
  mova              [outputq +  48], m12
  mova              [outputq +  80], m14
  mova              [outputq + 112], m4
  mova              [outputq + 144], m3
  mova              [outputq + 176], m5
  mova              [outputq + 208], m7
  mova              [outputq + 240], m13
%endif

%endmacro

INIT_XMM ssse3
cglobal fdct16x16, 3, 7, 16, input, output, stride
  ALLOC_STACK 512, 7

  mov                r5, inputq
  lea                r3, [2 * strideq]
  lea                r4, [4 * strideq]

  FDCT16_1D          inputq,  0
  mov                inputq,  r5
  add                inputq,  16
  add                rsp,     256
  FDCT16_1D          inputq,  0

  sub                rsp,     256
  lea                r3,      [32]
  lea                r4,      [64]
  mov                r5,      rsp
  mov                inputq,  r5
  FDCT16_1D          inputq,  1
  mov                inputq,  r5
  add                inputq,  16
  add                outputq, 256
  FDCT16_1D          inputq,  1

  RESTORE_STACK
  RET
%endif
