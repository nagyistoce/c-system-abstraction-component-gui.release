#include <stdhdrs.h>
#include <system.h>
#include <timers.h>
#ifdef __WINDOWS__
// kinda awkward under linux - cause sometimes this thing itself
// hosts systray...
#include <systray.h>
#endif
#include <procreg.h>
#include <construct.h>

int main( void )
{
   int(CPROC *f)(void);
	SetInterfaceConfigFile( WIDE("MessageService.conf") );
#ifdef __STATIC__
#define NAME "display_service.static"
#else
#define NAME "display_service"
#endif
	// default proc returned by LoadPrivateFunction is void(void)
	// the function really is declared as int(void) returning the status
   // Is Display Loaded? (yes=1,no=0)
	if( (f = (int(CPROC*)(void))LoadPrivateFunction( NAME
										 , WIDE("IsDisplayLoadedOkay")
										 )) &&
	  f() )
	{
#ifdef __WINDOWS__
		RegisterIcon( NULL );
#endif
      LoadComplete();
  		WakeableSleep( SLEEP_FOREVER );
	}
	printf( WIDE("Failed to start the display service.") );
   return 0;
}
