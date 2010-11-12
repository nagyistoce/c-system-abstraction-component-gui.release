#include <sqlgetoption.h>
#include "../intershell/intershell_registry.h"

#include "../intershell/vlc_hook/vlcint.h"



OnCreateControl( "Video Capture-VLC") (PSI_CONTROL parent,S_32 x,S_32 y,_32 width,_32 height)
{
   char argsline[256];
	PSI_CONTROL pc = MakeNamedControl( parent, STATIC_TEXT_NAME, x, y, width, height, -1 );
	SACK_GetProfileString( "streamlib", "default capture vlc", "dshow://", argsline, sizeof( argsline ) );
	PlayItemIn( pc, argsline );
   return (PTRSZVAL)pc;
}

OnGetControl( "Video Capture-VLC" )( PTRSZVAL psv )
{
   return (PSI_CONTROL)psv;
}


