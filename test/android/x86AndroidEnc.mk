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
#$(info $(LOCAL_PATH))
BINDINGS_DIR := $(CUR_WD)/../..
LOCAL_PATH := $(CUR_WD)/../..

#libwebm
include $(CLEAR_VARS)
include $(BINDINGS_DIR)/third_party/libwebm/Android.mk
LOCAL_PATH := $(CUR_WD)/../..

#libvpx
include $(CLEAR_VARS)
LOCAL_STATIC_LIBRARIES := libwebm
include $(BINDINGS_DIR)/build/make/x86Android.mk
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

#libvpx_test
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := vpxenc
LOCAL_STATIC_LIBRARIES := libwebm
LOCAL_SHARED_LIBRARIES := vpx
LOCAL_C_INCLUDES := $(LOCAL_PATH)/third_party/libyuv/include/
include $(LOCAL_PATH)/examples.mk
include $(LOCAL_PATH)/vp9/vp9cx.mk
include $(LOCAL_PATH)/vp8/vp8cx.mk
LOCAL_SRC_FILES := $(sort $(filter %.cc %.c, $(vpxenc.SRCS)))
include $(BUILD_EXECUTABLE)

