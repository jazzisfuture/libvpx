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
%define q0                    rbp+0x000
%define p0                    rbp+0x010
%define q1                    rbp+0x020
%define p1                    rbp+0x030
%define q2                    rbp+0x040
%define p2                    rbp+0x050
%define q3                    rbp+0x060
%define p3                    rbp+0x070
%if ARCH_X86_64
%define q0_lo                 rbp+0x0A0
%define q0_hi                 rbp+0x0F0
%define flat2_p2              rbp+0x100
%define flat2_p1              rbp+0x110
%define flat2_p0              rbp+0x120
%define flat2_q0              rbp+0x130
%define flat2_q1              rbp+0x140
%define flat2_q2              rbp+0x150
%else
%define p2_lo                 rbp+0x090
%define p1_lo                 rbp+0x0A0
%define p0_lo                 rbp+0x0B0
%define q0_lo                 rbp+0x0C0
%define q1_lo                 rbp+0x0D0
%define q2_lo                 rbp+0x0E0
%define p2_hi                 rbp+0x110
%define p1_hi                 rbp+0x120
%define p0_hi                 rbp+0x130
%define q0_hi                 rbp+0x140
%define q1_hi                 rbp+0x150
%define q2_hi                 rbp+0x160
%define flat2_p2              rbp+0x180
%define flat2_p1              rbp+0x190
%define flat2_p0              rbp+0x1A0
%define flat2_q0              rbp+0x1B0
%define flat2_q1              rbp+0x1C0
%define flat2_q2              rbp+0x1D0
%define org_q0                rbp+0x080
%define org_p1                rbp+0x090
%define flat                  rbp+0x0A0
%define hev                   rbp+0x0B0
%endif


%macro SET_SOURCE_POINTER 0
%if ARCH_X86
   %define src_evq r4q
   %define src_odq r5q
%else
   %define src_evq  r8q
   %define src_odq  r9q
%endif
    mov         src_evq, srcq
    mov         src_odq, srcq
    sub         src_odq, pq
%endm

%macro ZERO_EXTEND 6
    movu             %1, [src_evq]        ; load qi
    movu             %2, [src_odq]        ; load pi
    mova             %3, %1
    mova             %4, %2

    punpcklbw        %3, m0               ; extend from 8bit to 16bit
    mova           [%5], %2
    punpcklbw        %4, m0
    mova           [%6], %1
    punpckhbw        %2, m0
    punpckhbw        %1, m0
%endm

%macro ADD2XP2 2
    mova             %1, m2
    mova             %2, m1

    psllw            %1, 1
    psllw            %2, 1

    paddw            m5, %1               ; add p2x2 lo
    paddw            m6, %2               ; add p2x2 hi

%endm

%macro ADD3XP3 4
    movu             %1, [src_evq]        ; load p3
    mova             %2, %1
    mova           [p3], %1
    punpcklbw        %2, m0               ; extent lo p3
    punpckhbw        %1, m0               ; extent hi p3
    mova             %3, %2
    mova             %4, %1
    psllw            %3, 1
    psllw            %4, 1

    paddw            m5, %2
    paddw            m6, %1
    paddw            m5, %3               ; add p3x3 lo
    paddw            m6, %4               ; add p3x3 hi
%endm

%macro SAVE_FLAT2_QI_PI 4-8
%ifnidn %3, m5
    mova             %3, m5
    mova             %4, m6
%endif
    psrlw            %3, 3                ; sum>>=3
    psrlw            %4, 3
%ifnidn %3, m5
    psubw            m5, %5               ; substruct pi_lo
    psubw            m6, %6               ; substruct pi_hi
%endif
    packuswb         %3, %4
%ifnidn %3, m5
%ifidn %1, p
    psubw            m5, %7               ; substruct p3_lo
    psubw            m6, %8               ; substruct p3_hi
%else
    paddw            m5, %7               ; substruct q3_lo
    paddw            m6, %8               ; substruct q3_hi
%endif
%endif
    mova     [flat2_%1 %+ %2], %3
%endm

%macro ADDING_QI 5
%ifnidn %5, q1
    lea         src_odq, [src_odq + pq]
%else
    lea         src_odq, [srcq + pq]
%endif
    movu             %1, [src_odq]        ; load qi
    mova             %2, %1
    mova           [%5], %1
    paddw            m5, %3
    punpcklbw        %2, m0               ; extent to 16bit lo and hi
    paddw            m6, %4
    punpckhbw        %1, m0
    paddw            m5, %2               ; add qi lo
    paddw            m6, %1               ; add qi hi
%endm

%macro SUBSTRUCTING_PI 4
    paddw            m5, %1
    paddw            m6, %2
    psubw            m5, %3               ; substruct pi lo
    psubw            m6, %4               ; substruct pi hi
%endm

%macro ABS_SUB 8
    mova             %1, %2
    mova             %3, %4
    mova             %5, %6
    mova             %7, %8
    psubusb          %1, %4               ; pi-p(i-1)
    psubusb          %3, %2               ; p(i-1)-pi
    psubusb          %5, %8               ; qi-q(i-1)
    psubusb          %7, %6               ; q(i-1)-qi
    por              %1, %3               ; |pi-p(i-1)|
    por              %5, %7               ; |qi-q(i-1)|
%endm

%macro CALC_MASK_PART1 4
%if ARCH_X86
    mov        blimit1q, dword blimit1m
%endif
    mova             %1, [blimit0q]
    pand             %2, [GLOBAL(fe)]     ; |q1-p1| & 254
    paddusb          %3, %3               ; |p0-q0|*2
    psrlw            %2, 1                ; (|p1-p1| & 254)>>1
    punpcklqdq       %1, [blimit1q]       ; concatinate bilimit0,bilmit1
    paddusb          %3, %2               ; mask=|p0-q0|*2+((|q1-p1|&254)>>1)
    pxor             %4, %4
    psubusb          %3, %1               ; mask=mask-bilimit
%endm

%macro CALC_MASK_PART2 7
    mova             %1, [p2]
    mova             %2, [p3]
    mova             %3, %4
    mova             %5, %1
%if ARCH_X86
    mov        thresh0q, dword thresh0m
%endif
    pcmpeqb          m1, %6               ; mask = (mask == 0)
    psubusb          %3, %1               ; p1-p2
    psubusb          %5, %2               ; p2-p3
    mova             %7, [thresh0q]
    psubusb          %2, %1               ; p3-p2
    psubusb          %1, %4               ; p2-p1
%if ARCH_X86
    mov             r5q, dword thresh1m
    por              %2, %5               ; |p3-p2|
    por              %1, %3               ; |p2-p1|
    punpcklqdq       %7, [r5q]            ; concatinate thresh0,thresh1
%else
    punpcklqdq       %7, [thresh1q]       ; concatinate thresh0,thresh1
%endif
%endm

%macro CALC_MASK_PART3 6
    mova             %1, [q2]             ; load q2
    mova             %2, [q3]             ; load q3
%if ARCH_X86
    mova          [hev], m3
%endif
    mova             %3, %1
    mova             %4, %5
%if ARCH_X86_64
    pmaxub           m1, m5               ; mask = max(mask, flat)
%endif
    psubusb          %3, %2               ; q2-q3
    psubusb          %4, %1               ; q1-q2
%if ARCH_X86_64
    pmaxub           m6, m13              ; tmp=max(|p2-p1|,|p3-p2|)
%endif
    psubusb          %2, %1               ; q3-q2
    pmaxub           m1, %6               ; mask=max(tmp,mask)
    psubusb          %1, %5               ; q2-q1
    por              %2, %3               ; |q3-q2|
    por              %1, %4               ; |q2-q1|
%endm

%macro CALC_FLAT 9
    mova             %1, [%9 %+ 2]        ; load p2/q2
    mova             %2, [%9 %+ 3]
    pmaxub           %8, %3               ; mask=max(mask,|q/p3-q/p2|)
%if ARCH_X86
    mova             %4, [%9 %+ 0]
    pmaxub           %8, %7               ; mask=max(mask,|q/p2-q/p1|)
%endif
    mova             %5, %4
    mova             %6, %4
%if ARCH_X86_64
    pmaxub           %8, %7               ; mask=max(mask,|q/p2-q/p1|)
%endif
    psubusb          %5, %1               ; p/q0-p/q2
    psubusb          %6, %2               ; p/q0-p/q3
    psubusb          %1, %4               ; p/q2-p/q0
    psubusb          %2, %4               ; p/q3-p/q0
    por              %1, %5               ; |p/q2-p/q0|
    por              %2, %6               ; |p/q3-p/q0|
%endm

%macro CALC_FLAT_MASK_LAST 7
%if ARCH_X86
    mov         limit0q, dword limit0m
%endif
    mova             %1, [limit0q]
%if ARCH_X86_64
    mova             %2, [GLOBAL(one)]
%endif
%if ARCH_X86
    mov             r4q, dword limit1m
    punpcklqdq       %1, [r4q]
%else
    punpcklqdq       %1, [limit1q]        ; concatinate limit0, limit1
%endif
    pmaxub           %3, %4               ; flat=max(flat, |q2-q0|)
    pxor             %7, %7
    pmaxub           %3, %5               ; flat=max(flat, |q3-q0|)
    psubusb          %6, %1               ; mask=mask-limit
    psubusb          %3, %2               ; flat=flat - 1
    pcmpeqb          %6, %7               ; mask=(mask == 0)
    pcmpeqb          %3, %7               ; flat=(flat == 0)
    movdqa           m6, [GLOBAL(t80)]
    pand             %3, %6               ; flat=flat & mask
%endm

%macro CALC_FILTER4_PART1 13-14
    pxor             %1, %2               ; Oq1=q1 ^ 128
    pxor             %3, %2               ; Op1=p1 ^ 128
    pxor             %4, %2               ; Oq0=q0 ^ 128
    pxor             %5, %2               ; Op0=p0 ^ 128
    mova             %6, %4
    mova             %7, %3
%if ARCH_X86
    mova             %8, [hev]
%endif
    psubsb           %3, %1               ; Op1 - Oq1
    psubsb           %4, %5               ; Oq0 - Op0
    pand             %3, %8               ; (Op1 - Oq1)& hev
%if ARCH_X86_64
    movdqa          %13, [GLOBAL(te0)]
%endif
    paddsb           %3, %4               ; filt=(Op1 - Oq1)&hev+(Oq0-Op0)*3
    paddsb           %3, %4
    paddsb           %3, %4
%if ARCH_X86_64
    movdqa          %14, [GLOBAL(t1f)]
%endif
    pand             %3, %9               ; filt=filt & mask
    mova            %10, %3
    paddsb           %3, [GLOBAL(t4)]     ; filter1=filt + 4
    paddsb          %10, [GLOBAL(t3)]     ; filter2=filt + 3
    pxor             m1, m1
%if ARCH_X86
    pxor            %13, %13
%endif
    mova            %11, %3
    mova            %12, %10
%endm

%macro CALC_FILTER4_PART2 15
    pcmpgtb          %1, %2               ; filter1 < 0
    pcmpgtb          %3, %4               ; filter2 < 0
    psrlw            %5, 3                ; filter1>>=3
    psrlw            %6, 3                ; filter2>>=3
    pand             %1, %14              ; work_a_filter1=(filter1 < 0)&224
    pand             %3, %14              ; work_a_filter2=(filter2 < 0)&224
    pxor             %2, %2
    pand             %5, %15              ; filter1=filter1 & 31
    pand             %6, %15              ; filter2=filter2 & 31
%if ARCH_X86
    mova             %7, [GLOBAL(one)]
%endif
    por              %5, %1               ; filter1=filter1 | work_a_filter1
    por              %6, %3               ; filter2=filter2 | work_a_filter2
    paddsb           %7, %5               ; filt=1 + fillter1
%if ARCH_X86
    mova             %8, [org_q0]
%endif
    pcmpgtb          %2, %7               ; work_a=(0 > filt)
    psrlw            %7, 1                ; filt<<=1
    pand             %2, %13              ; work_a=work_a & 128
    psubsb           %8, %5               ; Oq0_filter1=Oq0 - filter1
    paddsb           %6, m4               ; Op0_filter2=filter2 + Op0
    pand             %7, [GLOBAL(t7f)]    ; filt=filt & 127
%if ARCH_X86
    mova             %9, [flat]
%endif
    por              %7, %2               ; filt=filt | work_a
%if ARCH_X86
    mova            %10, [hev]
%endif
    mova            %11, %9
    mova            %12, %9
    pxor             %8, %13              ; Oq0_filter1=Oq0_filter1 ^ 128
    pxor             %6, %13              ; Op0_filter2=Op0_filter2 ^ 128
    pandn           %10, %7               ; filt=(~hev) & filt
    pandn           %11, %8               ; Oq0_NotFlat=(~flat) & Oq0_filter1
    pandn           %12, %6               ; Op0_NotFlat=(~flat) & Op0_filter2
%endm

%macro SAVE_FINAL_RESULT 5
    mova             %2, [flat2_q %+ %1]
    mova             %3, [flat2_p %+ %1]
    pand             %2, m5               ; Oqi_Flat=flat2_qi & flat
    pand             %3, m5               ; Opi_Flat=flat2_pi & flat
    por              %2, %4               ; qi=Oqi_Flat | Oqi_NotFlat
    por              %3, %5               ; pi=Opi_Flat | Opi_NotFlat
    movu      [src_evq], %2               ; store qi
    movu      [src_odq], %3               ; store pi
%endm

%macro CALC_Q1_P1_NOT_FLAT 6
    psubsb           %1, %2               ; Oq1_filt=Oq1 - filt
    paddsb           %2, %5               ; Op1_filt=filt + Op1
    pxor             %1, %6               ; Oq1_filt=Oq1_filt ^ 128
    pxor             %2, %6               ; Op1_filt=Op1_filt ^ 128
    add         src_evq, pq
    sub         src_odq, pq
    mova             %3, m5
    mova             %4, m5
    pandn            %3, %1               ; Oq1_NotFlat=(~flat) & Oq1_filt
    pandn            %4, %2               ; Op1_NotFlat=(~flat) & Op1_filt
%endm

%macro SAVE_RESULT_Q2_P2 6
    add         src_evq, pq
    sub         src_odq, pq
    mova             %1, m5
    mova             %2, m5
    mova             %3, [q2]
    mova             %4, [p2]
    pandn            %1, %3               ; Oq2_NotFlat=(~flat) & q2
    pandn            %2, %4               ; Op2_NotFlat=(~flat) & p2
    SAVE_FINAL_RESULT 2, %5, %6, %1, %2
%endm

%macro LOAD_P1_Q1_P0_Q0 0
    mova             m3, [p1]             ; load p1
    mova             m4, [p0]             ; load p0
    mova             m7, [q1]             ; load q1
    mova             m2, [q0]             ; load q0
%endm


cglobal lpf_horizontal_8_dual_sse2, 6+(ARCH_X86_64*2), 6+(ARCH_X86_64*6), 8+(ARCH_X86_64*8), 0x160+(ARCH_X86*0x090), \
                                           src, p, blimit0, limit0, thresh0, blimit1, limit1, thresh1
    SET_SOURCE_POINTER
%if ARCH_X86
    %xdefine limit0m  [esp + stack_offset + 4*3 + 4]
    %xdefine thresh0m [esp + stack_offset + 4*4 + 4]
    %xdefine blimit1m [esp + stack_offset + 4*5 + 4]
    %xdefine limit1m  [esp + stack_offset + 4*6 + 4]
    %xdefine thresh1m [esp + stack_offset + 4*7 + 4]
%endif

    xchg rbp, rsp
%if ARCH_X86
   %define pq2 r3q
%else
   %define pq2 r11q
%endif
    lea             pq2, [pq + pq]
    pxor             m0, m0
    ZERO_EXTEND      m6, m3, m5, m4, p0, q0
    mova        [q0_lo], m5
    mova        [q0_hi], m6
    mova             m2, [GLOBAL(four)]
    paddw            m5, m4
    paddw            m6, m3
%if ARCH_X86
    mova              [p0_lo], m4
    mova              [p0_hi], m3
    sub               src_evq, pq2
    sub               src_odq, pq2
    paddw                  m5, m2
    paddw                  m6, m2
    ZERO_EXTEND            m7, m1, m4, m2, p2, p1
    movu              [p2_lo], m2
    movu              [p2_hi], m1
    paddw                  m5, m4
    movu              [p1_lo], m4
    paddw                  m6, m7
    movu              [p1_hi], m7
    ADD2XP2                m3, m4
    sub               src_evq, pq2
    ADD3XP3                m7, m3, m4, m2
    SAVE_FLAT2_QI_PI        p, 2, m1, m4, \
    [p2_lo], [p2_hi], m3, m7

    ADDING_QI              m4, m2, [p1_lo], \
    [p1_hi], q1

    mova              [q1_lo], m2
    mova              [q1_hi], m4
    SAVE_FLAT2_QI_PI        p, 1, m1, m4, \
    [p1_lo], [p1_hi], m3, m7

    ADDING_QI              m4, m2, [p0_lo], \
    [p0_hi], q2

    mova              [q2_lo], m2
    mova              [q2_hi], m4
    SAVE_FLAT2_QI_PI        p, 0, m1, m4, \
    [p0_lo], [p0_hi], m3, m7

    ADDING_QI              m4, m2, [q0_lo], \
    [q0_hi], q3

    SAVE_FLAT2_QI_PI        q, 0, m1, m7, \
    [q0_lo], [q0_hi], m2, m4

    mova                   m0, [q1_lo]
    mova                   m3, [q1_hi]
    SUBSTRUCTING_PI        m0, m3, [p2_lo], \
    [p2_hi]

    SAVE_FLAT2_QI_PI        q, 1, m1, m7, m0, \
    m3, m2, m4

    SUBSTRUCTING_PI   [q2_lo], [q2_hi], \
    [p1_lo], [p1_hi]

    SAVE_FLAT2_QI_PI        q, 2, m5, m6
    LOAD_P1_Q1_P0_Q0
    ABS_SUB                m0, m3, m1, m4, m5, \
    m7, m6, m2

    mova                   m1, m4
    mova                   m6, m2
    psubusb                m1, m2                ; p0-q0
    psubusb                m6, m4                ; q0-p0
    pmaxub                 m0, m5                ; flat = max(|q1-q0|,|p1-p0|)
    mova               [flat], m0
    por                    m1, m6                ; |p0-q0|
    mova                   m0, m3
    mova                   m6, m7
    psubusb                m0, m7                ; p1-q1
    psubusb                m6, m3                ; q1-p1
    por                    m0, m6                ; |p1-q1|
    CALC_MASK_PART1        m5, m0, m1, m0
    CALC_MASK_PART2        m5, m6, m4, m3, m2,\
    m0, m0
    mova                   m3, [flat]
    mova                   m2, [GLOBAL(ff)]
    pxor                   m1, m2                ; mask = mask ^ 255
    pxor                   m4, m4
    pmaxub                 m1, m3                ; mask = max(mask, flat)
    psubusb                m3, m0                ; hev=flat-thresh
    pmaxub                 m5, m6                ; max (|p2-p1|,|p3-p2|)
    pcmpeqb                m3, m4                ; hev = hev == 0
    pxor                   m3, m2                ; hev = hev ^ 255
    CALC_MASK_PART3        m4, m2, m6, m0, m7, \
    m5

    CALC_FLAT              m5, m6, m2, m7, m0, \
    m2, m4, m1, p

    mova                   m0, [flat]
    CALC_FLAT              m4, m2, m5, m3, m5, \
    m7, m6, m0, q

    CALC_FLAT_MASK_LAST    m6,  [GLOBAL(one)], \
    m0, m4, m2, m1, m5

    mova                   m7, [q1]
    mova                   m2, [p1]
    mova                   m4, [p0]
    mova               [flat], m0
    CALC_FILTER4_PART1     m7, m6, m2, m3, m4, \
    [org_q0], [org_p1], m5, m1, m0, m5, m6, m3

    CALC_FILTER4_PART2     m1, m2, m3, m0, m5, \
    m6, m0, m1, m5, m4, m3, m2, [GLOBAL(t80)], \
    [GLOBAL(te0)], [GLOBAL(t1f)]

    SET_SOURCE_POINTER
    SAVE_FINAL_RESULT       0, m1, m0, m3, m2
    CALC_Q1_P1_NOT_FLAT    m7, m4, m6, m3, \
    [org_p1], [GLOBAL(t80)]

    SAVE_FINAL_RESULT       1, m2, m0, m6, m3
    SAVE_RESULT_Q2_P2      m7, m2, m1, m6, m4, m3
%else
    sub            src_evq, pq2
    sub            src_odq, pq2
    paddw               m5, m2
    paddw               m6, m2
    ZERO_EXTEND         m7, m1, m8, m2, p2, p1
    paddw               m5, m8
    paddw               m6, m7
    ADD2XP2             m9, m10
    sub            src_evq, pq2
    ADD3XP3            m11, m12, m10, m9
    SAVE_FLAT2_QI_PI     p, 2, m13, m14, m2, \
    m1, m12, m11

    ADDING_QI          m10, m9, m8, m7, q1
    SAVE_FLAT2_QI_PI     p, 1, m13, m14, m8, \
    m7, m12, m11

    ADDING_QI          m15, m14, m4, m3, q2
    SAVE_FLAT2_QI_PI     p, 0, m13, m0, m4, \
    m3, m12, m11

    pxor                m0, m0
    ADDING_QI           m4, m3, [q0_lo], \
    [q0_hi], q3

    SAVE_FLAT2_QI_PI     q, 0, m13, m12, \
    [q0_lo], [q0_hi], m3, m4

    SUBSTRUCTING_PI     m9, m10, m2, m1
    SAVE_FLAT2_QI_PI     q, 1, m13, m12, \
    m9, m10, m3, m4

    SUBSTRUCTING_PI    m14, m15, m8, m7
    SAVE_FLAT2_QI_PI     q, 2, m5, m6
    LOAD_P1_Q1_P0_Q0
    ABS_SUB              m9, m3, m10, m4, \
    m11, m7, m12, m2

    ABS_SUB              m1, m4, m0, m2, \
    m5, m3, m8, m7

    pmaxub               m9, m11                 ; flat = max(|q1-q0|,|p1-p0|)
    CALC_MASK_PART1     m10, m5, m1, m15

    mova                 m5, m9
    CALC_MASK_PART2      m6, m13, m12, m3, \
    m14, m15, m8

    mova                m11, [GLOBAL(ff)]
    psubusb              m9, m8                  ; hev=flat-thresh
    pxor                 m1, m11                 ; mask = mask ^ 255
    pcmpeqb              m9, m15                 ; hev = hev == 0
    por                 m13, m14                 ; |p3-p2|
    por                  m6, m12                 ; |p2-p1|
    pxor                 m9, m11                 ; hev = hev ^ 255
    CALC_MASK_PART3      m8, m0, m14, m10, \
    m7, m6

    CALC_FLAT           m11, m12, m0, m4, \
    m13, m6, m8, m1, p

    CALC_FLAT           m13, m8, m11, m2, \
    m0, m6, m12, m5, q

    CALC_FLAT_MASK_LAST m10, m11, m5, m13, \
    m8, m1, m15
    CALC_FILTER4_PART1   m7, m6, m3, m2, \
    m4, m0, m14, m9, m1, m8, m10, m2, m13, m12

    CALC_FILTER4_PART2  m15, m3, m1, m8, \
    m10, m2, m11, m0, m5, m9, m8, m13, m6, \
    m13, m12

    SET_SOURCE_POINTER
    SAVE_FINAL_RESULT     0, m11, m4, m8, m13
    CALC_Q1_P1_NOT_FLAT  m7, m9, m10, m11, \
    m14, m6

    SAVE_FINAL_RESULT     1, m15, m12, m10, \
    m11

    SAVE_RESULT_Q2_P2    m4, m2, m11, m8, \
    m15, m12

%endif
    xchg rbp, rsp
    RET
