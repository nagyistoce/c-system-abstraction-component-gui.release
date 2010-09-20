#
# FBI data replicator for Jumbo Bingo.
#
#

MKCFLAGS = -D_GNU_SOURCE -DSYSLOG_NAMES -DBINGOLINK_SOURCE#-DNDEBUG

APP_DEFAULT_DATA1=$(foreach img,$(shell ls images/logos),images/logos/$(img)) \
	$(foreach img,$(shell ls sounds),sounds/$(img))\
        $(foreach img,$(shell ls frames),frames/$(img)) 
PROJECTS=ianson

INCLUDEDIRS=$(ALTANIK_BASE)/src/apps/milk
INCLUDEDIRS+=$(ALTANIK_BASE)/include

#### GUI CLIENT #####
ifndef __NO_GUI__
LITERAL_LIBNAME2=alpha2.isp
SRCS2=stateserver db bdata_tcp_relay
CXFLAGS2=-DINTERSHELL_PLUGIN
MORELIBS2=
ifdef __WINDOWS__
MORE_LIBS2+=altanik-core
SYSLIBS2+=odbc32
else
MORE_LIBS2+=altanik-core
SYSLIBS2=odbc
endif
endif

#### The state server. ####
PROGNAME3=alpha2server

SRCS3=stateserver db bdata_tcp_relay
MORELIBS3=
MORE_LIBS3=altanik-core
ifndef __WINDOWS__
SYSLIBS3=odbc
else
endif

#announcement sounds
PROGNAME4=announcement
SRCS4=annce
ifndef __WINDOWS__
SYSLIBS4=odbc# fhal
else
SYSLIBS4+=odbc32
endif

ifndef __NO_GUI__
LITERAL_LIBNAME5=link_state.isp
SRCS5=statebuttons db dvdactive  mediaactive 
CXFLAGS5=-DINTERSHELL_PLUGIN
MORELIBS5= altanik.widgets
MORE_LIBS5=altanik-core
endif

include $(ALTANIK_BASE)/makefiles/makefile.many
