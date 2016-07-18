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
struct MATX_8U;
struct ALT_REF_AQ;


/*!\brief Constructor
 *
 * \return Instance of the class
 */
struct ALT_REF_AQ* vp9_alt_ref_aq_create();

/*!\brief Return pointer to the altref segmentation map
 *
 * \param    self  Instance of the class
 * \return         Header for the segmentation map
 */
struct MATX_8U* vp9_alt_ref_aq_segm_map(struct ALT_REF_AQ *self);

/*!\brief Set number of segments
 *
 * it is used for delta quantizer computations
 * and thus it can be larger than
 * maximum value of the segmentation map
 *
 * \param    self        Instance of the class
 * \param    nsegments   Maximum number of segments
 */
void vp9_alt_ref_aq_set_nsegments(struct ALT_REF_AQ *self, int nsegments);


/*!\brief Set up LOOKAHEAD_AQ segmentation mode
 *
 * Set up segmentation mode to LOOKAHEAD_AQ
 * (expected future frames prediction
 *  quality refering to the current frame).
 *
 * \param    self    Instance of the class
 * \param    cpi     Encoder context
 */
void vp9_alt_ref_aq_setup_mode(struct ALT_REF_AQ *self,
                               struct VP9_COMP *cpi);

/*!\brief Set up LOOKAHEAD_AQ segmentation map and delta quantizers
 *
 * \param    self    Instance of the class
 * \param    cpi     Encoder context
 */
void vp9_alt_ref_aq_setup_map(struct ALT_REF_AQ *self,
                              struct VP9_COMP *cpi);

/*!\brief Restore main segmentation map mode and reset the class variables
 *
 * \param    self    Instance of the class
 * \param    cpi     Encoder context
 */
void vp9_alt_ref_aq_unset_all(struct ALT_REF_AQ *self,
                              struct VP9_COMP *cpi);

/*!\brief Destructor
 *
 * \param    self    Instance of the class
 */
void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ *self);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_ALT_REF_AQ_H_
