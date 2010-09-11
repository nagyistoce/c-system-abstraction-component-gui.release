
all .DEFAULT:
	make -C ..

ifdef bananarama
ifdef __WINDOWS__
ifeq '$(FINALDEST)' ''
MAKE_PROJECTS=1
endif
ifeq '$(FINALDEST)' '$(SACK_BASE)'
MAKE_PROJECTS=1
endif
ifeq '$(MAKEPROJECTS)' '1'
PROJECTS=rm mkdir cp
endif
endif
PROJECTS+=filemon

CXFLAGS=-DFILESYSTEM_LIBRARY_SOURCE

LIBNAME=filesys

SRCS=filescan pathops
MORELIBS=syslog

include $(SACK_BASE)/makefiles/makefile.single
endif
