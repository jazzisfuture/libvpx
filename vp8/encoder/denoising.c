/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "denoising.h"

#include "vp8/common/reconinter.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_rtcd.h"

#include <emmintrin.h>
#ifdef _MSC_VER /* visual c++ */
# define ALIGN16_BEG __declspec(align(16))
# define ALIGN16_END
#else /* gcc or icc */
# define ALIGN16_BEG
# define ALIGN16_END __attribute__((aligned(16)))
#endif

static const unsigned int NOISE_MOTION_THRESHOLD = 20*20;
static const unsigned int NOISE_DIFF2_THRESHOLD = 75;
// SSE_DIFF_THRESHOLD is selected as ~95% confidence assuming var(noise) ~= 100.
static const unsigned int SSE_DIFF_THRESHOLD = 16*16*20;
static const unsigned int SSE_THRESHOLD = 16*16*40;

static unsigned int denoiser_motion_compensate(YV12_BUFFER_CONFIG* src,
                                               YV12_BUFFER_CONFIG* dst,
                                               MACROBLOCK* x,
                                               unsigned int best_sse,
                                               unsigned int zero_mv_sse,
                                               int recon_yoffset,
                                               int recon_uvoffset)
{
  MACROBLOCKD filter_xd = x->e_mbd;
  int mv_col;
  int mv_row;
  int sse_diff = zero_mv_sse - best_sse;
  // Compensate the running average.
  filter_xd.pre.y_buffer = src->y_buffer + recon_yoffset;
  filter_xd.pre.u_buffer = src->u_buffer + recon_uvoffset;
  filter_xd.pre.v_buffer = src->v_buffer + recon_uvoffset;
  // Write the compensated running average to the destination buffer.
  filter_xd.dst.y_buffer = dst->y_buffer + recon_yoffset;
  filter_xd.dst.u_buffer = dst->u_buffer + recon_uvoffset;
  filter_xd.dst.v_buffer = dst->v_buffer + recon_uvoffset;
  // Use the best MV for the compensation.
  filter_xd.mode_info_context->mbmi.ref_frame = LAST_FRAME;
  filter_xd.mode_info_context->mbmi.mode = filter_xd.best_sse_inter_mode;
  filter_xd.mode_info_context->mbmi.mv = filter_xd.best_sse_mv;
  filter_xd.mode_info_context->mbmi.need_to_clamp_mvs =
      filter_xd.need_to_clamp_best_mvs;
  mv_col = filter_xd.best_sse_mv.as_mv.col;
  mv_row = filter_xd.best_sse_mv.as_mv.row;
  if (filter_xd.mode_info_context->mbmi.mode <= B_PRED ||
      (mv_row*mv_row + mv_col*mv_col <= NOISE_MOTION_THRESHOLD &&
       sse_diff < SSE_DIFF_THRESHOLD))
  {
    // Handle intra blocks as referring to last frame with zero motion and
    // let the absolute pixel difference affect the filter factor.
    // Also consider small amount of motion as being random walk due to noise,
    // if it doesn't mean that we get a much bigger error.
    // Note that any changes to the mode info only affects the denoising.
    filter_xd.mode_info_context->mbmi.ref_frame = LAST_FRAME;
    filter_xd.mode_info_context->mbmi.mode = ZEROMV;
    filter_xd.mode_info_context->mbmi.mv.as_int = 0;
    x->e_mbd.best_sse_inter_mode = ZEROMV;
    x->e_mbd.best_sse_mv.as_int = 0;
    best_sse = zero_mv_sse;
  }
  if (!x->skip)
  {
    vp8_build_inter_predictors_mb(&filter_xd);
  }
  else
  {
    vp8_build_inter16x16_predictors_mb(&filter_xd,
                                       filter_xd.dst.y_buffer,
                                       filter_xd.dst.u_buffer,
                                       filter_xd.dst.v_buffer,
                                       filter_xd.dst.y_stride,
                                       filter_xd.dst.uv_stride);
  }
  return best_sse;
}

// The filtering coefficients used for denoizing are adjusted for static
// blocks, or blocks with very small motion vectors. This is done through
// the motion magnitude parameter.
//
// There are currently 2048 possible mapping from absolute difference to
// filter coefficient depending on the motion magnitude. Each mapping is
// in a LUT table. All these tables are staticly allocated but they are only
// filled on their first use.
//
// Each entry is a pair of 16b values, the coefficient and its complement
// to 256. Each of these value should only be 8b but they are 16b wide to
// avoid slow partial register manipulations.
enum {num_motion_magnitude_adjustments = 2048};
uint32_t filter_coeff_LUT[num_motion_magnitude_adjustments][256];
uint8_t  filter_coeff_LUT_initialized[num_motion_magnitude_adjustments] = {0};


uint32_t *get_filter_coeff_LUT(unsigned int motion_magnitude) {
  unsigned int motion_magnitude_adjustment = motion_magnitude >> 3;
  if (motion_magnitude_adjustment >= num_motion_magnitude_adjustments) {
    motion_magnitude_adjustment = num_motion_magnitude_adjustments - 1;
  }

  uint32_t *LUT = filter_coeff_LUT[motion_magnitude_adjustment];
  if (!filter_coeff_LUT_initialized[motion_magnitude_adjustment]) {
    int absdiff;
    for (absdiff = 0; absdiff < 256; ++absdiff) {
      unsigned int filter_coefficient;
      filter_coefficient = (255 << 8) / (256 + ((absdiff * 330) >> 3));
      filter_coefficient += filter_coefficient /
            (3 + motion_magnitude_adjustment);
      if (filter_coefficient > 255) {
        filter_coefficient = 255;
      }
      uint16_t coeff_pair[2] = {filter_coefficient, 256 - filter_coefficient};
      LUT[absdiff] = *((uint32_t *)coeff_pair);
    }
    filter_coeff_LUT_initialized[motion_magnitude_adjustment] = 1;
  }

  return LUT;
}



void denoiser_filter_c(YV12_BUFFER_CONFIG* mc_running_avg,
                       YV12_BUFFER_CONFIG* running_avg,
                       MACROBLOCK* signal, unsigned int motion_magnitude,
                       int y_offset, int uv_offset) {
  unsigned char* sig = signal->thismb;
  int sig_stride = 16;
  unsigned char* mc_running_avg_y = mc_running_avg->y_buffer + y_offset;
  int mc_avg_y_stride = mc_running_avg->y_stride;
  unsigned char* running_avg_y = running_avg->y_buffer + y_offset;
  int avg_y_stride = running_avg->y_stride;
  const uint32_t *LUT = get_filter_coeff_LUT(motion_magnitude);
  int r, c;

  for (r = 0; r < 16; ++r) {
    // Calculate absolute differences
    unsigned char abs_diff[16];
    for (c = 0; c < 16; ++c) {
      int absdiff = sig[c] - mc_running_avg_y[c];
      absdiff = absdiff > 0 ? absdiff : -absdiff;
      abs_diff[c] = absdiff;
    }

    // Use LUT to get filter coefficients (two 16b value; f and 256-f)
    uint32_t filter_coefficient[16];
    for (c = 0; c < 16; ++c) {
      filter_coefficient[c] = LUT[abs_diff[c]];
    }

    // Filtering...
    for (c = 0; c < 16; ++c) {
      const uint16_t state = (uint16_t)(mc_running_avg_y[c]);
      const uint16_t sample = (uint16_t)(sig[c]);
      const uint16_t *f_coeff = (uint16_t *)(&filter_coefficient[c]);
      running_avg_y[c] = (f_coeff[0] * state + f_coeff[1] * sample + 128) >> 8;
    }

    // Depending on the magnitude of the difference between the signal and the
    // filtered version, either replace the signal by the filtered one or
    // update the filter state with the signal when the change in a pixel
    // isn't classified as noise.
    for (c = 0; c < 16; ++c) {
      const int diff = sig[c] - running_avg_y[c];
      if (diff * diff < NOISE_DIFF2_THRESHOLD) {
        sig[c] = running_avg_y[c];
      } else {
        running_avg_y[c] = sig[c];
      }
    }

    // Update pointers for next iteration.
    sig += sig_stride;
    mc_running_avg_y += mc_avg_y_stride;
    running_avg_y += avg_y_stride;
  }
}

void denoiser_filter_sse2(YV12_BUFFER_CONFIG* mc_running_avg,
                          YV12_BUFFER_CONFIG* running_avg,
                          MACROBLOCK* signal, unsigned int motion_magnitude,
                          int y_offset, int uv_offset) {
  unsigned char* sig = signal->thismb;
  int sig_stride = 16;
  unsigned char* mc_running_avg_y = mc_running_avg->y_buffer + y_offset;
  int mc_avg_y_stride = mc_running_avg->y_stride;
  unsigned char* running_avg_y = running_avg->y_buffer + y_offset;
  int avg_y_stride = running_avg->y_stride;
  const uint32_t *LUT = get_filter_coeff_LUT(motion_magnitude);
  int r, c;

  for (r = 0; r < 16; ++r) {
    // Calculate absolute differences
    ALIGN16_BEG unsigned char ALIGN16_END abs_diff[16];
    __m128i v_sig = _mm_loadu_si128 ((__m128i *)(&sig[0]));
    __m128i v_mc_running_avg_y = _mm_loadu_si128 (
          (__m128i *)(&mc_running_avg_y[0]));
    __m128i a_minus_b = _mm_subs_epu8(v_sig, v_mc_running_avg_y);
    __m128i b_minus_a = _mm_subs_epu8(v_mc_running_avg_y, v_sig);
    __m128i v_abs_diff = _mm_adds_epu8(a_minus_b, b_minus_a);
    _mm_store_si128((__m128i *)(&abs_diff[0]), v_abs_diff);

    // Use LUT to get filter coefficients (two 16b value; f and 256-f)
    ALIGN16_BEG uint32_t ALIGN16_END filter_coefficient[16];
    for (c = 0; c < 16; ++c) {
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
    __m128i res0 = _mm_madd_epi16 (filter_coefficient_00, state0);
    __m128i res1 = _mm_madd_epi16 (filter_coefficient_04, state1);
    __m128i res2 = _mm_madd_epi16 (filter_coefficient_08, state2);
    __m128i res3 = _mm_madd_epi16 (filter_coefficient_12, state3);
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
    __m128i diff0 = _mm_sub_epi16 (v_sig0, res0);
    __m128i diff1 = _mm_sub_epi16 (v_sig1, res2);
    __m128i diff0sq = _mm_mullo_epi16 (diff0, diff0);
    __m128i diff1sq = _mm_mullo_epi16 (diff1, diff1);
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

int vp8_denoiser_allocate(VP8_DENOISER *denoiser, int width, int height)
{
  assert(denoiser);
  denoiser->yv12_running_avg.flags = 0;
  if (vp8_yv12_alloc_frame_buffer(&(denoiser->yv12_running_avg), width,
                                  height, VP8BORDERINPIXELS) < 0)
  {
      vp8_denoiser_free(denoiser);
      return 1;
  }
  denoiser->yv12_mc_running_avg.flags = 0;
  if (vp8_yv12_alloc_frame_buffer(&(denoiser->yv12_mc_running_avg), width,
                                  height, VP8BORDERINPIXELS) < 0)
  {
      vp8_denoiser_free(denoiser);
      return 1;
  }
  vpx_memset(denoiser->yv12_running_avg.buffer_alloc, 0,
             denoiser->yv12_running_avg.frame_size);
  vpx_memset(denoiser->yv12_mc_running_avg.buffer_alloc, 0,
             denoiser->yv12_mc_running_avg.frame_size);
  return 0;
}

void vp8_denoiser_free(VP8_DENOISER *denoiser)
{
  assert(denoiser);
  vp8_yv12_de_alloc_frame_buffer(&denoiser->yv12_running_avg);
  vp8_yv12_de_alloc_frame_buffer(&denoiser->yv12_mc_running_avg);
}

void vp8_denoiser_denoise_mb(VP8_DENOISER *denoiser,
                             MACROBLOCK *x,
                             unsigned int best_sse,
                             unsigned int zero_mv_sse,
                             int recon_yoffset,
                             int recon_uvoffset) {
  int mv_row;
  int mv_col;
  unsigned int motion_magnitude2;
  // Motion compensate the running average.
  best_sse = denoiser_motion_compensate(&denoiser->yv12_running_avg,
                                        &denoiser->yv12_mc_running_avg,
                                        x,
                                        best_sse,
                                        zero_mv_sse,
                                        recon_yoffset,
                                        recon_uvoffset);

  mv_row = x->e_mbd.best_sse_mv.as_mv.row;
  mv_col = x->e_mbd.best_sse_mv.as_mv.col;
  motion_magnitude2 = mv_row*mv_row + mv_col*mv_col;
  if (best_sse > SSE_THRESHOLD ||
      motion_magnitude2 > 8 * NOISE_MOTION_THRESHOLD)
  {
    // No filtering of this block since it differs too much from the predictor,
    // or the motion vector magnitude is considered too big.
    vp8_copy_mem16x16(x->thismb, 16,
                      denoiser->yv12_running_avg.y_buffer + recon_yoffset,
                      denoiser->yv12_running_avg.y_stride);
    return;
  }
  // Filter.
  denoiser_filter(&denoiser->yv12_mc_running_avg,
                  &denoiser->yv12_running_avg,
                  x,
                  motion_magnitude2,
                  recon_yoffset,
                  recon_uvoffset);
}
