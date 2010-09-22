
LINK_MINIMAL=1

ifdef __WINDOWS__
PROGNAME1=runres
SRCS1=runres
SUBSYSTEM1=windows
SYSLIBS+=wsock32

PROGNAME2=fixedres
SRCS2=fixedres
SUBSYSTEM2=windows
SYSLIBS2+=wsock32

include $(SACK_BASE)/makefiles/makefile.many
else
all .DEFAULT: ;
endif
