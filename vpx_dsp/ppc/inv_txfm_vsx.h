#include "vpx_dsp/ppc/types_vsx.h"

static const uint8x16_t mask1 = {
  0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
};

#define PIXEL_ADD4(out, in) out = vec_sra(vec_add(in, add8), shift4);

#define PACK_STORE(v0, v1)                            \
  tmp16_0 = vec_add(vec_perm(d_u0, d_u1, mask1), v0); \
  tmp16_1 = vec_add(vec_perm(d_u2, d_u3, mask1), v1); \
  output_v = vec_packsu(tmp16_0, tmp16_1);            \
                                                      \
  vec_vsx_st(output_v, 0, tmp_dest);                  \
  for (i = 0; i < 4; i++)                             \
    for (j = 0; j < 4; j++) dest[j * stride + i] = tmp_dest[j * 4 + i];

void round_store4x4_vsx(int16x8_t *in, int16x8_t *out, uint8_t *dest,
                        int stride);
void idct4_vsx(int16x8_t *in, int16x8_t *out);
void iadst4_vsx(int16x8_t *in, int16x8_t *out);
