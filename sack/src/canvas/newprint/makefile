
INCLUDEDIRS=$(ALTANIK_BASE)/include

PROGNAME1=print
#SUBSYSTEM1=windows
#PROGNAME2=nprint.exe
#PROGNAME3=wprint.exe
#PROGNAME4=fprint.exe

SRCS1=print template templatedraw
#SRCS2:=$(SRCS1)
#SRCS3:=$(SRCS1)
#SRCS4:=$(SRCS1)

MORELIBS1=psi image jpeg png zlib network shmem syslog fraction containers
#SYSLIBS1=wsock32 winspool
#MORELIBS2:=$(MORELIBS1)
#SYSLIBS2:=$(SYSLIBS1)
#MORELIBS3:=$(MORELIBS1)
#SYSLIBS3:=$(SYSLIBS1)
#MORELIBS4:=$(MORELIBS1)
#SYSLIBS4:=$(SYSLIBS1)


CXFLAGS1=-DPRINT_WINDOW
#CXFLAGS3=-DPRINT_WINDOW
#CXFLAGS2=-DPRINT_NETWORK
#CXFLAGS4=-DPRINT_FILE

include $(SACK_BASE)/makefiles/makefile.many

#fCC = lcc
#CFLAGS = -g -D_DEBUG -D__STATIC__ -DNEWCODE -IM:/common/include -IP:/source 
#LIBPATH=-LM:/common/lib/debug
#LIBS=images.lib jpegs.lib pngs.lib zlibs.lib networks.lib shmems.lib \
#	syslogs.lib wsock32.lib typelibs.lib winspool.lib 

#CFLAGS = -g -D_DEBUG -DNEWCODE -Is:\\source -IM:\\common\\include
#LIBPATH=-LM:/common/lib/debug
#LIBS=image.lib network.lib shmem.lib wsock32.lib syslog.lib typelib.lib winspool.lib 

#all: print.exe nprint.exe wprint.exe fprint.exe


#clean: 
#	rm -f print.exe
#	rm -f print.obj template.obj templatedraw.obj fractions.obj
#
#print.exe: print.obj template.obj templatedraw.obj fractions.obj
#	lcclnk -s -map print.map /subsystem:windows print.obj fractions.obj template.obj templatedraw.obj  $(LIBPATH) $(LIBS) msvcrt.lib
#
#nprint.exe: nprint.obj template.obj templatedraw.obj fractions.obj
#	lcclnk -s -map print.map /subsystem:windows nprint.obj fractions.obj template.obj templatedraw.obj  $(LIBPATH) $(LIBS) msvcrt.lib timerss.lib
#
#wprint.exe: wprint.obj template.obj templatedraw.obj fractions.obj
#	lcclnk -s -map print.map /subsystem:windows wprint.obj fractions.obj template.obj templatedraw.obj  $(LIBPATH) $(LIBS) msvcrt.lib
#
#fprint.exe: fprint.obj template.obj templatedraw.obj fractions.obj
#	lcclnk -s -map print.map /subsystem:windows fprint.obj fractions.obj template.obj templatedraw.obj  $(LIBPATH) $(LIBS) msvcrt.lib
#
#print.obj: print.c
#	$(CC) -DPRINT_DIRECT $(CFLAGS) -Fo$@ $< 
#
#wprint.obj: print.c
#	$(CC) -DPRINT_WINDOW $(CFLAGS) -Fo$@ $< 
#3
#nprint.obj: print.c
#3	$(CC) -DPRINT_NETWORK $(CFLAGS) -Fo$@ $< 
#
#fprint.obj: print.c
#	$(CC) -DPRINT_FILE $(CFLAGS) -Fo$@ $< 
   

#template.obj: template.c
#	$(CC) $(CFLAGS) $< 
#
#templatedraw.obj: templatedraw.c
#	$(CC) $(CFLAGS) $< 

#fractions.obj: fractions.c
#	$(CC) $(CFLAGS) $< 