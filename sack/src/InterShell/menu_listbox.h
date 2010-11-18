

struct menu_listbox_tag
{
	struct {
		BIT_FIELD bMultiSelect : 1;
	} flags;
	PSI_CONTROL list;

   _32 scrollbar_width; // might be nice to override this seperate from the font...

	Font *font;
	CTEXTSTR font_name;

   // for edit to have a temp place to set value...
   Font *new_font; 
   CTEXTSTR new_font_name;
};


