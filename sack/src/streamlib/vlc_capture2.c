
//#define DEFINE_DEFAULT_RENDER_INTERFACE
//#define USE_IMAGE_INTERFACE GetImageInterface()
#include <stdhdrs.h>
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
	struct flags
	{
		BIT_FIELD shutdown : 1;
	}flags;
   _32 last_attempt;
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

ATEXIT( CloseVlcWindows )
{
	lprintf( "Terminating task..." );
   l.flags.shutdown = 1;
   TerminateProgram( l.task );
}

EasyRegisterControl( "Video Control", sizeof( MY_CONTROL) );

void SendUpdate( _32 x, _32 y, _32 w, _32 h )
{
	_32 buffer[7];
	buffer[0] = GetTickCount();
	buffer[1] = x;
	buffer[2] = y;
	buffer[3] = w;
	buffer[4] = h;
	//lprintf( "update %d %d %d %d", x, y, w, h );
   //LogBinary( l.sendto, 50 );
   SendUDPEx( l.pc, buffer, sizeof( buffer ), l.sendto );
   WakeableSleep( 10 );
   SendUDPEx( l.pc, buffer, sizeof( buffer ), l.sendto );
   WakeableSleep( 10 );
   SendUDPEx( l.pc, buffer, sizeof( buffer ), l.sendto );
}

static int OnDrawCommon( "Video Control" )( PSI_CONTROL pc )
{
   return 1;
}

void CPROC TaskEnded( PTRSZVAL psv, PTASK_INFO task )
{
	if( task )
		l.task = NULL;

   // don't respawn when exiting.
	if( l.flags.shutdown )
		return;

   if( !l.task )
	{
		CTEXTSTR args[3];
		args[0] = "vlc_window.exe";
		args[1] = "dshow://";
		args[2] = NULL;
		if( l.last_attempt + 5000 < GetTickCount() )
		{
			l.task = LaunchProgramEx( "vlc_window.exe", NULL, args, TaskEnded, 0 );
		}
      l.last_attempt = GetTickCount();
	}
}

static int OnCreateCommon( "Video Control" )( PSI_CONTROL pc )
{
	MyValidatedControlData( PMY_CONTROL, control, pc );
	if( !l.pc )
	{
		l.pc = ConnectUDP( NULL, 2998, "127.0.0.1", 2999, NULL, NULL );
      l.sendto = CreateSockAddress( "127.0.0.1", 2999 );
		// just call it once when we startup the socket... (bad form)
		TaskEnded( 0, NULL );
	}
   return TRUE;
}

PUBLIC( void, link_vlc_stream )( void )
{
}



static void OnHideCommon( "Video Control" )( PSI_CONTROL pc )
{
	MyValidatedControlData( PMY_CONTROL, control, pc );
	if( control )
	{
      //lprintf( "Posting VLC Update." );
      //SendUpdate( 0, 0, 0, 0 );
	}
}

static void OnRevealCommon( "Video Control" )( PSI_CONTROL pc )
{
   MyValidatedControlData( PMY_CONTROL, control, pc );
	if( control )
	{
		S_32 x = 0;
		S_32 y = 0;
      Image image = GetControlSurface( pc );
		GetPhysicalCoordinate( pc, &x, &y, TRUE );
      //lprintf( "Posting VLC Update." );
		SendUpdate( x, y, image->width, image->height );
	}

}



