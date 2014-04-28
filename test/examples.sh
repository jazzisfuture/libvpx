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
##  This file tests the libvpx decode_to_md5 example. To add new tests to this
##  file, do the following:
##    1. Write a shell function (this is your test).
##    2. Add the function to decode_to_md5_tests (on a new line).
##
. $(dirname $0)/tools_common.sh

# Source each test script so that exporting variables can be avoided.
. $(dirname $0)/decode_to_md5.sh
. $(dirname $0)/simple_decoder.sh
. $(dirname $0)/simple_encoder.sh
