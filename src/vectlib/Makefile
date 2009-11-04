
all .DEFAULT:
	make -C .. $(MAKECMDGOALS)
ifeq '0' '1'

CXFLAGS=-DVECTOR_LIBRARY_SOURCE

LIBNAME=vector

SRCS = vectlib

MORELIBS=shmem syslog

ifndef __WINDOWS__
SYSLIBS=m
endif

-include $(SACK_BASE)/makefiles/makefile.single
endif
