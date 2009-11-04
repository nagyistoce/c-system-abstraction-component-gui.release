
LINK_MINIMAL=1
export LINK_MINIMAL
MORE_PROJECTS=$(foreach proj, proxy forward scan user echo whois, $(SACK_BASE)/src/netlib/$(proj)/Makefile )

include $(SACK_BASE)/makefiles/makefile.single

#all .DEFAULT: 
#	$(MAKE) -C $(SACK_BASE) -f Makefile.bag $(MAKECMDGOALS)

