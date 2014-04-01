#ifndef VP9_INTRA_RS_H
#define VP9_INTRA_RS_H
#include "vp9/common/vp9_enums.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_reconintra.h"
#include "vp9/common/vp9_idct.h"
#include "vp9/decoder/vp9_onyxd_int.h"
#include <dlfcn.h>
#define STABLE_BUFFER_SIZE_RS  4704000
#define MAX_TILE 4

typedef struct {
  uint8_t *token_cache;

  int16_t *dqcoeff;
  uint16_t *eobs;
  int plane_type;
  int subsampling_x;
  int subsampling_y;
  uint8_t *dst;
  int stride;

  int mode;
  int uv_mode;
  int ref_frame[2];
  int tx_size;
  unsigned char skip_coeff;
  int sb_type;
  int as_mode[4];


  int up_available;
  int left_available;

  int mb_to_left_edge;
  int mb_to_right_edge;
  int mb_to_top_edge;
  int mb_to_bottom_edge;

  int y_width;
  int y_height;
  int uv_width;
  int uv_height;
  int lossless;
  int bsize;
}INTRA_PARAM;

typedef struct {
  INTRA_PARAM *param;
  INTRA_PARAM *intra_arg[MAX_TILE];
} INTRA_OBJ;

typedef struct intra_predict_args_rs {
  MACROBLOCKD *xd;
  uint8_t *token_cache;
} INTRA_ARGS;

void vp9_intra_prepare_rs(void *func, MACROBLOCKD *xd,
                          VP9_DECODER_RECON *decoder_recon, int tile_num);
void vp9_init_intra_rs();
void vp9_release_intra_rs();
void vp9_do_intra_rs(int index, int block_count);
void vp9_check_intra_rs(VP9_COMMON *const cm, int tile_count);
extern int rs_intra_init;
#endif
