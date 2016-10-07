/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./av1_rtcd.h"
#include "./aom_dsp_rtcd.h"

#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/transform_test_base.h"
#include "test/util.h"
#include "aom_ports/mem.h"

using libaom_test::ACMRandom;

namespace {
typedef void (*IhtFunc)(const tran_low_t *in, uint8_t *out, int stride,
                        int tx_type);
using std::tr1::tuple;
using libaom_test::FhtFunc;
typedef tuple<FhtFunc, IhtFunc, int, aom_bit_depth_t, int> Ht32x32Param;

void fht32x32_ref(const int16_t *in, tran_low_t *out, int stride, int tx_type) {
  av1_fht32x32_c(in, out, stride, tx_type);
}

#if CONFIG_AOM_HIGHBITDEPTH
typedef void (*IHbdHtFunc)(const tran_low_t *in, uint8_t *out, int stride,
                           int tx_type, int bd);
typedef void (*HbdHtFunc)(const int16_t *input, int32_t *output, int stride,
                          int tx_type, int bd);

// Target optimized function, tx_type, bit depth
typedef tuple<HbdHtFunc, int, int> HighbdHt32x32Param;

void highbd_fht32x32_ref(const int16_t *in, int32_t *out, int stride,
                         int tx_type, int bd) {
  av1_fwd_txfm2d_32x32_c(in, out, stride, tx_type, bd);
}
#endif  // CONFIG_AOM_HIGHBITDEPTH

#if HAVE_AVX2
void dummy_inv_txfm(const tran_low_t *in, uint8_t *out, int stride,
                    int tx_type) {
  (void)in;
  (void)out;
  (void)stride;
  (void)tx_type;
}
#endif

class AV1Trans32x32HT : public libaom_test::TransformTestBase,
                        public ::testing::TestWithParam<Ht32x32Param> {
 public:
  virtual ~AV1Trans32x32HT() {}

  virtual void SetUp() {
    fwd_txfm_ = GET_PARAM(0);
    inv_txfm_ = GET_PARAM(1);
    tx_type_ = GET_PARAM(2);
    pitch_ = 32;
    fwd_txfm_ref = fht32x32_ref;
    bit_depth_ = GET_PARAM(3);
    mask_ = (1 << bit_depth_) - 1;
    num_coeffs_ = GET_PARAM(4);
  }
  virtual void TearDown() { libaom_test::ClearSystemState(); }

 protected:
  void RunFwdTxfm(const int16_t *in, tran_low_t *out, int stride) {
    fwd_txfm_(in, out, stride, tx_type_);
  }

  void RunInvTxfm(const tran_low_t *out, uint8_t *dst, int stride) {
    inv_txfm_(out, dst, stride, tx_type_);
  }

  FhtFunc fwd_txfm_;
  IhtFunc inv_txfm_;
};

TEST_P(AV1Trans32x32HT, CoeffCheck) { RunCoeffCheck(); }

#if CONFIG_AOM_HIGHBITDEPTH
class AV1HighbdTrans32x32HT
    : public ::testing::TestWithParam<HighbdHt32x32Param> {
 public:
  virtual ~AV1HighbdTrans32x32HT() {}

  virtual void SetUp() {
    fwd_txfm_ = GET_PARAM(0);
    fwd_txfm_ref_ = highbd_fht32x32_ref;
    tx_type_ = GET_PARAM(1);
    bit_depth_ = GET_PARAM(2);
    mask_ = (1 << bit_depth_) - 1;
    num_coeffs_ = 1024;

    input_ = reinterpret_cast<int16_t *>(
        aom_memalign(32, sizeof(int16_t) * num_coeffs_));
    output_ = reinterpret_cast<int32_t *>(
        aom_memalign(32, sizeof(int32_t) * num_coeffs_));
    output_ref_ = reinterpret_cast<int32_t *>(
        aom_memalign(32, sizeof(int32_t) * num_coeffs_));
  }

  virtual void TearDown() {
    aom_free(input_);
    aom_free(output_);
    aom_free(output_ref_);
    libaom_test::ClearSystemState();
  }

 protected:
  void RunBitexactCheck();

 private:
  HbdHtFunc fwd_txfm_;
  HbdHtFunc fwd_txfm_ref_;
  int tx_type_;
  int bit_depth_;
  int mask_;
  int num_coeffs_;
  int16_t *input_;
  int32_t *output_;
  int32_t *output_ref_;
};

void AV1HighbdTrans32x32HT::RunBitexactCheck() {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  int i, j;
  const int stride = 32;
  const int num_tests = 1000;

  for (i = 0; i < num_tests; ++i) {
    for (j = 0; j < num_coeffs_; ++j) {
      input_[j] = (rnd.Rand16() & mask_) - (rnd.Rand16() & mask_);
    }

    fwd_txfm_ref_(input_, output_ref_, stride, tx_type_, bit_depth_);
    ASM_REGISTER_STATE_CHECK(
        fwd_txfm_(input_, output_, stride, tx_type_, bit_depth_));

    for (j = 0; j < num_coeffs_; ++j) {
      EXPECT_EQ(output_ref_[j], output_[j])
          << "Not bit-exact result at index: " << j << " at test block: " << i;
    }
  }
}

TEST_P(AV1HighbdTrans32x32HT, HighbdCoeffCheck) { RunBitexactCheck(); }
#endif  // CONFIG_AOM_HIGHBITDEPTH

using std::tr1::make_tuple;

#if 0 && HAVE_SSE2
const Ht32x32Param kArrayHt32x32Param_sse2[] = {
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 0, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 1, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 2, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 3, AOM_BITS_8,
             1024),
#if CONFIG_EXT_TX
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 4, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 5, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 6, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 7, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 8, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 10, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 11, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 12, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 13, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 14, AOM_BITS_8,
             1024),
  make_tuple(&av1_fht32x32_sse2, &av1_iht32x32_1024_add_sse2, 15, AOM_BITS_8,
             1024)
#endif  // CONFIG_EXT_TX
};
INSTANTIATE_TEST_CASE_P(SSE2, AV1Trans32x32HT,
                        ::testing::ValuesIn(kArrayHt32x32Param_sse2));
#endif  // HAVE_SSE2

#if HAVE_AVX2
const Ht32x32Param kArrayHt32x32Param_avx2[] = {
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 0, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 1, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 2, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 3, AOM_BITS_8, 1024),
#if CONFIG_EXT_TX
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 4, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 5, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 6, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 7, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 8, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 10, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 11, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 12, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 13, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 14, AOM_BITS_8, 1024),
  make_tuple(&av1_fht32x32_avx2, dummy_inv_txfm, 15, AOM_BITS_8, 1024)
#endif  // CONFIG_EXT_TX
};
INSTANTIATE_TEST_CASE_P(AVX2, AV1Trans32x32HT,
                        ::testing::ValuesIn(kArrayHt32x32Param_avx2));
#endif  // HAVE_AVX2

#if 0 && HAVE_SSE4_1 && CONFIG_AOM_HIGHBITDEPTH
const HighbdHt32x32Param kArrayHBDHt32x32Param_sse4_1[] = {
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 0, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 0, 12),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 1, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 1, 12),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 2, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 2, 12),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 3, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 3, 12),
#if CONFIG_EXT_TX
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 4, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 4, 12),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 5, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 5, 12),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 6, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 6, 12),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 7, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 7, 12),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 8, 10),
  make_tuple(&av1_fwd_txfm2d_32x32_sse4_1, 8, 12),
#endif  // CONFIG_EXT_TX
};
INSTANTIATE_TEST_CASE_P(SSE4_1, AV1HighbdTrans32x32HT,
                        ::testing::ValuesIn(kArrayHBDHt32x32Param_sse4_1));
#endif  // HAVE_SSE4_1 && CONFIG_AOM_HIGHBITDEPTH
#define SPEED_TEST (1)
#define TEST_NUM (200000)
const int txfm_size (32 * 32);
const int txfm_stride = 32;
#if SPEED_TEST
#if CONFIG_EXT_TX
TEST(AV1Trans32x32HTSpeedTest, Old_AVX2_version) {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    const int count_test_block = TEST_NUM;
    int bit_depth = 8;
    int mask = (1 << bit_depth) - 1;
    const int num_coeffs = txfm_size;
    int16_t *input = new int16_t[num_coeffs];
    tran_low_t *output = new tran_low_t[num_coeffs];
    const int stride = txfm_stride;
    //int tx_type;

    for (int j = 0; j < num_coeffs; ++j) {
      input[j] = (rnd.Rand8() & mask) - (rnd.Rand8() & mask);
    }
    for (int i = 0; i < count_test_block; ++i) {
      aom_fdct32x32_avx2(input, output, stride);
    }

    delete[] input;
    delete[] output;
}
#endif  // CONFIG_EXT_TX

#if HAVE_AVX2 && CONFIG_EXT_TX
TEST(AV1Trans32x32HTSpeedTest, AVX2_version) {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    const int count_test_block = TEST_NUM;
    int bit_depth = 8;
    int mask = (1 << bit_depth) - 1;
    const int num_coeffs = txfm_size;
    int16_t *input = new int16_t[num_coeffs];
    tran_low_t *output = reinterpret_cast<tran_low_t *>(
        aom_memalign(16, num_coeffs * sizeof(tran_low_t)));
    const int stride = txfm_stride;
    int tx_type = DCT_DCT;

    for (int j = 0; j < num_coeffs; ++j) {
      input[j] = (rnd.Rand8() & mask) - (rnd.Rand8() & mask);
    }
    for (int i = 0; i < count_test_block; ++i) {
      av1_fht32x32_avx2(input, output, stride, tx_type);
    }

    delete[] input;
    aom_free(output);
}
#endif
#endif  // SPEED_TEST
}  // namespace
