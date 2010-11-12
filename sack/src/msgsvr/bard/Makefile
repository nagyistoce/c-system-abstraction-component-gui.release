
PROJECTS=bardtest


LIBNAME2=bardclient
SRCS2=bardclient
CXFLAGS2=-DBARD_CLIENT_FLAG -DBARD_SOURCE
MORELIBS=

PROGNAME3=bardservice
SRCS3=bardservice
CXFLAGS3=-DBARD_SERVICE_FLAG 
SUBSYSTEM3=windows
ifdef __WINDOWS__
#MORELIBS3=bag.psi
endif
MORELIBS3+=bardclient
ifndef __WINDOWS__
#SYSLIBS1=
endif


include $(SACK_BASE)/makefiles/makefile.many

