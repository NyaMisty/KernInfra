ARCHS = arm64 arm64e
#TARGET := iphone:clang:14.3:7.0

include $(THEOS)/makefiles/common.mk

USE_LIBKRW = 1

SUBPROJECT_NAME = kerninfra
LIBRARY_NAME = kerninfra
LIB_DIR := lib

kerninfra_FILES = patchfinder/libdimentio.c rw_wrap/rw_wrap.cpp kern_func.cpp kerninfra.mm
kerninfra_CFLAGS = -fobjc-arc -Iincludes
kerninfra_CCFLAGS = -std=c++2a -fvisibility=hidden
kerninfra_FRAMEWORKS = IOKit
#kerninfra_INSTALL_PATH = /usr/local/lib
#kerninfra_LINKAGE_TYPE = static
#kerninfra_LDFLAGS = -lcompression
kerninfra_LIBRARIES = compression

ifdef USE_TFP0
	kerninfra_FILES += rw_prov/rw_prov_tfp0/rw_prov_tfp0.c
	kerninfra_CFLAGS += -DUSE_TFP0
endif
ifdef USE_LIBKRW
	#kerninfra_FRAMEWORKS += kerninfra/libkrw
	kerninfra_LIBRARIES += krw
	kerninfra_LDFLAGS += -Lrw_prov/rw_prov_libkrw/libkrw
	kerninfra_FILES += rw_prov/rw_prov_libkrw/rw_prov_libkrw.c
	kerninfra_CFLAGS += -DUSE_LIBKRW -I
endif

include $(THEOS_MAKE_PATH)/subproject.mk
#include $(THEOS_MAKE_PATH)/library.mk
