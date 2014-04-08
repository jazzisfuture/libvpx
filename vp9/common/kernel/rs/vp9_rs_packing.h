#ifndef VP_RS_PACKING_H
#define VP_RS_PACKING_H

#ifdef __cplusplus
extern "C" {
#endif
int init_rs(int pred_param_size, int param_index_size, int buff_size,
            int pool_size, int global_size, int tile_count);

void release_rs(int tile_count);

void invoke_inter_rs(int tile_index, int counts_8x8);

uint8_t *get_alloc_ptr(int flag, int index);

void finish_rs();

#ifdef __cplusplus
}
#endif
#endif
