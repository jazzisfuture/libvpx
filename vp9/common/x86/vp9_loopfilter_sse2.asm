
%include "vpx_ports/x86_abi_support.asm"

;void vp9_lpf_horizontal_8_dual_sse2
;(
; uint8_t *s,
; int p,
; const uint8_t *_blimit0,
; const uint8_t *_limit0,
; const uint8_t *_thresh0,
; const uint8_t *_blimit1,
; const uint8_t *_limit1,
; const uint8_t *_thresh1)
global sym(vp9_lpf_horizontal_8_dual_sse2) PRIVATE
sym(vp9_lpf_horizontal_8_dual_sse2):
	push    rbp
        mov     rbp, rsp
        SHADOW_ARGS_TO_STACK 8
        SAVE_XMM 7
        push	rdi
        push	rsi
        push	rbx
        ALIGN_STACK 16, rax
%if ABI_IS_32BIT=0
        sub	rsp, 288
        mov	rdx, arg(0)
        mov	rax, arg(1)
        mov	rsi, rdx
        mov	rdi, rdx
        sub	rsi, rax
        ; symmetric computation calculating p2,p1,p0,q0,q1,q2 when flat==1
        ; and mask==1
        pxor xmm0, xmm0
        ; load q0, p0
        movdqu	xmm3, [rdi]
        movdqu	xmm7, [rsi]
        movdqa	xmm4, xmm3
        movdqa	xmm5, xmm7
        ; extend from 8bit to 16bit
        punpcklbw xmm4, xmm0
        movdqa	xmm6, xmm7
        movdqa	[rsp], xmm3
        movdqa	[rsp+128], xmm4
        punpcklbw xmm5, xmm0
        punpckhbw xmm3, xmm0
        lea	rcx, [rax*2]
        punpckhbw xmm6, xmm0
        movdqa	xmm8, xmm5
        movdqa	xmm15, xmm6
        movdqa	[rsp+176], xmm3
        ; calculate sum = q0+ p0
        paddw	xmm5, xmm4
         sub	rsi, rcx
        paddw	xmm6, xmm3
        movdqa	[rsp+16], xmm7
        movdqa	xmm2, [GLOBAL(LC0)]
        sub	rdi, rcx
        ; calculate sum+=4
        paddw	xmm5, xmm2
        paddw	xmm6, xmm2
        ; load p1, p2 and extend
        movdqu	xmm4, [rsi]
        movdqu	xmm7, [rdi]
        movdqa	xmm3, xmm4
        movdqa	xmm2, xmm7
        punpcklbw xmm4, xmm0
        movdqa	[rsp+32], xmm3
        movdqa	xmm10, xmm4
        movdqa	[rsp+48], xmm7
        punpckhbw xmm3, xmm0
        ; calculate p2*2
        psllw	xmm4, 1
        punpcklbw xmm7, xmm0
        movdqa	xmm11, xmm3
        ;movdqa	xmm12, xmm7(64)
        punpckhbw xmm2, xmm0
        sub	rdi, rcx
        psllw	xmm3, 1
        ; caulcate sum+=p2*2
        paddw	xmm6, xmm3
        paddw	xmm5, xmm4
        ; load p3 and extend
        movdqu	xmm3, [rdi]
        ; add sum+=p1
        paddw	xmm5, xmm7
        paddw	xmm6, xmm2
        movdqa	[rsp+64], xmm3
        ;movdqa  xmm13, xmm2
        movdqa	xmm4, xmm3
        punpcklbw xmm3, xmm0
        punpckhbw xmm4, xmm0
        movdqa  xmm13, xmm3
        psllw xmm3, 1
        ; add sum+=p3
        paddw xmm6, xmm4
        paddw xmm5, xmm13
        lea rsi, [rdx+rax]
        movdqa  xmm14, xmm4
        ;movdqa  [rsp+416], xmm13
        psllw xmm4, 1
        ; add sum+=p3*2
        paddw xmm5, xmm3
        paddw xmm6, xmm4
        movdqa  xmm1,  xmm5
        movdqa	xmm12, xmm6
        ; sum>>=3
        psrlw xmm1, 3
        ; sum-=p3
        psubw xmm5, xmm13
        psrlw xmm12, 3
        psubw xmm6, xmm14
         ; sum-=p2
        psubw xmm5, xmm10
        ; sum-=p2
        packuswb xmm1, xmm12
        ; load q1 and extend
        movdqu xmm3, [rsi]
         ; add p1
        paddw xmm5, xmm7
        movdqa [rsp+80], xmm3
        ; save p2
        movdqa [rsp+192], xmm1
        psubw xmm6, xmm11
        movdqa xmm4, xmm3
        punpcklbw xmm4, xmm0
        punpckhbw xmm3, xmm0
         ; sum+=q1
        paddw xmm5, xmm4
        ; sum+=p1
        paddw xmm6, xmm2
        ;movdqa [rsp+144], xmm4
        ;movdqa [rsp+160], xmm3
        paddw xmm6, xmm3
        movdqa xmm12, xmm5
        movdqa xmm1, xmm6
        ; sum-=p3
        psubw  xmm5, xmm13
        ; sum>>=3
        psrlw  xmm12, 3
        lea    rsi,  [rsi+rax]
        psubw  xmm6, xmm14
        psrlw  xmm1, 3
        ; load q2 and extend
        movdqu xmm9, [rsi]
        packuswb xmm12, xmm1
        ; sum-=p1
        psubw  xmm5, xmm7
         ; save p1
        movdqa [rsp+208], xmm12
        psubw  xmm6, xmm2
        movdqa xmm12, xmm9
        ; sum+=p0
        paddw xmm5, xmm8
        movdqa [rsp+96], xmm9
        paddw xmm6, xmm15
        punpckhbw xmm9, xmm0
        punpcklbw xmm12, xmm0

        ; sum+=q2
        paddw xmm6, xmm9
        movdqa [rsp+144], xmm9
        movdqa [rsp+160], xmm12

        paddw xmm5, xmm12
        movdqa xmm1, xmm6
        movdqa xmm12, xmm5
         ; load q3 and extend
        movdqu xmm9, [rsi+rax]
        ; sum-=p3
        psubw xmm6, xmm14
        psubw xmm5, xmm13
        ; sum>>=3
        psrlw xmm12, 3
        psrlw xmm1, 3
        ; sum-=p0
        psubw xmm5, xmm8
        psubw  xmm6, xmm15
        movdqa [rsp+112], xmm9
        packuswb xmm12, xmm1
        movdqa xmm13, [rsp+128]
        movdqa xmm14,  xmm9
        ; sum+=q0
        paddw  xmm5, xmm13
        punpcklbw xmm9, xmm0
        ; save p0
        movdqa   [rsp+224], xmm12
        paddw xmm6, [rsp+176]

        punpckhbw xmm14, xmm0
        ; sum+=q3
        paddw xmm5, xmm9
        movdqa xmm12, xmm5
        paddw xmm6, xmm14
        psrlw xmm12, 3
        ; sum-=p2
        psubw xmm5, xmm10
        movdqa xmm1, xmm6
         ; sum-=q0
        psubw xmm5, xmm13
        psubw xmm6, xmm11
        psrlw xmm1, 3
        psubw xmm6, [rsp+176]

        packuswb xmm12, xmm1
        ; sum+=q3
        paddw xmm5, xmm9
        ; sum+=q1
        paddw xmm5, xmm4
        ; save q0
        movdqa [rsp+240], xmm12
        paddw xmm6, xmm3

        movdqa xmm11, xmm5
        paddw xmm6, xmm14
        movdqa xmm1, xmm6
        psrlw xmm11, 3
        ; sum-=p1
        psubw xmm5, xmm7
        psubw xmm6, xmm2
        ; sum-=q1
        psubw xmm5, xmm4
        psrlw xmm1, 3
        packuswb xmm11, xmm1
        psubw xmm6, xmm3

        ; sum+=q2
        paddw xmm5, [rsp+160]

        ; save q1
        movdqa [rsp+256], xmm11
        paddw xmm6, [rsp+144]
        ; sum+=q3
        paddw xmm5, xmm9
        ; load blimit0
        mov rbx, arg(2)
        paddw xmm6, xmm14
        ; sum>>=3
        psrlw xmm5, 3
        ; load p1
        movdqa xmm4, [rsp+48]
        ; load p0
        movdqa xmm7, [rsp+16]
        psrlw xmm6, 3
        packuswb xmm5, xmm6
        ; load q1
        movdqa xmm3, [rsp+80]
        ; load q0
        movdqa xmm2, [rsp]
        ; save q2
        movdqa [rsp+272], xmm5
        movdqa xmm5, xmm4
        movdqa xmm1, xmm7
        movdqa xmm6, xmm3
        ; p1-p0
        psubusb xmm5, xmm7
        ; p0-p1
        psubusb xmm1, xmm4
        ; q1-q0
        psubusb xmm6, xmm2
        ; |p1-p0|
        por xmm5, xmm1
        movdqa xmm1, xmm2
        movdqa  xmm0, xmm2
        ; q0-q1
        psubusb xmm1, xmm3
        ; |q1-q0|
        por xmm1, xmm6
        ; q0-p0
        psubusb xmm0, xmm7
        movdqa  xmm6, xmm7
        movdqa xmm8, xmm3
        ; p0-q0
        psubusb xmm6, xmm2
        movdqa xmm9, xmm4
        ; |p0-q0|
        por xmm0, xmm6
        ; q1-p1
        psubusb xmm8, xmm4
        movdqa  xmm6, [rbx]
        ; p1-q1
        psubusb xmm9, xmm3
        ; flat = max(|q1-q0|,|p1-p0|)
        pmaxub xmm1, xmm5
        ; |p1-q1|
        por xmm8, xmm9
        mov rbx, arg(5)
        ; |p1-p0| & 254
        pand xmm8, [GLOBAL(LC1)]
        movdqa xmm12, xmm1
        ; load bilimit1
        movdqa xmm10, [rbx]
        ; |p0-q0|*2
        paddusb xmm0, xmm0
        ;  (|p1-p0| & 254)>>1
        psrlw xmm8, 1
        ; load p2
        movdqa xmm11, [rsp+32]
        ; concatinate bilimit0,bilmit1
        punpcklqdq xmm6, xmm10
        ; |p0-q0|*2 + ((|p1-p0| & 254)>>1)
        paddusb xmm0, xmm8
        mov rbx, arg(4)
        ; load p3
        movdqa xmm10, [rsp+64]
        ; mask = (p0-q0|*2 + ((|p1-p0| & 254)>>1)) - bilimit
        psubusb xmm0, xmm6
        pxor xmm15, xmm15
        ; load thresh0
        movdqa xmm6, [rbx]
        movdqa xmm9, xmm11
        mov rbx, arg(7)
        ; p2-p3
        psubusb xmm9, xmm10
        movdqa xmm8, [rbx]
        ; mask = (mask == 0)
        pcmpeqb xmm0, xmm15
        ; concatinate thresh0,thresh1
        punpcklqdq xmm6, xmm8
        ; p3-p2
        psubusb xmm10, xmm11
        movdqa xmm8, [GLOBAL(LC9)]
        ; hev=flat-thresh
        psubusb xmm1, xmm6
        ; mask = mask ^ 255
        pxor xmm0, xmm8
        ; hev = hev == 0
        pcmpeqb xmm1, xmm15
        ; |p3-p2|
        por xmm10, xmm9
        ; flat
        ;movdqa xmm12, [rsp+96]
        ; hev = hev ^ 255
        pxor xmm1, xmm8
        ; mask = max(mask, flat)
        pmaxub xmm0, xmm12
        ;movdqa [rsp+64], xmm1
        ; load p1
        ;movdqa xmm6, [rsp+16]
        ; load q3
        movdqa xmm13, [rsp+112]
        movdqa xmm9, xmm4
        ; p1-p2
        psubusb xmm9, xmm11
        ; load q2
        movdqa xmm8, [rsp+96]
        ; p2-p1
        psubusb xmm11, xmm4
        movdqa xmm5, xmm8
        mov rbx, arg(3)
        ; |p2-p1|
        por xmm11, xmm9
        ; q2-q3
        psubusb xmm5, xmm13
        ; max (|p2-p1|,|p3-p2|)
        pmaxub xmm11, xmm10
        ; q3-q2
        psubusb xmm13, xmm8
        ; mask = max(max (|p2-p1|,|p3-p2|), mask)
        pmaxub xmm11, xmm0
        ; load q1
        ;movdqa xmm3, [rsp]
        ; |q3-q2|
        por xmm5, xmm13
        movdqa xmm14, [rbx]
        movdqa xmm6, xmm3
        ; load q0
        ;movdqa xmm4, [rsp+48]
        mov rbx, arg(6)
        ; mask = max(mask,|q3-q2|)
        pmaxub xmm11, xmm5
        movdqa xmm5, xmm8
        movdqa xmm0, xmm2
        ; q1-q2
        psubusb xmm6, xmm8
        ; q0-q2
        psubusb xmm0, xmm5
        ; concatinate limit0, limit1
        punpcklqdq xmm14, [rbx]
        ; q2-q1
        psubusb xmm8, xmm3
        ; q2-q0
        psubusb xmm5, xmm2
        ; |q2-q1|
        por xmm6, xmm8
        ; mask= max(mask,|q2-q1|)
        pmaxub xmm11, xmm6
        ; load p0
        ;movdqa xmm7, [rsp+80]
        ; mask = mask-limit
        psubusb xmm11, xmm14
        ; |q2-q0|
        por xmm0, xmm5
        movdqa xmm13, xmm7
        ; mask = (mask == 0)
        pcmpeqb xmm11, xmm15
        ; load p2
        movdqa xmm5, [rsp+32]
        ; load q3
        movdqa xmm6, [rsp+112]
        ; p0-p2
        psubusb xmm13, xmm5
        movdqa xmm9, xmm2
        ; p2-p0
        psubusb xmm5, xmm7
        ; q0-q3
        psubusb xmm9, xmm6
        ; |p2-p0|
        por xmm13, xmm5
        ; q3-q0
        psubusb xmm6, xmm2
        ; load flat
        ;movdqa xmm12, [rsp+96]
        ; work=max(|q2-q0|,|p2-p0|)
        pmaxub xmm0, xmm13
        ; |q3-q0|
        por xmm9, xmm6
        ; flat = max(work, flat)
        pmaxub xmm0, xmm12
        movdqa xmm8, [GLOBAL(LC3)]
        ; load p1
        ;movdqa xmm5, [rsp+16]
        ; Oq0 = q0 ^ 128
        pxor xmm2, xmm8
        ; Op1 = p1 ^ 128
        pxor xmm4, xmm8
        ;movdqa [rsp+16], xmm4
        ; load p3
        movdqa xmm10, [rsp+64]
        ;movdqa [rsp+48], xmm5
        movdqa xmm12, xmm7
        ; p0-p3
        psubusb xmm12, xmm10
        ; load q1
        ;movdqa xmm5, [rsp]
        ; p3-p0
        psubusb xmm10, xmm7
        ; Oq1 = q1 ^ 128
        pxor xmm3, xmm8
        ; |p3-p0|
        por xmm10, xmm12
        ; Op0 = p0 ^ 128
        pxor xmm7, xmm8
        ;movdqa [rsp+32], xmm3
        ; work = max(|p3-p0|, |q3-q0|)
        pmaxub xmm10, xmm9
        ;movdqa [rsp], xmm7
        movdqa xmm13, xmm4
        ; load Op1
        ;movdqa xmm6, [rsp+48]
        movdqa xmm6, [GLOBAL(LC2)]
        ; flat = max(work, flat)
        pmaxub xmm10, xmm0
        ; Op1 - Oq1
        psubsb xmm13, xmm3
        ; flat = flat - 1
        psubusb xmm10, xmm6
        ; load Oq0
        movdqa xmm0, xmm15
        ;movdqa xmm2, [rsp+16]
        movdqa xmm12, xmm2
        ; flat = (flat == 0)
        pcmpeqb xmm10, xmm15
        ; (Oq1 - Op1)& hev
        pand xmm13, xmm1
        ; Oq0-Op0
        psubsb xmm12, xmm7
        ; flat = flat & mask
        pand xmm10, xmm11
        ; filt = (Oq1 - Op1)& hev + (Oq0-Op0)*3
        paddsb xmm13, xmm12
        paddsb xmm13, xmm12
        paddsb xmm13, xmm12
        ; filt = filt & mask
        pand xmm13, xmm11
        movdqa xmm12, xmm13
        movdqa xmm14, [GLOBAL(LC6)]

        ; filter1 = filt + 4
        paddsb xmm12, [GLOBAL(LC4)]
        ; filter2 = filt + 3
        paddsb xmm13, [GLOBAL(LC5)]
        ; filter1 < 0
        pcmpgtb xmm15, xmm12
        ; filter2 < 0
        pcmpgtb xmm0, xmm13
        ; filter1>>=1
        psrlw xmm12, 3
        movdqa xmm5, [GLOBAL(LC7)]
        ; filter2>>=1
        psrlw xmm13, 3
        ; work_a_filter1 = (filter1 < 0) & 224
        pand xmm15, xmm14
        ; filter1 = filter1 & 31
        pand xmm12, xmm5
        ; work_a_filter2 = (filter2 < 0) & 224
        pand xmm14, xmm0
        ; filter1 = filter1 | work_a_filter1
        por xmm12, xmm15
        ; filter2 = filter2 & 31
        pand xmm13, xmm5
        pxor xmm0, xmm0
        ; filt = 1 + fillter1
        paddsb xmm6, xmm12
        movdqa xmm11, [GLOBAL(LC3)]
        ; work_a = (0 < filt)
        pcmpgtb xmm0, xmm6
        ; filter2 = filter2 | work_a_filter2
        por xmm13, xmm14
        ; filt<<=1
        psrlw xmm6, 1
        ; work_a = work_a & 128
        pand xmm0, xmm11
        ; Op0_filter2 = filter2 + Op0
        paddsb xmm13, xmm7
        ; filt = filt & 127
        pand xmm6, [GLOBAL(LC8)]
        ;movdqa xmm7, [rsp+64]
        movdqa xmm14, xmm10
        ; filt = filt | work_a
        por xmm6, xmm0
        ; Op0_filter2 = Op0_filter2 ^ 128
        pxor xmm13, xmm11
        ; filt = (~hev) & filt
        pandn xmm1, xmm6
        ;movdqa xmm0, [rsp+16]
        ;movdqa xmm4, [rsp+32]
        ; Op0_NotFlat = (~flat) & Op0_filter2
        pandn xmm14, xmm13
        ; Oq0_filter1 = Oq0 - filter1
        psubsb xmm2, xmm12
        ; Oq1_filt = Oq1 - filt
        psubsb xmm3, xmm1
        mov rdi, rdx
        ; Op1_filt = filt + Op1
        paddsb xmm1, xmm4
        ; Oq0_filter1 = Oq0_filter1 ^ 128
        pxor xmm2, xmm11
        ; Op1_filt = Op1_filt ^ 128
        pxor xmm1, xmm11
        movdqa xmm6, xmm10
        sub rdi, rcx
         ; Op1_NotFlat =  (~flat) & Op1_filt
        pandn xmm6, xmm1
        movdqa xmm5, [rsp+208]
        movdqa xmm7, [rsp+224]
        ; Op1_Flat = p1 & flat
        pand xmm5, xmm10
        ; p1 = Op1_Flat | Op1_NotFlat
        por xmm5, xmm6
        ; Oq1_filt = Oq1_filt ^ 128
        pxor xmm3, xmm11
        ; save p1
        movdqu [rdi], xmm5
        ; Op0_Flat = p0 & flat
        pand xmm7, xmm10
        add rdi, rax
        movdqa xmm13, xmm10
        ; p0 = Op0_Flat | Op0_NotFlat
        por xmm7, xmm14
        ; q0_NotFlat = (~flat) & Oq0_filter1
        pandn xmm13, xmm2
        ; save p0
        movdqu [rdi], xmm7
        movdqa xmm12, [rsp+240]
        movdqa xmm6, xmm10
        ; Oqo_Flat = q0 & flat
        pand xmm12, xmm10
        movdqa xmm0, [rsp+256]
        ; q1_NotFlat = (~flat) & Oq1_filt
        pandn xmm6, xmm3
        ; q0 = q0_NotFlat | Oqo_Flat
        por xmm13, xmm12
        ; q1_Flat = q1 & flat
        pand xmm0, xmm10
        ; save q0
        movdqu [rdx],xmm13
        movdqa xmm3, [rsp+96]
        ; q1 = q1_Flat & q1_NotFlat
        por xmm0, xmm6
        sub rdi, rcx
        movdqa xmm4, xmm10
        ; save q1
        movdqu [rdx+rax], xmm0
        ; Oq2_NotFlat = (~flat) & q2
        pandn xmm4, xmm3
        movdqa xmm6, xmm10
        movdqa xmm7, [rsp+272]
        movdqa xmm5, [rsp+32]
        ; Oq2_Flat = Oq2 & flat
        pand xmm7, xmm10
          ; Op2_NotFlat = (~flat) & p2
        pandn xmm6, xmm5
        ; q2 = Oq2_NotFlat | Oq2_Flat
        por xmm4, xmm7
        ; Op2_Flat = flat & Op2
        pand xmm10, [rsp+192]
        ; save q2
        movdqu [rsi], xmm4
        ; p2 = Op2_NotFlat | Op2_Flat
        por xmm6, xmm10
        ; save p2
        movdqu [rdi], xmm6
        add rsp, 288
%else
        sub	rsp, 432
        mov	rdx, arg(0)
        mov	rax, arg(1)
        mov	rsi, rdx
        mov	rdi, rdx
        sub	rsi, rax
        ; symmetric computation calculating p2,p1,p0,q0,q1,q2 when flat==1
        ; and mask==1
        pxor xmm0, xmm0
        ; load q0, p0
        movdqu	xmm3, [rdi]
        movdqu	xmm7, [rsi]
        movdqa	xmm4, xmm3
        movdqa	xmm5, xmm7
        ; extend from 8bit to 16bit
        punpcklbw xmm4, xmm0
        movdqa	xmm6, xmm7
        movdqa	[rsp+48], xmm3
        movdqa	[rsp+96], xmm4
        punpcklbw xmm5, xmm0
        punpckhbw xmm3, xmm0
        lea	rcx, [rax*2]
        punpckhbw xmm6, xmm0
        movdqa	[rsp+192], xmm5
        movdqa	[rsp+208], xmm6
        movdqa	[rsp+112], xmm3
        ; calculate sum = q0+ p0
        paddw	xmm5, xmm4
         sub	rsi, rcx
        paddw	xmm6, xmm3
        movdqa	[rsp+80], xmm7
        movdqa	xmm2, [GLOBAL(LC0)]
        sub	rdi, rcx
        ; calculate sum+=4
        paddw	xmm5, xmm2
        paddw	xmm6, xmm2
        ; load p1, p2 and extend
        movdqu	xmm4, [rsi]
        movdqu	xmm7, [rdi]
        movdqa	xmm3, xmm4
        movdqa	xmm2, xmm7
        punpcklbw xmm4, xmm0
        movdqa	[rsp+128], xmm3
        movdqa	[rsp+224], xmm4
        movdqa	[rsp+16], xmm7
        punpckhbw xmm3, xmm0
        ; calculate p2*2
        psllw	xmm4, 1
        punpcklbw xmm7, xmm0
        movdqa	[rsp+240], xmm3
        movdqa	[rsp+64], xmm7
        punpckhbw xmm2, xmm0
        sub	rdi, rcx
        psllw	xmm3, 1
        ; caulcate sum+=p2*2
        paddw	xmm6, xmm3
        paddw	xmm5, xmm4
        ; load p3 and extend
        movdqu	xmm3, [rdi]
        ; add sum+=p1
        paddw	xmm5, xmm7
        paddw	xmm6, xmm2
        movdqa	[rsp+32], xmm3
        movdqa  [rsp+272], xmm2
        movdqa	xmm4, xmm3
        punpcklbw xmm3, xmm0
        punpckhbw xmm4, xmm0
        movdqa  xmm7, xmm3
        psllw xmm3, 1
        ; add sum+=p3
        paddw xmm6, xmm4
        paddw xmm5, xmm7
        lea rsi, [rdx+rax]
        movdqa  [rsp+256], xmm4
        movdqa  [rsp+416], xmm7
        psllw xmm4, 1
        ; add sum+=p3*2
        paddw xmm5, xmm3
        paddw xmm6, xmm4
        movdqa  xmm1,  xmm5
        movdqa	xmm2, xmm6
        ; sum>>=3
        psrlw xmm1, 3
        ; sum-=p3
        psubw xmm5, xmm7
        psrlw xmm2, 3
        psubw xmm6, [rsp+256]
         ; sum-=p2
        psubw xmm5, [rsp+224]
        ; sum-=p2
        packuswb xmm1, xmm2
        ; load q1 and extend
        movdqu xmm3, [rsi]
         ; add p1
        paddw xmm5, [rsp+64]
        movdqa [rsp], xmm3
        ; save p2
        movdqa [rsp+288], xmm1
        psubw xmm6, [rsp+240]
        movdqa xmm4, xmm3
        punpcklbw xmm4, xmm0
        punpckhbw xmm3, xmm0
         ; sum+=q1
        paddw xmm5, xmm4
        ; sum+=p1
        paddw xmm6, [rsp+272]
        movdqa [rsp+144], xmm4
        movdqa [rsp+160], xmm3
        paddw xmm6, xmm3
        movdqa xmm2, xmm5
        movdqa xmm1, xmm6
        ; sum-=p3
        psubw  xmm5, xmm7
        ; sum>>=3
        psrlw  xmm2, 3
        lea    rsi,  [rsi+rax]
        psubw  xmm6, [rsp+256]
        psrlw  xmm1, 3
        ; load q2 and extend
        movdqu xmm7, [rsi]
        ; sum-=p1
        psubw  xmm5, [rsp+64]
        movdqa xmm4, xmm7
        psubw  xmm6, [rsp+272]
        ; sum+=p0
        paddw xmm5, [rsp+192]
        movdqa [rsp+176], xmm7
        paddw xmm6, [rsp+208]
        punpckhbw xmm4, xmm0
        punpcklbw xmm7, xmm0
         packuswb xmm2, xmm1
        ; sum+=q2
        paddw xmm6, xmm4
        movdqa [rsp+336], xmm4
        movdqa [rsp+320], xmm7
         ; save p1
        movdqa [rsp+304], xmm2
        paddw xmm5, xmm7
        movdqa xmm1, xmm6
        movdqa xmm2, xmm5
         ; load q3 and extend
        movdqu xmm4, [rsi+rax]
        ; sum-=p3
        psubw xmm6, [rsp+256]
        psubw xmm5, [rsp+416]
        ; sum>>=3
        psrlw xmm2, 3
        psrlw xmm1, 3
        ; sum-=p0
        psubw xmm5, [rsp+192]
        psubw  xmm6, [rsp+208]
        movdqa [rsp+192], xmm4
        packuswb xmm2, xmm1
        movdqa xmm3,  xmm4
        ; sum+=q0
        paddw  xmm5, [rsp+96]
        punpcklbw xmm4, xmm0
        ; save p0
        movdqa   [rsp+352], xmm2
        paddw xmm6, [rsp+112]

        punpckhbw xmm3, xmm0
        ; sum+=q3
        paddw xmm5, xmm4
        movdqa xmm2, xmm5
        paddw xmm6, xmm3
        psrlw xmm2, 3
        ; sum-=p2
        psubw xmm5, [rsp+224]
        movdqa xmm1, xmm6
         ; sum-=q0
        psubw xmm5, [rsp+96]
        psubw xmm6, [rsp+240]
        psrlw xmm1, 3
        psubw xmm6, [rsp+112]

        packuswb xmm2, xmm1
        ; sum+=q3
        paddw xmm5, xmm4
        ; sum+=q1
        paddw xmm5, [rsp+144]
        ; save q0
        movdqa [rsp+368], xmm2
        paddw xmm6, [rsp+160]

        movdqa xmm2, xmm5
        paddw xmm6, xmm3
        movdqa xmm1, xmm6
        psrlw xmm2, 3
        ; sum-=p1
        psubw xmm5, [rsp+64]
        psubw xmm6, [rsp+272]
        ; sum-=q1
        psubw xmm5, [rsp+144]
        psrlw xmm1, 3
        packuswb xmm2, xmm1
        psubw xmm6, [rsp+160]
        ; sum+=q2
        paddw xmm5, [rsp+320]
        ; save q1
        movdqa [rsp+384], xmm2
        paddw xmm6, [rsp+336]
        ; sum+=q3
        paddw xmm5, xmm4
        ; load blimit0
        mov rbx, arg(2)
        paddw xmm6, xmm3
        ; sum>>=3
        psrlw xmm5, 3
        ; load p1
        movdqa xmm4, [rsp+16]
        ; load p0
        movdqa xmm7, [rsp+80]
        psrlw xmm6, 3
        packuswb xmm5, xmm6
        ; load q1
        movdqa xmm3, [rsp]
        ; load q0
        movdqa xmm2, [rsp+48]
        ; save q2
        movdqa [rsp+400], xmm5
        movdqa xmm5, xmm4
        movdqa xmm1, xmm7
        movdqa xmm6, xmm3
        ; p1-p0
        psubusb xmm5, xmm7
        ; p0-p1
        psubusb xmm1, xmm4
        ; q1-q0
        psubusb xmm6, xmm2
        ; |p1-p0|
        por xmm5, xmm1
        movdqa xmm1, xmm2
        movdqa  xmm0, xmm2
        ; q0-q1
        psubusb xmm1, xmm3
        ; |q1-q0|
        por xmm1, xmm6
        ; q0-p0
        psubusb xmm0, xmm7
        movdqa  xmm6, xmm7
        movdqa xmm7, xmm3
        ; p0-q0
        psubusb xmm6, xmm2
        movdqa xmm2, xmm4
        ; |p0-q0|
        por xmm0, xmm6
        ; q1-p1
        psubusb xmm7, xmm4
        movdqa  xmm6, [rbx]
        ; p1-q1
        psubusb xmm2, xmm3
        ; flat = max(|q1-q0|,|p1-p0|)
        pmaxub xmm1, xmm5
        ; |p1-q1|
        por xmm7, xmm2
        mov rbx, arg(5)
        ; |p1-p0| & 254
        pand xmm7, [GLOBAL(LC1)]
        movdqa [rsp+96], xmm1
        ; load bilimit1
        movdqa xmm4, [rbx]
        ; |p0-q0|*2
        paddusb xmm0, xmm0
        ;  (|p1-p0| & 254)>>1
        psrlw xmm7, 1
        ; load p2
        movdqa xmm2, [rsp+128]
        ; concatinate bilimit0,bilmit1
        punpcklqdq xmm6, xmm4
        ; |p0-q0|*2 + ((|p1-p0| & 254)>>1)
        paddusb xmm0, xmm7
        mov rbx, arg(4)
        ; load p3
        movdqa xmm4, [rsp+32]
        ; mask = (p0-q0|*2 + ((|p1-p0| & 254)>>1)) - bilimit
        psubusb xmm0, xmm6
        pxor xmm5, xmm5
        ; load thresh0
        movdqa xmm6, [rbx]
        movdqa xmm3, xmm2
        mov rbx, arg(7)
        ; p2-p3
        psubusb xmm3, xmm4
        movdqa xmm7, [rbx]
        ; mask = (mask == 0)
        pcmpeqb xmm0, xmm5
        ; concatinate thresh0,thresh1
        punpcklqdq xmm6, xmm7
        ; p3-p2
        psubusb xmm4, xmm2
        movdqa xmm7, [GLOBAL(LC9)]
        ; hev=flat-thresh
        psubusb xmm1, xmm6
        ; mask = mask ^ 255
        pxor xmm0, xmm7
        ; hev = hev == 0
        pcmpeqb xmm1, xmm5
        ; |p3-p2|
        por xmm4, xmm3
        ; flat
        movdqa xmm5, [rsp+96]
        ; hev = hev ^ 255
        pxor xmm1, xmm7
        ; mask = max(mask, flat)
        pmaxub xmm0, xmm5
        movdqa [rsp+64], xmm1
        ; load p1
        movdqa xmm6, [rsp+16]
        ; load q3
        movdqa xmm1, [rsp+192]
        movdqa xmm3, xmm6
        ; p1-p2
        psubusb xmm3, xmm2
        ; load q2
        movdqa xmm7, [rsp+176]
        ; p2-p1
        psubusb xmm2, xmm6
        movdqa xmm5, xmm7
        mov rbx, arg(3)
        ; |p2-p1|
        por xmm2, xmm3
        ; q2-q3
        psubusb xmm5, xmm1
        ; max (|p2-p1|,|p3-p2|)
        pmaxub xmm2, xmm4
        ; q3-q2
        psubusb xmm1, xmm7
        ; mask = max(max (|p2-p1|,|p3-p2|), mask)
        pmaxub xmm2, xmm0
        ; load q1
        movdqa xmm3, [rsp]
        ; |q3-q2|
        por xmm5, xmm1
        movdqa xmm1, [rbx]
        movdqa xmm6, xmm3
        ; load q0
        movdqa xmm4, [rsp+48]
        mov rbx, arg(6)
        ; mask = max(mask,|q3-q2|)
        pmaxub xmm2, xmm5
        movdqa xmm5, xmm7
        movdqa xmm0, xmm4
        ; q1-q2
        psubusb xmm6, xmm7
        ; q0-q2
        psubusb xmm0, xmm5
        ; concatinate limit0, limit1
        punpcklqdq xmm1, [rbx]
        ; q2-q1
        psubusb xmm7, xmm3
        ; q2-q0
        psubusb xmm5, xmm4
        ; |q2-q1|
        por xmm6, xmm7
        pxor xmm3, xmm3
        ; mask= max(mask,|q2-q1|)
        pmaxub xmm2, xmm6
        ; load p0
        movdqa xmm7, [rsp+80]
        ; mask = mask-limit
        psubusb xmm2, xmm1
        ; |q2-q0|
        por xmm0, xmm5
        movdqa xmm1, xmm7
        ; mask = (mask == 0)
        pcmpeqb xmm2, xmm3
        ; load p2
        movdqa xmm5, [rsp+128]
        ; load q3
        movdqa xmm6, [rsp+192]
        ; p0-p2
        psubusb xmm1, xmm5
        movdqa xmm3, xmm4
        ; p2-p0
        psubusb xmm5, xmm7
        ; q0-q3
        psubusb xmm3, xmm6
        ; |p2-p0|
        por xmm1, xmm5
        ; q3-q0
        psubusb xmm6, xmm4
        ; load flat
        movdqa xmm5, [rsp+96]
        ; work=max(|q2-q0|,|p2-p0|)
        pmaxub xmm0, xmm1
        ; |q3-q0|
        por xmm3, xmm6
        ; flat = max(work, flat)
        pmaxub xmm0, xmm5
        movdqa xmm6, [GLOBAL(LC3)]
        ; load p1
        movdqa xmm5, [rsp+16]
        ; Oq0 = q0 ^ 128
        pxor xmm4, xmm6
        ; Op1 = p1 ^ 128
        pxor xmm5, xmm6
        movdqa [rsp+16], xmm4
        ; load p3
        movdqa xmm1, [rsp+32]
        movdqa [rsp+48], xmm5
        movdqa xmm4, xmm7
        ; p0-p3
        psubusb xmm4, xmm1
        ; load q1
        movdqa xmm5, [rsp]
        ; p3-p0
        psubusb xmm1, xmm7
        ; Oq1 = q1 ^ 128
        pxor xmm5, xmm6
        ; |p3-p0|
        por xmm1, xmm4
        ; Op0 = p0 ^ 128
        pxor xmm7, xmm6
        movdqa [rsp+32], xmm5
        ; work = max(|p3-p0|, |q3-q0|)
        pmaxub xmm1, xmm3
        movdqa [rsp], xmm7
        ; load Op1
        movdqa xmm6, [rsp+48]
        movdqa xmm4, [GLOBAL(LC2)]
        ; flat = max(work, flat)
        pmaxub xmm1, xmm0
        ; Op1 - Oq1
        psubsb xmm6, xmm5
        ; flat = flat - 1
        psubusb xmm1, xmm4
        ; load Oq0
        movdqa xmm3, [rsp+16]
        pxor xmm0, xmm0
        ; flat = (flat == 0)
        pcmpeqb xmm1, xmm0
        ; (Op1 - Oq1)& hev
        pand xmm6, [rsp+64]
        ; Oq0-Op0
        psubsb xmm3, xmm7
        ; flat = flat & mask
        pand xmm1, xmm2
        ; filt = (Oq1 - Op1) + (Oq0-Op0)*3
        paddsb xmm6, xmm3
        paddsb xmm6, xmm3
        paddsb xmm6, xmm3
        pxor xmm7, xmm7
        ; filt = filt & mask
        pand xmm6, xmm2
        movdqa xmm3, [GLOBAL(LC6)]
        movdqa xmm5, xmm6
        ; filter1 = filt + 4
        paddsb xmm6, [GLOBAL(LC4)]
        ; filter2 = filt + 3
        paddsb xmm5, [GLOBAL(LC5)]
        ; filter1 < 0
        pcmpgtb xmm0, xmm6
        ; filter2 < 0
        pcmpgtb xmm7, xmm5
        ; filter1>>=1
        psrlw xmm6, 3
        ; filter2>>=1
        psrlw xmm5, 3
        ; work_a_filter1 = (filter1 < 0) & 224
        pand xmm0, xmm3
        ; filter1 = filter1 & 31
        pand xmm6, [GLOBAL(LC7)]
        ; work_a_filter2 = (filter2 < 0) & 224
        pand xmm3, xmm7
        ; filter1 = filter1 | work_a_filter1
        por xmm6, xmm0
        ; filter2 = filter2 & 31
        pand xmm5, [GLOBAL(LC7)]
        pxor xmm0, xmm0
        ; filt = 1 + fillter1
        paddsb xmm4, xmm6
        movdqa xmm2, [GLOBAL(LC3)]
        ; work_a = (0 < filt)
        pcmpgtb xmm0, xmm4
        ; filter2 = filter2 | work_a_filter2
        por xmm5, xmm3
        ; filt<<=1
        psrlw xmm4, 1
        ; work_a = work_a & 128
        pand xmm0, xmm2
        ; Op0_filter2 = filter2 + Op0
        paddsb xmm5, [rsp]
        ; filt = filt & 127
        pand xmm4, [GLOBAL(LC8)]
        movdqa xmm7, [rsp+64]
        movdqa xmm3, xmm1
        ; filt = filt | work_a
        por xmm4, xmm0
        ; Op0_filter2 = Op0_filter2 ^ 128
        pxor xmm5, xmm2
        ; filt = (~hev) & filt
        pandn xmm7, xmm4
        movdqa xmm0, [rsp+16]
        movdqa xmm4, [rsp+32]
        ; Op0_NotFlat = (~flat) & Op0_filter2
        pandn xmm3, xmm5
        ; Oq0_filter1 = Oq0 - filter1
        psubsb xmm0, xmm6
        ; Oq1_filt = Oq1 - filt
        psubsb xmm4, xmm7
        mov rdi, rdx
        ; Op1_filt = filt + Op1
        paddsb xmm7, [rsp+48]
        ; Oq0_filter1 = Oq0_filter1 ^ 128
        pxor xmm0, xmm2
        ; Op1_filt = Op1_filt ^ 128
        pxor xmm7, xmm2
        movdqa xmm6, xmm1
        sub rdi, rcx
         ; Op1_NotFlat =  (~flat) & Op1_filt
        pandn xmm6, xmm7
        movdqa xmm5, [rsp+304]
        movdqa xmm7, [rsp+352]
        ; Op1_Flat = p1 & flat
        pand xmm5, xmm1
        ; p1 = Op1_Flat | Op1_NotFlat
        por xmm5, xmm6
        ; Oq1_filt = Oq1_filt ^ 128
        pxor xmm4, xmm2
        ; save p1
        movdqu [rdi], xmm5
        ; Op0_Flat = p0 & flat
        pand xmm7, xmm1
        add rdi, rax
        movdqa xmm5, xmm1
        ; p0 = Op0_Flat | Op0_NotFlat
        por xmm7, xmm3
        ; q0_NotFlat = (~flat) & Oq0_filter1
        pandn xmm5, xmm0
        ; save p0
        movdqu [rdi], xmm7
        movdqa xmm3, [rsp+368]
        movdqa xmm6, xmm1
        ; Oqo_Flat = q0 & flat
        pand xmm3, xmm1
        movdqa xmm0, [rsp+384]
        ; q1_NotFlat = (~flat) & Oq1_filt
        pandn xmm6, xmm4
        ; q0 = q0_NotFlat | Oqo_Flat
        por xmm5, xmm3
        ; q1_Flat = q1 & flat
        pand xmm0, xmm1
        ; save q0
        movdqu [rdx],xmm5
        movdqa xmm3, [rsp+176]
        ; q1 = q1_Flat & q1_NotFlat
        por xmm0, xmm6
        sub rdi, rcx
        movdqa xmm4, xmm1
        ; save q1
        movdqu [rdx+rax], xmm0
        ; Oq2_NotFlat = (~flat) & q2
        pandn xmm4, xmm3
        movdqa xmm6, xmm1
        movdqa xmm7, [rsp+400]
        movdqa xmm5, [rsp+128]
        ; Oq2_Flat = Oq2 & flat
        pand xmm7, xmm1
        ; q2 = Oq2_NotFlat | Oq2_Flat
        por xmm4, xmm7
        ; Op2_Flat = flat & Op2
        pand xmm1, [rsp+288]
        ; Op2_NotFlat = (~flat) & p2
        pandn xmm6, xmm5
        ; save q2
        movdqu [rsi], xmm4
        ; p2 = Op2_NotFlat | Op2_Flat
        por xmm6, xmm1
        ; save p2
        movdqu [rdi], xmm6
        add rsp, 432
%endif
        pop rsp
        pop  rbx
        pop rsi
        pop rdi
     RESTORE_XMM
     UNSHADOW_ARGS
     pop rbp
     ret

SECTION_RODATA
align 16
LC0:
    dw  4, 4, 4, 4, 4, 4, 4, 4
align 16
LC1:
    db 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254
align 16
LC2:
    db 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
align 16
LC3:
    db 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
align 16
LC4:
    db  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
align 16
LC5:
    db  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
align 16
LC6:
    db 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224
align 16
LC7:
    db 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31
align 16
LC8:
    db  127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127
align 16
LC9:
    db 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
