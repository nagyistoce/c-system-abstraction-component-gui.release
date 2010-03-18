
all .DEFAULT:
	make -C .. $(MAKECMDGOALS)
        
ifdef bananarama

# if we're on a non-intel platform - please figure out
# how to specify that option, and remove the HAS_ASSEMBLY flag.
# otherwise NASM is used to assemble potions.
ifndef __ARM__
USE_NASM_ASSEMBLER=1
endif

SRCS=sharemem
CXFLAGS=-DMEM_LIBRARY_SOURCE

# hmm need to find a way to swap this out based on the config which
# is loaded much after here...
ifndef __ARM__
ifndef __LINUX64__
CXFLAGS+= -DHAS_ASSEMBLY
SRCS+=memoryasm
endif
endif

LIBNAME=shmem
SACK_LIBS=syslog

include $(SACK_BASE)/makefiles/makefile.single

#include $(SACK_BASE)/makefiles/makefile.master $(LIBLIST_1)

endif
