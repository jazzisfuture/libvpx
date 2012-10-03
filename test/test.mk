LIBVPX_TEST_SRCS-yes += acm_random.h
LIBVPX_TEST_SRCS-yes += test.mk
LIBVPX_TEST_SRCS-yes += test_libvpx.cc
LIBVPX_TEST_SRCS-yes += util.h

##
## BLACK BOX TESTS
##
## Black box tests only use the public API.
##
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += altref_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += config_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += encode_test_driver.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += encode_test_driver.h
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += i420_video_source.h
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += keyframe_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += resize_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += video_source.h
LIBVPX_TEST_SRCS-$(CONFIG_VP8_DECODER) += decode_test_driver.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_DECODER) += decode_test_driver.h

##
## WHITE BOX TESTS
##
## Whitebox tests invoke functions not exposed via the public API. Certain
## shared library builds don't make these functions accessible.
##
ifeq ($(CONFIG_SHARED),)

# These tests require both the encoder and decoder to be built.
ifeq ($(CONFIG_VP8_ENCODER)$(CONFIG_VP8_DECODER),yesyes)
LIBVPX_TEST_SRCS-yes                   += boolcoder_test.cc
endif

LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += fdct4x4_test.cc
LIBVPX_TEST_SRCS-yes                   += idctllm_test.cc
LIBVPX_TEST_SRCS-yes                   += intrapred_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_POSTPROC)    += pp_filter_test.cc
LIBVPX_TEST_SRCS-yes                   += sad_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += set_roi.cc
LIBVPX_TEST_SRCS-yes                   += sixtap_predict_test.cc
LIBVPX_TEST_SRCS-$(CONFIG_VP8_ENCODER) += subtract_test.cc

endif


##
## TEST DATA
##
LIBVPX_TEST_DATA-$(CONFIG_VP8_ENCODER) += hantro_collage_w352h288.yuv
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-001.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-002.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-003.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-004.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-005.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-006.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-007.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-008.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-009.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-010.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-011.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-012.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-013.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-014.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-015.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-016.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-00-comprehensive-017.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-01-intra-1400.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-01-intra-1411.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-01-intra-1416.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-01-intra-1417.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-02-inter-1402.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-02-inter-1412.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-02-inter-1418.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-02-inter-1424.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1401.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1403.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1407.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1408.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1409.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1410.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1413.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1414.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1415.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1425.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1426.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1427.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1432.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1435.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1436.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1437.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1441.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-03-segmentation-1442.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-04-partitions-1404.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-04-partitions-1405.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-04-partitions-1406.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1428.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1429.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1430.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1431.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1433.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1434.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1438.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1439.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1440.ivf
LIBVPX_TEST_DATA-$(CONFIG_VP8_DECODER) += vp80-05-sharpness-1443.ivf