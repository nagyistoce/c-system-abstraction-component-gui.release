ifndef SACK_BASE
all .DEFAULT:
	$(MAKE) -C .. $(MAKECMDGOALS)
else

ifndef __LINUX__
##################
# Standard video, with opengl support
#
__NO_AUTO_LIBS__=1
LINK_MINIMAL=1

# this doesn't include bag.psi by default for linking
__NO_GUI__=1  
#ifeq '1' '0'
LITERAL_LIBNAME1=$(if $(FORCE_NO_INTERFACE),,$(if $(STATIC),,bag.video.plugin))
#-D_OPENGL_ENABLED 

#DEFNAME1=video.def
MORELIBS1=bag
MORE_LITERAL_LIBS1=bag.image.plugin
SYSLIBS1= gdi32 user32 opengl32 glu32
#endif
SRCS1=vidlib viddll key opengl keydefs
CXFLAGS1=-D_OPENGL_ENABLED -DVIDEO_LIBRARY_SOURCE -DRENDER_LIBRARY_SOURCE -DBAG $(if $(__NO_OPTIONS__),-D__NO_OPTIONS__,) 
##################
# No opengl standardw indows
#
#ifeq '0' '1' 
# this is disabled, already have opengl, forced, sorry
# source modification for visusal studio development...
# eases portability between projects....
LITERAL_LIBNAME2=$(if $(FORCE_NO_INTERFACE),,bag.video.plugin.nogl)
#endif
CXFLAGS2=-DVIDEO_LIBRARY_SOURCE -DRENDER_LIBRARY_SOURCE -DBAG $(if $(__NO_OPTIONS__),-D__NO_OPTIONS__,)

SRCS2=vidlib viddll key keydefs
#DEFNAME2=video.def

MORELIBS2=$(MORELIBS1)
MORE_LITERAL_LIBS2=$(MORE_LITERAL_LIBS1)
SYSLIBS2= gdi32 user32


LIBNAME3=bag.video
SRCS3=$(SRCS1)
CXFLAGS3=$(CXFLAGS1)

MORE_LIBS3=bag bag.image
SYSLIBS3=$(SYSLIBS1)


endif

ifdef __LINUX__

CXFLAGS1=

SRCS1=xvidlib
LIBNAME1=xvideo

MORELIBS1=bag
SYSLIBS1= X11

endif

include $(SACK_BASE)/makefiles/makefile.many
endif
