
PROGNAME=menutest

SRCS=menutest

ifeq '$(filter-out static_debug static_release,$(DEST))' ''

ifndef __WINDOWS__
else
SYSLIBS+=wsock32
endif
endif

include $(SACK_BASE)/makefiles/makefile.single

