/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed  by a BSD-style license that can be
 *  found in the LICENSE file in the root of the source tree. An additional
 *  intellectual property  rights grant can  be found in the  file PATENTS.
 *  All contributing  project authors may be  found in the AUTHORS  file in
 *  the root of the source tree.
 */

/*
 *  \file vp9_alt_ref_aq_private.h
 *
 *  # Motivation
 *
 *      Altref  frames  are special  because  of  their non-visibility  and
 *  their  absence in  the  original unencoded  video  stream. In terms  of
 *  the  rate-distortion optimization  this  specifity  implies that  their
 *  inroduced rate is similar to the one  of common frames, but we can only
 *  implicitly  derive  their  introduced distortion  by  measuring  visual
 *  quality of  the future frames or  the combination of their  own quality
 *  and their future frame prediction power.
 *      Considering that altref frames usually  consume about a half of the
 *  entire bitrate allocated by the rate-control for non-key frames, it can
 *  be  interesting to  vary their  qp value  spatially in  accordance with
 *  their future-frame prediction ability.
 *      Having the  currect vp9  infrastructure this feature  is relatively
 *  easy to implement using segmentation map with delta-quantizer segments.
 *
 *
 *  # The approach
 *
 *  ## Creation of the prediction-power map
 *
 *      Encoder  creates the  new  altref frame  blurring together  several
 *  future frames,  i.e. lookahead  buffers (see  vp9_temporal_filter.c for
 *  details).  Technically,  encoder splits the most  distant sampled frame
 *  into the grid  of 16 by 16  blocks and compensate every  block with the
 *  correponding  blocks  from the  other  lookahead  buffers using  motion
 *  search.  Encoder  avarages blocks  that are similar  after compensation
 *  using discrete set of weights, namely 0,1 or 2.
 *      The  described process  of  the altref  creation naturally  defines
 *  per-blocks prediction-power estimates. For the blocks that are an exact
 *  copies from the-most-distant-frame  we do not want  to assign quantizer
 *  much better than the one  rate-control picks for their frame. Likewise,
 *  for the blocks  that encoder equally averages among  all sampled frames
 *  we have to rely on the quantizer that rate control picks for the altref
 *  frame. For any  other blocks it makes  sense to pick some  quantizer in
 *  between those  two. Currently, I use constant  change between segments,
 *  although I guess it is not the most appropriate interpolation.
 *
 *
 *  ## Fitting the macroblock-tree construction
 *
 *      To  fit the  existing macroblock-tree  creation process  I numerate
 *  delta-quantizer segments  from the one with  lowest delta-quantizer (in
 *  every reasonable  case just  zero quantizer)  to the  highest (usually,
 *  little lower than the one for the common frames).
 *      In  addition I  force rate-distortion-optimization  macroblock-tree
 *  construction  algorithm  (see  vp9_encodeframe.c  for  details)  to  be
 *  tolerant to patches with non-zero delta  quantizers as we indeed do not
 *  want to  penalize them for  having greater distortion. Currently  I use
 *  quite dumb  method, but in my  experiments it works totally  fine and I
 *  just need to  check that it does not requires  too much computations (I
 *  think it actually does).
 *
 *
 *  ## No-segmentation cases
 *
 *      Surprisingly, few of my test videos has at least some altref frames
 *  that were totally copied from the main lookahead buffer. In this case I
 *  do not  enable any segmentation and  just change frame qp  to match the
 *  one for the main lookahead buffer (actually, little lower).
 *
 *
 *  ## Interaction with other segmentation modes
 *
 *      The implementation of the special altref segmentation is compatible
 *  with  all common-frame  segmentations. Essentially,  having the  altref
 *  segmentation  forces encoder  to  update segmentation  map after  every
 *  altref frame.
 *
 *
 *  ## Drawbacks of the basic estimation
 *
 *      Although  theoretically the  described  above  approach looks  very
 *  promising,  I  found  during  my  experiments  that  the  only  way  to
 *  outperform the original encoder's rate-distortion ratio is to be pretty
 *  conservative. Precisely:
 *
 *  * Keep  the highest  delta quantizer little  lower than  the difference
 *    between the one for common frames and altref's one.
 *
 *  * Disable all intermidiate segments,  i.e. only keep segments with zero
 *    and maximum delta quantizer.
 *
 *  * Do not consider unit weights during altref creation.
 *
 *      Being conservative helps to beat  the original encoder with respect
 *  to  rate-distortion ratio  on  a FullHD  resolution  dataset. I do  not
 *  recommend to enable altref segmentation for the smaller resolution now.
 *      Trying to explain the need to be conservative I can suggest that:
 *
 *  * Lookahead buffers do not ideally represent all forthcoming frames.
 *
 *  * The fact that I estimate the  altref segmentation map for the grid of
 *    16  by 16  blocks introduces  some visual  quality degradation  since
 *    motion  compensation won't  respect this  grid and  actually operates
 *    over blocks up to 8 by 8 small. This problem should be worse for small
 *    resolutions.
 *
 *  * Forcing  encoder to  update segmentation  map I  introduce noticeable
 *    overhead for storing it. According to my experiments, the overhead on
 *    highly textured  videos is comparable  with the  gain I get  even for
 *    high target bitrates.
 *
 *
 *  # Code structure
 *
 *      The  main   logic  live  in   the  ALT_REF_AQ  struct   defined  in
 *  vp9_alt_ref_aq.c and vp9_alt_ref_aq_private.h files.  The public header
 *  vp9_alt_ref_aq.h  contains  forward  declarations for  the  struct  and
 *  functions  belonging  to it. This  should  be  helpful for  incremental
 *  compilation and it also makes code clearer by separating implementation
 *  from the interface.
 *      Inside the  vp9_alt_ref_aq.c I frequently  use MATX struct  which I
 *  created  resembling OpenCV  Matx class. It  is  meant to  be used  with
 *  relatively large images (as it  introduces some overhead for processing
 *  meta data). The  struct is  type-templated using C  preprocessor. I did
 *  not really use this feature, but it was fun to design.
 *
 *
 *  # Quantitative evaluation
 *
 *      I run  all my tests over  stdhd dataset with the  following command
 *  line parameters `--debug --cpu-used=2 --aq-mode=0 --alt-ref-aq=-1`. The
 *  best results are about 1.2 overall BDRate gain (-0.95 Avg) for the SSIM
 *  quality measure with 0.4 overall  BDRate gain (-0.36 Avg) attributed to
 *  the no-segmentation cases.
 *      PSNR values  are worse  (on some  videos up  to the  opposite), but
 *  comparing  the  results  visually  I  would say  that  I  should  trust
 *  SSIM. Thus, I do not report here PSNR values.
 */

#ifndef VP9_ENCODER_VP9_ALT_REF_AQ_PRIVATE_H_
#define VP9_ENCODER_VP9_ALT_REF_AQ_PRIVATE_H_

#include "vpx/vpx_integer.h"

#include "vp9/common/vp9_seg_common.h"

#include "vp9/common/vp9_enums.h"
#include "vp9/common/vp9_matx.h"

#include "vp9/encoder/vp9_encoder.h"

#ifdef __cplusplus

extern "C" {
#endif

struct ALT_REF_AQ {
  // private member used for segment ids convertion
  int __segment_changes[ALT_REF_MAX_FRAMES + 1];

  // private member used to compute qdelta steps
  int __nsteps;

  // number of delta-quantizer segments,
  // it can be different from nsteps
  // because of the range compression
  int nsegments;

  // the greater this number is, the lower
  // delta quantizer between segments is
  float DELTA_SHRINK;

  // the same as DELTA_SHRINK, but for the case,
  // when the entire frame gets same qdelta
  float SINGLE_SEGMENT_DELTA_SHRINK;

  // either DELTA_SHRINK or SINGLE_FRAME_DELTA_SHRINK
  float delta_shrink;

  // number of segments to assign
  // nonzero delta quantizers
  int NUM_NONZERO_SEGMENTS;

  // maximum segment area to be dropped
  // TODO(yury): May be, I need something smarter here
  float SEGMENT_MIN_AREA;

  // single qdelta step between segments
  float single_delta;

  // qdelta[i] = (int) single_delta*segment_deltas[i];
  // ------------------------------------------------
  // qdelta[0] has single delta quantizer
  // for the frame in the case we need it
  int segment_deltas[ALT_REF_MAX_FRAMES];

  // this one is just for debugging purposes
  int alt_ref_number;

  // basic aq mode (I keep original
  // aq mode when encoding altref frame)
  AQ_MODE aq_mode;

  // wrapper around basic segmentation_map
  struct MATX_8U cpi_segmentation_map;

  // altref segmentation
  struct MATX_8U segmentation_map;

  // distribution of the segment ids
  int segment_hist[ALT_REF_MAX_FRAMES];
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_ALT_REF_AQ_PRIVATE_H_
