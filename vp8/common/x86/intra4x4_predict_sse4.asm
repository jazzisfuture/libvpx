;
;  Copyright (c) 2011 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;


%include "vpx_ports/x86_abi_support.asm"
%include "asm_com_offsets.asm"

;void vp8_intra4x4_predict_sse4
;(
;    BLOCKD        *x,        rdi
;    int            b_mode,   rsi
;    unsigned char *predictor rdx
;    int            stride    rcx
;)
global sym(vp8_intra4x4_predict_sse4)
sym(vp8_intra4x4_predict_sse4):

    %define x          rdi
    %define b_mode     rsi
    %define predictor  rdx
    %define stride     rcx

    %define base_dst   r8
    %define dst        r9
    %define dst_stride r9

    %define stride3    r8

    %define out0       [predictor]
    %define out1       [predictor + stride]
    %define out2       [predictor + 2 * stride]
    %define out3       [predictor + stride3]

    ALIGN_STACK 16, rax
    sub rsp, 16
    %define pp         rsp
    %macro ret 0
      add rsp, 16
      pop rsp
      ret
    %endmacro

    movsxd  dst, dword ptr [x + blockd_dst]
    mov     base_dst, [x + blockd_base_dst]
    mov     base_dst, [base_dst] ; unsigned char **
    add     base_dst, dst
    sub     base_dst, 1 ; start in Left/top_left

    movsxd  dst_stride, dword ptr [x + blockd_dst_stride]

    pxor    xmm0, xmm0
    pinsrb  xmm0, [base_dst], 3                  ; Left[0]
    pinsrb  xmm0, [base_dst + dst_stride], 2     ; Left[1]
    pinsrb  xmm0, [base_dst + 2 * dst_stride], 1 ; Left[2]

    sub     base_dst, dst_stride

    pinsrb  xmm0, [base_dst + 4 * dst_stride], 0 ; Left[3]

    movdqu  xmm4, [base_dst]                     ; top_left A[0-7]
    pslldq  xmm4, 4

    por     xmm0, xmm4

    ; pp[] = { Left[3-0], top_left, Above[0-7] }
    movdqa  [pp], xmm0

    lea     stride3, [stride + 2 * stride]

;    cmp     b_mode, B_DC_PRED_VAL
;    je      .B_DC_PRED_VAL
    cmp     b_mode, B_TM_PRED_VAL
    je      .B_TM_PRED_VAL
    cmp     b_mode, B_VE_PRED_VAL
    je      .B_VE_PRED_VAL
    cmp     b_mode, B_HE_PRED_VAL
    je      .B_HE_PRED_VAL
    cmp     b_mode, B_LD_PRED_VAL
    je      .B_LD_PRED_VAL
    cmp     b_mode, B_RD_PRED_VAL
    je      .B_RD_PRED_VAL
    cmp     b_mode, B_VR_PRED_VAL
    je      .B_VR_PRED_VAL
    cmp     b_mode, B_VL_PRED_VAL
    je      .B_VL_PRED_VAL
    cmp     b_mode, B_HD_PRED_VAL
    je      .B_HD_PRED_VAL
    cmp     b_mode, B_HU_PRED_VAL
    je      .B_HU_PRED_VAL

;    ret

.B_DC_PRED_VAL:
    movd    xmm3, [GLOBAL(t4)]
    pxor    xmm4, xmm4 ; zero
    movdqa  xmm5, [GLOBAL(t1)]

    movd    xmm0, [pp] ; L
    movd    xmm1, [pp + 5] ; A

    psllq xmm1, 32

    por xmm0, xmm1

    punpcklbw xmm0, xmm4

    pmaddwd xmm0, xmm5

    packssdw xmm0, xmm4

    pmaddwd xmm0, xmm5

    packssdw xmm0, xmm4

    pmaddwd xmm0, xmm5

    paddusw xmm0, xmm3

    psrlw xmm0, 0x3

    pshufb xmm0, xmm4

    movd out0, xmm0
    movd out1, xmm0
    movd out2, xmm0
    movd out3, xmm0

    ret

.B_TM_PRED_VAL:
; because of the order of the math (a - b + c)
; we need to expand up to words and use signed
; operations, then pack back down to bytes
; exploit movd 0'ing 0x04
    pxor    xmm3, xmm3 ; zero

    ;movd    xmm0, [pp] ; L ; have to be careful with this because sometimes we expect high values to be 0
    movd    xmm1, [pp + 5] ; A
    movd    xmm2, [pp + 4] ; top_left

    pshufb xmm0, [GLOBAL(tm_left_extract)]
    pshufb xmm1, [GLOBAL(tm_above_extract)]
    pshufb xmm2, [GLOBAL(tm_top_left_extract)]

    movdqa xmm4, xmm0
    punpcklbw xmm0, xmm3
    punpckhbw xmm4, xmm3

    psubsw xmm1, xmm2 ; A - top_left
    paddsw xmm0, xmm1 ; (A - top_left) + L
    paddsw xmm4, xmm1 ; (A - top_left) + L

    packuswb xmm0, xmm4

    pshufd  xmm1, xmm0, 0x0b

    movd out3, xmm0
    psrlq xmm0, 32
    movd out2, xmm0

    movd out0, xmm1
    psrlq xmm1, 32
    movd out1, xmm1

    ret

.B_VE_PRED_VAL:
    movd    xmm0, [pp+5] ; A[0-3]
    movd    xmm2, [pp+4] ; top_left A[0-2]
    movd    xmm3, [pp+6] ; A[1-4]
    pshufb xmm0, [GLOBAL(tm_above_extract)]
    pshufb xmm2, [GLOBAL(tm_above_extract)]
    pshufb xmm3, [GLOBAL(tm_above_extract)]

    paddusw xmm0, xmm0 ; A[0-3] * 2
    paddusw xmm0, xmm2 ; += top_left A[0-2] (first element)
    paddusw xmm0, xmm3 ; += A[1-4] (third element)
    paddusw xmm0, [GLOBAL(t2)] ; += 2 (fourth element)
    psrlw xmm0, 2 ; >>= 2

    packuswb xmm0, xmm0

    movd out0, xmm0
    movd out1, xmm0
    movd out2, xmm0
    movd out3, xmm0

    ret

.B_HE_PRED_VAL:
    movd    xmm0, [pp] ; L[3-0]
    movdqa  xmm1, xmm0
    movd    xmm2, [pp+1] ; L[2-0] top_left
    pshufb xmm0, [GLOBAL(tm_above_extract)]
    pshufb xmm1, [GLOBAL(tm_he_dup_extract)] ; L[3] L[3-1]
    pshufb xmm2, [GLOBAL(tm_above_extract)]

    paddusw xmm0, xmm0 ; L[3-0] * 2
    paddusw xmm0, xmm2 ; += L[2-0] top_left(first element)
    paddusw xmm0, xmm1 ; += L[3] L[3-1] (third element)
    paddusw xmm0, [GLOBAL(t2)] ; += 2 (fourth element)
    psrlw xmm0, 2 ; >>= 2

    packuswb xmm0, xmm0
    pshufb xmm0, [GLOBAL(tm_left_extract)]

    pshufd  xmm1, xmm0, 0x0b

    movd out3, xmm0
    psrlq xmm0, 32
    movd out2, xmm0

    movd out0, xmm1
    psrlq xmm1, 32
    movd out1, xmm1

    ret

.B_LD_PRED_VAL:
    pxor    xmm3, xmm3

    movq    xmm0, [pp+5] ; A[0-6]
    movdqa  xmm1, xmm0
    movdqa  xmm2, xmm0

    psrlq   xmm1, 8 ; A[1-7]

    punpcklbw xmm0, xmm3
    punpcklbw xmm1, xmm3
    pshufb xmm2, [GLOBAL(ld_dup_last)] ; A[2-7] A[7]

    paddusw xmm1, xmm1 ; A[1-7] * 2
    paddusw xmm1, xmm0 ; += A[0-6] top_left(first element)
    paddusw xmm1, xmm2 ; += A[2-7] A[7] (third element)
    paddusw xmm1, [GLOBAL(t2)] ; += 2 (fourth element)
    psrlw xmm1, 2 ; >>= 2

    packuswb xmm1, xmm1
    movd out0, xmm1
    psrlq xmm1, 8
    movd out1, xmm1
    psrlq xmm1, 8
    movd out2, xmm1
    psrlq xmm1, 8
    movd out3, xmm1
    ret

    ret

.B_RD_PRED_VAL:
; similar to LD_PRED. write out in reverse
    pxor    xmm3, xmm3

    ;movq    xmm0, [pp] ; L[3-0] top_left A[0-1]
    movq    xmm1, [pp + 1] ; L[2-0] top_left A[0-2]
    movq    xmm2, [pp + 2] ; L[1-0] top_left A[0-3]

    punpcklbw xmm0, xmm3
    punpcklbw xmm1, xmm3
    punpcklbw xmm2, xmm3

    paddusw xmm1, xmm1 ; L[2-0] top_left A[0-2] * 2
    paddusw xmm1, xmm0 ; += L[3-0] top_left A[0-1] (first element)
    paddusw xmm1, xmm2 ; += L[1-0] top_left A[0-3] (third element)
    paddusw xmm1, [GLOBAL(t2)] ; += 2 (fourth element)
    psrlw xmm1, 2 ; >>= 2

    packuswb xmm1, xmm1
    movd out3, xmm1
    psrlq xmm1, 8
    movd out2, xmm1
    psrlq xmm1, 8
    movd out1, xmm1
    psrlq xmm1, 8
    movd out0, xmm1

    ret

.B_VR_PRED_VAL:
    pxor    xmm7, xmm7

    movq    xmm1, [pp + 1] ; L[2-0] top_left A[0-1]
    movq    xmm2, [pp + 2] ; L[1-0] top_left A[0-2]
    movq    xmm3, [pp + 3] ; L[0] top_left A[0-3]
    movq    xmm4, [pp + 4] ; top_left A[0-2]
    movq    xmm5, [pp + 5] ; A[0-3]

    punpcklbw xmm1, xmm7
    punpcklbw xmm2, xmm7
    punpcklbw xmm3, xmm7
    punpcklbw xmm4, xmm7
    punpcklbw xmm5, xmm7

    paddusw xmm2, xmm2 ; L[1-0] top_left A[0-2] * 2
    paddusw xmm2, xmm1 ; += L[2-0] top_left A[0-1] (first element)
    paddusw xmm2, xmm3 ; += L[0] top_left A[0-3] (third element)
    paddusw xmm2, [GLOBAL(t2)] ; += 2 (fourth element)
    psrlw xmm2, 2 ; >>= 2
    packuswb xmm2, xmm7
    movdqa xmm1, xmm2 ; for [0]
    movdqa xmm3, xmm2 ; for [1]
    psrlq xmm2, 16 ; >>= 2
    movd out1, xmm2

    ; instead of a mask could shl/shr. already shifting in one direction
    pand xmm1, [first_element]
    psllq xmm2, 8
    por xmm1, xmm2
    movd out3, xmm1

    psrlq xmm3, 8
    pand xmm3, [first_element]

    ; there should be a way to do this as part of the earlier combinations
    paddusw xmm4, xmm5 ; top_left A[0-2] + A[0-3]
    paddusw xmm4, [GLOBAL(t1)] ; += 1
    psrlw xmm4, 1 ; >>= 1
    packuswb xmm4, xmm7
    movd out0, xmm4

    psllq xmm4, 8
    por xmm4, xmm3
    movd out2, xmm4

    ret

.B_VL_PRED_VAL:
    pxor    xmm3, xmm3

    movq    xmm0, [pp + 5] ; A[0-5]
    movq    xmm1, [pp + 6] ; A[1-6]
    movq    xmm2, [pp + 7] ; A[2-7]

    punpcklbw xmm0, xmm3
    punpcklbw xmm1, xmm3
    punpcklbw xmm2, xmm3

    paddusw xmm0, xmm1 ; A[0-5] + A[1-6] for 0/2/4/6
    paddusw xmm1, xmm0 ; += A[1-6] for 1/3/5/7/8/9
    paddusw xmm1, xmm2 ; += A[2-7]

    paddusw xmm0, [GLOBAL(t1)] ; += 1
    psrlw xmm0, 1; >>= 1
    packuswb xmm0, xmm3
    movd out0, xmm0

    paddusw xmm1, [GLOBAL(t2)] ; += 2
    psrlw xmm1, 2; >>= 2
    packuswb xmm1, xmm3
    movd out1, xmm1

    movdqa xmm2, xmm1
    psrlq xmm2, 32
    psllq xmm2, 24
    psrld xmm0, 8 ; also clears high bits
    por xmm0, xmm2
    movd out2, xmm0

    psrlq xmm2, 32 ; clear low bits, move to the bottom
    psllq xmm2, 24 ; put 8 in high
    psrld xmm1, 8 ; clear high bits
    por xmm1, xmm2
    movd out3, xmm1

    ret

.B_HD_PRED_VAL:
    pxor    xmm3, xmm3

    ;movq    xmm0, [pp] ; L[3-0] top_left A[0-5]
    movq    xmm1, [pp + 1] ; A[1-6]
    movq    xmm2, [pp + 2] ; A[2-7]

    punpcklbw xmm0, xmm3
    punpcklbw xmm1, xmm3
    punpcklbw xmm2, xmm3

    paddusw xmm0, xmm1 ; A[0-3] + A[1-4] for 0/2/4/6
    paddusw xmm1, xmm0 ; += A[1-6] for 1/3/5/7/8/9
    paddusw xmm1, xmm2 ; += A[2-7]

    paddusw xmm0, [GLOBAL(t1)] ; += 1
    psrlw xmm0, 1; >>= 1
    packuswb xmm0, xmm3

    paddusw xmm1, [GLOBAL(t2)] ; += 2
    psrlw xmm1, 2; >>= 2
    packuswb xmm1, xmm3

    movdqa xmm2, xmm1 ; 8/9
    psrlq xmm2, 32
    psllq xmm2, 16

    punpcklbw xmm0, xmm1

    movd out3, xmm0
    psrlq xmm0, 16
    movd out2, xmm0
    psrlq xmm0, 16
    movd out1, xmm0
    psrlq xmm0, 16
    por xmm0, xmm2
    movd out0, xmm0

    ret

.B_HU_PRED_VAL:
    pxor    xmm4, xmm4
    movd    xmm0, [pp] ; L[3-0]

    movdqa xmm1, xmm0
    movdqa xmm2, xmm0
    movdqa xmm3, xmm0
    movdqa xmm5, xmm0

    pslld xmm1, 8 ; L[3-1]
    pslld xmm2, 16 ; L[3-2]
    pslld xmm3, 24 ; L[3]
    psrld xmm3, 16 ; L[3]
    por xmm2, xmm3 ; L[3] L[3-2]

    pshufb xmm5, [GLOBAL(tm_left_extract)] ; dup L[3]

    punpcklbw xmm0, xmm4
    punpcklbw xmm1, xmm4
    punpcklbw xmm2, xmm4

    paddusw xmm0, xmm1 ; L[3-0] + L[2-0]
    paddusw xmm1, xmm0 ; += L[2-0]
    paddusw xmm1, xmm2 ; += L[1-0] L[0]

    paddusw xmm0, [GLOBAL(t1)] ; += 1
    psrlw xmm0, 1; >>= 1
    packuswb xmm0, xmm4

    paddusw xmm1, [GLOBAL(t2)] ; += 2
    psrlw xmm1, 2; >>= 2
    packuswb xmm1, xmm4

    punpcklbw xmm0, xmm1

    pshufb xmm0, [GLOBAL(reverse)]

    movd out0, xmm0
    psrlq xmm0, 16
    movd out1, xmm0
    psrlq xmm0, 16

    movd out3, xmm5

    pslld xmm5, 16
    por xmm0, xmm5
    movd out2, xmm0

    ret

SECTION_RODATA
align 16
t1:
    times 8 dw 0x1
align 16
t2:
    times 8 dw 0x2
align 16
t4:
    dw 0x4
align 16
tm_above_extract:
    dw 0x0400
    dw 0x0401
    dw 0x0402
    dw 0x0403
    dw 0x0400
    dw 0x0401
    dw 0x0402
    dw 0x0403
align 16
tm_top_left_extract:
    times 8 dw 0x0400
align 16
tm_left_extract:
    dw 0x0
    dw 0x0
    dw 0x0101
    dw 0x0101
    dw 0x0202
    dw 0x0202
    dw 0x0303
    dw 0x0303
align 16
tm_he_dup_extract:
    dw 0x0400
    dw 0x0400
    dw 0x0401
    dw 0x0402
    dw 0x0400
    dw 0x0400
    dw 0x0401
    dw 0x0402
align 16
ld_dup_last:
    dw 0x0802
    dw 0x0803
    dw 0x0804
    dw 0x0805
    dw 0x0806
    dw 0x0807
    dw 0x0807
    dw 0x0808 ; empty
align 16
first_element:
    dw 0x00FF
    times 7 dw 0x0000
align 16
reverse:
    dw 0x0706
    dw 0x0504
    dw 0x0302
    dw 0x0808
    dw 0x0808
    dw 0x0808
    dw 0x0808
    dw 0x0808
