
#__NO_GUI__=1
LINK_MINIMAL=1

MORE_PROJECTS=$(if $(ALTANIK_BASE),Makefile2,)

APP_DEFAULT_DATA=PAD.ICO $(foreach frame,$(shell ls frames),frames/$(frame))
SUBSYSTEM=windows
PROGNAME=launchpad
SRCS=launchpad
RSRCS=launchpad
MORELIBS=


include $(SACK_BASE)/makefiles/makefile.single
