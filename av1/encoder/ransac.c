/*
 *   (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <memory.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "av1/encoder/ransac.h"

#define MAX_MINPTS 4
#define MAX_DEGENERATE_ITER 10
#define MINPTS_MULTIPLIER 5

int IMAX(int a, int b) { return (((a) < (b)) ? (b) : (a)); }

static int is_collinear3(double *p1, double *p2, double *p3) {
  static const double collinear_eps = 1e-3;
  const double v =
      (p2[0] - p1[0]) * (p3[1] - p1[1]) - (p2[1] - p1[1]) * (p3[0] - p1[0]);
  return fabs(v) < collinear_eps;
}

static int isDegenerateTranslation(double *p) {
  return (p[0] - p[2]) * (p[0] - p[2]) + (p[1] - p[3]) * (p[1] - p[3]) <= 2;
}

static int isDegenerateAffine(double *p) {
  return is_collinear3(p, p + 2, p + 4);
}

static int isDegenerateHomography(double *p) {
  return is_collinear3(p, p + 2, p + 4) || is_collinear3(p, p + 2, p + 6) ||
         is_collinear3(p, p + 4, p + 6) || is_collinear3(p + 2, p + 4, p + 6);
}

////////////////////////////////////////////////////////////////////////////////
// ransac
typedef int (*isDegenerateType)(double *p);
typedef void (*normalizeType)(double *p, int np, double *T);
typedef void (*denormalizeType)(double *H, double *T1, double *T2);
typedef int (*findTransformationType)(int points, double *points1,
                                      double *points2, double *H);

static int get_rand_indices(int npoints, int minpts, int *indices) {
  int i, j;
  unsigned int seed = (unsigned int)npoints;
  int ptr = rand_r(&seed) % npoints;
  if (minpts > npoints) return 0;
  indices[0] = ptr;
  ptr = (ptr == npoints - 1 ? 0 : ptr + 1);
  i = 1;
  while (i < minpts) {
    int index = rand_r(&seed) % npoints;
    while (index) {
      ptr = (ptr == npoints - 1 ? 0 : ptr + 1);
      for (j = 0; j < i; ++j) {
        if (indices[j] == ptr) break;
      }
      if (j == i) index--;
    }
    indices[i++] = ptr;
  }
  return 1;
}

int ransac_(double *matched_points, int npoints, int *number_of_inliers,
            int *best_inlier_mask, double *bestH, const int minpts,
            const int paramdim, isDegenerateType isDegenerate,
            normalizeType normalize, denormalizeType denormalize,
            findTransformationType findTransformation,
            ProjectPointsType projectpoints, TransformationType type) {
  static const double INLIER_THRESHOLD_NORMALIZED = 0.1;
  static const double INLIER_THRESHOLD_UNNORMALIZED = 1.0;
  static const double PROBABILITY_REQUIRED = 0.9;
  static const double EPS = 1e-12;
  static const int MIN_TRIALS = 20;

  const double inlier_threshold =
      (normalize && denormalize ? INLIER_THRESHOLD_NORMALIZED
                                : INLIER_THRESHOLD_UNNORMALIZED);
  int N = 10000, trial_count = 0;
  int i;
  int ret_val = 0;

  int max_inliers = 0;
  double best_variance = 0.0;
  double H[MAX_PARAMDIM];
  WarpedMotionParams wm;
  double points1[2 * MAX_MINPTS];
  double points2[2 * MAX_MINPTS];
  int indices[MAX_MINPTS];

  double *best_inlier_set1;
  double *best_inlier_set2;
  double *inlier_set1;
  double *inlier_set2;
  double *corners1;
  int *corners1_int;
  double *corners2;
  int *image1_coord;
  int *inlier_mask;

  double *cnp1, *cnp2;
  double T1[9], T2[9];

  // srand((unsigned)time(NULL)) ;
  // better to make this deterministic for a given sequence for ease of testing
  srand(npoints);

  *number_of_inliers = 0;
  if (npoints < minpts * MINPTS_MULTIPLIER) {
    printf("Cannot find motion with %d matches\n", npoints);
    return 1;
  }

  memset(&wm, 0, sizeof(wm));
  best_inlier_set1 = (double *)malloc(sizeof(*best_inlier_set1) * npoints * 2);
  best_inlier_set2 = (double *)malloc(sizeof(*best_inlier_set2) * npoints * 2);
  inlier_set1 = (double *)malloc(sizeof(*inlier_set1) * npoints * 2);
  inlier_set2 = (double *)malloc(sizeof(*inlier_set2) * npoints * 2);
  corners1 = (double *)malloc(sizeof(*corners1) * npoints * 2);
  corners1_int = (int *)malloc(sizeof(*corners1_int) * npoints * 2);
  corners2 = (double *)malloc(sizeof(*corners2) * npoints * 2);
  image1_coord = (int *)malloc(sizeof(*image1_coord) * npoints * 2);
  inlier_mask = (int *)malloc(sizeof(*inlier_mask) * npoints);

  for (cnp1 = corners1, cnp2 = corners2, i = 0; i < npoints; ++i) {
    *(cnp1++) = *(matched_points++);
    *(cnp1++) = *(matched_points++);
    *(cnp2++) = *(matched_points++);
    *(cnp2++) = *(matched_points++);
  }
  matched_points -= 4 * npoints;

  if (normalize && denormalize) {
    normalize(corners1, npoints, T1);
    normalize(corners2, npoints, T2);
  }

  while (N > trial_count) {
    int num_inliers = 0;
    double sum_distance = 0.0;
    double sum_distance_squared = 0.0;

    int degenerate = 1;
    int num_degenerate_iter = 0;
    while (degenerate) {
      num_degenerate_iter++;
      if (!get_rand_indices(npoints, minpts, indices)) {
        ret_val = 1;
        goto finish_ransac;
      }
      i = 0;
      while (i < minpts) {
        int index = indices[i];
        // add to list
        points1[i * 2] = corners1[index * 2];
        points1[i * 2 + 1] = corners1[index * 2 + 1];
        points2[i * 2] = corners2[index * 2];
        points2[i * 2 + 1] = corners2[index * 2 + 1];
        i++;
      }
      degenerate = isDegenerate(points1);
      if (num_degenerate_iter > MAX_DEGENERATE_ITER) {
        ret_val = 1;
        goto finish_ransac;
      }
    }

    if (findTransformation(minpts, points1, points2, H)) {
      trial_count++;
      continue;
    }

    for (i = 0; i < npoints; ++i) {
      corners1_int[2 * i] = (int)corners1[i * 2];
      corners1_int[2 * i + 1] = (int)corners1[i * 2 + 1];
    }

    av1_integerize_model(H, type, &wm);
    projectpoints((int16_t *)wm.wmmat, corners1_int, image1_coord, npoints, 2,
                  2, 0, 0);

    for (i = 0; i < npoints; ++i) {
      double dx =
          (image1_coord[i * 2] >> WARPEDPIXEL_PREC_BITS) - corners2[i * 2];
      double dy = (image1_coord[i * 2 + 1] >> WARPEDPIXEL_PREC_BITS) -
                  corners2[i * 2 + 1];
      double distance = sqrt(dx * dx + dy * dy);

      inlier_mask[i] = distance < inlier_threshold;
      if (inlier_mask[i]) {
        inlier_set1[num_inliers * 2] = corners1[i * 2];
        inlier_set1[num_inliers * 2 + 1] = corners1[i * 2 + 1];
        inlier_set2[num_inliers * 2] = corners2[i * 2];
        inlier_set2[num_inliers * 2 + 1] = corners2[i * 2 + 1];
        num_inliers++;
        sum_distance += distance;
        sum_distance_squared += distance * distance;
      }
    }

    if (num_inliers >= max_inliers) {
      double mean_distance = sum_distance / ((double)num_inliers);
      double variance = sum_distance_squared / ((double)num_inliers - 1.0) -
                        mean_distance * mean_distance * ((double)num_inliers) /
                            ((double)num_inliers - 1.0);
      if ((num_inliers > max_inliers) ||
          (num_inliers == max_inliers && variance < best_variance)) {
        best_variance = variance;
        max_inliers = num_inliers;
        memcpy(bestH, H, paramdim * sizeof(*bestH));
        memcpy(best_inlier_set1, inlier_set1,
               num_inliers * 2 * sizeof(*best_inlier_set1));
        memcpy(best_inlier_set2, inlier_set2,
               num_inliers * 2 * sizeof(*best_inlier_set2));
        memcpy(best_inlier_mask, inlier_mask,
               npoints * sizeof(*best_inlier_mask));

        if (num_inliers > 0) {
          double fracinliers = (double)num_inliers / (double)npoints;
          double pNoOutliers = 1 - pow(fracinliers, minpts);
          int temp;
          pNoOutliers = fmax(EPS, pNoOutliers);
          pNoOutliers = fmin(1 - EPS, pNoOutliers);
          temp = (int)(log(1.0 - PROBABILITY_REQUIRED) / log(pNoOutliers));
          if (temp > 0 && temp < N) {
            N = IMAX(temp, MIN_TRIALS);
          }
        }
      }
    }
    trial_count++;
  }
  findTransformation(max_inliers, best_inlier_set1, best_inlier_set2, bestH);
  if (normalize && denormalize) {
    denormalize(bestH, T1, T2);
  }
  *number_of_inliers = max_inliers;
finish_ransac:
  free(best_inlier_set1);
  free(best_inlier_set2);
  free(inlier_set1);
  free(inlier_set2);
  free(corners1);
  free(corners2);
  free(image1_coord);
  free(inlier_mask);
  return ret_val;
}

int ransacTranslation(double *matched_points, int npoints,
                      int *number_of_inliers, int *best_inlier_mask,
                      double *bestH) {
  return ransac_(matched_points, npoints, number_of_inliers, best_inlier_mask,
                 bestH, 3, 2, isDegenerateTranslation,
                 NULL,  // normalizeHomography,
                 NULL,  // denormalizeRotZoom,
                 findTranslation, projectPointsTranslation, TRANSLATION);
}

int ransacRotZoom(double *matched_points, int npoints, int *number_of_inliers,
                  int *best_inlier_mask, double *bestH) {
  return ransac_(matched_points, npoints, number_of_inliers, best_inlier_mask,
                 bestH, 3, 4, isDegenerateAffine,
                 NULL,  // normalizeHomography,
                 NULL,  // denormalizeRotZoom,
                 findRotZoom, projectPointsRotZoom, ROTZOOM);
}

int ransacAffine(double *matched_points, int npoints, int *number_of_inliers,
                 int *best_inlier_mask, double *bestH) {
  return ransac_(matched_points, npoints, number_of_inliers, best_inlier_mask,
                 bestH, 3, 6, isDegenerateAffine,
                 NULL,  // normalizeHomography,
                 NULL,  // denormalizeAffine,
                 findAffine, projectPointsAffine, AFFINE);
}

int ransacHomography(double *matched_points, int npoints,
                     int *number_of_inliers, int *best_inlier_mask,
                     double *bestH) {
  int result = ransac_(matched_points, npoints, number_of_inliers,
                       best_inlier_mask, bestH, 4, 8, isDegenerateHomography,
                       NULL,  // normalizeHomography,
                       NULL,  // denormalizeHomography,
                       findHomography, projectPointsHomography, HOMOGRAPHY);
  if (!result) {
    // normalize so that H33 = 1
    int i;
    double m = 1.0 / bestH[8];
    for (i = 0; i < 8; ++i) bestH[i] *= m;
    bestH[8] = 1.0;
  }
  return result;
}
