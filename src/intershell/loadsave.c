//#define DEBUG_CONIG_STATE

#include <stdhdrs.h>

#if defined( __WINDOWS__ ) || defined( _MSC_VER )
#include <io.h> // unlink() // also no chsize() (if I can get it to work)
#else
#include <unistd.h> // unlink()
#endif
#include <stdio.h>
#include <sharemem.h>
#include <configscript.h>
/* these are quick hacks implemented to generilize the above */
/* they should perhaps be moved out to intershell_registry.h for use by OnSave and OnLoad method */
#define MakeElem( w, name, text ) genxElement name = genxDeclareElement( w, NULL, text, &l.status )
#define MakeAttr( w, name, text ) genxAttribute name = genxDeclareAttribute( w, NULL, text, &l.status )
#define AddAttr( attr, format, ... ) { char tmp[256]; snprintf( tmp, sizeof( tmp ), format,## __VA_ARGS__ ); genxAddAttribute( attr, (constUtf8)tmp ); }

#include <filesys.h> // pathrchr
#include <sexpat/sexpat.h>
#include "intershell_local.h"
#include "resource.h"
#include <pssql.h>
#include <sqlgetoption.h>
#include "widgets/include/banner.h"
#include "menu_real_button.h"

#include <psi.h>

#include "pages.h"


extern CONTROL_REGISTRATION menu_surface;

static struct {
	struct {
		// current psv passed is a PTRSZVAL(custom control result) not a MENU_BUTTON
		_32 bCustom : 1;
	} flags;
	PLINKSTACK current_button; // use this cause sometimes psv isn't a MENU_BUTTON
	PLINKSTACK current_canvas; // this is a stack of PSI_CONTROLs that are type menu_surface.TypeID, (PCanvasData)
	genxStatus status;
   PLIST unhandled_global_lines;
} l;

static PCONFIG_HANDLER my_current_handler;

void AddCommonButtonConfig( PCONFIG_HANDLER pch, PMENU_BUTTON button );



PCONFIG_HANDLER InterShell_GetCurrentConfigHandler( void )
{
   return my_current_handler;
}

PTRSZVAL CPROC SetMenuRowCols( PTRSZVAL psv, arg_list args );

void SetDefaultRowsCols( void )
{
	va_args args;
	PCanvasData canvas = GetCanvas( (PSI_CONTROL)PeekLink( &l.current_canvas ) );
	init_args( args );

	if( !canvas->flags.bSetResolution )
	{
		PushArgument( args, _64, 5 );
		PushArgument( args, _64, 5 );
		SetMenuRowCols( 0, pass_args( args ) );
		PopArguments( args );
	}
}

PMENU_BUTTON InterShell_GetCurrentLoadingControl( void )
{
   return (PMENU_BUTTON)PeekLink( &l.current_button );

}

static PTRSZVAL CPROC ResetConfig( PTRSZVAL psv, arg_list args )
{
	PMENU_BUTTON current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button && psv )
	{
		if( current_button )
		{
#ifdef DEBUG_CONIG_STATE
			lprintf( "Previous psvUser was %p is now %p", current_button->psvUser, psv );
#endif
			current_button->psvUser = psv;
		}
	}
	EndConfiguration( my_current_handler );

	current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
#ifdef DEBUG_CONIG_STATE
		lprintf( "Button's psv is %ld",current_button->psvUser );
#endif
		return current_button->psvUser;
	}
	return 0;
}

//---------------------------------------------------------------------------
static PTRSZVAL CPROC ResetCanvasConfig( PTRSZVAL psv, arg_list args )
{
	PSI_CONTROL pc_canvas = (PSI_CONTROL)PopLink( &l.current_canvas );
	PCanvasData canvas = GetCanvas( pc_canvas=(PSI_CONTROL)PeekLink( &l.current_canvas ) );
	if( g.flags.multi_edit )
	{
		RestorePage( pc_canvas, canvas, canvas->current_page, TRUE );
	}
	ShellSetCurrentPageEx( pc_canvas, "first" );
   // really this behaves more like a pop configuration.
	EndConfiguration( my_current_handler );
	return psv;
}
//---------------------------------------------------------------------------
static PTRSZVAL CPROC ResetMainCanvasConfig( PTRSZVAL psv, arg_list args )
{
   // this is not allowed to pop the current (master/main) canvas.
	PSI_CONTROL pc_canvas = (PSI_CONTROL)PeekLink( &l.current_canvas );
	PCanvasData canvas = GetCanvas( pc_canvas=(PSI_CONTROL)PeekLink( &l.current_canvas ) );
	if( g.flags.multi_edit )
	{
		RestorePage( pc_canvas, canvas, canvas->current_page, TRUE );
	}
	//ShellSetCurrentPageEx( pc_canvas, "first" );
	return psv;
}
//---------------------------------------------------------------------------
static PTRSZVAL CPROC SetListMultiSelect( PTRSZVAL psv, arg_list args )
{
	if( psv )
	{
		PARAM( args, LOGICAL, bMultiSelect );
		PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
 
		SetListboxMultiSelect( (PSI_CONTROL)current_button->control.control, bMultiSelect );
	}
	return psv;
}

//---------------------------------------------------------------------------
static PTRSZVAL CPROC SetListMultiLazySelect( PTRSZVAL psv, arg_list args )
{
	if( psv )
	{
		PARAM( args, LOGICAL, bMultiSelect );
		PARAM( args, LOGICAL, bLazyMulti );
		PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
 
		SetListboxMultiSelectEx( (PSI_CONTROL)current_button->control.control, bMultiSelect, bLazyMulti );
	}
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC UnhandledLine( PTRSZVAL psv, CTEXTSTR line )
{
	if( line && strlen( line ) )
	{
		PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
		if( current_button )
		{
			AddLink( &current_button->extra_config, StrDup( line ) );
         lprintf( "Unhandled line added to current button..." );
		}
		else
		{
			AddLink( &l.unhandled_global_lines, StrDup( line ) );
         lprintf( "Unhandled line added to global config button..." );
		}
		xlprintf(LOG_ALWAYS)( "Received unhandled line: %s", line );
	}
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC ProcessLast( PTRSZVAL psv )
{
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
#ifdef DEBUG_CONIG_STATE
		lprintf( "POP BUTTON" );
#endif
		PopLink( &l.current_button );
	}
	current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
		return current_button->psvUser;
	}
	return 0; //psv; // psv is often user defined, and resembles l.current_button->key
}

//---------------------------------------------------------------------------

PLIST prior_configs;

LOGICAL BeginSubConfiguration( char *control_type_name, const char *end_type_name )
{
	//lprintf( "Beginning a sub configuration for %s ending at %s", control_type_name, end_type_name );
	char buf[256];
   snprintf( buf, sizeof( buf ), "%s/%s", control_type_name, end_type_name );
	if( !BeginNamedConfiguration( my_current_handler, buf ) )
	{
		// these have to be added to this one.
		// the configuration handler may itself start another
		// sub-configuration, and these would be stacked in the wrong spot otherwise.
		AddConfigurationMethod( my_current_handler, end_type_name, ResetConfig );
		SetConfigurationUnhandled( my_current_handler, UnhandledLine );
		{
			char rootname[256];
			void (CPROC*f)(PCONFIG_HANDLER,PTRSZVAL);
			PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
#ifdef DEBUG_CONIG_STATE
			lprintf( "Push current (%s end at %s)button. (double push, cause we may be calling a macro which will change this state?)", control_type_name, end_type_name );
#endif
			PushLink( &l.current_button
					  , current_button );
			snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", control_type_name );
			f = GetRegisteredProcedure2( rootname, void, WIDE("control_config"), (PCONFIG_HANDLER,PTRSZVAL) );
			if( f )
			{
				//lprintf( "Gave control a chance to register additional methods on current config... " );
				f( my_current_handler, current_button?current_button->psvUser:0 );
			}

			// add additional security plugin stuff...
			if( current_button )
			{
				CTEXTSTR name;
				PCLASSROOT data = NULL;
				//lprintf( "Gave control a chance to register additional security methods on current config... " );
				for( name = GetFirstRegisteredName( TASK_PREFIX "/common/Load Security", &data );
					 name;
					  name = GetNextRegisteredName( &data ) )
				{
					void (CPROC*f)(PCONFIG_HANDLER);
					//snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/common/save common/%s", name );
					f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PCONFIG_HANDLER) );
					if( f )
						f( my_current_handler );
				}
			}
		}
		/* sub configuraiton end (EndConfigruation(pch)), invokes this, so we can pop the correct steps */
		SetConfigurationEndProc( my_current_handler, ProcessLast );
      return FALSE;
	}
	else
	{
		// recoveredconfig, don't need to call application to have additional config methods..
      return TRUE;
	}
   //lprintf( "Done setting up subconfig" );
}

//---------------------------------------------------------------------------

static PTRSZVAL CPROC AddAllowedSystemShow( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
      AddLink( &current_button->show_on, SegCreateFromText( name ) );
	}
   return psv;
}

//---------------------------------------------------------------------------

static PTRSZVAL CPROC AddDisallowedSystemShow( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
      AddLink( &current_button->no_show_on, SegCreateFromText( name ) );
	}
   return psv;
}



//---------------------------------------------------------------------------

PTRSZVAL CPROC CreateNewControl( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, type );
	PARAM( args, _64, col );
	PARAM( args, _64, row );
	// this will be backwards compatible if we should
	// happen to save a new menu, the old menu uhh
	// will probably puke and die...
	PARAM( args, _64, width );
	PARAM( args, _64, height );
	PSI_CONTROL pc_canvas;
	//PCanvasData canvas;
   pc_canvas=(PSI_CONTROL)PeekLink( &l.current_canvas );
	//canvas = GetCanvas( pc_canvas );
	SetDefaultRowsCols();
	if( psv )
	{
		// pass any previous button a notification that it's done...
		// actually ... just wait until the button shows.
	}
	//if( row < PARTSX && col < PARTSY )
	{
		// everything is a generic control anymore
		// create some control first creates a button (to track position of control)
		// then tries to create a custom control (issue_pos/control/create_control)
		// then tries invoke "key_create" issue_pos/control/key_create
		// then tries "button_create"
		// then tries "contained_button_create"
		if( strncmp( type, WIDE("generic"), 7 ) == 0 )
		{
			PMENU_BUTTON button;
			char *control_type_name = strchr( type, ' ' );
			if( control_type_name )
			{
            LOGICAL bRecovered;
				while( control_type_name[0] == ' ' ) control_type_name++;
				// control_type_name is the name of the type of control to create...
				// there may be nothing of that name (anymore?)
				//lprintf( WIDE("Set current button...") );
#ifdef DEBUG_CONIG_STATE
				lprintf( "(push)create a: %s", control_type_name );
#endif
				PushLink( &l.current_button
						  , button = CreateSomeControl( pc_canvas, col, row, width, height, control_type_name ) );
				if( button )
				{
#define GetRegisteredProcedure2(nc,rtype,name,args) (rtype (CPROC*)args)GetRegisteredProcedureEx((nc),#rtype, name, #args )
					bRecovered = BeginSubConfiguration( control_type_name, WIDE("control done") );
               if( !bRecovered )
						if( !button->flags.bNoCreateMethod )
							AddCommonButtonConfig( my_current_handler, button );
				}
				if( !bRecovered )
				{
					if( button && button->flags.bListbox )
					{
						AddConfigurationMethod( my_current_handler, "multi select? %b lazy? %b", SetListMultiLazySelect );
						AddConfigurationMethod( my_current_handler, "multi select? %b", SetListMultiSelect );
					}
					//lprintf( "Adding configuration to button thing " );
					AddConfigurationMethod( my_current_handler, WIDE("Allow show on %m" ), AddAllowedSystemShow );
					AddConfigurationMethod( my_current_handler, WIDE("Disallow show on %m" ), AddDisallowedSystemShow );
				}
				//lprintf( "..." );
				// automagic fields for all controls to control show on controls
            //lprintf( "Button psv = %08x", button->psvUser );
				return button->psvUser;
			}
			else
			{
				lprintf( "Unknown control name: %s", type );
			}
		}
	}
	return  NULL;
}

void SetCurrentLoadingButton( PMENU_BUTTON button )
{
	/* replace the current button with some specified button , this is specifically for use by macros! */
#ifdef DEBUG_CONIG_STATE
	lprintf( "Set (push)current button" );
#endif
   //PopLink( &l.current_button );
	PushLink( &l.current_button, button );
}

//---------------------------------------------------------------------------

static PTRSZVAL CPROC SetMenuButtonColor( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CDATA, color );
	//lprintf( WIDE("menubutton color..") );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
		current_button->color = color;
   return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetMenuButtonHighlightColor( PTRSZVAL psv, arg_list args )
{
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	PARAM( args, CDATA, color );
	//lprintf( WIDE("...") );
	if( current_button )
		current_button->highlight_color = color;
   return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetMenuButtonSecondaryColor( PTRSZVAL psv, arg_list args )
{
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	PARAM( args, CDATA, color );
	//lprintf( WIDE("...") );
	if( current_button )
		current_button->secondary_color = color;
   return psv;
}

//---------------------------------------------------------------------------

static PTRSZVAL CPROC SetMenuButtonTextColor( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CDATA, color );
	// lprintf( WIDE("...") );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
		current_button->textcolor = color;
	return psv;
}

PTRSZVAL CPROC SetMenuBackgroundColor( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CDATA, color );
	//lprintf( WIDE("...") );
	PCanvasData canvas = GetCanvas( (PSI_CONTROL)PeekLink( &l.current_canvas ) );
	canvas->current_page->background_color = color;
	return psv;
}

PTRSZVAL CPROC SetMenuBackground( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, filename );
	PCanvasData canvas = GetCanvas( (PSI_CONTROL)PeekLink( &l.current_canvas ) );
	if( canvas->current_page->background ) Release( (POINTER)canvas->current_page->background );
	if( canvas->current_page->background_image ) UnmakeImageFile( canvas->current_page->background_image );
	canvas->current_page->background = StrDup( filename );
	canvas->current_page->background_image = NULL;
	//SmudgeCommon( canvas ); // updated the background image, paint it now for further transparent controls.

	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetMenuButtonImageMargin( PTRSZVAL psv, arg_list args )
{
   PARAM( args, S_64, hMargin );
	PARAM( args, S_64, vMargin );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
		current_button->decal_horiz_margin = hMargin;
      current_button->decal_vert_margin = vMargin;
		SetKeyImageMargin( current_button->control.key, hMargin, vMargin );
	}

   return psv;
}
PTRSZVAL CPROC SetMenuButtonImage( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, text );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
		InterShell_SetButtonImage( current_button, text );
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetMenuButtonText( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, text );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
		if( current_button->text )
			Release( current_button->text );
		current_button->text = StrDup( text );
	}
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetButtonRound( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, type );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
      current_button->glare_set = GetGlareSet( type );
	}
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetAllowEdit( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bAllow );
	g.flags.bNoEdit = !bAllow;
	g.flags.bNoEditSet = 1;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetAllowMultiInstance( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bAllow );
	if( bAllow )
	{
		Release( g.mem_lock );
		g.mem_lock = NULL;
	}
	g.flags.bAllowMultiLaunch = bAllow;
	g.flags.bAllowMultiSet = 1;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetRoundGlare( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, type );
	PARAM( args, CTEXTSTR, filename );
	PGLARE_SET glare_set = GetGlareSet( type );
	if( strlen( filename ) )
		glare_set->glare = StrDup( filename );
	else
		glare_set->glare = NULL;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetRoundUp( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, type );
	PARAM( args, CTEXTSTR, filename );
	PGLARE_SET glare_set = GetGlareSet( type );
	if( strlen( filename ) )
		glare_set->up = StrDup( filename );
	else
		glare_set->up = NULL;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetRoundMonoShade( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, type );
	PGLARE_SET glare_set = GetGlareSet( type );
	glare_set->flags.bShadeBackground = 1;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetRoundMultiShade( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, type );
	PGLARE_SET glare_set = GetGlareSet( type );
	glare_set->flags.bMultiShadeBackground = 1;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetControlFontPreset( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, fontname );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
		current_button->font_preset_name = StrDup( fontname );
		current_button->font_preset = UseAFont( current_button->font_preset_name );
	}
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetRoundDown( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, type );
	PARAM( args, CTEXTSTR, filename );
	PGLARE_SET glare_set = GetGlareSet( type );
	if( strlen( filename ) )
		glare_set->down = StrDup( filename );
	else
		glare_set->down = NULL;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetRoundMask( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, type );
	PARAM( args, CTEXTSTR, filename );
	PGLARE_SET glare_set = GetGlareSet( type );
	if( strlen( filename ) )
		glare_set->mask = StrDup( filename );
	else
		glare_set->mask = NULL;
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetButtonNoPress( PTRSZVAL psv, arg_list args )
{
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
		current_button->flags.bNoPress = 1;
	}
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetMenuRowCols( PTRSZVAL psv, arg_list args )
{
	PARAM( args, _64, cols );
	PARAM( args, _64, rows );
	_32 button_rows, button_cols, button_space;
	// 25 PART_RESOLUTION's?
	PCanvasData canvas = GetCanvas( (PSI_CONTROL)PeekLink( &l.current_canvas ) );
	button_space = 0;
	button_rows = rows;
	button_cols = cols;

	if( button_cols == 0 )
	      button_cols = 5;
	if( button_cols < 11 )
		canvas->current_page->grid.nPartsX =
			canvas->nPartsX = (button_cols*8);
	else
		canvas->current_page->grid.nPartsX =
			canvas->nPartsX = (button_cols);
	if( button_rows == 0 )
	      button_rows = 5;
	if( button_cols < 11 )
		canvas->current_page->grid.nPartsY =
			canvas->nPartsY = (button_rows*8);
	else
		canvas->current_page->grid.nPartsY =
			canvas->nPartsY = button_rows;
	canvas->flags.bSetResolution = 1;

	//lprintf( WIDE("Attempt to make font for screen 40x25")
	//  	 , X( (PART_RESOLUTION-(g.button_space*g.button_cols))/(g.button_cols*10)  )
	//  	 , Y( (PART_RESOLUTION-(g.button_space*g.button_rows))/(g.button_rows*5) ), 0 )
	if( !g.keyfont )
	{
		g.keyfont = RenderFontFile( WIDE("./fonts/arialbd.ttf")
										  , _X(canvas, (PART_RESOLUTION)/(canvas->nPartsX) )
										  , _Y(canvas, (PART_RESOLUTION)/(canvas->nPartsY) )
										  , 3 ); // 8 bit alpha (1=0,2=1,4=2,8=3)
	}
	// this scaled font... what's the reverse of this if applied
	// with g.width_scale and g.height_scale?  (which is the screen proportion scalar)
   // the above is a button surface relative scaling...
   return psv;
}

//---------------------------------------------------------------------------

#if 0
static PTRSZVAL CPROC EndConfig( PTRSZVAL psv, char *line )
{
	if( line )
	{
		lprintf( WIDE("unsupported line: %s"), line );
		SetPaperIssueText( ((PMENU_BUTTON)psv)->key, ((PPAPER_INFO)((PMENU_BUTTON)psv)->psvUser) );
		EndConfiguration( my_current_handler );
	}
   return psv;
}
#endif

PTRSZVAL GetButtonExtension( PMENU_BUTTON button )
{
	return button->psvUser;
}

static PTRSZVAL CPROC ReadNextPage( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	PMENU_BUTTON   current_button = (PMENU_BUTTON)PeekLink( &l.current_button );
	if( current_button )
	{
		current_button->pPageName = StrDup( name );
	}
   return psv;
}

void AddCommonButtonConfig( PCONFIG_HANDLER pch, PMENU_BUTTON button )
{
	// older config allows these anywhere... which is okay I guess... everything's a button anyhow
	// even if it's only basically the size parameters that matter...
	// SAVE will cure the fact that this does not apply to custom buttons
	//if( !button->flags.bCustom )
	{
		AddConfigurationMethod( pch, WIDE("color=%c"), SetMenuButtonColor );
		AddConfigurationMethod( pch, WIDE("highlight color=%c"), SetMenuButtonHighlightColor );
		AddConfigurationMethod( pch, WIDE("secondary color=%c"), SetMenuButtonSecondaryColor );
		AddConfigurationMethod( pch, WIDE("text color=%c"), SetMenuButtonTextColor );
		AddConfigurationMethod( pch, WIDE("image=%m"), SetMenuButtonImage );
		AddConfigurationMethod( pch, WIDE("image_margin=%i,%i"), SetMenuButtonImageMargin );
		AddConfigurationMethod( pch, WIDE("button is %m"), SetButtonRound );
		AddConfigurationMethod( pch, WIDE("button is unpressable"), SetButtonNoPress );
		AddConfigurationMethod( pch, WIDE("text=%m"), SetMenuButtonText );
		AddConfigurationMethod( pch, WIDE("next page=%m"), ReadNextPage );
		AddConfigurationMethod( pch, WIDE("font name=%m" ), SetControlFontPreset );
	}
	//AddConfigurationMethod( pch, WIDE("control %m at %i,%i sized %i,%i"), ResetConfig );
	//AddConfigurationMethod( pch, WIDE("control done"), ResetConfig );
}

void PublicAddCommonButtonConfig( PMENU_BUTTON button )
{
   AddCommonButtonConfig( my_current_handler, button );
}


PTRSZVAL CPROC CreateTitledPage( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, title );
	PSI_CONTROL pc_canvas = (PSI_CONTROL)PeekLink( &l.current_canvas );
   CreateNamedPage( pc_canvas, title );
	return psv;
}

//---------------------------------------------------------------------------

void AddCommonCanvasConfig( PCONFIG_HANDLER pch )
{
	// pages are actually top-level sort of things... they don't have a 'Page Done'
	// sub configuration yet...
	AddConfigurationMethod( pch, WIDE("page titled %m"), CreateTitledPage );
	AddConfigurationMethod( pch, WIDE("menu background image %m"), SetMenuBackground );
	AddConfigurationMethod( pch, WIDE("menu background color %c"), SetMenuBackgroundColor );
	AddConfigurationMethod( pch, WIDE("background image %m"), SetMenuBackground );
	AddConfigurationMethod( pch, WIDE("background color %c"), SetMenuBackgroundColor );
	AddConfigurationMethod( pch, WIDE("page layout %i by %i"), SetMenuRowCols );
	AddConfigurationMethod( pch, WIDE("Allow Edit? %b"), SetAllowEdit );
	AddConfigurationMethod( pch, WIDE("Allow Multi Run? %b"), SetAllowMultiInstance );

	AddConfigurationMethod( pch, WIDE("control %m at %i,%i sized %i,%i"), CreateNewControl );
	AddConfigurationMethod( my_current_handler, "Canvas Done", ResetCanvasConfig );
	AddConfigurationMethod( my_current_handler, WIDE("page layout %i by %i"), SetMenuRowCols );
}

void BeginCanvasConfiguration( PSI_CONTROL pc_canvas )
{
	/*
	 * need to create a new canvas here?
	 */
   //lprintf( "Push new canvas" );
	PushLink( &l.current_canvas, pc_canvas );
	BeginConfiguration( my_current_handler );

	//AddConfigurationMethod( my_current_handler, "Canvas Done", ResetCanvasConfig );
	//AddConfigurationMethod( my_current_handler, WIDE("page layout %i by %i"), SetMenuRowCols );
	AddConfigurationMethod( my_current_handler, "Canvas Done", ResetCanvasConfig );
	AddCommonCanvasConfig( my_current_handler );
	SetConfigurationUnhandled( my_current_handler, UnhandledLine );
}


void InvokeLoadCommon( void )
{
		CTEXTSTR name;
		PCLASSROOT data = NULL;
		for( name = GetFirstRegisteredName( TASK_PREFIX "/common/common_config", &data );
			 name;
			  name = GetNextRegisteredName( &data ) )
		{
			void (CPROC*f)(PCONFIG_HANDLER);
			f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PCONFIG_HANDLER pch) );
			if( f )
			{
				TEXTCHAR buf[256];
				snprintf( buf, sizeof( buf ), "%s:%s", name, WIDE("executed") );
				if( !GetRegisteredIntValue( (CTEXTSTR)data, buf ) )
				{
					RegisterIntValue( (CTEXTSTR)data, buf, 1 );
					f( my_current_handler );
				}
			}
		}
}

void LoadButtonConfig( PSI_CONTROL pc_canvas, TEXTSTR filename )
{
	PCONFIG_HANDLER pch;
	//ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, g.single_frame );
   //lprintf( "Push initial current canvas" );
	PushLink( &l.current_canvas, pc_canvas );
	my_current_handler = pch = CreateConfigurationEvaluator();
	// if this is not done first, then the system will default to 5.

	// these are only on the very top level canvas...
	AddConfigurationMethod( pch, WIDE("%m button mono shade"), SetRoundMonoShade );
	AddConfigurationMethod( pch, WIDE("%m button multi shade"), SetRoundMultiShade );
	AddConfigurationMethod( pch, WIDE("%m button glare=%m"), SetRoundGlare );
	AddConfigurationMethod( pch, WIDE("%m button up=%m"), SetRoundUp );
	AddConfigurationMethod( pch, WIDE("%m button down=%m"), SetRoundDown );
	AddConfigurationMethod( pch, WIDE("%m button mask=%m"), SetRoundMask );
	// BeginCanvasConfiguration( g.single_frame );
	AddConfigurationMethod( pch, "Canvas Done", ResetMainCanvasConfig );
	SetConfigurationUnhandled( pch, UnhandledLine );
	AddCommonCanvasConfig( pch );

	InvokeLoadCommon();

	SetConfigurationEndProc( pch, ProcessLast );
	if( !g.flags.local_config  // local only config, don't do SQL
		&& !g.flags.forceload ) // force load, don't read from config..
	{
		_32 buflen;
		char *buffer;
		//lprintf( WIDE("long wait...") );
		if( g.flags.bSQLConfig )
		{
			BannerNoWait( WIDE("Read SQL Config...") );
#ifndef __NO_SQL__
#ifndef __ARM__
			if( SACK_GetProfileBlobOdbc( NULL, WIDE("intershell/configuration"), filename, &buffer, &buflen ) )
			{
				FILE *out = fopen( filename, WIDE("wb") );
				if( out )
				{
					fwrite( buffer, buflen, 1, out );
					fclose( out );
				}
				Release( buffer );
			}
#endif
#endif
		}
		//lprintf( WIDE("long wait...") );
	}
	ProcessConfigurationFile( pch, filename, 0 );
	DestroyConfigurationEvaluator( pch );
	if( g.flags.bSQLConfig )
	{
		if( g.flags.forceload )
		{
			FILE *file;
			_32 size = 0;
        		 _32 real_file_size = 0;
			POINTER mem = OpenSpace( NULL, filename, &size );
			if( mem && size )
			{
				file = fopen( filename, "rb" );
				fseek( file, 0, SEEK_END );
				real_file_size = ftell( file );
				fclose( file );
				if( g.flags.restoreload )
				{
					TEXTSTR ext;
					// the name portion of this will have already been
					// validated, so here, just blindly slice and dice the filename
#ifdef __cplusplus
#ifndef __cplusplus_cli
					/* what the FUCK?! CTEXTSTR is NOT a const...
					*/

					// with this, there are overloaded equivalent functions
					// which make typecast ierrelvant.  THe input is a char *
					// which is a wriatable string for sure...
					// The C Code results with const char * caues ti acceps a const char *
					// cause it can, since the function does not modify the content...
					// but then the rsult type and the passed type must be the same....
					ext =
#ifndef __cplusplus
						(TEXTSTR)
#endif
						StrRChr( filename, '.' );
					ext[0] = 0;
					ext = pathrchr( filename );
#endif
#else
					// see comments in the above cplusplus section....
					ext = (char*)StrRChr( (const char *)filename, '.' );
					ext[0] = 0;
					ext = (char*)pathrchr( (const char *)filename );
#endif
					if( ext )
					{
						//char *newname = StrDup( ext + 1 ); // derr - need to copy this before releasing it....
						/* if there was a pathpart on it for some reason, get rid of it
						*/
						//Release( g.config_filename );
						//g.config_filename = newname;
					}
				}
#ifndef __NO_SQL__
#ifndef __ARM__
				SACK_WriteProfileBlobOdbc( NULL, TASK_PREFIX "/configuration", filename, (char*)mem, real_file_size );
#endif
#endif
				g.flags.forceload = 0;
				CloseSpace( mem );
			}
		}
	}
	else
	{
#ifndef __ARM__
		if( g.flags.restoreload )
		{
			_32 size = 0;
			POINTER mem = OpenSpace( NULL, filename, &size );
			FILE *file;
			_32 real_file_size = 0;
			if( mem && size )
			{
				file = fopen( filename, "rb" );
				fseek( file, 0, SEEK_END );
				real_file_size = ftell( file );
				fclose( file );
				if( g.flags.restoreload )
				{
					TEXTSTR ext;
					ext =
#ifndef __cplusplus
						(TEXTSTR)
#endif
						StrRChr( filename, '.' );
					ext[0] = 0;
				}
				file = fopen( filename, "wb" );
				fwrite( mem, 1, real_file_size, file );
				fclose( file );
			}
		}
#endif
	}
	// should be the last link...
	BannerNoWait( WIDE("Config Done...") );
   // and just in case we had no defaults....
	SetDefaultRowsCols();
	// need to pop this last - set default rows/cols needs canvas
	// ShellSetCurrentPage( "first" );  // this is done when 'restart()' works
	PopLink( &l.current_canvas );
}

//---------------------------------------------------------------------------

CTEXTSTR EscapeMenuString( CTEXTSTR string )
{
	if( string )
	{
		static char *escaped;
		static int len;
		int pos = 0;
		int this_len = strlen( string );
		if( this_len == 0 )
			return NULL;
		//lprintf( WIDE("Escapeing string [%s]"), string );
		if( !escaped || ( ( this_len * 2 ) > len ) )
		{
			if( escaped )
				Release( escaped );
			escaped = NewArray( char, len = this_len * 2 );
		}
		while( string[0] )
		{
			if( string[0] == '\\' )
			{
				escaped[pos++] = '\\';
				escaped[pos++] = '\\';
			}
			else if( string[0] == '#' )
			{
				escaped[pos++] = '\\';
				escaped[pos++] = '#';
			}
			else
				escaped[pos++] = string[0];
			string++;
		}
		escaped[pos] = string[0];
		return escaped;
	}
   return NULL;
}

//---------------------------------------------------------------------------

void DumpCommonButton( FILE *file, PMENU_BUTTON button )
{
		if( button && !button->flags.bNoCreateMethod )
		{
			if( button->color )
				fprintf( file, WIDE("color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->color )
						 , RedVal( button->color )
						 , GreenVal( button->color )
						 , BlueVal( button->color )
						 );
			if( button->secondary_color )
				fprintf( file, WIDE("secondary color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->secondary_color )
						 , RedVal( button->secondary_color )
						 , GreenVal( button->secondary_color )
						 , BlueVal( button->secondary_color )
						 );
			if( button->highlight_color )
				fprintf( file, WIDE("highlight color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->highlight_color )
						 , RedVal( button->highlight_color )
						 , GreenVal( button->highlight_color )
						 , BlueVal( button->highlight_color )
						 );
			if( button->textcolor )
				fprintf( file, WIDE("text color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->textcolor )
						 , RedVal( button->textcolor )
						 , GreenVal( button->textcolor )
						 , BlueVal( button->textcolor )
						 );
			if( button->text )
				fprintf( file, WIDE("text=%s\n"), button->text );
			fprintf( file, WIDE("button is %s\n"), EscapeMenuString( button->glare_set->name ) );
			if( button->flags.bNoPress )
				fprintf( file, WIDE("button is unpressable\n") );
			if( button->pImage[0] )
			{
				fprintf( file, WIDE("image=%s\n"), EscapeMenuString( button->pImage ) );
            fprintf( file, WIDE("image_margin=%d,%d\n" ), button->decal_horiz_margin, button->decal_vert_margin );
			}
			if( button->pPageName )
				fprintf( file, WIDE("next page=%s\n"), button->pPageName );
			if( button->font_preset_name )
				fprintf( file, WIDE("font name=%s\n"), button->font_preset_name );
		}
}

//---------------------------------------------------------------------------

void XML_DumpCommonButton( genxWriter w, PMENU_BUTTON button )
{
	static char buffer[4096];
	int offset = 0;
   // this is just a quick hack, it's a lot of typing still to port this...
	MakeAttr( w, generic_xx, "generic" );
		if( button )
		{
			if( button->color )
				offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->color )
						 , RedVal( button->color )
						 , GreenVal( button->color )
						 , BlueVal( button->color )
						 );
			if( button->secondary_color )
				offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("secondary color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->secondary_color )
						 , RedVal( button->secondary_color )
						 , GreenVal( button->secondary_color )
						 , BlueVal( button->secondary_color )
						 );
			if( button->highlight_color )
				offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("highlight color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->highlight_color )
						 , RedVal( button->highlight_color )
						 , GreenVal( button->highlight_color )
						 , BlueVal( button->highlight_color )
						 );
			if( button->textcolor )
				offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("text color=$%02lX%02lX%02lX%02lX\n")
						 , AlphaVal( button->textcolor )
						 , RedVal( button->textcolor )
						 , GreenVal( button->textcolor )
						 , BlueVal( button->textcolor )
						 );
         if( button->text )
				offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("text=%s\n"), button->text );
			offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("button is %s\n"), button->glare_set->name );
			if( button->flags.bNoPress )
            offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("button is unpressable\n") );
			if( button->pImage && button->pImage[0] )
				offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("image=%s\n"), EscapeMenuString( button->pImage ) );
         if( button->pPageName )
				offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("next page=%s\n"), button->pPageName );
			if( button->font_preset_name )
            offset += snprintf( buffer + offset, sizeof( buffer ) - offset, WIDE("font name=%s\n"), button->font_preset_name );
		}
		AddAttr( generic_xx, "%s", buffer );
}

//-----------------------------------------------------------------

static PMENU_BUTTON saving;

void InterShell_SaveCommonButtonParameters( FILE *file )
{
   DumpCommonButton( file, saving );
}

//-----------------------------------------------------------------

void DumpGeneric( FILE *file, PMENU_BUTTON button )
{
#define GetRegisteredProcedure2(nc,rtype,name,args) (rtype (CPROC*)args)GetRegisteredProcedureEx((nc),#rtype, name, #args )
   void (CPROC*f)(FILE*,PMENU_BUTTON,PTRSZVAL);
   void (CPROC*f2)(FILE*,PTRSZVAL);
	char rootname[256];
	saving = button;
	if( button->pTypeName )
	{
		if( button->flags.bNoCreateMethod ) // control failed create... log this, otherwise the control will have logged its own stuf
		{
			INDEX idx;
			CTEXTSTR line;
			LIST_FORALL( button->extra_config, idx, CTEXTSTR, line )
			{
				fprintf( file, WIDE( "%s\n" ), EscapeMenuString( line ) );
			}
		}
		if( button->flags.bListbox )
		{
			int multi;
			int lazy;
			GetListboxMultiSelectEx( button->control.control, &multi, &lazy );
			fprintf( file, "multi select?%s lazy?%s\n"
				, multi?"yes":"no" 
					 , multi?(lazy?"yes":"no"):"no"
				);
		}
		if( !button->flags.bCustom )
		{
			snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
			f = GetRegisteredProcedure2( rootname, void, WIDE("button_save"), (FILE*,PMENU_BUTTON,PTRSZVAL) );
			if( f )
			{
				f( file, button,button->psvUser );
			}
			{
				// add additional security plugin stuff...
				CTEXTSTR name;
				PCLASSROOT data = NULL;
				for( name = GetFirstRegisteredName( TASK_PREFIX "/common/Save Security", &data );
					 name;
					  name = GetNextRegisteredName( &data ) )
				{
					void (CPROC*f)(FILE*,PMENU_BUTTON);
					//snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/common/save common/%s", name );
					f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (FILE*,PMENU_BUTTON) );
					if( f )
						f( file, button );
				}
			}
		}
		snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
		f2 = GetRegisteredProcedure2( rootname, void, WIDE("control_save"), (FILE*,PTRSZVAL) );
		if( f2 )
		{
			//DumpCommonButton( file, button );
 			f2( file, button->psvUser );
		}
		if( !button->flags.bCustom && !button->flags.bNoCreateMethod )
			DumpCommonButton( file, button );
		if( button->show_on )
		{
				INDEX idx;
				PTEXT name;
				LIST_FORALL( button->show_on, idx, PTEXT, name )
				{
					fprintf( file, WIDE("Allow show on %s\n" ), GetText(name) );
				}
		}
		if( button->no_show_on )
		{
			INDEX idx;
			PTEXT name;
			LIST_FORALL( button->no_show_on, idx, PTEXT, name )
			{
				fprintf( file, WIDE("Disallow show on %s\n" ), GetText(name) );
			}
		}
	}
}

void XML_DumpGeneric( genxWriter w, PMENU_BUTTON button )
{
#define GetRegisteredProcedure2(nc,rtype,name,args) (rtype (CPROC*)args)GetRegisteredProcedureEx((nc),#rtype, name, #args )
   //void (CPROC*f)(genxWriter,PMENU_BUTTON,PTRSZVAL);
   void (CPROC*f2)(genxWriter,PTRSZVAL);
	char rootname[256];
   MakeElem( w, generic_dump_region, (constUtf8)"control" );
	MakeAttr( w, location, (constUtf8)"position" );
   MakeAttr( w, size, (constUtf8)"size" );
	MakeAttr( w, control_type, (constUtf8)"type" );

	saving = button;

	genxStartElement( generic_dump_region );

   AddAttr( location, "%lld,%lld", button->x, button->y );
   AddAttr( size, "%lld,%lld", button->x, button->y );
   AddAttr( control_type, "%s", button->pTypeName );

	if( button->pTypeName )
	{
		if( button->flags.bListbox )
		{
			int multi;
			int lazy;
         MakeAttr( w, attr_multiselect, (constUtf8)"multi-select" );
			MakeAttr( w, attr_lazy, (constUtf8)"lazy" );

			GetListboxMultiSelectEx( button->control.control, &multi, &lazy );

			AddAttr( attr_multiselect, "%s", multi?"yes":"no" );
			AddAttr( attr_lazy, "%s", lazy?"yes":"no" );
		}
		snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
		f2 = GetRegisteredProcedure2( rootname, void, WIDE("control_save_xml"), (genxWriter,PTRSZVAL) );
		if( f2 )
		{
			//DumpCommonButton( w, button );
 			f2( w, button->psvUser );
		}
		if( !button->flags.bCustom )
		{
			XML_DumpCommonButton( w, button );
		}
		{
         MakeAttr( w, show, (constUtf8)"show" );
         MakeAttr( w, hide, (constUtf8)"hide" );
			if( button->show_on )
			{
				INDEX idx;
				PTEXT name;
				LIST_FORALL( button->show_on, idx, PTEXT, name )
				{
               AddAttr( show, "%s", GetText( name ) );
               //fprintf( w, WIDE("Allow show on %s\n" ), GetText(name) );
				}
			}
			if( button->no_show_on )
			{
				INDEX idx;
				PTEXT name;
				LIST_FORALL( button->no_show_on, idx, PTEXT, name )
				{
               AddAttr( hide, "%s", GetText( name ) );
               //fprintf( w, WIDE("Disallow show on %s\n" ), GetText(name) );
				}
			}
		}
	}
	genxEndElement( w );
}


void InvokeSavePage( FILE *file, PPAGE_DATA page )
{
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	for( name = GetFirstRegisteredName( TASK_PREFIX "/common/save page", &data );
		  name;
		  name = GetNextRegisteredName( &data ) )
	{
		void (CPROC*f)(FILE *file, PPAGE_DATA);
		f = GetRegisteredProcedure2( data, void, name, (FILE *, PPAGE_DATA) );
		if( f )
			f( file, page );
	}
	//if( name )
	//	return FALSE;
   //return TRUE;
}

void XML_InvokeSavePage( genxWriter w, PPAGE_DATA page )
{
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	for( name = GetFirstRegisteredName( TASK_PREFIX "/common/xml save page", &data );
		  name;
		  name = GetNextRegisteredName( &data ) )
	{
		void (CPROC*f)(genxWriter w, PPAGE_DATA);
		f = GetRegisteredProcedure2( data, void, name, (genxWriter, PPAGE_DATA) );
		if( f )
			f( w, page );
	}
	//if( name )
	//	return FALSE;
   //return TRUE;
}


void SaveAPage( FILE *file, PPAGE_DATA page )
{
	PMENU_BUTTON button;
	INDEX idx;
   if( page->title )
		fprintf( file, WIDE("page titled %s\n"), page->title );
	fprintf( file, WIDE("page layout %d by %d\n"), page->grid.nPartsX, page->grid.nPartsY );
	fprintf( file, WIDE("%sbackground color $%lX\n")
			 , page->background_color?"":"#"
			 , page->background_color
			 );
	fprintf( file, WIDE("%sbackground image %s\n")
			 , page->background?page->background[0]?"":"#":"#"
			 , page->background?EscapeMenuString( (TEXTSTR)page->background ):"" );
   InvokeSavePage( file, page );
	LIST_FORALL( page->controls, idx, PMENU_BUTTON, button )
	{
		fprintf( file
#ifdef _MSC_VER
				 , WIDE("control generic %s at %lld,%lld sized %lld,%lld\n")
#else
				 , WIDE("control generic %s at %Ld,%Ld sized %Ld,%Ld\n")
#endif
				 , button->pTypeName
				 , button->x, button->y
				 , button->w, button->h );
		fflush( file );
		DumpGeneric( file, button );
		fprintf( file, WIDE("control done\n\n") );
		fflush( file );
	}
}

void XML_SaveAPage( genxWriter w, PPAGE_DATA page )
{
	PMENU_BUTTON button;
	INDEX idx;
	//genxStatus status;
   //char tmp[256];
   MakeElem( w, page_element, (constUtf8)"page" );
   //MakeAttr( w, aspect, "aspect" );
   //MakeAttr( w, layout, "layout" );
   MakeAttr( w, title, (constUtf8)"title" );
   MakeAttr( w, grid, (constUtf8)"grid" );

	MakeElem( w, eBackground, (constUtf8)"background" );

	MakeAttr( w, back_image, (constUtf8)"image" );
	MakeAttr( w, back_color, (constUtf8)"color" );

   //MakeAttr( w, editable, "editable" );
   //MakeAttr( w, allow_multiple, "allow multiple" );

	genxStartElement( page_element );
	//AddAttr( aspect, "%d,%d", page->grid.aspect.numerator, page->grid.aspect.denominator );
	AddAttr( grid, "%d,%d", page->grid.nPartsX, page->grid.nPartsY );
	if( page->title )
   		AddAttr( title, "%s", page->title );
	//AddAttr( layout, "%d,%d,%d,%d", canvas->nPartsX/8, canvas->nPartsY/8 );
	//AddAttr( editable, "%s",  );

	genxStartElement( eBackground );
	if( page->background_color )
		AddAttr( back_color, "$%lX", page->background_color );
	if( page->background )
		AddAttr( back_image, "%s", page->background );
	genxEndElement( w );

	XML_InvokeSavePage( w, page );

	LIST_FORALL( page->controls, idx, PMENU_BUTTON, button )
	{
      /* includes start and stop element for each button... */
		XML_DumpGeneric( w, button );
	}
	genxEndElement( w );
}

void SaveSQLButtonConfig( void )
{
	// need to define a table structure to do this to...
	// but it should be easy enough to craft right here a way to save
	// the configuration in a common place that may be referenced
	// by all systems running this program.

   // the method to store information should support undo capability?
}

void RenameConfig( char *config_filename, char *source, int source_name_len, int n )
{
	FILE *file;
	file = fopen( source, WIDE("rt") );
	if( file )
	{
      char backup[256];
		fclose( file );
		// move file to backup..
		snprintf( backup, sizeof( backup ), "%*.*s.AutoConfigBackup%d"
				  , source_name_len
				  , source_name_len
				  , config_filename, n );
		if( n < 10 )
		{
			RenameConfig( config_filename, backup
							, source_name_len
							, n+1 );
		}
		else
			unlink( source );
		rename( source, backup );
	}
}


static genxStatus WriteBuffer( void *UserData, constUtf8 s )
{
	vtprintf( (PVARTEXT)UserData, WIDE("%s"), s );
   return GENX_SUCCESS;
}

static genxStatus WriteBufferBounded( void *UserData, constUtf8 s, constUtf8 end )
{
   vtprintf( (PVARTEXT)UserData, WIDE("%*.*s"), end-s, end-s, s );
   return GENX_SUCCESS;
}

static genxStatus Flush( void *UserData )
{
   return GENX_SUCCESS;
}

static genxSender senderprocs = { WriteBuffer
								 , WriteBufferBounded
								 , Flush };

void SaveCanvasConfiguration( FILE *file, PSI_CONTROL pc_canvas )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc_canvas );
	if( canvas )
	{
      /* now saved on a per-page basis... parts are page specific */
		//fprintf( file, WIDE("page layout %d by %d\n"), canvas->nPartsX/8, canvas->nPartsY/8 );
		{
			INDEX pageidx;
			PPAGE_DATA page;
			SaveAPage( file, canvas->default_page );
			LIST_FORALL( canvas->pages, pageidx, PPAGE_DATA, page )
			{
				SaveAPage( file, page );
			}
		}
		fprintf( file, "Canvas Done\n" );
	}
}

void SaveCanvasConfiguration_XML( genxWriter w, PSI_CONTROL pc_canvas )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc_canvas );
   MakeElem( w, canvas_element, (constUtf8)"canvas" );
	if( canvas )
	{
		genxStartElement( canvas_element );
		{
			INDEX pageidx;
			PPAGE_DATA page;
			XML_SaveAPage( w, canvas->default_page );
			LIST_FORALL( canvas->pages, pageidx, PPAGE_DATA, page )
			{
				XML_SaveAPage( w, page );
			}
		}
		genxEndElement( w );
		//fprintf( file, "Canvas Done\n" );
	}
}


void SaveButtonConfig( PSI_CONTROL pc_canvas, char *filename )
{
	FILE *file;
   //ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc_canvas );

   // -- additional code for XML output...
	PVARTEXT pvt;
	genxWriter w = genxNew(NULL,NULL,NULL);
	genxStartDocSender( w, &senderprocs );
	pvt = VarTextCreate();
	genxSetUserData( w, pvt );
	// ----------------------------------


	RenameConfig( filename, filename, strlen( filename ), 1 );

	file = fopen( filename, WIDE("wt") );
	if( file )
	{
      //fprintf( file, WIDE("[config]\n") ); // make this look like an INI so some standard tools work.
		//fprintf( file, WIDE("\n\n") );

#define fn(n) ((n)?EscapeMenuString(n):"")
		{
			INDEX idx;
			PGLARE_SET glare_set;
			LIST_FORALL( g.glare_sets, idx, PGLARE_SET, glare_set )
			{
				CTEXTSTR setname = StrDup( EscapeMenuString( glare_set->name ) );
				if( glare_set->flags.bShadeBackground )
					fprintf( file, WIDE( "%s button mono shade\n" ), setname );
				if( glare_set->flags.bMultiShadeBackground )
					fprintf( file, WIDE( "%s button multi shade\n" ), setname );
				fprintf( file, WIDE("%s button glare=%s\n"), setname, fn(glare_set->glare) );
				fprintf( file, WIDE("%s button up=%s\n"), setname, fn(glare_set->up) );
				fprintf( file, WIDE("%s button down=%s\n"), setname, fn(glare_set->down) );
				fprintf( file, WIDE("%s button mask=%s\n"), setname, fn(glare_set->mask) );
				fprintf( file, WIDE("\n") );
			}
			fprintf( file, WIDE("\n") );
		}
		{
			CTEXTSTR name;
			PCLASSROOT data = NULL;
			for( name = GetFirstRegisteredName( TASK_PREFIX "/common/save common", &data );
				 name;
				  name = GetNextRegisteredName( &data ) )
			{
				void (CPROC*f)(FILE*);
				f = GetRegisteredProcedure2( data, void, name, (FILE *) );
				if( f )
					f( file );
			}
		}

		{
			CTEXTSTR line;
         INDEX idx;
			LIST_FORALL( l.unhandled_global_lines, idx, CTEXTSTR, line )
			{
            fprintf( file, WIDE("%s\n" ), EscapeMenuString( line ) );
			}
		}


		// these are buried somwhere in the middle
		// so the casual browser doesn't observe these flags
		// someday this config file should be encrypted or something
      // since it is meant to be machine readable only.
		if( g.flags.bNoEditSet )
			fprintf( file, "Allow Edit? %s\n", g.flags.bNoEdit?"No":"Yes" );
		if( g.flags.bAllowMultiSet )
			fprintf( file, "Allow Multi Run? %s\n", g.flags.bAllowMultiLaunch?"Yes":"No" );


		// -- additional code for XML output...
		SaveCanvasConfiguration_XML( w, pc_canvas );
		genxEndDocument( w );
		{
			PTEXT text = VarTextGet( pvt );
			fprintf( file, "# Begin XML Expirament\n# " );
			//fwrite( GetText( text ), sizeof( TEXTCHAR ), GetTextSize( text ), file );
         fprintf( file, "\n\n" );
			LineRelease( text );
		}
		VarTextDestroy( &pvt );
		genxDispose( w );
      // ----------------------------------

		SaveCanvasConfiguration( file, pc_canvas );

		fclose( file );
	}
#ifndef __ARM__
	if( g.flags.bSQLConfig )
		if( !g.flags.local_config )
		{
			_32 size = 0;
			POINTER mem = OpenSpace( NULL, filename, &size );
			if( mem && size )
			{
#ifndef __NO_SQL__
				SACK_WriteProfileBlobOdbc( NULL, TASK_PREFIX "/configuration", filename, (char*)mem, size );
#endif
				g.flags.forceload = 0;
				CloseSpace( mem );
			}
		}
#endif
}

