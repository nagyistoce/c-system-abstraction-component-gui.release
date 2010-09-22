
DESTINC1=$(SACK_BASE)/include/genxml
MORE_TARGETS1:=$(DESTINC1)/genx.h $(DESTINC1)
LIBNAME1=genxml
SRCS1=genx charProps
CXFLAGS1=-DGENX_SOURCE

PROGNAME2=test_genxml
SRCS2=tgx
MORELIBS2=genxml

include $(SACK_BASE)/makefiles/makefile.many

$(DESTINC1)/genx.h: | $(DESTINC1)
	-cp genx.h $@

$(DESTINC1): 
	-mkdir $(call SYSCMDPATH,$@)

$(call CLEANTARGET,$(DESTINC1)/genx.h):
	-rm -f $(subst /,$(SYSPATHCHAR),$(DESTINC1)/genx.h)
        
$(call CLEANTARGET,$(DESTINC1)):
	-rm -rf $(subst /,$(SYSPATHCHAR),$(DESTINC1))

