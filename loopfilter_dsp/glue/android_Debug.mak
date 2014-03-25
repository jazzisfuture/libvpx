# pakman tree build file
_@ ?= @

.PHONY: MAKE_D_3_LIBDIR MAKE_D_4_INCDIR tree MAKE_D_3_LIBDIR_clean MAKE_D_4_INCDIR_clean tree_clean MAKE_D_3_LIBDIR_ADSPMSGD MAKE_D_3_LIBDIR_clean_ADSPMSGD

tree: MAKE_D_3_LIBDIR MAKE_D_3_LIBDIR MAKE_D_4_INCDIR MAKE_D_4_INCDIR
	$(call job,,make V=android_Debug,making .)

tree_clean: MAKE_D_3_LIBDIR_clean MAKE_D_3_LIBDIR_clean MAKE_D_4_INCDIR_clean MAKE_D_4_INCDIR_clean
	$(call job,,make V=android_Debug clean,cleaning .)

MAKE_D_3_LIBDIR_ADSPMSGD: MAKE_D_4_INCDIR MAKE_D_4_INCDIR
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/adspmsgd,make V=android_Debug,making $(HEXAGON_SDK_ROOT)/lib/common/adspmsgd)

MAKE_D_3_LIBDIR_clean_ADSPMSGD: MAKE_D_4_INCDIR_clean MAKE_D_4_INCDIR_clean
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/adspmsgd,make V=android_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/adspmsgd)

MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_Debug,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_Debug,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

W := $(findstring ECHO,$(shell echo))# W => Windows environment
@LOG = $(if $W,$(TEMP)\\)$@-build.log

C = $(if $1,cd $1 && )$2
job = $(_@)echo $3 && ( $C )> $(@LOG) && $(if $W,del,rm) $(@LOG) || ( echo ERROR $3 && $(if $W,type,cat) $(@LOG) && $(if $W,del,rm) $(@LOG) && exit 1)
ifdef VERBOSE
  job = $(_@)echo $3 && $C
endif
