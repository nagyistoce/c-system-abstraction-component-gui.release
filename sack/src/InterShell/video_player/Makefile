
ifndef DEKWARE_BASE
all .DEFAULT:
	@echo DEKWARE_BASE was not defined, skipping video player build.
else
INCLUDEDIRS=$(DEKWARE_BASE)/include
LITERAL_LIBNAME=vplayer.isp
CXFLAGS=-DSACK_CORE_BUILD
SRCS=vplayer
MORELIBS=psi.console sack.widgets
MORE_LITERAL_LIBS=dekware.core

include $(SACK_BASE)/makefiles/makefile.single
endif
