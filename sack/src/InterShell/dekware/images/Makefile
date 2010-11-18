
SUITS=Clubs Diamonds Hearts Spades
FILES=$(foreach file,$(shell $(DIR_CMD) cards$(SYSPATHCHAR)*.config),cards/$(file))    \
	$(foreach suit,$(SUITS)  \
          ,$(foreach file,$(shell $(DIR_CMD) cards$(SYSPATHCHAR)newdeck$(SYSPATHCHAR)$(suit)$(SYSPATHCHAR)LifeSize$(SYSPATHCHAR)*.png),cards/newdeck/$(suit)/LifeSize/$(file)))
        

APP_DEFAULT_DATA=$(foreach cards,$(FILES),images/$(cards))

include $(SACK_BASE)/makefiles/makefile.single

#$(warning  $(APP_DEFAULT_DATA) )
#$(warning $(FILES))

#$(FINALDEST)$(DATAPATH)$(DEST_SUFFIX)/images/cards/: 
#	-mkdir $(subst /,$(SYSPATHCHAR),$(subst /.,,$@.))

$(foreach thing,$(FILES),$(eval $(FINALDEST)$(DATAPATH)$(DEST_SUFFIX)/images/$(thing): $(thing); cp $$< $$@ ))


$(foreach suit,$(SUITS),$(eval $(FINALDEST)$(DATAPATH)$(DEST_SUFFIX)/images/cards/newdeck/$(suit): ;mkdir $$(subst /,$$(SYSPATHCHAR),$$(subst /.,,$$@.))))
	

