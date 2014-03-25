# pakman tree build file
_@ ?= @

.PHONY: MAKE_D_4_LIBDIR MAKE_D_3_LIBDIR MAKE_D_4_INCDIR tree MAKE_D_4_LIBDIR_clean MAKE_D_3_LIBDIR_clean MAKE_D_4_INCDIR_clean tree_clean

tree: MAKE_D_4_LIBDIR MAKE_D_3_LIBDIR MAKE_D_3_LIBDIR MAKE_D_4_INCDIR MAKE_D_4_LIBDIR MAKE_D_4_INCDIR
	$(call job,,make V=hexagonv5_Release_dynamic,making .)

tree_clean: MAKE_D_4_LIBDIR_clean MAKE_D_3_LIBDIR_clean MAKE_D_3_LIBDIR_clean MAKE_D_4_INCDIR_clean MAKE_D_4_LIBDIR_clean MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagonv5_Release_dynamic clean,cleaning .)

MAKE_D_4_INCDIR: 
	$(call job,../../../lib/common/rpcmem,make V=hexagonv5_Release_dynamic,making ../../../lib/common/rpcmem)

MAKE_D_4_INCDIR_clean: 
	$(call job,../../../lib/common/rpcmem,make V=hexagonv5_Release_dynamic clean,cleaning ../../../lib/common/rpcmem)

MAKE_D_3_LIBDIR: 
	$(call job,../../../lib/fastcv/dspCV,make V=hexagonv5_Release_dynamic,making ../../../lib/fastcv/dspCV)

MAKE_D_3_LIBDIR_clean: 
	$(call job,../../../lib/fastcv/dspCV,make V=hexagonv5_Release_dynamic clean,cleaning ../../../lib/fastcv/dspCV)

MAKE_D_4_LIBDIR: 
	$(call job,../../../test/common/test_util,make V=hexagonv5_Release_dynamic,making ../../../test/common/test_util)

MAKE_D_4_LIBDIR_clean: 
	$(call job,../../../test/common/test_util,make V=hexagonv5_Release_dynamic clean,cleaning ../../../test/common/test_util)

W := $(findstring ECHO,$(shell echo))# W => Windows environment
@LOG = $(if $W,$(TEMP)\\)$@-build.log

C = $(if $1,cd $1 && )$2
job = $(_@)echo $3 && ( $C )> $(@LOG) && $(if $W,del,rm) $(@LOG) || ( echo ERROR $3 && $(if $W,type,cat) $(@LOG) && $(if $W,del,rm) $(@LOG) && exit 1)
ifdef VERBOSE
  job = $(_@)echo $3 && $C
endif
