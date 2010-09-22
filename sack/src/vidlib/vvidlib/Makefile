
INCLUDEDIRS=$(DEKWARE_BASE)/include

LITERAL_LIBNAME=bag.video.http.plugin
SRCS=vvidlib imagepng
# link directly to the image library.
# do not interface here?... 
# what if we were using a remote image service also?
#MORE_LITERAL_LIBS=bag.real_image.plugin
ifdef __LINUX__
SYSLIBS=png
endif
MORE_LITERAL_LIBS=dekware.core filter.http.nex 


include $(SACK_BASE)/makefiles/makefile.single
