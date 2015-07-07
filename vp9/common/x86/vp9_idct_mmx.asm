;
;  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;
%include "third_party/x86inc/x86inc.asm"

SECTION .text

%macro TRANSFORM_COLS 0
  paddw           m0,        m2
  psubw           m3,        m1
  psubw           m4,        m0,        m3
  psraw           m4,        1
  psubw           m5,        m4,        m1 ;b1
  psubw           m4,        m2 ;c1
  psubw           m0,        m5
  paddw           m3,        m4
                                ; m0 a0
  SWAP            1,         4  ; m1 c1
  SWAP            2,         3  ; m2 d1
  SWAP            3,         5  ; m3 b1
%endmacro

%macro TRANSPOSE_4X4 0
%if mmsize == 8
  movq            m4,        m0
  movq            m5,        m2
  punpcklwd       m4,        m1
  punpckhwd       m0,        m1
  punpcklwd       m5,        m3
  punpckhwd       m2,        m3
  movq            m1,        m4
  movq            m3,        m0
  punpckldq       m1,        m5
  punpckhdq       m4,        m5
  punpckldq       m3,        m2
  punpckhdq       m0,        m2
%else
  SWAP            4, 0
  SWAP            5, 2
  punpcklwd       m4,        m1
  pshufd          m0,        m4, 0x0e
  punpcklwd       m5,        m3
  pshufd          m2,        m5, 0x0e
  SWAP            1, 4
  SWAP            3, 0
  punpckldq       m1,        m5
  pshufd          m4,        m1, 0x0e
  punpckldq       m3,        m2
  pshufd          m0,        m3, 0x0e
%endif
  SWAP            2, 3, 0, 1, 4
%endmacro

; transposes a 4x4 int16 matrix in xmm0 and xmm1 to the bottom half of xmm0-xmm3
%macro TRANSPOSE_4X4_WIDE 0
  mova            m3, m0
  punpcklwd       m0, m1
  punpckhwd       m3, m1
  mova            m2, m0
  punpcklwd       m0, m3
  punpckhwd       m2, m3
  pshufd          m1, m0, 0x0e
  pshufd          m3, m2, 0x0e
%endmacro

%macro ADD_STORE_4P_2X 5  ; src1, src2, tmp1, tmp2, zero
  movq            m%3,       [outputq]
  movq            m%4,       [outputq + strideq]
  punpcklbw       m%3,       m%5
  punpcklbw       m%4,       m%5
  paddw           m%1,       m%3
  paddw           m%2,       m%4
  packuswb        m%1,       m%5
  packuswb        m%2,       m%5
  movd            [outputq], m%1
  movd            [outputq + strideq], m%2
%endmacro

%macro IWHT_4X4_16_ADD 0
cglobal iwht4x4_16_add, 3, 4, 7, input, output, stride
%if mmsize == 8
  movq            m0,        [inputq +  0] ;a1
  movq            m1,        [inputq +  8] ;b1
  movq            m2,        [inputq + 16] ;c1
  movq            m3,        [inputq + 24] ;d1

  psraw           m0,        2
  psraw           m1,        2
  psraw           m2,        2
  psraw           m3,        2

  TRANSPOSE_4X4
%else
  mova            m0,        [inputq +  0] ;a1
  mova            m1,        [inputq + 16] ;c1

  psraw           m0,        2
  psraw           m1,        2

  TRANSPOSE_4X4_WIDE
%endif
  TRANSFORM_COLS
  TRANSPOSE_4X4
  TRANSFORM_COLS

  pxor            m4, m4
  ADD_STORE_4P_2X  0, 1, 5, 6, 4
  lea             outputq, [outputq + 2 * strideq]
  ADD_STORE_4P_2X  2, 3, 5, 6, 4

  RET
%endmacro

INIT_MMX mmx
IWHT_4X4_16_ADD

INIT_XMM sse2
IWHT_4X4_16_ADD
