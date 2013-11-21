/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./warnings.h"

#include <stdio.h>
#include <stdlib.h>

#include "vpx/vpx_encoder.h"

#include "./tools_common.h"

int continue_prompt() {
  int c;
  fprintf(stderr, "Continue? (y to continue) ");
  c = getchar();
  return c == 'y';
}

void check_quantizer(int min_q, int max_q) {
  int check_failed = 0;

  if (min_q == max_q || abs(max_q - min_q) < 8) {
    check_failed = 1;
  }

  if (check_failed) {
    warn("Bad quantizer values. Quantizer values must not be equal, and "
         "should differ by at least 8.");

    if (!continue_prompt())
      exit(EXIT_FAILURE);
  }
}

void check_encoder_config(struct vpx_codec_enc_cfg* config) {
  check_quantizer(config->rc_min_quantizer, config->rc_max_quantizer);
}
