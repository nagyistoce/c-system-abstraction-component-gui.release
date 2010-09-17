
HOSTS=bdata.redrock \
  bdata.boulderstation \
  bdata.palacestation \
  bdata.santafestation \
  bdata.texasstation  \
  bdata.sunsetstation

#CONTROL=$(TEST) /usr/src/FortuNet/bdata_controls
CONTROL=$(TEST) /usr/src/FortuNet/alpha2/bdata_controls

all .DEFAULT:
	$(CONTROL) slave $(shell cat /tmp/current_host)
	sleep 2
	$(if $(HOST),echo $(HOST) > /tmp/current_host,)
	$(if $(HOST),$(CONTROL) host $(HOST) $(filter-out $(HOST),$(HOSTS)),)

set_current:
	echo $(HOST) >/tmp/current_host

local:
	$(CONTROL) slave $(shell cat /tmp/current_host)
