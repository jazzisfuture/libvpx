#!/bin/sh
set -e

# All tests must call test_begin() with the name of the test. This allows for
# failure reporting upon exit from the test due to failure via the VPX_TOOL_TEST
# variable.
test_begin() {
  VPX_TOOL_TEST=$1
}

# All tests must call test_end() to clear the VPX_TOOL_TEST variable.
test_end() {
  if [ "$1" != "${VPX_TOOL_TEST}" ]; then
    echo "FAIL completed test mismatch!."
    echo "  completed test: $1"
    echo "  active test: ${VPX_TOOL_TEST}."
    return 1
  fi
  VPX_TOOL_TEST='<unset>'
}

# Trap function used for failure reports and tool output directory removal.
cleanup() {
  if [ -n "${VPX_TOOL_TEST}" ] && [ ${VPX_TOOL_TEST} != '<unset>' ]; then
    echo "FAIL: $VPX_TOOL_TEST"
  fi
  if [ -n ${OUTPUT_DIR} ] && [ -d ${OUTPUT_DIR} ]; then
    echo echo only rm -rf ${OUTPUT_DIR}
  fi
}

# This script requires that the LIBVPX_BUILD_PATH and
# LIBVPX_TEST_DATA_PATH variables are in the environment: Confirm that
# the variables are set and that they both evaluate to directory paths.
verify_vpx_test_environment() {
  test_begin $FUNCNAME
  if [ ! -d "${LIBVPX_BUILD_PATH}" ]; then
    echo "The LIBVPX_BUILD_PATH environment variable must be set."
    return 1
  fi
  if [ ! -d "${LIBVPX_TEST_DATA_PATH}" ]; then
    echo "The LIBVPX_TEST_DATA_PATH environment variable must be set."
    return 1
  fi
  test_end $FUNCNAME
}

# Greps vpx_config.h in LIBVPX_BUILD_PATH for positional parameter 1, which
# should be a LIBVPX preprocessor flag. Echos yes to stdout when the feature is
# available.
vpx_config_option_enabled() {
  vpx_config_option=$1
  vpx_config_file="${LIBVPX_BUILD_PATH}/vpx_config.h"
  config_line=$(grep ${vpx_config_option} ${vpx_config_file})
  if echo $config_line | egrep -q '1$'; then
    echo yes
  fi
}

# Echoes yes to stdout when the file named by positional parameter 1 exists in
# LIBVPX_BUILD_PATH, and is executable.
vpx_tool_available() {
  [ -x ${LIBVPX_BUILD_PATH}/$1 ] && echo yes
}

# Echoes yes to stdout when vpx_config_option_enabled() reports yes for
# CONFIG_VP8_DECODER.
vp8_decode_available() {
  [ $(vpx_config_option_enabled CONFIG_VP8_DECODER) = "yes" ] && echo yes
}

# Echoes yes to stdout when vpx_config_option_enabled() reports yes for
# CONFIG_VP8_ENCODER.
vp8_encode_available() {
  [ $(vpx_config_option_enabled CONFIG_VP8_ENCODER) = "yes" ] && echo yes
}

# Echoes yes to stdout when vpx_config_option_enabled() reports yes for
# CONFIG_VP9_DECODER.
vp9_decode_available() {
  [ $(vpx_config_option_enabled CONFIG_VP9_DECODER) = "yes" ] && echo yes
}

# Echoes yes to stdout when vpx_config_option_enabled() reports yes for
# CONFIG_VP9_ENCODER.
vp9_encode_available() {
  [ $(vpx_config_option_enabled CONFIG_VP9_ENCODER) = "yes" ] && echo yes
}

# Echoes yes to stdout when vpx_config_option_enabled() reports yes for
# CONFIG_WEBM_IO.
webm_io_available() {
  [ $(vpx_config_option_enabled CONFIG_WEBM_IO) = "yes" ] && echo yes
}

# Echoes yes to stdout when vpxdec exists according to vpx_tool_available().
vpxdec_available() {
  [ -n $(vpx_tool_available vpxdec) ] && echo yes
}

# Wrapper function for running vpxdec in noblit mode. Requires that
# LIBVPX_BUILD_PATH points to the directory containing vpxdec.
vpxdec() {
  input=$1
  decoder=${LIBVPX_BUILD_PATH}/vpxdec
  ${decoder} $input --summary --noblit > /dev/null 2>&1
}

# Echoes yes to stdout when vpxenc exists according to vpx_tool_available().
vpxenc_available() {
  [ -n $(vpx_tool_available vpxenc) ] && echo yes
}

# Wrapper function for running vpxenc.
vpxenc() {
  encoder=${LIBVPX_BUILD_PATH}/vpxenc
  codec=$1
  width=$2
  height=$3
  frames=$4
  input=$5
  output=${OUTPUT_DIR}/$6
  extra_flags=$7

  # Because --ivf must be within the command line to get IVF from vpxenc.
  if echo $output | egrep -q 'ivf$'; then
    use_ivf=--ivf
  else
    unset use_ivf
  fi

  ${encoder} --codec=$codec --width=$width --height=$height --limit=$frames \
      ${use_ivf} ${extra_flags} -o $output $input > /dev/null 2>&1
  if [ ! -e $output ]; then
    # Return non-zero exit status: output file doesn't exist, so something
    # definitely went wrong.
    return 1
  fi
}

# Create temporary output directory, and a trap to clean it up.
OUTPUT_DIR=$(mktemp -d)
trap cleanup EXIT

verify_vpx_test_environment

