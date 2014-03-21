#!/bin/sh
LIBVPX_BUILD_PATH=${1:-.}
LIBVPX_TEST_DATA_PATH=${LIBVPX_TEST_DATA_PATH:-.}

source tools_common.sh

YUV_RAW_INPUT=${LIBVPX_TEST_DATA_PATH}/hantro_collage_w352h288.yuv
YUV_RAW_INPUT_WIDTH=352
YUV_RAW_INPUT_HEIGHT=288
TEST_FRAMES=10

# MOVE ME
  if [ ! -e "${YUV_RAW_INPUT}" ]; then
    echo "The file ${YUV_RAW_INPUT##*/} must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi

verify_env

decode_tests="vpxdec_vp8_ivf
              vpxdec_vp8_webm
              vpxdec_vp9_ivf
              vpxdec_vp9_webm"

# TODO(tomfinegan): Test Y4M input to vpxenc.
encode_tests="vpxenc_vp8_ivf_from_y4m
              vpxenc_vp8_webm_from_y4m
              vpxenc_vp9_ivf_from_y4m
              vpxenc_vp9_webm_from_y4m"

all_tests="${decode_tests} ${encode_tests}"

filter=$1
if [ -n "${filter}" ]; then
  for test in ${all_tests}; do
    if $(egrep -q $filter <<< $test); then
      filtered_tests="${filtered_tests} $test"
    fi
  done
else
  filtered_tests=${all_tests}
fi

vpxdec_test() {
  VPX_TOOL_TEST=$1
  input=$2
  vpxdec $input
  unset VPX_TOOL_TEST
}


# Creates the following 10 frame files:
# - VP8 in IVF
# - VP8 in WebM
create_vp8_inputs() {
  output_basename=$1
  input_file=${YUV_RAW_INPUT}
  vpxenc vp8 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
      ${input_file} ${output_basename}.ivf
  vpxenc vp8 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
      ${input_file} ${output_basename}.webm
}

# Creates the following 10 frame files:
# - VP9 in IVF
# - VP9 in WebM
create_vp9_inputs() {
  output_basename=$1
  input_file=${YUV_RAW_INPUT}
  vpxenc vp9 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
      ${input_file} ${output_basename}.ivf
  vpxenc vp9 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
      ${input_file} ${output_basename}.webm
}


# Create test input files
if $(grep -q vpxdec_vp8 <<< ${filtered_tests}); then
  echo "Creating VP8 input files..."
  create_vp8_inputs ${OUTPUT_DIR}/vp8
  echo "Done."
fi

if $(grep -q vpxdec_vp9 <<< ${filtered_tests}); then
  echo "Creating VP9 input files..."
  create_vp9_inputs ${OUTPUT_DIR}/vp9
  echo "Done."
fi

for test in ${filtered_tests}; do
  if [[ $test =~ ^vpxdec_vp8 ]]; then
    if [[ $test =~ ivf$ ]]; then
      vpxdec_test $test ${OUTPUT_DIR}/vp8.ivf
    elif [[ $test =~ webm$ ]]; then
      vpxdec_test $test ${OUTPUT_DIR}/vp8.webm
    fi
  elif [[ $test =~ ^vpxdec_vp9 ]]; then
    if [[ $test =~ ivf$ ]]; then
      vpxdec_test $test ${OUTPUT_DIR}/vp9.ivf
    elif [[ $test =~ webm$ ]]; then
      vpxdec_test $test ${OUTPUT_DIR}/vp9.webm
    fi
  fi
done

echo Done: All tests pass.

