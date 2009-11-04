#include <stdhdrs.h>
#include <sack_types.h>
#include <logging.h>
#include <timers.h>

void RunServer( void )
{
	PMSGQUEUE pmq_in = CreateMsgQueue( WIDE("test in"), 16384 );
	PMSGQUEUE pmq_out = CreateMsgQueue( WIDE("test out"), 16384 );
	if( pmq_in && pmq_out )
	{
		_32 msg[256];
      _32 msgsize;
		msgsize = DequeMsg( pmq_in, 0, msg, sizeof( msg ), 0 );
		lprintf( WIDE("server Received Message...") );
      EnqueMsg( pmq_out, msg, msgsize, 0 );
	}
}

void RunClient( void )
{
	PMSGQUEUE pmq_in = CreateMsgQueue( WIDE("test out"), 16384 );
	PMSGQUEUE pmq_out = CreateMsgQueue( WIDE("test in"), 16384 );
	if( pmq_in && pmq_out )
	{
		_32 msg[256];
		_32 msgsize;
      msg[0] = 1;
		EnqueMsg( pmq_out, msg, msgsize, 0 );
		msgsize = DequeMsg( pmq_in, 0, msg, sizeof( msg ), 0 );
		lprintf( WIDE("client Received Message...") );
	}

}

int main( int argc, char **argv )
{
	SetSystemLog( SYSLOG_FILE, stdout );
   SystemLogTime( SYSLOG_TIME_CPU|SYSLOG_TIME_DELTA );
	if( argc > 1 )
	{
		if( stricmp( argv[1], WIDE("server") ) == 0 )
		{
         ThreadTo( RunServer, 0 );
		}
		if( stricmp( argv[1], WIDE("client") ) == 0 )
		{
         ThreadTo( RunClient, 0 );
		}
		while( 1 )
         usleep( 1000000 );
	}
}

