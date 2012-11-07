/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdio.h>
#include "tools_common.h"
#if defined(_WIN32) || defined(__OS2__)
#include <io.h>
#include <fcntl.h>

#ifdef __OS2__
#define _setmode    setmode
#define _fileno     fileno
#define _O_BINARY   O_BINARY
#endif
#endif

<<<<<<< HEAD   (82b1a3 Merge other top-level C code)
FILE *set_binary_mode(FILE *stream) {
  (void)stream;
#if defined(_WIN32) || defined(__OS2__)
  _setmode(_fileno(stream), _O_BINARY);
=======
FILE* set_binary_mode(FILE *stream)
{
    (void)stream;
#if defined(_WIN32) || defined(__OS2__)
    _setmode(_fileno(stream), _O_BINARY);
>>>>>>> BRANCH (3c8007 Merge "ads2gas.pl: various enhancements to work with flash.")
#endif
  return stream;
}
