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
 *  \file vp9_alt_ref_aq.h
 *
 *  This file  public interface  for setting  up adaptive  segmentation for
 *  altref frames.  Go to alt_ref_aq_private.h for implmentation details.
 */

#ifndef VP9_ENCODER_VP9_ALT_REF_AQ_H_
#define VP9_ENCODER_VP9_ALT_REF_AQ_H_

#ifdef __cplusplus

extern "C" {
#endif


struct VP9_COMP;
struct ALT_REF_AQ;


/*!\brief Constructor
 *
 * \param    self    Instance of the class
 */
struct ALT_REF_AQ* vp9_alt_ref_aq_create();

/*!\brief Set up segmentation mode based on future frames prediction quality
 *
 * This  function   expects  that  internal  segmentation   map  is  already
 * created  somewhere (probably,  inside vp9_temporal_filter.c),  it applies
 * some  postprocessing,  i.e.,  range compression,  inversion,  and  change
 * cpi->oxcf.aq_mode to LOOKAHEAD_AQ.
 *
 * \param    self    Instance of the class
 * \param    cpi     Encoder context
 */
void vp9_alt_ref_aq_make_map(struct ALT_REF_AQ* const self,
                             struct VP9_COMP* const cpi);

/*!\brief Set up encoder context segmentation to the prediction-based one
 *
 * \param    self    Instance of the class
 * \param    cpi     Encoder context
 */
void vp9_alt_ref_aq_setup(struct ALT_REF_AQ* const self,
                          struct VP9_COMP* const cpi);

/*!\brief Restore main segmentation map mode and reset the class variables
 *
 * \param    self    Instance of the class
 * \param    cpi     Encoder context
 */
void vp9_alt_ref_aq_unset(struct ALT_REF_AQ* const self,
                          struct VP9_COMP* const cpi);

/*!\brief Destructor
 *
 * \param    self    Instance of the class
 */
void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ* const self);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_ALT_REF_AQ_H_
