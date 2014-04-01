.file "filter16_v.S"
/*
static void filter16_v(uint8_t *data) {
  uint8_t p7 = data[0], p6 = data[1], p5 = data[2], p4 = data[3],
      p3 = data[4], p2 = data[5], p1 = data[6], p0 = data[7];

  uint8_t q0 = data[8], q1 = data[9], q2 = data[10], q3 = data[11], q4 =
      data[12], q5 = data[13], q6 = data[14], q7 = data[15];

  uint16_t tmp1, tmp2;
  uint16_t d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14;

//   unsigned long long top1=0, bot1=1;
//   top1 = Q6_P_vraddub_PP(top1, bot1);

  tmp1 = p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0;
  tmp2 = q0 + q1 + q2 + q3 + q4 + q5 + q6 + q7;

  d1 = 6 * p7 + tmp1 + q0 + p6;
  d2 = 5 * p7 + tmp1 + q0 + q1 + p5;
  d3 = 4 * p7 + tmp1 + q0 + q1 + q2 + p4;
  d4 = 3 * p7 + tmp1 + q0 + q1 + q2 + q3 + p3;
  d5 = 2 * p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + p2;
  d6 = p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + p1;
  d7 = tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + p0;
  d8 = p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q0;
  d9 = p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + q1;
  d10 = p4 + p3 + p2 + p1 + p0 + tmp2 + 2 * q7 + q2;
  d11 = p3 + p2 + p1 + p0 + tmp2 + 3 * q7 + q3;
  d12 = p2 + p1 + p0 + tmp2 + 4 * q7 + q4;
  d13 = p1 + p0 + tmp2 + 5 * q7 + q5;
  d14 = p0 + tmp2 + 6 * q7 + q6;

  // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
  data[1] = (d1 + 8) >> 4;
  data[2] = (d2 + 8) >> 4;
  data[3] = (d3 + 8) >> 4;
  data[4] = (d4 + 8) >> 4;
  data[5] = (d5 + 8) >> 4;
  data[6] = (d6 + 8) >> 4;
  data[7] = (d7 + 8) >> 4;
  data[8] = (d8 + 8) >> 4;
  data[9] = (d9 + 8) >> 4;
  data[10] = (d10 + 8) >> 4;
  data[11] = (d11 + 8) >> 4;
  data[12] = (d12 + 8) >> 4;
  data[13] = (d13 + 8) >> 4;
  data[14] = (d14 + 8) >> 4;
}
*/

/*
  r0 = uint8_t op7,
  r1 = uint8_t op6,
  r2 = uint8_t op5,
  r3 = uint8_t op4,
  r4 = uint8_t op3,
  r5 = uint8_t op2,
  r6 = uint8_t op1,
  r7 = uint8_t op0,
  r8 = uint8_t oq0,
  r9 = uint8_t oq1,
  r10 = uint8_t oq2,
  r11 = uint8_t oq3,
  r12 = uint8_t oq4,
  r13 = uint8_t oq5,
  r14 = uint8_t oq6,
  r15 = uint8_t oq7
*/

#define STACK_SIZE 32

.text
.p2align 4
.globl filter16_v_dsp
.type filter16_v_dsp, @function
filter16_v_dsp:
  {
    allocframe(#STACK_SIZE)
    r0 = memw(r0)
    r28 = r0
  }
  {
    memd(sp) = r17:16
    memd(sp + #8) = r19:18
    r18 = #0
  }
  {
    memd(sp + #16) = r21:20
    memd(sp + #24) = r23:22
  }
  {
    r1:0 = vzxtbh(r0) // p7...
    r4 = memw(r28 + #4)
    r8 = memw(r28 + #8)
  }
  {
    r5:4 = vzxtbh(r4) // p3...
    r9:8 = vzxtbh(r8) // q0...
    r12 = memw(r28 + #12)
    r22 = combine(r18.H, r0.L)  // p7
  }
  {
    r16 = vradduh(r1:0, r5:4) // tmp1 = p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0
    r19 = combine(r18.H, r8.L)  // q0
    r18 = mpyi(r22, #6) // 6*p7
  }
  {
    r20 = add(r18, r16) // 6*p7 + tmp1
    r3:2 = vzxthw(r1) // [p4, p5]
    r1:0 = vzxthw(r0) // [p6, p7]
    r22 = sub(r16, r22) // p6 + p5 + p4 + p3 + p2 + p1 + p0
  }
  {
    r20 = add(r20, add(r19, #8))  // 6*p7 + tmp1 + q0 + 8
    r7:6 = vzxthw(r5) // [p0, p1]
    r19 = combine(r18.H, r8.H)  // q1
  }
  {
    r21 = add(r20, r1)  // 6*p7 + tmp1 + q0 + p6 + 8
    r5:4 = vzxthw(r4) // [p2, p3]
    r13:12 = vzxtbh(r12)  // q4, q5, q6, q7
    r20 = sub(r20, r0)  // 5*p7 + tmp1 + q0 + 8
  }
  {
    r21 = lsr(r21, #4)
    memb(r28 + #1) = r21.new
    r17 = vradduh(r9:8, r13:12) // tmp2 = q0 + q1 + q2 + q3 + q4 + q5 + q6 + q7
    r20 = add(r20, r19) // 5*p7 + tmp1 + q0 + q1 + 8
  }
  {
    r21 = add(r20, r2)  // 5*p7 + tmp1 + q0 + q1 + p5 + 8
    r11:10 = vzxthw(r9) // [q3, q2]
    r9:8 = vzxthw(r8) // [q1, q0]
    r20 = sub(r20, r0)  // 4*p7 + tmp1 + q0 + q1 + 8
  }
  {
    r21 = lsr(r21, #4)
    memb(r28 + #2) = r21.new
    r15:14 = vzxthw(r13)  // [q7, q6]
    r20 = add(r20, r10) // 4*p7 + tmp1 + q0 + q1 + q2 + 8
  }
  {
    r13:12 = vzxthw(r12)  // [q5, q4]
    r21 = add(r20, r3)  // 4*p7 + tmp1 + q0 + q1 + q2 + p4 + 8
    r20 = sub(r20, r0)  // 3*p7 + tmp1 + q0 + q1 + q2 + 8
    r22 = add(r22, add(r17, #8)) // p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + 8
  }
  {
    r21 = lsr(r21, #4)
    memb(r28 + #3) = r21.new
    r20 = add(r20, r11) // 3*p7 + tmp1 + q0 + q1 + q2 + q3 + 8
    r23 = add(r22, r8)  // p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q0 + 8
  }
  {
    r21 = add(r20, r4)  // 3*p7 + tmp1 + q0 + q1 + q2 + q3 + p3 + 8
    r20 = sub(r20, r0)  // 2*p7 + tmp1 + q0 + q1 + q2 + q3 + 8
    r23 = lsr(r23, #4)
    memb(r28 + #8) = r23.new
  }
  {
    r21 = lsr(r21, #4)
    memb(r28 + #4) = r21.new
    r20 = add(r20, r12) // 2*p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + 8
    r22 = sub(r22, r1)  // p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q0 + 8
  }
  {
    r21 = add(r20, r5)  // 2*p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + p2 + 8
    r20 = sub(r20, r0)  // p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + 8
    r22 = add(r22, r15) // p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + 8
  }
  {
    r21 = lsr(r21, #4)
    memb(r28 + #5) = r21.new
    r20 = add(r20, r13) // p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + 8
    r23 = add(r22, r9)  // p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + q1 + 8
  }
  {
    r21 = add(r20, r6)  // p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + p1 + 8
    r20 = sub(r20, r0)  // tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + 8
    r23 = lsr(r23, #4)
    memb(r28 + #9) = r23.new
  }
  {
    r21 = lsr(r21, #4)
    memb(r28 + #6) = r21.new
    r20 = add(r20, r14) // tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + 8
    r22 = sub(r22, r2)  // p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + 8
  }
  {
    r21 = add(r20, r7)  // tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + p0 + 8
    r22 = add(r22, r15) // p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + 8
  }
  {
    r21 = lsr(r21, #4)
    memb(r28 + #7) = r21.new
    r23 = add(r22, r10) // p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + q2 + 8
    r22 = sub(r22, r3)  // p3 + p2 + p1 + p0 + tmp2 + 2*q7 + 8
  }
  {
    r23 = lsr(r23, #4)
    memb(r28 + #10) = r23.new
    r22 = add(r22, r15) // p3 + p2 + p1 + p0 + tmp2 + 3*q7 + 8
  }
  {
    r23 = add(r22, r11) // p3 + p2 + p1 + p0 + tmp2 + 3*q7 + q3 + 8
    r22 = sub(r22, r4)  // p2 + p1 + p0 + tmp2 + 3*q7 + 8
  }
  {
    r23 = lsr(r23, #4)
    memb(r28 + #11) = r23.new
    r22 = add(r22, r15) // p2 + p1 + p0 + tmp2 + 4*q7 + 8
  }
  {
    r23 = add(r22, r12) // p2 + p1 + p0 + tmp2 + 4*q7 + q4 + 8
    r22 = sub(r22, r5)  // p1 + p0 + tmp2 + 4*q7 + 8
  }
  {
    r23 = lsr(r23, #4)
    memb(r28 + #12) = r23.new
    r22 = add(r22, r15) // p1 + p0 + tmp2 + 5*q7 + 8
  }
  {
    r23 = add(r22, r13) // p1 + p0 + tmp2 + 5*q7 + q5 + 8
    r22 = sub(r22, r6)  // p0 + tmp2 + 5*q7 + 8
    r0 = add(r15, r14)  // q7 + q6
    r17:16 = memd(sp)
  }
  {
    r23 = lsr(r23, #4)
    memb(r28 + #13) = r23.new
    r22 = add(r22, r0) // p0 + tmp2 + 6*q7 + q6 + 8
    r19:18 = memd(sp + #8)
  }
  {
    r23 = lsr(r22, #4)
    memb(r28 + #14) = r23.new
    r21:20 = memd(sp + #16)
  }
  {
    r23:22 = memd(sp + #24)
    dealloc_return
  }
