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
    rm -rf ${OUTPUT_DIR}
  fi
}

# This script requires that the LIBVPX_BUILD_PATH and
# LIBVPX_TEST_DATA_PATH variables are in the environment: Confirm that
# the variables are set and that they both evaluate to directory paths.
verify_vpx_test_environment() {
  if [ ! -d "${LIBVPX_BUILD_PATH}" ]; then
    echo "The LIBVPX_BUILD_PATH environment variable must be set."
    return 1
  fi
  if [ ! -d "${LIBVPX_TEST_DATA_PATH}" ]; then
    echo "The LIBVPX_TEST_DATA_PATH environment variable must be set."
    return 1
  fi
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

# Filters strings from positional parameter one using the filter specified by
# positional parameter two. Filter behavior depends on the presence of a third
# positional parameter. When parameter three is present, string that match the
# filterare excluded. When omitted, strings matching the filter are included.
# The filtered string is echoed to stdout.
filter_strings() {
  strings=$1
  filter=$2
  exclude=$3

  if [ -n "${exclude}" ]; then
    # When positional parameter three exists the caller wants to remove strings.
    # Tell grep to invert matches using the -v argument.
    exclude=-v 
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
  tests_to_filter=$2

  if [ "${VPX_TEST_RUN_DISABLED_TESTS}" != "yes" ]; then
    # Filter out DISABLED tests.
    tests_to_filter=$(filter_strings "${vpxenc_tests}" ^DISABLED exclude)
  fi

  if [ -n "${VPX_TEST_FILTER}" ]; then
    # Remove tests not matching the user's filter.
    tests_to_filter=$(filter_strings "${tests_to_filter}" ${VPX_TEST_FILTER})
  fi

  tests_to_run="${env_tests} ${tests_to_filter}"

  # Run tests.
  for test in ${tests_to_run}; do
    test_begin $test
    $test
    test_end $test
  done

  echo $(basename ${0%.*}): Done, all tests pass.
}

vpx_test_usage() {
cat << EOF
  Usage: ${0##*/} [-bdft]
    b <path to libvpx build directory>
    d: Run disabled tests (including disabled tests)
    f <filter>: User test filter. Only tests matching filter are run.
    t <path to libvpx test data directory>:

    When the -b option is not specified the script attempts to use
    \$LIBVPX_BUILD_PATH and then the current directory.

    When the -t option is not specified the script attempts to use
    \$LIBVPX_TEST_DATA_PATH and then the current directory.
EOF
}

args=$(getopt b:df:t: $*)

if [ $? -ne 0 ]; then
  vpx_test_usage
  exit 1
fi

set -- ${args}
while [ $# -gt 0 ]; do
  case "$1" in
    (-b)
      LIBVPX_BUILD_PATH=$2
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
  esac
  shift
done

LIBVPX_BUILD_PATH=${LIBVPX_BUILD_PATH:-.}
LIBVPX_TEST_DATA_PATH=${LIBVPX_TEST_DATA_PATH:-.}

# Create temporary output directory, and a trap to clean it up.
OUTPUT_DIR=$(mktemp -d)
trap cleanup EXIT

