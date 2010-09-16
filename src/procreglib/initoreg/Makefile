MAKELEVEL=1

PROGNAME=ini2reg
SRCS=ini2reg
MORELIBS=procreg shmem filesys syslog

ifndef __WINDOWS__
else
MORELIBS+=configscript osalot syslog containers timers filesys network idle
SYSLIBS+=wsock32
endif

include $(SACK_BASE)/makefiles/makefile.single
