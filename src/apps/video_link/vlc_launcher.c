#include <stdhdrs.h>
#include <system.h>
#include <deadstart.h>
#include <procreg.h>
#include "link_events.h"

static struct vlc_launcher_local
{
	PLIST tasks;
   int display; // number of the display to show vlc on when showing videos.
   PTASK_INFO vlc_task;
} vlc_launcher_local;
#define l vlc_launcher_local

PRELOAD( InitVLCLauncher )
{
   l.display = SACK_GetProfileInt( GetProgramName(), "vlc_launcher/Default Display", 2 );
}

static void CPROC GetOutput( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 length )
{
   lprintf( "buffer" );
}

static void CPROC MyTaskEnd( PTRSZVAL psv, PTASK_INFO task )
{
   lprintf( "Task %p ended.", task );
	DeleteLink( &l.tasks, task );
}

static void LaunchVLC( PVARTEXT pvt_command, CTEXTSTR program )
{
	if( l.vlc_task )
	{
		StopProgram( l.vlc_task );
	}
	{
		TEXTCHAR **args;
		int nArgs;
		ParseIntoArgs( GetText( VarTextPeek( pvt_command ) ), &nArgs, &args );
		VarTextEmpty( pvt_command );
		l.vlc_task = LaunchPeerProgram( program, NULL, (PCTEXTSTR)args,  GetOutput, MyTaskEnd, 0 );
      AddLink( &l.tasks, l.vlc_task );
	}
}


static void VideoLinkCommandServeMaster( "VLC Process" )( void )
{
	PVARTEXT pvt_cmd = VarTextCreate();
	vtprintf( pvt_cmd, "(unused program name) -stream dshow://" );
   LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandServeDelegate( "VLC Process" )( void )
{
	PVARTEXT pvt_cmd = VarTextCreate();
	vtprintf( pvt_cmd, "(unused program name) -stream dshow://" );
   LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandConnectToMaster( "VLC Process" )( CTEXTSTR address )
{
	PVARTEXT pvt_cmd = VarTextCreate();
	vtprintf( pvt_cmd, "(unused program name) -display %d http://%s", l.display, address );
   LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandConnectToDelegate( "VLC Process" )( CTEXTSTR address )
{
	PVARTEXT pvt_cmd = VarTextCreate();
	vtprintf( pvt_cmd, "(unused program name) -display %d http://%s", l.display, address );
	LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandReset( "VLC Process" )( LOGICAL soft_reset )
{
	if( l.vlc_task )
	{
		StopProgram( l.vlc_task );
	}
}
