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
##  This file runs all of the tests for the libvpx examples.
##
. $(dirname $0)/tools_common.sh

# Source each test script so that exporting variables can be avoided.
. $(dirname $0)/decode_to_md5.sh
. $(dirname $0)/simple_decoder.sh
. $(dirname $0)/simple_encoder.sh
