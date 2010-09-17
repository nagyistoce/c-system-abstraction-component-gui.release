/*
 *  bdata_tcp_relay.c
 *  Copyright: FortuNet, Inc.  2006
 *  Product:  BingLink      Project: alpha2server   Version: Alpha2
 *  Contributors:    Jim Buckeyne, Christopher Green
 *  Manages socket communication between video server and bdata controllers.
 *
 */


#include <stdhdrs.h>
#include <sharemem.h>
#include <timers.h>
#include <network.h>
//#include <flashdrive.h>
#include "server.h"


#define DEFAULT_PORT 6594

// if a second host connects, close the first, then set this variable.
PCLIENT pcHost; // the current hosting bdata server...
// bdata boxes connect to this port.
PCLIENT pcHostListen;
// the timer to make new connections to enabled clients, if hosting
_32 state_poll_timer;
// a static buffer to read from the host and send to participants
_8 buffer[4096];

int added_one;
int bUDPNetwork;
int bTCPNetwork;

void CPROC ServerRecieve( PCLIENT pc, POINTER buf, int size )
{
   if( buf )
	{
		PBINGHALL hall;
		INDEX idx;

		LIST_FORALL( l.pHallList, idx, PBINGHALL, hall )
		{
			if( hall->LinkHallState.pcBdata  )
			{
				//xlprintf(LOG_NOISE)("[SR][ESS ARE] sending to %s"
				//						 , hall->stIdentity.szBdataAddr
				//						 );
				SendTCP( hall->LinkHallState.pcBdata, buf, size );
			}
		}
	}
   // don't do anything after ReadTCP
	ReadTCP( pc, buffer, 4097 );
}
void CPROC ClientClosed( PCLIENT pc )
{
	PBINGHALL hall;
	INDEX idx;

	lprintf( "Master connection has terminated, killing all clients." );
	xlprintf(LOG_NOISE)("[BTR][BEE TEA ARE]Client %p went away.\n", pc );
	LIST_FORALL( l.pHallList, idx, PBINGHALL, hall )
	{
		if( hall->LinkHallState.pcBdata )
		{
			xlprintf(LOG_NOISE)("[BTR][BEE TEA ARE] Terminating %p"
									 , hall->LinkHallState.pcBdata
									 );
			RemoveClient( hall->LinkHallState.pcBdata);
		}
	}
   l.pMyHall->LinkHallState.flags.bConnected = 0;
	l.pMyHall->LinkHallState.flags.bHostAssigned = 0;

	pcHost = NULL;

}

void CPROC ClientClosing( PCLIENT pc )
{
	PBINGHALL hall;
	INDEX idx;

	LIST_FORALL( l.pHallList, idx, PBINGHALL, hall )
	{
		if( hall->LinkHallState.pcBdata == pc )
		{
			xlprintf(LOG_NOISE)("[BTR][BEE TEA ARE] Closing %p"
									 , hall->LinkHallState.pcBdata
									 );
			hall->LinkHallState.nTickClosed = GetTickCount();
			hall->LinkHallState.pcBdata  = NULL;
			break;
		}
	}
	if( !hall )
	{
		lprintf( "FAILED TO FIND THE hall state bdata client to disconnect" );
	}

}

void CPROC ClientConnected( PCLIENT pListen, PCLIENT pNew )
{
	//xlprintf(LOG_NOISE)("[BTR][BEE TEA ARE] Client %p connected"
	//							 , pNew
	//							 );
	if( pcHost )
	{
      lprintf( "was previously hosting someone, and I've been reconnected to!!!!" );
		RemoveClient( pcHost );
	}
   lprintf( "beginning new pcHost %p", pNew );
   pcHost = pNew;
	SetNetworkReadComplete( pNew, ServerRecieve );
	SetNetworkCloseCallback( pNew, ClientClosed );
}


void CPROC ConnectTimer( PTRSZVAL psv )
{
	// reconnect/initial connect to client bdata boxes...
	// for all halls, which are enabled, not prohibited, and are not this
	// current connecting hall...
	PBINGHALL hall;
	INDEX idx;
   // not quiet done with init yet...
	if( !l.pMyHall )
	{
      lprintf( "my local hall is not setup... waiting..." );
		return;
	}
	if( ( ( l.pMyHall->LinkHallState.prohibited ) ||
		 ( !l.pMyHall->LinkHallState.enabled ) ||
       ( l.pMyHall->LinkHallState.hall_id !=  l.current_state.LinkState.master_hall_id ) ||
		 ( !l.current_state.LinkState.master_hall_id )
		)
	  )
	{
		//lprintf( "Terminating host... who is no longer enabled or prohib" );
		if( pcHost  &&
			bTCPNetwork ) // this is a truth flag if stations_netowrk is enabled...
		{
         char cmd[80];
			union {
				_32 dw;
            unsigned char bytes[4];
			} IP;
			IP.dw = GetNetworkLong( pcHost, GNL_IP );
			snprintf( cmd, sizeof( cmd ), "%s/bdata_control slave %d.%d.%d.%d"
					  , l.command.szExtendedScriptSupportPath
					  , IP.bytes[0]
					  , IP.bytes[1]
					  , IP.bytes[2]
					  , IP.bytes[3] );
			lprintf( "Bdata command: %s", cmd );
         system( cmd );
			//RemoveClient( pcHost );
		}
	}

	// otherwise, in the following state, we're master, and in a ready state....
	// for each hall enabled, not prohibited, and not myself, add to list of
   // BDATA recipients.
	if( l.pMyHall->LinkHallState.master_ready && 
		l.current_state.LinkState.master_hall_id 
		)
	{
		LIST_FORALL( l.pHallList, idx, PBINGHALL, hall )
		{
			if( ( hall->LinkHallState.hall_id != l.current_state.LinkState.master_hall_id ) )
			{
				//lprintf( "Foreign hall" );
				if(   ( !hall->LinkHallState.prohibited ) &&
					  ( hall->LinkHallState.enabled ) 
					  )					
				{
					// old TCP network ...
					if( pcHost  &&
						bTCPNetwork )
					{
						//lprintf( "Enabled, and allowed" );
						if( !hall->LinkHallState.pcBdata  )
						{
							if( !hall->LinkHallState.nTickClosed ||
								( ( hall->LinkHallState.nTickClosed + 1000 ) < GetTickCount() ) )
							{
								if( hall->LinkHallState.flags.uiBdataFailures < 3 )
								{
									hall->LinkHallState.pcBdata =  OpenTCPClientEx( hall->stIdentity.szBdataAddr, 6740
																								 , NULL, ClientClosing, NULL );
									if(  ( g.flags.bDeleteLinkOnOpenTCPFail ) &&
										( !hall->LinkHallState.pcBdata )
									  )
									{
										hall->LinkHallState.flags.uiBdataFailures++;
										xlprintf(LOG_NOISE)("hall->LinkHallState.pcBdata is %p and is no bdata connectivity to %s(%lu), uiBdataFailures for this hall is %lu", hall->LinkHallState.pcBdata, hall->stIdentity.szSiteName	, hall->LinkHallState.hall_id	, hall->LinkHallState.flags.uiBdataFailures	);
									}
									else
									{
										hall->LinkHallState.flags.uiBdataFailures = 0;
									}
								}
							}
						}
					}
				}
				else
				{
					if( pcHost  &&
						bTCPNetwork )
					{
						if( hall->LinkHallState.pcBdata )
						{
							RemoveClient( hall->LinkHallState.pcBdata );
						}
					}
				}
			}
		}
	}
}

PRELOAD( InitBdataRelayService )
{
   int add_timer = 0;
	if( bTCPNetwork = SACK_GetPrivateProfileInt( "Bdata Link TCP", "Enable", 0, "server.ini" ) )
	{
		SOCKADDR *addr;
		NetworkWait( NULL, 256, 16 );

		addr = CreateSockAddress( "0.0.0.0", DEFAULT_PORT );
		pcHostListen = OpenTCPListenerAddrEx( addr, ClientConnected );  // creates the socket from the return value of CreateSockAddress

		if(!pcHostListen)
		{
			lprintf( "Failed to open server port for bdata connection!!!!" );
		}
		else
			add_timer = 1;
	}
   if( add_timer )
		AddTimer( 100, ConnectTimer, 0 );
}

