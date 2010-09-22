
__NO_GUI__=1

PROGNAME1=scanner
SRCS1=scanner

PROGNAME2=pinger
SRCS2=pinger
# this program breaks rules by referencing ntohl
ifneq '$(shell uname -o)' 'GNU/Linux'
SYSLIBS2=wsock32
endif

PROGNAME3=nameof
SRCS3=nameof

PROGNAME4=ipof
SRCS4=ipof

include $(SACK_BASE)/makefiles/makefile.many
