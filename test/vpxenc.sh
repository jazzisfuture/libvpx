#!/bin/sh

vpxenc_usage() {
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

# TODO: Try to move this to a function...
args=$(getopt b:df:t: $*)

if [ $? -ne 0 ]; then
  vpxenc_usage
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
      VPXENC_RUN_DISABLED_TESTS=yes
      ;;
    (-f)
      VPXENC_FILTER=$2
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

# TODO: Remove this debug spam
echo LIBVPX_TEST_DATA_PATH=$LIBVPX_TEST_DATA_PATH
echo LIBVPX_BUILD_PATH=$LIBVPX_BUILD_PATH
echo VPXENC_RUN_DISABLED_TESTS=$VPXENC_RUN_DISABLED_TESTS
echo VPXENC_FILTER=$VPXENC_FILTER

USAGE_FUNCTION=vpxenc_usage
. $(dirname $0)/tools_common.sh

YUV_RAW_INPUT=${LIBVPX_TEST_DATA_PATH}/hantro_collage_w352h288.yuv
YUV_RAW_INPUT_WIDTH=352
YUV_RAW_INPUT_HEIGHT=288
TEST_FRAMES=10

vpxenc_tests="vpxenc_vp8_ivf
              vpxenc_vp8_webm
              vpxenc_vp9_ivf
              vpxenc_vp9_webm
              DISABLED_vpxenc_vp9_ivf_lossless"

# Environment check: Make sure input is available.
vpxenc_verify_environment() {
  test_begin $FUNCNAME
  if [ ! -e ${YUV_RAW_INPUT} ]; then
    echo "The file ${YUV_RAW_INPUT##*/} must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
  test_end $FUNCNAME
}

vpxenc_can_encode_vp8() {
  if [ $(vpxenc_available) = "yes" ] && \
     [ $(vp8_encode_available) = "yes" ]; then
    echo yes
  fi
}

vpxenc_can_encode_vp9() {
  if [ $(vpxenc_available) = "yes" ] && \
     [ $(vp9_encode_available) = "yes" ]; then
    echo yes
  fi
}

vpxenc_vp8_ivf() {
  test_begin $FUNCNAME
  if [ $(vpxenc_can_encode_vp8) = "yes" ]; then
    vpxenc vp8 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
        ${YUV_RAW_INPUT} vp8.ivf
  fi
  test_end $FUNCNAME
}

vpxenc_vp8_webm() {
  test_begin $FUNCNAME
  if [ $(vpxenc_can_encode_vp8) = "yes" ] &&
     [ $(webm_io_available) = "yes" ] ; then
    vpxenc vp8 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
        ${YUV_RAW_INPUT} vp8.webm
  fi
  test_end $FUNCNAME
}

vpxenc_vp9_ivf() {
  test_begin $FUNCNAME
  if [ $(vpxenc_can_encode_vp9) = "yes" ]; then
    vpxenc vp9 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
        ${YUV_RAW_INPUT} vp9.ivf
  fi
  test_end $FUNCNAME
}

vpxenc_vp9_webm() {
  test_begin $FUNCNAME
  if [ $(vpxenc_can_encode_vp9) = "yes" ] &&
     [ $(webm_io_available) = "yes" ] ; then
    vpxenc vp9 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
        ${YUV_RAW_INPUT} vp9.webm
  fi
  test_end $FUNCNAME
}

DISABLED_vpxenc_vp9_ivf_lossless() {
  test_begin $FUNCNAME
  if [ $(vpxenc_can_encode_vp9) = "yes" ]; then
    vpxenc vp9 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
        ${YUV_RAW_INPUT} vp9_lossless.ivf --lossless
  fi
  test_end $FUNCNAME
}

vpxenc_verify_environment

echo vpxenc_tests=$vpxenc_tests

if [ ${VPXENC_RUN_DISABLED_TESTS} = "yes" ]; then
  # Filter out DISABLED tests.
  tests_to_run=$(filter_strings "${vpxenc_tests}" ^DISABLED)
  echo tests_to_run=$tests_to_run after filter_strings ^DISABLED
fi

if [ -n ${VPXENC_FILTER} ]; then
  # filter out user filtered tests.
  tests_to_run=$(filter_strings ${tests_to_run} ${VPXENC_FILTER})
  echo tests_to_run=$tests_to_run after filter_strings ${VPXENC_FILTER}
fi

# Run tests.
for test in ${tests_to_run}; do
  $test
done

echo Done: All tests pass.

