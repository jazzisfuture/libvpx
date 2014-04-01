#include "obj/ScriptC_inter_rs.h"
#include "obj/ScriptC_inter_rs_h.h"
#include "obj/ScriptC_inter_rs_v.h"
#include <RenderScript.h>
#include "vp9_rs_packing.h"

#define MAX_TILE 4
using namespace android::RSC;
void log2file(const char *name, uint8_t *buf, int len);
typedef struct context_rs {
  sp<Allocation> a_pool_buf;
  sp<Allocation> a_block_index[MAX_TILE];
  sp<Allocation> a_block_param[MAX_TILE];
  sp<Allocation> a_dst_buf[MAX_TILE];
  sp<Allocation> a_mid_buf[MAX_TILE];
  sp<Allocation> a_gsize;
  sp<RS> rs;
  int has_init;
  int pool_size;
  int per_frame_size;
  int block_param_size;
  int block_index_size;
  ScriptC_inter_rs *sc;
  ScriptC_inter_rs_h *sc_h;
  ScriptC_inter_rs_v *sc_v;
  bool QCT;
} context_rs;

context_rs context;
void create_buffer_rs(int pred_param_size, int param_index_size,
                      int buf_size, int pool_size, int global_size,
                      int tile_count, uint8_t **block, uint8_t **index,
                      uint8_t **dst_buf, uint8_t **mid_buf) {
  sp<const Element> const_e = Element::U8(context.rs);
  sp<Element> e = (Element*)(const_e.get());
  Type::Builder *tb = NULL;
  sp<const Type> type;
  context.per_frame_size = buf_size;
  context.pool_size = pool_size;
  context.block_index_size = param_index_size;
  context.block_param_size = pred_param_size;

  tb = new Type::Builder(context.rs, e);
  tb->setX(pool_size);
  type = tb->create();
  context.a_pool_buf = Allocation::createTyped(context.rs, type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT, NULL);
  delete tb;
  tb = new Type::Builder(context.rs, e);
  tb->setX(global_size);
  type = tb->create();
  context.a_gsize = Allocation::createTyped(context.rs, type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT, NULL);
    delete tb;
  int tile_index = 0;
  for (tile_index = 0; tile_index < tile_count; tile_index++) {
    tb = new Type::Builder(context.rs, e);
    tb->setX(pred_param_size);
    type = tb->create();
    context.a_block_param[tile_index] = Allocation::createTyped(
        context.rs,
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED,
        block[tile_index]);
    delete tb;

    tb = new Type::Builder(context.rs, e);
    tb->setX(param_index_size);
    type = tb->create();
    context.a_block_index[tile_index] = Allocation::createTyped(
        context.rs,
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED,
        index[tile_index]);
    delete tb;

    tb = new Type::Builder(context.rs, e);
    tb->setX(buf_size);
    type = tb->create();
    context.a_dst_buf[tile_index] = Allocation::createTyped(context.rs,
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED,
        dst_buf[tile_index]);
    delete tb;

    tb = new Type::Builder(context.rs, e);
    tb->setX(buf_size);
    type = tb->create();
    context.a_mid_buf[tile_index] = Allocation::createTyped(context.rs,
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED,
        mid_buf[tile_index]);
    delete tb;
  }
}

int init_rs(int pred_param_size, int param_index_size, int buf_size,
            int pool_size, int global_size, int tile_index,
            uint8_t *block_param, uint8_t *index_param,
            uint8_t *dst_buf, uint8_t *mid_buf) {

  do {
    if (context.has_init) break;
    char *qct_enable = getenv("QCTENABLE");
    if (qct_enable) {
      printf("run QCT kernel\n");
      context.QCT = true;
    } else {
      printf("run mcw kernel\n");
      context.QCT = false;
    }
    context.per_frame_size = buf_size;
    context.pool_size = pool_size;
    context.block_index_size = param_index_size;
    context.block_param_size = pred_param_size;
    context.rs = new RS();
    context.rs->init();
    sp<const Element> const_e = Element::U8(context.rs);
    sp<Element> e = (Element*)(const_e.get());
    Type::Builder *tb = NULL;
    sp<const Type> type;

    tb = new Type::Builder(context.rs, e);
    tb->setX(pool_size);
    type = tb->create();
    context.a_pool_buf = Allocation::createTyped(context.rs, type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT, NULL);
    delete tb;
    tb = new Type::Builder(context.rs, e);
    tb->setX(global_size);
    type = tb->create();
    context.a_gsize = Allocation::createTyped(context.rs, type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT, NULL);
    delete tb;
    context.sc = new ScriptC_inter_rs(context.rs);
    context.sc_h = new ScriptC_inter_rs_h(context.rs);
    context.sc_v = new ScriptC_inter_rs_v(context.rs);
  } while (0);
  sp<const Element> const_e = Element::U8(context.rs);
  sp<Element> e = (Element*)(const_e.get());
  Type::Builder *tb = NULL;
  sp<const Type> type;

  tb = new Type::Builder(context.rs, e);
  tb->setX(pred_param_size);
  type = tb->create();
  context.a_block_param[tile_index] = Allocation::createTyped(
      context.rs,
      type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED, block_param);
  delete tb;

  tb = new Type::Builder(context.rs, e);
  tb->setX(param_index_size);
  type = tb->create();
  context.a_block_index[tile_index] = Allocation::createTyped(
      context.rs,
      type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED, index_param);
  delete tb;

  tb = new Type::Builder(context.rs, e);
  tb->setX(buf_size);
  type = tb->create();
  context.a_dst_buf[tile_index] = Allocation::createTyped(
      context.rs,
      type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED, dst_buf);
  delete tb;
  
  tb = new Type::Builder(context.rs, e);
  tb->setX(buf_size);
  type = tb->create();
  context.a_mid_buf[tile_index] = Allocation::createTyped(
      context.rs,
      type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED, mid_buf);
  delete tb;
  context.has_init = 1;
}

void update_pool_rs(unsigned char *per_buf, int frame_num) {
  context.a_pool_buf->copy1DRangeFrom(frame_num * context.per_frame_size,
      context.per_frame_size, per_buf);
}

void update_buffer_size_rs(int pred_param_size, int param_index_size,
                           int buf_size, int pool_size) {
  context.per_frame_size = buf_size;
  context.pool_size = pool_size;
  context.block_index_size = param_index_size;
  context.block_param_size = pred_param_size;
}

void get_rs_res(unsigned char *dst_buf, int index) {
  context.a_dst_buf[index]->copy1DRangeTo(0, context.per_frame_size, dst_buf);
}


void invoke_inter_rs(unsigned char *dst_buf, unsigned char *block_param,
                     unsigned char *index_param, int index,
                     int counts_8x8, int block_param_size,
                     int index_param_size) {
  context.a_block_index[index]->copy1DRangeFrom(0,
      index_param_size, index_param);
  context.a_block_param[index]->copy1DRangeFrom(0,
      block_param_size, block_param);
  if (!context.QCT) {
    context.sc->bind_pred_param(context.a_block_param[index]);
    context.sc->bind_block_index(context.a_block_index[index]);
    context.sc->bind_pool_buf(context.a_pool_buf);
    context.sc->bind_dst_buf(context.a_dst_buf[index]);
    context.sc->set_buffer_size(context.per_frame_size);
    context.sc->set_counts_8x8(counts_8x8);
    context.sc->forEach_root(context.a_gsize);
  } else {
    context.sc_h->bind_pred_param(context.a_block_param[index]);
    context.sc_h->bind_block_index(context.a_block_index[index]);
    context.sc_h->bind_pool_buf(context.a_pool_buf);
    context.sc_h->bind_mid_buf(context.a_mid_buf[index]);
    context.sc_h->set_buffer_size(context.per_frame_size);
    context.sc_h->set_counts_8x8(counts_8x8);
    context.sc_h->forEach_root(context.a_gsize);

    context.sc_v->bind_pred_param(context.a_block_param[index]);
    context.sc_v->bind_block_index(context.a_block_index[index]);
    context.sc_v->bind_dst_buf(context.a_dst_buf[index]);
    context.sc_v->bind_mid_buf(context.a_mid_buf[index]);
    context.sc_v->set_buffer_size(context.per_frame_size);
    context.sc_v->set_counts_8x8(counts_8x8);
    context.sc_v->forEach_root(context.a_gsize);
  }
}

void release_buffer_rs(int tile_count) {
  for (int i = 0; i < tile_count; i++) {
    if (context.a_block_param[i] != NULL) {
      context.a_block_param[i].clear();
    }
    if (context.a_block_index[i] != NULL) {
      context.a_block_index[i].clear();
    }
    if (context.a_dst_buf[i] != NULL) {
      context.a_dst_buf[i].clear();
    }
    if (context.a_mid_buf[i] != NULL) {
      context.a_mid_buf[i].clear();
    }
  }
  if (context.a_pool_buf != NULL) {
    context.a_pool_buf.clear();
  }
  if (context.a_gsize != NULL) {
    context.a_gsize.clear();
  }
}

void release_rs(int tile_count) {
  for (int i = 0; i < tile_count; i++) {
    if (context.a_block_param[i] != NULL) {
      context.a_block_param[i].clear();
    }
    if (context.a_block_index[i] != NULL) {
      context.a_block_index[i].clear();
    }
    if (context.a_dst_buf[i] != NULL) {
      context.a_dst_buf[i].clear();
    }
  }
  if (context.a_pool_buf != NULL) {
    context.a_pool_buf.clear();
  }
  if (context.a_gsize != NULL) {
    context.a_gsize.clear();
  }
  if (context.rs != NULL) {
    context.rs.clear();
  }

  if (context.sc != NULL) {
    delete context.sc;
    context.sc = NULL;
  }

  if (context.sc_v != NULL) {
    delete context.sc_v;
    context.sc_v = NULL;
  }
  if (context.sc_h != NULL) {
    delete context.sc_h;
    context.sc_h = NULL;
  }
}
