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
##  This file tests libvpx example code. To add new tests to this file, do the
##  following:
##    1. Write a shell function (this is your test).
##    2. Add the function to example_tests (on a new line).
##
. $(dirname $0)/tools_common.sh

VP8_IVF_FILE="${LIBVPX_TEST_DATA_PATH}/vp80-00-comprehensive-001.ivf"
VP9_WEBM_FILE="${LIBVPX_TEST_DATA_PATH}/vp90-2-00-quantizer-00.webm"

# Environment check: Make sure input is available.
example_tests_verify_environment() {
  if [ ! -e "${VP8_IVF_FILE}" ] || [ ! -e "${VP9_WEBM_FILE}" ]; then
    echo "Libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
}

# Echoes path to $1 based on assumptions for the current target and
# ${LIBVPX_BIN_PATH}.
example_path() {
  local example="$1"
  local example_path
  if [ "$(is_windows_target)" = "yes" ]; then
    example_path="${LIBVPX_BIN_PATH}/${example}.exe"
  else
    example_path="${LIBVPX_BIN_PATH}/examples/${example}"
  fi
  echo "${example_path}"
}

# Echoes yes when $1 (after expansion via example_path()) is an executable file.
example_available() {
  local example="$(example_path $1)"
  [ -x "${example}" ] && echo yes
}

simple_decoder() {
  if [ "$(vpx_tool_available examples/simple_decoder)" = "yes" ]; then
    if [ "$(vp8_decode_available)" = "yes" ]; then
      echo running vp8 simple_decoder
    fi
    if [ "$(vp9_decode_available)" = "yes" ]; then
      echo running vp9 simple_decoder
    fi
  fi
}

example_tests="simple_decoder"

#run_tests example_tests_verify_environment "${example_tests}"
echo example_available simple_decoder=$(example_available simple_decoder)
echo example_available random_junk=$(example_available random_junk)
