#include <stdhdrs.h>
#include <system.h>
#include <deadstart.h>
#include <procreg.h>
#include <configscript.h>

#define USES_VIDEO_SERVER_INTERFACE
#define DEFINES_VIDEO_SERVER_INTERFACE
#include "link_events.h"

static struct vlc_launcher_local
{
	struct {
		BIT_FIELD bMaster : 1;
		BIT_FIELD bDelegate : 1;
		BIT_FIELD bParticipant : 1;
		BIT_FIELD bLaunching : 1;
	} flags;
	PLIST tasks;
   int display; // number of the display to show vlc on when showing videos.
	PTASK_INFO vlc_task;
   PCONFIG_HANDLER pch;
} vlc_launcher_local;
#define l vlc_launcher_local

PRELOAD( InitVLCLauncher )
{
   l.display = SACK_GetProfileInt( GetProgramName(), "vlc_launcher/Default Display", 2 );
}

PTRSZVAL EventPlaying CPROC( PTRSZVAL psv, arg_list args )
{
	if( l.flags.bMaster )
	{
		lprintf( "Gotcha - really playing." );
		MarkMasterServing();
		l.flags.bLaunching = 0;
		MarkTaskDone();
	}
	if( l.flags.bDelegate )
	{
		lprintf( "Gotcha - really playing." );
		MarkDelegateServing();
		l.flags.bLaunching = 0;
		MarkTaskDone();
	}
	if( l.flags.bParticipant )
	{
		lprintf( "Gotcha - really playing." );
		MarkParticipating();
		l.flags.bLaunching = 0;
		MarkTaskDone();
	}
}

PTRSZVAL CPROC GenericLog( PTRSZVAL psv, CTEXTSTR line )
{
   lprintf( "%s", line );
}

static void CPROC GetOutput( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 length )
{
	if( !l.pch )
	{
		l.pch = CreateConfigurationHandler();
		AddConfigurationMethod( l.pch, "%m):Really playing.", EventPlaying );
      SetConfigurationUnhandled( l.pch, GenericLog );
	}
	ProcessConfigurationInput( l.pch, buffer, length, 0 );
}

static void CPROC MyTaskEnd( PTRSZVAL psv, PTASK_INFO task )
{
   lprintf( "Task %p ended.", task );
	DeleteLink( &l.tasks, task );
	if( l.flags.bMaster )
	{
      l.flags.bMaster = 0;
		MarkMasterEnded();
	}
	if( l.flags.bDelegate )
	{
      l.flags.bDelegate = 0;
		MarkDelegateEnded();
	}
	if( l.flags.bParticipant )
	{
      l.flags.bParticipant = 0;
		MarkParticipantEnded();
	}
	if( l.flags.bLaunching )
	{
		l.flags.bLaunching = 0;
      MarkTaskDone();
	}
}

static void LaunchVLC( PVARTEXT pvt_command, CTEXTSTR program )
{
	if( l.vlc_task )
	{
		TerminateProgram( l.vlc_task );
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
	l.flags.bLaunching = 1;
   MarkTaskStarting();
	l.flags.bMaster = 1;
	vtprintf( pvt_cmd, "(unused_program_name) -stream H:\\videos\\*.avi" );
   LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandServeDelegate( "VLC Process" )( void )
{
	PVARTEXT pvt_cmd = VarTextCreate();
	l.flags.bLaunching = 1;
   MarkTaskStarting();
	l.flags.bDelegate = 1;
	vtprintf( pvt_cmd, "(unused_program_name) -stream dshow://" );
   LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandConnectToMaster( "VLC Process" )( CTEXTSTR address )
{
	PVARTEXT pvt_cmd = VarTextCreate();
	l.flags.bLaunching = 1;
   MarkTaskStarting();
	l.flags.bParticipant = 1;
	vtprintf( pvt_cmd, "(unused_program_name) -display %d http://%s:1234", l.display, address );
   LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandConnectToDelegate( "VLC Process" )( CTEXTSTR address )
{
	PVARTEXT pvt_cmd = VarTextCreate();
	l.flags.bLaunching = 1;
   MarkTaskStarting();
	l.flags.bParticipant = 1;
	vtprintf( pvt_cmd, "(unused_program_name) -display %d http://%s:1234", l.display, address );
	LaunchVLC( pvt_cmd, "vlc_test.exe" );
   VarTextDestroy( &pvt_cmd );
}

static void VideoLinkCommandReset( "VLC Process" )( LOGICAL initial_reset )
{
   lprintf( "Reset received %d", initial_reset );
	if( l.vlc_task )
	{
      lprintf( "Stopping what I started..." );
		TerminateProgram( l.vlc_task );
		if( l.flags.bLaunching )
		{
         MarkTaskDone();
			l.flags.bLaunching = 0;
		}
	}
}

ATEXIT( Shutdown )
{
	if( l.vlc_task )
	{
      lprintf( "Final Stopping what I started..." );
		TerminateProgram( l.vlc_task );
	}
}
