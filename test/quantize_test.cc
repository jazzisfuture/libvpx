/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "./vp8_rtcd.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

#include "vpx/vpx_integer.h"
#include "vp8/common/blockd.h"
#include "vp8/common/onyx.h"
#include "vp8/encoder/block.h"
#include "vp8/encoder/onyx_int.h"
#include "vp8/encoder/quantize.h"
#include "vpx_mem/vpx_mem.h"


namespace {

typedef void (*VP8Quantize)(BLOCK *b, BLOCKD *d);
typedef void (*VP8QuantizePair)(BLOCK *b0, BLOCK *b1,
                                BLOCKD *d0, BLOCKD *d1);

using libvpx_test::ACMRandom;

// Create and populate a VP8_COMP instance which has a complete set of
// quantization inputs as well as a second MACROBLOCKD for output.
class QuantizeTestBase {
 public:
  virtual ~QuantizeTestBase() {
    vp8_remove_compressor(&vp8_comp_);
    vp8_comp_ = NULL;
    vpx_free(macroblockd_dst_);
    macroblockd_dst_ = NULL;
    libvpx_test::ClearSystemState();
  }

 protected:
  void SetupCompressor() {
    rnd_.Reset(ACMRandom::DeterministicSeed());

    // The full configuration is necessary to generate the quantization tables.
    VP8_CONFIG *vp8_config;
    vp8_config = (VP8_CONFIG *)vpx_memalign(32, sizeof(*vp8_config));
    vpx_memset(vp8_config, 0, sizeof(*vp8_config));

    vp8_comp_ = vp8_create_compressor(vp8_config);

    // Set the tables based on a quantizer of 0.
    vp8_set_quantizer(vp8_comp_, 0);

    // Set up all the block/blockd pointers for the mb in vp8_comp_.
    vp8cx_frame_init_quantizer(vp8_comp_);

    // Copy macroblockd from the reference to get pre-set-up dequant values.
    macroblockd_dst_ = (MACROBLOCKD *)vpx_memalign(32, sizeof(*macroblockd_dst_));
    vpx_memcpy(macroblockd_dst_, &vp8_comp_->mb.e_mbd, sizeof(*macroblockd_dst_));
    // Fix block pointers - currently they point to the blocks in the reference
    // structure.
    vp8_setup_block_dptrs(macroblockd_dst_);

  }

  void UpdateQuantizer(int q) {
    vp8_set_quantizer(vp8_comp_, q);

    vpx_memcpy(macroblockd_dst_, &vp8_comp_->mb.e_mbd, sizeof(*macroblockd_dst_));
    vp8_setup_block_dptrs(macroblockd_dst_);
  }

  void FillCoeffConstant(int16_t c) {
    // The coeff struct has 400 elements: vp8/encoder/block.h
    for (int i = 0; i < 400; ++i) {
      vp8_comp_->mb.coeff[i] = c;
    }
  }

  void FillCoeffRandom() {
    // The coeff struct has 400 elements: vp8/encoder/block.h
    for (int i = 0; i < 400; ++i) {
      vp8_comp_->mb.coeff[i] = rnd_.Rand8();
    }
  }

  void CheckOutput() {
    EXPECT_EQ(0, memcmp(vp8_comp_->mb.e_mbd.qcoeff, macroblockd_dst_->qcoeff, sizeof(short) * 400)) << "qcoeff mismatch";
    EXPECT_EQ(0, memcmp(vp8_comp_->mb.e_mbd.dqcoeff, macroblockd_dst_->dqcoeff, sizeof(short) * 400)) << "dqcoeff mismatch";
    EXPECT_EQ(0, memcmp(vp8_comp_->mb.e_mbd.eobs, macroblockd_dst_->eobs, sizeof(char) * 25)) << "eobs mismatch";
  }

  ACMRandom rnd_;
  VP8_COMP *vp8_comp_;
  MACROBLOCKD *macroblockd_dst_;

};

class QuantizeTestFast
    : public QuantizeTestBase,
      public ::testing::TestWithParam<VP8Quantize> {
 protected:
  void SetFunctionPointers() {
    SetupCompressor();
    ASMQuant_ = GetParam();
    CQuant_ = vp8_fast_quantize_b_c;
  }

  VP8Quantize ASMQuant_;
  VP8Quantize CQuant_;
};

class QuantizeTestRegular
    : public QuantizeTestBase,
      public ::testing::TestWithParam<VP8Quantize> {
 protected:
  void SetFunctionPointers() {
    SetupCompressor();
    ASMQuant_ = GetParam();
    CQuant_ = vp8_regular_quantize_b_c;
  }

  VP8Quantize ASMQuant_;
  VP8Quantize CQuant_;
};

class QuantizeTestFastPair
    : public QuantizeTestBase,
      public ::testing::TestWithParam<VP8QuantizePair> {
 protected:
  void SetFunctionPointers() {
    SetupCompressor();
    ASMQuantPair_ = GetParam();
    CQuantPair_ = vp8_fast_quantize_b_pair_c;
  }

  VP8QuantizePair ASMQuantPair_;
  VP8QuantizePair CQuantPair_;
};

// Fast Quantize
TEST_P(QuantizeTestFast, TestZeroInput) {
  SetFunctionPointers();
  FillCoeffConstant(0);

  for (int i = 0; i < 24; ++i) {
    ASM_REGISTER_STATE_CHECK(CQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]));
    ASM_REGISTER_STATE_CHECK(ASMQuant_(&vp8_comp_->mb.block[i], &macroblockd_dst_->block[i]));
  }
  CheckOutput();
}

TEST_P(QuantizeTestFast, TestRandomInput) {
  SetFunctionPointers();
  FillCoeffRandom();

  for (int i = 0; i < 24; ++i) {
    ASM_REGISTER_STATE_CHECK(CQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]));
    ASM_REGISTER_STATE_CHECK(ASMQuant_(&vp8_comp_->mb.block[i], &macroblockd_dst_->block[i]));
  }
  CheckOutput();
}


TEST_P(QuantizeTestFast, TestMultipleQ) {
  SetFunctionPointers();

  for (int q = 0; q < QINDEX_RANGE; ++q) {
    UpdateQuantizer(q);

    FillCoeffRandom();

    for (int i = 0; i < 24; ++i) {
      ASM_REGISTER_STATE_CHECK(CQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]));
      ASM_REGISTER_STATE_CHECK(ASMQuant_(&vp8_comp_->mb.block[i], &macroblockd_dst_->block[i]));
    }
    CheckOutput();
  }
}

TEST_P(QuantizeTestFast, SpeedTest) {
  SetFunctionPointers();
  FillCoeffRandom();

  for (int j = 0; j < 1000000; j++)
    for (int i = 0; i < 24; ++i)
      ASMQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]);
}

// Regular Quantize
TEST_P(QuantizeTestRegular, TestZeroInput) {
  SetFunctionPointers();
  FillCoeffConstant(0);

  for (int i = 0; i < 24; ++i) {
    ASM_REGISTER_STATE_CHECK(CQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]));
    ASM_REGISTER_STATE_CHECK(ASMQuant_(&vp8_comp_->mb.block[i], &macroblockd_dst_->block[i]));
  }
  CheckOutput();
}

TEST_P(QuantizeTestRegular, TestRandomInput) {
  SetFunctionPointers();
  FillCoeffRandom();

  for (int i = 0; i < 24; ++i) {
    ASM_REGISTER_STATE_CHECK(CQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]));
    ASM_REGISTER_STATE_CHECK(ASMQuant_(&vp8_comp_->mb.block[i], &macroblockd_dst_->block[i]));
  }
  CheckOutput();
}


TEST_P(QuantizeTestRegular, TestMultipleQ) {
  SetFunctionPointers();

  for (int q = 0; q < QINDEX_RANGE; ++q) {
    UpdateQuantizer(q);

    FillCoeffRandom();

    for (int i = 0; i < 24; ++i) {
      ASM_REGISTER_STATE_CHECK(CQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]));
      ASM_REGISTER_STATE_CHECK(ASMQuant_(&vp8_comp_->mb.block[i], &macroblockd_dst_->block[i]));
    }
    CheckOutput();
  }
}

TEST_P(QuantizeTestRegular, SpeedTest) {
  SetFunctionPointers();
  FillCoeffRandom();

  for (int j = 0; j < 100000; j++)
    for (int i = 0; i < 24; ++i)
      ASMQuant_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.e_mbd.block[i]);
}

// No implementations of Regular Quantize Pair.

// Fast Quantize Pair.
TEST_P(QuantizeTestFastPair, TestZeroInput) {
  SetFunctionPointers();
  FillCoeffConstant(0);

  for (int i = 0; i < 24; i += 2) {
    ASM_REGISTER_STATE_CHECK(CQuantPair_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.block[i+1],
                                         &vp8_comp_->mb.e_mbd.block[i], &vp8_comp_->mb.e_mbd.block[i+1]));
    ASM_REGISTER_STATE_CHECK(ASMQuantPair_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.block[i+1],
                                           &macroblockd_dst_->block[i], &macroblockd_dst_->block[i+1]));
  }
  CheckOutput();
}

TEST_P(QuantizeTestFastPair, TestRandomInput) {
  SetFunctionPointers();
  FillCoeffRandom();

  for (int i = 0; i < 24; i += 2) {
    ASM_REGISTER_STATE_CHECK(CQuantPair_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.block[i+1],
                                         &vp8_comp_->mb.e_mbd.block[i], &vp8_comp_->mb.e_mbd.block[i+1]));
    ASM_REGISTER_STATE_CHECK(ASMQuantPair_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.block[i+1],
                                           &macroblockd_dst_->block[i], &macroblockd_dst_->block[i+1]));
  }
  CheckOutput();
}

TEST_P(QuantizeTestFastPair, TestMultipleQ) {
  SetFunctionPointers();

  for (int q = 0; q < QINDEX_RANGE; ++q) {
    UpdateQuantizer(q);

    FillCoeffRandom();

    for (int i = 0; i < 24; i += 2) {
      ASM_REGISTER_STATE_CHECK(CQuantPair_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.block[i+1],
                                           &vp8_comp_->mb.e_mbd.block[i], &vp8_comp_->mb.e_mbd.block[i+1]));
      ASM_REGISTER_STATE_CHECK(ASMQuantPair_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.block[i+1],
                                             &macroblockd_dst_->block[i], &macroblockd_dst_->block[i+1]));
    }
    CheckOutput();
  }
}

TEST_P(QuantizeTestFastPair, SpeedTest) {
  SetFunctionPointers();
  FillCoeffRandom();

  for (int j = 0; j < 1000000; j++)
    for (int i = 0; i < 24; i += 2)
      ASM_REGISTER_STATE_CHECK(ASMQuantPair_(&vp8_comp_->mb.block[i], &vp8_comp_->mb.block[i+1],
                                             &macroblockd_dst_->block[i], &macroblockd_dst_->block[i+1]));
}

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, QuantizeTestFast,
                        ::testing::Values(vp8_regular_quantize_b_sse2));
INSTANTIATE_TEST_CASE_P(SSE2, QuantizeTestRegular,
                        ::testing::Values(vp8_regular_quantize_b_sse2));
#endif  // HAVE_SSE2

#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, QuantizeTestFast,
                        ::testing::Values(vp8_fast_quantize_b_ssse3));
#endif  // HAVE_SSSE3

#if HAVE_SSE4_1
INSTANTIATE_TEST_CASE_P(SSE4_1, QuantizeTestRegular,
                        ::testing::Values(vp8_regular_quantize_b_sse4_1));
#endif  // HAVE_SSE4_1

#if HAVE_MEDIA
INSTANTIATE_TEST_CASE_P(MEDIA, QuantizeTestFast,
                        ::testing::Values(vp8_fast_quantize_b_armv6));
#endif  // HAVE_MEDIA

#if HAVE_NEON_ASM
INSTANTIATE_TEST_CASE_P(NEON, QuantizeTestFast,
                        ::testing::Values(vp8_fast_quantize_b_neon));
INSTANTIATE_TEST_CASE_P(NEON, QuantizeTestFastPair,
                        ::testing::Values(vp8_fast_quantize_b_pair_neon));
#endif  // HAVE_NEON_ASM
}  // namespace
