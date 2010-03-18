
__NO_GUI__=$(if $(__WINDOWS__),,1)


#ifeq '$(filter static_debug static_release,$(DEST))' ''
# server.exe 
# would have to be linked with each library
# although I suppose they themselves can be linked statically
# with like LITERAL_LIBNAME extensions
LITERAL_LIBNAME1=bag.msg.core.service
#PROG_MAKELIB1=1
SRCS1=server2
CXFLAGS1=-DBAG
SACK_LIBS1=


PROGNAME2=msgsvr
#PROG_MAKELIB2=1
SRCS2=server3
SUBSYSTEM2=windows
CXFLAGS2=-DSERVERMSG_SOURCE -DBAG
APP_DEFAULT_DATA2=msgsvr.conf

include $(SACK_BASE)/makefiles/makefile.many
#else

#clean distclean all DEFAULT: ;
#endif
