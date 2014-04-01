# pakman tree build file
_@ ?= @

.PHONY: MAKE_D_3_LIBDIR MAKE_D_4_INCDIR android_Release MAKE_D_4_LIBDIR hexagonv5_Debug_MAKE_D_3_LIBDIR hexagonv5_Debug_MAKE_D_4_INCDIR hexagonv5_Debug android_ReleaseG_MAKE_D_3_LIBDIR android_ReleaseG_MAKE_D_4_INCDIR android_ReleaseG hexagon_Release_MAKE_D_4_LIBDIR hexagon_Release_MAKE_D_3_LIBDIR hexagon_Release_MAKE_D_4_INCDIR hexagon_Release hexagonv5_Release_MAKE_D_4_LIBDIR hexagonv5_Release_MAKE_D_3_LIBDIR hexagonv5_Release_MAKE_D_4_INCDIR hexagonv5_Release hexagon_ReleaseG_MAKE_D_4_LIBDIR hexagon_ReleaseG_MAKE_D_3_LIBDIR hexagon_ReleaseG_MAKE_D_4_INCDIR hexagon_ReleaseG hexagon_Debug_MAKE_D_4_LIBDIR hexagon_Debug_MAKE_D_3_LIBDIR hexagon_Debug_MAKE_D_4_INCDIR hexagon_Debug hexagon_Debug_dynamic_MAKE_D_4_LIBDIR hexagon_Debug_dynamic_MAKE_D_3_LIBDIR hexagon_Debug_dynamic_MAKE_D_4_INCDIR hexagon_Debug_dynamic hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR hexagon_ReleaseG_dynamic hexagonv5_ReleaseG_MAKE_D_4_LIBDIR hexagonv5_ReleaseG_MAKE_D_3_LIBDIR hexagonv5_ReleaseG_MAKE_D_4_INCDIR hexagonv5_ReleaseG hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR hexagonv5_ReleaseG_dynamic hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR hexagonv5_Release_dynamic_MAKE_D_4_INCDIR hexagonv5_Release_dynamic android_Debug_MAKE_D_3_LIBDIR android_Debug_MAKE_D_4_INCDIR android_Debug hexagon_Release_dynamic_MAKE_D_4_LIBDIR hexagon_Release_dynamic_MAKE_D_3_LIBDIR hexagon_Release_dynamic_MAKE_D_4_INCDIR hexagon_Release_dynamic hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR hexagonv5_Debug_dynamic tree MAKE_D_3_LIBDIR_clean MAKE_D_4_INCDIR_clean android_Release_clean MAKE_D_4_LIBDIR_clean hexagonv5_Debug_MAKE_D_3_LIBDIR_clean hexagonv5_Debug_MAKE_D_4_INCDIR_clean hexagonv5_Debug_clean android_ReleaseG_MAKE_D_3_LIBDIR_clean android_ReleaseG_MAKE_D_4_INCDIR_clean android_ReleaseG_clean hexagon_Release_MAKE_D_4_LIBDIR_clean hexagon_Release_MAKE_D_3_LIBDIR_clean hexagon_Release_MAKE_D_4_INCDIR_clean hexagon_Release_clean hexagonv5_Release_MAKE_D_4_LIBDIR_clean hexagonv5_Release_MAKE_D_3_LIBDIR_clean hexagonv5_Release_MAKE_D_4_INCDIR_clean hexagonv5_Release_clean hexagon_ReleaseG_MAKE_D_4_LIBDIR_clean hexagon_ReleaseG_MAKE_D_3_LIBDIR_clean hexagon_ReleaseG_MAKE_D_4_INCDIR_clean hexagon_ReleaseG_clean hexagon_Debug_MAKE_D_4_LIBDIR_clean hexagon_Debug_MAKE_D_3_LIBDIR_clean hexagon_Debug_MAKE_D_4_INCDIR_clean hexagon_Debug_clean hexagon_Debug_dynamic_MAKE_D_4_LIBDIR_clean hexagon_Debug_dynamic_MAKE_D_3_LIBDIR_clean hexagon_Debug_dynamic_MAKE_D_4_INCDIR_clean hexagon_Debug_dynamic_clean hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean hexagon_ReleaseG_dynamic_clean hexagonv5_ReleaseG_MAKE_D_4_LIBDIR_clean hexagonv5_ReleaseG_MAKE_D_3_LIBDIR_clean hexagonv5_ReleaseG_MAKE_D_4_INCDIR_clean hexagonv5_ReleaseG_clean hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean hexagonv5_ReleaseG_dynamic_clean hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_Release_dynamic_MAKE_D_4_INCDIR_clean hexagonv5_Release_dynamic_clean android_Debug_MAKE_D_3_LIBDIR_clean android_Debug_MAKE_D_4_INCDIR_clean android_Debug_clean hexagon_Release_dynamic_MAKE_D_4_LIBDIR_clean hexagon_Release_dynamic_MAKE_D_3_LIBDIR_clean hexagon_Release_dynamic_MAKE_D_4_INCDIR_clean hexagon_Release_dynamic_clean hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR_clean hexagonv5_Debug_dynamic_clean tree_clean

tree: android_Release hexagonv5_Debug android_ReleaseG hexagon_Release hexagonv5_Release hexagon_ReleaseG hexagon_Debug hexagon_Debug_dynamic hexagon_ReleaseG_dynamic hexagonv5_ReleaseG hexagonv5_ReleaseG_dynamic hexagonv5_Release_dynamic android_Debug hexagon_Release_dynamic hexagonv5_Debug_dynamic
	$(call job,,make -f all.mak,making .)

tree_clean: android_Release_clean hexagonv5_Debug_clean android_ReleaseG_clean hexagon_Release_clean hexagonv5_Release_clean hexagon_ReleaseG_clean hexagon_Debug_clean hexagon_Debug_dynamic_clean hexagon_ReleaseG_dynamic_clean hexagonv5_ReleaseG_clean hexagonv5_ReleaseG_dynamic_clean hexagonv5_Release_dynamic_clean android_Debug_clean hexagon_Release_dynamic_clean hexagonv5_Debug_dynamic_clean
	$(call job,,make -f all.mak clean,cleaning .)

hexagonv5_Debug_dynamic: hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR
	$(call job,,make V=hexagonv5_Debug_dynamic,making .)

hexagonv5_Debug_dynamic_clean: hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR_clean hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagonv5_Debug_dynamic clean,cleaning .)

hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Debug_dynamic,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Debug_dynamic_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Debug_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Debug_dynamic,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Debug_dynamic_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Debug_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Debug_dynamic,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_Debug_dynamic_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Debug_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Release_dynamic: hexagon_Release_dynamic_MAKE_D_4_LIBDIR hexagon_Release_dynamic_MAKE_D_3_LIBDIR hexagon_Release_dynamic_MAKE_D_3_LIBDIR hexagon_Release_dynamic_MAKE_D_4_INCDIR hexagon_Release_dynamic_MAKE_D_4_LIBDIR hexagon_Release_dynamic_MAKE_D_4_INCDIR
	$(call job,,make V=hexagon_Release_dynamic,making .)

hexagon_Release_dynamic_clean: hexagon_Release_dynamic_MAKE_D_4_LIBDIR_clean hexagon_Release_dynamic_MAKE_D_3_LIBDIR_clean hexagon_Release_dynamic_MAKE_D_3_LIBDIR_clean hexagon_Release_dynamic_MAKE_D_4_INCDIR_clean hexagon_Release_dynamic_MAKE_D_4_LIBDIR_clean hexagon_Release_dynamic_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagon_Release_dynamic clean,cleaning .)

hexagon_Release_dynamic_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Release_dynamic,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Release_dynamic_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Release_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Release_dynamic_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Release_dynamic,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Release_dynamic_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Release_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Release_dynamic_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Release_dynamic,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Release_dynamic_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Release_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

android_Debug: android_Debug_MAKE_D_3_LIBDIR android_Debug_MAKE_D_3_LIBDIR android_Debug_MAKE_D_4_INCDIR android_Debug_MAKE_D_4_INCDIR
	$(call job,,make V=android_Debug,making .)

android_Debug_clean: android_Debug_MAKE_D_3_LIBDIR_clean android_Debug_MAKE_D_3_LIBDIR_clean android_Debug_MAKE_D_4_INCDIR_clean android_Debug_MAKE_D_4_INCDIR_clean
	$(call job,,make V=android_Debug clean,cleaning .)

android_Debug_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_Debug,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

android_Debug_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

android_Debug_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_Debug,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

android_Debug_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Release_dynamic: hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR hexagonv5_Release_dynamic_MAKE_D_4_INCDIR hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR hexagonv5_Release_dynamic_MAKE_D_4_INCDIR
	$(call job,,make V=hexagonv5_Release_dynamic,making .)

hexagonv5_Release_dynamic_clean: hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_Release_dynamic_MAKE_D_4_INCDIR_clean hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_Release_dynamic_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagonv5_Release_dynamic clean,cleaning .)

hexagonv5_Release_dynamic_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Release_dynamic,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Release_dynamic_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Release_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Release_dynamic,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Release_dynamic_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Release_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Release_dynamic,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_Release_dynamic_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Release_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_ReleaseG_dynamic: hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR
	$(call job,,make V=hexagonv5_ReleaseG_dynamic,making .)

hexagonv5_ReleaseG_dynamic_clean: hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagonv5_ReleaseG_dynamic clean,cleaning .)

hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_ReleaseG_dynamic,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_ReleaseG_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_ReleaseG_dynamic,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_ReleaseG_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_ReleaseG_dynamic,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_ReleaseG_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_ReleaseG: hexagonv5_ReleaseG_MAKE_D_4_LIBDIR hexagonv5_ReleaseG_MAKE_D_3_LIBDIR hexagonv5_ReleaseG_MAKE_D_3_LIBDIR hexagonv5_ReleaseG_MAKE_D_4_INCDIR hexagonv5_ReleaseG_MAKE_D_4_LIBDIR hexagonv5_ReleaseG_MAKE_D_4_INCDIR
	$(call job,,make V=hexagonv5_ReleaseG,making .)

hexagonv5_ReleaseG_clean: hexagonv5_ReleaseG_MAKE_D_4_LIBDIR_clean hexagonv5_ReleaseG_MAKE_D_3_LIBDIR_clean hexagonv5_ReleaseG_MAKE_D_3_LIBDIR_clean hexagonv5_ReleaseG_MAKE_D_4_INCDIR_clean hexagonv5_ReleaseG_MAKE_D_4_LIBDIR_clean hexagonv5_ReleaseG_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagonv5_ReleaseG clean,cleaning .)

hexagonv5_ReleaseG_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_ReleaseG,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_ReleaseG_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_ReleaseG_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_ReleaseG,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_ReleaseG_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_ReleaseG_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_ReleaseG,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_ReleaseG_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_ReleaseG_dynamic: hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR
	$(call job,,make V=hexagon_ReleaseG_dynamic,making .)

hexagon_ReleaseG_dynamic_clean: hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagon_ReleaseG_dynamic clean,cleaning .)

hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_ReleaseG_dynamic,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_ReleaseG_dynamic_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_ReleaseG_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_ReleaseG_dynamic,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_ReleaseG_dynamic_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_ReleaseG_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_ReleaseG_dynamic,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_ReleaseG_dynamic_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_ReleaseG_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Debug_dynamic: hexagon_Debug_dynamic_MAKE_D_4_LIBDIR hexagon_Debug_dynamic_MAKE_D_3_LIBDIR hexagon_Debug_dynamic_MAKE_D_3_LIBDIR hexagon_Debug_dynamic_MAKE_D_4_INCDIR hexagon_Debug_dynamic_MAKE_D_4_LIBDIR hexagon_Debug_dynamic_MAKE_D_4_INCDIR
	$(call job,,make V=hexagon_Debug_dynamic,making .)

hexagon_Debug_dynamic_clean: hexagon_Debug_dynamic_MAKE_D_4_LIBDIR_clean hexagon_Debug_dynamic_MAKE_D_3_LIBDIR_clean hexagon_Debug_dynamic_MAKE_D_3_LIBDIR_clean hexagon_Debug_dynamic_MAKE_D_4_INCDIR_clean hexagon_Debug_dynamic_MAKE_D_4_LIBDIR_clean hexagon_Debug_dynamic_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagon_Debug_dynamic clean,cleaning .)

hexagon_Debug_dynamic_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Debug_dynamic,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Debug_dynamic_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Debug_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Debug_dynamic_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Debug_dynamic,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Debug_dynamic_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Debug_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Debug_dynamic_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Debug_dynamic,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Debug_dynamic_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Debug_dynamic clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Debug: hexagon_Debug_MAKE_D_4_LIBDIR hexagon_Debug_MAKE_D_3_LIBDIR hexagon_Debug_MAKE_D_3_LIBDIR hexagon_Debug_MAKE_D_4_INCDIR hexagon_Debug_MAKE_D_4_LIBDIR hexagon_Debug_MAKE_D_4_INCDIR
	$(call job,,make V=hexagon_Debug,making .)

hexagon_Debug_clean: hexagon_Debug_MAKE_D_4_LIBDIR_clean hexagon_Debug_MAKE_D_3_LIBDIR_clean hexagon_Debug_MAKE_D_3_LIBDIR_clean hexagon_Debug_MAKE_D_4_INCDIR_clean hexagon_Debug_MAKE_D_4_LIBDIR_clean hexagon_Debug_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagon_Debug clean,cleaning .)

hexagon_Debug_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Debug,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Debug_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Debug_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Debug,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Debug_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Debug_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Debug,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Debug_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_ReleaseG: hexagon_ReleaseG_MAKE_D_4_LIBDIR hexagon_ReleaseG_MAKE_D_3_LIBDIR hexagon_ReleaseG_MAKE_D_3_LIBDIR hexagon_ReleaseG_MAKE_D_4_INCDIR hexagon_ReleaseG_MAKE_D_4_LIBDIR hexagon_ReleaseG_MAKE_D_4_INCDIR
	$(call job,,make V=hexagon_ReleaseG,making .)

hexagon_ReleaseG_clean: hexagon_ReleaseG_MAKE_D_4_LIBDIR_clean hexagon_ReleaseG_MAKE_D_3_LIBDIR_clean hexagon_ReleaseG_MAKE_D_3_LIBDIR_clean hexagon_ReleaseG_MAKE_D_4_INCDIR_clean hexagon_ReleaseG_MAKE_D_4_LIBDIR_clean hexagon_ReleaseG_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagon_ReleaseG clean,cleaning .)

hexagon_ReleaseG_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_ReleaseG,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_ReleaseG_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_ReleaseG_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_ReleaseG,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_ReleaseG_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_ReleaseG_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_ReleaseG,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_ReleaseG_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_Release: hexagonv5_Release_MAKE_D_4_LIBDIR hexagonv5_Release_MAKE_D_3_LIBDIR hexagonv5_Release_MAKE_D_3_LIBDIR hexagonv5_Release_MAKE_D_4_INCDIR hexagonv5_Release_MAKE_D_4_LIBDIR hexagonv5_Release_MAKE_D_4_INCDIR
	$(call job,,make V=hexagonv5_Release,making .)

hexagonv5_Release_clean: hexagonv5_Release_MAKE_D_4_LIBDIR_clean hexagonv5_Release_MAKE_D_3_LIBDIR_clean hexagonv5_Release_MAKE_D_3_LIBDIR_clean hexagonv5_Release_MAKE_D_4_INCDIR_clean hexagonv5_Release_MAKE_D_4_LIBDIR_clean hexagonv5_Release_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagonv5_Release clean,cleaning .)

hexagonv5_Release_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Release,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Release_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Release clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Release_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Release,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Release_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Release clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Release_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Release,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagonv5_Release_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Release clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Release: hexagon_Release_MAKE_D_4_LIBDIR hexagon_Release_MAKE_D_3_LIBDIR hexagon_Release_MAKE_D_3_LIBDIR hexagon_Release_MAKE_D_4_INCDIR hexagon_Release_MAKE_D_4_LIBDIR hexagon_Release_MAKE_D_4_INCDIR
	$(call job,,make V=hexagon_Release,making .)

hexagon_Release_clean: hexagon_Release_MAKE_D_4_LIBDIR_clean hexagon_Release_MAKE_D_3_LIBDIR_clean hexagon_Release_MAKE_D_3_LIBDIR_clean hexagon_Release_MAKE_D_4_INCDIR_clean hexagon_Release_MAKE_D_4_LIBDIR_clean hexagon_Release_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagon_Release clean,cleaning .)

hexagon_Release_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Release,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Release_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagon_Release clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagon_Release_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Release,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Release_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagon_Release clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagon_Release_MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Release,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

hexagon_Release_MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagon_Release clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

android_ReleaseG: android_ReleaseG_MAKE_D_3_LIBDIR android_ReleaseG_MAKE_D_3_LIBDIR android_ReleaseG_MAKE_D_4_INCDIR android_ReleaseG_MAKE_D_4_INCDIR
	$(call job,,make V=android_ReleaseG,making .)

android_ReleaseG_clean: android_ReleaseG_MAKE_D_3_LIBDIR_clean android_ReleaseG_MAKE_D_3_LIBDIR_clean android_ReleaseG_MAKE_D_4_INCDIR_clean android_ReleaseG_MAKE_D_4_INCDIR_clean
	$(call job,,make V=android_ReleaseG clean,cleaning .)

android_ReleaseG_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_ReleaseG,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

android_ReleaseG_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

android_ReleaseG_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_ReleaseG,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

android_ReleaseG_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_ReleaseG clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Debug: MAKE_D_4_LIBDIR hexagonv5_Debug_MAKE_D_3_LIBDIR hexagonv5_Debug_MAKE_D_3_LIBDIR hexagonv5_Debug_MAKE_D_4_INCDIR MAKE_D_4_LIBDIR hexagonv5_Debug_MAKE_D_4_INCDIR
	$(call job,,make V=hexagonv5_Debug,making .)

hexagonv5_Debug_clean: MAKE_D_4_LIBDIR_clean hexagonv5_Debug_MAKE_D_3_LIBDIR_clean hexagonv5_Debug_MAKE_D_3_LIBDIR_clean hexagonv5_Debug_MAKE_D_4_INCDIR_clean MAKE_D_4_LIBDIR_clean hexagonv5_Debug_MAKE_D_4_INCDIR_clean
	$(call job,,make V=hexagonv5_Debug clean,cleaning .)

hexagonv5_Debug_MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Debug,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Debug_MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=hexagonv5_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

hexagonv5_Debug_MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Debug,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

hexagonv5_Debug_MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=hexagonv5_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

MAKE_D_4_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Debug,making $(HEXAGON_SDK_ROOT)/test/common/test_util)

MAKE_D_4_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/test/common/test_util,make V=hexagonv5_Debug clean,cleaning $(HEXAGON_SDK_ROOT)/test/common/test_util)

android_Release: MAKE_D_3_LIBDIR MAKE_D_3_LIBDIR MAKE_D_4_INCDIR MAKE_D_4_INCDIR
	$(call job,,make V=android_Release,making .)

android_Release_clean: MAKE_D_3_LIBDIR_clean MAKE_D_3_LIBDIR_clean MAKE_D_4_INCDIR_clean MAKE_D_4_INCDIR_clean
	$(call job,,make V=android_Release clean,cleaning .)

MAKE_D_4_INCDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_Release,making $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

MAKE_D_4_INCDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/common/rpcmem,make V=android_Release clean,cleaning $(HEXAGON_SDK_ROOT)/lib/common/rpcmem)

MAKE_D_3_LIBDIR: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_Release,making $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

MAKE_D_3_LIBDIR_clean: 
	$(call job,$(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV,make V=android_Release clean,cleaning $(HEXAGON_SDK_ROOT)/lib/fastcv/dspCV)

W := $(findstring ECHO,$(shell echo))# W => Windows environment
@LOG = $(if $W,$(TEMP)\\)$@-build.log

C = $(if $1,cd $1 && )$2
job = $(_@)echo $3 && ( $C )> $(@LOG) && $(if $W,del,rm) $(@LOG) || ( echo ERROR $3 && $(if $W,type,cat) $(@LOG) && $(if $W,del,rm) $(@LOG) && exit 1)
ifdef VERBOSE
  job = $(_@)echo $3 && $C
endif
