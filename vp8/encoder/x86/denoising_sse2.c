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

#ifdef __SSE2__
#if ARCH_X86 || ARCH_X86_64
#include <emmintrin.h>
#ifdef _MSC_VER /* visual c++ */
# define ALIGN16_BEG __declspec(align(16))
# define ALIGN16_END
#else /* gcc or icc */
# define ALIGN16_BEG
# define ALIGN16_END __attribute__((aligned(16)))
#endif
#endif
#endif

#ifdef __SSE2__
#if ARCH_X86 || ARCH_X86_64
void denoiser_filter_sse2(YV12_BUFFER_CONFIG *mc_running_avg,
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
    const uint32_t *LUT = get_filter_coeff_LUT(motion_magnitude);
    int r, c;

    for (r = 0; r < 16; ++r)
    {
        // Calculate absolute differences
        ALIGN16_BEG unsigned char ALIGN16_END abs_diff[16];
        __m128i v_sig = _mm_loadu_si128((__m128i *)(&sig[0]));
        __m128i v_mc_running_avg_y = _mm_loadu_si128(
                                         (__m128i *)(&mc_running_avg_y[0]));
        __m128i a_minus_b = _mm_subs_epu8(v_sig, v_mc_running_avg_y);
        __m128i b_minus_a = _mm_subs_epu8(v_mc_running_avg_y, v_sig);
        __m128i v_abs_diff = _mm_adds_epu8(a_minus_b, b_minus_a);
        _mm_store_si128((__m128i *)(&abs_diff[0]), v_abs_diff);

        // Use LUT to get filter coefficients (two 16b value; f and 256-f)
        ALIGN16_BEG uint32_t ALIGN16_END filter_coefficient[16];

        for (c = 0; c < 16; ++c)
        {
            filter_coefficient[c] = LUT[abs_diff[c]];
        }

        // Filtering...
        const __m128i k_zero = _mm_set1_epi16(0);
        const __m128i k_128 = _mm_set1_epi32(128);
        // load filter coefficients (two 16b value; f and 256-f)
        __m128i filter_coefficient_00 = _mm_load_si128(
                                            (__m128i *)(&filter_coefficient[ 0]));
        __m128i filter_coefficient_04 = _mm_load_si128(
                                            (__m128i *)(&filter_coefficient[ 4]));
        __m128i filter_coefficient_08 = _mm_load_si128(
                                            (__m128i *)(&filter_coefficient[ 8]));
        __m128i filter_coefficient_12 = _mm_load_si128(
                                            (__m128i *)(&filter_coefficient[12]));
        // expand sig from 8b to 16b
        __m128i v_sig0 = _mm_unpacklo_epi8(v_sig, k_zero);
        __m128i v_sig1 = _mm_unpackhi_epi8(v_sig, k_zero);
        // expand mc_running_avg_y from 8b to 16b
        __m128i v_mc_running_avg_y0 = _mm_unpacklo_epi8(v_mc_running_avg_y, k_zero);
        __m128i v_mc_running_avg_y1 = _mm_unpackhi_epi8(v_mc_running_avg_y, k_zero);
        // interleave sig and mc_running_avg_y for upcoming multiply-add
        __m128i state0 = _mm_unpacklo_epi16(v_mc_running_avg_y0, v_sig0);
        __m128i state1 = _mm_unpackhi_epi16(v_mc_running_avg_y0, v_sig0);
        __m128i state2 = _mm_unpacklo_epi16(v_mc_running_avg_y1, v_sig1);
        __m128i state3 = _mm_unpackhi_epi16(v_mc_running_avg_y1, v_sig1);
        // blend values
        __m128i res0 = _mm_madd_epi16(filter_coefficient_00, state0);
        __m128i res1 = _mm_madd_epi16(filter_coefficient_04, state1);
        __m128i res2 = _mm_madd_epi16(filter_coefficient_08, state2);
        __m128i res3 = _mm_madd_epi16(filter_coefficient_12, state3);
        res0 = _mm_add_epi32(res0, k_128);
        res1 = _mm_add_epi32(res1, k_128);
        res2 = _mm_add_epi32(res2, k_128);
        res3 = _mm_add_epi32(res3, k_128);
        res0 = _mm_srai_epi32(res0, 8);
        res1 = _mm_srai_epi32(res1, 8);
        res2 = _mm_srai_epi32(res2, 8);
        res3 = _mm_srai_epi32(res3, 8);
        // combine the 32b results into a single 8b vector
        res0 = _mm_packs_epi32(res0, res1);
        res2 = _mm_packs_epi32(res2, res3);
        __m128i v_running_avg_y = _mm_packus_epi16(res0, res2);

        // Depending on the magnitude of the difference between the signal and the
        // filtered version, either replace the signal by the filtered one or
        // update the filter state with the signal when the change in a pixel
        // isn't classified as noise.
        __m128i diff0 = _mm_sub_epi16(v_sig0, res0);
        __m128i diff1 = _mm_sub_epi16(v_sig1, res2);
        __m128i diff0sq = _mm_mullo_epi16(diff0, diff0);
        __m128i diff1sq = _mm_mullo_epi16(diff1, diff1);
        __m128i diff_sq = _mm_packus_epi16(diff0sq, diff1sq);
        const __m128i kNOISE_DIFF2_THRESHOLD = _mm_set1_epi8(NOISE_DIFF2_THRESHOLD);
        __m128i take_running = _mm_cmplt_epi8(diff_sq, kNOISE_DIFF2_THRESHOLD);
        __m128i p0 = _mm_and_si128(take_running, v_running_avg_y);
        __m128i p1 = _mm_andnot_si128(take_running, v_sig);
        __m128i p2 = _mm_or_si128(p0, p1);
        _mm_storeu_si128((__m128i *)(&running_avg_y[0]), p2);
        _mm_storeu_si128((__m128i *)(&sig[0]), p2);

        // Update pointers for next iteration.
        sig += sig_stride;
        mc_running_avg_y += mc_avg_y_stride;
        running_avg_y += avg_y_stride;
    }
}
#endif
#else
void denoiser_filter_sse2(YV12_BUFFER_CONFIG *mc_running_avg,
                          YV12_BUFFER_CONFIG *running_avg,
                          MACROBLOCK *signal, unsigned int motion_magnitude,
                          int y_offset, int uv_offset)
{
    denoiser_filter_c(mc_running_avg, running_avg, signal, motion_magnitude,
                      y_offset, uv_offset);
}
#endif
