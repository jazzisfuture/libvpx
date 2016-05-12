;
;  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
;
;  Use of this source code is governed by a BSD-style license
;  that can be found in the LICENSE file in the root of the source
;  tree. An additional intellectual property rights grant can be found
;  in the file PATENTS.  All contributing project authors may
;  be found in the AUTHORS file in the root of the source tree.
;

%include "third_party/x86inc/x86inc.asm"

SECTION_RODATA 16
zero:    times 16 db 0x0
one:     times 16 db 0x1
fe:      times 16 db  0xfe
ff:      times 16 db  0xff
te0:     times 16 db  0xe0
t80:     times 16 db 0x80
t7f:     times 16 db  0x7f
t1f:     times 16 db  0x1f
t4:      times 16 db  0x4
t3:      times 16 db  0x3
four:    times 8 dw  0x4
eight:   times 8 dw  0x8

SECTION .text
%define basestksize           0x380
%define stkreg rbp
%if ARCH_X86
%define q0                    0x000
%define p0                    q0+0x010
%define q1                    p0+0x010
%define p1                    q1+0x010
%define q2                    p1+0x010
%define p2                    q2+0x010
%define q3                    p2+0x010
%define p3                    q3+0x010
%define add32bstack           p3+0x010
%else
%define add32bstack           0x000
%endif

%define q4                    add32bstack
%define p4                    q4+0x010
%define q5                    p4+0x010
%define p5                    q5+0x010
%define q6                    p5+0x010
%define p6                    q6+0x010
%define q7                    p6+0x010
%define p7                    q7+0x010
%define mask                  p7+0x010
%define flat                  mask+0x010
%define flat2                 flat+0x010
%define hev                   flat2+0x010
%define org_p0                mask
%define org_q0                hev+0x010
%define org_p1                org_q0+0x010
%define org_q1                hev


%define flat2_lo              org_p1+0x010
%define flat2_hi              flat2_lo+0x010

%define p0_lo                 flat2_hi+0x010
%define p0_hi                 p0_lo+0x010
%define q0_lo                 p0_hi+0x010
%define q0_hi                 q0_lo+0x010
%define p1_lo                 q0_hi+0x010
%define p1_hi                 p1_lo+0x010
%define p2_lo                 p1_hi+0x010
%define p2_hi                 p2_lo+0x010
%define p3_lo                 p2_hi+0x010
%define p3_hi                 p3_lo+0x010
%define q1_lo                 p3_hi+0x010
%define q1_hi                 q1_lo+0x010
%define q2_lo                 q1_hi+0x010
%define q2_hi                 q2_lo+0x010
%define q3_lo                 q2_hi+0x010
%define q3_hi                 q3_lo+0x010

%define p4_lo                 flat2_lo
%define p4_hi                 flat2_hi

%define p5_lo                 q3_hi+0x010
%define p5_hi                 p5_lo+0x010
%define p6_lo                 p5_hi+0x010
%define p6_hi                 p6_lo+0x010


%define q4_lo                 p6_hi+0x010
%define q4_hi                 q4_lo+0x010
%define q5_lo                 q4_hi+0x010
%define q5_hi                 q5_lo+0x010
%define q6_lo                 q5_hi+0x010
%define q6_hi                 q6_lo+0x010


%define flat_p2               q6_hi+0x010
%define flat_p1               flat_p2+0x010
%define flat_p0               flat_p1+0x010
%define flat_q0               flat_p0+0x010
%define flat_q1               flat_q0+0x010
%define flat_q2               flat_q1+0x010


%define flat2_p6              flat_q2+0x010
%define flat2_p5              flat2_p6+0x010
%define flat2_p4              flat2_p5+0x010
%define flat2_p3              flat2_p4+0x010
%define flat2_p2              flat2_p3+0x010
%define flat2_p1              flat2_p2+0x010
%define flat2_p0              flat2_p1+0x010
%define flat2_q0              flat2_p0+0x010
%define flat2_q1              p6_lo
%define flat2_q2              p6_hi
%define flat2_q3              p5_lo
%define flat2_q4              p5_hi
%define flat2_q5              p4_lo

%macro ABS_SUB2 10
    mova              %1, %2
    mova              %3, %4
    psubusb           %5, %6
    psubusb           %7, %8
    psubusb           %9, %2
    psubusb          %10, %4
    por               %9, %5                           ; abs_diff(qi, q(i-1))
    por              %10, %7                           ; abs_diff(pi, p(i-1))
%endm

%macro ABS_SUB_ZERO 3
%if ARCH_X86
    mova              m3, m6
    mova              m1, m5
%else
    mova              m3, m8
    mova              m1, m9
%endif
    mova    [stkreg+q%3], %1
    mova    [stkreg+p%3], %2
    psubusb           m3, %1
    psubusb           m1, %2
%if ARCH_X86
    psubusb           %1, m6
    psubusb           %2, m5
%else
    psubusb           %1, m8
    psubusb           %2, m9
%endif
    por               %1, m3                           ; abs_diff(q5, p0)
    por               %2, m1                           ; abs_diff(p5, p0)
%endm

%macro ABS_SUB_Q0 3
    mova     [stkreg+q%3], %1
    mova     [stkreg+p%3], %2
    ABS_SUB2           m3, m6, m1, m5, m3, %1, m1, %2, %1, %2
%endm

%macro FLAT_COMP_PART1 11
    mova                  %3, %1
    mova                  %4, %2
    mova                  %5, %3
    mova                  %6, %4
    punpcklbw             %3, m7
    punpcklbw             %4, m7
    punpckhbw             %5, m7
    punpckhbw             %6, m7
%ifidn %9, flat                                        ; calc filter4
    mova      [stkreg+%10lo], %3
    paddw                 m0, %4                       ; sum_lo+=pi(low)
    mova      [stkreg+%10hi], %5
    paddw                 m2, %6                       ; sum_hi+=pi(high)
    mova      [stkreg+%11lo], %4
    paddw                 m0, %7                       ; sum_lo+= qj(low)
    movdqa    [stkreg+%11hi], %6
    paddw                 m2, %8                       ; sum_hi+= qj(high)
%else
    ; calc filter8
    mova      [stkreg+%10lo], %3
    paddw                 m0, %4                       ; sum_lo+=pi(low)
    mova      [stkreg+%10hi], %5
    paddw                 m1, %6                       ; sum_hi+=pi(high)
    mova      [stkreg+%11lo], %4
    paddw                 m0, %7                       ; sum_lo+= qj(low)
    movdqa    [stkreg+%11hi], %6
    paddw                 m1, %8                       ; sum_hi+= qj(high)
%endif
%endm


%macro FLATX_QIPI_OR_QIPI 6
    mova           m4, %5
    mova           %6, %5
    pand           %1, %5                              ; flat_qi if flat=1

    pandn          m4, %3                              ; qi if flat=0
    pand           %2, %5                              ; flat_pi if flat=1
    pandn          %6, %4                              ; pi if flat=0

    por            %1, m4                              ; save qi_tmp
    por            %6, %2                              ; save pi_tmp
%endm


%macro SAVE_FLAT2_QIPI_OR_QIPI 5
    sub                  src_qiq, pq
    lea                  src_piq, [src_piq + pq]
    FLATX_QIPI_OR_QIPI        %1, %2, %3, %4, m5, %5
    movu               [src_qiq], %1                   ; save qi
    movu               [src_piq], %5                   ; save pi
%endm

%macro FULL_SAVE_FLAT2_QIPI_OR_QIPI 3
%if %3 < 3
    mova                      m7, [stkreg+flat_q%3]
    mova                      m1, [stkreg+flat_p%3]
%if %3=2 && ARCH_X86_64
    FLATX_QIPI_OR_QIPI        m7, m1, m12, m13, m0, m3
%elif %3<2
    FLATX_QIPI_OR_QIPI        m7, m1, [stkreg+org_q%3], [stkreg+org_p%3], m0, \
    m3
%else
    FLATX_QIPI_OR_QIPI        m7, m1, [stkreg+q%3], [stkreg+p%3], m0, m3
%endif
%endif
    mova                      %1, [stkreg+flat2_q%3]
    mova                      %2, [stkreg+flat2_p%3]
%if %3 > 3
    SAVE_FLAT2_QIPI_OR_QIPI   %1, %2, [stkreg+q%3], [stkreg+p%3], m3
%else
%if %3=3 && ARCH_X86_64
    SAVE_FLAT2_QIPI_OR_QIPI   %1, %2, m14, m15, m3
%elif %3 < 3
    SAVE_FLAT2_QIPI_OR_QIPI   %1, %2, m7, m3, m1
%else
    SAVE_FLAT2_QIPI_OR_QIPI   %1, %2, [stkreg+q%3], [stkreg+p%3], m3
%endif
%endif
%endm

%macro FLATX_COMP_PART2 12
    paddw           %1, %3                             ; sum_lo+=q/pi(lo)
    paddw           %2, %4                             ; sum_hi+=q/pi(hi)
    mova            %5, %1
    mova            %6, %2
%if %12=1
    psrlw           %5, 0x4
    psubw           %1, %7                             ; sum_lo-=pj(lo)
    psrlw           %6, 0x4
%else
    psrlw           %5, 0x3
    psubw           %1, %7                             ; sum_lo-=pj(lo)
    psrlw           %6, 0x3
%endif
    psubw           %2, %8                             ; sum_hi-=pj(hi)
    packuswb        %5, %6
    psubw           %1, %9                             ; sum_lo-=q/pi(lo)
    psubw           %2, %10                            ; sum_hi-=q/pi(hi)
    mova  [stkreg+%11], %5
%endm

%macro FLAT_COMP_PART3 9
    mova            m1, %9                             ; only for filter4
    mova            m4, m1
    punpcklbw       m1, m7
    punpckhbw       m4, m7
    paddw           m5, m1                             ; sum_lo+=qi(lo)
    paddw           m3, m4                             ; sum_hi+=qi(hi)
    mova            %1, %2                             ; save qi(low)
    mova            %3, %4                             ; save qi(high)
    mova            %5, %6
    mova            %7, %8
%endm

%macro FLATX_COMP_PART4 8
    mova            %1, %2                             ; save q/pi(low)
    mova            %3, %4                             ; save q/pi(hi)
    paddw           %5, %6                             ; sum_lo+=qj(lo)
    paddw           %7, %8                             ; sum_hi+=qj(hi)
%endm

%macro FLAT2_COMP_PART5 3                              ; for filter8
    pxor            m7, m7
%ifnidn %1, q7                                         ; if not q7
    mova            m2, [stkreg+%1]                           ; load qi
    mova            m3, m2
    punpcklbw       m2, m7
    punpckhbw       m3, m7
    paddw           m0, m2                             ; sum_lo+=qi(lo)
    paddw           m1, m3                             ; sum_hi+=qi(hi)
    mova [stkreg+%2lo], m2
    mova [stkreg+%2hi], m3
%else
    mova            m4, [stkreg+%1]
    mova            m6, m4
    punpcklbw       m4, m7
    punpckhbw       m6, m7
    paddw           m0, m4                              ; sum_lo+=qi(lo)
    paddw           m1, m6                              ; sum_hi+=qi(hi)
%endif
    mova            m2, [stkreg+%3lo]
    mova            m3, [stkreg+%3hi]
%endm

%macro SET_SOURCE_QI_PI 0
%if ARCH_X86
   %define src_qiq r4q
   %define src_piq r5q
%else
   %define src_qiq  r5q
   %define src_piq  r6q
%endif
    mov        src_qiq, srcq
    mov        src_piq, srcq
    sub        src_piq, pq
%endm

INIT_XMM sse2
cglobal lpf_horizontal_edge_16, 6, 6+(ARCH_X86_64*1), \
8+(ARCH_X86_64*8), basestksize+(ARCH_X86*add32bstack), src, p, blimit, limit,\
thresh, count
    SET_SOURCE_QI_PI
%if ARCH_X86
    %xdefine limitm   [esp + stack_offset + 4*3 + 4]
    %xdefine threshm  [esp + stack_offset + 4*4 + 4]
    GET_GOT r3q
%if GET_GOT_DEFINED=1
    add rsp, gprsize
%endif
%endif
    xchg                        rbp, rsp

    movu                         m3, [src_qiq]         ; load q0
    movu                         m7, [src_piq]         ; load p0
    lea                     src_qiq, [src_qiq + pq]
    sub                     src_piq, pq
    movu                         m2, [src_qiq]         ; load q1
    movu                         m5, [src_piq]         ; load p1
    mova                         m6, m3
    mova                         m4, m7
    mova                         m1, m2
    mova                         m0, m5

%if ARCH_X86
    ABS_SUB2            [stkreg+q0], m3, [stkreg+p0], m7, m6, m2, m4, m5, m1, m0
%else
    ABS_SUB2                     m8, m3, m9, m7, m6, m2, m4, m5, m1, m0
%endif

    pmaxub                       m0, m1                ; max(|p1-p0|,|q1-q0|)
    mova              [stkreg+flat], m0
    mova                         m1, m7
    mova                         m6, m2
    mova                         m4, m5
%if ARCH_X86
    mova                [stkreg+q1], m2
    mova                [stkreg+p1], m5
%else
    mova                        m10, m2
    mova                        m11, m5
%endif
    psubusb                      m1, m3
    psubusb                      m3, m7
    psubusb                      m4, m2
    psubusb                      m6, m5
    por                          m1, m3                ; abs_diff(p0,q0)
    por                          m4, m6                ; abs_diff(p1,q1)
    lea                     src_qiq, [src_qiq + pq]
    sub                     src_piq, pq
    pand                         m4, [GLOBAL(fe)]
    paddusb                      m1, m1                ; 2*abs_diff(p0,q0)

    mova                         m6, [blimitq]
    psrlw                        m4, 1                 ; abs_diff(p1,q1)/2
    paddusb                      m4, m1
    movu                         m7, [src_qiq]         ; load p2
    movu                         m3, [src_piq]         ; load q2

    ; mask=2*|p0-q0|+(|p1-q1|<<1)-bilimit
    psubusb                      m4, m6
    pxor                         m6, m6
    mova                         m1, m7
    pcmpeqb                      m4, m6                ; mask=mask>0

    ; mask = _mm_xor_si128(mask, ff)
    pxor                         m4, [GLOBAL(ff)]
    mova                         m6, m3
    pmaxub                       m0, m4

%if ARCH_X86                                           ; |p2-p1|
    ABS_SUB2            [stkreg+q2], m7, [stkreg+p2], m3, m1, m2, m6, m5, m2, m5
%else
    ABS_SUB2                    m12, m7, m13, m3, m1, m2, m6, m5, m2, m5
%endif

    lea                     src_qiq, [src_qiq + pq]
    sub                     src_piq, pq

    ; mask=max(mask, abs_diff(p2,p1))
    pmaxub                       m0, m2
    movu                         m1, [src_qiq]         ; load q3
    movu                         m4, [src_piq]         ; load p3

    ; mask=max(mask, abs_diff(q2,q1))
    pmaxub                       m0, m5
    mova                         m2, m1
    mova                         m6, m4

%if ARCH_X86
    mov                         r2q, dword limitm
                                                       ; |p3-p2|
    ABS_SUB2            [stkreg+q3], m1, [stkreg+p3], m4, m2, m7, m6, m3, m7, m3
    pmaxub                       m0, m7
    mova                         m2, [r2q]

    ; mask=max(mask, abs_diff(p3,p2))
    pmaxub                       m0, m3
    mova                         m6, [stkreg+q0]
    mova                         m5, [stkreg+p0]

    ; mask=_mm_subs_epu8(mask, limit)
    psubusb                      m0, m2
    pxor                         m7, m7
    ; mask=_mm_cmpeq_epi8(mask, zero)
    pcmpeqb                      m0, m7
                                                       ; |p3-p0| , |q3-q0|
    ABS_SUB2                     m3, m6, m2, m5, m3, m1, m2, m4, m1, m4

    mova                         m7, [stkreg+q2]
    mova                         m2, [stkreg+p2]

    ; flat=max(|p3-p0|, |q3-q0|)
    pmaxub                       m4, m1
    ABS_SUB2                     m3, m6, m1, m5, m3, m7, m1, m2, m7, m2

    mov                         r4q, dword threshm
%else
                                                       ; |p3-p2|
    ABS_SUB2                    m14, m1, m15, m4, m2, m7, m6, m3, m7, m3

     ; mask=max(mask, abs_diff(q3,q2))
    pmaxub                       m0, m7
    mova                         m2, [limitq]

    ; mask=max(mask, abs_diff(p3,p2))
    pmaxub                       m0, m3
    ; mask=_mm_subs_epu8(mask, limit)
    psubusb                      m0, m2
    pxor                         m7, m7
    ; mask = _mm_cmpeq_epi8(mask, zero)
    pcmpeqb                      m0, m7
                                                       ; |p3-p0| , |q3-q0|
    ABS_SUB2                     m3, m8, m2, m9, m3, m1, m2, m4, m1, m4
    mova                         m7, m12
    mova                         m2, m13

    ; flat=max(|p3-p0|, |q3-q0|)
    pmaxub                       m4, m1
    ABS_SUB2                     m3, m8, m1, m9, m3, m7, m1, m2, m7, m2
%endif

    mova              [stkreg+mask], m0

    ; work=max(|p2-p0|, |q2-q0|)
    pmaxub                       m7, m2
    mova                         m1, [stkreg+flat]

    ; flat=_mm_max_epu8(flat, work)
    pmaxub                       m4, m7
    pxor                         m3, m3
%if ARCH_X86
    mova                         m2, [r4q]
%else
    mova                         m2, [threshq]
%endif
    ; flat= _mm_max_epu8(flat, work)
    pmaxub                       m4, m1
    ; flat = _mm_subs_epu8(flat, one);
    psubusb                      m4, [GLOBAL(one)]
    psubusb                      m1, m2
    pcmpeqb                      m4, m3                ; flat = flat > 0;

    ; hev = _mm_cmpeq_epi8(hev, zero)
    pcmpeqb                      m1, m3
%if ARCH_X86
    lea                     src_qiq, [srcq + 4*pq]
%else
    lea                     src_qiq, [src_qiq + pq]
%endif
    sub                     src_piq, pq
    pand                         m4, m0

    ; hev = _mm_xor_si128(hev, ff)
    pxor                         m1, [GLOBAL(ff)]
    mova              [stkreg+flat], m4
    mova               [stkreg+hev], m1
    movu                         m7, [src_qiq]         ; load q4
    movu                         m4, [src_piq]         ; load p4
    mova                [stkreg+q4], m7
    mova                [stkreg+p4], m4
    ABS_SUB_ZERO                 m7, m4, 4             ; |p4-p0|, |q4-q0|
    lea                     src_qiq, [src_qiq + pq]
    sub                     src_piq, pq
    movu                         m2, [src_qiq]         ; load q5
    movu                         m0, [src_piq]         ; load p5

    ; flat2=max(|q4-q0|, |p4-p0|)
    pmaxub                       m7, m4
    ABS_SUB_ZERO                 m2, m0, 5             ; |p5-p0|, |q5-q0|

    ; flat2=max(flat2, |p5-p0|)
    pmaxub                       m7, m2
    lea                     src_qiq, [src_qiq + pq]
    sub                     src_piq, pq

    ; flat2=max(flat2,|q5-q0|)
    pmaxub                       m7, m0
    movu                         m4, [src_qiq]         ; load q6
    movu                         m2, [src_piq]         ; load p6
    ABS_SUB_ZERO                 m4, m2, 6             ; |p6-p0|, |q6-q0|

    ; flat2=max(flat2, |p6-p0|)
    pmaxub                       m7, m4
    lea                     src_qiq, [src_qiq + pq]
    sub                     src_piq, pq

    ; flat2=max(flat2, |q6-q0|)
    pmaxub                       m7, m2
    movu                         m4, [src_qiq]         ; load q7
    movu                         m0, [src_piq]         ; load p7
    ABS_SUB_ZERO                 m4, m0, 7             ; |p7-p0|, |q7-q0|

    ; flat2=max(flat2, |p7-p0|)
    pmaxub                       m7, m4

    ; flat2=max(flat2, |q7-q0|)
    pmaxub                       m7, m0
    pxor                         m3, m3

    ; flat2=_mm_subs_epu8(flat2, one);
    psubusb                      m7, [GLOBAL(one)]
    mova                         m4, [stkreg+hev]

    ; flat2=_mm_subs_epu8(flat2, one);
    pcmpeqb                      m7, m3
    pand                         m7, [stkreg+flat]

%if ARCH_X86
    mova                         m1, [stkreg+p1]
    mova                         m2, [stkreg+q1]
%else
    mova                         m1, m11
    mova                         m2, m10
    mova                         m6, m8
    mova                         m5, m9
%endif
    mova             [stkreg+flat2], m7
    mova                         m0, [GLOBAL(t80)]
    pxor                         m7, m7

    ; op1=_mm_xor_si128(p1, t80)
    ; oq1=_mm_xor_si128(q1, t80)
    ; op0=_mm_xor_si128(p0, t80)
    ; work=_mm_subs_epi8(op1, oq1)
    ; oq0=_mm_xor_si128(q0, t80)
    pxor                         m1, m0
    pxor                         m2, m0
    mova            [stkreg+org_p1], m1
    pxor                         m6, m0
    psubsb                       m1, m2
    pxor                         m5, m0

    ; filt=_mm_and_si128(work, hev)
    pand                         m1, m4
    mova            [stkreg+org_q0], m6

    ; work_a=_mm_subs_epi8(oq0, op0)
    psubsb                       m6, m5
    mova                         m3, m7

    ; filt=_mm_adds_epi8(filt, work_a)
    ; filt=_mm_adds_epi8(filt, work_a)
    ; filt=_mm_adds_epi8(filt, work_a)
    paddsb                       m1, m6
    paddsb                       m1, m6
    paddsb                       m1, m6

    pand                         m1, [stkreg+mask]
    mova                         m4, m1

    ; filter1=_mm_adds_epi8(filt, t4);
    ; filter2=_mm_adds_epi8(filt, t3);
    paddsb                       m1, [GLOBAL(t4)]
    paddsb                       m4, [GLOBAL(t3)]

    ; work_=_mm_cmpgt_epi8(zero, filter1)
    ; work_=_mm_cmpgt_epi8(zero, filter2)
    pcmpgtb                      m7, m1
    pcmpgtb                      m3, m4

    ; filter1=_mm_srli_epi16(filter1, 3)
    ; filter2=_mm_srli_epi16(filter2, 3)
    psrlw                        m1, 0x3
    psrlw                        m4, 0x3

    ; work_=_mm_and_si128(work_, te0)
    ; work_=_mm_and_si128(work_, te0)
    pand                         m7, [GLOBAL(te0)]
    pand                         m3, [GLOBAL(te0)]

    ; filter1=_mm_and_si128(filter1, t1f)
    ; filter2=_mm_and_si128(filter2, t1f)
    pand                         m1, [GLOBAL(t1f)]
    pand                         m4, [GLOBAL(t1f)]

     ; filter1=_mm_or_si128(filter1, work_)
     ; filter2=_mm_or_si128(filter2, work_)
    por                          m1, m7
    mova                         m6, [stkreg+org_q0]
    por                          m4, m3

    ; work1=_mm_subs_epi8(oq0, filter1)
    ; work2=_mm_adds_epi8(op0, filter2)
    psubsb                       m6, m1
    paddsb                       m5, m4
    pxor                         m7, m7

    ; _mm_xor_si128(work1, t80)
    ; _mm_xor_si128(work2, t80)
    pxor                         m6, m0
    pxor                         m5, m0
    mova                         m3, [stkreg+hev]
    mova            [stkreg+org_q0], m6
    mova            [stkreg+org_p0], m5

    ; filt=_mm_adds_epi8(filter1, t1)
    ; work_a=_mm_cmpgt_epi8(zero, filt)
    ; filter1=_mm_srli_epi16(filt, 1)
    ; work_a=_mm_and_si128(work_a, t80)
    ; filter1=_mm_and_si128(filter1, t1f)
    paddsb                       m1, [GLOBAL(one)]
    pcmpgtb                      m7, m1
    psrlw                        m1, 0x1
    pand                         m7, m0
    pand                         m1, [GLOBAL(t7f)]
    mova                         m4, [stkreg+org_p1]

    ; filt=_mm_or_si128(filt, work_a)
    ; filt=_mm_andnot_si128(hev, filt)
    ; work1=_mm_adds_epi8(op1, filt)
    ; work2=_mm_subs_epi8(oq1, filt)
    ; _mm_xor_si128(work1, t80)
    ; _mm_xor_si128(work2, t80)
    por                          m1, m7
    pandn                        m3, m1
    paddsb                       m4, m3
    psubsb                       m2, m3
    pxor                         m4, m0
    pxor                         m2, m0
    mova            [stkreg+org_p1], m4
    mova            [stkreg+org_q1], m2

    ; calculating filter4
    pxor                         m7, m7
%if ARCH_X86

    ; sum_lo=q0(low)+p0(low)+4
    ; sum_hi=q0(high)+p0(high)+4
    FLAT_COMP_PART1     [stkreg+p0], [stkreg+q0], m0, m1, m2, m3, \
    [GLOBAL(four)], [GLOBAL(four)], flat, p0_, q0_

    ; sum_lo+=p1(low)+p2(low)
    ; sum_hi+=p1(high)+p2(high)
    FLAT_COMP_PART1     [stkreg+p1], [stkreg+p2], m4, m5, m1, m3, m4, m1, flat, \
    p1_, p2_
%else

    ; sum_lo=q0(low)+p0(low)+4
    ; sum_hi=q0(high)+p0(high)+4
    FLAT_COMP_PART1              m9, m8, m0, m1, m2, m3,  [GLOBAL(four)], \
    [GLOBAL(four)], flat, p0_, q0_

    ; sum_lo+=p1(low)+p2(low)
    ; sum_hi+=p1(high)+p2(high)
    FLAT_COMP_PART1             m11, m13, m4, m5, m1, m3, m4, m1, flat, p1_, p2_
%endif

    paddw                        m5, m0                ; sum2_lo=sum_lo+p2(lo)
    paddw                        m3, m2                ; sum2_hi=sum_hi+p2(hi)
%if ARCH_X86
    mova                         m4, [stkreg+p3]
%else
    mova                         m4, m15
%endif
    mova                         m1, m4
    punpcklbw                    m4, m7
    punpckhbw                    m1, m7
    paddw                        m5, m4                ; sum2_lo+=p3(lo)
    paddw                        m3, m1                ; sum2_hi+=p3(hi)
    mova             [stkreg+p3_lo], m4
    paddw                        m0, m4                ; sum_lo=sum_lo+p3(lo)
    mova             [stkreg+p3_hi], m1
    paddw                        m2, m1                ; sum_hi=sum_lo+p3(hi)

    mova          [stkreg+flat2_lo], m0
    mova          [stkreg+flat2_hi], m2

    mova                         m0, m4
    mova                         m2, m1
    psllw                        m4, 0x1               ; p3*2(lo)
    psllw                        m1, 0x1               ; p3*2(hi)


    ; sum_lo+=p3*2(low),sum_hi+=p3*2(high)
    ; calc flat_p2
    ; sum_lo-=p3(low),sum_hi-=p3(high)
    ; sum_lo-=p2(lo), sum_hi-=p2(high)
    FLATX_COMP_PART2             m5, m3, m4, m1, m6, m4, m0, m2, \
    [stkreg+p2_lo], [stkreg+p2_hi], flat_p2, 0
%if ARCH_X86

    ; sum_lo+=q1(low),sum_hi+=q1(high)
    FLAT_COMP_PART3  [stkreg+q1_lo], m1, [stkreg+q1_hi], m4, m1, \
    [stkreg+p1_lo], m4, [stkreg+p1_hi], [stkreg+q1]
%else
    FLAT_COMP_PART3  [stkreg+q1_lo], m1, [stkreg+q1_hi], m4, m1, \
    [stkreg+p1_lo], m4, [stkreg+p1_hi], m10
%endif

    ; sum_lo+=p1(low),sum_hi+=p1(high)
    ; calc flat_p1
    ; sum_lo-=p3(low),sum_hi-=p3(high)
    ; sum_lo-=p1(lo), sum_hi-=p1(high)
    FLATX_COMP_PART2             m5, m3, m1, m4, m6, m7, m0, m2, m1, m4, \
    flat_p1, 0
    pxor                         m7, m7
%if ARCH_X86

    ; sum_lo+=q2(low),sum_hi+=q2(high)
    FLAT_COMP_PART3  [stkreg+q2_lo], m1, [stkreg+q2_hi], m4, m1, \
    [stkreg+p0_lo], m4, [stkreg+p0_hi], [stkreg+q2]
%else
    FLAT_COMP_PART3  [stkreg+q2_lo], m1, [stkreg+q2_hi], m4, m1, \
    [stkreg+p0_lo], m4, [stkreg+p0_hi], m12
%endif

    ; sum_lo+=p0(low),sum_hi+=p0(high)
    ; calc flat_p0
    ; sum_lo-=p3(low),sum_hi-=p3(high)
    ; sum_lo-=p0(lo), sum_hi-=p0(high)
    FLATX_COMP_PART2             m5, m3, m1, m4, m6, m7, m0, m2, m1, m4, \
    flat_p0, 0
    pxor                         m7, m7
%if ARCH_X86

    ; sum_lo+=q3(low),sum_hi+=q3(high)
    FLAT_COMP_PART3              m0, [stkreg+q0_lo], m2, [stkreg+q0_hi], \
    [stkreg+q3_lo], m1, [stkreg+q3_hi], m4, [stkreg+q3]
%else
    FLAT_COMP_PART3              m0, [stkreg+q0_lo], m2, [stkreg+q0_hi], \
    [stkreg+q3_lo], m1, [stkreg+q3_hi], m4, m14
%endif

    ; sum_lo+=q0(low),sum_hi+=q0(high)
    ; calc flat_q0
    ; sum_lo-=p3(low),sum_hi-=p3(high)
    ; sum_lo-=q0(lo), sum_hi-=q0(high)
    FLATX_COMP_PART2             m5, m3, m0, m2, m6, m7, [stkreg+p2_lo], \
    [stkreg+p2_hi], m0, m2, flat_q0, 0


    ; sum_lo+=q3(low),sum_hi+=q3(high)
    FLATX_COMP_PART4             m0, [stkreg+q1_lo], m2, [stkreg+q1_hi], \
    m5, m1, m3, m4

    ; sum_lo+=q1(low),sum_hi+=q1(high)
    ; calc flat_q1
    ; sum_lo-=p1(low),sum_hi-=p1(high)
    ; sum_lo-=q1(low),sum_hi-=q1(high)
    FLATX_COMP_PART2             m5, m3, m0, m2, m6, m7, [stkreg+p1_lo], \
    [stkreg+p1_hi], m0, m2, flat_q1, 0

    paddw                        m5, m1                ; sum_lo+=q3(lo)
    paddw                        m3, m4                ; sum_hi+=q3(hi)
    paddw                        m5, [stkreg+q2_lo]    ; sum_lo+=q2(lo)
    paddw                        m3, [stkreg+q2_hi]    ; sum_hi+=q2(hi)

    psrlw                        m5, 0x3
    psrlw                        m3, 0x3
    packuswb                     m5, m3
    mova           [stkreg+flat_q2], m5

    ; load sum_lo=q0+p0+p1+p2+p3+4
    ; load sum_hi=q0+p0+q1+p2+p3+4
    mova                         m0, [stkreg+flat2_lo]
    mova                         m1, [stkreg+flat2_hi]
    pxor                         m7, m7

    paddw                        m0, [GLOBAL(four)]    ; sum_lo+=4
    paddw                        m1, [GLOBAL(four)]    ; sum_hi+=4

    ; sum_lo+=p4(low),sum_hi+=p4(high)
    ; sum_lo+=p5(low),sum_hi+=p5(high)
    FLAT_COMP_PART1     [stkreg+p4], [stkreg+p5], m2, m4, m3, m6, m2, m3, \
    flat2, p4_, p5_

    mova                         m2, [stkreg+p6]
    mova                         m4, [stkreg+p7]
    mova                         m3, m2
    mova                         m6, m4
    punpcklbw                    m2, m7
    punpcklbw                    m4, m7
    punpckhbw                    m3, m7
    punpckhbw                    m6, m7
    mova             [stkreg+p6_lo], m2
    mova             [stkreg+p6_hi], m3
    psllw                        m2, 0x1               ; p6*2(lo)
    psllw                        m3, 0x1               ; p6*2(hi)
    paddw                        m0, m2                ; sum_lo+=p6*2(lo)
    paddw                        m1, m3                ; sum_hi+=p6*2(hi)
    mova                         m2, m4
    mova                         m3, m6
    psllw                        m2, 0x3               ; p7*8(lo)
    psllw                        m3, 0x3               ; p7*8(hi)
    psubw                        m2, m4                ; p7*7(lo)
    psubw                        m3, m6                ; p7*7(hi)

    ; sum_lo+=p7*7(lo), sum_hi+=p7*7(high)
    ; calc flat2_p6
    ; sum_lo-=p7(low), sum_hi-=p7(high)
    ; sum_lo-=p6(low), sum_hi-=p6(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, m4, m6, \
    [stkreg+p6_lo], [stkreg+p6_hi], flat2_p6, 1

    ; sum_lo+=q1(lo), sum_hi+=q1(high)
    FLATX_COMP_PART4             m2, [stkreg+p5_lo], m3, [stkreg+p5_hi], m0, \
    [stkreg+q1_lo], m1, [stkreg+q1_hi]

    ; sum_lo+=p5(lo), sum_hi+=p5(high)
    ; calc flat2_p5
    ; sum_lo-=p7(low), sum_hi-=p7(high)
    ; sum_lo-=p5(low), sum_hi-=p5(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, m4, m6, m2, m3, \
    flat2_p5, 1

    ; sum_lo+=q2(lo), sum_hi+=q2(high)
    FLATX_COMP_PART4             m2, [stkreg+p4_lo], m3, [stkreg+p4_hi], \
    m0, [stkreg+q2_lo], m1, [stkreg+q2_hi]

    ; sum_lo+=p4(lo), sum_hi+=p4(high)
    ; calc flat2_p4
    ; sum_lo-=p7(low), sum_hi-=p7(high)
    ; sum_lo-=p4(low), sum_hi-=p4(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, m4, m6, m2, m3, \
    flat2_p4, 1

    ; sum_lo+=q3(lo), sum_hi+=q3(high)
    FLATX_COMP_PART4             m2, [stkreg+p3_lo], m3, [stkreg+p3_hi], \
    m0, [stkreg+q3_lo], m1, [stkreg+q3_hi]

    ; sum_lo+=p3(lo), sum_hi+=p3(high)
    ; calc flat2_p3
    ; sum_lo-=p7(low), sum_hi-=p7(high)
    ; sum_lo-=p3(low), sum_hi-=p3(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, m4, m6, m2, m3, \
    flat2_p3, 1

    ; sum_lo+=q4(lo), sum_hi+=q4(high)
    FLAT2_COMP_PART5             q4, q4_, p2_

    ; sum_lo+=p2(lo), sum_hi+=p2(high)
    ; calc flat2_p2
    ; sum_lo-=p7(low), sum_hi-=p7(high)
    ; sum_lo-=p2(low), sum_hi-=p2(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, m4, m6, m2, m3, \
    flat2_p2, 1

    ; sum_lo+=q5(lo), sum_hi+=q5(high)
    FLAT2_COMP_PART5             q5, q5_, p1_

    ; sum_lo+=p1(lo), sum_hi+=p1(high)
    ; calc flat2_p1
    ; sum_lo-=p7(low), sum_hi-=p7(high)
    ; sum_lo-=p1(low), sum_hi-=p1(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, m4, m6, m2, m3, \
    flat2_p1, 1

    ; sum_lo+=q6(lo), sum_hi+=q6(high)
    FLAT2_COMP_PART5             q6, q6_, p0_

    ; sum_lo+=p0(lo), sum_hi+=p0(high)
    ; calc flat2_p0
    ; sum_lo-=p7(low), sum_hi-=p7(high)
    ; sum_lo-=p0(lo), sum_hi-=p0(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, m4, m6, m2, m3, \
    flat2_p0, 1

    ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLAT2_COMP_PART5             q7, q7_, q0_

    ; sum_lo+=q0(lo), sum_hi+=q0(high)
    ; calc flat2_q0
    ; sum_lo-=p6(low), sum_hi-=p6(high)
    ; sum_lo-=q0(lo), sum_hi-=q0(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, [stkreg+p6_lo], \
    [stkreg+p6_hi], m2, m3, flat2_q0, 1

    ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4             m2, [stkreg+q1_lo], m3, [stkreg+q1_hi], m0, \
    m4, m1, m6

    ; sum_lo+=q1(lo), sum_hi+=q1(high)
    ; calc flat2_q1
    ; sum_lo-=p5(low), sum_hi-=p5(high)
    ; sum_lo-=q1(lo), sum_hi-=q1(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, [stkreg+p5_lo], \
    [stkreg+p5_hi], m2, m3, flat2_q1, 1

    ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4             m2, [stkreg+q2_lo], m3, [stkreg+q2_hi], m0, \
    m4, m1, m6

    ; sum_lo+=q2(lo), sum_hi+=q2(high)
    ; calc flat2_q2
    ; sum_lo-=p4(low), sum_hi-=p4(high)
    ; sum_lo-=q2(lo), sum_hi-=q2(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, [stkreg+p4_lo], \
    [stkreg+p4_hi], m2, m3, flat2_q2, 1

    ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4             m2, [stkreg+q3_lo], m3, [stkreg+q3_hi], m0, \
    m4, m1, m6

    ; sum_lo+=q3(lo), sum_hi+=q3(high)
    ; calc flat2_q3
    ; sum_lo-=p3(low), sum_hi-=p3(high)
    ; sum_lo-=q3(lo), sum_hi-=q3(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, [stkreg+p3_lo], \
    [stkreg+p3_hi], m2, m3, flat2_q3, 1

    ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4             m2, [stkreg+q4_lo], m3, [stkreg+q4_hi], m0, \
    m4, m1, m6

    ; sum_lo+=q4(lo), sum_hi+=q4(high)
    ; calc flat2_q4
    ; sum_lo-=p2(low), sum_hi-=p2(high)
    ; sum_lo-=q4(lo), sum_hi-=q4(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, [stkreg+p2_lo], \
    [stkreg+p2_hi], m2, m3, flat2_q4, 1

    ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4             m2, [stkreg+q5_lo], m3, [stkreg+q5_hi], m0, \
    m4, m1, m6

    ; sum_lo+=q5(lo), sum_hi+=q5(high)
    ; calc flat2_q5
    ; sum_lo-=p1(low), sum_hi-=p1(high)
    ; sum_lo-=q5(lo), sum_hi-=q5(high)
    FLATX_COMP_PART2             m0, m1, m2, m3, m5, m7, [stkreg+p1_lo], \
    [stkreg+p1_hi], m2, m3, flat2_q5, 1

    paddw                        m0, m4                ; sum_lo+=q7(lo)
    paddw                        m1, m6                ; sum_hi+=q7(hi)

    paddw                        m0, [stkreg+q6_lo]    ; sum_lo+=q6(lo)
    paddw                        m1, [stkreg+q6_hi]    ; sum_hi-=q6(hi)
    psrlw                        m0, 0x4
    psrlw                        m1, 0x4
    packuswb                     m0, m1

    mova                         m5, [stkreg+flat2]
    mova                         m7, [stkreg+flat2_p6]

    ; save to p6
    SAVE_FLAT2_QIPI_OR_QIPI      m0, m7, [stkreg+q6], [stkreg+p6], m3

    ; save to p5
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m7, m1, 5

    ; save to p4
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m7, m1, 4

    ; save to p3
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m7, m1, 3

    mova                         m0, [stkreg+flat]

    ; save to p2
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m6, m2, 2

    ; save to p1
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m6, m2, 1

    ; save to p0
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m6, m2, 0
    xchg rbp, rsp
    RET
