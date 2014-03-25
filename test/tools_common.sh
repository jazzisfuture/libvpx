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
##  This file contains shell code shared by test scripts for libvpx tools.
set -e

# Sets $VPX_TOOL_TEST to the name specified by positional parameter one.
test_begin() {
  VPX_TOOL_TEST=$1
}

# Clears the VPX_TOOL_TEST variable after confirming that $VPX_TOOL_TEST matches
# positional parameter one.
test_end() {
  if [ "$1" != "${VPX_TOOL_TEST}" ]; then
    echo "FAIL completed test mismatch!."
    echo "  completed test: $1"
    echo "  active test: ${VPX_TOOL_TEST}."
    return 1
  fi
  VPX_TOOL_TEST='<unset>'
}

# Echoes the target configuration being tested.
test_configuration_target() {
  vpx_config_file=${LIBVPX_CONFIG_PATH}/vpx_config.c
  config_line=$(cat ${vpx_config_file} | grep target)
  # Split target line with awk using '=' as field separator to produce
  # 'target-name;"', then consume the '";' portion with tr while allowing tr to
  # send the result to stdout.
  echo ${config_line} | awk -F '=' -- '{ print $NF }' | tr -d ';"'
}

# Trap function used for failure reports and tool output directory removal.
# When the contents of $VPX_TOOL_TEST do not match the string '<unset>', reports
# failure of test stored in $VPX_TOOL_TEST.
cleanup() {
  if [ -n "${VPX_TOOL_TEST}" ] && [ ${VPX_TOOL_TEST} != '<unset>' ]; then
    echo "FAIL: $VPX_TOOL_TEST"
  fi
  if [ -n ${OUTPUT_DIR} ] && [ -d ${OUTPUT_DIR} ]; then
    rm -rf ${OUTPUT_DIR}
  fi
}

# Echoes git hash stored in vpx_config.h to stdout.
config_hash() {
  vpx_version_file=${LIBVPX_CONFIG_PATH}/vpx_version.h
  vpx_version_string=$(cat ${vpx_version_file} | grep -w VERSION_STRING)
  # Echo the version string to awk, split the string using '-' as field
  # separator and print the last field to get the '-g<git short hash>"' portion
  # of the version string, and then pipe it to tr to remove the trailing '"'.
  git_hash=$(echo ${vpx_version_string} | awk -F - -- '{print $NF}' | tr -d '"')
  # Now $git_hash is 'gGITHASH': remove the g using sed while allowing sed to
  # write the final to stdout.
  echo ${git_hash} | sed -e "s/g//"
}

# Echoes the short form of the current git hash.
current_hash() {
  if git --version > /dev/null 2>&1; then
    git rev-parse --short HEAD
  else
    # Return the config hash if git is unavailable: Fail silently, git hashes
    # are used only for warnings.
    config_hash
  fi
}

# Echoes warnings to stdout when git hash in vpx_config.h goes not match the
# current git hash.
check_git_hashes() {
  hash_at_configure_time=$(config_hash)
  hash_now=$(current_hash)

  if [ "${hash_at_configure_time}" != "${hash_now}" ]; then
    echo "Warning: git hash has changed since last configure."
  fi
}

# This script requires that the LIBVPX_BIN_PATH, LIBVPX_CONFIG_PATH, and
# LIBVPX_TEST_DATA_PATH variables are in the environment: Confirm that
# the variables are set and that they both evaluate to directory paths.
verify_vpx_test_environment() {
  if [ ! -d "${LIBVPX_BIN_PATH}" ]; then
    echo "The LIBVPX_BIN_PATH environment variable must be set."
    return 1
  fi
  if [ ! -d "${LIBVPX_CONFIG_PATH}" ]; then
    echo "The LIBVPX_CONFIG_PATH environment variable must be set."
    return 1
  fi
  if [ ! -d "${LIBVPX_TEST_DATA_PATH}" ]; then
    echo "The LIBVPX_TEST_DATA_PATH environment variable must be set."
    return 1
  fi
}

# Greps vpx_config.h in LIBVPX_CONFIG_PATH for positional parameter one, which
# should be a LIBVPX preprocessor flag. Echoes yes to stdout when the feature is
# available.
vpx_config_option_enabled() {
  vpx_config_option=$1
  vpx_config_file="${LIBVPX_CONFIG_PATH}/vpx_config.h"
  config_line=$(grep ${vpx_config_option} ${vpx_config_file})
  if echo $config_line | egrep -q '1$'; then
    echo yes
  fi
}

# Echoes yes to stdout when the file named by positional parameter one exists
# in LIBVPX_BIN_PATH, and is executable.
vpx_tool_available() {
  [ -x ${LIBVPX_BIN_PATH}/$1 ] && echo yes
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
# LIBVPX_BIN_PATH points to the directory containing vpxdec. Positional
# parameter one is used as the input file path. Positional parameter two, when
# present, is interpreted as a boolean flag that means the input should be sent
# to vpxdec via pipe from cat instead of directly.
vpxdec() {
  input=$1
  pipe_input=$2
  decoder=${LIBVPX_BIN_PATH}/vpxdec

  if [ -z "${pipe_input}" ]; then
    ${decoder} $input --summary --noblit > /dev/null 2>&1
  else
    cat ${input} | ${decoder} - --summary --noblit > /dev/null 2>&1
  fi

}

# Echoes yes to stdout when vpxenc exists according to vpx_tool_available().
vpxenc_available() {
  [ -n $(vpx_tool_available vpxenc) ] && echo yes
}

# Wrapper function for running vpxenc. Positional parameters are interpreted as
# follows:
#   1 - codec name
#   2 - input width
#   3 - input height
#   4 - number of frames to encode
#   5 - path to input file
#   6 - path to output file
#       Note: The output file path must end in .ivf to output an IVF file.
#   7 - extra flags
#       Note: Extra flags currently supports a special case: when set to "-"
#             input is piped to vpxenc via cat.
vpxenc() {
  encoder=${LIBVPX_BIN_PATH}/vpxenc
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

  if [ "${extra_flags}" = "-" ]; then
    unset extra_flags
    pipe_input=yes
  else
    unset pipe_input
  fi

  if [ -z "${pipe_input}" ]; then
    ${encoder} --codec=${codec} --width=${width} --height=${height} \
        --limit=${frames} ${use_ivf} ${extra_flags} -o $output $input \
        > /dev/null 2>&1
  else
    cat ${input} | ${encoder} --codec=${codec} --width=${width} \
        --height=${height} --limit=${frames} ${use_ivf} ${extra_flags} \
        -o $output - > /dev/null 2>&1
  fi

  if [ ! -e $output ]; then
    # Return non-zero exit status: output file doesn't exist, so something
    # definitely went wrong.
    return 1
  fi
}

# Filters strings from positional parameter one using the filter specified by
# positional parameter two. Filter behavior depends on the presence of a third
# positional parameter. When parameter three is present, strings that match the
# filter are excluded. When omitted, strings matching the filter are included.
# The filtered string is echoed to stdout.
filter_strings() {
  strings=$1
  filter=$2
  exclude=$3

  if [ -n "${exclude}" ]; then
    # When positional parameter three exists the caller wants to remove strings.
    # Tell grep to invert matches using the -v argument.
    exclude=-v
  else
    unset exclude
  fi

  if [ -n "${filter}" ]; then
    for s in ${strings}; do
      if echo $s | egrep -q ${exclude} ${filter} > /dev/null 2>&1; then
        filtered_strings="${filtered_strings} $s"
      fi
    done
  else
    filtered_strings=${strings}
  fi
  echo ${filtered_strings}
}

# Runs user test functions passed via positional parameters one and two.
# Funtions in positional parameter one are treated as environment verification
# functions and are run unconditionally. Functions in positional parameter two
# are run according to the rules specified in vpx_test_usage().
run_tests() {
  env_tests="verify_vpx_test_environment $1"
  tests_to_filter="$2"

  if [ "${VPX_TEST_RUN_DISABLED_TESTS}" != "yes" ]; then
    # Filter out DISABLED tests.
    tests_to_filter=$(filter_strings "${tests_to_filter}" ^DISABLED exclude)
  fi

  if [ -n "${VPX_TEST_FILTER}" ]; then
    # Remove tests not matching the user's filter.
    tests_to_filter=$(filter_strings "${tests_to_filter}" ${VPX_TEST_FILTER})
  fi

  tests_to_run="${env_tests} ${tests_to_filter}"

  check_git_hashes

  # Run tests.
  for test in ${tests_to_run}; do
    test_begin $test
    $test
    [ "${VPX_TEST_VERBOSE_OUTPUT}" = "yes" ] && echo PASS $test
    test_end $test
  done

  tested_configuration="$(test_configuration_target) @ $(current_hash)"
  echo $(basename ${0%.*}): Done, all tests pass for ${tested_configuration}.
}

vpx_test_usage() {
cat << EOF
  Usage: ${0##*/} [-bcdftv]
    b <path to libvpx binaries directory>
    c <path to libvpx config directory>
    d: Run disabled tests (including disabled tests)
    f <filter>: User test filter. Only tests matching filter are run.
    t <path to libvpx test data directory>
    v: Verbose output.

    When the -b option is not specified the script attempts to use
    \$LIBVPX_BIN_PATH and then the current directory.

    When the -c option is not specified the script attempts to use
    \$LIBVPX_CONFIG_PATH and then the current directory.

    When the -t option is not specified the script attempts to use
    \$LIBVPX_TEST_DATA_PATH and then the current directory.
EOF
}

VPX_TEST_OPTIONS="b:c:df:t:v?"
if command -v getopt > /dev/null 2>&1; then
  # getopt is available, use it to parse the command line.
  args=$(getopt ${VPX_TEST_OPTIONS} $*)

  if [ $? -ne 0 ]; then
    vpx_test_usage
    exit 1
  fi

  set -- ${args}
  while [ $# -gt 0 ]; do
    case "$1" in
      (-b)
        LIBVPX_BIN_PATH=$2
        shift
        ;;
      (-c)
        LIBVPX_CONFIG_PATH=$2
        shift
        ;;
      (-d)
        VPX_TEST_RUN_DISABLED_TESTS=yes
        ;;
      (-f)
        VPX_TEST_FILTER=$2
        shift
        ;;
      (-t)
        LIBVPX_TEST_DATA_PATH=$2
        shift
        ;;
      (-v)
        VPX_TEST_VERBOSE_OUTPUT=yes
        ;;
      (-?|*)
        vpx_test_usage
        exit 1
        ;;
    esac
    shift
  done
elif command -v getopts > /dev/null 2>&1; then
  # getopts is available, use it to parse the command line.
  while getopts ${VPX_TEST_OPTIONS} arg_name; do
    case ${arg_name} in
      (b)
        LIBVPX_BIN_PATH="${OPTARG}"
        ;;
      (c)
        LIBVPX_CONFIG_PATH="${OPTARG}"
        ;;
      (d)
        VPX_TEST_RUN_DISABLED_TESTS=yes
        ;;
      (f)
        VPX_TEST_FILTER="${OPTARG}"
        ;;
      (t)
        LIBVPX_TEST_DATA_PATH="${OPTARG}"
        ;;
      (v)
        VPX_TEST_VERBOSE_OUTPUT=yes
        ;;
      (?|*)
        vpx_test_usage
        exit 1
        ;;
      esac
  done
else
cat << EOF
  Warning: command line parsing is not available: Arguments cannot be parsed.
           To control behavior of ${0##*/} use the following environment
           variables.
             LIBVPX_BIN_PATH
             LIBVPX_CONFIG_PATH
             LIBVPX_TEST_DATA_PATH
             VPX_TEST_VERBOSE_OUTPUT (To enable: Must be set to yes)
             VPX_TEST_FILTER
             VPX_TEST_RUN_DISABLED_TESTS (To enable: Must be set to yes)
EOF
fi

LIBVPX_BIN_PATH=${LIBVPX_BIN_PATH:-.}
LIBVPX_CONFIG_PATH=${LIBVPX_CONFIG_PATH:-.}
LIBVPX_TEST_DATA_PATH=${LIBVPX_TEST_DATA_PATH:-.}

# Create temporary output directory, and a trap to clean it up.
OUTPUT_DIR=$(mktemp -d)
trap cleanup EXIT

echo LIBVPX_BIN_PATH=$LIBVPX_BIN_PATH
echo LIBVPX_CONFIG_PATH=$LIBVPX_CONFIG_PATH
echo LIBVPX_TEST_DATA_PATH=$LIBVPX_TEST_DATA_PATH
echo VPX_TEST_VERBOSE_OUTPUT=$VPX_TEST_VERBOSE_OUTPUT
echo VPX_TEST_FILTER=$VPX_TEST_FILTER
echo VPX_TEST_RUN_DISABLED_TESTS=$VPX_TEST_RUN_DISABLED_TESTS

