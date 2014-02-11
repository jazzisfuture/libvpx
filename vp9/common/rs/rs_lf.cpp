#include <stdio.h>
#include <RenderScript.h>
#include "lf_types.h"
#include "rs_lf.h"
#include "obj/ScriptC_loopfilter.h"

// timing code
#include <sys/time.h>
typedef struct {
#if defined(_WIN32)
 LARGE_INTEGER  begin, end;
#else
 struct timeval begin, end;
#endif
} vpx_usec_timer;


static void
vpx_usec_timer_start( vpx_usec_timer *t) {
#if defined(_WIN32)
 QueryPerformanceCounter(&t->begin);
#else
 gettimeofday(&t->begin, NULL);
#endif
}


static void
vpx_usec_timer_mark( vpx_usec_timer *t) {
#if defined(_WIN32)
 QueryPerformanceCounter(&t->end);
#else
 gettimeofday(&t->end, NULL);
#endif
}


static int64_t
vpx_usec_timer_elapsed( vpx_usec_timer *t) {
#if defined(_WIN32)
 LARGE_INTEGER freq, diff;

 diff.QuadPart = t->end.QuadPart - t->begin.QuadPart;

 QueryPerformanceFrequency(&freq);
 return diff.QuadPart * 1000000 / freq.QuadPart;
#else
 struct timeval diff;

 timersub(&t->end, &t->begin, &diff);
 return diff.tv_sec * 1000000 + diff.tv_usec;
#endif
}
// end


#define Y_LVL_SHIFT 3
#define Y_STEP_MAX 8
#define Y_ROW_MULT 1
#define Y_BLOCK_DIM 64


using namespace android::RSC;

typedef struct context_rs {
  sp<RS> rs;

} context_rs;


bool is_rs_initialized = false;
context_rs context;
static double tot_time = 0;

extern void lf_rs(const LOOP_FILTER_MASK* lfm,
                  const cuda_loop_filter_thresh* lft,
                  uint32_t rows,
                  uint32_t cols,
                  uint8_t * y_buf,
                  uint32_t uv_rows,
                  uint32_t uv_cols,
                  uint8_t * u_buf,
                  uint8_t * v_buf) {


  if (!is_rs_initialized) {
    context.rs = new RS();
    context.rs->init();
    is_rs_initialized = true;
  }

  sp<Allocation> d_y_buf;
  sp<Allocation> d_u_buf;
  sp<Allocation> d_v_buf;
  sp<Allocation> d_raw_lfm;
  sp<Allocation> d_raw_lft;

  sp<const Element> const_e = Element::U8(context.rs);
  Type::Builder tb(context.rs, const_e);
  sp<const Type> type;

  // allocate buffer for i/o
  tb.setX(rows * cols);
  type = tb.create();

  // copy over buffer
  d_y_buf = Allocation::createTyped(context.rs, type);
  d_y_buf->copy1DRangeFrom(0, rows * cols, y_buf);
  d_u_buf = Allocation::createTyped(context.rs, type);
  d_u_buf->copy1DRangeFrom(0, uv_rows * uv_cols, u_buf);
  d_v_buf = Allocation::createTyped(context.rs, type);
  d_v_buf->copy1DRangeFrom(0, uv_rows * uv_cols, v_buf);

  // setup wavefront parameters
  uint32_t sb_rows = (rows + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_cols = (cols + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_count = sb_rows * sb_cols;
  uint32_t mask_bytes = sb_count * sizeof(LOOP_FILTER_MASK);

  uint32_t max = sb_rows + sb_cols - 1;
  // copy over masks
  tb.setX(mask_bytes);
  type = tb.create();

  d_raw_lfm = Allocation::createTyped(context.rs, type);
  d_raw_lfm->copy1DRangeFrom(0, mask_bytes, lfm);

  // copy over thresholds
  const uint32_t lft_bytes = (MAX_LOOP_FILTER + 1) * sizeof(cuda_loop_filter_thresh);
  tb.setX(lft_bytes);
  type = tb.create();

  d_raw_lft = Allocation::createTyped(context.rs, type);
  d_raw_lft->copy1DRangeFrom(0, lft_bytes, lft);

  // start the wave
  sp<ScriptC_loopfilter> filter_y = new ScriptC_loopfilter(context.rs);
  sp<ScriptC_loopfilter> filter_u = new ScriptC_loopfilter(context.rs);
  sp<ScriptC_loopfilter> filter_v = new ScriptC_loopfilter(context.rs);
  double elapsed_secs;
  vpx_usec_timer g;
  vpx_usec_timer_start(&g);
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
    sp<Allocation> y_work_block = Allocation::createTyped(context.rs, type);

    tb.setX(threads/2);
    type = tb.create();
    sp<Allocation> uv_work_block = Allocation::createTyped(context.rs, type);

    // filter cols
    // Y plane
    filter_y->set_start_sx(c_d);
    filter_y->set_start_sy(r_d);
    filter_y->bind_raw_lfm(d_raw_lfm);
    filter_y->bind_buf(d_y_buf);
    filter_y->bind_raw_lft(d_raw_lft);
    filter_y->set_sb_cols(sb_cols);
    filter_y->set_rows(rows);
    filter_y->set_cols(cols);
    filter_y->forEach_filter_cols_y(y_work_block);

    // U plane
    filter_u->set_start_sx(c_d);
    filter_u->set_start_sy(r_d);
    filter_u->bind_raw_lfm(d_raw_lfm);
    filter_u->bind_buf(d_u_buf);
    filter_u->bind_raw_lft(d_raw_lft);
    filter_u->set_sb_cols(sb_cols);
    filter_u->set_rows(uv_rows);
    filter_u->set_cols(uv_cols);
    filter_u->forEach_filter_cols_uv(uv_work_block);

    // V plane
    filter_v->set_start_sx(c_d);
    filter_v->set_start_sy(r_d);
    filter_v->bind_raw_lfm(d_raw_lfm);
    filter_v->bind_buf(d_v_buf);
    filter_v->bind_raw_lft(d_raw_lft);
    filter_v->set_sb_cols(sb_cols);
    filter_v->set_rows(uv_rows);
    filter_v->set_cols(uv_cols);
    filter_v->forEach_filter_cols_uv(uv_work_block);

    // filter rows
    // y plane
    filter_y->set_start_sx(c_d);
    filter_y->set_start_sy(r_d);
    filter_y->bind_raw_lfm(d_raw_lfm);
    filter_y->bind_buf(d_y_buf);
    filter_y->bind_raw_lft(d_raw_lft);
    filter_y->set_sb_cols(sb_cols);
    filter_y->set_rows(rows);
    filter_y->set_cols(cols);
    filter_y->forEach_filter_rows_y(y_work_block);

    // U plane
    filter_u->set_start_sx(c_d);
    filter_u->set_start_sy(r_d);
    filter_u->bind_raw_lfm(d_raw_lfm);
    filter_u->bind_buf(d_u_buf);
    filter_u->bind_raw_lft(d_raw_lft);
    filter_u->set_sb_cols(sb_cols);
    filter_u->set_rows(uv_rows);
    filter_u->set_cols(uv_cols);
    filter_u->forEach_filter_rows_uv(uv_work_block);

    // V plane
    filter_v->set_start_sx(c_d);
    filter_v->set_start_sy(r_d);
    filter_v->bind_raw_lfm(d_raw_lfm);
    filter_v->bind_buf(d_v_buf);
    filter_v->bind_raw_lft(d_raw_lft);
    filter_v->set_sb_cols(sb_cols);
    filter_v->set_rows(uv_rows);
    filter_v->set_cols(uv_cols);
    filter_v->forEach_filter_rows_uv(uv_work_block);
  }
  vpx_usec_timer_mark(&g);
  elapsed_secs = (double)(vpx_usec_timer_elapsed(&g))
                              / 1000.0;
  tot_time += elapsed_secs;

  printf("GPU - %f %f\n", elapsed_secs, tot_time);
  // copy back loop filtered results
  d_y_buf->copy1DRangeTo(0, rows * cols, y_buf);
  d_u_buf->copy1DRangeTo(0, uv_rows * uv_cols, u_buf);
  d_v_buf->copy1DRangeTo(0, uv_rows * uv_cols, v_buf);
}


