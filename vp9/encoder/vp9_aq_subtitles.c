#include "vp9/encoder/vp9_aq_subtitles.h"

#include "vp9_encoder.h"
#include "vp9_firstpass.h"
#include "vp9_segmentation.h"

#define NO_SUB 0
#define SUB 1

#define QINDEX_DELTA -10

#define FILE_NAME "subtitles.txt"

int vp9_subtitles_setup(VP9_COMP *const cpi) {
  int start_x = 0, start_y = 0, end_x = 0, end_y = 0, frame_start = 0,
      frame_end = 0;
  int i, j;

  VP9_COMMON *cm = &cpi->common;
  struct segmentation *seg = &cm->seg;
  unsigned char *const seg_map = cpi->segmentation_map;

  FILE *fp;

  // Reset segmentation struct
  vp9_enable_segmentation(seg);
  vp9_clearall_segfeatures(seg);
  seg->abs_delta = SEGMENT_DELTADATA;

  // Clear segment
  memset(seg_map, NO_SUB, cm->mi_rows * cm->mi_cols);

  // Open file and read each line of segments where subtitles exist and frame
  // duration
  fp = fopen(FILE_NAME, "r");
  
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
        *(seg_map + i * cm->mi_cols + j) = SUB;
      }
    }
  }

  // Disable feature for no subtitles region to set delta_Q to 0
  vp9_disable_segfeature(seg, NO_SUB, SEG_LVL_ALT_Q);
  if ((cm->base_qindex + QINDEX_DELTA) > 0) {
    // increase quality of subtitle region
    vp9_enable_segfeature(seg, SUB, SEG_LVL_ALT_Q);
    vp9_set_segdata(seg, SUB, SEG_LVL_ALT_Q, QINDEX_DELTA);
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
