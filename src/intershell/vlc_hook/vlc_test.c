#define DEFINE_DEFAULT_RENDER_INTERFACE
#include <stdhdrs.h>
#include <idle.h>
#include <controls.h>
#include "vlcint.h"

int main( int argc, char ** argv )
{
   char *file_to_play;
	_32 w, h;
	PLIST names = NULL;
	SetSystemLog( SYSLOG_FILE, stderr );
	GetDisplaySize( &w, &h );

	w = 1024;
   h = 768;
	{
		int n;
		for( n = 1; n < argc; n++ )
		{
			AddLink( &names, argv[n] );
		}
		//PlayItem( argv[1] );
		//while( 1 )
		//   WakeableSleep( 100000 );
		if( n == 1 )
		{
			//AddLink( &names, "F:\\temp\\videos\\CrapsHor1024.mpg" );
			//AddLink( &names, "F:\\temp\\videos\\ElvisHor1024.mpg" );
			//AddLink( &names, "F:\\temp\\videos\\HawXmasHor1024.mpg" );
			//AddLink( &names, "F:\\temp\\videos\\HelloHor1024.mpg" );
			//AddLink( &names, "F:\\temp\\videos\\OysterBarSushiHor1024.mpg" );
			//AddLink( &names, "F:\\temp\\videos\\SalvatoresNowOpenHor1024.mpg" );
			//AddLink( &names, "F:\\temp\\videos\\SeattlesBestHor1024.mpg" );
			AddLink( &names, "m:\\tmp\\videos\\CrapsHor1024.mpg" );
			AddLink( &names, "m:\\tmp\\videos\\ElvisHor1024.mpg" );
			AddLink( &names, "m:\\tmp\\videos\\HawXmasHor1024.mpg" );
			AddLink( &names, "m:\\tmp\\videos\\HelloHor1024.mpg" );
			AddLink( &names, "m:\\tmp\\videos\\OysterBarSushiHor1024.mpg" );
			AddLink( &names, "m:\\tmp\\videos\\SalvatoresNowOpenHor1024.mpg" );
			AddLink( &names, "m:\\tmp\\videos\\SeattlesBestHor1024.mpg" );
		}
	}
	PlayList( names, 0, 0, w, h );

	lprintf( "EXCEPTION CAUSED EARLY BREAK?!" );
	//return 0;

	if( argc < 2 )
	{
		return 0;
	}
	{
		PRENDERER transparent = OpenDisplaySizedAt( DISPLAY_ATTRIBUTE_LAYERED, w, h, 0, 0 );
		PSI_CONTROL surface = CreateFrameFromRenderer( "Video Display", BORDER_NONE|BORDER_NOCAPTION, transparent );
      DisableMouseOnIdle( transparent, TRUE );
		//UpdateDisplay( transparent );
		DisplayFrame( surface );
		//MakeTopmost( transparent );
		{
			static TEXTCHAR buf[4096];
			int n;
         int ofs = 0;
			for(n = 2; n < argc; n++ )
			{
				if( strchr( argv[n], ' ' ) )
					ofs += snprintf( buf, sizeof( buf ) - ofs, "\"%s\" ", argv[n] );
				else
					ofs += snprintf( buf, sizeof( buf ) - ofs, "%s ", argv[n] );
			}
			PlayItemInEx( surface, argv[1], buf );
		}
		while( 1 )
		{
			IdleFor( 250 );
			if( !DisplayIsValid( transparent ) )
            break;
		}
	}
   return 0;
}
