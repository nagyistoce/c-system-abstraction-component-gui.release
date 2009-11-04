
#__NO_AUTO_LIBS__=1
LINK_MINIMAL=1
__NO_GUI__=1
INCLUDEDIRS=w32/include glob
PROGNAME=pmake

SRCS=variable rule remote-stub commands file \
	getloadavg default signame expand dir \
        main getopt1 job read version getopt \
        arscan hash remake misc2 ar function vpath implicit \
        glob/glob glob/fnmatch w32/pathstuff  \
        w32/subproc/misc w32/subproc/sub_proc w32/subproc/w32err \
        print debug trace strcache debugger/cmd
        

CXFLAGS=-D__C89 -DHAVE_CONFIG_H -DWINDOWS32 -DTIVOLI $(if $(CPLUSPLUS_BUILD),-D__STDC__=1,)
#MORELIBS=syslog

ifdef __WINDOWS__
SYSLIBS=wsock32
endif

include $(SACK_BASE)/makefiles/makefile.single

