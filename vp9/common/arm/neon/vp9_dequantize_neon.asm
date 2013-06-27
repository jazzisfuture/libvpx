;
;   Copyright (c) 2013 The WebM project authors. All Rights Reserved.
; 
;   Use of this source code is governed by a BSD-style license
;   that can be found in the LICENSE file in the root of the source
;   tree. An additional intellectual property rights grant can be found
;   in the file PATENTS.  All contributing project authors may
;   be found in the AUTHORS file in the root of the source tree.
;
 
    EXPORT |vp9_add_constant_residual_8x8_neon|
    EXPORT |vp9_add_constant_residual_16x16_neon|
    EXPORT |vp9_add_constant_residual_32x32_neon|
    ARM

    AREA ||.text||, CODE, READONLY, ALIGN=2

    MACRO
    LD_16x8 $src, $stride
    vld1.8              {q0},       [$src],     $stride
    vld1.8              {q1},       [$src],     $stride
    vld1.8              {q2},       [$src],     $stride
    vld1.8              {q3},       [$src],     $stride
    vld1.8              {q4},       [$src],     $stride
    vld1.8              {q5},       [$src],     $stride
    vld1.8              {q6},       [$src],     $stride
    vld1.8              {q7},       [$src],     $stride
    MEND

    MACRO
    ADD_DIFF_16x8 $diff
    vqadd.u8            q0,         q0,         $diff
    vqadd.u8            q1,         q1,         $diff
    vqadd.u8            q2,         q2,         $diff
    vqadd.u8            q3,         q3,         $diff
    vqadd.u8            q4,         q4,         $diff
    vqadd.u8            q5,         q5,         $diff
    vqadd.u8            q6,         q6,         $diff
    vqadd.u8            q7,         q7,         $diff
    MEND

    MACRO
    SUB_DIFF_16x8 $diff
    vqsub.u8            q0,         q0,         $diff
    vqsub.u8            q1,         q1,         $diff
    vqsub.u8            q2,         q2,         $diff
    vqsub.u8            q3,         q3,         $diff
    vqsub.u8            q4,         q4,         $diff
    vqsub.u8            q5,         q5,         $diff
    vqsub.u8            q6,         q6,         $diff
    vqsub.u8            q7,         q7,         $diff
    MEND

    MACRO
    ST_16x8 $dst, $stride
    vst1.8              {q0},       [$dst],     $stride
    vst1.8              {q1},       [$dst],     $stride
    vst1.8              {q2},       [$dst],     $stride
    vst1.8              {q3},       [$dst],     $stride
    vst1.8              {q4},       [$dst],     $stride
    vst1.8              {q5},       [$dst],     $stride
    vst1.8              {q6},       [$dst],     $stride
    vst1.8              {q7},       [$dst],     $stride
    MEND

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   Func:
;       vp9_add_constant_residual
;   Description:
;       add constant diff to pred and store to dest
;   Author:
;       Herman Chen(chm@rock-chips.com)
;   Function Arguments Register:
;       r0      : const int16_t diff
;       r1      : const uint8_t *pred
;       r2      : int pitch
;       r3      : uint8_t *dest
;       sp[0]   : int stride
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
|vp9_add_constant_residual_8x8_neon| PROC
    vld1.8              {d0},       [r1],       r2
    vld1.8              {d1},       [r1],       r2
    vld1.8              {d2},       [r1],       r2
    vld1.8              {d3},       [r1],       r2
    vld1.8              {d4},       [r1],       r2
    vld1.8              {d5},       [r1],       r2
    vld1.8              {d6},       [r1],       r2
    vld1.8              {d7},       [r1],       r2

    ldr                 r2,         [sp]                    ; r2: stride
    cmp                 r0,         #0
    bge                 DIFF_POSITIVE_8x8

DIFF_NEGTIVE_8x8                                            ; diff < 0
    rsbs                r0,         r0,         #0
    usat                r0,         #8,         r0
    vdup.u8             q4,         r0

    vqsub.u8            q0,         q0,         q4
    vqsub.u8            q1,         q1,         q4
    vqsub.u8            q2,         q2,         q4
    vqsub.u8            q3,         q3,         q4
    b                   DIFF_SAVE_8x8   

DIFF_POSITIVE_8x8                                           ; diff >= 0
    usat                r0,         #8,         r0
    vdup.u8             q4,         r0

    vqadd.u8            q0,         q0,         q4
    vqadd.u8            q1,         q1,         q4
    vqadd.u8            q2,         q2,         q4
    vqadd.u8            q3,         q3,         q4

DIFF_SAVE_8x8
    vst1.8              {d0},       [r3],       r2
    vst1.8              {d1},       [r3],       r2
    vst1.8              {d2},       [r3],       r2
    vst1.8              {d3},       [r3],       r2
    vst1.8              {d4},       [r3],       r2
    vst1.8              {d5},       [r3],       r2
    vst1.8              {d6},       [r3],       r2
    vst1.8              {d7},       [r3],       r2

    bx                  lr
    ENDP


|vp9_add_constant_residual_16x16_neon| PROC
    push                {r4,lr}
    LD_16x8             r1,         r2
    ldr                 r4,         [sp,#8]
    cmp                 r0,         #0
    bge                 DIFF_POSITIVE_16x16

|DIFF_NEGTIVE_16x16|
    rsbs                r0,         r0,         #0         
    usat                r0,         #8,         r0
    vdup.u8             q8,         r0

    SUB_DIFF_16x8       q8
    ST_16x8             r3,         r4
    LD_16x8             r1,         r2
    SUB_DIFF_16x8       q8
    b                   DIFF_SAVE_16x16

|DIFF_POSITIVE_16x16|
    usat                r0,         #8,         r0
    vdup.u8             q8,         r0

    ADD_DIFF_16x8       q8
    ST_16x8             r3,         r4
    LD_16x8             r1,         r2
    ADD_DIFF_16x8       q8

|DIFF_SAVE_16x16|
    ST_16x8             r3,         r4
    pop                 {r4,pc}
    ENDP


|vp9_add_constant_residual_32x32_neon| PROC
    push                {r4-r6,lr}
    ldr                 r4,         [sp,#16]
    pld                 [r1]
    add                 r5,         r1,         #16         ; r5 pred + 16 for second loop
    add                 r6,         r3,         #16         ; r6 dest + 16 for second loop
    cmp                 r0,         #0
    bge                 DIFF_POSITIVE_32x32

|DIFF_NEGTIVE_32x32|
    rsbs                r0,         r0,         #0         
    usat                r0,         #8,         r0
    vdup.u8             q8,         r0
    mov                 r0,         #4

|DIFF_NEGTIVE_32x32_LOOP|
    sub                 r0,         #1
    LD_16x8             r1,         r2
    SUB_DIFF_16x8       q8
    ST_16x8             r3,         r4

    LD_16x8             r1,         r2
    SUB_DIFF_16x8       q8
    ST_16x8             r3,         r4
    cmp                 r0,         #2
    moveq               r1,         r5
    moveq               r3,         r6
    cmp                 r0,         #0
    bne                 DIFF_NEGTIVE_32x32_LOOP
    pop                 {r4-r6,pc}

|DIFF_POSITIVE_32x32|
    usat                r0,         #8,         r0
    vdup.u8             q8,         r0
    mov                 r0,         #4

|DIFF_POSITIVE_32x32_LOOP|
    sub                 r0,         #1
    LD_16x8             r1,         r2
    ADD_DIFF_16x8       q8
    ST_16x8             r3,         r4

    LD_16x8             r1,         r2
    ADD_DIFF_16x8       q8
    ST_16x8             r3,         r4
    cmp                 r0,         #2
    moveq               r1,         r5
    moveq               r3,         r6
    cmp                 r0,         #0
    bne                 DIFF_POSITIVE_32x32_LOOP
    pop                 {r4-r6,pc}
    ENDP

    END
