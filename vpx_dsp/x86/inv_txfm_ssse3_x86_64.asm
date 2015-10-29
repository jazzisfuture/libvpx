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

; This file provides SSSE3 version of the inverse transformation. Part
; of the functions are originally derived from the ffmpeg project.
; Note that the current version applies to x86 64-bit only.

SECTION_RODATA

pw_11585x2: times 8 dw 23170

pw_m2404x2: times 8 dw  -2404*2
pw_m4756x2: times 8 dw  -4756*2
pw_m5520x2: times 8 dw  -5520*2

pw_16364x2: times 8 dw 16364*2
pw_16207x2: times 8 dw 16207*2
pw_16069x2: times 8 dw 16069*2
pw_16305x2: times 8 dw 16305*2
pw_15893x2: times 8 dw 15893*2
pw_15679x2: times 8 dw 15679*2
pw_15426x2: times 8 dw 15426*2
pw__3981x2: times 8 dw  3981*2
pw__3196x2: times 8 dw  3196*2
pw__1606x2: times 8 dw  1606*2
pw___804x2: times 8 dw   804*2



pd_8192:    times 4 dd 8192
pw_512:     times 8 dw 512
pw_32:      times 8 dw 32
pw_16:      times 8 dw 16

%macro TRANSFORM_COEFFS 2
pw_%1_%2:   dw  %1,  %2,  %1,  %2,  %1,  %2,  %1,  %2
pw_m%2_%1:  dw -%2,  %1, -%2,  %1, -%2,  %1, -%2,  %1
;
pw_m%1_m%2: dw -%1, -%2, -%1, -%2, -%1, -%2, -%1, -%2
%endmacro

TRANSFORM_COEFFS    6270, 15137
TRANSFORM_COEFFS    3196, 16069
TRANSFORM_COEFFS   13623,  9102

; constants for 32x32_34
;TRANSFORM_COEFFS      804, 16364
;TRANSFORM_COEFFS    15426,  5520
;TRANSFORM_COEFFS     3981, 15893
;TRANSFORM_COEFFS    16207,  2404
;TRANSFORM_COEFFS     1606, 16305
;TRANSFORM_COEFFS    15679,  4756
;TRANSFORM_COEFFS    11585, 11585

%macro PAIR_PP_COEFFS 2
dpw_%1_%2:   dw  %1,  %1,  %1,  %1,  %2,  %2,  %2,  %2
%endmacro

%macro PAIR_MP_COEFFS 2
dpw_m%1_%2:  dw -%1, -%1, -%1, -%1,  %2,  %2,  %2,  %2
%endmacro

%macro PAIR_MM_COEFFS 2
dpw_m%1_m%2: dw -%1, -%1, -%1, -%1, -%2, -%2, -%2, -%2
%endmacro

PAIR_PP_COEFFS     30274, 12540
PAIR_PP_COEFFS      6392, 32138
PAIR_MP_COEFFS     18204, 27246

PAIR_PP_COEFFS     12540, 12540
PAIR_PP_COEFFS     30274, 30274
PAIR_PP_COEFFS      6392,  6392
PAIR_PP_COEFFS     32138, 32138
PAIR_MM_COEFFS     18204, 18204
PAIR_PP_COEFFS     27246, 27246

; constants for 32x32_34
PAIR_PP_COEFFS     11585, 11585
PAIR_MP_COEFFS     11585, 11585

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
  MUL_ADD_2X         %7,  %6,  %6,  %5, [pw_m%4_%3], [pw_%3_%4]
  punpcklwd          m%2, m%1
  MUL_ADD_2X         %1,  %2,  %2,  %5, [pw_m%4_%3], [pw_%3_%4]
  packssdw           m%1, m%7
  packssdw           m%2, m%6
%endmacro

%macro BUTTERFLY_4Xmm 7 ; dst1, dst2, coef1, coef2, round, tmp1, tmp2
  punpckhwd          m%6, m%2, m%1
  MUL_ADD_2X         %7,  %6,  %6,  %5, [pw_m%4_%3], [pw_m%3_m%4]
  punpcklwd          m%2, m%1
  MUL_ADD_2X         %1,  %2,  %2,  %5, [pw_m%4_%3], [pw_m%3_m%4]
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

%macro IDCT8_1D 0
  SUM_SUB          0,    4,    9
  BUTTERFLY_4X     2,    6,    6270, 15137,  m8,  9,  10
  pmulhrsw        m0,  m12
  pmulhrsw        m4,  m12
  BUTTERFLY_4X     1,    7,    3196, 16069,  m8,  9,  10
  BUTTERFLY_4X     5,    3,   13623,  9102,  m8,  9,  10

  SUM_SUB          1,    5,    9
  SUM_SUB          7,    3,    9
  SUM_SUB          0,    6,    9
  SUM_SUB          4,    2,    9
  SUM_SUB          3,    5,    9
  pmulhrsw        m3,  m12
  pmulhrsw        m5,  m12

  SUM_SUB          0,    7,    9
  SUM_SUB          4,    3,    9
  SUM_SUB          2,    5,    9
  SUM_SUB          6,    1,    9

  SWAP             3,    6
  SWAP             1,    4
%endmacro

; This macro handles 8 pixels per line
%macro ADD_STORE_8P_2X 5;  src1, src2, tmp1, tmp2, zero
  paddw           m%1, m11
  paddw           m%2, m11
  psraw           m%1, 5
  psraw           m%2, 5

  movh            m%3, [outputq]
  movh            m%4, [outputq + strideq]
  punpcklbw       m%3, m%5
  punpcklbw       m%4, m%5
  paddw           m%3, m%1
  paddw           m%4, m%2
  packuswb        m%3, m%5
  packuswb        m%4, m%5
  movh               [outputq], m%3
  movh     [outputq + strideq], m%4
%endmacro

INIT_XMM ssse3
; full inverse 8x8 2D-DCT transform
cglobal idct8x8_64_add, 3, 5, 13, input, output, stride
  mova     m8, [pd_8192]
  mova    m11, [pw_16]
  mova    m12, [pw_11585x2]

  lea      r3, [2 * strideq]

  mova     m0, [inputq +   0]
  mova     m1, [inputq +  16]
  mova     m2, [inputq +  32]
  mova     m3, [inputq +  48]
  mova     m4, [inputq +  64]
  mova     m5, [inputq +  80]
  mova     m6, [inputq +  96]
  mova     m7, [inputq + 112]

  TRANSPOSE8X8  0, 1, 2, 3, 4, 5, 6, 7, 9
  IDCT8_1D
  TRANSPOSE8X8  0, 1, 2, 3, 4, 5, 6, 7, 9
  IDCT8_1D

  pxor    m12, m12
  ADD_STORE_8P_2X  0, 1, 9, 10, 12
  lea              outputq, [outputq + r3]
  ADD_STORE_8P_2X  2, 3, 9, 10, 12
  lea              outputq, [outputq + r3]
  ADD_STORE_8P_2X  4, 5, 9, 10, 12
  lea              outputq, [outputq + r3]
  ADD_STORE_8P_2X  6, 7, 9, 10, 12

  RET

; inverse 8x8 2D-DCT transform with only first 10 coeffs non-zero
cglobal idct8x8_12_add, 3, 5, 13, input, output, stride
  mova       m8, [pd_8192]
  mova      m11, [pw_16]
  mova      m12, [pw_11585x2]

  lea        r3, [2 * strideq]

  mova       m0, [inputq +  0]
  mova       m1, [inputq + 16]
  mova       m2, [inputq + 32]
  mova       m3, [inputq + 48]

  punpcklwd  m0, m1
  punpcklwd  m2, m3
  punpckhdq  m9, m0, m2
  punpckldq  m0, m2
  SWAP       2, 9

  ; m0 -> [0], [0]
  ; m1 -> [1], [1]
  ; m2 -> [2], [2]
  ; m3 -> [3], [3]
  punpckhqdq m10, m0, m0
  punpcklqdq m0,  m0
  punpckhqdq m9,  m2, m2
  punpcklqdq m2,  m2
  SWAP       1, 10
  SWAP       3,  9

  pmulhrsw   m0, m12
  pmulhrsw   m2, [dpw_30274_12540]
  pmulhrsw   m1, [dpw_6392_32138]
  pmulhrsw   m3, [dpw_m18204_27246]

  SUM_SUB    0, 2, 9
  SUM_SUB    1, 3, 9

  punpcklqdq m9, m3, m3
  punpckhqdq m5, m3, m9

  SUM_SUB    3, 5, 9
  punpckhqdq m5, m3
  pmulhrsw   m5, m12

  punpckhqdq m9, m1, m5
  punpcklqdq m1, m5
  SWAP       5, 9

  SUM_SUB    0, 5, 9
  SUM_SUB    2, 1, 9

  punpckhqdq m3, m0, m0
  punpckhqdq m4, m1, m1
  punpckhqdq m6, m5, m5
  punpckhqdq m7, m2, m2

  punpcklwd  m0, m3
  punpcklwd  m7, m2
  punpcklwd  m1, m4
  punpcklwd  m6, m5

  punpckhdq  m4, m0, m7
  punpckldq  m0, m7
  punpckhdq  m10, m1, m6
  punpckldq  m5, m1, m6

  punpckhqdq m1, m0, m5
  punpcklqdq m0, m5
  punpckhqdq m3, m4, m10
  punpcklqdq m2, m4, m10


  pmulhrsw   m0, m12
  pmulhrsw   m6, m2, [dpw_30274_30274]
  pmulhrsw   m4, m2, [dpw_12540_12540]

  pmulhrsw   m7, m1, [dpw_32138_32138]
  pmulhrsw   m1, [dpw_6392_6392]
  pmulhrsw   m5, m3, [dpw_m18204_m18204]
  pmulhrsw   m3, [dpw_27246_27246]

  mova       m2, m0
  SUM_SUB    0, 6, 9
  SUM_SUB    2, 4, 9
  SUM_SUB    1, 5, 9
  SUM_SUB    7, 3, 9

  SUM_SUB    3, 5, 9
  pmulhrsw   m3, m12
  pmulhrsw   m5, m12

  SUM_SUB    0, 7, 9
  SUM_SUB    2, 3, 9
  SUM_SUB    4, 5, 9
  SUM_SUB    6, 1, 9

  SWAP       3, 6
  SWAP       1, 2
  SWAP       2, 4


  pxor    m12, m12
  ADD_STORE_8P_2X  0, 1, 9, 10, 12
  lea              outputq, [outputq + r3]
  ADD_STORE_8P_2X  2, 3, 9, 10, 12
  lea              outputq, [outputq + r3]
  ADD_STORE_8P_2X  4, 5, 9, 10, 12
  lea              outputq, [outputq + r3]
  ADD_STORE_8P_2X  6, 7, 9, 10, 12

  RET

%macro IDCT32X32_34 0
  ; save input for second pass
  mova [stp + 16 *  16], m1  ; save input[1]
  mova [stp + 16 *  19], m7  ; save input[7]
  mova [stp + 16 *  20], m5  ; save input[5]
  mova [stp + 16 *  23], m3  ; save input[3]

; FIRST PASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ; STAGE 1 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova              m14, [pw_11585x2]

  ; STAGE 2 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova             m11, m2
  pmulhrsw          m2, [pw__1606x2] ; stp2_8
  pmulhrsw         m11, [pw_16305x2] ; stp2_15

  mova             m12, m6
  pmulhrsw          m6, [pw_15679x2]  ; stp2_12
  pmulhrsw         m12, [pw_m4756x2]  ; stp2_11

  ; STAGE 3 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova             m13, m4
  pmulhrsw          m4, [pw__3196x2]  ; stp2_4
  pmulhrsw         m13, [pw_16069x2]  ; stp2_7

  ; STAGE 4 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  pmulhrsw        m0, m14    ; stp2_0 = stp2_1

  mova m5, m11 ;save stp2_15
  mova m1, m2  ;save stp2_8
  BUTTERFLY_4X     11,    2,   6270, 15137,  m8,  9,  10 ; stp2_9, stp2_14


  mova m3, m12 ;save stp2_11
  mova m7, m6  ;save stp2_12
  BUTTERFLY_4Xmm   6,    12,   6270, 15137,  m8,  9,  10 ; stp2_13, stp2_10

  ; STAGE 5 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SUM_SUB            1,    3,    9 ; stp1_8, stp1_11
  SUM_SUB           11,   12,    9 ; stp1_9, stp1_10
  SUM_SUB            5,   7,    9 ; stp1_15, stp1_12
  SUM_SUB            2,   6,    9 ; stp1_14, stp1_13

  mova [stp + 16 * 14], m2  ; stp1_14
  mova [stp + 16 * 15], m5  ; stp1_15

  mova              m5, m13 ;save stp1_7
  mova              m2, m4  ;save stp1_4
  SUM_SUB           13,   4,    9
  pmulhrsw          m4, m14 ; stp1_5
  pmulhrsw         m13, m14 ; stp1_6

  ; STAGE 6 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova              m8, [pw_11585x2]
  mova             m15, m0 ; stp2_3 = stp2_0
  mova             m10, m0 ; stp2_2 = stp2_0
  mova             m14, m0 ; stp2_1 = stp2_0
  SUM_SUB           15,    2,    9 ; stp2_3, stp2_4
  SUM_SUB           10,    4,    9 ; stp2_2, stp2_5
  SUM_SUB            0,    5,    9 ; stp2_0, stp2_7
  SUM_SUB           14,   13,    9 ; stp2_1, stp2_6

  SUM_SUB            6,   12,    9
  SUM_SUB            7,    3,    9
  pmulhrsw          m6, m8 ; stp2_13
  pmulhrsw         m12, m8 ; stp2_10
  pmulhrsw          m3, m8 ; stp2_11
  pmulhrsw          m7, m8 ; stp2_12

  ; STAGE 7 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SUM_SUB           15,   7,    9 ; stp1_3, stp1_12
  mova [stp + 16 *  3], m15
  mova [stp + 16 * 12], m7

  SUM_SUB            2,    3,    9 ; stp1_4, stp1_11
  mova [stp + 16 *  4], m2
  mova [stp + 16 * 11], m3

  SUM_SUB            4,  12,    9 ; stp1_5, stp1_10
  mova [stp + 16 *  5], m4
  mova [stp + 16 * 10], m12

  SUM_SUB           13,  11,    9 ; stp1_6, stp1_9
  mova [stp + 16 *  6], m13
  mova [stp + 16 *  9], m11

  SUM_SUB            5,   1,    9 ; stp1_7, stp1_8
  mova [stp + 16 *  7], m5
  mova [stp + 16 *  8], m1

  ; TODO: move to stg6 .. saves load, store
  mova              m3, [stp + 16 * 15]
  SUM_SUB            0,   3,    9 ; stp1_0, stp1_15
  mova [stp + 16 *  0], m0
  mova [stp + 16 * 15], m3

  SUM_SUB           10,    6,    9 ; stp1_2, stp1_13
  mova [stp + 16 *  2], m10
  mova [stp + 16 * 13], m6

  ; TODO: move to stg6 .. saves load, store
  mova              m5, [stp + 16 * 14]
  SUM_SUB           14,   5,    9 ; stp1_1, stp1_14
  mova [stp + 16 *  1], m14
  mova [stp + 16 * 14], m5

  mova              m8, [pd_8192]
; FIRST PASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

; SECOND PASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova              m1, [stp + 16 *  16]  ; restore input[1]
  mova              m7, [stp + 16 *  19]  ; restore input[7]
  mova              m5, [stp + 16 *  20]  ; restore input[5]
  mova              m3, [stp + 16 *  23]  ; restore input[3]

  ; STAGE 1 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova             m11, m1
  pmulhrsw          m1, [pw___804x2] ; stp1_16
  pmulhrsw         m11, [pw_16364x2] ; stp1_31

  mova             m12, m7
  pmulhrsw         m12, [pw_m5520x2] ; stp1_19
  pmulhrsw          m7, [pw_15426x2] ; stp1_28

  mova             m13, m5
  pmulhrsw          m5, [pw__3981x2] ; stp1_20
  pmulhrsw         m13, [pw_15893x2] ; stp1_27

  mova             m14, m3
  pmulhrsw         m14, [pw_m2404x2] ; stp1_23
  pmulhrsw          m3, [pw_16207x2] ; stp1_24

  ; STAGE 2 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova             m15, m1         ; stp1_16
  mova             m10, m12        ; stp1_19
  SUM_SUB           15,  10,  9 ; STG4: stp2_16, stp2_19

  mova              m6, m14 ; stp1_23
  mova              m4, m5  ; stp1_20
  SUM_SUB            6,   4,  9 ; STG4: stp2_23, stp2_20
  SUM_SUB           15,   6,  9 ; STG6: stp2_16, stp2_23

  mova [stp + 16 * 16], m15  ; stp2_16
  mova [stp + 16 * 19], m10  ; stp2_19
  mova [stp + 16 * 20], m4   ; stp2_20
  mova [stp + 16 * 23], m6   ; stp2_23

  mova             m15, m3  ; stp1_24
  mova              m2, m13 ; stp1_27
  SUM_SUB           15,   2,  9 ; STG4: stp2_24, stp2_27

  mova             m10, [pw_11585x2]
  mova              m0, m11     ; stp1_31
  mova              m4, m7      ; stp1_28
  SUM_SUB            0,   4,  9 ; STG4: stp2_31, stp2_28
  SUM_SUB            0,  15,  9 ; STG6: stp2_31, stp2_24

  SUM_SUB           15,   6,  9
  pmulhrsw          m6, m10     ; stp1_23
  pmulhrsw         m15, m10     ; stp1_24
;  BUTTERFLY_4X      15,  6, 11585, 11585,  m8,  9, 10 ; STG7: stp1_23, stp1_24

  ; STAGE 3 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  BUTTERFLY_4X      11,  1,  3196, 16069,  m8,  9, 10 ; stp1_17, stp1_30
  BUTTERFLY_4Xmm     7, 12,  3196, 16069,  m8,  9, 10 ; stp1_29, stp1_18
  BUTTERFLY_4X      13,  5, 13623,  9102,  m8,  9, 10 ; stp1_21, stp1_26
  BUTTERFLY_4Xmm     3, 14, 13623,  9102,  m8,  9, 10 ; stp1_25, stp1_22

  mova [stp + 16 * 23], m6   ; stp1_23
  mova [stp + 16 * 24], m15  ; stp1_24
  mova [stp + 16 * 27], m2   ; stp2_27
  mova [stp + 16 * 28], m4   ; stp2_28
  mova [stp + 16 * 31], m0   ; stp2_31

  ; STAGE 4 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova             m10, [pw_11585x2]
  SUM_SUB           11,  12,  9 ; stp2_17, stp2_18
  SUM_SUB           14,  13,  9 ; stp2_22, stp2_21
  SUM_SUB            3,   5,  9 ; stp2_25, stp2_26
  SUM_SUB            1,   7,  9 ; stp2_30, stp2_29

  SUM_SUB           11,  14,  9 ; STG6: stp2_17, stp2_22
  SUM_SUB            1,   3,  9 ; STG6: stp2_30, stp2_25

  SUM_SUB            3,  14,  9
  pmulhrsw         m14, m10     ; stp1_22
  pmulhrsw          m3, m10     ; stp1_25

  mova [stp + 16 * 17], m11  ; stp2_17
  mova [stp + 16 * 22], m14  ; stp2_22
  mova [stp + 16 * 25], m3   ; stp2_25
  mova [stp + 16 * 30], m1   ; stp2_30

  ; STAGE 5 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova             m14, [stp + 16 * 19]  ; stp2_19
  mova              m0, [stp + 16 * 20]  ; stp2_20
  mova              m1, [stp + 16 * 27]  ; stp2_27
  mova             m15, [stp + 16 * 28]  ; stp2_28
  BUTTERFLY_4X       7, 12,  6270, 15137,  m8,  9, 10 ; stp1_18, stp1_29
  BUTTERFLY_4X      15, 14,  6270, 15137,  m8,  9, 10 ; stp1_19, stp1_28
  BUTTERFLY_4Xmm     1,  0,  6270, 15137,  m8,  9, 10 ; stp1_27, stp1_20
  BUTTERFLY_4Xmm     5, 13,  6270, 15137,  m8,  9, 10 ; stp1_26, stp1_21

  ; STAGE 6 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mova             m10, [pw_11585x2]
  SUM_SUB            7,  13,    9 ; stp2_18, stp2_21
  SUM_SUB           15,   0,    9 ; stp2_19, stp2_20
  SUM_SUB           12,   5,    9 ; stp2_29, stp2_26
  SUM_SUB           14,   1,    9 ; stp2_28, stp2_27
  mova [stp + 16 * 18], m7   ; stp2_18
  mova [stp + 16 * 19], m15  ; stp2_19
  mova [stp + 16 * 28], m14  ; stp2_28
  mova [stp + 16 * 29], m12  ; stp2_29

  ; STAGE 7 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SUM_SUB            1,   0,  9
  pmulhrsw          m1, m10 ; stp1_27
  pmulhrsw          m0, m10 ; stp1_20
  SUM_SUB            5,  13,  9
  pmulhrsw          m5, m10 ; stp1_26
  pmulhrsw         m13, m10 ; stp1_21

  mova [stp + 16 * 20], m0    ; stp1_20
  mova [stp + 16 * 21], m13   ; stp1_21
  mova [stp + 16 * 26], m5    ; stp1_26
  mova [stp + 16 * 27], m1    ; stp1_27
; SECOND PASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
%endmacro

%macro FINAL_STAGE 0
  mov         r3, stp
  lea         r4, [stp + 16 * 28]
  mov         r5, 4
%%sum_sub_loop:
  mova        m0, [r3 +  0]
  mova        m2, [r3 + 16]
  mova        m4, [r3 + 32]
  mova        m6, [r3 + 48]
  mova        m1, [r4 +  0]
  mova        m3, [r4 + 16]
  mova        m5, [r4 + 32]
  mova        m7, [r4 + 48]
  SUM_SUB      0, 7, 9
  SUM_SUB      2, 5, 9
  SUM_SUB      4, 3, 9
  SUM_SUB      6, 1, 9
  mova [r3 +  0], m0
  mova [r3 + 16], m2
  mova [r3 + 32], m4
  mova [r3 + 48], m6
  mova [r4 +  0], m1
  mova [r4 + 16], m3
  mova [r4 + 32], m5
  mova [r4 + 48], m7
  add         r3, 32*2
  sub         r4, 32*2
  dec         r5
  jnz %%sum_sub_loop
%endmacro

%macro RECON_AND_STORE 1
  mova            m11, [pw_32]
  lea             stp, [rsp + %1]
  mov              r6, 32
  pxor             m8, m8
%%recon_and_store:
  mova             m0, [stp + 16 * 32 * 0]
  mova             m1, [stp + 16 * 32 * 1]
  mova             m2, [stp + 16 * 32 * 2]
  mova             m3, [stp + 16 * 32 * 3]
  add             stp, 16

  ; TODO: use pmulhrsw here
  paddw            m0, m11
  paddw            m1, m11
  paddw            m2, m11
  paddw            m3, m11
  psraw            m0, 6
  psraw            m1, 6
  psraw            m2, 6
  psraw            m3, 6
  movh             m4, [outputq +  0]
  movh             m5, [outputq +  8]
  movh             m6, [outputq + 16]
  movh             m7, [outputq + 24]
  punpcklbw        m4, m8
  punpcklbw        m5, m8
  punpcklbw        m6, m8
  punpcklbw        m7, m8
  paddw            m0, m4
  paddw            m1, m5
  paddw            m2, m6
  paddw            m3, m7
  packuswb         m0, m1
  packuswb         m2, m3
  mova [outputq +  0], m0
  mova [outputq + 16], m2
  lea         outputq, [outputq + strideq]
  dec              r6
  jnz %%recon_and_store
%endmacro

%define LOCAL_VARS_SIZE 16*32*4

%define stp r8

INIT_XMM ssse3
cglobal idct32x32_34_add, 3, 8, 16, LOCAL_VARS_SIZE, input, output, stride
  mova             m8, [pd_8192]
  mova             m0, [inputq +       0]
  mova             m1, [inputq +  32 * 2]
  mova             m2, [inputq +  64 * 2]
  mova             m3, [inputq +  96 * 2]
  mova             m4, [inputq + 128 * 2]
  mova             m5, [inputq + 160 * 2]
  mova             m6, [inputq + 192 * 2]
  mova             m7, [inputq + 224 * 2]

  TRANSPOSE8X8  0, 1, 2, 3, 4, 5, 6, 7, 9

  lea             stp, [rsp + 16 * 96]

  IDCT32X32_34
  FINAL_STAGE

  mov              r7, 4
  mov          inputq, stp
  lea             stp, [rsp + 16 * 0]
i32x32_34_loop:
  mova             m0, [inputq + 16 * 0]
  mova             m1, [inputq + 16 * 1]
  mova             m2, [inputq + 16 * 2]
  mova             m3, [inputq + 16 * 3]
  mova             m4, [inputq + 16 * 4]
  mova             m5, [inputq + 16 * 5]
  mova             m6, [inputq + 16 * 6]
  mova             m7, [inputq + 16 * 7]
  lea          inputq, [inputq + 16 * 8]

  TRANSPOSE8X8      0, 1, 2, 3, 4, 5, 6, 7, 9

  IDCT32X32_34
  FINAL_STAGE

  lea             stp, [stp + 16 * 32]
  dec              r7
  jnz i32x32_34_loop

  RECON_AND_STORE 0

  RET
%endif
