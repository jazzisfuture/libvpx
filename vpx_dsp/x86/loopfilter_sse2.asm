%include "third_party/x86inc/x86inc.asm"

SECTION_RODATA
align 16
zero:    times 16 db 0x0
align 16
one:     times 16 db 0x1
align 16
fe:      times 16 db  0xfe
align 16
ff:      times 16 db  0xff
align 16
te0:     times 16 db  0xe0
align 16
t80:     times 16 db 0x80
align 16
t7f:     times 16 db  0x7f
align 16
t1f:     times 16 db  0x1f
align 16
t4:      times 16 db  0x4
align 16
t3:      times 16 db  0x3
align 16
four:    times 8 dw  0x4
align 16
eight:   times 8 dw  0x8

SECTION .text
%undef q0
%undef p0
%undef q1
%undef p1
%undef q2
%undef p2
%undef q3
%undef p3
%undef p3_lo
%undef p2_lo
%undef p1_lo
%undef p0_lo
%undef q0_lo
%undef q1_lo
%undef q2_lo
%undef p3_hi
%undef p2_hi
%undef p1_hi
%undef p0_hi
%undef q0_hi
%undef q1_hi
%undef q2_hi
%undef flat2_p2
%undef flat2_p1
%undef flat2_p0
%undef flat2_q0
%undef flat2_q1
%undef flat2_q2
%undef mask
%undef flat
%undef flat2
%undef hev
%undef org_p0
%undef org_q0
%undef org_p1
%undef org_q1

%if ARCH_X86
%define q0                    rbp+0x000
%define p0                    rbp+0x010
%define q1                    rbp+0x020
%define p1                    rbp+0x030
%define q2                    rbp+0x040
%define p2                    rbp+0x050
%define q3                    rbp+0x060
%define p3                    rbp+0x070
%define add32bstack           0x080
%else
%define add32bstack           0x000
%endif

%define q4                    rbp+0x000+add32bstack
%define p4                    rbp+0x010+add32bstack
%define q5                    rbp+0x020+add32bstack
%define p5                    rbp+0x030+add32bstack
%define q6                    rbp+0x040+add32bstack
%define p6                    rbp+0x050+add32bstack
%define q7                    rbp+0x060+add32bstack
%define p7                    rbp+0x070+add32bstack
%define mask                  rbp+0x080+add32bstack
%define flat                  rbp+0x090+add32bstack
%define flat2                 rbp+0x0A0+add32bstack
%define hev                   rbp+0x0B0+add32bstack
%define org_p0                rbp+0x080+add32bstack
%define org_q0                rbp+0x0C0+add32bstack
%define org_p1                rbp+0x0D0+add32bstack
%define org_q1                rbp+0x0B0+add32bstack

%define flat2_lo              rbp+0x0E0+add32bstack
%define flat2_hi              rbp+0x0F0+add32bstack

%define p0_lo                 rbp+0x100+add32bstack
%define p0_hi                 rbp+0x110+add32bstack
%define q0_lo                 rbp+0x120+add32bstack
%define q0_hi                 rbp+0x130+add32bstack
%define p1_lo                 rbp+0x140+add32bstack
%define p1_hi                 rbp+0x150+add32bstack
%define p2_lo                 rbp+0x160+add32bstack
%define p2_hi                 rbp+0x170+add32bstack
%define p3_lo                 rbp+0x180+add32bstack
%define p3_hi                 rbp+0x190+add32bstack
%define q1_lo                 rbp+0x1A0+add32bstack
%define q1_hi                 rbp+0x1B0+add32bstack
%define q2_lo                 rbp+0x1C0+add32bstack
%define q2_hi                 rbp+0x1D0+add32bstack
%define q3_lo                 rbp+0x1E0+add32bstack
%define q3_hi                 rbp+0x1F0+add32bstack

%define p4_lo                 rbp+0x0E0+add32bstack
%define p4_hi                 rbp+0x0F0+add32bstack
%define p5_lo                 rbp+0x200+add32bstack
%define p5_hi                 rbp+0x210+add32bstack
%define p6_lo                 rbp+0x220+add32bstack
%define p6_hi                 rbp+0x230+add32bstack

%define q4_lo                 rbp+0x240+add32bstack
%define q4_hi                 rbp+0x250+add32bstack
%define q5_lo                 rbp+0x260+add32bstack
%define q5_hi                 rbp+0x270+add32bstack
%define q6_lo                 rbp+0x280+add32bstack
%define q6_hi                 rbp+0x290+add32bstack


%define flat_p2               rbp+0x2A0+add32bstack
%define flat_p1               rbp+0x2B0+add32bstack
%define flat_p0               rbp+0x2C0+add32bstack
%define flat_q0               rbp+0x2D0+add32bstack
%define flat_q1               rbp+0x2E0+add32bstack
%define flat_q2               rbp+0x2F0+add32bstack

%define flat2_p6              rbp+0x300+add32bstack
%define flat2_p5              rbp+0x310+add32bstack
%define flat2_p4              rbp+0x320+add32bstack
%define flat2_p3              rbp+0x330+add32bstack
%define flat2_p2              rbp+0x340+add32bstack
%define flat2_p1              rbp+0x350+add32bstack
%define flat2_p0              rbp+0x360+add32bstack
%define flat2_q0              rbp+0x370+add32bstack
%define flat2_q1              rbp+0x220+add32bstack
%define flat2_q2              rbp+0x230+add32bstack
%define flat2_q3              rbp+0x200+add32bstack
%define flat2_q4              rbp+0x210+add32bstack
%define flat2_q5              rbp+0x0E0+add32bstack

%macro ABS_SUB2 10
    mova           %1, %2
    mova           %3, %4
    psubusb        %5, %6
    psubusb        %7, %8
    psubusb        %9, %2
    psubusb       %10, %4
    por            %9, %5                                ; abs_diff(qi, q(i-1))
    por           %10, %7                                ; abs_diff(pi, p(i-1))
%endm

%macro ABS_SUB_ZERO 3
%if ARCH_X86
    mova           m3, m6
    mova           m1, m5
%else
    mova           m3, m8
    mova           m1, m9
%endif
    mova        [q%3], %1
    mova        [p%3], %2
    psubusb        m3, %1
    psubusb        m1, %2
%if ARCH_X86
    psubusb        %1, m6
    psubusb        %2, m5
%else
    psubusb        %1, m8
    psubusb        %2, m9
%endif
    por            %1, m3                                ; abs_diff(q5, p0)
    por            %2, m1                                ; abs_diff(p5, p0)
%endm

%macro ABS_SUB_Q0 3
    mova       [q%3], %1
    mova       [p%3], %2
    ABS_SUB2      m3, m6, m1, m5, m3, %1, m1, %2, %1, %2
%endm

%macro FLAT_COMP_PART1 11
    mova           %3, %1
    mova           %4, %2
    mova           %5, %3
    mova           %6, %4
    punpcklbw      %3, m7
    punpcklbw      %4, m7
    punpckhbw      %5, m7
    punpckhbw      %6, m7
%ifidn %9, flat                                          ; calc filter4
    mova      [%10lo], %3
    paddw          m0, %4                                ; sum_lo+=pi(low)
    mova      [%10hi], %5
    paddw          m2, %6                                ; sum_hi+=pi(high)
    mova      [%11lo], %4
    paddw          m0, %7                                ; sum_lo+= qj(low)
    movdqa    [%11hi], %6
    paddw          m2, %8                                ; sum_hi+= qj(high)
%else                                                    ; calc filter8
    mova      [%10lo], %3
    paddw          m0, %4                                ; sum_lo+=pi(low)
    mova      [%10hi], %5
    paddw          m1, %6                                ; sum_hi+=pi(high)
    mova      [%11lo], %4
    paddw          m0, %7                                ; sum_lo+= qj(low)
    movdqa    [%11hi], %6
    paddw          m1, %8                                ; sum_hi+= qj(high)
%endif
%endm


%macro FLATX_QIPI_OR_QIPI 6
    mova           m4, %5
    mova           %6, %5
    pand           %1, %5                                ; flat_qi if flat=1

    pandn          m4, %3                                ; qi if flat=0
    pand           %2, %5                                ; flat_pi if flat=1
    pandn          %6, %4                                ; pi if flat=0

    por            %1, m4                                ; save qi_tmp
    por            %6, %2                                ; save pi_tmp
%endm


%macro SAVE_FLAT2_QIPI_OR_QIPI 5
    sub                  src_qiq, pq
    lea                  src_piq, [src_piq + pq]
    FLATX_QIPI_OR_QIPI        %1, %2, %3, %4, m5, %5
    movu               [src_qiq], %1                     ; save qi
    movu               [src_piq], %5                     ; save pi
%endm

%macro FULL_SAVE_FLAT2_QIPI_OR_QIPI 3
%if %3 < 3
    mova                    m7, [flat_q%3]
    mova                    m1, [flat_p%3]
%if %3=2 && ARCH_X86_64
    FLATX_QIPI_OR_QIPI      m7, m1, m12, m13, m0, m3
%elif %3<2
    FLATX_QIPI_OR_QIPI      m7, m1, [org_q%3], \
    [org_p%3], m0, m3
%else
    FLATX_QIPI_OR_QIPI      m7, m1, [q%3], [p%3], m0,\
    m3
%endif
%endif
    mova                    %1, [flat2_q%3]
    mova                    %2, [flat2_p%3]
%if %3 > 3
    SAVE_FLAT2_QIPI_OR_QIPI %1, %2, [q%3], [p%3], m3
%else
%if %3=3 && ARCH_X86_64
    SAVE_FLAT2_QIPI_OR_QIPI %1, %2, m14, m15, m3
%elif %3 < 3
    SAVE_FLAT2_QIPI_OR_QIPI %1, %2, m7, m3, m1
%else
    SAVE_FLAT2_QIPI_OR_QIPI %1, %2, [q%3], [p%3], m3
%endif
%endif
%endm

%macro FLATX_COMP_PART2 12
    paddw          %1, %3                                ; sum_lo+=q/pi(lo)
    paddw          %2, %4                                ; sum_hi+=q/pi(hi)
    mova           %5, %1
    mova           %6, %2
%if %12=1
    psrlw          %5, 0x4
    psubw          %1, %7                                ; sum_lo-=pj(lo)
    psrlw          %6, 0x4
%else
    psrlw          %5, 0x3
    psubw          %1, %7                                ; sum_lo-=pj(lo)
    psrlw          %6, 0x3
%endif
    psubw          %2, %8                                ; sum_hi-=pj(hi)
    packuswb       %5, %6
    psubw          %1, %9                                ; sum_lo-=q/pi(lo)
    psubw          %2, %10                               ; sum_hi-=q/pi(hi)
    mova        [%11], %5
%endm

%macro FLAT_COMP_PART3 9
    mova           m1, %9                                ; only for filter4
    mova           m4, m1
    punpcklbw      m1, m7
    punpckhbw      m4, m7
    paddw          m5, m1                                ; sum_lo+=qi(lo)
    paddw          m3, m4                                ; sum_hi+=qi(hi)
    mova           %1, %2                                ; save qi(low)
    mova           %3, %4                                ; save qi(high)
    mova           %5, %6
    mova           %7, %8
%endm

%macro FLATX_COMP_PART4 8
    mova           %1, %2                                ; save q/pi(low)
    mova           %3, %4                                ; save q/pi(hi)
    paddw          %5, %6                                ; sum_lo+=qj(lo)
    paddw          %7, %8                                ; sum_hi+=qj(hi)
%endm

%macro FLAT2_COMP_PART5 3                                ; for filter8
    pxor           m7, m7
%ifnidn %1, q7                                           ; if not q7
    mova           m2, [%1]                              ; load qi
    mova           m3, m2
    punpcklbw      m2, m7
    punpckhbw      m3, m7
    paddw          m0, m2                                ; sum_lo+=qi(lo)
    paddw          m1, m3                                ; sum_hi+=qi(hi)
    mova      [%2lo], m2
    mova      [%2hi], m3
%else
    mova           m4, [%1]
    mova           m6, m4
    punpcklbw      m4, m7
    punpckhbw      m6, m7
    paddw          m0, m4                                 ; sum_lo+=qi(lo)
    paddw          m1, m6                                 ; sum_hi+=qi(hi)
%endif
    mova           m2, [%3lo]
    mova           m3, [%3hi]
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

cglobal lpf_horizontal_edge_w_sse2_16, 6, 6+(ARCH_X86_64*1), \
8+(ARCH_X86_64*8), 0x380+(ARCH_X86*add32bstack), src, p, blimit, limit,\
thresh, count
    SET_SOURCE_QI_PI
%if ARCH_X86
    %xdefine limitm  [esp + stack_offset + 4*3 + 4]
    %xdefine threshm  [esp + stack_offset + 4*4 + 4]
    GET_GOT r3q
%else
    GET_GOT r7q
%endif
%if ARCH_X86=1 && CONFIG_PIC=1
    add rsp, gprsize
%endif
    xchg rbp, rsp

    movu           m3, [src_qiq]        ; load q0
    movu           m7, [src_piq]        ; load p0
    lea        src_qiq,  [src_qiq + pq]
    sub        src_piq, pq
    movu           m2, [src_qiq]        ; load q1
    movu           m5, [src_piq]        ; load p1
    mova           m6, m3
    mova           m4, m7
    mova           m1, m2
    mova           m0, m5

%if ARCH_X86
    ;ABS_SUB2     [q0], m3, [p0], m7, \
    ;m6, m2, m4, m5, m1, m0
    mova           [q0], m3
    mova           [p0], m7
    psubusb        m6, m2
    psubusb        m4, m5
    psubusb        m1, m3
    psubusb        m0, m7
    por            m1, m6                                ; abs_diff(qi, q(i-1))
    por            m0, m4                                ; abs_diff(pi, p(i-1))
%else
    ABS_SUB2       m8, m3, m9, m7, \
    m6, m2, m4, m5, m1, m0
%endif

    pmaxub         m0, m1               ; max(|p1-p0|,|q1-q0|)
    mova       [flat], m0
    mova           m1, m7
    mova           m6, m2
    mova           m4, m5
%if ARCH_X86
    mova         [q1], m2
    mova         [p1], m5
%else
    mova          m10, m2
    mova          m11, m5
%endif
    psubusb        m1, m3
    psubusb        m3, m7
    psubusb        m4, m2
    psubusb        m6, m5
    por            m1, m3               ; abs_diff(p0,q0)
    por            m4, m6               ; abs_diff(p1,q1)
    lea        src_qiq,  [src_qiq + pq]
    sub        src_piq, pq
    pand           m4, [GLOBAL(fe)]
    paddusb        m1, m1               ; 2*abs_diff(p0,q0)

    mova           m6, [blimitq]
    psrlw          m4, 1                ; abs_diff(p1,q1)/2
    paddusb        m4, m1               ;
    movu           m7, [src_qiq]        ; load p2
    movu           m3, [src_piq]        ; load q2
    psubusb        m4, m6               ; mask=2*|p0-q0|+(|p1-q1|<<1)-bilimit
    pxor           m6, m6
    mova           m1, m7
    pcmpeqb        m4, m6               ; mask=mask>0
    pxor           m4, [GLOBAL(ff)]     ; mask = _mm_xor_si128(mask, ff)
    mova           m6, m3
    pmaxub         m0, m4

%if ARCH_X86                            ; |p2-p1|
    ABS_SUB2     [q2], m7, [p2], m3, \
    m1, m2, m6, m5, m2, m5
%else
    ABS_SUB2      m12, m7, m13, m3, \
    m1, m2, m6, m5, m2, m5
%endif

    lea        src_qiq, [src_qiq + pq]
    sub        src_piq, pq
    pmaxub         m0, m2               ; mask=max(mask, abs_diff(p2,p1))
    movu           m1, [src_qiq]        ; load q3
    movu           m4, [src_piq]        ; load p3
    pmaxub         m0, m5               ; mask=max(mask, abs_diff(q2,q1))
    mova           m2, m1
    mova           m6, m4

%if ARCH_X86
    mov          r2q, dword limitm
                                        ; |p3-p2|
    ABS_SUB2    [q3], m1, [p3], m4, \
    m2, m7, m6, m3, m7, m3    
    pmaxub        m0, m7                ; mask=max(mask, abs_diff(q3,q2))
    mova          m2, [r2q]
    pmaxub        m0, m3                ; mask=max(mask, abs_diff(p3,p2))
    mova          m6, [q0]
    mova          m5, [p0]
    psubusb       m0, m2                ; mask=_mm_subs_epu8(mask, limit)
    pxor          m7, m7
    pcmpeqb       m0, m7                ; mask=_mm_cmpeq_epi8(mask, zero)
    ABS_SUB2      m3, m6, m2, m5, m3, \
    m1, m2, m4, m1, m4                  ; |p3-p0| , |q3-q0|

    mova          m7, [q2]
    mova          m2, [p2]
    pmaxub        m4, m1                ; flat=max(|p3-p0|, |q3-q0|)
    ABS_SUB2      m3, m6, m1, m5, m3, \
    m7, m1, m2, m7, m2

    mov          r4q, dword threshm
%else
                                        ; |p3-p2|
    ABS_SUB2     m14, m1, m15, m4, \
     m2, m7, m6, m3, m7, m3
    pmaxub        m0, m7                ; mask=max(mask, abs_diff(q3,q2))
    mova          m2, [limitq]
    pmaxub        m0, m3                ; mask=max(mask, abs_diff(p3,p2))
    psubusb       m0, m2                ; mask=_mm_subs_epu8(mask, limit)
    pxor          m7, m7
    pcmpeqb       m0, m7                ; mask = _mm_cmpeq_epi8(mask, zero)
    ABS_SUB2      m3, m8, m2, m9, m3, \
    m1, m2, m4, m1, m4                  ; |p3-p0| , |q3-q0|

    mova          m7, m12
    mova          m2, m13
    pmaxub        m4, m1                ; flat=max(|p3-p0|, |q3-q0|)
    ABS_SUB2      m3, m8, m1, m9, m3, \
    m7, m1, m2, m7, m2
%endif

    mova      [mask], m0
    pmaxub        m7, m2                ; work=max(|p2-p0|, |q2-q0|)
    mova          m1, [flat]
    pmaxub        m4, m7                ; flat=_mm_max_epu8(flat, work)
    pxor          m3, m3
%if ARCH_X86
    mova          m2, [r4q]
%else
    mova          m2, [threshq]
%endif
    pmaxub        m4, m1                ; flat=_mm_max_epu8(flat, work)
    psubusb       m4, [GLOBAL(one)]     ; flat = _mm_subs_epu8(flat, one);
    psubusb       m1, m2
    pcmpeqb       m4, m3                ; flat = flat > 0;
    pcmpeqb       m1, m3                ; hev = _mm_cmpeq_epi8(hev, zero)
%if ARCH_X86
    lea       src_qiq, [srcq + 4*pq]
%else
    lea       src_qiq, [src_qiq + pq]
%endif
    sub       src_piq, pq
    pand          m4, m0
    pxor          m1, [GLOBAL(ff)]      ; hev = _mm_xor_si128(hev, ff)
    mova      [flat], m4
    mova       [hev], m1

    movu          m7, [src_qiq]         ; load q4
    movu          m4, [src_piq]         ; load p4
    mova        [q4], m7
    mova        [p4], m4

    ABS_SUB_ZERO  m7, m4, 4             ; |p4-p0|, |q4-q0|

    lea       src_qiq, [src_qiq + pq]
    sub       src_piq, pq
    movu          m2, [src_qiq]         ; load q5
    movu          m0, [src_piq]         ; load p5
    pmaxub        m7, m4                ; flat2=max(|q4-q0|, |p4-p0|)
    ABS_SUB_ZERO  m2, m0, 5             ; |p5-p0|, |q5-q0|

    pmaxub        m7, m2                ; flat2=max(flat2, |p5-p0|)
    lea       src_qiq, [src_qiq + pq]
    sub       src_piq, pq
    pmaxub        m7, m0                ; flat2=max(flat2,|q5-q0|)
    movu          m4, [src_qiq]         ; load q6
    movu          m2, [src_piq]         ; load p6
    ABS_SUB_ZERO  m4, m2, 6             ; |p6-p0|, |q6-q0|
    pmaxub        m7, m4                ; flat2=max(flat2, |p6-p0|)
    lea       src_qiq, [src_qiq + pq]
    sub       src_piq, pq
    pmaxub        m7, m2                ; flat2=max(flat2, |q6-q0|)
    movu          m4, [src_qiq]         ; load q7
    movu          m0, [src_piq]         ; load p7
    ABS_SUB_ZERO  m4, m0, 7             ; |p7-p0|, |q7-q0|
    pmaxub        m7, m4                ; flat2=max(flat2, |p7-p0|)
    pmaxub        m7, m0                ; flat2=max(flat2, |q7-q0|)
    pxor          m3, m3
    psubusb       m7, [GLOBAL(one)]     ; flat2=_mm_subs_epu8(flat2, one);
    mova          m4, [hev]
    pcmpeqb       m7, m3                ; flat2=_mm_subs_epu8(flat2, one);
    pand          m7, [flat]

%if ARCH_X86
    mova          m1, [p1]
    mova          m2, [q1]
%else
    mova          m1, m11
    mova          m2, m10
    mova          m6, m8
    mova          m5, m9
%endif
    mova     [flat2], m7
    mova          m0, [GLOBAL(t80)]
    pxor          m7, m7
    pxor          m1, m0                ; op1=_mm_xor_si128(p1, t80)
    pxor          m2, m0                ; oq1=_mm_xor_si128(q1, t80)

    mova    [org_p1], m1
    pxor          m6, m0                ; op0=_mm_xor_si128(p0, t80)
    psubsb        m1, m2                ; work=_mm_subs_epi8(op1, oq1)
    pxor          m5, m0                ; oq0=_mm_xor_si128(q0, t80)
    pand          m1, m4                ; filt=_mm_and_si128(work, hev)


    mova    [org_q0], m6
    psubsb        m6, m5                ; work_a=_mm_subs_epi8(oq0, op0)
    mova          m3, m7
    paddsb        m1, m6                ; filt=_mm_adds_epi8(filt, work_a)
    paddsb        m1, m6                ; filt=_mm_adds_epi8(filt, work_a)
    paddsb        m1, m6                ; filt=_mm_adds_epi8(filt, work_a)
    pand          m1, [mask]
    mova          m4, m1
    paddsb        m1, [GLOBAL(t4)]      ; filter1=_mm_adds_epi8(filt, t4);
    paddsb        m4, [GLOBAL(t3)]      ; filter2=_mm_adds_epi8(filt, t3);
    pcmpgtb       m7, m1                ; work_=_mm_cmpgt_epi8(zero, filter1)
    pcmpgtb       m3, m4                ; work_=_mm_cmpgt_epi8(zero, filter2)
    psrlw         m1, 0x3               ; filter1=_mm_srli_epi16(filter1, 3)
    psrlw         m4, 0x3               ; filter2=_mm_srli_epi16(filter2, 3)
    pand          m7, [GLOBAL(te0)]     ; work_=_mm_and_si128(work_, te0)
    pand          m3, [GLOBAL(te0)]     ; work_=_mm_and_si128(work_, te0)
    pand          m1, [GLOBAL(t1f)]     ; filter1=_mm_and_si128(filter1, t1f)
    pand          m4, [GLOBAL(t1f)]     ; filter2=_mm_and_si128(filter2, t1f)
    por           m1, m7                ; filter1=_mm_or_si128(filter1, work_)
    mova          m6, [org_q0]
    por           m4, m3                ; filter2=_mm_or_si128(filter2, work_)
    psubsb        m6, m1                ; work1=_mm_subs_epi8(oq0, filter1)
    paddsb        m5, m4                ; work2=_mm_adds_epi8(op0, filter2)
    pxor          m7, m7
    pxor          m6, m0                ; _mm_xor_si128(work1, t80)
    pxor          m5, m0                ; _mm_xor_si128(work2, t80)
    mova          m3, [hev]
    mova    [org_q0], m6
    mova    [org_p0], m5
    paddsb        m1, [GLOBAL(one)]     ; filt=_mm_adds_epi8(filter1, t1)
    pcmpgtb       m7, m1                ; work_a=_mm_cmpgt_epi8(zero, filt)
    psrlw         m1, 0x1               ; filter1=_mm_srli_epi16(filt, 1)
    pand          m7, m0                ; work_a=_mm_and_si128(work_a, t80)
    pand          m1, [GLOBAL(t7f)]     ; filter1=_mm_and_si128(filter1, t1f)
    mova          m4, [org_p1]
    por           m1, m7                ; filt=_mm_or_si128(filt, work_a)
    pandn         m3, m1                ; filt=_mm_andnot_si128(hev, filt)
    paddsb        m4, m3                ; work1=_mm_adds_epi8(op1, filt)
    psubsb        m2, m3                ; work2=_mm_subs_epi8(oq1, filt)
    pxor          m4, m0                ; _mm_xor_si128(work1, t80)
    pxor          m2, m0                ; _mm_xor_si128(work2, t80)
    mova    [org_p1], m4
    mova    [org_q1], m2


    pxor           m7, m7               ; calculating filter4
%if ARCH_X86
                                        ; sum_lo=q0(low)+p0(low)+4
                                        ; sum_hi=q0(high)+p0(high)+4
    FLAT_COMP_PART1  [p0], [q0], m0, \
    m1, m2, m3,  [GLOBAL(four)], \
    [GLOBAL(four)], flat, p0_, q0_
                                        ; sum_lo+=p1(low)+p2(low)
                                        ; sum_hi+=p1(high)+p2(high)
    FLAT_COMP_PART1 [p1], [p2], m4, \
    m5, m1, m3, m4, m1, flat, p1_, p2_
%else
                                        ; sum_lo=q0(low)+p0(low)+4
                                        ; sum_hi=q0(high)+p0(high)+4
    FLAT_COMP_PART1  m9, m8, m0, m1, \
    m2, m3,  [GLOBAL(four)], \
    [GLOBAL(four)], flat, p0_, q0_
                                        ; sum_lo+=p1(low)+p2(low)
                                        ; sum_hi+=p1(high)+p2(high)
    FLAT_COMP_PART1 m11, m13, m4, m5, \
    m1, m3, m4, m1, flat, p1_, p2_
%endif

    paddw          m5, m0               ; sum2_lo=sum_lo+p2(lo)
    paddw          m3, m2               ; sum2_hi=sum_hi+p2(hi)
%if ARCH_X86
    mova           m4, [p3]
%else
    mova           m4, m15
%endif
    mova           m1, m4
    punpcklbw      m4, m7
    punpckhbw      m1, m7
    paddw          m5, m4               ; sum2_lo+=p3(lo)
    paddw          m3, m1               ; sum2_hi+=p3(hi)
    mova      [p3_lo], m4
    paddw          m0, m4               ; sum_lo=sum_lo+p3(lo)
    mova      [p3_hi], m1
    paddw          m2, m1               ; sum_hi=sum_lo+p3(hi)

    mova   [flat2_lo], m0
    mova   [flat2_hi], m2

    mova           m0, m4
    mova           m2, m1
    psllw          m4, 0x1              ; p3*2(lo)
    psllw          m1, 0x1              ; p3*2(hi)

                                        ; sum_lo+=p3*2(low),sum_hi+=p3*2(high)
                                        ; calc flat_p2
                                        ; sum_lo-=p3(low),sum_hi-=p3(high)
                                        ; sum_lo-=p2(lo), sum_hi-=p2(high)
    FLATX_COMP_PART2 m5, m3, m4, m1, \
    m6, m4, m0, m2, [p2_lo], [p2_hi], \
    flat_p2, 0
%if ARCH_X86                            ; sum_lo+=q1(low),sum_hi+=q1(high)
    FLAT_COMP_PART3 [q1_lo], m1, \
    [q1_hi], m4, m1, [p1_lo], m4, \
    [p1_hi], [q1]
%else
    FLAT_COMP_PART3 [q1_lo], m1, \
    [q1_hi], m4, m1, [p1_lo], m4, \
    [p1_hi], m10
%endif
                                        ; sum_lo+=p1(low),sum_hi+=p1(high)
                                        ; calc flat_p1
                                        ; sum_lo-=p3(low),sum_hi-=p3(high)
                                        ; sum_lo-=p1(lo), sum_hi-=p1(high)
    FLATX_COMP_PART2 m5, m3, m1, m4, \
    m6, m7, m0, m2, m1, m4, flat_p1, 0
    pxor           m7, m7
%if ARCH_X86                            ; sum_lo+=q2(low),sum_hi+=q2(high)
    FLAT_COMP_PART3 [q2_lo], m1, \
    [q2_hi], m4, m1, [p0_lo], m4, \
    [p0_hi], [q2]
%else
    FLAT_COMP_PART3 [q2_lo], m1, \
    [q2_hi], m4, m1, [p0_lo], m4, \
    [p0_hi], m12
%endif
                                        ; sum_lo+=p0(low),sum_hi+=p0(high)
                                        ; calc flat_p0
                                        ; sum_lo-=p3(low),sum_hi-=p3(high)
                                        ; sum_lo-=p0(lo), sum_hi-=p0(high)
    FLATX_COMP_PART2 m5, m3, m1, m4, \
    m6, m7, m0, m2, m1, m4, flat_p0, 0
    pxor           m7, m7
%if ARCH_X86                            ; sum_lo+=q3(low),sum_hi+=q3(high)
    FLAT_COMP_PART3 m0, [q0_lo], m2, \
    [q0_hi], [q3_lo], m1, [q3_hi], \
    m4, [q3]
%else
    FLAT_COMP_PART3 m0, [q0_lo], m2, \
    [q0_hi], [q3_lo], m1, [q3_hi], \
    m4, m14
%endif
                                        ; sum_lo+=q0(low),sum_hi+=q0(high)
                                        ; calc flat_q0
    FLATX_COMP_PART2 m5, m3, m0, m2, \
    m6, m7, [p2_lo], [p2_hi], m0, m2, \
    flat_q0, 0                          ; sum_lo-=p3(low),sum_hi-=p3(high)
                                        ; sum_lo-=q0(lo), sum_hi-=q0(high)

                                        ; sum_lo+=q3(low),sum_hi+=q3(high)
    FLATX_COMP_PART4 m0, [q1_lo], m2, \
    [q1_hi], m5, m1, m3, m4
                                        ; sum_lo+=q1(low),sum_hi+=q1(high)
                                        ; calc flat_q1
                                        ; sum_lo-=p1(low),sum_hi-=p1(high)
                                        ; sum_lo-=q1(low),sum_hi-=q1(high)
    FLATX_COMP_PART2 m5, m3, m0, m2, \
    m6, m7, [p1_lo], [p1_hi], m0, m2, \
    flat_q1, 0

    paddw          m5, m1               ; sum_lo+=q3(lo)
    paddw          m3, m4               ; sum_hi+=q3(hi)
    paddw          m5, [q2_lo]          ; sum_lo+=q2(lo)
    paddw          m3, [q2_hi]          ; sum_hi+=q2(hi)

    psrlw          m5, 0x3
    psrlw          m3, 0x3
    packuswb       m5, m3
    mova    [flat_q2], m5

    mova           m0, [flat2_lo]       ; load sum_lo=q0+p0+p1+p2+p3+4
    mova           m1, [flat2_hi]       ; load sum_hi=q0+p0+q1+p2+p3+4
    pxor           m7, m7

    paddw          m0, [GLOBAL(four)]   ; sum_lo+=4
    paddw          m1, [GLOBAL(four)]   ; sum_hi+=4

                                        ; sum_lo+=p4(low),sum_hi+=p4(high)
                                        ; sum_lo+=p5(low),sum_hi+=p5(high)
    FLAT_COMP_PART1 [p4], [p5], m2, \
    m4, m3, m6, m2, m3, flat2, p4_, p5_

    mova           m2, [p6]
    mova           m4, [p7]
    mova           m3, m2
    mova           m6, m4
    punpcklbw      m2, m7
    punpcklbw      m4, m7
    punpckhbw      m3, m7
    punpckhbw      m6, m7
    mova      [p6_lo], m2
    mova      [p6_hi], m3
    psllw          m2, 0x1              ; p6*2(lo)
    psllw          m3, 0x1              ; p6*2(hi)
    paddw          m0, m2               ; sum_lo+=p6*2(lo)
    paddw          m1, m3               ; sum_hi+=p6*2(hi)
    mova           m2, m4
    mova           m3, m6
    psllw          m2, 0x3              ; p7*8(lo)
    psllw          m3, 0x3              ; p7*8(hi)
    psubw          m2, m4               ; p7*7(lo)
    psubw          m3, m6               ; p7*7(hi)

                                        ; sum_lo+=p7*7(lo), sum_hi+=p7*7(high)
                                        ; calc flat2_p6
                                        ; sum_lo-=p7(low), sum_hi-=p7(high)
                                        ; sum_lo-=p6(low), sum_hi-=p6(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, m4, m6, [p6_lo], [p6_hi], \
    flat2_p6, 1
                                        ; sum_lo+=q1(lo), sum_hi+=q1(high)
    FLATX_COMP_PART4 m2, [p5_lo], m3, \
    [p5_hi], m0, [q1_lo], m1, [q1_hi]

                                        ; sum_lo+=p5(lo), sum_hi+=p5(high)
                                        ; calc flat2_p5
                                        ; sum_lo-=p7(low), sum_hi-=p7(high)
                                        ; sum_lo-=p5(low), sum_hi-=p5(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, m4, m6, m2, m3, flat2_p5, 1

                                        ; sum_lo+=q2(lo), sum_hi+=q2(high)
    FLATX_COMP_PART4 m2, [p4_lo], m3, \
    [p4_hi], m0, [q2_lo], m1, [q2_hi]

                                        ; sum_lo+=p4(lo), sum_hi+=p4(high)
                                        ; calc flat2_p4
                                        ; sum_lo-=p7(low), sum_hi-=p7(high)
                                        ; sum_lo-=p4(low), sum_hi-=p4(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, m4, m6, m2, m3, flat2_p4, 1

                                        ; sum_lo+=q3(lo), sum_hi+=q3(high)
    FLATX_COMP_PART4 m2, [p3_lo], m3, \
    [p3_hi], m0, [q3_lo], m1, [q3_hi]

                                        ; sum_lo+=p3(lo), sum_hi+=p3(high)
                                        ; calc flat2_p3
                                        ; sum_lo-=p7(low), sum_hi-=p7(high)
                                        ; sum_lo-=p3(low), sum_hi-=p3(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, m4, m6, m2, m3, flat2_p3, 1

    FLAT2_COMP_PART5 q4, q4_, p2_       ; sum_lo+=q4(lo), sum_hi+=q4(high)
                                        ; sum_lo+=p2(lo), sum_hi+=p2(high)
                                        ; calc flat2_p2
                                        ; sum_lo-=p7(low), sum_hi-=p7(high)
                                        ; sum_lo-=p2(low), sum_hi-=p2(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, m4, m6, m2, m3, flat2_p2, 1

    FLAT2_COMP_PART5 q5, q5_, p1_       ; sum_lo+=q5(lo), sum_hi+=q5(high)
                                        ; sum_lo+=p1(lo), sum_hi+=p1(high)
                                        ; calc flat2_p1
                                        ; sum_lo-=p7(low), sum_hi-=p7(high)
                                        ; sum_lo-=p1(low), sum_hi-=p1(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, m4, m6, m2, m3, flat2_p1, 1

    FLAT2_COMP_PART5 q6, q6_, p0_       ; sum_lo+=q6(lo), sum_hi+=q6(high)
                                        ; sum_lo+=p0(lo), sum_hi+=p0(high)
                                        ; calc flat2_p0
                                        ; sum_lo-=p7(low), sum_hi-=p7(high)
                                        ; sum_lo-=p0(lo), sum_hi-=p0(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, m4, m6, m2, m3, flat2_p0, 1

    FLAT2_COMP_PART5 q7, q7_, q0_       ; sum_lo+=q7(lo), sum_hi+=q7(high)
                                        ; sum_lo+=q0(lo), sum_hi+=q0(high)
                                        ; calc flat2_q0
                                        ; sum_lo-=p6(low), sum_hi-=p6(high)
                                        ; sum_lo-=q0(lo), sum_hi-=q0(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, [p6_lo], [p6_hi], m2, m3, \
    flat2_q0, 1
                                        ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4 m2, [q1_lo], m3, \
    [q1_hi], m0, m4, m1, m6

                                        ; sum_lo+=q1(lo), sum_hi+=q1(high)
                                        ; calc flat2_q1
                                        ; sum_lo-=p5(low), sum_hi-=p5(high)
                                        ; sum_lo-=q1(lo), sum_hi-=q1(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, [p5_lo], [p5_hi], m2, m3, \
    flat2_q1, 1
                                        ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4 m2, [q2_lo], m3, \
    [q2_hi], m0, m4, m1, m6

                                        ; sum_lo+=q2(lo), sum_hi+=q2(high)
                                        ; calc flat2_q2
                                        ; sum_lo-=p4(low), sum_hi-=p4(high)
                                        ; sum_lo-=q2(lo), sum_hi-=q2(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, [p4_lo], [p4_hi], m2, m3, \
    flat2_q2, 1

                                        ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4 m2, [q3_lo], m3, \
    [q3_hi], m0, m4, m1, m6

                                        ; sum_lo+=q3(lo), sum_hi+=q3(high)
                                        ; calc flat2_q3
                                        ; sum_lo-=p3(low), sum_hi-=p3(high)
                                        ; sum_lo-=q3(lo), sum_hi-=q3(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, [p3_lo], [p3_hi], m2, m3, \
    flat2_q3, 1

                                        ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4 m2, [q4_lo], m3, \
    [q4_hi], m0, m4, m1, m6

                                        ; sum_lo+=q4(lo), sum_hi+=q4(high)
                                        ; calc flat2_q4
                                        ; sum_lo-=p2(low), sum_hi-=p2(high)
                                        ; sum_lo-=q4(lo), sum_hi-=q4(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, [p2_lo], [p2_hi], m2, m3, \
    flat2_q4, 1

                                        ; sum_lo+=q7(lo), sum_hi+=q7(high)
    FLATX_COMP_PART4 m2, [q5_lo], m3, \
    [q5_hi], m0, m4, m1, m6

                                        ; sum_lo+=q5(lo), sum_hi+=q5(high)
                                        ; calc flat2_q5
                                        ; sum_lo-=p1(low), sum_hi-=p1(high)
                                        ; sum_lo-=q5(lo), sum_hi-=q5(high)
    FLATX_COMP_PART2 m0, m1, m2, m3, \
    m5, m7, [p1_lo], [p1_hi], m2, m3, \
    flat2_q5, 1

    paddw         m0, m4                ; sum_lo+=q7(lo)
    paddw         m1, m6                ; sum_hi+=q7(hi)

    paddw         m0, [q6_lo]           ; sum_lo+=q6(lo)
    paddw         m1, [q6_hi]           ; sum_hi-=q6(hi)
    psrlw         m0, 0x4
    psrlw         m1, 0x4
    packuswb      m0, m1

    mova           m5, [flat2]
    mova           m7, [flat2_p6]
                                        ; save to p6
    SAVE_FLAT2_QIPI_OR_QIPI m0, m7, \
    [q6], [p6], m3
                                        ; save to p5
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m7, \
    m1, 5

                                        ; save to p4
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m7, \
    m1, 4
                                        ; save to p3
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m7, \
    m1, 3

    mova           m0, [flat]
                                        ; save to p2
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m6, \
    m2, 2
                                        ; save to p1
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m6, \
    m2, 1
                                        ; save to p0
    FULL_SAVE_FLAT2_QIPI_OR_QIPI m6, \
    m2, 0
    xchg rbp, rsp
%if ARCH_X86_64=1 || CONFIG_PIC=0
    RESTORE_GOT
%endif
    RET
