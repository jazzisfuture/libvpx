/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <string.h>  // memset

// Pack the constants. With ssse3, this allows interleaving the source elements
// and pmaddubsw will add and multiply in one fell swoop.
static const uint16_t bilinear_filters_ssse3[8] = {
  // Intended combinations:
  //   a   14, 2   12, 4   10, 6   50/50   6, 10   4, 12,  2, 14
  0x0000, 0x0e02, 0x0c04, 0x0a06, 0x0000, 0x060a, 0x040c, 0x020e
};

// Given a half (or less) full pair of registers, calculate
// x = a * filter[0] + b * filter[1] + 8 >> 4
static __m128i filter_8(const __m128i a_u8, const __m128i an_u8,
                        const __m128i filter) {
  const __m128i round = _mm_set1_epi16(1 << (4 - 1));
#if (SUBPEL_OPT == 2)
  const __m128i zero = _mm_setzero_si128();
  // Given that filter[n][0] = 16 - n * 2 and
  //            filter[n][1] = n * 2
  // convert the equation from x = (16 - n)a + nb + 8 >> 4 to
  //                           x = a + (n(b - a) + 8 >> 4)
  // This makes it so we only need to store 'n', which was the second element in
  // the table, freeing up precious registers. Especially precious because
  // intrinsics give us much more limited control over their use. It also trades
  // a multiply for a subtraction.
  const __m128i a_u16 = _mm_unpacklo_epi8(a_u8, zero);
  const __m128i an_u16 = _mm_unpacklo_epi8(an_u8, zero);

  // a[j + 1] - a[j]
  const __m128i diff = _mm_sub_epi16(an_u16, a_u16);

  // (filter * 2) * (a[j + 1] - a[j])
  const __m128i mul_filter = _mm_mullo_epi16(filter, diff);

  // (filter * 2) * (a[j + 1] - a[j]) + 8
  const __m128i sum = _mm_add_epi16(mul_filter, round);

  // ((filter * 2) * (a[j + 1] - a[j]) + 8) >> 4
  const __m128i shifted = _mm_srai_epi16(sum, 4);

  // a[j] + ((filter * 2) * (a[j + 1] - a[j]) + 8) >> 4
  const __m128i result = _mm_add_epi16(a_u16, shifted);
#elif (SUBPEL_OPT == 3)
  // Packing a_u8 *second* puts the values *first*.
  const __m128i a_an_u16 = _mm_unpacklo_epi8(an_u8, a_u8);

  const __m128i sum = _mm_maddubs_epi16(a_an_u16, filter);
  const __m128i sum_round = _mm_add_epi16(sum, round);

  const __m128i result = _mm_srli_epi16(sum_round, 4);
#endif
  return _mm_packus_epi16(result, result);
}

// Given a full pair of registers, calculate
// x = a * filter[0] + b * filter[1] + 8 >> 4
static __m128i filter_16(const __m128i a_u8, const __m128i an_u8,
                         const __m128i filter) {
  const __m128i round = _mm_set1_epi16(1 << (4 - 1));
#if (SUBPEL_OPT == 2)
  const __m128i zero = _mm_setzero_si128();
  const __m128i a0_u16 = _mm_unpacklo_epi8(a_u8, zero);
  const __m128i a1_u16 = _mm_unpackhi_epi8(a_u8, zero);
  const __m128i an0_u16 = _mm_unpacklo_epi8(an_u8, zero);
  const __m128i an1_u16 = _mm_unpackhi_epi8(an_u8, zero);

  const __m128i diff0 = _mm_sub_epi16(an0_u16, a0_u16);
  const __m128i diff1 = _mm_sub_epi16(an1_u16, a1_u16);

  const __m128i mul_filter0 = _mm_mullo_epi16(filter, diff0);
  const __m128i mul_filter1 = _mm_mullo_epi16(filter, diff1);

  const __m128i sum0 = _mm_add_epi16(mul_filter0, round);
  const __m128i sum1 = _mm_add_epi16(mul_filter1, round);

  const __m128i shifted0 = _mm_srai_epi16(sum0, 4);
  const __m128i shifted1 = _mm_srai_epi16(sum1, 4);

  const __m128i result0 = _mm_add_epi16(a0_u16, shifted0);
  const __m128i result1 = _mm_add_epi16(a1_u16, shifted1);
#elif (SUBPEL_OPT == 3)
  const __m128i a_an0_u16 = _mm_unpacklo_epi8(an_u8, a_u8);
  const __m128i a_an1_u16 = _mm_unpackhi_epi8(an_u8, a_u8);

  const __m128i sum0 = _mm_maddubs_epi16(a_an0_u16, filter);
  const __m128i sum1 = _mm_maddubs_epi16(a_an1_u16, filter);

  const __m128i sum_round0 = _mm_add_epi16(sum0, round);
  const __m128i sum_round1 = _mm_add_epi16(sum1, round);

  const __m128i result0 = _mm_srli_epi16(sum_round0, 4);
  const __m128i result1 = _mm_srli_epi16(sum_round1, 4);
#endif
  return _mm_packus_epi16(result0, result1);
}

static void first_pass(const uint8_t *a, unsigned int stride, uint8_t *b,
                       int filter, const unsigned int width,
                       const unsigned int height) {
  if (filter == 0) {
    if (stride == width) {
      memcpy(b, a, width * height);
    } else {
      unsigned int i;
      for (i = 0; i < height + 1; ++i) {
        memcpy(b + i * width, a + i * stride, width);
      }
    }
  } else if (filter == 4) {
    // If filter == 4 it is equivalent to halfsies.
    unsigned int i;
    if (width == 4) {
      for (i = 0; i < height + 1; ++i) {
        const __m128i a_u8 = _mm_cvtsi32_si128(*(const int *)a);
        const __m128i an_u8 = _mm_cvtsi32_si128(*(const int *)(a + 1));

        const __m128i average = _mm_avg_epu8(a_u8, an_u8);

        *(uint32_t *)(b) = _mm_cvtsi128_si32(average);

        a += stride;
        b += width;
      }
    } else if (width == 8) {
      for (i = 0; i < height + 1; ++i) {
        const __m128i a_u8 = _mm_loadl_epi64((const __m128i *)a);
        const __m128i an_u8 = _mm_loadl_epi64((const __m128i *)(a + 1));

        const __m128i average = _mm_avg_epu8(a_u8, an_u8);

        _mm_storel_epi64((__m128i *)b, average);

        a += stride;
        b += width;
      }
    } else {
      for (i = 0; i < height + 1; ++i) {
        const __m128i a_u8 = _mm_loadu_si128((const __m128i *)a);
        const __m128i an_u8 = _mm_loadu_si128((const __m128i *)(a + 1));

        const __m128i average = _mm_avg_epu8(a_u8, an_u8);

        _mm_store_si128((__m128i *)b, average);

        if (width > 16) {
          const __m128i a1_u8 = _mm_loadu_si128((const __m128i *)(a + 16));
          const __m128i an1_u8 = _mm_loadu_si128((const __m128i *)(a + 17));

          const __m128i average1 = _mm_avg_epu8(a1_u8, an1_u8);

          _mm_store_si128((__m128i *)(b + 16), average1);
        }

        if (width == 64) {
          const __m128i a2_u8 = _mm_loadu_si128((const __m128i *)(a + 32));
          const __m128i an2_u8 = _mm_loadu_si128((const __m128i *)(a + 33));
          const __m128i a3_u8 = _mm_loadu_si128((const __m128i *)(a + 48));
          const __m128i an3_u8 = _mm_loadu_si128((const __m128i *)(a + 49));

          const __m128i average2 = _mm_avg_epu8(a2_u8, an2_u8);
          const __m128i average3 = _mm_avg_epu8(a3_u8, an3_u8);

          _mm_store_si128((__m128i *)(b + 32), average2);
          _mm_store_si128((__m128i *)(b + 48), average3);
        }

        a += stride;
        b += width;
      }
    }
  } else {
#if (SUBPEL_OPT == 2)
    const __m128i filter_u16 = _mm_set1_epi16(filter * 2);
#elif (SUBPEL_OPT == 3)
    const __m128i filter_u16 = _mm_set1_epi16(bilinear_filters_ssse3[filter]);
#endif

    unsigned int i;
    if (width < 16) {
      for (i = 0; i < height + 1; ++i) {
        __m128i a_u8;
        __m128i an_u8;
        __m128i narrowed;

        if (width == 4) {
          a_u8 = _mm_cvtsi32_si128(*(const int *)a);
          an_u8 = _mm_cvtsi32_si128(*(const int *)(a + 1));
        } else if (width == 8) {
          a_u8 = _mm_loadl_epi64((const __m128i *)a);
          an_u8 = _mm_loadl_epi64((const __m128i *)(a + 1));
        }

        narrowed = filter_8(a_u8, an_u8, filter_u16);

        if (width == 4) {
          *(uint32_t *)(b) = _mm_cvtsi128_si32(narrowed);
        } else if (width == 8) {
          _mm_storel_epi64((__m128i *)b, narrowed);
        }

        a += stride;
        b += width;
      }
    } else {
      for (i = 0; i < height + 1; ++i) {
        const __m128i a_u8 = _mm_loadu_si128((const __m128i *)a);
        const __m128i an_u8 = _mm_loadu_si128((const __m128i *)(a + 1));

        const __m128i narrowed = filter_16(a_u8, an_u8, filter_u16);

        _mm_storeu_si128((__m128i *)b, narrowed);

        if (width > 16) {
          const __m128i a1_u8 = _mm_loadu_si128((const __m128i *)(a + 16));
          const __m128i an1_u8 = _mm_loadu_si128((const __m128i *)(a + 17));

          const __m128i narrowed1 = filter_16(a1_u8, an1_u8, filter_u16);

          _mm_storeu_si128((__m128i *)(b + 16), narrowed1);
        }

        if (width == 64) {
          const __m128i a2_u8 = _mm_loadu_si128((const __m128i *)(a + 32));
          const __m128i an2_u8 = _mm_loadu_si128((const __m128i *)(a + 33));
          const __m128i a3_u8 = _mm_loadu_si128((const __m128i *)(a + 48));
          const __m128i an3_u8 = _mm_loadu_si128((const __m128i *)(a + 49));

          const __m128i narrowed2 = filter_16(a2_u8, an2_u8, filter_u16);
          const __m128i narrowed3 = filter_16(a3_u8, an3_u8, filter_u16);

          _mm_storeu_si128((__m128i *)(b + 32), narrowed2);
          _mm_storeu_si128((__m128i *)(b + 48), narrowed3);
        }

        a += stride;
        b += width;
      }
    }
  }
}

static void second_pass(uint8_t *a, const int filter, const unsigned int width,
                        const unsigned int height) {
  if (filter == 0) {
  } else if (filter == 4) {
    unsigned int i;
    // if filter == 4, it's equivalent to halfsies.
    if (width < 16) {
      for (i = 0; i < width * height / 16; ++i) {
        const __m128i a_u8 = _mm_load_si128((const __m128i *)(a));
        const __m128i an_u8 = _mm_loadu_si128((const __m128i *)(a + width));

        const __m128i average = _mm_avg_epu8(a_u8, an_u8);

        _mm_store_si128((__m128i *)a, average);
        a += 16;
      }
    } else if (width == 16) {
      __m128i a_u8 = _mm_load_si128((const __m128i *)(a));
      for (i = 0; i < width * height / 16; ++i) {
        __m128i an_u8 = _mm_load_si128((const __m128i *)(a + width));

        const __m128i average = _mm_avg_epu8(a_u8, an_u8);
        a_u8 = an_u8;

        _mm_store_si128((__m128i *)a, average);
        a += width;
      }
    } else if (width == 32) {
      __m128i a0_u8 = _mm_load_si128((const __m128i *)(a));
      __m128i a1_u8 = _mm_load_si128((const __m128i *)(a + 16));
      for (i = 0; i < width * height / 32; ++i) {
        __m128i an0_u8 = _mm_load_si128((const __m128i *)(a + width));
        __m128i an1_u8 = _mm_load_si128((const __m128i *)(a + width + 16));

        const __m128i average0 = _mm_avg_epu8(a0_u8, an0_u8);
        const __m128i average1 = _mm_avg_epu8(a1_u8, an1_u8);
        a0_u8 = an0_u8;
        a1_u8 = an1_u8;

        _mm_store_si128((__m128i *)a, average0);
        _mm_store_si128((__m128i *)(a + 16), average1);
        a += width;
      }
    } else if (width == 64) {
      __m128i a0_u8 = _mm_load_si128((const __m128i *)(a));
      __m128i a1_u8 = _mm_load_si128((const __m128i *)(a + 16));
      __m128i a2_u8 = _mm_load_si128((const __m128i *)(a + 32));
      __m128i a3_u8 = _mm_load_si128((const __m128i *)(a + 48));
      for (i = 0; i < width * height / 64; ++i) {
        __m128i an0_u8 = _mm_load_si128((const __m128i *)(a + width));
        __m128i an1_u8 = _mm_load_si128((const __m128i *)(a + width + 16));
        __m128i an2_u8 = _mm_load_si128((const __m128i *)(a + width + 32));
        __m128i an3_u8 = _mm_load_si128((const __m128i *)(a + width + 48));

        const __m128i average0 = _mm_avg_epu8(a0_u8, an0_u8);
        const __m128i average1 = _mm_avg_epu8(a1_u8, an1_u8);
        const __m128i average2 = _mm_avg_epu8(a2_u8, an2_u8);
        const __m128i average3 = _mm_avg_epu8(a3_u8, an3_u8);
        a0_u8 = an0_u8;
        a1_u8 = an1_u8;
        a2_u8 = an2_u8;
        a3_u8 = an3_u8;

        _mm_store_si128((__m128i *)a, average0);
        _mm_store_si128((__m128i *)(a + 16), average1);
        _mm_store_si128((__m128i *)(a + 32), average2);
        _mm_store_si128((__m128i *)(a + 48), average3);
        a += width;
      }
    }
  } else {
#if (SUBPEL_OPT == 2)
    const __m128i filter_u16 = _mm_set1_epi16(filter * 2);
#elif (SUBPEL_OPT == 3)
    const __m128i filter_u16 = _mm_set1_epi16(bilinear_filters_ssse3[filter]);
#endif

    unsigned int i;
    if (width != 16 && width != 32) {
      for (i = 0; i < width * height / 16; ++i) {
        const __m128i a_u8 = _mm_load_si128((const __m128i *)(a));
        const __m128i an_u8 = _mm_loadu_si128((const __m128i *)(a + width));

        const __m128i narrowed = filter_16(a_u8, an_u8, filter_u16);

        _mm_store_si128((__m128i *)a, narrowed);
        a += 16;
      }
    } else if (width == 16) {
      __m128i a_u8 = _mm_load_si128((const __m128i *)a);

      for (i = 0; i < height; ++i) {
        const __m128i an_u8 = _mm_load_si128((const __m128i *)(a + width));

        const __m128i narrowed = filter_16(a_u8, an_u8, filter_u16);

        _mm_storeu_si128((__m128i *)a, narrowed);

        // Theoretically could store the unpacked values and rotate them here
        // for sse2 but unpack is cheap and registers are scarce.
        a_u8 = an_u8;

        a += width;
      }
    } else if (width == 32) {
      __m128i a0_u8 = _mm_load_si128((const __m128i *)(a));
      __m128i a1_u8 = _mm_load_si128((const __m128i *)(a + 16));

      for (i = 0; i < height; ++i) {
        const __m128i an0_u8 = _mm_load_si128((const __m128i *)(a + width));
        const __m128i an1_u8 =
            _mm_load_si128((const __m128i *)(a + 16 + width));

        const __m128i narrowed0 = filter_16(a0_u8, an0_u8, filter_u16);
        const __m128i narrowed1 = filter_16(a1_u8, an1_u8, filter_u16);

        _mm_storeu_si128((__m128i *)(a), narrowed0);
        _mm_storeu_si128((__m128i *)(a + 16), narrowed1);

        a0_u8 = an0_u8;
        a1_u8 = an1_u8;

        a += width;
      }
    }
  }
}

static void avg_pred(uint8_t *ref, const uint8_t *pred,
                     const unsigned int width, const unsigned int height) {
  unsigned int i;
  for (i = 0; i < width * height / 16; ++i) {
    const __m128i r = _mm_load_si128((const __m128i *)ref);
    const __m128i p = _mm_load_si128((const __m128i *)pred);
    const __m128i avg = _mm_avg_epu8(r, p);
    _mm_store_si128((__m128i *)ref, avg);
    ref += 16;
    pred += 16;
  }
}
