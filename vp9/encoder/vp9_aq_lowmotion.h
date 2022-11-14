#ifndef VPX_VP9_ENCODER_VP9_AQ_LOWMOTION_H_
#define VPX_VP9_ENCODER_VP9_AQ_LOWMOTION_H_

#ifdef __cplusplus
extern "C" {
#endif

struct VP9_COMP;

int vp9_low_motion_setup(struct VP9_COMP *const cpi);

void vp9_low_motion_print_segmentation_map(struct VP9_COMP *const cpi);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif