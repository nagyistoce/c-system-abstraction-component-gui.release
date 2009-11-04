
#include "widgets/include/banner.h"
#include "intershell_export.h"
#include "intershell_registry.h"

typedef struct banner_button BANNER_BUTTON, *PBANNER_BUTTON;

struct banner_button {
	struct {
		BIT_FIELD bTopmost : 1;
		BIT_FIELD forced_delay : 1; // also known as 'ignore clicks'
		BIT_FIELD allow_continue : 1; // also known as 'ignore clicks'
		BIT_FIELD yes_no : 1; // allow yesno on banner
		BIT_FIELD okay_cancel : 1; // allow yesno on banner
		BIT_FIELD explorer : 1; // banner only over where the explorer bar is...
	} flags;
	_32 delay; // amount of time forced wait.
	CTEXTSTR text;
	CTEXTSTR imagename;
	Image image; // may add an image to the banner?
	PMENU_BUTTON button;
	PBANNER banner; // this sort of thing is a banner.
};

enum {
	EDIT_BANNER_TEXT = 3000
		, EDIT_BANNER_DELAY
		, CHECKBOX_TOPMOST
		, CHECKBOX_CONTINUE
		, CHECKBOX_YESNO
		, CHECKBOX_NOCLICK
		, CHECKBOX_OKAYCANCEL
		, EDIT_CONTROL_TEXT // used for macro element text
		, CHECKBOX_EXPLORER
};
static struct {
	PLIST banners; // active banner list.
} l;

PRELOAD( RegisterResources )
{
	EasyRegisterResource( "InterShell/banner", CHECKBOX_TOPMOST            , RADIO_BUTTON_NAME );
	EasyRegisterResource( "InterShell/banner", CHECKBOX_CONTINUE            , RADIO_BUTTON_NAME );
	EasyRegisterResource( "InterShell/banner", CHECKBOX_YESNO            , RADIO_BUTTON_NAME );
	EasyRegisterResource( "InterShell/banner", CHECKBOX_OKAYCANCEL            , RADIO_BUTTON_NAME );
	EasyRegisterResource( "InterShell/banner", CHECKBOX_NOCLICK         , RADIO_BUTTON_NAME );
	EasyRegisterResource( "InterShell/banner", CHECKBOX_EXPLORER         , RADIO_BUTTON_NAME );
	EasyRegisterResource( "InterShell/banner", EDIT_BANNER_DELAY           , EDIT_FIELD_NAME );
	EasyRegisterResource( "InterShell/banner", EDIT_BANNER_TEXT           , EDIT_FIELD_NAME );

	EasyRegisterResource( "InterShell/banner", EDIT_CONTROL_TEXT           , EDIT_FIELD_NAME );
}

OnCreateMenuButton( "Banner Message" )( PMENU_BUTTON button )
{
	PBANNER_BUTTON banner = New( BANNER_BUTTON );
	MemSet( banner, 0, sizeof( *banner ) );
	banner->button = button;
	return (PTRSZVAL)banner;
}

OnKeyPressEvent( "Banner Message" )( PTRSZVAL psvBanner )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	char buffer[256];
	int yes_no;
	_32 timeout = 0;
   _32 delay = 0;
	if((banner->flags.explorer || !banner->flags.allow_continue)&&banner->delay)
		timeout = BANNER_TIMEOUT;
	if( !banner->flags.forced_delay && !banner->flags.yes_no && !banner->flags.okay_cancel && !banner->flags.allow_continue )
		timeout = BANNER_TIMEOUT;
	if( timeout && !banner->delay )
		delay = 2000;
	else
      delay = banner->delay;
	yes_no = CreateBannerEx( NULL, &banner->banner
								  , InterShell_TranslateLabelText( NULL, buffer, sizeof( buffer ), banner->text )
								  , (banner->flags.bTopmost?BANNER_TOP:0)
									| ((banner->flags.allow_continue&&(!banner->flags.yes_no))?BANNER_NOWAIT:0)
									| (banner->flags.forced_delay?BANNER_DEAD:BANNER_CLICK)
									| (banner->flags.explorer?BANNER_EXPLORER:0)
									| (banner->flags.yes_no?BANNER_OPTION_YESNO:0)
									| (banner->flags.okay_cancel?BANNER_OPTION_OKAYCANCEL:0)
									| (((banner->flags.explorer || !banner->flags.allow_continue)&&banner->delay)?BANNER_TIMEOUT:0)
								  , timeout?delay:0
									//((banner->flags.allow_continue&&!banner->flags.explorer)?((!banner->flags.forced_delay)?2000:0):banner->delay)
								  );

	if( !banner->flags.allow_continue )
	{
		if( banner->flags.yes_no || banner->flags.okay_cancel )
			SetMacroResult( yes_no );// set allow_continue to yes_no
      if( banner->banner )
			RemoveBannerEx( &banner->banner DBG_SRC );
	}
	else
	{
      // can have a whole slew of banners each with their own removes ?
		AddLink( &l.banners, banner );
	}
	//BannerMessage( "Yo, whatcha want!?" );
}

OnCreateMenuButton( "Banner Message Remove" )( PMENU_BUTTON button )
{
	// this button should only exist as an invisible/macro button....
	PBANNER_BUTTON banner = New( BANNER_BUTTON );
	MemSet( banner, 0, sizeof( *banner ) );
	banner->button = button;
	return (PTRSZVAL)banner;
}

OnKeyPressEvent( "Banner Message Remove" )( PTRSZVAL psvBanner )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	INDEX idx;
	LIST_FORALL( l.banners, idx, PBANNER_BUTTON, banner )
	{
		RemoveBanner( banner->banner );
		SetLink( &l.banners, idx, NULL );
	}
	//BannerMessage( "Yo, whatcha want!?" );
}

void StripConfigString( TEXTSTR out, CTEXTSTR in )
{
	// convert \r\n to literal characters...
	if( !in )
	{
		out[0] = 0;
		return;
	}
	for( ; in[0]; in++,out++ )
	{
		if( in[0] == '\\' )
		{
			switch( in[1] )
			{
			case 'n':
				in++;
				out[0] = '\n';
				break;
			default:
				out[0] = in[1];
				in++;
				break;
			}
		}
		else
			out[0] = in[0];
	}
	out[0] = in[0];
}

void ExpandConfigString( TEXTSTR out, CTEXTSTR in )
{
	// convert \r\n to literal characters...
	if( !in )
	{
		out[0] = 0;
		return;
	}
	for( ; in[0]; in++,out++ )
	{
		if( in[0] == '\n' )
		{
			out[0] = '\\';
			out++;
			out[0] = 'n';
		}
		else if( in[0] == '\\' )
		{
			out[0] = '\\';
			out++;
			out[0] = '\\';
		}
		else
			out[0] = in[0];
	}
	out[0] = in[0];
}

OnConfigureControl( "Banner Message" )( PTRSZVAL psvBanner, PSI_CONTROL parent )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PSI_CONTROL frame = LoadXMLFrameOver( parent, "EditBannerMessage.isFrame" );
	if( frame )
	{
		char buffer[256];
		int okay = 0;
		int done = 0;
		// some sort of check button for topmost
		// delay edit field
		// yesno
		// color?
		// image name?
		// if delay is set, make sure that the banner is unclickable.
		snprintf( buffer, sizeof( buffer ), "%d", banner->delay );
		SetControlText( GetControl( frame, EDIT_BANNER_DELAY ), buffer );
		ExpandConfigString( buffer, banner->text );
		SetCheckState( GetControl( frame, CHECKBOX_NOCLICK ), banner->flags.forced_delay );
		SetCheckState( GetControl( frame, CHECKBOX_EXPLORER ), banner->flags.explorer );
		SetCheckState( GetControl( frame, CHECKBOX_TOPMOST ), banner->flags.bTopmost );
		SetCheckState( GetControl( frame, CHECKBOX_YESNO ), banner->flags.yes_no );
		SetCheckState( GetControl( frame, CHECKBOX_OKAYCANCEL ), banner->flags.okay_cancel );
		SetCheckState( GetControl( frame, CHECKBOX_CONTINUE ), banner->flags.allow_continue );
		SetControlText( GetControl( frame, EDIT_BANNER_TEXT ), buffer );
		{
			char buffer[256];
			InterShell_GetButtonText( banner->button, buffer, sizeof( buffer ) );
			SetControlText( GetControl( frame, EDIT_CONTROL_TEXT ), buffer );
		}


		SetCommonButtons( frame, &done, &okay );
		DisplayFrameOver( frame, parent );
		CommonWait( frame );
		if( okay )
		{
			char buffer[256];
			char buffer2[256];
			GetControlText( GetControl( frame, EDIT_BANNER_DELAY ), buffer, sizeof( buffer ) );
			banner->delay = atoi( buffer );
			banner->flags.allow_continue = GetCheckState( GetControl( frame, CHECKBOX_CONTINUE ) );
			banner->flags.bTopmost = GetCheckState( GetControl( frame, CHECKBOX_TOPMOST ) );
			banner->flags.explorer = GetCheckState( GetControl( frame, CHECKBOX_EXPLORER ) );
			banner->flags.yes_no = GetCheckState( GetControl( frame, CHECKBOX_YESNO ) );
			banner->flags.okay_cancel = GetCheckState( GetControl( frame, CHECKBOX_OKAYCANCEL ) );
			GetControlText( GetControl( frame, EDIT_BANNER_TEXT ), buffer, sizeof( buffer ) );
			banner->flags.forced_delay = GetCheckState( GetControl( frame, CHECKBOX_NOCLICK ) );
			StripConfigString( buffer2, buffer );
			banner->text = StrDup( buffer2 );

			GetControlText( GetControl( frame, EDIT_CONTROL_TEXT ), buffer, sizeof( buffer ) );
			InterShell_SetButtonText( banner->button, buffer );
		}
		DestroyFrame( &frame );

	}
	return psvBanner;
}



OnSaveControl( "Banner Message" )( FILE *file, PTRSZVAL psvBanner )
{
	char buffer[256];
	char buffer2[256];
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	ExpandConfigString( buffer, banner->text );
	ExpandConfigString( buffer2, buffer );
	fprintf( file, "banner text=%s\n", buffer2 );
	fprintf( file, "banner timeout=%d\n", banner->delay );
	fprintf( file, "banner continue=%s\n", banner->flags.allow_continue?"yes":"no" );
	fprintf( file, "banner force delay=%s\n", banner->flags.forced_delay?"yes":"no" );
	fprintf( file, "banner topmost=%s\n", banner->flags.bTopmost?"yes":"no" );
	fprintf( file, "banner yes or no=%s\n", banner->flags.yes_no?"yes":"no" );
	fprintf( file, "banner explorer=%s\n", banner->flags.explorer?"yes":"no" );
	fprintf( file, "banner okay or cancel=%s\n", banner->flags.okay_cancel?"yes":"no" );
}


static PTRSZVAL CPROC ConfigSetBannerText( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, CTEXTSTR, text );
	TEXTCHAR buffer[256];
	StripConfigString( buffer, text );
	if( banner->text )
		Release( (POINTER)banner->text );
	banner->text = StrDup( buffer );
	return psvBanner;
}

static PTRSZVAL CPROC ConfigSetBannerTimeout( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, S_64, delay );
	if( delay < 0 )
		banner->delay = 0;
	else
		banner->delay = delay;

	return psvBanner;
}

static PTRSZVAL CPROC ConfigSetBannerContinue( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, LOGICAL, contin );
	banner->flags.allow_continue = contin;
	return psvBanner;
}

static PTRSZVAL CPROC ConfigSetBannerForced( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, LOGICAL, forced );
	banner->flags.forced_delay = forced;
	return psvBanner;
}

static PTRSZVAL CPROC ConfigSetBannerTopmost( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, LOGICAL, topmost );
	banner->flags.bTopmost = topmost;
	return psvBanner;
}

static PTRSZVAL CPROC ConfigSetBannerYesNo( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, LOGICAL, yesno );
	banner->flags.yes_no = yesno;
	return psvBanner;
}

static PTRSZVAL CPROC ConfigSetBannerExplorer( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, LOGICAL, yesno );
	banner->flags.explorer = yesno;
	return psvBanner;
}

static PTRSZVAL CPROC ConfigSetBannerOkayCancel( PTRSZVAL psvBanner, arg_list args )
{
	PBANNER_BUTTON banner = (PBANNER_BUTTON)psvBanner;
	PARAM( args, LOGICAL, okaycancel );
	banner->flags.okay_cancel = okaycancel;
	return psvBanner;
}

OnLoadControl( "Banner Message" )( PCONFIG_HANDLER pch, PTRSZVAL psvBanner )
{
	AddConfigurationMethod( pch, "banner text=%m", ConfigSetBannerText );
	AddConfigurationMethod( pch, "banner timeout=%i", ConfigSetBannerTimeout );
	AddConfigurationMethod( pch, "banner continue=%b", ConfigSetBannerContinue );
	AddConfigurationMethod( pch, "banner force delay=%b", ConfigSetBannerForced );
	AddConfigurationMethod( pch, "banner topmost=%b", ConfigSetBannerTopmost );
	AddConfigurationMethod( pch, "banner yes or no=%b", ConfigSetBannerYesNo );
	AddConfigurationMethod( pch, "banner explorer=%b", ConfigSetBannerExplorer );
	AddConfigurationMethod( pch, "banner okay or cancel=%b", ConfigSetBannerOkayCancel );
}

