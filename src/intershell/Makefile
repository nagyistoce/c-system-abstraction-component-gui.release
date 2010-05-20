ifndef SACK_BASE
all .DEFAULT:
	$(MAKE) -C .. $(MAKECMDGOALS)
else

all .DEFAULT:
	$(MAKE) -C widgets $(MAKECMDGOALS)
	$(MAKE) -f make1 $(MAKECMDGOALS)
endif
