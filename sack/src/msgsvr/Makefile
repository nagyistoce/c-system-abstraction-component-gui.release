
all .DEFAULT:
	make -C .. $(MAKECMDGOALS)

ifdef bananarama
BASE_PATH=.

PROJECTS=client

ifndef SRC_DIR_LEVEL_MAKE
# this should be made in peer with these, 
# it requires systray, which requires much else... so 
# only build this if we're not coming from $(SACK_BASE)/src/Makefile
PROJECTS+=server summoner servicelist
endif

include $(SACK_BASE)/makefiles/makefile.projects

endif
