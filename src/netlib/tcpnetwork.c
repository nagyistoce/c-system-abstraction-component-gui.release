// DEBUG FALGS in netstruc.h


//TODO: after the connect and just before the call to the connect callback fill in the PCLIENT's MAC addr field.
//TODO: After the accept, put in this code:
/*

NETWORK_PROC( int, GetMacAddress)(CTEXTSTR device, CTEXTSTR buffer )//int get_mac_addr (char *device, unsigned char *buffer)
{
int fd;
struct ifreq ifr;

fd = socket(PF_UNIX, SOCK_DGRAM, 0);
if (fd == -1)
{
perr ("Unable to create socket for device: %s", device);
return -1;
}

strcpy (ifr.ifr_name, device);

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

memcpy (buffer, ifr.ifr_hwaddr.sa_data, 6);

return 0;
}

*/
//#define LOG_SOCKET_CREATION
#define LIBRARY_DEF
#include <stdhdrs.h>
#include <timers.h>
#include "netstruc.h"
#include <network.h>
#ifndef UNDER_CE
#include <fcntl.h>
#endif
#include <idle.h>
#ifdef __UNIX__
#include <unistd.h>
#ifdef __LINUX__

#undef s_addr
#include <netinet/in.h> // IPPROTO_TCP
//#include <linux/in.h>  // IPPROTO_TCP
#include <netinet/tcp.h> // TCP_NODELAY
//#include <linux/tcp.h> // TCP_NODELAY
#include <fcntl.h>

#else
#endif

#include <sys/ioctl.h>
#include <signal.h> // SIGHUP defined

#define NetWakeSignal SIGHUP

#else
#define ioctl ioctlsocket

#endif

#include <sharemem.h>
#include <procreg.h>

//#define NO_LOGGING // force neverlog....
#include "logging.h"

SACK_NETWORK_NAMESPACE
	extern int CPROC ProcessNetworkMessages( PTRSZVAL unused );

_TCP_NAMESPACE

//----------------------------------------------------------------------------
LOGICAL TCPDrainRead( PCLIENT pClient );
//----------------------------------------------------------------------------

void AcceptClient(PCLIENT pListen)
{
#ifdef __LINUX__
	socklen_t
#else
		int
#endif
		nTemp;
   PCLIENT pNewClient = NULL;// just to be safe.

   pNewClient = GetFreeNetworkClient();
   // new client will be locked... 
	if( !pNewClient )
	{
      SOCKADDR *junk = AllocAddr();
		nTemp = MAGIC_SOCKADDR_LENGTH;
		Log("GetFreeNetwork() returned NULL. Exiting AcceptClient, accept and drop connection");
		closesocket( accept( pListen->Socket, junk ,&nTemp	));
      ReleaseAddress( junk );
		return;
	}

   // without setting this value - the pointer to the value
   // contains a value which may be less than a valid address
   // length... usually didn't JAB: 980203
	nTemp = MAGIC_SOCKADDR_LENGTH;

   pNewClient->Socket = accept( pListen->Socket
										, pNewClient->saClient
										,&nTemp
										);

#ifdef LOG_SOCKET_CREATION
	Log2( WIDE("Accepted socket %d  (%d)"), pNewClient->Socket, nTemp );
#endif
	//DumpAddr( WIDE("Client's Address"), pNewClient->saClient );
	{
#ifdef __LINUX__
		socklen_t
#else
			int
#endif
			nLen = MAGIC_SOCKADDR_LENGTH;
		if( !pNewClient->saSource )
			pNewClient->saSource = AllocAddr();
		if( getsockname( pNewClient->Socket, pNewClient->saSource, &nLen ) )
		{
			lprintf( WIDE("getsockname errno = %d"), errno );
		}
	}
   pNewClient->read.ReadComplete = pListen->read.ReadComplete;
   pNewClient->psvRead = pListen->psvRead;
   pNewClient->close.CloseCallback     = pListen->close.CloseCallback;
   pNewClient->psvClose = pListen->psvClose;
   pNewClient->write.WriteComplete     = pListen->write.WriteComplete;
   pNewClient->psvWrite = pListen->psvWrite;
   pNewClient->dwFlags |= CF_CONNECTED | ( pListen->dwFlags & CF_CALLBACKTYPES );
   if( IsValid(pNewClient->Socket) )
   { // and we get one from the accept...
#ifdef _WIN32
#  ifdef USE_WSA_EVENTS
	pNewClient->event = WSACreateEvent();
	WSAEventSelect( pNewClient->Socket, pNewClient->event, FD_READ|FD_WRITE|FD_CLOSE );
#  else
      if( WSAAsyncSelect( pNewClient->Socket,g.ghWndNetwork,SOCKMSG_TCP,
                               FD_READ | FD_WRITE | FD_CLOSE))
      { // if there was a select error...
         Log(" Accept select Error");
         InternalRemoveClientEx( pNewClient, TRUE, FALSE );
         LeaveCriticalSec( &pNewClient->csLock );
         pNewClient = NULL;
      }
      else
#  endif
#else
      // yes this is an ugly transition from the above dangling
      // else...
      {
          int t = TRUE;
          ioctl( pNewClient->Socket, FIONBIO, &t );
      }
      //fcntl( pNewClient->Socket, F_SETFL, O_NONBLOCK );
#endif
      { 
          //Log( WIDE("Accepted and notifying...") );
			if( pListen->connect.ClientConnected )
			{
          	 if( pListen->dwFlags & CF_CPPCONNECT )
	             pListen->connect.CPPClientConnected( pListen->psvConnect, pNewClient );
          	 else
					 pListen->connect.ClientConnected( pListen, pNewClient );
			}

          // signal initial read.
          //Log(" Initial notifications...");
          pNewClient->dwFlags |= CF_READREADY; // may be... at least we can fail sooner...
			if( pNewClient->read.ReadComplete )
			{
          	  if( pListen->dwFlags & CF_CPPREAD )
   	           pNewClient->read.CPPReadComplete( pNewClient->psvRead, NULL, 0 );  // process read to get data already pending...
          	  else
					  pNewClient->read.ReadComplete( pNewClient, NULL, 0 );  // process read to get data already pending...
			}

          if( pNewClient->write.WriteComplete  &&
             !pNewClient->bWriteComplete )
          {
              pNewClient->bWriteComplete = TRUE;
              if( pNewClient->dwFlags & CF_CPPWRITE )
	              pNewClient->write.CPPWriteComplete( pNewClient->psvWrite );
              else
	              pNewClient->write.WriteComplete( pNewClient );
              pNewClient->bWriteComplete = FALSE;
          }
          LeaveCriticalSec( &pNewClient->csLock );
      }
   }
   else // accept failed...
   {
       InternalRemoveClientEx( pNewClient, TRUE, FALSE );
       LeaveCriticalSec( &pNewClient->csLock );
       pNewClient = NULL;
   }

   if( !pNewClient )
   {
      Log("Failed Accept...");
   }
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, CPPOpenTCPListenerAddrEx )(SOCKADDR *pAddr
																	, cppNotifyCallback NotifyCallback
																	, PTRSZVAL psvConnect )
{
	PCLIENT pListen;
	if( !pAddr )
      return NULL;
	pListen = GetFreeNetworkClient();
	if( !pListen )
	{
      lprintf( WIDE("Network has not been started.") );
		return NULL;
	}
	//	pListen->Socket = socket( *(_16*)pAddr, SOCK_STREAM, 0 );
#ifdef __WINDOWS__
	pListen->Socket = OpenSocket( ((*(_16*)pAddr) == AF_INET)?TRUE:FALSE, TRUE, FALSE );
	if( pListen->Socket == INVALID_SOCKET )
#endif
		pListen->Socket = socket( *(_16*)pAddr
										, SOCK_STREAM
										, (((*(_16*)pAddr) == AF_INET)||((*(_16*)pAddr) == AF_INET6))?IPPROTO_TCP:0 );
#ifdef LOG_SOCKET_CREATION
	lprintf( "Created new socket %d", pListen->Socket );
#endif
	pListen->dwFlags &= ~CF_UDP; // make sure this flag is clear!
	pListen->dwFlags |= CF_LISTEN;
   if( pListen->Socket == INVALID_SOCKET )
   {
      lprintf( WIDE(" Open Listen Socket Fail... %d"), errno);
      InternalRemoveClientEx( pListen, TRUE, FALSE );
		LeaveCriticalSec( &pListen->csLock );
		pListen = NULL;
      return NULL;
   }
#ifdef _WIN32
#  ifdef USE_WSA_EVENTS
	pListen->event = WSACreateEvent();
	WSAEventSelect( pListen->Socket, pListen->event, FD_ACCEPT|FD_CLOSE );
#  else
   if( WSAAsyncSelect( pListen->Socket, g.ghWndNetwork,
                       SOCKMSG_TCP, FD_ACCEPT|FD_CLOSE ) )
	{
      DebugBreak();
		Log1( WIDE("Windows AsynchSelect failed: %d"), WSAGetLastError() );
		InternalRemoveClientEx( pListen, TRUE, FALSE );
		LeaveCriticalSec( &pListen->csLock );
		return NULL;
   }
#  endif
#else
	{
		int t = TRUE;
		setsockopt( pListen->Socket, SOL_SOCKET, SO_REUSEADDR, &t, 4 );
		t = TRUE;
		ioctl( pListen->Socket, FIONBIO, &t );
	}
#endif
#ifndef _WIN32
	if( pAddr->sa_family==AF_UNIX )
		unlink( (char*)(((_16*)pAddr)+1));
#endif

   if (!pAddr || 
		 bind(pListen->Socket ,pAddr
#ifdef __WINDOWS__
			  ,sizeof(SOCKADDR)
#else
			  ,(pAddr->sa_family==AF_INET)?sizeof(struct sockaddr):110
#endif
			  ))
	{
		Log1( WIDE("Cant bind to address..:%d"), WSAGetLastError() );
		InternalRemoveClientEx( pListen, TRUE, FALSE );
		LeaveCriticalSec( &pListen->csLock );
		return NULL;
	}
	pListen->saSource = DuplicateAddress( pAddr );

	if(listen(pListen->Socket,5) == SOCKET_ERROR )
	{
		Log1( WIDE("listen(5) failed: %d"), WSAGetLastError() );
		InternalRemoveClientEx( pListen, TRUE, FALSE );
		LeaveCriticalSec( &pListen->csLock );
		return NULL;
	}
	pListen->connect.CPPClientConnected = (void(CPROC*)(PTRSZVAL,PCLIENT))NotifyCallback;
	pListen->psvConnect = psvConnect;
	pListen->dwFlags |= CF_CPPCONNECT;
	LeaveCriticalSec( &pListen->csLock );

   // make sure to schedule this socket for events (connect)
#ifdef USE_WSA_EVENTS
#ifdef LOG_NOTICES
	lprintf( "SET GLOBAL EVENT" );
#endif
	SetEvent( g.event );
#endif
#ifdef __LINUX__
	WakeThread( g.pThread );
#endif
	return pListen;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, OpenTCPListenerAddrEx )(SOCKADDR *pAddr
															, cNotifyCallback NotifyCallback )
{
	PCLIENT result = CPPOpenTCPListenerAddrEx( pAddr, (cppNotifyCallback)NotifyCallback, 0 );
   if( result )
		result->dwFlags &= ~CF_CPPCONNECT;
	return result;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, CPPOpenTCPListenerEx )(_16 wPort
                                          , cppNotifyCallback NotifyCallback
                                          , PTRSZVAL psvConnect )
{
   SOCKADDR *lpMyAddr = CreateLocal(wPort);
   PCLIENT pc = CPPOpenTCPListenerAddrEx( lpMyAddr, NotifyCallback, psvConnect );
   ReleaseAddress( lpMyAddr );
   return pc;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, OpenTCPListenerEx )(_16 wPort, cNotifyCallback NotifyCallback )
{
	PCLIENT result = CPPOpenTCPListenerEx( wPort, (cppNotifyCallback)NotifyCallback, 0 );
   if( result )
		result->dwFlags &= ~CF_CPPCONNECT;
	return result;
}

//----------------------------------------------------------------------------

static PCLIENT InternalTCPClientAddrExx(SOCKADDR *lpAddr,
													 int bCPP,
													 cppReadComplete  pReadComplete,
													 PTRSZVAL psvRead,
													 cppCloseCallback CloseCallback,
													 PTRSZVAL psvClose,
													 cppWriteComplete WriteComplete,
													 PTRSZVAL psvWrite,
													 cppConnectCallback pConnectComplete,
													 PTRSZVAL psvConnect
													)
{
   // Server's Port and Name.
	PCLIENT pResult;
	if( !lpAddr )
      return NULL;
		pResult = GetFreeNetworkClient();

   if( pResult )
	{
		// use the sockaddr to switch what type of socket this is.
#ifdef __WINDOWS__
		pResult->Socket = OpenSocket( ((*(_16*)lpAddr) == AF_INET)?TRUE:FALSE, TRUE, FALSE );
      if( pResult->Socket == INVALID_SOCKET )
#endif
			pResult->Socket=socket( *(_16*)lpAddr
										 , SOCK_STREAM
										 , (((*(_16*)lpAddr) == AF_INET)||((*(_16*)lpAddr) == AF_INET6))?IPPROTO_TCP:0 );
#ifdef LOG_SOCKET_CREATION
		lprintf( "Created new socket %d", pResult->Socket );
#endif
		if (pResult->Socket==INVALID_SOCKET)
        {
			lprintf( WIDE("Create socket failed. %d"), WSAGetLastError() );
			InternalRemoveClientEx( pResult, TRUE, FALSE );
			LeaveCriticalSec( &pResult->csLock );
			return NULL;
        }
        else
        {
            int err;
#ifdef _WIN32
#  ifdef USE_WSA_EVENTS
			pResult->event = WSACreateEvent();
			WSAEventSelect( pResult->Socket, pResult->event, FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE );
#  else
			if( WSAAsyncSelect( pResult->Socket,g.ghWndNetwork,SOCKMSG_TCP,
                                FD_READ|FD_WRITE|FD_CLOSE|FD_CONNECT) )
			{
				Log1( WIDE(" Select NewClient Fail! %d"), WSAGetLastError() );
				InternalRemoveClientEx( pResult, TRUE, FALSE );
				LeaveCriticalSec( &pResult->csLock );
				pResult = NULL;
				goto LeaveNow;
			}
#  endif
#else
            {
                int t = TRUE;
                ioctl( pResult->Socket, FIONBIO, &t );
            }
            fcntl( pResult->Socket, F_SETFL, O_NONBLOCK );
#endif
            pResult->saClient = DuplicateAddress( lpAddr );

            // set up callbacks before asynch select...
            pResult->connect.CPPThisConnected= pConnectComplete;
				pResult->psvConnect = psvConnect;
            pResult->read.CPPReadComplete    = pReadComplete;
				pResult->psvRead = psvRead;
				pResult->close.CPPCloseCallback        = CloseCallback;
				pResult->psvClose = psvClose;
				pResult->write.CPPWriteComplete        = WriteComplete;
				pResult->psvWrite = psvWrite;
            if( bCPP )
					pResult->dwFlags |= ( CF_CALLBACKTYPES );
				                  
            pResult->dwFlags |= CF_CONNECTING;
				//DumpAddr( WIDE("Connect to"), &pResult->saClient );
				if( ( err = connect( pResult->Socket, pResult->saClient
										 , SOCKADDR_LENGTH( pResult->saClient ) ) ) )
            {
                _32 dwError;
                dwError = WSAGetLastError();
                if( dwError != WSAEWOULDBLOCK 
#ifdef __UNIX__
                   && dwError != EINPROGRESS
#else
                  && dwError != WSAEINPROGRESS
#endif
                  )
                {
                    lprintf( WIDE("Connect FAIL: %d %d %") _32f, pResult->Socket, err, dwError );
			           InternalRemoveClientEx( pResult, TRUE, FALSE );
			           LeaveCriticalSec( &pResult->csLock );
                    pResult = NULL;
                    goto LeaveNow;
                }
                else
                {
                    //Log( WIDE("Pending connect has begun...") );
                }
				}
				else
				{
#ifdef VERBOSE_DEBUG
					Log( WIDE("Connected before we even get a chance to wait") );
#endif
				}


				//Log( WIDE("Leaving Client's critical section") );
				LeaveCriticalSec( &pResult->csLock ); // allow main thread to process

				// socket should now get scheduled for events, after unlocking it?
#ifdef USE_WSA_EVENTS
#ifdef LOG_NOTICES
				lprintf( "SET GLOBAL EVENT" );
#endif
				SetEvent( g.event );
#endif
#ifdef __UNIX__
				{
					//kill( (_32)(g.pThread->ThreadID), SIGHUP );
               WakeThread( g.pThread );
            }
#endif
            if( !pConnectComplete )
            {
                int Start, bProcessing = 0;
                Start = (GetTickCount()&0xFFFFFFF);
                // caller was expecting connect to block....
                while( !( pResult->dwFlags & (CF_CONNECTED|CF_CONNECTERROR) ) &&
                       ( ( (GetTickCount()&0xFFFFFFF) - Start ) < g.dwConnectTimeout ) )
                {
						 // may be this thread itself which connects...
						 if( !bProcessing )
						 {
							 if( ProcessNetworkMessages( 0 ) >= 0 )
								 bProcessing = 1;
							 else
								bProcessing = 2;
						 }
						 else
						 {
							 if( bProcessing >= 0 )
								 ProcessNetworkMessages( 0 );
						 }
						 if( bProcessing == 2 )
						 {
#ifdef LOG_NOTICES
							 lprintf( "Falling asleep 3 seconds waiting for connect." );
#endif
							 pResult->pWaiting = MakeThread();
							 WakeableSleep( 3000 );
                      pResult->pWaiting = NULL;
						 }
						 else
						 {
							 lprintf( WIDE( "Spin wait for connect" ) );
							 Relinquish();
						 }
					 }
					 if( (( (GetTickCount()&0xFFFFFFF) - Start ) >= 10000)
                    || (pResult->dwFlags &  CF_CONNECTERROR ) )
                {
                	  if( pResult->dwFlags &  CF_CONNECTERROR )
                	  {
                	  	  //DumpAddr( WIDE("Connect to: "), lpAddr );
	                    //Log( WIDE("Connect FAIL: message result") );
	                 }
	                 else
	                    Log( WIDE("Connect FAIL: Timeout") );
                    InternalRemoveClientEx( pResult, TRUE, FALSE );
				        pResult = NULL;
                    goto LeaveNow;
                }
                //Log( WIDE("Connect did complete... returning to application"));
            }
#ifdef VERBOSE_DEBUG
            else
					Log( WIDE("Connect in progress, will notify application when done.") );
#endif
      }
   }

LeaveNow:
   if( !pResult )  // didn't break out of the loop with a good return.
   {
      //Log( WIDE("Failed Open TCP Client.") );
   }   
   return pResult;
}

//----------------------------------------------------------------------------
NETWORK_PROC( PCLIENT, CPPOpenTCPClientAddrExx )(SOCKADDR *lpAddr, 
             cppReadComplete  pReadComplete,
             PTRSZVAL psvRead,
             cppCloseCallback CloseCallback,
             PTRSZVAL psvClose,
             cppWriteComplete WriteComplete,
             PTRSZVAL psvWrite,
             cppConnectCallback pConnectComplete,
             PTRSZVAL psvConnect
																)
{
	return InternalTCPClientAddrExx( lpAddr, TRUE
											 , pReadComplete, psvRead
											 , CloseCallback, psvClose
											 , WriteComplete, psvWrite
											 , pConnectComplete, psvConnect );
}

//----------------------------------------------------------------------------
NETWORK_PROC( PCLIENT, OpenTCPClientAddrExx )(SOCKADDR *lpAddr, 
             cReadComplete     pReadComplete,
             cCloseCallback    CloseCallback,
             cWriteComplete    WriteComplete,
             cConnectCallback  pConnectComplete )
{
	return InternalTCPClientAddrExx( lpAddr, FALSE
											 , (cppReadComplete)pReadComplete, 0
											 , (cppCloseCallback)CloseCallback, 0
											 , (cppWriteComplete)WriteComplete, 0
											 , (cppConnectCallback)pConnectComplete, 0 );
}
//----------------------------------------------------------------------------
NETWORK_PROC( PCLIENT, OpenTCPClientAddrEx )(SOCKADDR *lpAddr, 
             cReadComplete  pReadComplete,
             cCloseCallback CloseCallback,
             cWriteComplete WriteComplete )
{
   return OpenTCPClientAddrExx( lpAddr, pReadComplete, CloseCallback, WriteComplete, NULL );
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, CPPOpenTCPClientExx )(CTEXTSTR lpName,_16 wPort,
             cppReadComplete	 pReadComplete, PTRSZVAL psvRead,
				 cppCloseCallback CloseCallback, PTRSZVAL psvClose,
             cppWriteComplete WriteComplete, PTRSZVAL psvWrite,
             cppConnectCallback pConnectComplete, PTRSZVAL psvConnect )
{
   PCLIENT pClient;
   SOCKADDR *lpsaDest;
   pClient = NULL;
   if( lpName && 
       (lpsaDest = CreateSockAddress(lpName,wPort) ) )
   {
      pClient = CPPOpenTCPClientAddrExx( lpsaDest,
                                     pReadComplete,
                                     psvRead, 
                                     CloseCallback,
                                     psvClose, 
                                     WriteComplete,
                                     psvWrite, 
                                     pConnectComplete, 
                                     psvConnect );
      ReleaseAddress( lpsaDest );
   }   
   return pClient;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, OpenTCPClientExx )(CTEXTSTR lpName,_16 wPort,
             cReadComplete	 pReadComplete,
				 cCloseCallback CloseCallback,
             cWriteComplete WriteComplete,
             cConnectCallback pConnectComplete )
{
   PCLIENT pClient;
   SOCKADDR *lpsaDest;
   pClient = NULL;
   if( lpName && 
       (lpsaDest = CreateSockAddress(lpName,wPort) ) )
   {
      pClient = OpenTCPClientAddrExx( lpsaDest,
                                     pReadComplete,
                                     CloseCallback,
                                     WriteComplete,
                                     pConnectComplete );
      ReleaseAddress( lpsaDest );
   }   
   return pClient;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, OpenTCPClientEx)(CTEXTSTR lpName,_16 wPort,
             cReadComplete	 pReadComplete,
				 cCloseCallback CloseCallback,
             cWriteComplete WriteComplete )
{
   return OpenTCPClientExx( lpName, wPort, pReadComplete, CloseCallback, WriteComplete, NULL );
}


//----------------------------------------------------------------------------

int FinishPendingRead(PCLIENT lpClient DBG_PASS )  // only time this should be called is when there IS, cause
                                 // we definaly have already gotten SOME data to leave in
                                 // a pending state...
{
	int nRecv;
#ifdef VERBOSE_DEBUG
	int nCount;
#endif
	_32 dwError;
	// returns amount of information left to get.
#ifdef VERBOSE_DEBUG
	nCount = 0;
#endif
	if( lpClient->bDraining )
	{
		Log("LOG:ERROR trying to read during a drain state..." );
		return -1; // why error on draining with pending finish??
	}

	if( !(lpClient->dwFlags & CF_CONNECTED)  )
		return lpClient->RecvPending.dwUsed; // amount of data available...
	//lprintf( WIDE("FinishPendingRead of %d"), lpClient->RecvPending.dwAvail );
	if( !( lpClient->dwFlags & CF_READPENDING ) )
	{
      // without a pending read, don't read, the buffers are not correct.
		return 0;
	}
	while( lpClient->RecvPending.dwAvail )  // if any room is availiable.
	{
#ifdef VERBOSE_DEBUG
		nCount++;
		_lprintf( DBG_RELAY )( WIDE("FinishPendingRead %d %") _32f WIDE(""), nCount
									, lpClient->RecvPending.dwAvail );
#endif
		nRecv = recv(lpClient->Socket,
						 (char*)lpClient->RecvPending.buffer.p +
						 lpClient->RecvPending.dwUsed,
						 lpClient->RecvPending.dwAvail,0);
      if (nRecv == SOCKET_ERROR)
		{
			dwError = WSAGetLastError();
			switch( dwError)
			{
			case WSAEWOULDBLOCK: // no data avail yet...
				//Log( WIDE("Pending Receive would block...") );
				lpClient->dwFlags &= ~CF_READREADY;
				return lpClient->RecvPending.dwAvail;
#ifdef __UNIX__
			case ECONNRESET:
#else
			case WSAECONNRESET:
			case WSAECONNABORTED:
#endif
#ifdef LOG_DEBUG_CLOSING
				lprintf( WIDE("Read from reset connection - closing. %p"), lpClient );
#endif
				if(0)
				{
				default:
					Log5( WIDE("Failed reading from %d (err:%d) into %p %") _32f WIDE(" bytes %") _32f WIDE(" read already."),
						  lpClient->Socket,
						  WSAGetLastError(),
						  lpClient->RecvPending.buffer.p,
						  lpClient->RecvPending.dwAvail,
						  lpClient->RecvPending.dwUsed );
					Log1(WIDE("LOG:ERROR FinishPending discovered unhandled error (closing connection) %") _32f WIDE(""), dwError );
				}
				//InternalRemoveClient( lpClient );  // invalid channel now.
				lpClient->dwFlags |= CF_TOCLOSE;
				return -1;   // return pending finished...
			}
		}
		else if (!nRecv) // channel closed if received 0 bytes...
		{           // otherwise WSAEWOULDBLOCK would be generated.
			//_lprintf( DBG_RELAY )( WIDE("Closing closed socket... Hope there's also an event... "));
			lpClient->dwFlags |= CF_TOCLOSE;
			break; // while dwAvail... try read...
			//return -1;
		}
		else
		{
			if( lpClient->RecvPending.s.bStream )
				lpClient->dwFlags &= ~CF_READREADY;
			lpClient->RecvPending.dwLastRead = nRecv;
			lpClient->RecvPending.dwAvail -= nRecv;
			lpClient->RecvPending.dwUsed  += nRecv;
			if( lpClient->RecvPending.s.bStream &&
				lpClient->RecvPending.dwAvail )
				break;
			//else
			//	Log( WIDE("Was not a stream read - try reading more...") );
		}
	}

	// if read notification okay - then do callback.
	if( !( lpClient->dwFlags & CF_READWAITING ) )
	{
#ifdef LOG_PENDING
		lprintf( WIDE("Waiting on a queued read... result to callback.") );
#endif
		if( ( !lpClient->RecvPending.dwAvail || // completed all of the read
			  ( lpClient->RecvPending.dwUsed &&   // or completed some of the read
				lpClient->RecvPending.s.bStream ) ) )
		{
#ifdef LOG_PENDING
			Log( WIDE("Sending completed read to application") );
#endif
			lpClient->dwFlags &= ~CF_READPENDING;
			if( lpClient->read.ReadComplete )  // and there's a read complete callback available
			{
				// need to clear dwUsed...
            // otherwise the on-close notificatino can cause this to dispatch again.
				int length = lpClient->RecvPending.dwUsed;
				lpClient->RecvPending.dwUsed = 0;
            if( lpClient->dwFlags & CF_CPPREAD )
				{
					lpClient->read.CPPReadComplete( lpClient->psvRead
															, lpClient->RecvPending.buffer.p
															, length );
				}
				else
				{
					lpClient->read.ReadComplete( lpClient,
														 lpClient->RecvPending.buffer.p,
														 length );
				}
			}
			//Log( WIDE("Back from application") );
		}
	}
	else
	{
#ifdef LOG_PENDING
		lprintf( WIDE("Client is waiting for this data... should we wake him? %d"), lpClient->RecvPending.s.bStream );
#endif
		if( ( !lpClient->RecvPending.dwAvail || // completed all of the read
			  ( lpClient->RecvPending.dwUsed &&   // or completed some of the read
				lpClient->RecvPending.s.bStream ) ) )
		{
			//lprintf( WIDE("Wake waiting thread... clearing pending read flag.") );
			lpClient->dwFlags &= ~CF_READPENDING;
			if( lpClient->pWaiting )
				WakeThread( lpClient->pWaiting );
		}
	}
	if( lpClient->dwFlags & CF_TOCLOSE )
		return -1;
	return (int)lpClient->RecvPending.dwUsed; // returns amount of information which is available NOW.
}



//----------------------------------------------------------------------------

NETWORK_PROC( int, doReadExx)(PCLIENT lpClient,POINTER lpBuffer,int nBytes, LOGICAL bIsStream, LOGICAL bWait )
{
   //lprintf( "Reading ... %p(%d)", lpClient, lpClient->Socket );
   if( !lpClient || !lpBuffer )
      return 0; // nothing read.... ???
   if( TCPDrainRead( lpClient ) &&  //draining....
       lpClient->RecvPending.dwAvail ) // and already queued next read.
   {
   	Log("LOG:ERROR is draining, and NEXT pending queued ALSO...");
      return -1;  // read not queued... (ERROR)
   }

   if( !lpClient->RecvPending.s.bStream && // existing read was not a stream...
       lpClient->RecvPending.dwAvail )   // AND is not at completion...
   {
   	Log1(WIDE("LOG:ERROR was not a stream, and is not complete...%") _32f WIDE(" left "),
   					lpClient->RecvPending.dwAvail );
      return -1; // this I guess is an error since we're queueing another
                 // read on top of existing incoming 'guaranteed data'
   }
	EnterCriticalSec( &lpClient->csLock );
	if( !(lpClient->dwFlags & CF_ACTIVE ) )
	{
		LeaveCriticalSec( &lpClient->csLock );
		return -1;
	}
   if( nBytes )   // only worry if there IS data to read.
   {
      // we can assume there is nothing now pending...

      lpClient->RecvPending.buffer.p = lpBuffer;
      lpClient->RecvPending.dwAvail = nBytes;
      lpClient->RecvPending.dwUsed = 0;
      lpClient->RecvPending.s.bStream = bIsStream;

      // if the pending finishes it will call the ReadComplete Callback.
      // otherwise there will be more data to read...
		//Log( WIDE("Ok ... buffers set up - now we can handle read events") );
      //lprintf( "Setup read pending" );
		lpClient->dwFlags |= CF_READPENDING;
		if( bWait )
		{
         //lprintf( WIDE("setting read waiting so we get awoken... and callback dispatch does not happen.") );
			lpClient->dwFlags |= CF_READWAITING;
		}
		//else
      //   lprintf( WIDE("No read waiting... allow forward going...") );
		//Log2( WIDE("Setting Read buffer pending for next select...%d(%d)")
		//				, lpClient - Clients
		//				, lpClient->Socket);
#ifdef __UNIX__
		// if not unix, then the socket is already generating
		// WindowsMessage_ReadReady when there is something to
      // read...
		if( lpClient->dwFlags & CF_READREADY )
		{
			// Log( WIDE("Data already present for read...") );
         int status = FinishPendingRead( lpClient DBG_SRC );
			if( lpClient->dwFlags & CF_ACTIVE )
			{
				LeaveCriticalSec( &lpClient->csLock );
				return status; // returns bytes pending...
			}
			// else we shouldn't leave a critical section
			// of a client object which is not active...
         lprintf( WIDE("Leaving read from a bad state... adn we do not unlock.") );
         LeaveCriticalSec( &lpClient->csLock );
			return 0;
		}
		else
		{
			WakeThread( g.pThread );
		}
#endif
      //lprintf( WIDE("This missing leave sec...") );
      //LeaveCriticalSec( &lpClient->csLock );
   }
   else
	{
		if( !bWait )
		{
			if( lpClient->read.ReadComplete )  // and there's a read complete callback available
			{
            lprintf( WIDE( "Read complete with 0 bytes immediate..." ) );
				lpClient->read.ReadComplete( lpClient, lpBuffer, 0 );
			}
		}
		else
		{
         lpClient->dwFlags &= ~CF_READWAITING;
 		}
   }
	if( bWait )
	{
      int timeout = 0;
		//lprintf( WIDE("Waiting for TCP data result...") );
      /*
		if( IsThisThread( g.pThread ) )
		{
			lpClient->pWaiting = NULL;
			LeaveCriticalSec( &lpClient->csLock );
			while( lpClient->dwFlags & CF_READPENDING )
			{
            //lprintf( WIDE("Network thread itself waiting... uhh idle...") );
				Idle();
			}
         //lprintf( WIDE("Return...") );
		}
		else
      */
		{
         _32 tick = GetTickCount();
			lpClient->pWaiting = MakeThread();
			LeaveCriticalSec( &lpClient->csLock );
			while( lpClient->dwFlags & CF_READPENDING )
			{
            // wait 5 seconds, then bail.
				if( ( tick + g.dwReadTimeout ) < GetTickCount() )
				{
					//lprintf( "pending has timed out! return now." );
					timeout = 1;
               break;
				}
				if( !Idle() )
				{
               //lprintf( WIDE("Nothing significant to idle on... going to sleep forever.") );
					WakeableSleep( 1000 );
				}
			}
		}
		EnterCriticalSec( &lpClient->csLock );
		if( !(lpClient->dwFlags & CF_ACTIVE ) )
		{
			LeaveCriticalSec( &lpClient->csLock );
			return -1;
		}
 		lpClient->dwFlags &= ~CF_READWAITING;
		LeaveCriticalSec( &lpClient->csLock );
		if( timeout )
			return 0;
      else
			return lpClient->RecvPending.dwUsed;
	}
	else
		LeaveCriticalSec( &lpClient->csLock );

   return 0; // unknown result really... success prolly
}

NETWORK_PROC( int, doReadEx)(PCLIENT lpClient,POINTER lpBuffer,int nBytes, LOGICAL bIsStream)
{
   return doReadExx( lpClient, lpBuffer, nBytes, bIsStream, FALSE );
}
//----------------------------------------------------------------------------

static void PendWrite( PCLIENT pClient
							, CPOINTER lpBuffer
							, int nLen, int bLongBuffer )
{
   PendingBuffer *lpPend;
#ifdef LOG_PENDING
   {
      Log1( WIDE("Pending %d Bytes to network...") , nLen );
   }
#endif
   lpPend = New( PendingBuffer );

   lpPend->dwAvail  = nLen;
   lpPend->dwUsed   = 0;
   lpPend->lpNext   = NULL;
   if( !bLongBuffer )
   {
       lpPend->s.bDynBuffer = TRUE;
       lpPend->buffer.p = Allocate( nLen );
       MemCpy( lpPend->buffer.p, lpBuffer, nLen );
   }
   else
   {
       lpPend->s.bDynBuffer = FALSE;
       lpPend->buffer.c = lpBuffer;
   }
   if (pClient->lpLastPending)
      pClient->lpLastPending->lpNext = lpPend;
   else
      pClient->lpFirstPending = lpPend;
   pClient->lpLastPending = lpPend;
}

//----------------------------------------------------------------------------

int TCPWriteEx(PCLIENT pc DBG_PASS)
#define TCPWrite(pc) TCPWriteEx(pc DBG_SRC)
{
   int nSent;
   PendingBuffer *lpNext;
   if( !pc || !(pc->dwFlags & CF_CONNECTED ) )
   	return TRUE;

   while (pc->lpFirstPending)
	{
#ifdef VERBOSE_DEBUG
		if( pc->dwFlags & CF_CONNECTING )
			Log( WIDE("Sending previously queued data.") );
#endif

   	if( pc->lpFirstPending->dwAvail )
		{
			nSent = send(pc->Socket,
							 (char*)pc->lpFirstPending->buffer.c +
							 pc->lpFirstPending->dwUsed,
							 pc->lpFirstPending->dwAvail,
                         0);
            if (nSent == SOCKET_ERROR)
            {
                if( WSAGetLastError() == WSAEWOULDBLOCK )  // this is alright.
					 {
#ifdef VERBOSE_DEBUG
						 Log( WIDE("Pending write...") );
#endif
                    pc->dwFlags |= CF_WRITEPENDING;
#ifdef __UNIX__
                    //if( !(pc->dwFlags & CF_WRITEISPENDED ) )
                    //{
                    //	   Log( WIDE("Sending signal") );
                    //    WakeThread( g.pThread );
                    //}
                    //else
                    //    Log( WIDE("Yes... it was already pending..(no signal)") );
#endif
						  //LeaveCriticalSec( &pc->csLock );

                    return TRUE;
                }
					 {
						 _lprintf(DBG_RELAY)(WIDE(" Network Send Error: %5d(buffer:%p ofs: %") _32f WIDE("  Len: %") _32f WIDE(")"),
													WSAGetLastError(),
													pc->lpFirstPending->buffer.c,
													pc->lpFirstPending->dwUsed,
													pc->lpFirstPending->dwAvail );
						 if( WSAGetLastError() == 10057 // ENOTCONN
							 ||WSAGetLastError() == 10014 // EFAULT
#ifdef __UNIX__
							 || WSAGetLastError() == EPIPE
#endif
							)
						 {
							 InternalRemoveClient( pc );
						 }
						 //LeaveCriticalSec( &pc->csLock );
						 return FALSE; // get out of here!
					 }
	      }
      	else if (!nSent)  // other side closed.
	      {
         	Log( WIDE("sent zero bytes - assume it was closed - and HOPE there's an event...") );
				InternalRemoveClient( pc );
             // if this happened - don't return TRUE result which would
             // result in queuing a pending buffer...
   	      return FALSE;  // no sence processing the rest of this.
	      }
      }
      else
      	nSent = 0;

      {  // sent some data - update pending buffer status.
         if( pc->lpFirstPending )
         {
            pc->lpFirstPending->dwAvail -= nSent;
            pc->lpFirstPending->dwUsed  += nSent;
            if (!pc->lpFirstPending->dwAvail)  // no more to send...
            {
               lpNext = pc->lpFirstPending -> lpNext;
               if( pc->lpFirstPending->s.bDynBuffer )
                  Release(pc->lpFirstPending->buffer.p );
            // there is one pending holder in the client
            // structure that was NOT allocated...
               if( pc->lpFirstPending != &pc->FirstWritePending )
               {
#ifdef LOG_PENDING
                  {
                     Log( WIDE("Finished sending a pending buffer.") );
                  }
#endif
	               Release(pc->lpFirstPending ); 
               }
               else
               {
#ifdef LOG_PENDING
                  if(0) // this happens 99.99% of the time.
                  {
                     Log( WIDE("Normal send complete.") );
                  }
#endif
               }
               if (!lpNext)
                  pc->lpLastPending = NULL;
               pc->lpFirstPending = lpNext;

               if( pc->write.WriteComplete &&
                   !pc->bWriteComplete )
               {
            	   pc->bWriteComplete = TRUE;
            	   if( pc->dwFlags & CF_CPPWRITE )
	                  pc->write.CPPWriteComplete( pc->psvWrite );  // SOME WRITE!!!
            	   else
   	               pc->write.WriteComplete( pc );  // SOME WRITE!!!
            	   pc->bWriteComplete = FALSE;
               }
               if( !pc->lpFirstPending )
                   pc->dwFlags &= ~CF_WRITEPENDING;
            }
            else
				{
					 if( !(pc->dwFlags & CF_WRITEPENDING) )
					 {
						 pc->dwFlags |= CF_WRITEPENDING;
#ifdef USE_WSA_EVENTS
#ifdef LOG_NOTICES
						 lprintf( "SET GLOBAL EVENT" );
#endif
						 SetEvent( g.event );
#endif
#ifdef __UNIX__
						 //kill( (_32)(g.pThread->ThreadID), SIGHUP );
                   WakeThread( g.pThread );
#endif
					 }
                return TRUE;
            }
         }
      }
	}
   return FALSE; // 0 = everything sent / nothing left to send...
}

//----------------------------------------------------------------------------

NETWORK_PROC( LOGICAL, doTCPWriteExx)( PCLIENT lpClient
												 , CPOINTER pInBuffer
												 , int nInLen
												 , int bLongBuffer
												 , int failpending
												  DBG_PASS
						)
{
   if( !lpClient )
   {
#ifdef VERBOSE_DEBUG
		Log( WIDE("TCP Write failed - invalid client.") );
#endif
      return FALSE;  // cannot process a closed channel. data not sent.
   }

   EnterCriticalSec( &lpClient->csLock );
   if( !(lpClient->dwFlags & CF_ACTIVE ) )
   {
#ifdef VERBOSE_DEBUG
		Log( WIDE("TCP Write failed - client is inactive") );
#endif
       // change to inactive status by the time we got here...
       LeaveCriticalSec( &lpClient->csLock );
       return FALSE;
   }

   if( lpClient->lpFirstPending ) // will already be in a wait on network state...
   {
#ifdef VERBOSE_DEBUG
      Log2(  "%s(%d)Data pending, pending buffer... ", pFile, nLine );
#endif
     	if( !failpending )
      {
#ifdef VERBOSE_DEBUG
			 Log( WIDE("Queuing pending data anyhow...") );
#endif
          PendWrite( lpClient, pInBuffer, nInLen, bLongBuffer );
          TCPWrite( lpClient ); // make sure we don't lose a write event during the queuing...
          LeaveCriticalSec( &lpClient->csLock );
          return TRUE;
      }
      else
      {
#ifdef VERBOSE_DEBUG
          Log( WIDE("Failing pend.") );
#endif
          LeaveCriticalSec( &lpClient->csLock );
          return FALSE;
      }
   }
   else
   {
   	// have to steal the buffer - :(   

      lpClient->FirstWritePending.buffer.c   = pInBuffer;
      lpClient->FirstWritePending.dwAvail    = nInLen;
      lpClient->FirstWritePending.dwUsed     = 0;

      lpClient->FirstWritePending.s.bStream    = FALSE;
      lpClient->FirstWritePending.s.bDynBuffer = FALSE;
      lpClient->FirstWritePending.lpNext       = NULL;

      lpClient->lpLastPending = 
      lpClient->lpFirstPending = &lpClient->FirstWritePending;
      if( TCPWrite( lpClient ) )
      {
#ifdef VERBOSE_DEBUG
          Log2( WIDE("Data not sent, pending buffer... %d bytes %d remain"), nInLen, lpClient->FirstWritePending.dwAvail );
#endif
          if( !bLongBuffer ) // caller will assume the buffer usable on return
          {
              lpClient->FirstWritePending.buffer.p = Allocate( nInLen );
              MemCpy( lpClient->FirstWritePending.buffer.p, pInBuffer, nInLen );
              lpClient->FirstWritePending.s.bDynBuffer = TRUE;
          }
		}
#ifdef VERBOSE_DEBUG
		else
			_xlprintf( 1 DBG_RELAY )( WIDE("Data has been compeltely sent.") );
#endif
   }
   LeaveCriticalSec( &lpClient->csLock );   
   return TRUE; // assume the data was sent.
}

//----------------------------------------------------------------------------

#define DRAIN_MAX_READ 2048
// used internally to read directly to drain buffer.
LOGICAL TCPDrainRead( PCLIENT pClient )
{
   int nDrainRead;
   char byBuffer[DRAIN_MAX_READ];
   while( pClient && pClient->nDrainLength )
   {
      nDrainRead = pClient->nDrainLength;
      if( nDrainRead > DRAIN_MAX_READ )
         nDrainRead = DRAIN_MAX_READ;
      nDrainRead = recv( pClient->Socket, 
                         byBuffer, nDrainRead, 0 );
      if( nDrainRead == SOCKET_ERROR )
      {
         if( WSAGetLastError() == WSAEWOULDBLOCK )
         {
         	if( !pClient->bDrainExact )
         		pClient->nDrainLength = 0;
            break;
         }
         Log5(WIDE(" Network Error during drain: %d (from: %d  to: %p  has: %") _32f WIDE("  toget: %") _32f WIDE(")"),
                      WSAGetLastError(),
                      pClient->Socket,
                      pClient->RecvPending.buffer.p,
                      pClient->RecvPending.dwUsed,
                      pClient->RecvPending.dwAvail );
         InternalRemoveClient( pClient );
     	   LeaveCriticalSec( &pClient->csLock );
         return FALSE;
      }
      if( nDrainRead == 0 )
      {
         InternalRemoveClient( pClient ); // closed.
     	   LeaveCriticalSec( &pClient->csLock );
         return FALSE;
      }
      if( pClient->bDrainExact )
	      pClient->nDrainLength -= nDrainRead;
   }
   if( pClient )
      return pClient->bDraining = (pClient->nDrainLength != 0);
   return 0; // no data available....
}

//----------------------------------------------------------------------------

NETWORK_PROC( LOGICAL, TCPDrainEx)( PCLIENT pClient, int nLength, int bExact )
{
	if( pClient )
	{
		LOGICAL bytes;
		EnterCriticalSec( &pClient->csLock );
	   if( pClient->bDraining )
   	{
      	pClient->nDrainLength += nLength;
	   }
   	else
	   {
   	   pClient->bDraining = TRUE;
      	pClient->nDrainLength = nLength;
	   }
		pClient->bDrainExact = bExact;
		if( !pClient->bDrainExact )
			pClient->nDrainLength = DRAIN_MAX_READ; // default optimal read
	   bytes = TCPDrainRead( pClient );
  		LeaveCriticalSec( &pClient->csLock );
	   return bytes;
	}
	return 0;
}


//----------------------------------------------------------------------------

NETWORK_PROC( void, SetTCPNoDelay)( PCLIENT pClient, int bEnable )
{
   if( setsockopt( pClient->Socket, IPPROTO_TCP, 
                   TCP_NODELAY,
                   (const char *)&bEnable, sizeof(bEnable) ) == SOCKET_ERROR )
   {
   	Log1( WIDE("Error setting Delay to : %d"), bEnable );
      // log some sort of error... and ignore...
   }
}

//----------------------------------------------------------------------------

NETWORK_PROC( void, SetClientKeepAlive)( PCLIENT pClient, int bEnable )
{
   if( setsockopt( pClient->Socket, SOL_SOCKET, 
                   SO_KEEPALIVE,
                   (const char *)&bEnable, sizeof(bEnable) ) == SOCKET_ERROR )
   {
   	Log1( WIDE("Error setting Delay to : %d"), bEnable );
      // log some sort of error... and ignore...
   }
}
SACK_NETWORK_TCP_NAMESPACE_END

// $Log: tcpnetwork.c,v $
// Revision 1.87  2005/06/24 16:24:54  jim
// heh - don't log silly things... like now working leavecriticalsections
//
// Revision 1.86  2005/05/26 23:17:09  jim
// Added wait mode reading for TCP connections.  (Prior, deleted revision, had a problem with critical sections being left.
//
// Revision 1.86  2005/05/23 17:27:57  jim
// Added wait mode read support for TCP connections.
//
// Revision 1.85  2005/05/16 19:07:57  jim
// Modified logging to account for source of events.  Fixed wait loop to WaitMessage() under windows. instead of fixed 10ms delay
//
// Revision 1.84  2005/04/13 22:58:57  chrisg
// Changed calls to socket(addressfamily,sockettype,protocol) in TCP-related calls to actually use 0 as the protocol if the address family type is not AF_INET
//
// Revision 1.83  2005/03/22 12:42:41  panther
// Clean up some noisy logging messages.
//
// Revision 1.82  2005/03/22 10:06:34  panther
// Clean up some extra loggings that were injected.  Fix some minor windows issues.
//
// Revision 1.81  2005/03/21 20:32:06  chrisg
// Extend unix socket support.
//
// Revision 1.80  2005/03/15 20:10:48  panther
// Add support for creating unix socket addresses, then those are passable to OPenTCP to switch handling to a unix socket type instead of TCP
//
// Revision 1.79  2005/01/27 07:37:11  panther
// Linux cleaned.
//
// Revision 1.78  2004/09/29 00:49:49  d3x0r
// Added fancy wait for PSI frames which allows non-polling sleeping... Extended Idle() to result in meaningful information.
//
// Revision 1.77  2004/09/10 01:24:01  d3x0r
// With new changes to allow network re-initialzation to add user data or client space... the new global user client space was not set... also, move tcp write logging deeper.
//
// Revision 1.76  2004/07/27 23:17:09  d3x0r
// Fixup client sock address
//
// Revision 1.75  2004/07/27 10:13:05  d3x0r
// When adding a listening socket, wake the main thread to queue its events.
//
// Revision 1.74  2003/11/10 03:35:09  panther
// Fixed cpp open for text names
//
// Revision 1.73  2003/11/09 22:35:33  panther
// Remove noisy logging.  Critical Sections are stable now
//
// Revision 1.72  2003/11/08 02:37:30  panther
// Fix the CPP open and the resetting of the flags...
//
// Revision 1.71  2003/11/08 02:13:22  panther
// Fix handling NULL address
//
// Revision 1.70  2003/11/08 01:44:27  panther
// Fix error handling on server sockets
//
// Revision 1.69  2003/10/27 01:27:14  panther
// Implement semaphore on linux for perma sleep
//
// Revision 1.68  2003/10/23 15:42:54  panther
// Added some noisy logging.  Fixed on econnect
//
// Revision 1.67  2003/10/21 07:14:31  panther
// Fix log statement
//
// Revision 1.66  2003/10/21 03:53:46  panther
// Trying to track down lost event... still doesn't work..
//
// Revision 1.65  2003/09/25 08:29:29  panther
// ...New test
//
// Revision 1.64  2003/09/24 22:32:54  panther
// Update for latest type procs...
//
// Revision 1.63  2003/09/24 15:10:54  panther
// Much mangling to extend C++ network interface...
//
// Revision 1.62  2003/07/29 09:27:14  panther
// Add Keep Alive option, enable use on proxy
//
// Revision 1.61  2003/07/27 14:21:25  panther
// Wrap else in ifdef logging
//
// Revision 1.60  2003/07/25 15:06:09  panther
// Remove noisy logging.  Revert thread ID change
//
// Revision 1.59  2003/06/04 11:39:31  panther
// Stripped carriage returns
//
// Revision 1.58  2003/05/19 07:23:27  panther
// move setting of dynamic buffer down.
//
// Revision 1.57  2003/04/21 20:04:32  panther
// Handle abortion due to WSAEFAULT
//
// Revision 1.56  2003/04/18 09:28:04  panther
// Revert ProcessMessagesEx
//
// Revision 1.55  2003/04/18 09:26:04  panther
// Fixed network wake issues associated with bad wake threads
//
// Revision 1.54  2003/04/18 07:05:01  panther
// Allow a dispatched timer to delete other timers....
//
// Revision 1.53  2003/02/13 09:52:54  panther
// Oops - return from TCPWrite when not connected was WRONG
//
// Revision 1.52  2003/02/13 08:51:41  panther
// Logging changes, changes to proxy so back connect works
//
// Revision 1.51  2003/02/13 08:27:09  panther
// Tweaked proxy, added a log message to TCPAccept
//
// Revision 1.50  2003/01/31 17:28:43  panther
// Use standard WakeThread/WakableSleep on Unix
//
// Revision 1.49  2002/12/22 00:14:11  panther
// Cleanup function declarations and project defines.
//
// Revision 1.48  2002/12/02 12:55:53  panther
// Ug - one day we'll actuall get these changes in for network
// server method inheritance...
//
// Revision 1.47  2002/11/15 09:03:43  panther
// Updated timers to export wakeablesleep/wakethread
// Updated TCP network to cause new clients to inherit server methods.
// Updated makecleanset to work, display to build with minor mods to
// interfaces.  Control library-call base functions instead of ActiveImg_*
//
// Revision 1.46  2002/09/26 09:16:11  panther
// Minor mods to add Linux compat.  Minor mods to allow the LITERAL name
// of a library to be specified.  Mod to log the address being connected to.
//
// Revision 1.45  2002/08/01 13:09:06  panther
// Modified logging in TCPnetwork.
// Check for NULL pclient in UNLOCK!
//
// Revision 1.44  2002/07/23 11:23:43  panther
// New function option to tcp write - the ability to NOT pend outgoing data,
// and therefore return failure.
// UDP Changes? (fix log entry)
//
// Revision 1.43  2002/07/17 11:32:55  panther
// Logging changes in UDP network so we can figure out this ICMP failure.
// TCP network new option on TCPWrite - do not allow pending buffers.
//
// Revision 1.42  2002/05/22 15:04:44  panther
// Changes additions and stuff... mostly linux compat
//
// Revision 1.41  2002/04/29 15:37:02  panther
// Removed commented typedefs from sack_types.h, modified error path for connect
// especially on windows side.
//
// Revision 1.40  2002/04/22 22:32:44  panther
// Modified Unix logging... fairly quiet except exceptions... still have a long
// delay for a transaction unix->windows not sure where still.
//
// Revision 1.39  2002/04/22 22:08:06  panther
// Verbatim pending write logs...
//
// Revision 1.38  2002/04/21 22:03:14  panther
// Minor changes to enter/leave critical sections...
//
// Revision 1.37  2002/04/19 22:35:02  panther
// Forgot some leave cricital sections on read/write errors...
//
// Revision 1.36  2002/04/19 21:37:06  panther
// Missed a rollback on the udpnetwork.c to use windows asynch select...
// Added much more logging on udp path for errors and information.
//
// Revision 1.35  2002/04/19 20:53:53  panther
// Added bBlockNotify to all InternalRemoveClients for TCP on connect, listen,
// accept path.  This led (at some points) to getting close notice on
// incompletely opened clients.
//
// Revision 1.34  2002/04/19 20:46:22  panther
// Modified internal usage to InternalRemoveClient, extern (application) will
// immediately close a client.  Erased usage of internal connection count.
// Test proxy success on both delayed and immediate connect.
//
// Revision 1.33  2002/04/19 18:09:51  panther
// Unix cleanup of massive changes....
//
// Revision 1.32  2002/04/19 17:42:07  panther
// Sweeping changes added queues of sockets in various states, closes
// rehang the client into a closed structure which is then timed out...
// Added event timers, cleaned up global code into global structure...
//
// Revision 1.31  2002/04/18 23:59:30  panther
// Okay - so we didn't flag it right to set sockets non blocking for windows...
// but NOW we're using ioctl(FIONBLK) and all is good?
//
// Revision 1.30  2002/04/18 23:25:45  panther
// Well - let's go towards the unix-y select thing... Seems to work so far
// and we won't have queue'd events clogging up things, plus we won't need
// a blank window....
//
// Revision 1.29  2002/04/18 20:38:13  panther
// Remove some redundant reads if _WIN32 - no auto finishpending...
// Otherwise we had many empty FD_READ operations... This also caused
// the FD_READ ops to be queued pending process, and the FD_CLOSE was
// queued way after... This SEEMS to be stable.
//
// Revision 1.28  2002/04/18 20:25:47  panther
// Begin using client local critical section locks (buggy)
// Still fighting double-remove problems...
//
// Revision 1.27  2002/04/18 17:26:02  panther
// Removed excess debugging messages - extended timer for auto scanning closed
// and pending clients...
//
// Revision 1.26  2002/04/18 15:01:42  panther
// Added Log History
// Stage 1: Delay schedule closes - only network thread will close.
//

