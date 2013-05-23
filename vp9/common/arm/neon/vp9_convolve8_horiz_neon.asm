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
    ; This function is only valid when x_step_q4 == 16
    ; and w%4 and h%4 == 0
    push            {r4-r10, lr}

    ldr             r4, [sp, #32]           ; filter_x
    ldr             r5, [sp, #36]           ; x_step_q4
    cmp             r5, #16
    bne             call_c_convolve         ; x_step_q4 != 16

    ldr             r6, [sp, #48]           ; w
    ldr             r7, [sp, #52]           ; h

    vld1.s16        {q0}, [r4]              ; filter_x

    add             r8, r1, r1, lsl #1      ; src_stride * 3
    add             r8, r8, #4              ; src_stride * 3 + 4
    sub             r0, r0, #3

    mov             r10, r6                 ; w loop counter

loop
    vld4.u8         {d24[0], d25[0], d26[0], d27[0]}, [r0]!
    vld4.u8         {d24[4], d25[4], d26[4], d27[4]}, [r0]!
    vld3.u8         {d28[0], d29[0], d30[0]}, [r0]

    sub             r0, r0, #8
    add             r0, r0, r1

    vld4.u8         {d24[1], d25[1], d26[1], d27[1]}, [r0]!
    vld4.u8         {d24[5], d25[5], d26[5], d27[5]}, [r0]!
    vld3.u8         {d28[1], d29[1], d30[1]}, [r0]

    sub             r0, r0, #8
    add             r0, r0, r1

    vld4.u8         {d24[2], d25[2], d26[2], d27[2]}, [r0]!
    vld4.u8         {d24[6], d25[6], d26[6], d27[6]}, [r0]!
    vld3.u8         {d28[2], d29[2], d30[2]}, [r0]

    sub             r0, r0, #8
    add             r0, r0, r1

    vld4.u8         {d24[3], d25[3], d26[3], d27[3]}, [r0]!
    vld4.u8         {d24[7], d25[7], d26[7], d27[7]}, [r0]!
    vld3.u8         {d28[3], d29[3], d30[3]}, [r0]

    sub             r0, r8                  ; src - src_stride * 3 + 4

    ; extract to s16
    vmovl.u8        q8, d24
    vmovl.u8        q9, d25
    vmovl.u8        q10, d26
    vmovl.u8        q11, d27
    vtrn.32         d28, d29 ; only the first half is populated
    vmovl.u8        q12, d28
    vmovl.u8        q13, d30

    ; src[0] * filter_x
    vmull.s16       q1, d16, d0[0]
    vmlal.s16       q1, d18, d0[1]
    vmlal.s16       q1, d20, d0[2]
    vmlal.s16       q1, d22, d0[3]
    vmlal.s16       q1, d17, d1[0]
    vmlal.s16       q1, d19, d1[1]
    vmlal.s16       q1, d21, d1[2]
    vmlal.s16       q1, d23, d1[3]

    ; src[1] * filter_x
    vmull.s16       q2, d18, d0[0]
    vmlal.s16       q2, d20, d0[1]
    vmlal.s16       q2, d22, d0[2]
    vmlal.s16       q2, d17, d0[3]
    vmlal.s16       q2, d19, d1[0]
    vmlal.s16       q2, d21, d1[1]
    vmlal.s16       q2, d23, d1[2]
    vmlal.s16       q2, d24, d1[3]

    ; src[2] * filter_x
    vmull.s16       q14, d20, d0[0]
    vmlal.s16       q14, d22, d0[1]
    vmlal.s16       q14, d17, d0[2]
    vmlal.s16       q14, d19, d0[3]
    vmlal.s16       q14, d21, d1[0]
    vmlal.s16       q14, d23, d1[1]
    vmlal.s16       q14, d24, d1[2]
    vmlal.s16       q14, d25, d1[3]

    ; src[3] * filter_x
    vmull.s16       q15, d22, d0[0]
    vmlal.s16       q15, d17, d0[1]
    vmlal.s16       q15, d19, d0[2]
    vmlal.s16       q15, d21, d0[3]
    vmlal.s16       q15, d23, d1[0]
    vmlal.s16       q15, d24, d1[1]
    vmlal.s16       q15, d25, d1[2]
    vmlal.s16       q15, d26, d1[3]

    ; += 64 >> 7
    vqrshrun.s32    d2, q1, #7
    vqrshrun.s32    d3, q2, #7
    vqrshrun.s32    d4, q14, #7
    vqrshrun.s32    d5, q15, #7

    ; saturate
    vqshrn.u16      d2, q1, #0
    vqshrn.u16      d3, q2, #0

    ; transpose
    vtrn.16         d2, d3
    vtrn.32         d2, d3
    vtrn.8          d2, d3

    vst1.u32        {d2[0]}, [r2], r3
    vst1.u32        {d3[0]}, [r2], r3
    vst1.u32        {d2[1]}, [r2], r3
    vst1.u32        {d3[1]}, [r2], r3

    sub             r2, r2, r3, lsl #2      ; dst - dst_stride * 4
    add             r2, r2, #4

    subs            r6, r6, #4              ; w -= 4
    bgt             loop

    ; outer loop
    mov             r6, r10                 ; restore w counter
    sub             r0, r0, r10             ; src -= w
    add             r0, r0, r1, lsl #2      ; src += src_stride * 4
    sub             r2, r2, r10             ; dst -= w
    add             r2, r2, r3, lsl #2      ; dst += dst_stride * 4
    subs            r7, r7, #4              ; h -= 4
    bgt loop

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
