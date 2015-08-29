
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "vp10/encoder/dct.c"
#include "test/acm_random.h"

using libvpx_test::ACMRandom;

namespace {
void reference_dct_1d(double *in, double *out, int size){
  const double PI = 3.141592653589793238462643383279502884;
  const double kInvSqrt2 = 0.707106781186547524400844362104;
  for(int k = 0; k < size; k++){
      out[k] = 0; // initialize out[k]
      for(int n = 0; n < size; n++){
        out[k] += in[n] * cos(PI * (2 * n + 1) * k / (2*size));
      }
      if(k==0)
          out[k] = out[k] * kInvSqrt2;
  }
}


TEST(vp10_dct_test, fdct4){
  int size = 4;
  tran_low_t input[4];
  tran_low_t output[4];
  double refInput[4];
  double refOutput[4];

  ACMRandom rnd(ACMRandom::DeterministicSeed());

  for(int r= 0; r< 10000; r++){
    for(int i = 0; i < size; i++){
      input[i] = rnd.Rand8() - rnd.Rand8(); 
      refInput[i] = (double)input[i];
    }

    fdct4(input, output);
    reference_dct_1d(refInput, refOutput, size);

    for(int i = 0; i < size; i++){
      EXPECT_LE(abs(output[i]-(tran_low_t)round(refOutput[i])), 1);
    }
  }

}

TEST(vp10_dct_test, fdct8){
  int size = 8;
  tran_low_t input[8];
  tran_low_t output[8];
  double refInput[8];
  double refOutput[8];

  ACMRandom rnd(ACMRandom::DeterministicSeed());

  for(int r = 0; r < 10000; r++){
    for(int i = 0; i < size; i++){
      input[i] = rnd.Rand8() - rnd.Rand8(); 
      refInput[i] = (double)input[i];
    }

    fdct8(input, output);
    reference_dct_1d(refInput, refOutput, size);

    for(int i = 0; i < size; i++){
      EXPECT_LE(abs(output[i]-(tran_low_t)round(refOutput[i])), 1);
    }
  }

}

TEST(vp10_dct_test, fdct16){
  int size = 16;
  tran_low_t input[16];
  tran_low_t output[16];
  double refInput[16];
  double refOutput[16];

  ACMRandom rnd(ACMRandom::DeterministicSeed());

  for(int r = 0; r < 10000; r++){
    for(int i = 0; i < size; i++){
      input[i] = rnd.Rand8() - rnd.Rand8(); 
      refInput[i] = (double)input[i];
    }

    fdct16(input, output);
    reference_dct_1d(refInput, refOutput, size);

    for(int i = 0; i < size; i++){
      EXPECT_LE(abs(output[i]-(tran_low_t)round(refOutput[i])), 2);
    }
  }

}

TEST(vp10_dct_test, fdct32){
  int size = 32;
  tran_low_t input[32];
  tran_low_t output[32];
  double refInput[32];
  double refOutput[32];

  ACMRandom rnd(ACMRandom::DeterministicSeed());

  for(int r = 0; r < 10000; r++){
    for(int i = 0; i < size; i++){
      input[i] = rnd.Rand8() - rnd.Rand8(); 
      refInput[i] = (double)input[i];
    }

    fdct32(input, output);
    reference_dct_1d(refInput, refOutput, size);

    for(int i = 0; i < size; i++){
      EXPECT_LE(abs(output[i]-(tran_low_t)round(refOutput[i])), 4);
    }
  }

}

}





