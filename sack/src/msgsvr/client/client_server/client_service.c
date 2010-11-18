#include <stdio.h>
#include <msgclient.h>
#include <timers.h>


_32 BaseID;
PTRSZVAL CPROC ClientThread( PTHREAD thread )
{
	int n;
   _32 mid = GetThreadParam( thread );

	for( n = 0; n < 10; n++ )
	{
      SendServiceEvent( mid, BaseID + n, NULL, 0 );
	}
   return 0;
}


static int CPROC ServiceFunction( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length )
{
	// echo the data we got back to the client...
// most other things will do useful functions in functions.
   //ThreadTo( ClientThread, params[-1] );
	MemCpy( result, params, param_length );
   *result_length = param_length;
   return TRUE;
}

static int CPROC ServiceFunction2( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length )
{
	// echo the data we got back to the client...
// most other things will do useful functions in functions.
	//ThreadTo( ClientThread, params[-1] );
   WakeableSleep( 333 );
	MemCpy( result, params, param_length );
   *result_length = param_length;
   return TRUE;
}


static int CPROC MessageHandler( _32 source, _32 MsgID
										 , _32 *params, _32 param_length
										 , _32 *result, _32 *result_length )
{
	switch( MsgID )
	{
	case MSG_MateEnded:
      return TRUE;
	case MSG_MateStarted:
		printf( WIDE("Client %ld is connecting to %s\n"), source, params );
      result[0] = 256;
		return TRUE;
	case MSG_EventUser:
		//ThreadTo( ClientThread, source );
      MemCpy( result, params, param_length );
      return TRUE;
	case MSG_EventUser + 1:
      WakeableSleep( 1000 ); // need to wait less than 3000...
      MemCpy( result, params, param_length );
      return TRUE;
	default:
		printf( WIDE("Received message %ld from %ld\n"), MsgID, source );
      break;
	}
   return FALSE;
}

#define NUM_FUNCTIONS (sizeof(functions)/sizeof( server_function))
SERVER_FUNCTION functions[] =
{
	ServerFunctionEntry( NULL ) // disconnect
, ServerFunctionEntry( NULL  ) // connect
, ServerFunctionEntry( NULL ) // nothing
, ServerFunctionEntry( NULL ) // nothing
, ServerFunctionEntry( ServiceFunction )
, ServerFunctionEntry( ServiceFunction2 )
};

int main( int argc, char **argv )
{
	printf( "Usage: %s <first service>\n" );
	printf( "   <if no service, registers 'Test Service 1'  Service has two methods\n" );
	printf( "     method 1 - reply with same data\n" );
	printf( "     method 2 - reply with same data after wiating 333 milliseconds\n" );
   printf( "      (uses function table registration)\n" );
	printf( "   <if service, registers  Service has two methods\n" );
	printf( "     method 1 - reply with same data\n" );
	printf( "     method 2 - reply with same data after wiating 1000 milliseconds\n" );
   printf( "      (uses handler function registration)\n" );
   printf( "both attempt to dump current service log...\n" );

	if( argc < 2 )
	{
		if( BaseID = RegisterService( WIDE("Test Service 1"), functions, NUM_FUNCTIONS ) )
		{
         // dumps to log.
         DumpServiceList();
			while( 1 )
				WakeableSleep( SLEEP_FOREVER );
		}
		else
			printf( WIDE("Sorry, could not register a service.") );
	}
	else
	{
		char logname[64];
		sprintf( logname, WIDE("client_service_%s.log"), argv[1] );
      SetSystemLog( SYSLOG_FILENAME, logname );
		if( BaseID = RegisterServiceHandler( argv[1], MessageHandler ) )
		{
         // dumps to log.
         DumpServiceList();
			while( 1 )
				WakeableSleep( SLEEP_FOREVER );
		}
		else
			printf( WIDE("Sorry, could not register a service.") );
	}
   return 1;
}