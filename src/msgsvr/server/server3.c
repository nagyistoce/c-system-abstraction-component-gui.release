#include <stdhdrs.h>
#include <timers.h>
#include <system.h>
#ifdef __WINDOWS__
#include <systray.h>
#endif
#include <procreg.h>

#ifdef BAG
#define PREFIX "bag."
#else
#define PREFIX
#endif

#ifdef _MSC_VER
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow )
#else
int main( void )
#endif
{
   // doesn't matter...
	SetInterfaceConfigFile( WIDE("MessageService.conf") );
#ifdef __STATIC__
	if( LoadPrivateFunction( PREFIX "msg.core.service.static", NULL ) )
#else
	if( LoadPrivateFunction( PREFIX "msg.core.service", NULL ) )
#endif
	{
#ifdef __WINDOWS__
#ifndef __NO_GUI__
		RegisterIcon( NULL );
#endif
#endif
      while( 1 )
			WakeableSleep( SLEEP_FOREVER );
	}
   else
		printf( WIDE("Failed to load message core service.\n") );
   return 0;
}
