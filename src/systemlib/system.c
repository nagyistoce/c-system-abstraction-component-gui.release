#define TASK_INFO_DEFINED
#include <stdhdrs.h>
#include <sack_types.h>
#include <deadstart.h>
#include <sharemem.h>
#include <sqlgetoption.h>
#include <timers.h>
#include <filesys.h>

#ifdef __LINUX__
#include <sys/wait.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
extern char **environ;
#endif


#include <system.h>


#ifdef __cplusplus
using namespace sack::timers;
#endif

//--------------------------------------------------------------------------
struct task_info_tag;

SACK_SYSTEM_NAMESPACE
//typedef void (CPROC*TaskEnd)(PTRSZVAL, struct task_info_tag *task_ended);

#include "taskinfo.h"

typedef struct task_info_tag TASK_INFO;

static struct {
	TEXTCHAR *load_path;
	struct system_local_flags{
		BIT_FIELD bLog : 1;
	} flags;
} local_systemlib;

#define l local_systemlib

#ifdef HAVE_ENVIRONMENT
CTEXTSTR OSALOT_GetEnvironmentVariable(CTEXTSTR name)
{

#ifdef __WINDOWS__
	static int env_size;
	static TEXTCHAR *env;
	int size;
	if( size = GetEnvironmentVariable( name, NULL, 0 ) )
	{
		if( size > env_size )
		{
			if( env )
				Release( (POINTER)env );
			env = NewArray( TEXTCHAR, size + 10 );
			env_size = size + 10;
		}
		if( GetEnvironmentVariable( name, env, env_size ) )
			return env;
	}
	return NULL;
#else
	return getenv( name );
#endif
}

void OSALOT_AppendEnvironmentVariable(CTEXTSTR name, CTEXTSTR value)
{
#if defined( __WINDOWS__ ) || defined( __CYGWIN__ )
	TEXTCHAR *oldpath;
   TEXTCHAR *newpath;
	{
		int oldlen;
		oldpath = NewArray( TEXTCHAR, oldlen = ( GetEnvironmentVariable( name, NULL, 0 ) + 1 ) );
		GetEnvironmentVariable( name, oldpath, oldlen );
	}
	newpath = NewArray( TEXTCHAR, (_32)(strlen( oldpath ) + 2 + strlen(value)) );
	sprintf( newpath, WIDE("%s;%s"), oldpath, value );
	SetEnvironmentVariable( name, newpath );
	Release( newpath );
   Release( oldpath );
#else
	TEXTCHAR *oldpath = getenv( name );
	TEXTCHAR *newpath;
	newpath = NewArray( TEXTCHAR, strlen( oldpath ) + strlen( value ) + 1 );
	sprintf( newpath, WIDE("%s:%s"), oldpath, newpath );
	setenv( name, newpath, TRUE );
   Release( newpath );
#endif
}


void OSALOT_PrependEnvironmentVariable(CTEXTSTR name, CTEXTSTR value)
{
#if defined( __WINDOWS__ )|| defined( __CYGWIN__ )
	TEXTCHAR *oldpath;
   TEXTCHAR *newpath;
	{
		int oldlen;
		oldpath = NewArray( TEXTCHAR, oldlen = ( GetEnvironmentVariable( name, NULL, 0 ) + 1 ) );
		GetEnvironmentVariable( name, oldpath, oldlen );
	}
	newpath = NewArray( TEXTCHAR, (_32)(strlen( oldpath ) + 2 + strlen(value)) );
	sprintf( newpath, WIDE("%s;%s"), value, oldpath );
	SetEnvironmentVariable( name, newpath );
	Release( newpath );
   Release( oldpath );
#else
	TEXTCHAR *oldpath = getenv( name );
	TEXTCHAR *newpath;
	newpath = NewArray( TEXTCHAR, strlen( oldpath ) + strlen( value ) + 1 );
	sprintf( newpath, WIDE("%s:%s"), value, oldpath );
	setenv( name, newpath, TRUE );
   Release( newpath );
#endif
}
#endif

static void SetupSystemServices( void )
{
#ifdef _WIN32
	{

#ifdef HAVE_ENVIRONMENT
		int length;
#endif
		TEXTCHAR filepath[256];
		TEXTCHAR *ext, *e1, *e2;//, *filename;
		GetModuleFileName( NULL, filepath, sizeof( filepath ) );
		ext = (TEXTSTR)StrRChr( (CTEXTSTR)filepath, '.' );
		if( ext )
			StrCpy( ext, WIDE(".log") );
		e1 = (TEXTSTR)StrRChr( (CTEXTSTR)filepath, '\\' );
		e2 = (TEXTSTR)StrRChr( (CTEXTSTR)filepath, '/' );
		if( e1 && e2 && ( e1 > e2 ) )
			e1[0] = 0;
		else if( e1 && e2 )
			e2[0] = 0;
		else if( e1 )
			e1[0] = 0;
		else if( e2 )
			e2[0] = 0;

		l.load_path = StrDup( filepath );

#ifdef HAVE_ENVIRONMENT
		{
			TEXTCHAR *oldpath;
			TEXTCHAR *newpath;
			if( !SetEnvironmentVariable( WIDE("MY_LOAD_PATH"), filepath ) )
			{
				//lprintf( WIDE("FAILED SET LOADPATH") );
			}
			//lprintf( WIDE("Defined MY_LOAD_PATH as %s %s"), filepath, getenv( WIDE("MY_LOAD_PATH") ) );
			{
				int oldlen;
				oldpath = NewArray( TEXTCHAR, oldlen = ( GetEnvironmentVariable( WIDE("PATH"), NULL, 0 ) + 1 ) );
				GetEnvironmentVariable( WIDE("PATH"), oldpath, oldlen );
			}
			newpath = NewArray( TEXTCHAR, length=(_32)(strlen( oldpath ) + 2 + strlen(filepath)) );
			snprintf( newpath, length, WIDE("%s;%s"), filepath, oldpath );
			SetEnvironmentVariable( WIDE("PATH"), newpath );
			//lprintf( WIDE("Updated is now %s"), getenv( WIDE("PATH") ) );
			Release( newpath );
			Release( oldpath );

			GetCurrentPath( filepath, sizeof( filepath ) );
			SetEnvironmentVariable( WIDE( "MY_WORK_PATH" ), filepath );
		}
#endif
	}
#else
// this might be clever to do, auto export the LD_LIBRARY_PATH
// but if we loaded this library, then didn't we already have a good path?
			  // use /proc/self to get to cmdline
			  // which has the whole invokation of this process.
	{
	/* #include unistd.h, stdio.h, string.h */
		{
			char buf[256], *pb;
			int n;
			n = readlink("/proc/self/exe",buf,256);
			if( n >= 0 )
			{
				buf[n]=0; //linux
				if( !n )
				{
					strcpy( buf, WIDE(".") );
					buf[ n = readlink( WIDE("/proc/curproc/"),buf,256)]=0; // fbsd
				}
			}
			else
				strcpy( buf, WIDE(".")) ;
			pb = strrchr(buf,'/');
			if( pb )
				pb[0]=0;
			//lprintf( WIDE("My execution: %s"), buf);
			l.load_path = StrDup( buf );
			setenv( WIDE("MY_LOAD_PATH"), buf, TRUE );
			//strcpy( pMyPath, buf );

			GetCurrentPath( buf, sizeof( buf ) );
			setenv( WIDE( "MY_WORK_PATH" ), buf, TRUE );
		}
	{
		TEXTCHAR *oldpath;
      TEXTCHAR *newpath;
		oldpath = getenv( "LD_LIBRARY_PATH" );
		if( oldpath )
		{
			newpath = (char*)Allocate( (_32)((oldpath?strlen( oldpath ):0) + 2 + strlen(l.load_path)) );
			sprintf( newpath, WIDE("%s:%s"), l.load_path
					 , oldpath );
			setenv( WIDE("LD_LIBRARY_PATH"), newpath, 1 );
			Release( newpath );
		}
	}
	{
		TEXTCHAR *oldpath;
      TEXTCHAR *newpath;
		oldpath = getenv( "PATH" );
		if( oldpath )
		{
			newpath = (char*)Allocate( (_32)((oldpath?strlen( oldpath ):0) + 2 + strlen(l.load_path)) );
			sprintf( newpath, WIDE("%s:%s"), l.load_path
					 , oldpath );
			setenv( WIDE("PATH"), newpath, 1 );
			Release( newpath );
		}
	}
		//<x`int> rathar: main() { char buf[1<<7]; buf[readlink("/proc/self/exe",buf,1<<7)]=0; puts(buf); }
		//<x`int> main() {  }
		//<x`int>
	}
#endif
}

#ifdef __WATCOM_CPLUSPLUS__
#pragma initialize 36
#endif
PRIORITY_PRELOAD( SetupPath, OSALOT_PRELOAD_PRIORITY )
{
   SetupSystemServices();
}

#ifndef __NO_OPTIONS__
PRELOAD( SetupSystemOptions )
{
	l.flags.bLog = SACK_GetProfileIntEx( GetProgramName(), "SACK/System/Enable Logging", 0, TRUE );
}
#endif



//--------------------------------------------------------------------------

#ifdef WIN32
#ifndef _M_CEE_PURE
BOOL CALLBACK CheckWindow( HWND hWnd, LPARAM lParam )
{
	_32 idThread, idProcess;
	PTASK_INFO task = (PTASK_INFO)lParam;
	idThread = GetWindowThreadProcessId( hWnd, (LPDWORD)&idProcess );
	if( task->pi.dwProcessId == idProcess )
		PostThreadMessage( idThread, WM_QUIT, 0xD1E, 0 );
	return TRUE;
}
#endif

//--------------------------------------------------------------------------

void CPROC EndTaskWindow( PTASK_INFO task )
{
	EnumWindows( CheckWindow, (LPARAM)task );
}
#endif

//--------------------------------------------------------------------------

LOGICAL CPROC StopProgram( PTASK_INFO task )
{
#ifndef UNDER_CE
	if( !GenerateConsoleCtrlEvent( CTRL_C_EVENT, task->pi.dwProcessId ) )
	{
      lprintf( "Failed to send CTRL_C_EVENT %d", GetLastError() );
		if( !GenerateConsoleCtrlEvent( CTRL_BREAK_EVENT, task->pi.dwProcessId ) )
		{
			lprintf( "Failed to send CTRL_BREAK_EVENT %d", GetLastError() );
         	return FALSE;
		}
	}
#endif
    return TRUE;
}

PTRSZVAL CPROC TerminateProgram( PTASK_INFO task )
{
	if( task )
	{
#if defined( WIN32 )
		int bDontCloseProcess = 0;
#endif
		if( !task->flags.closed )
		{
			task->flags.closed = 1;
#if defined( WIN32 )
			if( WaitForSingleObject( task->pi.hProcess, 0 ) != WAIT_OBJECT_0 )
			{
				// try using ctrl-c, ctrl-break to end process...
				if( !StopProgram( task ) )
				{
					// if ctrl-c fails, try finding the window, and sending exit (systray close)
					EndTaskWindow( task );
				}
				if( WaitForSingleObject( task->pi.hProcess, 500 ) != WAIT_OBJECT_0 )
				{
					if( WaitForSingleObject( task->pi.hProcess, 500 ) != WAIT_OBJECT_0 )
					{
						bDontCloseProcess = 1;
						if( !TerminateProcess( task->pi.hProcess, 0xD1E ) )
						{
							lprintf( WIDE("Failed to terminate process...") );
						}
					}
				}
			}
			if( !task->EndNotice )
			{
				// task end notice - will get the event and close these...
				CloseHandle( task->pi.hThread );
				task->pi.hThread = 0;
				if( !bDontCloseProcess )
				{
					CloseHandle( task->pi.hProcess );
					task->pi.hProcess = 0;
				}
			}
			else
            lprintf( WIDE( "Would have close handles rudely." ) );
#else
         kill( task->pid, SIGTERM );
			// wait a moment for it to die...
#endif
		}
		//if( !task->EndNotice )
		//{
		//	Release( task );
		//}
		//task = NULL;
	}
	return 0;
}

//--------------------------------------------------------------------------

SYSTEM_PROC( void, SetProgramUserData )( PTASK_INFO task, PTRSZVAL psv )
{
	if( task )
		task->psvEnd = psv;
}

//--------------------------------------------------------------------------

_32 GetTaskExitCode( PTASK_INFO task )
{
	if( task )
		return task->exitcode;
   return 0;
}


PTRSZVAL CPROC WaitForTaskEnd( PTHREAD pThread )
{
	PTASK_INFO task = (PTASK_INFO)GetThreadParam( pThread );
#ifdef __LINUX__
	while( !task->pid ) {
		Relinquish();
	}
#endif
	//if( task->EndNotice )
	{
      // allow other people to delete it...
      Hold( task );
#if defined( WIN32 )
		WaitForSingleObject( task->pi.hProcess, INFINITE );
		GetExitCodeProcess( task->pi.hProcess, &task->exitcode );
#elif defined( __LINUX__ )
      waitpid( task->pid, NULL, 0 );
#endif
      if( task->EndNotice )
			task->EndNotice( task->psvEnd, task );
#if defined( WIN32 )
		if( task->pi.hProcess )
			CloseHandle( task->pi.hProcess );
		if( task->pi.hThread )
			CloseHandle( task->pi.hThread );
#endif
      Release( task );
	}
	//TerminateProgram( task );
   return 0;
}


//--------------------------------------------------------------------------

#ifdef WIN32
static int DumpError( void )
{
	lprintf( WIDE("Failed create process:%d"), GetLastError() );
   return 0;
}
#endif

//--------------------------------------------------------------------------

// Run a program completely detached from the current process
// it runs independantly.  Program does not suspend until it completes.
// No way at all to know if the program works or fails.
SYSTEM_PROC( PTASK_INFO, LaunchProgramEx )( CTEXTSTR program, TEXTSTR path, PCTEXTSTR args, TaskEnd EndNotice, PTRSZVAL psv )
{
	PTASK_INFO task;
	if( program && program[0] )
	{
#ifdef WIN32
	PVARTEXT pvt = VarTextCreate();
	PTEXT cmdline;
	int first = TRUE;
	TEXTCHAR saved_path[256];
	task = (PTASK_INFO)Allocate( sizeof( TASK_INFO ) );
	MemSet( task, 0, sizeof( TASK_INFO ) );
	task->psvEnd = psv;
	task->EndNotice = EndNotice;
	if( strchr( program, ' ' ) )
		vtprintf( pvt, WIDE("\"%s\"" ), program );
	else
		vtprintf( pvt, WIDE("%s"), program );
	if( args && args[0] )// arg[0] is passed with linux programs, and implied with windows.
		args++;
	while( args && args[0] )
	{
		/* restore quotes around parameters with spaces */
		if( strchr( args[0], ' ' ) )
			vtprintf( pvt, WIDE(" \"%s\"" ), args[0] );
		else
			vtprintf( pvt, WIDE(" %s"), args[0] );
		first = FALSE;
		args++;
	}
	cmdline = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	MemSet( &task->si, 0, sizeof( STARTUPINFO ) );
	task->si.cb = sizeof( STARTUPINFO );
	GetCurrentPath( saved_path, sizeof( saved_path ) );
	SetCurrentPath( path );
	if( ( CreateProcess( NULL //program
						  , GetText( cmdline )
						  , NULL, NULL, FALSE
						  , 
#ifdef UNDER_CE 
						  0
#else
						  CREATE_NEW_PROCESS_GROUP|CREATE_NEW_CONSOLE
#endif
						  , NULL
						  , path
						  , &task->si
						  , &task->pi ) || DumpError() ) ||
		( CreateProcess( program
						 , GetText( cmdline )
						 , NULL, NULL, FALSE
						 , 
#ifdef UNDER_CE 
						  0
#else
						  CREATE_NEW_PROCESS_GROUP|CREATE_NEW_CONSOLE
#endif
						 , NULL
						 , path
						 , &task->si
						 , &task->pi ) || DumpError() ) ||
		( CreateProcess( program
						 , NULL // GetText( cmdline )
						 , NULL, NULL, FALSE
						 , 
#ifdef UNDER_CE 
						  0
#else
						  CREATE_NEW_PROCESS_GROUP|CREATE_NEW_CONSOLE
#endif
						 , NULL
						 , path
						 , &task->si
						 , &task->pi ) || DumpError() )
	  )
	{
		lprintf( WIDE("Success running %s[%s]: %d"), program, GetText( cmdline ), GetLastError() );
		if( EndNotice )
			ThreadTo( WaitForTaskEnd, (PTRSZVAL)task );
	}
	else
	{
		lprintf( WIDE("Failed to run %s[%s]: %d"), program, GetText( cmdline ), GetLastError() );
      Release( task );
      task = NULL;
	}
	LineRelease( cmdline );
	SetCurrentPath( saved_path );
	return task;
#endif
#ifdef __LINUX__
	{
      pid_t newpid;
		task = (PTASK_INFO)Allocate( sizeof( TASK_INFO ) );
		MemSet( task, 0, sizeof( TASK_INFO ) );
		task->psvEnd = psv;
		task->EndNotice = EndNotice;
		//if( EndNotice )
		ThreadTo( WaitForTaskEnd, (PTRSZVAL)task );
		if( !( newpid = fork() ) )
		{
		// in case exec fails, we need to
		// drop any registered exit procs...
			DispelDeadstart();

                        chdir( path );

                        execve( program, (char *const*)args, environ );
                        {
                            char *tmp = StrDup( getenv( "PATH" ) );
                            char *tok;
                            for( tok = strtok( tmp, ":" ); tok; tok = strtok( NULL, ":" ) )
                            {
                                char fullname[256];
                                snprintf( fullname, sizeof( fullname ), "%s/%s", tok, program );

                                lprintf( "program:[%s][%s][%s]", fullname,args[0], getenv("PATH") );
                                ((char**)args)[0] = fullname;
                                execve( fullname, (char*const*)args, environ );
                            }
                        }
         lprintf( "exec failed - and this is ALLL bad...[%s]%d", program, errno );
         //DebugBreak();
			// well as long as this runs before
			// the other all will be well...
			task = NULL;
			// shit - what can I do now?!
			exit(0); // just in case exec fails... need to fault this.
		}
		task->pid = newpid;
      //lprintf( WIDE("Forked, and set the pid..") );
		// how can I know if the command failed?
		// well I can't - but the user's callback will be invoked
      // when the above exits.
		return task;
	}
#endif
	}
	return FALSE;
}

SYSTEM_PROC( PTASK_INFO, LaunchProgram )( CTEXTSTR program, TEXTSTR path, PCTEXTSTR args )
{
   return LaunchProgramEx( program, path, args, NULL, 0 );
}

//--------------------------------------------------------------------------

SYSTEM_PROC( int, CallProgram )( CTEXTSTR program, CTEXTSTR path, PCTEXTSTR args, ... )
{
	// should hook into stdin,stdout,stderr...
	// also keep track of process handles so that
	// the close of said program could be monitored.....
   return 0;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//  Function/library manipulation routines...
//-------------------------------------------------------------------------


typedef struct loaded_function_tag
{
	_32 references;
	void (CPROC*function)(void );
	struct loaded_library_tag *library;
	DeclareLink( struct loaded_function_tag );
	TEXTCHAR name[];
} FUNCTION, *PFUNCTION;

#ifdef __WINDOWS__
typedef HMODULE HLIBRARY;
#else
typedef void* HLIBRARY;
#endif
typedef struct loaded_library_tag
{
	int nLibrary; // when unloading...
	HLIBRARY library;
	PFUNCTION functions;
	DeclareLink( struct loaded_library_tag );
	TEXTCHAR *name;
	TEXTCHAR full_name[];
} LIBRARY, *PLIBRARY;

static PLIBRARY libraries;
static PTREEROOT pFunctionTree;

//-------------------------------------------------------------------------

SYSTEM_PROC( generic_function, LoadFunctionExx )( CTEXTSTR libname, CTEXTSTR funcname, LOGICAL bPrivate  DBG_PASS )
{
	static int nLibrary;
	PLIBRARY library = libraries;
	while( library )
	{
		if( StrCmp( library->name, libname ) == 0 )
			break;
		library = library->next;
	}
   // don't really NEED anything else, in case we need to start before deadstart invokes.
	if( !l.load_path )
	{
		lprintf( WIDE( "Init Load Path" ) );
		SetupSystemServices();
	}
	if( !library )
	{
		size_t maxlen = sizeof( TEXTCHAR ) * (strlen( l.load_path ) + 1 + strlen( libname ) + 1 );
		lprintf( WIDE( "%s  %s" ), l.load_path, libname );
		library = (PLIBRARY)Allocate( sizeof( LIBRARY ) + ((maxlen<0xFFFFFF)?(_32)maxlen:0) );
		library->name = library->full_name
						  + snprintf( library->full_name, maxlen, WIDE("%s/"), l.load_path );
		snprintf( library->name, maxlen, WIDE("%s"), libname );
		//strcpy( library->name, libname );
		library->functions = NULL;
#ifdef _WIN32
		SuspendDeadstart();
		// with deadstart suspended, the library can safely register
		// all of its preloads.  Then invoke will release suspend
      // so final initializers in application can run.
		library->library = LoadLibrary( library->name );
		if( !library->library )
		{
			library->library = LoadLibrary( library->full_name );
			if( !library->library )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to load %s[%s](%s) failed: %d."), libname, library->full_name, funcname?funcname:WIDE("all"), GetLastError() );
				Release( library );
				return NULL;
			}
		}
#else
		library->library = dlopen( library->name, RTLD_NOW|(
#ifndef __ARM__
																			 bPrivate?RTLD_LOCAL:
#endif
																			 RTLD_GLOBAL) );
		if( !library->library )
		{
			_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to load %s(%s) failed: %s."), libname, funcname?funcname:"all", dlerror() );
			library->library = dlopen( library->full_name, RTLD_NOW|(bPrivate?RTLD_LOCAL:RTLD_GLOBAL) );
			if( !library->library )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to load %s(%s) failed: %s."), library->full_name, funcname?funcname:"all", dlerror() );
				Release( library );
				return NULL;
			}
		}
#endif
#ifdef __cplusplus_cli
		{
			void (CPROC *f)( void );
			if( l.flags.bLog )
				lprintf( "GetInvokePreloads" );
			f = (void(CPROC*)(void))GetProcAddress( library->library, "InvokePreloads" );
			if( f )
				f();
		}
#endif
		{
			//DebugBreak();
			InvokeDeadstart();
		}
		library->nLibrary = ++nLibrary;
		LinkThing( libraries, library );
	}
	if( funcname )
	{
		PFUNCTION function = library->functions;
		while( function )
		{
			if( strcmp( function->name, funcname ) == 0 )
				break;
			function = function->next;
		}
		if( !function )
		{
			int len;
			function = (PFUNCTION)Allocate( sizeof( FUNCTION ) + (len=(sizeof(TEXTCHAR)*( (_32)strlen( funcname ) + 1 ) ) ) );
			snprintf( function->name, len, WIDE( "%s" ), funcname );
			function->library = library;
			function->references = 0;
#ifdef _WIN32
#ifdef __cplusplus_cli
			char *procname = CStrDup( function->name );
			if( l.flags.bLog )
				lprintf( WIDE( "Get:%s" ), procname );
			if( !(function->function = (generic_function)GetProcAddress( library->library, procname )) )
#else
  			if( l.flags.bLog )
  				lprintf( WIDE( "Get:%s" ), function->name );
			if( !(function->function = (generic_function)GetProcAddress( library->library, function->name )) )
#endif
			{
				TEXTCHAR tmpname[128];
				snprintf( tmpname, sizeof( tmpname ), WIDE("_%s"), funcname );
#ifdef __cplusplus_cli
				char *procname = CStrDup( tmpname );
				if( l.flags.bLog )
					lprintf( WIDE( "Get:%s" ), procname );
				function->function = (generic_function)GetProcAddress( library->library, procname );
				Release( procname );
#else
				if( l.flags.bLog )
					lprintf( WIDE( "Get:%s" ), function->name );
				function->function = (generic_function)GetProcAddress( library->library, tmpname );
#endif
			}
#ifdef __cplusplus_cli
			Release( procname );
#endif
			if( !function->function )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to get function %s from %s failed. %d"), funcname, libname, GetLastError() );
				Release( function );
				return NULL;
			}
#else
			if( !(function->function = (generic_function)dlsym( library->library, function->name )) )
			{
				char tmpname[128];
				sprintf( tmpname, WIDE("_%s"), funcname );
				function->function = (generic_function)dlsym( library->library, tmpname );
			}
			if( !function->function )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to get function %s from %s failed. %s"), funcname, libname, dlerror() );
				Release( function );
				return NULL;
			}
#endif
			if( !pFunctionTree )
				pFunctionTree = CreateBinaryTree();
			//lprintf( WIDE("Adding function %p"), function->function );
			AddBinaryNode( pFunctionTree, function, (PTRSZVAL)function->function );
			LinkThing( library->functions, function );
		}
		function->references++;
		return function->function;
	}
	else
	{
		return (generic_function)(/*extend precisionfirst*/(PTRSZVAL)library->nLibrary); // success, but no function possible.
	}
	return NULL;
}

SYSTEM_PROC( generic_function, LoadPrivateFunctionEx )( CTEXTSTR libname, CTEXTSTR funcname DBG_PASS )
{
   return LoadFunctionExx( libname, funcname, TRUE DBG_RELAY );
}

SYSTEM_PROC( generic_function, LoadFunctionEx )( CTEXTSTR libname, CTEXTSTR funcname DBG_PASS )
{
   return LoadFunctionExx( libname, funcname, FALSE DBG_RELAY );
}
#undef LoadFunction
SYSTEM_PROC( generic_function, LoadFunction )( CTEXTSTR libname, CTEXTSTR funcname )
{
	return LoadFunctionEx( libname,funcname DBG_SRC);
}

//-------------------------------------------------------------------------

// pass the address of the function pointer - this
// will gracefully erase that reference also.
SYSTEM_PROC( int, UnloadFunctionEx )( generic_function *f DBG_PASS )
{
	if( !f  )
		return 0;
	_xlprintf( 1 DBG_RELAY )( WIDE("Unloading function %p"), *f );
	if( (PTRSZVAL)(*f) < 1000 )
	{
		// unload library only...
		if( !(*f) )  // invalid result...
			return 0;
		{
			PLIBRARY library;
			int nFind = (PTRSZVAL)(*f);
			for( library = libraries; library; library = NextLink( library ) )
			{
				if( nFind == library->nLibrary )
				{
#ifdef _WIN32
               // should make sure noone has loaded a specific function.
					FreeLibrary ( library->library );
					UnlinkThing( library );
					Release( library );
#else
#endif
				}
			}
		}
	}
	{
		PFUNCTION function = (PFUNCTION)FindInBinaryTree( pFunctionTree, (PTRSZVAL)(*f) );
		PLIBRARY library;
		if( function &&
		    !(--function->references) )
		{
			UnlinkThing( function );
			lprintf( WIDE( "Should remove the node from the tree here... but it crashes intermittantly. (tree list is corrupted)" ) );
			//RemoveLastFoundNode( pFunctionTree );
			library = function->library;
			if( !library->functions )
			{
#ifdef _WIN32
				FreeLibrary( library->library );
#else
				dlclose( library->library );
#endif
				UnlinkThing( library );
				Release( library );
			}
			Release( function );
			*f = NULL;
		}
		else
		{
			lprintf( WIDE("function was not found - or ref count = %") _32f WIDE(" (5566 means no function)"), function?function->references:5566 );
		}
	}
	return FALSE;
}

//-------------------------------------------------------------------------

SYSTEM_PROC( PTHREAD, SpawnProcess )( CTEXTSTR filename, va_list args )
{
	PTRSZVAL (CPROC *newmain)( PTHREAD pThread );
	newmain = (PTRSZVAL(CPROC*)(PTHREAD))LoadFunction( filename, WIDE("main") );
	if( newmain )
	{
		// hmm... suppose I should even thread through my own little header here
      // then when the thread exits I can get a positive acknowledgement?
      return ThreadTo( newmain, (PTRSZVAL)args );
	}
	return NULL;
}

//---------------------------------------------------------------------------

TEXTSTR GetArgsString( PCTEXTSTR pArgs )
{
	static TEXTCHAR args[256];
	int len = 0, n;
	args[0] = 0;
	// arg[0] should be the same as program name...
	for( n = 1; pArgs && pArgs[n]; n++ )
	{
		int space = (strchr( pArgs[n], ' ' )!=NULL);
		len += snprintf( args + len, sizeof( args ) - len , WIDE("%s%s%s%s")
							, n>1?WIDE(" "):WIDE("")
							, space?WIDE("\""):WIDE("")
							, pArgs[n]
							, space?WIDE("\""):WIDE("")
							);
	}
	return args;
}

SACK_SYSTEM_NAMESPACE_END


