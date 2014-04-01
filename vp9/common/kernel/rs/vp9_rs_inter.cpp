#include <RenderScript.h>
#include "vp9_rs_inter.h"

#define MAX_TILE 4
using namespace android::RSC;

typedef struct context_inter {
  sp<ScriptIntrinsicVP9InterPred> sc[MAX_TILE];
  sp<Allocation> a_ref_buf[MAX_TILE];
  sp<Allocation> a_param[MAX_TILE];
  sp<Allocation> a_gsize[MAX_TILE];
  sp<RS> rs[MAX_TILE];
} context_inter;

static context_inter context;

int init_inter_rs(int frame_size, int param_size, int tile_index,
            uint8_t *ref_buf, uint8_t *param) {
  context.rs[tile_index] = new RS();
  context.rs[tile_index]->init("/data/data/com.example.vp9");

  sp<const Element> const_e = Element::U8(context.rs[tile_index]);
  sp<Element> e = (Element*)(const_e.get());
  Type::Builder *tb = NULL;
  sp<const Type> type;

  tb = new Type::Builder(context.rs[tile_index], e);
  tb->setX(frame_size);
  type = tb->create();
  context.a_ref_buf[tile_index] = Allocation::createTyped(
      context.rs[tile_index],
      type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED,
      ref_buf);
  delete tb;

  tb = new Type::Builder(context.rs[tile_index], e);
  tb->setX(param_size);
  type = tb->create();
  context.a_param[tile_index] = Allocation::createTyped(
      context.rs[tile_index],
      type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED,
      param);
  delete tb;

  tb = new Type::Builder(context.rs[tile_index], e);
  tb->setX(1);
  type = tb->create();
  context.a_gsize[tile_index] = Allocation::createTyped(
      context.rs[tile_index],
      type,
      RS_ALLOCATION_MIPMAP_NONE,
      RS_ALLOCATION_USAGE_SCRIPT,
      NULL);
  delete tb;

  context.sc[tile_index] = ScriptIntrinsicVP9InterPred::create(
                           context.rs[tile_index], const_e);
  context.sc[tile_index]->setRef(context.a_ref_buf[tile_index]);
  context.sc[tile_index]->setParam(context.a_param[tile_index]);

  return 0;
}

void invoke_inter_rs(int fri_count, int sec_count, int offset, int index) {
  context.sc[index]->setParamCount(fri_count, sec_count, offset);
  context.sc[index]->forEach(context.a_gsize[index]);
  context.rs[index]->finish();
}

void release_inter_rs(int tile_count) {
  for (int i = 0; i < tile_count; i++) {
    if (context.rs[i] != NULL) {
      context.rs[i].clear();
    }
    if (context.sc[i] != NULL) {
      context.sc[i].clear();
    }
    if (context.a_param[i] != NULL) {
      context.a_param[i].clear();
    }
    if (context.a_gsize[i] != NULL) {
      context.a_gsize[i].clear();
    }
    if (context.a_ref_buf[i]!= NULL) {
      context.a_ref_buf[i].clear();
    }
  }
}
