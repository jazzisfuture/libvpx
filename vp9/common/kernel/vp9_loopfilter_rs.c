/*
 * Copyright (C) 2013 MultiCoreWare, Inc. All rights reserved.
 * Guanlin Liang <guanlin@multicorewareinc.com>
 */

#include <stdio.h>
#include <dlfcn.h>
#include "./vpx_config.h"
#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_loopfilter.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9_loopfilter_rs.h"

typedef int (*vp9_loop_filter_rs_init_fun)(void **handle_);
typedef void (*vp9_loop_filter_rs_fini_fun)(void *handle);

static int rs_inited = 0;
static void *lib_handle = NULL;
//static void *rs_handle = NULL;
vp9_loop_filter_rs_init_fun vp9_loop_filter_rs_init_impl = NULL;
vp9_loop_filter_rs_fini_fun vp9_loop_filter_rs_fini_impl = NULL;
vp9_loop_filter_rs_create_buffer_fun vp9_loop_filter_rs_create_buffer = NULL;
vp9_loop_filter_rows_work_rs_fun vp9_loop_filter_rows_work_rs = NULL;

int vp9_loop_filter_rs_init(void **handle) {
  if (rs_inited)
    return vp9_loop_filter_rs_init_impl(handle);
  lib_handle = dlopen("libvp9rsif.so", RTLD_NOW);
  if (lib_handle == NULL) {
    printf("%s\n", dlerror());
    return -1;
  }

  vp9_loop_filter_rs_init_impl = dlsym(lib_handle, "vp9_loop_filter_rs_init");
  if (!vp9_loop_filter_rs_init_impl) return -1;
  vp9_loop_filter_rs_fini_impl = dlsym(lib_handle, "vp9_loop_filter_rs_fini");
  if (!vp9_loop_filter_rs_fini_impl) return -1;
  vp9_loop_filter_rows_work_rs = dlsym(lib_handle, "vp9_loop_filter_rows_work_rs");
  if (!vp9_loop_filter_rows_work_rs) return -1;
  vp9_loop_filter_rs_create_buffer = dlsym(lib_handle, "vp9_loop_filter_rs_create_buffer");
  if (!vp9_loop_filter_rs_create_buffer) return -1;

  if (vp9_loop_filter_rs_init_impl(handle) < 0) return -1;
  rs_inited = 1;
  return 0;
}

void vp9_loop_filter_rs_fini(void *handle) {
  vp9_loop_filter_rs_fini_impl(handle);
  dlclose(lib_handle);
}
