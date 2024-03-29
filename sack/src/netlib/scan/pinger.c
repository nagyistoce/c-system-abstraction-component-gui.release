#include <stdhdrs.h>
#include <network.h>


void PingResult( _32 dwIP, CTEXTSTR name
						, int min, int max, int avg
						, int drop, int hops )
{
	if( dwIP ) // else was a timeout.
	printf( WIDE("Result: %25s(%12s) %d %d %d\n"), name, inet_ntoa( *(struct in_addr*)&dwIP )
                 							, min, max, avg );
}

int main( int argc, char **argv )
{
	if( argc < 2 )
	{
		printf( WIDE("Please enter a IP - class c will be scanned\n") );
		return 0;
	}
   NetworkWait(NULL,2000,4);
	{
		char address[256];
		_32 IP;
		IP = inet_addr(argv[1]);
		IP = ntohl( IP );
		for( ; ( IP & 0xFFff ) != 0xFFFF ; IP++ )
		{
			_32 junk = htonl(IP);
			strcpy( address, inet_ntoa( *(struct in_addr*)&junk ) );
			//printf( WIDE("Trying %s...\n"), address );
			 DoPing( address, 
      	         0,  // no ttl - just ping
         	    50,  // short timeout
            	 1,    // just one time
	             NULL, 
   	          TRUE, // no RDNS for now
      	       PingResult );
		}
	}
	return 0;
}


// $Log: pinger.c,v $
// Revision 1.3  2005/01/27 07:37:11  panther
// Linux cleaned.
//
// Revision 1.2  2003/03/25 08:45:55  panther
// Added CVS logging tag
//
