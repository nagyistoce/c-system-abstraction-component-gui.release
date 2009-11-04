
#all .DEFAULT:
#	$(MAKE) -C .. $(MAKECMDGOALS)

PROJECTS=$(if $(MASTER_LEVEL_DISTCLEAN),,..) $(if $(__NO_OPTIONS__),,checkoptions getopttest editoption importini )
#importini

include $(SACK_BASE)/makefiles/makefile.single
