/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>
#include <stdlib.h>
#include <new>

#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/acm_random.h"
#include "test/util.h"
#include "./vpx_config.h"
#include "vpx_dsp/ssim.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/msvc.h"
#include "vpx_scale/yv12config.h"


using libvpx_test::ACMRandom;

namespace {

typedef double (*LBDMetricFunc)(const YV12_BUFFER_CONFIG *source,
                                const YV12_BUFFER_CONFIG *dest);
typedef double (*HBDMetricFunc)(const YV12_BUFFER_CONFIG *source,
                                const YV12_BUFFER_CONFIG *dest,
                                uint32_t bd);

double compute_hbd_fastssim(const YV12_BUFFER_CONFIG *source,
                            const YV12_BUFFER_CONFIG *dest,
                            uint32_t bit_depth) {
  double tempy, tempu, tempv;
  return vpx_calc_hbd_fastssim(source, dest,
                               &tempy, &tempu, &tempv, bit_depth);
}

double compute_fastssim(const YV12_BUFFER_CONFIG *source,
                        const YV12_BUFFER_CONFIG *dest) {
  double tempy, tempu, tempv;
  return vpx_calc_fastssim(source, dest,
                           &tempy, &tempu, &tempv);
}

double compute_hbd_vpxssim(const YV12_BUFFER_CONFIG *source,
                           const YV12_BUFFER_CONFIG *dest,
                            uint32_t bit_depth) {
  double ssim, weight;
  ssim = vpx_highbd_calc_ssim(source, dest, &weight, bit_depth);
  return 100 * pow(ssim / weight, 8.0);
}

double compute_vpxssim(const YV12_BUFFER_CONFIG *source,
  const YV12_BUFFER_CONFIG *dest) {
  double ssim, weight;
  ssim = vpx_calc_ssim(source, dest, &weight);
  return 100 * pow(ssim / weight, 8.0);
}

class HBDMetricsTestBase {
 public:
  virtual ~HBDMetricsTestBase() {}

 protected:
  void RunAccuracyCheck() {
    const int width = 1920;
    const int height = 1080;
    int i = 0;
    const uint8_t kPixFiller = 128;
    YV12_BUFFER_CONFIG lbd_src, lbd_dst;
    YV12_BUFFER_CONFIG hbd_src, hbd_dst;
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    double lbd_db, hbd_db;

    memset(&lbd_src, 0, sizeof(lbd_src));
    memset(&lbd_dst, 0, sizeof(lbd_dst));
    memset(&hbd_src, 0, sizeof(hbd_src));
    memset(&hbd_dst, 0, sizeof(hbd_dst));

    vpx_alloc_frame_buffer(&lbd_src, width, height, 1, 1, 0, 32, 16);
    vpx_alloc_frame_buffer(&lbd_dst, width, height, 1, 1, 0, 32, 16);
    vpx_alloc_frame_buffer(&hbd_src, width, height, 1, 1, 1, 32, 16);
    vpx_alloc_frame_buffer(&hbd_dst, width, height, 1, 1, 1, 32, 16);

    memset(lbd_src.buffer_alloc, kPixFiller, lbd_src.frame_size);
    while (i < lbd_src.frame_size) {
      // create some distortion for dst buffer
      lbd_dst.buffer_alloc[i] = kPixFiller + (rnd.Rand8() >> 6) - 2;
      ((uint16_t*)(hbd_src.buffer_alloc))[i] =
          lbd_src.buffer_alloc[i] << (bit_depth_ - 8);
      ((uint16_t*)(hbd_dst.buffer_alloc))[i] =
          lbd_dst.buffer_alloc[i] << (bit_depth_ - 8);
      i++;
    }

    lbd_db = lbd_metric_(&lbd_src, &lbd_dst);
    hbd_db = hbd_metric_(&hbd_src, &hbd_dst, bit_depth_);

    vpx_free_frame_buffer(&lbd_src);
    vpx_free_frame_buffer(&lbd_dst);
    vpx_free_frame_buffer(&hbd_src);
    vpx_free_frame_buffer(&hbd_dst);


    // ssim constants c1 and c2 are not perfectly scaled from 8 bit to 10
    // and 12bits, therefor the dB numbers can be off by a small amount.
    EXPECT_LE(fabs(lbd_db - hbd_db) , 0.03);
  }

  int bit_depth_;
  LBDMetricFunc lbd_metric_;
  HBDMetricFunc hbd_metric_;
};

typedef std::tr1::tuple<LBDMetricFunc, HBDMetricFunc, int> MetricTestTParam;
class HBDMetricsTest
    : public HBDMetricsTestBase,
      public ::testing::TestWithParam<MetricTestTParam> {
 public:
  virtual void SetUp() {
    lbd_metric_ = GET_PARAM(0);
    hbd_metric_ = GET_PARAM(1);
    bit_depth_ = GET_PARAM(2);
  }
  virtual void TearDown() {}
};

TEST_P(HBDMetricsTest, RunAccuracyCheck) {
  RunAccuracyCheck();
}

INSTANTIATE_TEST_CASE_P(
    VPXSSIM, HBDMetricsTest,
    ::testing::Values(
        MetricTestTParam(&compute_vpxssim, &compute_hbd_vpxssim, 10),
        MetricTestTParam(&compute_vpxssim, &compute_hbd_vpxssim, 12)));

INSTANTIATE_TEST_CASE_P(
    FASTSSIM, HBDMetricsTest,
    ::testing::Values(
        MetricTestTParam(&compute_fastssim, &compute_hbd_fastssim, 10),
        MetricTestTParam(&compute_fastssim, &compute_hbd_fastssim, 12)));
}  // namespace

