#ifndef VP_RS_PACKING_H
#define VP_RS_PACKING_H

#ifdef __cplusplus
extern "C" {
#endif
double now_ms();
int init_rs(int pred_param_size, int param_index_size, int buff_size,
            int pool_size, int global_size, int tile_index,
            uint8_t *block_param, uint8_t *index_param, uint8_t *dst_buf,
            uint8_t *mid_buf);
void release_rs(int tile_count);
void create_buffer_rs(int pred_param_size, int param_index_size,
                      int buf_size, int pool_size, int global_size,
                      int tile_count, uint8_t **block_param,
                      uint8_t **index_param, uint8_t **dst_buf,
                      uint8_t **mid_buf);
void release_rs(int tile_count);
void update_buffer_size_rs(int pred_param_size, int param_index_size,
                           int buf_size, int pool_size);

void release_buffer_rs(int tile_count);
void update_pool_rs(unsigned char *per_buf, int frame_num);
void invoke_inter_rs(unsigned char *dst_buf, unsigned char *block_param,
                     unsigned char *index_param, int tile_index,
                     int counts_8x8, int block_param_size,
                     int index_param_size);
void get_rs_res(unsigned char *dst_buf, int index);
#ifdef __cplusplus
}
#endif
#endif
