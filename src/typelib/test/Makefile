
PROGNAME1=testmsgq
SRCS1=testmsgq
MORELIBS1=containers
ifdef STATIC
MORELIBS1+=shmem syslog
ifdef __WINDOWS__
SYSLIBS1=wsock32
endif
endif

include $(SACK_BASE)/makefiles/makefile.many