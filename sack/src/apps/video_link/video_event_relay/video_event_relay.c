
#include <stdhdrs.h>
#include <msgsvr/bard.h>


void CPROC ExternalStateChange( PTRSZVAL psv, char *extra )
{

   lprintf( "Received extern event... extra is %s", extra );
   BARD_IssueSimpleEvent( extra );
}

int main( void )
{
	BARD_RegisterForSimpleEvent( "extern video state change", ExternalStateChange, 0 );
	while( 1 )
		WakeableSleep( SLEEP_FOREVER );
   return 0;
}



