/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/decoder/vp9_device.h"

static struct device devs[] = {
  {
    DEV_CPU,            // type
    4,                  // threads_count
    6,                  // max_queue_tasks
    DEV_STAT_ENABLED,   //
  },
  {
    DEV_GPU,            // type
    1,                  // threads_count
    2,                  // max_queue_tasks
    DEV_STAT_ENABLED,   //
  },
  {
    DEV_DSP,            // type
    3,                  // threads_count
    6,                  // max_queue_tasks
    DEV_STAT_ENABLED,   //
  },
};

void vp9_register_devices(struct scheduler *sched) {
  // For now, we just use CPU dev
  char *rs_enable = getenv("RSENABLE");
  if (rs_enable) {
    printf("enable renderscript\n");
    scheduler_add_devices(sched, devs, 2);
  } else {
    printf("if want to enanble renderscript, please set RSENABLE as environment variable\n");
    scheduler_add_devices(sched, devs, 1);
  }
}
