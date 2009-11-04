
ifndef __WINDOWS__
__NO_GUI__=1
endif


APP_DEFAULT_DATA1=summoner.config frames/SummonerControl.Frame
PROGNAME1=summoner
SUBSYSTEM1=windows
SRCS1=summoner
#RSRCS_IDS1=resource.h
CXFLAGS1=-DBAG

LIBNAME2=construct
SRCS2=construct
CXFLAGS2=-DBAG
CXFLAGS2=-DCONSTRUCT_SOURCE
MORELIBS2=

APP_DEFAULT_DATA3=testsummoner.config
PROGNAME3=test_construct
SRCS3=test_construct
CXFLAGS3=-DBAG
MORELIBS3=construct

ifndef __NO_GUI__
PROGNAME4=summoner_avatar
SRCS4=avatar
CXFLAGS4=-DBAG
endif

include $(SACK_BASE)/makefiles/makefile.many
