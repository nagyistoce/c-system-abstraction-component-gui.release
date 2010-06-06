
SET( INTERSHELL_FRAMES
ConfigureAnimationButton.isFrame
ConfigureMacroButton.isFrame
EditBannerMessage.isFrame
EditGenericButton.isFrame
EditGenericControl.isFrame
EditGenericListbox.isFrame
EditGlareSets.isFrame
EditPlugins.isFrame
InterShellPageProperty.isFrame
ListboxProperty.isFrame
MilkPageProperty.isFrame
configure_text_setvar_button.isFrame
fbi_gui_property.isFrame
font_preset_property.isFrame
page_change_property.isFrame
page_label_property.isFrame
property.isFrame
user_stat_prop.isFrame

)




INSTALL( 
  FILES
    ${INTERSHELL_FRAMES}
  DESTINATION
    ${DATA_INSTALL_PREFIX}/frames
)




