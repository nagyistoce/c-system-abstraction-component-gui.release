
all .DEFAULT:
	make -C ../.. $(MAKECMDGOALS)

ifdef bananarama
LIBNAME=filemon
SRCS=allfiles
CXFLAGS=-DFILEMONITOR_SOURCE
ifndef __WINDOWS__
SRCS+=linuxfiles
else
SRCS+=windowsfiles
endif
MORELIBS=timers containers filesys shmem syslog

include $(SACK_BASE)/makefiles/makefile.single
endif
