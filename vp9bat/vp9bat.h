#ifndef VP9BAT_H
#define VP9BAT_H

enum VideoInfoMode {
  MODE_PRED,
  MODE_RES,
  MODE_RECON,
  MODE_LF,
  MODE_YUV,
  MODE_HEAT,
  MODE_EFF,
  MODE_PSNR,
  NUM_MODES
};

enum ComponentSamples {
  SAMPLES_Y,
  SAMPLES_U,
  SAMPLES_V,
};




#endif
