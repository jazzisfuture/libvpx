#include <RenderScript.h>
#include <string.h>

using namespace android::RSC;

#define SIMD_WIDTH          16
#define MAX_LOOP_FILTER     63
#define MAX_SEGMENTS        8
#define MAX_REF_FRAMES      4
#define MAX_MODE_LF_DELTAS  2
#define MB_MODE_COUNT       14
#define BLOCK_SIZES         13

#define MI_SIZE_LOG2 3
#define MI_BLOCK_SIZE_LOG2 (6 - MI_SIZE_LOG2)  // 64 = 2^6

#define MI_SIZE (1 << MI_SIZE_LOG2)  // pixels per mi-unit
#define MI_BLOCK_SIZE (1 << MI_BLOCK_SIZE_LOG2)  // mi-units per max block


#if (defined(__GNUC__) && __GNUC__) || defined(__SUNPRO_C)
#define DECLARE_ALIGNED(n,typ,val)  typ val __attribute__ ((aligned (n)))
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(n,typ,val)  __declspec(align(n)) typ val
#else
#warning No alignment directives known for this compiler.
#define DECLARE_ALIGNED(n,typ,val)  typ val
#endif

// block transform size
typedef enum {
  TX_4X4 = 0,                      // 4x4 transform
  TX_8X8 = 1,                      // 8x8 transform
  TX_16X16 = 2,                    // 16x16 transform
  TX_32X32 = 3,                    // 32x32 transform
  TX_SIZES
} TX_SIZE;

// This structure holds bit masks for all 8x8 blocks in a 64x64 region.
// Each 1 bit represents a position in which we want to apply the loop filter.
// Left_ entries refer to whether we apply a filter on the border to the
// left of the block.   Above_ entries refer to whether or not to apply a
// filter on the above border.   Int_ entries refer to whether or not to
// apply borders on the 4x4 edges within the 8x8 block that each bit
// represents.
// Since each transform is accompanied by a potentially different type of
// loop filter there is a different entry in the array for each transform size.
typedef struct {
  uint64_t left_y[TX_SIZES];
  uint64_t above_y[TX_SIZES];
  uint64_t int_4x4_y;
  uint16_t left_uv[TX_SIZES];
  uint16_t above_uv[TX_SIZES];
  uint16_t int_4x4_uv;
  uint8_t lfl_y[64];
  uint8_t lfl_uv[16];
} LOOP_FILTER_MASK;

// Need to align this structure so when it is declared and
// passed it can be loaded into vector registers.
typedef struct {
  DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, mblim[SIMD_WIDTH]);
  DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, lim[SIMD_WIDTH]);
  DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, hev_thr[SIMD_WIDTH]);
} loop_filter_thresh;

typedef struct {
  loop_filter_thresh lfthr[MAX_LOOP_FILTER + 1];
  uint8_t lvl[MAX_SEGMENTS][MAX_REF_FRAMES][MAX_MODE_LF_DELTAS];
  uint8_t mode_lf_lut[MB_MODE_COUNT];
} loop_filter_info_n;

#define FRAME_BUFFERS   ((1 << 3) + 4)
typedef struct {
  int                 set_params;
  int                 start;
  int                 stop;
  sp<RS>              rs;
  sp<const Element>   e;
  sp<ScriptIntrinsicVP9LoopFilter> sc;
  uint8_t             *fb_pointers[FRAME_BUFFERS];
  sp<Allocation>      frame_buffers[FRAME_BUFFERS];
} vp9_loop_filter_rs_handle;

extern "C" int vp9_loop_filter_rs_init(void **handle_) {
  vp9_loop_filter_rs_handle *handle = new vp9_loop_filter_rs_handle;

  *handle_ = handle;
  handle->set_params = 0;
  handle->start = 0;
  handle->stop = 0;
  handle->rs = new RS();
  handle->rs->init("/data/data/com.example.vp9");
  handle->e = Element::U8(handle->rs);
  handle->sc = ScriptIntrinsicVP9LoopFilter::create(handle->rs, handle->e);
  memset((uint8_t *)&handle->fb_pointers, 0, sizeof(handle->fb_pointers));

  return 0;
}

extern "C" int vp9_loop_filter_rs_create_buffer(void *handle_,
                                                int idx,
                                                int size,
                                                uint8_t *pointer) {
  vp9_loop_filter_rs_handle *handle = (vp9_loop_filter_rs_handle *)handle_;

  if (handle->fb_pointers[idx] != pointer) {
    sp<const Type> t = Type::create(handle->rs, handle->e, size, 0, 0);
    handle->frame_buffers[idx] =
        Allocation::createTyped(handle->rs,
                                t,
                                RS_ALLOCATION_MIPMAP_NONE,
                                RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT,
                                pointer);
    handle->fb_pointers[idx] = pointer;
  }
  return 0;
}

extern "C" void vp9_loop_filter_rs_fini(void *handle_) {
  vp9_loop_filter_rs_handle *handle = (vp9_loop_filter_rs_handle *)handle_;
  handle->rs->finish();
  delete handle;
}

extern "C" void vp9_loop_filter_rows_work_rs(void *handle_,
                                             int start,
                                             int stop,
                                             int num_planes,
                                             int mi_rows,
                                             int mi_cols,
                                             int frame_buffer_idx,
                                             int y_offset,
                                             int u_offset,
                                             int v_offset,
                                             int y_stride,
                                             int uv_stride,
                                             loop_filter_info_n *lf_info,
                                             LOOP_FILTER_MASK *lfms) {
  vp9_loop_filter_rs_handle *handle = (vp9_loop_filter_rs_handle *)handle_;

  if (!handle->set_params) {
    sp<const Type> t_lf_info = Type::create(handle->rs,
                                            handle->e,
                                            sizeof(loop_filter_info_n), 0, 0);

    int size_lfms = (stop + MI_BLOCK_SIZE - start) / MI_BLOCK_SIZE *
                    (mi_cols + MI_BLOCK_SIZE) / MI_BLOCK_SIZE *
                    sizeof(LOOP_FILTER_MASK);
    sp<const Type> t_mask = Type::create(handle->rs, handle->e, size_lfms, 0, 0);

    sp<Allocation> lf_info_buffer =
        Allocation::createTyped(handle->rs,
                                t_lf_info,
                                RS_ALLOCATION_MIPMAP_NONE,
                                RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT,
                                lf_info);
    sp<Allocation> mask_buffer =
        Allocation::createTyped(handle->rs,
                                t_mask,
                                RS_ALLOCATION_MIPMAP_NONE,
                                RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT,
                                lfms);
    ScriptIntrinsicVP9LoopFilter::BufferInfo buf_info = {y_offset,
                                                         u_offset,
                                                         v_offset,
                                                         y_stride,
                                                         uv_stride};
    handle->sc->setBufferInfo(&buf_info);
    handle->sc->setLoopFilterInfo(lf_info_buffer);
    handle->sc->setLoopFilterMasks(mask_buffer);
    handle->set_params = 1;
  }
  if (start != handle->start || stop != handle->stop) {
    handle->sc->setLoopFilterDomain(start, stop, num_planes, mi_rows, mi_cols);
    handle->start = start;
    handle->stop = stop;
  }
  handle->sc->forEach(handle->frame_buffers[frame_buffer_idx]);
  handle->rs->finish();
}
