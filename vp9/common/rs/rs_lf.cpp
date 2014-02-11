#include <stdio.h>
#include <RenderScript.h>
#include "lf_types.h"
#include "rs_lf.h"
#include "obj/ScriptC_loopfilter.h"

#define Y_LVL_SHIFT 3
#define Y_STEP_MAX 8
#define Y_ROW_MULT 1
#define Y_BLOCK_DIM 64

#define UV_LVL_SHIFT 3
#define UV_STEP_MAX 8
#define UV_ROW_MULT 1
#define UV_BLOCK_DIM 64


using namespace android::RSC;

typedef struct context_rs {
  sp<Allocation> buf;
  sp<Allocation> raw_lfm;
  sp<Allocation> raw_lft;
  sp<RS> rs;

} context_rs;


bool is_rs_initialized = false;
context_rs context;

extern void lf_rs(const LOOP_FILTER_MASK* lfm,
                  const cuda_loop_filter_thresh* lft,
                  uint32_t rows,
                  uint32_t cols,
                  uint8_t * buf,
                  uint32_t uv_rows,
                  uint32_t uv_cols,
                  uint8_t * u_buf,
                  uint8_t * v_buf) {

  if (!is_rs_initialized) {
    context.rs = new RS();
    context.rs->init();
  }
  sp<const Element> const_e = Element::U8(context.rs);
  sp<Element> e = (Element*)const_e.get();
  Type::Builder *tb = NULL;
  sp<const Type> type;

  // allocate buffer for i/o
  tb = new Type::Builder(context.rs, e);
  tb->setX(rows * cols);
  type = tb->create();

  // copy over buffer
  context.buf = Allocation::createTyped(context.rs, type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT, NULL);

  context.buf->copy1DRangeFrom(0, rows * cols, buf);
  delete tb;

  // setup wavefront parameters
  uint32_t sb_rows = (rows + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_cols = (cols + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_count = sb_rows * sb_cols;
  uint32_t mask_bytes = sb_count * sizeof(LOOP_FILTER_MASK);

  uint32_t max = sb_rows + sb_cols - 1;

  // copy over masks
  tb = new Type::Builder(context.rs, e);
  tb->setX(mask_bytes);
  type = tb->create();

  context.raw_lfm = Allocation::createTyped(context.rs, type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT, NULL);

  context.raw_lfm->copy1DRangeFrom(0, mask_bytes, lfm);
  delete tb;

  // copy over thresholds
  const uint32_t lft_bytes = (MAX_LOOP_FILTER + 1) * sizeof(cuda_loop_filter_thresh);
  tb = new Type::Builder(context.rs, e);
  tb->setX(lft_bytes);
  type = tb->create();
  context.raw_lft = Allocation::createTyped(context.rs, type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT, NULL);

  context.raw_lft->copy1DRangeFrom(0,
     lft_bytes, lft);
  delete tb;

  // setup other parameters
  // y only for now
  ScriptC_loopfilter* sc = new ScriptC_loopfilter(context.rs);
  sc->bind_raw_lfm(context.raw_lfm);
  sc->bind_buf(context.buf);
  sc->bind_raw_lft(context.raw_lft);
  sc->set_sb_cols(sb_cols);
  sc->set_rows(rows);
  sc->set_cols(cols);
  sc->set_step_max(Y_STEP_MAX);
  sc->set_lvl_shift(Y_LVL_SHIFT);
  sc->set_row_mult(Y_ROW_MULT);
  sc->set_blockDim(Y_BLOCK_DIM);
  sc->set_is_y(true);

  // start the wave
  for (uint32_t d = 0; d < max; d++) {
    uint32_t r_d, c_d;
    // iterate over all the columns, left to right, when we have exhausted
    // all of the columns start iterating over the rows
    if (d < sb_cols) {
      r_d = 0;
      c_d = d;
    }
    else {
      r_d = d - sb_cols + 1;
      c_d = sb_cols - 1;
    }
    sc->set_start_sx(c_d);
    sc->set_start_sy(r_d);
    uint32_t sb_in_d = std::min(c_d + 1, sb_rows - r_d);
    uint32_t threads = sb_in_d * SUPER_BLOCK_DIM;

    // Parameters for actual kernel launch
    sp<const Type> loopfilter = Type::create(context.rs, e, threads, 0, 0);
    sp<Allocation> blocks = Allocation::createTyped(context.rs, loopfilter,
          RS_ALLOCATION_MIPMAP_NONE,
          RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT, NULL);

    // filter cols
    sc->set_do_filter_cols(true);
    sc->forEach_root(blocks);

    // filter rows
    sc->set_do_filter_cols(false);
    sc->forEach_root(blocks);

  }

  // copy back loop filtered results
  context.buf->copy1DRangeTo(0, rows * cols, buf);
}
