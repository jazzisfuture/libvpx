#!/bin/sh
LIBVPX_BUILD_PATH=${1:-.}
LIBVPX_TEST_DATA_PATH=${LIBVPX_TEST_DATA_PATH:-.}

. $(dirname $0)/tools_common.sh

YUV_RAW_INPUT=${LIBVPX_TEST_DATA_PATH}/hantro_collage_w352h288.yuv
YUV_RAW_INPUT_WIDTH=352
YUV_RAW_INPUT_HEIGHT=288
TEST_FRAMES=10

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

vpxenc_vp9_lossless() {
  test_begin $FUNCNAME
  if [ $(vpxenc_can_encode_vp9) = "yes" ]; then
    vpxenc vp9 ${YUV_RAW_INPUT_WIDTH} ${YUV_RAW_INPUT_HEIGHT} ${TEST_FRAMES} \
        ${YUV_RAW_INPUT} vp9_lossless.ivf --lossless
  fi
  test_end $FUNCNAME
}

vpxenc_verify_environment
vpxenc_vp8_ivf
vpxenc_vp8_webm
vpxenc_vp9_ivf
vpxenc_vp9_webm
vpxenc_vp9_lossless

echo Done: All tests pass.

