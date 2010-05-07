#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE


#include <stdhdrs.h> // debugbreak


#include <intershell_export.h>
#include <intershell_registry.h>

#include "../../include/banner.h"
#define KEYPAD_ISP_SOURCE
#include "keypad.h"

/* this resource.h is actually in InterShell */
#include "resource.h"

//KEYPAD_KEYS

//	SYMNAME( KEYPAD_KEYS, WIDE("Keypad Control") )

enum {
   LISTBOX_KEYPAD_TYPE = 4000
};

PRELOAD( RegisterKeypadIDs )
{
   EasyRegisterResource( "InterShell/Keypad" TARGETNAME, LISTBOX_KEYPAD_TYPE, LISTBOX_CONTROL_NAME );
   //EasyRegisterResource( "InterShell/" TARGETNAME, CHECKBOX_DAY_OF_WEEK, RADIO_BUTTON_NAME );
   //EasyRegisterResource( "InterShell/" TARGETNAME, CHECKBOX_ANALOG, RADIO_BUTTON_NAME );
   //EasyRegisterResource( "InterShell/" TARGETNAME, CHECKBOX_DATE, RADIO_BUTTON_NAME );
}
typedef struct page_keypad
{
	PPAGE_DATA page;
   PSI_CONTROL keypad;
   CTEXTSTR keypad_type;
	S_32 x;
	S_32 y;
	_32 w;
	_32 h;
} PAGE_KEYPAD, *PPAGE_KEYPAD;

static struct {
	PSI_CONTROL keypad; // only use one of these, else keybindings suck.
   PLIST keypads;
   PLIST keypad_types;
} l;

void CPROC InvokeKeypadEnterEvent( PTRSZVAL psv, PSI_CONTROL pcKeypad )
{
	PPAGE_KEYPAD keypad = (PPAGE_KEYPAD)psv;

	if( keypad->keypad_type )
	{
		INDEX idx;
		CTEXTSTR name;
		LIST_FORALL( l.keypad_types, idx, CTEXTSTR, name )
		{
			char buffer[256];
			if( StrCmp( name, keypad->keypad_type ) == 0 )
			{
				snprintf( buffer, sizeof( buffer ), TASK_PREFIX "/common/%s/keypad enter", name );
				GETALL_REGISTERED( buffer, void, (PSI_CONTROL) )
				{ /* creates a magic f variable :( */
					if(f) f(pcKeypad);
				}
				ENDALL_REGISTERED();
			}
		}
	}
	else
	{
		GETALL_REGISTERED( TASK_PREFIX "/common/keypad enter", void,(PSI_CONTROL) )
		{  /* creates a magic f variable :( */
			if(f) f(pcKeypad);
		}
		ENDALL_REGISTERED();
	}



}



static int IsGetKeypadControlForType( PSI_CONTROL frame, PPAGE_KEYPAD keypad )
{
	INDEX idx;
	PPAGE_KEYPAD ppk;
	int found = 0;

	LIST_FORALL( l.keypads, idx, PPAGE_KEYPAD, ppk )
	{
		if( keypad != ppk )
		{
			if( StrCmp( keypad->keypad_type, ppk->keypad_type ) == 0 )
			{
            found = 1;
				if( ppk->keypad )
				{
					keypad->keypad = ppk->keypad;
               return TRUE;
				}
			}
		}
	}
	if( !found )
	{
		if( keypad->keypad )
		{
			//HideCommon( keypad->keypad ); // someone else's control...
			// if noone else is using this control it may be destroyed...
			// or it may be promtoed - but then you have to figure out
			// if it is l.keypad (which should never be destroyed)
			if( keypad->keypad != l.keypad )
			{
            /* yeah here, if noone else is using it, destroy it */
			}
		}

		if( !frame )
         DebugBreak(); // where is it that you want this created!?
		keypad->keypad = MakeKeypad( frame
											, keypad->x
											, keypad->y
											, keypad->w
											, keypad->h
											, KEYPAD_KEYS
											, KEYPAD_FLAG_ENTRY|KEYPAD_FLAG_DISPLAY
											, keypad->keypad_type
											);
		SetKeypadEnterEvent( keypad->keypad, InvokeKeypadEnterEvent, (PTRSZVAL)keypad );
		if( !keypad->keypad_type )
			l.keypad = keypad->keypad;
      return TRUE;
	}
	else
	{
		lprintf( "Myself, and someone else in the list other than me don't have  akeypad..." );
      DebugBreak(); // please code a different loop that first finds a keypad, adn then copies to all
	}
   return FALSE;
}


PSI_CONTROL GetPOSKeypad( void )
{
   return l.keypad;
}

PUBLIC( PSI_CONTROL, GetKeypadOfType )( CTEXTSTR type )
{
	INDEX idx;
	PPAGE_KEYPAD keypad;

	LIST_FORALL( l.keypads, idx, PPAGE_KEYPAD, keypad )
	{
		if( keypad->keypad_type && ( StrCaseCmp( keypad->keypad_type, type ) == 0 ) )
         return l.keypad;
	}
   return NULL;
}

OnEditControl( WIDE( "Keypad" ) )( PTRSZVAL psv, PSI_CONTROL pc_parent )
{
	PPAGE_KEYPAD keypad = (PPAGE_KEYPAD)psv;
	PSI_CONTROL frame = LoadXMLFrameOver( pc_parent, "ConfigureKeypad.isFrame" );
	if( frame )
	{
		PSI_CONTROL list;
		int done = 0;
      int okay = 0;
		SetCommonButtons( frame, &done, &okay );
		{
			list = GetControl( frame, LISTBOX_KEYPAD_TYPE );
			if( list )
			{
				INDEX idx;
				CTEXTSTR name;
            AddListItem( list, "- None" );
				LIST_FORALL( l.keypad_types, idx, CTEXTSTR, name )
				{
               AddListItem( list, name );
				}
			}
		}

		DisplayFrame( frame );
      CommonWait( frame );
		DestroyFrame( &frame );
		if( okay )
		{
			if( list )
			{
				PLISTITEM pli = GetSelectedItem( list );
				if( pli )
				{
					TEXTCHAR buffer[256];
					GetListItemText( pli, buffer, sizeof( buffer ) );
					if( keypad->keypad_type )
						Release( (POINTER)keypad->keypad_type );
					if( buffer[0] != '-' )
					{
						keypad->keypad_type = StrDup( buffer );
                  // builds the control attached to keypad thing for typename (only one per type)
						IsGetKeypadControlForType( GetCommonParent( keypad->keypad ), keypad );
					}
					else
					{
						keypad->keypad_type = NULL;
                  // builds the control attached to keypad thing (only one per type)
						IsGetKeypadControlForType( GetCommonParent( keypad->keypad ), keypad );
					}
				}
			}
		}
	}
   return psv;
}

OnCreateControl( WIDE("Keypad") )( PSI_CONTROL frame, S_32 x, S_32 y, _32 w, _32 h )
{
	PPAGE_KEYPAD page_keypad = NULL;
	PKEYPAD keypad;
	{
		//INDEX idx;
		//LIST_FORALL( l.keypads, idx, PPAGE_KEYPAD, page_keypad )
		//{
		//	if( page_keypad->page == ShellGetCurrentPage() )
      //      break;
		//}
		if( !page_keypad )
		{
			page_keypad = New( PAGE_KEYPAD );
			page_keypad->page = ShellGetCurrentPage();
         page_keypad->keypad_type = NULL; // no type... thereofre is default.
         page_keypad->x = x;
         page_keypad->y = y;
         page_keypad->w = w;
			page_keypad->h = h;
			IsGetKeypadControlForType( frame, page_keypad );
			MoveSizeControl( l.keypad
								, page_keypad->x, page_keypad->y
								, page_keypad->w, page_keypad->h );
         AddLink( &l.keypads, page_keypad );
		}
		else
			return NULL;
	}
   return (PTRSZVAL)page_keypad;
}

OnChangePage( WIDE("keypad") )( void )
{
   ClearKeyedEntry( l.keypad );
   return TRUE;
}

OnGetControl( WIDE("Keypad") )(PTRSZVAL psv )
{
	PPAGE_KEYPAD keypad = (PPAGE_KEYPAD)psv;
   return keypad->keypad;
}


OnSaveControl( WIDE("Keypad") )( FILE *file, PTRSZVAL psv )
{
   PPAGE_KEYPAD keypad = (PPAGE_KEYPAD)psv;
   fprintf( file, "Keypad type='%s'\n", keypad->keypad_type );
}

static PTRSZVAL CPROC SetKeypadType( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
   PPAGE_KEYPAD keypad = (PPAGE_KEYPAD)psv;
	keypad->keypad_type = StrDup( name );
	IsGetKeypadControlForType( GetCommonParent( keypad->keypad ), keypad );
   return psv;
}

OnLoadControl( WIDE("Keypad") )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
   AddConfigurationMethod( pch, "Keypad type='%m'", SetKeypadType );
}

// part of restore, might as well be done in fixup?
// fixup is more for content change... editend is for
// edit screen changes - which resize is a property of...
OnQueryShowControl( WIDE("Keypad") )( PTRSZVAL psv )
{
	{
      PPAGE_KEYPAD page_keypad;
		INDEX idx;
		LIST_FORALL( l.keypads, idx, PPAGE_KEYPAD, page_keypad )
		{
			if( page_keypad->page == ShellGetCurrentPage() )
            break;
		}
		if( page_keypad )
		{
			MoveSizeControl( l.keypad
								, page_keypad->x, page_keypad->y
								, page_keypad->w, page_keypad->h );
         return TRUE;
		}
	}
   return FALSE;
}

typedef struct hotkey
{
	struct {
		_32 bNegative : 1;
	} flags;
	PMENU_BUTTON button;
	_64 value;
   Font *font;
	Font *new_font;
   CTEXTSTR preset_name;
} HOTKEY, *PHOTKEY;

OnKeyPressEvent( WIDE("Keypad Hotkey") )( PTRSZVAL psv )
{
	// becuae of the way this has to be created, this event has a funny
   // rule about its parameters...
	PHOTKEY hotkey = (PHOTKEY)(psv);
	//BannerMessage( WIDE("HAH!") );
	if( hotkey->flags.bNegative )
		KeypadInvertValue( l.keypad );
	else
		KeyIntoKeypad( l.keypad, hotkey->value );
}

OnCreateMenuButton( WIDE("Keypad Hotkey") )( PMENU_BUTTON button )
{
	PHOTKEY hotkey = New( HOTKEY );
	MemSet( hotkey, 0, sizeof( *hotkey ) );
   hotkey->button = button;
   InterShell_SetButtonStyle( button, "bicolor square" );
	InterShell_SetButtonText( button, "10" );
	hotkey->value = 10;
   return (PTRSZVAL)hotkey;
}

static void CPROC PickHotkeyFont( PTRSZVAL psv, PSI_CONTROL pc )
{
	PHOTKEY hotkey = (PHOTKEY)psv;
	Font *font = SelectAFont( GetFrame( pc )
									, &hotkey->preset_name
									);
	//PickScaledFont( 0, 0, &g.width_scale, &g.height_scale, &page_label->fontdatalen, &page_label->fontdata, (PCOMMON)GetFrame(pc) );
	if( font )
	{
		hotkey->new_font = font;
	}
}


OnEditControl( WIDE("Keypad Hotkey") )(PTRSZVAL psv, PSI_CONTROL parent_frame )
{
	PHOTKEY hotkey = (PHOTKEY)psv;
   int okay = 0, done = 0;
	PSI_CONTROL frame = LoadXMLFrame( WIDE("hotkey_change_property.isframe") );
	if( frame )
	{
		SetCommonButtons( frame, &done, &okay );
		SetCommonButtonControls( frame );
		if( hotkey->flags.bNegative )
		{
			SetControlText( GetControl( frame, TXT_CONTROL_TEXT ), "-" );
		}
		else
		{
			char value[32];
         snprintf( value, sizeof( value ), WIDE("%Ld"), hotkey->value );
			SetControlText( GetControl( frame, TXT_CONTROL_TEXT ), value );
		}
		SetButtonPushMethod( GetControl( frame, BTN_PICKFONT ), PickHotkeyFont, psv );

      EditFrame( frame, TRUE );
		DisplayFrameOver( frame, parent_frame );
		CommonWait( frame );
		if( okay )
		{
			char buffer[128];
			GetCommonButtonControls( frame );
			GetControlText( GetControl( frame, TXT_CONTROL_TEXT ), buffer, sizeof( buffer ) );
         hotkey->font = hotkey->new_font;
			InterShell_SetButtonFont( hotkey->button, hotkey->font );
			if( strcmp( buffer, "-" ) == 0 )
			{
            hotkey->flags.bNegative = TRUE;
			}
         else
				if( !sscanf( buffer, WIDE("%Ld"), &hotkey->value ) )
					BannerMessage("It's No Good!" );
			UpdateButton( hotkey->button );
		}
       DestroyFrame( &frame );
	}
   return psv;
}

OnSaveControl( WIDE("Keypad Hotkey") )( FILE *file, PTRSZVAL psv )
{
	PHOTKEY hotkey = (PHOTKEY)psv;
	if( hotkey )
	{
      if( hotkey->preset_name )
			fprintf( file, WIDE("hotkey font=%s\n"), hotkey->preset_name );
		if( hotkey->flags.bNegative )
			fprintf( file, WIDE("hotkey is negative sign\n" ) );
      else
			fprintf( file, WIDE("hotkey value=%Ld\n"), hotkey->value );
	}
}

static PTRSZVAL CPROC SetHotkeyValue( PTRSZVAL psv, arg_list args )
{
	PARAM( args, _64, value );
	PHOTKEY hotkey = (PHOTKEY)psv;
	if( hotkey )
	{
      hotkey->value = value;
	}
   return psv;
}
static PTRSZVAL CPROC SetHotkeyFontByName( PTRSZVAL psv, arg_list args )
{
	PHOTKEY hotkey = (PHOTKEY)psv;
	PARAM( args, char *, name );
	hotkey->font = UseAFont( name );
   hotkey->preset_name = StrDup( name );
	return psv;
}

static PTRSZVAL CPROC SetHotkeyNegative( PTRSZVAL psv, arg_list args )
{
	PHOTKEY hotkey = (PHOTKEY)psv;
   hotkey->flags.bNegative = 1;
   return psv;
}

OnLoadControl( WIDE("Keypad Hotkey") )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
   AddConfigurationMethod( pch, WIDE("hotkey font=%m"), SetHotkeyFontByName );
   AddConfigurationMethod( pch, WIDE("hotkey value=%i"), SetHotkeyValue );
   AddConfigurationMethod( pch, WIDE("hotkey is negative sign"), SetHotkeyNegative );
}


OnFixupControl( WIDE("Keypad Hotkey") )( PTRSZVAL psv )
{
	PHOTKEY hotkey = (PHOTKEY)psv;
	char buffer[256];
	int len;
	InterShell_SetButtonFont( hotkey->button, hotkey->font );
	if( hotkey->flags.bNegative )
		InterShell_SetButtonText( hotkey->button, "-" );
	else
	{
		len = snprintf( buffer, sizeof(buffer), WIDE("%Ld"), hotkey->value );
		buffer[len+1] = 0; // double null terminate
		InterShell_SetButtonText( hotkey->button, buffer );
	}
}

void CreateKeypadType( CTEXTSTR name )
{
   AddLink( &l.keypad_types, StrDup( name ) );
}
