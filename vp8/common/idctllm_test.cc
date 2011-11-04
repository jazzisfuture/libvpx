extern "C" {
    void vp8_short_idct4x4llm_c(short *input, unsigned char *pred_ptr,
                            int pred_stride, unsigned char *dst_ptr,
                            int dst_stride);
}

#include "vpx_config.h"
#include "idctllm_test.h"
namespace
{

INSTANTIATE_TEST_CASE_P(C, IDCTTest,
                        ::testing::Values(vp8_short_idct4x4llm_c));
INSTANTIATE_TEST_CASE_P(C, IDCT16Test,
                        ::testing::Values(vp8_short_idct4x4llm_c));

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
