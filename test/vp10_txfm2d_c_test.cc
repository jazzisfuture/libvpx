#include "vp10/common/vp10_txfm2d_c.h"
#include "vp10/common/vp10_txfm2d_cfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

namespace {
template <typename Type1, typename Type2>
double compute_avg_abs_error(const Type1* a, const Type2* b, const int size) {
  double error = 0;
  for (int i = 0; i < size; i++) {
    error += fabs(static_cast<double>(a[i]) - static_cast<double>(b[i]));
  }
  error = error / size;
  return error;
}

TEST(txfm2d_c, round_trip) {
  const int txfm_num = 16;
  TXFM_2D_CFG fwd_txfm_cfg_ls[16] = {
      fwd_txfm_2d_cfg_dct_dct_4,    fwd_txfm_2d_cfg_dct_dct_8,
      fwd_txfm_2d_cfg_dct_dct_16,   fwd_txfm_2d_cfg_dct_dct_32,
      fwd_txfm_2d_cfg_dct_adst_4,   fwd_txfm_2d_cfg_dct_adst_8,
      fwd_txfm_2d_cfg_dct_adst_16,  fwd_txfm_2d_cfg_dct_adst_32,
      fwd_txfm_2d_cfg_adst_adst_4,  fwd_txfm_2d_cfg_adst_adst_8,
      fwd_txfm_2d_cfg_adst_adst_16, fwd_txfm_2d_cfg_adst_adst_32,
      fwd_txfm_2d_cfg_adst_dct_4,   fwd_txfm_2d_cfg_adst_dct_8,
      fwd_txfm_2d_cfg_adst_dct_16,  fwd_txfm_2d_cfg_adst_dct_32};

  TXFM_2D_CFG inv_txfm_cfg_ls[16] = {
      inv_txfm_2d_cfg_dct_dct_4,    inv_txfm_2d_cfg_dct_dct_8,
      inv_txfm_2d_cfg_dct_dct_16,   inv_txfm_2d_cfg_dct_dct_32,
      inv_txfm_2d_cfg_dct_adst_4,   inv_txfm_2d_cfg_dct_adst_8,
      inv_txfm_2d_cfg_dct_adst_16,  inv_txfm_2d_cfg_dct_adst_32,
      inv_txfm_2d_cfg_adst_adst_4,  inv_txfm_2d_cfg_adst_adst_8,
      inv_txfm_2d_cfg_adst_adst_16, inv_txfm_2d_cfg_adst_adst_32,
      inv_txfm_2d_cfg_adst_dct_4,   inv_txfm_2d_cfg_adst_dct_8,
      inv_txfm_2d_cfg_adst_dct_16,  inv_txfm_2d_cfg_adst_dct_32};
  for (int i = 0; i < txfm_num; i++) {
    TXFM_2D_CFG fwd_txfm_cfg = fwd_txfm_cfg_ls[i];
    TXFM_2D_CFG inv_txfm_cfg = inv_txfm_cfg_ls[i];
    int txfm_size = fwd_txfm_cfg.txfm_size;
    int sqr_txfm_size = txfm_size * txfm_size;
    int16_t* input = new int16_t[sqr_txfm_size];
    uint16_t* ref_input = new uint16_t[sqr_txfm_size];
    int32_t* output = new int32_t[sqr_txfm_size];

    int base = (1 << 10);
    srand(0);
    int count = 5000;
    double avg_abs_error = 0;
    for (int ci = 0; ci < count; ci++) {
      for (int ni = 0; ni < sqr_txfm_size; ++ni) {
        input[ni] = rand() % base;
        ref_input[ni] = 0;
      }

      vp10_fdct_2d_c(input, output, txfm_size, &fwd_txfm_cfg);
      vp10_idct_2d_add_c(output, ref_input, txfm_size, &inv_txfm_cfg);

      for (int ni = 0; ni < sqr_txfm_size; ++ni) {
        EXPECT_LE(abs(input[ni] - ref_input[ni]), 1);
      }
      avg_abs_error += compute_avg_abs_error<int16_t, uint16_t>(
          input, ref_input, sqr_txfm_size);
    }

    avg_abs_error /= count;
    // max_abs_avg_error comes from upper bound of
    // printf("avg_abs_error: %f\n", avg_abs_error);
    double max_abs_avg_error = 0.002;
    EXPECT_LE(avg_abs_error, max_abs_avg_error);

    delete[] input;
    delete[] ref_input;
    delete[] output;
  }
}
}  // anonymous namespace
