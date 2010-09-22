
ifdef __WINDOWS__
##################
# Standard video, with opengl support
#
__NO_AUTO_LIBS__=1
LINK_MINIMAL=1

# this doesn't include bag.psi by default for linking
__NO_GUI__=1  

LITERAL_LIBNAME1=bag.video.plugin
#-D_OPENGL_ENABLED 
CXFLAGS1=-DVIDEO_LIBRARY_SOURCE -DRENDER_LIBRARY_SOURCE -DBAG 

SRCS1=vidlib key opengl keydefs
#DEFNAME1=video.def
MORELIBS1=bag
MORE_LITERAL_LIBS1=bag.image.plugin
SYSLIBS1= gdi32 user32 opengl32 glu32
##################
# No opengl standardw indows
#
ifeq '0' '1' 
# this is disabled, already have opengl, forced, sorry
# source modification for visusal studio development...
# eases portability between projects....
LITERAL_LIBNAME2=bag.video.plugin.nogl
endif
CXFLAGS2=-DVIDEO_LIBRARY_SOURCE -DRENDER_LIBRARY_SOURCE -DBAG

SRCS2=vidlib viddll key keydefs
#DEFNAME2=video.def

MORELIBS2=$(MORELIBS1)
MORE_LITERAL_LIBS2=$(MORE_LITERAL_LIBS1)
SYSLIBS2= gdi32 user32


LIBNAME3=bag.video
SRCS3=$(SRCS1)
CXFLAGS3=$(CXFLAGS1)

MORELIBS3=bag
MORE_LITERAL_LIBS3=bag.image.plugin
SYSLIBS3=$(SYSLIBS1)



else # ifdef __LINUX__

CXFLAGS1=

SRCS1=xvidlib
LIBNAME1=xvideo

MORELIBS1=bag
SYSLIBS1= X11

endif

include $(SACK_BASE)/makefiles/makefile.many
