#!/bin/sh
##
##  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##
##  This file tests the libvpx vpx_temporal_svc_encoder example. To add new
##  tests to this file, do the following:
##    1. Write a shell function (this is your test).
##    2. Add the function to vpx_tsvc_encoder_tests (on a new line).
##
. $(dirname $0)/tools_common.sh

# Environment check: $YUV_RAW_INPUT is required.
vpx_tsvc_encoder_verify_environment() {
  if [ ! -e "${YUV_RAW_INPUT}" ]; then
    echo "Libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
}

# Runs vpx_tsvc_encoder using the codec specified by $1.
vpx_tsvc_encoder() {
  local encoder="${LIBVPX_BIN_PATH}/vpx_temporal_svc_encoder"
  encoder="${encoder}${VPX_TEST_EXE_SUFFIX}"
  local codec="$1"
  local output_file="${VPX_TEST_OUTPUT_DIR}/vpx_tsvc_encoder_${codec}.ivf"
  local timebase_num="1"
  local timebase_den="1000"
  local speed="6"
  local frame_drop_thresh="95"

  shift

  [ -x "${encoder}" ] || return 1

  eval "${encoder}" "${YUV_RAW_INPUT}" "${output_file}" "${codec}" \
      "${YUV_RAW_INPUT_WIDTH}" "${YUV_RAW_INPUT_HEIGHT}" \
      "${timebase_num}" "${timebase_den}" "${speed}" "${frame_drop_thresh}" \
      "$@" \
      ${devnull}

  # TODO: This won't work for this example: it outputs files on a per stream
  # basis.
  [ -e "${output_file}" ] || return 1
}

vpx_tsvc_encoder_vp8_mode_0() {
  if [ "$(vp8_encode_available)" = "yes" ]; then
    vpx_tsvc_encoder vp8 0 200 || return 1
  fi
}

vpx_tsvc_encoder_vp8_mode_1() {
  if [ "$(vp8_encode_available)" = "yes" ]; then
    vpx_tsvc_encoder vp8 1 200 400 || return 1
  fi
}

vpx_tsvc_encoder_vp8_mode_2() {
  if [ "$(vp8_encode_available)" = "yes" ]; then
    vpx_tsvc_encoder vp8 2 200 400 || return 1
  fi
}

vpx_tsvc_encoder_vp8_mode_3() {
  if [ "$(vp8_encode_available)" = "yes" ]; then
    vpx_tsvc_encoder vp8 3 200 400 600 || return 1
  fi
}

# TODO(tomfinegan): Add a frame limit param to vpx_tsvc_encoder and enable this
# test. VP9 is just too slow right now: This test takes 31m16s+ on a fast
# machine.
DISABLED_vpx_tsvc_encoder_vp9() {
  if [ "$(vp9_encode_available)" = "yes" ]; then
    vpx_tsvc_encoder vp9 || return 1
  fi
}

vpx_tsvc_encoder_tests="vpx_tsvc_encoder_vp8_mode_0
                        vpx_tsvc_encoder_vp8_mode_1
                        vpx_tsvc_encoder_vp8_mode_2
                        vpx_tsvc_encoder_vp8_mode_3
                        DISABLED_vpx_tsvc_encoder_vp9"

run_tests vpx_tsvc_encoder_verify_environment "${vpx_tsvc_encoder_tests}"
