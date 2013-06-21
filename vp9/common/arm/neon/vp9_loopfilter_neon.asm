;
;  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;

    EXPORT  |vp9_loop_filter_horizontal_edge_neon|
    EXPORT  |vp9_loop_filter_vertical_edge_neon|
    ARM

    AREA ||.text||, CODE, READONLY, ALIGN=2

; Currently vp9 only works on iterations 8 at a time. The vp8 loop filter
; works on 16 iterations at a time.
; TODO(fgalligan): See about removing the count code as this function is only
; called with a count of 1.
;
; void vp9_loop_filter_horizontal_edge_neon(uint8_t *s,
;                                           int p /* pitch */,
;                                           const uint8_t *blimit,
;                                           const uint8_t *limit,
;                                           const uint8_t *thresh,
;                                           int count)
;
; r0    uint8_t *s,
; r1    int p, /* pitch */
; r2    const uint8_t *blimit,
; r3    const uint8_t *limit,
; sp    const uint8_t *thresh,
; sp+4  int count
|vp9_loop_filter_horizontal_edge_neon| PROC
    push        {r4-r6, lr}

    ldrb        r4, [r2,#0]                ; r4 = *blimit
    ldr         r6, [sp,#20]               ; r6 = count
    ldrb        r5, [r3,#0]                ; r5 = *limit
    cmp         r6, #0
    beq         end_vp9_lf_h_edge

count_lf_h_loop
    vdup.u8     d0, r4                     ; duplicate blimit
    vdup.u8     d1, r5                     ; duplicate limit
    sub         r2, r0, r1, lsl #2         ; move src pointer down by 4 lines
    ldr         r3, [sp, #16]              ; load thresh
    add         r12, r2, r1
    add         r1, r1, r1

    vdup.u8     d2, r3                     ; duplicate thresh

    vld1.u8     {d3}, [r2@64], r1          ; p3
    vld1.u8     {d4}, [r12@64], r1         ; p2
    vld1.u8     {d5}, [r2@64], r1          ; p1
    vld1.u8     {d6}, [r12@64], r1         ; p0
    vld1.u8     {d7}, [r2@64], r1          ; q0
    vld1.u8     {d8}, [r12@64], r1         ; q1
    vld1.u8     {d9}, [r2@64]              ; q2
    vld1.u8     {d10}, [r12@64]            ; q3

    sub         r2, r2, r1, lsl #1
    sub         r12, r12, r1, lsl #1

    bl          vp9_loop_filter_neon

    vst1.u8     {d5}, [r2@64], r1          ; store op1
    vst1.u8     {d6}, [r12@64], r1         ; store op0
    vst1.u8     {d7}, [r2@64], r1          ; store oq0
    vst1.u8     {d8}, [r12@64], r1         ; store oq1

    add         r0, r0, #8
    subs        r6, r6, #1
    bne         count_lf_h_loop

end_vp9_lf_h_edge
    pop         {r4-r6, pc}
    ENDP        ; |vp9_loop_filter_horizontal_edge_neon|

; Currently vp9 only works on iterations 8 at a time. The vp8 loop filter
; works on 16 iterations at a time.
; TODO(fgalligan): See about removing the count code as this function is only
; called with a count of 1.
;
; void vp9_loop_filter_vertical_edge_neon(uint8_t *s,
;                                         int p /* pitch */,
;                                         const uint8_t *blimit,
;                                         const uint8_t *limit,
;                                         const uint8_t *thresh,
;                                         int count)
;
; r0    uint8_t *s,
; r1    int p, /* pitch */
; r2    const uint8_t *blimit,
; r3    const uint8_t *limit,
; sp    const uint8_t *thresh,
; sp+4  int count
|vp9_loop_filter_vertical_edge_neon| PROC
    push        {r4-r6, lr}

    ldrb        r4, [r2,#0]                ; r4 = *blimit
    ldr         r6, [sp,#20]               ; r6 = count
    ldrb        r5, [r3,#0]                ; r5 = *limit
    cmp         r6, #0
    beq         end_vp9_lf_v_edge

count_lf_v_loop
    vdup.u8     d0, r4                     ; duplicate blimit
    sub         r12, r0, #4                ; move s pointer down by 4 columns
    vdup.u8     d1, r5                     ; duplicate limit

    vld1.u8     {d3}, [r12], r1            ; load s data
    vld1.u8     {d4}, [r12], r1
    vld1.u8     {d5}, [r12], r1
    vld1.u8     {d6}, [r12], r1
    vld1.u8     {d7}, [r12], r1
    vld1.u8     {d8}, [r12], r1
    vld1.u8     {d9}, [r12], r1
    vld1.u8     {d10}, [r12]

    ldr        r12, [sp, #16]              ; load thresh

    ;transpose to 8x16 matrix
    vtrn.32     d3, d7
    vtrn.32     d4, d8
    vtrn.32     d5, d9
    vtrn.32     d6, d10

    vdup.u8     d2, r12                    ; duplicate thresh

    vtrn.16     d3, d5
    vtrn.16     d4, d6
    vtrn.16     d7, d9
    vtrn.16     d8, d10

    vtrn.8      d3, d4
    vtrn.8      d5, d6
    vtrn.8      d7, d8
    vtrn.8      d9, d10

    bl          vp9_loop_filter_neon

    sub         r0, r0, #2

    ;store op1, op0, oq0, oq1
    vst4.8      {d5[0], d6[0], d7[0], d8[0]}, [r0], r1
    vst4.8      {d5[1], d6[1], d7[1], d8[1]}, [r0], r1
    vst4.8      {d5[2], d6[2], d7[2], d8[2]}, [r0], r1
    vst4.8      {d5[3], d6[3], d7[3], d8[3]}, [r0], r1
    vst4.8      {d5[4], d6[4], d7[4], d8[4]}, [r0], r1
    vst4.8      {d5[5], d6[5], d7[5], d8[5]}, [r0], r1
    vst4.8      {d5[6], d6[6], d7[6], d8[6]}, [r0], r1
    vst4.8      {d5[7], d6[7], d7[7], d8[7]}, [r0]

    add         r0, r0, r1, lsl #3        ; s += pitch * 8
    subs        r6, r6, #1
    bne         count_lf_v_loop

end_vp9_lf_v_edge
    pop         {r4-r6, pc}
    ENDP        ; |vp9_loop_filter_vertical_edge_neon|

; void vp9_loop_filter_neon();
; This is a helper function for the loopfilters. The individual functions do
; the necessary load, transpose (if necessary) and store.
;
; r0-r3 PRESERVE
; d0    flimit
; d1    limit
; d2    thresh
; d3    p3
; d4    p2
; d5    p1
; d6    p0
; d7    q0
; d8    q1
; d9    q2
; d10   q3
|vp9_loop_filter_neon| PROC
    ; vp9_filter_mask
    vabd.u8     d11, d3, d4                 ; abs(p3 - p2)
    vabd.u8     d12, d4, d5                 ; abs(p2 - p1)
    vabd.u8     d13, d5, d6                 ; abs(p1 - p0)
    vabd.u8     d14, d8, d7                 ; abs(q1 - q0)
    vabd.u8     d3, d9, d8                  ; abs(q2 - q1)
    vabd.u8     d4, d10, d9                 ; abs(q3 - q2)

    vmax.u8     d11, d11, d12
    vmax.u8     d12, d13, d14
    vmax.u8     d3, d3, d4
    vmax.u8     d15, d11, d12

    vabd.u8     d9, d6, d7                  ; abs(p0 - q0)

    ; vp9_hevmask
    vcgt.u8     d13, d13, d2                ; (abs(p1 - p0) > thresh)*-1
    vcgt.u8     d14, d14, d2                ; (abs(q1 - q0) > thresh)*-1
    vmax.u8     d15, d15, d3

    vmov.u8     d10, #0x80                  ; 0x80

    vabd.u8     d2, d5, d8                  ; a = abs(p1 - q1)
    vqadd.u8    d9, d9, d9                  ; b = abs(p0 - q0) * 2

    vcge.u8     d15, d1, d15

    ; vp9_filter() function
    ; convert to signed
    veor        d7, d7, d10                 ; qs0
    vshr.u8     d2, d2, #1                  ; a = a / 2
    veor        d6, d6, d10                 ; ps0

    veor        d5, d5, d10                 ; ps1
    vqadd.u8    d9, d9, d2                  ; a = b + a

    veor        d8, d8, d10                 ; qs1

    vmov.u8     d10, #3                     ; #3

    vsub.s8     d2, d14, d12                ; ( qs0 - ps0)

    vcge.u8     d9, d0, d9                  ; (a > flimit * 2 + limit) * -1

    vqsub.s8    d1, d5, d8                  ; vp9_filter = clamp(ps1-qs1)
    vorr        d14, d13, d14               ; vp9_hevmask

    vmull.s8    q11, d2, d10                ; 3 * ( qs0 - ps0)

    vand        d1, d1, d14                 ; vp9_filter &= hev
    vand        d15, d15, d9                ; vp9_filter_mask

    vaddw.s8    q11, q11, d1

    vmov.u8     d9, #4                      ; #4

    ; vp9_filter = clamp(vp9_filter + 3 * ( qs0 - ps0))
    vqmovn.s16  d1, q11

    vand        d1, d1, d15                 ; vp9_filter &= mask

    vqadd.s8    d2, d1, d10                 ; Filter2 = clamp(vp9_filter+3)
    vqadd.s8    d1, d1, d9                  ; Filter1 = clamp(vp9_filter+4)
    vshr.s8     d2, d2, #3                  ; Filter2 >>= 3
    vshr.s8     d1, d1, #3                  ; Filter1 >>= 3


    vqadd.s8    d11, d6, d2                 ; u = clamp(ps0 + Filter2)
    vqsub.s8    d10, d7, d1                 ; u = clamp(qs0 - Filter1)

    ; outer tap adjustments: ++vp9_filter >> 1
    vrshr.s8    d1, d1, #1
    vbic        d1, d1, d14                 ; vp9_filter &= ~hev
    vmov.u8     d0, #0x80                   ; 0x80
    vqadd.s8    d13, d5, d1                 ; u = clamp(ps1 + vp9_filter)
    vqsub.s8    d12, d8, d1                 ; u = clamp(qs1 - vp9_filter)

    veor        d6, d11, d0                 ; *op0 = u^0x80
    veor        d7, d10, d0                 ; *oq0 = u^0x80
    veor        d5, d13, d0                 ; *op1 = u^0x80
    veor        d8, d12, d0                 ; *oq1 = u^0x80

    bx          lr
    ENDP        ; |vp9_loop_filter_neon|

    END
