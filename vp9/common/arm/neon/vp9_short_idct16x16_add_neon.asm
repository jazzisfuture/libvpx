;
;  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;

    EXPORT  |vp9_short_idct16x16_add_neon_pass1|
    EXPORT  |vp9_short_idct16x16_add_neon_pass2|
    ARM
    REQUIRE8
    PRESERVE8

    AREA ||.text||, CODE, READONLY, ALIGN=2

    ; Transpose a 8x8 16bit data matrix. Datas are loaded in q8-q15.
    MACRO
    TRANSPOSE8X8
    vswp            d17, d24
    vswp            d23, d30
    vswp            d21, d28
    vswp            d19, d26
    vtrn.32         q8, q10
    vtrn.32         q9, q11
    vtrn.32         q12, q14
    vtrn.32         q13, q15
    vtrn.16         q8, q9
    vtrn.16         q10, q11
    vtrn.16         q12, q13
    vtrn.16         q14, q15
    MEND

    AREA    Block, CODE, READONLY ; name this block of code
;void |vp9_short_idct16x16_add_neon_pass1|(int16_t *input, int16_t *output, int output_stride)
;
; r0  int16_t input
; r1  int16_t *output
; r2  int  output_stride)

; idct16 stage1 - stage6 on all the elements loaded in q8-q15. The output
; will be stored back into q8-q15 registers. This function will touch q0-q7
; registers and use them as buffer during calculation.
|vp9_short_idct16x16_add_neon_pass1| PROC
    push            {r4-r9}

    ; load elements of 0, 2, 4, 6, 8, 10, 12, 14 into q8 - q15
    vld2.s16        {q8,q9}, [r0]!
    vld2.s16        {q9,q10}, [r0]!
    vld2.s16        {q10,q11}, [r0]!
    vld2.s16        {q11,q12}, [r0]!
    vld2.s16        {q12,q13}, [r0]!
    vld2.s16        {q13,q14}, [r0]!
    vld2.s16        {q14,q15}, [r0]!
    vld2.s16        {q0,q1}, [r0]!
    vmov.s16        q15, q0

    ; transpose the input data
    TRANSPOSE8X8

    ; generate  cospi_28_64 = 3196
    mov             r3, #0xc00
    add             r3, #0x7c

    ; generate cospi_4_64  = 16069
    mov             r4, #0x3e00
    add             r4, #0xc5

    ; generate cospi_12_64 = 13623
    mov             r5, #0x3500
    add             r5, #0x37

    ; generate cospi_20_64 = 9102
    mov             r6, #0x2300
    add             r6, #0x8e

    ; generate cospi_16_64 = 11585
    mov             r7, #0x2d00
    add             r7, #0x41

    ; generate cospi_24_64 = 6270
    mov             r8, #0x1800
    add             r8, #0x7e

    ; generate cospi_8_64 = 15137
    mov             r9, #0x3b00
    add             r9, #0x21

    ; stage 3
    vdup.16         d0, r3;                   ; duplicate cospi_28_64
    vdup.16         d1, r4;                   ; duplicate cospi_4_64

    ; step2[4] * cospi_28_64 = input[2] * cospi_28_64
    vmull.s16       q2, d18, d0
    vmull.s16       q3, d19, d0

    ; step2[7] * cospi_4_64 = input[14] * cospi_4_64
    vmull.s16       q4, d30, d1
    vmull.s16       q5, d31, d1

    ; step2[4] * cospi_28_64 - step2[7] * cospi_4_64
    vsub.s32        q6, q2, q4
    vsub.s32        q7, q3, q5

    ; dct_const_round_shift(temp1);
    vqrshrn.s32     d8, q6, #14               ; >> 14
    vqrshrn.s32     d9, q7, #14               ; >> 14

    ; step2[4] * cospi_4_64
    vmull.s16       q2, d18, d1
    vmull.s16       q3, d19, d1

    ; step2[7] * cospi_4_64
    vmull.s16       q9, d30, d0
    vmull.s16       q15, d31, d0

    ; step2[4] * cospi_4_64 + step2[7] * cospi_28_64
    vadd.s32        q2, q2, q9
    vadd.s32        q3, q3, q15

    ; dct_const_round_shift(temp2);
    vqrshrn.s32     d14, q2, #14              ; >> 14
    vqrshrn.s32     d15, q3, #14              ; >> 14

    vdup.16         d0, r5;                   ; duplicate cospi_12_64
    vdup.16         d1, r6;                   ; duplicate cospi_20_64

    ; step2[5] * cospi_12_64 = input[10] * cospi_12_64
    vmull.s16       q2, d26, d0
    vmull.s16       q3, d27, d0

    ; step2[6] * cospi_20_64 = input[6] * cospi_20_64
    vmull.s16       q5, d22, d1
    vmull.s16       q6, d23, d1

    ; input[5] * cospi_12_64 - input[3] * cospi_20_64
    vsub.s32        q2, q2, q5
    vsub.s32        q3, q3, q6

    ; dct_const_round_shift(temp1);
    vqrshrn.s32     d10, q2, #14              ; >> 14
    vqrshrn.s32     d11, q3, #14              ; >> 14

    ; step2[5] * cospi_20_64
    vmull.s16       q2, d26, d1
    vmull.s16       q3, d27, d1

    ; step2[6] * cospi_12_64
    vmull.s16       q13, d22, d0
    vmull.s16       q11, d23, d0

    ; step2[5] * cospi_20_64 + step2[6] * cospi_12_64
    vadd.s32        q0, q2, q13
    vadd.s32        q1, q3, q11

    ; dct_const_round_shift(input_dc * cospi_16_64)
    vqrshrn.s32     d12, q0, #14              ; >> 14
    vqrshrn.s32     d13, q1, #14              ; >> 14

    ;The following instruction may not be needed, but they will make
    ; the data more easy to manage as data will be grouped in q0 - q7.
    vmov.s16        q0, q8
    vmov.s16        q1, q12
    vmov.s16        q2, q10
    vmov.s16        q3, q14

    ; stage 4
    vdup.16         d30, r7;                   ; cospi_16_64

    ; step1[0] * cospi_16_64 = input[0] * cospi_16_64
    vmull.s16       q10, d0, d30
    vmull.s16       q11, d1, d30

    ; step1[1] * cospi_16_64 = input[8] * cospi_16_64
    vmull.s16       q12, d2, d30
    vmull.s16       q13, d3, d30

    ; (step1[0] + step1[1]) * cospi_16_64;
    vadd.s32        q14, q10, q12
    vadd.s32        q15, q11, q13

    ; step2[0] = dct_const_round_shift(temp1)
    vqrshrn.s32     d16, q14, #14             ; >> 14
    vqrshrn.s32     d17, q15, #14             ; >> 14

    ; (step1[0] - step1[1]) * cospi_16_64
    vsub.s32        q14, q10, q12
    vsub.s32        q15, q11, q13

    ; dct_const_round_shift(input_dc * cospi_16_64)
    vqrshrn.s32     d18, q14, #14             ; >> 14
    vqrshrn.s32     d19, q15, #14             ; >> 14

    ; step1[2] * cospi_24_64 - step1[3] * cospi_8_64;
    vdup.16         d30, r8;                  ; duplicate cospi_24_64
    vdup.16         d31, r9;                  ; duplicate cospi_8_64

    ; step1[2] * cospi_24_64
    vmull.s16       q0, d4, d30
    vmull.s16       q1, d5, d30

    ; step1[3] * cospi_8_64
    vmull.s16       q11, d6, d31
    vmull.s16       q12, d7, d31

    ; input[1] * cospi_24_64 - input[3] * cospi_8_64
    vsub.s32        q0, q0, q11
    vsub.s32        q1, q1, q12

    ; dct_const_round_shift(input_dc * cospi_16_64)
    vqrshrn.s32     d20, q0, #14              ; >> 14
    vqrshrn.s32     d21, q1, #14              ; >> 14

    ; step1[2] * cospi_8_64
    vmull.s16       q0, d4, d31
    vmull.s16       q1, d5, d31

    ; step1[3] * cospi_24_64
    vmull.s16       q11, d6, d30
    vmull.s16       q12, d7, d30

    ; input[1] * cospi_8_64 + input[3] * cospi_24_64
    vadd.s32        q2, q0, q11
    vadd.s32        q3, q1, q12

    ; dct_const_round_shift(temp2);
    vqrshrn.s32     d22, q2, #14              ; >> 14
    vqrshrn.s32     d23, q3, #14              ; >> 14

    vadd.s16        q12, q4, q5               ; step2[4] = step1[4] + step1[5];
    vsub.s16        q13, q4, q5               ; step2[5] = step1[4] - step1[5];
    vsub.s16        q14, q7, q6               ; step2[6] = -step1[6] + step1[7];
    vadd.s16        q15, q6, q7               ; step2[7] = step1[6] + step1[7];

    ; stage 5
    vadd.s16        q0, q8, q11               ; step1[0] = step2[0] + step2[3];
    vadd.s16        q1, q9, q10               ; step1[1] = step2[1] + step2[2];
    vsub.s16        q2, q9, q10               ; step1[2] = step2[1] - step2[2];
    vsub.s16        q3, q8, q11               ; step1[3] = step2[0] - step2[3];
    vmov.s16        q4, q12                   ; step1[4] = step2[4]

    vdup.16         d16, r7;                  ; duplicate cospi_16_64

    ; step2[6] * cospi_16_64
    vmull.s16       q9, d28, d16
    vmull.s16       q10, d29, d16

    ; step2[5] * cospi_16_64
    vmull.s16       q11, d26, d16
    vmull.s16       q12, d27, d16

    ; (step2[6] - step2[5]) * cospi_16_64
    vsub.s32        q6, q9, q11
    vsub.s32        q7, q10, q12

    ; step1[5] = dct_const_round_shift(temp1);
    vqrshrn.s32     d10, q6, #14              ; >> 14
    vqrshrn.s32     d11, q7, #14              ; >> 14

    ; temp2 = (step2[5] + step2[6]) * cospi_16_64;
    vadd.s32        q9, q9, q11
    vadd.s32        q10, q10, q12

    ; step1[6] = dct_const_round_shift(temp2);
    vqrshrn.s32     d12, q9, #14              ; >> 14
    vqrshrn.s32     d13, q10, #14             ; >> 14

    ; step1[7] = step2[7];
    vmov.s16         q7, q15

    ; stage 6
    vadd.s16        q8, q0, q7;               ; step2[0] = step1[0] + step1[7];
    vadd.s16        q9, q1, q6;               ; step2[1] = step1[1] + step1[6];
    vadd.s16        q10, q2, q5;              ; step2[2] = step1[2] + step1[5];
    vadd.s16        q11, q3, q4;              ; step2[3] = step1[3] + step1[4];
    vsub.s16        q12, q3, q4;              ; step2[4] = step1[3] - step1[4];
    vsub.s16        q13, q2, q5;              ; step2[5] = step1[2] - step1[5];
    vsub.s16        q14, q1, q6;              ; step2[6] = step1[1] - step1[6];
    vsub.s16        q15, q0, q7;              ; step2[7] = step1[0] - step1[7];

    ; store the data
    vst1.64         {d16}, [r1], r2
    vst1.64         {d17}, [r1], r2
    vst1.64         {d18}, [r1], r2
    vst1.64         {d19}, [r1], r2
    vst1.64         {d20}, [r1], r2
    vst1.64         {d21}, [r1], r2
    vst1.64         {d22}, [r1], r2
    vst1.64         {d23}, [r1], r2
    vst1.64         {d24}, [r1], r2
    vst1.64         {d25}, [r1], r2
    vst1.64         {d26}, [r1], r2
    vst1.64         {d27}, [r1], r2
    vst1.64         {d28}, [r1], r2
    vst1.64         {d29}, [r1], r2
    vst1.64         {d30}, [r1], r2
    vst1.64         {d31}, [r1], r2

    pop             {r4-r9}
    bx              lr
    ENDP  ; |vp9_short_idct16x16_add_neon_pass1|

;void vp9_short_idct16x16_add_neon_pass2(int16_t *src,
;                                        int16_t *output,
;                                        int16_t *pass1Output,
;                                        int16_t skip_adding,
;                                        uint8_t *dest,
;                                        int dest_stride)
;
; r0  int16_t *src
; r1  int16_t *output,
; r2  int16_t *pass1Output,
; r3  int16_t skip_adding,
; r4  uint8_t *dest,
; r5  int dest_stride)

; idct16 stage1 - stage7 on all the elements loaded in q8-q15. The output
; will be stored back into q8-q15 registers. This function will touch q0-q7
; registers and use them as buffer during calculation.
|vp9_short_idct16x16_add_neon_pass2| PROC
    push            {r3-r9}

    ; load elements of 0, 2, 4, 6, 8, 10, 12, 14 into q8 - q15
    vld2.s16        {q8,q9}, [r0]!
    vld2.s16        {q9,q10}, [r0]!
    vld2.s16        {q10,q11}, [r0]!
    vld2.s16        {q11,q12}, [r0]!
    vld2.s16        {q12,q13}, [r0]!
    vld2.s16        {q13,q14}, [r0]!
    vld2.s16        {q14,q15}, [r0]!
    vld2.s16        {q0,q1}, [r0]!
    vmov.s16        q15, q0;

    ; transpose the input data
    TRANSPOSE8X8

    ; generate  cospi_30_64 = 1606
    mov             r3, #0x0600
    add             r3, #0x46

    ; generate cospi_2_64  = 16305
    mov             r4, #0x3f00
    add             r4, #0xb1

    ; generate cospi_14_64 = 12665
    mov             r5, #0x3100
    add             r5, #0x79

    ; generate cospi_18_64 = 10394
    mov             r6, #0x2800
    add             r6, #0x9a

    ; generate cospi_22_64 = 7723
    mov             r7, #0x1e00
    add             r7, #0x2b

    ; generate cospi_10_64 = 14449
    mov             r8, #0x3800
    add             r8, #0x71

    ; generate cospi_6_64 = 15679
    mov             r9, #0x3d00
    add             r9, #0x3f

    ; stage 3
    vdup.16         d12, r3;                  ; duplicate cospi_30_64
    vdup.16         d13, r4;                  ; duplicate cospi_2_64

    ; step1[8] * cospi_30_64 = input[1] * cospi_30_64
    vmull.s16       q2, d16, d12
    vmull.s16       q3, d17, d12

    ; step1[15] * cospi_2_64 = input[15] * cospi_2_64
    vmull.s16       q4, d30, d13
    vmull.s16       q5, d31, d13

    ; step1[8] * cospi_30_64 - step1[15] * cospi_2_64
    vsub.s32        q2, q2, q4
    vsub.s32        q3, q3, q5

    ; dct_const_round_shift(temp1);
    vqrshrn.s32     d0, q2, #14               ; >> 14
    vqrshrn.s32     d1, q3, #14               ; >> 14

    ; step1[8] * cospi_2_64
    vmull.s16       q2, d16, d13
    vmull.s16       q3, d17, d13

    ; step1[15] * cospi_30_64
    vmull.s16       q4, d30, d12
    vmull.s16       q5, d31, d12

    ; step1[8] * cospi_2_64 + step1[15] * cospi_30_64
    vadd.s32        q2, q2, q4
    vadd.s32        q3, q3, q5

    ; dct_const_round_shift(temp2);
    vqrshrn.s32     d14, q2, #14              ; >> 14
    vqrshrn.s32     d15, q3, #14              ; >> 14

    vdup.16         d30, r5;                  ; duplicate cospi_14_64
    vdup.16         d31, r6;                  ; duplicate cospi_18_64

    ; step1[9] * cospi_14_64 = input[9] * cospi_12_64
    vmull.s16       q2, d24, d30
    vmull.s16       q3, d25, d30

    ; step1[14] * cospi_18_64 = input[7] * cospi_20_64
    vmull.s16       q4, d22, d31
    vmull.s16       q5, d23, d31

    ; step1[9] * cospi_14_64 - step1[14] * cospi_18_64
    vsub.s32        q2, q2, q4
    vsub.s32        q3, q3, q5

    ; dct_const_round_shift(temp1);
    vqrshrn.s32     d2, q2, #14               ; >> 14
    vqrshrn.s32     d3, q3, #14               ; >> 14

    ; step1[9] * cospi_18_64
    vmull.s16       q2, d24, d31
    vmull.s16       q3, d25, d31

    ; step1[14] * cospi_14_64
    vmull.s16       q4, d22, d30
    vmull.s16       q5, d23, d30

    ; step1[9] * cospi_18_64 + step1[14] * cospi_14_64
    vadd.s32        q2, q2, q4
    vadd.s32        q3, q3, q5

    ; dct_const_round_shift(temp1)
    vqrshrn.s32     d12, q2, #14              ; >> 14
    vqrshrn.s32     d13, q3, #14              ; >> 14

    vdup.16         d30, r7;                  ; duplicate cospi_22_64
    vdup.16         d31, r8;                  ; duplicate cospi_10_64

    ; step1[10] * cospi_22_64 = input[5] * cospi_22_64
    vmull.s16       q11, d20, d30
    vmull.s16       q12, d21, d30

    ; step1[13] * cospi_10_64 = input[11] * cospi_10_64
    vmull.s16       q3, d26, d31
    vmull.s16       q4, d27, d31

    ; step1[10] * cospi_22_64 - step1[13] * cospi_10_64
    vsub.s32        q11, q11, q3
    vsub.s32        q12, q12, q4

    ; dct_const_round_shift(temp1);
    vqrshrn.s32     d4, q11, #14              ; >> 14
    vqrshrn.s32     d5, q12, #14              ; >> 14

    ; step1[10] * cospi_10_64
    vmull.s16       q11, d20, d31
    vmull.s16       q12, d21, d31

    ; step1[13] * cospi_22_64step1[14] * cospi_14_64
    vmull.s16       q3, d26, d30
    vmull.s16       q4, d27, d30

    ; step1[10] * cospi_10_64 + step1[13] * cospi_22_64
    vadd.s32        q11, q11, q3
    vadd.s32        q12, q12, q4

    ; dct_const_round_shift(input_dc * cospi_16_64)
    vqrshrn.s32     d10, q11, #14             ; >> 14
    vqrshrn.s32     d11, q12, #14             ; >> 14

    vdup.16         d30, r9;                  ; duplicate cospi_6_64

    ; generate cospi_26_64 = 4756
    mov             r9, #0x1200
    add             r9, #0x94
    vdup.16         d31, r9;                 ; duplicate cospi_26_64

    ; step1[11] * cospi_6_64 = input[13] * cospi_6_64
    vmull.s16       q10, d28, d30
    vmull.s16       q11, d29, d30

    ; step1[12] * cospi_26_64 = input[3] * cospi_26_64
    vmull.s16       q12, d18, d31
    vmull.s16       q13, d19, d31

    ; step1[11] * cospi_6_64 - step1[12] * cospi_26_64
    vsub.s32        q10, q10, q12
    vsub.s32        q11, q11, q13

    ; dct_const_round_shift(temp1);
    vqrshrn.s32     d6, q10, #14              ; >> 14
    vqrshrn.s32     d7, q11, #14              ; >> 14

    ; step1[11] * cospi_26_64
    vmull.s16       q10, d28, d31
    vmull.s16       q11, d29, d31

    ; step1[12] * cospi_6_64
    vmull.s16       q12, d18, d30
    vmull.s16       q13, d19, d30

    ; step1[10] * cospi_10_64 + step1[13] * cospi_22_64
    vadd.s32        q10, q10, q12
    vadd.s32        q11, q11, q13

    ; dct_const_round_shift(input_dc * cospi_16_64)
    vqrshrn.s32     d8, q10, #14              ; >> 14
    vqrshrn.s32     d9, q11, #14              ; >> 14

    ; stage 3
    vadd.s16        q8, q0, q1                ; step1[8] = step2[8] + step2[9]
    vsub.s16        q9, q0, q1                ; step1[9] = step2[8] - step2[9]
    vsub.s16        q10, q3, q2               ; step1[10] = -step2[10] + step2[11]
    vadd.s16        q11, q2, q3               ; step1[11] = step2[10] + step2[11]
    vadd.s16        q12, q4, q5               ; step1[12] = step2[12] + step2[13]
    vsub.s16        q13, q4, q5               ; step1[13] = step2[12] - step2[13];
    vsub.s16        q14, q7, q6               ; step1[14] = -step2[14] + step2[15]
    vadd.s16        q15, q6, q7               ; step1[15] = step2[14] + step2[15]

    ; stage 4
    vmov.s16        q0, q8
    vmov.s16        q7, q15

    ; generate cospi_24_64 = 6270
    mov             r3, #0x1800
    add             r3, #0x7e

    ; generate cospi_8_64 = 15137
    mov             r4, #0x3b00
    add             r4, #0x21

    ; -step1[9] * cospi_8_64 + step1[14] * cospi_24_64;
    vdup.16         d30, r4;                  ; duplicate cospi_8_64
    vdup.16         d31, r3;                  ; duplicate cospi_24_64

    ; step1[9] * cospi_8_64
    vmull.s16       q2, d18, d30
    vmull.s16       q3, d19, d30

    ; step1[14] * cospi_24_64
    vmull.s16       q4, d28, d31
    vmull.s16       q5, d29, d31

    ; -step1[9] * cospi_8_64 + step1[14] * cospi_24_64
    vsub.s32        q4, q4, q2
    vsub.s32        q5, q5, q3

    ; dct_const_round_shift(temp1)
    vqrshrn.s32     d2, q4, #14               ; >> 14
    vqrshrn.s32     d3, q5, #14               ; >> 14

    ; step1[9] * cospi_24_64
    vmull.s16       q2, d18, d31
    vmull.s16       q3, d19, d31

    ; step1[14] * cospi_8_64
    vmull.s16       q4, d28, d30
    vmull.s16       q5, d29, d30

    ; step1[9] * cospi_24_64 + step1[14] * cospi_8_64
    vadd.s32        q4, q2, q4
    vadd.s32        q5, q3, q5

    ; dct_const_round_shift(temp1)
    vqrshrn.s32     d12, q4, #14              ; >> 14
    vqrshrn.s32     d13, q5, #14              ; >> 14

    vmov.s16        q3, q11
    vmov.s16        q4, q12

    ; -step1[10] * cospi_24_64 - step1[13] * cospi_8_64
    mov              r5, #0
    sub              r5, r4
    vdup.16         d30, r5;                  ; duplicate -cospi_8_64

    ; -step1[10] * cospi_24_64
    vmull.s16       q8, d20, d31
    vmull.s16       q9, d21, d31

    ; - step1[13] * cospi_8_64
    vmull.s16       q11, d26, d30
    vmull.s16       q12, d27, d30

    ; -step1[10] * cospi_24_64 - step1[13] * cospi_8_64
    vsub.s32        q11, q11, q8
    vsub.s32        q12, q12, q9

    ; dct_const_round_shift(temp1)
    vqrshrn.s32     d4, q11, #14              ; >> 14
    vqrshrn.s32     d5, q12, #14              ; >> 14


    ; -step1[10] * cospi_8_64
    vmull.s16       q8, d20, d30
    vmull.s16       q9, d21, d30

    ; step1[13] * cospi_24_64
    vmull.s16       q11, d26, d31
    vmull.s16       q12, d27, d31

    ; -step1[10] * cospi_8_64 + step1[13] * cospi_24_64
    vadd.s32        q11, q8, q11
    vadd.s32        q12, q9, q12

    ; dct_const_round_shift(temp2)
    vqrshrn.s32     d10, q11, #14             ; >> 14
    vqrshrn.s32     d11, q12, #14             ; >> 14

    ; stage 5
    vadd.s16        q8, q0, q3                ; step1[8] = step2[8] + step2[11];
    vadd.s16        q9, q1, q2                ; step1[9] = step2[9] + step2[10];
    vsub.s16        q10, q1, q2               ; step1[10] = step2[9] - step2[10];
    vsub.s16        q11, q0, q3               ; step1[11] = step2[8] - step2[11];
    vsub.s16        q12, q7, q4               ; step1[12] = -step2[12] + step2[15];
    vsub.s16        q13, q6, q5               ; step1[13] = -step2[13] + step2[14];
    vadd.s16        q14, q6, q5               ; step1[14] = step2[13] + step2[14];
    vadd.s16        q15, q7, q4               ; step1[15] = step2[12] + step2[15];

    ; stage 6. The following instruction may not be needed, but they will make
    ; the data more easy to manage as data will be grouped in q0 - q7.
    vmov.s16        q0, q8
    vmov.s16        q1, q9
    vmov.s16        q6, q14
    vmov.s16        q7, q15

    ; generate cospi_16_64 = 11585
    mov             r7, #0x2d00
    add             r7, #0x41

    vdup.16         d30, r7;                  ; duplicate cospi_16_64

    ; step1[10] * cospi_16_64
    vmull.s16       q8, d20, d30
    vmull.s16       q9, d21, d30

    ; step1[13] * cospi_16_64
    vmull.s16       q3, d26, d30
    vmull.s16       q4, d27, d30

    ; (-step1[10] + step1[13]) * cospi_16_64;
    vsub.s32        q5, q3, q8
    vsub.s32        q14, q4, q9

    ; dct_const_round_shift(temp1)
    vqrshrn.s32     d4, q5, #14               ; >> 14
    vqrshrn.s32     d5, q14, #14              ; >> 14

    ; (step1[10] + step1[13]) * cospi_16_64
    vadd.s32        q5, q3, q8
    vadd.s32        q14, q4, q9

    ; dct_const_round_shift(temp2);
    vqrshrn.s32     d10, q5, #14              ; >> 14
    vqrshrn.s32     d11, q14, #14             ; >> 14

    ; (-step1[11] + step1[12]) * cospi_16_64;
    ; step1[11] * cospi_16_64
    vmull.s16       q8, d22, d30
    vmull.s16       q9, d23, d30

    ; step1[12] * cospi_16_64
    vmull.s16       q13, d24, d30
    vmull.s16       q14, d25, d30

    ; (-step1[11] + step1[12]) * cospi_16_64
    vsub.s32        q10, q13, q8
    vsub.s32        q4, q14, q9

    ; dct_const_round_shift(input_dc * cospi_16_64)
    vqrshrn.s32     d6, q10, #14              ; >> 14
    vqrshrn.s32     d7, q4, #14               ; >> 14


    ; (step1[11] + step1[12]) * cospi_16_64
    vadd.s32        q13, q13, q8
    vadd.s32        q14, q14, q9

    ; dct_const_round_shift(temp2);
    vqrshrn.s32     d8, q13, #14              ; >> 14
    vqrshrn.s32     d9, q14, #14              ; >> 14

    mov              r4, #16                  ; pass1Output stride
    ldr              r3, [sp]             ; load skip_adding
    cmp              r3, #0                   ; check if need adding dest data
    beq              skip_adding_dest

    ldr              r7, [sp, #28]            ; dest used to save element 0,1,2,3,4,5,6,7
    mov              r9, r7                   ; save dest pointer for later use
    ldr              r8, [sp, #32]            ; load dest_stride

    ; stage 7
    ; load the data in pass1
    vld1.s16        {q8}, [r2], r4            ; load data step2[0]
    vld1.s16        {q9}, [r2], r4            ; load data step2[1]
    vld1.s16        {q10}, [r2], r4           ; load data step2[2]
    vld1.s16        {q11}, [r2], r4           ; load data step2[3]
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vadd.s16        q12, q8, q7               ; step2[0] + step2[15]
    vadd.s16        q13, q9, q6               ; step2[1] + step2[14]
    vrshr.s16       q12, q12, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vrshr.s16       q13, q13, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vaddw.u8        q12, q12, d28             ;  + dest[j * dest_stride + i]
    vaddw.u8        q13, q13, d29             ;  + dest[j * dest_stride + i]
    vqmovun.s16     d28, q12                  ; clip pixel
    vqmovun.s16     d29, q13                  ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vst1.64         {d29}, [r9], r8           ; store the stata
    vsub.s16        q6, q9, q6                ; step2[1] - step2[14]
    vsub.s16        q7, q8, q7                ; step2[0] - step2[15]
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vadd.s16        q12, q10, q5              ; step2[2] + step2[13]
    vadd.s16        q13, q11, q4              ; step2[3] + step2[12]
    vrshr.s16       q12, q12, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vrshr.s16       q13, q13, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vaddw.u8        q12, q12, d28             ; + dest[j * dest_stride + i]
    vaddw.u8        q13, q13, d29             ; + dest[j * dest_stride + i]
    vqmovun.s16     d28, q12                  ; clip pixel
    vqmovun.s16     d29, q13                  ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vst1.64         {d29}, [r9], r8           ; store the stata
    vsub.s16        q4, q11, q4               ; step2[3] - step2[12]
    vsub.s16        q5, q10, q5               ; step2[2] - step2[13]
    vld1.s16        {q8}, [r2], r4            ; load data step2[4]
    vld1.s16        {q9}, [r2], r4            ; load data step2[5]
    vld1.s16        {q10}, [r2], r4           ; load data step2[6]
    vld1.s16        {q11}, [r2], r4           ; load data step2[7]
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vadd.s16        q12, q8, q3               ; step2[4] + step2[11]
    vadd.s16        q13, q9, q2               ; step2[5] + step2[10]
    vrshr.s16       q12, q12, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vrshr.s16       q13, q13, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vaddw.u8        q12, q12, d28             ; + dest[j * dest_stride + i]
    vaddw.u8        q13, q13, d29             ; + dest[j * dest_stride + i]
    vqmovun.s16     d28, q12                  ; clip pixel
    vqmovun.s16     d29, q13                  ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vst1.64         {d29}, [r9], r8           ; store the stata
    vsub.s16        q2, q9, q2                ; step2[5] - step2[10]
    vsub.s16        q3, q8, q3                ; step2[4] - step2[11]
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vadd.s16        q12, q10, q1              ; step2[6] + step2[9]
    vadd.s16        q13, q11, q0              ; step2[7] + step2[8]
    vrshr.s16       q12, q12, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vrshr.s16       q13, q13, #6              ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vaddw.u8        q12, q12, d28             ; + dest[j * dest_stride + i]
    vaddw.u8        q13, q13, d29             ; + dest[j * dest_stride + i]
    vqmovun.s16     d28, q12                  ; clip pixel
    vqmovun.s16     d29, q13                  ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vst1.64         {d29}, [r9], r8           ; store the stata
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vsub.s16        q0, q11, q0               ; step2[7] - step2[8]
    vsub.s16        q1, q10, q1               ; step2[6] - step2[9]

    ; store the data  output 8,9,10,11,12,13,14,15
    vrshr.s16       q0, q0, #6                ; ROUND_POWER_OF_TWO(temp_out[j], 6)
    vaddw.u8        q0, q0, d28               ; + dest[j * dest_stride + i]
    vqmovun.s16     d28, q0                   ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vrshr.s16       q1, q1, #6
    vaddw.u8        q1, q1, d29               ; + dest[j * dest_stride + i]
    vqmovun.s16     d29, q1                   ; clip pixel
    vst1.64         {d29}, [r9], r8           ; store the stata
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vrshr.s16       q2, q2, #6
    vaddw.u8        q2, q2, d28               ; + dest[j * dest_stride + i]
    vqmovun.s16     d28, q2                   ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vrshr.s16       q3, q3, #6
    vaddw.u8        q3, q3, d29               ; + dest[j * dest_stride + i]
    vqmovun.s16     d29, q3                   ; clip pixel
    vst1.64         {d29}, [r9], r8           ; store the stata
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vrshr.s16       q4, q4, #6
    vaddw.u8        q4, q4, d28               ; + dest[j * dest_stride + i]
    vqmovun.s16     d28, q4                   ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vrshr.s16       q5, q5, #6
    vaddw.u8        q5, q5, d29               ; + dest[j * dest_stride + i]
    vqmovun.s16     d29, q5                   ; clip pixel
    vst1.64         {d29}, [r9], r8           ; store the stata
    vld1.64         {d29}, [r7], r8           ; load destinatoin data
    vrshr.s16       q6, q6, #6
    vaddw.u8        q6, q6, d28               ; + dest[j * dest_stride + i]
    vqmovun.s16     d28, q6                   ; clip pixel
    vst1.64         {d28}, [r9], r8           ; store the stata
    vld1.64         {d28}, [r7], r8           ; load destinatoin data
    vrshr.s16       q7, q7, #6
    vaddw.u8        q7, q7, d29               ; + dest[j * dest_stride + i]
    vqmovun.s16     d29, q7                   ; clip pixel
    vst1.64         {d29}, [r9], r8           ; store the stata
    b               end_idct16x16_pass2

skip_adding_dest
    ; stage 7
    ; load the data in pass1
    mov              r5, #24
    mov              r3, #8

    vld1.s16        {q8}, [r2], r4            ; load data step2[0]
    vld1.s16        {q9}, [r2], r4            ; load data step2[1]
    vld1.s16        {q10}, [r2], r4           ; load data step2[2]
    vld1.s16        {q11}, [r2], r4           ; load data step2[3]
    vadd.s16        q12, q8, q7               ; step2[0] + step2[15]
    vadd.s16        q13, q9, q6               ; step2[1] + step2[14]
    vst1.64         {d24}, [r1], r3           ; store output[0]
    vst1.64         {d25}, [r1], r5
    vst1.64         {d26}, [r1], r3           ; store output[1]
    vst1.64         {d27}, [r1], r5
    vsub.s16        q6, q9, q6                ; step2[1] - step2[14]
    vsub.s16        q7, q8, q7                ; step2[0] - step2[15]
    vadd.s16        q12, q10, q5              ; step2[2] + step2[13]
    vadd.s16        q13, q11, q4              ; step2[3] + step2[12]
    vst1.64         {d24}, [r1], r3           ; store output[2]
    vst1.64         {d25}, [r1], r5
    vst1.64         {d26}, [r1], r3           ; store output[3]
    vst1.64         {d27}, [r1], r5
    vsub.s16        q4, q11, q4               ; step2[3] - step2[12]
    vsub.s16        q5, q10, q5               ; step2[2] - step2[13]
    vld1.s16        {q8}, [r2], r4            ; load data step2[4]
    vld1.s16        {q9}, [r2], r4            ; load data step2[5]
    vld1.s16        {q10}, [r2], r4           ; load data step2[6]
    vld1.s16        {q11}, [r2], r4           ; load data step2[7]
    vadd.s16        q12, q8, q3               ; step2[4] + step2[11]
    vadd.s16        q13, q9, q2               ; step2[5] + step2[10]
    vst1.64         {d24}, [r1], r3           ; store output[4]
    vst1.64         {d25}, [r1], r5
    vst1.64         {d26}, [r1], r3           ; store output[5]
    vst1.64         {d27}, [r1], r5
    vsub.s16        q2, q9, q2                ; step2[5] - step2[10]
    vsub.s16        q3, q8, q3                ; step2[4] - step2[11]
    vadd.s16        q12, q10, q1              ; step2[6] + step2[9]
    vadd.s16        q13, q11, q0              ; step2[7] + step2[8]
    vst1.64         {d24}, [r1], r3           ; store output[6]
    vst1.64         {d25}, [r1], r5
    vst1.64         {d26}, [r1], r3           ; store output[7]
    vst1.64         {d27}, [r1], r5
    vsub.s16        q0, q11, q0               ; step2[7] - step2[8]
    vsub.s16        q1, q10, q1               ; step2[6] - step2[9]

    ; store the data  output 8,9,10,11,12,13,14,15
    vst1.64         {d0}, [r1], r3
    vst1.64         {d1}, [r1], r5
    vst1.64         {d2}, [r1], r3
    vst1.64         {d3}, [r1], r5
    vst1.64         {d4}, [r1], r3
    vst1.64         {d5}, [r1], r5
    vst1.64         {d6}, [r1], r3
    vst1.64         {d7}, [r1], r5
    vst1.64         {d8}, [r1], r3
    vst1.64         {d9}, [r1], r5
    vst1.64         {d10}, [r1], r3
    vst1.64         {d11}, [r1], r5
    vst1.64         {d12}, [r1], r3
    vst1.64         {d13}, [r1], r5
    vst1.64         {d14}, [r1], r3
    vst1.64         {d15}, [r1], r5

end_idct16x16_pass2
    pop             {r3-r9}
    bx              lr

    ENDP  ; |vp9_short_idct16x16_add_neon_pass2|
    END
