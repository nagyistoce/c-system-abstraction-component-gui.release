
PROGNAME=fonttst
SRCS= fnttest
CXFLAGS = 

ifeq '$(filter-out static_debug static_release,$(DEST))' ''
ifndef __WINDOWS__
SYSLIBS+=$(if $(__ARM__),,$(shell freetype-config --libs|sed 's/-L[^ ]*//g;s/-l//g;s/-Wl[^ ]*//g')) pthread
else
SYSLIBS+=wsock32
endif
endif

include $(SACK_BASE)/makefiles/makefile.single
