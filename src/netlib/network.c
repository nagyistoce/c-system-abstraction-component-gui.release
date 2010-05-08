///////////////////////////////////////////////////////////////////////////
//
// Filename    -  Network.C
//
// Description -  Network services for Communications Client
//
// Author      -  James Buckeyne
//
// Create Date -  Before now.
// Conversion update for Linux GLIBC 2.1 9/26/2000
//
///////////////////////////////////////////////////////////////////////////

//
//  DEBUG FLAGS IN netstruc.h
//

#include <stdhdrs.h>
#include <stddef.h>
#include <ctype.h>
#include <sack_types.h>
#include <deadstart.h>
#include <sqlgetoption.h>
#define MAIN_PROGRAM
#include "netstruc.h"
#include <network.h>

//#define DO_LOGGING // override no _DEBUG def to do loggings...
//#define NO_LOGGING // force neverlog....

#include <logging.h>
#include <procreg.h>
#ifdef __LINUX__
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#endif

#include <sharemem.h>
#include <timers.h>
#include <idle.h>

//for GetMacAddress
#ifdef __LINUX__
#include <net/if.h>
#include <sys/timeb.h>

//*******************8
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
//*******************8

#endif
#ifdef __WINDOWS__
#include <windows.h>
#include <stdio.h>
#ifdef __CYGWIN__
#include <mingw/tchar.h>
#else
#include <tchar.h>
#endif
#include <iphlpapi.h>
#endif

//for GetMacAddress

SACK_NETWORK_NAMESPACE

static LOGICAL bThreadInitComplete = FALSE
    , bThreadExit = FALSE
    , bNetworkReady
    , bThreadInitOkay = TRUE
	;

PRIORITY_PRELOAD( InitGlobal, GLOBAL_INIT_PRELOAD_PRIORITY )
{
	SimpleRegisterAndCreateGlobal( global_network_data );
	if( !g.system_name )
	{
  		g.system_name = WIDE("no.network");
	}
#ifndef __NO_OPTIONS__
	g.dwReadTimeout = SACK_GetProfileIntEx( "SACK", "Network/Read wait timeout", 5000, TRUE );
	g.dwConnectTimeout = SACK_GetProfileIntEx( "SACK", "Network/Connect timeout", 10000, TRUE );
#else
	g.dwReadTimeout = 5000;
	g.dwConnectTimeout = 10000;
#endif
}

//----------------------------------------------------------------------------
// forward declaration for the window proc...
_TCP_NAMESPACE 
void AcceptClient(PCLIENT pc);
int TCPWriteEx(PCLIENT pc DBG_PASS);
#define TCPWrite(pc) TCPWriteEx(pc DBG_SRC)

int FinishPendingRead(PCLIENT lpClient DBG_PASS );
LOGICAL TCPDrainRead( PCLIENT pClient );

void DumpSocket( PCLIENT pc );
_TCP_NAMESPACE_END

//----------------------------------------------------------------------------
#if defined( WIN32 ) && defined( NETDLL_EXPORTS ) || defined( __LCC__ )
LOGICAL APIENTRY DllMain( HINSTANCE hModule,
                       _32  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
   return TRUE;
}
#endif


//----------------------------------------------------------------------------
//
//    GetMacAddress( version cpg01032007 )
//
//----------------------------------------------------------------------------
NETWORK_PROC( int, GetMacAddress)(PCLIENT pc )//int get_mac_addr (char *device, unsigned char *buffer)
{
#ifdef __LINUX__
#ifdef __THIS_CODE_GETS_MY_MAC_ADDRESS___
	int fd;
	struct ifreq ifr;

	fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (fd == -1)
	{
		lprintf(WIDE ("Unable to create socket for pclient: %p"), pc);
		return -1;
	}

	strcpy (ifr.ifr_name, GetNetworkLong(pc,GNL_IP));

	if (ioctl (fd, SIOCGIFFLAGS, &ifr) < 0)
	{
		close (fd);
		return -1;
	}

	if (ioctl (fd, SIOCGIFHWADDR, &ifr) < 0)
	{
		close (fd);
		return -1;
	}

	close (fd);

	memcpy (pc->hwClient, ifr.ifr_hwaddr.sa_data, 6);

	return 0;
#endif
   /* this code queries the arp table to figure out who the other side is */
	//int fd;
	struct arpreq arpr;
   struct ifconf ifc;
	MemSet( &arpr, 0, sizeof( arpr ) );
   lprintf( "this is broken." );
	MemCpy( &arpr.arp_pa, pc->saClient, sizeof( SOCKADDR ) );
	arpr.arp_ha.sa_family = AF_INET;
	{
		char buf[256];
		ifc.ifc_len = sizeof( buf );
		ifc.ifc_buf = buf;
		ioctl( pc->Socket, SIOCGIFCONF, &ifc );
		{
         int i;
			struct ifreq *IFR;
			IFR = ifc.ifc_req;
			for( i = ifc.ifc_len / sizeof( struct ifreq); --i >=0; IFR++ )
			{
				printf( "IF: %s\n", IFR->ifr_name );
				strcpy( arpr.arp_dev, "eth0" );
			}
		}
	}
   DebugBreak();
	if( ioctl( pc->Socket, SIOCGARP, &arpr ) < 0 )
	{
		lprintf( "Error of some sort ... %s", strerror( errno ) );
      DebugBreak();
	}

	return 0;
#endif
#ifdef __WINDOWS__

    HRESULT hr;
    ULONG   ulLen;
	
	ulLen = 6;
    
	//needs ws2_32.lib and iphlpapi.lib in the linker.
//	hr = SendARP (GetNetworkLong(pc,GNL_IP), 0, (PULONG)pc->hwClient, &ulLen);
//  The second parameter of SendARP is a PULONG, which is typedef'ed to a pointer to 
//  an unsigned long.  The pc->hwClient is a pointer to an array of _8 (unsigned chars), 
//  actually defined in netstruc.h as _8 hwClient[6]; Well, in the end, they are all
//  just addresses, whether they be address to information of eight bits in length, or
//  of (sizeof(unsigned)) in length.  Although this may, in the future, throw a warning.
	hr = SendARP (GetNetworkLong(pc,GNL_IP), 0, (PULONG)pc->hwClient, &ulLen);
    lprintf (WIDE("Return %08x, length %8d\n"), hr, ulLen);

	return 0;
#endif
}

//----------------------------------------------------------------------------

void DumpLists( void )
{
	int c = 0;
	PCLIENT pc;
	for( pc = g.AvailableClients; c < 50 && pc; pc = pc->next )
	{
		//lprintf( "Available %p", pc );
		if( (*pc->me) != pc )
         DebugBreak();
      c++;
	}
	if( c > 50 )
	{
      lprintf( WIDE( "Overflow available clients." ) );
		//DebugBreak();
	}

	c = 0;
	for( pc = g.ActiveClients; c < 50 && pc; pc = pc->next )
	{
		//lprintf( WIDE( "Active %p(%d)" ), pc, pc->Socket );
		if( (*pc->me) != pc )
         DebugBreak();
      c++;
	}
	if( c > 50 )
	{
      lprintf( WIDE( "Overflow active clients." ) );
		DebugBreak();
	}

	c = 0;
	for( pc = g.ClosedClients; c < 50 && pc; pc = pc->next )
	{
		//lprintf( WIDE( "Closed %p(%d)" ), pc, pc->Socket );
		if( (*pc->me) != pc )
         DebugBreak();
      c++;
	}
	if( c > 50 )
	{
      lprintf( WIDE( "Overflow closed clients." ) );
		DebugBreak();
	}
}


//----------------------------------------------------------------------------

SOCKADDR *AllocAddr( void )
{
#define MAGIC_LENGTH sizeof(SOCKADDR_IN)< 256?256:sizeof( SOCKADDR_IN)
	SOCKADDR *lpsaAddr=(SOCKADDR*)Allocate( MAGIC_SOCKADDR_LENGTH + sizeof(PTRSZVAL) );
   lpsaAddr = (SOCKADDR*)( ( (PTRSZVAL)lpsaAddr ) + sizeof(PTRSZVAL) );
   MemSet( lpsaAddr, 0, MAGIC_SOCKADDR_LENGTH );
   return lpsaAddr;
}
//----------------------------------------------------------------------------

PCLIENT GrabClientEx( PCLIENT pClient DBG_PASS )
#define GrabClient(pc) GrabClientEx( pc DBG_SRC )
{
	if( pClient )
	{
#ifdef LOG_CLIENT_LISTS
		_lprintf(DBG_RELAY)( WIDE( "grabbed client %p(%d)" ), pClient, pClient->Socket );
		lprintf( WIDE( "grabbed client %p Ac:%p(%p(%d)) Av:%p(%p(%d)) Cl:%p(%p(%d))" )
				 , pClient->me
					 , &g.ActiveClients, g.ActiveClients, g.ActiveClients?g.ActiveClients->Socket:0
					 , &g.AvailableClients, g.AvailableClients, g.AvailableClients?g.AvailableClients->Socket:0
					 , &g.ClosedClients, g.ClosedClients, g.ClosedClients?g.ClosedClients->Socket:0
				 );
		DumpLists();
#endif
#ifdef USE_WSA_EVENTS
		//if( pClient->dwFlags & CF_ACTIVE )
		{
         //lprintf( "SET EVENT" );
			//SetEvent( g.event );
		}
#endif
		pClient->dwFlags &= ~CF_STATEFLAGS;
		pClient->LastEvent = GetTickCount();
		if( ( (*pClient->me) = pClient->next ) )
			pClient->next->me = pClient->me;
	}
	return pClient;
}

//----------------------------------------------------------------------------

PCLIENT AddAvailable( PCLIENT pClient )
{
	if( pClient )
	{
#ifdef LOG_CLIENT_LISTS
		lprintf( WIDE( "Add Avail client %p(%d)" ), pClient, pClient->Socket );
		DumpLists();
#endif
		pClient->dwFlags |= CF_AVAILABLE;
		pClient->LastEvent = GetTickCount();
		pClient->me = &g.AvailableClients;
		if( ( pClient->next = g.AvailableClients ) )
			g.AvailableClients->me = &pClient->next;
		g.AvailableClients = pClient;
	}
	return pClient;
}

//----------------------------------------------------------------------------

PCLIENT AddActive( PCLIENT pClient )
{
	if( pClient )
	{
#ifdef LOG_CLIENT_LISTS
		lprintf( WIDE( "Add Active client %p(%d)" ), pClient, pClient->Socket );
		DumpLists();
#endif
		pClient->dwFlags |= CF_ACTIVE;
		pClient->LastEvent = GetTickCount();
		pClient->me = &g.ActiveClients;
		if( ( pClient->next = g.ActiveClients ) )
			g.ActiveClients->me = &pClient->next;
		g.ActiveClients = pClient;
		/*
       // something else will probably happen after getting activated.
#ifdef USE_WSA_EVENTS
      // an active client needs to be scheduled for select.
		lprintf( "SET EVENT" );
		SetEvent( g.event );
#endif
      */

	}
	return pClient;
}

//----------------------------------------------------------------------------

PCLIENT AddClosed( PCLIENT pClient )
{
	if( pClient )
	{
#ifdef LOG_CLIENT_LISTS
		lprintf( WIDE( "Add Closed client %p(%d)" ), pClient, pClient->Socket );
		DumpLists();
#endif
		pClient->dwFlags |= CF_CLOSED;
		pClient->LastEvent = GetTickCount();
		pClient->me = &g.ClosedClients;
		if( ( pClient->next = g.ClosedClients ) )
			g.ClosedClients->me = &pClient->next;
		g.ClosedClients = pClient;
	}
	return pClient;
}

//----------------------------------------------------------------------------

void ClearClient( PCLIENT pc )
{
   POINTER pbtemp;
   PCLIENT next;
	PCLIENT *me;
	CRITICALSECTION cs;
   // keep the closing flag until it's really been closed. (getfreeclient will try to nab it)
   _32   dwFlags = pc->dwFlags & (CF_STATEFLAGS|CF_CLOSING);
   me = pc->me;
   next = pc->next;
   pbtemp = pc->lpUserData;
   cs = pc->csLock;
	// sets socket to 0 - so it's not quite == INVALID_SOCKET
	MemSet( pc, 0, sizeof( CLIENT ) ); // clear all information...
   pc->csLock = cs;
	pc->lpUserData = (P_8)pbtemp;
   if( pc->lpUserData )
		MemSet( pc->lpUserData, 0, g.nUserData * sizeof( PTRSZVAL ) );
   pc->next = next;
   pc->me = me;
   pc->dwFlags = dwFlags;
}

//----------------------------------------------------------------------------

void TerminateClosedClientEx( PCLIENT pc DBG_PASS )
{
#ifdef VERBOSE_DEBUG
	_lprintf(DBG_RELAY)( WIDE( "terminate? " ) );
#endif
	if( !pc )
		return;
	if( pc->dwFlags & CF_CLOSED )
	{
		PendingBuffer * lpNext;
		EnterCriticalSec( &g.csNetwork );
      /*
		if( !NetworkLock( pc ) )
		{
			lprintf( WIDE( "-------- FAILURE to Lock client..." ) );

			//return;
			}
			*/
      //lprintf( WIDE( "Terminating closed client..." ) );
		if( IsValid( pc->Socket ) )
		{
#ifdef VERBOSE_DEBUG
         lprintf( WIDE( "close socket." ) );
#endif
			closesocket( pc->Socket );
   	   while( pc->lpFirstPending )
      	{
            lpNext = pc->lpFirstPending -> lpNext;

            if( pc->lpFirstPending->s.bDynBuffer )
               Release( pc->lpFirstPending->buffer.p );

            if( pc->lpFirstPending != &pc->FirstWritePending )
            {
#ifdef LOG_PENDING
               {
                  Log( WIDE(WIDE( "Data queued...Deleting in remove." )) );
               }
#endif
	            Release(pc->lpFirstPending);
            }
            else
            {
#ifdef LOG_PENDING
               {
                  Log( WIDE("Normal send queued...Deleting in remove.") );
               }
#endif
            }
            if (!lpNext)
               pc->lpLastPending = NULL;
            pc->lpFirstPending = lpNext;
	      }
		}
		ClearClient( pc );
      // this should move from g.close to g.available.
		AddAvailable( GrabClient( pc ) );
		pc->dwFlags &= ~CF_CLOSING; // it's no longer closing.  (was set during the course of closure)
		LeaveCriticalSec( &g.csNetwork );
      //NetworkUnlock( pc );
	}
	else
		Log( WIDE("Client's state was not CLOSED...") );
}

//----------------------------------------------------------------------------

void CPROC PendingTimer( PTRSZVAL unused )
{
	PCLIENT pc, next;
#ifdef VERBOSE_DEBUG
	Log( WIDE("Enter timer network lock...") );
#endif
	EnterCriticalSec( &g.csNetwork );
#ifdef VERBOSE_DEBUG
	Log( WIDE("Have network lock.") );
#endif
restart:
	for( pc = g.ActiveClients; pc; pc = next )
	{
		// pc can go away during check, so just grab his next now.
		next = pc->next;
#if defined( LOG_CLIENT_LISTS ) && defined( VERBOSE_DEBUG )
		lprintf( "Checking active client %p(%d)", pc, pc->Socket );
#endif
		if( IsValid(pc->Socket) &&
			!(pc->dwFlags & CF_UDP ) )
		{
			//Log( WIDE("Entering non UDP client lock...") );
			if( EnterCriticalSecNoWait( &pc->csLock, NULL ) == 1 )
			{
				LeaveCriticalSec( &g.csNetwork );

				//Log( WIDE("Have non UDP client lock.") );
				if( !(pc->dwFlags & CF_ACTIVE ) )
				{
					// change to inactive status by the time we got here...
					LeaveCriticalSec( &pc->csLock );
					goto restart;
				}
				if( pc->dwFlags & CF_ACTIVE )
				{
					if( pc->lpFirstPending )
					{
						Log( WIDE("Pending Timer:Write") );
						TCPWrite( pc );
					}
					if( (pc->dwFlags & CF_CONNECTED) && (pc->dwFlags & CF_ACTIVE))
					{
						//Log( WIDE("Pending Timer:Read"));
						FinishPendingRead( pc DBG_SRC );
						if( pc->dwFlags & CF_TOCLOSE )
						{
							lprintf( WIDE( "Pending read failed - reset connection. (posting to application)" ) );
							InternalRemoveClientEx( pc, FALSE, FALSE );
						}
					}
				}
				LeaveCriticalSec( &pc->csLock );
				EnterCriticalSec( &g.csNetwork );
			}
			//else
			//   Log( WIDE("Failed network lock on non UDP client.") );
		}
	}
	// fortunatly closed clients are owned by this timer...
	for( pc = g.ClosedClients; pc; pc = next )
	{
		next = pc->next; // will dissappear when closed so save it.
		if( GetTickCount() > (pc->LastEvent + 2000) )
		{
			//Log( WIDE("Closing delay terminated client.") );
			TerminateClosedClient( pc );
		}
	}
	//Log( WIDE("Leaving network lock.") );
	LeaveCriticalSec( &g.csNetwork );
}

#ifndef __LINUX__

//----------------------------------------------------------------------------

static int NetworkStartup( void )
{
   static int attempt = 0;
   static int nStep = 0,
              nError;
   static SOCKET sockMaster;
   static SOCKADDR remSin;
   //WSADATA ws;  // used to start up the socket services...
   // can tick before the timer happens...
	//if( !uNetworkPauseTimer )
	//	return 2;
   switch( nStep )
   {
	case 0 :
		SystemCheck();
      /*

      if( WSAStartup( MAKEWORD(2,0), &ws ) )
      {
         nError = WSAGetLastError();
         Log1( WIDE("Failed network startup! (%d)"), nError );
         nStep = 0; // reset...
         return NetworkQuit();
		}
		lprintf("Winsock Version: %d.%d", LOBYTE(ws.wVersion), HIBYTE(ws.wVersion));
      */
      nStep++;
      attempt = 0;
   case 1 :
		// Sit around, waiting for the network to start...

		//--------------------
		// sorry this is really really ugly to read!
#ifdef __WINDOWS__
		sockMaster = OpenSocket( TRUE, FALSE, FALSE );
		if( sockMaster == INVALID_SOCKET )
		{
			lprintf( WIDE( "Clever OpenSOcket failed... fallback... and survey sez..." ) );
#endif
			//--------------------


			sockMaster = socket( AF_INET, SOCK_DGRAM, 0);


			//--------------------
#ifdef __WINDOWS__
		}
#endif
		//--------------------


      if( sockMaster == INVALID_SOCKET )
		{
			nError = WSAGetLastError();

			lprintf( WIDE( "Failed to create a socket - error is %ld" ), WSAGetLastError() );
			if( nError == 10106 ) // provvider init fail )
			{
            if( ++attempt >= 30 ) return NetworkQuit();
            return -2;
			}
         if( nError == WSAENETDOWN )
         {
            if( ++attempt >= 30 ) return NetworkQuit();
            //else return NetworkPause(WIDE( "Socket is delaying..." ));
         }
          nStep = 0; // reset...
			if( ++attempt >= 30 ) return NetworkQuit();
         return 0;//NetworkQuit();
      }

      // Retrieve my IP address and UDP Port number
      remSin.sa_family=AF_INET;
      remSin.sa_data[0]=0;
      remSin.sa_data[1]=0;
      ((SOCKADDR_IN*)&remSin)->sin_addr.s_addr=INADDR_ANY;

      nStep++;
      attempt = 0;
      // Fall into next state..............
   case 2 :
      // Associate an address with a socket. (bind)
      if( bind( sockMaster, (PSOCKADDR)&remSin, sizeof(remSin))
          == SOCKET_ERROR )
      {
         if( WSAGetLastError() == WSAENETDOWN )
         {
            if( ++attempt >= 30 ) return NetworkQuit();
            //else return NetworkPause(WIDE( "Bind is Delaying" ));
         }
         nStep = 0; // reset...
         return NetworkQuit();
      }
      nStep++;
      attempt = 0;
      // Fall into next state..............
   case 3 :
      closesocket(sockMaster);
      sockMaster = INVALID_SOCKET;
      nStep = 0; // reset...
      break;
   }
   return 0;
}

//----------------------------------------------------------------------------

void CPROC NetworkPauseTimer( PTRSZVAL psv )
{
   int nResult;
   nResult = NetworkStartup();
   if( nResult == 0 )
   {
   	while( !g.uNetworkPauseTimer )
   		Relinquish();
      RemoveTimer( g.uNetworkPauseTimer );
      bNetworkReady = TRUE;
      bThreadInitOkay = TRUE;
   }
   else if( nResult == -1 )
   {
      bNetworkReady = TRUE;
      bThreadInitOkay = FALSE;
      // exiting ... bad stuff happens
   }
   else if( nResult == -2 )
   {
      // delaying... okay....
   }
}

//----------------------------------------------------------------------------

#if defined( USE_WSA_EVENTS )

void HandleEvent( PCLIENT pClient )
{
	WSANETWORKEVENTS networkEvents;
	if( !pClient )
	{
      lprintf( "How did a NULL client get here?!" );
		return;
	}
	//lprintf( "Client event on %p", pClient );
	if( WSAEnumNetworkEvents( pClient->Socket, pClient->event, &networkEvents ) == ERROR_SUCCESS )
	{
#ifdef LOG_NOTICES
		lprintf( "RESET CLIENT EVENT" );
#endif
		ResetEvent( pClient->event );
		{
			if( pClient->dwFlags & CF_UDP )
			{
				if( networkEvents.lNetworkEvents & FD_READ )
				{
#ifdef LOG_NOTICES
					lprintf( "Got UDP FD_READ" );
#endif
					FinishUDPRead( pClient );
				}
			}
			else
			{
				THREAD_ID prior = 0;
				//lprintf( WIDE("network notification message...") );
				//Log( WIDE("Enter network section...") );
			start_lock:
				EnterCriticalSec( &g.csNetwork );
				//Log( WIDE("In network section...") );
				//retry:
#ifdef LOG_CLIENT_LISTS
				lprintf( "client lists Ac:%p(%p(%d)) Av:%p(%p(%d)) Cl:%p(%p(%d))"
						 , &g.ActiveClients, g.ActiveClients, g.ActiveClients?g.ActiveClients->Socket:0
						 , &g.AvailableClients, g.AvailableClients, g.AvailableClients?g.AvailableClients->Socket:0
						 , &g.ClosedClients, g.ClosedClients, g.ClosedClients?g.ClosedClients->Socket:0
						 );
#endif
				{
#ifdef LOG_NOTICES
					lprintf( "found client in active client list... check closed." );
#endif

					if( pClient->dwFlags & CF_WANTS_GLOBAL_LOCK )
					{
						LeaveCriticalSec( &g.csNetwork );
						goto start_lock;
					}

					if( EnterCriticalSecNoWait( &pClient->csLock, &prior ) < 1 )
					{
						// unlock the global section for a moment..
						// client may be requiring both local and global locks (already has local lock)
						LeaveCriticalSec( &g.csNetwork );
						Relinquish();
						goto start_lock;
					}
					//EnterCriticalSec( &pClient->csLock );
					if( !(pClient->dwFlags & CF_ACTIVE ) )
					{
						// change to inactive status by the time we got here...
						LeaveCriticalSec( &pClient->csLock );
						LeaveCriticalSec( &g.csNetwork );
						return;
					}
				}
				LeaveCriticalSec( &g.csNetwork );
				// if an unknown socket issued a
				// notification - close it - unknown handling of unknown socket.
				if( networkEvents.lNetworkEvents & FD_CONNECT )
				{
					{
						_16 wError = networkEvents.iErrorCode[FD_CONNECT_BIT];
#ifdef LOG_NOTICES
						lprintf( WIDE("FD_CONNECT") );
#endif
						if( !wError )
							pClient->dwFlags |= CF_CONNECTED;
						else
						{
							lprintf( WIDE("Connect error: %d"), wError );
							pClient->dwFlags |= CF_CONNECTERROR;
						}
						if( !( pClient->dwFlags & CF_CONNECTERROR ) )
						{
							// since this is done before connecting is clear, tcpwrite
							// may make notice of previously queued data to
							// connection opening...
							//Log( WIDE("Sending any previously queued data.") );

							// with events, we get a FD_WRITE also... which calls tcpwrite.
							//TCPWrite( pClient );
						}
						pClient->dwFlags &= ~CF_CONNECTING;
						if( pClient->connect.ThisConnected )
							if( pClient->dwFlags & CF_CPPCONNECT )
								pClient->connect.CPPThisConnected( pClient->psvConnect, wError );
							else
								pClient->connect.ThisConnected( pClient, wError );
						//lprintf( WIDE("Returned from application connected callback") );
						// check to see if the read was queued before the connect
						// actually completed...
						if( (pClient->dwFlags & ( CF_ACTIVE | CF_CONNECTED )) ==
							( CF_ACTIVE | CF_CONNECTED ) )
						{
							if( pClient->read.ReadComplete )
								if( pClient->dwFlags & CF_CPPREAD )
									pClient->read.CPPReadComplete( pClient->psvRead, NULL, 0 );
								else
									pClient->read.ReadComplete( pClient, NULL, 0 );
						}
						if( pClient->pWaiting )
							WakeThread( pClient->pWaiting );
						//lprintf( WIDE("Returned from application inital read complete.") );
					}
				}
				if( networkEvents.lNetworkEvents & FD_READ )
				{
#ifdef LOG_NOTICES
					lprintf( WIDE("FD_READ") );
#endif
					if( pClient->bDraining )
					{
						TCPDrainRead( pClient );
					}
					else
					{
						FinishPendingRead( pClient DBG_SRC );
						if( pClient->dwFlags & CF_TOCLOSE )
						{
							lprintf( WIDE( "Pending read failed - and wants to close." ) );
							//InternalRemoveClientEx( pc, TRUE, FALSE );
						}
					}
				}
				if( networkEvents.lNetworkEvents & FD_WRITE )
				{
#ifdef LOG_NOTICES
					lprintf( WIDE("FD_Write") );
#endif
					TCPWrite(pClient);
				}

				if( networkEvents.lNetworkEvents & FD_CLOSE )
				{
					if( !pClient->bDraining )
					{
						// act of reading can result in a close...
						// there are things like IE which close and send
						// adn we might get the close notice at application level indicating there might still be data...
						while( FinishPendingRead( pClient DBG_SRC) > 0 ); // try and read...
						//if( pClient->dwFlags & CF_TOCLOSE )
						{
							//lprintf( "Pending read failed - reset connection. (well this is FD_CLOSE so yeah...???)" );
							//InternalRemoveClientEx( pc, TRUE, FALSE );
						}
					}
#ifdef LOG_NOTICES
					lprintf("FD_CLOSE... %d", pClient->Socket );
#endif
					if( pClient->dwFlags & CF_ACTIVE )
					{
						// might already be cleared and gone..
						InternalRemoveClient( pClient );
						TerminateClosedClient( pClient );
					}
					// section will be blank after termination...(correction, we keep the section state now)
					pClient->dwFlags &= ~CF_CLOSING; // it's no longer closing.  (was set during the course of closure)
				}
				if( networkEvents.lNetworkEvents & FD_ACCEPT )
				{
#ifdef LOG_NOTICES
					lprintf( WIDE("FD_ACCEPT") );
#endif
					AcceptClient(pClient);
				}
				//lprintf( WIDE("leaveing event handler...") );
				//lprintf( WIDE("Left event handler CS.") );
			}
			LeaveCriticalSec( &pClient->csLock );
		}
	}
	else
	{
		DWORD dwError = WSAGetLastError();
		if( dwError == 10038 )
		{
         // no longer a socket, probably in a closed or closing state.
		}
      else
			lprintf( WIDE( "Event enum failed... do what? close socket? %d %d" ), pClient->Socket, dwError );
	}
}
#endif  //defined(USE_WSA_EVENTS)


#if !defined( USE_WSA_EVENTS )
int CALLBACK StandardNetworkProcess( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam )
{
   switch(uMsg)
   {
   case WM_CREATE:
   	g.uPendingTimer = AddTimer( 5000, PendingTimer, 0 );
      g.uNetworkPauseTimer = AddTimerEx( 1, 1000, NetworkPauseTimer, 0 );
#ifndef UNDER_CE
	case WM_NCCREATE:
		return TRUE;
#endif
	case WM_DESTROY:
		if( g.uNetworkPauseTimer )
		{
			RemoveTimer( g.uNetworkPauseTimer );
			g.uNetworkPauseTimer = 0;
		}
		if( g.uPendingTimer )
		{
			RemoveTimer( g.uPendingTimer );
			g.uPendingTimer = 0;
		}
		g.ghWndNetwork = NULL;
		break;
   case SOCKMSG_CLOSE:
    // Also close UDP sockets...
    // and TCP Listen sockets
      {
         while( g.ActiveClients )
         	InternalRemoveClient( g.ActiveClients );
      }
      WSACleanup();
      Log( _WIDE(__FILE__) WIDE("(") STRSYM(__LINE__) WIDE("): Post WM_QUIT Message to thread.") );
		g.pThread = NULL;
		DestroyWindow( g.ghWndNetwork );
      break;
   case SOCKMSG_TCP:
      {
			THREAD_ID prior = 0;
			PCLIENT pClient = NULL;
         //lprintf( WIDE("network notification message...") );
         if( wParam == -1 )
         {
         	Log( WIDE("message on a closed thing?!") );
         	return TRUE;
         }
			//Log( WIDE("Enter network section...") );
		start_lock:
			EnterCriticalSec( &g.csNetwork );
         //Log( WIDE("In network section...") );
		//retry:
#ifdef LOG_CLIENT_LISTS
			lprintf( "client lists Ac:%p(%p(%d)) Av:%p(%p(%d)) Cl:%p(%p(%d))"
					 , &g.ActiveClients, g.ActiveClients, g.ActiveClients?g.ActiveClients->Socket:0
					 , &g.AvailableClients, g.AvailableClients, g.AvailableClients?g.AvailableClients->Socket:0
					 , &g.ClosedClients, g.ClosedClients, g.ClosedClients?g.ClosedClients->Socket:0
				 );
#endif
			for( pClient = g.ActiveClients; pClient; pClient = pClient->next )
			{
#ifdef LOG_CLIENT_LISTS
				lprintf( "is %p(%d) = %d ?", pClient, pClient->Socket, wParam );
#endif
            if( pClient->Socket == (SOCKET)wParam )
					break;
			}
         if( pClient )
			{
#ifdef LOG_NOTICES
				lprintf( "found client in active client list... check closed." );
#endif

				if( pClient->dwFlags & CF_WANTS_GLOBAL_LOCK )
				{
					LeaveCriticalSec( &g.csNetwork );
					goto start_lock;
				}

				if( EnterCriticalSecNoWait( &pClient->csLock, &prior ) < 1 )
				{
					// unlock the global section for a moment..
               // client may be requiring both local and global locks (already has local lock)
					LeaveCriticalSec( &g.csNetwork );
               Relinquish();
               goto start_lock;
				}
         	//EnterCriticalSec( &pClient->csLock );
				if( !(pClient->dwFlags & CF_ACTIVE ) )
				{
					// change to inactive status by the time we got here...
					LeaveCriticalSec( &pClient->csLock );
               LeaveCriticalSec( &g.csNetwork );
               return TRUE;
				}
         }
			else
			{
				// junk any messages to a closed client... (except FD_CLOSE)
#ifdef LOG_NOTICES
				lprintf( "Didn't find client in active client list... check closed." );
#endif
	         for( pClient = g.ClosedClients; pClient; pClient = pClient->next )
   	         if( pClient->Socket == (SOCKET)wParam )
					{
#ifdef LOG_NOTICES
						lprintf( "Found client in closed list..." );
#endif
       	         if( LOWORD(lParam) != FD_CLOSE )
       	         {
							LeaveCriticalSec( &g.csNetwork );
#ifdef LOG_NOTICES
							lprintf( "Left section, returning, ignoring message." );
#endif
							return TRUE;
						}
						if( EnterCriticalSecNoWait( &pClient->csLock, &prior ) < 1 )
						{
							// unlock the global section for a moment..
							// client may be requiring both local and global locks (already has local lock)
							LeaveCriticalSec( &g.csNetwork );
							goto start_lock;
						}
#ifdef LOG_NOTICES
						lprintf( "FD_CLOSE was received... continue" );
#endif
                  break;
   	         }
			}
         LeaveCriticalSec( &g.csNetwork );
         // if an unknown socket issued a
         // notification - close it - unknown handling of unknown socket.
         if( !pClient &&
             LOWORD(lParam) != FD_ACCEPT)
         {
            // this sometimes happens since the recv() failes
            // on the server socket which causes a close before
            // this notification occurs.
            // well related to this are persistant connections...
            // they get the close notice, and reopen the socket...
            // same handle, and therefore it IS known about and the FD_CLOSE
            // still closes that socket.
				lprintf( WIDE("Closing a socket which we knew nothing about. %d (%s)")
						 , wParam
						 , LOWORD(lParam)==FD_CLOSE?"FD_CLOSE"
						  : LOWORD(lParam)==FD_READ?"FD_READ"
						  : LOWORD(lParam)==FD_WRITE?"FD_WRITE"
						  : LOWORD(lParam)==FD_CONNECT?"FD_CONNECT"
						  : "Unknown Event"
						 );
            closesocket((SOCKET)wParam);
            break;
         }
         switch(LOWORD(lParam))
         {
         case FD_READ:
#ifdef LOG_NOTICES
         	lprintf( WIDE("FD_READ") );
#endif
            if( pClient->bDraining )
            {
               TCPDrainRead( pClient );
            }
            else
            {
               FinishPendingRead( pClient DBG_SRC );
					if( pClient->dwFlags & CF_TOCLOSE )
					{
						lprintf( WIDE( "Pending read failed - and wants to close." ) );
						//InternalRemoveClientEx( pc, TRUE, FALSE );
					}
				}
				LeaveCriticalSec( &pClient->csLock );
            return(TRUE);
         case FD_WRITE:
#ifdef LOG_NOTICES
         	lprintf( WIDE("FD_Write") );
#endif
            TCPWrite(pClient);
            break;
         case FD_CLOSE:
				if( !pClient->bDraining )
				{
					// act of reading can result in a close...
					// there are things like IE which close and send
               // adn we might get the close notice at application level indicating there might still be data...
					while( FinishPendingRead( pClient DBG_SRC) > 0 ); // try and read...
					//if( pClient->dwFlags & CF_TOCLOSE )
					{
						//lprintf( "Pending read failed - reset connection. (well this is FD_CLOSE so yeah...???)" );
						//InternalRemoveClientEx( pc, TRUE, FALSE );
					}
				}
#ifdef LOG_NOTICES
            lprintf("FD_CLOSE... %d", pClient->Socket );
#endif
				if( pClient->dwFlags & CF_ACTIVE )
				{
               // might already be cleared and gone..
					InternalRemoveClient( pClient );
					TerminateClosedClient( pClient );
				}
				// section will be blank after termination...(correction, we keep the section state now)
            pClient->dwFlags &= ~CF_CLOSING; // it's no longer closing.  (was set during the course of closure)
            LeaveCriticalSec( &pClient->csLock );
            return TRUE;
            break;
         case FD_ACCEPT:
#  ifdef LOG_NOTICES
         	lprintf( WIDE("FD_ACCEPT") );
#  endif
            AcceptClient(pClient);
            break;
         case FD_CONNECT:
            {
               _16 wError = HIWORD(lParam);
#  ifdef LOG_NOTICES
	         	lprintf( WIDE("FD_CONNECT") );
#  endif
               if( !wError )
                  pClient->dwFlags |= CF_CONNECTED;
               else
               {
               	lprintf( WIDE("Connect error: %d"), wError );
               	pClient->dwFlags |= CF_CONNECTERROR;
					}
					if( !( pClient->dwFlags & CF_CONNECTERROR ) )
					{
						// since this is done before connecting is clear, tcpwrite
						// may make notice of previously queued data to
                  // connection opening...
						//Log( WIDE("Sending any previously queued data.") );
						TCPWrite( pClient );
					}
               pClient->dwFlags &= ~CF_CONNECTING;
               if( pClient->connect.ThisConnected )
               	if( pClient->dwFlags & CF_CPPCONNECT )
	                  pClient->connect.CPPThisConnected( pClient->psvConnect, wError );
               	else
	                  pClient->connect.ThisConnected( pClient, wError );
               //lprintf( WIDE("Returned from application connected callback") );
               // check to see if the read was queued before the connect
               // actually completed...
               if( (pClient->dwFlags & ( CF_ACTIVE | CF_CONNECTED )) ==
               		( CF_ACTIVE | CF_CONNECTED ) )
               {
	               if( pClient->read.ReadComplete )
	               	if( pClient->dwFlags & CF_CPPREAD )
								pClient->read.CPPReadComplete( pClient->psvRead, NULL, 0 );
	               	else
								pClient->read.ReadComplete( pClient, NULL, 0 );
					}
               //lprintf( WIDE("Returned from application inital read complete.") );
	            break;
            }
			}
         //lprintf( WIDE("leaveing event handler...") );
			LeaveCriticalSec( &pClient->csLock );
         //lprintf( WIDE("Left event handler CS.") );
      }
      return TRUE;
   case SOCKMSG_UDP:
		switch(LOWORD(lParam))
		{
		case FD_READ:
			{
				PCLIENT pc;
#ifdef LOG_NOTICES
				lprintf( "Got UDP FD_READ" );
#endif
				for( pc = g.ActiveClients; pc; pc = pc->next )
					if( pc->Socket == (SOCKET)wParam )
						break;
				if( !pc )
				{
					lprintf( WIDE("Failed to find UDP Socket... ") );
					return 0;
				}
				FinishUDPRead( pc );
            }
			return(TRUE);
#  ifdef LOG_NOTICES
		default:
			lprintf( "Received unknown/unhnadled event %d", LOWORD(lParam ) );
			break;
#  endif
		}
		break;
#  ifndef _ARM_
   case WM_QUERYENDSESSION:
   	    return TRUE; // uhmm okay.
#  endif
   }
   return FALSE;
}
#endif // if !defined( USE_EVENTS )

#endif // if !defined( __LINUX__ )

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetNetworkWriteComplete)( PCLIENT pClient,
                                         cWriteComplete WriteComplete )
{
   if( pClient && IsValid( pClient->Socket ) )
   {
   	pClient->write.WriteComplete = WriteComplete;
   }
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetCPPNetworkWriteComplete)( PCLIENT pClient
                                         , cppWriteComplete WriteComplete 
                                         , PTRSZVAL psv)
{                                        
   if( pClient && IsValid( pClient->Socket ) )
   {
   	pClient->write.CPPWriteComplete = WriteComplete;
   	pClient->psvWrite = psv;
   	pClient->dwFlags |= CF_CPPWRITE;
   }
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetNetworkCloseCallback)( PCLIENT pClient,
                                         cCloseCallback CloseCallback )
{
   if( pClient && IsValid(pClient->Socket) )
   {
      pClient->close.CloseCallback = CloseCallback;
   }
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetCPPNetworkCloseCallback)( PCLIENT pClient
                                         , cppCloseCallback CloseCallback 
                                         , PTRSZVAL psv)
{
   if( pClient && IsValid(pClient->Socket) )
   {
      pClient->close.CPPCloseCallback = CloseCallback;
      pClient->psvClose = psv;
      pClient->dwFlags |= CF_CPPCLOSE;
   }
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetNetworkReadComplete)( PCLIENT pClient,
                                        cReadComplete pReadComplete )
{
   if( pClient && IsValid(pClient->Socket) )
   {
      pClient->read.ReadComplete = pReadComplete;
   }
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetCPPNetworkReadComplete)( PCLIENT pClient
                                        , cppReadComplete pReadComplete 
                                        , PTRSZVAL psv)
{
   if( pClient && IsValid(pClient->Socket) )
   {
      pClient->read.CPPReadComplete = pReadComplete;
      pClient->psvRead = psv;
      pClient->dwFlags |= CF_CPPREAD;
   }
}

//----------------------------------------------------------------------------


#ifndef __LINUX__
//----------------------------------------------------------------------------

void SetNetworkCallbackWindow( HWND hWnd )
{
}

// this is passed '0' when it is called internally
// this is passed '1' when it is called by idleproc
int CPROC ProcessNetworkMessages( PTRSZVAL quick_check )
{
	//lprintf( WIDE("Check messages.") );
	if( g.bQuit )
      return -1;
	if( IsThisThread( g.pThread ) )
	{
#  if defined( USE_WSA_EVENTS )
      int Count = 0;
		// this is the thread that should be waiting here.
		static PLIST events = NULL;
      static PLIST clients = NULL; // clients are added parallel, so events are in order too.
		PCLIENT pc;
      // disallow changing of the lists.
		EnterCriticalSec( &g.csNetwork );
		EmptyList( &events );
      EmptyList( &clients );
		AddLink( &events, g.event );
		AddLink( &clients, (POINTER)1 ); // cannot add a NULL.
      Count++;
		for( pc = g.ActiveClients; pc; pc = pc->next )
		{
         // socket might not be quite opened yet... or maybe the event isn't created just yet
			if( !IsValid( pc->Socket ) || !(pc->event) || !(pc->Socket) )
				continue;
         // make sure we drop inactive clients immediately...
			if( !( pc->dwFlags & CF_ACTIVE ) )
            continue;
			if( pc->dwFlags & (CF_LISTEN|CF_READPENDING) )
			{
				AddLink( &events, pc->event );
            AddLink( &clients, pc );
				Count++;
#    ifdef LOG_NOTICES
				lprintf( "Added for read select %p(%d) %d", pc, pc->Socket, pc->event );
#    endif
            continue;
			}

			if( pc->dwFlags & ( CF_WRITEPENDING | CF_CONNECTING ) )
			{
				pc->dwFlags |= CF_WRITEISPENDED;
#    ifdef LOG_NOTICES
				Log( WIDE("Pending write... Setting socket into write list... %p(%d) %d"), pc, pc->Socket, pc->event );
#    endif
            AddLink( &events, pc->event );
				AddLink( &clients, pc );
				Count++;
				continue;
			}

			AddLink( &events, pc->event );
			AddLink( &clients, pc );
			Count++;
#    ifdef LOG_NOTICES
			lprintf( "Added for except event %p(%d) %d", pc, pc->Socket, pc->event );
#    endif

		}
		// okay, we have a picture of what was active
      // they could still go away in the middle.
		LeaveCriticalSec( &g.csNetwork );
      while( 1 )
		{
			S_32 result;
			// want to wait here anyhow...
#    ifdef LOG_NOTICES
			lprintf( "Waiting on %d", Count );
#    endif
			result = WSAWaitForMultipleEvents( Count
														, events->pNode
														, FALSE
														, (quick_check)?0:WSA_INFINITE
														, FALSE
														);
#    ifdef LOG_NOTICES
			lprintf( WIDE("Event Wait Result was %d"), result );
#    endif
			if( result == WSA_WAIT_FAILED )
			{
				DWORD dwError = WSAGetLastError();
				if( dwError == WSA_INVALID_HANDLE )
				{
               lprintf( "Rebuild list, have a bad event handle somehow." );
					break;
				}
				lprintf( WIDE( "error of wait is %d" ), WSAGetLastError() );
            break;
			}
#ifndef UNDER_CE
			else if( result == WSA_WAIT_IO_COMPLETION )
			{
            // reselect... not sure where io completion fits for network...
				continue;
			}
#endif
			else if( result == WSA_WAIT_TIMEOUT )
			{
				if( quick_check )
               return 1;
			}
			else if( result >= WSA_WAIT_EVENT_0 )
			{
            // if result is _0, then it's the global event, and we just return.
				if( result > WSA_WAIT_EVENT_0 )
				{
               //lprintf( "index is %d", result-(WSA_WAIT_EVENT_0) );
					HandleEvent( (PCLIENT)GetLink( &clients, result-(WSA_WAIT_EVENT_0) ) );
				}
				else
				{
#ifdef LOG_NOTICES
					lprintf( "RESET GLOBAL EVENT" );
#endif
					ResetEvent( g.event );
				}
				//DeleteList( &clients );
				//DeleteList( &events );
				return 1;
			}
		}
		// result 0... we had nothing to do
      // but we are this thread.
		//DeleteList( &clients );
		//DeleteList( &events );
#  else
		MSG Msg;
		if( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
		{
         if( !Msg.hwnd ||  // try something NEW here - an AND instead of OR
             Msg.message == SOCKMSG_CLOSE )
			{
            //lprintf( WIDE("Got a message... dispatching to network.") );
            StandardNetworkProcess( g.ghWndNetwork, Msg.message,
                                    Msg.wParam, Msg.lParam );
			}
			else
			{
				DispatchMessage( &Msg );
			}
			return 1;
		}
#  endif
		return 0;
	}
	// return -1, in case we are an idle proc, this will
	// de-elect this proc as an idle candiate for this thread
	return -1;
}

//----------------------------------------------------------------------------


static PTRSZVAL CPROC NetworkThreadProc( PTHREAD thread )
{
#  ifndef USE_WSA_EVENTS
   static WNDCLASS wc;  // zero init.
//   _16 wIdx;
//   LPSTR lpText[2];
   //nConnections = 0;   // no connections present.
   MemSet( &wc, 0, sizeof( WNDCLASS ) );
   wc.lpfnWndProc   = (WNDPROC)StandardNetworkProcess;
   wc.hInstance     = GetModuleHandle( NULL ) ;
   wc.lpszClassName = WIDE("NetworkMessageHandler");
   if( !RegisterClass(&wc) )
   {
   	TEXTCHAR byBuf[256];
      if( GetLastError() != ERROR_CLASS_ALREADY_EXISTS )
      {
         lprintf( WIDE("RegisterClassError: %08lx %d"), GetModuleHandle( NULL ), GetLastError() );
   	   MessageBox( NULL, byBuf, WIDE("BAD"), MB_OK );
         bThreadInitComplete = TRUE;
         bThreadInitOkay = FALSE;
         return FALSE;   // stop thread
      }
   }

   g.ghWndNetwork = CreateWindow(wc.lpszClassName,
								WIDE("StandardNetworkHandler"),
											0,0,0,0,0,NULL,NULL,NULL,NULL);
	if (!g.ghWndNetwork)
	{
		TEXTCHAR byBuf[256];
		snprintf( byBuf, sizeof( byBuf ), WIDE("CreateWindowError: %d"), GetLastError() );
		MessageBox( NULL, byBuf, WIDE("BAD"), MB_OK );
		bThreadInitComplete = TRUE;
		bThreadInitOkay = FALSE;
		UnregisterClass( WIDE("NetworkMessageHandler"), GetModuleHandle( NULL ) );
		return(FALSE);
	}
#  else
   // and when unloading should remove these timers.
	g.uPendingTimer = AddTimer( 5000, PendingTimer, 0 );
	g.uNetworkPauseTimer = AddTimerEx( 1, 1000, NetworkPauseTimer, 0 );
   g.event = CreateEvent( NULL, 0, FALSE, NULL );
#  endif
	while( !g.pThread ) // creator won't pass until bThreadInitComplete is set.
		Relinquish();

	bThreadInitComplete = TRUE;
	while(g.pThread && !g.bQuit)
	{

		if( !ProcessNetworkMessages(0) )
		{
#ifndef UNDER_CE
			WaitMessage();
#endif
			//Sleep( 10 );
		}
	}

	EnterCriticalSec( &g.csNetwork );

#  ifndef USE_WSA_EVENTS
	DestroyWindow( g.ghWndNetwork );
	g.ghWndNetwork = NULL; // clear value and everything....
	UnregisterClass( WIDE("NetworkMessageHandler"), GetModuleHandle( NULL ) );
#  endif
	bThreadExit = TRUE;



   lprintf( WIDE("Shutting down network thread.") );
/*
	// close all open sockets?
   {
   	int i;
   	for(i = 0; i <
   }
*/
	Release( g.pUserData ); // should be first one pointed to...
	{
		INDEX idx;
      PCLIENT_SLAB slab;
		LIST_FORALL( g.ClientSlabs, idx, PCLIENT_SLAB, slab )
		{
         Release( slab );
		}
      DeleteList( &g.ClientSlabs );
	}
	g.AvailableClients = NULL;
	g.ActiveClients = NULL;
	g.ClosedClients = NULL;
	g.pUserData = NULL;
   bThreadInitComplete = FALSE;
	g.pThread = NULL;
	bNetworkReady = FALSE;
	LeaveCriticalSec( &g.csNetwork );
	DeleteCriticalSec( &g.csNetwork );    //spv:980303
   return 0;
}
#else // if !__LINUX__

int CPROC ProcessNetworkMessages( PTRSZVAL unused )
{
	fd_set read, write, except;
	int cnt, maxcnt;
	PCLIENT pc;
	struct timeval time;
	if( g.bQuit )
		return -1;
	if( IsThisThread( g.pThread ) )
	{
		FD_ZERO( &read );
		FD_ZERO( &write );
		FD_ZERO( &except );
		maxcnt = 0;
		EnterCriticalSec( &g.csNetwork );
		for( pc = g.ActiveClients; pc; pc = pc->next )
		{
         // socket might not be quite opened yet...
			if( !IsValid( pc->Socket ) )
				continue;
			if( pc->dwFlags & (CF_LISTEN|CF_READPENDING) )
			{
				if( pc->Socket >= maxcnt )
					maxcnt = pc->Socket+1;
				FD_SET( pc->Socket, &read );
#ifdef LOG_NOTICES
				lprintf( "Added for read select %p(%d)", pc, pc->Socket );
#endif
			}

			if( pc->dwFlags & CF_LISTEN ) // only check read...
				continue;

			if( pc->dwFlags & ( CF_WRITEPENDING | CF_CONNECTING ) )
			{
				if( pc->Socket >= maxcnt )
					maxcnt = pc->Socket+1;
				pc->dwFlags |= CF_WRITEISPENDED;
#ifdef LOG_NOTICES
				Log( WIDE("Pending write... Setting socket into write list...") );
#endif
				FD_SET( pc->Socket, &write );
			}

			if( pc->Socket >= maxcnt )
				maxcnt = pc->Socket+1;
			FD_SET( pc->Socket, &except );
#ifdef LOG_NOTICES
			lprintf( "Added for except event %p(%d)", pc, pc->Socket );
#endif

		}
		LeaveCriticalSec( &g.csNetwork );
		if( !maxcnt )
			return 0;
		time.tv_sec = 5;
		time.tv_usec = 0; // need timer to be quick to watch new sockets
		// should be able to make a signal handle
#ifdef LOG_NOTICES
		Log( WIDE("Entering select....") );
#endif
		cnt = select( maxcnt, &read, &write, &except, &time );
		if( cnt < 0 )
		{
			if( errno == EINTR )
				return 1;
			if( errno == EBADF )
			{
            PCLIENT next;
				EnterCriticalSec( &g.csNetwork );
				for( pc = g.ActiveClients; pc; pc = next )
				{
               next = pc->next;
					if( IsValid( pc->Socket ) )
					{
						if( pc->Socket >= maxcnt )
							maxcnt = pc->Socket+1;
						FD_ZERO( &except );
						FD_SET( pc->Socket, &except );
						time.tv_sec = 0;
						time.tv_usec = 0;
						cnt = select( maxcnt, NULL, NULL, &except, &time );
						if( cnt < 0 )
						{
							if( errno == EBADF )
							{
								Log1( WIDE("Bad descriptor is : %d"), pc->Socket );
								InternalRemoveClient( pc );
								LeaveCriticalSec( &g.csNetwork );
								return 1;
							}
						}
					}
				}
				LeaveCriticalSec( &g.csNetwork );
			}
			Log1( WIDE("Sorry Select call failed... %d"), errno );
		}
		restart_scan:
		if( cnt > 0 )
		{
			THREAD_ID prior = 0;
         PCLIENT next;
		start_lock:
			EnterCriticalSec( &g.csNetwork );
			for( pc = g.ActiveClients; pc; pc = next )
			{
				next = pc->next;
				if( !IsValid( pc->Socket ) )
               continue;
				if( pc->dwFlags & CF_WANTS_GLOBAL_LOCK)
				{
					LeaveCriticalSec( &g.csNetwork );
					goto start_lock;
				}

				if( FD_ISSET( pc->Socket, &read ) )
				{
					if( EnterCriticalSecNoWait( &pc->csLock, &prior ) < 1 )
					{
						// unlock the global section for a moment..
						// client may be requiring both local and global locks (already has local lock)
						LeaveCriticalSec( &g.csNetwork );
						goto start_lock;
					}
					//EnterCriticalSec( &pc->csLock );
					LeaveCriticalSec( &g.csNetwork );
					if( !(pc->dwFlags & CF_ACTIVE ) )
					{
						// change to inactive status by the time we got here...
						LeaveCriticalSec( &pc->csLock );
						goto start_lock;
					}

					if( pc->dwFlags & CF_LISTEN )
					{
#ifdef LOG_NOTICES
						Log( WIDE("accepting...") );
#endif
						AcceptClient( pc );
					}
					else if( pc->dwFlags & CF_UDP )
					{
#ifdef LOG_NOTICES
						Log( WIDE("UDP Read Event..."));
#endif
						FinishUDPRead( pc );
					}
					else if( pc->bDraining )
					{
#ifdef LOG_NOTICES
						Log( WIDE("TCP Drain Event..."));
#endif
						TCPDrainRead( pc );
					}
					else if( pc->dwFlags & CF_READPENDING )
					{
#ifdef LOG_NOTICES
						Log( WIDE("TCP Read Event..."));
#endif
						// packet oriented things may probably be reading only
						// partial messages at a time...
						FinishPendingRead( pc DBG_SRC );
						if( pc->dwFlags & CF_TOCLOSE )
						{
							lprintf( "Pending read failed - reset connection." );
							InternalRemoveClientEx( pc, FALSE, FALSE );
							TerminateClosedClient( pc );
						}
						else if( !pc->RecvPending.s.bStream )
							pc->dwFlags |= CF_READREADY;
					}
					else
					{
#ifdef LOG_NOTICES
						Log( WIDE("TCP Set read ready...") );
#endif
						pc->dwFlags |= CF_READREADY;
					}
					cnt--;
					FD_CLR( pc->Socket, &read );
					LeaveCriticalSec( &pc->csLock );
					goto restart_scan;
				}
				if( FD_ISSET( pc->Socket, &write ) )
				{
					if( EnterCriticalSecNoWait( &pc->csLock, &prior ) < 1 )
					{
						// unlock the global section for a moment..
						// client may be requiring both local and global locks (already has local lock)
						LeaveCriticalSec( &g.csNetwork );
						goto start_lock;
					}
					//EnterCriticalSec( &pc->csLock );
					LeaveCriticalSec( &g.csNetwork );
					if( !(pc->dwFlags & CF_ACTIVE ) )
					{
						// change to inactive status by the time we got here...
						LeaveCriticalSec( &pc->csLock );
						goto start_lock;
					}

					if( pc->dwFlags & CF_CONNECTING )
					{
#ifdef LOG_NOTICES
						Log( WIDE("Connected!") );
#endif
						pc->dwFlags |= CF_CONNECTED;
						pc->dwFlags &= ~CF_CONNECTING;
						{
							int error;
							socklen_t errlen = sizeof(error);
							getsockopt( pc->Socket, SOL_SOCKET
										 , SO_ERROR
										 , &error, &errlen );
							lprintf( WIDE("Error checking for connect is: %s"), strerror(error) );
							if( pc->pWaiting )
							{
#ifdef LOG_NOTICES
							    lprintf( "Got connect event, waking waiter.." );
#endif
							    WakeThread( pc->pWaiting );
							}
							if( pc->connect.ThisConnected )
								pc->connect.ThisConnected( pc, error );
							// if connected okay - issue first read...
							if( !error )
							{
								if( pc->read.ReadComplete )
								{
									pc->read.ReadComplete( pc, NULL, 0 );
								}
								if( pc->lpFirstPending )
								{
									Log( WIDE("Data was pending on a connecting socket, try sending it now") );
									TCPWrite( pc );
								}
							}
							else
							{
								pc->dwFlags |= CF_CONNECTERROR;
								FD_CLR( pc->Socket, &read );
								LeaveCriticalSec( &pc->csLock );
								goto restart_scan;
							}
						}
					}
					else if( pc->dwFlags & CF_UDP )
					{
						// udp write event complete....
						// do we ever care? probably sometime...
					}
					else
					{
#ifdef LOG_NOTICES
						Log( WIDE("TCP Write Event...") );
#endif
						pc->dwFlags &= ~CF_WRITEISPENDED;
						TCPWrite( pc );
					}
					cnt--;
					FD_CLR( pc->Socket, &write );
					LeaveCriticalSec( &pc->csLock );
					goto restart_scan;
				}

				if( FD_ISSET( pc->Socket, &except ) )
				{
					if( EnterCriticalSecNoWait( &pc->csLock, &prior ) < 1 )
					{
						// unlock the global section for a moment..
						// client may be requiring both local and global locks (already has local lock)
						LeaveCriticalSec( &g.csNetwork );
						goto start_lock;
					}
					//EnterCriticalSec( &pc->csLock );
					LeaveCriticalSec( &g.csNetwork );
#ifdef LOG_NOTICES
					Log( WIDE("Except Event...") );
#endif
					if( !(pc->dwFlags & CF_ACTIVE ) )
					{
						// change to inactive status by the time we got here...
						LeaveCriticalSec( &pc->csLock );
						goto restart_scan;
					}
					if( !pc->bDraining )
						while( FinishPendingRead( pc DBG_SRC ) > 0 ); // try and read...
#ifdef LOG_NOTICES
					Log(" FD_CLOSE...\n");
#endif
					InternalRemoveClient( pc );
               TerminateClosedClient( pc );
					cnt--;
					FD_CLR( pc->Socket, &except );
					LeaveCriticalSec( &pc->csLock );
					goto restart_scan;
				}
			}
			// had some event  - return 1 to continue working...
			LeaveCriticalSec( &g.csNetwork );
		}
		return 1;
	}
	//Log( WIDE("Attempting to wake thread (send sighup!)") );
	//WakeThread( g.pThread );
	//Log( WIDE("Only reason should get here is if it's not this thread...") );
	return -1;
}


static PTRSZVAL CPROC NetworkThreadProc( PTHREAD thread )
{
// idle loop to handle select() call....
// this thread has many other protections against starting
// a second time... and this may result from the parent thread
// before even starting the first, invalidating this wait
// for global thread.
	//if( g.pThread )
	//{
   //   lprintf( WIDE("Thread has already been started.") );
	//	return 0;
	//}
   //Relinquish();
   g.uPendingTimer = AddTimer( 5000, PendingTimer, 0 );
   signal( SIGPIPE, SIG_IGN );
	while( !g.pThread )
		Relinquish(); // wait for pThread to be done
   bThreadInitComplete = TRUE;
   bNetworkReady = TRUE;
   //Log( WIDE("Network Thread Began...") );

   while( !g.bQuit )
	{
      int n = ProcessNetworkMessages(0 );
      if( n < 0 )
			break;
		else if( !n )
		{
			// should sleep when there's no sockets to listen to...
			WakeableSleep( SLEEP_FOREVER ); // groovy we can Sleep!!!
		}
	}
	{
		INDEX idx;
      PCLIENT_SLAB slab;
		LIST_FORALL( g.ClientSlabs, idx, PCLIENT_SLAB, slab )
		{
         Release( slab );
		}
	}
	DeleteList( &g.ClientSlabs );
	Release( g.pUserData );
	g.pUserData = NULL;
   g.pThread = NULL; // confirm thread exit.
	bNetworkReady = FALSE;
   return 0;
}

#endif

//----------------------------------------------------------------------------
int NetworkQuit(void)
{
#ifdef __STATIC__
	if( !global_network_data )
		return 0;
#endif

	while( g.ActiveClients )
	{
      lprintf( WIDE("Remove active client %p"), g.ActiveClients );
		InternalRemoveClient( g.ActiveClients );
	}
	g.bQuit = TRUE;
#ifndef __LINUX__
   if( IsWindow( g.ghWndNetwork ) )
	{
		// okay forget this... at exit, cannot guarantee that
		// any other thread other than myself has any rights to do anything.
#  ifdef LOG_NOTICES
		lprintf( WIDE( "Post SOCKMSG_CLOSE" ) );
#  endif
#  ifdef USE_WSA_EVENTS
		SetEvent( g.event );
#  else
		PostMessage( g.ghWndNetwork, SOCKMSG_CLOSE, 0, 0 );
#  endif
      // also remove PCLIENT clients, and all client->pUserData allocated...
   }
#else
	//while( g.pThread )
	//	Sleep(0);
	// should kill Our thread.... and close any active ockets...
#endif
	bThreadInitComplete = FALSE;
	RemoveIdleProc( ProcessNetworkMessages );
	if( g.pThread )
	{
		_32 started = GetTickCount() + 500;
#ifdef USE_WSA_EVENTS
#ifdef LOG_NOTICES
		lprintf( "SET GLOBAL EVENT" );
#endif
		SetEvent( g.event );
#endif
		WakeThread( g.pThread );
		Relinquish(); // allow network thread to gracefully exit
		while( bNetworkReady && GetTickCount() < started )
			IdleFor( 20 );
		if( bNetworkReady )
		{
#ifdef LOG_STARTUP_SHUTDOWN
			lprintf( WIDE( "Network was locked up?  Failed to allow network to exit in half a second (500ms)" ) );
#endif
		}
	}

   return -1;
}

ATEXIT( NetworkShutdown )
{
	NetworkQuit();
}

//----------------------------------------------------------------------------

LOGICAL NetworkAlive( void )
{
	return !bThreadExit;
}

//----------------------------------------------------------------------------

void ReallocClients( _16 wClients, int nUserData )
{
	P_8 pUserData;
	PCLIENT_SLAB pClientSlab;
	if( !MAX_NETCLIENTS )
	{
		//Log( WIDE("Starting Network Init!") );
		InitializeCriticalSec( &g.csNetwork );
	}
	// else Log( WIDE("Restarting network init!") );

   // protect all structures.
   EnterCriticalSec( &g.csNetwork );
   if( !wClients )
		wClients = 16;  // default 16 clients...

   // keep the max of specified connections..
	if( wClients < MAX_NETCLIENTS )
      wClients = MAX_NETCLIENTS;
	if( wClients > MAX_NETCLIENTS )
	{
		_32 n;
		//Log1( WIDE("Creating %d Client Resources"), MAX_NETCLIENTS );
		pClientSlab = NULL;
		pClientSlab = (PCLIENT_SLAB)Allocate( n = my_offsetof(&pClientSlab,client[wClients - MAX_NETCLIENTS] ) );
		MemSet( pClientSlab, 0, n ); // can't clear the lpUserData Address!!!
		pClientSlab->count = wClients - MAX_NETCLIENTS;
		for( n = 0; n < pClientSlab->count; n++ )
		{
			pClientSlab->client[n].Socket = INVALID_SOCKET; // unused sockets on all clients.
         pClientSlab->client[n].saClient = AllocAddr();
			AddAvailable( pClientSlab->client + n );
		}
      AddLink( &g.ClientSlabs, pClientSlab );
	}

   // keep the max of specified data..
	if( nUserData < g.nUserData )
		nUserData = g.nUserData;
	if( nUserData > g.nUserData || wClients > MAX_NETCLIENTS )
	{
		INDEX idx;
		PCLIENT_SLAB slab;
		_32 n;
		int tot = 0;
		g.nUserData = nUserData;
		pUserData = (P_8)Allocate( g.nUserData * sizeof( PTRSZVAL ) * wClients );
		MemSet( pUserData, 0, g.nUserData * sizeof( PTRSZVAL ) * wClients );
		LIST_FORALL( g.ClientSlabs, idx, PCLIENT_SLAB, slab )
		{
			for( n = 0; n < slab->count; n++ )
			{
				if( slab->client[n].lpUserData )
					MemCpy( (char*)pUserData + (tot * (nUserData * sizeof( PTRSZVAL )))
							, slab->client[n].lpUserData
                     , g.nUserData * sizeof( PTRSZVAL ) );
				slab->client[n].lpUserData = (unsigned char*)pUserData + (tot * (nUserData * sizeof( PTRSZVAL )));
				tot++;
			}
		}
		if( g.pUserData )
			Release( g.pUserData );
		g.pUserData = pUserData;
	}
   MAX_NETCLIENTS = wClients;
	g.nUserData = nUserData;
   LeaveCriticalSec( &g.csNetwork );
}

#ifdef __LINUX__
NETWORK_PROC( LOGICAL, NetworkWait )(POINTER unused,_16 wClients,int wUserData)
#else
NETWORK_PROC( LOGICAL, NetworkWait )(HWND hWndNotify,_16 wClients,int wUserData)
#endif
{
	ReallocClients( wClients, wUserData );

	//-------------------------
	// please be mindful of the following data declared immediate...
	if( g.pThread )
   {
		Log( WIDE("Thread already active...") );
      // might do something... might not...
		return TRUE; // network thread active, do not realloc
   }
#ifndef __LINUX__
   if( IsWindow( g.ghWndNetwork ) ) // assume we already started...
   {
      Log1( WIDE("Window Exists! %08lx"), g.ghWndNetwork );
      return TRUE;
   }
   SetNetworkCallbackWindow( hWndNotify );
#endif
	g.pThread = ThreadTo( NetworkThreadProc, 0 );
   AddIdleProc( ProcessNetworkMessages, 1 );
	//Log( WIDE("Network Initialize..."));
	//Log( WIDE("Create network thread.") );
	while( !bThreadInitComplete )
		Relinquish();
	if( !bThreadInitOkay )
		return FALSE;
	while( !bNetworkReady )
		Relinquish(); // wait for actual network...
	{
		char buffer[256];
		if( gethostname( buffer, sizeof( buffer ) ) == 0)
			g.system_name = DupCStr( buffer );
	}
#ifdef __WINDOWS__
	{
		ADDRINFO *result;
		ADDRINFO *test;
		getaddrinfo( g.system_name, NULL, NULL, &result );
		for( test = result; test; test = test->ai_next )
		{
			//if( test->ai_family == AF_INET )
			{
            SOCKADDR *tmp;
				AddLink( &g.addresses, tmp = AllocAddr() );
            MemCpy( tmp, test->ai_addr, test->ai_addrlen );
			}
		}
	}
#endif

   //Log( WIDE("Network Initialize Complete!") );
   return bThreadInitOkay;  // return status of thread initialization
}

//----------------------------------------------------------------------------

PCLIENT GetFreeNetworkClientEx( DBG_VOIDPASS )
{
	PCLIENT pClient = NULL;
get_client:
	EnterCriticalSec( &g.csNetwork );
	for( pClient = g.AvailableClients; pClient; pClient = pClient->next )
		if( !( pClient->dwFlags & CF_CLOSING ) )
			break;
	if( pClient )
	{
		// oterhwise we'll deadlock the closing client...
		// an opening condition has global lock (above)
      // and a closing socket will want the global lock before it's done.
   	pClient = AddActive( GrabClient( pClient ) );
      EnterCriticalSecEx( &pClient->csLock DBG_RELAY );
      ClearClient( pClient ); // clear client is redundant here... but saves the critical section now
      //Log1( WIDE("New network client %p"), client );
   }
	else
	{
		LeaveCriticalSec( &g.csNetwork );
		RescheduleTimerEx( g.uPendingTimer, 1 );
		Relinquish();
		if( g.AvailableClients )
		{
         lprintf( WIDE( "there were clients available... just in a closing state..." ) );
			goto get_client;
		}
		Log( WIDE("No unused network clients are available.") );
      return NULL;
	}
   LeaveCriticalSec( &g.csNetwork );
   return pClient;
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetNetworkLong )(PCLIENT lpClient, int nLong, PTRSZVAL dwValue)
{
   if( lpClient && ( nLong < g.nUserData ) )
   {
      *(PTRSZVAL*)(lpClient->lpUserData+(nLong * sizeof(PTRSZVAL))) = dwValue;
   }
   return;
}

//----------------------------------------------------------------------------

int GetAddressParts( SOCKADDR *sa, _32 *pdwIP, _16 *pdwPort )
{
	if( sa )
	{
		if( sa->sa_family == AF_INET )
		{
			if( pdwIP )
				(*pdwIP) = (_32)(((SOCKADDR_IN*)sa)->sin_addr.s_addr);
			if( pdwPort )
				(*pdwPort) = ntohs((_16)( (SOCKADDR_IN*)sa)->sin_port);
         return TRUE;
		}
	}
   return FALSE;
}

NETWORK_PROC( PTRSZVAL, GetNetworkLong )(PCLIENT lpClient,int nLong)
{
   if( !lpClient )
   {
      return (PTRSZVAL)-1;
   }
   if( nLong < 0 )
   {
      switch( nLong )
      {
      case GNL_IP:  // IP of destination
         return *(_32*)(lpClient->saClient->sa_data+2);
      case GNL_PORT:  // port of server...  STUPID PATCH?!  maybe...
         return ntohs( *(_16*)(lpClient->saClient->sa_data) );
      case GNL_MYPORT:  // port of server...  STUPID PATCH?!  maybe...
         return ntohs( *(_16*)(lpClient->saSource->sa_data) );
		case GNL_MYIP: // IP of myself (after connect?)
			return *(_32*)(lpClient->saSource->sa_data+2);

			//TODO if less than zero return a (high/low)portion of the  hardware address (MAC).
      }
   }
   else if( nLong < g.nUserData )
   {
      return(*(PTRSZVAL*)(lpClient->lpUserData + (nLong * sizeof(PTRSZVAL))));
   }

   return (PTRSZVAL)-1;   //spv:980303
}

//----------------------------------------------------------------------------

/*
NETWORK_PROC( _16, GetNetworkWord )(PCLIENT lpClient,int nWord)
{
   if( !lpClient )
      return 0xFFFF;
   if( nWord < (g.nUserData *2) )
	   return(*(_16*)(lpClient->lpUserData + (nWord * 2)));
	return 0xFFFF;
}
*/
//---------------------------------------------------------------------------



NETWORK_PROC( SOCKADDR *, DuplicateAddress )( SOCKADDR *pAddr ) // return a copy of this address...
{
   POINTER tmp = (POINTER)( ( (PTRSZVAL)pAddr ) - sizeof(PTRSZVAL) );
	SOCKADDR *dup = AllocAddr();
   POINTER tmp2 = (POINTER)( ( (PTRSZVAL)dup ) - sizeof(PTRSZVAL) );
	MemCpy( tmp2, tmp, MAGIC_SOCKADDR_LENGTH + sizeof(PTRSZVAL) );
   return dup;
}


//---------------------------------------------------------------------------

NETWORK_PROC( SOCKADDR *,CreateAddress_hton)( _32 dwIP,_16 nHisPort)
{
   SOCKADDR_IN *lpsaAddr=(SOCKADDR_IN*)AllocAddr();
   if (!lpsaAddr)
      return(NULL);
   lpsaAddr->sin_family       = AF_INET;         // InetAddress Type.
   lpsaAddr->sin_addr.s_addr  = htonl(dwIP);
   lpsaAddr->sin_port         = htons(nHisPort);
   return((SOCKADDR*)lpsaAddr);
}

//---------------------------------------------------------------------------
#if defined( __LINUX__ ) && !defined( __CYGWIN__ )
 #define UNIX_PATH_MAX    108

struct sockaddr_un {
	sa_family_t  sun_family;		/* AF_UNIX */
	char	       sun_path[UNIX_PATH_MAX]; /* pathname */
};

NETWORK_PROC( SOCKADDR *,CreateUnixAddress)( CTEXTSTR path )
{
   struct sockaddr_un *lpsaAddr;
   lpsaAddr=(struct sockaddr_un*)AllocAddr();
   if (!lpsaAddr)
		return(NULL);
	lpsaAddr->sun_family = PF_UNIX;
	strncpy( lpsaAddr->sun_path, path, 107 );
   return((SOCKADDR*)lpsaAddr);
}
#else
NETWORK_PROC( SOCKADDR *,CreateUnixAddress)( CTEXTSTR path )
{
   lprintf( WIDE( "-- CreateUnixAddress -- not available. " ) );
   return NULL;
}
#endif
//---------------------------------------------------------------------------

SOCKADDR *CreateAddress( _32 dwIP,_16 nHisPort)
{
   SOCKADDR_IN *lpsaAddr=(SOCKADDR_IN*)AllocAddr();
   if (!lpsaAddr)
      return(NULL);
   lpsaAddr->sin_family       = AF_INET;         // InetAddress Type.
   lpsaAddr->sin_addr.s_addr  = dwIP;
   lpsaAddr->sin_port         = htons(nHisPort);
   return((SOCKADDR*)lpsaAddr);
}

//---------------------------------------------------------------------------

SOCKADDR *CreateRemote(CTEXTSTR lpName,_16 nHisPort)
{
	SOCKADDR_IN *lpsaAddr;
   int conversion_success = FALSE;
#ifndef __WINDOWS__
	PHOSTENT phe;
	// a IP type name will never have a / in it, therefore
	// we can assume it's a unix type address....
	if( lpName && strchr( lpName, '/' ) )
		return CreateUnixAddress( lpName );
#endif
	lpsaAddr=(SOCKADDR_IN*)AllocAddr();
	if (!lpsaAddr)
		return(NULL);

	// if it's a numeric name... attempt to use as an address.
#ifdef __LINUX__
	if( lpName &&
		( lpName[0] >= '0' && lpName[0] <= '9' )
	  && StrChr( lpName, '.' ) )
	{
#ifdef UNICODE
		char *tmp = CStrDup( lpName );
		if( inet_pton( AF_INET, tmp, (struct in_addr*)&lpsaAddr->sin_addr ) > 0 )
		{
			lpsaAddr->sin_family       = AF_INET;         // InetAddress Type.
			conversion_success = TRUE;
		}
		Release( tmp );
#else
		if( inet_pton( AF_INET, lpName, (struct in6_addr*)&lpsaAddr->sin_addr ) > 0 )
		{
			lpsaAddr->sin_family       = AF_INET;         // InetAddress Type.
			conversion_success = TRUE;
		}
#endif
	}
	else 	if( lpName &&
		( lpName[0] >= '0' && lpName[0] <= '9' )
				&& StrChr( lpName, ':' )!=StrRChr( lpName, ':' ) )
	{
#ifdef UNICODE
		char *tmp = CStrDup( lpName );
		if( inet_pton( AF_INET6, tmp, (struct in_addr*)&lpsaAddr->sin_addr ) > 0 )
		{
			lpsaAddr->sin_family       = AF_INET6;         // InetAddress Type.
			conversion_success = TRUE;
		}
		Release( tmp );
#else
		if( inet_pton( AF_INET6, lpName, (struct in6_addr*)&lpsaAddr->sin_addr ) > 0 )
		{
			lpsaAddr->sin_family       = AF_INET6;         // InetAddress Type.
			conversion_success = TRUE;
		}
#endif
	}
#endif
   if( !conversion_success )
	{
		if( lpName )
		{
#ifdef __WINDOWS__
			{
				ADDRINFO *result;
				ADDRINFO *test;
				getaddrinfo( lpName, NULL, NULL, &result );
				for( test = result; test; test = test->ai_next )
				{
					//SOCKADDR *tmp;
					//AddLink( &g.addresses, tmp = AllocAddr() );
					MemCpy( lpsaAddr, test->ai_addr, test->ai_addrlen );
					SET_SOCKADDR_LENGTH( lpsaAddr, test->ai_addrlen );
               break;
				}
			}
#else //__WINDOWS__

			char *tmp = CStrDup( lpName );
			if(!(phe=gethostbyname(tmp)))
			{
				if( !(phe=gethostbyname2(tmp,AF_INET6) ) )
				{
					if( !(phe=gethostbyname2(tmp,AF_INET) ) )
					{
 						// could not find the name in the host file.
						Log1( WIDE("Could not Resolve to %s"), lpName );
						Release(lpsaAddr);
						Release( tmp );
						return(NULL);
					}
					else
					{
						lprintf( "Strange, gethostbyname failed, but AF_INET worked..." );
						SET_SOCKADDR_LENGTH( pc->saLastClient, 16 );
						lpsaAddr->sin_family = AF_INET;
					}
				}
				else
				{
					SET_SOCKADDR_LENGTH( pc->saLastClient, 28 );
					lpsaAddr->sin_family = AF_INET6;         // InetAddress Type.
				}
			}
			else
			{
				Release( tmp );
				SET_SOCKADDR_LENGTH( pc->saLastClient, 16 );
				lpsaAddr->sin_family = AF_INET;         // InetAddress Type.
			}
			memcpy( &lpsaAddr->sin_addr.s_addr,           // save IP address from host entry.
					 phe->h_addr,
					 phe->h_length);
#endif
		}
		else
		{
			lpsaAddr->sin_family      = AF_INET;         // InetAddress Type.
			lpsaAddr->sin_addr.s_addr = 0;
		}
	}
	// put in his(destination) port number...
	lpsaAddr->sin_port         = htons(nHisPort);
	return((SOCKADDR*)lpsaAddr);
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
namespace udp {
#endif
NETWORK_PROC( void, DumpAddrEx)( CTEXTSTR name, SOCKADDR *sa DBG_PASS )
{
     Log9( WIDE("%s: %03d,%03d,%03d,%03d,%03d,%03d,%03d,%03d "), name,
    				*(((unsigned char *)sa)+0),
    				*(((unsigned char *)sa)+1),
    				*(((unsigned char *)sa)+2),
    				*(((unsigned char *)sa)+3),
    				*(((unsigned char *)sa)+4),
    				*(((unsigned char *)sa)+5),
    				*(((unsigned char *)sa)+6),
    				*(((unsigned char *)sa)+7) );
}
#ifdef __cplusplus
}
#endif
//----------------------------------------------------------------------------

NETWORK_PROC( SOCKADDR *, SetAddressPort )( SOCKADDR *pAddr, _16 nDefaultPort )
{
	if( pAddr )
		((SOCKADDR_IN *)pAddr)->sin_port = htons(nDefaultPort);
   return pAddr;
}

//----------------------------------------------------------------------------

NETWORK_PROC( SOCKADDR *, SetNonDefaultPort )( SOCKADDR *pAddr, _16 nDefaultPort )
{
	if( pAddr && !((SOCKADDR_IN *)pAddr)->sin_port )
      ((SOCKADDR_IN *)pAddr)->sin_port = htons(nDefaultPort);
   return pAddr;
}

//----------------------------------------------------------------------------

NETWORK_PROC( SOCKADDR *,CreateSockAddress)( CTEXTSTR name, _16 nDefaultPort )
{
// blah... should process a ip:port - but - default port?!
	_32 bTmpName = 0;
	TEXTSTR tmp;
	SOCKADDR *sa = NULL;
	TEXTCHAR *port;
	_16 wPort;
	if( name && ( port = strrchr( (TEXTSTR)name, ':' ) ) )
	{
		tmp = StrDup( name );
		bTmpName = 1;
		port = tmp + (port-name);
		name = tmp;
		//Log1( WIDE("Found ':' assuming %s is IP:PORT"), name );
		*port = 0;
		port++;
		if( isdigit( *port ) )
		{
			wPort = (short)atoi( port );
		}
		else
		{
			struct servent *se;
			char *tmp2 = CStrDup( port );
			se = getservbyname( tmp2, NULL );
			Release( tmp2 );
			if( !se )
			{
				Log1( WIDE("Could not resolve \"%s\" as a valid service name"), port);
				if( bTmpName ) Release( tmp );
				return NULL;
			}
			wPort = htons(se->s_port);
					//Log1( WIDE("port alpha - name resolve to %d"), wPort );
		}
		sa = CreateRemote( name, wPort );
		if( port )
		{
			port[-1] = ':';  // incase we obliterated it
		}
	}
	else  // no port specification...
	{
		//Log1( WIDE("%s does not have a ':'"), name );
		sa = CreateRemote( name, nDefaultPort );
	}
	if( bTmpName ) Release( tmp );
	return sa;
}

//----------------------------------------------------------------------------

SOCKADDR *CreateLocal(_16 nMyPort)
{
   char lpHostName[HOSTNAME_LEN];

   if (gethostname(lpHostName,HOSTNAME_LEN))
   {
//      EventLog(SOH_VER_HOST_NAME,gwCategory,0,NULL,0,NULL);
      return(NULL);
   }
   return CreateRemote( WIDE("0.0.0.0"), nMyPort );
}

//----------------------------------------------------------------------------

LOGICAL CompareAddress( SOCKADDR *sa1, SOCKADDR *sa2 )
{
	if( sa1 && sa2 )
	{
		if( ((SOCKADDR_IN*)sa1)->sin_family == ((SOCKADDR_IN*)sa2)->sin_family )
		{
			switch( ((SOCKADDR_IN*)sa1)->sin_family )
			{
			case AF_INET:
				{
					SOCKADDR_IN *sin1 = (SOCKADDR_IN*)sa1;
					SOCKADDR_IN *sin2 = (SOCKADDR_IN*)sa2;
					if( MemCmp( sin1, sin2, sizeof( SOCKADDR_IN ) ) == 0 )
						return 1;
				}
				break;
			default:
				xlprintf( LOG_ALWAYS )( WIDE("unhandled address type passed to compare, resulting FAILURE") );
				return 0;
			}
		}
	}
   return 0;
}

//----------------------------------------------------------------------------

LOGICAL IsThisAddressMe( SOCKADDR *addr, _16 myport )
{
	SOCKADDR *test_addr;
	INDEX idx;
	LIST_FORALL( g.addresses, idx, SOCKADDR*, test_addr )
	{
		if( ((SOCKADDR_IN*)addr)->sin_family == ((SOCKADDR_IN*)test_addr)->sin_family )
		{
			switch( ((SOCKADDR_IN*)addr)->sin_family )
			{
			case AF_INET:
				{
					if( MemCmp( &((SOCKADDR_IN*)addr)->sin_addr, &((SOCKADDR_IN*)test_addr)->sin_addr, sizeof(((SOCKADDR_IN*)addr)->sin_addr)  ) == 0 )
					{
                  return TRUE;
					}
				}
            break;
			default:
            lprintf( "Unknown comparison" );
			}
		}
	}
	return FALSE;
}

//----------------------------------------------------------------------------

void ReleaseAddress(SOCKADDR *lpsaAddr)
{
   // sockaddr is often skewed from what I would expect it.
   Release ((POINTER)( ( (PTRSZVAL)lpsaAddr ) - sizeof(PTRSZVAL) ));
}

//----------------------------------------------------------------------------
// creates class C broadcast address
SOCKADDR *CreateBroadcast(_16 nPort)
{
   SOCKADDR_IN *bcast=(SOCKADDR_IN*)AllocAddr();
   SOCKADDR *lpMyAddr;
   if (!bcast)
      return(NULL);
   lpMyAddr = CreateLocal(0);
   bcast->sin_family       = AF_INET;
   bcast->sin_addr.s_addr  = ((SOCKADDR_IN*)lpMyAddr)->sin_addr.s_addr;
   bcast->sin_addr.s_impno = 0xFF; // Fake a subnet broadcast address
   bcast->sin_port        = htons(nPort);
   ReleaseAddress(lpMyAddr);
   return((SOCKADDR*)bcast);
}

//----------------------------------------------------------------------------

void DumpSocket( PCLIENT pc )
{
	DumpAddr( WIDE("REMOT"), pc->saClient );
	DumpAddr( WIDE("LOCAL"), pc->saSource );
   return;
}

#ifdef __cplusplus
namespace udp {
#endif
#undef DumpAddr
NETWORK_PROC( void, DumpAddr)( CTEXTSTR name, SOCKADDR *sa )
{
   DumpAddrEx( name, sa DBG_SRC );
}

#ifdef __cplusplus
}
#endif


//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, NetworkLockEx)( PCLIENT lpClient DBG_PASS )
{
	if( lpClient )
	{
		THREAD_ID prior = 0;
	start_lock:
      lpClient->dwFlags |= CF_WANTS_GLOBAL_LOCK;
		EnterCriticalSecEx( &g.csNetwork DBG_RELAY );
      lpClient->dwFlags &= CF_WANTS_GLOBAL_LOCK;
		if( EnterCriticalSecNoWaitEx( &lpClient->csLock, &prior DBG_RELAY ) < 1 )
		{
			// unlock the global section for a moment..
			// client may be requiring both local and global locks (already has local lock)
			LeaveCriticalSecEx( &g.csNetwork  DBG_RELAY);
			goto start_lock;
		}
		//EnterCriticalSec( &lpClient->csLock );
		LeaveCriticalSecEx( &g.csNetwork  DBG_RELAY);
   	if( !(lpClient->dwFlags & CF_ACTIVE ) )
	   {
   	    // change to inactive status by the time we got here...
      	 LeaveCriticalSec( &lpClient->csLock );
	       // this client is not available for client use!
   	    return NULL;
	   }
	}
	return lpClient;
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, NetworkUnlockEx)( PCLIENT lpClient DBG_PASS )
{
	// simple unlock.
	if( lpClient )
		LeaveCriticalSecEx( &lpClient->csLock DBG_RELAY );

}

#undef NetworkLock
#undef NetworkUnlock
NETWORK_PROC( PCLIENT, NetworkLock)( PCLIENT lpClient )
{
   return NetworkLockEx( lpClient DBG_SRC );
}

NETWORK_PROC( void, NetworkUnlock)( PCLIENT lpClient )
{
   NetworkUnlockEx( lpClient DBG_SRC );
}
//----------------------------------------------------------------------------

void InternalRemoveClientExx(PCLIENT lpClient, LOGICAL bBlockNofity, LOGICAL bLinger DBG_PASS )
{
#ifdef LOG_SOCKET_CREATION
   _lprintf( DBG_RELAY )( WIDE("Removing this client %p (%d)"), lpClient, lpClient->Socket );
#endif

   if( lpClient && IsValid(lpClient->Socket) )
	{

      if( !bLinger )
		{
#ifdef LOG_DEBUG_CLOSING
			lprintf( "Setting quick close?!" );
#endif
			{
				int nAllowReuse = 1;
				if (setsockopt(lpClient->Socket, SOL_SOCKET, SO_REUSEADDR,
									(char*)&nAllowReuse, sizeof(nAllowReuse)) <0 )
				{
					//cerr << "NFMSim:setHost:ERROR: could not set socket to reuse addr." << endl;
				}

				/*
             // missing symbol in windows?
				if (setsockopt(lpClient->Socket, SOL_SOCKET, SO_REUSEPORT,
									(char*)&nAllowReuse, sizeof(nAllowReuse)) <0 )
				{
					//cerr << "NFMSim:setHost:ERROR: could not set socket to reuse port." << endl;
					}
               */
			}
			{
			struct linger lingerSet;
			lingerSet.l_onoff = 1; // on , with no time = off.
			lingerSet.l_linger = 0;
			// set server to allow reuse of socket port
			if (setsockopt(lpClient->Socket, SOL_SOCKET, SO_LINGER,
								(char*)&lingerSet, sizeof(lingerSet)) <0 )
			{
            lprintf( WIDE( "error setting no linger in close." ) );
				//cerr << "NFMSim:setHost:ERROR: could not set socket to linger." << endl;
			}
			}
		}

   	if( !(lpClient->dwFlags & CF_ACTIVE) )
   	{
   		if( lpClient->dwFlags & CF_AVAILABLE )
   		{
				Log( WIDE("Client was inactive?!?!?! removing from list and putting in available") );
            AddAvailable( GrabClient( lpClient ) );
			}
			// this is probably true, we've definatly already moved it from
         // active list to clsoed list.
   		else if( !(lpClient->dwFlags & CF_CLOSED) )
   		{
   		   Log( WIDE("Client was NOT already closed?!?!") );
   		}
#ifdef LOG_DEBUG_CLOSING
   		else
				Log( WIDE("Client's state is CLOSED") );
#endif
   		return;
		}
		{
			THREAD_ID prior = 0;
		start_lock:
			lpClient->dwFlags |= CF_WANTS_GLOBAL_LOCK;
			EnterCriticalSec( &g.csNetwork );
			lpClient->dwFlags &= ~CF_WANTS_GLOBAL_LOCK;
			{
				if( EnterCriticalSecNoWait( &lpClient->csLock, &prior ) < 1 )
				{
					// unlock the global section for a moment..
					// client may be requiring both local and global locks (already has local lock)
					LeaveCriticalSec( &g.csNetwork );
					goto start_lock;
				}
			}
		}
		//EnterCriticalSecEx( &lpClient->csLock DBG_RELAY ); // modifying client availability...
		LeaveCriticalSec( &g.csNetwork );

      // allow application a chance to clean it's references
      // to this structure before closing and cleaning it.

      if( !bBlockNofity )
      {
         if( !(lpClient->dwFlags & CF_CLOSING) ) // prevent multiple notifications...
			{
#ifdef LOG_DEBUG_CLOSING
				lprintf( WIDE( "Marked closing first, and dispatching callback?" ) );
#endif
         	lpClient->dwFlags |= CF_CLOSING;
				if( lpClient->close.CloseCallback )
				{
					// during thisi if it wants a lock... and the application
               // is dispatching like
            	if( lpClient->dwFlags & CF_CPPCLOSE )
	               lpClient->close.CPPCloseCallback( lpClient->psvClose );
            	else
						lpClient->close.CloseCallback( lpClient );
				}
#ifdef LOG_DEBUG_CLOSING
				else
					lprintf( WIDE( "no close callback!?" ) );
#endif
				// leave the flag closing set... we'll use that later
            // to avoid the double-lock;
         	//lpClient->dwFlags &= ~CF_CLOSING;
			}
#ifdef LOG_DEBUG_CLOSING
			else
				lprintf( WIDE( "socket was already ispatched callback?" ) );
#endif
		}
		else
		{
#ifdef LOG_DEBUG_CLOSING
         lprintf( WIDE( "blocknotify on close..." ) );
#endif
		}
      EnterCriticalSec( &g.csNetwork );
#ifdef LOG_DEBUG_CLOSING
		lprintf( WIDE( "Adding current client to closed clients." ) );
#endif
		AddClosed( GrabClient( lpClient ) );
#ifdef LOG_DEBUG_CLOSING
		lprintf( WIDE( "Leaving client critical section" ) );
#endif
		LeaveCriticalSec( &lpClient->csLock );
      //lprintf( WIDE( "Leaving network critical section" ) );
		LeaveCriticalSec( &g.csNetwork );
      //lprintf( WIDE( "And no nothing is locked?!" ) );
   }
#ifdef LOG_DEBUG_CLOSING
   else
   {
   	Log( WIDE("No Client, or socket already closed?") );
	}
#endif
}

void RemoveClientExx(PCLIENT lpClient, LOGICAL bBlockNotify, LOGICAL bLinger DBG_PASS )
{
	InternalRemoveClientExx( lpClient, bBlockNotify, bLinger DBG_RELAY );
	TerminateClosedClient( lpClient );
}

CTEXTSTR GetSystemName( void )
{
   return g.system_name;
}

SACK_NETWORK_NAMESPACE_END

// $Log: network.c,v $
// Revision 1.103  2005/05/27 16:41:28  d3x0r
// Moved network section critical lock to avoid deadlock.
//
// Revision 1.113  2005/05/26 21:22:32  jim
// Lock network section around client lock like everywhere else - otherwise there's a possible deadlock on the csNetwork/pc->cs locking...
//
// Revision 1.112  2005/05/16 19:07:57  jim
// Modified logging to account for source of events.  Fixed wait loop to WaitMessage() under windows. instead of fixed 10ms delay
//
// Revision 1.111  2005/05/16 17:21:37  jim
// Add proposed network shutdown at priority 40... 40 is he same level as message service.
//
// Revision 1.110  2005/05/13 21:17:39  jim
// Fix address building.
//
// Revision 1.109  2005/03/31 20:23:19  panther
// Don't check a NULL name for a unix socket '/'
//
// Revision 1.108  2005/03/22 12:42:41  panther
// Clean up some noisy logging messages.
//
// Revision 1.107  2005/03/21 20:53:17  panther
// Make CreateRemote smarter and auto handle creation of Unix sockets.
//
// Revision 1.106  2005/03/15 20:22:15  chrisd
// Oopps build unix socket as PF_UNIX not PF_INET
//
// Revision 1.105  2005/03/15 20:13:41  panther
// Okay and then that shoudl work.
//
// Revision 1.104  2005/03/15 20:10:48  panther
// Add support for creating unix socket addresses, then those are passable to OPenTCP to switch handling to a unix socket type instead of TCP
//
// Revision 1.103  2005/01/27 07:37:11  panther
// Linux cleaned.
//
// Revision 1.102  2005/01/26 06:52:58  panther
// Add error message if thread is double started
//
// Revision 1.101  2004/09/29 00:49:49  d3x0r
// Added fancy wait for PSI frames which allows non-polling sleeping... Extended Idle() to result in meaningful information.
//
// Revision 1.100  2004/09/10 01:24:01  d3x0r
// With new changes to allow network re-initialzation to add user data or client space... the new global user client space was not set... also, move tcp write logging deeper.
//
// Revision 1.99  2004/09/10 00:22:37  d3x0r
// It would help if we did save the user data size...
//
// Revision 1.98  2004/09/02 10:20:30  d3x0r
// Minor fixes for linux compile...
//
// Revision 1.97  2004/08/18 23:53:07  d3x0r
// Cleanups - also enhanced network init to expand if called with larger params.
//
// Revision 1.96  2004/08/18 23:39:40  jim
// Redo network initialize to handle re-initialzation... needed cause sometimes I need to start networking without knowing what the actual working params will be.
//
// Revision 1.91  2003/11/10 03:35:09  panther
// Fixed cpp open for text names
//
// Revision 1.90  2003/11/09 22:35:33  panther
// Remove noisy logging.  Critical Sections are stable now
//
// Revision 1.89  2003/11/09 05:12:56  panther
// Cleanup idle proc at quit
//
// Revision 1.88  2003/11/09 03:32:15  panther
// Added some address functions to set port and override default port
//
// Revision 1.87  2003/10/27 01:27:13  panther
// Implement semaphore on linux for perma sleep
//
// Revision 1.86  2003/10/23 15:42:54  panther
// Added some noisy logging.  Fixed on econnect
//
// Revision 1.85  2003/10/22 02:10:21  panther
// Remove some final trailing critical section marks
//
// Revision 1.84  2003/10/21 03:53:46  panther
// Trying to track down lost event... still doesn't work..
//
// Revision 1.83  2003/10/17 00:56:05  panther
// Rework critical sections.
// Moved most of heart of these sections to timers.
// When waiting, sleep forever is used, waking only when
// released... This is preferred rather than continuous
// polling of section with a Relinquish.
// Things to test, event wakeup order uner linxu and windows.
// Even see if the wake ever happens.
// Wake should be able to occur across processes.
// Also begin implmeenting MessageQueue in containers....
// These work out of a common shared memory so multiple
// processes have access to this region.
//
// Revision 1.82  2003/09/24 22:32:54  panther
// Update for latest type procs...
//
// Revision 1.81  2003/09/24 15:10:54  panther
// Much mangling to extend C++ network interface...
//
// Revision 1.80  2003/07/29 10:40:55  panther
// clean minor warnings, unused var, parents around assignment
//
// Revision 1.79  2003/07/25 15:06:09  panther
// Remove noisy logging.  Revert thread ID change
//
// Revision 1.78  2003/07/24 22:50:10  panther
// Updates to make watcom happier
//
// Revision 1.77  2003/07/21 14:16:49  panther
// Add idle proc support
//
// Revision 1.76  2003/06/04 11:39:31  panther
// Stripped carriage returns
//
// Revision 1.75  2003/04/20 01:14:13  panther
// Added another optional log_notices
//
// Revision 1.74  2003/04/18 09:28:35  panther
// Oops...
//
// Revision 1.73  2003/04/18 09:28:04  panther
// Revert ProcessMessagesEx
//
// Revision 1.72  2003/04/18 09:26:04  panther
// Fixed network wake issues associated with bad wake threads
//
// Revision 1.71  2003/04/17 13:17:09  panther
// Respond OKAY to WM_QUERYENDSESSION
//
// Revision 1.70  2003/03/17 22:45:33  panther
// Don't wait for timer to be set before allowing timer to run - but do wait before removing it
//
// Revision 1.69  2003/03/17 22:05:47  panther
// Fix timer fix - handle event messages lighter
//
// Revision 1.68  2003/03/17 22:05:05  panther
// Schedule timer to init network initial short.
//
// Revision 1.67  2003/02/13 10:57:38  panther
// Added checking - if pending data on a newly connected socket, do the send data
//
// Revision 1.66  2003/01/31 17:28:42  panther
// Use standard WakeThread/WakableSleep on Unix
//
// Revision 1.65  2003/01/31 17:13:03  panther
// : was used instead of a ;
//
// Revision 1.64  2003/01/29 06:48:58  panther
// Fix dangling sockaddr allocation
//
// Revision 1.63  2003/01/29 05:26:49  panther
// Don't unmake thread - and wait for thread to gracefully exit
//
// Revision 1.62  2003/01/29 04:38:09  panther
// Use standard ThreadTo from timer lib.  Clean up exit code
//
// Revision 1.61  2003/01/28 02:24:43  panther
// Fixes to network - common timer for network pause... minor updates which should have been commited already
//
// Revision 1.60  2002/12/22 00:14:11  panther
// Cleanup function declarations and project defines.
//
// Revision 1.59  2002/11/24 21:37:41  panther
// Mods - network - fix server->accepted client method inheritance
// display - fix many things
// types - merge chagnes from verious places
// ping - make function result meaningful yes/no
// controls - fixes to handle lack of image structure
// display - fixes to handle moved image structure.
//
// Revision 1.59  2002/11/21 19:13:29  jim
// Added CreateAdress, CreateAddress_hton
//
// Revision 1.58  2002/08/22 20:09:33  panther
// Added relinquish - seems to fix creation of timer thread issue under linux
// (sorcerer 2.4.19, newest glibc)
//
// Revision 1.57  2002/08/01 13:09:06  panther
// Modified logging in TCPnetwork.
// Check for NULL pclient in UNLOCK!
//
// Revision 1.56  2002/07/31 08:27:43  panther
// Did not check for CONNECT_ERROR when connect event was received.  This
// caused read_complete to be called before the socket was connected.
//
// Revision 1.55  2002/07/25 12:59:02  panther
// Added logging, removed logging....
// Network: Added NetworkLock/NetworkUnlock
// Timers: Modified scheduling if the next timer delta was - how do you say -
// to fire again before now.
//
// Revision 1.54  2002/05/22 15:14:05  panther
// Set CONNECTERROR Flag instead of just dumping the socket.
//
// Revision 1.53  2002/05/22 15:04:44  panther
// Changes additions and stuff... mostly linux compat
//
// Revision 1.52  2002/05/20 16:52:59  panther
// Lets add a nowait enter critical secion...
//
// Revision 1.51  2002/05/01 14:59:40  panther
// Memlib - minor log change?
// Network - modification on FD_CLOSE events to not leave critical sec
// controls - changed loop... added then removed logging
//
// Revision 1.50  2002/04/29 15:37:02  panther
// Removed commented typedefs from sack_types.h, modified error path for connect
// especially on windows side.
//
// Revision 1.49  2002/04/25 04:44:02  panther
// Fixed memset to MemSet...
//
// Revision 1.48  2002/04/25 00:05:04  panther
// Added logging of PostQuitMessages/WM_QUIT...
//
// Revision 1.47  2002/04/23 15:31:20  panther
// Forgot a #else in NetworkQuit
//
// Revision 1.46  2002/04/22 23:29:51  panther
// Removed temporary #define LOG_NOTICES
// Added LOG_NOTICES wrappers around unix events.
//
// Revision 1.45  2002/04/22 22:58:18  panther
// Refitted Windows event notification so that FD_CONNECT generates an inital
// read callback, and we should get a FD_READ shortly thereafter... think this
// was badly evolved since we didn't always process FD_CONNECT...
//
// Revision 1.44  2002/04/22 22:32:44  panther
// Modified Unix logging... fairly quiet except exceptions... still have a long
// delay for a transaction unix->windows not sure where still.
//
// Revision 1.43  2002/04/22 21:46:15  panther
// Reworked linux select loop lockings...
//
// Revision 1.42  2002/04/21 22:03:14  panther
// Minor changes to enter/leave critical sections...
//
// Revision 1.41  2002/04/19 22:56:30  panther
// Okay windows notice - FD_CLOSE need LeaveCriticalSection
// then when we check this socket we're locking for - check stateflags.
//
// Revision 1.40  2002/04/19 22:35:02  panther
// Forgot some leave cricital sections on read/write errors...
//
// Revision 1.39  2002/04/19 21:38:27  panther
// Oops - forgot a DWORD -> _32
//
// Revision 1.38  2002/04/19 21:37:06  panther
// Missed a rollback on the udpnetwork.c to use windows asynch select...
// Added much more logging on udp path for errors and information.
//
// Revision 1.37  2002/04/19 20:46:22  panther
// Modified internal usage to InternalRemoveClient, extern (application) will
// immediately close a client.  Erased usage of internal connection count.
// Test proxy success on both delayed and immediate connect.
//
// Revision 1.36  2002/04/19 18:09:51  panther
// Unix cleanup of massive changes....
//
// Revision 1.35  2002/04/19 17:42:07  panther
// Sweeping changes added queues of sockets in various states, closes
// rehang the client into a closed structure which is then timed out...
// Added event timers, cleaned up global code into global structure...
//
// Revision 1.34  2002/04/18 23:25:45  panther
// Well - let's go towards the unix-y select thing... Seems to work so far
// and we won't have queue'd events clogging up things, plus we won't need
// a blank window....
//
// Revision 1.33  2002/04/18 20:42:52  panther
// minor cleanup.
//
// Revision 1.32  2002/04/18 20:38:13  panther
// Remove some redundant reads if _WIN32 - no auto finishpending...
// Otherwise we had many empty FD_READ operations... This also caused
// the FD_READ ops to be queued pending process, and the FD_CLOSE was
// queued way after... This SEEMS to be stable.
//
// Revision 1.31  2002/04/18 20:25:47  panther
// Begin using client local critical section locks (buggy)
// Still fighting double-remove problems...
//
// Revision 1.30  2002/04/18 17:28:20  panther
// Added conditional option flag - LOG_NOTICES to log windows event messages.
//
// Revision 1.29  2002/04/18 17:26:02  panther
// Removed excess debugging messages - extended timer for auto scanning closed
// and pending clients...
//
// Revision 1.28  2002/04/18 16:16:53  panther
// Minor log changes for further information...
//
// Revision 1.27  2002/04/18 15:13:55  panther
// Added per client cricticalsection locked(unused)
// Added timers for the pending closes (mostly)
// Fixed minor warning in ping.c (no type default int, missing prototypes)
//
// Revision 1.26  2002/04/18 15:01:42  panther
// Added Log History
// Stage 1: Delay schedule closes - only network thread will close.
//
