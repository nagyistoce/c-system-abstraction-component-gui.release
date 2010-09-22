#define VERSION "2.01"
#include <stdhdrs.h>
#include <system.h>
#ifdef __WINDOWS__
#include <shellapi.h>
#endif
#include <sharemem.h>
#include <network.h>
#include <logging.h>
#ifdef __WINDOWS__
HWND ghWndIcon;
#define WM_USERICONMSG (WM_USER + 212)

	HMENU hMainMenu;
#define MNU_EXIT 1000
#endif


#define CLASSNAME "LaunchPad"

static CTEXTSTR class_name; // only respect commands to this class... if NULL accept all
static PCLIENT pcCommand;
static PCLIENT pcTrack; // connection tracks remote...
static int track_port;
static SOCKADDR *send_to;

static void CPROC UDPRead( PCLIENT pc, POINTER buffer, int size, SOCKADDR *sa )
{
	if( buffer )
	{
		buffer = Allocate( 2048 );
	}
	else
	{
#ifdef __cplusplus
		::
#endif
		system( (char*)buffer );
	}
	ReadUDP( pc, buffer, 2048 );
}

static int BeginNetwork( void )
{
	if( !NetworkStart() )
		return 0;
	pcCommand = ConnectUDP( NULL, 3007, "255.255.255.255", 3006, UDPRead, NULL );
	if( !pcCommand )
		pcCommand = ConnectUDP( NULL, 3008, "255.255.255.255", 3006, UDPRead, NULL );
	if( !pcCommand )
		pcCommand = ConnectUDP( NULL, 3009, "255.255.255.255", 3006, UDPRead, NULL );
	if( !pcCommand )
		pcCommand = ConnectUDP( NULL, 3010, "255.255.255.255", 3006, UDPRead, NULL );
	if( !pcCommand )
		pcCommand = ConnectUDP( NULL, 3011, "255.255.255.255", 3006, UDPRead, NULL );
	if( !pcCommand )
		pcCommand = ConnectUDP( NULL, 3012, "255.255.255.255", 3006, UDPRead, NULL );
	if( !pcCommand )
	{
      srand( GetTickCount() );
		pcCommand = ConnectUDP( NULL, (_16)(3000 + rand() % 10000), "255.255.255.255", 3006, UDPRead, NULL );
	}


	if( !pcCommand )
	{
      printf( "Failed to bind to any port!\n" );
		return 0;
	}
	return 1;
}

static int clients;

static void CPROC Received( PCLIENT pc, POINTER buffer, int size )
{
	if( !buffer )
	{
      buffer = Allocate( 4096 );
	}
	else
	{
		((char*)buffer)[size] = 0;
#ifdef CONSOLE
		printf( "%s", buffer );
#else
		lprintf( "Received %s", buffer );
#endif
	}
   ReadTCP( pc, buffer, 4095 );
}

static void CPROC Closed( PCLIENT pc )
{
   clients--;
}

static PTRSZVAL CPROC SendConsoleInput( PTHREAD thread )
{
   char cmd[256];
	PCLIENT pc = (PCLIENT)GetThreadParam( thread );
   cmd[0] = '@'; // prefix with output to send
	while( fgets( cmd+1, sizeof( cmd )-1, stdin ) )
	{
      SendTCP( pc, cmd, strlen( cmd ) );
	}
   return 0;
}

static void CPROC Connected( PCLIENT pc_server, PCLIENT pc_new )
{
   clients++;
	SetNetworkReadComplete( pc_new, Received );
	SetNetworkCloseCallback( pc_new, Closed );
   ThreadTo( SendConsoleInput, (PTRSZVAL)pc_new );
#ifdef CONSOLE
	//lprintf( "Sending back status 'connect ok'" );
	printf( "~CONNECT OK\n" );
	fflush( stdout );
#endif
}

#if defined(WINDOWS_MODE)
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow )
{
	int argc;
	char **argv;
   ParseIntoArgs( GetCommandLine(), &argc, &argv );
#else
int main( int argc, char **argv )
#endif
{
	if( argc == 1 )
	{
#ifdef __WINDOWS__
		MessageBox( NULL,
#else
          printf(
#endif
					  "Usage: launchcmd [-l] [-r] [-h] [-s address] [-c class]   ....remote command....\n"
					  " -l : listen to remote output, stays connected, and ... (do nothing - work in progress)\n"
					  " -r : auto-restart when task ends on remote side.\n"
					  " -h : launch task hidden\n"
					  " -s : specify target address to send to (instead of local broadcast)\n"
								 " -c : specify class of launchpads to respond to commands (matches -c arguments on launchpad)\n"
#ifdef __WINDOWS__
								, "Usage"
								, MB_OK );
#else
                  );
#endif
      return 0;
	}
	if( BeginNetwork() )
	{
		static char lpCmd[4096];
		int ofs = 0;
		int n;
		int want_restart = 0;
      int want_hide = 0;
      int listen_output = 0;
		int arg_ofs;
		_32 sequence = GetTickCount(); // this should be fairly unique...
#ifdef CONSOLE
		printf( "launchcmd version %s\n", VERSION );
		fflush( stdout );
#endif
		{
         int done = 0;
			for( arg_ofs = 1; !done && arg_ofs < argc; arg_ofs++ )
			{
				if( argv[arg_ofs][0] == '-' )
				{
					switch( argv[arg_ofs][1] )
					{
					case 'l':
						listen_output = 1;
						{
							int n;
							for( n = 0; n < 32; n++ )
							{
								pcTrack = OpenTCPListenerEx( track_port = 3013+n, Connected );
								if( pcTrack )
									break;
							}
							if( !pcTrack )
							{
                        // failed to open TCP listener... forget this.
#ifdef __WINDOWS__
								MessageBox( NULL,
#else
											  printf(
#endif
														"Failed to open receiving monitor socket..."
#ifdef __WINDOWS__
													  , "Abort Launch"
													  , MB_OK );
#else
											 );
#endif

								return 0;
							}
						}

						break;
					case 'r':
                  want_restart = 1;
                  break;
					case 'h':
                  want_hide = 1;
						break;
					case 's':
                  if( argv[arg_ofs][2] )
							send_to = CreateSockAddress( argv[arg_ofs]+2, 3006 );
						else
						{
                     arg_ofs++;
							send_to = CreateSockAddress( argv[arg_ofs], 3006 );
						}
                  break;
					case 'c':
                  if( argv[arg_ofs][2] )
							class_name = StrDup( argv[arg_ofs]+2 );
						else
						{
                     arg_ofs++;
							class_name = StrDup( argv[arg_ofs] );
						}
                  break;
					case '-':
						done = 1;
                  break;
					}
				}
				else
               break;
			}
		}
		ofs = 0;
		if( listen_output )
			ofs += snprintf( lpCmd + ofs, sizeof( lpCmd ) - ofs, "~capture%d", track_port );

		if( want_hide )
         ofs += snprintf( lpCmd + ofs, sizeof( lpCmd ) - ofs, "~hide" );

		if( class_name )
			ofs += snprintf( lpCmd + ofs, sizeof( lpCmd ) - ofs, "$%s", class_name );
		ofs += snprintf( lpCmd + ofs, sizeof( lpCmd ) - ofs, "^%ld%c", sequence, want_restart?'@':'#' );
		for( n = arg_ofs; n < argc; n++ )
		{
			ofs += snprintf( lpCmd + ofs, sizeof( lpCmd ) - ofs
								, "%s"
								, argv[n] );
         ofs++; // include '\0' in buffer so we can recover exact parameters...
		}
		//lprintf( "Sending [%s]", lpCmd );
		if( send_to )
		{
			SendUDPEx( pcCommand, lpCmd, ofs, send_to );
			Sleep( 25 );
			SendUDPEx( pcCommand, lpCmd, ofs, send_to );
			Sleep( 25 );
			SendUDPEx( pcCommand, lpCmd, ofs, send_to );
		}
		else
		{
			SendUDP( pcCommand, lpCmd, ofs );
			Sleep( 25 );
			SendUDP( pcCommand, lpCmd, ofs );
			Sleep( 25 );
			SendUDP( pcCommand, lpCmd, ofs );
		}
		if( listen_output )
		{
			_32 tick_start = GetTickCount();
			while( !clients && ( ( GetTickCount() - tick_start ) < 6000 ) )
			{
            //lprintf( "waiting a second for reverse connect..." );
				WakeableSleep( 100 );
			}
			if( !clients )
			{
#ifdef __WINDOWS__
				MessageBox( NULL,
#else
							  printf(
#endif
                              "No remote launchpads accepted this command.  Request was to monitor output, to guarantee task execution"
#ifdef __WINDOWS__
									  , "Network Failure"
									  , MB_OK );
#else
                  );
#endif
			}
			while( clients )
			{
				//lprintf( "waiting..." );
				WakeableSleep( 500 );
			}
		}
	}
	else
		printf( "Failed to start network\n" );
	RemoveClient( pcCommand );
	return 0;
}
#if defined(WINDOWS_MODE)
}
#endif
