
#define DEFINE_DEFAULT_RENDER_INTERFACE
#define USE_IMAGE_INTERFACE GetImageInterface()
#include <stdhdrs.h>
#include <stdlib.h>
#include <psi.h>
#include <system.h>
#include <network.h>
#include <timers.h>
#include <sqlgetoption.h>
#include "../intershell/vlc_hook/vlcint.h"


#ifdef __LINUX__
#define INVALID_HANDLE_VALUE -1
#endif

static struct {
	PCLIENT pc;
	PTASK_INFO task;
   SOCKADDR *sendto;
} l;

//--------------------------------------------------------------------

typedef struct MyControl *PMY_CONTROL;
typedef struct MyControl MY_CONTROL;
struct MyControl
{
	PRENDERER surface;
	struct {
		BIT_FIELD bShown : 1;
	} flags;
};


void SendUpdate( _32 x, _32 y, _32 w, _32 h )
{
	_32 buffer[7];
	buffer[0] = GetTickCount();
	buffer[1] = x;
	buffer[2] = y;
	buffer[3] = w;
	buffer[4] = h;
	lprintf( "update %d %d %d %d", x, y, w, h );
   LogBinary( l.sendto, 50 );
   SendUDPEx( l.pc, buffer, sizeof( buffer ), l.sendto );
   WakeableSleep( 10 );
   SendUDPEx( l.pc, buffer, sizeof( buffer ), l.sendto );
   WakeableSleep( 10 );
   SendUDPEx( l.pc, buffer, sizeof( buffer ), l.sendto );
}

int main( int argc, char **argv )
{
   int x, y, w, h;
   NetworkStart();
	l.pc = ConnectUDP( NULL, 2997, "127.0.0.1", 5151, NULL, NULL );
   if( !l.pc )
		l.pc = ConnectUDP( NULL, 2996, "127.0.0.1", 5151, NULL, NULL );
   UDPEnableBroadcast( l.pc, TRUE );
	l.sendto = CreateSockAddress( "127.0.0.1", 2999 );
	x = atoi( argv[1] );
	y = atoi( argv[2] );
	w = atoi( argv[3] );
	h = atoi( argv[4] );
	{
		int j;
		for( j = 0; j < 200; j++ )
		{
			SendUpdate( x, y, w+((rand()*600)/RAND_MAX), h+((rand()*600)/RAND_MAX) );
		}
	}
   return 0;
}


