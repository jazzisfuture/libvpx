;
;  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license and patent
;  grant that can be found in the LICENSE file in the root of the source
;  tree. All contributing project authors may be found in the AUTHORS
;  file in the root of the source tree.
;


    EXPORT  |vp9_dc_only_idct_add_neon|
    ARM
    REQUIRE8
    PRESERVE8

    AREA ||.text||, CODE, READONLY, ALIGN=2

;void vp9_dc_only_idct_add_c(int input_dc, uint8_t *pred_ptr,
;                            uint8_t *dst_ptr, int pitch, int stride)
; r0  input_dc
; r1  pred_ptr
; r2  dst_ptr
; r3  pitch
; sp  stride

|vp9_dc_only_idct_add_neon| PROC

    mov              r9, #0x2d00
    add              r9, #0x41
    ldr              r12, [sp]
    mul              r0, r0, r9
    add              r0, r0, #0x2000
    asr              r0, r0, #14
    mul              r0, r0, r9
    add              r0, r0, #0x2000
    asr              r0, r0, #14
    add              r0, r0, #8
    asr              r0, r0, #4
    cmp              r0, #0
    vdup.16         q0, r0;

    vld1.32         {d2[0]}, [r1], r3
    vld1.32         {d2[1]}, [r1], r3
    vld1.32         {d4[0]}, [r1], r3
    vld1.32         {d4[1]}, [r1]

    vaddw.u8        q1, q0, d2
    vaddw.u8        q2, q0, d4

    vqmovun.s16     d2, q1
    vqmovun.s16     d4, q2

    vst1.32         {d2[0]}, [r2], r12
    vst1.32         {d2[1]}, [r2], r12
    vst1.32         {d4[0]}, [r2], r12
    vst1.32         {d4[1]}, [r2]

    bx              lr

    ENDP

    END
