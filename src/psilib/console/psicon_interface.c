
#include <psi.h>
#include <psi/console.h>

#include "consolestruc.h"
#include "WinLogic.h"

PSI_CONSOLE_NAMESPACE

	extern CONTROL_REGISTRATION ConsoleClass;
static PTEXT eol;

int PSIConsoleOutput( PSI_CONTROL pc, PTEXT lines )
{
	ValidatedControlData( PCONSOLE_INFO, ConsoleClass.TypeID, console, pc );
	// ansi filter?
	// conditions for getting text lines which have format elements
	// break lines?
	if( !eol )
      eol = SegCreateFromText( "\n" );
	if( console )
	{
		PTEXT parsed;
		PTEXT next;
      PTEXT remainder = NULL;
      PTEXT tmp;
		parsed = burst( lines );
		for( tmp = parsed; tmp; tmp = next )
		{
         next = NEXTLINE( tmp );
			if( !GetTextSize( tmp ) )
			{
				PTEXT prior;
            PTEXT que;
				prior = SegBreak( tmp );
				if( prior )
				{
					SetStart( prior );
               prior->format.position.offset.spaces += console->pending_spaces;
               prior->format.position.offset.tabs += console->pending_tabs;
					que = BuildLine( prior );
					if( !console->flags.bNewLine )
                  que->flags |= TF_NORETURN;
					WinLogicWriteEx( console, que, 0 );
					LineRelease( prior );
				}
            else
					WinLogicWriteEx( console, SegCreate( 0 ), 0 );

				console->flags.bNewLine = 1;

            // throw away the blank... don't really need it on the display
				SegGrab( tmp );
            console->pending_spaces = tmp->format.position.offset.spaces;
            console->pending_tabs = tmp->format.position.offset.tabs;
				LineRelease( tmp );
            remainder = next;
			}
		}
		if( remainder )
		{
			PTEXT que = BuildLine( remainder );
			WinLogicWriteEx( console, que, 0 );
			console->flags.bNewLine = 0;
		}
		else
		{
			console->flags.bNewLine = 1;
		}
      //LineRelease( parsed );
		//PTEXT debug =DumpText( parsed );
		//lprintf( "%s", GetText( debug ) );
		//LineRelease( debug );
		//WinLogicWrite( console, lines );
		//lprintf( WIDE("Smudging") );
		SmudgeCommon( pc );
		//lprintf( WIDE("Smudged") );
	}
	return 0;
}

void PSIConsoleInputEvent( PSI_CONTROL pc, void(CPROC*Event)(PTRSZVAL,PTEXT), PTRSZVAL psv )
{
	ValidatedControlData( PCONSOLE_INFO, ConsoleClass.TypeID, console, pc );
	if( console )
	{
      // this should be set with an appropriate method.
		console->CommandInfo->CollectedEvent = Event;
      console->CommandInfo->psvCollectedEvent = psv;
	}
}

void PSIConsoleLoadFile( PSI_CONTROL pc, CTEXTSTR file )
{
	// reset history and read a file into history buffer
	// this also implies setting the cursor position at the start of the history buffer

}

int vpcprintf( PSI_CONTROL pc, CTEXTSTR format, va_list args )
{
	ValidatedControlData( PCONSOLE_INFO, ConsoleClass.TypeID, console, pc );
	if( console )
	{
		PVARTEXT pvt = VarTextCreate();
		PTEXT output;
		vvtprintf( pvt, format, args );
		output = VarTextGet( pvt );
		PSIConsoleOutput( pc, output );
	}
	return 1;
}

int pcprintf( PSI_CONTROL pc, CTEXTSTR format, ... )
{
	va_list args;
	va_start( args, format );
	return vpcprintf( pc, format, args );
}

PSI_CONSOLE_NAMESPACE_END
