# This file.
DSP_SRCS-yes += vpx_dsp.mk

ifeq ($(CONFIG_ENCODERS),yes)
DSP_SRCS-yes            += sad.c

DSP_SRCS-$(HAVE_MEDIA)  += arm/sad_media.asm
DSP_SRCS-$(HAVE_NEON)   += arm/sad_neon.c

DSP_SRCS-$(HAVE_MMX)    += x86/sad_mmx.asm
DSP_SRCS-$(HAVE_SSE2)   += x86/sad_sse2.asm
DSP_SRCS-$(HAVE_SSE2)   += x86/sad4d_sse2.asm
DSP_SRCS-$(HAVE_SSE3)   += x86/sad_sse3.asm
DSP_SRCS-$(HAVE_SSSE3)  += x86/sad_ssse3.asm
DSP_SRCS-$(HAVE_SSE4_1) += x86/sad_sse4.asm
DSP_SRCS-$(HAVE_AVX2)   += x86/sad4d_avx2.c
DSP_SRCS-$(HAVE_AVX2)   += x86/sad_avx2.c

#DSP_SRCS-$(HAVE_SSE2) += x86/vp9_highbd_sad_sse2.asm
#DSP_SRCS-$(HAVE_SSE2) += x86/vp9_highbd_sad4d_sse2.asm
endif

DSP_SRCS-no += $(DSP_SRCS_REMOVE-yes)

DSP_SRCS-yes += vpx_dsp_rtcd.c
DSP_SRCS-yes += vpx_dsp_rtcd_defs.pl

$(eval $(call rtcd_h_template,vpx_dsp_rtcd,vpx_dsp/vpx_dsp_rtcd_defs.pl))
