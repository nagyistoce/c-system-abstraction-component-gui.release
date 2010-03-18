
__NO_AUTO_LIBS__=1
LINK_MINIMAL=1
PBASEDIR=.

LIBNAME=png
ifdef __WINDOWS__
# included for zlib support.
MORELIBS=bag
endif

ZBASEDIR=src/zlib-1.2.3
INCLUDEDIRS+=$(SACK_BASE)/$(ZBASEDIR)

include Makefile.bag.part

include $(SACK_BASE)/makefiles/makefile.single

include Makefile.bag.part.end


