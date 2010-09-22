
LIBNAME1=sha1
SRCS1=sha1
CXFLAGS1=-DSHA1_SOURCE
ifdef BCC16
DEFNAME1=sha1.def
endif
SACK_LIBS1= syslog

PROGNAME2=sha1test
SRCS2=sha1test
SACK_LIBS2=sha1 syslog
ifdef __WINDOWS__
SYSLIBS2+=wsock32
endif

include $(SACK_BASE)/makefiles/makefile.many
