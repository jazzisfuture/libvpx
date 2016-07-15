/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed  by a BSD-style license that can be
 *  found in the LICENSE file in the root of the source tree. An additional
 *  intellectual property  rights grant can  be found in the  file PATENTS.
 *  All contributing  project authors may be  found in the AUTHORS  file in
 *  the root of the source tree.
 */


#ifndef VP9_COMMON_VP9_MATX_ENUMS_H_
#define VP9_COMMON_VP9_MATX_ENUMS_H_

#include "vpx/vpx_integer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MATX;

typedef void* MATX_PTR;
typedef const void* CONST_MATX_PTR;

typedef enum {
  MATX_NO_TYPE,  // no type assigned
  TYPE_8U,       // uint8_t
  TYPE_8S,       // int8_t
  TYPE_16U,      // uint16_t
  TYPE_16S,      // int16_t
  TYPE_32U,      // uint32_t
  TYPE_32S,      // int32_t
  TYPE_32F,      // float
  TYPE_64F,      // double
  MATX_NTYPES,   // size of enum
} MATX_TYPE;

typedef void ELEMTYPE;

typedef uint8_t ELEMTYPE_8U;
typedef  int8_t ELEMTYPE_8S;

typedef uint16_t ELEMTYPE_16U;
typedef  int16_t ELEMTYPE_16S;

typedef uint32_t ELEMTYPE_32U;
typedef  int32_t ELEMTYPE_32S;

typedef  float ELEMTYPE_32F;
typedef double ELEMTYPE_64F;


#endif  // VP9_COMMON_VP9_MATX_ENUMS_H_
