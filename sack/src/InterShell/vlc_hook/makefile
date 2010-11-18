
MORE_PROJECTS=intershell/Makefile

ifndef VLC_PATH
VLC_PATH=c:/tools/vlc-1.1.4/sdk/include
export VLC_PATH
endif

INCLUDEDIRS+=$(VLC_PATH)
LIBNAME=vlcint
SRCS=vlc_interface

PROGNAME2=vlc_test
SRCS2=vlc_test
MORELIBS2=vlcint

PROGNAME4=vlc_window
SRCS4=vlc_win
MORELIBS4=vlcint

PROGNAME3=vlc_sound_test
SRCS3=vlc_sound_test
#SUBSYSTEM3=windows
MORELIBS3=vlcint

include $(SACK_BASE)/makefiles/makefile.single
