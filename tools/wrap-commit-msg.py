#!/usr/bin/env python
##  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##
"""Wraps paragraphs of text, preserving manual formatting

This is like fold(1), but has the special convention of not modifying lines
that start with whitespace.
"""

__author__ = "jkoleszar@google.com"
import textwrap
import sys

def wrap(text):
    return (textwrap.fill(text, 70) + '\n') if text else ""

def main(fileobj):
    text = ""
    output = ""
    while True:
        line = fileobj.readline()
        if not line:
            break

        if line.lstrip() == line:
            text += line
        else:
            output += wrap(text)
            text=""
            output += line
    output += wrap(text)

    # Replace the file or write to stdout.
    if fileobj == sys.stdin:
        fileobj = sys.stdout
    else:
        fileobj.truncate(0)
    fileobj.write(output)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        main(open(sys.argv[1], "r+"))
    else:
        main(sys.stdin)
