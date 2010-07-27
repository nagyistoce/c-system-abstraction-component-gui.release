
#ifndef FIRST_SYMBOL_VALUE
#define FIRST_SYMBOL_VALUE 1
#endif

#ifndef BUILD_NAMES
#define FIRST_SYMNAME(name,control_type_name)    FIRST_SYMBOL = FIRST_SYMBOL_VALUE, name = FIRST_SYMBOL_VALUE
#define SYMNAME(name,control_type_name)        , name
#define SYMNAME_SKIP(prior, range, name,control_type_name)        , prior, name = prior+range

enum resource_enum {
#endif

FIRST_SYMNAME( BTN_OKAY, NORMAL_BUTTON_NAME )
SYMNAME(BTN_CANCEL,NORMAL_BUTTON_NAME)
SYMNAME( EDT_X     , EDIT_FIELD_NAME )
SYMNAME( EDT_Y     , EDIT_FIELD_NAME )
SYMNAME( EDT_WIDTH , EDIT_FIELD_NAME )
SYMNAME( EDT_HEIGHT, EDIT_FIELD_NAME )
SYMNAME( EDT_CAPTION, EDIT_FIELD_NAME)
SYMNAME( EDT_ID     , EDIT_FIELD_NAME)
SYMNAME( EDT_IDNAME , EDIT_FIELD_NAME)
SYMNAME( LABEL_X    , STATIC_TEXT_NAME )
SYMNAME( LABEL_Y    , STATIC_TEXT_NAME)
SYMNAME( LABEL_WIDTH  , STATIC_TEXT_NAME)
SYMNAME( LABEL_HEIGHT , STATIC_TEXT_NAME)
SYMNAME( LABEL_CAPTION, STATIC_TEXT_NAME)
SYMNAME( LABEL_ID     , STATIC_TEXT_NAME)
SYMNAME( LISTBOX_IDS , LISTBOX_CONTROL_NAME  )


SYMNAME( SLD_GREENBAR    , SLIDER_CONTROL_NAME )
SYMNAME( PAL_COLORS      , WIDE( "Color Matrix" ) )
SYMNAME( BTN_PRESET      , NORMAL_BUTTON_NAME ) // define a preset
SYMNAME( CHK_ALPHA       , RADIO_BUTTON_NAME  )
SYMNAME( CST_SHADE       , WIDE("Shade Well") )
SYMNAME( CST_ZOOM        , WIDE("Shade Well") )
SYMNAME( CST_SHADE_RED   , WIDE("Shade Well") )
SYMNAME( CST_SHADE_BLUE  , WIDE("Shade Well") )
SYMNAME( CST_SHADE_GREEN , WIDE("Shade Well") )
SYMNAME_SKIP( BTN_PRESET_BASE, 64, BTN_PRESET_LAST, CUSTOM_BUTTON_NAME )

#ifndef BUILD_NAMES
};
#endif
#undef SYMNAME
#undef FIRST_SYMNAME
#undef SYMNAME_SKIP

