#include "vp9/encoder/vp9_aq_lowmotion.h"

#include "vp9_encoder.h"
#include "vp9_firstpass.h"

#define SEGMENT_COUNT 2

#define REG_MOTION_IDX 0
#define LOW_MOTION_IDX 1

#define MAX_Q_VALUE 225
#define MAX_MI 4

#define RME_THRESHOLD 15.0

#define QINDEX_DELTA 30

#define FILE_NAME "output.txt"

int vp9_low_motion_setup(VP9_COMP *const cpi) {
  int i, j, k, l;
  VP9_COMMON *cm = &cpi->common;
  struct segmentation *seg = &cm->seg;

  // Reset segmentation struct
  vp9_enable_segmentation(seg);
  vp9_clearall_segfeatures(seg);
  seg->abs_delta = SEGMENT_DELTADATA;

  FILE *fp;
  int idx;
  double rme_delta;
  // cpi->segmentation_map = calloc(cm->mi_rows * cm->mi_cols, sizeof(unsigned char));
  unsigned char *const seg_map = cpi->segmentation_map;
  // printf("mb_rows %d , mb_cols %d\n", cm->mb_rows, cm->mb_cols);
  // printf("mi_rows %d , mi_cols %d\n", cm->mi_rows, cm->mi_cols);
  
  fp = fopen(FILE_NAME, "r");
  // RME deltas are for macroblocks, need to find the relative macroblock for every microblock
  // and assign segment ids accordingly
  for (i = 0; i < cm->mb_rows; i++) {
    for (j = 0; j < cm->mb_cols; j++) {
      // read the rme_delta value for each macroblock
      fscanf(fp, "%lf ", &rme_delta);
      // update the segment value for each microblock in the macroblock
      for (k = 0; k < 2; k++) {
        for (l = 0; l < 2; l++) {
          int y_idx = 2*i+k;
          int x_idx = 2*j+l;
          if (y_idx >= cm->mi_rows || x_idx >= cm->mi_cols) {
            continue;
          }
          *(seg_map + y_idx * cm->mi_cols + x_idx) = rme_delta > RME_THRESHOLD ? REG_MOTION_IDX : LOW_MOTION_IDX;
        }
      }
    }
    fscanf(fp, "\n");
  }

  fclose(fp);

  // Segment REG_MOTION is disabled so it defaults to baseline Q
  


  if ((cm->base_qindex + QINDEX_DELTA) > 0) { 
    // Segment LOW_MOTION should have a high Q index to reduce the rate in those blocks
    vp9_enable_segfeature(seg, LOW_MOTION_IDX, SEG_LVL_ALT_Q);
    vp9_enable_segfeature(seg, REG_MOTION_IDX, SEG_LVL_ALT_Q);

    vp9_set_segdata(seg, REG_MOTION_IDX, SEG_LVL_ALT_Q, -5);
    
    if (cm->frame_type == KEY_FRAME) {
      vp9_set_segdata(seg, LOW_MOTION_IDX, SEG_LVL_ALT_Q, 5);
      // printf("Key frame %d\n", cm->seg.feature_data[LOW_MOTION_IDX][0]);
    } else if (cpi->refresh_alt_ref_frame) {
      // seg->update_map=0;
      vp9_set_segdata(seg, LOW_MOTION_IDX, SEG_LVL_ALT_Q, 5);
      // printf("Altref %d\n", cm->seg.feature_data[LOW_MOTION_IDX][0]);
    } else {
      // seg->update_map = 0;
      vp9_set_segdata(seg, LOW_MOTION_IDX, SEG_LVL_ALT_Q, 5);
      // printf("Normal frame %d\n", cm->seg.feature_data[LOW_MOTION_IDX][0]);
    }
  }

  return 0;
}

void vp9_low_motion_print_segmentation_map(VP9_COMP *const cpi) {
  int i, j;
  VP9_COMMON *const cm = &cpi->common;
  unsigned char *const seg_map = cpi->segmentation_map;
  for (i = 0; i < cm->mi_rows; i++) {
    for (j = 0; j < cm->mi_cols; j++) {
      printf("%x ", *(seg_map+(i*cm->mi_cols+j)));
    }
    printf("\n");
  }
}