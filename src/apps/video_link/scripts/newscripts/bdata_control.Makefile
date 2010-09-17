
HOSTS=bdata.redrock \
  bdata.boulderstation \
  bdata.sunsetstation \
  bdata.palacestation \
  bdata.santafestation \
  bdata.texasstation

CONTROL=$(TEST) /usr/src/FortuNet/bdata_controls

all .DEFAULT:
	$(CONTROL) reset $(HOSTS)
	sleep 2
	$(if $(HOST),$(CONTROL) host $(HOST) $(filter-out $(HOST),$(HOSTS)),)

local:
	$(CONTROL) slave bdata.$(shell hostname)
