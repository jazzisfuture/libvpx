;
;  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;


    EXPORT  |vp9_convolve8_vert_neon|
    IMPORT  |vp9_convolve8_vert_c|
    ARM
    REQUIRE8
    PRESERVE8

    AREA ||.text||, CODE, READONLY, ALIGN=2
; r0    const uint8_t *src
; r1    int src_stride
; r2    uint8_t *dst
; r3    int dst_stride
; sp[]const int16_t *filter_x ; unused
; sp[]int x_step_q4           ; unused
; sp[]const int16_t *filter_y
; sp[]int y_step_q4
; sp[]int w
; sp[]int h

; TODO(wwcv): assert at build time
; taps = 8
; VP9_FILTER_WEIGHT = 128
; VP9_FILTER_SHIFT = 7

|vp9_convolve8_vert_neon| PROC
    ; This function is only valid when x_step_q4 == 16 and filter_x[3] != 128
    push            {r4-r10, lr}

    ldr             r6, [sp, #40]           ; filter_y
    ldr             r7, [sp, #44]           ; y_step_q4
    cmp             r7, #16
    bne             call_c_convolve         ; y_step_q4 != 16
    ;b             call_c_convolve         ; y_step_q4 != 16

    ldr             r8, [sp, #48]           ; w
    ldr             r9, [sp, #52]           ; h

    vld1.s16        {q0}, [r6]              ; filter_y

    add             r4, r1, r1, lsl #1      ; stride * 3
    add             r5, r4, r1, lsl #3          ; stride * 11
    sub             r0, r0, r4              ; src -= 3 * stride

    sub             r4, r1, r8

    mov             r10, r8                  ; w loop counter

loop
    ; always process a 4x4 block at a time
    vld1.u32        {d16[0]}, [r0], r1
    vld1.u32        {d16[1]}, [r0], r1
    vld1.u32        {d18[0]}, [r0], r1
    vld1.u32        {d18[1]}, [r0], r1
    vld1.u32        {d20[0]}, [r0], r1
    vld1.u32        {d20[1]}, [r0], r1
    vld1.u32        {d22[0]}, [r0], r1
    vld1.u32        {d22[1]}, [r0], r1
    vld1.u32        {d24[0]}, [r0], r1
    vld1.u32        {d24[1]}, [r0], r1
    vld1.u32        {d26[0]}, [r0], r1

    subs            r0, r0, r5              ; reset src
    add             r0, r0, #4              ; prepare for next row

    ; extract to s16
    vmovl.u8        q8, d16
    vmovl.u8        q9, d18
    vmovl.u8        q10, d20
    vmovl.u8        q11, d22
    vmovl.u8        q12, d24
    vmovl.u8        q13, d26

    ; src[0] * filter_y
    vmov.i32        q1, #0
    vmlal.s16       q1, d16, d0[0]
    vmlal.s16       q1, d17, d0[1]
    vmlal.s16       q1, d18, d0[2]
    vmlal.s16       q1, d19, d0[3]
    vmlal.s16       q1, d20, d1[0]
    vmlal.s16       q1, d21, d1[1]
    vmlal.s16       q1, d22, d1[2]
    vmlal.s16       q1, d23, d1[3]

    ; src[stride] * filter_y
    vmov.i32        q2, #0
    vmlal.s16       q2, d17, d0[0]
    vmlal.s16       q2, d18, d0[1]
    vmlal.s16       q2, d19, d0[2]
    vmlal.s16       q2, d20, d0[3]
    vmlal.s16       q2, d21, d1[0]
    vmlal.s16       q2, d22, d1[1]
    vmlal.s16       q2, d23, d1[2]
    vmlal.s16       q2, d24, d1[3]

    ; src[2 * stride] * filter_y
    vmov.i32        q14, #0
    vmlal.s16       q14, d18, d0[0]
    vmlal.s16       q14, d19, d0[1]
    vmlal.s16       q14, d20, d0[2]
    vmlal.s16       q14, d21, d0[3]
    vmlal.s16       q14, d22, d1[0]
    vmlal.s16       q14, d23, d1[1]
    vmlal.s16       q14, d24, d1[2]
    vmlal.s16       q14, d25, d1[3]

    ; src[3 * stride] * filter_y
    vmov.i32        q15, #0
    vmlal.s16       q15, d19, d0[0]
    vmlal.s16       q15, d20, d0[1]
    vmlal.s16       q15, d21, d0[2]
    vmlal.s16       q15, d22, d0[3]
    vmlal.s16       q15, d23, d1[0]
    vmlal.s16       q15, d24, d1[1]
    vmlal.s16       q15, d25, d1[2]
    vmlal.s16       q15, d26, d1[3]

    vqrshrun.s32     d2, q1, #7 ; + 64 >> 7
    vqrshrun.s32     d3, q2, #7 ; + 64 >> 7
    vqrshrun.s32     d4, q14, #7 ; + 64 >> 7
    vqrshrun.s32     d5, q15, #7 ; + 64 >> 7
    ; saturate
    vqshrn.u16       d2, q1, #0
    vqshrn.u16       d3, q2, #0

    vst1.u32         {d2[0]}, [r2], r3
    vst1.u32         {d2[1]}, [r2], r3

    vst1.u32         {d3[0]}, [r2], r3
    vst1.u32         {d3[1]}, [r2], r3

    sub        r2, r2, r3, lsl #2
    add        r2, r2, #4

    subs            r8, r8, #4              ; w -= 4
    bgt             loop

    ; outer loop
    mov             r8, r10                 ; restore w counter
    sub             r0, r0, r8              ; src -= w
    add             r0, r0, r1, lsl #2      ; src += 4 * src_stride
    sub             r2, r2, r8              ; dst -= w
    add             r2, r2, r3, lsl #2      ; dst += 4 * dst_stride
    subs            r9, r9, #4              ; h -= 4
    bgt             loop

    pop             {r4-r10, pc}

call_c_convolve
    ldr             r8, [sp, #48]           ; w
    ldr             r9, [sp, #52]           ; h
    push            {r4-r9}
    bl              vp9_convolve8_vert_c
    pop             {r4-r9}

    pop             {r4-r10, pc}

    ENDP
    END
