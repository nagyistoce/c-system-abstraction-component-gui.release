
DESTINC=$(SACK_BASE)/include/sexpat
MORE_TARGETS=$(DESTINC)/sexpat.h $(DESTINC)/sexpat_external.h

LIBNAME=sexpat
SRCS=xmlparse xmlrole xmltok
CXFLAGS=-DSEXPAT_SOURCE 
CXFLAGS+=-DXML_MIN_SIZE
#SYSLIBS=assert

include $(SACK_BASE)/makefiles/makefile.single

$(DESTINC)/sexpat.h: | expat.h $(DESTINC)
	-cp expat.h $@

$(DESTINC)/sexpat_external.h: expat_external.h | $(DESTINC)
	-cp expat_external.h $@

$(DESTINC): 
	-mkdir $(call SYSCMDPATH,$@)

$(call CLEANTARGET,$(DESTINC)/sexpat.h):
	-rm -f $(subst /,$(SYSPATHCHAR),$(DESTINC)/sexpat.h)

$(call CLEANTARGET,$(DESTINC)/sexpat_external.h):
	-rm -f $(subst /,$(SYSPATHCHAR),$(DESTINC)/sexpat_external.h)
        
$(call CLEANTARGET,$(DESTINC)):
	-rm -rf $(subst /,$(SYSPATHCHAR),$(DESTINC))


