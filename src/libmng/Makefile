include Makefile.bag

ifdef bananarama

#!/bin/bash

BAG=1
LINK_MINIMAL=1

USE_NASM_ASSEMBLER=1

ifdef __WINDOWS__
LITERAL_LIBNAME1=bag.mng.plugin
else
LITERAL_LIBNAME1=bag.real_mng.plugin
endif

CXFLAGS1=-DBAG
CXFLAGS1+=-DMNG_BUILD_DLL


ifdef __WINDOWS__
INCLUDEDIRS=$(SACK_BASE)/include/jpeg $(SACK_BASE)/include/zlib $(SACK_BASE)/include/png $(SACK_BASE)/include/imglib
endif
SRCS1=	libmng_callback_xs \
	libmng_chunk_io \
	libmng_chunk_descr \
	libmng_chunk_prc \
	libmng_chunk_xs \
	libmng_cms \
	libmng_display \
	libmng_dither \
	libmng_error \
	libmng_filter \
	libmng_hlapi \
	libmng_jpeg \
	libmng_object_prc \
	libmng_pixels \
	libmng_prop_xs \
	libmng_read \
	libmng_trace \
	libmng_write \
	libmng_zlib
      




# Add support for freetype library
# it's kinda funkiliscious because windows doesn't natively
# have anything like freetype, therefore it's easy to find, 
# linux, who knows where anything goes... so some libs have these *-config files
ifndef __WINDOWS__
 ifneq '$(COMPILER)' 'arm'
CXFLAGS1+= $(shell freetype-config --cflags)
# filter out to only -wL,path options
LINK_LIBDIRS1=$(shell freetype-config --libs|sed 's/-l[^ ]*//g;s/-Wl[^ ]*//g;s/-L//' )
# filter out to only the library name
SYSLIBS1+=$(shell freetype-config --libs|sed 's/-L[^ ]*//g;s/-l//g;s/-Wl[^ ]*//g' ) 
SYSLIBS1+=$(if $(__ARM__),,jpeg png z)
 else
INCLUDEDIRS+=$(SACK_BASE)/include/png $(SACK_BASE)/include/zlib $(SACK_BASE)/include/jpeg
 endif
else
endif


#LIBNAME2=bag.image
#ifdef __LINUX__
#SRCS2:=$(SRCS1)
#else
#SRCS2:=$(SRCS1)
#endif
#CXFLAGS2=$(CXFLAGS1)
#CFLAGS2=$(CFLAGS1)
#LINK_LIBDIRS2=$(LINK_LIBDIRS1)
#SYSLIBS2=$(SYSLIBS1)
#MORELIBS2=$(MORELIBS1)

include $(SACK_BASE)/makefiles/makefile.many


ifndef __WINDOWS__
else
CFLAGS2=$(CFLAGS1)
endif
endif
