#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#include <stdhdrs.h>
#include "../../intershell_registry.h"
#include "../../intershell_export.h"
#include "../vlcint.h"



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


typedef struct vlc_surface {
	PSI_CONTROL pc;
   struct my_vlc_interface *vlc;
} *PVLC;

EasyRegisterControlWithBorder( "VLC Surface", sizeof( struct vlc_surface ), BORDER_NONE );

OnCreateControl( "VLC/Player" )( PSI_CONTROL parent, S_32 x, S_32 y, _32 w, _32 h )
{

	PSI_CONTROL pc = MakeNamedControl( parent, "VLC Surface", x, y, w, h, -1 );
	MyValidatedControlData( PVLC, vlc, pc );
   vlc->pc = pc;
	//PlayItemIn( pc, "dshow://" );
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
	vlc->vlc = PlayItemInEx( vlc->pc, "screen://", "--screen-fps=10" );

}

OnHideControl( "VLC/Player" )(PTRSZVAL psv )
{
   PVLC vlc = (PVLC)psv;
	StopItem(  vlc->vlc );

}

#ifdef __WATCOMC__
PUBLIC( void, ExportThis )( void )
{
}
#endif
