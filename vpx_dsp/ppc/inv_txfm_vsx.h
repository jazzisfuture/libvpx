#include "vpx_dsp/ppc/types_vsx.h"

void round_store4x4_vsx(int16x8_t *in, int16x8_t *out, uint8_t *dest,
                        int stride);
void idct4_vsx(int16x8_t *in, int16x8_t *out);
void iadst4_vsx(int16x8_t *in, int16x8_t *out);

void round_store8x8_vsx(int16x8_t *in, uint8_t *dest, int stride);
void idct8_vsx(int16x8_t *in, int16x8_t *out);
void iadst8_vsx(int16x8_t *in, int16x8_t *out);
