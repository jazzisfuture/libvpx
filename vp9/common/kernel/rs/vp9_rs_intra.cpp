#include <RenderScript.h>
#include "vp9_rs_intra.h"

#define MAX_TILE 4
using namespace android::RSC;

typedef struct context_intra {
  sp<ScriptIntrinsicVP9IntraPred> sc[MAX_TILE];
  sp<Allocation> a_param_buf[MAX_TILE];
  sp<Allocation> a_gsize[MAX_TILE];
  sp<RS> rs[MAX_TILE];
} context_intra;

static context_intra context;

int init_intra_rs(int frame_size, int tile_count,
            uint8_t *param_buf, int param_size) {
  for (int i = 0; i < tile_count; i++) {
    context.rs[i] = new RS();
    context.rs[i]->init("/data/data/com.example.vp9");
    sp<const Element> const_e = Element::U8(context.rs[i]);
    sp<Element> e = (Element*)(const_e.get());
    Type::Builder *tb = NULL;
    sp<const Type> type;

    tb = new Type::Builder(context.rs[i], e);
    int tile_sz = param_size;
    tb->setX(tile_sz);
    type = tb->create();
    context.a_param_buf[i] = Allocation::createTyped(
        context.rs[i],
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT | RS_ALLOCATION_USAGE_SHARED,
        param_buf + tile_sz * i);
    delete tb;
    tb = new Type::Builder(context.rs[i], e);
    tb->setX(1);
    type = tb->create();
    context.a_gsize[i] = Allocation::createTyped(
        context.rs[i],
        type,
        RS_ALLOCATION_MIPMAP_NONE,
        RS_ALLOCATION_USAGE_SCRIPT,
        NULL);
    delete tb;

    context.sc[i] = ScriptIntrinsicVP9IntraPred::create(
                           context.rs[i], const_e);
    context.sc[i]->setParamBuffer(context.a_param_buf[i]);
  }
  return 0;
}

void invoke_intra_rs(int tile_index, int block_cnt) {
  context.sc[tile_index]->setTileInfo(block_cnt);
  context.sc[tile_index]->forEach(context.a_gsize[tile_index]);
  context.rs[tile_index]->finish();
}

void release_intra_rs(int tile_count) {
  for (int i = 0; i < MAX_TILE; i++) {
    if (context.a_param_buf[i] != NULL) {
      context.a_param_buf[i].clear();
    }
    if (context.a_gsize[i] != NULL) {
      context.a_gsize[i].clear();
    }
    if (context.rs[i] != NULL) {
      context.rs[i].clear();
    }
    if (context.sc[i] != NULL) {
      context.sc[i].clear();
    }
  }
}
