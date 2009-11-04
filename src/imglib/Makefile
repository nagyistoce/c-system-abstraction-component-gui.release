#!/bin/bash

NO_MNG=1
BAG=1
LINK_MINIMAL=1

# the bag.image.plugin is replaced by
# bag.display.image.plugin (msgsvr based thing)
LITERAL_LIBNAME1=$(if $(FORCE_NO_INTERFACE),,$(if $(shell $(uname)),bag.real_image.plugin,bag.image.plugin))

CXFLAGS1=-DBAG

INCLUDEDIRS=$(if $(shell $(uname)),,$(SACK_BASE)/include/jpeg $(SACK_BASE)/include/png $(SACK_BASE)/include/imglib)

C_SRCS=gifimage bmpimage font lucidaconsole \
      sprite sprite_common image line \
      jpgimage  pngimage \
      blotdirect \
      blotscaled alphatab alphastab interface \
      fntcache fntrender
SRCS1=$(C_SRCS) $(ASM_SRCS)
      
ifndef NO_MNG
SRCS1+= mngsprite
endif

ifndef __LINUX64__
ifndef __ARM__
ASM_SRCS=$(if $(HAVE_NASM),lineasm  \
      plotasm  \
      blatcolor blotdirasm blotdirshadeasm \
      blotdirmshadeasm \
      blotscaasm blotscashadeasm blotscamshadeasm,)
endif
endif

#include rendering of fonts in the image library itself
# this avoids having to transport the font across the message pipes
# it also allows this library to render character by character.
#SRCS1+= fntcache fntrender


# Add support for freetype library
# it's kinda funkiliscious because windows doesn't natively
# have anything like freetype, therefore it's easy to find, 
# linux, who knows where anything goes... so some libs have these *-config files
ifneq '$(shell uname)' ''
 ifneq '$(COMPILER)' 'arm'
CXFLAGS1+= $(shell freetype-config --cflags)
# filter out to only -wL,path options
LINK_LIBDIRS1=$(shell freetype-config --libs|sed 's/-l[^ ]*//g;s/-Wl[^ ]*//g;s/-L//' )
# filter out to only the library name
SYSLIBS1+=$(shell freetype-config --libs|sed 's/-L[^ ]*//g;s/-l//g;s/-Wl[^ ]*//g' ) 
SYSLIBS1+=$(if $(__ARM__),,jpeg png z mng)
 else
INCLUDEDIRS+=$(SACK_BASE)/src/freetype-2.3.7/include
 endif
else
 ifndef NO_MNG
  CXFLAGS1+=-DMNG_USE_DLL
  MORELIBS1+=mng
 endif
 MORELIBS1+=bag
INCLUDEDIRS+=$(SACK_BASE)/src/freetype-2.3.7/include

 MORELIBS1+=$(if $(STATIC),png,)
endif


LIBNAME2=bag.image
SRCS2=$(C_SRCS) $(ASM_SRCS)
CXFLAGS2=$(CXFLAGS1)
CFLAGS2=$(CFLAGS1)
LINK_LIBDIRS2=$(LINK_LIBDIRS1)
SYSLIBS2=$(SYSLIBS1)
MORELIBS2=$(MORELIBS1)

include $(SACK_BASE)/makefiles/makefile.many


ifneq '$(shell $(uname))' ''
else
CFLAGS2=$(CFLAGS1)
endif
