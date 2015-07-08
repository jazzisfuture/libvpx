%include "vpx_ports/x86_abi_support.asm"

SECTION_ROTEXT

%define q0                    rbp+0x000
%define p0                    rbp+0x010
%define q1                    rbp+0x020
%define p1                    rbp+0x030
%define q2                    rbp+0x040
%define p2                    rbp+0x050
%define q3                    rbp+0x060
%define p3                    rbp+0x070
%if ABI_IS_32BIT=0
%define q0_lo                 rbp+0x080
%define q2_lo                 rbp+0x090
%define q0_hi                 rbp+0x0A0
%define q2_hi                 rbp+0x0B0
%define flat2_p2              rbp+0x0C0
%define flat2_p1              rbp+0x0D0
%define flat2_p0              rbp+0x0E0
%define flat2_q0              rbp+0x0F0
%define flat2_q1              rbp+0x100
%define flat2_q2              rbp+0x110
%else
%define p3_lo                 rbp+0x080
%define p2_lo                 rbp+0x090
%define p1_lo                 rbp+0x0A0
%define p0_lo                 rbp+0x0B0
%define q0_lo                 rbp+0x0C0
%define q1_lo                 rbp+0x0D0
%define q2_lo                 rbp+0x0E0
%define p3_hi                 rbp+0x0F0
%define p2_hi                 rbp+0x100
%define p1_hi                 rbp+0x110
%define p0_hi                 rbp+0x120
%define q0_hi                 rbp+0x130
%define q1_hi                 rbp+0x140
%define q2_hi                 rbp+0x150
%define flat2_p2              rbp+0x160
%define flat2_p1              rbp+0x170
%define flat2_p0              rbp+0x180
%define flat2_q0              rbp+0x190
%define flat2_q1              rbp+0x1A0
%define flat2_q2              rbp+0x1B0
%endif
%define max_abs_p1p0q1q0      rbp+0x080
%define mask                  rbp+0x090
%define flat                  rbp+0x0A0
%define flat2                 rbp+0x0B0
%define hev                   rbp+0x0C0
%define org_p0                rbp+0x010
%define org_q0                rbp+0x000
%define org_p1                rbp+0x030
%define org_q1                rbp+0x020

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
  GET_GOT     rbx
  push rdi
  push rsi
  push rbx
  ALIGN_STACK 16, rax
%if ABI_IS_32BIT=0
  sub rsp, 0x120
  xchg  rbp, rsp
  mov rdx, arg_rsp(0)
  mov rax, arg_rsp(1)
  mov rsi, rdx
  mov rdi, rdx
  sub rsi, rax
  ; symmetric computation calculating p2,p1,p0,q0,q1,q2 when flat==1
  ; and mask==1
  pxor xmm0, xmm0
  ; load q0, p0
  movdqu xmm3, [rdi]
  movdqu xmm7, [rsi]
  movdqa xmm4, xmm3
  movdqa xmm5, xmm7

  ; extend from 8bit to 16bit
  punpcklbw xmm4, xmm0
  movdqa    xmm6, xmm7
  movdqa    [q0], xmm3
  movdqa    [q0_lo], xmm4
  punpcklbw xmm5, xmm0
  punpckhbw xmm3, xmm0
  lea     rcx,  [rax*2]
  punpckhbw xmm6, xmm0
  movdqa    xmm8, xmm5
  movdqa    xmm15, xmm6
  movdqa    [q0_hi], xmm3

  ; calculate sum = q0+ p0
  paddw xmm5, xmm4
  sub  rsi, rcx
  paddw  xmm6, xmm3
  movdqa [p0], xmm7
  movdqa xmm2, [GLOBAL(four)]
  sub  rdi, rcx

  ; calculate sum+=4
  paddw xmm5, xmm2
  paddw xmm6, xmm2

  ; load p1, p2 and extend
  movdqu    xmm4, [rsi]
  movdqu    xmm7, [rdi]
  movdqa    xmm3, xmm4
  movdqa    xmm2, xmm7
  punpcklbw xmm4, xmm0
  movdqa    [p2], xmm3
  movdqa    xmm10, xmm4
  movdqa    [p1], xmm7
  punpckhbw xmm3, xmm0

  ; calculate p2*2
  psllw     xmm4, 1
  punpcklbw xmm7, xmm0
  movdqa    xmm11, xmm3
  punpckhbw xmm2, xmm0
  sub       rdi, rcx
  psllw     xmm3, 1

  ; caulcate sum+=p2*2
  paddw xmm6, xmm3
  paddw xmm5, xmm4

  ; load p3 and extend
  movdqu xmm3, [rdi]
  ; add sum+=p1
  paddw     xmm5, xmm7
  paddw     xmm6, xmm2
  movdqa    [p3], xmm3
  movdqa    xmm4, xmm3
  punpcklbw xmm3, xmm0
  punpckhbw xmm4, xmm0
  movdqa    xmm13, xmm3
  psllw     xmm3, 1

  ; add sum+=p3
  paddw   xmm6, xmm4
  paddw   xmm5, xmm13
  lea     rsi, [rdx+rax]
  movdqa  xmm14, xmm4
  psllw   xmm4, 1
  ; add sum+=p3*2
  paddw   xmm5, xmm3
  paddw   xmm6, xmm4
  movdqa  xmm1,  xmm5
  movdqa  xmm12, xmm6

  ; sum>>=3
  psrlw xmm1, 3
  ; sum-=p3
  psubw xmm5, xmm13
  psrlw xmm12, 3
  psubw xmm6, xmm14
  ; sum-=p2
  psubw    xmm5, xmm10
  packuswb xmm1, xmm12

  ; load q1 and extend
  movdqu xmm3, [rsi]
  ; add p1
  paddw xmm5, xmm7
  movdqa [q1], xmm3
  ; save p2
  movdqa    [flat2_p2], xmm1
  psubw     xmm6, xmm11
  movdqa    xmm4, xmm3
  punpcklbw xmm4, xmm0
  punpckhbw xmm3, xmm0
  ; sum+=q1
  paddw  xmm5, xmm4
  paddw  xmm6, xmm2
  paddw  xmm6, xmm3
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
  movdqa [flat2_p1], xmm12
  psubw  xmm6, xmm2
  movdqa xmm12, xmm9

  ; sum+=p0
  paddw     xmm5, xmm8
  movdqa    [q2], xmm9
  paddw     xmm6, xmm15
  punpckhbw xmm9, xmm0
  punpcklbw xmm12, xmm0

  ; sum+=q2
  paddw xmm6, xmm9
  movdqa [q2_hi], xmm9
  movdqa [q2_lo], xmm12
  paddw  xmm5, xmm12
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
  psubw    xmm5, xmm8
  psubw    xmm6, xmm15
  movdqa   [q3], xmm9
  packuswb xmm12, xmm1
  movdqa   xmm13, [q0_lo]
  movdqa   xmm14,  xmm9

  ; sum+=q0
  paddw     xmm5, xmm13
  punpcklbw xmm9, xmm0

  ; save p0
  movdqa    [flat2_p0], xmm12
  paddw     xmm6, [q0_hi]
  punpckhbw xmm14, xmm0

  ; sum+=q3
  paddw  xmm5, xmm9
  movdqa xmm12, xmm5
  paddw  xmm6, xmm14
  psrlw  xmm12, 3

  ; sum-=p2
  psubw  xmm5, xmm10
  movdqa xmm1, xmm6

  ; sum-=q0
  psubw xmm5, xmm13
  psubw xmm6, xmm11
  psrlw xmm1, 3
  psubw xmm6, [q0_hi]
  packuswb xmm12, xmm1

  ; sum+=q3
  paddw xmm5, xmm9

  ; sum+=q1
  paddw xmm5, xmm4

  ; save q0
  movdqa [flat2_q0], xmm12
  paddw  xmm6, xmm3
  movdqa xmm11, xmm5
  paddw  xmm6, xmm14
  movdqa xmm1, xmm6
  psrlw  xmm11, 3

  ; sum-=p1
  psubw xmm5, xmm7
  psubw xmm6, xmm2

  ; sum-=q1
  psubw    xmm5, xmm4
  psrlw    xmm1, 3
  packuswb xmm11, xmm1
  psubw    xmm6, xmm3

  ; sum+=q2
  paddw xmm5, [q2_lo]

  ; save q1
  movdqa [flat2_q1], xmm11
  paddw  xmm6, [q2_hi]
  ; sum+=q3
  paddw xmm5, xmm9
  ; load blimit0
  mov   rbx, arg_rsp(2)
  paddw xmm6, xmm14
  ; sum>>=3
  psrlw xmm5, 3
  ; load p1
  movdqa xmm4, [p1]
  ; load p0
  movdqa   xmm7, [p0]
  psrlw    xmm6, 3
  packuswb xmm5, xmm6
  ; load q1
  movdqa   xmm3, [q1]
  ; load q0
  movdqa xmm2, [q0]
  ; save q2
  movdqa [flat2_q2], xmm5
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
  por    xmm5, xmm1
  movdqa xmm1, xmm2
  movdqa xmm0, xmm2
  ; q0-q1
  psubusb xmm1, xmm3
  ; |q1-q0|
  por xmm1, xmm6
  ; q0-p0
  psubusb xmm0, xmm7
  movdqa  xmm6, xmm7
  movdqa  xmm8, xmm3
  ; p0-q0
  psubusb xmm6, xmm2
  movdqa  xmm9, xmm4
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
  mov rbx, arg_rsp(5)
  ; |p1-p0| & 254
  pand   xmm8, [GLOBAL(fe)]
  movdqa xmm12, xmm1
  ; load bilimit1
  movdqa xmm10, [rbx]
  ; |p0-q0|*2
  paddusb xmm0, xmm0
  ; (|p1-p0| & 254)>>1
  psrlw xmm8, 1
  ; load p2
  movdqa xmm11, [p2]
  ; concatinate bilimit0,bilmit1
  punpcklqdq xmm6, xmm10
  ; |p0-q0|*2 + ((|q1-p1| & 254)>>1)
  paddusb xmm0, xmm8
  mov     rbx, arg_rsp(4)
  ; load p3
  movdqa xmm10, [p3]
  ; mask = (p0-q0|*2 + ((|q1-p1| & 254)>>1)) - bilimit
  psubusb xmm0, xmm6
  pxor xmm15, xmm15
  ; load thresh0
  movdqa xmm6, [rbx]
  movdqa xmm9, xmm11
  mov    rbx, arg_rsp(7)
  ; p2-p3
  psubusb xmm9, xmm10
  movdqa  xmm8, [rbx]
  ; mask = (mask == 0)
  pcmpeqb xmm0, xmm15
  ; concatinate thresh0,thresh1
  punpcklqdq xmm6, xmm8
  ; p3-p2
  psubusb xmm10, xmm11
  movdqa  xmm8, [GLOBAL(ff)]
  ; hev=flat-thresh
  psubusb xmm1, xmm6
  ; mask = mask ^ 255
  pxor xmm0, xmm8
  ; hev = hev == 0
  pcmpeqb xmm1, xmm15
  ; |p3-p2|
  por xmm10, xmm9
  ; hev = hev ^ 255
  pxor xmm1, xmm8
  ; mask = max(mask, flat)
  pmaxub xmm0, xmm12
  ; load q3
  movdqa xmm13, [q3]
  movdqa xmm9, xmm4
  ; p1-p2
  psubusb xmm9, xmm11
  ; load q2
  movdqa xmm8, [q2]
  ; p2-p1
  psubusb xmm11, xmm4
  movdqa  xmm5, xmm8
  mov     rbx, arg_rsp(3)
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
  ; |q3-q2|
  por xmm5, xmm13
  movdqa xmm14, [rbx]
  movdqa xmm6, xmm3
  mov    rbx, arg_rsp(6)
  ; mask = max(mask,|q3-q2|)
  pmaxub xmm11, xmm5
  movdqa xmm0, xmm2
  movdqa xmm5, xmm8
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
  ; mask = mask-limit
  psubusb xmm11, xmm14
  ; |q2-q0|
  por xmm0, xmm5
  movdqa xmm13, xmm7
  ; mask = (mask == 0)
  pcmpeqb xmm11, xmm15
  ; load p2
  movdqa xmm5, [p2]
  ; load q3
  movdqa xmm6, [q3]
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
  ; work=max(|q2-q0|,|p2-p0|)
  pmaxub xmm0, xmm13
  ; |q3-q0|
  por xmm9, xmm6
  ; flat = max(work, flat)
  pmaxub xmm0, xmm12
  movdqa xmm8, [GLOBAL(t80)]
  ; Oq0 = q0 ^ 128
  pxor xmm2, xmm8
  ; Op1 = p1 ^ 128
  pxor xmm4, xmm8
  ; load p3
  movdqa xmm10, [p3]
  movdqa xmm12, xmm7
  ; p0-p3
  psubusb xmm12, xmm10
  ; p3-p0
  psubusb xmm10, xmm7
  ; Oq1 = q1 ^ 128
  pxor xmm3, xmm8
  ; |p3-p0|
  por xmm10, xmm12
  ; Op0 = p0 ^ 128
  pxor xmm7, xmm8
  ; work = max(|p3-p0|, |q3-q0|)
  pmaxub xmm10, xmm9
  movdqa xmm13, xmm4
  movdqa xmm6, [GLOBAL(one)]
  ; flat = max(work, flat)
  pmaxub xmm10, xmm0
  ; Op1 - Oq1
  psubsb xmm13, xmm3
  ; flat = flat - 1
  psubusb xmm10, xmm6
  ; load Oq0
  movdqa xmm0, xmm15
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
  pand   xmm13, xmm11
  movdqa xmm12, xmm13
  movdqa xmm14, [GLOBAL(te0)]
  ; filter1 = filt + 4
  paddsb xmm12, [GLOBAL(t4)]
  ; filter2 = filt + 3
  paddsb xmm13, [GLOBAL(t3)]
  ; filter1 < 0
  pcmpgtb xmm15, xmm12
  ; filter2 < 0
  pcmpgtb xmm0, xmm13
  ; filter1>>=1
  psrlw  xmm12, 3
  movdqa xmm5, [GLOBAL(t1f)]
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
  movdqa xmm11, [GLOBAL(t80)]
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
  pand xmm6, [GLOBAL(t7f)]
  movdqa xmm14, xmm10
  ; filt = filt | work_a
  por xmm6, xmm0
  ; Op0_filter2 = Op0_filter2 ^ 128
  pxor xmm13, xmm11
  ; filt = (~hev) & filt
  pandn xmm1, xmm6
  ; Op0_NotFlat = (~flat) & Op0_filter2
  pandn xmm14, xmm13
  ; Oq0_filter1 = Oq0 - filter1
  psubsb xmm2, xmm12
  ; Oq1_filt = Oq1 - filt
  psubsb xmm3, xmm1
  mov    rdi, rdx
  ; Op1_filt = filt + Op1
  paddsb xmm1, xmm4
  ; Oq0_filter1 = Oq0_filter1 ^ 128
  pxor xmm2, xmm11
  ; Op1_filt = Op1_filt ^ 128
  pxor   xmm1, xmm11
  movdqa xmm6, xmm10
  sub    rdi, rcx
  ; Op1_NotFlat =  (~flat) & Op1_filt
  pandn  xmm6, xmm1
  movdqa xmm5, [flat2_p1]
  movdqa xmm7, [flat2_p0]
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
  movdqa xmm12, [flat2_q0]
  movdqa xmm6, xmm10
  ; Oqo_Flat = q0 & flat
  pand xmm12, xmm10
  movdqa xmm0, [flat2_q1]
  ; q1_NotFlat = (~flat) & Oq1_filt
  pandn xmm6, xmm3
  ; q0 = q0_NotFlat | Oqo_Flat
  por xmm13, xmm12
  ; q1_Flat = q1 & flat
  pand xmm0, xmm10
  ; save q0
  movdqu [rdx],xmm13
  movdqa xmm3, [q2]
  ; q1 = q1_Flat & q1_NotFlat
  por    xmm0, xmm6
  sub    rdi, rcx
  movdqa xmm4, xmm10
  ; save q1
  movdqu [rdx+rax], xmm0
  ; Oq2_NotFlat = (~flat) & q2
  pandn  xmm4, xmm3
  movdqa xmm6, xmm10
  movdqa xmm7, [flat2_q2]
  movdqa xmm5, [p2]
  ; Oq2_Flat = Oq2 & flat
  pand xmm7, xmm10
  ; Op2_NotFlat = (~flat) & p2
  pandn xmm6, xmm5
  ; q2 = Oq2_NotFlat | Oq2_Flat
  por xmm4, xmm7
  ; Op2_Flat = flat & Op2
  pand xmm10, [flat2_p2]
  ; save q2
  movdqu [rsi], xmm4
  ; p2 = Op2_NotFlat | Op2_Flat
  por xmm6, xmm10
  ; save p2
  movdqu [rdi], xmm6
  xchg  rbp, rsp
  add   rsp, 0x120
%else
  sub   rsp, 0x1C0
  xchg  rbp, rsp
  mov   rdx, arg_rsp(0)
  mov   rax, arg_rsp(1)
  mov   rsi, rdx
  mov   rdi, rdx
  sub   rsi, rax
  ; symmetric computation calculating p2,p1,p0,q0,q1,q2 when flat==1
  ; and mask==1
  pxor xmm0, xmm0
  ; load q0, p0
  movdqu xmm3, [rdi]
  movdqu xmm7, [rsi]
  movdqa xmm4, xmm3
  movdqa xmm5, xmm7
  ; extend from 8bit to 16bit
  punpcklbw xmm4, xmm0
  movdqa    xmm6, xmm7
  movdqa    [q0], xmm3
  movdqa    [q0_lo], xmm4
  punpcklbw xmm5, xmm0
  punpckhbw xmm3, xmm0
  lea     rcx, [rax*2]
  punpckhbw xmm6, xmm0
  movdqa    [p0_lo], xmm5
  movdqa    [p0_hi], xmm6
  movdqa    [q0_hi], xmm3
  ; calculate sum = q0+ p0
  paddw  xmm5, xmm4
  sub    rsi, rcx
  paddw  xmm6, xmm3
  movdqa [p0], xmm7
  movdqa xmm2, [GLOBAL(four)]
  sub  rdi, rcx
  ; calculate sum+=4
  paddw  xmm5, xmm2
  paddw  xmm6, xmm2
  ; load p1, p2 and extend
  movdqu    xmm4, [rsi]
  movdqu    xmm7, [rdi]
  movdqa    xmm3, xmm4
  movdqa    xmm2, xmm7
  punpcklbw xmm4, xmm0
  movdqa    [p2], xmm3
  movdqa    [p2_lo], xmm4
  movdqa    [p1], xmm7
  punpckhbw xmm3, xmm0
  ; calculate p2*2
  psllw     xmm4, 1
  punpcklbw xmm7, xmm0
  movdqa    [p2_hi], xmm3
  movdqa    [p1_lo], xmm7
  punpckhbw xmm2, xmm0
  sub     rdi, rcx
  psllw     xmm3, 1
  ; caulcate sum+=p2*2
  paddw xmm6, xmm3
  paddw xmm5, xmm4
  ; load p3 and extend
  movdqu xmm3, [rdi]
  ; add sum+=p1
  paddw     xmm5, xmm7
  paddw     xmm6, xmm2
  movdqa    [p3], xmm3
  movdqa    [p1_hi], xmm2
  movdqa    xmm4, xmm3
  punpcklbw xmm3, xmm0
  punpckhbw xmm4, xmm0
  movdqa    xmm7, xmm3
  psllw     xmm3, 1
  ; add sum+=p3
  paddw   xmm6, xmm4
  paddw   xmm5, xmm7
  lea     rsi, [rdx+rax]
  movdqa  [p3_hi], xmm4
  movdqa  [p3_lo], xmm7
  psllw   xmm4, 1
  ; add sum+=p3*2
  paddw   xmm5, xmm3
  paddw   xmm6, xmm4
  movdqa  xmm1,  xmm5
  movdqa  xmm2, xmm6
  ; sum>>=3
  psrlw xmm1, 3
  ; sum-=p3
  psubw xmm5, xmm7
  psrlw xmm2, 3
  psubw xmm6, [p3_hi]
  ; sum-=p2
  psubw xmm5, [p2_lo]
  ; sum-=p2
  packuswb xmm1, xmm2
  ; load q1 and extend
  movdqu xmm3, [rsi]
  ; add p1
  paddw  xmm5, [p1_lo]
  movdqa [q1], xmm3
  ; save p2
  movdqa    [flat2_p2], xmm1
  psubw     xmm6, [p2_hi]
  movdqa    xmm4, xmm3
  punpcklbw xmm4, xmm0
  punpckhbw xmm3, xmm0
  ; sum+=q1
  paddw xmm5, xmm4
  ; sum+=p1
  paddw xmm6, [p1_hi]
  movdqa [q1_lo], xmm4
  movdqa [q1_hi], xmm3
  paddw  xmm6, xmm3
  movdqa xmm2, xmm5
  movdqa xmm1, xmm6
  ; sum-=p3
  psubw  xmm5, xmm7
  ; sum>>=3
  psrlw  xmm2, 3
  lea    rsi,  [rsi+rax]
  psubw  xmm6, [p3_hi]
  psrlw  xmm1, 3
  ; load q2 and extend
  movdqu xmm7, [rsi]
  ; sum-=p1
  psubw  xmm5, [p1_lo]
  movdqa xmm4, xmm7
  psubw  xmm6, [p1_hi]
  ; sum+=p0
  paddw     xmm5, [p0_lo]
  movdqa    [q2], xmm7
  paddw     xmm6, [p0_hi]
  punpckhbw xmm4, xmm0
  punpcklbw xmm7, xmm0
  packuswb  xmm2, xmm1
  ; sum+=q2
  paddw  xmm6, xmm4
  movdqa [q2_hi], xmm4
  movdqa [q2_lo], xmm7
  ; save p1
  movdqa [flat2_p1], xmm2
  paddw  xmm5, xmm7
  movdqa xmm1, xmm6
  movdqa xmm2, xmm5
  ; load q3 and extend
  movdqu xmm4, [rsi+rax]
  ; sum-=p3
  psubw xmm6, [p3_hi]
  psubw xmm5, [p3_lo]
  ; sum>>=3
  psrlw xmm2, 3
  psrlw xmm1, 3
  ; sum-=p0
  psubw    xmm5, [p0_lo]
  psubw    xmm6, [p0_hi]
  movdqa   [q3], xmm4
  packuswb xmm2, xmm1
  movdqa   xmm3,  xmm4
  ; sum+=q0
  paddw     xmm5, [q0_lo]
  punpcklbw xmm4, xmm0
  ; save p0
  movdqa [flat2_p0], xmm2
  paddw  xmm6, [q0_hi]

  punpckhbw xmm3, xmm0
  ; sum+=q3
  paddw  xmm5, xmm4
  movdqa xmm2, xmm5
  paddw  xmm6, xmm3
  psrlw  xmm2, 3
  ; sum-=p2
  psubw  xmm5, [p2_lo]
  movdqa xmm1, xmm6
  ; sum-=q0
  psubw xmm5, [q0_lo]
  psubw xmm6, [p2_hi]
  psrlw xmm1, 3
  psubw xmm6, [q0_hi]

  packuswb xmm2, xmm1
  ; sum+=q3
  paddw xmm5, xmm4
  ; sum+=q1
  paddw xmm5, [q1_lo]
  ; save q0
  movdqa [flat2_q0], xmm2
  paddw xmm6, [q1_hi]

  movdqa xmm2, xmm5
  paddw  xmm6, xmm3
  movdqa xmm1, xmm6
  psrlw  xmm2, 3
  ; sum-=p1
  psubw  xmm5, [p1_lo]
  psubw  xmm6, [p1_hi]
  ; sum-=q1
  psubw    xmm5, [q1_lo]
  psrlw    xmm1, 3
  packuswb xmm2, xmm1
  psubw    xmm6, [q1_hi]
  ; sum+=q2
  paddw xmm5, [q2_lo]
  ; save q1
  movdqa [flat2_q1], xmm2
  paddw  xmm6, [q2_hi]
  ; sum+=q3
  paddw xmm5, xmm4
  ; load blimit0
  mov   rbx, arg_rsp(2)
  paddw xmm6, xmm3
  ; sum>>=3
  psrlw xmm5, 3
  ; load p1
  movdqa xmm4, [p1]
  ; load p0
  movdqa   xmm7, [p0]
  psrlw    xmm6, 3
  packuswb xmm5, xmm6
  ; load q1
  movdqa xmm3, [q1]
  ; load q0
  movdqa xmm2, [q0]
  ; save q2
  movdqa [flat2_q2], xmm5
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
  movdqa xmm0, xmm2
  ; q0-q1
  psubusb xmm1, xmm3
  ; |q1-q0|
  por xmm1, xmm6
  ; q0-p0
  psubusb xmm0, xmm7
  movdqa  xmm6, xmm7
  movdqa  xmm7, xmm3
  ; p0-q0
  psubusb xmm6, xmm2
  movdqa  xmm2, xmm4
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
  mov rbx, arg_rsp(5)
  ; |p1-p0| & 254
  pand   xmm7, [GLOBAL(fe)]
  movdqa [max_abs_p1p0q1q0], xmm1
  ; load bilimit1
  movdqa xmm4, [rbx]
  ; |p0-q0|*2
  paddusb xmm0, xmm0
  ;  (|p1-p0| & 254)>>1
  psrlw xmm7, 1
  ; load p2
  movdqa xmm2, [p2]
  ; concatinate bilimit0,bilmit1
  punpcklqdq xmm6, xmm4
  ; |p0-q0|*2 + ((|p1-p0| & 254)>>1)
  paddusb xmm0, xmm7
  mov     rbx, arg_rsp(4)
  ; load p3
  movdqa xmm4, [p3]
  ; mask = (p0-q0|*2 + ((|p1-p0| & 254)>>1)) - bilimit
  psubusb xmm0, xmm6
  pxor    xmm5, xmm5
  ; load thresh0
  movdqa xmm6, [rbx]
  movdqa xmm3, xmm2
  mov    rbx, arg_rsp(7)
  ; p2-p3
  psubusb xmm3, xmm4
  movdqa  xmm7, [rbx]
  ; mask = (mask == 0)
  pcmpeqb xmm0, xmm5
  ; concatinate thresh0,thresh1
  punpcklqdq xmm6, xmm7
  ; p3-p2
  psubusb xmm4, xmm2
  movdqa  xmm7, [GLOBAL(ff)]
  ; hev=flat-thresh
  psubusb xmm1, xmm6
  ; mask = mask ^ 255
  pxor xmm0, xmm7
  ; hev = hev == 0
  pcmpeqb xmm1, xmm5
  ; |p3-p2|
  por xmm4, xmm3
  ; flat
  movdqa xmm5, [max_abs_p1p0q1q0]
  ; hev = hev ^ 255
  pxor xmm1, xmm7
  ; mask = max(mask, flat)
  pmaxub xmm0, xmm5
  movdqa [hev], xmm1
  ; load p1
  movdqa xmm6, [p1]
  ; load q3
  movdqa xmm1, [q3]
  movdqa xmm3, xmm6
  ; p1-p2
  psubusb xmm3, xmm2
  ; load q2
  movdqa xmm7, [q2]
  ; p2-p1
  psubusb xmm2, xmm6
  movdqa  xmm5, xmm7
  mov     rbx, arg_rsp(3)
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
  movdqa xmm3, [q1]
  ; |q3-q2|
  por    xmm5, xmm1
  movdqa xmm1, [rbx]
  movdqa xmm6, xmm3
  ; load q0
  movdqa xmm4, [q0]
  mov    rbx, arg_rsp(6)
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
  por  xmm6, xmm7
  pxor xmm3, xmm3
  ; mask= max(mask,|q2-q1|)
  pmaxub xmm2, xmm6
  ; load p0
  movdqa xmm7, [p0]
  ; mask = mask-limit
  psubusb xmm2, xmm1
  ; |q2-q0|
  por    xmm0, xmm5
  movdqa xmm1, xmm7
  ; mask = (mask == 0)
  pcmpeqb xmm2, xmm3
  ; load p2
  movdqa xmm5, [p2]
  ; load q3
  movdqa xmm6, [q3]
  ; p0-p2
  psubusb xmm1, xmm5
  movdqa  xmm3, xmm4
  ; p2-p0
  psubusb xmm5, xmm7
  ; q0-q3
  psubusb xmm3, xmm6
  ; |p2-p0|
  por xmm1, xmm5
  ; q3-q0
  psubusb xmm6, xmm4
  ; load flat
  movdqa xmm5, [max_abs_p1p0q1q0]
  ; work=max(|q2-q0|,|p2-p0|)
  pmaxub xmm0, xmm1
  ; |q3-q0|
  por xmm3, xmm6
  ; flat = max(work, flat)
  pmaxub xmm0, xmm5
  movdqa xmm6, [GLOBAL(t80)]
  ; load p1
  movdqa xmm5, [p1]
  ; Oq0 = q0 ^ 128
  pxor xmm4, xmm6
  ; Op1 = p1 ^ 128
  pxor xmm5, xmm6
  movdqa [org_q0], xmm4
  ; load p3
  movdqa xmm1, [p3]
  movdqa [org_p1], xmm5
  movdqa xmm4, xmm7
  ; p0-p3
  psubusb xmm4, xmm1
  ; load q1
  movdqa xmm5, [q1]
  ; p3-p0
  psubusb xmm1, xmm7
  ; Oq1 = q1 ^ 128
  pxor xmm5, xmm6
  ; |p3-p0|
  por xmm1, xmm4
  ; Op0 = p0 ^ 128
  pxor   xmm7, xmm6
  movdqa [org_q1], xmm5
  ; work = max(|p3-p0|, |q3-q0|)
  pmaxub xmm1, xmm3
  movdqa [org_p0], xmm7
  ; load Op1
  movdqa xmm6, [org_p1]
  movdqa xmm4, [GLOBAL(one)]
  ; flat = max(work, flat)
  pmaxub xmm1, xmm0
  ; Op1 - Oq1
  psubsb xmm6, xmm5
  ; flat = flat - 1
  psubusb xmm1, xmm4
  ; load Oq0
  movdqa xmm3, [org_q0]
  pxor   xmm0, xmm0
  ; flat = (flat == 0)
  pcmpeqb xmm1, xmm0
  ; (Op1 - Oq1)& hev
  pand xmm6, [hev]
  ; Oq0-Op0
  psubsb xmm3, xmm7
  ; flat = flat & mask
  pand xmm1, xmm2
  ; filt = (Oq1 - Op1) + (Oq0-Op0)*3
  paddsb xmm6, xmm3
  paddsb xmm6, xmm3
  paddsb xmm6, xmm3
  pxor   xmm7, xmm7
  ; filt = filt & mask
  pand   xmm6, xmm2
  movdqa xmm3, [GLOBAL(te0)]
  movdqa xmm5, xmm6
  ; filter1 = filt + 4
  paddsb xmm6, [GLOBAL(t4)]
  ; filter2 = filt + 3
  paddsb xmm5, [GLOBAL(t3)]
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
  pand xmm6, [GLOBAL(t1f)]
  ; work_a_filter2 = (filter2 < 0) & 224
  pand xmm3, xmm7
  ; filter1 = filter1 | work_a_filter1
  por xmm6, xmm0
  ; filter2 = filter2 & 31
  pand xmm5, [GLOBAL(t1f)]
  pxor xmm0, xmm0
  ; filt = 1 + fillter1
  paddsb xmm4, xmm6
  movdqa xmm2, [GLOBAL(t80)]
  ; work_a = (0 < filt)
  pcmpgtb xmm0, xmm4
  ; filter2 = filter2 | work_a_filter2
  por xmm5, xmm3
  ; filt<<=1
  psrlw xmm4, 1
  ; work_a = work_a & 128
  pand xmm0, xmm2
  ; Op0_filter2 = filter2 + Op0
  paddsb xmm5, [org_p0]
  ; filt = filt & 127
  pand   xmm4, [GLOBAL(t7f)]
  movdqa xmm7, [hev]
  movdqa xmm3, xmm1
  ; filt = filt | work_a
  por xmm4, xmm0
  ; Op0_filter2 = Op0_filter2 ^ 128
  pxor xmm5, xmm2
  ; filt = (~hev) & filt
  pandn  xmm7, xmm4
  movdqa xmm0, [org_q0]
  movdqa xmm4, [org_q1]
  ; Op0_NotFlat = (~flat) & Op0_filter2
  pandn  xmm3, xmm5
  ; Oq0_filter1 = Oq0 - filter1
  psubsb xmm0, xmm6
  ; Oq1_filt = Oq1 - filt
  psubsb xmm4, xmm7
  mov rdi, rdx
  ; Op1_filt = filt + Op1
  paddsb xmm7, [org_p1]
  ; Oq0_filter1 = Oq0_filter1 ^ 128
  pxor xmm0, xmm2
  ; Op1_filt = Op1_filt ^ 128
  pxor xmm7, xmm2
  movdqa xmm6, xmm1
  sub rdi, rcx
  ; Op1_NotFlat =  (~flat) & Op1_filt
  pandn  xmm6, xmm7
  movdqa xmm5, [flat2_p1]
  movdqa xmm7, [flat2_p0]
  ; Op1_Flat = p1 & flat
  pand xmm5, xmm1
  ; p1 = Op1_Flat | Op1_NotFlat
  por xmm5, xmm6
  ; Oq1_filt = Oq1_filt ^ 128
  pxor xmm4, xmm2
  ; save p1
  movdqu [rdi], xmm5
  ; Op0_Flat = p0 & flat
  pand   xmm7, xmm1
  add    rdi, rax
  movdqa xmm5, xmm1
  ; p0 = Op0_Flat | Op0_NotFlat
  por xmm7, xmm3
  ; q0_NotFlat = (~flat) & Oq0_filter1
  pandn xmm5, xmm0
  ; save p0
  movdqu [rdi], xmm7
  movdqa xmm3, [flat2_q0]
  movdqa xmm6, xmm1
  ; Oqo_Flat = q0 & flat
  pand   xmm3, xmm1
  movdqa xmm0, [flat2_q1]
  ; q1_NotFlat = (~flat) & Oq1_filt
  pandn xmm6, xmm4
  ; q0 = q0_NotFlat | Oqo_Flat
  por xmm5, xmm3
  ; q1_Flat = q1 & flat
  pand xmm0, xmm1
  ; save q0
  movdqu [rdx],xmm5
  movdqa xmm3, [q2]
  ; q1 = q1_Flat & q1_NotFlat
  por xmm0, xmm6
  sub rdi, rcx
  movdqa xmm4, xmm1
  ; save q1
  movdqu [rdx+rax], xmm0
  ; Oq2_NotFlat = (~flat) & q2
  pandn  xmm4, xmm3
  movdqa xmm6, xmm1
  movdqa xmm7, [flat2_q2]
  movdqa xmm5, [p2]
  ; Oq2_Flat = Oq2 & flat
  pand xmm7, xmm1
  ; q2 = Oq2_NotFlat | Oq2_Flat
  por xmm4, xmm7
  ; Op2_Flat = flat & Op2
  pand xmm1, [flat2_p2]
  ; Op2_NotFlat = (~flat) & p2
  pandn xmm6, xmm5
  ; save q2
  movdqu [rsi], xmm4
  ; p2 = Op2_NotFlat | Op2_Flat
  por xmm6, xmm1
  ; save p2
  movdqu [rdi], xmm6
  xchg rbp, rsp
  add  rsp, 0x1C0
%endif
  pop rsp
  pop rbx
  pop rsi
  pop rdi
  RESTORE_GOT
  RESTORE_XMM
  UNSHADOW_ARGS
  pop rbp
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;mb_lpf_horizontal_edge_w_sse2_16;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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
%undef max_abs_p1p0q1q0
%undef mask
%undef flat
%undef flat2
%undef hev
%undef org_p0
%undef org_q0
%undef org_p1
%undef org_q1


%define q0                    rbp+0x000
%define p0                    rbp+0x010
%define q1                    rbp+0x020
%define p1                    rbp+0x030
%define q2                    rbp+0x040
%define p2                    rbp+0x050
%define q3                    rbp+0x060
%define p3                    rbp+0x070
%define q4                    rbp+0x080
%define p4                    rbp+0x090
%define q5                    rbp+0x0A0
%define p5                    rbp+0x0B0
%define q6                    rbp+0x0C0
%define p6                    rbp+0x0D0
%define q7                    rbp+0x0E0
%define p7                    rbp+0x0F0
%define max_abs_p1p0q1q0      rbp+0x100
%define mask                  rbp+0x110
%define flat                  rbp+0x120
%define flat2                 rbp+0x100
%define hev                   rbp+0x130
%define org_p0                rbp+0x140
%define org_q0                rbp+0x150
%define org_p1                rbp+0x160
%define org_q1                rbp+0x110

%define p6_lo                 rbp+0x130
%define p5_lo                 rbp+0x170
%define p4_lo                 rbp+0x180
%define p3_lo                 rbp+0x190
%define p2_lo                 rbp+0x1A0
%define p1_lo                 rbp+0x1B0
%define p0_lo                 rbp+0x1C0
%define q0_lo                 rbp+0x1D0
%define q1_lo                 rbp+0x1E0
%define q2_lo                 rbp+0x1F0
%define q3_lo                 rbp+0x200
%define q4_lo                 rbp+0x210
%define q5_lo                 rbp+0x220
%define q6_lo                 rbp+0x230
%define p6_hi                 rbp+0x240
%define p5_hi                 rbp+0x250
%define p4_hi                 rbp+0x260
%define p3_hi                 rbp+0x270
%define p2_hi                 rbp+0x280
%define p1_hi                 rbp+0x290
%define p0_hi                 rbp+0x2A0
%define q0_hi                 rbp+0x2B0
%define q1_hi                 rbp+0x2C0
%define q2_hi                 rbp+0x2D0
%define q3_hi                 rbp+0x2E0
%define q4_hi                 rbp+0x2F0
%define q5_hi                 rbp+0x300
%define q6_hi                 rbp+0x310
%define flat2_lo              rbp+0x320
%define flat2_hi              rbp+0x330

%define flat_p2               rbp+0x340
%define flat_p1               rbp+0x350
%define flat_p0               rbp+0x360
%define flat_q0               rbp+0x370
%define flat_q1               rbp+0x380
%define flat_q2               rbp+0x390

%define flat2_p6              rbp+0x3A0
%define flat2_p5              rbp+0x320
%define flat2_p4              rbp+0x330
%define flat2_p3              rbp+0x3B0
%define flat2_p2              rbp+0x3C0
%define flat2_p1              rbp+0x3D0
%define flat2_p0              rbp+0x3E0
%define flat2_q0              rbp+0x130
%define flat2_q1              rbp+0x240
%define flat2_q2              rbp+0x170
%define flat2_q3              rbp+0x250
%define flat2_q4              rbp+0x180
%define flat2_q5              rbp+0x260
%define flat2_q6              rbp+0x190

; void mb_lpf_horizontal_edge_w_sse2_16
; (
;   unsigned char *s
;   int p
;   const unsigned char *_blimit
;   const unsigned char *_limit
;   const unsigned char *_thresh
;   int count)
global sym(mb_lpf_horizontal_edge_w_sse2_16) PRIVATE
sym(mb_lpf_horizontal_edge_w_sse2_16):
  push rbp
  mov  rbp, rsp
  SHADOW_ARGS_TO_STACK 5
  SAVE_XMM 7
  GET_GOT     rbx
  push rbx
%if ABI_IS_32BIT
  push rdi
  push rsi
%endif
  ; end prolog
  ALIGN_STACK 16, rax
  sub  rsp, 0x3F0
  xchg rbp, rsp
  mov  rdx, arg_rsp(0)
  mov  rax, arg_rsp(1)
  ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ; mask
  mov rsi, rdx
  mov rdi, rdx
  sub rsi, rax

  movdqu  xmm3, [rdi]         ; load q0
  movdqu  xmm7, [rsi]         ; load p0
  lea     rdi,  [rdi + rax]
  sub     rsi, rax
  movdqu  xmm2, [rdi]       ; load q1
  movdqu  xmm5, [rsi]       ; load p1
  movdqa  xmm4, xmm7
  movdqa  xmm1, xmm2
  movdqa  xmm0, xmm5
  movdqa  [q0], xmm3
  psubusb xmm1, xmm3
  psubusb xmm4, xmm5
  movdqa  [p0], xmm7
  psubusb xmm3, xmm2
  psubusb xmm0, xmm7
  movdqa  [p1], xmm5
  por     xmm1, xmm3
  por     xmm4, xmm0
  movdqa  [q1], xmm2
  pmaxub  xmm4, xmm1         ; max(abs_diff(p1,p0),abs_diff(q1,q0))
  movdqa  xmm6, xmm2
  movdqa  xmm1, xmm7
  movdqa  xmm0, xmm5
  movdqa  xmm3, [q0]
  movdqa  [max_abs_p1p0q1q0], xmm4
  psubusb xmm6, xmm5
  psubusb xmm1, xmm3
  psubusb xmm0, xmm2
  psubusb xmm3, xmm7
  por     xmm0, xmm6          ; abs_diff(p1,q1)
  por     xmm1, xmm3          ; abs_diff(p0,q0)
  pand    xmm0, [GLOBAL(fe)]
  paddusb xmm1, xmm1
  mov     rbx, arg_rsp(2)
  lea     rdi,  [rdi + rax]
  sub     rsi,  rax
  psrlw   xmm0, 1
  pxor    xmm6, xmm6
  paddusb xmm0, xmm1
  movdqu  xmm3, [rsi]          ; load p2
  movdqu  xmm7, [rdi]          ; load q2
  psubusb xmm0, [rbx]          ; mask = _mm_subs_epu8(_mm_adds_epu8(abs_diff(p0,q0),abs_diff(p1,q1)), blimit)
  movdqa  xmm1, xmm7
  pcmpeqb xmm0, xmm6
  movdqa  [p2], xmm3
  pxor    xmm0, [GLOBAL(ff)]             ; mask = _mm_xor_si128(_mm_cmpeq_epi8(mask, zero), ff)
  movdqa  xmm6, xmm3
  pmaxub  xmm0, xmm4
  lea     rdi,  [rdi + rax]
  sub     rsi,  rax
  psubusb xmm6, xmm5
  psubusb xmm1, xmm2
  movdqa  [q2], xmm7
  psubusb xmm5, xmm3
  psubusb xmm2, xmm7
  movdqu  xmm4, [rsi]            ; load p3
  por     xmm6, xmm5             ; abs_diff(p2,p1)
  por     xmm1, xmm2             ; abs_diff(q2,q1)
  pmaxub  xmm0, xmm6             ; max(mask,abs_diff(p2,p1))
  movdqu  xmm5, [rdi]            ; load q3
  pmaxub  xmm0, xmm1             ; max(mask,abs_diff(q2,q1))
  movdqa  xmm6, xmm4
  movdqa  xmm2, xmm5
  psubusb xmm6, xmm3
  psubusb xmm2, xmm7
  movdqa  [p3], xmm4
  mov     rbx,  arg_rsp(3)        ; load limit
  psubusb xmm7, xmm5
  psubusb xmm3, xmm4
  movdqa  [q3], xmm5
  movdqa  xmm1, [p0]
  por     xmm3, xmm6              ; abs_diff(p3,p2)
  por     xmm7, xmm2              ; abs_diff(q3,q2)
  pmaxub  xmm0, xmm3              ; mask = _mm_max_epu8(mask, abs_diff(p3, p2))
  movdqa  xmm2, xmm1
  pmaxub  xmm0, xmm7              ; mask = _mm_max_epu8(mask, abs_diff(q3, q2))
  movdqa  xmm6, [q0]
  psubusb xmm0, [rbx]             ; mask = _mm_subs_epu8(mask, limit)
  pxor    xmm7, xmm7
  movdqa  xmm3, xmm6
  pcmpeqb xmm0, xmm7              ; mask = _mm_cmpeq_epi8(mask, zero)
  psubusb xmm2, xmm4
  psubusb xmm3, xmm5
  movdqa  xmm7, [p2]
  psubusb xmm4, xmm1
  psubusb xmm5, xmm6
  por     xmm2, xmm4              ; abs_diff(p3, p0)
  por     xmm3, xmm5              ; abs_diff(q3, q0)
  lea     rdi, [rdi+ rax]
  sub     rsi, rax
  movdqa  xmm4, [q2]
  pmaxub  xmm2, xmm3              ; flat = _mm_max_epu8(abs_diff(p3, p0), abs_diff(q3, q0))
  movdqa  xmm5, xmm1
  movdqa  xmm3, xmm6
  psubusb xmm5, xmm7
  psubusb xmm3, xmm4
  psubusb xmm7, xmm1
  psubusb xmm4, xmm6
  pmaxub  xmm2, [max_abs_p1p0q1q0]
  movdqa  [mask], xmm0
  por     xmm5, xmm7              ; abs_diff(p2, p0)
  por     xmm3, xmm4              ; abs_diff(q2, p0)
  movdqu  xmm0, [rsi]             ; load p4
  movdqu  xmm7, [rdi]             ; load q4
  pmaxub  xmm5, xmm3              ; work =  _mm_max_epu8(abs_diff(p2, p0), abs_diff(q2, q0))
  movdqa  xmm4, xmm1
  movdqa  xmm3, xmm6
  pmaxub  xmm2, xmm5              ; flat =  _mm_max_epu8(flat, work)
  lea     rdi,  [rdi + rax]
  sub     rsi,  rax
  movdqa  [p4], xmm0
  psubusb xmm3, xmm7
  psubusb xmm4, xmm0
  movdqa  [q4], xmm7
  psubusb xmm2, [GLOBAL(one)]      ; flat = _mm_subs_epu8(flat, one);
  psubusb xmm0, xmm1
  pcmpeqb xmm2, [GLOBAL(zero)]     ; flat = _mm_subs_epu8(flat, one);
  psubusb xmm7, xmm6
  pand    xmm2, [mask]
  por     xmm4, xmm0               ; abs_diff(p4, p0)
  por     xmm3, xmm7               ; abs_diff(q4, p0)
  movdqu  xmm5, [rsi]              ; load p5
  movdqu  xmm7, [rdi]              ; load q5
  pmaxub  xmm3, xmm4               ; flat2 =  _mm_max_epu8(abs_diff(q4, p0), abs_diff(p4, q0))
  movdqa  [flat], xmm2
  lea     rdi,  [rdi + rax]
  sub     rsi,  rax
  movdqa  [p5], xmm5
  movdqa  xmm4, xmm1
  movdqa  xmm2, xmm6
  movdqa  [q5], xmm7
  psubusb xmm4, xmm5
  psubusb xmm2, xmm7
  psubusb xmm5, xmm1
  psubusb xmm7, xmm6
  por     xmm4, xmm5                ; abs_diff(p5, p0)
  por     xmm2, xmm7                ; abs_diff(q5, p0)
  movdqu  xmm0, [rsi]               ; load p6
  movdqu  xmm5, [rdi]               ; load q6
  pmaxub  xmm2, xmm4                ; flat2 = _mm_max_epu8(flat2, abs_diff(p5, q0))
  lea     rdi,  [rdi + rax]
  sub     rsi,  rax
  pmaxub  xmm3, xmm2                ; flat2 = _mm_max_epu8(flat2,abs_diff(q5, q0))
  movdqa  [p6], xmm0
  movdqa  xmm7, xmm1
  movdqa  xmm4, xmm6
  movdqa  [q6], xmm5
  psubusb xmm7, xmm0
  psubusb xmm4, xmm5
  psubusb xmm0, xmm1
  psubusb xmm5, xmm6
  por     xmm7, xmm0                 ; abs_diff(p6, p0)
  por     xmm4, xmm5                 ; abs_diff(q6, p0)
  movdqu  xmm2, [rsi]                ; load p7
  movdqu  xmm0, [rdi]                ; load q7
  pmaxub  xmm4, xmm7                 ; flat2 = _mm_max_epu8(flat2, abs_diff(p6, q0))
  movdqa  [p7], xmm2
  pmaxub  xmm3, xmm4                 ; flat2 = _mm_max_epu8(flat2,abs_diff(q6, q0))
  movdqa  xmm5, xmm1
  movdqa  xmm7, xmm6
  movdqa  [q7], xmm0
  psubusb xmm5, xmm2
  psubusb xmm7, xmm0
  psubusb xmm2, xmm1
  psubusb xmm0, xmm6
  por     xmm5, xmm2                  ; abs_diff(p7, p0)
  por     xmm7, xmm0                  ; abs_diff(q7, p0)
  pxor    xmm6, xmm6
  pmaxub  xmm7, xmm5                  ; flat2 = _mm_max_epu8(flat2, abs_diff(p7, q0))
  movdqa  xmm4, [max_abs_p1p0q1q0]
  mov     rbx,  arg_rsp(4)
  pmaxub  xmm3, xmm7                  ; flat2 = _mm_max_epu8(flat2,abs_diff(q7, q0))
  movdqa  xmm0, [rbx]
  psubusb xmm3, [GLOBAL(one)]         ; flat2 = _mm_subs_epu8(flat2, one);
  psubusb xmm4, xmm0
  pcmpeqb xmm3, xmm6                  ; flat = _mm_subs_epu8(flat, one);
  movdqa  xmm1, [p1]
  movdqa  xmm2, [q1]
  pand    xmm3, [flat]
  movdqa  xmm6, [GLOBAL(t80)]
  pxor    xmm7, xmm7
  movdqa  [flat2], xmm3                ; save flat2
  pxor    xmm1, xmm6                   ; op1 = _mm_xor_si128(p1, t80)
  pcmpeqb xmm4, xmm7                   ; hev = _mm_cmpeq_epi8(hev, zero)
  movdqa  [org_p1], xmm1
  pxor    xmm2, xmm6                   ; oq1 = _mm_xor_si128(q1, t80)
  pxor    xmm4, [GLOBAL(ff)]           ; hev = _mm_xor_si128(_mm_cmpeq_epi8(hev, zero), ff)
  psubsb xmm1, xmm2                    ; _mm_subs_epi8(op1, oq1)
  movdqa  xmm0, [p0]
  movdqa  xmm5, [q0]
  pand    xmm1, xmm4                   ; filt = _mm_and_si128(_mm_subs_epi8(op1, oq1), hev)
  pxor    xmm0, xmm6                   ; op0 = _mm_xor_si128(p0, t80)
  pxor    xmm5, xmm6                   ; oq0 = _mm_xor_si128(q0, t80)
  movdqa  [org_q0], xmm5
  movdqa  [hev], xmm4                  ; save hev
  psubsb xmm5, xmm0                    ; work_a = _mm_subs_epi8(oq0, op0)
  movdqa  xmm3, xmm7
  paddsb  xmm1, xmm5                   ; filt = _mm_adds_epi8(filt, work_a)
  paddsb  xmm1, xmm5                   ; filt = _mm_adds_epi8(filt, work_a)
  paddsb  xmm1, xmm5                   ; filt = _mm_adds_epi8(filt, work_a)
  pand    xmm1, [mask]
  movdqa  xmm4, xmm1
  paddsb  xmm1, [GLOBAL(t4)]           ; filter1 = _mm_adds_epi8(filt, t4);
  paddsb  xmm4, [GLOBAL(t3)]           ; filter2 = _mm_adds_epi8(filt, t3);
  pcmpgtb xmm7, xmm1                   ; work_a = _mm_cmpgt_epi8(zero, filter1)
  pcmpgtb xmm3, xmm4                   ; work_a = _mm_cmpgt_epi8(zero, filter2)
  psrlw   xmm1, 0x3                    ; filter1 = _mm_srli_epi16(filter1, 3)
  psrlw   xmm4, 0x3                    ; filter2 = _mm_srli_epi16(filter2, 3)
  pand    xmm7, [GLOBAL(te0)]          ; work_a = _mm_and_si128(work_a, te0)
  pand    xmm3, [GLOBAL(te0)]          ; work_a = _mm_and_si128(work_a, te0)
  pand    xmm1, [GLOBAL(t1f)]          ; filter1 = _mm_and_si128(filter1, t1f)
  pand    xmm4, [GLOBAL(t1f)]          ; filter2 = _mm_and_si128(filter2, t1f)
  por     xmm1, xmm7                   ; filter1 = _mm_or_si128(filter1, work_a)
  movdqa  xmm5, [org_q0]
  por     xmm4, xmm3                   ; filter2 = _mm_or_si128(filter2, work_a)
  psubsb xmm5, xmm1                    ; _mm_subs_epi8(oq0, filter1)
  paddsb xmm0, xmm4                    ; _mm_adds_epi8(op0, filter2)
  pxor    xmm7, xmm7
  pxor    xmm5, xmm6                   ; _mm_xor_si128(_mm_subs_epi8(oq0, filter1), t80)
  pxor    xmm0, xmm6                   ; _mm_xor_si128(_mm_adds_epi8(op0, filter2), t80)
  movdqa  xmm3, [hev]
  movdqa  [org_q0], xmm5
  movdqa  [org_p0], xmm0
  paddsb  xmm1, [GLOBAL(one)]          ; filt = _mm_adds_epi8(filter1, t1)
  pcmpgtb xmm7, xmm1                   ; work_a = _mm_cmpgt_epi8(zero, filter1)
  psrlw   xmm1, 0x1                    ; filter1 = _mm_srli_epi16(filter1, 3)
  pand    xmm7, xmm6                   ; work_a = _mm_and_si128(work_a, te0)
  pand    xmm1, [GLOBAL(t7f)]          ; filter1 = _mm_and_si128(filter1, t1f)
  movdqa  xmm4, [org_p1]
  por     xmm1, xmm7                   ; filt = _mm_or_si128(filt, work_a)
  pandn   xmm3, xmm1                   ; filt = _mm_andnot_si128(hev, filt);
  paddsb  xmm4, xmm3                   ; _mm_adds_epi8(op1, filt)
  psubsb xmm2, xmm3                    ; _mm_subs_epi8(oq1, filt)
  pxor    xmm4, xmm6                   ; _mm_xor_si128(_mm_adds_epi8(op1, filt), t80)
  pxor    xmm2, xmm6                   ; _mm_xor_si128(_mm_subs_epi8(oq1, filt), t80)
  movdqa  [org_p1], xmm4
  movdqa  [org_q1], xmm2

  ; calculating filter4 and filter8
  movdqa     xmm0, [p0]
  movdqa     xmm1, [q0]
  pxor       xmm7, xmm7
  movdqa     xmm2, xmm0
  movdqa     xmm3, xmm1
  punpcklbw  xmm0, xmm7
  punpcklbw  xmm1, xmm7
  punpckhbw  xmm2, xmm7
  movdqa     [p0_lo], xmm0
  punpckhbw  xmm3, xmm7
  movdqa     [p0_hi], xmm2
  paddw      xmm0, xmm1                    ; p0+q0(low)
  movdqa     [q0_lo], xmm1
  paddw      xmm2, xmm3                    ; p0+q0(high)
  movdqa     [q0_hi], xmm3
  paddw      xmm0, [GLOBAL(eight)]         ; sum_lo = p0+q0+8(low)
  paddw      xmm2, [GLOBAL(eight)]         ; sum_hi = p0+q0+8(high)
  movdqa     xmm4, [p1]
  movdqa     xmm5, [p2]
  movdqa     xmm1, xmm4
  movdqa     xmm3, xmm5
  punpcklbw  xmm4, xmm7
  punpcklbw  xmm5, xmm7
  punpckhbw  xmm1, xmm7
  punpckhbw  xmm3, xmm7
  paddw      xmm0, xmm4                     ; sum_lo+=p1(low)
  movdqa     [p2_lo], xmm5
  paddw      xmm2, xmm1                     ; sum_hi+=p1(high)
  movdqa     [p2_hi],  xmm3
  paddw      xmm0, xmm5                     ; sum_lo+=p2(low)
  movdqa     [p1_lo], xmm4
  paddw      xmm2, xmm3                     ; sum_hi+=p2(high)
  movdqa     [p1_hi], xmm1
  paddw      xmm5, xmm0                     ; sum_filter4_lo= sum_lo+p2(lo)
  paddw      xmm3, xmm2                     ; sum_filter4_hi= sum_hi+p2(hi)
  movdqa     xmm4, [p3]
  movdqa     xmm1, xmm4
  punpcklbw  xmm4, xmm7
  punpckhbw  xmm1, xmm7
  paddw      xmm5, xmm4                     ; sum_filter4_lo+=p3(lo)
  paddw      xmm3, xmm1                     ; sum_filter4_hi+=p3(hi)
  movdqa     [p3_lo], xmm4
  paddw      xmm0, xmm4                     ; sum_lo+=p3(lo)
  movdqa     [p3_hi], xmm1
  paddw      xmm2, xmm1                     ; sum_hi+=p3(hi)
  movdqa     [flat2_lo], xmm0
  movdqa     [flat2_hi], xmm2
  movdqa     xmm0, xmm4
  movdqa     xmm2, xmm1
  psubw      xmm5, [GLOBAL(four)]             ; sum_filter4_lo-=4
  psllw      xmm4, 0x1                        ; p3*2(lo)
  psubw      xmm3, [GLOBAL(four)]             ; sum_filter4_hi-=4
  psllw      xmm1, 0x1                        ; p3*2(hi)
  paddw      xmm5, xmm4                       ; sum_filter4_lo+=p3*2(lo)
  paddw      xmm3, xmm1                       ; sum_filter4_hi+=p3*2(hi)
  movdqa     xmm6, xmm5
  movdqa     xmm4, xmm3
  psrlw      xmm6, 0x3
  psubw      xmm5, xmm0                       ; sum_filter4_lo-=p3(lo)
  psrlw      xmm4, 0x3
  psubw      xmm3, xmm2                       ; sum_filter4_hi-=p3(hi)
  packuswb   xmm6, xmm4
  psubw      xmm5, [p2_lo]                    ; sum_filter4_lo-=p2(lo)
  psubw      xmm3, [p2_hi]                    ; sum_filter4_hi-=p2(hi)
  movdqa     [flat_p2], xmm6
  movdqa     xmm1, [q1]
  movdqa     xmm4, xmm1
  punpcklbw  xmm1, xmm7
  punpckhbw  xmm4, xmm7
  paddw      xmm5, xmm1                        ; sum_filter4_lo+=q1(lo)
  movdqa     [q1_lo], xmm1
  paddw      xmm3, xmm4                        ; sum_filter4_hi+=q1(hi)
  movdqa     [q1_hi], xmm4
  movdqa     xmm1, [p1_lo]
  movdqa     xmm4, [p1_hi]
  paddw      xmm5, xmm1                        ; sum_filter4_lo+=p1(lo)
  paddw      xmm3, xmm4                        ; sum_filter4_hi+=p1(hi)
  movdqa     xmm6, xmm5
  movdqa     xmm7, xmm3
  psrlw      xmm6, 0x3
  psubw      xmm5, xmm0                        ; sum_filter4_lo-=p3(lo)
  psrlw      xmm7, 0x3
  psubw      xmm3, xmm2                        ; sum_filter4_hi-=p3(hi)
  packuswb   xmm6, xmm7
  psubw      xmm5, xmm1                        ; sum_filter4_lo-=p1(lo)
  psubw      xmm3, xmm4                        ; sum_filter4_hi-=p1(hi)
  movdqa     [flat_p1], xmm6
  pxor       xmm7, xmm7
  movdqa     xmm1, [q2]
  movdqa     xmm4, xmm1
  punpcklbw  xmm1, xmm7
  punpckhbw  xmm4, xmm7
  paddw      xmm5, xmm1                        ; sum_filter4_lo+=q2(lo)
  movdqa     [q2_lo], xmm1
  paddw      xmm3, xmm4                        ; sum_filter4_hi+=q2(hi)
  movdqa     [q2_hi],  xmm4
  movdqa     xmm1, [p0_lo]
  movdqa     xmm4, [p0_hi]
  paddw      xmm5, xmm1                        ; sum_filter4_lo+=p0(lo)
  paddw      xmm3, xmm4                        ; sum_filter4_hi+=p0(hi)
  movdqa     xmm6, xmm5
  movdqa     xmm7, xmm3
  psrlw      xmm6, 0x3
  psubw      xmm5, xmm0                        ;  sum_filter4_lo-=p3(lo)
  psrlw      xmm7, 0x3
  psubw      xmm3, xmm2                        ;  sum_filter4_hi-=p3(hi)
  packuswb   xmm6, xmm7
  psubw      xmm5, xmm1                        ; sum_filter4_lo-=p0(lo)
  psubw      xmm3, xmm4                        ; sum_filter4_hi-=p0(hi)
  movdqa     [flat_p0], xmm6
  pxor       xmm7, xmm7
  movdqa     xmm1, [q3]
  movdqa     xmm4, xmm1
  punpcklbw  xmm1, xmm7
  punpckhbw  xmm4, xmm7
  movdqa     xmm0, [q0_lo]
  movdqa     xmm2, [q0_hi]
  paddw      xmm5, xmm1                        ; sum_filter4_lo+=q3(lo)
  paddw      xmm3, xmm4                        ; sum_filter4_hi+=q3(hi)
  paddw      xmm5, xmm0                        ; sum_filter4_lo+=q0(lo)
  paddw      xmm3, xmm2                        ; sum_filter4_hi+=q0(hi)
  movdqa     [q3_lo], xmm1
  movdqa     [q3_hi], xmm4
  movdqa     xmm6, xmm5
  movdqa     xmm7, xmm3
  psrlw      xmm6, 0x3
  psubw      xmm5, [p2_lo]                     ; sum_filter4_lo-=p2(lo)
  psrlw      xmm7, 0x3
  psubw      xmm3, [p2_hi]                     ; sum_filter4_hi-=p2(hi)
  packuswb   xmm6, xmm7
  psubw      xmm5, xmm0                        ; sum_filter4_lo-=q0(lo)
  psubw      xmm3, xmm2                        ; sum_filter4_hi-=q0(hi)
  movdqa     [flat_q0], xmm6
  movdqa     xmm0, [q1_lo]
  movdqa     xmm2, [q1_hi]
  paddw      xmm5, xmm1                         ; sum_filter4_lo+=q3(lo)
  paddw      xmm3, xmm4                         ; sum_filter4_hi+=q3(hi)
  paddw      xmm5, xmm0                         ; sum_filter4_lo+=q1(lo)
  paddw      xmm3, xmm2                         ; sum_filter4_hi+=q1(hi)
  movdqa     xmm6, xmm5
  movdqa     xmm7, xmm3
  psrlw      xmm6, 0x3
  psubw      xmm5, [p1_lo]                      ; sum_filter4_lo-=p1(lo)
  psrlw      xmm7, 0x3
  psubw      xmm3, [p1_hi]                      ; sum_filter4_hi-=p1(hi)
  packuswb   xmm6, xmm7
  psubw      xmm5, xmm0                         ; sum_filter4_lo-=q1(lo)
  psubw      xmm3, xmm2                         ; sum_filter4_hi-=q1(hi)
  movdqa     [flat_q1], xmm6
  paddw      xmm5, xmm1                          ; sum_filter4_lo+=q3(lo)
  paddw      xmm3, xmm4                          ; sum_filter4_hi+=q3(hi)
  paddw      xmm5, [q2_lo]                       ; sum_filter4_lo+=q2(lo)
  paddw      xmm3, [q2_hi]                       ; sum_filter4_hi+=q2(hi)
  psrlw      xmm5, 0x3
  movdqa     xmm0, [flat2_lo]                    ; load sum_filter8_lo
  psrlw      xmm3, 0x3
  movdqa     xmm1, [flat2_hi]                    ; load sum_filter8_hi
  pxor       xmm7, xmm7
  packuswb   xmm5, xmm3
  movdqa     xmm2, [p4]
  movdqa     xmm4, [p5]
  movdqa     xmm3, xmm2
  movdqa     xmm6, xmm4
  movdqa     [flat_q2], xmm5
  punpcklbw  xmm2, xmm7
  punpckhbw  xmm3, xmm7
  movdqa     [p4_lo], xmm2
  paddw      xmm0, xmm2                           ; sum_filter8_lo+=p4(lo)
  paddw      xmm1, xmm3                           ; sum_filter8_hi+=p4(hi)
  movdqa     [p4_hi], xmm3
  punpcklbw  xmm4, xmm7
  punpckhbw  xmm6, xmm7
  paddw      xmm0, xmm4                           ; sum_filter8_lo+=p5(lo)
  movdqa     [p5_lo], xmm4
  paddw      xmm1, xmm6                           ; sum_filter8_hi+=p5(hi)
  movdqa     [p5_hi], xmm6
  movdqa     xmm2, [p6]
  movdqa     xmm3, xmm2
  punpcklbw  xmm2, xmm7
  punpckhbw  xmm3, xmm7
  movdqa     [p6_lo], xmm2
  movdqa     xmm4, [p7]
  movdqa     [p6_hi], xmm3
  psllw      xmm2, 0x1                             ; p6*2(lo)
  movdqa     xmm6, xmm4
  psllw      xmm3, 0x1                             ; p6*2(hi)
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p6*2(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p6*2(hi)
  punpcklbw  xmm4, xmm7
  punpckhbw  xmm6, xmm7
  movdqa     xmm2, xmm4
  movdqa     xmm3, xmm6
  psllw      xmm2, 0x3                             ; p7*8(lo)
  psllw      xmm3, 0x3                             ; p7*8(hi)
  psubw      xmm2, xmm4                            ; p7*7(lo)
  psubw      xmm3, xmm6                            ; p7*7(hi)
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p7*7(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p7*7(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, xmm4                            ; sum_filter8_lo-=p7(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, xmm6                            ; sum_filter8_hi-=p7(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, [p6_lo]                         ; sum_filter8_lo-=p6(lo)
  psubw      xmm1, [p6_hi]                         ; sum_filter8_hi-=p6(hi)
  movdqa     [flat2_p6], xmm5
  movdqa     xmm2, [p5_lo]
  movdqa     xmm3, [p5_hi]
  paddw      xmm0, [q1_lo]                         ; sum_filter8_lo+=q1(lo)
  paddw      xmm1, [q1_hi]                         ; sum_filter8_hi+=q1(hi)
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p5(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p5(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, xmm4                            ; sum_filter8_lo-=p7(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, xmm6                            ; sum_filter8_hi-=p7(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                            ; sum_filter8_lo-=p5(lo)
  psubw      xmm1, xmm3                            ; sum_filter8_hi-=p5(hi)
  movdqa     [flat2_p5], xmm5
  movdqa     xmm2, [p4_lo]
  movdqa     xmm3, [p4_hi]
  paddw      xmm0, [q2_lo]                         ; sum_filter8_lo+=q2(lo)
  paddw      xmm1, [q2_hi]                         ; sum_filter8_hi+=q2(hi)
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p4(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p4(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, xmm4                            ; sum_filter8_lo-=p7(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, xmm6                            ; sum_filter8_hi-=p7(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                            ; sum_filter8_lo+=p4(lo)
  psubw      xmm1, xmm3                            ; sum_filter8_hi+=p4(hi)
  movdqa     [flat2_p4], xmm5
  movdqa     xmm2, [p3_lo]
  movdqa     xmm3, [p3_hi]
  paddw      xmm0, [q3_lo]                         ; sum_filter8_lo+=q3(lo)
  paddw      xmm1, [q3_hi]                         ; sum_filter8_hi+=q3(hi)
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p3(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p3(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, xmm4                            ; sum_filter8_lo-=p7(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, xmm6                            ; sum_filter8_hi-=p7(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                            ; sum_filter8_lo-=p3(lo)
  psubw      xmm1, xmm3                            ; sum_filter8_hi-=p3(hi)
  movdqa     [flat2_p3], xmm5
  pxor       xmm7, xmm7
  movdqa     xmm2, [q4]
  movdqa     xmm3, xmm2
  punpcklbw  xmm2, xmm7
  punpckhbw  xmm3, xmm7
  movdqa     [q4_lo], xmm2
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=q4(lo)
  movdqa     [q4_hi], xmm3
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=q4(hi)
  movdqa     xmm2, [p2_lo]
  movdqa     xmm3, [p2_hi]
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p2(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p2(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, xmm4                            ; sum_filter8_lo-=p7(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, xmm6                            ; sum_filter8_hi-=p7(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                            ; sum_filter8_lo-=p2(lo)
  psubw      xmm1, xmm3                            ; sum_filter8_hi-=p2(hi)
  movdqa     [flat2_p2], xmm5
  pxor       xmm7, xmm7
  movdqa     xmm2, [q5]
  movdqa     xmm3, xmm2
  punpcklbw  xmm2, xmm7
  punpckhbw  xmm3, xmm7
  movdqa     [q5_lo], xmm2
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=q5(lo)
  movdqa     [q5_hi], xmm3
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=q5(hi)
  movdqa     xmm2, [p1_lo]
  movdqa     xmm3, [p1_hi]
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p1(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p1(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, xmm4                            ; sum_filter8_lo-=p7(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, xmm6                            ; sum_filter8_hi-=p7(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                            ; sum_filter8_lo-=p1(lo)
  psubw      xmm1, xmm3                            ; sum_filter8_hi-=p1(hi)
  movdqa     [flat2_p1], xmm5
  pxor       xmm7, xmm7
  movdqa     xmm2, [q6]
  movdqa     xmm3, xmm2
  punpcklbw  xmm2, xmm7
  punpckhbw  xmm3, xmm7
  movdqa     [q6_lo], xmm2
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=q6(lo)
  movdqa     [q6_hi], xmm3
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=q6(hi)
  movdqa     xmm2, [p0_lo]
  movdqa     xmm3, [p0_hi]
  paddw      xmm0, xmm2                            ; sum_filter8_lo+=p0(lo)
  paddw      xmm1, xmm3                            ; sum_filter8_hi+=p0(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, xmm4                            ; sum_filter8_lo-=p7(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, xmm6                            ; sum_filter8_hi-=p7(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                            ; sum_filter8_lo-=p0(lo)
  psubw      xmm1, xmm3                            ; sum_filter8_hi-=p0(hi)
  movdqa     [flat2_p0], xmm5
  pxor       xmm7, xmm7
  movdqa     xmm4, [q7]
  movdqa     xmm6, xmm4
  punpcklbw  xmm4, xmm7
  punpckhbw  xmm6, xmm7
  movdqa     xmm2, [q0_lo]
  movdqa     xmm3, [q0_hi]
  paddw      xmm0, xmm4                             ; sum_filter8_lo+=q7(lo)
  paddw      xmm1, xmm6                             ; sum_filter8_hi+=q7(hi)
  paddw      xmm0, xmm2                             ; sum_filter8_lo+=q0(lo)
  paddw      xmm1, xmm3                             ; sum_filter8_hi+=q0(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, [p6_lo]                          ; sum_filter8_lo-=p6(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, [p6_hi]                          ; sum_filter8_hi-=p6(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                             ; sum_filter8_lo-=q0(lo)
  psubw      xmm1, xmm3                             ; sum_filter8_hi-=q0(hi)
  movdqa     [flat2_q0], xmm5
  paddw      xmm0, xmm4                             ; sum_filter8_lo+=q7(lo)
  paddw      xmm1, xmm6                             ; sum_filter8_hi+=q7(hi)
  movdqa     xmm2, [q1_lo]
  movdqa     xmm3, [q1_hi]
  paddw      xmm0, xmm2                             ; sum_filter8_lo+=q1(lo)
  paddw      xmm1, xmm3                             ; sum_filter8_hi+=q1(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, [p5_lo]                          ; sum_filter8_lo-=p5(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, [p5_hi]                          ; sum_filter8_hi-=p5(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                             ; sum_filter8_lo-=q1(lo)
  psubw      xmm1, xmm3                             ; sum_filter8_hi-=q1(hi)
  movdqa     [flat2_q1], xmm5
  paddw      xmm0, xmm4                             ; sum_filter8_lo+=q7(lo)
  paddw      xmm1, xmm6                             ; sum_filter8_hi+=q7(hi)
  movdqa     xmm2, [q2_lo]
  movdqa     xmm3, [q2_hi]
  paddw      xmm0, xmm2                             ; sum_filter8_lo+=q2(lo)
  paddw      xmm1, xmm3                             ; sum_filter8_hi+=q2(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, [p4_lo]                          ; sum_filter8_lo-=p4(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, [p4_hi]                          ; sum_filter8_hi-=p4(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                             ; sum_filter8_lo-=q2(lo)
  psubw      xmm1, xmm3                             ; sum_filter8_hi-=q2(hi)
  movdqa     [flat2_q2], xmm5
  paddw      xmm0, xmm4                             ; sum_filter8_lo+=q7(lo)
  paddw      xmm1, xmm6                             ; sum_filter8_hi+=q7(hi)
  movdqa     xmm2, [q3_lo]
  movdqa     xmm3, [q3_hi]
  paddw      xmm0, xmm2                             ; sum_filter8_lo+=q3(lo)
  paddw      xmm1, xmm3                             ; sum_filter8_hi+=q3(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, [p3_lo]                          ; sum_filter8_lo-=p3(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, [p3_hi]                          ; sum_filter8_hi-=p3(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                             ; sum_filter8_lo-=q3(lo)
  psubw      xmm1, xmm3                             ; sum_filter8_hi-=q3(hi)
  movdqa     [flat2_q3], xmm5
  paddw      xmm0, xmm4                             ; sum_filter8_lo+=q7(lo)
  paddw      xmm1, xmm6                             ; sum_filter8_hi+=q7(hi)
  movdqa     xmm2, [q4_lo]
  movdqa     xmm3, [q4_hi]
  paddw      xmm0, xmm2                             ; sum_filter8_lo+=q4(lo)
  paddw      xmm1, xmm3                             ; sum_filter8_hi+=q4(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, [p2_lo]                          ; sum_filter8_lo-=p2(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, [p2_hi]                          ; sum_filter8_hi-=p2(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                             ; sum_filter8_lo-=q4(lo)
  psubw      xmm1, xmm3                             ; sum_filter8_hi-=q4(hi)
  movdqa     [flat2_q4], xmm5
  paddw      xmm0, xmm4                             ; sum_filter8_lo+=q7(lo)
  paddw      xmm1, xmm6                             ; sum_filter8_hi+=q7(hi)
  movdqa     xmm2, [q5_lo]
  movdqa     xmm3, [q5_hi]
  paddw      xmm0, xmm2                             ; sum_filter8_lo+=q5(lo)
  paddw      xmm1, xmm3                             ; sum_filter8_hi+=q5(hi)
  movdqa     xmm5, xmm0
  movdqa     xmm7, xmm1
  psrlw      xmm5, 0x4
  psubw      xmm0, [p1_lo]                          ; sum_filter8_lo-=p1(lo)
  psrlw      xmm7, 0x4
  psubw      xmm1, [p1_hi]                          ; sum_filter8_hi-=p1(hi)
  packuswb   xmm5, xmm7
  psubw      xmm0, xmm2                             ; sum_filter8_lo-=q5(lo)
  psubw      xmm1, xmm3                             ; sum_filter8_hi-=q5(hi)
  movdqa     [flat2_q5], xmm5
  paddw      xmm0, xmm4                             ; sum_filter8_lo+=q7(lo)
  paddw      xmm1, xmm6                             ; sum_filter8_hi+=q7(hi)
  movdqa     xmm2, [q6_lo]
  movdqa     xmm3, [q6_hi]
  paddw      xmm0, xmm2                             ; sum_filter8_lo+=q6(lo)
  paddw      xmm1, xmm3                             ; sum_filter8_hi-=q6(hi)
  psrlw      xmm0, 0x4
  lea        rsi,  [rsi + rax]
  psrlw      xmm1, 0x4
  sub        rdi,  rax
  packuswb   xmm0, xmm1
  movdqa     xmm5, [flat2]
  movdqa     xmm7, [flat2_p6]
  movdqa     xmm4, xmm5
  pand       xmm0, xmm5                              ; flat2_q6 if flat2=1
  pandn      xmm4, [q6]                              ; q6 if flat2=0
  movdqa     xmm3, xmm5
  por        xmm0, xmm4
  pand       xmm7, xmm5                              ; flat2_p6 if flat2=1
  pandn      xmm3, [p6]                              ; p6 if flat2=0
  por        xmm3, xmm7
  movdqu     [rdi], xmm0                             ; save q6
  movdqu     [rsi], xmm3                             ; save p6
  movdqa     xmm7, [flat2_q5]
  movdqa     xmm4, xmm5
  lea        rsi,  [rsi + rax]
  sub        rdi,  rax
  pand       xmm7, xmm5                              ; flat2_q5 if flat2=1
  pandn      xmm4, [q5]                              ; q5 if flat2=0
  movdqa     xmm1, [flat2_p5]
  movdqa     xmm3, xmm5
  por        xmm7, xmm4
  pand       xmm1, xmm5                             ; flat2_p5 if flat2=1
  pandn      xmm3, [p5]                             ; p5 if flat2=0
  por        xmm3, xmm1
  movdqu     [rdi], xmm7                            ; save q5
  movdqu     [rsi], xmm3                            ; save p5
  movdqa     xmm7, [flat2_q4]
  movdqa     xmm4, xmm5
  lea        rsi,  [rsi + rax]
  sub        rdi,  rax
  pand       xmm7, xmm5                              ; flat2_q4 if flat2=1
  pandn      xmm4, [q4]                              ; q4 if flat2=0
  movdqa     xmm1, [flat2_p4]
  movdqa     xmm3, xmm5
  por        xmm7, xmm4
  pand       xmm1, xmm5                              ; flat2_p4 if flat2=1
  pandn      xmm3, [p4]                              ; p4 if flat2=0
  por        xmm3, xmm1
  movdqu     [rdi], xmm7                             ; save q4
  movdqu     [rsi], xmm3                             ; save p4
  movdqa     xmm7, [flat2_q3]
  movdqa     xmm4, xmm5
  lea        rsi,  [rsi + rax]
  sub        rdi,  rax
  pand       xmm7, xmm5                              ; flat2_p3 if flat2=1
  pandn      xmm4, [q3]                              ; q3 if flat2=0
  movdqa     xmm1, [flat2_p3]
  movdqa     xmm3, xmm5
  por        xmm7, xmm4
  pand       xmm1, xmm5                              ; flat2_p3 if flat2=1
  pandn      xmm3, [p3]                              ; p3 if flat2=0
  por        xmm3, xmm1
  movdqu     [rdi], xmm7                             ; save q3
  movdqu     [rsi], xmm3                             ; save p3
  movdqa     xmm0, [flat]
  movdqa     xmm7, [flat_q2]
  movdqa     xmm4, xmm0
  lea        rsi,  [rsi + rax]
  sub        rdi,  rax
  pand       xmm7, xmm0                              ; flat_q2 if flat=1
  pandn      xmm4, [q2]                              ; q2 if flat=0
  movdqa     xmm1, [flat_p2]
  movdqa     xmm3, xmm0
  por        xmm7, xmm4                              ; save q2_tmp
  pand       xmm1, xmm0                              ; flat_p2 if flat=1
  pandn      xmm3, [p2]                              ; p2 if flat=0
  movdqa     xmm6, [flat2_q2]
  movdqa     xmm4, xmm5
  por        xmm3, xmm1                              ; save p2_tmp
  pand       xmm6, xmm5                              ; flat2_q2 if flat2=1
  pandn      xmm4, xmm7                              ; q2_tmp if flat2=0
  movdqa     xmm2, [flat2_p2]
  movdqa     xmm1, xmm5
  por        xmm4, xmm6
  pand       xmm2, xmm5                              ; flat2_p2 if flat2=1
  pandn      xmm1, xmm3                              ; p2_tmp if flat2=0
  por        xmm1, xmm2
  movdqu     [rdi], xmm4                             ; save q2
  movdqu     [rsi], xmm1                             ; save p2
  movdqa     xmm7, [flat_q1]
  movdqa     xmm4, xmm0
  lea        rsi,  [rsi + rax]
  sub        rdi,  rax
  pand       xmm7, xmm0                              ; flat_q1 if flat=1
  pandn      xmm4, [org_q1]                          ; q1 if flat=0
  movdqa     xmm1, [flat_p1]
  movdqa     xmm3, xmm0
  por        xmm7, xmm4                              ; save q1_tmp
  pand       xmm1, xmm0                              ; flat_p1 if flat=1
  pandn      xmm3, [org_p1]                          ; p1 if flat=0
  movdqa     xmm6, [flat2_q1]
  movdqa     xmm4, xmm5
  por        xmm3, xmm1                              ; save p1_tmp
  pand       xmm6, xmm5                              ; flat2_q1 if flat2=1
  pandn      xmm4, xmm7                              ; q1_tmp if flat2=0
  movdqa     xmm2, [flat2_p1]
  por        xmm4, xmm6
  movdqa     xmm1, xmm5
  pand       xmm2, xmm5                             ; flar2_p1 if flat2=1
  pandn      xmm1, xmm3                             ; p1_tmp flat2=0
  por        xmm1, xmm2
  movdqu     [rdi], xmm4                            ; save q1
  movdqu     [rsi], xmm1                            ; save p1
  movdqa     xmm7, [flat_q0]
  movdqa     xmm4, xmm0
  lea        rsi,  [rsi + rax]
  sub        rdi,  rax
  pand       xmm7, xmm0                             ; flat_q0 if flat=1
  pandn      xmm4, [org_q0]                         ; q0 if flat=0
  movdqa     xmm1, [flat_p0]
  movdqa     xmm3, xmm0
  por        xmm7, xmm4                             ; save q0_tmp
  pand       xmm1, xmm0                             ; flat_p0 if flat=1
  pandn      xmm3, [org_p0]                         ; p0 if flat=0
  movdqa     xmm6, [flat2_q0]
  movdqa     xmm4, xmm5
  por        xmm3, xmm1                             ; save p0_tmp
  pand       xmm6, xmm5                             ; flat2_q0 if flat2=1
  pandn      xmm4, xmm7                             ; q0_tmp if flat2=0
  movdqa     xmm2, [flat2_p0]
  por        xmm4, xmm6
  movdqa     xmm1, xmm5
  pand       xmm2, xmm5                             ; flat2_p0 if flat2=1
  pandn      xmm1, xmm3                             ; p0_tmp if flat2=0
  por        xmm1, xmm2
  movdqu     [rdi], xmm4                            ; save q0
  movdqu     [rsi], xmm1                            ; save p0
  xchg  rbp, rsp
  add        rsp, 0x3F0
  pop rsp
%if ABI_IS_32BIT
  pop rsi
  pop rdi
%endif
  ; begin epilog
  pop rbx
  RESTORE_GOT
  RESTORE_XMM
  UNSHADOW_ARGS
  pop rbp
  ret


SECTION_RODATA
align 16
zero:
     times 16 db 0x0
align 16
one:
     times 16 db 0x1
align 16
fe:
    times 16 db  0xfe
align 16
ff:
    times 16 db  0xff
align 16
te0:
    times 16 db  0xe0
align 16
t80:
    times 16 db 0x80
align 16
t7f:
    times 16 db  0x7f
align 16
t1f:
    times 16 db  0x1f
align 16
t4:
    times 16 db  0x4
align 16
t3:
    times 16 db  0x3
align 16
four:
    times 8 dw  0x4
align 16
eight:
    times 8 dw  0x8
