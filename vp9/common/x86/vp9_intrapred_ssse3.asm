;
;  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;

%include "third_party/x86inc/x86inc.asm"

SECTION_RODATA

pw_2: times 8 dw 2
pb_7m1: times 8 db 7, -1
pb_15: times 16 db 15

sh_w01234577: db 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 7, -1, 7, -1
sh_w12345677: db 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1, 7, -1
sh_w23456777: db 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1, 7, -1, 7, -1
sh_w01234567: db 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1
sh_w12345678: db 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1, 8, -1
sh_w23456789: db 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1, 8, -1, 9, -1
sh_w89abcdef: db 8, -1, 9, -1, 10, -1, 11, -1, 12, -1, 13, -1, 14, -1, 15, -1
sh_w9abcdeff: db 9, -1, 10, -1, 11, -1, 12, -1, 13, -1, 14, -1, 15, -1, 15, -1
sh_wabcdefff: db 10, -1, 11, -1, 12, -1, 13, -1, 14, -1, 15, -1, 15, -1, 15, -1
sh_b123456789abcdeff: db 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15

SECTION .text

INIT_MMX ssse3
cglobal h_predictor_4x4, 2, 4, 3, dst, stride, line, left
  movifnidn          leftq, leftmp
  add                leftq, 4
  mov                lineq, -2
  pxor                  m0, m0
.loop:
  movd                  m1, [leftq+lineq*2  ]
  movd                  m2, [leftq+lineq*2+1]
  pshufb                m1, m0
  pshufb                m2, m0
  movd      [dstq        ], m1
  movd      [dstq+strideq], m2
  lea                 dstq, [dstq+strideq*2]
  inc                lineq
  jnz .loop
  REP_RET

INIT_MMX ssse3
cglobal h_predictor_8x8, 2, 4, 3, dst, stride, line, left
  movifnidn          leftq, leftmp
  add                leftq, 8
  mov                lineq, -4
  pxor                  m0, m0
.loop:
  movd                  m1, [leftq+lineq*2  ]
  movd                  m2, [leftq+lineq*2+1]
  pshufb                m1, m0
  pshufb                m2, m0
  movq      [dstq        ], m1
  movq      [dstq+strideq], m2
  lea                 dstq, [dstq+strideq*2]
  inc                lineq
  jnz .loop
  REP_RET

INIT_XMM ssse3
cglobal h_predictor_16x16, 2, 4, 3, dst, stride, line, left
  movifnidn          leftq, leftmp
  add                leftq, 16
  mov                lineq, -8
  pxor                  m0, m0
.loop:
  movd                  m1, [leftq+lineq*2  ]
  movd                  m2, [leftq+lineq*2+1]
  pshufb                m1, m0
  pshufb                m2, m0
  mova      [dstq        ], m1
  mova      [dstq+strideq], m2
  lea                 dstq, [dstq+strideq*2]
  inc                lineq
  jnz .loop
  REP_RET

INIT_XMM ssse3
cglobal h_predictor_32x32, 2, 4, 3, dst, stride, line, left
  movifnidn          leftq, leftmp
  add                leftq, 32
  mov                lineq, -16
  pxor                  m0, m0
.loop:
  movd                  m1, [leftq+lineq*2  ]
  movd                  m2, [leftq+lineq*2+1]
  pshufb                m1, m0
  pshufb                m2, m0
  mova   [dstq           ], m1
  mova   [dstq        +16], m1
  mova   [dstq+strideq   ], m2
  mova   [dstq+strideq+16], m2
  lea                 dstq, [dstq+strideq*2]
  inc                lineq
  jnz .loop
  REP_RET

INIT_XMM ssse3
cglobal d45_predictor_4x4, 3, 3, 3, dst, stride, above
  movq                m0, [aboveq]
  pshufb              m2, m0, [sh_w23456777]
  pshufb              m1, m0, [sh_w12345677]
  pshufb              m0, [sh_w01234577]
  paddw               m1, m1
  paddw               m0, m2
  paddw               m1, [pw_2]
  paddw               m0, m1
  psraw               m0, 2
  packuswb            m0, m0
  movd    [dstq        ], m0
  psrldq              m0, 1
  movd    [dstq+strideq], m0
  lea               dstq, [dstq+strideq*2]
  psrldq              m0, 1
  movd    [dstq        ], m0
  psrldq              m0, 1
  movd    [dstq+strideq], m0
  RET

INIT_XMM ssse3
cglobal d45_predictor_8x8, 3, 4, 4, dst, stride, above, line
  movq                m0, [aboveq]
  DEFINE_ARGS dst, stride, stride3, line
  lea           stride3q, [strideq*3]
  pshufb              m3, m0, [sh_w23456777]
  pshufb              m2, m0, [sh_w12345677]
  pshufb              m1, m0, [sh_w01234567]
  pshufb              m0, [pb_7m1]
  paddw               m2, m2
  paddw               m1, m3
  paddw               m2, [pw_2]
  paddw               m1, m2
  psraw               m1, 2
  packuswb            m1, m0
  mov              lined, 2
.loop:
  movq  [dstq          ], m1
  psrldq              m1, 1
  movq  [dstq+strideq  ], m1
  psrldq              m1, 1
  movq  [dstq+strideq*2], m1
  psrldq              m1, 1
  movq  [dstq+stride3q ], m1
  psrldq              m1, 1
  lea               dstq, [dstq+strideq*4]
  dec              lined
  jnz .loop
  REP_RET

INIT_XMM ssse3
cglobal d45_predictor_16x16, 3, 4, 6, dst, stride, above, line
  mova                m0, [aboveq]
  DEFINE_ARGS dst, stride, stride3, line
  lea           stride3q, [strideq*3]
  pshufb              m5, m0, [sh_wabcdefff]
  pshufb              m4, m0, [sh_w23456789]
  pshufb              m3, m0, [sh_w9abcdeff]
  pshufb              m2, m0, [sh_w12345678]
  pshufb              m1, m0, [sh_w89abcdef]
  pshufb              m0, [sh_w01234567]
  paddw               m3, m3
  paddw               m2, m2
  paddw               m0, m4
  paddw               m1, m5
  paddw               m2, [pw_2]
  paddw               m3, [pw_2]
  paddw               m0, m2
  paddw               m1, m3
  psraw               m0, 2
  psraw               m1, 2
  mova                m2, [sh_b123456789abcdeff]
  packuswb            m0, m1
  mov              lined, 4
.loop:
  mova  [dstq          ], m0
  pshufb              m0, m2
  mova  [dstq+strideq  ], m0
  pshufb              m0, m2
  mova  [dstq+strideq*2], m0
  pshufb              m0, m2
  mova  [dstq+stride3q ], m0
  pshufb              m0, m2
  lea               dstq, [dstq+strideq*4]
  dec              lined
  jnz .loop
  REP_RET

INIT_XMM ssse3
cglobal d45_predictor_32x32, 3, 4, 8, dst, stride, above, line
  mova                   m0, [aboveq]
  mova                   m7, [aboveq+16]
  DEFINE_ARGS dst, stride, stride3, line
  lea              stride3q, [strideq*3]
  pshufb                 m6, m7, [sh_wabcdefff]
  pshufb                 m5, m7, [sh_w23456789]
  pshufb                 m4, m7, [sh_w9abcdeff]
  pshufb                 m3, m7, [sh_w12345678]
  pshufb                 m2, m7, [sh_w89abcdef]
  pshufb                 m7, [sh_w01234567]
  paddw                  m4, m4
  paddw                  m3, m3
  paddw                  m5, m7
  paddw                  m6, m2
  paddw                  m3, [pw_2]
  paddw                  m4, [pw_2]
  paddw                  m5, m3
  paddw                  m6, m4
  psraw                  m5, 2
  psraw                  m6, 2
  packuswb               m5, m6
  pshufb                 m4, m0, [sh_w23456789]
  pshufb                 m2, m0, [sh_w12345678]
  pshufb                 m3, m0, [sh_w89abcdef]
  pshufb                 m0, [sh_w01234567]
  palignr                m6, m7, m3, 2
  palignr                m7, m3, 4
  paddw                  m2, m2
  paddw                  m6, m6
  paddw                  m0, m4
  paddw                  m3, m7
  paddw                  m2, [pw_2]
  paddw                  m6, [pw_2]
  paddw                  m0, m2
  paddw                  m3, m6
  psraw                  m0, 2
  psraw                  m3, 2
  mova                   m2, [sh_b123456789abcdeff]
  packuswb               m0, m3
  mov                 lined, 8
.loop:
  mova  [dstq             ], m0
  mova  [dstq          +16], m5
  palignr                m1, m5, m0, 1
  pshufb                 m5, m2
  mova  [dstq+strideq     ], m1
  mova  [dstq+strideq  +16], m5
  palignr                m0, m5, m1, 1
  pshufb                 m5, m2
  mova  [dstq+strideq*2   ], m0
  mova  [dstq+strideq*2+16], m5
  palignr                m1, m5, m0, 1
  pshufb                 m5, m2
  mova  [dstq+stride3q    ], m1
  mova  [dstq+stride3q +16], m5
  palignr                m0, m5, m1, 1
  pshufb                 m5, m2
  lea                  dstq, [dstq+strideq*4]
  dec                 lined
  jnz .loop
  REP_RET
