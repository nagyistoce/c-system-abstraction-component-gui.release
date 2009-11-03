LIBNAME=jpeg
DEFNAME=libjpeg.def

SYSDEPMEM=jmemnobs

# library object files common to compression and decompression
COMSRCS= jcomapi jutils jerror jmemmgr $(SYSDEPMEM)

# compression library object files
CLIBSRCS= jcapimin jcapistd jctrans jcparam jdatadst \
        jcinit jcmaster jcmarker jcmainct jcprepct \
        jccoefct jccolor jcsample jchuff jcphuff \
        jcdctmgr jfdctfst jfdctflt jfdctint

# decompression library object files
DLIBSRCS= jdapimin jdapistd jdtrans jdatasrc \
        jdmaster jdinput jdmarker jdhuff jdphuff \
        jdmainct jdcoefct jdpostct jddctmgr jidctfst \
        jidctflt jidctint jidctred jdsample jdcolor \
        jquant1 jquant2 jdmerge
# These objectfiles are included in libjpeg.lib
SRCS= $(CLIBSRCS) $(COMSRCS) $(DLIBSRCS)
CXFLAGS=-DJPEG_SOURCE -I$(SACK_BASE)/include/jpeg
# object files for sample applications (excluding library files)
COBJECTS= cjpeg.$(OBJEXT) rdppm.$(OBJEXT) rdgif.$(OBJEXT) rdtarga.$(OBJEXT) rdrle.$(OBJEXT) rdbmp.$(OBJEXT) \
        rdswitch.$(OBJEXT) cdjpeg.$(OBJEXT)

DOBJECTS= djpeg.$(OBJEXT) wrppm.$(OBJEXT) wrgif.$(OBJEXT) wrtarga.$(OBJEXT) wrrle.$(OBJEXT) wrbmp.$(OBJEXT) \
        rdcolmap.$(OBJEXT) cdjpeg.$(OBJEXT)

TROBJECTS= jpegtran.$(OBJEXT) rdswitch.$(OBJEXT) cdjpeg.$(OBJEXT) transupp.$(OBJEXT)

include $(SACK_BASE)/makefiles/makefile.single
