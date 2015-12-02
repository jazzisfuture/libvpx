/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be
 *  found  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#define MAX_LEVELS 8
#define MAX_ERROR  0.00001
#define RELATIVE_ERROR(a, b) (fabs(((a) - (b)) / ((b) + 0.000001)))
#define WRAP_PIXEL(p, size) ((p) < 0 ? abs((p)) - 1 : \
                            ((p) >= (size) ? (2 * (size) - 1 - (p)) : (p)))

#define CHAR_TO_DOUBLE(sz, loc) ((sz) == sizeof(unsigned char) ? \
                                (double)*loc : *((double*)(loc)))

typedef struct {
  int width, height;
  double *a1;
  double *a2;
  double *confidence;
  double *u;
  double *v;
} flowMV;

typedef struct {
  int n_levels;
  int widths[MAX_LEVELS];
  int heights[MAX_LEVELS];
  int strides[MAX_LEVELS];
  unsigned char **levels;
} imPyramid;

void destruct_pyramid(imPyramid *pyr);


void compute_flow(unsigned char *frm, unsigned char *ref,
                  flowMV *flow, int width, int height,
                  int frm_stride, int ref_stride);

void optical_flow_per_pixel(double *dx, double *dy, double *dt, double *window,
                      int wind_size, flowMV *flow, int width,
                      int height, int stride, int corner1, int corner2);

void coarse_to_fine_flow(unsigned char *frm, unsigned char *ref,
                         flowMV *flow, int width, int height,
                         int frm_stride, int ref_stride, int n_refinements,
                         int n_levels);

void image_pyramid(unsigned char *img, imPyramid *pyr, int width,
                   int height, int stride, double resize_factor,
                   int n_levels);

void iterative_refinement(unsigned char *ref, double *smooth_frm,
                          double *frm_dx, double *frm_dy, flowMV *flow,
                          int width, int height, int ref_stride,
                          int smooth_frm_stride, int n_refinements);

void lucas_kanade_base(double *smooth_frm, double * smooth_ref,
                        double *dx, double *dy, flowMV *flow,
                        int width, int height, int frm_stride, int ref_stride);

void optical_flow_per_pixel(double *dx, double *dy, double *dt,
                            double *window, int wind_size, flowMV *flow,
                            int width, int height, int stride, int corner1,
                            int corner2);

void pointwise_matrix_mult(unsigned char *mat1, unsigned char *mat2,
                           size_t elem_size, double *product, int width,
                           int height, int stride1, int stride2);

void pointwise_matrix_sub(unsigned char *mat1, unsigned char *mat2,
                           size_t elem_size, double *difference, int width,
                           int height, int stride1, int stride2);

void pointwise_matrix_add(unsigned char *mat1, unsigned char *mat2,
                           size_t elem_size, double *sum, int width,
                           int height, int stride1, int stride2);

void pointwise_matrix_div(unsigned char *mat1, unsigned char *mat2,
                           size_t elem_size, double *sum, int width,
                           int height, int stride1, int stride2);

double selection(double *vals, int length, int k);

void median_filter(unsigned char *source, double *filtered, size_t elem_size,
                    int block_size, int width, int height, int stride);

void differentiate(unsigned char *img, size_t elem_size, int width, int height,
                   int stride, double **dx_img, double **dy_img);

void blur_img(unsigned char *img, int width, int height,
                int stride, double **smooth_img, int smoothing_size,
                double smoothing_sig, double smoothing_amp);

void gaussian_kernel(double *mat, int size, double sig, double amp);

