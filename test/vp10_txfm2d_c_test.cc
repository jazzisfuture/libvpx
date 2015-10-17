#include "vp10/common/vp10_txfm2d_c.h"

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

void reference_dct_1d(double* in, double* out, int size) {
  const double kInvSqrt2 = 0.707106781186547524400844362104;
  for (int k = 0; k < size; k++) {
    out[k] = 0;  // initialize out[k]
    for (int n = 0; n < size; n++) {
      out[k] += in[n] * cos(M_PI * (2 * n + 1) * k / (2 * size));
    }
    if (k == 0) out[k] = out[k] * kInvSqrt2;
  }
}

void reference_adst_1d(double* in, double* out, int size) {
  for (int k = 0; k < size; k++) {
    out[k] = 0;  // initialize out[k]
    for (int n = 0; n < size; n++) {
      out[k] += in[n] * sin(M_PI * (2 * n + 1) * (2 * k + 1) /
                            (4 * size));
    }
  }
}

void reference_hybrid_2d(double* in, double* out, int size, int type0,
                         int type1) {
  double* tempOut = new double[size * size];

  for (int r = 0; r < size; r++) {
    // out ->tempOut
    for (int c = 0; c < size; c++) {
      tempOut[r * size + c] = in[c * size + r];
    }
  }

  // dct each row: in -> out
  for (int r = 0; r < size; r++) {
    if (type0 == TYPE_DCT)
      reference_dct_1d(tempOut + r * size, out + r * size, size);
    else
      reference_adst_1d(tempOut + r * size, out + r * size, size);
  }

  for (int r = 0; r < size; r++) {
    // out ->tempOut
    for (int c = 0; c < size; c++) {
      tempOut[r * size + c] = out[c * size + r];
    }
  }

  for (int r = 0; r < size; r++) {
    if (type1 == TYPE_DCT)
      reference_dct_1d(tempOut + r * size, out + r * size, size);
    else
      reference_adst_1d(tempOut + r * size, out + r * size, size);
  }
  delete[] tempOut;
}

const int txfm_num = 4; // TODO test dct only for now
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

TEST(vp10_txfm2d_c, round_trip) {
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
    printf("round_trip_avg_abs_error: %f\n", avg_abs_error);
    double max_abs_avg_error = 0.002;
    EXPECT_LE(avg_abs_error, max_abs_avg_error);

    delete[] input;
    delete[] ref_input;
    delete[] output;
  }
}

TEST(vp10_txfm2d_c, accuracy) {
  for (int i = 0; i < txfm_num; i++) {
    TXFM_2D_CFG fwd_txfm_cfg = fwd_txfm_cfg_ls[i];
    int txfm_size = fwd_txfm_cfg.txfm_size;
    int sqr_txfm_size = txfm_size * txfm_size;
    int16_t* input = new int16_t[sqr_txfm_size];
    int32_t* output = new int32_t[sqr_txfm_size];
    double* ref_input = new double[sqr_txfm_size];
    double* ref_output = new double[sqr_txfm_size];

    // TODO add amplify_bit to config
    int amplify_bit = fwd_txfm_cfg.shift[0]
                    + fwd_txfm_cfg.shift[1]
                    + fwd_txfm_cfg.shift[2];
    double amplify_factor = amplify_bit >= 0 ? (1 << amplify_bit) : (1.0 / (1 << -amplify_bit));

    int base = (1 << 10);
    srand(0);
    int count = 5000;
    double avg_abs_error = 0;
    for (int ci = 0; ci < count; ci++) {
      for (int ni = 0; ni < sqr_txfm_size; ++ni) {
        input[ni] = rand() % base;
        ref_input[ni] = static_cast<double>(input[ni]);
        output[ni] = 0;
        ref_output[ni] = 0;
      }

      vp10_fdct_2d_c(input, output, txfm_size, &fwd_txfm_cfg);
      reference_hybrid_2d(ref_input, ref_output, txfm_size, TYPE_DCT, TYPE_DCT); // TODO test dct only for now

      for (int ni = 0; ni < sqr_txfm_size; ++ni) {
        ref_output[ni] *= amplify_factor; 
        //EXPECT_LE(fabs(output[ni] - ref_output[ni])/amplify_factor, 5);
        //printf("%d: %d %f\n", ni, output[ni], ref_output[ni]);
      }
      avg_abs_error += compute_avg_abs_error<int32_t, double>(
          output, ref_output, sqr_txfm_size);
    }

    avg_abs_error /= amplify_factor;
    avg_abs_error /= count;
    // max_abs_avg_error comes from upper bound of
    printf("accuracy_avg_abs_error: %f\n", avg_abs_error);
    //double max_abs_avg_error = 0.002;
    //EXPECT_LE(avg_abs_error, max_abs_avg_error);

    delete[] input;
    delete[] ref_input;
    delete[] output;
  }
}

/*
TEST(vp10_txfm2d_c, extreme) {
  for (int i = 0; i < txfm_num; i++) {
    TXFM_2D_CFG fwd_txfm_cfg = fwd_txfm_cfg_ls[i];
    TXFM_2D_CFG inv_txfm_cfg = inv_txfm_cfg_ls[i];
    int txfm_size = fwd_txfm_cfg.txfm_size;
    int sqr_txfm_size = txfm_size * txfm_size;
    int16_t* input = new int16_t[sqr_txfm_size];
    int16_t* ref_input = new int16_t[sqr_txfm_size];
    int32_t* output = new int32_t[sqr_txfm_size];


    int base = (1 << 10);
    int extreme_input = base - 1;
    srand(0);
    int count = 5000;
    double avg_abs_error = 0;
    for (int ci = 0; ci < count; ci++) {
      for (int ni = 0; ni < sqr_txfm_size; ++ni) {
        if(rand() % 2 == 1)
          input[ni] = extreme_input;
        else
          input[ni] = -extreme_input;
        ref_input[ni] = 0;
      }

      vp10_fdct_2d_c(input, output, txfm_size, &fwd_txfm_cfg);
      vp10_idct_2d_add_c(output, ref_input, txfm_size, &inv_txfm_cfg);

      for (int ni = 0; ni < sqr_txfm_size; ++ni) {
      //  EXPECT_LE(abs(input[ni] - ref_input[ni]), 1);
        //printf("%d\n", input[ni]);
      }
      avg_abs_error += compute_avg_abs_error<int16_t, int16_t>(
          input, ref_input, sqr_txfm_size);
    }

    avg_abs_error /= count;
    // max_abs_avg_error comes from upper bound of
    printf("extreme_avg_abs_error: %f\n", avg_abs_error);
    //double max_abs_avg_error = 0.002;
    //EXPECT_LE(avg_abs_error, max_abs_avg_error);

    delete[] input;
    delete[] ref_input;
    delete[] output;
  }
}
*/
}  // anonymous namespace
