/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp8/encoder/denoising.h"

#include "vp8/common/reconinter.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_rtcd.h"

#include <emmintrin.h>

union sum_union {
    __m128i v;
    signed char e[16];
};

int vp8_denoiser_filter_sse2(YV12_BUFFER_CONFIG *mc_running_avg,
                             YV12_BUFFER_CONFIG *running_avg,
                             MACROBLOCK *signal, unsigned int motion_magnitude,
                             int y_offset, int uv_offset)
{
    unsigned char *sig = signal->thismb;
    int sig_stride = 16;
    unsigned char *mc_running_avg_y = mc_running_avg->y_buffer + y_offset;
    int mc_avg_y_stride = mc_running_avg->y_stride;
    unsigned char *running_avg_y = running_avg->y_buffer + y_offset;
    int avg_y_stride = running_avg->y_stride;
    int r;
    __m128i l3, l32, l21;

    __m128i acc_diff = _mm_set1_epi8(0);
    const __m128i k_0 = _mm_set1_epi8(0);

    /* Modify each level's adjustment according to motion_magnitude. */
    if(motion_magnitude <= MOTION_MAGNITUDE_THRESHOLD)
        l3 = _mm_set1_epi8(7);
    else
        l3 = _mm_set1_epi8(6);

    /* Difference between level 3 and level 2 is 2. */
    l32 = _mm_set1_epi8(2);
    /* Difference between level 2 and level 1 is 1. */
    l21 = _mm_set1_epi8(1);

    for (r = 0; r < 16; ++r)
    {
        const __m128i k_fe= _mm_set1_epi8(0xfe);
        const __m128i k_ff= _mm_set1_epi8(0xff);
        const __m128i k_4 = _mm_set1_epi8(4);
        const __m128i k_8 = _mm_set1_epi8(8);
        const __m128i k_2 = _mm_set1_epi8(2);

        __m128i adj, adj0, padj, nadj, mask0, mask1;

        /* Calculate differences */
        __m128i v_sig = _mm_loadu_si128((__m128i *)(&sig[0]));
        __m128i v_mc_running_avg_y = _mm_loadu_si128(
                                         (__m128i *)(&mc_running_avg_y[0]));
        __m128i v_running_avg_y;
        __m128i pdiff = _mm_subs_epu8(v_mc_running_avg_y, v_sig);
        __m128i ndiff = _mm_subs_epu8(v_sig, v_mc_running_avg_y);
        __m128i absdiff, abshdiff, sign0, sign1;

        /* Obtain the sign. */
        sign0 = _mm_avg_epu8 (pdiff, k_0);
        sign0 = _mm_cmpgt_epi8 (sign0, k_0);

        /* Calculate half absolute diff. */
        absdiff = _mm_or_si128 (pdiff, ndiff);
        abshdiff = _mm_and_si128 (absdiff, k_fe);
        abshdiff = _mm_srli_epi16 (abshdiff, 1);

        /* l2 adjustments */
        mask0 = _mm_cmpgt_epi8 (k_8, abshdiff);
        adj0 = _mm_and_si128 (mask0, l32);
        adj = _mm_sub_epi8 (l3, adj0);

        /* l1 adjustments */
        mask0 = _mm_cmpgt_epi8 (k_4, abshdiff);
        adj0 = _mm_and_si128 (mask0, l21);
        adj = _mm_sub_epi8 (adj, adj0);

        /* l0 adjustments */
        mask0 = _mm_cmpgt_epi8 (k_2, abshdiff);
        mask1 = _mm_xor_si128 (mask0, k_ff);
        adj0 = _mm_and_si128 (mask0, absdiff);
        adj = _mm_and_si128 (adj, mask1);
        adj = _mm_or_si128 (adj, adj0);

        /* Restore the sign. */
        padj = _mm_and_si128 (adj, sign0);
        sign1 = _mm_xor_si128 (sign0, k_ff);
        nadj = _mm_and_si128(adj, sign1);

        /* Calculate filtered value. */
        v_running_avg_y = _mm_adds_epu8 (v_sig, padj);
        v_running_avg_y = _mm_subs_epu8 (v_running_avg_y, nadj);
        _mm_storeu_si128((__m128i *)running_avg_y, v_running_avg_y);

        /* Adjustments <=7, and each element in acc_diff can fit in signed
         * char.
         */
        acc_diff = _mm_adds_epi8 (acc_diff, padj);
        acc_diff = _mm_subs_epi8 (acc_diff, nadj);

        /* Update pointers for next iteration. */
        sig += sig_stride;
        mc_running_avg_y += mc_avg_y_stride;
        running_avg_y += avg_y_stride;
    }

    {
        /* Compute the sum of all pixel differences of this MB. */
        union sum_union s;
        int sum_diff = 0;
        s.v = acc_diff;
        sum_diff = s.e[0] + s.e[1] + s.e[2] + s.e[3] + s.e[4] + s.e[5]
                 + s.e[6] + s.e[7] + s.e[8] + s.e[9] + s.e[10] + s.e[11]
                 + s.e[12] + s.e[13] + s.e[14] + s.e[15];

        if (abs(sum_diff) > SUM_DIFF_THRESHOLD)
        {
            return COPY_BLOCK;
        }
    }

    vp8_copy_mem16x16(running_avg->y_buffer + y_offset, avg_y_stride, signal->thismb, sig_stride);
    return FILTER_BLOCK;
}
