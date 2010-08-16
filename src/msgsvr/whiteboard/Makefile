

PROGNAME1=whiteboard_service
SRCS1=whiteboard 
# if debug add whiteboard_control
# which will allow this service to create a visible
# board which will monitor all clients...
CXFLAGS1=-DWHITEBOARD_SERVICE_SOURCE
MORELIBS1=

LIBNAME2=whiteboard_client
SRCS2=whiteboard_client
CXFLAGS2=-DWHITEBOARD_CLIENT
MORELIBS2=

ifeq '0' '1'
ifndef DEKWARE_BASE
DEKWARE_BASE=$(SACK_BASE)/../dekware
endif
LITERAL_LIBNAME3=whiteboard.nex
SRCS3=ntlink whiteboard_client
MORE_LITERAL_LIBS3=dekware.core
CXFLAGS3=-I$(DEKWARE_BASE)/include -DDEKWARE_PLUGIN -DWHITEBOARD_CLIENT
endif

include $(SACK_BASE)/makefiles/makefile.many

