
#include "vp9/encoder/vp9_aq_roi_file.h"

#include "vp9_encoder.h"
#include "vp9_firstpass.h"
#include "vp9_segmentation.h"

#if CONFIG_ROI_FILE_AQ

enum roi_segments { no_sub, sub };

#define ROI_QINDEX_DELTA -10

#define ROI_AQ_FILE_NAME "subtitles.txt"

int vp9_subtitles_setup(VP9_COMP *const cpi) {
  int start_x = 0, start_y = 0, end_x = 0, end_y = 0, frame_start = 0,
      frame_end = 0;
  int i, j;
  enum roi_segments roi_segs;

  VP9_COMMON *cm = &cpi->common;
  struct segmentation *seg = &cm->seg;
  unsigned char *const seg_map = cpi->segmentation_map;

  FILE *fp;

  // Reset segmentation struct
  vp9_enable_segmentation(seg);
  vp9_clearall_segfeatures(seg);
  seg->abs_delta = SEGMENT_DELTADATA;

  // Clear segment
  roi_segs = no_sub;
  memset(seg_map, roi_segs, cm->mi_rows * cm->mi_cols);

  // Open file and read each line of segments where subtitles exist and frame
  // duration
  fp = fopen(ROI_AQ_FILE_NAME, "r");

  // File format: repeated lines of "start_x, start_y, end_x, end_y,
  // frame_start, frame_end\n"
  while (fscanf(fp, "%d,%d,%d,%d,%d,%d\n", &start_x, &start_y, &end_x, &end_y,
                &frame_start, &frame_end) == 6) {
    // see if current frame is within the set of frames that these subtitles
    // exist in
    int curr_frame = cm->current_frame_coding_index;
    if (curr_frame < frame_start || curr_frame > frame_end) {
      continue;
    }

    // pixel coordinates to segment coordinates
    start_x /= 8;
    start_y /= 8;
    end_x /= 8;
    end_y /= 8;

    // Set segment map values
    for (i = start_y; i <= end_y; i++) {
      for (j = start_x; j <= end_x; j++) {
        roi_segs = sub;
        *(seg_map + i * cm->mi_cols + j) = roi_segs;
      }
    }
  }

  fclose(fp);

  // Disable feature for no subtitles region to set delta_Q to 0
  roi_segs = no_sub;
  vp9_disable_segfeature(seg, roi_segs, SEG_LVL_ALT_Q);
  if ((cm->base_qindex + ROI_QINDEX_DELTA) > 0) {
    // increase quality of subtitle region
    roi_segs = sub;
    vp9_enable_segfeature(seg, roi_segs, SEG_LVL_ALT_Q);
    vp9_set_segdata(seg, roi_segs, SEG_LVL_ALT_Q, ROI_QINDEX_DELTA);
  }

  return 0;
}

// For debugging: print segmentation map
void vp9_subtitles_print_segmentation_map(VP9_COMP *const cpi) {
  int i, j;
  VP9_COMMON *const cm = &cpi->common;
  unsigned char *const seg_map = cpi->segmentation_map;
  for (i = 0; i < cm->mi_rows; i++) {
    for (j = 0; j < cm->mi_cols; j++) {
      printf("%x ", *(seg_map + (i * cm->mi_cols + j)));
    }
    printf("\n");
  }
  printf("\n");
}
#endif