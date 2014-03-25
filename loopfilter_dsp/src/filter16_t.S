.file "filter16_t.S"
/*
static INLINE void filter16_t(uint8_t op7, uint8_t *op6, uint8_t *op5, uint8_t *op4,
    uint8_t *op3, uint8_t *op2, uint8_t *op1, uint8_t *op0, uint8_t *oq0,
    uint8_t *oq1, uint8_t *oq2, uint8_t *oq3, uint8_t *oq4, uint8_t *oq5,
    uint8_t *oq6, uint8_t oq7) {
  uint8_t p7 = op7, p6 = *op6, p5 = *op5, p4 = *op4, p3 = *op3, p2 = *op2,
      p1 = *op1, p0 = *op0;

  uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3, q4 = *oq4, q5 = *oq5,
      q6 = *oq6, q7 = oq7;

  uint16_t tmp1, tmp2;

//   unsigned long long top1=0, bot1=1;
//   top1 = Q6_P_vraddub_PP(top1, bot1);

  tmp1 = p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0;
  tmp2 = q0 + q1 + q2 + q3 + q4 + q5 + q6 + q7;

  // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
  *op6 = ROUND_POWER_OF_TWO(6*p7 + tmp1 + q0 + p6, 4);
  *op5 = ROUND_POWER_OF_TWO(5*p7 + tmp1 + q0 + q1 + p5, 4);
  *op4 = ROUND_POWER_OF_TWO(4*p7 + tmp1 + q0 + q1 + q2 + p4, 4);
  *op3 = ROUND_POWER_OF_TWO(3*p7 + tmp1 + q0 + q1 + q2 + q3 + p3, 4);
  *op2 = ROUND_POWER_OF_TWO(2*p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + p2, 4);
  *op1 = ROUND_POWER_OF_TWO(p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + p1, 4);
  *op0 = ROUND_POWER_OF_TWO(tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + p0, 4);
  *oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q0, 4);
  *oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + q1, 4);
  *oq2 = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + q2, 4);
  *oq3 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 + tmp2 + 3*q7 + q3, 4);
  *oq4 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + tmp2 + 4*q7 + q4, 4);
  *oq5 = ROUND_POWER_OF_TWO(p1 + p0 + tmp2 + 5*q7 + q5, 4);
  *oq6 = ROUND_POWER_OF_TWO(p0 + tmp2 + 6*q7 + q6, 4);
}
*/

/*
  r0 = uint8_t op7,
  r1 = uint8_t *op6,
  r2 = uint8_t *op5,
  r3 = uint8_t *op4,
  r4 = uint8_t *op3,
  r5 = uint8_t *op2,
  r6 = uint8_t *op1,
  r7 = uint8_t *op0,
  r8 = uint8_t *oq0,
  r9 = uint8_t *oq1,
  r10 = uint8_t *oq2,
  r11 = uint8_t *oq3,
  r12 = uint8_t *oq4,
  r13 = uint8_t *oq5,
  r14 = uint8_t *oq6,
  r15 = uint8_t oq7
*/

#define STACK_SIZE 48
#define PARAMS_OFF 56

.text
.p2align 4
.globl filter16_t_dsp
.type filter16_t_dsp, @function
filter16_t_dsp:
  {
    allocframe(#STACK_SIZE)
    r7:6 = memd(sp)
  }
  {
    memd(sp) = r17:16
    memd(sp+#8) = r19:18
  }
  {
    r17:16 = memb_fifo(r1)
    r19:18 = memb_fifo(r4)
  }
  {
    r17:16 = memb_fifo(r2)
    r19:18 = memb_fifo(r5)
  }
  {
    r17:16 = memb_fifo(r3)
    r19:18 = memb_fifo(r6)
  }
  {
    r17 = insert(r0, #8, #0)
    r31 = mpyi(r0, #6) // p7*6
    r19:18 = memb_fifo(r7)
    r9:8 = memd(sp + #(8+PARAMS_OFF))
  }
  {
    r17:16 = vzxtbh(r17)
    memd(sp+#16) = r21:20
    r21:20 = memb_fifo(r8)
  }
  {
    r19:18 = vzxtbh(r19)
    r11:10 = memd(sp + #(16+PARAMS_OFF))
    r21:20 = memb_fifo(r9)
  }
  {
    r13:12 = memd(sp + #(24+PARAMS_OFF))
    r21:20 = memb_fifo(r10)
  }
  {
    memd(sp+#32) = r25:24
    r21:20 = memb_fifo(r11)
    r24 = vradduh(r17:16, r19:18) // tmp1 = p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0
  }
  {
    r25 = add(r31, add(r24, #8)) // 6*p7 + tmp1 + 8
    memd(sp+#24) = r23:22
    r15:14 = memd(sp + #(32+PARAMS_OFF))
    r21:20 = vzxtbh(r21)
  }
  {
    memd(sp+#40) = r27:26
    r25 = add(r20.L, r25.L) // 6*p7 + tmp1 + q0 + 8
    r23:22 = memb_fifo(r12)
    r28 = combine(r0.H, r20.H)  // q1
  }
  {
    r26 = add(r25.L, r16.H) // 6*p7 + tmp1 + q0 + p6 + 8
    r23:22 = memb_fifo(r13)
    r25 = sub(r25, r0) // 5*p7 + tmp1 + q0 + 8
  }
  {
    r26 = lsr(r26, #4)  // (6*p7 + tmp1 + q0 + p6 + 8) >> 4
    memb(r1) = r26.new  // r1 is free for use now
    r23:22 = memb_fifo(r14)
    r25 = add(r25, r28) // 5*p7 + tmp1 + q0 + q1 + 8
  }
  {
    r23 = insert(r15, #8, #0)
    r26 = add(r25.L, r17.L)  // 5*p7 + tmp1 + q0 + q1 + p5 + 8
    r25 = sub(r25, r0) // 4*p7 + tmp1 + q0 + q1 + 8
    r28 = combine(r0.H, r21.L)  // q2
  }
  {
    r23:22 = vzxtbh(r23)
    r26 = lsr(r26, #4)  // (5*p7 + tmp1 + q0 + q1 + p5 + 8) >> 4
    memb(r2) = r26.new
    r25 = add(r25, r28) // 4*p7 + tmp1 + q0 + q1 + q2 + 8
  }
  {
    r26 = add(r25.L, r17.H)  // 4*p7 + tmp1 + q0 + q1 + q2 + p4 + 8
    r25 = sub(r25, r0) // 3*p7 + tmp1 + q0 + q1 + q2 + 8
    r28 = vradduh(r21:20, r23:22) // tmp2 = q0 + q1 + q2 + q3 + q4 + q5 + q6 + q7
  }
  {
    r26 = lsr(r26, #4)  // (4*p7 + tmp1 + q0 + q1 + q2 + p4 + 8) >> 4
    memb(r3) = r26.new
    r25 = add(r25.L, r21.H) // 3*p7 + tmp1 + q0 + q1 + q2 + q3 + 8
    r27 = sub(r24, r0) // p6 + p5 + p4 + p3 + p2 + p1 + p0
  }
  {
    r26 = add(r25.L, r18.L)  // 3*p7 + tmp1 + q0 + q1 + q2 + q3 + p3 + 8
    r25 = sub(r25, r0) // 2*p7 + tmp1 + q0 + q1 + q2 + q3 + p3 + 8
    r27 = add(r27, add(r28, #8)) // p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + 8
  }
  {
    r26 = lsr(r26, #4)  // (3*p7 + tmp1 + q0 + q1 + q2 + q3 + p3 + 8) >> 4
    memb(r4) = r26.new
    r25 = add(r25.L, r22.H) // 2*p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + 8
    r24 = combine(r0.H, r16.H) // p6
  }
  {
    r26 = add(r25.L, r18.H) // 2*p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + p2 + 8
    r25 = sub(r25, r0) // p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + 8
    r28 = add(r27.L, r20.L) // p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q0 + 8
    r27 = sub(r27, r24)  // p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + 8
  }
  {
    r26 = lsr(r26, #4)  // (2*p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + p2 + 8) >> 4
    memb(r5) = r26.new
    r25 = add(r25.L, r23.L) // p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + 8
    r27 = add(r27, r15) // p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + 8
  }
  {
    r26 = add(r25.L, r19.L) // p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + p1 + 8
    r25 = sub(r25, r0) // tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + 8
    r28 = lsr(r28, #4)
    memb(r8) = r28.new
  }
  {
    r26 = lsr(r26, #4)  // (p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + p1 + 8) >> 4
    memb(r6) = r26.new
    r25 = add(r25.L, r23.H) // tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + 8
    r24 = add(r27, r15) // p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + 8
  }
  {
    r26 = add(r25.L, r19.H) // tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + p0 + 8
    r1 = add(r27.L, r20.H)  // p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + q1 + 8
    r0 = combine(r0.H, r17.L) // p5
  }
  {
    r26 = lsr(r26, #4)  // (tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + p0 + 8) >> 4
    memb(r7) = r26.new
    r25 = lsr(r1, #4) // (p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + q1 + 8) >> 4
    r24 = sub(r24, r0)  // p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + 8
  }
  {
    memb(r9) = r25
    r25 = add(r24.L, r21.L) // p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + q2 + 8
    r24 = sub(r24.L, r17.H) // p3 + p2 + p1 + p0 + tmp2 + 2*q7 + 8
  }
  {
    r25 = lsr(r25, #4)  // (p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + q2 + 8) >> 4
    memb(r10) = r25.new
    r24 = add(r24, r15) // p3 + p2 + p1 + p0 + tmp2 + 3*q7 + 8
  }
  {
    r25 = add(r24.L, r21.H) // p3 + p2 + p1 + p0 + tmp2 + 3*q7 + q3 + 8
    r24 = sub(r24.L, r18.L) // p2 + p1 + p0 + tmp2 + 3*q7 + 8
  }
  {
    r25 = lsr(r25, #4)
    memb(r11) = r25.new
    r24 = add(r24, r15) // p2 + p1 + p0 + tmp2 + 4*q7 + 8
  }
  {
    r25 = add(r24.L, r22.H) // p2 + p1 + p0 + tmp2 + 4*q7 + q4 + 8
    r24 = sub(r24.L, r18.H) // p1 + p0 + tmp2 + 4*q7 + 8
  }
  {
    r25 = lsr(r25, #4)  // (p2 + p1 + p0 + tmp2 + 4*q7 + q4 + 8) >> 4
    memb(r12) = r25.new
    r24 = add(r24, r15) // p1 + p0 + tmp2 + 5*q7 + 8
    r2 = combine(r0.H, r23.H)
  }
  {
    r25 = add(r24.L, r23.L) // p1 + p0 + tmp2 + 5*q7 + q5 + 8
    r24 = sub(r24.L, r19.L) // p0 + tmp2 + 5*q7 + 8
    r2 = add(r15, r2) // q7 + q6
  }
  {
    r25 = lsr(r25, #4)  //  (p1 + p0 + tmp2 + 5*q7 + q5 + 8) >> 4
    memb(r13) = r25.new
    r24 = add(r24, r2) // p0 + tmp2 + 6*q7 + q6 + 8
  }
  {
    r25 = lsr(r24, #4)
    memb(r14) = r25.new
    r17:16 = memd(sp)
  }
  {
    r19:18 = memd(sp+#8)
    r21:20 = memd(sp+#16)
  }
  {
    r23:22 = memd(sp+#24)
    r25:24 = memd(sp+#32)
  }
  {
    r27:26 = memd(sp+#40)
    dealloc_return
  }
