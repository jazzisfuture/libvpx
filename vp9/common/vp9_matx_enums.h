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
#include "vpx_dsp/vpx_dsp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Infer dependent template types/variables to MATX_SUFFIX.
// I use them in *.def templated includes.
#define _MATX_JOIN_TOKENS(a, b) a##b
#define MATX_JOIN_TOKENS(a, b) _MATX_JOIN_TOKENS(a, b)

#define TOUPPER_TEMP_PREFIX_x
#define TOUPPER_TEMP_PREFIX_8u _8U
#define TOUPPER_TEMP_PREFIX_8s _8S
#define TOUPPER_TEMP_PREFIX_16u _16U
#define TOUPPER_TEMP_PREFIX_16s _16S
#define TOUPPER_TEMP_PREFIX_32u _32U
#define TOUPPER_TEMP_PREFIX_32s _32S
#define TOUPPER_TEMP_PREFIX_64u _64U
#define TOUPPER_TEMP_PREFIX_64s _64S
#define TOUPPER_TEMP_PREFIX_32f _32f
#define TOUPPER_TEMP_PREFIX_64f _64f

#define MATX_SUFFIX_TOUPPER(suffix) \
  MATX_JOIN_TOKENS(TOUPPER_TEMP_PREFIX_, suffix)

#define T_MATX_STRUCT MATX_JOIN_TOKENS(MATX, MATX_SUFFIX_TOUPPER(MATX_SUFFIX))

#define T_MATX_TYPEID MATX_JOIN_TOKENS(TYPE, MATX_SUFFIX_TOUPPER(MATX_SUFFIX))

#define T_MATX_ELEMTYPE \
  MATX_JOIN_TOKENS(MATX_ELEMTYPE, MATX_SUFFIX_TOUPPER(MATX_SUFFIX))

#define T_MATX_INTEGRAL_ELEMTYPE \
  MATX_JOIN_TOKENS(MATX_INTEGRAL_ELEMTYPE, MATX_SUFFIX_TOUPPER(MATX_SUFFIX))

#define MATX_DEFINE_FUNC(funcname)                         \
  MATX_JOIN_TOKENS(MATX_JOIN_TOKENS(vp9_mat, MATX_SUFFIX), \
                   MATX_JOIN_TOKENS(_, funcname))

#define MATX_APPLY_FUNC(funcname) MATX_DEFINE_FUNC(funcname)

// This is little hacky but allows pointer
// conversion to the super type without warning
typedef void *MATX_PTR;
typedef const void *CONST_MATX_PTR;

typedef enum {
  MATX_BORDER_REPEAT,
  MATX_BORDER_REFLECT,
  MATX_BORDER_NTYPES,
} MATX_BORDER_TYPE;

typedef enum {
  MATX_NO_TYPE,  // no type assigned
  TYPE_8U,       // uint8_t
  TYPE_8S,       // int8_t
  TYPE_16U,      // uint16_t
  TYPE_16S,      // int16_t
  TYPE_32U,      // uint32_t
  TYPE_32S,      // int32_t
  TYPE_64U,      // uint64_t
  TYPE_64S,      // int64_t
  TYPE_32F,      // float
  TYPE_64F,      // double
  MATX_NTYPES,   // size of enum
} MATX_TYPE;

typedef void MATX_ELEMTYPE;

typedef uint8_t MATX_ELEMTYPE_8U;
typedef int8_t MATX_ELEMTYPE_8S;

typedef int32_t MATX_INTEGRAL_ELEMTYPE_8U;
typedef int32_t MATX_INTEGRAL_ELEMTYPE_8S;

typedef uint16_t MATX_ELEMTYPE_16U;
typedef int16_t MATX_ELEMTYPE_16S;

typedef int64_t MATX_INTEGRAL_ELEMTYPE_16U;
typedef int64_t MATX_INTEGRAL_ELEMTYPE_16S;

typedef uint32_t MATX_ELEMTYPE_32U;
typedef int32_t MATX_ELEMTYPE_32S;

typedef int64_t MATX_INTEGRAL_ELEMTYPE_32U;
typedef int64_t MATX_INTEGRAL_ELEMTYPE_32S;

typedef uint64_t MATX_ELEMTYPE_64U;
typedef int64_t MATX_ELEMTYPE_64S;

typedef int64_t MATX_INTEGRAL_ELEMTYPE_64U;
typedef int64_t MATX_INTEGRAL_ELEMTYPE_64S;

typedef float MATX_ELEMTYPE_32F;
typedef double MATX_ELEMTYPE_64F;

typedef double MATX_INTEGRAL_ELEMTYPE_32F;
typedef double MATX_INTEGRAL_ELEMTYPE_64F;

#endif  // VP9_COMMON_VP9_MATX_ENUMS_H_
