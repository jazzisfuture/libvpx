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
