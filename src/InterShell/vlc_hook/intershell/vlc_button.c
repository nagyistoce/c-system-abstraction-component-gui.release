#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#include <stdhdrs.h>
#include <image.h>
#include "/binglink/link_events.h"
#include "../../intershell_registry.h"
#include "../../intershell_export.h"
#include "../vlcint.h"


struct player_button
{
   PMENU_BUTTON button;
   CTEXTSTR url_name;
};
static struct vlc_button_local {
	struct my_vlc_interface *vlc_serve;
   PLIST players;
} l;

typedef struct vlc_surface {
	PSI_CONTROL pc;
	struct my_vlc_interface *vlc;
   LOGICAL playing; // is showing something.
} *PVLC;

EasyRegisterControlWithBorder( "VLC Surface", sizeof( struct vlc_surface ), BORDER_NONE );

void PlayOnPlayers( CTEXTSTR url_name )
{
   PVLC vlc;
	INDEX idx;
	if( !url_name )
      return;
	LIST_FORALL( l.players, idx, PVLC, vlc )
	{
		if( vlc->playing )
			StopItemIn( vlc->pc );
      lprintf( "PC is %p", vlc->pc );
		PlayItemInEx( vlc->pc, url_name, NULL  );
      vlc->playing = TRUE;
	}
}

OnKeyPressEvent( "VLC/button" )( PTRSZVAL psv )
{
   static int n;
	PMENU_BUTTON button = (PMENU_BUTTON)psv;
   PSI_CONTROL pc_button = InterShell_GetButtonControl( button );
	PRENDERER renderer = GetButtonAnimationLayer( pc_button );
	n++;
	if( n == 5 )
      n = 0;
}

OnCreateMenuButton( "VLC/button" )( PMENU_BUTTON button )
{

   return (PTRSZVAL)button;
}


OnKeyPressEvent( "VLC/Player button" )( PTRSZVAL psv )
{
	struct player_button *play = (struct player_button *)psv;
   PlayOnPlayers( play->url_name );
}

OnCreateMenuButton( "VLC/Player button" )( PMENU_BUTTON button )
{
	struct player_button *play = New( struct player_button );
	play->button = button;
   play->url_name = NULL;
	InterShell_SetButtonStyle( button, "bicolor square" );
   InterShell_SetButtonText( button, "Play_Video" );
   return (PTRSZVAL)button;
}

OnKeyPressEvent( "VLC/Stop Player button" )( PTRSZVAL psv )
{
	struct player_button *play = (struct player_button *)psv;
   PVLC vlc;
	INDEX idx;
	LIST_FORALL( l.players, idx, PVLC, vlc )
	{
		if( vlc->playing )
			StopItemIn( vlc->pc );
      vlc->playing = FALSE;
	}


}

OnCreateMenuButton( "VLC/Stop Player button" )( PMENU_BUTTON button )
{
	struct player_button *play = New( struct player_button );
	play->button = button;
   play->url_name = NULL;
	InterShell_SetButtonStyle( button, "bicolor square" );
   InterShell_SetButtonText( button, "Stop_Player" );
   return (PTRSZVAL)button;
}

static PTRSZVAL CPROC SetButtonMedia( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, url );
	struct player_button *play = (struct player_button *)psv;
	if( play->url_name )
		Release( play->url_name );
   play->url_name = StrDup( url );
   return psv;
}

OnLoadControl( "VLC/Player button" )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
	AddConfigurationMethod( pch, WIDE( "Play Media:%m" ), SetButtonMedia );
}

OnSaveControl( "VLC/Player button" )( FILE *file, PTRSZVAL psv )
{
	struct player_button *play = (struct player_button *)psv;
   if( play->url_name )
		fprintf( file, WIDE( "Play Media:%s\n" ), play->url_name );
}



OnCreateControl( "VLC/Player" )( PSI_CONTROL parent, S_32 x, S_32 y, _32 w, _32 h )
{

	PSI_CONTROL pc = MakeNamedControl( parent, "VLC Surface", x, y, w, h, -1 );
	MyValidatedControlData( PVLC, vlc, pc );
   vlc->pc = pc;
   AddLink( &l.players, vlc );
   return (PTRSZVAL)vlc;
}

OnGetControl( "VLC/Player")(PTRSZVAL psv )
{
   PVLC vlc = (PVLC)psv;
   return vlc->pc;
}

OnShowControl( "VLC/Player" )(PTRSZVAL psv )
{
   PVLC vlc = (PVLC)psv;
	//vlc->vlc = PlayItemInEx( vlc->pc, "dshow://", NULL );

}

OnHideControl( "VLC/Player" )(PTRSZVAL psv )
{
   PVLC vlc = (PVLC)psv;
	//StopItem(  vlc->vlc );

}

//------------------------------------------------------------------------------------

OnCreateControl( "VLC/Video Link" )( PSI_CONTROL parent, S_32 x, S_32 y, _32 w, _32 h )
{
}


//------------------------------------------------------------------------------------
/*
static void VideoLinkCommandServeMaster( "VLC_Video Link" )( void )
{
	// enable reading dshow:// and writing a stream out, Need the service vlc_interface.
	if( !l.vlc_serve )
	{
      Image image = MakeImageFile( 320, 240 );
      l.vlc_serve = PlayItemAgainst( image, "dshow:// --sout '#transcode{vcodec=mp4v,acodec=mpga,vb=3072,height=480,width=720,ab=192,channels=2}:duplicate{dst=display,dst=standard{access=http,mux=ts,dst=0.0.0.0:1234,height=480,width=720}}' >/tmp/vlc.log 2>&1" );
	}
}
*/
#ifdef __WATCOMC__
PUBLIC( void, ExportThis )( void )
{
}
#endif
