
// InterShell Modular Interface Layout Konsole
//

// Evo me&nu evolved from Alt Menu
// Evo menu adds supports for resizable/editable buttons, based on a low resolution grid
//   where objects therein occupy a concave set of cells.
//   Objects may not overlap, and when moving, will not move
//   from their prior position until the new position is valid.
//   This then leads us to those spiffy sliding tile puzzles....

//

//#define DEBUG_BACKGROUND_UPDATE

#define LOG_UPDATE_AND_REFRESH_LEVEL LOG_ALWAYS
//#define LOG_UPDATE_AND_REFRESH_LEVEL LOG_NEVER
//#define USE_EDIT_GLARE

#include <stdhdrs.h>
#include <deadstart.h>
#include <system.h>
#include <filesys.h>
#include <network.h>
#include <idle.h>
#include <pssql.h>
#include <sqlgetoption.h>
#define DECLARE_GLOBAL
#include "intershell_local.h"
#include "resource.h"
#include "loadsave.h"
//#define DEBUG_SCALED_BLOT
#include "widgets/include/banner.h"
#include <psi.h>
#include "pages.h"
#include "intershell_security.h"
#include "intershell_registry.h"
#include "menu_real_button.h"
#include "sprites.h"
#ifndef __NO_ANIMATION__
#include "animation.h"
#include "animation_plugin.h"
#endif
#include "fonts.h"
#ifdef __WINDOWS__
#include <vidlib/vidstruc.h>
#endif

// this structure maintains the current edit button
// what it is, and temp data for configuing it via
// Set/GetCommonButtonControls()



static void (CPROC*EditControlBehaviors)( PSI_CONTROL pc );

enum {
	MENU_CONTROL_BUTTON
	, MENU_CONTROL_CLOCK
};

extern CONTROL_REGISTRATION menu_surface;

// I hate this! but I don't have time to unravel the
// chicken from the egg.
void CPROC AbortConfigureKeys( PSI_CONTROL pc, _32 keycode );


PRELOAD( blah )
{
#undef SYMNAME
#define SYMNAME( a,b) EasyRegisterResource( "intershell/visibility", a, b );
		SYMNAME( LIST_ALLOW_SHOW, LISTBOX_CONTROL_NAME )
		SYMNAME( LIST_DISALLOW_SHOW, LISTBOX_CONTROL_NAME )
		SYMNAME( LIST_SYSTEMS, LISTBOX_CONTROL_NAME ) // known systems list
		SYMNAME( EDIT_SYSTEM_NAME, EDIT_FIELD_NAME )
		SYMNAME( BTN_ADD_SYSTEM, NORMAL_BUTTON_NAME )
		SYMNAME( BTN_ADD_SYSTEM_TO_DISALLOW, NORMAL_BUTTON_NAME )
		SYMNAME( BTN_ADD_SYSTEM_TO_ALLOW, NORMAL_BUTTON_NAME )
		SYMNAME( BTN_REMOVE_SYSTEM_FROM_DISALLOW, NORMAL_BUTTON_NAME )
		SYMNAME( BTN_REMOVE_SYSTEM_FROM_ALLOW, NORMAL_BUTTON_NAME )
#undef SYMNAME
#define SYMNAME( a,b) EasyRegisterResource( "intershell/page property", a, b );
		SYMNAME( EDIT_PAGE_GRID_PARTS_X, EDIT_FIELD_NAME )
		SYMNAME( EDIT_PAGE_GRID_PARTS_Y, EDIT_FIELD_NAME )
#undef SYMNAME
#define SYMNAME( a,b) EasyRegisterResource( "intershell/Button General/security", a, b );
		SYMNAME( LISTBOX_SECURITY_MODULE, LISTBOX_CONTROL_NAME );
		SYMNAME( EDIT_SECURITY, NORMAL_BUTTON_NAME );

}


struct image_tag{
	CTEXTSTR name;
	Image image;
	int references;
};
static PLIST images;

void InterShell_Hide( void )
{
	// find the current page, and hide it...
	//RemoveBannerEx( NULL DBG_SRC );
	InterShell_DisablePageUpdate( TRUE );
	HideCommon( g.single_frame );
}
void InterShell_Reveal( void )
{
	// check to see if a button is processing still before
	// revealing the surface... might be hiding again (in the case of macro)
	//
	if( g.flags.bButtonProcessing )
	{
		lprintf( "Canvas processing, don't show yet..." );
		g.flags.bShowCanvas = 1;
		return;
	}
	//lprintf( "Restoring..." );
	//if( g.flags.bShowCanvas )
	{
		InterShell_DisablePageUpdate( FALSE );
		RevealCommon( g.single_frame );
		g.flags.bShowCanvas = 0;
	}
}

Image InterShell_CommonImageLoad( CTEXTSTR name )
{
	struct image_tag *image;
	INDEX idx;
	LIST_FORALL( images, idx, struct image_tag *, image )
	{
		if( strcmp( image->name, name ) == 0 )
		{
			break;
		}
	}
	if( !image )
	{
		image = New( struct image_tag );
		image->name = StrDup( name );
		image->image = LoadImageFile( image->name );
		image->references = 1;
	}
	else
		image->references++;
	return image->image;
}

void InterShell_CommonImageUnloadByName( CTEXTSTR name )
{
	INDEX idx;
	struct image_tag *image;
	LIST_FORALL( images, idx, struct image_tag *, image )
	{
		if( strcmp( image->name, name ) == 0 )
		{
			break;
		}
	}
	if( image )
	{
		image->references--;
		if( !image->references )
		{
			UnmakeImageFile( image->image );
			Release( (POINTER)image->name );
			Release( image );
			SetLink( &images, idx, NULL );
		}
	}
}

void InterShell_CommonImageUnloadByImage( Image unload )
{
	INDEX idx;
	struct image_tag *image;
	LIST_FORALL( images, idx, struct image_tag *, image )
	{
		if( image->image == unload )
		{
			break;
		}
	}
	if( image )
	{
		image->references--;
		if( !image->references )
		{
			UnmakeImageFile( image->image );
			Release( (POINTER)image->name );
			Release( image );
			SetLink( &images, idx, NULL );
		}
	}
}

//---------------------------------------------------------------------------

static PMENU_BUTTON MouseInControl( PCanvasData canvas, int x, int y )
{
	INDEX idx;
	PMENU_BUTTON pmc;
	PPAGE_DATA current_page;
	current_page = canvas->current_page;
	if( current_page )
	{
		LIST_FORALL( current_page->controls, idx, PMENU_BUTTON, pmc )
		{
			//lprintf( WIDE("stuff... %d<=%d %d>%d %d<=%d %d>%d")
			//		 , x, pmc->x
			//		 , x, pmc->x + pmc->w
			//		 , y, pmc->y
			//		 , y, pmc->y + pmc->h );
			if( ( ( (x) >= pmc->x )
				&& ( x < ( pmc->x + pmc->w ) )
				&& ( (y) >= pmc->y )
				&& ( y < ( pmc->y + pmc->h ) )
				) )
			{
				//lprintf( WIDE("success - intersect, result FALSE ") );
				return pmc;
			}
		}
	}
	return NULL;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static LOGICAL IsControlSelected( PCanvasData canvas, PMENU_BUTTON pInclude )
{
	INDEX idx;
	PMENU_BUTTON pmc;
	LIST_FORALL( canvas->selected_list, idx, PMENU_BUTTON, pmc )
	{
		if( pmc == pInclude)
			return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------

static int SelectItems( PCanvasData canvas, PMENU_BUTTON pExclude, int x, int y, int w, int h )
{

	INDEX idx;
	int count = 0;
	PMENU_BUTTON pmc;
	PPAGE_DATA current_page;
	current_page = canvas->current_page;
	EmptyList( &canvas->selected_list );
	LIST_FORALL( current_page->controls, idx, PMENU_BUTTON, pmc )
	{
		if( pmc == pExclude )
			continue;
		//lprintf( WIDE("stuff... %d<=%d %d>%d %d<=%d %d>%d")
		//		 , x+w, pmc->x
		//		 , x, pmc->x + w
		//		 , y+h, pmc->y
		//		 , y, pmc->y + h );



		if( !( ( ((x)+w)   <= pmc->x )
			|| ( (x)     >= ( pmc->x + pmc->w ) )
			|| ( ((y)+h) <= pmc->y )
			|| ( (y)     >= ( pmc->y + pmc->h ) )
			) )
		{
			AddLink( &canvas->selected_list, pmc );
         count++;
		}
	}
   return count;
}


static LOGICAL IsSelectionValidEx( PCanvasData canvas, PMENU_BUTTON pExclude, int x, int y, int *dx, int *dy, int w, int h )
{
	INDEX idx;
	int zero = 0;
	PMENU_BUTTON pmc;
	PPAGE_DATA current_page;
	current_page = canvas->current_page;
	if( !dx )
		dx = &zero;
	if( !dy )
		dy = &zero;
	LIST_FORALL( current_page->controls, idx, PMENU_BUTTON, pmc )
	{
		if( pmc == pExclude )
			continue;
		//lprintf( WIDE("stuff... %d<=%d %d>%d %d<=%d %d>%d")
		//		 , x+w, pmc->x
		//		 , x, pmc->x + w
		//		 , y+h, pmc->y
		//		 , y, pmc->y + h );



		if( !( ( ((x+(*dx))+w)   <= pmc->x )
			|| ( (x+(*dx))     >= ( pmc->x + pmc->w ) )
			|| ( ((y+(*dy))+h) <= pmc->y )
			|| ( (y+(*dy))     >= ( pmc->y + pmc->h ) )
			) )
		{
			// if the Y delta causes an overlap...
			if( !( ( ((x)+w)   <= pmc->x )
				|| ( (x)     >= ( pmc->x + pmc->w ) )
				|| ( ((y+(*dy))+h) <= pmc->y )
				|| ( (y+(*dy))     >= ( pmc->y + pmc->h ) )
				) )
			{
				// maybe if the X delta is okay use it?
				if( !( ( ((x+(*dx))+w)   <= pmc->x )
					|| ( (x+(*dx))     >= ( pmc->x + pmc->w ) )
					|| ( ((y)+h) <= pmc->y )
					|| ( (y)     >= ( pmc->y + pmc->h ) )
					) )
				{
					// neither the X or Y delta can be corrected to allivate the issue?
					return FALSE;
				}
				else
				{
					// the x delta alone is okay, clear the Y
					(*dy) = 0;
				}
			}
			else
			{
				// otherwise, the Y delta alone is okay, so clear the X
				(*dx) = 0;
			}

			//lprintf( WIDE("Failure - intersect, result FALSE ") );
			return TRUE;
		}
	}
	return TRUE;
}

PMENU_BUTTON CreateButton( void )
{
	PMENU_BUTTON button = (PMENU_BUTTON)Allocate( sizeof( MENU_BUTTON ) );
	MemSet( button, 0, sizeof( MENU_BUTTON ) );
	// applicaitons end up setting this color...
	//button->page = ShellGetCurrentPage();
	button->color = BASE_COLOR_BLUE;
	button->textcolor = BASE_COLOR_WHITE;
	button->glare_set = GetGlareSet( WIDE("DEFAULT") );
	//button->flags.bSquare = TRUE; // good enough default
	return button;
}

LOGICAL InterShell_IsButtonVirtual( PMENU_BUTTON button )
{
	if( button )
	{
		lprintf( "Button is virtual? %d", button->flags.bInvisible );
		return button->flags.bInvisible;
	}
	return 1;
}


void InvokeDestroy( PMENU_BUTTON button )
{
	char rootname[256];
	void (CPROC*f)(PTRSZVAL);
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, void, WIDE("button_destroy"), (PTRSZVAL) );
	if( f )
		f(button->psvUser);
}

void InvokeShowControl( PMENU_BUTTON button )
{
	char rootname[256];
	void (CPROC*f)(PTRSZVAL);
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, void, WIDE("show_control"), (PTRSZVAL) );
	if( f )
		f(button->psvUser);
}

void InvokeInterShellShutdown( void )
{
   static int bInvoked;
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	if( bInvoked )
		return;
   bInvoked = TRUE;

	InvokeShutdownMacro();

	for( name = GetFirstRegisteredName( TASK_PREFIX "/common/intershell shutdown", &data );
		name;
		name = GetNextRegisteredName( &data ) )
	{
		void (CPROC *f)(void);
		//snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/common/save common/%s", name );
		f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (void) );
		if( f )
			f();
	}
}

PCanvasData GetCanvas( PSI_CONTROL pc )
{
	if( !pc )
		pc = g.single_frame;
	{
		ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
		if( !canvas )
		{
			PSI_CONTROL parent;
			//lprintf( "Control %p is not a canvas, go to parent, check it..." );
			for( parent = GetCommonParent( pc ); parent; parent = GetCommonParent( parent ) )
			{
				ValidatedControlData( PCanvasData, menu_surface.TypeID, _canvas, parent );
				if( _canvas )
				{
					canvas = _canvas;
					break;
				}
				//lprintf( "Control %p is not a canvas, go to parent, check it..." );
			}
		}
		return canvas;
	}
	//return NULL;
}
void InvokeCopyControl( PMENU_BUTTON button )
{
	char rootname[256];
	void (CPROC*f)(PTRSZVAL);
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, void, "copy_control", (PTRSZVAL) );
	if( f )
		f( button->psvUser );
}

void InvokeCloneControl( PMENU_BUTTON button, PMENU_BUTTON original )
{
	char rootname[256];
	void (CPROC*f)(PTRSZVAL,PTRSZVAL);
	// may validate that button->pTypeName == original->pTypeName
	// due to invokation constraints this would be impossible, until
	// some other developer mangles stuff.
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, void, "clone_control", (PTRSZVAL,PTRSZVAL) );
	if( f )
		f( button->psvUser, original->psvUser );
}

void InvokePasteControl( PMENU_BUTTON button )
{
	char rootname[256];
	void (CPROC*f)(PTRSZVAL);
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, void, "paste_control", (PTRSZVAL));
	if( f )
		f( button->psvUser );
}

void DestroyButton( PMENU_BUTTON button )
{
	PCanvasData canvas = GetCanvas( GetCommonParent( QueryGetControl( button ) ) );
	if( !canvas )
		return;
	if( button == g.clonebutton )
		g.clonebutton = NULL;
	DeleteLink( &canvas->current_page->controls, button );

	InvokeDestroy( button );
	if( button->flags.bListbox )
	{
		DestroyCommon( &button->control.control );
		button->flags.bListbox = 0;
	}
	else if( !button->flags.bCustom )
		DestroyKey( &button->control.key );
}

PSI_CONTROL QueryGetControl( PMENU_BUTTON button )
{
	char rootname[256];
	PSI_CONTROL (CPROC*f)(PTRSZVAL);
	if( !button )
		return NULL;
	if( button->flags.bListbox )
		return button->control.control;
	else if( button->flags.bCustom )
	{
		snprintf( rootname
			, sizeof( rootname )
			, TASK_PREFIX "/control/%s"
			, button->pTypeName );
		f = GetRegisteredProcedure2( rootname, PSI_CONTROL, WIDE("get_control"), (PTRSZVAL) );
		if( f )
			return f(button->psvUser);
	}
	else
		return GetKeyCommon( button->control.key );
	return NULL;
}


int InvokeFixup( PMENU_BUTTON button )
{
	//if( button->flags.bCustom )
	{
		char rootname[256];
		void (CPROC*f2)(PTRSZVAL);
		PMENU_BUTTON prior = configure_key_dispatch.button;
		//PCanvasData prior_canvas = configure_key_dispatch.canvas;
		configure_key_dispatch.button = button;
		snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
		f2 = GetRegisteredProcedure2( rootname, void, WIDE("control_fixup"), (PTRSZVAL) );
		if( f2 )
		{
			//lprintf( WIDE("running fixup for %s"), rootname );
			f2(button->psvUser);
			configure_key_dispatch.button = prior;
			return TRUE;
		}
		configure_key_dispatch.button = prior;
		//else
		//	lprintf( WIDE("not running fixup for %s"), rootname );
	}
	return FALSE;
}

PGLARE_SET CreateGlareSet( CTEXTSTR name )
{
	PGLARE_SET glare_set = (PGLARE_SET)Allocate( sizeof( *glare_set ) );
	MemSet( glare_set, 0, sizeof( *glare_set ) );
	glare_set->name = StrDup( name );
	AddLink( &g.glare_sets, glare_set );
	return glare_set;
}

PGLARE_SET CheckGlareSet( CTEXTSTR name )
{
	PGLARE_SET glare_set;
	INDEX idx;
	LIST_FORALL( g.glare_sets, idx, PGLARE_SET, glare_set )
	{
		if( strcmp( name, glare_set->name ) == 0 )
			break;
	}
	return glare_set;
}
PGLARE_SET GetGlareSet( CTEXTSTR name )
{
	PGLARE_SET glare_set = CheckGlareSet( name );
	if( !glare_set )
		glare_set = CreateGlareSet( name );
	return glare_set;
}

void SetGlareSetFlags( char *name, int flags )
{
	PGLARE_SET glare_set = GetGlareSet( name );
	if( glare_set )
	{
		glare_set->flags.bMultiShadeBackground = 0;
		glare_set->flags.bShadeBackground = 0;

		if( flags & GLARE_FLAG_MULTISHADE )
			glare_set->flags.bMultiShadeBackground = 1;
		else if ( flags & GLARE_FLAG_SHADE )
			glare_set->flags.bShadeBackground = 1;
	}
}

void MakeGlareSet( char *name, char *glare, char *up, char *down, char *mask )
{
	PGLARE_SET glare_set = GetGlareSet( name );
#define SetGlareName(n)	if( glare_set->n ) \
	Release( glare_set->n );             \
	glare_set->n = n?StrDup( n ):(char*)NULL;
	SetGlareName( glare );
	SetGlareName( up );
	SetGlareName( down );
	SetGlareName( mask );
}


struct glare_set_edit{
	PGLARE_SET current;
};

PRELOAD( RegisterGlaresetResources )
{
	EasyRegisterResource( "intershell/glareset", MNU_EDIT_GLARES, WIDE("Popup Menu") );
	EasyRegisterResource( "intershell/glareset", LISTBOX_GLARE_SETS, LISTBOX_CONTROL_NAME );
	EasyRegisterResource( "intershell/glareset", EDIT_GLARESET_GLARE, EDIT_FIELD_NAME );
	EasyRegisterResource( "intershell/glareset", EDIT_GLARESET_UP, EDIT_FIELD_NAME );
	EasyRegisterResource( "intershell/glareset", EDIT_GLARESET_DOWN, EDIT_FIELD_NAME );
	EasyRegisterResource( "intershell/glareset", EDIT_GLARESET_MASK, EDIT_FIELD_NAME );
	EasyRegisterResource( "intershell/glareset", CHECKBOX_GLARESET_MULTISHADE, RADIO_BUTTON_NAME );
	EasyRegisterResource( "intershell/glareset", CHECKBOX_GLARESET_SHADE, RADIO_BUTTON_NAME );
	EasyRegisterResource( "intershell/glareset", CHECKBOX_GLARESET_FIXED, RADIO_BUTTON_NAME );
	EasyRegisterResource( "intershell/glareset", GLARESET_APPLY_CHANGES, NORMAL_BUTTON_NAME );
	EasyRegisterResource( "intershell/glareset", GLARESET_CREATE, NORMAL_BUTTON_NAME );
}

void CPROC OnGlareSetSelect( PTRSZVAL psv, PSI_CONTROL list, PLISTITEM pli )
{
	struct glare_set_edit *params = (struct glare_set_edit*)psv;
	PGLARE_SET glare_set = (PGLARE_SET)GetItemData( pli );
	params->current = glare_set;
	SetCommonText( GetNearControl( list, EDIT_GLARESET_GLARE ), glare_set->glare );
	SetCommonText( GetNearControl( list, EDIT_GLARESET_UP ), glare_set->up );
	SetCommonText( GetNearControl( list, EDIT_GLARESET_DOWN ), glare_set->down );
	SetCommonText( GetNearControl( list, EDIT_GLARESET_MASK ), glare_set->mask );
	SetCheckState( GetNearControl( list, CHECKBOX_GLARESET_MULTISHADE )
		, glare_set->flags.bMultiShadeBackground );
	SetCheckState( GetNearControl( list, CHECKBOX_GLARESET_SHADE )
		, glare_set->flags.bShadeBackground );
	SetCheckState( GetNearControl( list, CHECKBOX_GLARESET_FIXED )
		, !glare_set->flags.bMultiShadeBackground &&
		!glare_set->flags.bShadeBackground );
}

void CPROC ApplyGlareSetChanges( PTRSZVAL psv, PSI_CONTROL button )
{
	struct glare_set_edit *params = (struct glare_set_edit*)psv;
	PGLARE_SET glare_set = params->current;
	char buffer[256];
#define SetNewGlareImage( gs, var1, var2 )             \
	if( gs->var1 && stricmp( buffer, gs->var1 ) )      \
	{                                              \
	Release( gs->var1 );                \
	if( gs->var2 )                     \
	{                                           \
	UnmakeImageFile( gs->var2 );    \
	gs->var2 = NULL;                \
	}   }                                        \
	if( buffer[0] )                            \
	gs->var1 = StrDup( buffer );        \
	else                                \
	gs->var1 = NULL;

	if( !glare_set )return;

	GetControlText( GetNearControl( button, EDIT_GLARESET_GLARE ), buffer, sizeof( buffer ) );
	SetNewGlareImage( glare_set, glare, iGlare );
	GetControlText( GetNearControl( button, EDIT_GLARESET_MASK ), buffer, sizeof( buffer ) );
	SetNewGlareImage( glare_set, mask, iMask );
	GetControlText( GetNearControl( button, EDIT_GLARESET_UP ), buffer, sizeof( buffer ) );
	SetNewGlareImage( glare_set, up, iNormal );
	GetControlText( GetNearControl( button, EDIT_GLARESET_DOWN ), buffer, sizeof( buffer ) );
	SetNewGlareImage( glare_set, down, iPressed );
	glare_set->flags.bMultiShadeBackground = GetCheckState( GetNearControl( button, CHECKBOX_GLARESET_MULTISHADE ) );
	glare_set->flags.bShadeBackground = GetCheckState( GetNearControl( button, CHECKBOX_GLARESET_SHADE ) );
	/* if multi edit, should smudge all pages...*/
	//if( g.current_page )
	//  SmudgeCommon( g.current_page->frame );
}

void CPROC ButtonCreateGlareSet( PTRSZVAL psv, PSI_CONTROL button )
{
	char buffer[256];
	if( SimpleUserQuery( buffer, sizeof( buffer ), WIDE( "Enter New Glareset Name" )
		, button ) )
	{
		if( CheckGlareSet( buffer ) )
		{
			BannerMessage( "Glare set already exists!" );
		}
		else
		{
			PGLARE_SET glare_set = GetGlareSet( buffer );
			SetItemData( AddListItem( GetNearControl( button, LISTBOX_GLARE_SETS )
				, buffer )
				, (PTRSZVAL)glare_set );
		}
	}
}

void EditGlareSets( PSI_CONTROL parent )
{
	PSI_CONTROL frame;
	frame = LoadXMLFrame( "EditGlareSets.isFrame" );
	if( frame )
	{
		struct glare_set_edit params;
		int okay = 0;
		int done = 0;
		params.current = NULL;
		SetCommonButtons( frame, &done, &okay );
		{
			PSI_CONTROL list;
			list = GetControl( frame, LISTBOX_GLARE_SETS );
			if( list )
			{
				PGLARE_SET glare_set;
				INDEX idx;
				LIST_FORALL( g.glare_sets, idx, PGLARE_SET, glare_set )
				{
					SetItemData( AddListItem( list, glare_set->name ), (PTRSZVAL)glare_set );
				}
				SetSelChangeHandler( list, OnGlareSetSelect, (PTRSZVAL)&params );
			}
			SetButtonGroupID( GetControl( frame, CHECKBOX_GLARESET_MULTISHADE ), 2 );
			SetButtonGroupID( GetControl( frame, CHECKBOX_GLARESET_SHADE ), 2 );
			SetButtonGroupID( GetControl( frame, CHECKBOX_GLARESET_FIXED ), 2 );
			SetButtonPushMethod( GetControl( frame, GLARESET_APPLY_CHANGES ), ApplyGlareSetChanges, (PTRSZVAL)&params );
			SetButtonPushMethod( GetControl( frame, GLARESET_CREATE ), ButtonCreateGlareSet, (PTRSZVAL)&params );
		}
		DisplayFrameOver( frame, parent );
		CommonWait( frame );
		if( okay )
		{
		}
		DestroyFrame( &frame );
	}
}


//---------------------------------------------------------------------------


CTEXTSTR InterShell_GetSystemName( void )
{
	return g.system_name;
}


void InterShell_SetButtonStyle( PMENU_BUTTON button, char *style )
{
	if( !button )
		button = configure_key_dispatch.button;
	if( button )
		button->glare_set = GetGlareSet( style );
}

//---------------------------------------------------------------------------

static void SetButtonText( PMENU_BUTTON button )
{
	if( !button )
		button = configure_key_dispatch.button;
	if( button )
	{
		if( button->flags.bListbox )
			return;
		{
			char button_text[256];
			// take data in psv and apply to the button appropriately...
			CTEXTSTR p = InterShell_TranslateLabelText( NULL, button_text, sizeof( button_text ), button->text );
			char newmenuname[256];
			int pos;
			// Get info from dialog...
			newmenuname[0] = 'A';
			pos = 1;
			if( p )
			{
				while( p && p[0] )
				{
					if( p[0] == '_' )
					{
						newmenuname[pos++] = '\0';
						newmenuname[pos++] = 'A';
					}
					else
					{
						newmenuname[pos++] = p[0];
					}
					p++;
				}
				// double null terminate string.
				newmenuname[pos++] = 0;
				newmenuname[pos++] = 0;
				//p = Allocate( pos );
				if( button->control.key )
					SetKeyText( button->control.key, newmenuname );
			}
			else
			{
				if( button->control.key )
					SetKeyText( button->control.key, NULL );
			}
		}
	}
}

//---------------------------------------------------------------------------

void FlushToKey( PMENU_BUTTON button )
{
				if( button->control.key )
				{
					EnableKeyPresses( button->control.key, !button->flags.bNoPress );
					SetKeyTextColor( button->control.key, button->textcolor );
					SetKeyColor( button->control.key, button->color );
					SetButtonText( button );
					if( button->glare_set->flags.bMultiShadeBackground )
					{
						if( button->secondary_color )
						{
							SetKeyMultiShading( button->control.key, 0 //BASE_COLOR_BLACK //BASE_COLOR_DARKGRAY
													, button->color
													, button->secondary_color
													);
							SetKeyMultiShadingHighlights( button->control.key, 0
																 , button->color
																 , button->highlight_color );
						}
						else
						{
							SetKeyMultiShading( button->control.key
													, 0 //button->color
													, button->color
													, button->color
													);
						}
					}
					else if( button->glare_set->flags.bShadeBackground )
						SetKeyShading( button->control.key, button->color );
					else
						SetKeyShading( button->control.key, 0 ); // magic value to remove shading factor.
					if( button->pImage[0] )
					{
						if( !button->decal_image )
							button->decal_image = InterShell_CommonImageLoad( button->pImage );
						SetKeyImage( button->control.key, button->decal_image );
						SetKeyImageMargin( button->control.key, button->decal_horiz_margin, button->decal_vert_margin );
						SetKeyImageAlpha( button->control.key, button->decal_alpha );
					}
					else
					{
						if( button->decal_image )
						{
							InterShell_CommonImageUnloadByImage( button->decal_image );
							button->decal_image = NULL;
						}
						SetKeyImage( button->control.key, NULL );
						SetKeyImageAlpha( button->control.key, 0 );
					}
#ifndef __NO_ANIMATION__
					if( button->pAnimation[0] )
					{
						PCanvasData canvas = GetCanvas( GetCommonParent( QueryGetControl( button ) ) );
						if( button->decal_animation )
							DeInitAnimationEngine( button->decal_animation );

						button->decal_animation = InitAnimationEngine();
						GenerateAnimation( button->decal_animation, g.single_frame /*pc_button*/ /*GetFrameRenderer( g.single_frame)*/
											  , button->pAnimation, PARTX(button->x), PARTY(button->y), PARTW( button->x, button->w ), PARTH( button->y, button->h ));

					}
					else
					{
						if( button->decal_animation )
						{
							DeInitAnimationEngine( button->decal_animation );
							button->decal_animation = NULL;
						}
					}
#endif
					//SetKeyColor( button->key, button->color );
				}
}

void FixupButtonEx( PMENU_BUTTON button DBG_PASS )
{
	int show;
	PSI_CONTROL pc_button = QueryGetControl( button );
	PCanvasData canvas = GetCanvas( GetCommonParent( QueryGetControl( button ) ) );
   //lprintf( "Button fixup..." );
#define LoadImg(n) ((n)?LoadImageFile((TEXTSTR)n):NULL)
	if( canvas )
		if( canvas->flags.bEditMode ) // don't do fixup/reveal if editing...
		{
#ifndef __NO_ANIMATION__
			if( button->decal_animation )
			{
				DeInitAnimationEngine( button->decal_animation );
				button->decal_animation = NULL;
			}
#endif
			return;
		}
		//lprintf( "--- updating a button's visual aspects, disable updates..." );
		//   AddUse( pc_button );
		if( !g.flags.bPageUpdateDisabled )
			EnableCommonUpdates( pc_button, FALSE );
		show = QueryShowControl( button );
		if( !show )
		{
			/* this doesn't matter... */
			HideCommon( pc_button );
#ifndef __NO_ANIMATION__
			if( button->decal_animation )
			{
				DeInitAnimationEngine( button->decal_animation );
				button->decal_animation = NULL;
			}
#endif
			return;
		}
		if( !button->flags.bCustom &&
			!button->flags.bListbox )
		{
			PGLARE_SET glare_set = button->glare_set;
			if( glare_set )
			{
				int updated = 0;
				//TEXTCHAR buf[2566];
				//GetCurrentPath( buf, sizeof(buf) );
            //lprintf( "Current path %s", buf );
				// good a time as any to load images for the glare set...
				if( !glare_set->iGlare && glare_set->glare )
				{
					updated = 1;
					glare_set->iGlare = LoadImg( glare_set->glare );
				}
				if( !glare_set->iPressed && glare_set->down )
				{
					updated = 1;
					glare_set->iPressed = LoadImg( glare_set->down );
				}
				if( !glare_set->iNormal && glare_set->up )
				{
					updated = 1;
					glare_set->iNormal = LoadImg( glare_set->up );
				}
				if( !glare_set->iMask && glare_set->mask )
				{
					updated = 1;
					glare_set->iMask = LoadImg( glare_set->mask );
				}
				if( button->control.key )
				{
					SetKeyLenses( button->control.key, glare_set->iGlare, glare_set->iPressed, glare_set->iNormal, glare_set->iMask );
				}
			}
			else
			{
				if( button->control.key )
					SetKeyLenses( button->control.key, NULL, NULL, NULL, NULL );
			}
		}
		//_xlprintf(LOG_NOISE DBG_RELAY)( WIDE("Fixing up button :%s"), button->pTypeName );
		{
			// fixup can set ... background color?
			// text color?
			if( button->font_preset )
				SetCommonFont( pc_button, (*button->font_preset) );
			InvokeFixup( button ); // no fixup method...
			if( !button->flags.bCustom && !button->flags.bListbox && button->control.key )
			{
				// not a custom button, therefore fixup common properties of a button control...
            FlushToKey( button );
			}
			// allow user method to override anything pre-set in common
		}
		if( show )
		{
			//lprintf( "-!!!!! ---- reveal?" );
			RevealCommon( pc_button );
		}

		if( !g.flags.bPageUpdateDisabled )
		{
			//lprintf( "allow button to smudge... " );
			EnableCommonUpdates( pc_button, TRUE );
			SmudgeCommon( pc_button );
		}
		//   DeleteUse( pc_button );
		//lprintf( "--- updated a button's visual aspects, disable updates..." );
}

void InterShell_DisablePageUpdateEx( PSI_CONTROL canvas, LOGICAL bDisable )
{
}

void InterShell_DisablePageUpdate( LOGICAL bDisable )
{
	INDEX idx;
	INDEX idx_page;
	PCanvasData canvas = GetCanvas( g.single_frame );
	PPAGE_DATA current_page;
	PMENU_BUTTON control;
	//lprintf( "------- %s Page Updates ----------", bDisable?"DISABLE":"ENABLE" );
	g.flags.bPageUpdateDisabled = bDisable;

	//IJ 05.15.2007

	if( !canvas )
		return;


	if( g.flags.multi_edit )
	{
		//IJ 05.15.2007		if( !canvas )
		//IJ                      return;

		LIST_FORALL( canvas->pages, idx_page, PPAGE_DATA, current_page )
		{
			LIST_FORALL( current_page->controls, idx, PMENU_BUTTON, control )
			{
				EnableCommonUpdates( QueryGetControl( control ), !bDisable );
			}
			EnableCommonUpdates( current_page->frame, !bDisable );
			if( !bDisable )
				SmudgeCommon( current_page->frame );
		}
		{
			LIST_FORALL( canvas->default_page->controls, idx, PMENU_BUTTON, control )
			{
				EnableCommonUpdates( QueryGetControl( control ), !bDisable );
			}
			EnableCommonUpdates( canvas->default_page->frame, !bDisable );
			if( !bDisable )
				SmudgeCommon( canvas->default_page->frame );
		}
	}
	else
	{
		if( ( current_page = ShellGetCurrentPage() ) )
		{
			LIST_FORALL( current_page->controls, idx, PMENU_BUTTON, control )
			{
				EnableCommonUpdates( QueryGetControl( control ), !bDisable );
			}
			EnableCommonUpdates( current_page->frame, !bDisable );
			if( !bDisable )
			{
            UpdateCommon( current_page->frame );
				//SmudgeCommon( current_page->frame );
			}
		}
	}
}

int InvokeEditEnd( PMENU_BUTTON button )
{
	char rootname[256];
	void (CPROC*f)(PTRSZVAL);
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, void, WIDE("on_menu_edit_end"), (PTRSZVAL) );
	if( f )
	{
		f(button->psvUser);
		return TRUE;
	}
	return FALSE;
}

int QueryShowControl( PMENU_BUTTON button )
{
	char rootname[256];
	LOGICAL (CPROC*f)(PTRSZVAL);
	if( button->page != button->canvas->current_page )
		return 0;
	if( button->show_on )
	{
		INDEX idx;
		PTEXT name;
		int checked = 0;
		LIST_FORALL( button->show_on, idx, PTEXT, name )
		{
			checked++;
			if( CompareMask( GetText( name ), g.system_name, FALSE ) )
				break;
		}
		if( !name && checked )
			return 0;
	}
	if( button->no_show_on )
	{
		INDEX idx;
		PTEXT name;
		LIST_FORALL( button->no_show_on, idx, PTEXT, name )
		{
			if( CompareMask( GetText( name ), g.system_name, FALSE ) )
				break;
		}
		if( name )
			return 0;
	}
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, LOGICAL, WIDE("query can show"), (PTRSZVAL) );
	if( f )
	{
		return f(button->psvUser);
	}
	// if they control does not support this, show it.
	return TRUE;
}

void InvokeEditBegin( PMENU_BUTTON button )
{
	char rootname[256];
	void (CPROC*f)(PTRSZVAL);
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	f = GetRegisteredProcedure2( rootname, void, WIDE("on_menu_edit_begin"), (PTRSZVAL) );
	if( f )
		f(button->psvUser);
}

static void CPROC AllPresses( PTRSZVAL psv, PKEY_BUTTON key )
{
	static int bDoingButton;

	PMENU_BUTTON button = (PMENU_BUTTON)psv;
	PTRSZVAL psv_security;

	// serialize button presses to allow one to complete (also a quick hack for handling
	// password pause dialogs that do not cover screen... )
	if( bDoingButton )
		return;
   g.flags.bButtonProcessing = 1;
	bDoingButton = 1;
	// if the security context fails, then abort everything about the button.
	if( ( psv_security = CreateSecurityContext( button ) ) == INVALID_INDEX )
	{
		//if( button->iSecurityContext == INVALID_INDEX )
		g.flags.bButtonProcessing = 0;
		if( g.flags.bShowCanvas )
         InterShell_Reveal();
		bDoingButton = 0;
		return;
	}

	{
		//PCanvasData canvas = GetCanvas( NULL );
		//GenerateSprites( GetFrameRenderer( g.single_frame ), PARTX(button->x) + (PARTW(button->x,button->w) /2), PARTY(button->y) + (PARTH(button->y,button->h)/2));
	}


	if( button->original_keypress )
		button->original_keypress( button->psvUser );
	//lprintf( "Restore should have happened..." );
	if( button->pPageName )
	{
		if( button->canvas && !button->flags.bIgnorePageChange )
		{
			//lprintf( "Changing pages, but only virtually don't activate the page always" );
			ShellSetCurrentPageEx( button->canvas->pc_canvas, button->pPageName );
		}
		button->flags.bIgnorePageChange = 0;
	}

	/* not sure what this does... */
	if( psv_security )
		CloseSecurityContext( button, psv_security );

	// stop doing button, allow other presses to happen...
	bDoingButton = 0;
   g.flags.bButtonProcessing = 0;
	if( g.flags.bShowCanvas )
		InterShell_Reveal();

}

static void CPROC ListBoxSelectionChanged( PTRSZVAL psv, PSI_CONTROL list, PLISTITEM pli )
{
	PMENU_BUTTON button = (PMENU_BUTTON)psv;
	{
		char rootname[256];
		void (CPROC*f)(PTRSZVAL,PLISTITEM);
		snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
		f = GetRegisteredProcedure2( rootname, void, WIDE("listbox_selection_changed"), (PTRSZVAL,PLISTITEM) );
		if( f )
			f(button->psvUser, pli);
		{
			CTEXTSTR name;
			PCLASSROOT data = NULL;
			snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s/listbox_selection_changed", button->pTypeName );
			for( name = GetFirstRegisteredName( rootname, &data );
				name;
				name = GetNextRegisteredName( &data ) )
			{
				f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PTRSZVAL,PLISTITEM) );
				if( f )
					f(button->psvUser,pli);
			}
		}
	}

}

static void CPROC ListBoxDoubleChanged( PTRSZVAL psv, PSI_CONTROL list, PLISTITEM pli )
{
	PMENU_BUTTON button = (PMENU_BUTTON)psv;
	{
		char rootname[256];
		void (CPROC*f)(PTRSZVAL,PLISTITEM);
		snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
		f = GetRegisteredProcedure2( rootname, void, WIDE("listbox_double_changed"), (PTRSZVAL,PLISTITEM) );
		if( f )
			f(button->psvUser, pli);
		{
			CTEXTSTR name;
			PCLASSROOT data = NULL;
			snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s/listbox_double_changed", button->pTypeName );
			for( name = GetFirstRegisteredName( rootname, &data );
				name;
				name = GetNextRegisteredName( &data ) )
			{
				f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PTRSZVAL,PLISTITEM) );
				if( f )
					f(button->psvUser,pli);
			}
		}
	}

}

// return FALSE creation method fails.
static LOGICAL InvokeButtonCreate( PSI_CONTROL pc_canvas, PMENU_BUTTON button, LOGICAL bVisible )
{
	char rootname[256];
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc_canvas );
	PTRSZVAL (CPROC*f)(PSI_CONTROL,S_32 x, S_32 y, _32 w, _32 h);
	snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/control/%s", button->pTypeName );
	button->flags.bNoCreateMethod = TRUE; // assume there's no creator for this control
   //lprintf( "..." );
	//if( StrCmp( button->pTypeName, "Task" ) == 0 )
	//   DebugBreak();
	if( canvas )
		f = GetRegisteredProcedure2( rootname, PTRSZVAL, WIDE("control_create"), (PSI_CONTROL,S_32,S_32,_32,_32) );
	else
		f = NULL;
	if( f )
	{
      button->flags.bNoCreateMethod = 0; // found a creation method for this button.
		button->flags.bCustom = TRUE;
		button->psvUser = f( pc_canvas
			, PARTX( button->x )
			, PARTY( button->y )
			, PARTW( button->x, button->w )
			, PARTH( button->y, button->h )
			);
	}
	else
	{
		if( bVisible )
		{
			PTRSZVAL (CPROC*f_list)(PSI_CONTROL);
			if( canvas )
				f_list = GetRegisteredProcedure2( rootname, PTRSZVAL, WIDE("listbox_create"), (PSI_CONTROL) );
			else
				f_list = NULL;
			if( f_list )
			{
				button->flags.bNoCreateMethod = 0; // found a creation method for this button.
				button->flags.bListbox = TRUE;
				button->control.control = MakeNamedControl( pc_canvas, LISTBOX_CONTROL_NAME
					, PARTX( button->x )
					, PARTY( button->y )
					, PARTW( button->x, button->w )
					, PARTH( button->y, button->h )
					, -1
					);
				SetSelChangeHandler( button->control.control, ListBoxSelectionChanged, (PTRSZVAL)button );
				SetDoubleClickHandler( button->control.control, ListBoxDoubleChanged, (PTRSZVAL)button );
				SetListboxSort( button->control.control, LISTBOX_SORT_DISABLE );
				button->psvUser = f_list( button->control.control );
				if( !button->psvUser )
				{
					lprintf( "User Create failed (return psv=0), hiding control." );
					HideCommon( button->control.control );
					DestroyCommon( &button->control.control );
					button->flags.bListbox = 0;
				}
			}
			else
			{
				button->psvUser = 0;
				button->control.control = NULL;
			}
			// allow user to override default selection methods...
			// maybe?
		}
		if( ( !bVisible || canvas ) && !button->psvUser )
		{
			if( bVisible )
			{
				button->control.key = MakeKeyExx( pc_canvas
				, PARTX( button->x )
				, PARTY( button->y )
				, PARTW( button->x, button->w )
				, PARTH( button->y, button->h )
				, 0
				, NULL//g.iGlare
				, NULL//g.iNormal
				, NULL//g.iPressed
				, NULL//g.iMask
				, 0 // set to image and pass image pointer for texture
				, BASE_COLOR_BLACK
				, WIDE("AUndefined\0")
				, (*button->font_preset)
				, NULL // user click function
				, button->pTypeName  // user click function name (default)
				, 0  // this gets set later...
				, 0 );
			}
			else
			{
				{
					SimplePressHandler handler;
					char realname[256];
					snprintf( realname, sizeof(realname), WIDE("sack/widgets/keypad/press handler/%s"), button->pTypeName );
#define GetRegisteredProcedure2(nc,rtype,name,args) (rtype (CPROC*)args)GetRegisteredProcedureEx((nc),#rtype, name, #args )

					handler = GetRegisteredProcedure2( realname, void, WIDE("on_keypress_event"), (PTRSZVAL) );
					if( handler )
					{
						button->flags.bNoCreateMethod = 0; // found a creation method for this button.
						button->original_keypress = handler;
					}
				}
			}
			{
				PTRSZVAL (CPROC*f)(PMENU_BUTTON button);
				f = GetRegisteredProcedure2( rootname, PTRSZVAL, WIDE("button_create"), (PMENU_BUTTON) );
				if( f )
				{
					button->flags.bNoCreateMethod = 0; // found a creation method for this button.
					button->psvUser = f(button);
				}
				if( button->control.key )
				{
					if( !button->psvUser )
					{
						HideKey( button->control.key );
					}
					else
					{
						// sneak in and nab the keypress event.. forward it later...
						GetKeySimplePressEvent( button->control.key, &button->original_keypress, NULL );
						SetKeyPressEvent( button->control.key, AllPresses, (PTRSZVAL)button );
					}
					// hide this key since we're still editing...
					{
						//ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, g.current_page->frame );
						if( canvas->flags.bEditMode )
						{
							if( !canvas->edit_glare )
								HideKey( button->control.key );
						}
					}
				}
			}
		}
	}
   //lprintf( "..." );
	return TRUE;
}

PMENU_BUTTON CreateInvisibleControl( char *name )
{
	if( name )
	{
		PMENU_BUTTON button = CreateButton();
		button->flags.bCustom = FALSE;
		button->flags.bListbox = FALSE;
		button->flags.bInvisible = TRUE;
		button->pTypeName = StrDup( name );
		button->font_preset = &g.keyfont;
		button->font_preset_name = NULL;
		//lprintf( "Creating a virtual control %s", name );
		InvokeButtonCreate( NULL, button, FALSE );
		return button;
	}
	return NULL;
}

PMENU_BUTTON CreateSomeControl( PSI_CONTROL pc_canvas, int x, int y, int w, int h
							   , CTEXTSTR name )
{
	PMENU_BUTTON button = CreateButton();
	PMENU_BUTTON prior = configure_key_dispatch.button;
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc_canvas );
	configure_key_dispatch.button = button;
	button->flags.bCustom = FALSE;
	button->flags.bListbox = FALSE;
	button->x = x;
	button->y = y;
	button->w = w;
	button->h = h;
	button->pTypeName = StrDup( name );
	button->font_preset = &g.keyfont;
	button->font_preset_name = NULL; 
	//lprintf( "Creating a control %s on page %s", name, g.current_page->title );
	{
		button->page = canvas->current_page;
		button->canvas = canvas;
		AddLink( &canvas->current_page->controls, button );
		InvokeButtonCreate( pc_canvas, button, TRUE );
	}
	//lprintf( "Hiding the button that was just created... later it will be asked to show..." );
	//HideCommon( QueryGetControl( button ) );
   //lprintf( "..." );
	configure_key_dispatch.button = prior;
	return button;
}

PTRSZVAL  InterShell_CreateControl( CTEXTSTR type, int x, int y, int w, int h )
{
	PMENU_BUTTON button = CreateSomeControl( g.single_frame, x, y, w, h, type );
   return button->psvUser;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

CTEXTSTR CPROC InterShell_GetButtonFontName( PMENU_BUTTON pc )
{
	if( pc )
		return pc->font_preset_name;
	return NULL;
}

Font* CPROC InterShell_GetButtonFont( PMENU_BUTTON pc )
{
	if( pc )
		return pc->font_preset;
	return NULL;
}


Font* CPROC InterShell_GetCurrentButtonFont( void )
{
   return InterShell_GetButtonFont( configure_key_dispatch.button );
}


PMENU_BUTTON CPROC InterShell_GetCurrentButton( void )
{
	return ( configure_key_dispatch.button );
}       

PMENU_BUTTON InterShell_GetPhysicalButton( PMENU_BUTTON button )
{
	//lprintf( "Finding container of %p %p", button, button->container_button );
	while( button && button->container_button )
	{
		button = button->container_button;
		//lprintf( "Finding container of %p %p", button, button->container_button );
	}
   return button;
}       

void InterShell_SetButtonFont( PMENU_BUTTON button, Font *font )
{
	if( button && font )
		button->font_preset = font;
}

void CPROC InterShell_SetButtonFontName( PMENU_BUTTON button, CTEXTSTR name )
{
	if( button )
	{
		if( button->font_preset_name )
         Release( (POINTER)button->font_preset_name );
		button->font_preset_name = StrDup( name );
      button->font_preset = UseAFont( button->font_preset_name );
	}
}

//---------------------------------------------------------------------------

void InterShell_GetButtonText( PMENU_BUTTON button, TEXTSTR text, int text_buf_len )
{
	if( text )
	{
		if( button && button->text)
			StrCpyEx( text, button->text, text_buf_len );
		else
			text[0] = 0;
	}
}


void InterShell_SetButtonText( PMENU_BUTTON button, CTEXTSTR text )
{
	if( button )
	{
		if( button->text )
			Release( button->text );
		button->text = StrDup( text );
		SetButtonText( button );
	}
}

//---------------------------------------------------------------------------

void InterShell_SetButtonImage( PMENU_BUTTON button, CTEXTSTR name )
{
	if( button )
	{
		// already have this image on the button?
		if( name && ( strcmp( name, button->pImage ) == 0 ) )
			return;
		// it's not the same image, so we need to unload...
		if( button->decal_image )
		{
			InterShell_CommonImageUnloadByImage( button->decal_image );
			button->decal_image = NULL;
		}
		// set the new name into name string...
		if( name )
			strcpy( button->pImage, name );
		else
			button->pImage[0] = 0;
	}
}

//---------------------------------------------------------------------------

#ifndef __NO_ANIMATION__
INTERSHELL_PROC( void, InterShell_SetButtonAnimation )( PMENU_BUTTON button, CTEXTSTR name )
{
	if( button )
	{
		// already have this animation on the button?
		if( name && ( strcmp( name, button->pAnimation ) == 0 ) )
			return;
		// it's not the same animation, so we need to unload...
		if( button->decal_animation )
		{
			DeInitAnimationEngine( button->decal_animation );
			button->decal_animation = NULL;
		}
		// set the new name into name string...
		if( name )
			strcpy( button->pAnimation, name );
		else
			button->pAnimation[0] = 0;
	}
}
#endif
//---------------------------------------------------------------------------

INTERSHELL_PROC( void, InterShell_SetButtonImageAlpha )( PMENU_BUTTON button, S_16 alpha )
{
	if( button )
	{
		// already have this image on the button?
		button->decal_alpha = alpha;
	}
}

//---------------------------------------------------------------------------

INTERSHELL_PROC( void, InterShell_SetButtonColors )( PMENU_BUTTON button, CDATA cText, CDATA cBackground1, CDATA cBackground2, CDATA cBackground3 )
{
	if( !button )
		button = configure_key_dispatch.button;

   button = InterShell_GetPhysicalButton( button );
	if( button && button->glare_set && button->glare_set->flags.bMultiShadeBackground )
	{
		if( cBackground1 )
		{
			if( cBackground1 == COLOR_DISABLE )
				button->color = 0;
			else
				button->color = cBackground1;
		}
		if( cBackground2 )
		{
			if( cBackground2 == COLOR_DISABLE )
				button->secondary_color = 0;
			else
				button->secondary_color = cBackground2;
		}
		if( cBackground3 )
		{
			if( cBackground3 == COLOR_DISABLE )
				button->highlight_color = 0;
			else
            button->highlight_color = cBackground3;
		}
		if( cText )
		{
			if( cText == COLOR_DISABLE )
				button->textcolor = 0;
			else
				button->textcolor = cText;
		}
	}
	else if( button )
	{
		if( cText )
		{
			if( cText == COLOR_DISABLE )
				button->textcolor = 0;
			else
				button->textcolor = cText;
		}
		if( cBackground1 )
		{
			if( cBackground1 == COLOR_DISABLE )
				button->color = 0;
			else
				button->color = cBackground1;
		}
	}
}


//---------------------------------------------------------------------------

INTERSHELL_PROC( void, InterShell_SetButtonColor )( PMENU_BUTTON button, CDATA primary, CDATA secondary )
{
	InterShell_SetButtonColors( button, 0, primary, secondary, 0 );
}

//---------------------------------------------------------------------------

INTERSHELL_PROC( void, InterShell_GetButtonColors )( PMENU_BUTTON button, CDATA *primary, CDATA *secondary
										, CDATA *ring_color
										, CDATA *highlight_ring_color )
{
	if( !button )
		button = configure_key_dispatch.button;
   button = InterShell_GetPhysicalButton( button );
	if( button )
	{
		if( primary )
			(*primary) = button->textcolor;
		if( secondary )
			(*secondary) = button->color;
		if( ring_color )
			(*ring_color ) = button->secondary_color;
		if( highlight_ring_color )
			(*highlight_ring_color ) = button->highlight_color;
	}
}

//---------------------------------------------------------------------------

void AddSystemName( PSI_CONTROL list, CTEXTSTR name )
{
	struct system_info *system = g.systems;
	for( ;system; system = NextLink( system ) )
	{
		if( TextLike( system->name, name ) )
			break;
	}
	if( !system )
	{
		system = New( struct system_info );
		MemSet( system, 0, sizeof( struct system_info ) );
		system->name = SegCreateFromText( name );
		LinkThing( g.systems, system );
		if( list )
		{
			AddListItem( list, name );
		}
	}
}

//---------------------------------------------------------------------------

void GetAllowDisallowControls( void )
{
	PSI_CONTROL frame = configure_key_dispatch.frame;
	PMENU_BUTTON button = configure_key_dispatch.button;
	if( !button || button->flags.bInvisible )
		return;
	{
		PSI_CONTROL list;
		INDEX idx;
		PLISTITEM pli;
		list = GetControl( frame, LIST_SYSTEMS );
		if( list )
		{
			for( idx = 0; ( pli = GetNthItem( list, idx ) ); idx++ )
			{
				TEXTCHAR buffer[256];
				GetListItemText( pli, buffer, sizeof( buffer ) );
				{
					AddSystemName( NULL, buffer );
				}
			}
		}
		list = GetControl( frame, LIST_ALLOW_SHOW );
		if( list )
		{
			PTEXT name;
			LIST_FORALL( button->show_on, idx, PTEXT, name )
			{
				LineRelease( name );
				SetLink( &button->show_on, idx, NULL );
			}
			for( idx = 0; ( pli = GetNthItem( list, idx ) ); idx++ )
			{
				TEXTCHAR buffer[256];
				GetListItemText( pli, buffer, sizeof( buffer ) );
				{
					INDEX idx;
					LIST_FORALL( button->show_on, idx, PTEXT, name )
					{
						if( TextLike( name, buffer ) )
							break;
					}
					if( !name )
						AddLink( &button->show_on, SegCreateFromText( buffer ) );
				}
			}
		}
		list = GetControl( frame, LIST_DISALLOW_SHOW );
		if( list )
		{
			PTEXT name;
			LIST_FORALL( button->no_show_on, idx, PTEXT, name )
			{
				LineRelease( name );
				SetLink( &button->no_show_on, idx, NULL );
			}
			for( idx = 0; ( pli = GetNthItem( list, idx ) ); idx++ )
			{
				TEXTCHAR buffer[256];
				GetListItemText( pli, buffer, sizeof( buffer ) );
				{
					INDEX idx;
					LIST_FORALL( button->no_show_on, idx, PTEXT, name )
					{
						if( TextLike( name, buffer ) )
							break;
					}
					if( !name )
						AddLink( &button->no_show_on, SegCreateFromText( buffer ) );
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

void GetCommonButtonControls( PSI_CONTROL frame )
{
   PSI_CONTROL pc;
	// test for same frame?
	// this should have been set already by SetCommonButtonControls.
	if( configure_key_dispatch.frame != frame )
	{
		lprintf( "Aren't we busted?  isn't there more than one config dialog up?!" );
		DebugBreak();
	}
	if( !configure_key_dispatch.button ) // nothing for this to do... nothing ocmmon about it.
		return;

	GetAllowDisallowControls();
	// custom controls don't have a button characteristic like this...
	configure_key_dispatch.button->color = GetColorFromWell( GetControl( frame, CLR_BACKGROUND ) );
	configure_key_dispatch.button->secondary_color = GetColorFromWell( GetControl( frame, CLR_RING_BACKGROUND ) );
	configure_key_dispatch.button->textcolor = GetColorFromWell( GetControl( frame, CLR_TEXT_COLOR ) );
	configure_key_dispatch.button->highlight_color = GetColorFromWell( GetControl( frame, CLR_RING_HIGHLIGHT ) );
	{
		char buffer[128];
		PSI_CONTROL list = GetControl( frame, LST_PAGES );
		buffer[0] = 0; // this should have been cleard by GetItemText, but it's lazy, apparently
		if( list )
		{
			PLISTITEM pli = GetSelectedItem( list );
			GetItemText( pli, sizeof( buffer ), buffer );
			if( strcmp( buffer, WIDE("-- NONE --") ) == 0 )
				buffer[0] = 0;
			else if( strcmp( buffer, WIDE("-- Startup Page") ) == 0 )
				strcpy( buffer, WIDE("first" ) );
			if( strcmp( buffer, WIDE("-- Next") ) == 0 )
				strcpy( buffer, WIDE("next" ) );

			if( configure_key_dispatch.button->pPageName )
				Release( configure_key_dispatch.button->pPageName );
			if( buffer[0] )
				configure_key_dispatch.button->pPageName = StrDup( buffer );
			else
				configure_key_dispatch.button->pPageName = NULL;
		}
	}
	// font was picked...
	if( configure_key_dispatch.new_font &&
		( !configure_key_dispatch.button->font_preset_name ||
		strcmp( configure_key_dispatch.new_font_name
		, configure_key_dispatch.button->font_preset_name ) ) )
	{
		if( configure_key_dispatch.button->font_preset_name )
			Release( (POINTER)configure_key_dispatch.button->font_preset_name );
		configure_key_dispatch.button->font_preset_name = configure_key_dispatch.new_font_name;
		configure_key_dispatch.new_font_name = NULL;
		configure_key_dispatch.button->font_preset = configure_key_dispatch.new_font;
		SetCommonFont( QueryGetControl( configure_key_dispatch.button )
			, (*configure_key_dispatch.button->font_preset ) );
	}
	else
	{
		if( configure_key_dispatch.new_font_name )
			Release( (POINTER)configure_key_dispatch.new_font_name );
	}
	// IsButtonChecked( GetControl( frame, BTN_BACK_IMAGE ) )
   if( ( pc = GetControl( frame, TXT_IMAGE_NAME) ) )
	{
		char buf[256];
		GetControlText( pc
			, buf
			, sizeof( buf ) );
		InterShell_SetButtonImage( configure_key_dispatch.button, buf );
	}
   if( ( pc = GetControl( frame, TXT_IMAGE_V_MARGIN ) ) )
	{
		char buf[256];
		GetControlText( pc
			, buf
						  , sizeof( buf ) );
      configure_key_dispatch.button->decal_vert_margin = atoi( buf );
	}
   if( ( pc = GetControl( frame, TXT_IMAGE_H_MARGIN ) ) )
	{
		char buf[256];
		GetControlText( pc
			, buf
						  , sizeof( buf ) );
      configure_key_dispatch.button->decal_horiz_margin = atoi( buf );
	}

#ifndef __NO_ANIMATION__
	{
		char buf[256];
		GetControlText( GetControl( frame, TXT_ANIMATION_NAME)
			, buf
			, sizeof( buf ) );
		InterShell_SetButtonAnimation( configure_key_dispatch.button, buf );
	}
#endif
	{
		PSI_CONTROL style_list = GetControl( frame, LST_BUTTON_STYLE );
		if( style_list )
		{
			PLISTITEM pli = GetSelectedItem( style_list );
			if( pli )
			{
				configure_key_dispatch.button->glare_set = (PGLARE_SET)GetItemData( pli );
			}
		}
	}
	{
		PSI_CONTROL name_field = GetControl( frame, TXT_CONTROL_TEXT );
		if( name_field )
		{
			char text[256];
			GetControlText( name_field, text, sizeof( text ) );
			if( configure_key_dispatch.button->text )
				Release( configure_key_dispatch.button->text );
			configure_key_dispatch.button->text = StrDup( text );
		}
	}
	configure_key_dispatch.button->flags.bNoPress = GetCheckState( GetControl( frame, CHK_NOPRESS ) );
	SetButtonText( configure_key_dispatch.button );
	// only get this once.
}

//---------------------------------------------------------------------------

void ApplyCommonButtonControls( PSI_CONTROL frame )
{
	// test for same frame?
	PMENU_BUTTON button = configure_key_dispatch.button;
	GetCommonButtonControls( frame );
	// restore this so we can get it again...
	configure_key_dispatch.button = button;
}

//---------------------------------------------------------------------------

static void CPROC PickMenuControlFont( PTRSZVAL psv, PSI_CONTROL pc )
{
	Font *font = SelectAFont( configure_key_dispatch.frame
		, &configure_key_dispatch.new_font_name
		);
	if( font )
		configure_key_dispatch.new_font = font;
}

//---------------------------------------------------------------------------

static void CPROC AddNewSystemName( PTRSZVAL psv, PSI_CONTROL button )
{
	PSI_CONTROL list = GetNearControl( button, LIST_SYSTEMS );
	TEXTCHAR buffer[256];
	GetControlText( GetNearControl( button, EDIT_SYSTEM_NAME ), buffer, sizeof( buffer ) );
	if( !buffer[0] )
		return;
	AddListItem( list, buffer );

}

//---------------------------------------------------------------------------

static void CPROC AddSystemNameToAllow( PTRSZVAL psv, PSI_CONTROL button )
{
	PSI_CONTROL list1, list2;
	PLISTITEM pli;
	char buffer[256];
	list1 = GetNearControl( button, LIST_SYSTEMS );
	list2 = GetNearControl( button, LIST_ALLOW_SHOW );
	pli = GetSelectedItem( list1 );
	if( !pli )
	{
		// message, need select a system name
	}
	GetListItemText( pli, buffer, sizeof( buffer ) );
	SetItemData( AddListItem( list2, buffer ), GetItemData( pli ) );
}

//---------------------------------------------------------------------------

static void CPROC RemoveSystemNameAllow( PTRSZVAL psv, PSI_CONTROL button )
{
	PSI_CONTROL list2;
	PLISTITEM pli;
	list2 = GetNearControl( button, LIST_ALLOW_SHOW );
	pli = GetSelectedItem( list2 );
	DeleteListItem( list2, pli );
}

//---------------------------------------------------------------------------

static void CPROC AddSystemNameToDisallow( PTRSZVAL psv, PSI_CONTROL button )
{
	PSI_CONTROL list1, list2;
	char buffer[256];
	PLISTITEM pli;
	list1 = GetNearControl( button, LIST_SYSTEMS );
	list2 = GetNearControl( button, LIST_DISALLOW_SHOW );
	pli = GetSelectedItem( list1 );
	if( !pli )
	{
		// message, need select a system name
	}
	GetListItemText( pli, buffer, sizeof( buffer ) );
	SetItemData( AddListItem( list2, buffer ), GetItemData( pli ) );

}

//---------------------------------------------------------------------------

static void CPROC RemoveSystemNameDisallow( PTRSZVAL psv, PSI_CONTROL button )
{
	PSI_CONTROL list2;
	PLISTITEM pli;
	list2 = GetNearControl( button, LIST_DISALLOW_SHOW );
	pli = GetSelectedItem( list2 );
	DeleteListItem( list2, pli );
}

//---------------------------------------------------------------------------

void SetAllowDisallowControls( void )
{
	// uses configure_key_dispatch informaation
	PSI_CONTROL frame = configure_key_dispatch.frame;
	PMENU_BUTTON button = configure_key_dispatch.button;
	PSI_CONTROL list;
	PSI_CONTROL list_sys;
	if( !button || button->flags.bInvisible )
		return;
	list = GetControl( frame, LIST_SYSTEMS );
	if( list )
	{
		struct system_info *system;
		list_sys = list;
		for( system = g.systems; system; system = NextThing( system ) )
		{
			AddListItem( list, GetText( system->name ) );
		}
	}
	else
		list_sys = NULL;
	list = GetControl( frame, LIST_ALLOW_SHOW );
	if( list )
	{
		INDEX idx;
		PTEXT name;
		LIST_FORALL( button->show_on, idx, PTEXT, name )
		{
			AddListItem( list, GetText( name ) );
			AddSystemName( list_sys, GetText( name ) );
		}
	}
	list = GetControl( frame, LIST_DISALLOW_SHOW );
	if( list )
	{
		INDEX idx;
		PTEXT name;
		LIST_FORALL( button->no_show_on, idx, PTEXT, name )
		{
			AddListItem( list, GetText( name ) );
			AddSystemName( list_sys, GetText( name ) );
		}
	}

	SetButtonPushMethod( GetControl( frame, BTN_ADD_SYSTEM ), AddNewSystemName, 0 );
	SetButtonPushMethod( GetControl( frame, BTN_ADD_SYSTEM_TO_DISALLOW ), AddSystemNameToDisallow, 0 );
	SetButtonPushMethod( GetControl( frame, BTN_ADD_SYSTEM_TO_ALLOW ), AddSystemNameToAllow, 0 );
	SetButtonPushMethod( GetControl( frame, BTN_REMOVE_SYSTEM_FROM_DISALLOW ), RemoveSystemNameDisallow, 0 );
	SetButtonPushMethod( GetControl( frame, BTN_REMOVE_SYSTEM_FROM_ALLOW ), RemoveSystemNameAllow, 0 );

}

/* this is function has a duplicately named function in pages.c */

static void CPROC ChooseImage( PTRSZVAL psv, PCONTROL pc )
{
	char result[256];
	if( PSI_PickFile( pc, WIDE("."), NULL, result, sizeof( result ), FALSE ) )
	{
		SetControlText( GetNearControl( pc, TXT_IMAGE_NAME ), result );
	}
}
#ifndef __NO_ANIMATION__
static void CPROC ChooseAnimation( PTRSZVAL psv, PCONTROL pc )
{
	char result[256];
	if( PSI_PickFile( pc, WIDE("."), NULL, result, sizeof( result ), FALSE ) )
	{
		SetControlText( GetNearControl( pc, TXT_ANIMATION_NAME ), result );
	}
}
#endif

char *I(_32 val)
{
	static char buf[256];
	snprintf( buf, sizeof( buf ), "%ld", val );
   return buf;
}

void SetCommonButtonControls( PSI_CONTROL frame )
{
	// keep this, someday we might want to know it again...
	configure_key_dispatch.frame = frame;
	if( !configure_key_dispatch.button
		|| configure_key_dispatch.button->flags.bInvisible ) // nothing for this to do... nothing ocmmon about it.
	{
		HideCommon( GetControl( frame, LST_BUTTON_STYLE ) );
		HideCommon( GetControl( frame, BTN_PICKFONT ) );
		HideCommon( GetControl( frame, CLR_TEXT_COLOR) );
		HideCommon( GetControl( frame, CLR_RING_BACKGROUND) );
		HideCommon( GetControl( frame, CLR_BACKGROUND) );

		HideCommon( GetControl( frame, CLR_RING_HIGHLIGHT) );
		HideCommon( GetControl( frame, TXT_IMAGE_NAME ) );
		HideCommon( GetControl( frame, TXT_IMAGE_V_MARGIN ) );
		HideCommon( GetControl( frame, TXT_IMAGE_H_MARGIN ) );
#ifndef __NO_ANIMATION__
		HideCommon( GetControl( frame, TXT_ANIMATION_NAME ) );
#endif
		HideCommon( GetControl( frame, TXT_CONTROL_TEXT ) );
		HideCommon( GetControl( frame, CHK_NOPRESS ) );

		HideCommon( GetControl( frame, LST_PAGES ) );

		HideCommon( GetControl( frame, LIST_ALLOW_SHOW ) );
		HideCommon( GetControl( frame, LIST_DISALLOW_SHOW ) );
		HideCommon( GetControl( frame, LIST_SYSTEMS ) );
		HideCommon( GetControl( frame, LIST_ALLOW_SHOW ) );
		HideCommon( GetControl( frame, BTN_ADD_SYSTEM ) );
		HideCommon( GetControl( frame, BTN_ADD_SYSTEM_TO_DISALLOW ) );
		HideCommon( GetControl( frame, BTN_ADD_SYSTEM_TO_ALLOW ) );

		return;
	}
	if( !configure_key_dispatch.button->flags.bListbox )
	{
		PSI_CONTROL style_list = GetControl( frame, LST_BUTTON_STYLE );
		if( style_list )
		{
			PGLARE_SET glare_set;
			INDEX idx;
			LIST_FORALL( g.glare_sets, idx, PGLARE_SET, glare_set )
			{
				PLISTITEM pli = AddListItem( style_list, glare_set->name );
				SetItemData( pli, (PTRSZVAL)glare_set );
				if( strcmp( configure_key_dispatch.button->glare_set->name
					, glare_set->name ) == 0 )
				{
					SetSelectedItem( style_list, pli );
				}
			}
		}
	}
	if( configure_key_dispatch.button->font_preset_name )
		configure_key_dispatch.new_font_name = StrDup( configure_key_dispatch.button->font_preset_name );
	else
		configure_key_dispatch.new_font_name = NULL;
	configure_key_dispatch.new_font = NULL;
	SetButtonPushMethod( GetControl( frame, BTN_PICKFONT ), PickMenuControlFont, 0 );
	SetButtonPushMethod( GetControl( frame, BTN_PICKFILE ), ChooseImage, 0 );
#ifndef __NO_ANIMATION__
	SetButtonPushMethod( GetControl( frame, BTN_PICKANIMFILE ), ChooseAnimation, 0 );
#endif
	SetAllowDisallowControls();
	{
		PSI_CONTROL listbox;
		listbox = GetControl( frame, LISTBOX_SECURITY_MODULE );
		if( listbox )
		{
			INDEX idx;
			CTEXTSTR name;
			LIST_FORALL( g.security_property_names, idx, CTEXTSTR, name )
			{    
				SetItemData( AddListItem( listbox, name ), (PTRSZVAL)name );
			}
			SetDoubleClickHandler( listbox, SelectEditSecurity, (PTRSZVAL)configure_key_dispatch.button );
			SetButtonPushMethod( GetControl( frame, EDIT_SECURITY ), EditSecurity, (PTRSZVAL)configure_key_dispatch.button );
		}
		else
		{
			SetButtonPushMethod( GetControl( frame, EDIT_SECURITY ), EditSecurityNoList, 0 );
		}
	}
	EnableColorWellPick( SetColorWell( GetControl( frame, CLR_TEXT_COLOR), configure_key_dispatch.button->textcolor ), TRUE );
	EnableColorWellPick( SetColorWell( GetControl( frame, CLR_RING_BACKGROUND), configure_key_dispatch.button->secondary_color ), TRUE );
	EnableColorWellPick( SetColorWell( GetControl( frame, CLR_BACKGROUND), configure_key_dispatch.button->color ), TRUE );
	if( !configure_key_dispatch.button->flags.bListbox &&
		!configure_key_dispatch.button->flags.bCustom )
	{
		EnableColorWellPick( SetColorWell( GetControl( frame, CLR_RING_HIGHLIGHT), configure_key_dispatch.button->highlight_color ), TRUE );
		SetControlText( GetControl( frame, TXT_IMAGE_NAME ), configure_key_dispatch.button->pImage );
		SetControlText( GetControl( frame, TXT_IMAGE_H_MARGIN ), I(configure_key_dispatch.button->decal_horiz_margin) );
		SetControlText( GetControl( frame, TXT_IMAGE_V_MARGIN ), I(configure_key_dispatch.button->decal_horiz_margin) );

#ifndef __NO_ANIMATION__
		SetControlText( GetControl( frame, TXT_ANIMATION_NAME ), configure_key_dispatch.button->pAnimation );
#endif
		if( configure_key_dispatch.button->text )
		{
			SetControlText( GetControl( frame, TXT_CONTROL_TEXT ), configure_key_dispatch.button->text );
		}
		SetCheckState( GetControl( frame, CHK_NOPRESS )
			, configure_key_dispatch.button->flags.bNoPress );
		{
			PCanvasData canvas = GetCanvas( GetCommonParent( QueryGetControl( configure_key_dispatch.button ) ) );
			PSI_CONTROL list = GetControl( frame, LST_PAGES );
			if( canvas && list )
			{
				INDEX idx;
				PPAGE_DATA page;
				PLISTITEM pli;
				pli = AddListItem( list, WIDE("-- NONE -- ") );
				if( !configure_key_dispatch.button->pPageName )
					SetSelectedItem( list, pli );
				pli = AddListItem( list, WIDE("-- Startup Page") );
				if( configure_key_dispatch.button->pPageName &&
					stricmp( configure_key_dispatch.button->pPageName, WIDE("first") ) == 0 )
					SetSelectedItem( list, pli );
				pli = AddListItem( list, WIDE("-- Next") );
				if( configure_key_dispatch.button->pPageName &&
					stricmp( configure_key_dispatch.button->pPageName, WIDE("next") ) == 0 )
					SetSelectedItem( list, pli );
				LIST_FORALL( canvas->pages, idx, PPAGE_DATA, page )
				{
					pli = AddListItem( list, page->title );
					if( configure_key_dispatch.button->pPageName &&
						stricmp( configure_key_dispatch.button->pPageName, page->title ) == 0 )
					{
						SetSelectedItem( list, pli );
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

// uses the currently selected button...
void InterShell_EditButton( PSI_CONTROL pc_parent )
{

	PSI_CONTROL frame = LoadXMLFrameOver( pc_parent, "EditGenericButton.isframe" ); // can use this frame also, just default controls
	if( frame )
	{
		int okay = 0;
		int done = 0;
		SetCommonButtons( frame, &done, &okay );
		SetCommonButtonControls( frame );  // magically knows which button we're editing at the moment.
		DisplayFrameOver( frame, pc_parent );
		EditFrame( frame, TRUE );
		CommonWait( frame );
		if( okay )
		{
			GetCommonButtonControls( frame );
		}
		DestroyFrame( &frame );
	}
	//return psv;

}

//---------------------------------------------------------------------------

// uses the currently selected button...
void InterShell_EditGeneric( PSI_CONTROL pc_parent )
{
	PSI_CONTROL frame = LoadXMLFrameOver( pc_parent, "EditGenericControl.isframe" ); // can use this frame also, just default controls
	if( frame )
	{
		int okay = 0;
		int done = 0;
		SetCommonButtons( frame, &done, &okay );
		SetCommonButtonControls( frame );  // magically knows which button we're editing at the moment.
		DisplayFrameOver( frame, pc_parent );
		EditFrame( frame, TRUE );
		CommonWait( frame );
		if( okay )
		{
			GetCommonButtonControls( frame );
		}
		DestroyFrame( &frame );
	}
}

//---------------------------------------------------------------------------

// uses the currently selected button...
void InterShell_EditListbox( PSI_CONTROL pc_parent )
{
	PSI_CONTROL frame = LoadXMLFrameOver( pc_parent, "EditGenericListbox.isframe" ); // can use this frame also, just default controls
	if( frame )
	{
		PSI_CONTROL list = (PSI_CONTROL)configure_key_dispatch.button->control.control;
		int okay = 0;
		int done = 0;
		SetCommonButtons( frame, &done, &okay );
		SetCommonButtonControls( frame );  // magically knows which button we're editing at the moment.
		{
			int multi, lazy;
			GetListboxMultiSelectEx( list, &multi, &lazy );
			SetCheckState( GetControl( frame, CHECKBOX_LIST_MULTI_SELECT ), multi );
			SetCheckState( GetControl( frame, CHECKBOX_LIST_LAZY_MULTI_SELECT ), lazy );
		}
		DisplayFrameOver( frame, pc_parent );
		EditFrame( frame, TRUE );
		CommonWait( frame );
		if( okay )
		{
			PSI_CONTROL pc;
			GetCommonButtonControls( frame );
			pc = GetControl( frame, CHECKBOX_LIST_LAZY_MULTI_SELECT );
			if( pc )
			{
				SetListboxMultiSelectEx( list, GetCheckState( GetControl( frame, CHECKBOX_LIST_MULTI_SELECT ) ), GetCheckState( pc ) );
			}
			else
			{
				SetListboxMultiSelect( list, GetCheckState( GetControl( frame, CHECKBOX_LIST_MULTI_SELECT ) ) );
			}
		}
		DestroyFrame( &frame );
	}
	//return psv;

}

//---------------------------------------------------------------------------

struct configure_info
{
	struct {
		BIT_FIELD complete : 1;
		BIT_FIELD received : 1;
		BIT_FIELD bIgnorePrivate : 1;
	} flags;
	PTHREAD waiting; // if waiting, flags.complete will be set, and this thread will wake.
	PSI_CONTROL parent;
	PMENU_BUTTON button;
	PCanvasData canvas;
};

PTRSZVAL CPROC ThreadConfigureButton( PTHREAD thread )
{
	struct configure_info *info = (struct configure_info *)GetThreadParam( thread );
	struct configure_key_dispatch save; // other child windows cannot complete.
	//PSI_CONTROL parent = info->parent;
	PCanvasData canvas     = info->canvas;
	int bIgnorePrivate     = info->flags.bIgnorePrivate;
	PSI_CONTROL pc_parent  = info->parent;
	PMENU_BUTTON button    = (PMENU_BUTTON)info->button;
	PMENU_BUTTON prioredit = configure_key_dispatch.button;
	PTHREAD wake           = info->waiting;

	info->flags.received = 1;
	// do not use (*info) after this point! 
	// the calling thread is GONE and so is this info.
	save = configure_key_dispatch;
	MemSet( &configure_key_dispatch, 0, sizeof( configure_key_dispatch ) );

	{
		char rootname[256];
		PTRSZVAL (CPROC*f)(PTRSZVAL,PSI_CONTROL);
		//EnterCriticalSec( &configure_key_dispatch.cs );
		while( !button->flags.bInvisible && configure_key_dispatch.button )
			IdleFor(100);
		configure_key_dispatch.canvas = canvas;
		configure_key_dispatch.button = button;
		snprintf( rootname, sizeof( rootname ), TASK_PREFIX WIDE("/control/%s"), button->pTypeName );
		if( !bIgnorePrivate )
		{
			f = GetRegisteredProcedure2( rootname, PTRSZVAL, WIDE("control_edit"), (PTRSZVAL,PSI_CONTROL) );
			if( f )
			{
				button->psvUser = f(button->psvUser, pc_parent );
				FixupButton( button );
			}
		}
		else
			f = NULL;
		if( !f )
		{
			// depricated method...
			if( !bIgnorePrivate )
			{
				snprintf( rootname, sizeof( rootname ), TASK_PREFIX WIDE("/control/%s"), button->pTypeName );
				f = GetRegisteredProcedure2( rootname, PTRSZVAL, WIDE("button_edit"), (PTRSZVAL,PSI_CONTROL) );
				if( f )
				{
					button->psvUser = f(button->psvUser, pc_parent );
					FixupButton( button );
				}
			}
			if( !f )
			{
				// if it's a standard button... attempt to edit standard properties on it.
				if( button->flags.bListbox )
					InterShell_EditListbox( pc_parent );
				else if( !button->flags.bCustom )
					InterShell_EditButton( pc_parent );
				else if( button->flags.bCustom )
					InterShell_EditGeneric( pc_parent );
			}
		}
		configure_key_dispatch.button = prioredit;
	}
	configure_key_dispatch = save;
	// done with this...
	if( canvas )
	{
		if( !configure_key_dispatch.button )
		{
			canvas->flags.bIgnoreKeys = 0;
		}
		else
		{
			DebugBreak();
		}
	}
	if( wake ) 
	{
		// this parameter means the calling thread is waiting for completion, thereofre info is still a valid pointer.
		info->flags.complete = 1;
		WakeThread( wake );
	}

	return 0;
}

//---------------------------------------------------------------------------

void ConfigureKeyExx( PCanvasData canvas, PMENU_BUTTON button, int bWaitComplete, int bIgnoreControlOverload )
{
	//PMENU_BUTTON prior_edit;
	//ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, parent );
	struct configure_info info;
	info.parent = canvas->edit_glare_frame;
	info.button = button;
	info.canvas = canvas;
	info.flags.received = 0;
	info.flags.bIgnorePrivate = bIgnoreControlOverload;
	if( bWaitComplete )
		info.waiting = MakeThread();
	else
		info.waiting = NULL;
	// okay yeah done with that key...
	if( !button->flags.bInvisible && canvas->flags.bIgnoreKeys )
		return;
	if( canvas )
		canvas->flags.bIgnoreKeys = 1;

	ThreadTo( ThreadConfigureButton, (PTRSZVAL)&info );
	// allow thread to read parameters from info structure.
	while( !info.flags.received )
		Relinquish();
	if( bWaitComplete )
		while( !info.flags.complete )
			IdleFor( 250 );
}

void ConfigureKeyEx( PCanvasData parent, PMENU_BUTTON button )
{
	ConfigureKeyExx( parent, button, FALSE, FALSE );
}
//---------------------------------------------------------------------------

int ProcessPageMenuResult( PSI_CONTROL pc_canvas, _32 result )
{
	if( result >= MNU_GLOBAL_PROPERTIES && result <= MNU_GLOBAL_PROPERTIES_MAX )
	{
		ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc_canvas );
		void (CPROC*f)(PSI_CONTROL);
		f = (void (CPROC*)(PSI_CONTROL) )GetLink( &g.global_properties, result - MNU_GLOBAL_PROPERTIES );
		if( f )
			f( canvas->edit_glare_frame );
		return 1;
	}
	if( result >= MNU_CHANGE_PAGE && result <= MNU_CHANGE_PAGE_MAX )
	{
		SetCurrentPageID( pc_canvas, result - MNU_CHANGE_PAGE );
		return 1;
	}
	if( result >= MNU_DESTROY_PAGE && result <= MNU_DESTROY_PAGE_MAX )
	{
		DestroyPageID( pc_canvas, result - MNU_DESTROY_PAGE  );
		return 1;
	}
	if( result >= MNU_UNDELETE_PAGE && result <= MNU_UNDELETE_PAGE_MAX )
	{
		// move from the undelete menu to the real page menu
		UnDestroyPageID( pc_canvas, result - MNU_DESTROY_PAGE  );
		return 1;
	}
	return 0;
}

void CloneCommonButtonProperties( PMENU_BUTTON clone, PMENU_BUTTON  clonebutton )
{
	clone->color           = clonebutton->color;
	clone->secondary_color = clonebutton->secondary_color;
	clone->textcolor       = clonebutton->textcolor;
	clone->highlight_color = clonebutton->highlight_color;
	clone->font_preset     = clonebutton->font_preset;
	clone->font_preset_name = clonebutton->font_preset_name;
	clone->text            = StrDup( clonebutton->text );
	strcpy( clone->pImage, clonebutton->pImage );
#ifndef __NO_ANIMATION__
	strcpy( clone->pAnimation, clonebutton->pAnimation );
#endif
	clone->flags.bNoPress  = clonebutton->flags.bNoPress;
	clone->flags.bIgnorePageChange= clonebutton->flags.bIgnorePageChange;
	clone->flags.bSecure   = clonebutton->flags.bSecure;
	//clone->flags.bSecureEndContext ;
	clone->pPageName       = StrDup( clonebutton->pPageName );
	clone->glare_set       = clonebutton->glare_set; // glares used on this button

	// if this context is already entered, then the security check is not done.
	//TEXTSTR security_context; // once entered, this context is set...
	//TEXTSTR security_reason; // reason to log into permission_user_info
	//TEXTSTR security_token; // filter list of users by these tokens, sepeareted by ','
	//INDEX iSecurityContext; // index into login_history that identifies the context of this login..
}

PMENU_BUTTON GetCloneButton( PCanvasData canvas, int px, int py, int bInvisible )
{
	if( g.clonebutton )
	{
		// commented bit creates new controled centered on mouse
		PMENU_BUTTON clone;
		if( bInvisible || !canvas )
			clone = CreateInvisibleControl( g.clonebutton->pTypeName );
		else
		{
			lprintf( "This is where cloned controls are created." );
			clone = CreateSomeControl( canvas->current_page->frame
											 , px //- (g.clonebutton->w/2)
											 , py //- (g.clonebutton->h/2)
											 , (_32)g.clonebutton->w
											 , (_32)g.clonebutton->h
											 , g.clonebutton->pTypeName );
		}
		CloneCommonButtonProperties( clone, g.clonebutton );
		InvokeCloneControl( clone, g.clonebutton );
		SmudgeCommon( QueryGetControl( clone ) );
		return clone;
	}
   return NULL;
}

//---------------------------------------------------------------------------
int CPROC MouseEditGlare( PTRSZVAL psv, S_32 x, S_32 y, _32 b );

int CPROC ShellMouse( PCOMMON pc, S_32 x, S_32 y, _32 b )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	static _32 _b;
	static S_32 _x, _y;
	//int px, py;
#define PARTOFX(xc) ( ( xc ) * canvas->current_page->grid.nPartsX ) / canvas->width
#define PARTOFY(yc) ( ( yc ) * canvas->current_page->grid.nPartsY ) / canvas->height
	//px = PARTOFX( x );
	//py = PARTOFY( y );
	if( canvas->flags.bEditMode )
	{
      return MouseEditGlare( (PTRSZVAL)canvas, x, y, b );
	}
	// shell mouse is frame mouse?
	//lprintf( WIDE("Shell mosue %d,%d %d"), x, y,  b );

	_b = b;
	_x = x;
	_y = y;
	return 1;
}


//---------------------------------------------------------------------------

#ifdef USE_EDIT_GLARE
void CPROC DrawEditGlare( PTRSZVAL psv, PRENDERER edit_glare )
#else
void CPROC DrawEditGlare( PTRSZVAL psv, Image surface )
#endif
{
	{
      PCanvasData canvas = (PCanvasData)psv;
		//_32 w, h;
		PPAGE_DATA current_page = canvas->current_page;

		{
			// should use X to capture an image of the current menu config
			// then during editing we do not have to redraw all controls all the time..
			ClearImageTo( surface, 0x01000000 );
			//lprintf( ".... some kinda mystical update here" );
			//EnableFrameUpdates( pf, FALSE );
			//ReleaseCommonUse(pf);
			//EnableFrameUpdates( pf, TRUE );
			lprintf( ".... some kinda mystical update end here" );
			//lprintf( WIDE("Continue background") );

			{
				PMENU_BUTTON button;
				INDEX idx;
				int nButton = 1;
				CDATA base_color;
				LIST_FORALL( current_page->controls, idx, PMENU_BUTTON, button )
				{
					int selected = 0;
					long long x, y;
					long long w, h;
					char buttonname[128];
					{
						INDEX idx2;
						PMENU_BUTTON button2;
						base_color = BASE_COLOR_GREEN;
						LIST_FORALL( canvas->selected_list, idx2, PMENU_BUTTON, button2 )
						{
							if( button2 == button )	
							{
								selected = 1;
								base_color = BASE_COLOR_MAGENTA;
								break;
							}
						}
					}
					if( !selected )
						if( button == canvas->pCurrentControl )
							selected = 1;
					//lprintf( WIDE("Drawing at %d,%d..."), button->x, button->y );
					x = (PARTX( button->x ) + 5); y = (PARTY( button->y ) + 5);
					w=(PARTW( button->x, button->w )+1 - 10);
					h=(PARTH( button->y, button->h )+1 - 10);
					if( x < 0 )
					{
						w += x;
						x = 0;
					}
					if( y < 0 )
					{
						h += y;
						y = 0;
					}
					//lprintf( "region is %Ld,%Ld %Ld,%Ld", x, y, w, h) ;
					BlatColorAlpha( surface, (S_32)x, (S_32)y, (_32)w, (_32)h
						, SetAlpha( (selected
									&&canvas->flags.dragging)?BASE_COLOR_YELLOW:base_color
									, (button==canvas->pCurrentControl
									&&canvas->flags.sizing_by_corner == DRAG_BEGIN)?180:90 )
						);
					BlatColorAlpha( surface
						, PARTX(button->x), PARTY(button->y)
						, PARTW(button->x,1), PARTH(button->y,1)
						, SetAlpha( (button==canvas->pCurrentControl
									&&canvas->flags.sizing)?BASE_COLOR_YELLOW:BASE_COLOR_RED
									, ( button==canvas->pCurrentControl
									&&canvas->flags.sizing_by_corner == UPPER_LEFT)?180:90 ) 
						);
					BlatColorAlpha( surface
						, PARTX(button->x), PARTY(button->y+button->h-1)
						, PARTW(button->x,1), PARTH(button->y+button->h-1,1)
						, SetAlpha( (button==canvas->pCurrentControl
									&&canvas->flags.sizing)?BASE_COLOR_YELLOW:BASE_COLOR_RED, (button==canvas->pCurrentControl
									&&canvas->flags.sizing_by_corner == LOWER_LEFT)?180:90 ) );
					BlatColorAlpha( surface
						, PARTX(button->x+button->w-1), PARTY(button->y)
						, PARTW(button->x+button->w,1), PARTH(button->y,1)
						, SetAlpha( (button==canvas->pCurrentControl
									&&canvas->flags.sizing)?BASE_COLOR_YELLOW:BASE_COLOR_RED, (button==canvas->pCurrentControl
									&&canvas->flags.sizing_by_corner == UPPER_RIGHT)?180:90 ) );
					BlatColorAlpha( surface
						, PARTX(button->x+button->w-1), PARTY(button->y+button->h-1)
						, PARTW(button->x+button->w,1), PARTH(button->y+button->h-1,1)
						, SetAlpha( (button==canvas->pCurrentControl
									&&canvas->flags.sizing)?BASE_COLOR_YELLOW:BASE_COLOR_RED, (button==canvas->pCurrentControl
									&&canvas->flags.sizing_by_corner == LOWER_RIGHT)?180:90 ) );
					snprintf( buttonname, sizeof( buttonname ), WIDE("%s(%d)")
						, button->pTypeName
						, nButton++
						);
					PutString( surface
						, (S_32)x + 15, (S_32)y + 15, BASE_COLOR_WHITE, 0
						, buttonname );
				}
			}

			{
				int x, y;

				for( x = 0; x <= PARTSX; x++ )
				{
					//lprintf( WIDE("Line %d has %d/%d"), x, COMPUTEX(x,PARTSX), MODX( x, PARTSX ) );
					do_vlineAlpha( surface, PARTX(x), 0, surface->height, SetAlpha( BASE_COLOR_WHITE, 48 ) );
					//if( MODX( x, PARTSX ) > 1900 )
					//	do_vline( surface, COMPUTEX(x,PARTSX)+1, 0, surface->height, BASE_COLOR_DARKGREY );
				}

				for( y = 0; y <= PARTSY; y++ )
				{
					//lprintf( WIDE("Line %d has partial %d/%d"), y, COMPUTEY(y,PARTSY), MODY( y, PARTSY ) );
					do_hlineAlpha( surface, PARTY(y), 0, surface->width, SetAlpha( BASE_COLOR_WHITE, 48 ) );
					//if( MODX( y, PARTSY ) > 1800 )
					//	do_hline( surface, COMPUTEY(y,PARTSY)+1, 0, surface->width, BASE_COLOR_DARKGREY );
				}

				if( canvas->flags.selected )
				{
					//lprintf( WIDE("Our fancy coords could be %d,%d %d,%d"), PARTX( canvas->selection.x ), PARTY( canvas->selection.y )
					//		 , PARTW( canvas->selection.x, canvas->selection.w )
					//		 , PARTH( canvas->selection.y, canvas->selection.h ));
					// and to look really pretty select the outer edge on the bottom, also
					BlatColorAlpha( surface, PARTX( canvas->selection.x ), PARTY( canvas->selection.y )
						, PARTW( canvas->selection.x, canvas->selection.w )+1
						, PARTH( canvas->selection.y, canvas->selection.h )+1
						, canvas->flags.selecting
						?SetAlpha( ColorAverage( BASE_COLOR_BLUE, BASE_COLOR_WHITE, 50, 100 ), 170 )
						:SetAlpha( BASE_COLOR_BLUE, 120 )
						);
				}

			}
		}
	}
#ifdef USE_EDIT_GLARE
	UpdateDisplay( edit_glare );
#endif
}


//---------------------------------------------------------------------------

int CPROC DrawFrameBackground( PCOMMON pf )
{
   //lprintf( "----------g.flags.bPageUpdateDisabled %d", g.flags.bPageUpdateDisabled );
	if( !g.flags.bPageUpdateDisabled )
	{
		ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pf );
		PPAGE_DATA current_page;
		//_32 w, h;
		Image surface = GetFrameSurface( pf );
		//GetPageSize( pf, &w, &h );
		//if( g.flags.multi_edit )
		current_page = canvas->current_page;
      //lprintf( "--- AM DRAWING BACKGROUND" );
		// update the canvas's dimensions...
		////////-s-s-s-s
		if( (( canvas->width != surface->width )&& canvas->width ) ||
			(( canvas->height != surface->height )&& canvas->height ) )
		{
			INDEX idx;
			PMENU_BUTTON button;
			INDEX idx_page;
			PPAGE_DATA page;
			S_32 x, y;
			_32 w, h;
			GetDisplayPosition( canvas->renderer, &x, &y, &w, &h );

			MoveSizeDisplay( canvas->edit_glare, x, y, w, h );

			canvas->width = surface->width;
			canvas->height = surface->height;
			LIST_FORALL( canvas->pages, idx_page, PPAGE_DATA, page )
			{
				LIST_FORALL( page->controls, idx, PMENU_BUTTON, button )
				{
					MoveSizeCommon( QueryGetControl( button )
						, PARTX( button->x )
						, PARTY( button->y )
						, PARTW( button->x, button->w )
						, PARTH( button->y, button->h )
						);
				}
			}
			{
				LIST_FORALL( canvas->default_page->controls, idx, PMENU_BUTTON, button )
				{
					MoveSizeCommon( QueryGetControl( button )
						, PARTX( button->x )
						, PARTY( button->y )
						, PARTW( button->x, button->w )
						, PARTH( button->y, button->h )
						);
				}
			}
		}
		//else
		//	current_page = g.current_page;
		//EnableFrameUpdates( pf, FALSE );
		//DumpFrameContents( pf );
		if( current_page )
		{
			if( !current_page->background_image && current_page->background )
				current_page->background_image = LoadImageFile( (char*)current_page->background );
			if( current_page->background_color )
				ClearImageTo( surface, current_page->background_color );
			if( current_page->background_image )
			{
				// hmm - hazy edges will be bad... need to
				// option to fix the constant background shade or not...
#ifdef DEBUG_BACKGROUND_UPDATE
				xlprintf(LOG_UPDATE_AND_REFRESH_LEVEL)( WIDE("-------------------------------------- Draw Background Image -------------------------------------") );
#endif
				BlotScaledImageSizedToAlpha( surface, current_page->background_image
					, 0, 0
					, surface->width, surface->height
					, ALPHA_TRANSPARENT );
			}
			else if( !current_page->background_color ) // otherwise it's transparent black. (dark clear)
			{
#ifdef DEBUG_BACKGROUND_UPDATE
				xlprintf(LOG_UPDATE_AND_REFRESH_LEVEL)( WIDE("-------------------------------------- Draw Background Color -------------------------------------") );
#endif
            if( canvas->flags.bEditMode )
					ClearImageTo( surface, 0x01000000 );//BASE_COLOR_BLACK );
				else
               ClearImage( surface );
			}
#ifdef DEBUG_BACKGROUND_UPDATE
			else
			{
				xlprintf(LOG_UPDATE_AND_REFRESH_LEVEL)( WIDE("-------------------------------------- NO Draw Background -------------------------------------") );
			}
#endif
		}
		else
			DebugBreak();

#ifndef USE_EDIT_GLARE
		if( canvas->flags.bEditMode )
		{
			DrawEditGlare( (PTRSZVAL)canvas, surface );
		}
#else
		if( canvas->flags.bEditMode && 0 )
		{
			// should use X to capture an image of the current menu config
			// then during editing we do not have to redraw all controls all the time..
			static Image x;
			//Image surface = GetFrameSurface( pf );
			//lprintf( ".... some kinda mystical update here" );
			//EnableFrameUpdates( pf, FALSE );
			//ReleaseCommonUse(pf);
			//EnableFrameUpdates( pf, TRUE );
			lprintf( ".... some kinda mystical update end here" );
			//lprintf( WIDE("Continue background") );
			if( x )
			{
				BlotScaledImageSizedToAlpha( surface, x
					, 0, 0
					, surface->width, surface->height
					, ALPHA_TRANSPARENT );
			}

			{
				PMENU_BUTTON button;
				INDEX idx;
				int nButton = 1;
				LIST_FORALL( current_page->controls, idx, PMENU_BUTTON, button )
				{
					long long x, y;
					long long w, h;
					char buttonname[128];
					//lprintf( WIDE("Drawing at %d,%d..."), button->x, button->y );
					x = (PARTX( button->x ) + 5); y = (PARTY( button->y ) + 5);
					w=(PARTW( button->x, button->w )+1 - 10);
					h=(PARTH( button->y, button->h )+1 - 10);
					if( x < 0 )
					{
						w += x;
						x = 0;
					}
					if( y < 0 )
					{
						h += y;
						y = 0;
					}
					//lprintf( "region is %Ld,%Ld %Ld,%Ld", x, y, w, h) ;
					BlatColorAlpha( surface, (S_32)x, (S_32)y, (_32)w, (_32)h
						, SetAlpha( BASE_COLOR_GREEN, 90 )
						);
					BlatColorAlpha( surface
						, PARTX(button->x), PARTY(button->y)
						, PARTW(button->x,1), PARTH(button->y,1)
						, SetAlpha( BASE_COLOR_RED, 90 ) );
					BlatColorAlpha( surface
						, PARTX(button->x), PARTY(button->y+button->h-1)
						, PARTW(button->x,1), PARTH(button->y+button->h-1,1)
						, SetAlpha( BASE_COLOR_RED, 90 ) );
					BlatColorAlpha( surface
						, PARTX(button->x+button->w-1), PARTY(button->y)
						, PARTW(button->x+button->w,1), PARTH(button->y,1)
						, SetAlpha( BASE_COLOR_RED, 90 ) );
					BlatColorAlpha( surface
						, PARTX(button->x+button->w-1), PARTY(button->y+button->h-1)
						, PARTW(button->x+button->w,1), PARTH(button->y+button->h-1,1)
						, SetAlpha( BASE_COLOR_RED, 90 ) );
					snprintf( buttonname, sizeof( buttonname ), WIDE("%s(%d)")
						, button->pTypeName
						, nButton++
						);
					PutString( surface
						, (S_32)x + 15, (S_32)y + 15, BASE_COLOR_WHITE, 0
						, buttonname );
				}
			}

			{
				int x, y;

				for( x = 0; x <= PARTSX; x++ )
				{
					//lprintf( WIDE("Line %d has %d/%d"), x, COMPUTEX(x,PARTSX), MODX( x, PARTSX ) );
					do_vlineAlpha( surface, PARTX(x), 0, surface->height, SetAlpha( BASE_COLOR_WHITE, 48 ) );
					//if( MODX( x, PARTSX ) > 1900 )
					//	do_vline( surface, COMPUTEX(x,PARTSX)+1, 0, surface->height, BASE_COLOR_DARKGREY );
				}

				for( y = 0; y <= PARTSY; y++ )
				{
					//lprintf( WIDE("Line %d has partial %d/%d"), y, COMPUTEY(y,PARTSY), MODY( y, PARTSY ) );
					do_hlineAlpha( surface, PARTY(y), 0, surface->width, SetAlpha( BASE_COLOR_WHITE, 48 ) );
					//if( MODX( y, PARTSY ) > 1800 )
					//	do_hline( surface, COMPUTEY(y,PARTSY)+1, 0, surface->width, BASE_COLOR_DARKGREY );
				}

				if( canvas->flags.selected )
				{
					//lprintf( WIDE("Our fancy coords could be %d,%d %d,%d"), PARTX( canvas->selection.x ), PARTY( canvas->selection.y )
					//		 , PARTW( canvas->selection.x, canvas->selection.w )
					//		 , PARTH( canvas->selection.y, canvas->selection.h ));
					// and to look really pretty select the outer edge on the bottom, also
					BlatColorAlpha( surface, PARTX( canvas->selection.x ), PARTY( canvas->selection.y )
						, PARTW( canvas->selection.x, canvas->selection.w )+1
						, PARTH( canvas->selection.y, canvas->selection.h )+1
						, canvas->flags.selecting
						?SetAlpha( ColorAverage( BASE_COLOR_BLUE, BASE_COLOR_WHITE, 50, 100 ), 170 )
						:SetAlpha( BASE_COLOR_BLUE, 120 )
						);
				}

			}
		}
#endif
	}
	//lprintf( "Done with a pass of drawing the canvas background... but the controls are still being revealed oh when?" );
	return TRUE;
}

//---------------------------------------------------------------------------

static int ProcessContextMenu( PCanvasData canvas, PSI_CONTROL pc, S_32 px, S_32 py )
{
			if( canvas->flags.bEditMode )
			{
				canvas->pCurrentControl = MouseInControl( canvas, px, py );
				if( canvas->pCurrentControl && !canvas->flags.bIgnoreKeys )
				{
#ifdef USE_EDIT_GLARE
					_32 result = TrackPopup( canvas->pControlMenu, canvas->edit_glare_frame );
					OwnMouse( canvas->edit_glare, TRUE );
#else
					_32 result = TrackPopup( canvas->pControlMenu, canvas->edit_glare_frame );
#endif
					if( g.flags.multi_edit )
					{
						PPAGE_DATA page = GetPageFromFrame( pc );
						canvas->current_page = page;
					}
					if( !ProcessPageMenuResult( pc, result ) ) switch( result )
					{
					case MNU_EDIT_BEHAVIORS:
						if( EditControlBehaviors )
						{
							BannerMessage( "EditControlBehaviors has been disabled" );
							/*
							 if( canvas->pCurrentControl->flags.bCustom )
							 EditControlBehaviors( canvas->pCurrentControl->control );
							 else
							 EditControlBehaviors( GetKeyCommon( canvas->pCurrentControl->key ) );
							 */
						}
						break;
					case MNU_CLONE:
						g.clonebutton = canvas->pCurrentControl;
						break;
					case MNU_COPY:
						g.clonebutton = canvas->pCurrentControl;
						InvokeCopyControl( canvas->pCurrentControl );
						break;
					case MNU_PASTE:
						g.clonebutton = canvas->pCurrentControl;
						InvokePasteControl( canvas->pCurrentControl );
						break;
					case MNU_EDIT_CONTROL:
						ConfigureKeyEx( canvas, canvas->pCurrentControl );
						break;
					case MNU_EDIT_CONTROL_COMMON:
						ConfigureKeyExx( canvas, canvas->pCurrentControl, FALSE, TRUE );
						break;
					case MNU_DESTROY_CONTROL:
						// first locate it I suppose...
						DestroyButton( canvas->pCurrentControl );
						canvas->pCurrentControl = NULL;
						break;
					}
				}
				else if( ( px >= canvas->selection.x && px < ( canvas->selection.x + canvas->selection.w ) )
						  && ( py >= canvas->selection.y && py < ( canvas->selection.y + canvas->selection.h ) )
						  && !canvas->flags.bIgnoreKeys )
				{
#ifdef USE_EDIT_GLARE
					_32 result = TrackPopup( g.pSelectionMenu, canvas->edit_glare_frame );
					OwnMouse( canvas->edit_glare, TRUE );
#else
					_32 result = TrackPopup( g.pSelectionMenu, canvas->pc_canvas );
#endif
					if( g.flags.multi_edit )
					{
						PPAGE_DATA page = GetPageFromFrame( pc );
						canvas->current_page = page;
					}
					if( !ProcessPageMenuResult( pc, result ) )
					{
						if( result >= MNU_CREATE_EXTRA && result <= MNU_CREATE_EXTRA_MAX )
						{
							char *name;
							// okay well... get the name from the menu item then?
							result -= MNU_CREATE_EXTRA;
							name = (char*)GetLink( &g.extra_types, result );
							CreateSomeControl( pc, canvas->selection.x, canvas->selection.y, canvas->selection.w, canvas->selection.h, name );
						}
						else switch( result )
						{
						case MNU_CREATE_ISSUE:
							CreateSomeControl( pc, canvas->selection.x, canvas->selection.y, canvas->selection.w, canvas->selection.h, WIDE("Paper Issue") );
							break;
						case MNU_CREATE_CONTROL:
							//CreateMenuControl( g.selection.x, g.selection.y, canvas->selection.w, canvas->selection.h, TRUE, FALSE);
							//SmudgeCommon( canvas->frame );
							break;
						case MNU_EXTRA_CONTROL:
							//CreateMenuControl( canvas->selection.x, canvas->selection.y, canvas->selection.w, canvas->selection.h, TRUE, FALSE);
							//SmudgeCommon( canvas->frame );
							break;
						}
						//SaveButtonConfig();
					}
				}
				else
				{

					if( canvas->flags.bSuperMode )
					{
						if( canvas->flags.bEditMode && !canvas->flags.bIgnoreKeys )
						{
#ifdef USE_EDIT_GLARE
							_32 result = TrackPopup( canvas->pEditMenu, canvas->edit_glare_frame );
							OwnMouse( canvas->edit_glare, TRUE );
#else
							_32 result = TrackPopup( canvas->pEditMenu, canvas->pc_canvas );
#endif
							if( g.flags.multi_edit )
							{
								PPAGE_DATA page = GetPageFromFrame( pc );
								canvas->current_page = page;
							}
							if( !ProcessPageMenuResult( pc, result ) ) switch( result )
							{
							case MNU_MAKE_CLONE:
								GetCloneButton( canvas, px, py, FALSE );
								break;
							case MNU_EDIT_GLARES:
								EditGlareSets( canvas->edit_glare_frame );
								break;
							case MNU_EDIT_FONTS:
								SelectAFont( canvas->edit_glare_frame, NULL );
								break;
							case MNU_PAGE_PROPERTIES:
								EditCurrentPageProperties(canvas->edit_glare_frame,canvas);
								break;
							case MNU_CREATE_PAGE:
								CreateNewPage(canvas->edit_glare_frame, canvas);
								break;
							case MNU_RENAME_PAGE:
								RenamePage( canvas->pc_canvas );
								break;
							case MNU_EDIT_DONE:
								AbortConfigureKeys( canvas->pc_canvas, 0 );
								break;
							default:
								Log1( WIDE("Unhandled menu option: %ld"), result );
								break;
							}

						}
						else if( !canvas->flags.bIgnoreKeys )
						{
#if 0
							// someday, finish this bit of code to handle right click
							// (with authentication) to edit configuration.
							_32 result = TrackPopup( canvas->pSuperMenu, g.frame );
							if( !ProcessPageMenuResult( pc, result ) ) switch( result )
							{
							case MNU_EDIT_SCREEN:
								canvas->flags.bEditMode = !canvas->flags.bEditMode;
								break;
							default:
								Log1( WIDE("Unhandled menu option: %ld"), result );
								break;
							}
#endif
						}
					}
				}
			}
         return 0;
}

static void MouseFirstDown( PCanvasData canvas, PTRSZVAL psv, S_32 px, S_32 py )
{
#ifdef USE_EDIT_GLARE
				OwnMouse( canvas->edit_glare, TRUE );
				SetDisplayNoMouse( canvas->edit_glare, FALSE );
#endif
				// click down left, first time....
				if( canvas->flags.selected && IsControlSelected( canvas, canvas->pCurrentControl ) )
				{
					canvas->flags.sizing_by_corner = DRAG_BEGIN;
#ifndef USE_EDIT_GLARE
					SmudgeCommon( canvas->pc_canvas );
#else
					DrawEditGlare( psv, canvas->edit_glare );
#endif
				}
				else
				{
					PMENU_BUTTON pmc;
					canvas->flags.selected = 0;
					canvas->flags.sizing_by_corner = NO_SIZE_OP;

					if( ( pmc = canvas->pCurrentControl ) )
					{
						if( ( px == pmc->x )
							&& ( py == pmc->y ) )
						{
							canvas->flags.sizing_by_corner = UPPER_LEFT;
						}
						else if( (px == pmc->x) && (py == (pmc->y+pmc->h-1)) )
						{
							canvas->flags.sizing_by_corner = LOWER_LEFT;
						}
						else if( (px == (pmc->x+pmc->w-1)) && (py==pmc->y ) )
						{
							canvas->flags.sizing_by_corner = UPPER_RIGHT;
						}
						else if( (px==(pmc->x+pmc->w-1)) && (py==(pmc->y+pmc->h-1 )) )
						{
							canvas->flags.sizing_by_corner = LOWER_RIGHT;
						}
						else
							canvas->flags.sizing_by_corner = DRAG_BEGIN;
#ifndef USE_EDIT_GLARE
						SmudgeCommon( canvas->pc_canvas );
#else
						DrawEditGlare( psv, canvas->edit_glare );
#endif
					}
				}
}

static void MouseDrag( PCanvasData canvas, PTRSZVAL psv
					  , S_32 px, S_32 py 
					  // , pad_left/right, pad top/button (sizing margins)
					  )
{
	/* if first drag */
				if( ( !canvas->flags.dragging 
					&& !canvas->flags.sizing ) 
					)
				{
					PMENU_BUTTON pmc;
					if( ( pmc = canvas->pCurrentControl ) )
					{
						if( canvas->flags.sizing_by_corner )
						{
							if( canvas->flags.sizing_by_corner == DRAG_BEGIN )
								canvas->flags.dragging = 1;
							else
								canvas->flags.sizing = 1;
							g._px = px;
							g._py = py;
#ifndef USE_EDIT_GLARE
							SmudgeCommon( canvas->pc_canvas );
#else
							DrawEditGlare( psv, canvas->edit_glare );
#endif
						}
					}
					else if( !canvas->flags.selecting )
					{
						canvas->flags.selected = 1;
						canvas->flags.selecting = 1;
						canvas->selection._x = canvas->selection.x = px;
						canvas->selection._y = canvas->selection.y = py;
						canvas->selection.w = 1;
						canvas->selection.h = 1;
#ifndef USE_EDIT_GLARE
						SmudgeCommon( canvas->pc_canvas );
#else
						DrawEditGlare( psv, canvas->edit_glare );
#endif
					}
				}
				
				{
									/* else - subsequent drag */
					if( canvas->flags.selecting )
					{
						int nx, ny, nw, nh;
						// kinda hard to explain why the subtract is -2...
						// but the cell which we start in is always selected,
						// and is one of the corners of the resulting rectangle.
						nx = canvas->selection._x;
						if( ( nw = (px - canvas->selection._x+1) ) <= 0 )
						{
							nx = canvas->selection._x + (nw-1);
							nw = -(nw-2);
						}
						ny = canvas->selection._y;
						if( ( nh = (py - canvas->selection._y+1) ) <= 0 )
						{
							ny = canvas->selection._y + (nh-1);
							nh = -(nh-2);
						}
						SelectItems( canvas, NULL, nx, ny, nw, nh );
						//if( IsSelectionValidEx( canvas, NULL, nx, ny, NULL, NULL, nw, nh ) )
						{
							canvas->selection.x = nx;
							canvas->selection.y = ny;
							canvas->selection.w = nw;
							canvas->selection.h = nh;
						}
						
						//lprintf( WIDE("And now our selection is %d,%d %d,%d") );
#ifndef USE_EDIT_GLARE
						SmudgeCommon( canvas->pc_canvas );
#else
						DrawEditGlare( psv, canvas->edit_glare );
#endif
						//UpdateCommon( pc );
						//SmudgeCommon( pc );
					}
					else if( canvas->flags.sizing || canvas->flags.dragging )
					{
						if( canvas->flags.sizing )
						{
							int dx = px - g._px
								, dy = py - g._py;
							//if( IsSelectionValidEx( canvas->frame
							//                       , canvas->pCurrentControl
							//							 , canvas->pCurrentControl->x
							//							 , canvas->pCurrentControl->y
							//  						 , &dx, &dy
							//							 , canvas->pCurrentControl->w
							//							 , canvas->pCurrentControl->h ) )
							{
retry:
								switch( canvas->flags.sizing_by_corner )
								{
								case UPPER_LEFT:
									if( ( (int)canvas->pCurrentControl->w - (int)dx ) <= 0 )
									{
										canvas->flags.sizing_by_corner = UPPER_RIGHT;
										goto retry;
									}
									if( ( (int)canvas->pCurrentControl->h - (int)dy ) <= 0 )
									{
										canvas->flags.sizing_by_corner = LOWER_LEFT;
										goto retry;
									}
									canvas->pCurrentControl->x += dx;
									canvas->pCurrentControl->w -= dx;
									canvas->pCurrentControl->y += dy;
									canvas->pCurrentControl->h -= dy;

									break;
								case UPPER_RIGHT:
									if( ( (int)canvas->pCurrentControl->w + (int)dx ) <= 0 )
									{
										canvas->flags.sizing_by_corner = UPPER_LEFT;
										goto retry;
									}
									if( ( (int)canvas->pCurrentControl->h - (int)dy ) <= 0 )
									{
										canvas->flags.sizing_by_corner = LOWER_RIGHT;
										goto retry;
									}
									canvas->pCurrentControl->w += dx;
									canvas->pCurrentControl->y += dy;
									canvas->pCurrentControl->h -= dy;
									break;
								case LOWER_LEFT:
									if( ( (int)canvas->pCurrentControl->w - (int)dx ) <= 0 )
									{
										canvas->flags.sizing_by_corner = LOWER_RIGHT;
										goto retry;
									}
									if( ( (int)canvas->pCurrentControl->h + (int)dy ) <= 0 )
									{
										canvas->flags.sizing_by_corner = UPPER_LEFT;
										goto retry;
									}
									canvas->pCurrentControl->x += dx;
									canvas->pCurrentControl->w -= dx;
									canvas->pCurrentControl->h += dy;
									break;
								case LOWER_RIGHT:
									if( ( (int)canvas->pCurrentControl->w + (int)dx ) <= 0 )
									{
										canvas->flags.sizing_by_corner = LOWER_LEFT;
										goto retry;
									}
									if( ( (int)canvas->pCurrentControl->h + (int)dy ) <= 0 )
									{
										canvas->flags.sizing_by_corner = UPPER_RIGHT;
										goto retry;
									}
									canvas->pCurrentControl->w += dx;
									canvas->pCurrentControl->h += dy;
									break;
								}
								canvas->pCurrentControl->flags.bMoved = 1;
								g._px += dx;
								g._py += dy;
							}
						}
						else if( canvas->flags.dragging )
						{
							if( canvas->flags.selected )
							{
								int dx = px - g._px
									, dy = py - g._py;
								if( ( dx || dy ) )
								{
									INDEX idx;
									PMENU_BUTTON item;
									LIST_FORALL( canvas->selected_list, idx, PMENU_BUTTON, item )
									{
										item->x += dx;
										item->y += dy;
										MoveCommon( QueryGetControl( item )
											, PARTX( item->x )
											, PARTY( item->y )
											);
									}
									g._px += dx;
									g._py += dy;
								}
#ifndef USE_EDIT_GLARE
								SmudgeCommon( canvas->pc_canvas );
#else
								DrawEditGlare( psv, canvas->edit_glare );
#endif
							}
							else
							{
								int dx = px - g._px
									, dy = py - g._py;
								if( ( dx || dy ) && canvas->pCurrentControl )
								{
									lprintf(" Moving control %d,%d", dx, dy );
									if( IsSelectionValidEx( canvas
										, canvas->pCurrentControl
										, (S_32)canvas->pCurrentControl->x
										, (S_32)canvas->pCurrentControl->y
										, &dx, &dy
										, (_32)canvas->pCurrentControl->w
										, (_32)canvas->pCurrentControl->h ) )
									{
										canvas->pCurrentControl->x += dx;
										canvas->pCurrentControl->y += dy;
										g._px += dx;
										g._py += dy;
										canvas->pCurrentControl->flags.bMoved = 1;
									}
								}
							}
						}

						if( canvas->pCurrentControl->flags.bMoved )
						{
							canvas->pCurrentControl->flags.bMoved = 0;
							MoveSizeCommon( QueryGetControl( canvas->pCurrentControl )
								, PARTX( canvas->pCurrentControl->x )
								, PARTY( canvas->pCurrentControl->y )
								, PARTW( canvas->pCurrentControl->x, canvas->pCurrentControl->w )
								, PARTH( canvas->pCurrentControl->y, canvas->pCurrentControl->h )
								);
#ifndef USE_EDIT_GLARE
							SmudgeCommon( canvas->pc_canvas );
#else
							DrawEditGlare( psv, canvas->edit_glare );
#endif
						}
					}
				}
}

static void MouseFirstRelease( PCanvasData canvas, PTRSZVAL psv )
{
		


				// first release after having been down until now.
				if( canvas->flags.sizing ||( canvas->flags.selecting )||( canvas->flags.dragging ))
				{
					canvas->flags.sizing_by_corner = NO_SIZE_OP;
					canvas->flags.selecting = 0;
					canvas->flags.sizing = 0;
					canvas->flags.dragging = 0;
#ifndef USE_EDIT_GLARE
					SmudgeCommon( canvas->pc_canvas );
#else
					DrawEditGlare( psv, canvas->edit_glare );
#endif
				}
				else
				{
				// first release after having been down until now.
					if( canvas->flags.sizing_by_corner)
					{
						canvas->flags.sizing_by_corner = NO_SIZE_OP;
#ifndef USE_EDIT_GLARE
						SmudgeCommon( canvas->pc_canvas );
#else
						DrawEditGlare( psv, canvas->edit_glare );
#endif
					}
					if( canvas->pCurrentControl && canvas->flags.bEditMode )
					{
						// got put in a current control.
						// and edit mode. 
						// did not begin selecting, sizeing or draging
						// pop up current configuration?
						if( canvas->pCurrentControl && canvas->flags.bEditMode )
						{
							// if not a custom control it's a fancy key-button
							//if( !canvas->pCurrentControl->flags.bCustom )
							//	ReleaseCommonUse( GetKeyCommon( canvas->pCurrentControl->key ) );
							ConfigureKeyEx( canvas, canvas->pCurrentControl );
						}
					}
				}
#ifdef USE_EDIT_GLARE
				OwnMouse( canvas->edit_glare, FALSE );
				SetDisplayNoMouse( canvas->edit_glare, FALSE );
#endif
}

int CPROC MouseEditGlare( PTRSZVAL psv, S_32 x, S_32 y, _32 b )
{
	PCanvasData canvas = (PCanvasData)psv;
   PSI_CONTROL pc = canvas->pc_canvas; // was the canvas mouse routine so...
	static _32 _b;
	static S_32 _x, _y;
	int px, py;
   lprintf( "Glare mouse %d %d %d", x, y, b );
#define PARTOFX(xc) ( ( xc ) * canvas->current_page->grid.nPartsX ) / canvas->width
#define PARTOFY(yc) ( ( yc ) * canvas->current_page->grid.nPartsY ) / canvas->height
	px = PARTOFX( x );
	py = PARTOFY( y );

	// shell mouse is frame mouse?
	//lprintf( WIDE("Shell mosue %d,%d %d"), x, y,  b );

	// current control will only be picked while mouse button up...
	if( !(b & MK_LBUTTON) )
	{
		canvas->pCurrentControl = MouseInControl( canvas, px, py );
	}



	if( canvas->flags.bEditMode )
	{
		if( b & MK_LBUTTON)
		{
			//lprintf( WIDE("left down.") );
			if( !(_b & MK_LBUTTON ) )
			{
				MouseFirstDown( canvas, psv, px, py );
			}
			else
			{
				// was down, is down....
				MouseDrag( canvas, psv, px, py );
			}
		}
		else
		{
			if( _b & MK_LBUTTON )
			{
				MouseFirstRelease( canvas, psv );
			}
			else
			{
				// was released, still released.
			}
		}
	}


	//-----------------------------------------------------
	///   Right button context menu stuff
	//-----------------------------------------------------
	//Log5( WIDE("Mouse event: %d,%d %x %x %x"), x, y, b, _b, MK_RBUTTON );
	if( b & MK_RBUTTON )
	{
		if( !(_b & MK_RBUTTON) )
		{
			if( ProcessContextMenu( canvas, pc, px, py ) )
				b &= ~MK_RBUTTON;
		}
	}

	_b = b;
	_x = x;
	_y = y;
	return 0;
}



//---------------------------------------------------------------------------

void CPROC QuitMenu( PSI_CONTROL pc, _32 keycodeUnused )
{
	//ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	lprintf( WIDE("!!!!!!!!!!! QUIT MENU !!!!!!!!!!!!!!!") );
	g.flags.bExit = 1;
	BannerTopNoWait( "Shutting down..." );
	InvokeInterShellShutdown();
	{
		//PMENU_BUTTON button;
		//INDEX idx;
		if( g.flags.multi_edit )
		{
			//PPAGE_DATA page;
			//INDEX idx;
			/*
			LIST_FORALL( g.pages, idx, PPAGE_DATA, page )
			{
			EnableFrameUpdates( page->frame, FALSE );
			}
			*/
		}
		else
			EnableFrameUpdates( g.single_frame, FALSE );
		//LIST_FORALL( canvas->current_page->controls, idx, PMENU_BUTTON, button )
		//{
		//   lprintf( WIDE("Destroy button...") );
		//   DestroyButton( button );
		//}
		// destroy frame here (in keyboard handler)
		// ends up hanging forever waiting for the destruction of self
		//DestroyFrame( &g.frame );
	}
	lprintf( WIDE("Waking thread...") );
	WakeThread( g.pMainThread );
}

//---------------------------------------------------------------------------

void CPROC RestartMenu( PTRSZVAL psv, _32 keycode )
{
	g.flags.bExit = 2;
	WakeThread( g.pMainThread );
}

//---------------------------------------------------------------------------

void CPROC ResumeMenu( PTRSZVAL psv, _32 keycode )
{
	g.flags.bExit = 3;
	WakeThread( g.pMainThread );
}

//---------------------------------------------------------------------------

void EndEditingPage( PSI_CONTROL pc_canvas, PPAGE_DATA page )
{
	PCanvasData canvas = GetCanvas( pc_canvas );
	EnableFrameUpdates( page->frame, FALSE );
	RestorePage( pc_canvas, canvas, canvas->current_page, TRUE );
	EnableFrameUpdates( page->frame, TRUE );
	SmudgeCommon( page->frame );
}

void CPROC AbortConfigureKeys( PSI_CONTROL pc, _32 keycode )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	if( canvas->flags.bEditMode )
	{
		lprintf( WIDE("Disallow frame updates now..") );
		canvas->flags.bEditMode = FALSE;

#ifdef USE_EDIT_GLARE
		HideDisplay( canvas->edit_glare );
#endif

      canvas->flags.selected = 0;
		SaveButtonConfig( g.single_frame, g.config_filename );
		EndEditingPage( pc, canvas->current_page );
		lprintf( WIDE("And having enabled them... then...") );
	}
}

void CPROC EventAbortConfigureKeys( PTRSZVAL psv, _32 keycode )
{
   AbortConfigureKeys( (PSI_CONTROL)psv, keycode );
}

//---------------------------------------------------------------------------

void BeginEditingPage( PPAGE_DATA page )
{
	PMENU_BUTTON button;
	INDEX idx;
	LIST_FORALL( page->controls, idx, PMENU_BUTTON, button )
	{
		HideCommon( QueryGetControl( button ) );
		InvokeEditBegin( button );
	}
}

void CPROC ConfigureKeys( PSI_CONTROL pc, _32 keycode )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	if( !canvas )
	{
		return;
	}
	// hide everything, turn on edit mode
	// which draws fake controls, so that
	// we can provide appropriate configuration
	// menus.
	canvas->flags.bEditMode = TRUE;
#ifdef USE_EDIT_GLARE

   lprintf( "Restore glare..." );
	RestoreDisplay( canvas->edit_glare );
   lprintf( "Glare restored..." );
	MakeTopmost( canvas->edit_glare );
#else
	EnableFrameUpdates( pc, FALSE );
	BeginEditingPage( canvas->current_page );
	EnableFrameUpdates( pc, TRUE );
	SmudgeCommon( pc );
#endif

}

void CPROC Exit( PTRSZVAL psv, PKEY_BUTTON key )
{
	exit(0);
}

void CPROC CheckMemStats( PTRSZVAL psv )
{
	static _32 _a, _b, _c, _d;
	_32 a, b,c, d;
#define f(a) ((_##a)==(a))
#define h(a) ((_##a)=(a))
	GetMemStats( &a, &b, &c, &d );
	if( f(a)||f(b)||f(c)||f(d))
	{
		h(a),h(b),h(c),h(d);
		lprintf( "---***--- MemStats  free: %"_32f" used: %"_32f" used chunks: %"_32f" free chunks: %"_32f"", a, b, c-d, d );
		if( _c != c )
		{
			_c = c;
			//DebugDumpMem();
		}
	}
#undef f
#undef h
}

INTERSHELL_PROC( void, SetButtonTextField )( PMENU_BUTTON pKey, PTEXT_PLACEMENT pField, char *text )
{
	SetKeyTextField( pKey->control.key, pField, text );
}
INTERSHELL_PROC( PTEXT_PLACEMENT, AddButtonLayout )( PMENU_BUTTON pKey, int x, int y, Font *font, CDATA color, _32 flags )
{
	return AddKeyLayout( pKey->control.key, x, y, font, color, flags );
}
INTERSHELL_PROC( PSI_CONTROL, InterShell_GetButtonControl )( PMENU_BUTTON button )
{
	return GetKeyCommon( button->control.key );
}


void GetPageSizeEx( PSI_CONTROL pc_canvas, P_32 width, P_32 height )
{
	if( !pc_canvas )
	{
		if( width )
			(*width) = g.width;
		if( height )
			(*height) = g.height;
	}
	else
	{
		ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc_canvas );
		if( canvas )
		{
			GetFrameSize( pc_canvas, (int*)width, (int*)height );
		}
	}
}

void GetPageSize( P_32 width, P_32 height )
{
	//return
	GetPageSizeEx( NULL, width, height );
}


void SetupSystemsListAndGlobalSingleFrame(void )
{
	//SystemLogTime( SYSLOG_TIME_DELTA|SYSLOG_TIME_CPU );
	//AddTimer( 15000, CheckMemStats, 0 );
	//Log( WIDE("Menu started\n") );

	if( !g.systems && g.flags.bSQLConfig )
	{
		CTEXTSTR *result;
		// get systems from SQL list right now
		// should be a small count...
		// maybe I don't want this sort of thing here at all?
#ifndef __NO_SQL__
		if( DoSQLRecordQuery( "select count(*) from systems", NULL, &result, NULL ) && result )
		{
			int n;
			g.systems = (struct system_info*)Allocate( sizeof( *g.systems ) * ( atoi( result[0] ) + 1 ) );
			g.systems[0].me = &g.systems;
			g.systems[0].name = SegCreateFromText( g.system_name );
			g.systems[0].ID = INVALID_INDEX;
			g.systems[0].next = NULL;

			for( n = 1, DoSQLRecordQuery( "select name from systems", NULL, &result, NULL );
				result;
				n++, GetSQLRecord( &result ) )
			{
				g.systems[n].name = SegCreateFromText( result[0] );
				g.systems[n].ID = INVALID_INDEX;
				if( n )
				{
					/* don't release this... will have to rework what this is... */
					g.systems[n].me = &g.systems[n-1].next;
					g.systems[n].next = NULL;
					g.systems[n-1].next = g.systems + n;
				}
			}
		}
		else
#endif
		{
			//int n;
			g.systems = (struct system_info*)Allocate( sizeof( *g.systems ) * ( 1 ) );
			g.systems[0].me = &g.systems;
			g.systems[0].name = SegCreateFromText( g.system_name );
			g.systems[0].ID = INVALID_INDEX;
			g.systems[0].next = NULL;
		}
	}
#ifdef USE_INTERFACES
	if( !SetControlInterface( g.pRenderInterface = GetDisplayInterface() ) )
	{
		Log( WIDE("Failed to set display interface...") );
		return;
	}
	if( !SetControlImageInterface( g.pImageInterface = GetImageInterface() ) )
	{
		Log( WIDE("Failed to set image interface...") );
		DropDisplayInterface( g.pRenderInterface );
		return;
	}
#endif
#ifndef __NO_OPTIONS__
	g.flags.multi_edit = SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Windowed mode (not full screen)", 0, TRUE );
#endif
	SetBlotMethod( BLOT_C );
	GetDisplaySize( &g.width, &g.height );
	if( g.flags.multi_edit )
	{
		g.width = g.width * 3 / 8;
		g.height = g.height * 3 / 8;
	}

	// always have at least 1 frame.  and it is a menu canvas
	InterShell_DisablePageUpdate( TRUE );

#ifndef __NO_OPTIONS__
#  ifndef __LINUX__
	{
		int display = SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Use Screen Number", 0, TRUE );
		if( display > 0 )
		{
			_32 w, h;
			S_32 x, y;
         GetDisplaySizeEx( display, &x, &y, &w, &h );
			g.single_frame = MakeControl( NULL, menu_surface.TypeID
												 , x, y
												 , w, h, 0 );
		}
	}
#  endif
	if( !g.single_frame )
		if( SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Use Custom Positioning", 0, TRUE ) )
		{
			int x = SACK_GetProfileInt( GetProgramName(), "Intershell Layout/X Position", 0 );
			int y = SACK_GetProfileInt( GetProgramName(), "Intershell Layout/Y Position", 0 );
			int _w = SACK_GetProfileInt( GetProgramName(), "Intershell Layout/Width", g.width );
			int _h = SACK_GetProfileInt( GetProgramName(), "Intershell Layout/Height", g.height );
			g.width = _w;
			g.height = _h;
			g.single_frame = MakeControl( NULL, menu_surface.TypeID, x, y, _w, _h, 0 );

		}
		else
			if( SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Use Second Display(horizontal)", 0, TRUE ) )
				g.single_frame = MakeControl( NULL, menu_surface.TypeID, g.width, 0, g.width, g.height, 0 );
			else
				if( g.flags.bSpanDisplay )
					g.single_frame = MakeControl( NULL, menu_surface.TypeID, 0, 0, g.width*2, g.height, 0 );
				else
#endif
					g.single_frame = MakeControl( NULL, menu_surface.TypeID, 0, 0, g.width, g.height, 0 );

	if( !g.flags.multi_edit )
	{
	}
	else
	{
		SetCommonText( g.single_frame, GetProgramName() );
		SetCommonBorder( g.single_frame, BORDER_NORMAL|BORDER_RESIZABLE );

		// this is going to have to be handled when the page
		// is created, or at some point later than this...
		//lprintf( "Not showing multiedit starting form." );
		//OpenPageFrame( canvas->default_page );
	}
}


//---------------------------------------------------------------------------
static INDEX iFocus = INVALID_INDEX;
void CPROC DoKeyRight( PSI_CONTROL pc, _32 key )
{
	PMENU_BUTTON button;
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	iFocus++;
	button = (PMENU_BUTTON)GetLink( &canvas->current_page->controls, iFocus );
	// should query (if can focus)
	if( button )
		SetCommonFocus( QueryGetControl( button ) );
}

void CPROC DoKeyLeft( PSI_CONTROL pc, _32 key )
{
	PMENU_BUTTON button;
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	if( iFocus )
		iFocus--;
	button = (PMENU_BUTTON)GetLink( &canvas->current_page->controls, iFocus );
	// should query (if can focus)
	if( button )
		SetCommonFocus( QueryGetControl( button ) );
}

void CPROC DoKeyUp( PSI_CONTROL pc, _32 key )
{
	PMENU_BUTTON button;
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	if( iFocus )
		iFocus--;
	button = (PMENU_BUTTON)GetLink( &canvas->current_page->controls, iFocus );
	// should query (if can focus)
	if( button )
		SetCommonFocus( QueryGetControl( button ) );
}

void CPROC DoKeyDown( PSI_CONTROL pc, _32 key )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	PMENU_BUTTON button;
	iFocus++;
	button = (PMENU_BUTTON)GetLink( &canvas->current_page->controls, iFocus );
	// should query (if can focus)
	if( button )
		SetCommonFocus( QueryGetControl( button ) );
}

//---------------------------------------------------------------------------
PMENU MakeControlsMenu( PMENU parent, char *basename, CTEXTSTR priorname )
{
	static int n = 0;
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	PMENU pExtraCreate = NULL;
	for( name = GetFirstRegisteredName( basename, &data );
		name;
		name = GetNextRegisteredName( &data ) )
	{
		PMENU submenu;
		if( priorname &&
			( ( strcmp( name, "button_create" ) == 0 ) ||
			( strcmp( name, "control_create" ) == 0 ) ||
			( strcmp( name, "listbox_create" ) == 0 ) ) )
		{
			// okay then add this one...
			//snprintf( newname, sizeof( newname ), WIDE("%s/%s"), basename, name );
			//if( NameHasBranches( &data ) )
			{
				// eat the first two parts - intershell/controls/
				// create the control name as that...
				char *controlpath = strchr( basename, '/' );
				if( controlpath )
				{
					controlpath++;
					controlpath = strchr( controlpath, '/' );
					if( controlpath )
						controlpath++;
				}

				AddLink( &g.extra_types, StrDup( controlpath ) );

				AppendPopupItem( parent, MF_STRING
					, MNU_CREATE_EXTRA + n
					, priorname );
				n++;
				break;
			}
		}
		else
		{
			if( NameHasBranches( &data ) )
			{
				char newname[256];
				if( !pExtraCreate )
					pExtraCreate = CreatePopup();
				snprintf( newname, sizeof( newname ), WIDE("%s/%s"), basename, name );
				submenu = MakeControlsMenu( pExtraCreate, newname, name );
				if( submenu )
					AppendPopupItem( pExtraCreate, MF_STRING|MF_POPUP
					, (PTRSZVAL)submenu
					, name );
			}
		}
	}
	return pExtraCreate;
}

//---------------------------------------------------------------------------

int CommonInitCanvas( PSI_CONTROL pc_canvas, PCanvasData canvas )
{
	if( !g.single_frame )
	{
		lprintf( "Half way through init, and now we set single_frame, could not wait for return to creator." );
		g.single_frame = pc_canvas;
	}

	g.flags.bExit = 0;

	if( !canvas->pPageMenu )
	{
		canvas->pPageMenu = CreatePopup();
		AppendPopupItem( canvas->pPageMenu, MF_STRING|MF_CHECKED
			, MNU_CHANGE_PAGE + ( canvas->default_page->ID = 0 )
			, WIDE("Fist Page") );
	}
	if( !canvas->pPageUndeleteMenu )
	{
		canvas->pPageUndeleteMenu = CreatePopup();
	}
	if( !canvas->pPageDestroyMenu )
	{
		canvas->pPageDestroyMenu = CreatePopup();
	}
	if( !g.pSelectionMenu )
	{
		g.pSelectionMenu = CreatePopup();
	}
	if( !canvas->pControlMenu )
	{
		canvas->pControlMenu = CreatePopup();
		AppendPopupItem( canvas->pControlMenu, MF_STRING, MNU_EDIT_CONTROL, WIDE("Edit") );
		AppendPopupItem( canvas->pControlMenu, MF_STRING, MNU_EDIT_CONTROL_COMMON, WIDE("Edit General") );
		AppendPopupItem( canvas->pControlMenu, MF_STRING, MNU_CLONE, WIDE("Clone") );
		if( EditControlBehaviors )
			AppendPopupItem( canvas->pControlMenu, MF_STRING, MNU_EDIT_BEHAVIORS, WIDE( "Edit Behaviors" ) );
		AppendPopupItem( canvas->pControlMenu, MF_STRING, MNU_DESTROY_CONTROL, WIDE("Destroy") );
	}

	if( !canvas->pSuperMenu )
	{
		canvas->pSuperMenu = CreatePopup();
		AppendPopupItem( canvas->pSuperMenu, MF_STRING|MF_UNCHECKED, MNU_EDIT_SCREEN, WIDE("Edit Screen") );
	}

	if( !canvas->pEditMenu )
	{
		canvas->pEditMenu = CreatePopup();
		AppendPopupItem( canvas->pEditMenu, MF_STRING, MNU_PAGE_PROPERTIES, WIDE("Properties") );
		if( !g.pGlobalPropertyMenu )
			g.pGlobalPropertyMenu = CreatePopup();
		AppendPopupItem( canvas->pEditMenu, MF_STRING|MF_POPUP, (PTRSZVAL)g.pGlobalPropertyMenu, WIDE( "Other Properties..." ) );
		AppendPopupItem( canvas->pEditMenu, MF_STRING, MNU_EDIT_FONTS, WIDE("Edit Fonts") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING, MNU_EDIT_GLARES, WIDE("Edit Button Glares") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING, MNU_CREATE_PAGE, WIDE("Create Page") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING, MNU_RENAME_PAGE, WIDE("Rename Page") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING, MNU_MAKE_CLONE, WIDE("Create Clone") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING|MF_POPUP, (PTRSZVAL)canvas->pPageMenu, WIDE("Change Page") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING|MF_POPUP, (PTRSZVAL)canvas->pPageDestroyMenu, WIDE("Destroy Page") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING|MF_POPUP, (PTRSZVAL)canvas->pPageUndeleteMenu, WIDE("Undestroy Page") );
		AppendPopupItem( canvas->pEditMenu, MF_STRING, MNU_EDIT_DONE, WIDE("Done") );
	}

	return 0;
}

//#if 0
static void OnHideCommon( WIDE( "menu canvas" ) )( PSI_CONTROL pc )
{
	//lprintf( "A control's hide has been invoked and that control is a cavnas... hide controls on my page." );
	HidePageEx( pc );
}

static void OnRevealCommon( WIDE( "menu canvas" ) )( PSI_CONTROL pc )
{
	//lprintf( "Restoring page..." );
	RestoreCurrentPage( pc );
	//lprintf( "restored page..." );
}
//#endif

void CPROC AcceptFiles( PSI_CONTROL pc, CTEXTSTR file, S_32 x, S_32 y )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );

	static int bInvoked;
   int px, py;
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	if( bInvoked )
		return;
	bInvoked = TRUE;

//#define PARTOFX(xc) ( ( xc ) * canvas->current_page->grid.nPartsX ) / canvas->width
//#define PARTOFY(yc) ( ( yc ) * canvas->current_page->grid.nPartsY ) / canvas->height
	px = PARTOFX( x );
	py = PARTOFY( y );
					
	for( name = GetFirstRegisteredName( TASK_PREFIX "/common/Drop Accept", &data );
		name;
		name = GetNextRegisteredName( &data ) )
	{
		LOGICAL (CPROC *f)(CTEXTSTR, int,int);
		//snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/common/save common/%s", name );
		f = GetRegisteredProcedure2( (CTEXTSTR)data, LOGICAL, name, (CTEXTSTR, int,int) );
		if( f )
			if( f(file,px,py) )
            break;
	}
   bInvoked = FALSE;
}

int CPROC InitMasterFrame( PCOMMON pc )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	SetCommonTransparent( pc, FALSE );
	{
		//PSI_CONTROL parent;
		//parent = GetFrame( pc );
		//if( parent )
		{
			int displays_wide = SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Expected displays wide", 1, TRUE );
			int displays_high = SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Expected displays highs", 1, TRUE );
			Image surface = GetControlSurface( pc );
			canvas->pc_canvas = pc; // self reference
			canvas->width = surface->width;
			canvas->height = surface->height;
			canvas->current_page =  (PPAGE_DATA)Allocate( sizeof( *canvas->current_page ) );
			MemSet( canvas->current_page, 0, sizeof( *canvas->current_page ) );
         canvas->current_page->flags.bActive = 1;
			canvas->width_scale.denominator = displays_wide * 1024;
			canvas->width_scale.numerator =  surface->width;
			canvas->height_scale.denominator = displays_high * 768;
			canvas->height_scale.numerator = surface->height;
			// current page is set to default page here (usually)
         // this sets the initial page with no config to 40x40 squares
			canvas->current_page->grid.nPartsX = 
				canvas->nPartsX =  displays_wide * 40;
			canvas->current_page->grid.nPartsY = 
				canvas->nPartsY = displays_high * 40;
			canvas->current_page->frame = canvas->pc_canvas;
			// really this should have been done the other way...
			canvas->default_page =  canvas->current_page;

			canvas->flags.bSuperMode = 1;
			AddLink( &g.frames, pc );
			CommonInitCanvas( pc, canvas );
		}
      AddCommonAcceptDroppedFiles( pc, AcceptFiles );
		//else
		//	canvas->current_page = parent->current_page;
	}
	return 1;
}

int CPROC PageFocusChanged( PSI_CONTROL pc, LOGICAL bFocused )
{
	ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, pc );
	if( g.flags.multi_edit )
	{
		if( bFocused )
		{
			canvas->current_page = GetPageFromFrame( pc );
			if( canvas->current_page )
				lprintf( "*** New current page: %s", canvas->current_page->title?canvas->current_page->title:"[StartupPage]" );
			else
				lprintf( "*** FAULT not a frame of a page..." );
			//g.single_frame = pc;
		}
		//canvas->frame = pc;
	}
	return 1;
}

void CPROC GoodQuitMenu( PTRSZVAL psvUnused, _32 keycodeUnused )
{
	BannerNoWait( WIDE("Exiting...") );
	if( g.flags.bTerminateStayResident )
	{
		BAG_Exit(0xd1e);
	}
	else
	{
		g.flags.bExit = 1;
		WakeThread( g.pMainThread );
	}
}

void CPROC DoConfigureKeys( PTRSZVAL psv, _32 keycodeUnused )
{
	if( !g.flags.bNoEdit )
		ConfigureKeys( g.single_frame, keycodeUnused );
}

int CPROC ShellKey( PSI_CONTROL pc, _32 key )
{
	if(( (key & (KEY_ALT_DOWN|KEY_SHIFT_DOWN))==(KEY_ALT_DOWN|KEY_SHIFT_DOWN) )
		||( (key & (KEY_ALT_DOWN))==(KEY_ALT_DOWN) )
		)
	{
		if( !g.flags.bNoEdit )
		{
			if( ( KEY_CODE( key ) == KEY_C ) )
			{
				ConfigureKeys( pc, key );
				return TRUE;
			}
		}
		//		if( ( KEY_CODE( key ) == KEY_X ) )
		//		{
		//			GoodQuitMenu( pc, key );
		//			return TRUE;
		//		}
	}
	if( KEY_CODE( key ) == KEY_ESCAPE )
	{
		AbortConfigureKeys( pc, key );
		return TRUE;
	}
	switch( KEY_CODE(key) )
	{
	case KEY_LEFT:
		DoKeyLeft( pc, key );
		break;
	case KEY_RIGHT:
		DoKeyRight( pc, key );
		break;
	case KEY_UP:
		DoKeyUp( pc, key );
		break;
	case KEY_DOWN:
		DoKeyDown( pc, key );
		break;
	}
	return 0;
}

CONTROL_REGISTRATION menu_surface = { "Menu Canvas"
, { { 512, 460 }, sizeof( CanvasData ), BORDER_WANTMOUSE|BORDER_NONE|BORDER_NOMOVE|BORDER_FIXED }
, InitMasterFrame
, NULL
, DrawFrameBackground
, ShellMouse
, ShellKey
, NULL, NULL,NULL,NULL,NULL,NULL
, PageFocusChanged
};
CONTROL_REGISTRATION menu_edit_glare = { "Edit Glare"
, { { 512, 460 }, 0, BORDER_WANTMOUSE|BORDER_NONE|BORDER_NOMOVE|BORDER_FIXED }
, NULL //InitMasterFrame
, NULL
, NULL //DrawFrameBackground
, NULL //ShellMouse
, NULL //ShellKey
, NULL, NULL,NULL,NULL,NULL,NULL
, NULL 
};

PRELOAD( RegisterMenuSurface ) { 
	DoRegisterControl( &menu_surface ); 
	DoRegisterControl( &menu_edit_glare );
}

/* opens a frame per page.... */
PSI_CONTROL OpenPageFrame( PPAGE_DATA page )
{
	// multi edit uses this...
	if( !page->frame )
	{
		static _32 xofs, yofs;
		PSI_CONTROL page_frame;
		page_frame = MakeControl( NULL, menu_surface.TypeID, xofs, yofs, g.width, g.height, g.flags.multi_edit?BORDER_NORMAL:0 );
		{
			ValidatedControlData( PCanvasData, menu_surface.TypeID, canvas, page_frame );
			xofs += 25;
			yofs += 25;
			//SetCommonUserData( page_frame, (PTRSZVAL)page );
			SetCommonText( page_frame, page->title?page->title:"Default Page" );
			SetCommonBorder( page_frame, BORDER_NORMAL );

			{
				//PRENDERER renderer;
				Image image = GetControlSurface( page_frame );
				canvas->renderer = OpenDisplaySizedAt( g.flags.bTransparent?DISPLAY_ATTRIBUTE_LAYERED:0
																 , image->width, image->height, image->x, image->y );
            SetRendererTitle( canvas->renderer, "Canvas" );
				AttachFrameToRenderer( page_frame, canvas->renderer );
			}
			DisplayFrame( page_frame );
#if 1
			InitSpriteEngine(  );
#endif
			BindEventToKey( NULL /*GetFrameRenderer( page_frame )*/, KEY_X, KEY_MOD_ALT, GoodQuitMenu, 0 );
			BindEventToKey( NULL /*GetFrameRenderer( page_frame )*/, KEY_C, KEY_MOD_ALT, DoConfigureKeys, 0 );
			BindEventToKey( NULL /*GetFrameRenderer( page_frame )*/, KEY_F4, KEY_MOD_ALT, GoodQuitMenu, 0 );
			{
				PRENDERER renderer;
				BindEventToKey( renderer = GetFrameRenderer( page_frame ), KEY_X, KEY_MOD_ALT, GoodQuitMenu, 0 );
				BindEventToKey( renderer, KEY_F4, KEY_MOD_ALT, GoodQuitMenu, 0 );
#ifdef __WINDOWS__
				(((PRENDERER)g.mem_lock)[0]) = renderer[0];
#endif
			}
			//canvas->page = page;
			page->frame = page_frame;
			lprintf( "Page %p is frame %p", page, page_frame );
			canvas->flags.bSuperMode = TRUE;
			return page_frame;
		}
	}
	return page->frame;
}    

void InvokeFinishInit( void );

int Init( LOGICAL bLoadConfig )
{
	SetupSystemsListAndGlobalSingleFrame();

	// Load the previous configuration file...
	// and make the controls found therein...
	{
		PGLARE_SET glare_set;
		if( bLoadConfig )
		{
			BannerNoWait( WIDE("Read config...") );
			LoadButtonConfig( g.single_frame, g.config_filename );
		}
		//g.button_space = 25;
		// default images...
		MakeGlareSet( WIDE("DEFAULT"), NULL, NULL, NULL, NULL );
		//SetGlareSetFlags( WIDE("DEFAULT"), 0 );
		if( !( glare_set = CheckGlareSet( WIDE( "round" ) ) ) ||
			!( glare_set->mask || glare_set->up ||glare_set->down ||glare_set->glare )
			)
		{
			MakeGlareSet( WIDE("round"), NULL
				, WIDE("images/round_ridge_up.png")
				, WIDE("images/round_ridge_down.png")
				, WIDE("images/round_mask.png") );
			SetGlareSetFlags( WIDE("roune"), GLARE_FLAG_SHADE );
		}
		if( !( glare_set = CheckGlareSet( WIDE( "square" ) ) )||
			!( glare_set->mask || glare_set->up ||glare_set->down ||glare_set->glare ) )
		{
			MakeGlareSet( WIDE("square")
				, WIDE("images/glare.jpg")
				, WIDE("images/ridge_up.png")
				, WIDE("images/ridge_down.png")
				, WIDE("images/square_mask.png") );
			SetGlareSetFlags( WIDE("round"), GLARE_FLAG_SHADE );
		}
		if( !( glare_set = CheckGlareSet( WIDE("bicolor square") ) )||
			!( glare_set->mask || glare_set->up ||glare_set->down ||glare_set->glare ) )
		{
			MakeGlareSet( WIDE("bicolor square")
				, NULL
				, WIDE("images/defaultLens.png")
				, WIDE("images/pressedLens.png")
				, WIDE("images/colorLayer.png") );
			SetGlareSetFlags( WIDE("bicolor square"), GLARE_FLAG_MULTISHADE );
		}

		if( bLoadConfig )
		{
			BannerNoWait( WIDE("Finish Config...") );
			InvokeFinishInit();
		}
		if( g.flags.bLogNames )
		{
			DumpRegisteredNames();
		}

		g.flags.bInitFinished = 1;
#ifdef DEBUG_BACKGROUND_UPDATE
		xlprintf(LOG_UPDATE_AND_REFRESH_LEVEL)( WIDE("-------------------------------------- BEGIN CHANGE PAGE ----------------------------------------------") );
#endif
	}
	return 0;
}


ATEXIT_PRIORITY( ExitMisc, ATEXIT_PRIORITY_DEFAULT + 1 )
{
	// not sure how we could be here without exit having
	// been set - perhaps a signal exit?
   InvokeInterShellShutdown();
	{
		PMENU_BUTTON button;
		INDEX idx;
		PPAGE_DATA page;
		INDEX idx2;
		if( !g.flags.multi_edit )
		{
			if( SACK_GetProfileIntEx( GetProgramName(), "Destroy Controls at exit", 0, TRUE ) )
			{
				InterShell_DisablePageUpdate( TRUE );
				LIST_FORALL( g.all_pages, idx2, PPAGE_DATA, page )
				{
					ChangePages( g.single_frame, page );
					LIST_FORALL( page->controls, idx, PMENU_BUTTON, button )
					{
						DestroyButton( button );
					}
					if( page->frame != g.single_frame )
					{
						DestroyFrame( &page->frame );
					}
					else
						page->frame = NULL;
				}
				InterShell_DisablePageUpdate( TRUE );
				//DestroyFrame( &g.single_frame );
			}
		}
		else
		{
			// destory g.frames
		}
		// Destroy popups...
	}
	if( !g.flags.bExit )
	{
		g.flags.bExit = 1;
		WakeThread( g.pMainThread );
	}
}

OnKeyPressEvent( WIDE("Quit POS") )( PTRSZVAL psv )
//void CPROC QuitPOS( PTRSZVAL psv, PKEY_BUTTON key )
{
	BannerNoWait( WIDE("Exiting...") );
#ifdef __cplusplus_cli
	//Application::Exit();
#endif
	if( g.flags.bTerminateStayResident )
	{
		BAG_Exit(0xd1e);
	}
	else
	{
		g.flags.bExit = 1;
		WakeThread( g.pMainThread );
	}
}

OnKeyPressEvent( WIDE("InterShell/Show Names" ) )( PTRSZVAL psv )
//void CPROC QuitPOS( PTRSZVAL psv, PKEY_BUTTON key )
{
	DumpRegisteredNames();
}
OnCreateMenuButton( WIDE("InterShell/Show Names") )( PMENU_BUTTON button )
{
	//SetKeyPressEvent( button->control.key , QuitPOS, 0 );
	button->color = BASE_COLOR_ORANGE;
	button->text = StrDup( WIDE("Show_Names") );
	button->secondary_color = BASE_COLOR_BLACK;
	button->glare_set = GetGlareSet( WIDE("bicolor square") );
	return 1;
}
OnCreateMenuButton( WIDE("Quit POS") )( PMENU_BUTTON button )
{
	//SetKeyPressEvent( button->control.key , QuitPOS, 0 );
	button->color = BASE_COLOR_RED;
	button->text = StrDup( WIDE("Quit") );
	button->secondary_color = BASE_COLOR_BLACK;
	button->glare_set = GetGlareSet( WIDE("bicolor square") );
	return 1;
}

void InvokeFinishAllInit( void )
{
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	for( name = GetFirstRegisteredName( TASK_PREFIX "/common/finish all init", &data );
		name;
		name = GetNextRegisteredName( &data ) )
	{
		void (CPROC*f)(void);
		f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (void) );
		if( f )
			f();
	}
}

void InvokeFinishInit( void )
{
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	//DumpRegisteredNames();
	for( name = GetFirstRegisteredName( TASK_PREFIX "/common/finish init", &data );
		name;
		name = GetNextRegisteredName( &data ) )
	{
		void (CPROC*f)(void);
		//snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/common/save common/%s", name );
		f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (void) );
		if( f )
			f();
	}
	if( g.pSelectionMenu )
	{
		PMENU pExtraCreate;
		pExtraCreate = MakeControlsMenu( g.pSelectionMenu, TASK_PREFIX "/control", NULL );
		//AppendPopupItem( canvas->pSelectionMenu, MF_STRING|MF_POPUP, (PTRSZVAL)canvas->pPageMenu, WIDE("Change Page") );
		//AppendPopupItem( canvas->pSelectionMenu, MF_STRING|MF_POPUP, (PTRSZVAL)canvas->pPageDestroyMenu, WIDE("Destroy Page") );
		//AppendPopupItem( canvas->pSelectionMenu, MF_STRING|MF_POPUP, (PTRSZVAL)canvas->pPageUndeleteMenu, WIDE("Undestroy Page") );
		AppendPopupItem( g.pSelectionMenu, MF_STRING|MF_POPUP, (PTRSZVAL)pExtraCreate, WIDE("Create other...") );
	}
	// additional global properties might ahve been registered...
	if( !g.global_properties )
	{
		int n = 0;
		CTEXTSTR name;
		PCLASSROOT data = NULL;
		for( name = GetFirstRegisteredName( TASK_PREFIX "/common/global properties", &data );
			name;
			name = GetNextRegisteredName( &data ) )
		{
			void (CPROC*f)(PSI_CONTROL);
			f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PSI_CONTROL) );
			if( f )
			{
				SetLink( &g.global_properties, n, f );
				SetLink( &g.global_property_names, n, StrDup( name ) );
				n++;
			}
		}
	}
	// additional global security modules might ahve been registered...
	if( !g.security_property_names )
	{
		CTEXTSTR name;
		PCLASSROOT data = NULL;
		for( name = GetFirstRegisteredName( TASK_PREFIX "/common/Edit Security", &data );
			name;
			name = GetNextRegisteredName( &data ) )
		{
			void (CPROC*f)(PSI_CONTROL);
			f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PSI_CONTROL) );
			if( f )
			{
				AddLink( &g.security_property_names, StrDup( name ) );
			}
		}
	}
	{
		INDEX idx;
		CTEXTSTR name;
		if( !g.pGlobalPropertyMenu )
			g.pGlobalPropertyMenu = CreatePopup();
		LIST_FORALL( g.global_property_names, idx, CTEXTSTR, name )
			AppendPopupItem( g.pGlobalPropertyMenu, MF_STRING, idx + MNU_GLOBAL_PROPERTIES, name );
	}
}

#ifdef USE_EDIT_GLARE
void AddGlareLayer( PCanvasData canvas, Image image )
{
	if( !canvas->edit_glare )
	{
		// must support layered windows.
    // somehow?
		canvas->edit_glare = OpenDisplaySizedAt( DISPLAY_ATTRIBUTE_LAYERED
															 //| DISPLAY_ATTRIBUTE_CHILD
															, image->width, image->height
															, image->x, image->y );
      lprintf( "glare is %p (%p)", canvas->edit_glare, GetNativeHandle( canvas->edit_glare ) );
		SetRendererTitle( canvas->edit_glare, "Edit Glare" );
		canvas->edit_glare_frame = MakeNamedControl( NULL, "Edit Glare", image->x, image->y, image->width, image->height, -1 );
		BindEventToKey( canvas->edit_glare, KEY_ESCAPE, 0, EventAbortConfigureKeys, (PTRSZVAL)canvas->pc_canvas );
		AttachFrameToRenderer( canvas->edit_glare_frame, canvas->edit_glare );
		SetRedrawHandler( canvas->edit_glare, DrawEditGlare, (PTRSZVAL)canvas );
		SetMouseHandler( canvas->edit_glare, MouseEditGlare, (PTRSZVAL)canvas );
		//lprintf( "MAKE TOPMOST" );
		MakeTopmost( canvas->edit_glare );
		//lprintf( "Is it topmost?" );
		// just create that thing, don't show it yet...
		//
	}
}
#endif

int restart( void )
{
   static int first_restart = 1;
	Init( TRUE );
#ifdef DEBUG_BACKGROUND_UPDATE
	xlprintf(LOG_UPDATE_AND_REFRESH_LEVEL)( WIDE("Displaying the frame on the real display...") );
#endif
	InvokeStartupMacro();
	BannerNoWait( WIDE("and we go...") );
#ifndef __NO_SQL__
	SQLSetFeedbackHandler( NULL );
#endif
	// have at least this one.
	if( !g.flags.multi_edit )
	{
		PCanvasData canvas = GetCanvas( g.single_frame );
		if( canvas )
		{
			//lprintf( "Making sure we start from NO Page, in case the first page is protected..." );
			//lprintf( "Should we have a rule for default forward, backward, first, last?  From NULL? from another?" );
			HidePageEx( g.single_frame );
			canvas->current_page = NULL;
			ChangePages( g.single_frame, canvas->default_page );
		}
	do
	{

		InterShell_DisablePageUpdate( FALSE );
		{
			//PRENDERER renderer;
			Image image = GetControlSurface( g.single_frame );
			PRENDERER banner_rend = GetBannerRenderer( NULL );
			canvas->renderer = OpenDisplayUnderSizedAt( banner_rend
				, g.flags.bTransparent?DISPLAY_ATTRIBUTE_LAYERED:0
				, image->width, image->height
				, image->x, image->y );
			SetRendererTitle( canvas->renderer, "Canvas Behind Banner" );
#ifdef __WINDOWS__
			(((PRENDERER)g.mem_lock)[0]) = canvas->renderer[0];
#endif

#ifdef USE_EDIT_GLARE
			AddGlareLayer( canvas, image );
#endif
			AttachFrameToRenderer( g.single_frame, canvas->renderer );
		}
		DisplayFrame( g.single_frame );
		// wow... how tedious... display does 'reveal' but reveal won't smudge if not hidden

#ifndef __NO_ANIMATION__
		InitSpriteEngine( );
		PlayAnimation( g.single_frame );
#endif

		BindEventToKey( NULL, KEY_C, KEY_MOD_ALT, DoConfigureKeys, 0 );
		BindEventToKey( GetFrameRenderer( g.single_frame ), KEY_X, KEY_MOD_ALT, GoodQuitMenu, 0 );
		BindEventToKey( GetFrameRenderer( g.single_frame ), KEY_F4, KEY_MOD_ALT, GoodQuitMenu, 0 );
		if( g.flags.bTopmost )
			MakeTopmost( GetFrameRenderer( g.single_frame ) );
      WakeableSleep( 250 );
		{
			PBANNER banner = NULL;
			// this has to wait... until... 
			// the first rendering pass is done... cause we're behind it...
			lprintf( " ---------- remove banner --------" ) ;// 
			RemoveBannerEx( &banner DBG_SRC );
		}
		if( first_restart )
		{
         first_restart = 0;
			InvokeFinishAllInit();
		}
#if 0
		while( !HasFocus( GetFrameRenderer( g.single_frame ) ) )
		{
         SyncRender( GetFrameRenderer( g.single_frame ) );
			ForceDisplayFront( GetFrameRenderer( g.single_frame ) );
			ForceDisplayFocus( GetFrameRenderer( g.single_frame ) );
		}
#endif
		if( !g.flags.bTerminateStayResident )
			while( !g.flags.bExit )
			{
				//ProcessControlMessages();					 
				lprintf("Waiting to exit..." );
				WakeableSleep( 10000 );
				//Relinquish();
			}
	} while( g.flags.bExit == 3 );
	}

	// exit = 1 == restart

	if( !g.flags.bTerminateStayResident && ( g.flags.bExit != 1 ) )
	{
		// This is for restart, destroyed everything we know and are
		// then reloads the configuration from the top.
		if( !g.flags.multi_edit )
		{
			//PMENU_BUTTON button;
			//INDEX idx;
			//EnableFrameUpdates( g.frame, FALSE );
			//LIST_FORALL( g.current_page->controls, idx, PMENU_BUTTON, button )
			//{
			//	DestroyButton( button );
			//}
			xlprintf(LOG_ALWAYS)( "!!!\n!!!\n!!!! Cleanup NEEDS to be finished!!! \n!!!\n!!!\n" );
			//DestroyFrame( &g.frame );
		}
	}
	return g.flags.bExit;
}

void InterShell_SetButtonHighlight( PMENU_BUTTON button, LOGICAL bEnable )
{
   button = InterShell_GetPhysicalButton( button );
   if( !button->flags.bCustom )
		SetKeyHighlight( button->control.key, bEnable );
}

static void CPROC HandleSQLFeedback( CTEXTSTR message )
{
	lprintf( WIDE("SQLMessage %s"), message );
	BannerNoWait( message );
}

PRIORITY_PRELOAD( LoadingMessage, DEFAULT_PRELOAD_PRIORITY+3 )
{
	BannerNoWait( WIDE("Loading...") );
	//void SQLSetFeedbackHandler( void (CPROC*HandleSQLFeedback*)(char *message) );
#ifndef __NO_SQL__
	SQLSetFeedbackHandler( HandleSQLFeedback );
#endif
}

#if defined( __WINDOWS__ )
PRIORITY_PRELOAD( ProgramLock, DEFAULT_PRELOAD_PRIORITY+2 )
{
	PTRSZVAL size = 0;
	char lockname[256];
	TEXTCHAR resource_path[256];
	snprintf( lockname, sizeof( lockname ), "%s.instance.lock", GetProgramName() );
#ifndef __NO_OPTIONS__
	g.flags.bSQLConfig = SACK_GetProfileIntEx( GetProgramName(), "Use SQL Configuration", 1, TRUE );
	SACK_GetProfileStringEx( "InterShell", "Default resource path"
								, ""
								, resource_path
								  , sizeof( resource_path ), TRUE );
	SACK_GetProfileStringEx( GetProgramName(), "resource path"
#ifdef __LINUX__
							 , resource_path[0]?resource_path:"~"
#else
							 , resource_path[0]?resource_path:"../resources"
#endif
							 , resource_path
							 , sizeof( resource_path ), TRUE );
#ifdef _WIN32
	if( !SetCurrentPath( resource_path ) )
	{
		MakePath( resource_path );
		SetCurrentPath( resource_path );
	}
#else
	if( !SetCurrentPath( resource_path ) )
	{
		fprintf( stderr, WIDE("Please create %s....\n"), resource_path );
	}
#endif

#endif

#ifndef __LINUX__
	g.mem_lock = OpenSpace( lockname
		, NULL
		//, WIDE("memory.delete")
		, &size );
	if( g.mem_lock )
	{
#ifdef __WINDOWS__
		PRENDER_INTERFACE pri = GetDisplayInterface();
		pri->_ForceDisplayFront( (PRENDERER)g.mem_lock );
#endif

		lprintf( WIDE("Menu already running, now exiting.") );
		exit(0);
		Release( g.mem_lock );
	}
	size = 4096;
	// defined by the make system - TARGETNAME
	// only one of a single version of this program?
	g.mem_lock = OpenSpace( lockname
		, NULL
		//, WIDE("memory.delete")
		, &size );
	if( !g.mem_lock )
	{
		lprintf( WIDE("Failed to create instance lock region.") );
		exit(0);
	}
#endif
}
#endif
void CPROC LoadAPlugin( PTRSZVAL psv, CTEXTSTR name, int flags )
{
	char msg[256];
	snprintf( msg, sizeof( msg ), "Loading Plugin: %s", name );
	SystemLog( msg );
	snprintf( msg, sizeof( msg ), "Loading Plugin...\n%s", name );
	BannerNoWait( msg );
	if( pathchr( name ) )
	{
		// if loading a plugin from a path, add that path to the PATH
		char *tmp = StrDup( name );
		CTEXTSTR old_environ = OSALOT_GetEnvironmentVariable( "PATH" );
		// safe conversion.
		char *trim = (char*)pathrchr( tmp );
		trim[0] = 0;
		if( !strstr( old_environ, tmp ) )
		{
			OSALOT_PrependEnvironmentVariable( "PATH", tmp );
		}
	}
	LoadFunction( name, NULL );
	snprintf( msg, sizeof( msg ), "Loaded Plugin: %s", name );
	SystemLog( msg );
	snprintf( msg, sizeof( msg ), "Loaded Plugin...\n%s", name );
	BannerNoWait( msg );
}

void LoadInterShellPlugins( CTEXTSTR mypath, CTEXTSTR mask )
{
	TEXTCHAR filename[256];
	POINTER info = NULL;
	int bLocalPath = 0;
	char *ext;
	if( !mypath )
	{
		mypath = StrDup( getenv( WIDE("MY_LOAD_PATH") ) );
		bLocalPath = TRUE;
	}
	lprintf( "Read line from file: %s", mask );
	if( !(mask[0] == '/') && !(mask[0] == '\\') && !(mask[1] == ':' ) )
		snprintf( filename, sizeof( filename ), "%s/%s", mypath, mask );
	else
		snprintf( filename, sizeof( filename ), "%s", mask );

	// save conversion
	ext = (char*)pathrchr( filename );
	if( ext )
		ext[0] = 0;
	else
	{
		ext = filename;
		strcpy( filename, "." );
	}
	lprintf( "Scanning as [%s] [%s]", filename, ext+1 );
	while( ScanFiles( filename, ext+1, &info, LoadAPlugin, 0, 0 ) );
	if( bLocalPath )
		Release( (POINTER)mypath );

}

// this should run after most everything else that could be loaded...
// At one point in time this was hard coded as 75
PRIORITY_PRELOAD( LoadExtra, DEADSTART_PRELOAD_PRIORITY + 5 ) // low priority... lower than most should think of.
{
	{
		char buf[256];
		// this is done before main... preloaded plugins were the standard once upon a time.
		snprintf( buf, sizeof( buf ), "%s.config", GetProgramName() );
		g.config_filename = StrDup( buf );
		//g.config_filename = "intershell.config";
	}
}


#ifdef DEKWARE_PLUGIN
PTRSZVAL CPROC MenuThread( PTHREAD thread )
{
	char *argv[] = { NULL };
	int argc = 1;
#else
//int main( int argc, char **argv )
//{
//PUBLIC( int, Start)( void )
PUBLIC( int, Main)( int argc, char **argv, int bConsole )
{
	//char *argv[] = { NULL };
	//int argc = 1;
#endif

	//SetAllocateLogging( TRUE );
#ifdef __WINDOWS__
	// LOL - yeah and then all the functionality of this is
	// ready for use... and it starts itself...

	// race conditions exist for exiting...
	// this makes this impractical, since dekware tries to
	// destroy external objects which were created, it doens't know any better
	// so when InterShell exits, dekware and intershell both try to close the frame
	//LoadFunction( WIDE("Dekware.core.dll"), NULL );
	//EditControlBehaviors = (void(CPROC*)(PSI_CONTROL))LoadFunction( WIDE("dialog.nex"), "EditControlBehaviors" );
#else
	//LoadFunction( WIDE("libdekware.core.so"), NULL );
	//EditControlBehaviors = LoadFunction( WIDE("dialog.nex"), "EditControlBehaviors" );
#endif
	//SetSystemLoggingLevel( LOG_NOISE) ;
	//DumpRegisteredNames();
	//SetAllocateLogging( TRUE );
#ifndef __NO_OPTIONS__
	g.flags.bTopmost = SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Display Topmost", 0, TRUE );
   g.flags.bTransparent = SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Display is transparent", 1, TRUE );
   g.flags.bSpanDisplay = SACK_GetProfileIntEx( GetProgramName(), "Intershell Layout/Use Both Displays(horizontal)", 0, TRUE );
#endif
	//SystemLogTime( SYSLOG_TIME_CPU| SYSLOG_TIME_DELTA );

	g.system_name = GetSystemName(); // Initialized here. Command argument -Sysname= may override.

	SetManualAllocateCheck( TRUE );
	BannerNoWait( WIDE("Starting...") );
	{
		char buf[256];
		snprintf( buf, sizeof( buf ), "%s.config", GetProgramName() );
		g.config_filename = StrDup( buf );
		//g.config_filename = "intershell.config";
	}
	{
		int n;
		for( n = 1; n < argc; n++ )
		{
			if( argv[n][0] == '-' )
			{
				// with subcanvas support, this cannot function, sorry
				// we get confused about which menu belongs to which frame
				// some thought will have to be done to figure this one out.
				if( stricmp( argv[n]+1, WIDE("multi") ) == 0 )
					g.flags.multi_edit = 1; // popup mulitiple framed windows instead of full screen mode.
				else if( stricmp( argv[n]+1, WIDE("force") ) == 0 )
					g.flags.forceload = 1;
				else if( stricmp( argv[n]+1, WIDE("restore") ) == 0 )
					g.flags.restoreload = 1;
				else if( stricmp( argv[n]+1, WIDE("SQL") ) == 0 )
					g.flags.bSQLConfig = 1;
				else if( stricmp( argv[n]+1, WIDE("Sysname=") ) == 0 )
					g.system_name = StrDup( argv[n] + 9 );
				else if( stricmp( argv[n]+1, WIDE("local") ) == 0 )
					g.flags.local_config = 1; // don't save in sql...
				else if( stricmp( argv[n]+1, WIDE("tsr") ) == 0 )
					g.flags.bTerminateStayResident = 1; // return to caller from main instead of exit and idle.
				else if( stricmp( argv[n]+1, WIDE("names" ) ) == 0 )
					g.flags.bLogNames = 1;
			}
			else
			{
				char *varval;
				if( ( varval = strchr( argv[n], '=' ) ) )
				{
					char *varname = argv[n];
					varval[0] = 0;
					varval++;
					SetVariable( varname, varval );


				}
				else
				{
					if( g.config_filename )
						Release( g.config_filename );
					g.config_filename = StrDup( argv[n] );
				}
			}
		}
	}
	if( g.flags.restoreload )
	{
		char *ext;
		ext = strrchr( g.config_filename, '.' );
		if( !ext || strnicmp( ext, ".AutoConfigBackup", 17 ) )
		{
			char msg[256];
			snprintf( msg, sizeof( msg ), "%s\nINVALID Configuration Name to Restore\nShould be like *.AutoConfigBackup*"
				, g.config_filename );
			BannerMessage( msg );
			return -1;
		}
		g.flags.forceload = 1; // -restore implies -force
	}
	//SetAllocateLogging( TRUE );
	g.pMainThread = MakeThread();
	if( !g.flags.bTerminateStayResident )
	{
		while( restart() != 1 );
		QuitMenu( 0, 0 );
#ifdef USE_INTERFACES
		DropDisplayInterface( g.pRenderInterface );
		DropImageInterface( g.pImageInterface );
#endif
		Release( g.mem_lock );
		return 0;
	}
	else
		restart(); // actually start, but it's oneshot.
	return 1;
}

#ifdef __cplusplus_cli
#include <vcclr.h>

namespace sack
{
namespace InterShell
{
	public ref class InterShell_Canvas
	{
		PSI_CONTROL this_frame;
		PCanvasData canvas;
		PRENDERER render;
			
	public:

		// preload should have already been invoked...
		InterShell_Canvas( System::IntPtr handle )
		{
			//g.flags.bLogNames = 1;
			InvokeDeadstart();
			Init( FALSE );
			//g.UseWindowHandle = handle;
			//ThreadTo( MenuThread, handle );
			{
				render = MakeDisplayFrom( (HWND)((int)handle) );
				RECT r;
				GetClientRect( (HWND)((int)handle), &r );
				this_frame = MakeNamedControl( NULL, "Menu Canvas", 0, 0
						, r.right-r.left+1, r.bottom-r.top+1, -1 );
				AttachFrameToRenderer( this_frame, render );
				DisplayFrame( this_frame );

				canvas = GetCanvas( this_frame );
				canvas->renderer = render;
				{
#ifdef USE_EDIT_GLARE
					Image image = GetControlSurface( this_frame );
					AddGlareLayer( canvas, image );
#endif
				}

				InterShell_DisablePageUpdate( FALSE );
				//return (int)this_frame;
			}
		}

		void Resize( int width, int height )
		{
			SizeDisplay( render, width, height );
			SizeCommon( this_frame, width, height );
			{
				S_32 x, y;
				_32 w, h;
				GetDisplayPosition( canvas->renderer, &x, &y, &w, &h );
				MoveSizeDisplay( canvas->edit_glare, x, y, w, h );
			}
		}
		void Focus( void )
		{
			ForceDisplayFocus( render );
		}

		void  Save( System::String^ string )
		{
			pin_ptr<const wchar_t> wch = PtrToStringChars(string);
			size_t convertedChars = 0;
			size_t  sizeInBytes = ((string->Length + 1) * 2);
			errno_t err = 0;
			char    *ch = NewArray(TEXTCHAR,sizeInBytes);


			err = wcstombs_s(&convertedChars, 
                    ch, sizeInBytes,
                    wch, sizeInBytes);
			
			SaveButtonConfig( this_frame, ch );
		}
		void  Load( System::String^ string )
		{
			pin_ptr<const wchar_t> wch = PtrToStringChars(string);
			char    *ch = WcharConvert( wch );			
			LoadButtonConfig( this_frame, ch );

			BannerNoWait( WIDE("Finish Config...") );
			/* this builds menus and junk based on plugins which have been loaded... */
			InvokeFinishInit();
			{
				// cleanup banners.
				PBANNER Null = NULL;
				RemoveBanner( Null );
			}
			InvokeFinishAllInit();
			InvokeStartupMacro();



		}

		char*  GetCanvasConfig( PSI_CONTROL canvas )
		{
			return "default_intershell.config";
		}
	};
}
}
#endif

#ifdef _DEFINE_INTERFACE 

static struct intershell_interface RealInterShellInterface = {
GetCommonButtonControls                
, SetCommonButtonControls				
, RestartMenu							
, ResumeMenu								
, InterShell_GetButtonColors					
, InterShell_SetButtonColors					
, InterShell_SetButtonColor					
, InterShell_SetButtonText						
, InterShell_GetButtonText						
, InterShell_SetButtonImage					
#ifndef __NO_ANIMATION__
, InterShell_SetButtonAnimation				
#endif
, InterShell_CommonImageLoad					
, InterShell_CommonImageUnloadByName			
, InterShell_CommonImageUnloadByImage			
, InterShell_SetButtonImageAlpha				
, InterShell_IsButtonVirtual					
, InterShell_SetButtonFont						
, InterShell_GetCurrentButtonFont				
, InterShell_SetButtonStyle					
, InterShell_SaveCommonButtonParameters		
, InterShell_GetSystemName						
, UpdateButtonExx						
, ShellGetCurrentPageEx					
, ShellGetCurrentPage					
, ShellGetNamedPage						
, ShellSetCurrentPage					
, ShellSetCurrentPageEx					
, ShellCallSetCurrentPage				
, ShellCallSetCurrentPageEx				
, ShellReturnCurrentPage					
, ClearPageList							
, InterShell_DisablePageUpdate					
, RestoreCurrentPage						
, HidePageExx
, InterShell_DisableButtonPageChange			
, CreateLabelVariable					
, CreateLabelVariableEx					
, LabelVariableChanged					
, LabelVariablesChanged					
, InterShell_Hide								
, InterShell_Reveal							
, GetPageSize							
, SetButtonTextField						
, AddButtonLayout						
, InterShell_GetButtonControl					
																 , InterShell_GetLabelText
																 , InterShell_TranslateLabelText
																 , InterShell_GetControlLabelText
																 , SelectAFont
																 , UseAFont
																 , CreateAFont
																 , BeginCanvasConfiguration
																 , SaveCanvasConfiguration
																 , SaveCanvasConfiguration_XML
																 , InterShell_GetCurrentConfigHandler
																 , BeginSubConfiguration
                                                 , EscapeMenuString
																 , InterShell_GetCurrentLoadingControl
																 , InterShell_GetButtonFont
																 , InterShell_GetButtonFontName
																 , InterShell_GetCurrentButton
																 , InterShell_SetButtonFontName
																 , InterShell_GetPhysicalButton
																				 , InterShell_SetButtonHighlight
                                                             , InterShell_CreateControl
};

POINTER CPROC LoadInterShellInterface( void )
{
	return (POINTER)&RealInterShellInterface;
}

void CPROC UnloadInterShellInterface( POINTER p )
{
}

PRELOAD( RegisterInterShellInterface )
{
	RegisterInterface( "InterShell", LoadInterShellInterface, UnloadInterShellInterface );
	if( SACK_GetProfileIntEx( GetProgramName(), "Alias InterShell for MILK", 1, TRUE) )
	{
		RegisterClassAlias( "system/interfaces/InterShell", "system/interfaces/MILK");
		RegisterClassAlias( "InterShell", "MILK" );
		RegisterClassAlias( "sack/widgets", "altanik/widgets" );
	}
}

OnKeyPressEvent( "InterShell/Debug Memory" )( PTRSZVAL psv )
{
	DebugDumpMem();
}

OnCreateMenuButton( "InterShell/Debug Memory" )( PMENU_BUTTON button )
{
	return 1;
}

#endif
