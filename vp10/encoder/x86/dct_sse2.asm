;
;  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;

%define private_prefix vp10

%include "third_party/x86inc/x86inc.asm"

SECTION .text

%macro TRANSFORM_COLS 0
  paddw           m0,        m1
  movq            m4,        m0
  psubw           m3,        m2
  psubw           m4,        m3
  psraw           m4,        1
  movq            m5,        m4
  psubw           m5,        m1 ;b1
  psubw           m4,        m2 ;c1
  psubw           m0,        m4
  paddw           m3,        m5
                                ; m0 a0
  SWAP            1,         4  ; m1 c1
  SWAP            2,         3  ; m2 d1
  SWAP            3,         5  ; m3 b1
%endmacro

%macro TRANSPOSE_4X4 0
                                ; 03 02 01 00
                                ; 13 12 11 10
                                ; 23 22 21 20
                                ; 33 32 31 30
  punpcklwd       m0,        m1 ; 13 03 12 02  11 01 10 00
  punpcklwd       m2,        m3 ; 33 23 32 22  31 21 30 20
  mova            m1,        m0
  punpckldq       m0,        m2 ; 31 21 11 01  30 20 10 00
  punpckhdq       m1,        m2 ; 33 23 13 03  32 22 12 02
%endmacro

INIT_XMM sse2
cglobal fwht4x4, 3, 4, 8, input, output, stride
  lea             r3q,       [inputq + strideq*4]
  movq            m0,        [inputq] ;a1
  movq            m1,        [inputq + strideq*2] ;b1
  movq            m2,        [r3q] ;c1
  movq            m3,        [r3q + strideq*2] ;d1

  TRANSFORM_COLS
  TRANSPOSE_4X4
  SWAP            1,         2
  psrldq          m1,        m0, 8
  psrldq          m3,        m2, 8
  TRANSFORM_COLS
  TRANSPOSE_4X4

  psllw           m0,        2
  psllw           m1,        2

%if CONFIG_VP9_HIGHBITDEPTH
  ; sign extension
  mova            m2,             m0
  mova            m3,             m1
  punpcklwd       m0,             m0
  punpcklwd       m1,             m1
  punpckhwd       m2,             m2
  punpckhwd       m3,             m3
  psrad           m0,             16
  psrad           m1,             16
  psrad           m2,             16
  psrad           m3,             16
  movu            [outputq],      m0
  movu            [outputq + 16], m2
  movu            [outputq + 32], m1
  movu            [outputq + 48], m3
%else
  movu            [outputq],      m0
  movu            [outputq + 16], m1
%endif

  RET
