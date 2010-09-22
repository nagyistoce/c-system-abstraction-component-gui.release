MKCFLAGS = -D_GNU_SOURCE -DSYSLOG_NAMES -DBINGOLINK_SOURCE#-DNDEBUG

INCLUDEDIRS=$(ALTANIK_BASE)/src/apps/milk
INCLUDEDIRS+=$(ALTANIK_BASE)/include

ifndef __NO_GUI__
LITERAL_LIBNAME5=controller_media.isom
SRCS5=importmedia selectmedia
MORELIBS5=altanik.widgets
MORE_LITERAL_LIBS5=altanik.core milk.core
endif

include $(ALTANIK_BASE)/makefiles/makefile.many
