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

;void vp8_intra4x4_predict_ssse3
;(
;    unsigned char *src,
;    int            src_stride,
;    int            b_mode,
;    unsigned char *dst,
;    int            dst_stride
;)
global sym(vp8_intra4x4_predict_ssse3)
sym(vp8_intra4x4_predict_ssse3):

%if ABI_IS_32BIT
  push        rbp
  mov         rbp, rsp
  GET_GOT     rbx
  push        rsi
  push        rdi

    %define     src         rcx
    %define     src_stride  rdx
    %define     low_byte    al

    mov         src, arg(0)
    mov         src_stride, arg(1)

    %define     b_mode      rax
    %define     dst         rsi
    %define     dst_stride  rdi
%else
  %ifidn __OUTPUT_FORMAT__,x64
    %define     src         rax
    %define     src_stride  rcx
    %define     b_mode      rdx
    %define     dst         r8
    %define     dst_stride  r9

    %define     low_byte    r10b
  %else
    %define     src         rdi
    %define     src_stride  rsi
    %define     b_mode      rdx
    %define     dst         rcx
    %define     dst_stride  r8

    %define     low_byte    r9b
  %endif
%endif

    ; defined in vp8/common/block.h and guarded in
    ; vp8/common/asm_com_offsets.c
    %define     B_DC_PRED   0
    %define     B_TM_PRED   1
    %define     B_VE_PRED   2
    %define     B_HE_PRED   3
    %define     B_LD_PRED   4
    %define     B_RD_PRED   5
    %define     B_VR_PRED   6
    %define     B_VL_PRED   7
    %define     B_HD_PRED   8
    %define     B_HU_PRED   9

    ;ALIGN_STACK 16, rax                    ; don't need to align stack. it
                                            ; isn't used with aligned
                                            ; instructions

    sub         rsp, 24                     ; when generating pp[] we only need
                                            ; to write out 13 bytes but must
                                            ; write out 24 (movdqu [pp+4])
    %define     pp          rsp             ; pp[13]

    ; Typically, we don't bother macro-izing the return. However, this function
    ; returns after each case and has non-trivial cleanup.
    %macro CLEAN_AND_RETURN 0
      add         rsp, 24
      %if ABI_IS_32BIT
        pop         rdi
        pop         rsi
        RESTORE_GOT
        pop         rbp
      %endif
      ret
    %endmacro

    sub         src, 1                      ; start in Left[0]
    ; Grab Left[2] so we can move src. Otherwise the offset is a pain
    mov         low_byte, [src + 2 * src_stride] ; Left[2]
    sub         src, src_stride             ; back up to top_left
    ; We could try and save a cycle by postponing this write but in 32bit the
    ; low_byte is overloaded as b_mode
    mov         [pp + 1], low_byte          ; pp[1]

    movdqu      xmm4, [src]                 ; top_left A[0-7]
    movdqu      [pp + 4], xmm4

%if ABI_IS_32BIT
    mov         b_mode, arg(2)
    mov         dst, arg(3)
    mov         dst_stride, arg(4)
%endif

; If we don't need Left, jump to the front of the line. Only VE, LD, and VL
; will work
    cmp         b_mode, B_VE_PRED
    je                 .B_VE_PRED
    cmp         b_mode, B_LD_PRED
    je                 .B_LD_PRED
    cmp         b_mode, B_VL_PRED
    je                 .B_VL_PRED

    mov         low_byte, [src + src_stride] ; Left[0]
    mov         [pp + 3], low_byte          ; pp[3]
    mov         low_byte, [src + 2 * src_stride] ; Left[1]
    mov         [pp + 2], low_byte          ; pp[2]


    mov         low_byte, [src + 4 * src_stride] ; Left[3]
    mov         [pp], low_byte              ; pp[0]


    ; pp[] = { Left[3-0], top_left, Above[0-7] }

%if ABI_IS_32BIT
    mov         b_mode, arg(2)
%endif

    cmp         b_mode, B_DC_PRED
    je                 .B_DC_PRED
    cmp         b_mode, B_TM_PRED
    je                 .B_TM_PRED
    cmp         b_mode, B_HE_PRED
    je                 .B_HE_PRED
    cmp         b_mode, B_RD_PRED
    je                 .B_RD_PRED
    cmp         b_mode, B_VR_PRED
    je                 .B_VR_PRED
    cmp         b_mode, B_HD_PRED
    je                 .B_HD_PRED
    cmp         b_mode, B_HU_PRED
    je                 .B_HU_PRED

    CLEAN_AND_RETURN

.B_DC_PRED:
    movd        xmm3, [GLOBAL(t4)]
    pxor        xmm4, xmm4                  ; zero
    movdqa      xmm5, [GLOBAL(t1)]

    movd        xmm0, [pp]                  ; L
    movd        xmm1, [pp + 5]              ; A

    psllq       xmm1, 32

    por         xmm0, xmm1

    punpcklbw   xmm0, xmm4

    pmaddwd     xmm0, xmm5

    packssdw    xmm0, xmm4

    pmaddwd     xmm0, xmm5

    packssdw    xmm0, xmm4

    pmaddwd     xmm0, xmm5

    paddusw     xmm0, xmm3

    psrlw       xmm0, 3

    pshufb      xmm0, xmm4

    movd        [dst], xmm0                 ; dst[0]
    add         dst, dst_stride
    movd        [dst], xmm0                 ; dst[1]
    movd        [dst + dst_stride], xmm0    ; dst[2]
    movd        [dst + 2 * dst_stride], xmm0 ; dst[3]

    CLEAN_AND_RETURN

.B_TM_PRED:
; because of the order of the math (a - b + c)
; we need to expand up to words and use signed
; operations, then pack back down to bytes
; exploit movd 0'ing 0x04
    pxor        xmm3, xmm3                  ; zero

    movd        xmm0, [pp]                  ; L
    movd        xmm1, [pp + 5]              ; A
    movd        xmm2, [pp + 4]              ; top_left

    pshufb      xmm0, [GLOBAL(tm_left_extract)]
    pshufb      xmm1, [GLOBAL(tm_above_extract)]
    pshufb      xmm2, [GLOBAL(tm_top_left_extract)]

    movdqa      xmm4, xmm0
    punpcklbw   xmm0, xmm3
    punpckhbw   xmm4, xmm3

    psubsw      xmm1, xmm2                  ; A - top_left
    paddsw      xmm0, xmm1                  ; (A - top_left) + L
    paddsw      xmm4, xmm1                  ; (A - top_left) + L

    packuswb    xmm0, xmm4

    pshufd      xmm1, xmm0, 0x0b

    movd        [dst], xmm1                 ; dst[0]
    add         dst, dst_stride
    psrlq       xmm1, 32
    movd        [dst], xmm1                 ; dst[1]

    movd        [dst + 2 * dst_stride], xmm0 ; dst[3]
    psrlq       xmm0, 32
    movd        [dst + dst_stride], xmm0    ; dst[2]

    CLEAN_AND_RETURN

.B_VE_PRED:
    movd        xmm0, [pp+5]                ; A[0-3]
    movd        xmm2, [pp+4]                ; top_left A[0-2]
    movd        xmm3, [pp+6]                ; A[1-4]
    pshufb      xmm0, [GLOBAL(tm_above_extract)]
    pshufb      xmm2, [GLOBAL(tm_above_extract)]
    pshufb      xmm3, [GLOBAL(tm_above_extract)]

    paddusw     xmm0, xmm0                  ; A[0-3] * 2
    paddusw     xmm0, xmm2                  ; += top_left A[0-2] (first element)
    paddusw     xmm0, xmm3                  ; += A[1-4] (third element)
    paddusw     xmm0, [GLOBAL(t2)]          ; += 2 (fourth element)
    psrlw       xmm0, 2                     ; >>= 2

    packuswb    xmm0, xmm0

    movd        [dst], xmm0                 ; dst[0]
    add         dst, dst_stride
    movd        [dst], xmm0                 ; dst[1]
    movd        [dst + dst_stride], xmm0    ; dst[2]
    movd        [dst + 2 * dst_stride], xmm0 ; dst[3]

    CLEAN_AND_RETURN

.B_HE_PRED:
    movd        xmm0, [pp]                  ; L[3-0]
    movdqa      xmm1, xmm0
    movd        xmm2, [pp+1]                ; L[2-0] top_left
    pshufb      xmm0, [GLOBAL(tm_above_extract)]
    pshufb      xmm1, [GLOBAL(tm_he_dup_extract)] ; L[3] L[3-1]
    pshufb      xmm2, [GLOBAL(tm_above_extract)]

    paddusw     xmm0, xmm0                  ; L[3-0] * 2
    paddusw     xmm0, xmm2                  ; += L[2-0] top_left(first element)
    paddusw     xmm0, xmm1                  ; += L[3] L[3-1] (third element)
    paddusw     xmm0, [GLOBAL(t2)]          ; += 2 (fourth element)
    psrlw       xmm0, 2                     ; >>= 2

    packuswb    xmm0, xmm0
    pshufb      xmm0, [GLOBAL(tm_left_extract)]

    pshufd      xmm1, xmm0, 0x0b

    movd        [dst], xmm1                 ; dst[0]
    add         dst, dst_stride
    psrlq       xmm1, 32
    movd        [dst], xmm1                 ; dst[1]

    movd        [dst + 2 * dst_stride], xmm0 ; dst[3]
    psrlq       xmm0, 32
    movd        [dst + dst_stride], xmm0    ; dst[2]

    CLEAN_AND_RETURN

.B_LD_PRED:
    pxor        xmm3, xmm3

    movq        xmm0, [pp+5]                ; A[0-6]
    movdqa      xmm1, xmm0
    movdqa      xmm2, xmm0

    psrlq       xmm1, 8                     ; A[1-7]

    punpcklbw   xmm0, xmm3
    punpcklbw   xmm1, xmm3
    pshufb      xmm2, [GLOBAL(ld_dup_last)] ; A[2-7] A[7]

    paddusw     xmm1, xmm1                  ; A[1-7] * 2
    paddusw     xmm1, xmm0                  ; += A[0-6] top_left(first element)
    paddusw     xmm1, xmm2                  ; += A[2-7] A[7] (third element)
    paddusw     xmm1, [GLOBAL(t2)]          ; += 2 (fourth element)
    psrlw       xmm1, 2                     ; >>= 2

    packuswb    xmm1, xmm1
    movd        [dst], xmm1                 ; dst[0]
    add         dst, dst_stride
    psrlq       xmm1, 8
    movd        [dst], xmm1                 ; dst[1]
    psrlq       xmm1, 8
    movd        [dst + dst_stride], xmm1    ; dst[2]
    psrlq       xmm1, 8
    movd        [dst + 2 * dst_stride], xmm1 ; dst[3]

    CLEAN_AND_RETURN

.B_RD_PRED:
; similar to LD_PRED. write out in reverse
    pxor        xmm3, xmm3

    movq        xmm0, [pp]                  ; L[3-0] top_left A[0-1]
    movq        xmm1, [pp + 1]              ; L[2-0] top_left A[0-2]
    movq        xmm2, [pp + 2]              ; L[1-0] top_left A[0-3]

    punpcklbw   xmm0, xmm3
    punpcklbw   xmm1, xmm3
    punpcklbw   xmm2, xmm3

    paddusw     xmm1, xmm1                  ; L[2-0] top_left A[0-2] * 2
    paddusw     xmm1, xmm0                  ; += L[3-0] top_left A[0-1] (first element)
    paddusw     xmm1, xmm2                  ; += L[1-0] top_left A[0-3] (third element)
    paddusw     xmm1, [GLOBAL(t2)]          ; += 2 (fourth element)
    psrlw       xmm1, 2                     ; >>= 2

    packuswb    xmm1, xmm3
    movdqa      xmm2, xmm1
    psrlq       xmm1, 24
    movd        [dst], xmm1                 ; dst[0]
    add         dst, dst_stride

    movd        [dst + 2 * dst_stride], xmm2 ; dst[3]
    psrlq       xmm2, 8
    movd        [dst + dst_stride], xmm2    ; dst[2]
    psrlq       xmm2, 8
    movd        [dst], xmm2                 ; dst[1]

    CLEAN_AND_RETURN

.B_VR_PRED:
    ; pp[1-8] = { L[2-0], top_left, A[0-3]
    pxor        xmm0, xmm0

    movq        xmm1, [pp + 1]              ; 123456
    movq        xmm2, [pp + 2]              ; 234567
    movq        xmm3, [pp + 3]              ; 345678

    punpcklbw   xmm1, xmm0
    punpcklbw   xmm2, xmm0
    punpcklbw   xmm3, xmm0

    paddusw     xmm3, xmm2                  ; 234567+345678
    movdqa      xmm4, xmm3
    paddusw     xmm3, xmm2                  ; 2*234567+345678
    paddusw     xmm3, xmm1                  ; 123456+2*234567+345678

    paddusw     xmm4, [GLOBAL(t1)]          ; 234567+345678+1
    psrlw       xmm4, 1                     ; 234567+345678+1>>1
    packuswb    xmm4, xmm0
    psrlq       xmm4, 16                    ; 4567+5678+1>>1

    movd        [dst], xmm4                 ; dst[0]
    add         dst, dst_stride

    psllq       xmm4, 8                     ; _456+_567+1>>1 out[2]

    paddusw     xmm3, [GLOBAL(t2)]          ; 123456+2*234567+345678+2
    psrlw       xmm3, 2                     ; 123456+2*234567+345678+2>>2
    packuswb    xmm3, xmm0

    movdqa      xmm1, xmm3
    movdqa      xmm2, xmm3
    psrlq       xmm2, 8
    pand        xmm1, [GLOBAL(first_element)] ; 1+2*2+3
    pand        xmm2, [GLOBAL(first_element)] ; 2+2*3+4

    psrlq       xmm3, 16
    movd        [dst], xmm3                 ; dst[1]

    por         xmm4, xmm2
    movd        [dst + dst_stride], xmm4    ; dst[2]

    psllq       xmm3, 8
    por         xmm3, xmm1
    movd        [dst + 2 * dst_stride], xmm3 ; dst[3]

    CLEAN_AND_RETURN

.B_VL_PRED:
    pxor        xmm3, xmm3

    movq        xmm0, [pp + 5]              ; A[0-5]
    movq        xmm1, [pp + 6]              ; A[1-6]
    movq        xmm2, [pp + 7]              ; A[2-7]

    punpcklbw   xmm0, xmm3
    punpcklbw   xmm1, xmm3
    punpcklbw   xmm2, xmm3

    paddusw     xmm0, xmm1                  ; A[0-5] + A[1-6] for 0/2/4/6
    paddusw     xmm1, xmm0                  ; += A[1-6] for 1/3/5/7/8/9
    paddusw     xmm1, xmm2                  ; += A[2-7]

    paddusw     xmm0, [GLOBAL(t1)]          ; += 1
    psrlw       xmm0, 1                     ; >>= 1
    packuswb    xmm0, xmm3
    movd        [dst], xmm0                 ; dst[0]

    add         dst, dst_stride

    paddusw     xmm1, [GLOBAL(t2)]          ; += 2
    psrlw       xmm1, 2                     ; >>= 2
    packuswb    xmm1, xmm3
    movd        [dst], xmm1                 ; dst[1]

    movdqa      xmm2, xmm1
    psrlq       xmm2, 32
    psllq       xmm2, 24
    psrld       xmm0, 8                     ; also clears high bits
    por         xmm0, xmm2
    movd        [dst + dst_stride], xmm0    ; dst[2]

    psrlq       xmm2, 32                    ; clear low bits, move to the bottom
    psllq       xmm2, 24                    ; put 8 in high
    psrld       xmm1, 8                     ; clear high bits
    por         xmm1, xmm2
    movd        [dst + 2 * dst_stride], xmm1 ; dst[3]

    CLEAN_AND_RETURN

.B_HD_PRED:
    pxor        xmm3, xmm3

    movq        xmm0, [pp]                  ; L[3-0] top_left A[0-5]
    movq        xmm1, [pp + 1]              ; A[1-6]
    movq        xmm2, [pp + 2]              ; A[2-7]

    punpcklbw   xmm0, xmm3
    punpcklbw   xmm1, xmm3
    punpcklbw   xmm2, xmm3

    paddusw     xmm0, xmm1                  ; A[0-3] + A[1-4] for 0/2/4/6
    paddusw     xmm1, xmm0                  ; += A[1-6] for 1/3/5/7/8/9
    paddusw     xmm1, xmm2                  ; += A[2-7]

    paddusw     xmm0, [GLOBAL(t1)]          ; += 1
    psrlw       xmm0, 1                     ; >>= 1
    packuswb    xmm0, xmm3

    paddusw     xmm1, [GLOBAL(t2)]          ; += 2
    psrlw       xmm1, 2                     ; >>= 2
    packuswb    xmm1, xmm3

    movdqa      xmm2, xmm1                  ; 8/9
    psrlq       xmm2, 32
    psllq       xmm2, 16

    punpcklbw   xmm0, xmm1

    movdqa      xmm1, xmm0
    psrlq       xmm0, 48
    por         xmm0, xmm2
    movd        [dst], xmm0
    add         dst, dst_stride

    movd        [dst + 2 * dst_stride], xmm1 ; dst[3]
    psrlq       xmm1, 16
    movd        [dst + dst_stride], xmm1    ; dst[2]
    psrlq       xmm1, 16
    movd        [dst], xmm1                 ; dst[1]

    CLEAN_AND_RETURN

.B_HU_PRED:
    pxor        xmm4, xmm4
    movd        xmm0, [pp]                  ; L[3-0]

    movdqa      xmm1, xmm0
    movdqa      xmm2, xmm0
    movdqa      xmm3, xmm0
    movdqa      xmm5, xmm0

    pslld       xmm1, 8                     ; L[3-1]
    pslld       xmm2, 16                    ; L[3-2]
    pslld       xmm3, 24                    ; L[3]
    psrld       xmm3, 16                    ; L[3]
    por         xmm2, xmm3                  ; L[3] L[3-2]

    pshufb       xmm5, [GLOBAL(tm_left_extract)] ; dup L[3]

    punpcklbw    xmm0, xmm4
    punpcklbw    xmm1, xmm4
    punpcklbw    xmm2, xmm4

    paddusw      xmm0, xmm1                 ; L[3-0] + L[2-0]
    paddusw      xmm1, xmm0                 ; += L[2-0]
    paddusw      xmm1, xmm2                 ; += L[1-0] L[0]

    paddusw      xmm0, [GLOBAL(t1)]         ; += 1
    psrlw        xmm0, 1                    ; >>= 1
    packuswb     xmm0, xmm4

    paddusw      xmm1, [GLOBAL(t2)]         ; += 2
    psrlw        xmm1, 2                    ; >>= 2
    packuswb     xmm1, xmm4

    punpcklbw    xmm0, xmm1

    pshufb       xmm0, [GLOBAL(reverse)]

    movd         [dst], xmm0                ; dst[0]
    add          dst, dst_stride
    psrlq        xmm0, 16
    movd         [dst], xmm0                ; dst[1]
    psrlq        xmm0, 16

    movd         [dst + 2 * dst_stride], xmm5 ; dst[3]

    pslld        xmm5, 16
    por          xmm0, xmm5
    movd         [dst + dst_stride], xmm0   ; dst[2]

    CLEAN_AND_RETURN

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
