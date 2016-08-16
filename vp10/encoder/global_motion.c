/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

#include "vp10/common/warped_motion.h"

#include "vp10/encoder/segmentation.h"
#include "vp10/encoder/global_motion.h"
#include "vp10/encoder/corner_detect.h"
#include "vp10/encoder/corner_match.h"
#include "vp10/encoder/ransac.h"

#define MAX_CORNERS 4096
#define MIN_INLIER_PROB 0.1

INLINE ransacType get_ransacType(TransformationType type) {
  switch (type) {
    case HOMOGRAPHY:
      return ransacHomography;
    case AFFINE:
      return ransacAffine;
    case ROTZOOM:
      return ransacRotZoom;
    case TRANSLATION:
      return ransacTranslation;
    default:
      assert(0);
      return NULL;
  }
}

static int compute_global_motion_params(TransformationType type,
                                        double *correspondences,
                                        int num_correspondences,
                                        double *H,
                                        int *inlier_map) {
  int i, result;
  double *mp, *matched_points;
  double *cp = correspondences;
  int num_inliers = 0;
  ransacType ransac = get_ransacType(type);
  if (ransac == NULL)
    return 0;
  matched_points =
      (double *)malloc(4 * num_correspondences * sizeof(*matched_points));

  for (mp = matched_points, cp = correspondences, i = 0;
       i < num_correspondences; ++i) {
    *mp++ = *cp++;
    *mp++ = *cp++;
    *mp++ = *cp++;
    *mp++ = *cp++;
  }
  result = ransac(matched_points, num_correspondences,
                  &num_inliers, inlier_map, H);
  if (!result && num_inliers < MIN_INLIER_PROB * num_correspondences) {
    result = 1;
    num_inliers = 0;
  }
  if (!result) {
    for (i = 0; i < num_correspondences; ++i) {
      inlier_map[i] = inlier_map[i] - 1;
    }
  }
  free(matched_points);
  return num_inliers;
}

int compute_global_motion_feature_based(TransformationType type,
                                        YV12_BUFFER_CONFIG *frm,
                                        YV12_BUFFER_CONFIG *ref,
                                        double *H) {
  int num_frm_corners, num_ref_corners;
  int num_correspondences;
  double *correspondences;
  int num_inliers;
  int frm_corners[2 * MAX_CORNERS], ref_corners[2 * MAX_CORNERS];
  int *inlier_map = NULL;

  num_frm_corners = FastCornerDetect(frm->y_buffer, frm->y_width,
                                     frm->y_height, frm->y_stride,
                                     frm_corners, MAX_CORNERS);
  num_ref_corners = FastCornerDetect(ref->y_buffer, ref->y_width,
                                     ref->y_height, ref->y_stride,
                                     ref_corners, MAX_CORNERS);

  correspondences = (double *) malloc(num_frm_corners * 4 *
                                      sizeof(*correspondences));
  num_correspondences = determine_correspondence(frm->y_buffer,
                                                 (int*)frm_corners,
                                                 num_frm_corners,
                                                 ref->y_buffer,
                                                 (int*)ref_corners,
                                                 num_ref_corners,
                                                 frm->y_width, frm->y_height,
                                                 frm->y_stride, ref->y_stride,
                                                 correspondences);
  inlier_map = (int *) malloc(num_correspondences * sizeof(*inlier_map));
  num_inliers = compute_global_motion_params(type, correspondences,
                                             num_correspondences, H,
                                             inlier_map);
  free(correspondences);
  free(inlier_map);
  return (num_inliers > 0);
}
