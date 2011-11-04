extern "C" {
    void vp8_short_idct4x4llm_mmx(short *input, unsigned char *pred_ptr,
                            int pred_stride, unsigned char *dst_ptr,
                            int dst_stride);
}

#include "vp8/common/idctllm_test.h"

namespace
{

INSTANTIATE_TEST_CASE_P(MMX, IDCTTest,
                        ::testing::Values(vp8_short_idct4x4llm_mmx));
INSTANTIATE_TEST_CASE_P(MMX, IDCT16Test,
                        ::testing::Values(vp8_short_idct4x4llm_mmx));

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
