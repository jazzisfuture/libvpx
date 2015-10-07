#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vp10_rtcd.h"
#include "./vpx_dsp_rtcd.h"
#include "vp10/common/vp10_fwd_txfm.h"
#include "vp10/common/vp10_inv_txfm.h"
#include "test/acm_random.h"

#include "vp10/common/idct.h"
#include "vp10/common/vp10_dct_idct_1024.c"

using libvpx_test::ACMRandom;

namespace {
const double PI = 3.141592653589793238462643383279502884;
const double kInvSqrt2 = 0.707106781186547524400844362104;

TEST(vp10_round_trip, vp10_round_trip){
  int txfm_size_ = 32;
  //int sqr_txfm_size_ = txfm_size_*txfm_size_;
  const int sqr_txfm_size_ = 1024;

  // dct
  int16_t *dct_input = new int16_t[sqr_txfm_size_];
  tran_low_t *dct_output = new tran_low_t[sqr_txfm_size_];

  // idct
  uint16_t *idct_output = new uint16_t[sqr_txfm_size_];

  //dct_idct
  tran_low_t *dct_idct_input = new tran_low_t[sqr_txfm_size_];
  tran_low_t *dct_idct_output = new tran_low_t[sqr_txfm_size_];

  ACMRandom rnd(ACMRandom::DeterministicSeed());
  int count = 1000;
  for(int bi = 1; bi < 11; bi++){
    double avg_psnr = 0;
    double avg_error = 0;
    double dct_idct_avg_psnr = 0;
    double dct_idct_avg_error = 0;
    for(int ci = 0; ci < count; ci++){
      double sig = 0;
      for(int i = 0; i < sqr_txfm_size_; i++){
        dct_input[i] = (rnd.Rand16() >> (16 - bi));
        sig += dct_input[i]*dct_input[i];
        dct_idct_input[i] = dct_input[i];
        dct_output[i] = 0;
        idct_output[i] = 0;
      }

      vp10_highbd_fdct32x32_c(dct_input, dct_output, txfm_size_);

      vp10_highbd_idct32x32_add(dct_output, CONVERT_TO_BYTEPTR(idct_output), txfm_size_, 1024, 10);
      //vp10_highbd_idct32x32_1024_add_c(dct_output, CONVERT_TO_BYTEPTR(idct_output), txfm_size_, 12);

      vp10_dct_idct_1024(dct_idct_input, dct_idct_output);

      // compare dct_input and idct_output
      double error = 0;
      for(int i = 0; i < sqr_txfm_size_; i++){
        double curr_error = (int16_t)idct_output[i] - dct_input[i];
        error += curr_error*curr_error;
      }

      sig = sqrt(sig);
      error = sqrt(error);
      avg_error += error;
      avg_psnr += error/sig;

      // compare dct_input and dct_idct_output
      double dct_idct_error = 0;
      for(int i = 0; i < sqr_txfm_size_; i++){
        double curr_error = dct_idct_output[i] - dct_input[i];
        dct_idct_error += curr_error*curr_error;
        //if(i < 32)
        //  printf("dct_idct_output: %d dct_idct_input: %d\n", dct_idct_output[i], dct_idct_input[i]);
      }

      dct_idct_error = sqrt(dct_idct_error);
      dct_idct_avg_error += dct_idct_error;
      dct_idct_avg_psnr += dct_idct_error/sig;

    }
    avg_error /= count;
    avg_psnr /= count;
    dct_idct_avg_error /= count;
    dct_idct_avg_psnr /= count;
    printf("(%d, avg_error: %f, dct_idct_avg_error: %f)\n", 
        bi, avg_error, dct_idct_avg_error);
  }
}

}
