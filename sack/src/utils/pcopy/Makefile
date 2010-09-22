
LINK_MINIMAL=1

ifeq '$(shell uname -o)' 'Cygwin'
__WINDOWS__=1
endif

PROGNAME=$(if $(__WINDOWS__),pcopy,)
SRCS=pcopy docopy
MORELIBS=
ifneq '$(filter static_debug static_release,$(DEST))' ''
SYSLIBS+=wsock32
endif

MORE_TARGETS=$(if $(__WINDOWS__),,$(FINALDEST)/bin/$(RINTDEST)/pcopy)


include $(SACK_BASE)/makefiles/makefile.single

ifndef __WINDOWS__
$(FINALDEST)/bin/$(RINTDEST)/pcopy: pcopy.sh
	$(QUIETCMD)cp pcopy.sh $@
	$(QUIETCMD)chmod 755 $@

$(call CLEANTARGET,$(FINALDEST)/bin/$(RINTDEST)/pcopy) :
	$(QUIETCMD)-rm $(FINALDEST)/bin/$(RINTDEST)/pcopy
endif

