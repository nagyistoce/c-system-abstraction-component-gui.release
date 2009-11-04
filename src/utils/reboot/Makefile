
LINK_MINIMAL=1

ifdef __WINDOWS__
PROGNAME1=rebootnow
SRCS1=reboot
CXFLAGS1=-DDEDICATE_REBOOT

ifndef __NO_GUI__
# requires systray function which is buried in psi.lib now :(
PROGNAME2=rebootmon
SRCS2=reboot
RSRCS2=reboot
endif

include $(SACK_BASE)/makefiles/makefile.many
else
all .DEFAULT:;
	   
endif

