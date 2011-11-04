/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
 
 
 #include "third_party/googletest/src/include/gtest/gtest.h"
typedef void (*idct_fn_t)(short *input, unsigned char *pred_ptr,
                            int pred_stride, unsigned char *dst_ptr,
                            int dst_stride);
namespace {
class IDCTTest : public ::testing::TestWithParam<idct_fn_t>
{
  protected:
    virtual void SetUp()
    {
        int i;

        UUT = GetParam();
        memset(input, 0, sizeof(input));
        /* Set up guard blocks */
        for(i=0; i<256; i++)
            output[i] = ((i&0xF)<4)?0:-1;
    }

    idct_fn_t UUT;
    short input[16];
    unsigned char output[256];
};

class IDCT16Test : public IDCTTest {};

TEST_P(IDCTTest, TestGuardBlocks)
{
    int i;

    for(i=0; i<256; i++)
        if((i&0xF) < 4)
            EXPECT_EQ(0, output[i]) << i;
        else
            EXPECT_EQ(255, output[i]);
}

TEST_P(IDCTTest, TestAllZeros)
{
    int i;

    UUT(input, output, 32, output, 32);

    for(i=0; i<256; i++)
        if((i&0xF) < 4)
            EXPECT_EQ(0, output[i]) << "i==" << i;
        else
            EXPECT_EQ(255, output[i]) << "i==" << i;
}

TEST_P(IDCTTest, TestAllOnes)
{
    int i;

    input[0] = 4;
    UUT(input, output, 32, output, 32);

    for(i=0; i<256; i++)
        if((i&0xF) < 4)
            if((i>=0 && i<4) || (i>=32 && i<36)
                || (i>=64 && i<68) || (i>=96 && i<100))
                EXPECT_EQ(1, output[i]) << "i==" << i;
            else
                EXPECT_EQ(0, output[i]) << "i==" << i;
        else
            EXPECT_EQ(255, output[i]) << "i==" << i;
}

TEST_P(IDCT16Test, TestWithData)
{
    int i;

    input[0] = 4;
    UUT(input, output, 32, output, 32);

    for(i=0; i<256; i++)
        if((i&0xF) < 4)
            if((i>=0 && i<4) || (i>=32 && i<36)
                || (i>=64 && i<68) || (i>=96 && i<100))
                EXPECT_EQ(1, output[i]) << "i==" << i;
            else
                EXPECT_EQ(0, output[i]) << "i==" << i;
        else
            EXPECT_EQ(255, output[i]) << "i==" << i;
}
}
