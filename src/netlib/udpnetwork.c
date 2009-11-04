#define LIBRARY_DEF

#ifdef __LINUX__
#include <sys/ioctl.h>
#include <signal.h> // SIGHUP defined

#define NetWakeSignal SIGHUP
#else
#define ioctl ioctlsocket
#endif
#include "netstruc.h"
#include <network.h>
#include <sharemem.h>
#include <deadstart.h>
#include <procreg.h>
//#define NO_LOGGING // force neverlog....
#define DO_LOGGING
#include "logging.h"


SACK_NETWORK_NAMESPACE

void DumpSocket( PCLIENT pc );

_UDP_NAMESPACE

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


NETWORK_PROC( PCLIENT, CPPServeUDPAddr )( SOCKADDR *pAddr
                  , cReadCompleteEx pReadComplete
                  , PTRSZVAL psvRead
                  , cCloseCallback Close
													 , PTRSZVAL psvClose
													 , int bCPP)
{
	PCLIENT pc;
   // open a UDP Port to listen for Pings for server...

	pc = GetFreeNetworkClient();
   if( !pc )
   {
	   Log( WIDE("Network Resource Fail"));
     	return NULL;
   }

#ifdef __WINDOWS__
	pc->Socket = OpenSocket(TRUE/*v4*/,FALSE, FALSE);
	if( pc->Socket == INVALID_SOCKET )
#endif
		pc->Socket = socket(PF_INET,SOCK_DGRAM,0);
   if( pc->Socket == INVALID_SOCKET )
   {
      Log( WIDE("UDP Socket Fail") );
      InternalRemoveClientEx( pc, TRUE, FALSE );
      LeaveCriticalSec( &pc->csLock );
      return NULL;
	}
#ifdef LOG_SOCKET_CREATION
	lprintf( "Created UDP %p(%d)", pc, pc->Socket );
#endif
   pc->dwFlags |= CF_UDP;

   if( pAddr? pc->saSource = *pAddr,1:0 )
   {
#ifdef LOG_SOCKET_CREATION
      Log8( WIDE(" %03d,%03d,%03d,%03d,%03d,%03d,%03d,%03d"),
       				*(((unsigned char *)pAddr)+0),
       				*(((unsigned char *)pAddr)+1),
       				*(((unsigned char *)pAddr)+2),
       				*(((unsigned char *)pAddr)+3),
       				*(((unsigned char *)pAddr)+4),
       				*(((unsigned char *)pAddr)+5),
       				*(((unsigned char *)pAddr)+6),
       				*(((unsigned char *)pAddr)+7) );
#endif
      if (bind(pc->Socket,&pc->saSource,sizeof(SOCKADDR)))
      {
         Log3( WIDE("%s(%d): Bind Fail: %d"), __FILE__, __LINE__, WSAGetLastError() );
	      InternalRemoveClientEx( pc, TRUE, FALSE );
   	   LeaveCriticalSec( &pc->csLock );
	      return NULL;
      }
   }
   else
   {
   	 Log( WIDE("Bind Will Fail") );
       InternalRemoveClientEx( pc, TRUE, FALSE );
       LeaveCriticalSec( &pc->csLock );
       return NULL;
   }

#ifdef _WIN32
   if (WSAAsyncSelect( pc->Socket, 
                       g.ghWndNetwork,
                       SOCKMSG_UDP, 
                       FD_READ|FD_WRITE ) )
   {
      Log( WIDE("Select Fail"));
      InternalRemoveClientEx( pc, TRUE, FALSE );
      LeaveCriticalSec( &pc->csLock );
      return NULL;
   }
#else
   {
      int t = TRUE;
      ioctl( pc->Socket, FIONBIO, &t );
   }
#endif
   pc->read.ReadCompleteEx = pReadComplete;
   pc->psvRead = psvRead;
   pc->close.CloseCallback = Close;
	pc->psvClose = psvClose;
   if( bCPP )
		pc->dwFlags |= (CF_CPPREAD|CF_CPPCLOSE );
	if( pReadComplete )
	{
   	if( pc->dwFlags & CF_CPPREAD )
	      pc->read.CPPReadCompleteEx( pc->psvRead, NULL, 0, &pc->saSource );
		else
			pc->read.ReadCompleteEx( pc, NULL, 0, &pc->saSource );
	}

#ifdef LOG_SOCKET_CREATION
	DumpSocket( pc );
#endif
   LeaveCriticalSec( &pc->csLock );
   return pc;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, ServeUDPAddr )( SOCKADDR *pAddr, 
                  cReadCompleteEx pReadComplete,
                  cCloseCallback Close)
{
	PCLIENT result = CPPServeUDPAddr( pAddr, pReadComplete, 0, Close, 0, FALSE );
   if( result )
		result->dwFlags &= ~(CF_CPPREAD|CF_CPPCLOSE);
	return result;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, ServeUDP )( CTEXTSTR pAddr, _16 wPort,
                  cReadCompleteEx pReadComplete,
                  cCloseCallback Close)
{
   SOCKADDR *lpMyAddr;

   if( pAddr )
      lpMyAddr = CreateSockAddress( pAddr, wPort);
   else
   {
      // NOTE this is NOT create local which binds to only 
      // one address and port - this IS "0.0.0.0" which is
      // any IP inteface on the box...
      lpMyAddr = CreateSockAddress( WIDE("0.0.0.0"), wPort ); // assume bind to any address...
   }

   return ServeUDPAddr( lpMyAddr, pReadComplete, Close );
}

//----------------------------------------------------------------------------
NETWORK_PROC( void, UDPEnableBroadcast)( PCLIENT pc, int bEnable )
{
   if( pc )
      if( setsockopt( pc->Socket, SOL_SOCKET
                  , SO_BROADCAST, (char*)&bEnable, sizeof( bEnable ) ) )
		{
			Log( WIDE("Failed to set sock opt - BROADCAST") );
		}
}

//----------------------------------------------------------------------------

LOGICAL GuaranteeAddr( PCLIENT pc, SOCKADDR *sa )
{
   int broadcast;

   if( sa )
   {
      pc->saClient=*sa;
      if( *(int*)(pc->saClient.sa_data+2) == -1 )
         broadcast = TRUE;
      else
			broadcast = FALSE;
#ifdef VERBOSE_DEBUG
		if( broadcast )
			Log( WIDE("Setting socket to broadcast!") );
		else
			Log( WIDE("Setting socket as not broadcast!") );
#endif
      UDPEnableBroadcast( pc, broadcast );
      //if( setsockopt( pc->Socket, SOL_SOCKET
      //            , SO_BROADCAST, (char*)&broadcast, sizeof( broadcast) ) )
		//{
		//	Log( WIDE("Failed to set sock opt - BROADCAST") );
		//}

      // if we don't connect we can use SendTo ..(95 limit only)
//      if( connect( pc->Socket, lpMyAddr, sizeof(SOCKADDR ) ) )
//      {
//         ReleaseAddress( lpMyAddr );
//   	   Log(" Connect on UDP Fail..." );
//   	   return FALSE;
//      }
   }
   else
      return FALSE;

#ifdef VERBOSE_DEBUG
	DumpSocket( pc );
#endif
   return TRUE;
}

//----------------------------------------------------------------------------

LOGICAL Guarantee( PCLIENT pc, CTEXTSTR pAddr, _16 wPort )
{
	SOCKADDR *lpMyAddr = CreateSockAddress( pAddr, wPort);
   int res = GuaranteeAddr( pc, lpMyAddr );
   ReleaseAddress( lpMyAddr );
   return res;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, CPPConnectUDPAddr)( SOCKADDR *sa, 
                        SOCKADDR *saTo, 
                    cReadCompleteEx pReadComplete,
                    PTRSZVAL psvRead, 
                    cCloseCallback Close,
                    PTRSZVAL psvClose )
{
	PCLIENT pc;
   int bFixed = FALSE;
   if( !sa )
   {
      sa = CreateLocal( 0 ); // use any address...
      bFixed = TRUE;
   }
   pc = ServeUDPAddr( sa, NULL, NULL );
   if( !pc )
   {
      Log( WIDE("Failed to establish incoming side of UDP Socket") );
      return NULL;
   }
   if( !GuaranteeAddr( pc, saTo ) )
   {
      Log( WIDE("Failed to set guaranteed UDP Send Address."));
      InternalRemoveClient( pc );
      return NULL;
   }

   if( bFixed )
      ReleaseAddress( sa );

   pc->read.ReadCompleteEx = pReadComplete;
   pc->psvRead = psvRead;
   pc->close.CloseCallback = Close;
   pc->psvClose = psvClose;
	pc->dwFlags |= (CF_CPPREAD|CF_CPPCLOSE );

   if( pReadComplete )
      pReadComplete( pc, NULL, 0, NULL ); // allow server to start a read method...

   return pc;
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, ConnectUDPAddr)( SOCKADDR *sa, 
                        SOCKADDR *saTo, 
                    cReadCompleteEx pReadComplete,
                    cCloseCallback Close )
{
	PCLIENT result = CPPConnectUDPAddr( sa, saTo, pReadComplete, 0, Close, 0 );
   if( result )
		result->dwFlags &= ~( CF_CPPREAD|CF_CPPCLOSE );
	return result;	
}

//----------------------------------------------------------------------------

NETWORK_PROC( PCLIENT, CPPConnectUDP )( CTEXTSTR pFromAddr, _16 wFromPort,
                    CTEXTSTR pToAddr, _16 wToPort,
                    cReadCompleteEx pReadComplete,
                    PTRSZVAL psvRead, 
                    cCloseCallback Close,
                    PTRSZVAL psvClose )
{
	PCLIENT pc;

   pc = ServeUDP( pFromAddr, wFromPort, NULL, NULL );
   if( !pc )
   {
      Log( WIDE("Failed to establish incoming side of UDP Socket") );
      return NULL;
   }
   if( !Guarantee( pc, pToAddr, wToPort ) )
   {
      Log( WIDE("Failed to set guaranteed UDP Send Address."));
      InternalRemoveClient( pc );
      return NULL;
   }

   pc->read.ReadCompleteEx = pReadComplete;
   pc->psvRead = psvRead;
   pc->close.CloseCallback = Close;
   pc->psvClose = psvClose;
	pc->dwFlags |= (CF_CPPREAD|CF_CPPCLOSE );
   if( pReadComplete )
      pReadComplete( pc, NULL, 0, NULL ); // allow server to start a read method...

   return pc;
}

NETWORK_PROC( PCLIENT, ConnectUDP )( CTEXTSTR pFromAddr, _16 wFromPort,
                    CTEXTSTR pToAddr, _16 wToPort,
                    cReadCompleteEx pReadComplete,
                    cCloseCallback Close )
{
	PCLIENT result = CPPConnectUDP( pFromAddr, wFromPort, pToAddr, wToPort
											, pReadComplete, 0, Close, 0 );
   if( result )
		result->dwFlags &= ~( CF_CPPREAD|CF_CPPCLOSE );
	return result;
	
}
//----------------------------------------------------------------------------

NETWORK_PROC( LOGICAL, SendUDPEx )( PCLIENT pc, CPOINTER pBuf, int nSize, SOCKADDR *sa )
{
   int nSent;
   if( !sa)
		sa = &pc->saClient;
	if( !pc )
      return FALSE;
   /*
   Log8( WIDE("SendUDP: Sendto %03d,%03d,%03d,%03d,%03d,%03d,%03d,%03d"),
       				*(((unsigned char *)sa)+0),
       				*(((unsigned char *)sa)+1),
       				*(((unsigned char *)sa)+2),
       				*(((unsigned char *)sa)+3),
       				*(((unsigned char *)sa)+4),
       				*(((unsigned char *)sa)+5),
       				*(((unsigned char *)sa)+6),
						*(((unsigned char *)sa)+7) );
   */
   nSent = sendto( pc->Socket,
                   (const char*)pBuf,
                   nSize,
                   0
                   ,(sa)?sa:&pc->saClient
                   ,sizeof( SOCKADDR ) 
                   );
   if( nSent < 0 )
   {
      Log1( WIDE("SendUDP: Error (%d)"), WSAGetLastError() );
      return FALSE;
   }
   else
      if( nSent < nSize ) // this is all so very vague.....
      {
      	Log( WIDE("SendUDP: Small send :(") );
         return FALSE;
      }
   return TRUE;
}


//----------------------------------------------------------------------------

NETWORK_PROC( LOGICAL, ReconnectUDP )( PCLIENT pc, CTEXTSTR pToAddr, _16 wPort )
{
   return Guarantee( pc, pToAddr, wPort );
}

//----------------------------------------------------------------------------
extern _32 uNetworkPauseTimer,
           uTCPPendingTimer;

//----------------------------------------------------------------------------

NETWORK_PROC( int, doUDPRead )( PCLIENT pc, POINTER lpBuffer, int nBytes )
{
	if( pc->RecvPending.dwAvail )
	{
		lprintf( "Read already pending for %d... not doing anything for this one.."
              , pc->RecvPending.dwAvail );
		return FALSE;
	}
   //Log1( WIDE("UDPRead Pending:%d bytes"), nBytes );
   pc->RecvPending.dwAvail = nBytes;
   pc->RecvPending.dwUsed = 0;
	pc->RecvPending.buffer.p = lpBuffer;
	{
		pc->dwFlags |= CF_READPENDING;
#ifdef __UNIX__
		{
			WakeThread( g.pThread );
		}
#endif
	}
   //FinishUDPRead( pc );  // do actual read.... (results in read callback)
   return TRUE;
}

//----------------------------------------------------------------------------

int FinishUDPRead( PCLIENT pc )
{  // all UDP Reads return the address of the other side's message...
	int nReturn;
#ifdef __LINUX__
	socklen_t
#else
		int
#endif
		Size=sizeof(SOCKADDR);  // echoed from server.

   if( !pc->RecvPending.buffer.p || !pc->RecvPending.dwAvail )  
   {
		lprintf( WIDE("UDP Read without queued buffer for result.") );
		return FALSE;
	}
	//do{
   nReturn = recvfrom( pc->Socket, 
                       (char*)pc->RecvPending.buffer.p,
                       pc->RecvPending.dwAvail,0,
                       &pc->saLastClient,
                       &Size);// get address...
	//lprintf( WIDE("Recvfrom result:%d"), nReturn );

   if (nReturn == SOCKET_ERROR)
   {
      _32 dwErr = WSAGetLastError();
      // shutdown the udp socket... not needed...???
		//lprintf( WIDE("Recvfrom result:%d"), dwErr );
      switch( dwErr )
      {
      case WSAEWOULDBLOCK: // NO data returned....
          pc->dwFlags |= CF_READPENDING;
#ifdef __UNIX__
          {
              WakeThread( g.pThread );
          }
#endif
          return TRUE;
#ifdef _WIN32
      // this happens on WIN2K/XP - ICMP Port Unreachable (nothing listening there)
      case WSAECONNRESET: // just ignore this error.
      	Log( WIDE("ICMP Port unreachable on previous send.") );
        return TRUE;
#endif
      default:
      	Log2( WIDE("FinishUDPRead Unknown error: %d %") _32f WIDE(""), WSAGetLastError(), pc->RecvPending.dwAvail );
         InternalRemoveClient( pc );
         return FALSE;
         break;
      }

   }
   //Log1( WIDE("UDPRead:%d bytes"), nReturn );
   pc->dwFlags &= ~CF_READPENDING;
   pc->RecvPending.dwAvail = 0;  // allow further reads...
   pc->RecvPending.dwUsed += nReturn;

   if( pc->read.ReadCompleteEx )
   {
   	if( pc->dwFlags & CF_CPPREAD )
	      pc->read.CPPReadCompleteEx( pc->psvRead, pc->RecvPending.buffer.p, nReturn, &pc->saLastClient );
		else
		{
         //lprintf( WIDE("Calling UDP complete %p %p %d"), pc, pc->RecvPending.buffer.p, nReturn );
			pc->read.ReadCompleteEx( pc, pc->RecvPending.buffer.p, nReturn, &pc->saLastClient );
		}
	}
	//}while(1);
   return TRUE;
}
_UDP_NAMESPACE_END
SACK_NETWORK_NAMESPACE_END

//----------------------------------------------------------------------------
// $Log: udpnetwork.c,v $
// Revision 1.30  2005/05/13 21:17:39  jim
// Fix address building.
//
// Revision 1.29  2004/07/19 06:44:27  d3x0r
// Fix UDP receive under windows... and streamline linux
//
// Revision 1.28  2004/07/18 10:54:09  d3x0r
// Remove useless message from udp read complete.
//
// Revision 1.27  2003/11/11 14:37:12  panther
// Handle failure to open udpserver
//
// Revision 1.26  2003/10/17 00:56:05  panther
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
// Revision 1.25  2003/09/24 22:32:54  panther
// Update for latest type procs...
//
// Revision 1.24  2003/09/24 15:10:54  panther
// Much mangling to extend C++ network interface...
//
// Revision 1.23  2003/06/04 11:39:31  panther
// Stripped carriage returns
//
// Revision 1.22  2003/01/31 17:28:43  panther
// Use standard WakeThread/WakableSleep on Unix
//
// Revision 1.21  2002/12/22 00:14:11  panther
// Cleanup function declarations and project defines.
//
// Revision 1.20  2002/07/23 11:26:26  panther
// CVS Log message change.
//
// Revision 1.19  2002/07/23 11:23:43  panther
// New function option to tcp write - the ability to NOT pend outgoing data,
// and therefore return failure.
// UDP Changes? IFDEF out ICMP failure error handling if not win32.
//
// Revision 1.18  2002/07/17 11:32:55  panther
// Logging changes in UDP network so we can figure out this ICMP failure.
// TCP network new option on TCPWrite - do not allow pending buffers.
//
// Revision 1.17  2002/07/15 08:39:22  panther
// Added function to enable/disable UDP Broadcast on the fly.  Use this common
// now.
//
// Revision 1.16  2002/04/19 22:35:02  panther
// Forgot some leave cricital sections on read/write errors...
//
// Revision 1.15  2002/04/19 21:37:06  panther
// Missed a rollback on the udpnetwork.c to use windows asynch select...
// Added much more logging on udp path for errors and information.
//
// Revision 1.14  2002/04/19 20:46:22  panther
// Modified internal usage to InternalRemoveClient, extern (application) will
// immediately close a client.  Erased usage of internal connection count.
// Test proxy success on both delayed and immediate connect.
//
// Revision 1.13  2002/04/19 17:42:07  panther
// Sweeping changes added queues of sockets in various states, closes
// rehang the client into a closed structure which is then timed out...
// Added event timers, cleaned up global code into global structure...
//
// Revision 1.12  2002/04/18 23:59:30  panther
// Okay - so we didn't flag it right to set sockets non blocking for windows...
// but NOW we're using ioctl(FIONBLK) and all is good?
//
// Revision 1.11  2002/04/18 20:25:47  panther
// Begin using client local critical section locks (buggy)
// Still fighting double-remove problems...
//
// Revision 1.10  2002/04/18 15:01:42  panther
// Added Log History
// Stage 1: Delay schedule closes - only network thread will close.
//
