#include "txfm_c.h"
#include "txfm2d_cfg.h"

void *vp10_alloc(int size, int align) { return malloc(size * align); }
void vp10_free(void *p) { free(p); }

void vp10_fdct_2d_c(const int16_t *input, int32_t *output, const int stride,
                    const TXFM_2D_CFG *cfg) {
  int i, j;
  const int txfm_size = cfg->txfm_size;
  const int8_t *shift = cfg->shift;
  const int8_t *stage_range_col = cfg->stage_range_col;
  const int8_t *stage_range_row = cfg->stage_range_row;
  const int8_t *cos_bit_col = cfg->cos_bit_col;
  const int8_t *cos_bit_row = cfg->cos_bit_row;
  const TxfmFunc txfm_func_col = cfg->txfm_func_col;
  const TxfmFunc txfm_func_row = cfg->txfm_func_row;

  int32_t *buf = vp10_alloc(txfm_size * txfm_size, sizeof(int32_t));
  int32_t *temp_in = vp10_alloc(txfm_size, sizeof(int32_t));
  int32_t *temp_out = vp10_alloc(txfm_size, sizeof(int32_t));

  // Columns
  for (i = 0; i < txfm_size; ++i) {
    for (j = 0; j < txfm_size; ++j) temp_in[j] = input[j * stride + i];
    round_shift_array(temp_in, txfm_size, -shift[0]);
    txfm_func_col(temp_in, temp_out, cos_bit_col, stage_range_col);
    round_shift_array(temp_out, txfm_size, -shift[1]);
    for (j = 0; j < txfm_size; ++j) buf[j * txfm_size + i] = temp_out[j];
  }

  // Rows
  for (i = 0; i < txfm_size; ++i) {
    for (j = 0; j < txfm_size; ++j) temp_in[j] = buf[j + i * txfm_size];
    txfm_func_row(temp_in, temp_out, cos_bit_row, stage_range_row);
    round_shift_array(temp_out, txfm_size, -shift[2]);
    for (j = 0; j < txfm_size; ++j)
      output[j + i * txfm_size] = (int32_t)temp_out[j];
  }

  vp10_free(buf);
  vp10_free(temp_in);
  vp10_free(temp_out);
}

void vp10_idct_2d_add_c(const int32_t *input, int16_t *output, int stride,
                        const TXFM_2D_CFG *cfg) {
  int i, j;
  const int txfm_size = cfg->txfm_size;
  const int8_t *shift = cfg->shift;
  const int8_t *stage_range_col = cfg->stage_range_col;
  const int8_t *stage_range_row = cfg->stage_range_row;
  const int8_t *cos_bit_col = cfg->cos_bit_col;
  const int8_t *cos_bit_row = cfg->cos_bit_row;
  const TxfmFunc txfm_func_col = cfg->txfm_func_col;
  const TxfmFunc txfm_func_row = cfg->txfm_func_row;

  int32_t *buf = vp10_alloc(txfm_size * txfm_size, sizeof(int32_t));
  int32_t *temp_in = vp10_alloc(txfm_size, sizeof(int32_t));
  int32_t *temp_out = vp10_alloc(txfm_size, sizeof(int32_t));

  int32_t *buf_ptr = buf;

  // Rows
  for (i = 0; i < txfm_size; ++i) {
    txfm_func_row(input, buf_ptr, cos_bit_row, stage_range_row);
    round_shift_array(buf_ptr, txfm_size, -shift[0]);
    input += txfm_size;
    buf_ptr += txfm_size;
  }

  // Columns
  for (i = 0; i < txfm_size; ++i) {
    for (j = 0; j < txfm_size; ++j) temp_in[j] = buf[j * txfm_size + i];
    txfm_func_col(temp_in, temp_out, cos_bit_col, stage_range_col);
    round_shift_array(temp_out, txfm_size, -shift[1]);
    for (j = 0; j < txfm_size; ++j) {
      output[j * stride + i] += temp_out[j];
    }
  }

  vp10_free(buf);
  vp10_free(temp_in);
  vp10_free(temp_out);
}
