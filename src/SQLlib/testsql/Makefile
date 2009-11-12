ifndef SACK_BASE
all .DEFAULT:
	$(MAKE) -C ..
else


__NO_GUI__=1
__LINK_MINIMAL__=1

#CLIENTLIB=bag.pssql
#Some sort of alias for this library to link with instead of direct inclusion.
#CLIENTLIB=pssql_client

LINK_MINIMAL=1

PROGNAME1=testsql1
SRCS1=main
MORELIBS1=$(CLIENTLIB)
ifdef __WINDOWS__
SYSLIBS1=odbc32
else
SYSLIBS1=odbc
endif


PROGNAME7=testsql4
SRCS7=testsql4
MORELIBS7=$(CLIENTLIB)
ifdef __WINDOWS__
SYSLIBS7=odbc32
else
SYSLIBS7=odbc
endif

PROGNAME2=testsql2
SRCS2=testsql
MORELIBS2=$(CLIENTLIB)
ifdef __WINDOWS__
SYSLIBS2=odbc32
else
SYSLIBS2=odbc
endif

PROGNAME3=testsql3
SRCS3=testsqlpush
MORELIBS3=$(CLIENTLIB)
ifdef __WINDOWS__
SYSLIBS3=odbc32
else
SYSLIBS3=odbc
endif

PROGNAME4=sqlcmd
SRCS4=sqlcmd
MORELIBS4=$(CLIENTLIB)
ifdef __WINDOWS__
SYSLIBS4=odbc32
else
SYSLIBS4=odbc
endif

PROGNAME5=testsqlparse
SRCS5=testparse
MORELIBS5=$(CLIENTLIB)
ifdef __WINDOWS__
SYSLIBS5=odbc32
else
SYSLIBS5=odbc
endif

include $(SACK_BASE)/makefiles/makefile.many

endif
