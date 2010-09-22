
PROGNAME=listbox
SRCS=listbox

ifneq '$(filter static_release static_debug,$(DEST))' ''
 ifndef __WINDOWS__
  SYSLIBS+=$(if $(__ARM__),,png jpeg z SDL)
 else
 SYSLIBS+=wsock32 opengl32 glu32
 endif
endif

include $(SACK_BASE)/makefiles/makefile.single

