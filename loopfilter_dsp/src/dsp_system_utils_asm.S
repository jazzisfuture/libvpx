.file "dsp_system_utils_asm.S"

  .text
  .p2align 4
  .globl sleep_5_cycles
  .type sleep_5_cycles, @function
sleep_5_cycles:
  {
    pause(#5)
  }
  {
    JUMPR R31                         // return
  }
  .size  sleep_5_cycles, .-sleep_5_cycles

  .text
  .p2align 4
  .globl sleep_10_cycles
  .type sleep_10_cycles, @function
sleep_10_cycles:
  {
    pause(#10)
  }
  {
    JUMPR R31                         // return
  }
  .size  sleep_10_cycles, .-sleep_10_cycles

  .text
  .p2align 4
  .globl sleep_25_cycles
  .type sleep_25_cycles, @function
sleep_25_cycles:
  {
    pause(#25)
  }
  {
    JUMPR R31                         // return
  }
  .size  sleep_25_cycles, .-sleep_25_cycles


  .text
  .p2align 4
  .globl sleep_50_cycles
  .type sleep_50_cycles, @function
sleep_50_cycles:
  {
    pause(#50)
  }
  {
    JUMPR R31                         // return
  }
  .size  sleep_50_cycles, .-sleep_50_cycles


  .text
  .p2align 4
  .globl sleep_100_cycles
  .type sleep_100_cycles, @function
sleep_100_cycles:
  {
    pause(#100)
  }
  {
    JUMPR R31                         // return
  }
  .size  sleep_100_cycles, .-sleep_100_cycles


  .text
  .p2align 4
  .globl sleep_200_cycles
  .type sleep_200_cycles, @function
sleep_200_cycles:
  {
    pause(#200)
  }
  {
    JUMPR R31                         // return
  }
  .size  sleep_200_cycles, .-sleep_200_cycles


  .text
  .p2align 4
  .globl sleep_250_cycles
  .type sleep_250_cycles, @function
sleep_250_cycles:
  {
    pause(#250)
  }
  {
    JUMPR R31                         // return
  }
  .size  sleep_250_cycles, .-sleep_250_cycles


  .text
  .p2align 4
  .globl atomic_read
  .type atomic_read, @function
atomic_read:
  {
    R0 = memw_locked(R0)
  }
  {
    JUMPR R31                         // return
  }
  .size  atomic_read, .-atomic_read
