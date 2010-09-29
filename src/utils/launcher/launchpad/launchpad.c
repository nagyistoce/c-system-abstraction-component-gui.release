#define DO_LOGGING
#include <stdhdrs.h>
#ifdef __WINDOWS__
#include <shellapi.h>
#endif
#include <sharemem.h>
#include <network.h>
#include <system.h>
#include <timers.h>
#define DO_LOGGING
#include <logging.h>
#include <systray.h>

// keep a list of all sequences... and expire them at some point?

typedef struct sequence_data *pSequence;
struct sequence_data {
	_32 sequence;
	_32 tick;
	/* we might end up with the same sequence for different packets...
    * happens when we launch two processes at the same time... */
	POINTER packet;
   int packet_length; // paramter to network callback is int
};
PLIST class_names;
//CTEXTSTR class_name;
static PLIST sequences;
PCLIENT pcListen;
static int bLogOutput;

typedef struct restartable_task RTASK;
typedef struct restartable_task *PRTASK;
struct restartable_task
{
   _32 fast_restart_count;
   _32 prior_tick;
	PCTEXTSTR args;
	CTEXTSTR program;
	struct {
		BIT_FIELD bLogOutput : 1;
		BIT_FIELD bHide : 1;
	} flags;
};


static PLIST restarts; // these tasks restart when exited...

static void CPROC ExpireSequences( PTRSZVAL psv )
{
	pSequence pSeq = NULL;
	INDEX idx;
	_32 tick = GetTickCount();
	do
	{
		LIST_FORALL( sequences, idx, pSequence, pSeq )
		{
			if( ( pSeq->tick + 5000 ) < tick )
			{
				// moves all in list down a bit...
				SetLink( &sequences, idx, NULL );
            Release( pSeq->packet );
            Release( pSeq );
				break;
			}
		}
	} while( pSeq );
}

static void CPROC GetTaskOutput( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 size )
{
	if( bLogOutput )
		lprintf( "%s", buffer );
	{
		PCLIENT pc = (PCLIENT)psv;
		if( pc )
		{
			SendTCP( pc, buffer, size );
		}
	}
}



static void CPROC RestartTask( PTRSZVAL psv, PTASK_INFO task )
{
	PRTASK restart = (PRTASK)psv;
	_32 now = GetTickCount();
   lprintf( "restarting." );
	if( ( now - restart->prior_tick ) < 3000 )
	{
		restart->fast_restart_count++;
	}
	else
		restart->fast_restart_count = 0;
	restart->prior_tick = now;
	if( restart->fast_restart_count < 3 )
	{
		if( restart->flags.bLogOutput)
		{
			LaunchPeerProgramExx(restart->program, NULL
									  , (PCTEXTSTR)restart->args
									  , restart->flags.bHide?0:LPP_OPTION_DO_NOT_HIDE
									  , GetTaskOutput
									  , RestartTask
									  , (PTRSZVAL)restart
										DBG_SRC
									  );
		}
		else
		{
			LaunchProgramEx( restart->program, NULL
								, (PCTEXTSTR)restart->args
								, RestartTask
								, (PTRSZVAL)restart );
		}
	}
	else
	{
		lprintf( "Task restart was too fast ( 3 times within 3 seconds each ). Failing task." );
		Release( (POINTER)restart->program );
      DeleteLink( &restarts, restart );
      Release( restart );
	}
}

static void CPROC TaskEnded( PTRSZVAL psv, PTASK_INFO task )
{
	//lprintf( "Caught task ended - eliminates zombies?" );
	if( psv )
	{
		PCLIENT pc = (PCLIENT)psv;
		// we were sending information back to launcher...
		//lprintf( "closing client to launchcmd. %p", pc );
      SetProgramUserData( task, 0 );
      SetNetworkLong( pc, 1, 0 );
      RemoveClient( pc );
	}
}

static void CPROC RemoteClosed( PCLIENT pc )
{
   PTASK_INFO task;
	if( bLogOutput )
		lprintf( "remote closed, terminating associated client" );
	task = (PTASK_INFO)GetNetworkLong( pc, 1 );
   SetNetworkLong( pc, 1, 0 );
	if( task )
	{
		SetProgramUserData( task, 0 );
      //lprintf( "terminating..." );
		TerminateProgram( task );
      //lprintf( "termated." );
	}
   // release TCP buffer
	Release( (POINTER)GetNetworkLong( pc, 0 ) );
}

static void CPROC RemoteSent( PCLIENT pc, POINTER p, int size )
{
	if( !p )
	{
		p = Allocate( 256 );
      SetNetworkLong( pc, 0, (PTRSZVAL)p );
	}
	else
	{
      P_8 buf = (P_8)p;
		// do nothing with received input...
		// it's just the other side probing connection alive... maybe... maybe there should be
		// a couple commands - one for control level
		// the other for sending to tasks's stdin channel...
		if( buf[0] = '@' )
		{
			PTASK_INFO task = (PTASK_INFO)GetNetworkLong( pc, 1 );
         buf[size] = 0;
			//lprintf( "Positing output to task:%s", buf+1 );
         pprintf( task, "%s", buf+1 );
		}
	}
   ReadTCP( pc, p, 256 );
}

static void CPROC RemoteReverseConnected( PCLIENT pc, int error_code )
{
	//lprintf( "reverse connect complete. %d", error_code );
	if( error_code )
	{
      // didn't actually connect....
	}
}

// packet format
				// [~option][~option]...
				//   option value supported now...
				//      'capture' - result to the invoking command the tasks's output
// [$classname]
// [^sequence]
// #command
//

struct thread_args
{
	char restart;
			
	SOCKADDR *sa;
	char *inbuf;
	int bCaptureOutput;
	int bNulArgs;
	char **args;
	char *seqbuf;
	int hide_process ;
	PRTASK restart_info;
};

static PTRSZVAL CPROC RemoteBackConnect( PTHREAD thread )
{
	struct thread_args *args = (struct thread_args *)GetThreadParam( thread );
	PTASK_INFO task;
	PCLIENT pc_output = NULL;
	if( args->bCaptureOutput )
	{
      //lprintf( "Want to capture task output, so back-connect." );
		//SetAddressPort( sa, capture_port );
		//DumpAddr( "connect to", sa );
		pc_output = OpenTCPClientAddrExx( args->sa
												 , RemoteSent // no reception
												 , RemoteClosed // no close notice
												  , NULL
												  , RemoteReverseConnected
												  ); // no write notic
	}
   //lprintf( "Launch the process..." );
	task = LaunchPeerProgramExx(args->inbuf, NULL
										, (PCTEXTSTR)args->args
										, args->hide_process?0:LPP_OPTION_DO_NOT_HIDE
										, GetTaskOutput
										, args->restart_info?RestartTask:TaskEnded
										, (PTRSZVAL)(args->restart_info?((PTRSZVAL)args->restart_info):((PTRSZVAL)pc_output))
										 DBG_SRC
										);
   //lprintf( "And finally hook the process to its socket.." );
	if( args->bCaptureOutput )
	{
		SetNetworkLong( pc_output, 1, (PTRSZVAL)task );
	}
	ReleaseAddress( args->sa );
   Release( args->inbuf );
	if( !args->restart )
	{
		// restartable tasks want to keep the args...
		Release( args->args );
	}

   Release( args );
   return 0;
}

void CPROC UDPRead( PCLIENT pc, POINTER buffer, int size, SOCKADDR *sa )
{
	do{
		if( !buffer )
		{
			buffer = Allocate( 2048 );
		}
		else
		{
         char restart = 0;
			char *inbuf;
         int bCaptureOutput = 0;
			int bNulArgs = 0;
			char **args = NULL;
			char *seqbuf;
         int hide_process = 0;
			PRTASK restart_info = NULL;
         int capture_port = 3013;
			inbuf = (char*)buffer;
			inbuf[size] = 0;
			//lprintf( "Received..." );
			//LogBinary( (P_8)buffer, size );
			while( inbuf[0] == '~' )
			{
				if( StrCaseCmpEx( inbuf+1, "capture", 7 ) == 0 )
				{
					int port = 0;
					char c;
					bCaptureOutput = 1;
					inbuf += 8;
					while( (c = inbuf[0]),(c >='0' && c <= '9' ) )
					{
						port *= 10;
						port += c - '0';
						inbuf++;
					}
					if( port )
						capture_port = port;
				}
				if( StrCaseCmpEx( inbuf+1, "hide", 4 ) == 0 )
				{
					hide_process = 1;
					inbuf += 5;
				}
			}
  			/* check to see if the message has a class assicated, if not, process as normal */
			if( inbuf[0] == '$' )
			{
				INDEX idx;
				CTEXTSTR class_name;
				if( class_names )
				{
					LIST_FORALL( class_names, idx, CTEXTSTR, class_name )
					{
						int len;
						/* has a class name, do we? */
						// message has class, we have a class, check if it matches
						if( StrCaseCmpEx( inbuf + 1, class_name, len = strlen( class_name ) ) == 0 )
						{
							if( inbuf[1+len] == '^' )
								break; // skip down to the ReadUDP, don't process this packet.
						}
					}
					if( !class_name )
						break;  // skip over remaining processing...
				}
				else
					break;
				inbuf = strchr( inbuf, '^' ); /* skip over class to sequence... */

			}
			if( !inbuf ) // malformed packet.
				break;
			if( inbuf[0] == '^' )
			{
				bNulArgs = 1;
				inbuf++;
			}
			seqbuf = inbuf;
			inbuf = strchr( seqbuf, '#' );
			if( !inbuf )
			{
				inbuf = strchr( seqbuf, '@' );
				if( inbuf )
               restart = 1;
			}
			if( inbuf )
			{
				inbuf[0] = 0;
				inbuf++;
				if( strchr( inbuf, '=' ) )
				{
					// build as a environvar...
					//also continue scanning for next part of real command
				}
				//LogBinary( (P_8)buffer, size );
				do
				{
					// keep a list of all sequences... and expire them at some point?
					_32 packet_sequence = atol( seqbuf );
					{
						struct sequence_data *pSeq = NULL;
						INDEX idx;
						LIST_FORALL( sequences, idx, pSequence, pSeq )
						{
							if( pSeq->sequence == packet_sequence )
							{
                        if( pSeq->packet_length == size )
									if( MemCmp( pSeq->packet, buffer, size ) == 0 )
										break;
							}
						}
						if( pSeq ) // bail out of do{}while(0)
							break;
						{
							struct sequence_data *new_seq = New( struct sequence_data );
							new_seq->sequence = packet_sequence;
							new_seq->tick = GetTickCount();
							new_seq->packet = Allocate( size );
                     new_seq->packet_length = size;
                     MemCpy( new_seq->packet, buffer, size );

							AddLink( &sequences, new_seq );
						}
						// need to do this so that we don't get a command prompt from "system"
						if( bNulArgs )
						{
							char *arg;
							int count = 0;
							arg = inbuf;
							while( ((PTRSZVAL)arg-(PTRSZVAL)buffer) < (PTRSZVAL)size )
							{
								count++;
								arg += strlen( arg ) + 1;
							}
							args = (char**)Allocate( (count+1) * sizeof( char * ) );

							arg = inbuf;
							count = 0;
							while( ((PTRSZVAL)arg-(PTRSZVAL)buffer) < (PTRSZVAL)size )
							{
								args[count++] = arg;
								arg += strlen( arg ) + 1;
							}
							args[count++] = NULL; // null terminate list of args..

							if( restart && !bCaptureOutput )
							{
								// capture output sends back to a host program.
								// will NOT do this reconnect for restartables...
                        // therefore it is not restartable.
								restart_info = New( RTASK );
								restart_info->fast_restart_count = 0;
                        restart_info->prior_tick = GetTickCount();
								restart_info->args = (PCTEXTSTR)args;
								restart_info->program = StrDup( inbuf );
								restart_info->flags.bLogOutput = bLogOutput;
								restart_info->flags.bHide = hide_process;
                        AddLink( &restarts, restart_info );
							}
							if( bCaptureOutput || bLogOutput)
							{
								if( bCaptureOutput )
								{
									struct thread_args *tmp_args = New( struct thread_args );
									sa = DuplicateAddress( sa );
									SetAddressPort( sa, capture_port );
									tmp_args->inbuf = StrDup( inbuf );
									tmp_args->args = args;
									tmp_args->hide_process = hide_process;
									tmp_args->restart_info = restart_info;
									tmp_args->sa = sa;
									tmp_args->bCaptureOutput = bCaptureOutput;
									tmp_args->restart = restart;
									lprintf( "Capturing task output to send back... begin back connect." );
									ThreadTo( RemoteBackConnect, (PTRSZVAL)tmp_args );
								}
								else
								{
									PTASK_INFO task;
									task = LaunchPeerProgramExx( inbuf, NULL
																		, (PCTEXTSTR)args
																		, hide_process?0:LPP_OPTION_DO_NOT_HIDE
																		, GetTaskOutput
																		, restart_info?RestartTask:TaskEnded
																		, (PTRSZVAL)(restart_info?((PTRSZVAL)restart_info):0)
																		 DBG_SRC
																		);
								}
#if 0
								PCLIENT pc_output = NULL;
								if( bCaptureOutput )
								{
									SetAddressPort( sa, capture_port );
									//DumpAddr( "connect to", sa );
									pc_output = OpenTCPClientAddrEx( sa
																			 , RemoteSent // no reception
																			 , RemoteClosed // no close notice
																			 , NULL ); // no write notic
								}
								task = LaunchPeerProgramExx(inbuf, NULL
																	, (PCTEXTSTR)args
																	, hide_process?0:LPP_OPTION_DO_NOT_HIDE
																	, GetTaskOutput
																	, restart_info?RestartTask:TaskEnded
																	, (PTRSZVAL)(restart_info?((PTRSZVAL)restart_info):((PTRSZVAL)pc_output))
																	 DBG_SRC
																	);
								if( bCaptureOutput )
								{
                           SetNetworkLong( pc_output, 1, (PTRSZVAL)task );
								}
#endif
							}
							else
							{
								LaunchProgramEx( inbuf, NULL
													, (PCTEXTSTR)args
													, restart_info?RestartTask:TaskEnded
													, (PTRSZVAL)restart_info );
								if( !restart )
								{
									// restartable tasks want to keep the args...
									Release( args );
								}
							}
						}
						else
						{
#ifdef __cplusplus
							::
#endif
							system( inbuf ); // old way...
						}
					}
				}
				while(0);
			}
			else
			{
				lprintf( "Running [%s]", inbuf );
#ifdef __cplusplus
							::
#endif
				system( inbuf );
			}
		}
      // at this point, just fall out... while(0) might not continue; correctly...
		break;
	} while( 1 );
	ReadUDP( pc, buffer, 2048 );
}

int BeginNetwork( void )
{
	if( !NetworkWait( NULL, 48, 2 ) )
		return 0;
	pcListen = ServeUDP( NULL, 3006, UDPRead, NULL );
	if( !pcListen )
	{
      lprintf( "Failed to listen on 3006" );
		return 0;
	}
   UDPEnableBroadcast( pcListen, TRUE );
   lprintf( "listening on 3006" );
	return 1;
}

void  SetTaskLogOutput(void)
{
   bLogOutput = TRUE;
}



#if defined( MILK_PLUGIN ) || defined( ISHELL_PLUGIN )
#if defined( MILK_PLUGIN )
#include <milk_export.h>
#endif

#if defined( ISHELL_PLUGIN )
#include <intershell_export.h>
#endif

PRELOAD( InitLaunchpad )
{
	if( !BeginNetwork() )
	{
      xlprintf(LOG_ALWAYS)( "Fatal error starting network portion of LaunchPad" );
		return;
	}
	AddTimer( 2500, ExpireSequences, 0 );
}
#else
#ifdef __WINDOWS__
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nCmdShow )
{
	int argc;
	char **argv;
	ParseIntoArgs( GetCommandLine(), &argc, &argv );
	{
		int n;
		for( n = 0; n < argc; n++ )
         lprintf( "Arg %d=%s", n, argv[n] );
	}
#ifndef __NO_GUI__
   TerminateIcon();
	RegisterIcon( "PadIcon" );
#endif
#else
	int main( int argc, char **argv )
#endif
	{
		{
			int arg_ofs;
			for( arg_ofs = 1; arg_ofs < argc; arg_ofs++ )
			{
				if( argv[arg_ofs][0] == '-' )
				{
					switch( argv[arg_ofs][1] )
					{
					case 'c':
						if( argv[arg_ofs][2] )
							AddLink( &class_names, StrDup( argv[arg_ofs]+2 ) );
						else
						{
							arg_ofs++;
							lprintf( "Dup string %s", argv[arg_ofs] );
							AddLink( &class_names, StrDup( argv[arg_ofs] ) );
						}
						break;
					case 'l':
						bLogOutput = 1;
						SetSystemLog( SYSLOG_AUTO_FILE, NULL );
						SetSystemLoggingLevel( 10000 );
						SystemLogTime( SYSLOG_TIME_ENABLE|SYSLOG_TIME_LOG_DAY|SYSLOG_TIME_HIGH );
						{
							FLAGSET( options, SYSLOG_OPT_MAX );
							int n;
							for( n = 0; n < SYSLOG_OPT_MAX; n++ )
							{
								RESETFLAG( options, n );
							}
							SETFLAG( options, SYSLOG_OPT_OPEN_BACKUP );
							SetSyslogOptions( options );
						}
					}
				}
			}
		}
                lprintf( "Usage: %s\n [-c <class_name>...]  [-l]\n"
                        " -l enables logging\n"
                        " as many class names as you awnt may be added.\n"
				  "  If a class_name is specified commands for specified class will be executed.\n"
				  "  If NO class_name is specified all received commands will be executed.\n"
				  "  If no class is specified in the message, it is also launched.", argv[0] );
		if( !BeginNetwork() )
		{
			lprintf( "Fatal error starting network portion of LaunchPad" );
			return 0;
		}
#ifdef _DEBUG
		{
			INDEX idx;
			CTEXTSTR class_name;
			LIST_FORALL( class_names, idx, CTEXTSTR, class_name )
			{
				lprintf( "Listening for class %s", class_name );
			}
		}
#endif
		AddTimer( 2500, ExpireSequences, 0 );
		while(1)
			WakeableSleep( SLEEP_FOREVER );

		return 0;
	}
#ifdef __WINDOWS__
}
#endif
#endif
