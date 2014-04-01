#ifndef __FILTERS_DSP_ASM_H_
#define __FILTERS_DSP_ASM_H_

#include <stdint.h>

void filter16_t_dsp(uint8_t op7, uint8_t *op6, uint8_t *op5, uint8_t *op4,
    uint8_t *op3, uint8_t *op2, uint8_t *op1, uint8_t *op0, uint8_t *oq0,
    uint8_t *oq1, uint8_t *oq2, uint8_t *oq3, uint8_t *oq4, uint8_t *oq5,
    uint8_t *oq6, uint8_t oq7);

void filter16_v_dsp(uint8_t *data);

#endif /* __FILTERS_DSP_ASM_H_ */
