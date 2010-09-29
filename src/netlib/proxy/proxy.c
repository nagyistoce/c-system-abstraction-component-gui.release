#include <stdio.h>

#define DO_LOGGING
#include <stdhdrs.h>
#include <network.h>
#include <service_hook.h>
#define DEFAULT_SOCKETS 64
#define DEFAULT_TIMEOUT 5000

#define NL_PATH  0
#define NL_ROUTE 1
#define NL_OTHER 2   // PCLIENT other - the other client to send to...
#define NL_BUFFER  3 // pointer to buffer reading into....
#define NL_CONNECT_START 4 // _32 GetTickCount() when connect started.
#define NETWORK_WORDS 5

	typedef struct path_tag {
		PCLIENT in;
		PCLIENT out;
		struct route_tag *route;
		struct path_tag *next, **me;
	} PATH, *PPATH;

typedef struct route_tag {
	char name[256];
	struct {
      // if set - send IP as first 4 bytes to new connection
		_32 ip_transmit : 1;
		//TODO: implement _32 hw_transmit, and wherever IP transmit is sent, makd sure MAC addr is sent as well.
		// if set - recieve first 4 bytes from connection, make
      // new connection to THAT IP, instead of the configured one.
		_32 ip_route : 1;
   } flags;
	SOCKADDR *in, *out;
	PCLIENT listen;
	PPATH paths;  // currently active connections..
	struct route_tag *next, **me;
} ROUTE, *PROUTE;

static struct local_proxy_tag
{
	struct proxy_flags{
		BIT_FIELD not_first_run : 1;
	} flags;
	PLIST pPendingList;

} local_proxy_data;
#define l local_proxy_data

PROUTE routes;
_32 dwTimeout = DEFAULT_TIMEOUT;
_16 wSockets; // default to 64 client/servers

//---------------------------------------------------------------------------

PCLIENT ConnectMate( PROUTE pRoute, PCLIENT pExisting, SOCKADDR *sa );

//---------------------------------------------------------------------------

void RemovePath( PPATH path )
{
	if( path )
	{
		Log2( WIDE("Removing path to %s %p"), path->route->name, path );
		GetMemStats( NULL, NULL, NULL, NULL );
		if( ( *path->me = path->next ) )
		{
		   Log1( WIDE("Have a next... %p"), path->me );
			GetMemStats( NULL, NULL, NULL, NULL );
			Log1( WIDE("Updating next me to my me %p"), &path->next->me );
			path->next->me = path->me;
			GetMemStats( NULL, NULL, NULL, NULL );
		}	
		GetMemStats( NULL, NULL, NULL, NULL );
		Release( path );
	}
	else
	{
		Log( WIDE("No path defined to remove?!") );
	}
}

//---------------------------------------------------------------------------

void CPROC TCPClose( PCLIENT pc )
{
	PCLIENT other = (PCLIENT)GetNetworkLong( pc, NL_OTHER );
   Log( WIDE("TCP Close") );
	Release( (POINTER)GetNetworkLong( pc, NL_BUFFER ) );
	if( other )
	{
		Release( (POINTER)GetNetworkLong( other, NL_BUFFER ) );
		RemoveClientEx( other, TRUE, FALSE );
	}
	RemovePath( (PPATH)GetNetworkLong( pc, NL_PATH ) );
	// hmm how to close mate...
}

//---------------------------------------------------------------------------

void CPROC TCPRead( PCLIENT pc, POINTER buffer, int size )
{
   //Log( WIDE("TCP Read Enter...") );
	if( !buffer )
	{
		PROUTE route = (PROUTE)GetNetworkLong( pc, NL_ROUTE );
      PPATH path = (PPATH)GetNetworkLong( pc, NL_PATH );
		buffer = Allocate( 4096 );
		SetNetworkLong( pc, NL_BUFFER, (PTRSZVAL)buffer );
		if( route->flags.ip_route && !path )
		{
         Log( WIDE("Was delayed connect, reading 4 bytes for address") );
			ReadTCPMsg( pc, buffer, 4 ); // MUST read 4 and only 4
         return;
		}
	}
	else
	{
		PCLIENT other;
		if( !( other = (PCLIENT)GetNetworkLong( pc, NL_OTHER ) ) )
		{
			// will NEVER have a OTHER if delayed connect
			// otherwise, we will not be here without an
			// other already associated, the other may fail later
			// but at this point it is known that there is an OTHER
			PROUTE route = (PROUTE)GetNetworkLong( pc, NL_ROUTE );
			// so - get the route of this one, see if we have to read
			// an IP...
			Log1( WIDE("Route address: %p"), route );
			if( !route )
			{
            Log( WIDE("FATALITY! I die!") );
			}
         else if( route->flags.ip_route )
			{
				SOCKADDR saTo = *route->out;
				((_32*)&saTo)[1] = *(_32*)buffer;
				if( !ConnectMate( route, pc, &saTo ) )
				{
               Log( WIDE("Connect mate failed... remove connection") );
					RemoveClient( pc );
					return;
				}
				else
               Log( WIDE("Successful connection to remote (pending)") );
			}
		}
		else
		{
#ifdef _DEBUG
			//Log2( WIDE("Sending %d bytes to mate %p"), size, other );
#endif
			SendTCP( other, buffer, size );
		}
	}
	ReadTCP( pc, buffer, 4096 );
}

//---------------------------------------------------------------------------

PPATH MakeNewPath( PROUTE pRoute, PCLIENT in, PCLIENT out )
{
   Log1( WIDE("Adding path to %s"), pRoute->name );
   {
	PPATH path = (PPATH)Allocate( sizeof( PATH ) );
	path->me = &pRoute->paths;
	if( ( path->next = pRoute->paths ) )
		path->next->me = &path->next;
	pRoute->paths = path;
   path->route = pRoute;
	path->in = in;
	path->out = out;
   // link the sockets...
	SetNetworkLong( in, NL_OTHER, (PTRSZVAL)out );
	SetNetworkLong( out, NL_OTHER, (PTRSZVAL)in );
   // link the paths...
	SetNetworkLong( in, NL_PATH, (PTRSZVAL)path );
	SetNetworkLong( out, NL_PATH, (PTRSZVAL)path );
 	return path;
 	}
}

//---------------------------------------------------------------------------

void CPROC TCPConnected( PCLIENT pc, int error )
{
	// delay connect finished...
	Log( WIDE("Connection finished...") );
	{
		// make sure we've set all data, and added it
		// it IS possible that this routine will be called
		// before the creator has finished initializing
		// this secondary thing...
		while( !GetNetworkLong( pc, NL_CONNECT_START ) )
			Relinquish();
		DeleteLink( &l.pPendingList, pc );
	}
	if( !error )
	{
      //Log( WIDE("Proxied connect success!") );
		// should be okay here....
	}
	else
	{
		Log( WIDE("Delayed connect failed... ") );
		RemoveClient( pc );
	}
}

//---------------------------------------------------------------------------

PCLIENT ConnectMate( PROUTE pRoute, PCLIENT pExisting, SOCKADDR *sa )
{
	PCLIENT out;
   Log( WIDE("Connecting mating connection... ") );
	out = OpenTCPClientAddrExx( sa, TCPRead, TCPClose, NULL, TCPConnected );
	if( out )
	{
      _32 dwIP = GetNetworkLong( pExisting, GNL_IP );
		if( pRoute->flags.ip_transmit )
		{
			Log1( WIDE("Sending initiant's IP : %08") _32fX WIDE(""), dwIP );
			SendTCP( out, &dwIP, 4 );
		}
		SetTCPNoDelay( out, TRUE );
		SetClientKeepAlive( out, TRUE );
		SetNetworkLong( out, NL_ROUTE, (PTRSZVAL)pRoute );
		MakeNewPath( pRoute, pExisting, out );
		SetNetworkLong( out, NL_CONNECT_START, timeGetTime() );
		AddLink( &l.pPendingList, out );
	}
	else
	{
		// should only fail on like - no clients at all...
		Log1( WIDE("Failed to make proxy connection for: %s"), pRoute->name );
		RemoveClient( pExisting );
	}
   return out;
}

//---------------------------------------------------------------------------

void CPROC TCPConnect( PCLIENT pServer, PCLIENT pNew )
{
	PCLIENT out;
	PROUTE pRoute = (PROUTE)GetNetworkLong( pServer, NL_ROUTE );
   Log( WIDE("TCP Connect received.") );
   SetTCPNoDelay( pNew, TRUE );
	SetClientKeepAlive( pNew, TRUE );
	if( pRoute->flags.ip_route )
	{
		// hmm this connection's outbound needs to be held off
		// until we get the desired destination....
      Log( WIDE("Delayed connection to mate...") );
	}
	else
	{
		out = ConnectMate( pRoute, pNew, pRoute->out );
		if( !out )  // if not out - then pNew is already closed.
         return;
	}
	// have to set the route.
	SetNetworkLong( pNew, NL_ROUTE, (PTRSZVAL)pRoute );
	SetNetworkCloseCallback( pNew, TCPClose );
	SetNetworkReadComplete( pNew, TCPRead );
}

//---------------------------------------------------------------------------

void RemoveRoute( PROUTE route )
{
   Log( WIDE("Remove route...") );
	if( ( *route->me = route->next ) )
		route->next->me = route->me;
	ReleaseAddress( route->in );
	ReleaseAddress( route->out );
	if( route->listen )
		RemoveClient( route->listen );
}

//---------------------------------------------------------------------------

void AddRoute( int set_ip_transmit
				 , int set_ip_route
				 , char *route_name
				 , char *src_name, int src_port
				 , char *dest_name, int dest_port )
{
	PROUTE route = (PROUTE)Allocate( sizeof( ROUTE ) );
	Log5( WIDE("Adding Route %s: %s:%d %s:%d")
					, route_name
					, src_name?src_name:"0.0.0.0", src_port
					, dest_name?dest_name:"0.0.0.0", dest_port );
	if( route_name )
		StrCpyEx( route->name, route_name, sizeof( route->name ) );
	else
		StrCpyEx( route->name, WIDE("unnamed"), sizeof( route->name ) );
	route->flags.ip_transmit = set_ip_transmit;
	route->flags.ip_route = set_ip_route;
	route->in = CreateSockAddress( src_name, src_port );
	route->out = CreateSockAddress( dest_name, dest_port );
 	route->paths = NULL;
 	route->me = &routes;
 	if( ( route->next = routes ) )
 		route->next->me = &route->next;
	routes = route;
}

//---------------------------------------------------------------------------

void BeginRouting( void )
{
	PROUTE start = routes, next;
	while( start )
	{
		next = start->next;
		start->listen = OpenTCPListenerAddrEx( start->in, TCPConnect );
		if( start->listen )
			SetNetworkLong( start->listen, NL_ROUTE, (PTRSZVAL)start );
		else
		{
			Log1( WIDE("Failed to open listener for route: %s"), start->name );
			RemoveRoute( start );
		}
		start = next;
	}
}

//---------------------------------------------------------------------------

void ReadConfig( FILE *file )
{
	char buffer[256];
	int len;
	while( fgets( buffer, 255, file ) )
	{
		int set_ip_transmit = 0;
      int set_ip_route = 0;
		char *start, *end
		   , *name
		   , *addr1
		   , *addr2;
		int port1, port2;
		buffer[255] = 0;
		len = strlen( buffer );
		if( len == 255 )
		{
			Log( WIDE("FATAL: configuration line is just toooo long.") );
			break;
		}
		buffer[len] = 0; len--; // terminate (remove '\n')

		end = strchr( buffer, '#' );
		if( end )
			*end = 0; // trim comments.

		start = buffer;
      // eat leading spaces...
		while( start[0] && (start[0] == ' ' || start[0] == '\t') )
			start++;
		end = start;
		// find the :
		while( end[0] && end[0] != ':' )
			end++;
		if( !end[0] )
		{
			//Log( WIDE("No name field, assuming no route on line.") );
			continue;
		}
		end[0] = 0; // terminate ':'
      end++;
		// consume possible blanks after the ':'
	check_other:
		while( end[0] == ' ' || end[0] == '\t' )
		{
         end[0] = 0; // lots of nulls...
			end++;
		}

		if( !strncmp( end, WIDE("ip"), 2 ) )
		{
         set_ip_transmit = 1;
         end += 2;
		}
		while( end[0] == ' ' || end[0] == '\t' )
		{
         end[0] = 0;
			end++;
		}
		if( !strncmp( end, WIDE("switch"), 6 ) )
		{
			set_ip_route = 1;
			end += 6;
         goto check_other;
		}
      /*
      // comsume possible trailing spaces after option.
		while( end[1] == ' ' || end[1] == '\t' )
		{
			end[0] = 0;
			end++;
		}
		*/

		// fix existing bug until configuration files can be fixed.
		if( end[0] == 'a' )
		{
			end[0] = 0;
         end++;
		}


		if( !strcmp( start, WIDE("sockets") ) )
		{
			wSockets = atoi( end );
			if( !wSockets )
				wSockets = DEFAULT_SOCKETS;
			if( !NetworkWait( 0, wSockets, NETWORK_WORDS ) )
			{
				Log( WIDE("Could not begin network!") );
			}
			Log1( WIDE("Setting socket limit to %d sockets"), wSockets );
		}
		else if( !strcmp( start, WIDE("timeout") ) )
		{
			dwTimeout = atol( end );
			if( !dwTimeout )
				dwTimeout = DEFAULT_TIMEOUT;
			Log1( WIDE("Setting socket connect timeout to %")_32f WIDE(" milliseconds"), dwTimeout );
		}
		else
		{
			name = start;
			addr1 = end;
			// find the next space after first address
			while( end[0] && end[0] != ' ' )
			{
				end++;
			}
			while( end[0] && end[0] == ' ' ) // kill spaces after first address
			{
				end[0] = 0;
            end++;
			}
			// now addr1 == second space delimted value...
         // if end is not a character - ran out of data...
			if( !end[0] )
			{
            // spaces followed name, but there was no name content...
				Log( WIDE("Only found one address on the line... invalid route.") );
				continue;
			}

			addr2 = end;
			while( end[0] && end[0] != ' ' )
			{
				end++;
			}
			while( end[0] && end[0] == ' ' ) // kill spaces after second address
			{
				end[0] = 0;
            end++;
			}

			// if there was a tertiary field, then it could be read here
         // after validating that end[0] was data...

			if( ( start = strchr( addr1, ':' ) ) )
			{
				start[0] = 0;
				start++;
				port1 = atoi( start );
			}
			else
			{
				port1 = atoi( addr1 );
				addr1 = NULL;
			}
			
			if( ( start = strchr( addr2, ':' ) ) )
			{
				start[0] = 0;
				start++;
				port2 = atoi( start );
			}
			else
			{
				port2 = atoi( addr2 );
				addr2 = NULL;
			}
			AddRoute( set_ip_transmit, set_ip_route
					  , name
					  , addr1, port1
					  , addr2, port2 );
		}
	}
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC CheckPendingConnects( PTHREAD pUnused )
{
	PCLIENT pending;
	INDEX idx;
	while( 1 )
	{
		LIST_FORALL( l.pPendingList, idx, PCLIENT, pending )
		{
			_32 dwStart = GetNetworkLong( pending, NL_CONNECT_START );

			//Log2( WIDE("Checking pending connect %ld vs %ld"),
			//     ( GetNetworkLong( pending, NL_CONNECT_START ) + dwTimeout ) , GetTickCount() );
			if( dwStart && ( ( dwStart + dwTimeout ) < timeGetTime() ) )
			{
				Log( WIDE("Failed Connect... timeout") );
				RemoveClient( pending );
				Log( WIDE("Done removing the pending client... "));
				SetLink( &l.pPendingList, idx, NULL ); // remove it from the list.
			}
		}
		WakeableSleep( 250 );
	}
   return 0;
}

//---------------------------------------------------------------------------

int task_done;
void CPROC MyTaskEnd( PTRSZVAL psv, PTASK_INFO task )
{
   task_done = 1;
}
void CPROC GetOutput( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 length )
{
   lprintf( "%s", buffer );
}

//---------------------------------------------------------------------------
static char *filename;

static void CPROC Start( void )
{
	FILE *file;
	// should clear all routes here, and reload them.
	file = sack_fopen( 0, filename, WIDE("rb") );
	lprintf( "config would be [%s]", filename );

	if( !l.flags.not_first_run )
	{
		NetworkStart();
		l.flags.not_first_run = 1;

		if( !file )
		{
			Log1( WIDE("Could not locate %s in current directory."), filename );
			return;
		}
		ReadConfig( file );
		fclose( file );

		if( !wSockets )
		{
			wSockets = DEFAULT_SOCKETS;
			if( !NetworkWait( 0, wSockets, NETWORK_WORDS ) )
			{
				Log( WIDE("Could not begin network!") );
				return;
			}
		}

		// successful test if client connects, loops though this
		// and then disconnects
		// fail test if client connects through loops, and last connection fails.

		//AddRoute( WIDE("Local1"), NULL, 4000, WIDE("localhost"), 4001 );
		//AddRoute( WIDE("Local1"), NULL, 4001, WIDE("localhost"), 4002 );
		//AddRoute( WIDE("Local1"), NULL, 4002, WIDE("localhost"), 4003 );

		//AddRoute( NULL, WIDE("mail.fortunet.com"), 3000, WIDE("172.16.100.1"), 3000 );
		//AddRoute( NULL, WIDE("mail.fortunet.com"), 3001, WIDE("172.16.100.1"), 3001 );
		//AddRoute( NULL, WIDE("mail.fortunet.com"), 3002, WIDE("172.16.100.1"), 23 );

		//AddRoute( WIDE("Local1"), NULL, 4000, WIDE("172.16.100.1"), 3001 );
		Log1( WIDE("Pending timer is set to %") _32f, dwTimeout );
		ThreadTo( CheckPendingConnects, 0 );
		//AddTimer( 250, CheckPendingConnects, 0 );
		BeginRouting();
	}
	else
	{
		lprintf( "Might want to re-read configuration and do something with it." );
      fclose( file );
	}

}

//---------------------------------------------------------------------------

int main( int argc, char **argv )
{

   int ofs = 0;
#ifdef BUILD_SERVICE
	if( argc > (1) && StrCaseCmp( argv[1], "install" ) == 0 )
	{
		ServiceInstall( GetProgramName() );
		return 0;
	}
	if( argc > (1) && StrCaseCmp( argv[1], "uninstall" ) == 0 )
	{
		ServiceUninstall( GetProgramName() );
		return 0;
	}
#endif
#ifdef BUILD_SERVICE
	{
		static TEXTCHAR tmp[256];
		snprintf( tmp, sizeof( tmp ), "%s/%s.conf", GetProgramPath(), GetProgramName() );
		filename = tmp;
	}
   // for some reason service registration requires a non-const string.  pretty sure it doesn't get modified....
	SetupService( (TEXTSTR)GetProgramName(), Start );
#else
	if( argc < 2 )
	{
		static TEXTCHAR tmp[256];
      snprintf( tmp, sizeof( tmp ), "%s/%s.conf", GetProgramPath(), GetProgramName() );
		filename = tmp;
	}
	else
		filename = argv[1];

	Start();

	while( 1 )
		Sleep( 100000 );
#endif
	return 0;
}

// $Log: proxy.c,v $
// Revision 1.32  2003/12/03 10:23:53  panther
// Fix minor issue
//
// Revision 1.31  2003/12/03 10:23:28  panther
// Init network as soon as we know how many intended sockets - enables name resolution
//
// Revision 1.30  2003/12/03 10:21:44  panther
// Tinkering with C++ networking
//
// Revision 1.29  2003/11/28 17:06:56  panther
// Fix thread return.  Fix LIST_ENDFORALL
//
// Revision 1.28  2003/09/26 10:22:56  panther
// Okay C++ is not valid possiblity under linux so quick mod it out
//
// Revision 1.27  2003/09/24 15:10:54  panther
// Much mangling to extend C++ network interface...
//
// Revision 1.26  2003/09/24 02:25:48  panther
// Add C++ usage of network library... perhaps this should convert to configscript also
//
// Revision 1.25  2003/07/29 09:27:15  panther
// Add Keep Alive option, enable use on proxy
//
// Revision 1.24  2003/07/24 22:50:10  panther
// Updates to make watcom happier
//
// Revision 1.23  2003/06/20 09:57:45  panther
// Ug - fixed parsing of command line data...
//
// Revision 1.22  2003/06/04 11:39:31  panther
// Stripped carriage returns
//
// Revision 1.21  2003/05/17 15:50:16  panther
// Fix bad linking on route creation
//
// Revision 1.20  2003/04/15 12:16:30  panther
// Okay - SO don't close the client too early - but wait for it to be ready then close it.
//
// Revision 1.19  2003/04/15 10:35:24  panther
// Add another thread for pending connects - timer conflict?
//
// Revision 1.18  2003/03/25 08:45:55  panther
// Added CVS logging tag
//
