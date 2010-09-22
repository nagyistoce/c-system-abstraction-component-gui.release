#include <psi.h>
#include <psi/console.h>

static int done;
static PTHREAD pThread;

void CPROC OutputHandle( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 size )
{
   lprintf( "output %s", buffer );
   pcprintf( (PSI_CONTROL)psv, "%s", buffer );
}

void CPROC TaskEnded( PTRSZVAL psv, PTASK_INFO task )
{
	done = TRUE;
   WakeThread( pThread );
}

void CPROC WindowInput( PTRSZVAL psv, PTEXT text )
{
	PTEXT out = BuildLine( text );
   LogBinary( GetText( out ), GetTextSize( out ) );
	pprintf( (PTASK_INFO)psv, "%s", GetText( out ) );
   LineRelease( out );
}

int main( int argc, char **argv )
{
	PTASK_INFO task;
   PSI_CONTROL pc;
	pc = MakeNamedCaptionedControl( NULL, "PSI Console", 0, 0, 640, 480, INVALID_INDEX, "Command Prompt" );
	DisplayFrame( pc );

	//task = LaunchPeerProgram( argc>1?argv[1]:"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe", ".", NULL, OutputHandle, TaskEnded, (PTRSZVAL)pc );
	task = LaunchPeerProgram( argc>1?argv[1]:"cmd.exe", ".", NULL, OutputHandle, TaskEnded, (PTRSZVAL)pc );
	if( task )
	{
		PSIConsoleInputEvent( pc, WindowInput, (PTRSZVAL)task );
		pThread = MakeThread();
		while( !done )
		{
			WakeableSleep( 25000 );
		}
	}
	return 0;
}

