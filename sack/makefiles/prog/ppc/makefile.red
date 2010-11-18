

MODE=DEBUG

CDEBUG_FLAGS=-g -D_DEBUG 
CPROFILE_FLAGS=-pg -g -D_DEBUG 
CRELEASE_FLAGS=-O9
CFLAGS= $(C$(MODE)_FLAGS) -c -D__GCC__ -D__LINUX__ -D__UNIXTIME__

LDDEBUG_FLAGS=-g
LDPROFILE_FLAGS=-pg -g
LDRELEASE_FLAGS=

.PHONY: all

all: ppc config.ppc

#MEMLIB=sharemem.o
MEMLIB=mem.o


OBJS = define.o links.o text.o input.o cppmain.o \
	fileio.o expr.o enum.o struct.o array.o $(MEMLIB)

define getconfig
       touch zzyzzxx.c; 
	echo '/> search starts here:$$/ {list=1}' >zzyzzxx.awk
	echo '/\/.*$$/ {if(list==1) list=2}' >>zzyzzxx.awk
	echo '/End of search list/ {list = 0}' >>zzyzzxx.awk
        echo '/define / { print $$0 }' >>zzyzzxx.awk
	echo '{if ( list > 1 )  print "#pragma systemincludepath" $$0 }' >>zzyzzxx.awk
        gcc -dM -E -v zzyzzxx.c 2>&1 | awk -f zzyzzxx.awk > config.ppc; 
	#rm zzyzzxx.*
endef

ppc: $(OBJS) makefile.lnx config.ppc
	gcc $(LD$(MODE)_FLAGS)  -oppc $(OBJS)

config.ppc: makefile.lnx;
	$(getconfig)
        
%.o:%.c
	gcc $(CFLAGS) -o $@ $<

links.o: links.c makefile.lnx $(MEMLIB)

fileio.o: fileio.c makefile.lnx $(MEMLIB)

expr.o: expr.c makefile.lnx $(MEMLIB)

text.o: text.c makefile.lnx $(MEMLIB)

input.o: input.c makefile.lnx $(MEMLIB)

mem.o: mem.c makefile.lnx

sharemem.o: sharemem.c makefile.lnx
	gcc -DMEM_LIBRARY_SOURCE $(CFLAGS) -o $@ $<

cppmain.o: cppmain.c makefile.lnx $(MEMLIB)

define.o: define.c makefile.lnx $(MEMLIB)

enum.o: enum.c makefile.lnx $(MEMLIB)

array.o: array.c makefile.lnx $(MEMLIB)

struct.o: struct.c makefile.lnx $(MEMLIB)

links.c fileio.c expr.c text.c input.c mem.c cppmain.c \
    define.c enum.c array.c struct.c: ;

makefile.lnx: ;
