#define TASK_INFO_DEFINED
#include <stdhdrs.h>
#include <sack_types.h>
#include <deadstart.h>
#include <sharemem.h>

#include <timers.h>
#include <filesys.h>

#ifdef __LINUX__
#include <sys/poll.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
extern char **environ;
#endif


#include <system.h>


//--------------------------------------------------------------------------

SACK_SYSTEM_NAMESPACE

#include "taskinfo.h"

typedef struct task_info_tag TASK_INFO;



//--------------------------------------------------------------------------

#ifdef WIN32
static int DumpError( void )
{
	lprintf( WIDE("Failed create process:%d"), GetLastError() );
   return 0;
}
#endif

//--------------------------------------------------------------------------
#ifdef __LINUX__
int CanRead( int handle )
{
	fd_set n;
	int rval;
	struct timeval tv;
	FD_ZERO( &n );
	FD_SET( handle, &n );
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	rval = select( handle+1, &n, NULL, NULL, &tv );
	//lprintf( "select : %d %d\n", rval, handle );
	if( rval > 0 )
		return TRUE;
	return  FALSE;
}
#endif

//--------------------------------------------------------------------------
extern PTRSZVAL CPROC WaitForTaskEnd( PTHREAD pThread );


static PTRSZVAL CPROC HandleTaskOutput(PTHREAD thread )
{
	PTASK_INFO task = (PTASK_INFO)GetThreadParam( thread );
//cpg26Dec2006   PHANDLEINFO phi = &task->hStdOut;
	{
		// read input from task, montiro close and dispatch TaskEnd Notification also.


		//PTRSZVAL CPROC CommandInputThread( PTHREAD pThread )
		{
#ifdef _WIN32
			HANDLE handles[2];
#endif
			PHANDLEINFO phi = &task->hStdOut;
			PTEXT pInput = SegCreate( 4096 );
			int done, lastloop, do_start = 1, check_time;
			_32 start;
         Hold( task );
#ifdef _WIN32
			handles[0] = phi->handle;
         handles[1] = task->pi.hProcess;
#endif
#ifdef _DEBUG
			{
				//	   DECLTEXT( msg, "Started system input thread!" );
				//   	EnqueLink( phi->pdp->&ps->Command->Output, &msg );
			}
#endif
			done = lastloop = FALSE;
			do
			{
//cpg26Dec2006            _32 result;
				_32 dwRead, dwAvail;
				if( done )
					lastloop = TRUE;
				if( do_start )
				{
					start = GetTickCount() + 50;
					do_start = FALSE;
					check_time = TRUE;
				}
#ifdef _WIN32
            //lprintf( "waiting..." );
				//result = WaitForMultipleObjects( 2, handles, FALSE, INFINITE );
            //lprintf( "Result is %ld", result );
            //if( result == WAIT_OBJECT_0 )
				while( PeekNamedPipe( phi->handle
						 				  , NULL, 0, NULL, (LPDWORD)&dwAvail, NULL ) &&
						 ( dwAvail > 0 )
					  )
#else
				while( CanRead( phi->handle ) )
#endif
				{
#ifdef _WIN32
					//lprintf( WIDE("reading...") );
					ReadFile( phi->handle
							  , GetText( pInput ), GetTextSize( pInput ) - 1
							  , (LPDWORD)&dwRead, NULL);  //read the  pipe
#else
                                        //lprintf( "reading..." );
					dwRead = read( phi->handle
									 , GetText( pInput )
									 , GetTextSize( pInput ) - 1 );
					if( !dwRead )
					{
#ifdef _DEBUG
                                            //lprintf( "Ending system thread because of broke pipe! %d", errno );
#endif
#ifdef WIN32
                                            continue;
#else
                                            //lprintf( "0 read = pipe failure." );
                                            break;
#endif
					}
#endif
                                        //lprintf( "result %d", dwRead );
					GetText( pInput )[dwRead] = 0;
					pInput->data.size = dwRead;
               //LogBinary( GetText( pInput ), GetTextSize( pInput ) );
					if( task->OutputEvent )
                                        {
						task->OutputEvent( task->psvEnd, task, GetText( pInput ), GetTextSize( pInput ) );
					}
					pInput->data.size = 4096;
					do_start = TRUE;
					check_time = FALSE;
				}
#ifdef _WIN32
#endif
				//allow a minor time for output to be gathered before sending
				// partial gathers...
#ifdef _WIN32
				if( WaitForSingleObject( task->pi.hProcess, 0 ) )
					Sleep(1);
				else
				{
					// Ending system thread because of process exit!
					if( !done )
						lprintf( WIDE("Launched task has exited...") );
					else
						lprintf( WIDE("Launched task has exited and final output checked...") );
					done = TRUE;
				}
#else
				//if( !dwRead )
				//	break;
				if( task->pid == waitpid( task->pid, NULL, WNOHANG ) )
				{
					Log( "Setting done event on system reader." );
					done = TRUE; // do one pass to make sure we completed read
				}
				else
                                {
                                    lprintf( "process active..." );
					Relinquish();
				}
#endif
			}
			while( !lastloop );
#ifdef _DEBUG
			if( lastloop )
			{
				//DECLTEXT( msg, "Ending system thread because of process exit!" );
				//EnqueLink( phi->pdp->&ps->Command->Output, &msg );
			}
			else
			{
				//DECLTEXT( msg, "Guess we exited from broken pipe" );
				//EnqueLink( phi->pdp->&ps->Command->Output, &msg );
			}
#endif
			LineRelease( pInput );
#ifdef _WIN32
			CloseHandle( phi->handle );
			//lprintf( "Closing process handle %p", task->pi.hProcess );
			phi->hThread = 0;
#else
			//close( phi->handle );
			close( task->hStdIn.pair[1] );
			close( task->hStdOut.pair[0] );
         //close( task->hStdErr.pair[0] );
#define INVALID_HANDLE_VALUE -1
#endif
			if( phi->handle == task->hStdIn.handle )
				task->hStdIn.handle = INVALID_HANDLE_VALUE;
			phi->handle = INVALID_HANDLE_VALUE;
#ifdef _WIN32
			GetExitCodeProcess( task->pi.hProcess, &task->exitcode );
#endif

			if( task->EndNotice )
				task->EndNotice( task->psvEnd, task );
#ifdef _WIN32
			CloseHandle( task->pi.hProcess );
			task->pi.hProcess = 0;
			CloseHandle( task->pi.hThread );
			task->pi.hThread = 0;
#endif
			//WakeAThread( phi->pdp->common.Owner );
			return 0xdead;
		}
	}
}

//--------------------------------------------------------------------------

static int FixHandles( PTASK_INFO task )
{
#ifdef WIN32
	if( task->pi.hProcess )
		CloseHandle( task->pi.hProcess );
	task->pi.hProcess = 0;
	if( task->pi.hProcess )
		CloseHandle( task->pi.hThread );
	task->pi.hThread = 0;
#endif
   return 0; // must return 0 so expression continues
}

//--------------------------------------------------------------------------

// Run a program completely detached from the current process
// it runs independantly.  Program does not suspend until it completes.
// No way at all to know if the program works or fails.
SYSTEM_PROC( PTASK_INFO, LaunchPeerProgramExx )( CTEXTSTR program, CTEXTSTR path, PCTEXTSTR args
															  , int flags
															  , TaskOutput OutputHandler
															  , TaskEnd EndNotice
															  , PTRSZVAL psv
																DBG_PASS
															  )
{
	PTASK_INFO task;
	if( program && program[0] )
	{
#ifdef WIN32
	PVARTEXT pvt = VarTextCreateEx( DBG_VOIDRELAY );
	PTEXT cmdline;
	PTEXT final_cmdline;
	int first = TRUE;
	TEXTCHAR saved_path[256];
	task = (PTASK_INFO)AllocateEx( sizeof( TASK_INFO ) DBG_RELAY );
	MemSet( task, 0, sizeof( TASK_INFO ) );
	task->psvEnd = psv;
	task->EndNotice = EndNotice;
	if( StrChr( program, ' ' ) )
		vtprintf( pvt, WIDE("\"%s\""), program );
	else
		vtprintf( pvt, WIDE("%s"), program );
	if( args && args[0] )// arg[0] is passed with linux programs, and implied with windows.
      args++;
	while( args && args[0] )
	{
		if( StrChr( args[0], ' ' ) )
			vtprintf( pvt, WIDE(" \"%s\""), args[0] );
		else
			vtprintf( pvt, WIDE(" %s"), args[0] );
      first = FALSE;
      args++;
	}
	cmdline = VarTextGet( pvt );
	vtprintf( pvt, "cmd.exe %s", GetText( cmdline ) );
   final_cmdline = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	MemSet( &task->si, 0, sizeof( STARTUPINFO ) );
	task->si.cb = sizeof( STARTUPINFO );
	GetCurrentPath( saved_path, sizeof( saved_path ) );
	SetCurrentPath( path );
	task->OutputEvent = OutputHandler;
   if( OutputHandler )
	{
		SECURITY_ATTRIBUTES sa;

		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;
		sa.nLength = sizeof( sa );

		CreatePipe( &task->hReadOut, &task->hWriteOut, &sa, 0 );
		//CreatePipe( &hReadErr, &hWriteErr, &sa, 0 );
		CreatePipe( &task->hReadIn, &task->hWriteIn, &sa, 0 );
		task->si.hStdInput = task->hReadIn;
		task->si.hStdError = task->hWriteOut;
		task->si.hStdOutput = task->hWriteOut;
		task->si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		if( !( flags & LPP_OPTION_DO_NOT_HIDE ) )
			task->si.wShowWindow = SW_HIDE;
		else
			task->si.wShowWindow = SW_SHOW;
	}
	else
	{
		task->si.dwFlags |= STARTF_USESHOWWINDOW;
		task->si.wShowWindow = SW_SHOW;
	}
	if( ( CreateProcess( NULL //program
						  , GetText( cmdline )
						  , NULL, NULL, TRUE
						  , 0//CREATE_NEW_PROCESS_GROUP
						  , NULL
						  , path
						  , &task->si
						  , &task->pi ) || FixHandles(task) || DumpError() ) ||
		( CreateProcess( program
						 , GetText( cmdline )
						 , NULL, NULL, TRUE
						 , 0//CREATE_NEW_PROCESS_GROUP
						 , NULL
						 , path
						 , &task->si
						 , &task->pi ) || FixHandles(task) || DumpError() ) ||
		( CreateProcess( program
						 , NULL // GetText( cmdline )
						 , NULL, NULL, TRUE
						 , 0//CREATE_NEW_PROCESS_GROUP
						 , NULL
						 , path
						 , &task->si
						 , &task->pi ) || FixHandles(task) || DumpError() ) ||
		( CreateProcess( "cmd.exe"
						 , GetText( final_cmdline )
						 , NULL, NULL, TRUE
						 , 0//CREATE_NEW_PROCESS_GROUP
						 , NULL
						 , path
						 , &task->si
						 , &task->pi ) || FixHandles(task) || DumpError() )
	  )
	{
		//CloseHandle( task->hReadIn );
		//CloseHandle( task->hWriteOut );
		lprintf( WIDE("Success running %s[%s](%p): %d"), program, GetText( cmdline ), task->pi.hProcess, GetLastError() );
		if( OutputHandler )
		{
			task->hStdIn.handle 	= task->hWriteIn;
			task->hStdIn.pLine 	= NULL;
			//task->hStdIn.pdp 		= pdp;
			task->hStdIn.hThread  = 0;
			task->hStdIn.bNextNew = TRUE;

			task->hStdOut.handle 	 = task->hReadOut;
			task->hStdOut.pLine 	 = NULL;
			//task->hStdOut.pdp 		 = pdp;
			task->hStdOut.bNextNew = TRUE;
			task->hStdOut.hThread  = ThreadTo( HandleTaskOutput, (PTRSZVAL)task );
		}
		else
		{
			//task->hThread =
				ThreadTo( WaitForTaskEnd, (PTRSZVAL)task );
		}
	}
	else
	{
		lprintf( WIDE("Failed to run %s[%s]: %d"), program, GetText( cmdline ), GetLastError() );
		CloseHandle( task->hWriteIn );
		CloseHandle( task->hReadIn );
		CloseHandle( task->hWriteOut );
		CloseHandle( task->hReadOut );
      CloseHandle( task->pi.hProcess );
      CloseHandle( task->pi.hThread );
      Release( task );
      task = NULL;
	}
	LineRelease( cmdline );
	LineRelease( final_cmdline );
	SetCurrentPath( saved_path );
	return task;
#endif
#ifdef __LINUX__
	{
      pid_t newpid;
		TEXTCHAR saved_path[256];
		task = (PTASK_INFO)Allocate( sizeof( TASK_INFO ) );
		MemSet( task, 0, sizeof( TASK_INFO ) );
		task->psvEnd = psv;
		task->EndNotice = EndNotice;
                task->OutputEvent = OutputHandler;
		{
			pipe(task->hStdIn.pair);
			task->hStdIn.handle = task->hStdIn.pair[1];

			pipe(task->hStdOut.pair);
			task->hStdOut.handle = task->hStdOut.pair[0];
		}

      // always have to thread to taskend so waitpid can clean zombies.
                ThreadTo( WaitForTaskEnd, (PTRSZVAL)task );
                if( path )
                {
                    GetCurrentPath( saved_path, sizeof( saved_path ) );
                    SetCurrentPath( path );
                }
		if( !( newpid = fork() ) )
		{
			// in case exec fails, we need to
			// drop any registered exit procs...
			//close( task->hStdIn.pair[1] );
			//close( task->hStdOut.pair[0] );
			//close( task->hStdErr.pair[0] );

			dup2( task->hStdIn.pair[0], 0 );
			dup2( task->hStdOut.pair[1], 1 );
                        dup2( task->hStdOut.pair[1], 2 );

			DispelDeadstart();

                        //usleep( 100000 );
                        execve( program, (char *const*)args, environ );
                        lprintf( "Direct execute failed... trying along path..." );
                        {
                            char *tmp = StrDup( getenv( "PATH" ) );
                            char *tok;
                            for( tok = strtok( tmp, ":" ); tok; tok = strtok( NULL, ":" ) )
                            {
                                char fullname[256];
                                snprintf( fullname, sizeof( fullname ), "%s/%s", tok, program );

                                lprintf( "program:[%s]", fullname );
                                ((char**)args)[0] = fullname;
                                execve( fullname, (char*const*)args, environ );
                            }
                            Release( tmp );
                        }
			close( task->hStdIn.pair[0] );
			close( task->hStdOut.pair[1] );
			//close( task->hWriteErr );
         close( 0 );
         close( 1 );
         close( 2 );
         lprintf( "exec failed - and this is ALLL bad... %d", errno );
         //DebugBreak();
			// well as long as this runs before
			// the other all will be well...
			task = NULL;
			// shit - what can I do now?!
			exit(0); // just in case exec fails... need to fault this.
		}
		else
		{
			close( task->hStdIn.pair[0] );
			close( task->hStdOut.pair[1] );
		}
      ThreadTo( HandleTaskOutput, (PTRSZVAL)task );
		task->pid = newpid;
      lprintf( WIDE("Forked, and set the pid..") );
		// how can I know if the command failed?
		// well I can't - but the user's callback will be invoked
      // when the above exits.
                if( path )
                {
                    // if path is NULL we didn't change the path...
                    SetCurrentPath( saved_path );
                }
		return task;
	}
#endif
	}
	return FALSE;
}

int vpprintf( PTASK_INFO task, CTEXTSTR format, va_list args )
{
	PVARTEXT pvt = VarTextCreate();
   PTEXT output;
	vvtprintf( pvt, format, args );
   output = VarTextGet( pvt );
	if(
#ifdef _WIN32
		WaitForSingleObject( task->pi.hProcess, 0 )
#else
		task->pid != waitpid( task->pid, NULL, WNOHANG )
#endif
	  )
	{
#ifdef _WIN32
		_32 dwWritten;
#endif
      //lprintf( "Allowing write to process pipe..." );
		{
			PTEXT seg = output;
			while( seg )
			{
#ifdef _WIN32
            //LogBinary( GetText( seg )
		      //			, GetTextSize( seg ) );
   	   	WriteFile( task->hStdIn.handle
      					, GetText( seg )
		      			, GetTextSize( seg )
      					, (LPDWORD)&dwWritten
      					, NULL );
#else
				{
					struct pollfd pfd = { task->hStdIn.handle, POLLHUP|POLLERR, 0 };
					if( poll( &pfd, 1, 0 ) &&
						 pfd.revents & POLLERR )
					{
						Log( "Pipe has no readers..." );
							break;
					}
               LogBinary( (_8*)GetText( seg ), GetTextSize( seg ) );
					write( task->hStdIn.handle
						 , GetText( seg )
						 , GetTextSize( seg ) );
				}
#endif
				seg = NEXTLINE(seg);
			}
		}
      LineRelease( output );
	}
	else
	{
		lprintf( WIDE("Task has ended, write  aborted.") );
	}
	VarTextDestroy( &pvt );
   return 0;
}

SYSTEM_PROC( PTASK_INFO, LaunchPeerProgramEx )( CTEXTSTR program, CTEXTSTR path, PCTEXTSTR args
															 , TaskOutput OutputHandler
															 , TaskEnd EndNotice
															 , PTRSZVAL psv
                                                DBG_PASS
															  )
{
   return LaunchPeerProgramExx( program, path, args, 0, OutputHandler, EndNotice, psv DBG_RELAY );
}

int pprintf( PTASK_INFO task, CTEXTSTR format, ... )
{
	va_list args;
	va_start( args, format );
	return vpprintf( task, format, args );
}


SACK_SYSTEM_NAMESPACE_END


//-------------------------------------------------------------------------

