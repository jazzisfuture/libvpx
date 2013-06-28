;
;  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;


    EXPORT  |vp9_copy_mem8x4_neon|
    ; ARM
    ; REQUIRE8
    ; PRESERVE8

    AREA    Block, CODE, READONLY ; name this block of code
;void vp9_copy_mem8x4_neon(const uint8_t *src, int src_pitch, uint8_t *dst, int dst_pitch)
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
|vp9_copy_mem8x4_neon| PROC
    vld1.u8     {d0}, [r0], r1
    vld1.u8     {d1}, [r0], r1
    vst1.u8     {d0}, [r2], r3
    vld1.u8     {d2}, [r0], r1
    vst1.u8     {d1}, [r2], r3
    vld1.u8     {d3}, [r0], r1
    vst1.u8     {d2}, [r2], r3
    vst1.u8     {d3}, [r2], r3

    mov     pc, lr

    ENDP  ; |vp9_copy_mem8x4_neon|

    END
