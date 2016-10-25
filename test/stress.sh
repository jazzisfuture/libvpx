#!/bin/sh
##
##  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##
##  This file performs a stress test. It runs 5 encodes and 30 decodes in
##  parallel.  To add new tests to this file, do the
##  following:
##    1. Write a shell function (this is your test).
##    2. Add the function to stress_tests (on a new line).
##
. $(dirname $0)/tools_common.sh

YUV="${LIBVPX_TEST_DATA_PATH}/niklas_1280_720_30.yuv"
VP8="${LIBVPX_TEST_DATA_PATH}/tos_vp8.webm"
VP9="${LIBVPX_TEST_DATA_PATH}/vp90-2-sintel_1920x818_tile_1x4_fpm_2279kbps.webm"

# Environment check: Make sure input is available.
stress_verify_environment() {
  if [ ! -e "${YUV}" ] || [ ! -e "${VP8}" ] || [ ! -e "${VP9}" ] ; then
    elog "Libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
  if [ -z "$(vpx_tool_path vpxenc)" ]; then
    elog "vpxenc not found. It must exist in LIBVPX_BIN_PATH or its parent."
    return 1
  fi
  if [ -z "$(vpx_tool_path vpxdec)" ]; then
    elog "vpxdec not found. It must exist in LIBVPX_BIN_PATH or its parent."
    return 1
  fi
}

# This function runs tests on libvpx that run multiple encodes and decodes
# in parallel in hopes of catching, synchronization and/or threading issues.
stress() {
  local readonly decoder="$(vpx_tool_path vpxdec)"
  local readonly encoder="$(vpx_tool_path vpxenc)"
  local readonly codec="$1"
  local readonly WEBM="$2"
  local readonly decode_count="$3"

  # Enable job control, so we can run multiple threads.
  set -m

  # Start 5 2 pass encode jobs in parallel. These are 2 pass encodes.
  for i in `seq 5`; do
    b=$(($i*20+300))
    eval "${VPX_TEST_PREFIX}" "${encoder}" "--codec=${codec} -w 1280 -h 720" \
      "${YUV}" "-t 4 --limit=150 --test-decode=fatal --target-bitrate=${b}" \
      "-o " "${i}.webm"&
  done

  # Start 5 rt encode jobs in parallel.
  for i in `seq 5`; do
    b=$(($i*20+300))
    eval "${VPX_TEST_PREFIX}" "${encoder}" "--codec=${codec} -w 1280 -h 720" \
      "${YUV}" "-t 4 --limit=150 --test-decode=fatal --target-bitrate=${b}" \
      "--lag-in-frames=0 --error-resilient=1" \
      "--kf-min-dist=3000 --kf-max-dist=3000 --cpu-used=-6 --static-thresh=1" \
      "--end-usage=cbr --min-q=2 --max-q=56 --undershoot-pct=100" \
      "--overshoot-pct=15 --buf-sz=1000 --buf-initial-sz=500" \
      "--buf-optimal-sz=600 --max-intra-rate=900 --resize-allowed=0" \
      "--drop-frame=0 --passes=1 --rt --noise-sensitivity=4" \
      "-o " "${i}.rt.webm"&
  done

  # Start 30 decode jobs in parallel.
  for i in `seq "${decode_count}"`; do
    eval "${decoder}" "-t 4" "${WEBM}" "--noblit"&
  done

  # Wait for all parallel jobs to finish.
  FAIL=0
  for job in `jobs -p`
  do
    wait $job || let "FAIL+=1"
  done
  echo "All done "$FAIL" Failed "
  return $FAIL
}

vp8_stress_test(){
  if [ "$(vp8_decode_available)" = "yes" -a \
       "$(vp8_encode_available)" = "yes" ]; then
    stress vp8 "${VP8}" 40
  fi
}

vp9_stress_test(){
  if [ "$(vp9_decode_available)" = "yes" -a \
       "$(vp9_encode_available)" = "yes" ]; then
    stress vp9 "${VP9}" 25
  fi
}


run_tests stress_verify_environment "vp8_stress_test vp9_stress_test"
