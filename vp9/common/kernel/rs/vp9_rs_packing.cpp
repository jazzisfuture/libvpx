#include "obj/ScriptC_inter_rs.h"
#include "obj/ScriptC_inter_rs_op.h"
#include "obj/ScriptC_inter_rs_v.h"
#include "obj/ScriptC_inter_rs_h.h"
#include "obj/ScriptC_inter_rs_h_op.h"
#include <RenderScript.h>
#include "vp9_rs_packing.h"

#define MAX_TILE 8
using namespace android::RSC;

typedef struct context_rs {
  sp<Allocation> a_pool_buf;
  sp<Allocation> a_dst_buf;
  sp<Allocation> a_block_index[MAX_TILE];
  sp<Allocation> a_block_param[MAX_TILE];
  sp<Allocation> a_mid_buf[MAX_TILE];
  sp<Allocation> a_gsize;
  sp<RS> rs;
  int pool_size;
  int per_frame_size;
  int block_param_size;
  int block_index_size;
  ScriptC_inter_rs *sc;
  ScriptC_inter_rs_op *sc_op;
  ScriptC_inter_rs_h *sc_h;
  ScriptC_inter_rs_h_op *sc_h_op;
  ScriptC_inter_rs_v *sc_v;
  bool QCT;
  bool OP;
} context_rs;

context_rs context;

int init_rs(int pred_param_size, int param_index_size, int buf_size,
            int pool_size, int global_size, int tile_count) {
  int i = 0;
  char *qct_enable = getenv("QCTENABLE");
  char *op_enable = getenv("OPENABLE");
  if (qct_enable && op_enable) {
    context.QCT = true;
    context.OP = true;
    printf("run QCT optimized kernel\n");
  } else if (qct_enable && !op_enable) {
    context.QCT = true;
    context.OP = false;
    printf("run QCT kernel\n");
  } else if (!qct_enable && op_enable) {
    context.QCT = false;
    context.OP = true;
    printf("run MCW optimized kernel\n");
  } else if (!qct_enable && !op_enable) {
    context.QCT = false;
    context.OP = false;
    printf("run MCW kernel\n");
  }
  context.per_frame_size = buf_size;
  context.pool_size = pool_size;
  context.block_index_size = param_index_size;
  context.block_param_size = pred_param_size;
  context.rs = new RS();
  context.rs->init("/data/data/com.example.vp9");
  context.sc = new ScriptC_inter_rs(context.rs);
  context.sc_h = new ScriptC_inter_rs_h(context.rs);
  context.sc_v = new ScriptC_inter_rs_v(context.rs);
  context.sc_op = new ScriptC_inter_rs_op(context.rs);
  context.sc_h_op = new ScriptC_inter_rs_h_op(context.rs);

  sp<const Element> const_e = Element::U8(context.rs);
  sp<Element> e = (Element*)(const_e.get());
  Type::Builder *tb = NULL;
  sp<const Type> type;

  tb = new Type::Builder(context.rs, e);
  tb->setX(pool_size);
  type = tb->create();
  context.a_pool_buf = Allocation::createTyped(context.rs, type,
    RS_ALLOCATION_MIPMAP_NONE,
    RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT, NULL);
  delete tb;

  tb = new Type::Builder(context.rs, e);
  tb->setX(buf_size);
  type = tb->create();
  context.a_dst_buf = Allocation::createTyped(context.rs, type,
    RS_ALLOCATION_MIPMAP_NONE,
    RS_ALLOCATION_USAGE_SHARED | RS_ALLOCATION_USAGE_SCRIPT, NULL);
  delete tb;

  tb = new Type::Builder(context.rs, e);
  tb->setX(global_size);
  type = tb->create();
  context.a_gsize = Allocation::createTyped(context.rs, type,
    RS_ALLOCATION_MIPMAP_NONE,
    RS_ALLOCATION_USAGE_SCRIPT, NULL);
  delete tb;
  for (i = 0; i < tile_count; i++) {

    tb = new Type::Builder(context.rs, e);
    tb->setX(pred_param_size);
    type = tb->create();
    context.a_block_param[i] = Allocation::createTyped(
        context.rs,
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED, NULL);
    delete tb;
    tb = new Type::Builder(context.rs, e);
    tb->setX(param_index_size);
    type = tb->create();
    context.a_block_index[i] = Allocation::createTyped(
        context.rs,
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED, NULL);
    delete tb;

    tb = new Type::Builder(context.rs, e);
    tb->setX(buf_size);
    type = tb->create();
    context.a_mid_buf[i] = Allocation::createTyped(
        context.rs,
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT, NULL);
    delete tb;
  }
}

uint8_t *get_alloc_ptr(int flag, int index) {
  uint8_t *ptr = NULL;
  switch (flag) {
  case 0:
    ptr = (uint8_t *)context.a_pool_buf->getPointer();
    break;
  case 1:
    ptr = (uint8_t *)context.a_block_param[index]->getPointer();
    break;
  case 2:
    ptr = (uint8_t *)context.a_block_index[index]->getPointer();
    break;
  case 3:
    ptr = (uint8_t *)context.a_dst_buf->getPointer();
    break;
  }
  return ptr;
}

void invoke_inter_rs(int index,
                     int counts_8x8) {
  if (!context.QCT) {
    if (context.OP) {
      context.sc_op->bind_pred_param(context.a_block_param[index]);
      context.sc_op->bind_block_index(context.a_block_index[index]);
      context.sc_op->bind_pool_buf(context.a_pool_buf);
      context.sc_op->bind_dst_buf(context.a_dst_buf);
      context.sc_op->set_buffer_size(context.per_frame_size);
      context.sc_op->set_counts_8x8(counts_8x8);
      context.sc_op->forEach_root(context.a_gsize);
    } else {
      context.sc->bind_pred_param(context.a_block_param[index]);
      context.sc->bind_block_index(context.a_block_index[index]);
      context.sc->bind_pool_buf(context.a_pool_buf);
      context.sc->set_buffer_size(context.per_frame_size);
      context.sc->set_counts_8x8(counts_8x8);
      context.sc->bind_dst_buf(context.a_dst_buf);
      context.sc->forEach_root(context.a_gsize);
    }
  } else {
    if (context.OP) {
      context.sc_h_op->bind_pred_param(context.a_block_param[index]);
      context.sc_h_op->bind_block_index(context.a_block_index[index]);
      context.sc_h_op->bind_pool_buf(context.a_pool_buf);
      context.sc_h_op->bind_mid_buf(context.a_mid_buf[index]);
      context.sc_h_op->set_buffer_size(context.per_frame_size);
      context.sc_h_op->set_counts_8x8(counts_8x8);
      context.sc_h_op->forEach_root(context.a_gsize);
    } else {
      context.sc_h->bind_pred_param(context.a_block_param[index]);
      context.sc_h->bind_block_index(context.a_block_index[index]);
      context.sc_h->bind_pool_buf(context.a_pool_buf);
      context.sc_h->bind_mid_buf(context.a_mid_buf[index]);
      context.sc_h->set_buffer_size(context.per_frame_size);
      context.sc_h->set_counts_8x8(counts_8x8);
      context.sc_h->forEach_root(context.a_gsize);
    }
    context.sc_v->bind_pred_param(context.a_block_param[index]);
    context.sc_v->bind_block_index(context.a_block_index[index]);
    context.sc_v->bind_mid_buf(context.a_mid_buf[index]);
    context.sc_v->bind_pool_buf(context.a_pool_buf);
    context.sc_v->bind_dst_buf(context.a_dst_buf);
    context.sc_v->set_buffer_size(context.per_frame_size);
    context.sc_v->set_counts_8x8(counts_8x8);
    context.sc_v->forEach_root(context.a_gsize);
  }
}

void finish_rs() {
  context.rs->finish();
}

void release_rs(int tile_count) {
  if (context.rs != NULL) {
    context.rs.clear();
  }
  if (context.sc != NULL) {
    delete context.sc;
    context.sc = NULL;
  }
  for (int i = 0; i < tile_count; i++) {
    if (context.a_block_param[i] != NULL) {
      context.a_block_param[i].clear();
    }
    if (context.a_block_index[i] != NULL) {
      context.a_block_index[i].clear();
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
