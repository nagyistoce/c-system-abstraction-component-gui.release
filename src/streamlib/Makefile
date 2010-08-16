APP_DEFAULT_DATA=stream.conf

ifndef __NO_GUI__

PROGNAME=streamtst
SRCS=capture network  $(if $(__WINDOWS__),v4w) $(if $(__LINUX__),v4l,)

#v4w
#compress decompress
LINK_GUI=1
ifdef __WINDOWS__
#MORELIBS+=xvidcore
#SRCS+=v4w
CXFLAGS+=-D_OBJC_NO_COM
SYSLIBS+=vfw32 
#MORE_LITERAL_LIBS=bag.image.plugin
else
#SRCS+=v4l
SYSLIBS+=m
endif

PROGNAME4=vstreamtst
SRCS4=vlc_cap_test
MORE_LITERAL_LIBS4=vstreamtst.isom

INCLUDEDIRS+=$(SACK_BASE)/src/xvid/src

#SYSLIBS+=xvidcore
############### MILK PLUGIN #####################
ifdef ALTANIK_BASE
INCLUDEDIRS+=$(ALTANIK_BASE)/include


ifndef VLC_PATH
VLC_PATH=e:/tools/vlc-0.9.9z/sdk/include
$(warning VLC_PATH ?  default (from this makefile) $(VLC_PATH))
endif

LITERAL_LIBNAME2=$(if $(shell $(DIR_CMD) $(subst /,$(SYSPATHCHAR),$(VLC_PATH))),vstreamtst.isom,)
SRCS2=milk_plugin vlc_capture
MORELIBS2=vlcint


LITERAL_LIBNAME7=$(if $(shell $(DIR_CMD)  $(subst /,$(SYSPATHCHAR),$(VLC_PATH))),streamtst.isp,)
SRCS7=ishell_plugin 
MORELIBS7=vlcint


LITERAL_LIBNAME5=vstreamtst2.isom
SRCS5=milk_plugin vlc_capture2


#v4w
#compress decompress
ifdef __WINDOWS__
#MORELIBS+=xvidcore
#SRCS2+=v4w
CXFLAGS2+=-D_OBJC_NO_COM
SYSLIBS2+=vfw32 
else
#SRCS+=v4l
SYSLIBS2+=m
endif

LITERAL_LIBNAME3=streamtst.isom
SRCS3=milk_plugin capture $(if $(__WINDOWS__),v4w) $(if $(__LINUX__),v4l,)

#v4w
#compress decompress
ifdef __WINDOWS__
#MORELIBS+=xvidcore
#SRCS3+=v4w
CXFLAGS3+=-D_OBJC_NO_COM
SYSLIBS3+=wsock32 vfw32 
else
#SRCS3+=v4l
SYSLIBS3+=m
endif


endif
############### END MILK PLUGIN #####################

endif

PROGNAME6=testmove
SRCS6=testmove



include $(SACK_BASE)/makefiles/makefile.single
