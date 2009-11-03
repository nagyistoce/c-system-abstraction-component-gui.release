

IMAGES=$(shell $(DIR_CMD) *.png *.jpg *.gif *.bmp)
FONTS=$(shell $(DIR_CMD) *.ttf)

DESTIMAGES=$(foreach image,$(IMAGES),images/$(image))
DESTFONTS=$(foreach font,$(FONTS),fonts/$(font))

APP_DEFAULT_DATA1=$(DESTIMAGES) $(DESTFONTS)

include $(SACK_BASE)/makefiles/makefile.common


Z_Z2=$(FINALDEST)$(BINPATH)$(DEST_SUFFIX)/$(2)/$(1): $(1);$(CP_CMD) $(1) $(subst /,$(SYSPATHCHAR),$(FINALDEST)$(BINPATH)$(DEST_SUFFIX)/$(2)/$(1))

$(foreach font,$(FONTS),$(eval $(call Z_Z2,$(font),fonts)))
$(foreach image,$(IMAGES),$(eval $(call Z_Z2,$(image),images)))

moreclean:
	rm -rf $(addprefix $(FINALDEST)$(DATAPATH)$(DEST_SUFFIX)/,$(APP_DEFAULT_DATA1) images fonts) 

