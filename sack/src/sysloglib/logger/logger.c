#include <stdhdrs.h>
#ifndef __LINUX__
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <network.h>
#include <timers.h>

PCLIENT pcListen;
FILE *logfile;
_32 tick, _tick;

void CPROC LogRead( PCLIENT pc, POINTER buffer, int nSize, SOCKADDR *sa )
{
   tick = GetTickCount();
	if( !buffer )
	{
		buffer = Allocate( 4096 );
      _tick = tick;
	}
	else
	{
		char msgtime[12];
		write( 1, msgtime, sprintf( msgtime, WIDE("%04") _32f WIDE("|"), tick - _tick ) );
      _tick = tick;
		write( 1, buffer, nSize );
      if( nSize < 4096 )
			write( 1, WIDE("\n"), 1 );
	}
   ReadUDP( pc, buffer, 4096 );
}

// command line option
//  time
//  output file...
int main( int argc, char **argv )
{
   NetworkStart();
	pcListen = ServeUDP( WIDE("0.0.0.0"), 514, LogRead, NULL );
	while( pcListen ) WakeableSleep( SLEEP_FOREVER );
	printf( WIDE("Failed to start.") );
   return 1;
}
