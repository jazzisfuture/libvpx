LIBVPX_TEST_SRCS-yes += test.mk
ifeq ($(CONFIG_VP8_ENCODER)$(CONFIG_VP8_DECODER),yesyes)
# These tests require both the encoder and decoder to be built.
LIBVPX_TEST_SRCS-yes += boolcoder_test.cc
endif

# These tests require only the encoder.
ifeq ($(CONFIG_VP8_ENCODER), yes)
LIBVPX_TEST_SRCS-yes += config_test.cc
LIBVPX_TEST_SRCS-yes += encode_test_driver.cc
LIBVPX_TEST_SRCS-yes += encode_test_driver.h
LIBVPX_TEST_SRCS-yes += fdct4x4_test.cc
LIBVPX_TEST_SRCS-yes += i420_video_source.h
LIBVPX_TEST_SRCS-yes += keyframe_test.cc
LIBVPX_TEST_SRCS-yes += resize_test.cc
LIBVPX_TEST_SRCS-yes += video_source.h

LIBVPX_TEST_DATA-yes += hantro_collage_w352h288.yuv
endif

# These tests are valid for both the encoder and decoder.
LIBVPX_TEST_SRCS-yes += idctllm_test.cc
LIBVPX_TEST_SRCS-yes += intrapred_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_POSTPROC) += pp_filter_test.cc
LIBVPX_TEST_SRCS-yes += test_libvpx.cc
