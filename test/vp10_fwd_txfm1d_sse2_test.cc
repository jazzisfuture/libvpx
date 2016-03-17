#include "vp10/common/vp10_fwd_txfm1d.h"
#include "vp10/common/x86/vp10_txfm1d_sse2.h"

#include "test/acm_random.h"
#include "test/vp10_txfm_test.h"

using libvpx_test::ACMRandom;

namespace {
TEST(Txfm1dSse2Test, round_shift_32_sse2) {
  int bit = 2;
  M128I a = _mm_setr_epi32(7, -7, 16, -16);
  M128I b = round_shift_32_sse2(a, bit);

  int* a_int = (int*)&a;
  int* b_int = (int*)&b;

  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(b_int[i], round_shift(a_int[i], bit));
  }
}

TEST(Txfm1dSse2Test, round_shift_array_32_sse2) {
  int bit = -2;
  M128I a[2];
  a[0] = _mm_setr_epi32(7, -7, 16, -16);
  a[1] = _mm_setr_epi32(7, -7, 16, -16);

  M128I b[2];
  int size = 2;
  int num_per_128 = 4;

  round_shift_array_32_sse2(a, b, 2, bit);

  int* a_int = (int*)&a;
  int* b_int = (int*)&b;

  for (int i = 0; i < size * num_per_128; i++) {
    EXPECT_EQ(b_int[i], round_shift(a_int[i], bit));
  }
}

TEST(Txfm1dSse2Test, btf_32_sse2_type0) {
  int bit = 2;
  int w0 = 3;
  int w1 = -7;
  M128I in0 = _mm_setr_epi32(5, 2, 2, 2);
  M128I in1 = _mm_setr_epi32(1, 4, -3, 4);
  M128I out0;
  M128I out1;
  int* in0_int = (int*)&in0;
  int* in1_int = (int*)&in1;
  int* out0_int = (int*)&out0;
  int* out1_int = (int*)&out1;

  btf_32_sse2_type0(w0, w1, in0, in1, out0, out1, bit);

  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(out0_int[i], round_shift(in0_int[i] * w0 + in1_int[i] * w1, bit));
    EXPECT_EQ(out1_int[i], round_shift(in0_int[i] * w1 - in1_int[i] * w0, bit));
  }
}
TEST(Txfm1dSse2Test, btf_32_sse2_type1) {
  int bit = 2;
  int w0 = 3;
  int w1 = -7;
  M128I in0 = _mm_setr_epi32(5, 2, 2, 2);
  M128I in1 = _mm_setr_epi32(1, 4, -3, 4);
  M128I out0;
  M128I out1;
  int* in0_int = (int*)&in0;
  int* in1_int = (int*)&in1;
  int* out0_int = (int*)&out0;
  int* out1_int = (int*)&out1;

  btf_32_sse2_type1(w0, w1, in0, in1, out0, out1, bit);

  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(out0_int[i], round_shift(in0_int[i] * w0 + in1_int[i] * w1, bit));
    EXPECT_EQ(out1_int[i],
              round_shift(-in0_int[i] * w1 + in1_int[i] * w0, bit));
  }
}

TEST(Txfm1dSse2Test, transpose_32_4x4) {
  int32_t input[16] = {7, 7, 7, 7, -2, -2, -2, -2, 3, 3, 3, 3, 1, 1, 1, 1};
  int32_t output[16];
  transpose_32_4x4(1, (M128I*)input, (M128I*)output);
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      EXPECT_EQ(input[r * 4 + c], output[c * 4 + r]);
    }
  }
}

TEST(Txfm1dSse2Test, transpose_32) {
  int32_t input[64];
  for (int i = 0; i < 64; i++) input[i] = i;
  int32_t output[64];

  transpose_32(8, (M128I*)input, (M128I*)output);

  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      EXPECT_EQ(input[r * 8 + c], output[c * 8 + r]);
    }
  }
}

typedef void (*TXFM_FUNC)(const int32_t* input, int32_t* output,
                          const int8_t* cos_bit, const int8_t* stage_range);

typedef void (*TXFM_FUNC_SSE2)(const M128I* input, M128I* output,
                               const int8_t* cos_bit,
                               const int8_t* stage_range);

TEST(Txfm1dSse2Test, Accuracy) {
  int32_t input[1024];
  int32_t input_sse2[1024];
  int32_t output[1024];
  int32_t output_sse2[1024];

  TXFM_FUNC txfm_func_list[] = {
      vp10_fdct4_new,  vp10_fdct8_new,  vp10_fdct16_new,  vp10_fdct32_new,
      vp10_fadst4_new, vp10_fadst8_new, vp10_fadst16_new, vp10_fadst32_new,
  };

  TXFM_FUNC_SSE2 txfm_func_sse2_list[] = {
      vp10_fdct4_new_sse2,   vp10_fdct8_new_sse2,   vp10_fdct16_new_sse2,
      vp10_fdct32_new_sse2,  vp10_fadst4_new_sse2,  vp10_fadst8_new_sse2,
      vp10_fadst16_new_sse2, vp10_fadst32_new_sse2,
  };

  int txfm_num = 8;
  int txfm_size_list[] = {
      4, 8, 16, 32, 4, 8, 16, 32,
  };

  for (int i = 0; i < txfm_num; i++) {
    int txfm_size = txfm_size_list[i];
    TXFM_FUNC txfm_func = txfm_func_list[i];
    TXFM_FUNC_SSE2 txfm_func_sse2 = txfm_func_sse2_list[i];
    // 12 is the maximum stage number of all transform
    int8_t cos_bit[12] = {14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14};
    int8_t stage_range[12] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};

    for (int row = 0; row < txfm_size; row++) {
      for (int col = 0; col < txfm_size; col++) {
        input_sse2[row * txfm_size + col] = rand() % (1 << 10);
        input[col * txfm_size + row] = input_sse2[row * txfm_size + col];
      }
    }

    for (int row = 0; row < txfm_size; row++) {
      txfm_func(input + txfm_size * row, output + txfm_size * row, cos_bit,
                stage_range);
    }
    txfm_func_sse2((M128I*)input_sse2, (M128I*)output_sse2, cos_bit,
                   stage_range);

    for (int r = 0; r < txfm_size; r++) {
      for (int c = 0; c < txfm_size; c++) {
        EXPECT_EQ(output[c * txfm_size + r], output_sse2[r * txfm_size + c]);
      }
    }
  }
}
}  // namespace
