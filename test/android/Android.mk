# Copyright (c) 2013 The WebM project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.
#
# This make file builds vpx_test app for android.
# The test app itself runs on the command line through adb shell
# The paths are really messed up as the libvpx make file
# expects to be made from a parent directory.
CUR_WD := $(call my-dir)
ifeq ($(TARGET_ARCH_ABI),x86)
BINDINGS_DIR := $(CUR_WD)/../..
LOCAL_PATH := $(CUR_WD)/../..
else
BINDINGS_DIR := $(CUR_WD)/../../..
LOCAL_PATH := $(CUR_WD)/../../..
endif

#libwebm
include $(CLEAR_VARS)
ifeq ($(TARGET_ARCH_ABI),x86)
include $(BINDINGS_DIR)/third_party/libwebm/Android.mk
LOCAL_PATH := $(CUR_WD)/../..
else
include $(BINDINGS_DIR)/libvpx/third_party/libwebm/Android.mk
LOCAL_PATH := $(CUR_WD)/../../..
endif

#libvpx
include $(CLEAR_VARS)
LOCAL_STATIC_LIBRARIES := libwebm
ifeq ($(TARGET_ARCH_ABI),x86)
include $(BINDINGS_DIR)/build/make/Android.mk
else
include $(BINDINGS_DIR)/libvpx/build/make/Android.mk
endif
LOCAL_PATH := $(CUR_WD)/../..

#libgtest
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := gtest
LOCAL_C_INCLUDES := $(LOCAL_PATH)/third_party/googletest/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/third_party/googletest/src/include/
LOCAL_SRC_FILES := ./third_party/googletest/src/src/gtest-all.cc
include $(BUILD_STATIC_LIBRARY)


ifeq ($(TARGET_ARCH_ABI),x86)
include $(CLEAR_VARS)
LOCAL_MODULE := vpxdec
LOCAL_STATIC_LIBRARIES := libwebm
LOCAL_SHARED_LIBRARIES := vpx
include $(LOCAL_PATH)/examples.mk
LOCAL_C_INCLUDES := $(LOCAL_PATH)/third_party/libyuv/include/
LOCAL_SRC_FILES := $(sort $(filter %.cc %.c, $(vpxdec.SRCS)))
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := vpxenc
LOCAL_STATIC_LIBRARIES := libwebm
LOCAL_SHARED_LIBRARIES := vpx
LOCAL_C_INCLUDES := $(LOCAL_PATH)/third_party/libyuv/include/
include $(LOCAL_PATH)/examples.mk
LOCAL_SRC_FILES := $(sort $(filter %.cc %.c, $(vpxenc.SRCS)))
include $(BUILD_EXECUTABLE)

else
#libvpx_test
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libvpx_test
LOCAL_STATIC_LIBRARIES := gtest libwebm
LOCAL_SHARED_LIBRARIES := vpx
include $(LOCAL_PATH)/test/test.mk
LOCAL_C_INCLUDES := $(BINDINGS_DIR)
FILTERED_SRC := $(sort $(filter %.cc %.c, $(LIBVPX_TEST_SRCS-yes)))
LOCAL_SRC_FILES := $(addprefix ./test/, $(FILTERED_SRC))
include $(BUILD_EXECUTABLE)
endif
