ifndef SACK_BASE
all .DEFAULT:
	$(MAKE) -C .. $(MAKECMDGOALS)
else

#autover \

PROJECTS= $(if a$(__NO_GUI__),,$(if $(__WINDOWS__),network_failure_monitor,) shoimg) dirdiff \
pcopy \
reboot \
runres \
runner \
strip \
launcher \
image_colorizer \
$(if $(__WINDOWS__),stripall key_tester waitwin,fbrestore )

#someday keytester could be added... but there's issues with the display library rightnow.


include $(SACK_BASE)/makefiles/makefile.single
endif
