ifndef SACK_BASE
all .DEFAULT:
	$(MAKE) -C .. $(MAKECMDGOALS)
else

PROGNAME=network_failure_monitor
CXFLAGS=-DPROGRAM
SRCS=network_failure_monitor myping
SUBSYSTEM=windows
MORELIBS=sack.widgets


LIBNAME2=network_failure_monitorx
SRCS2:=$(SRCS)
MORELIBS2:=$(MORELIBS)

include $(SACK_BASE)/makefiles/makefile.single

endif
