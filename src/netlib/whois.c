#include <stdhdrs.h>
#define LIBRARY_DEF
#include <stdio.h>
#ifdef __LINUX__
// close() for closesocket() alias.
#include <unistd.h>
#endif
#include <network.h>

#include <logging.h>
#include <string.h>

SACK_NETWORK_NAMESPACE

char DefaultServer[] = "whois.nsiregistry.net:43"; //whois";

LOGICAL DoWhois( CTEXTSTR pHost, CTEXTSTR pServer, TEXTSTR pResult )
{
	TEXTCHAR *pStartResult = pResult;
	SOCKET S;
	SOCKADDR *sa1;
	// hey if noone started it by now - they ain't gonna, and I will.
	NetworkStart();
	//SetSystemLog( SYSLOG_FILE, stdout );
	if( !pHost )
	{
		sprintf( pResult, WIDE("Must specify a host, handle, or domain name\n") );
		return FALSE;
	}
	S = socket( 2,1,6 );
	if( S == INVALID_SOCKET )
	{
		sprintf( pResult, WIDE("Could not allocate socket resource\n") );
		return FALSE;
	}

	if( !pServer )
		pServer = DefaultServer;

	sa1 = CreateSockAddress( pServer, 43 );
	if( connect( S, sa1, sizeof(*sa1) ) )
	{
		sprintf( pResult, WIDE("Failed to connect (%d)\n"), WSAGetLastError());
		closesocket( S );
		return FALSE;
	}

	{
		static char buf[4096];
		int  l;

		l = sprintf( buf, WIDE("%s\n"), pHost );

		if( send( S, buf, l, 0 ) != l )
		{
			sprintf( pResult, WIDE("Failed to be able to write data to the network\n") );
			closesocket( S );
			return FALSE;
		}

		// insert WAIT FOR RESPONCE code....
		//pResult += sprintf( pResult, WIDE("Version 1.0   ADA Software Developers, Inc.  Copyright 1999.\n") );
		while( ( l = recv( S, buf, sizeof( buf ), 0 ) ) > 0 )
		{
			if( l < sizeof(buf) )
				buf[l] = 0;
			pResult += sprintf( pResult, WIDE("%s"), buf );
		}
		{
			char *pNext, *pEnd;;
			if( ( pNext = strstr( pStartResult, WIDE("Whois Server: ") ) ) )
			{
				pNext += 14;
				pEnd = pNext;
				while( *pEnd != '\n' )
					pEnd++;
				strcpy( pEnd, WIDE(":43") ); // whois" );
				DoWhois( pHost, pNext, pStartResult );
			}
		}
	}
	closesocket( S );

	return TRUE;
}

SACK_NETWORK_NAMESPACE_END

// $Log: whois.c,v $
// Revision 1.8  2005/01/27 07:37:11  panther
// Linux cleaned.
//
// Revision 1.7  2004/01/11 23:24:43  panther
// Enable network in whois - if noone has noone will
//
// Revision 1.6  2002/04/18 15:01:42  panther
// Added Log History
// Stage 1: Delay schedule closes - only network thread will close.
//
