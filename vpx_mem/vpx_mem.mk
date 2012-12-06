MEM_SRCS-yes += vpx_mem.mk
MEM_SRCS-yes += vpx_mem.c
MEM_SRCS-yes += vpx_mem.h
MEM_SRCS-yes += yv12allocate.c
MEM_SRCS-yes += yv12config.h
MEM_SRCS-yes += yv12extend.c
MEM_SRCS-yes += include/vpx_mem_intrnl.h
MEM_SRCS-yes += vpx_mem_rtcd.c

MEM_SRCS-$(CONFIG_MEM_TRACKER) += vpx_mem_tracker.c
MEM_SRCS-$(CONFIG_MEM_TRACKER) += include/vpx_mem_tracker.h

MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_true.c
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_resize.c
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_shrink.c
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_largest.c
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_dflt_abort.c
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_base.c
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/include
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/include/hmm_intrnl.h
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/include/cavl_if.h
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/include/hmm_cnfg.h
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/include/heapmm.h
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/include/cavl_impl.h
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_grow.c
MEM_SRCS-$(CONFIG_MEM_MANAGER) += memory_manager/hmm_alloc.c

#neon
MEM_SRCS-$(HAVE_NEON)  += arm/neon/vp8_vpxyv12_copyframe_func_neon$(ASM)
MEM_SRCS-$(HAVE_NEON)  += arm/neon/vp8_vpxyv12_copy_y_neon$(ASM)
MEM_SRCS-$(HAVE_NEON)  += arm/neon/vp8_vpxyv12_copysrcframe_func_neon$(ASM)
MEM_SRCS-$(HAVE_NEON)  += arm/neon/vp8_vpxyv12_extendframeborders_neon$(ASM)
MEM_SRCS-$(HAVE_NEON)  += arm/neon/yv12extend_arm.c


$(eval $(call asm_offsets_template,\
	                 vpx_mem_asm_offsets.asm, vpx_mem/vpx_mem_asm_offsets.c))

$(eval $(call rtcd_h_template,vpx_mem_rtcd,vpx_mem/vpx_mem_rtcd.sh))
