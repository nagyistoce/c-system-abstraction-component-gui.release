
#include <system.h>
#include <timers.h>
#include "../intershell_registry.h"

#include <psi/console.h>


PRELOAD( LoadConsole )
{
   lprintf( "Loaded something..." );
   LoadFunction( "psicon.nex", NULL );
}


void CPROC tickthing( PTRSZVAL psv )
{
	PSI_CONTROL pc = (PSI_CONTROL)psv;

	//pcprintf( pc, "blah\n" );

}


OnCreateControl( "Dekware Console" )( PSI_CONTROL parent, S_32 x, S_32 y, _32 w, _32 h )
{
	PSI_CONTROL pc;
	pc = MakeNamedControl( parent, "Dekware PSI Console", x, y, w, h, -1 );
   AddTimer( 250, tickthing, (PTRSZVAL)pc );

   return (PTRSZVAL)pc;
}

OnGetControl( "Dekware Console" )( PTRSZVAL psv )
{
   return (PSI_CONTROL)psv;
}


