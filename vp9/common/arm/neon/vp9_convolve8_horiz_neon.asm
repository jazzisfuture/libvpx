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
    push            {r4-r7, lr}
    ENDP
    END
