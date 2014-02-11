#ifndef RS_LF_H
#define RS_LF_H
extern "C" {

extern void lf_rs(const LOOP_FILTER_MASK* lfm,
                  const cuda_loop_filter_thresh* lft,
                  uint32_t rows,
                  uint32_t cols,
                  uint8_t * buf,
                  uint32_t uv_rows,
                  uint32_t uv_cols,
                  uint8_t * u_buf,
                  uint8_t * v_buf);
}

#endif
