
#__NO_AUTO_LIBS__=1
LINK_MINIMAL=1
__NO_GUI__=1

PROGNAME=pmake

SRCS=variable rule remote-stub commands file \
	getloadavg default signame expand dir \
        main getopt1 job read version getopt \
        arscan remake misc ar function vpath implicit \
        hash glob fnmatch sub_proc pathstuff w32err w32misc 
        

CXFLAGS=-D__C89 -DHAVE_CONFIG_H -DWINDOWS32 -DTIVOLI
#MORELIBS=syslog

ifdef __WINDOWS__
SYSLIBS=wsock32
endif

include $(SACK_BASE)/makefiles/makefile.single

