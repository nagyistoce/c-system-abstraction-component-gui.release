
LINK_MINIMAL=1

PROGNAME1=runner
SRCS1=runner

PROGNAME2=packager
SRCS2=package


ifdef __WINDOWS__

# mode 0 - use WinMain, parse GetCommandLine() string
# into (argc, argv) attepting to use ' and " to contain
# parameters.  Quotes are passed as part of argument string.
CXFLAGS3=-DMODE=0
SRCS3=runwin
SUBSYSTEM3=windows
PROGNAME3=runwin

# mode 0 - use main() concat args where required
# for CommandLine() passing (Start I think takes that)
CXFLAGS4=-DMODE=1
SRCS4=runwin
SUBSYSTEM4=console
PROGNAME4=runcon

else
SRCS3=runlnx
PROGNAME3=runlnx
SYSLIBS3=dl pthread
endif

ifeq '0' '1'
ifndef FINALDEST
FINALDEST=..
endif

ifndef __WINDOWS__
SRCS=runlnx
PROGNAME=rundeck
SYSLIBS=dl pthread
include $(SACK_BASE)/makefiles/makefile.single

else

CXFLAGS1=-DMODE=0
SRCS1=runwin
SUBSYSTEM1=windows
PROGNAME1=runwin

CXFLAGS2=-DMODE=1
SRCS2=runwin
SUBSYSTEM2=console
PROGNAME2=runcon


endif

endif

include $(SACK_BASE)/makefiles/makefile.many
