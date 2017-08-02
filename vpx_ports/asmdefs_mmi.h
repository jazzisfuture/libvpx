/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_PORTS_ASMDEFS_MMI_H_
#define VPX_PORTS_ASMDEFS_MMI_H_

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"

#if HAVE_MMI

#if HAVE_MIPS64
#define mips_reg int64_t
#define PTR_ADDU "daddu "
#define PTR_ADDIU "daddiu "
#define PTR_ADDI "daddi "
#define PTR_SUBU "dsubu "
#define PTR_L "ld "
#define PTR_SRA "dsra "
#define PTR_SRL "dsrl "
#define PTR_SLL "dsll "
#define PTR_MTC1 "dmtc1 "
#define PTR_LI "dli "
#else
#define mips_reg int32_t
#define PTR_ADDU "addu "
#define PTR_ADDIU "addiu "
#define PTR_ADDI "daddi "
#define PTR_SUBU "subu "
#define PTR_L "lw "
#define PTR_SRA "sra "
#define PTR_SRL "srl "
#define PTR_SLL "sll "
#define PTR_MTC1 "mtc1 "
#define PTR_LI "li "
#endif /* HAVE_MIPS64 */

#endif /* HAVE_MMI */

#endif /* VPX_PORTS_ASMDEFS_MMI_H_ */
