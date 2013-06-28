;
;  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;


    EXPORT  |vp9_copy_mem16x16_neon|
    ; ARM
    ; REQUIRE8
    ; PRESERVE8

    AREA    Block, CODE, READONLY ; name this block of code
;void vp9_copy_mem16x16_neon(const uint8_t *src, int src_pitch, uint8_t *dst, int dst_pitch)
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
|vp9_copy_mem16x16_neon| PROC

    vld1.u8     {q0}, [r0], r1
    vld1.u8     {q1}, [r0], r1
    vld1.u8     {q2}, [r0], r1
    vst1.u8     {q0}, [r2], r3
    vld1.u8     {q3}, [r0], r1
    vst1.u8     {q1}, [r2], r3
    vld1.u8     {q0}, [r0], r1
    vst1.u8     {q2}, [r2], r3
    vld1.u8     {q1}, [r0], r1
    vst1.u8     {q3}, [r2], r3
    vld1.u8     {q2}, [r0], r1
    vst1.u8     {q0}, [r2], r3
    vld1.u8     {q3}, [r0], r1
    vst1.u8     {q1}, [r2], r3
    vld1.u8     {q8}, [r0], r1
    vst1.u8     {q2}, [r2], r3
    vld1.u8     {q9}, [r0], r1
    vst1.u8     {q3}, [r2], r3
    vld1.u8     {q10}, [r0], r1
    vst1.u8     {q8}, [r2], r3
    vld1.u8     {q11}, [r0], r1
    vst1.u8     {q9}, [r2], r3
    vld1.u8     {q12}, [r0], r1
    vst1.u8     {q10}, [r2], r3
    vld1.u8     {q13}, [r0], r1
    vst1.u8     {q11}, [r2], r3
    vld1.u8     {q14}, [r0], r1
    vst1.u8     {q12}, [r2], r3
    vld1.u8     {q15}, [r0], r1
    vst1.u8     {q13}, [r2], r3
    vst1.u8     {q14}, [r2], r3
    vst1.u8     {q15}, [r2], r3

    mov     pc, lr

    ENDP  ; |vp9_copy_mem16x16_neon|

    END
