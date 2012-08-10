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
    __m128i acc_diff = _mm_set1_epi8(0);
    const __m128i k_0 = _mm_set1_epi8(0);

    for (r = 0; r < 16; ++r)
    {
        const __m128i k_fe= _mm_set1_epi8(0xfe);
        const __m128i k_4 = _mm_set1_epi8(4);
        const __m128i k_3 = _mm_set1_epi8(3);
        const __m128i k_8 = _mm_set1_epi8(8);
        const __m128i k_7 = _mm_set1_epi8(7);
        const __m128i k_2 = _mm_set1_epi8(2);
        const __m128i k_6 = _mm_set1_epi8(6);

        __m128i pdiff20, pdiff21, pdiff2;
        __m128i ndiff20, ndiff21, ndiff2;
        __m128i pdiff40, pdiff41, pdiff4;
        __m128i ndiff40, ndiff41, ndiff4;
        __m128i pdiff6, ndiff6;
        __m128i padj, nadj;

        /* Calculate absolute differences */
        __m128i v_sig = _mm_loadu_si128((__m128i *)(&sig[0]));
        __m128i v_mc_running_avg_y = _mm_loadu_si128(
                                         (__m128i *)(&mc_running_avg_y[0]));
        __m128i v_running_avg_y;
        __m128i pdiff = _mm_subs_epu8(v_mc_running_avg_y, v_sig);
        __m128i ndiff = _mm_subs_epu8(v_sig, v_mc_running_avg_y);

        pdiff = _mm_and_si128 (pdiff, k_fe);
        ndiff = _mm_and_si128 (ndiff, k_fe);
        pdiff = _mm_srli_epi16 (pdiff, 1);
        ndiff = _mm_srli_epi16 (ndiff, 1);

        /* pdiff2 */
        pdiff20 = _mm_cmpgt_epi8 (pdiff, k_0);
        pdiff21 = _mm_cmpgt_epi8 (k_4, pdiff);
        pdiff2 =  _mm_and_si128 (pdiff20, pdiff21);
        pdiff2 =  _mm_and_si128 (pdiff2, k_2);
        /* ndiff2 */
        ndiff20 = _mm_cmpgt_epi8 (ndiff, k_0);
        ndiff21 = _mm_cmpgt_epi8 (k_4, ndiff);
        ndiff2 =  _mm_and_si128 (ndiff20, ndiff21);
        ndiff2 =  _mm_and_si128 (ndiff2, k_2);

        /* pdiff4 */
        pdiff40 = _mm_cmpgt_epi8 (pdiff, k_3);
        pdiff41 = _mm_cmpgt_epi8 (k_8, pdiff);
        pdiff4 =  _mm_and_si128 (pdiff40, pdiff41);
        pdiff4 =  _mm_and_si128 (pdiff4, k_4);
        /* ndiff4 */
        ndiff40 = _mm_cmpgt_epi8 (ndiff, k_3);
        ndiff41 = _mm_cmpgt_epi8 (k_8, ndiff);
        ndiff4 =  _mm_and_si128 (ndiff40, ndiff41);
        ndiff4 =  _mm_and_si128 (ndiff4, k_4);

        /* pdiff6 */
        pdiff6 = _mm_cmpgt_epi8 (pdiff, k_7);
        pdiff6 =  _mm_and_si128 (pdiff6, k_6);
        /* ndiff6 */
        ndiff6 = _mm_cmpgt_epi8 (ndiff, k_7);
        ndiff6 =  _mm_and_si128 (ndiff6, k_6);

        padj = _mm_or_si128 (pdiff2, pdiff4);
        padj = _mm_or_si128 (padj, pdiff6);
        nadj = _mm_or_si128 (ndiff2, ndiff4);
        nadj = _mm_or_si128 (nadj, ndiff6);

        v_running_avg_y = _mm_adds_epu8 (v_sig, padj);
        v_running_avg_y = _mm_subs_epu8 (v_running_avg_y, nadj);
        _mm_storeu_si128((__m128i *)running_avg_y, v_running_avg_y);

        /* Adjustments <=6, and each element in acc_diff can fit in signed
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
