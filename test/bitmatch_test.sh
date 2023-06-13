#!/bin/sh
##
##  Copyright (c) 2023 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##
##  This script checks the bit exactness between C and SIMD
##  implementations of VP9 the encoder.

# Environment check: $YUV_RAW_INPUT, YUV_240P_RAW_INPUT, YUV_480P_RAW_INPUT,
# YUV_720P_RAW_INPUT, Y4M_720P_INPUT and vpxenc are required.
. $(dirname $0)/tools_common.sh

bitmatch_test_verify_environment() {
  if [ ! -e "${YUV_RAW_INPUT}" ]; then
    echo "libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
  if [ ! -e "${YUV_240P_RAW_INPUT}" ]; then
    echo "libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
  if [ ! -e "${YUV_480P_RAW_INPUT}" ]; then
    echo "libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
  if [ ! -e "${YUV_720P_RAW_INPUT}" ]; then
    echo "libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
  if [ ! -e "${Y4M_720P_INPUT}" ]; then
    echo "libvpx test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
  if [ -z "$(vpx_tool_path vpxenc)" ]; then
    elog "vpxenc not found. It must exist in LIBVPX_BIN_PATH or its parent."
    return 1
  fi
}

cleanup_bitmatch_test() {
  # Clean-up vp9_generic_encoder
  rm -f  ${LIBVPX_BIN_PATH}/vp9_generic_encoder
}

# Configures for the generic target in gen_build_dir directory, and copy
# the encoder excutable to LIBVPX_BIN_PATH.
vp9_enc_build_target_generic() {
  local script_dir=$(dirname "$0")
  local libvpx_source_dir=$(cd ${script_dir}/..; pwd)
  if [ ! -d "$libvpx_source_dir" ]; then
    echo "$libvpx_source_dir does not exist."
    return 1
  fi
  local gen_build_dir=${VPX_TEST_OUTPUT_DIR}/build_target_generic
  mkdir -p $gen_build_dir
  cd $gen_build_dir
  local target="generic-gnu"
  local config_args_gen="--disable-internal-stats \
             --enable-static \
             --disable-unit-tests \
             --disable-docs \
             --disable-vp8 \
             --enable-vp9"
  echo "Building target: ${target}"

  eval "${libvpx_source_dir}/configure" --target="${target}" \
    "${config_args_gen}" \
    ${devnull}

  eval make -j$(nproc) ${devnull}
  echo "Done building target: ${target}"
  mv ${gen_build_dir}/vpxenc ${LIBVPX_BIN_PATH}/vp9_generic_encoder
  if [ -z "$(vpx_tool_path vp9_generic_encoder)" ]; then
    elog "vp9_generic_encoder not found. It must exist in LIBVPX_BIN_PATH or its parent."
    return 1
  fi
}

# Wrapper function for running vpxenc. Requires that LIBVPX_BIN_PATH points to
# the directory containing vpxenc. $1 one is used as the input file path and
# shifted away. $2 is used to configure as good or rt encoding mode.
vpxenc_vp9_encode() {
  local gen_enb=$1
  local cpu="$2"
  local testbitrate="$3"
  local output="$4"
  local input="$5"
  shift
  if [ $gen_enb -eq 1 ]; then
    local encoder="$(vpx_tool_path vp9_generic_encoder)"
  else
    local encoder="$(vpx_tool_path vpxenc)"
  fi
  eval "${encoder}" ${input} \
    "$@" \
    "--limit=${VPX_ENCODE_BITMATCH_TEST_FRAME_LIMIT}" \
    "--cpu-used=${cpu}" \
    "--target-bitrate=${testbitrate}" \
    "-o" \
    ${output} \
    ${devnull}
  if [ ! -e "${output}" ]; then
    elog "Output file does not exist."
    return 1
  fi
}

## VPX_SIMD_CAPS_MASK:
# AVX512 AVX2 AVX SSE4_1 SSSE3 SSE3 SSE2 SSE MMX
#   0     1    1    1      1    1    1    1   1  -> 255 -> Enable AVX2 and lower variants
#   0     0    1    1      1    1    1    1   1  -> 127 -> Enable AVX and lower variants
#   0     0    0    1      1    1    1    1   1  -> 63  -> Enable SSE4_1 and lower variants
#   0     0    0    0      1    1    1    1   1  -> 31  -> Enable SSSE3 and lower variants
#   0     0    0    0      0    1    1    1   1  -> 15  -> Enable SSE3 and lower variants
#   0     0    0    0      0    0    1    1   1  ->  7  -> Enable SSE2 and lower variants
#   0     0    0    0      0    0    0    1   1  ->  3  -> Enable SSE and lower variants
#   0     0    0    0      0    0    0    0   1  ->  1  -> Enable MMX
## NOTE: Since all x86_64 platforms implement sse2, enabling sse/mmx/c using "VPX_SIMD_CAPS_MASK"
#  is not possible.

bitmatch_test_x86() {
  local GEN=1
  local SIMD=0
  # In libvpx, the SIMD variants available for testing are limited to AVX2(255), SSE4_1(63),
  # SSSE3(31) and SSE2(7). To optimize testing time, this test is currently enabled exclusively
  # for these variants. Additional options can be enabled in the future if required.
  local enable_simd_mask="255 63 31 7"
  local test_bitrates="75 200 1200 1600 6400"
  local test_contents=("yuv_raw_input" "yuv_240p_raw_input" "yuv_480p_raw_input" \
   "yuv_720p_raw_input" "y4m_720p_input")

  # Generic build
  vp9_enc_build_target_generic

  # FOR GOOD ENCODING
  for cpu in {0..5}; do
    for content in ${test_contents[@]}; do
      for b in ${test_bitrates[@]}; do
        # For C
        vpxenc_vp9_encode ${GEN} "${cpu}" "${b}" \
         "${VPX_TEST_OUTPUT_DIR}/$content-good-${b}kbps-C-$cpu.ivf" \
          $($content) $(vpxenc_encode_test_good_params)
        # For SIMD
        for m in ${enable_simd_mask[@]}; do
          export VPX_SIMD_CAPS_MASK=$m
          vpxenc_vp9_encode ${SIMD} "${cpu}" "${b}" \
            "${VPX_TEST_OUTPUT_DIR}/$content-good-${b}kbps-SIMD-$cpu.ivf" \
            $($content) $(vpxenc_encode_test_good_params)
          unset VPX_SIMD_CAPS_MASK
          # Check if outfile_c and outfile_simd are identical. If not identical, this test fails.
          diff ${VPX_TEST_OUTPUT_DIR}/$content-good-${b}kbps-C-$cpu.ivf \
               ${VPX_TEST_OUTPUT_DIR}/$content-good-${b}kbps-SIMD-$cpu.ivf > /dev/null
          if [ $? -eq 1 ]; then
            return 1
          fi
        done
      done
    done
  done

 # FOR RT
  for cpu in {0..9}; do
    for content in ${testcontents[@]}; do
      for b in ${testbitrates[@]}; do
        # For C
        vpxenc_vp9_encode ${GEN} "${cpu}" "${b}" \
          "${VPX_TEST_OUTPUT_DIR}/$content-rt-${b}kbps-C-$cpu.ivf" \
          $($content) $(vpxenc_encode_test_rt_params)
        # For SIMD
        for m in ${enable_simd_mask[@]}; do
          export VPX_SIMD_CAPS_MASK=$m
          vpxenc_vp9_encode ${SIMD} "${cpu}" "${b}" \
            "${VPX_TEST_OUTPUT_DIR}/$content-rt-${b}kbps-SIMD-$cpu.ivf" \
            $($content) $(vpxenc_encode_test_rt_params)
          unset VPX_SIMD_CAPS_MASK
          # Check if outfile_c and outfile_simd are identical. If not identical, this test fails.
          diff ${VPX_TEST_OUTPUT_DIR}/$content-rt-${b}kbps-C-$cpu.ivf \
            ${VPX_TEST_OUTPUT_DIR}/$content-rt-${b}kbps-SIMD-$cpu.ivf > /dev/null
          if [ $? -eq 1 ]; then
            return 1
          fi
        done
      done
    done
  done
}

# Setup a trap function to clean up vp9_generic_encoder after tests complete.
trap cleanup_bitmatch_test EXIT

bitmatch_test_tests="bitmatch_test_x86"
run_tests bitmatch_test_verify_environment "${bitmatch_test_tests}"
