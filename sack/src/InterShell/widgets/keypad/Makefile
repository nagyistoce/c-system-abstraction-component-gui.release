
all .DEFAULT:
	$(MAKE) -C .. $(MAKECMDGOALS)

ifdef bananarama

MAKELEVEL=1

PROJECTS=images padtest

APP_DEFAULT_DATA=Crysta.ttf arialbd.ttf

LIBNAME=keypad
SRCS=buttons keypad accum
CXFLAGS=-DKEYPAD_SOURCE
INCLUDEDIRS=$(SACK_BASE)/src/pos/include
MORELIBS=fraction psi shmem syslog idle procreg timers

include $(SACK_BASE)/makefiles/makefile.single
endif
