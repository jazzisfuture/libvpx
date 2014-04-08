#include "obj/ScriptC_inter_rs.h"
#include <RenderScript.h>
#include "vp9_inter_rs.h"

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
} context_rs;

context_rs context;

int init_rs(int pred_param_size, int param_index_size, int buf_size,
            int pool_size, int global_size, int tile_count) {
  int i = 0;
  context.per_frame_size = buf_size;
  context.pool_size = pool_size;
  context.block_index_size = param_index_size;
  context.block_param_size = pred_param_size;
  context.rs = new RS();
  context.rs->init("/data/data/com.example.vp9");
  context.sc = new ScriptC_inter_rs(context.rs);

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

void invoke_inter_rs(int index, int counts_8x8) {
  context.sc->bind_pred_param(context.a_block_param[index]);
  context.sc->bind_block_index(context.a_block_index[index]);
  context.sc->bind_pool_buf(context.a_pool_buf);
  context.sc->set_buffer_size(context.per_frame_size);
  context.sc->set_counts_8x8(counts_8x8);
  context.sc->bind_dst_buf(context.a_dst_buf);
  context.sc->forEach_root(context.a_gsize);
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
