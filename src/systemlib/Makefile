
all .DEFAULT:
	make -C .. $(MAKECMDGOALS)
        
ifdef bananarama        

ifndef SRC_DIR_LEVEL_MAKE
PROJECTS=spawner
endif

LIBNAME=osalot
SRCS=system args
CXFLAGS=-DSYSTEM_SOURCE
MORELIBS=timers containers shmem syslog idle filesys

include $(SACK_BASE)/makefiles/makefile.single
endif
