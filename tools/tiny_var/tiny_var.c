// Compute variance for each block
// EXCEPT for blocks which are partially in the UMV

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

typedef enum BLOCK_SIZE_TYPE {
  BLOCK_SIZE_AB4X4, BLOCK_4X4 = BLOCK_SIZE_AB4X4,
  BLOCK_SIZE_SB4X8, BLOCK_4X8 = BLOCK_SIZE_SB4X8,
  BLOCK_SIZE_SB8X4, BLOCK_8X4 = BLOCK_SIZE_SB8X4,
  BLOCK_SIZE_SB8X8, BLOCK_8X8 = BLOCK_SIZE_SB8X8,
  BLOCK_SIZE_SB8X16, BLOCK_8X16 = BLOCK_SIZE_SB8X16,
  BLOCK_SIZE_SB16X8, BLOCK_16X8 = BLOCK_SIZE_SB16X8,
  BLOCK_SIZE_MB16X16, BLOCK_16X16 = BLOCK_SIZE_MB16X16,
  BLOCK_SIZE_SB16X32, BLOCK_16X32 = BLOCK_SIZE_SB16X32,
  BLOCK_SIZE_SB32X16, BLOCK_32X16 = BLOCK_SIZE_SB32X16,
  BLOCK_SIZE_SB32X32, BLOCK_32X32 = BLOCK_SIZE_SB32X32,
  BLOCK_SIZE_SB32X64, BLOCK_32X64 = BLOCK_SIZE_SB32X64,
  BLOCK_SIZE_SB64X32, BLOCK_64X32 = BLOCK_SIZE_SB64X32,
  BLOCK_SIZE_SB64X64, BLOCK_64X64 = BLOCK_SIZE_SB64X64,
  BLOCK_SIZE_TYPES, BLOCK_MAX_SB_SEGMENTS = BLOCK_SIZE_TYPES
} BLOCK_SIZE_TYPE;

typedef struct {
  int64_t sum_square_error;
  int64_t sum_error;
  int count;
  int variance;
} planevar;

typedef struct {
  planevar plane[3];
} var;

typedef struct {
  var none;
  var horz[2];
  var vert[2];
} partition_variance;

#define VT(TYPE, BLOCKSIZE) \
  typedef struct { \
    partition_variance vt; \
    BLOCKSIZE split[4]; } TYPE;

VT(v8x8, var)
VT(v16x16, v8x8)
VT(v32x32, v16x16)
VT(v64x64, v32x32)

typedef struct {
  partition_variance *vt;
  var *split[4];
} vt_node;

typedef enum {
  V16X16,
  V32X32,
  V64X64,
} TREE_LEVEL;

static void tree_to_node(void *data, BLOCK_SIZE_TYPE block_size, vt_node *node) {
  int i;
  switch (block_size) {
    case BLOCK_SIZE_SB64X64: {
      v64x64 *vt = (v64x64 *) data;
      node->vt = &vt->vt;
      for (i = 0; i < 4; i++)
        node->split[i] = &vt->split[i].vt.none;
      break;
    }
    case BLOCK_SIZE_SB32X32: {
      v32x32 *vt = (v32x32 *) data;
      node->vt = &vt->vt;
      for (i = 0; i < 4; i++)
        node->split[i] = &vt->split[i].vt.none;
      break;
    }
    case BLOCK_SIZE_MB16X16: {
      v16x16 *vt = (v16x16 *) data;
      node->vt = &vt->vt;
      for (i = 0; i < 4; i++)
        node->split[i] = &vt->split[i].vt.none;
      break;
    }
    case BLOCK_SIZE_SB8X8: {
      v8x8 *vt = (v8x8 *) data;
      node->vt = &vt->vt;
      for (i = 0; i < 4; i++)
        node->split[i] = &vt->split[i];
      break;
    }
    default:
      node->vt = 0;
      for (i = 0; i < 4; i++)
        node->split[i] = 0;
      assert(-1);
  }
}

// Set variance values given sum square error, sum error, count.
static void fill_variance(planevar *v, int64_t s2, int64_t s, int c) {
  v->sum_square_error = s2;
  v->sum_error = s;
  v->count = c;
  if (c > 0) {
#if 1
    v->variance = 256
        * (v->sum_square_error - v->sum_error * v->sum_error / v->count)
        / v->count;
#else
    v->variance = (v->sum_square_error - v->sum_error * v->sum_error / v->count);
#endif
  }
  else
    v->variance = 0;
}

// Combine 2 variance structures by summing the sum_error, sum_square_error,
// and counts and then calculating the new variance.
void sum_2_variances(var *r, var *a, var*b) {
  int p;
  for (p = 0; p < 3; p++)
    fill_variance(&r->plane[p], a->plane[p].sum_square_error + b->plane[p].sum_square_error,
                  a->plane[p].sum_error + b->plane[p].sum_error, a->plane[p].count + b->plane[p].count);
}

static void fill_variance_tree(void *data, BLOCK_SIZE_TYPE block_size) {
  vt_node node;
  tree_to_node(data, block_size, &node);
  sum_2_variances(&node.vt->horz[0], node.split[0], node.split[1]);
  sum_2_variances(&node.vt->horz[1], node.split[2], node.split[3]);
  sum_2_variances(&node.vt->vert[0], node.split[0], node.split[2]);
  sum_2_variances(&node.vt->vert[1], node.split[1], node.split[3]);
  sum_2_variances(&node.vt->none, &node.vt->vert[0], &node.vt->vert[1]);
}

void sse_sum(uint8_t *src, int stride, int width, int height,
             unsigned int *sse, int *sum)
{
  int y, x;
  for (y = 0 ; y < height; y++) {
    for (x = 0; x < width; x++) {
      *sum += src[x];
      *sse += src[x] * src[x];
    }
    src += stride;
  }
}

#define var_y(v) (v)->plane[0].variance
#define var_yuv(v) ((v)->plane[0].variance + ((v)->plane[1].variance + (v)->plane[2].variance) / 4)

int main(int argc, char **argv)
{
  int b, i, j, k, x, y;
  FILE *f;
  FILE *of[8];
  char oname[8][10000];
  uint8_t *buf;
  uint8_t *s[3];
  int w, h, n_frames;
  v64x64 vt;
  char *y_format = "%s.var-y-%d.dat";
  char *yuv_format = "%s.var-yuv-%d.dat";

  memset(&vt, 0, sizeof(vt));

  if (argc != 3) {
    fprintf(stderr, "Usage: %s filename.yuv WxH\n", argv[0]);
    return 1;
  }

  f = strcmp(argv[1], "-") ? fopen(argv[1], "rb") : stdin;
  for (b = 0; b < ARRAY_SIZE(of); b++) {
    char *format = b % 2 ? yuv_format : y_format;
    snprintf(oname[b], sizeof(oname[b]), format, argv[1], 1 << (b/2 + 3));
    of[b] = fopen(oname[b], "w");
  }
  sscanf(argv[2], "%dx%d", &w, &h);
  if (!f) {
    fprintf(stderr, "Could not open input files: %s\n",
            strerror(errno));
    return 1;
  }
  if (w <= 0 || h <= 0 || w & 1 || h & 1) {
    fprintf(stderr, "Invalid size %dx%d\n", w, h);
    return 1;
  }

  buf = malloc(w * h * 3 / 2);

  for (n_frames = 0; ; n_frames++) {
    if (!fread(buf, w * h * 3 / 2, 1, f))
      break;

    for (y = 0; y < h; y += 64) {
      for (x = 0; x < w; x += 64) {
        s[0] = buf                 + w   * y     + x;
        s[1] = buf + w * h         + w/2 * y / 2 + x / 2;
        s[2] = buf + w * h * 5 / 4 + w/2 * y / 2 + x / 2;

        for (i = 0; i < 4; i++) {
          const int x32_idx = ((i & 1) << 5);
          const int y32_idx = ((i >> 1) << 5);
          for (j = 0; j < 4; j++) {
            const int x16_idx = x32_idx + ((j & 1) << 4);
            const int y16_idx = y32_idx + ((j >> 1) << 4);
            v16x16 *vst = &vt.split[i].split[j];
            for (k = 0; k < 4; k++) {
              int x_idx = x16_idx + ((k & 1) << 3);
              int y_idx = y16_idx + ((k >> 1) << 3);
              unsigned int sse = 0;
              int sum = 0;
              int p;
              int in_frame = x + x_idx + 8 <= w && y + y_idx + 8 <= h;

              if (in_frame)
                sse_sum(s[0] + w * y_idx + x_idx, w, 8, 8, &sse, &sum);
              fill_variance(&vst->split[k].vt.none.plane[0], sse, sum, 64);

              for (p = 1; p < 3; p++) {
                int x_idx_c = x_idx / 2;
                int y_idx_c = y_idx / 2;
                sse = sum = 0;

                if (in_frame)
                  sse_sum(s[p] + w / 2 * y_idx_c + x_idx_c, w / 2, 4, 4, &sse, &sum);
                fill_variance(&vst->split[k].vt.none.plane[p], sse, sum, 16);
              }
              fprintf(of[0], "%d ", var_y(&vst->split[k].vt.none));
              fprintf(of[1], "%d ", var_yuv(&vst->split[k].vt.none));
            }
            fill_variance_tree(vst, BLOCK_SIZE_MB16X16);
            fprintf(of[2], "%d ", var_y(&vst->vt.none));
            fprintf(of[3], "%d ", var_yuv(&vst->vt.none));
          }
          fill_variance_tree(&vt.split[i], BLOCK_SIZE_SB32X32);
          fprintf(of[4], "%d ", var_y(&vt.split[i].vt.none));
          fprintf(of[5], "%d ", var_yuv(&vt.split[i].vt.none));
        }
        fill_variance_tree(&vt, BLOCK_SIZE_SB64X64);
        fprintf(of[6], "%d ", var_y(&vt.vt.none));
        fprintf(of[7], "%d ", var_yuv(&vt.vt.none));
      }
    }

    for (b = 0; b < ARRAY_SIZE(of); b++)
      fprintf(of[b], "\n");
  }

  free(buf);
  if (strcmp(argv[1], "-"))
    fclose(f);

  for (b = 0; b < ARRAY_SIZE(of); b++)
    fclose(of[b]);
  return 0;
}
