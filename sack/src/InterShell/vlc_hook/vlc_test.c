#define DEFINE_DEFAULT_RENDER_INTERFACE
#include <stdhdrs.h>
#include <idle.h>
#include <controls.h>
#include "vlcint.h"



int main( int argc, char ** argv )
{
   char *file_to_play;
	_32 w, h;
   int display = 0;
	S_32 x, y;
   static TEXTCHAR extra_opts[4096];
	int ofs = 0;
	int in_control = 0;
   int make_top = 0;
   int is_transparent = 0;
   int is_stream = 0;
	PLIST names = NULL;
	SetSystemLog( SYSLOG_FILE, stderr );

	if( argc == 1 )
	{
		MessageBox( NULL, "usage for vlc tester...\n"
					  " -display #  : show on display number\n"
					  " -top : make topmost\n"
					  " -control : use a control to host video\n"
					  " -transparent : use transparent display\n"
					 , "Usage", MB_OK );
      return 0;
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
				if( stricmp( argv[n]+1, WIDE("display") ) == 0 )
				{
					display = atoi( argv[n+1] );
					n++;
				}
				else if( stricmp( argv[n]+1, WIDE("transparent") ) == 0 )
				{
               is_transparent = 1;
				}
				else if( stricmp( argv[n]+1, WIDE("stream") ) == 0 )
				{
               is_stream = 1;
				}
				else if( stricmp( argv[n]+1, WIDE("control") ) == 0 )
				{
               in_control = 1;
				}
				else if( stricmp( argv[n]+1, WIDE("top") ) == 0 )
				{
               make_top = 1;
				}
				else
				{
               ofs += snprintf( extra_opts + ofs, 4096 - ofs, "%s", argv[n]+1 );
				}
			}
			else
			{
				AddLink( &names, argv[n] );
			}
		}
	}


	GetDisplaySizeEx( display, &x, &y, &w, &h );


	lprintf( "EXCEPTION CAUSED EARLY BREAK?!" );
	//return 0;

	if( argc < 2 )
	{
		return 0;
	}
	{
		PRENDERER transparent;
		PSI_CONTROL surface;

		if( !is_stream )
		{
			transparent = OpenDisplaySizedAt( is_transparent?DISPLAY_ATTRIBUTE_LAYERED:0, w, h, x, y );
			surface = in_control?CreateFrameFromRenderer( "Video Display"
																	  , BORDER_NONE|BORDER_NOCAPTION
																	  , transparent ):0;
			DisableMouseOnIdle( transparent, TRUE );
			if( surface )
				DisplayFrame( surface );
			else
				UpdateDisplay( transparent );
			if( make_top )
				MakeTopmost( transparent );
		}
		{
			CTEXTSTR tmp = (CTEXTSTR)GetLink( &names, 0 );
			if( tmp )
			{
				if( is_stream )
					PlayItemAtEx( NULL, tmp, extra_opts );
				else if( in_control )
					PlayItemInEx( surface, tmp, extra_opts );
            else
					PlayItemOnExx( transparent, tmp, extra_opts, is_transparent );
			}
		}
		while( 1 )
		{
         lprintf( "sleep..." );
			IdleFor( 250000 );
         if( !is_stream )
				if( !DisplayIsValid( transparent ) )
					break;
		}
	}
   return 0;
}
