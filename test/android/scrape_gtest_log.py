# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Standalone script which parses a gtest log for json and returns it as an
array.  This is dubious software engineering"""

import json
import sys
import re

def main():
  blob = sys.stdin.read()
  i =  iter(re.findall("\{([^}]*.?)\}", blob))
  jsonstring = '['
  for scrapedjson in i:
    jsonstring = jsonstring + "{" + scrapedjson + "}"
    if i.__length_hint__() > 0:
      jsonstring = jsonstring + ","
  jsonstring = jsonstring + "]"
  print json.dumps(json.loads(jsonstring), indent=4, sort_keys=True)

if __name__ == '__main__':
  sys.exit(main())