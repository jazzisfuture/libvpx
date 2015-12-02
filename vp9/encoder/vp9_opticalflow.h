/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be
 *  found  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
   Structure to hold optical flow information. With and height are
   dimensions of the flow field. Confidence is a confidence metric
   for the optical flow estimation based on harris corner score. U and
   V are the computed flow vectors and are each height x width.
*/
typedef struct {
  int width, height;
  double *confidence;
  double *u;
  double *v;
} flowMV;

// destructor for the flowMV struct
void destruct_flowMV(flowMV *flow);

/*
  Top level interface for computing optical flow. This function
  takes in two images and returns the pixelwise motion field that
  describes the motion between the two images. n_levels corresponds
  to the number of levels of an image pyramid to use for coarse to fine
  refinement. If no coarse to fine refinement is desired, this should be
  set to 1.

  Note that the warping needs to be done by SUBTRACTING the flow vectors
  from the locations in the second image passed in (ref) in order to produce
  a prediction for the first image passed in (frm).

  To compute the motion vectors for a single pixel, see the function
  optical_flow_per_pixel in vp9_opticalflow.c.
*/
void compute_flow(unsigned char *frm, unsigned char *ref,
                  flowMV *flow, int width, int height,
                  int frm_stride, int ref_stride, int n_levels);
