#!/usr/bin/python
##  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##
"""Performs style checking on each diff hunk."""
import getopt
import os
import StringIO
import subprocess
import sys

import diff


SHORT_OPTIONS = "h"
LONG_OPTIONS = ["help"]

TOPLEVEL_CMD = ["git", "rev-parse", "--show-toplevel"]
DIFF_CMD = ["git", "diff-index", "-u", "--cached", "HEAD", "--"]
SHOW_CMD = ["git", "show"]
CPPLINT_FILTERS = ["-readability/casting", "-runtime/int"]


class Usage(Exception):
  def __init__(self, msg):
    super(Usage, self).__init__()
    self.msg = msg


def main(argv=None):
  if argv is None:
    argv = sys.argv
  try:
    try:
      opts, _ = getopt.getopt(argv[1:], SHORT_OPTIONS, LONG_OPTIONS)
    except getopt.error, msg:
      raise Usage(msg)

    # process options
    for o, _ in opts:
      if o in ("-h", "--help"):
        print __doc__
        sys.exit(0)

    # Find the fully qualified path to the root of the tree
    tl = subprocess.Popen(TOPLEVEL_CMD, stdout=subprocess.PIPE)
    tl = tl.communicate()[0].strip()

    # Build the command line to execute cpplint
    cpplint_cmd = [os.path.join(tl, "tools", "cpplint.py"),
                   "--filter=" + ",".join(CPPLINT_FILTERS),
                   "-"]

    # Get a list of all affected lines
    file_affected_line_map = {}
    p = subprocess.Popen(DIFF_CMD, stdout=subprocess.PIPE)
    stdout = p.communicate()[0]
    for hunk in diff.ParseDiffHunks(StringIO.StringIO(stdout)):
        filename = hunk.right.filename[2:]
        if filename not in file_affected_line_map:
            file_affected_line_map[filename] = set()
        file_affected_line_map[filename].update(hunk.right.delta_line_nums)

    # Run each affected file through cpplint
    for filename, affected_lines in file_affected_line_map.iteritems():
        if filename.split(".")[-1] not in ("c", "h", "cpp"):
            continue
        show_cmd = SHOW_CMD + [":" + filename]
        show = subprocess.Popen(show_cmd, stdout=subprocess.PIPE)
        lint = subprocess.Popen(cpplint_cmd, stdin=show.stdout,
                                stderr=subprocess.PIPE)
        lint_out = lint.communicate()[1]
        for line in lint_out.split("\n"):
            fields = line.split(":")
            if fields[0] != "-":
                continue
            warning_line_num = int(fields[1])
            if warning_line_num in affected_lines:
                print "%s:%d:%s"%(filename, warning_line_num,
                                  ":".join(fields[2:]))

  except Usage, err:
    print >>sys.stderr, err.msg
    print >>sys.stderr, "for help use --help"
    return 2

if __name__ == "__main__":
    sys.exit(main())
