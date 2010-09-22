FINALDEST=$(CURDIR)
SACK_BASE=$(CURDIR)/..
LINK_MINIMAL=1
__NO_AUTO_LIBS__=1
BAG=1
MKCFLAGS=-D__NO_BAG__

PROGNAME1=typesize
SRCS1=simple

PROGNAME2=deadalign
SRCS2=../src/deadstart/deadstart_end dead1 dead2 dead3 deadmain ../src/deadstart/deadstop ../src/deadstart/deadstart ../src/deadstart/deadstart_list
CXFLAGS2=-DBAG

LIBNAME3=deadalign_dll
SRCS3=../src/deadstart/deadstart_end dead2 dead3 deadmain ../src/deadstart/deadstop ../src/deadstart/deadstart ../src/deadstart/deadstart_list
CXFLAGS3=-DBAG

PROGNAME4=deadalign_prog
SRCS4=../src/deadstart/deadstart_end dead1 deadmain ../src/deadstart/deadstart ../src/deadstart/deadstart_list 
MORELIBS4=deadalign_dll
CXFLAGS4=-DBAG
             
include ../makefiles/makefile.many
