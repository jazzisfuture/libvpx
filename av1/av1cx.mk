##
##  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##

AV1_CX_EXPORTS += exports_enc

AV1_CX_SRCS-yes += $(AV1_COMMON_SRCS-yes)
AV1_CX_SRCS-no  += $(AV1_COMMON_SRCS-no)
AV1_CX_SRCS_REMOVE-yes += $(AV1_COMMON_SRCS_REMOVE-yes)
AV1_CX_SRCS_REMOVE-no  += $(AV1_COMMON_SRCS_REMOVE-no)

AV1_CX_SRCS-yes += av1_cx_iface.c

AV1_CX_SRCS-yes += encoder/bitstream.c
AV1_CX_SRCS-yes += encoder/bitwriter.h
AV1_CX_SRCS-yes += encoder/context_tree.c
AV1_CX_SRCS-yes += encoder/context_tree.h
AV1_CX_SRCS-yes += encoder/variance_tree.c
AV1_CX_SRCS-yes += encoder/variance_tree.h
AV1_CX_SRCS-yes += encoder/cost.h
AV1_CX_SRCS-yes += encoder/cost.c
AV1_CX_SRCS-yes += encoder/dct.c
AV1_CX_SRCS-yes += encoder/hybrid_fwd_txfm.c
AV1_CX_SRCS-yes += encoder/hybrid_fwd_txfm.h
AV1_CX_SRCS-yes += encoder/encodeframe.c
AV1_CX_SRCS-yes += encoder/encodeframe.h
AV1_CX_SRCS-yes += encoder/encodemb.c
AV1_CX_SRCS-yes += encoder/encodemv.c
AV1_CX_SRCS-yes += encoder/ethread.h
AV1_CX_SRCS-yes += encoder/ethread.c
AV1_CX_SRCS-yes += encoder/extend.c
AV1_CX_SRCS-yes += encoder/firstpass.c
AV1_CX_SRCS-$(CONFIG_GLOBAL_MOTION) += encoder/global_motion.c
AV1_CX_SRCS-$(CONFIG_GLOBAL_MOTION) += encoder/global_motion.h
AV1_CX_SRCS-yes += encoder/block.h
AV1_CX_SRCS-yes += encoder/bitstream.h
AV1_CX_SRCS-yes += encoder/encodemb.h
AV1_CX_SRCS-yes += encoder/encodemv.h
AV1_CX_SRCS-yes += encoder/extend.h
AV1_CX_SRCS-yes += encoder/firstpass.h
AV1_CX_SRCS-yes += encoder/lookahead.c
AV1_CX_SRCS-yes += encoder/lookahead.h
AV1_CX_SRCS-yes += encoder/mcomp.h
AV1_CX_SRCS-yes += encoder/encoder.h
AV1_CX_SRCS-yes += encoder/quantize.h
AV1_CX_SRCS-yes += encoder/ratectrl.h
AV1_CX_SRCS-yes += encoder/rd.h
AV1_CX_SRCS-yes += encoder/rdopt.h
AV1_CX_SRCS-yes += encoder/tokenize.h
AV1_CX_SRCS-yes += encoder/treewriter.h
AV1_CX_SRCS-yes += encoder/mcomp.c
AV1_CX_SRCS-yes += encoder/encoder.c
AV1_CX_SRCS-yes += encoder/palette.h
AV1_CX_SRCS-yes += encoder/palette.c
AV1_CX_SRCS-yes += encoder/picklpf.c
AV1_CX_SRCS-yes += encoder/picklpf.h
AV1_CX_SRCS-$(CONFIG_LOOP_RESTORATION) += encoder/pickrst.c
AV1_CX_SRCS-$(CONFIG_LOOP_RESTORATION) += encoder/pickrst.h
AV1_CX_SRCS-yes += encoder/quantize.c
AV1_CX_SRCS-yes += encoder/ratectrl.c
AV1_CX_SRCS-yes += encoder/rd.c
AV1_CX_SRCS-yes += encoder/rdopt.c
AV1_CX_SRCS-yes += encoder/segmentation.c
AV1_CX_SRCS-yes += encoder/segmentation.h
AV1_CX_SRCS-yes += encoder/speed_features.c
AV1_CX_SRCS-yes += encoder/speed_features.h
AV1_CX_SRCS-yes += encoder/subexp.c
AV1_CX_SRCS-yes += encoder/subexp.h
AV1_CX_SRCS-yes += encoder/resize.c
AV1_CX_SRCS-yes += encoder/resize.h
AV1_CX_SRCS-$(CONFIG_INTERNAL_STATS) += encoder/blockiness.c
AV1_CX_SRCS-$(CONFIG_ANS) += encoder/buf_ans.h
AV1_CX_SRCS-$(CONFIG_ANS) += encoder/buf_ans.c

AV1_CX_SRCS-yes += encoder/tokenize.c
AV1_CX_SRCS-yes += encoder/treewriter.c
AV1_CX_SRCS-yes += encoder/aq_variance.c
AV1_CX_SRCS-yes += encoder/aq_variance.h
AV1_CX_SRCS-yes += encoder/aq_cyclicrefresh.c
AV1_CX_SRCS-yes += encoder/aq_cyclicrefresh.h
AV1_CX_SRCS-yes += encoder/aq_complexity.c
AV1_CX_SRCS-yes += encoder/aq_complexity.h
AV1_CX_SRCS-yes += encoder/temporal_filter.c
AV1_CX_SRCS-yes += encoder/temporal_filter.h
AV1_CX_SRCS-yes += encoder/mbgraph.c
AV1_CX_SRCS-yes += encoder/mbgraph.h
ifeq ($(CONFIG_DERING),yes)
AV1_CX_SRCS-yes += encoder/pickdering.c
endif
AV1_CX_SRCS-$(HAVE_SSE2) += encoder/x86/temporal_filter_apply_sse2.asm
AV1_CX_SRCS-$(HAVE_SSE2) += encoder/x86/quantize_sse2.c
ifeq ($(CONFIG_AOM_HIGHBITDEPTH),yes)
AV1_CX_SRCS-$(HAVE_SSE2) += encoder/x86/highbd_block_error_intrin_sse2.c
endif

AV1_CX_SRCS-$(HAVE_SSE2) += encoder/x86/dct_sse2.asm
AV1_CX_SRCS-$(HAVE_SSE2) += encoder/x86/error_sse2.asm

ifeq ($(ARCH_X86_64),yes)
AV1_CX_SRCS-$(HAVE_SSSE3) += encoder/x86/quantize_ssse3_x86_64.asm
endif

AV1_CX_SRCS-$(HAVE_SSE2) += encoder/x86/dct_intrin_sse2.c
AV1_CX_SRCS-$(HAVE_SSSE3) += encoder/x86/dct_ssse3.c
ifeq ($(CONFIG_AOM_HIGHBITDEPTH),yes)
AV1_CX_SRCS-$(HAVE_SSE4_1) += encoder/x86/highbd_fwd_txfm_sse4.c
AV1_CX_SRCS-$(HAVE_SSE4_1) += common/x86/highbd_inv_txfm_sse4.c
AV1_CX_SRCS-$(HAVE_SSE4_1) += encoder/x86/av1_highbd_quantize_sse4.c
endif

ifeq ($(CONFIG_EXT_INTER),yes)
AV1_CX_SRCS-yes += encoder/wedge_utils.c
AV1_CX_SRCS-$(HAVE_SSE2) += encoder/x86/wedge_utils_sse2.c
endif

AV1_CX_SRCS-$(HAVE_AVX2) += encoder/x86/error_intrin_avx2.c

ifneq ($(CONFIG_AOM_HIGHBITDEPTH),yes)
AV1_CX_SRCS-$(HAVE_NEON) += encoder/arm/neon/dct_neon.c
AV1_CX_SRCS-$(HAVE_NEON) += encoder/arm/neon/error_neon.c
endif
AV1_CX_SRCS-$(HAVE_NEON) += encoder/arm/neon/quantize_neon.c

AV1_CX_SRCS-$(HAVE_MSA) += encoder/mips/msa/error_msa.c
AV1_CX_SRCS-$(HAVE_MSA) += encoder/mips/msa/fdct4x4_msa.c
AV1_CX_SRCS-$(HAVE_MSA) += encoder/mips/msa/fdct8x8_msa.c
AV1_CX_SRCS-$(HAVE_MSA) += encoder/mips/msa/fdct16x16_msa.c
AV1_CX_SRCS-$(HAVE_MSA) += encoder/mips/msa/fdct_msa.h
AV1_CX_SRCS-$(HAVE_MSA) += encoder/mips/msa/temporal_filter_msa.c

AV1_CX_SRCS-yes := $(filter-out $(AV1_CX_SRCS_REMOVE-yes),$(AV1_CX_SRCS-yes))
