#ifndef TXFM2D_C_H_
#define TXFM2D_C_H_

#include "vp10/common/vp10_txfm2d_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif

void vp10_fdct_2d_c(const int16_t *input, int32_t *output, int stride,
                    const TXFM_2D_CFG *cfg);
void vp10_idct_2d_add_c(const int32_t *input, uint16_t *output, int stride,
                        const TXFM_2D_CFG *cfg);
#ifdef __cplusplus
}
#endif
#endif  // TXFM2D_C_H_
