#include <stdhdrs.h>
#include <msgclient.h>
#include <deadstart.h>
#include <construct.h>
#include <sqlgetoption.h>
#include "summoner.h"

#ifdef __cplusplus
namespace sack { namespace task { namespace construct {
using namespace sack::msg::client;
#endif

typedef struct local_tag
{
	_32 MsgBase;
	TEXTCHAR my_name[256];
	// handle a registry of external functions to call
	// for alive checks... otherwise be simple and claim
   // alive (at least the message services are alive)
} LOCAL;

static LOCAL l;

static int CPROC HandleSummonerEvents( _32 SourceID, _32 MsgID, _32 *data, _32 len )
{
	switch( MsgID )
	{
	case MSG_RU_ALIVE:
		// echo the data back to the server..
		// this helps mate requests and responces...
		SendRoutedServerMessage( SourceID, MSG_IM_ALIVE, data, len );
		break;
	case MSG_DIE:
		lprintf( WIDE("Command to die, therefore I shall...") );
		exit(0);
		break;
	default:
		lprintf( WIDE("Received unknown message %") _32f WIDE(" from %") _32f WIDE(""), MsgID, SourceID );
		break;
	}
   return TRUE;
}

//#if 0
PRELOAD( Started )
{
#ifndef __NO_OPTIONS__
	if( SACK_GetProfileIntEx( "SACK/Summoner", "Auto register with summoner?", 0, TRUE ) )
#else
   if( 0 )
#endif
	{
		l.MsgBase = LoadServiceEx( SUMMONER_NAME, HandleSummonerEvents );
		//lprintf( WIDE("Message base for service is %d"), l.MsgBase );
		if( l.MsgBase != INVALID_INDEX )
		{
			_32 result, result_length;
			result_length = sizeof( l.my_name );
			if( !TransactServerMessage( l.MsgBase + MSG_WHOAMI, NULL, 0
											  , &result, l.my_name, &result_length ) )
			{
				// since we JUST loaded it, this shold be nearly impossible to hit.
				lprintf( WIDE("Failed to find out who I am from summoner.") );
				UnloadService( l.MsgBase );
				l.MsgBase = INVALID_INDEX;
				return;
			}
			else if( result != ((MSG_WHOAMI+l.MsgBase)|SERVER_SUCCESS ) )
			{
				lprintf( WIDE("Server responce was in error... disable support") );
				UnloadService( l.MsgBase );
				l.MsgBase = INVALID_INDEX;
				return;
			}
			else if( !result_length )
			{
				lprintf( WIDE("Summoner is not responsible for us, and requires no notifications." ) );
				UnloadService( l.MsgBase );
				l.MsgBase = INVALID_INDEX;
				return;
			}
			//else l.my_name is my task name from sommoner.config
			if( !TransactServerMessage( l.MsgBase + MSG_IM_STARTING, l.my_name, (_32)strlen( l.my_name ) + 1
											  , NULL, NULL, 0 ) )
			{
				// this should almost be guaranteed to work...
				lprintf( WIDE("Failed to send starting to summoner... disable support") );
				UnloadService( l.MsgBase );
				l.MsgBase = INVALID_INDEX;
				return;
			}
			//lprintf( WIDE("We're starting, go ahead.") );
		}
	}
	else
		l.MsgBase = INVALID_INDEX;

}
//#endif

CONSTRUCT_PROC( void, LoadComplete )( void )
{
	_32 result;
   // if we registered with the summoner...
	if( l.MsgBase != INVALID_INDEX )
	{
		//lprintf( WIDE("Sending IM_READY to summoner...\n") );
		result = ((l.MsgBase+MSG_IM_READY) | SERVER_SUCCESS);
		if( TransactServerMessage( l.MsgBase + MSG_IM_READY, l.my_name, (_32)strlen( l.my_name ) + 1
										 , NULL /*&result*/, NULL, 0 )
		  )
		{
			if( result == ((l.MsgBase+MSG_IM_READY) | SERVER_SUCCESS) )
			{
			}
			else
			{
				lprintf( WIDE("Summoner has somehow complained that we're started?!") );
				DebugBreak();
			}
		}
		else
		{
			lprintf( WIDE("Summoner has dissappeared.  Disabling support.") );
			l.MsgBase = 0;
		}
	}
	//else
   //   lprintf( WIDE("Service has been disabled.") );
}

ATEXIT( Ended )
{
	if( l.MsgBase != INVALID_INDEX )
	{
		lprintf( WIDE("mark ready in summoner (dispatch as ended?)") );
		LoadComplete();
		UnloadService( l.MsgBase );
	}
}

#ifdef __cplusplus
}}} //namespace sack namespace 
#endif
