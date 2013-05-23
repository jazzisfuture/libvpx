;
;  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;


    EXPORT  |vp9_convolve8_horiz_neon|
    IMPORT  |vp9_convolve8_horiz_c|
    ARM
    REQUIRE8
    PRESERVE8

    AREA ||.text||, CODE, READONLY, ALIGN=2
; r0    const uint8_t *src
; r1    int src_stride
; r2    uint8_t *dst
; r3    int dst_stride
; sp[]const int16_t *filter_x
; sp[]int x_step_q4
; sp[]const int16_t *filter_y ; unused
; sp[]int y_step_q4           ; unused
; sp[]int w
; sp[]int h

; TODO(wwcv): assert at build time
; taps = 8
; VP9_FILTER_WEIGHT = 128
; VP9_FILTER_SHIFT = 7

|vp9_convolve8_horiz_neon| PROC
    ; This function is only valid when x_step_q4 == 16 and filter_x[3] != 128
    push            {r4-r10, lr}

    ldr             r4, [sp, #32]           ; filter_x
    ;ldr             r12, [r4, #6]           ; filter_x[3]
    ldr             r5, [sp, #36]           ; x_step_q4
    cmp             r5, #16
    bne             call_c_convolve         ; x_step_q4 != 16
    ; Unlike the x86, the neon code promotes src to 16 instead of compressing
    ; filter to 8. This means it can handle the case of filter_x[3] == 128
    ;cmp             r12, #128
    ;beq             call_c_convolve         ; filer_x[3] == 128

    ldr             r6, [sp, #48]           ; w
    ldr             r7, [sp, #52]           ; h

    vld1.s16        {q0}, [r4]              ; filter_x

    sub             r0, r0, #3              ; src -= 3
    sub             r1, r1, r6              ; src_stride - w
    sub             r3, r3, r6              ; dst_stride - w

    mov             r8, r6                  ; w loop counter
    mov             r9, #1                  ; post increment

loop
    vld1.u8         {d2}, [r0], r9          ; src++
    vmovl.u8        q1, d2                  ; extract to s16

    vmul.i16        q1, q0, q1              ; * filter_x
    ; sum the elements
    vpaddl.s16      q1, q1
    vpaddl.s32      q1, q1
    vadd.s64        d2, d2, d3

    vqrshrun.s64    d2, q1, #7              ; vrshr should round the appropriate amount
    ; saturate
    vqshrn.u32      d2, q1, #0
    vqshrn.u16      d2, q1, #0

    vst1.u8         {d2[0]}, [r2], r9

    subs            r8, r8, #1              ; w -= 1
    bgt             loop

    ; outer loop
    mov             r8, r6                  ; restore w counter
    add             r0, r1                  ; src += src_stride
    add             r2, r3                  ; dst += dst_stride
    subs            r7, r7, #1              ; h -= 1
    bgt             loop

    pop             {r4-r10, pc}

call_c_convolve
    ldr             r8, [sp, #48]           ; w
    ldr             r9, [sp, #52]           ; h
    push            {r4-r9}
    bl              vp9_convolve8_horiz_c
    pop             {r4-r9}

    pop             {r4-r10, pc}

    ENDP
    END
