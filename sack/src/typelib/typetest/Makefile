
PROGNAME1=testmsgq
SRCS1=testmsgq
MORELIBS1=containers syslog timers

ifeq '$(filter-out static_debug static_release,$(DEST))' ''
MORELIBS1+=idle shmem
ifdef __WINDOWS__
SYSLIBS1=wsock32
endif
endif

include $(SACK_BASE)/makefiles/makefile.many