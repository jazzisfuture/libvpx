/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vp10/common/onyxc_int.h"
#include "vp10/common/restoration.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"

#define BILATERAL_PARAM_PRECISION 16
#define BILATERAL_AMP_RANGE 256
#define BILATERAL_AMP_RANGE_SYM (2 * BILATERAL_AMP_RANGE + 1)

static uint8_t
    bilateral_filter_coeffs_r_kf[BILATERAL_LEVELS_KF][BILATERAL_AMP_RANGE_SYM];
static uint8_t
    bilateral_filter_coeffs_r[BILATERAL_LEVELS][BILATERAL_AMP_RANGE_SYM];
static uint8_t bilateral_filter_coeffs_s_kf[BILATERAL_LEVELS_KF]
                                           [RESTORATION_WIN][RESTORATION_WIN];
static uint8_t bilateral_filter_coeffs_s[BILATERAL_LEVELS][RESTORATION_WIN]
                                        [RESTORATION_WIN];

typedef struct bilateral_params {
  int sigma_x;  // spatial variance x
  int sigma_y;  // spatial variance y
  int sigma_r;  // range variance
} BilateralParamsType;

static BilateralParamsType bilateral_level_to_params_arr[BILATERAL_LEVELS] = {
  // Values are rounded to 1/16 th precision
  { 8, 9, 30 },   { 9, 8, 30 },   { 9, 11, 32 },  { 11, 9, 32 },
  { 14, 14, 32 }, { 18, 18, 36 }, { 24, 24, 40 }, { 32, 32, 40 },
};

static BilateralParamsType
    bilateral_level_to_params_arr_kf[BILATERAL_LEVELS_KF] = {
      // Values are rounded to 1/16 th precision
      { 8, 8, 30 },   { 9, 9, 32 },   { 10, 10, 32 }, { 12, 12, 32 },
      { 14, 14, 32 }, { 18, 18, 36 }, { 24, 24, 40 }, { 30, 30, 44 },
      { 36, 36, 48 }, { 42, 42, 48 }, { 48, 48, 48 }, { 48, 48, 56 },
      { 56, 56, 48 }, { 56, 56, 56 }, { 56, 56, 64 }, { 64, 64, 48 },
    };

typedef void (*restore_func_type)(uint8_t *data8, int width, int height,
                                  int stride, RestorationInternal *rst,
                                  uint8_t *tmpdata8, int tmpstride);
#if CONFIG_VP9_HIGHBITDEPTH
typedef void (*restore_func_highbd_type)(uint8_t *data8, int width, int height,
                                         int stride, RestorationInternal *rst,
                                         uint8_t *tmpdata8, int tmpstride,
                                         int bit_depth);
#endif  // CONFIG_VP9_HIGHBITDEPTH

static INLINE BilateralParamsType vp10_bilateral_level_to_params(int index,
                                                                 int kf) {
  return kf ? bilateral_level_to_params_arr_kf[index]
            : bilateral_level_to_params_arr[index];
}

typedef struct TileParams {
  int width;
  int height;
} TileParams;

static TileParams restoration_tile_sizes[RESTORATION_TILETYPES] = {
  { 64, 64 }, { 128, 128 },
};

int vp10_restoration_ntiles(const struct VP10Common *const cm, int tiletype) {
  int nhtiles, nvtiles;
  const int tile_width =
      (tiletype < 0) ? cm->width : restoration_tile_sizes[tiletype].width;
  const int tile_height =
      (tiletype < 0) ? cm->height : restoration_tile_sizes[tiletype].height;
  nhtiles = (cm->width + (tile_width >> 1)) / tile_width;
  nvtiles = (cm->height + (tile_height >> 1)) / tile_height;
  return (nhtiles * nvtiles);
}

void vp10_restoration_tile_size(int tiletype, int *tile_width,
                                int *tile_height) {
  *tile_width = restoration_tile_sizes[tiletype].width;
  *tile_height = restoration_tile_sizes[tiletype].height;
}

void vp10_loop_restoration_precal() {
  int i;
  for (i = 0; i < BILATERAL_LEVELS_KF; i++) {
    const BilateralParamsType param = vp10_bilateral_level_to_params(i, 1);
    const int sigma_x = param.sigma_x;
    const int sigma_y = param.sigma_y;
    const int sigma_r = param.sigma_r;
    const double sigma_r_d = (double)sigma_r / BILATERAL_PARAM_PRECISION;
    const double sigma_x_d = (double)sigma_x / BILATERAL_PARAM_PRECISION;
    const double sigma_y_d = (double)sigma_y / BILATERAL_PARAM_PRECISION;

    uint8_t *fr = bilateral_filter_coeffs_r_kf[i] + BILATERAL_AMP_RANGE;
    int j, x, y;
    for (j = 0; j <= BILATERAL_AMP_RANGE; j++) {
      fr[j] = (uint8_t)(0.5 +
                        RESTORATION_FILT_STEP *
                            exp(-(j * j) / (2 * sigma_r_d * sigma_r_d)));
      fr[-j] = fr[j];
    }
    for (y = -RESTORATION_HALFWIN; y <= RESTORATION_HALFWIN; y++) {
      for (x = -RESTORATION_HALFWIN; x <= RESTORATION_HALFWIN; x++) {
        bilateral_filter_coeffs_s_kf
            [i][y + RESTORATION_HALFWIN][x + RESTORATION_HALFWIN] =
                (uint8_t)(0.5 +
                          RESTORATION_FILT_STEP *
                              exp(-(x * x) / (2 * sigma_x_d * sigma_x_d) -
                                  (y * y) / (2 * sigma_y_d * sigma_y_d)));
      }
    }
  }
  for (i = 0; i < BILATERAL_LEVELS; i++) {
    const BilateralParamsType param = vp10_bilateral_level_to_params(i, 0);
    const int sigma_x = param.sigma_x;
    const int sigma_y = param.sigma_y;
    const int sigma_r = param.sigma_r;
    const double sigma_r_d = (double)sigma_r / BILATERAL_PARAM_PRECISION;
    const double sigma_x_d = (double)sigma_x / BILATERAL_PARAM_PRECISION;
    const double sigma_y_d = (double)sigma_y / BILATERAL_PARAM_PRECISION;

    uint8_t *fr = bilateral_filter_coeffs_r[i] + BILATERAL_AMP_RANGE;
    int j, x, y;
    for (j = 0; j <= BILATERAL_AMP_RANGE; j++) {
      fr[j] = (uint8_t)(0.5 +
                        RESTORATION_FILT_STEP *
                            exp(-(j * j) / (2 * sigma_r_d * sigma_r_d)));
      fr[-j] = fr[j];
    }
    for (y = -RESTORATION_HALFWIN; y <= RESTORATION_HALFWIN; y++) {
      for (x = -RESTORATION_HALFWIN; x <= RESTORATION_HALFWIN; x++) {
        bilateral_filter_coeffs_s[i][y +
                                     RESTORATION_HALFWIN][x +
                                                          RESTORATION_HALFWIN] =
            (uint8_t)(0.5 +
                      RESTORATION_FILT_STEP *
                          exp(-(x * x) / (2 * sigma_x_d * sigma_x_d) -
                              (y * y) / (2 * sigma_y_d * sigma_y_d)));
      }
    }
  }
}

int vp10_bilateral_level_bits(const VP10_COMMON *const cm) {
  return cm->frame_type == KEY_FRAME ? BILATERAL_LEVEL_BITS_KF
                                     : BILATERAL_LEVEL_BITS;
}

void vp10_loop_restoration_init(struct VP10Common *cm, RestorationInfo *rsi,
                                int kf, const YV12_BUFFER_CONFIG *frame) {
  int i, tile_idx;
  RestorationInternal *rst = &cm->rst_internal;
  rst->restoration_type = rsi->restoration_type;
  rst->subsampling_x = 0;
  rst->subsampling_y = 0;
  if (rsi->restoration_type == RESTORE_BILATERAL) {
    rst->bilateral_tiletype = rsi->bilateral_tiletype;
    rst->bilateral_ntiles = rsi->bilateral_ntiles;
    rst->bilateral_level = rsi->bilateral_level;
    rst->wr_lut =
        (uint8_t **)malloc(sizeof(*rst->wr_lut) * rst->bilateral_ntiles);
    assert(rst->wr_lut != NULL);
    rst->wx_lut = (uint8_t(**)[RESTORATION_WIN])malloc(sizeof(*rst->wx_lut) *
                                                       rst->bilateral_ntiles);
    assert(rst->wx_lut != NULL);
    for (tile_idx = 0; tile_idx < rst->bilateral_ntiles; ++tile_idx) {
      const int level = rsi->bilateral_level[tile_idx];
      if (level >= 0) {
        rst->wr_lut[tile_idx] = kf ? bilateral_filter_coeffs_r_kf[level]
                                   : bilateral_filter_coeffs_r[level];
        rst->wx_lut[tile_idx] = kf ? bilateral_filter_coeffs_s_kf[level]
                                   : bilateral_filter_coeffs_s[level];
      }
    }
  } else if (rsi->restoration_type == RESTORE_WIENER) {
    rst->wiener_tiletype = rsi->wiener_tiletype;
    rst->wiener_ntiles = rsi->wiener_ntiles;
    rst->wiener_process_tile = rsi->wiener_process_tile;
    rst->vfilter = (int(*)[RESTORATION_WIN])malloc(sizeof(*rst->vfilter) *
                                                   rst->wiener_ntiles);
    assert(rst->vfilter != NULL);
    rst->hfilter = (int(*)[RESTORATION_WIN])malloc(sizeof(*rst->hfilter) *
                                                   rst->wiener_ntiles);
    assert(rst->hfilter != NULL);
    for (tile_idx = 0; tile_idx < rst->wiener_ntiles; ++tile_idx) {
      rst->vfilter[tile_idx][RESTORATION_HALFWIN] =
          rst->hfilter[tile_idx][RESTORATION_HALFWIN] = RESTORATION_FILT_STEP;
      for (i = 0; i < RESTORATION_HALFWIN; ++i) {
        rst->vfilter[tile_idx][i] =
            rst->vfilter[tile_idx][RESTORATION_WIN - 1 - i] =
                rsi->vfilter[tile_idx][i];
        rst->hfilter[tile_idx][i] =
            rst->hfilter[tile_idx][RESTORATION_WIN - 1 - i] =
                rsi->hfilter[tile_idx][i];
        rst->vfilter[tile_idx][RESTORATION_HALFWIN] -=
            2 * rsi->vfilter[tile_idx][i];
        rst->hfilter[tile_idx][RESTORATION_HALFWIN] -=
            2 * rsi->hfilter[tile_idx][i];
      }
    }
  } else if (rsi->restoration_type == RESTORE_OFFSET) {
    // int c;
    const int width = cm->width;
    const int height = cm->height;
    const int stride = frame->y_stride;
    rst->classifier_mode = rsi->classifier_mode;
    rst->pixel_cls = (int *)malloc(sizeof(*rst->pixel_cls) * width * height);
    assert(rst->pixel_class != NULL);
    rst->nclasses =
        vp10_loop_restoration_nclasses(width, height, rst->classifier_mode);
    vp10_loop_restoration_classifier(frame->y_buffer, width, height, stride,
                                     rst->classifier_mode, rst->pixel_cls);
    rst->offsets = rsi->offsets;
  }
}

typedef struct {
  int tile_width;
  int tile_height;
  int num_bins;
} ClassifierParamsType;

static ClassifierParamsType classifier_params[CLASSIFIER_MODES] = {
  //  {16, 16, 8},
  //  {16, 16, 16},
  //  {16, 16, 32},
  //  {16, 16, 64},
  //  {32, 32, 8},
  //  {32, 32, 16},
  //  {32, 32, 32},
  //  {32, 32, 64},
  { 64, 64, 8 },
  { 64, 64, 16 },
  //  {64, 64, 32},
  //  {64, 64, 64},
  { 128, 128, 8 },
  { 128, 128, 16 },
  //  {128, 128, 32},
  //  {128, 128, 64},
};

static inline int min(int a, int b) { return ((a < b) ? a : b); }

static int offset_enc_params[OFFSET_ENC_MODES] = { 1, 2, 3, 4 };

int vp10_restoration_offset_enc_param(const int offset_enc_mode) {
  return offset_enc_params[offset_enc_mode];
}

#define ENABLE_TILE_CLASSIFIER
#define ENABLE_AMP_CLASSIFIER
//#define ENABLE_EDGE_CLASSIFIER
//#define ENABLE_VAR_CLASSIFIER

int vp10_loop_restoration_nclasses(int width, int height, int classifier_mode) {
  int nclasses;
  ClassifierParamsType params = classifier_params[classifier_mode];
#ifdef ENABLE_TILE_CLASSIFIER
  const int nhtiles = width / params.tile_width;
  const int nvtiles = height / params.tile_height;
  const int num_tile_cls = nhtiles * nvtiles;
#endif
#ifdef ENABLE_AMP_CLASSIFIER
  const int num_amp_cls = params.num_bins;
#endif
#ifdef ENABLE_EDGE_CLASSIFIER
  const int num_edge_cls = 2;
#endif
#ifdef ENABLE_VAR_CLASSIFIER
  const int num_var_cls = 2;
#endif

  // Compute number of classes
  nclasses = 1;
#ifdef ENABLE_TILE_CLASSIFIER
  nclasses *= num_tile_cls;
#endif
#ifdef ENABLE_AMP_CLASSIFIER
  nclasses *= num_amp_cls;
#endif
#ifdef ENABLE_EDGE_CLASSIFIER
  nclasses *= num_edge_cls;
#endif
#ifdef ENABLE_VAR_CLASSIFIER
  nclasses *= num_var_cls;
#endif
  return nclasses;
}

void vp10_loop_restoration_classifier(uint8_t *img, int width, int height,
                                      int stride, int classifier_mode,
                                      int *cls) {
  const ClassifierParamsType params = classifier_params[classifier_mode];
#ifdef ENABLE_TILE_CLASSIFIER
  int i, j, it, jt, tile_cls;
  const int nhtiles = width / params.tile_width;
  const int nvtiles = height / params.tile_height;
  const int num_tile_cls = nhtiles * nvtiles;
#endif
#ifdef ENABLE_AMP_CLASSIFIER
  int amp_cls;
  const int num_amp_cls = params.num_bins;
  const int bin_size = 256 / params.num_bins;
#endif
#ifdef ENABLE_EDGE_CLASSIFIER
  const int num_edge_cls = 2;
  int Gx, Gy, edge_cls;
#endif
#ifdef ENABLE_VAR_CLASSIFIER
  const int num_var_cls = 2;
  int nbr_sum, var_cls;
#endif

  for (i = 0; i < height; ++i) {
    for (j = 0; j < width; ++j) {
      cls[i * width + j] = 0;
#ifdef ENABLE_TILE_CLASSIFIER
      jt = j / params.tile_width;
      jt = (jt >= nhtiles) ? nhtiles - 1 : jt;
      it = i / params.tile_height;
      it = (it >= nvtiles) ? nvtiles - 1 : it;
      tile_cls = (it * nhtiles + jt);
      cls[i * width + j] = tile_cls + num_tile_cls * cls[i * width + j];
#endif
#ifdef ENABLE_AMP_CLASSIFIER
      amp_cls = (img[i * stride + j] / bin_size);
      cls[i * width + j] = amp_cls + num_amp_cls * cls[i * width + j];
#endif
#ifdef ENABLE_EDGE_CLASSIFIER
      if (i > 0 && j > 0 && i < (height - 1) && j < (width - 1)) {
        Gy = (img[(i + 1) * stride + j] - img[(i - 1) * stride + j]) * 2 +
             (img[(i + 1) * stride + j + 1] - img[(i - 1) * stride + j + 1]) +
             (img[(i + 1) * stride + j - 1] - img[(i - 1) * stride + j - 1]);
        Gx = (img[i * stride + j + 1] - img[i * stride + j - 1]) * 2 +
             (img[(i - 1) * stride + j + 1] - img[(i - 1) * stride + j - 1]) +
             (img[(i + 1) * stride + j + 1] - img[(i + 1) * stride + j - 1]);
        edge_cls = (Gx * Gx + Gy * Gy > 128 ? 1 : 0);
      } else {
        edge_cls = 0;
      }
      cls[i * width + j] = edge_cls + num_edge_cls * cls[i * width + j];
#endif
#ifdef ENABLE_VAR_CLASSIFIER
      if (i > 0 && j > 0 && i < (height - 1) && j < (width - 1)) {
        nbr_sum = img[(i - 1) * stride + j] + img[(i + 1) * stride + j];
        nbr_sum += img[i * stride + (j - 1)] + img[i * stride + (j + 1)];
        nbr_sum +=
            img[(i - 1) * stride + (j - 1)] + img[(i + 1) * stride + (j + 1)];
        nbr_sum +=
            img[(i + 1) * stride + (j - 1)] + img[(i - 1) * stride + (j + 1)];
        var_cls = (abs(nbr_sum - 8 * img[i * stride + j]) > 32) ? 1 : 0;
      } else {
        var_cls = 0;
      }
      cls[i * width + j] = var_cls + num_var_cls * cls[i * width + j];
#endif
    }
  }
}

static void loop_bilateral_filter(uint8_t *data, int width, int height,
                                  int stride, RestorationInternal *rst,
                                  uint8_t *tmpdata, int tmpstride) {
  int i, j, tile_idx, htile_idx, vtile_idx;
  int h_start, h_end, v_start, v_end;
  int tile_width, tile_height, nhtiles, nvtiles;

  if (rst->bilateral_tiletype >= 0) {
    tile_width =
        VPXMIN(restoration_tile_sizes[rst->bilateral_tiletype].width, width);
    tile_width >>= rst->subsampling_x;
    tile_height =
        VPXMIN(restoration_tile_sizes[rst->bilateral_tiletype].height, height);
    tile_height >>= rst->subsampling_y;
  } else {
    tile_width = width;
    tile_height = height;
  }
  nhtiles = (width + (tile_width >> 1)) / tile_width;
  nvtiles = (height + (tile_height >> 1)) / tile_height;

  for (tile_idx = 0; tile_idx < rst->bilateral_ntiles; ++tile_idx) {
    uint8_t *data_p, *tmpdata_p;
    const uint8_t *wr_lut_ = rst->wr_lut[tile_idx] + BILATERAL_AMP_RANGE;

    if (rst->bilateral_level[tile_idx] < 0) continue;

    htile_idx = tile_idx % nhtiles;
    vtile_idx = tile_idx / nhtiles;
    h_start =
        htile_idx * tile_width + ((htile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    h_end = (htile_idx < nhtiles - 1) ? ((htile_idx + 1) * tile_width)
                                      : (width - RESTORATION_HALFWIN);
    v_start =
        vtile_idx * tile_height + ((vtile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    v_end = (vtile_idx < nvtiles - 1) ? ((vtile_idx + 1) * tile_height)
                                      : (height - RESTORATION_HALFWIN);

    data_p = data + h_start + v_start * stride;
    tmpdata_p = tmpdata + h_start + v_start * tmpstride;

    for (i = 0; i < (v_end - v_start); ++i) {
      for (j = 0; j < (h_end - h_start); ++j) {
        int x, y;
        int flsum = 0, wtsum = 0, wt;
        uint8_t *data_p2 = data_p + j - RESTORATION_HALFWIN * stride;
        for (y = -RESTORATION_HALFWIN; y <= RESTORATION_HALFWIN; ++y) {
          for (x = -RESTORATION_HALFWIN; x <= RESTORATION_HALFWIN; ++x) {
            wt = (int)rst->wx_lut[tile_idx][y + RESTORATION_HALFWIN]
                                 [x + RESTORATION_HALFWIN] *
                 (int)wr_lut_[data_p2[x] - data_p[j]];
            wtsum += wt;
            flsum += wt * data_p2[x];
          }
          data_p2 += stride;
        }
        if (wtsum > 0)
          tmpdata_p[j] = clip_pixel((int)((flsum + wtsum / 2) / wtsum));
        else
          tmpdata_p[j] = data_p[j];
      }
      tmpdata_p += tmpstride;
      data_p += stride;
    }
    for (i = v_start; i < v_end; ++i) {
      memcpy(data + i * stride + h_start, tmpdata + i * tmpstride + h_start,
             (h_end - h_start) * sizeof(*data));
    }
  }
}

uint8_t hor_sym_filter(uint8_t *d, int *hfilter) {
  int32_t s =
      (1 << (RESTORATION_FILT_BITS - 1)) + d[0] * hfilter[RESTORATION_HALFWIN];
  int i;
  for (i = 1; i <= RESTORATION_HALFWIN; ++i)
    s += (d[i] + d[-i]) * hfilter[RESTORATION_HALFWIN + i];
  return clip_pixel(s >> RESTORATION_FILT_BITS);
}

uint8_t ver_sym_filter(uint8_t *d, int stride, int *vfilter) {
  int32_t s =
      (1 << (RESTORATION_FILT_BITS - 1)) + d[0] * vfilter[RESTORATION_HALFWIN];
  int i;
  for (i = 1; i <= RESTORATION_HALFWIN; ++i)
    s += (d[i * stride] + d[-i * stride]) * vfilter[RESTORATION_HALFWIN + i];
  return clip_pixel(s >> RESTORATION_FILT_BITS);
}

static void loop_wiener_filter(uint8_t *data, int width, int height, int stride,
                               RestorationInternal *rst, uint8_t *tmpdata,
                               int tmpstride) {
  int i, j, tile_idx, htile_idx, vtile_idx;
  int h_start, h_end, v_start, v_end;
  int tile_width, tile_height, nhtiles, nvtiles;
  uint8_t *data_p, *tmpdata_p;

  if (rst->wiener_tiletype >= 0) {
    tile_width =
        VPXMIN(restoration_tile_sizes[rst->wiener_tiletype].width, width);
    tile_width >>= rst->subsampling_x;
    tile_height =
        VPXMIN(restoration_tile_sizes[rst->wiener_tiletype].height, height);
    tile_height >>= rst->subsampling_y;
  } else {
    tile_width = width;
    tile_height = height;
  }
  nhtiles = (width + (tile_width >> 1)) / tile_width;
  nvtiles = (height + (tile_height >> 1)) / tile_height;

  // Initialize tmp buffer
  data_p = data;
  tmpdata_p = tmpdata;
  for (i = 0; i < height; ++i) {
    memcpy(tmpdata_p, data_p, sizeof(*data_p) * width);
    data_p += stride;
    tmpdata_p += tmpstride;
  }

  // Filter row-wise tile-by-tile
  for (tile_idx = 0; tile_idx < rst->wiener_ntiles; ++tile_idx) {
    if (rst->wiener_process_tile[tile_idx] == 0) continue;
    htile_idx = tile_idx % nhtiles;
    vtile_idx = tile_idx / nhtiles;
    h_start =
        htile_idx * tile_width + ((htile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    h_end = (htile_idx < nhtiles - 1) ? ((htile_idx + 1) * tile_width)
                                      : (width - RESTORATION_HALFWIN);
    v_start = vtile_idx * tile_height;
    v_end =
        (vtile_idx < nvtiles - 1) ? ((vtile_idx + 1) * tile_height) : height;
    data_p = data + h_start + v_start * stride;
    tmpdata_p = tmpdata + h_start + v_start * tmpstride;
    for (i = 0; i < (v_end - v_start); ++i) {
      for (j = 0; j < (h_end - h_start); ++j) {
        *tmpdata_p++ = hor_sym_filter(data_p++, rst->hfilter[tile_idx]);
      }
      data_p += stride - (h_end - h_start);
      tmpdata_p += tmpstride - (h_end - h_start);
    }
  }

  // Filter column-wise tile-by-tile (bands of thickness RESTORATION_HALFWIN
  // at top and bottom of tiles allow filtering overlap, and are not optimally
  // filtered)
  for (tile_idx = 0; tile_idx < rst->wiener_ntiles; ++tile_idx) {
    if (rst->wiener_process_tile[tile_idx] == 0) continue;
    htile_idx = tile_idx % nhtiles;
    vtile_idx = tile_idx / nhtiles;
    h_start = htile_idx * tile_width;
    h_end = (htile_idx < nhtiles - 1) ? ((htile_idx + 1) * tile_width) : width;
    v_start =
        vtile_idx * tile_height + ((vtile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    v_end = (vtile_idx < nvtiles - 1) ? ((vtile_idx + 1) * tile_height)
                                      : (height - RESTORATION_HALFWIN);
    data_p = data + h_start + v_start * stride;
    tmpdata_p = tmpdata + h_start + v_start * tmpstride;
    for (i = 0; i < (v_end - v_start); ++i) {
      for (j = 0; j < (h_end - h_start); ++j) {
        *data_p++ =
            ver_sym_filter(tmpdata_p++, tmpstride, rst->vfilter[tile_idx]);
      }
      data_p += stride - (h_end - h_start);
      tmpdata_p += tmpstride - (h_end - h_start);
    }
  }
}

static void loop_offset_correction(uint8_t *data, int width, int height,
                                   int stride, RestorationInternal *rst,
                                   uint8_t *tmpdata, int tmpstride) {
  int i, j, c;
  for (i = 0; i < height; ++i) {
    for (j = 0; j < width; ++j) {
      c = rst->pixel_cls[i * width + j];
      if (rst->offsets[c] != 0) {
        data[i * stride + j] =
            clip_pixel(data[i * stride + j] + rst->offsets[c]);
      }
    }
  }
  // Unused variables
  (void)(tmpdata);
  (void)(tmpstride);
}

#if CONFIG_VP9_HIGHBITDEPTH
static void loop_bilateral_filter_highbd(uint8_t *data8, int width, int height,
                                         int stride, RestorationInternal *rst,
                                         uint8_t *tmpdata8, int tmpstride,
                                         int bit_depth) {
  int i, j, tile_idx, htile_idx, vtile_idx;
  int h_start, h_end, v_start, v_end;
  int tile_width, tile_height, nhtiles, nvtiles;

  uint16_t *data = CONVERT_TO_SHORTPTR(data8);
  uint16_t *tmpdata = CONVERT_TO_SHORTPTR(tmpdata8);

  if (rst->bilateral_tiletype >= 0) {
    tile_width =
        VPXMIN(restoration_tile_sizes[rst->bilateral_tiletype].width, width);
    tile_width >>= rst->subsampling_x;
    tile_height =
        VPXMIN(restoration_tile_sizes[rst->bilateral_tiletype].height, height);
    tile_height >>= rst->subsampling_y;
  } else {
    tile_width = width;
    tile_height = height;
  }
  nhtiles = (width + (tile_width >> 1)) / tile_width;
  nvtiles = (height + (tile_height >> 1)) / tile_height;

  for (tile_idx = 0; tile_idx < rst->bilateral_ntiles; ++tile_idx) {
    uint16_t *data_p, *tmpdata_p;
    const uint8_t *wr_lut_ = rst->wr_lut[tile_idx] + BILATERAL_AMP_RANGE;

    if (rst->bilateral_level[tile_idx] < 0) continue;

    htile_idx = tile_idx % nhtiles;
    vtile_idx = tile_idx / nhtiles;
    h_start =
        htile_idx * tile_width + ((htile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    h_end = (htile_idx < nhtiles - 1) ? ((htile_idx + 1) * tile_width)
                                      : (width - RESTORATION_HALFWIN);
    v_start =
        vtile_idx * tile_height + ((vtile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    v_end = (vtile_idx < nvtiles - 1) ? ((vtile_idx + 1) * tile_height)
                                      : (height - RESTORATION_HALFWIN);

    data_p = data + h_start + v_start * stride;
    tmpdata_p = tmpdata + h_start + v_start * tmpstride;

    for (i = 0; i < (v_end - v_start); ++i) {
      for (j = 0; j < (h_end - h_start); ++j) {
        int x, y;
        int flsum = 0, wtsum = 0, wt;
        uint16_t *data_p2 = data_p + j - RESTORATION_HALFWIN * stride;
        for (y = -RESTORATION_HALFWIN; y <= RESTORATION_HALFWIN; ++y) {
          for (x = -RESTORATION_HALFWIN; x <= RESTORATION_HALFWIN; ++x) {
            wt = (int)rst->wx_lut[tile_idx][y + RESTORATION_HALFWIN]
                                 [x + RESTORATION_HALFWIN] *
                 (int)wr_lut_[data_p2[x] - data_p[j]];
            wtsum += wt;
            flsum += wt * data_p2[x];
          }
          data_p2 += stride;
        }
        if (wtsum > 0)
          tmpdata_p[j] =
              clip_pixel_highbd((int)((flsum + wtsum / 2) / wtsum), bit_depth);
        else
          tmpdata_p[j] = data_p[j];
      }
      tmpdata_p += tmpstride;
      data_p += stride;
    }
    for (i = v_start; i < v_end; ++i) {
      memcpy(data + i * stride + h_start, tmpdata + i * tmpstride + h_start,
             (h_end - h_start) * sizeof(*data));
    }
  }
}

uint16_t hor_sym_filter_highbd(uint16_t *d, int *hfilter, int bd) {
  int32_t s =
      (1 << (RESTORATION_FILT_BITS - 1)) + d[0] * hfilter[RESTORATION_HALFWIN];
  int i;
  for (i = 1; i <= RESTORATION_HALFWIN; ++i)
    s += (d[i] + d[-i]) * hfilter[RESTORATION_HALFWIN + i];
  return clip_pixel_highbd(s >> RESTORATION_FILT_BITS, bd);
}

uint16_t ver_sym_filter_highbd(uint16_t *d, int stride, int *vfilter, int bd) {
  int32_t s =
      (1 << (RESTORATION_FILT_BITS - 1)) + d[0] * vfilter[RESTORATION_HALFWIN];
  int i;
  for (i = 1; i <= RESTORATION_HALFWIN; ++i)
    s += (d[i * stride] + d[-i * stride]) * vfilter[RESTORATION_HALFWIN + i];
  return clip_pixel_highbd(s >> RESTORATION_FILT_BITS, bd);
}

static void loop_wiener_filter_highbd(uint8_t *data8, int width, int height,
                                      int stride, RestorationInternal *rst,
                                      uint8_t *tmpdata8, int tmpstride,
                                      int bit_depth) {
  uint16_t *data = CONVERT_TO_SHORTPTR(data8);
  uint16_t *tmpdata = CONVERT_TO_SHORTPTR(tmpdata8);
  int i, j, tile_idx, htile_idx, vtile_idx;
  int h_start, h_end, v_start, v_end;
  int tile_width, tile_height, nhtiles, nvtiles;
  uint16_t *data_p, *tmpdata_p;

  if (rst->wiener_tiletype >= 0) {
    tile_width =
        VPXMIN(restoration_tile_sizes[rst->wiener_tiletype].width, width);
    tile_width >>= rst->subsampling_x;
    tile_height =
        VPXMIN(restoration_tile_sizes[rst->wiener_tiletype].height, height);
    tile_height >>= rst->subsampling_y;
  } else {
    tile_width = width;
    tile_height = height;
  }
  nhtiles = (width + (tile_width >> 1)) / tile_width;
  nvtiles = (height + (tile_height >> 1)) / tile_height;

  // Initialize tmp buffer
  data_p = data;
  tmpdata_p = tmpdata;
  for (i = 0; i < height; ++i) {
    memcpy(tmpdata_p, data_p, sizeof(*data_p) * width);
    data_p += stride;
    tmpdata_p += tmpstride;
  }

  // Filter row-wise tile-by-tile
  for (tile_idx = 0; tile_idx < rst->wiener_ntiles; ++tile_idx) {
    if (rst->wiener_process_tile[tile_idx] == 0) continue;
    htile_idx = tile_idx % nhtiles;
    vtile_idx = tile_idx / nhtiles;
    h_start =
        htile_idx * tile_width + ((htile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    h_end = (htile_idx < nhtiles - 1) ? ((htile_idx + 1) * tile_width)
                                      : (width - RESTORATION_HALFWIN);
    v_start = vtile_idx * tile_height;
    v_end =
        (vtile_idx < nvtiles - 1) ? ((vtile_idx + 1) * tile_height) : height;
    data_p = data + h_start + v_start * stride;
    tmpdata_p = tmpdata + h_start + v_start * tmpstride;
    for (i = 0; i < (v_end - v_start); ++i) {
      for (j = 0; j < (h_end - h_start); ++j) {
        *tmpdata_p++ =
            hor_sym_filter_highbd(data_p++, rst->hfilter[tile_idx], bit_depth);
      }
      data_p += stride - (h_end - h_start);
      tmpdata_p += tmpstride - (h_end - h_start);
    }
  }

  // Filter column-wise tile-by-tile (bands of thickness RESTORATION_HALFWIN
  // at top and bottom of tiles allow filtering overlap, and are not optimally
  // filtered)
  for (tile_idx = 0; tile_idx < rst->wiener_ntiles; ++tile_idx) {
    if (rst->wiener_process_tile[tile_idx] == 0) continue;
    htile_idx = tile_idx % nhtiles;
    vtile_idx = tile_idx / nhtiles;
    h_start = htile_idx * tile_width;
    h_end = (htile_idx < nhtiles - 1) ? ((htile_idx + 1) * tile_width) : width;
    v_start =
        vtile_idx * tile_height + ((vtile_idx > 0) ? 0 : RESTORATION_HALFWIN);
    v_end = (vtile_idx < nvtiles - 1) ? ((vtile_idx + 1) * tile_height)
                                      : (height - RESTORATION_HALFWIN);
    data_p = data + h_start + v_start * stride;
    tmpdata_p = tmpdata + h_start + v_start * tmpstride;
    for (i = 0; i < (v_end - v_start); ++i) {
      for (j = 0; j < (h_end - h_start); ++j) {
        *data_p++ = ver_sym_filter_highbd(tmpdata_p++, tmpstride,
                                          rst->vfilter[tile_idx], bit_depth);
      }
      data_p += stride - (h_end - h_start);
      tmpdata_p += tmpstride - (h_end - h_start);
    }
  }
}

static void loop_offset_correction_highbd(uint8_t *data8, int width, int height,
                                          int stride, RestorationInternal *rst,
                                          uint8_t *tmpdata8,
                                          int tmpstride int bit_depth) {
  uint16_t *data = CONVERT_TO_SHORTPTR(data8);
  int i, j, c;
  for (i = 0; i < height; ++i) {
    for (j = 0; j < width; ++j) {
      c = rst->pixel_cls[i * width + j];
      if (rst->offsets[c] != 0) {
        data[i * stride + j] =
            clip_pixel_highbd(data[i * stride + j] + rst->offsets[c]);
      }
    }
  }
  // Unused variables
  (void)(tmpdata8);
  (void)(tmpstride);
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

void vp10_loop_restoration_rows(YV12_BUFFER_CONFIG *frame, VP10_COMMON *cm,
                                int start_mi_row, int end_mi_row, int y_only) {
  const int ywidth = frame->y_crop_width;
  const int ystride = frame->y_stride;
  const int uvwidth = frame->uv_crop_width;
  const int uvstride = frame->uv_stride;
  const int ystart = start_mi_row << MI_SIZE_LOG2;
  const int uvstart = ystart >> cm->subsampling_y;
  int yend = end_mi_row << MI_SIZE_LOG2;
  int uvend = yend >> cm->subsampling_y;

  YV12_BUFFER_CONFIG *tmp_buf;

  restore_func_type restore_func;
  if (cm->rst_internal.restoration_type == RESTORE_BILATERAL)
    restore_func = loop_bilateral_filter;
  else if (cm->rst_internal.restoration_type == RESTORE_WIENER)
    restore_func = loop_wiener_filter;
  else
    restore_func = loop_offset_correction;
#if CONFIG_VP9_HIGHBITDEPTH
  restore_func_highbd_type restore_func_highbd;
  if (cm->rst_internal.restoration_type == RESTORE_BILATERAL)
    restore_func_highbd = loop_bilateral_filter_highbd;
  else if (cm->rst_internal.restoration_type == RESTORE_WIENER)
    restore_func_highbd = loop_wiener_filter_highbd;
  else
    restore_func_highbd = loop_offset_correction_highbd;
#endif  // CONFIG_VP9_HIGHBITDEPTH

  yend = VPXMIN(yend, cm->height);
  uvend = VPXMIN(uvend, cm->subsampling_y ? (cm->height + 1) >> 1 : cm->height);

  if (vpx_realloc_frame_buffer(&cm->tmp_loop_buf, cm->width, cm->height,
                               cm->subsampling_x, cm->subsampling_y,
#if CONFIG_VP9_HIGHBITDEPTH
                               cm->use_highbitdepth,
#endif
                               VPX_DEC_BORDER_IN_PIXELS, cm->byte_alignment,
                               NULL, NULL, NULL) < 0)
    vpx_internal_error(&cm->error, VPX_CODEC_MEM_ERROR,
                       "Failed to allocate tmp restoration buffer");

  tmp_buf = &cm->tmp_loop_buf;

#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth)
    restore_func_highbd(frame->y_buffer + ystart * ystride, ywidth,
                        yend - ystart, ystride, &cm->rst_internal,
                        tmp_buf->y_buffer + ystart * tmp_buf->y_stride,
                        tmp_buf->y_stride, cm->bit_depth);
  else
#endif  // CONFIG_VP9_HIGHBITDEPTH
    restore_func(frame->y_buffer + ystart * ystride, ywidth, yend - ystart,
                 ystride, &cm->rst_internal,
                 tmp_buf->y_buffer + ystart * tmp_buf->y_stride,
                 tmp_buf->y_stride);

  if ((cm->rst_internal.restoration_type != RESTORE_OFFSET) && !y_only) {
    cm->rst_internal.subsampling_x = cm->subsampling_x;
    cm->rst_internal.subsampling_y = cm->subsampling_y;
#if CONFIG_VP9_HIGHBITDEPTH
    if (cm->use_highbitdepth) {
      restore_func_highbd(frame->u_buffer + uvstart * uvstride, uvwidth,
                          uvend - uvstart, uvstride, &cm->rst_internal,
                          tmp_buf->u_buffer + uvstart * tmp_buf->uv_stride,
                          tmp_buf->uv_stride, cm->bit_depth);
      restore_func_highbd(frame->v_buffer + uvstart * uvstride, uvwidth,
                          uvend - uvstart, uvstride, &cm->rst_internal,
                          tmp_buf->v_buffer + uvstart * tmp_buf->uv_stride,
                          tmp_buf->uv_stride, cm->bit_depth);
    } else {
#endif  // CONFIG_VP9_HIGHBITDEPTH
      restore_func(frame->u_buffer + uvstart * uvstride, uvwidth,
                   uvend - uvstart, uvstride, &cm->rst_internal,
                   tmp_buf->u_buffer + uvstart * tmp_buf->uv_stride,
                   tmp_buf->uv_stride);
      restore_func(frame->v_buffer + uvstart * uvstride, uvwidth,
                   uvend - uvstart, uvstride, &cm->rst_internal,
                   tmp_buf->v_buffer + uvstart * tmp_buf->uv_stride,
                   tmp_buf->uv_stride);
#if CONFIG_VP9_HIGHBITDEPTH
    }
#endif  // CONFIG_VP9_HIGHBITDEPTH
  }
  if (cm->rst_internal.restoration_type == RESTORE_BILATERAL) {
    free(cm->rst_internal.wr_lut);
    cm->rst_internal.wr_lut = NULL;
    free(cm->rst_internal.wx_lut);
    cm->rst_internal.wx_lut = NULL;
  }
  if (cm->rst_internal.restoration_type == RESTORE_WIENER) {
    free(cm->rst_internal.vfilter);
    cm->rst_internal.vfilter = NULL;
    free(cm->rst_internal.hfilter);
    cm->rst_internal.hfilter = NULL;
  }
  if (cm->rst_internal.restoration_type == RESTORE_OFFSET) {
    free(cm->rst_internal.pixel_cls);
    cm->rst_internal.pixel_cls = NULL;
  }
}

void vp10_loop_restoration_frame(YV12_BUFFER_CONFIG *frame, VP10_COMMON *cm,
                                 RestorationInfo *rsi, int y_only,
                                 int partial_frame) {
  int start_mi_row, end_mi_row, mi_rows_to_filter;
  if (rsi->restoration_type != RESTORE_NONE) {
    start_mi_row = 0;
    mi_rows_to_filter = cm->mi_rows;
    if (partial_frame && cm->mi_rows > 8) {
      start_mi_row = cm->mi_rows >> 1;
      start_mi_row &= 0xfffffff8;
      mi_rows_to_filter = VPXMAX(cm->mi_rows / 8, 8);
    }
    end_mi_row = start_mi_row + mi_rows_to_filter;
    vp10_loop_restoration_init(cm, rsi, cm->frame_type == KEY_FRAME, frame);
    vp10_loop_restoration_rows(frame, cm, start_mi_row, end_mi_row, y_only);
  }
}
