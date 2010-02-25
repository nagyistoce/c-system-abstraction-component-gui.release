//
// PING.C -- Ping program using ICMP and RAW Sockets
//
//#ifndef __LINUX__

#define LIBRARY_DEF
#include <stdhdrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sharemem.h> // lockedincrement, lockeddecrement.
#include <timers.h>

#include <network.h>
#include "ping.h"

#ifdef __UNIX__
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// included not for pclient structure... but for OpenSocket() definition.
#include "netstruc.h"

SACK_NETWORK_NAMESPACE

// Internal Functions
void Ping(TEXTSTR pstrHost, int maxTTL);
TEXTSTR ReportError(TEXTSTR pInto, TEXTSTR pstrFrom);
int  WaitForEchoReply(SOCKET s, _32 dwTime);
u_short in_cksum(u_short *addr, int len);

// ICMP Echo Request/Reply functions
int		SendEchoRequest(TEXTSTR, SOCKET, SOCKADDR_IN*);
int   	RecvEchoReply( TEXTSTR, SOCKET, SOCKADDR_IN*, u_char *);

#define MAX_HOPS     128 
#define MAX_NAME_LEN 255
typedef struct HopEntry_tag{
   _32 dwIP;                 // IP from returned
   _32 dwMinTime;
   _32 dwMaxTime;
   _32 dwAvgTime;
   _32 dwDropped;
//   _32 dwTime;
   TEXTCHAR  pName[MAX_NAME_LEN];  // bRDNS resulting...
   int TTL;                    // returned TTL from destination...
} HOPENT, *PHOPENT;

_32 dwThreadsActive;

PTRSZVAL CPROC RDNSThread( PTHREAD pThread )
{
   PHOPENT pHopEnt = (PHOPENT)GetThreadParam( pThread );
   struct hostent *phe;

   phe = gethostbyaddr( (char*)&pHopEnt->dwIP, 4, AF_INET );
   if( phe )
      strcpy( pHopEnt->pName,phe->h_name );

   LockedDecrement( &dwThreadsActive );
   /*
#ifdef _WIN32
   ExitThread( dwThreadsActive );
#else
#endif
   */
   return 0;
}

// Ping()
// Calls SendEchoRequest() and
// RecvEchoReply() and prints results


static LOGICAL DoPingExx( CTEXTSTR pstrHost
								,int maxTTL
								,_32 dwTime
								,int nCount
								,TEXTSTR pResult
								,LOGICAL bRDNS
								,void (*ResultCallback)( _32 dwIP, CTEXTSTR name, int min, int max, int avg, int drop, int hops )
								,void (*ResultCallbackEx)( PTRSZVAL psv, _32 dwIP, CTEXTSTR name, int min, int max, int avg, int drop, int hops )
								, PTRSZVAL psvUser )
{
	SOCKET	  rawSocket;
	struct hostent *lpHost;
	SOCKADDR_IN saDest;
	SOCKADDR_IN saSrc;
	_64	     dwTimeSent;
	u_char     cTTL;
	int        nLoop;
	int        nRet, nResult = 0;
	int        i;
	_64      MinTime, MaxTime, AvgTime;
	_32   Dropped;

	_32     dwIP,_dwIP;
   static CRITICALSECTION cs;
   static  HOPENT    Entry[MAX_HOPS];
   static  int       nEntry = 0;
   char     *pResultStart;
   pResultStart = pResult;

   if( maxTTL < 0 )
   {
       if( pResult )
           sprintf( pResult, WIDE("TTL Parameter Error ( <0 )\n") );
       return 0;
   }

   if( nCount < 0 )
   {
       if( pResult )
           sprintf( pResult, WIDE("Count Parameter Error ( <0 )\n") );
       return 0;
   }

   if( maxTTL > MAX_HOPS ) 
       maxTTL = MAX_HOPS;

   if( maxTTL )
   {
       if( nCount > 10 ) // limit hit count on tracert....
           nCount = 10;
   }

   // Lookup host
#ifdef __LINUX__
   if( !inet_aton( pstrHost, (struct in_addr*)&dwIP ) )
#else
   dwIP = inet_addr( pstrHost );
   if( dwIP == INADDR_NONE )
#endif
   {
       if( pResult )
           pResult += sprintf( pResult, WIDE("host was not numeric\n") );
       lpHost = gethostbyname(pstrHost);
       if (lpHost)
       {
           dwIP = *((u_long *) lpHost->h_addr);
       }
       else
       {
           if( pResult )
               sprintf( pResult, WIDE("(1)Host does not exist.(%s)\n"), pstrHost );
       }
   }

   if( dwIP == 0xFFFFFFFF )
   {
       if( pResult )
           sprintf( pResult, WIDE("Host does not exist.(%s)\n"), pstrHost );
       return 0;
   }
   nEntry = 0;
   // Create a Raw socket

#ifdef __WINDOWS__
	rawSocket = INVALID_SOCKET;//OpenSocket(TRUE,FALSE, TRUE);
	if( rawSocket == INVALID_SOCKET )
	{
      //lprintf( "Bad 'smart' open.. fallback..." );
		rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	}
#else
	rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
#endif
   if (rawSocket == SOCKET_ERROR)
	{
      if( pResult )
			pResult += sprintf( pResult, WIDE("Uhmm bad things happened for sockraw!\n") );
		else
			lprintf( WIDE("Uhmm bad things happened for sockraw!\n") );
		rawSocket = OpenSocket( TRUE, FALSE, TRUE );
		if( rawSocket == SOCKET_ERROR )
		{
       if( WSAGetLastError() == 10013 )
       {
           if( pResult )
               sprintf( pResult, WIDE("User is not an administrator, cannot create a RAW socket.\n")
                        WIDE("Unable to override this.\n"));
           return FALSE;
       }
       else
       {
           if( pResult )
               pResult = ReportError( pResult, WIDE("socket()"));
       }
		 return FALSE;
		}
   }

   // Setup destination socket address
   saDest.sin_addr.s_addr = dwIP;
   saDest.sin_family = AF_INET;
   saDest.sin_port = 0;

   //pResult += sprintf( pResult, WIDE("Version 1.0   ADA Software Developers, Inc.  Copyright 1999.\n") );
   // Tell the user what we're doing
   if( pResult )
   {
       pResult += sprintf( pResult, WIDE("Pinging %s [%s] with %d bytes of data:\n"),
                           pstrHost,
                           inet_ntoa(*(struct in_addr*)&saDest.sin_addr),
                           REQ_DATASIZE);

       if( maxTTL )
       {
           pResult += sprintf( pResult, WIDE("Hop  Size Min(ms) Max(ms) Avg(ms) Drop Hops? IP              Name\n") );
           pResult += sprintf( pResult, WIDE("--- ----- ------- ------- ------- ---- ----- --------------- -------->\n") );
       }
       else
       {
           pResult += sprintf( pResult, WIDE("Size  Min(ms) Max(ms) Avg(ms) Drop Hops? IP              Name\n") );
           pResult += sprintf( pResult, WIDE("----- ------- ------- ------- ---- ----- --------------- -------->\n") );
       }
   }
	EnterCriticalSec( &cs );

	// Ping multiple times
   _dwIP = 0;
   for( i = 0; i <= maxTTL && dwIP != _dwIP; i++ )
   {
      
      if( maxTTL )
      {
         if( !i )       // skip TTL 0...
            continue;
#ifndef IP_TTL
#if defined (__LINUX__)
#define IP_TTL 2
#else
#define IP_TTL	7
#endif
#endif
         setsockopt( rawSocket, IPPROTO_IP, IP_TTL, (const char*)&i, sizeof(int));
      }

      MinTime = (_64)-1; // longer than EVER
      MaxTime = 0;
      AvgTime = 0;
      Dropped = 0;

      for(nLoop = 0; nLoop < nCount; nLoop++) 
      {
         // Send ICMP echo request
         dwTimeSent = GetCPUTick();
         if( SendEchoRequest(pResult, rawSocket, &saDest) <= 0)
         {
            closesocket( rawSocket );
				LeaveCriticalSec( &cs );
            return FALSE; // failed on send
         }

         nRet = WaitForEchoReply(rawSocket, dwTime);
         if (nRet == SOCKET_ERROR)
         {
             if( pResult )
                 pResult = ReportError( pResult, WIDE("select()"));
             goto LoopBreakpoint;  // abort abort FAIL
         }
         else if (!nRet)
         {
            //AvgTime += dwTime;
            Dropped++; 
         }
         else
         {
            _64 dwTimeNow;
            dwTimeNow = GetCPUTick() - dwTimeSent;
            AvgTime += dwTimeNow;

            if( dwTimeNow > MaxTime )
               MaxTime = dwTimeNow;

            if( dwTimeNow < MinTime )
               MinTime = dwTimeNow;

            if( !RecvEchoReply( pResult, rawSocket, &saSrc, &cTTL) )
            {
                if( pResult )
                    pResult = ReportError( pResult, WIDE("recv()") );
                pResult = pResultStart;
                closesocket( rawSocket );
                LeaveCriticalSec( &cs );
                return FALSE;
            }
         }
      }      
      if( MinTime > MaxTime ) // no responces....
      {
         MinTime = 0;
         MaxTime = 0;
         Entry[nEntry].dwIP = 0;
      }
      else
      {
         Entry[nEntry].dwIP = _dwIP = saSrc.sin_addr.s_addr;
      }

      Entry[nEntry].TTL  = cTTL;
      Entry[nEntry].dwMaxTime = ConvertTickToMicrosecond( MaxTime );
      Entry[nEntry].dwMinTime = ConvertTickToMicrosecond( MinTime );
      if( nCount - Dropped )
          Entry[nEntry].dwAvgTime = ConvertTickToMicrosecond( AvgTime ) / ( nCount - Dropped );
      else
          Entry[nEntry].dwAvgTime = 0;
      Entry[nEntry].dwDropped = Dropped;
      Entry[nEntry].pName[0] = 0;
      nEntry++;
   }
LoopBreakpoint:
	nRet = closesocket(rawSocket);
	if (nRet == SOCKET_ERROR)
		if( pResult )
			pResult = ReportError( pResult, WIDE("closesocket()"));

   if( bRDNS )
   {
      for( i = 0; i < nEntry; i++ )
      {
         if( Entry[i].dwIP )
         {
            //_32 dwID;
            LockedIncrement( &dwThreadsActive );
            ThreadTo( RDNSThread, (PTRSZVAL)(Entry+i) );
            /*
#ifdef _WIN32
            CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)RDNSThread, Entry+i, 0, &dwID );
#else
            pthread_create( &dwID, NULL, (void*(*)(void*))RDNSThread, (void*)(Entry+i) );
#endif
            */
         }
      }

      while( dwThreadsActive ) 
         Sleep(0);
   }

   for( i = 0; i < nEntry; i++ )
   {
      char *pIPBuf;
      if( Entry[i].dwIP )  
      {
         saSrc.sin_addr.s_addr = Entry[i].dwIP;
         pIPBuf = inet_ntoa( *(struct in_addr*)&saSrc.sin_addr );
         nResult = TRUE;
      }
      else        
      {
         pIPBuf = "No Response.";
         Entry[i].pName[0] = 0;
      }
      if( maxTTL )
      {
          char Min[8], Max[8], Avg[8];
          if( ResultCallback )
              ResultCallback( Entry[i].dwIP
                             , Entry[i].pName
                             , Entry[i].dwMinTime
                             , Entry[i].dwMaxTime
                             , Entry[i].dwAvgTime
                             , Entry[i].dwDropped
                             , 256 - Entry[i].TTL );
          else if( ResultCallbackEx )
				 ResultCallbackEx( psvUser
									  , Entry[i].dwIP
                             , Entry[i].pName
                             , Entry[i].dwMinTime
                             , Entry[i].dwMaxTime
                             , Entry[i].dwAvgTime
                             , Entry[i].dwDropped
                             , 256 - Entry[i].TTL );
          if( pResult )
			 {
				 if( Entry[i].dwAvgTime )
					 sprintf( Avg, WIDE("%7") _32f, Entry[i].dwAvgTime );
				 else
					 strcpy( Avg, WIDE("    ***") );
				 if( Entry[i].dwMinTime )
					 sprintf( Min, WIDE("%7") _32f, Entry[i].dwMinTime );
				 else
					 strcpy( Min, WIDE("    ***") );
				 if( Entry[i].dwMaxTime )
					 sprintf( Max, WIDE("%7") _32f, Entry[i].dwMaxTime );
				 else
					 strcpy( Max, WIDE("    ***") );

				 pResult += sprintf( pResult, WIDE("%3d %5d %s %s %s %4") _32f WIDE(" %5d %15.15s %s\n"),
										  i + 1,
										  REQ_DATASIZE,
										  Min,
										  Max,
										  Avg,
										  Entry[i].dwDropped,
										  256 - Entry[i].TTL,
										  pIPBuf,
										  Entry[i].pName
										 );
			 }
		}
		else
		{
			if( ResultCallback )
				ResultCallback( Entry[i].dwIP
									, Entry[i].pName
									, Entry[i].dwMinTime
									, Entry[i].dwMaxTime
									, Entry[i].dwAvgTime
									, Entry[i].dwDropped
									, 256 - Entry[i].TTL );
			else if( ResultCallbackEx )
				ResultCallbackEx( psvUser
									 , Entry[i].dwIP
									 , Entry[i].pName
									 , Entry[i].dwMinTime
									 , Entry[i].dwMaxTime
									 , Entry[i].dwAvgTime
									 , Entry[i].dwDropped
									 , 256 - Entry[i].TTL );
			if( pResult )
				pResult += sprintf( pResult, WIDE("%5d %7") _32f WIDE(" %7") _32f WIDE(" %7") _32f WIDE(" %4") _32f WIDE(" %5d %15.15s %s\n"),
										 REQ_DATASIZE,
										 Entry[i].dwMinTime,
										 Entry[i].dwMaxTime,
										 Entry[i].dwAvgTime,
										 Entry[i].dwDropped,
										 256 - Entry[i].TTL,
										 pIPBuf,
										 Entry[i].pName
										);
		}
	}
	LeaveCriticalSec( &cs );
   return nResult;
}

NETWORK_PROC( LOGICAL, DoPing )( CTEXTSTR pstrHost,
             int maxTTL, 
             _32 dwTime, 
             int nCount, 
             TEXTSTR pResult, 
             LOGICAL bRDNS, 
             void (*ResultCallback)( _32 dwIP, CTEXTSTR name, int min, int max, int avg, int drop, int hops ) )
{
   return DoPingExx( pstrHost, maxTTL, dwTime, nCount, pResult, bRDNS, ResultCallback, NULL, 0 );
}

NETWORK_PROC( LOGICAL, DoPingEx )( CTEXTSTR pstrHost,
											 int maxTTL,
											 _32 dwTime,
											 int nCount,
											 TEXTSTR pResult,
											 LOGICAL bRDNS,
											 void (*ResultCallback)( PTRSZVAL psv, _32 dwIP, CTEXTSTR name, int min, int max, int avg, int drop, int hops )
											, PTRSZVAL psvUser )
{
   return DoPingExx( pstrHost, maxTTL, dwTime, nCount, pResult, bRDNS, NULL, ResultCallback, psvUser );
}
// SendEchoRequest()
// Fill in echo request header
// and send to destination
int SendEchoRequest(TEXTSTR pResult, SOCKET s,SOCKADDR_IN *lpstToAddr)
{
	static ECHOREQUEST echoReq;
	static int nId = 1;
	static int nSeq = 1;
	int nRet;

	// Fill in echo request
	echoReq.icmpHdr.Type		= ICMP_ECHOREQ;
	echoReq.icmpHdr.Code		= 0;
	echoReq.icmpHdr.Checksum	= 0;
	echoReq.icmpHdr.ID			= nId++;
	echoReq.icmpHdr.Seq			= nSeq++;

	// Fill in some data to send
	for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
		echoReq.cData[nRet] = ' '+nRet;

	// Save tick count when sent
	echoReq.dwTime				= GetCPUTick();

	// Put data in packet and compute checksum
	echoReq.icmpHdr.Checksum = in_cksum((u_short *)&echoReq, sizeof(ECHOREQUEST));

	// Send the echo request  								  
	nRet = sendto(s,						/* socket */
				 (TEXTSTR)&echoReq,			/* buffer */
				 sizeof(ECHOREQUEST),
				 0,							/* flags */
				 (SOCKADDR*)lpstToAddr, /* destination */
				 sizeof(SOCKADDR));   /* address length */

	if (nRet == SOCKET_ERROR) 
		if( pResult )
			ReportError( pResult, WIDE("sendto()"));
	return (nRet);
}


// RecvEchoReply()
// Receive incoming data
// and parse out fields
int RecvEchoReply(TEXTSTR pResult, SOCKET s, SOCKADDR_IN *lpsaFrom, u_char *pTTL)
{
	ECHOREPLY echoReply;
	int nRet;
#ifdef __LINUX__
	socklen_t
#else
   int
#endif
		nAddrLen = sizeof(struct sockaddr_in);

	// Receive the echo reply	
	nRet = recvfrom(s,					// socket
					(TEXTSTR)&echoReply,	// buffer
					sizeof(ECHOREPLY),	// size of buffer
					0,					// flags
					(SOCKADDR*)lpsaFrom,	// From address
					&nAddrLen);			// pointer to address len

	// Check return value
	if (nRet == SOCKET_ERROR) 
	{
		if( pResult )
			ReportError( pResult, WIDE("recvfrom()"));
      return 0;
	}
// if( echoReply.ipHdr.
	// return time sent and IP TTL
	*pTTL = echoReply.ipHdr.TTL;
	return 1;   		
}

// What happened?
TEXTSTR ReportError(TEXTSTR pInto, TEXTSTR pWhere)
{
    return pInto + sprintf( pInto, WIDE("\n%s error: %d\n"),
                            pWhere, WSAGetLastError());
}


// WaitForEchoReply()
// Use select() to determine when
// data is waiting to be read
int WaitForEchoReply(SOCKET s, _32 dwTime)
{
	struct timeval Timeout;
	fd_set readfds;
   FD_ZERO( &readfds );
   FD_SET( s, &readfds );
	Timeout.tv_sec = dwTime / 1000; // longer than a second is too long
    Timeout.tv_usec = ( dwTime % 1000 ) * 1000;

	return(select(1, &readfds, NULL, NULL, &Timeout));
}


//
// Mike Muuss' in_cksum() function
// and his comments from the original
// ping program
//
// * Author -
// *	Mike Muuss
// *	U. S. Army Ballistic Research Laboratory
// *	December, 1983

/*
 *			I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
u_short in_cksum(u_short *addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		u_short	u = 0;

		*(u_char *)(&u) = *(u_char *)w ;
		sum += u;
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}
SACK_NETWORK_NAMESPACE_END

//#endif
// $Log: ping.c,v $
// Revision 1.15  2005/01/27 07:37:11  panther
// Linux cleaned.
//
// Revision 1.14  2003/11/09 03:32:15  panther
// Added some address functions to set port and override default port
//
// Revision 1.13  2003/10/27 16:41:37  panther
// Go to standard abstract ThreadTO instead of CreateThread
//
// Revision 1.12  2002/12/22 00:14:11  panther
// Cleanup function declarations and project defines.
//
// Revision 1.11  2002/11/24 21:37:41  panther
// Mods - network - fix server->accepted client method inheritance
// display - fix many things
// types - merge chagnes from verious places
// ping - make function result meaningful yes/no
// controls - fixes to handle lack of image structure
// display - fixes to handle moved image structure.
//
// Revision 1.10  2002/11/21 00:50:57  jim
// Made result of ping be useful - TRUE if result, FALSE if no result.
//
// Revision 1.9  2002/10/09 13:16:02  panther
// Support for linux shared memory mapping.
// Support for better linux compilation of configuration scripts...
// Timers library is now Threads AND Timers.
//
// Revision 1.8  2002/06/02 11:46:01  panther
// Modifications to build under MSVC
// Added MSVC project files.
//
// Revision 1.7  2002/04/19 18:09:51  panther
// Unix cleanup of massive changes....
//
// Revision 1.6  2002/04/18 15:13:55  panther
// Added per client cricticalsection locked(unused)
// Added timers for the pending closes (mostly)
// Fixed minor warning in ping.c (no type default int, missing prototypes)
//
// Revision 1.5  2002/04/18 15:01:42  panther
// Added Log History
// Stage 1: Delay schedule closes - only network thread will close.
//
