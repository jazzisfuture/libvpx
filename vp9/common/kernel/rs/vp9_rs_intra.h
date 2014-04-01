#ifndef VP_RS_INTRA_H
#define VP_RS_INTRA_H
#ifdef __cplusplus
extern "C" {
#endif

int init_intra_rs(int frame_size, int tile_count, uint8_t *param_buf,
                  int param_size);

void release_intra_rs(int tile_count);

void invoke_intra_rs(int tile_index, int block_cnt);

#ifdef __cplusplus
}
#endif

#endif
