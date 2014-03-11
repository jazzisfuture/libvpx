#ifndef VP_RS_PACKING_H
#define VP_RS_PACKING_H

#ifdef __cplusplus
extern "C" {
#endif

int init_rs(int frame_size, int param_size, int tile_index,
            uint8_t *ref_buf, uint8_t *param);

void release_rs(int tile_count);

void invoke_inter_rs(int fri_count, int sec_count, int offset, int index);

#ifdef __cplusplus
}
#endif

#endif
