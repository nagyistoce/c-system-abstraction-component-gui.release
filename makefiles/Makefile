ifndef SACK_BASE
all .DEFAULT:
	$(MAKE) -C .. $(MAKECMDGOALS)
else

ifndef MAKEMANY_INCLUDE

ifndef COMPILER
COMPILER=lnx
endif

ifdef COMPILER
 COMMONMAKE:=makefile.$(COMPILER)
endif

#include $(SACK_BASE)/makefiles/$(COMMONMAKE).config

.PHONY: all clean distclean makebuild makeutils

all: makeutils

makeutils:
	-$(MAKE) -C prog/uname -f $(COMMONMAKE)
	-$(MAKE) -C prog/ppc -f $(COMMONMAKE)
ifeq '$(shell uname)' '' 
#aka ifdef __WINDOWS__
	-$(MAKE) -C prog/zerostamp -f $(COMMONMAKE)
endif        
	-$(MAKE) -C prog/echoto -f $(COMMONMAKE)

#$(SACK_BASE)/makefiles/$(COMMONMAKE).config: ;

endif


ifdef __WINDOWS__
include $(SACK_BASE)/makefiles/prog/ppc/makefile.depends
endif

include makefile.common

ifneq '$(filter distclean,$(MAKECMDGOALS))' 'distclean'
#if we're not doing distclean, include
# make-sys depending on mm.exe (which when built is also executed)
#$(SACK_BASE)/makefiles/$(RINTDEST)/make-sys: $(SACK_BASE)/makefiles/$(RINTDEST)/mm$(PROGEXT)

#-include $(SACK_BASE)/makefiles/$(RINTDEST)/make-sys
else
# coincidental switch point... no relation to the above line...
MASTER_LEVEL_DISTCLEAN=1
export MASTER_LEVEL_DISTCLEAN
endif

#distclean: ;

#-include $(SACK_BASE)/makefiles/$(RINTDEST)/make-sys

ifndef MAKEMANY_INCLUDE

makebuild: |debug-wc
	-$(QUIET)rm debug-wc/Makefile.cache
	$(QUIET)$(SACK_BASE)/makefiles/$(RINTDEST)/echoto $(RINTDEST)/Makefile.cache \#place holder file

MYMAKEFILES=$(SYSTEM_MAKE_FILES) $(RINTDEST)/make-sys  $(RINTDEST)/makefile.many.real $(RINTDEST)/makefile.mr

endif
endif
