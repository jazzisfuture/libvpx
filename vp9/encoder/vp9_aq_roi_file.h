#ifndef VPX_VP9_ENCODER_VP9_AQ_ROI_FILE_H_
#define VPX_VP9_ENCODER_VP9_AQ_ROI_FILE_H_

#include "./vpx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_ROI_FILE_AQ

struct VP9_COMP;

int vp9_subtitles_setup(struct VP9_COMP *const cpi);

void vp9_subtitles_print_segmentation_map(struct VP9_COMP *const cpi);

#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif