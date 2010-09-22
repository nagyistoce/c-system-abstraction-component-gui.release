
INCLUDEDIRS=$(SACK_BASE)/include/snmp
LIBNAME=snmplib
SRCS=asn1          coexistance   mib           mibii          mini-client      \
     new-parse     parse         snmp_api      snmp_api_error snmp_api_util \
     snmp_client   snmp_dump     snmp_error    snmp_extra     snmp_msg      \
     snmp_pdu      snmp_vars     test-mib      version

ifdef __WINDOWS__
MORELIBS=network
SYSLIBS=wsock32
CXFLAGS=-DWIN32
else
MORELIBS=network
endif

include $(SACK_BASE)/makefiles/makefile.single