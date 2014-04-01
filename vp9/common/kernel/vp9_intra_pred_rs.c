#include "vp9/common/kernel/vp9_intra_pred_rs.h"

typedef struct {
  void *lib_handle;
  int (*init_intra_rs)(int frame_size, int tile_count,
                 uint8_t *param_buf, int param_size);

  void (*release_intra_rs)(int tile_count);
  void (*invoke_intra_rs)(int tile_index,
                          int block_cnt);
} INTRA_CONTEXT;

INTRA_CONTEXT intra_context;
INTRA_OBJ intra_obj;
int rs_intra_init = -1;

static inline void * load_rs_library() {
  void * lib_handle;
  lib_handle = dlopen("libvp9rsif.so", RTLD_NOW);
  if (lib_handle == NULL) {
    printf("%s\n", dlerror());
  }
  return lib_handle;
}

static int init_library() {
  intra_context.lib_handle = load_rs_library();
  intra_context.init_intra_rs = dlsym(intra_context.lib_handle,
                                   "init_intra_rs");
  if (intra_context.init_intra_rs == NULL) {
    printf("get init_intra_rs failed %s\n", dlerror());
    return -1;
  }
  intra_context.invoke_intra_rs = dlsym(intra_context.lib_handle,
                                           "invoke_intra_rs");
  if (intra_context.invoke_intra_rs == NULL) {
    printf("get invoke_intra_rs %s\n", dlerror());
    return -1;
  }
  intra_context.release_intra_rs = dlsym(intra_context.lib_handle,
                                      "release_intra_rs");
  if (intra_context.release_intra_rs == NULL) {
    printf("get release_intra_rs %s\n", dlerror());
    return -1;
  }
  return 0;
}

static void init_buffer(int tile_count, int each_count) {
  int i = 0;
  intra_obj.param = (INTRA_PARAM*)malloc(sizeof(INTRA_PARAM)
                    * each_count * tile_count);
  for (i = 0; i < tile_count; i++) {
    intra_obj.intra_arg[i] = &intra_obj.param[i * each_count];
  }
}

static void vp9_init_buffer(int tile_cnt, int framesize) {
  int max_cnt = framesize >> 4;
  init_buffer(tile_cnt, max_cnt);
  intra_context.init_intra_rs(framesize, tile_cnt, (uint8_t *)intra_obj.param,
                        sizeof(INTRA_PARAM) * max_cnt);
}

void vp9_do_intra_rs(int index, int block_count) {
  intra_context.invoke_intra_rs(index, block_count);
}


void vp9_check_intra_rs(VP9_COMMON *const cm, int tile_count) {
  const YV12_BUFFER_CONFIG *cfg_source;
  static int checked = 0;
  if (checked) return;
  if (tile_count >= 8) {
    checked = 1;
    rs_intra_init = -1;
    vp9_release_intra_rs();
    return;
  }
  cfg_source = &cm->yv12_fb[cm->new_fb_idx];

  if (cfg_source->buffer_alloc_sz > STABLE_BUFFER_SIZE_RS) {
    vp9_release_intra_rs();
    vp9_init_buffer(tile_count, cfg_source->buffer_alloc_sz);
  }
  checked = 1;
  return;
}

void vp9_init_intra_rs() {
  char *rs_enable = getenv("RSENABLE");
  if (rs_enable) {
    if (*rs_enable == '1') {
      rs_intra_init = 0;
    } else {
      rs_intra_init = -1;
      return;
    }
  }
  if (init_library()) {
    rs_intra_init = -1;
    printf("init intra rs failed, back to cpu \n");
    return;
  }
  vp9_init_buffer(MAX_TILE, STABLE_BUFFER_SIZE_RS);
}

void vp9_release_intra_rs() {
  if (rs_intra_init == 0) {
    if (intra_obj.param != NULL) {
      free(intra_obj.param);
    }
    intra_context.release_intra_rs(MAX_TILE);
  }
}

static void assign_param_rs(INTRA_ARGS *arg, int bsize, int index,
    int tile_index) {
  int i;
  INTRA_PARAM *base = intra_obj.intra_arg[tile_index];
  INTRA_PARAM *dst = base + index * MAX_MB_PLANE;
  MACROBLOCKD *src = arg->xd;
  for (i = 0; i < MAX_MB_PLANE; i++) {
    dst[i].dqcoeff = src->plane[i].dqcoeff;
    dst[i].eobs = src->plane[i].eobs;
    dst[i].bsize = bsize;
    dst[i].plane_type = src->plane[i].plane_type;
    dst[i].subsampling_x = src->plane[i].subsampling_x;
    dst[i].subsampling_y = src->plane[i].subsampling_y;
    dst[i].dst = src->plane[i].dst.buf;
    dst[i].stride = src->plane[i].dst.stride;

    dst[i].up_available = src->up_available;
    dst[i].left_available = src->left_available;
    dst[i].mb_to_left_edge = src->mb_to_left_edge;
    dst[i].mb_to_right_edge = src->mb_to_right_edge;
    dst[i].mb_to_top_edge = src->mb_to_top_edge;
    dst[i].mb_to_bottom_edge = src->mb_to_bottom_edge;
    dst[i].lossless = src->lossless;
    dst[i].y_width = src->cur_buf->y_width;
    dst[i].y_height = src->cur_buf->y_height;
    dst[i].uv_width = src->cur_buf->uv_width;
    dst[i].uv_height = src->cur_buf->uv_height;

    dst[i].mode = src->mi_8x8[0]->mbmi.mode;
    dst[i].uv_mode = src->mi_8x8[0]->mbmi.uv_mode;
    dst[i].tx_size = src->mi_8x8[0]->mbmi.tx_size;
    dst[i].skip_coeff = src->mi_8x8[0]->mbmi.skip_coeff;
    dst[i].sb_type = src->mi_8x8[0]->mbmi.sb_type;
    dst[i].ref_frame[0] = src->mi_8x8[0]->mbmi.ref_frame[0];
    dst[i].ref_frame[1] = src->mi_8x8[0]->mbmi.ref_frame[1];

    dst[i].as_mode[0] = src->mi_8x8[0]->bmi[0].as_mode;
    dst[i].as_mode[1] = src->mi_8x8[0]->bmi[1].as_mode;
    dst[i].as_mode[2] = src->mi_8x8[0]->bmi[2].as_mode;
    dst[i].as_mode[3] = src->mi_8x8[0]->bmi[3].as_mode;

    dst[i].token_cache = arg->token_cache;
  }
}

void vp9_intra_prepare_rs(void *func, MACROBLOCKD *xd,
                          VP9_DECODER_RECON *decoder_recon, int tile_num) {
  int i = 0, bsize = 0;
  int index = 0;
  for (index = 0; index < decoder_recon->intra_blocks_count; index++) {
    INTRA_PRE_RECON *intra =
      &decoder_recon->intra_pre_recon[index];
    struct intra_predict_args_rs args = {
      xd, decoder_recon->token_cache
    };
    xd->mb_to_left_edge = intra->mb_to_left_edge;
    xd->mb_to_right_edge = intra->mb_to_right_edge;
    xd->mb_to_top_edge = intra->mb_to_top_edge;
    xd->mb_to_bottom_edge = intra->mb_to_bottom_edge;

    xd->up_available = intra->up_available;
    xd->left_available = intra->left_available;

    for (i = 0; i < MAX_MB_PLANE; i++) {
      xd->plane[i].dst.buf = intra->dst[i];
      xd->plane[i].dqcoeff =
          decoder_recon->dequant_recon[intra->qcoeff_flag].qcoeff[i] +
          intra->offset;
      xd->plane[i].eobs =
          decoder_recon->dequant_recon[intra->qcoeff_flag].eobs[i] +
          intra->offset / 16;
    }
    xd->mi_8x8 = intra->mi_8x8;
    xd->itxm_add = func;
    xd->lossless = intra->lossless;
    bsize = intra->bsize;
    assign_param_rs(&args, bsize, index, tile_num);
  }
}
