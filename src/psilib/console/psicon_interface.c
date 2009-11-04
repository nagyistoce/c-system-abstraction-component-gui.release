
#include <psi.h>
#include <psi/console.h>

#include "consolestruc.h"
#include "WinLogic.h"

PSI_CONSOLE_NAMESPACE

extern CONTROL_REGISTRATION ConsoleClass;

int PSIConsoleOutput( PSI_CONTROL pc, PTEXT lines )
{
	ValidatedControlData( PCONSOLE_INFO, ConsoleClass.TypeID, console, pc );
	// ansi filter?
	// conditions for getting text lines which have format elements
	// break lines?
	if( console )
	{
      PTEXT parsed = burst( lines );
		WinLogicWrite( console, parsed );
		//lprintf( WIDE("Smudging") );
		SmudgeCommon( pc );
		//lprintf( WIDE("Smudged") );
	}
	return 0;
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
