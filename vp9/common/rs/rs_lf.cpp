#include <stdio.h>
#include <RenderScript.h>
#include "lf_types.h"
#include "rs_lf.h"
#include "obj/ScriptC_loopfilter.h"

#define Y_LVL_SHIFT 3
#define Y_STEP_MAX 8
#define Y_ROW_MULT 1
#define Y_BLOCK_DIM 64


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
    is_rs_initialized = true;
  }

  sp<const Element> const_e = Element::U8(context.rs);
  Type::Builder tb(context.rs, const_e);
  sp<const Type> type;

  // allocate buffer for i/o
  tb.setX(rows * cols);
  type = tb.create();

  // copy over buffer
  context.buf = Allocation::createTyped(context.rs, type);
  context.buf->copy1DRangeFrom(0, rows * cols, buf);

  // setup wavefront parameters
  uint32_t sb_rows = (rows + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_cols = (cols + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_count = sb_rows * sb_cols;
  uint32_t mask_bytes = sb_count * sizeof(LOOP_FILTER_MASK);

  uint32_t max = sb_rows + sb_cols - 1;
  printf("%d %d %d %d\n", sb_rows, sb_cols, sb_count, mask_bytes);
  // copy over masks
  tb.setX(mask_bytes);
  type = tb.create();

  context.raw_lfm = Allocation::createTyped(context.rs, type);
  context.raw_lfm->copy1DRangeFrom(0, mask_bytes, lfm);

  // copy over thresholds
  const uint32_t lft_bytes = (MAX_LOOP_FILTER + 1) * sizeof(cuda_loop_filter_thresh);
  tb.setX(lft_bytes);
  type = tb.create();

  context.raw_lft = Allocation::createTyped(context.rs, type);
  context.raw_lft->copy1DRangeFrom(0, lft_bytes, lft);

  // start the wave
  sp<ScriptC_loopfilter> sc = new ScriptC_loopfilter(context.rs);
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
    uint32_t sb_in_d = std::min(c_d + 1, sb_rows - r_d);
    uint32_t threads = sb_in_d * SUPER_BLOCK_DIM;

    // Parameters for actual kernel launch
    tb.setX(threads);
    type = tb.create();
    sp<Allocation> work_block = Allocation::createTyped(context.rs, type);

    // filter cols
    // y only for now
    sc->set_start_sx(c_d);
    sc->set_start_sy(r_d);
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
    sc->set_do_filter_cols(true);
    sc->forEach_root(work_block);
    context.rs->finish();

    // filter rows
    // Parameters for actual kernel launch
    sc->set_start_sx(c_d);
    sc->set_start_sy(r_d);
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
    sc->set_do_filter_cols(false);
    sc->forEach_root(work_block);
    context.rs->finish();
  }
  // copy back loop filtered results
  context.buf->copy1DRangeTo(0, rows * cols, buf);
}


