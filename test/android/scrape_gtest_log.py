# Copyright (c) 2014 The WebM project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

"""Standalone script which parses a gtest log for json and returns it as an
array.  This script is used by the libvpx waterfall to gather json
results mixed in with gtest logs.  This is dubious software engineering"""

import json
import sys
import re

def main():
  blob = sys.stdin.read()
  jsonString = "[" + ",".join("{" + x + "}" for x in
      re.findall(r"{([^}]*.?)}", blob)) + "]"
  print(json.dumps(json.loads(jsonString), indent=4, sort_keys=True))

if __name__ == '__main__':
  sys.exit(main())
