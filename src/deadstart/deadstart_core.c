#ifdef _WIN64
#define __64__
#endif

// debugging only gets you the ordering logging and something else...
// useful logging is now controlled with l.flags.bLog
#define DISABLE_DEBUG_REGISTER_AND_DISPATCH
//#define DEBUG_SHUTDOWN
#define LOG_ALL 0

//
// core library load
//    all procs scheduled, initial = 0
// Application starts, invokes preloads
//    additional libraries load, scheduling because of suspend
//    library load completes by invoking the newly registered list
// final core application schedulging happens, after initial preload completes
//    additional preload scheduligin happens( not suspended, is initial)
//#define DEBUG_CYGWIN_START
//#ifndef __LINUX__
#define IS_DEADSTART
#ifdef __LINUX__
#include <signal.h>
#endif

#include <stdhdrs.h>
#include <sack_types.h>
#include <logging.h>
#include <deadstart.h>
#include <procreg.h>
#include <sqlgetoption.h>
#ifdef __NO_BAG__
#undef lprintf
#define lprintf printf
#define BAG_Exit exit
#else
#endif


#ifdef UNDER_CE
#define LockedExchange InterlockedExchange
#endif

SACK_DEADSTART_NAMESPACE

#undef PRELOAD

EXPORT_METHOD void RunDeadstart( void );

typedef struct startup_proc_tag {
	DeclareLink( struct startup_proc_tag );
	int bUsed;
   int priority;
	void (*proc)(void);
   CTEXTSTR func;
	CTEXTSTR file;
   int line;
} STARTUP_PROC, *PSTARTUP_PROC;

typedef struct shutdown_proc_tag {
	DeclareLink( struct shutdown_proc_tag );
	int bUsed;
   int priority;
	void (*proc)(void);
   CTEXTSTR func;
	CTEXTSTR file;
   int line;
} SHUTDOWN_PROC, *PSHUTDOWN_PROC;

struct deadstart_local_data_
{
	// this is a lot of procs...
	int nShutdownProcs;
#define nShutdownProcs l.nShutdownProcs
	SHUTDOWN_PROC shutdown_procs[512];
#define shutdown_procs l.shutdown_procs
	int bInitialDone;
#define bInitialDone l.bInitialDone
	LOGICAL bInitialStarted;
#define bInitialStarted l.bInitialStarted
   int bSuspend;
#define bSuspend l.bSuspend
	int bDispatched;
#define bDispatched l.bDispatched

	PSHUTDOWN_PROC shutdown_proc_schedule;
#define shutdown_proc_schedule l.shutdown_proc_schedule

	int nProcs; // count of used procs...
#define nProcs l.nProcs
	STARTUP_PROC procs[1024];
#define procs l.procs
	PSTARTUP_PROC proc_schedule;
#define proc_schedule l.proc_schedule
	struct
	{
		BIT_FIELD bInitialized : 1;
		BIT_FIELD bLog : 1;
	} flags;
};

#ifdef UNDER_CE
#  ifndef __STATIC_GLOBALS__
#    define __STATIC_GLOBALS__
#  endif
#endif

#ifndef __STATIC_GLOBALS__
static struct deadstart_local_data_ *deadstart_local_data;
#define l (*deadstart_local_data)
#else
static struct deadstart_local_data_ deadstart_local_data;
#define l (deadstart_local_data)
#endif

void RunExits( void )
{
   InvokeExits();
}

static void InitLocal( void )
{
#ifndef __STATIC_GLOBALS__
	if( !deadstart_local_data )
	{
		SimpleRegisterAndCreateGlobal( deadstart_local_data );
	}
#endif
	if( !l.flags.bInitialized )
	{
		atexit( RunExits );
		l.flags.bInitialized = 1;
	}
}


// parameter 4 is just used so the external code is not killed
// we don't actually do anything with this?
void RegisterPriorityStartupProc( void (*proc)(void), CTEXTSTR func,int priority, void *use_label, CTEXTSTR file,int line )
{
	if( LOG_ALL ||
#ifndef __STATIC_GLOBALS__
		 (deadstart_local_data 
#else 
		(1
#endif
		&& l.flags.bLog ))
		lprintf( WIDE("%s@%s(%d) %d register"), func,file,line, priority);
	InitLocal();
	procs[nProcs].proc = proc;
	procs[nProcs].func = func;
	procs[nProcs].file = file;
	procs[nProcs].line = line;
	procs[nProcs].priority = priority;
	procs[nProcs].bUsed = 1;
	//LinkLast( proc_schedule, PSTARTUP_PROC, procs + nProcs );
	{
		PSTARTUP_PROC check;
		for( check = proc_schedule; check; check = check->next )
		{
		// if the current one being added is less then the one in the list
         // then the one in the list becomes the new one's next...
			if( procs[nProcs].priority < check->priority )
			{
#ifndef  DISABLE_DEBUG_REGISTER_AND_DISPATCH
				lprintf( WIDE("%s(%d) is to run before %s")
						 , procs[nProcs].func
						 , nProcs
						 , check->func );
#endif
            procs[nProcs].next = check;
				procs[nProcs].me = check->me;
            (*check->me) = procs + nProcs;
				check->me = &procs[nProcs].next;
            break;
			}
		}
		if( !check )
		{
			LinkLast( proc_schedule, PSTARTUP_PROC, procs + nProcs );
#ifndef  DISABLE_DEBUG_REGISTER_AND_DISPATCH
			lprintf( WIDE("%s(%d) is to run after all")
					 , procs[nProcs].func
					 , nProcs
					 );
#endif
		//lprintf( WIDE("first routine is %s(%d)")
		//		 , shutdown_proc_schedule->func
		//		 , shutdown_proc_schedule->line );
		}
	}
	nProcs++;
	if( nProcs == 1024 )
	{
		lprintf( WIDE( "Excessive number of startup procs!" ) );
		DebugBreak();
	}
	
	if( bInitialDone && !bSuspend )
	{
#define ONE_MACRO(a,b) a,b
#ifdef _DEBUG
		_xlprintf(LOG_NOISE,file,line)( "Initial done, not suspended, dispatch immediate." );
#endif
      InvokeDeadstart();
	}
   //lprintf( WIDE("Total procs %d"), nProcs );
}

#if UNUSED
#ifdef __WATCOMC__ 
// this is really nice - to have a prioritized initializer list...
#  ifdef __cplusplus
#    define PRELOAD(name) MAGIC_PRIORITY_PRELOAD(name,DEADSTART_PRELOAD_PRIORITY)
#  else
#    define PRELOAD(name) static void name(void); \
	    struct rt_init __based(__segname("XI")) name##_ctor_label={0,DEADSTART_PRELOAD_PRIORITY,name}; \
	    static void name(void)
#  endif
#elif defined( _MSC_VER )
#  ifdef __cplusplus_cli
#    define PRELOAD(name) static void _##name(void); \
    public ref class name {   \
	public:name() { _##name(); \
		System::Console::WriteLine( /*lprintf( */gcnew System::String( WIDE("Startups.ADSF.. ") ) ); \
	  }  \
	}/* do_schedul_##name*/;     \
	static void _##name(void)
//PRIORITY_PRELOAD(name,DEADSTART_PRELOAD_PRIORITY)
#  else
#    if (_MSC_VER==1300)
#      define PRELOAD(name) static void name(void); \
	__declspec(allocate(_STARTSEG2_)) void (CPROC*pointer_##name)(void) = name; \
	void name(void)
#    else
#       define PRELOAD(name) static void name(void); \
	__declspec(allocate(_STARTSEG2_)) void (CPROC*pointer_##name)(void) = name; \
	void name(void)
#    endif
#  endif
#elif defined( __GNUC__ )
#    define PRELOAD(name) void name( void ) __attribute__((constructor)); \
void name( void )
#endif


#endif

#ifdef __LINUX__
// this handles the peculiarities of fork() and exit()
void ClearDeadstarts( void )
{
	// this is reserved for the sole use of
	// fork() success and then exec() failing...
	// when oh wait - __attribute__((destructor))
	// if( registered_pid != getppid() )
	shutdown_proc_schedule = NULL;
	// be rude - yes we lose resources. but everything goes away cause
	// this is just a clone..
}
#endif

#ifndef UNDER_CE
#  if defined( __WINDOWS__ )
#    ifndef __cplusplus_cli
static BOOL WINAPI CtrlC( DWORD dwCtrlType )
{
	switch( dwCtrlType )
	{
	case CTRL_BREAK_EVENT:
	case CTRL_C_EVENT:
		InvokeExits();
		// allow C api to exit, whatever C api we're using
      // (allows triggering atexit functions)
		exit(3);
		return TRUE;
	case CTRL_CLOSE_EVENT:
		break;
	case CTRL_LOGOFF_EVENT:
		break;
	case CTRL_SHUTDOWN_EVENT:
		break;
	}
   // default... return not processed.
   return FALSE;
}
#    endif
#  endif

#  ifndef __WINDOWS__
static void CtrlC( int signal )
{
   exit(3);
}
#  endif
#endif

//#ifdef __WINDOWS__

// wow no such thing as static-izing this... it's
// always retrieved with dynamic function loading, therefore
// MUST be exported if at all possible.
// !defined( __STATIC__ ) &&
// this one is used when a library is loaded.
void InvokeDeadstart( void )
{
	PSTARTUP_PROC proc;
	//if( !bInitialDone /*|| bDispatched*/ )
	//   return;
	InitLocal();
   bInitialStarted = 1;
	//bSuspend = 0; // if invoking, no longer suspend.
#ifdef __WINDOWS__
	if( !bInitialDone && !bDispatched )
	{
#  ifndef UNDER_CE
		if( GetConsoleWindow() )
		{
#    ifndef __cplusplus_cli
			//MessageBox( NULL, "!!--!! CtrlC", "blah", MB_OK );
			SetConsoleCtrlHandler( CtrlC, TRUE );
#    endif
		}
		else
		{
			//MessageBox( NULL, "!!--!! NO CtrlC", "blah", MB_OK );
			; // do nothing if we're no actually a console window. this should fix ctrl-c not working in CMD prompts launched by MILK/InterShell
		}
#  endif
	}
#endif

#ifdef __64__
	while( ( proc = (PSTARTUP_PROC)LockedExchange64( (PVPTRSZVAL)&proc_schedule, NULL ) ) != NULL )
#else
		while( ( proc = (PSTARTUP_PROC)LockedExchange( (PVPTRSZVAL)&proc_schedule, 0 ) ) != NULL )
#endif
	{
		if( LOG_ALL ||
#ifndef __STATIC_GLOBALS__
		 (deadstart_local_data 
#else 
		(1
#endif
		&& l.flags.bLog ))
			lprintf( WIDE("Dispatch %s@%s(%d)"), proc->func,proc->file,proc->line);
		//bDispatched = 1;
		{
			bDispatched = 1;
			//proc_schedule = NULL;
			if( proc->proc
#ifndef __LINUX__
#if  __WATCOMC__ >= 1280
				&& !IsBadCodePtr( (int(STDCALL*)(void))proc->proc )
#elif defined( __64__ )
				&& !IsBadCodePtr( (FARPROC)proc->proc )
#else
//				&& !IsBadCodePtr( (int STDCALL(*)(void))proc->proc )
#endif
#endif
			  )
			{
				proc->proc();
			}
			proc->bUsed = 0;
			bDispatched = 0;
			if( proc_schedule )
			{
				//lprintf( WIDE("Scheduled startups while dispatched.") );
				LinkLast( proc, PSTARTUP_PROC, proc_schedule );
			}
			proc_schedule = proc;
		}
		proc->bUsed = 0;
		UnlinkThing( proc );
	}
}

void MarkRootDeadstartComplete( void )
{
	bInitialDone = 1;
#ifndef __NO_OPTIONS__
	l.flags.bLog = SACK_GetProfileIntEx( "SACK/Deadstart", "Logging Enabled?", 0, TRUE );
#endif
}



#if defined( __GNUC__ )

#ifndef __cplusplus
static void RegisterStartups( void ) __attribute__((constructor));

// this one is used when the library loads.  (there is only one of these.)
// and constructors are run every time a library is loaded....
// I wonder whose fault that is....
void RegisterStartups( void )
{
#  ifndef paste
#    define paste(a,b) a##b
#  endif
#  define paste2(a,b) paste(a,b)
#  define DeclareList(n) paste2(n,TARGET_LABEL)
   extern struct rt_init DeclareList( begin_deadstart_ );
   extern struct rt_init DeclareList( end_deadstart_ );
	struct rt_init *begin = &DeclareList( begin_deadstart_ );
	struct rt_init *end = &DeclareList( end_deadstart_ );
	struct rt_init *current;
#  ifdef __NO_BAG__
   printf( "Not doing deadstarts\n" );
	return;
#  endif
   //cygwin_dll_init();
	if( begin[0].scheduled )
      return;
	if( (begin+1) < end )
	{
#  ifdef __CYGWIN__
		void (*MyRegisterPriorityStartupProc)( void (*proc)(void), CTEXTSTR func,int priority, CTEXTSTR file,int line );
		char myname[256];
      HMODULE mod;
      GetModuleFileName(NULL,myname,sizeof(myname));
		mod = LoadLibrary( myname );GetModuleFileName(NULL,myname,sizeof(myname));
		MyRegisterPriorityStartupProc = (void(*)( void(*)(void),CTEXTSTR,int,CTEXTSTR,int))GetProcAddress( mod, "RegisterPriorityStartupProc" );
#    ifdef DEBUG_CYGWIN_START
		fprintf( stderr, "mod is %p proc  is %p %s\n", mod, MyRegisterPriorityStartupProc, TARGETNAME );
#    endif
		if( MyRegisterPriorityStartupProc )
		{
#  endif
		for( current = begin + 1; current < end; current++ )
		{
			if( !current[0].scheduled )
			{
				if( ( LOG_ALL
#ifndef __STATIC_GLOBALS__
					|| deadstart_local_data
#endif
					 )	&& l.flags.bLog )
					lprintf( WIDE("Register %d %s@%s(%d)"), current->priority, current->funcname, current->file, current->line );
#  ifdef __CYGWIN__
#    ifdef DEBUG_CYGWIN_START
				fprintf( stderr, WIDE("Register %d %s@%s(%d)\n"), current->priority, current->funcname, current->file, current->line );
#    endif
				MyRegisterPriorityStartupProc( current->routine, current->funcname, current->priority, current->file, current->line );
#  else
				RegisterPriorityStartupProc( current->routine, current->funcname, current->priority, NULL, current->file, current->line );
#  endif
            current[0].scheduled = 1;
			}
			else
			{
#  ifndef  DISABLE_DEBUG_REGISTER_AND_DISPATCH
				lprintf( WIDE("Not Register(already did this once) %d %s@%s(%d)"), current->priority, current->funcname, current->file, current->line );
#  endif
			}
#  ifdef __CYGWIN__
		}
#  endif
		}
	}
   // should be setup in such a way that this ignores all external invokations until the core app runs.
	//InvokeDeadstart();
}
#endif  //__cplusplus

#endif // #if __GNUC__

// parameter 4 is just used so the external code is not killed
// we don't actually do anything with this?
void RegisterPriorityShutdownProc( void (*proc)(void), CTEXTSTR func, int priority,void *use_label, CTEXTSTR file,int line )
{
#ifdef  DEBUG_SHUTDOWN
	lprintf( WIDE("Exit Proc %s(%p) from %s(%d) registered...")
			 , func
			 , proc
			 , file
			 , line );
#endif
   InitLocal();
	shutdown_procs[nShutdownProcs].proc = proc;
	shutdown_procs[nShutdownProcs].func = func;
	shutdown_procs[nShutdownProcs].file = file;
	shutdown_procs[nShutdownProcs].line = line;
	shutdown_procs[nShutdownProcs].bUsed = 1;
	shutdown_procs[nShutdownProcs].priority = priority;
	{
		PSHUTDOWN_PROC check;
		for( check = shutdown_proc_schedule; check; check = check->next )
		{
			if( shutdown_procs[nShutdownProcs].priority >= check->priority )
			{
#ifdef DEBUG_SHUTDOWN
				lprintf( WIDE("%s(%d) is to run before %s(%d) %s")
						 , shutdown_procs[nShutdownProcs].func
						 , nShutdownProcs
						 , check->file
						 , check->line
						 , check->func );
#endif
            shutdown_procs[nShutdownProcs].next = check;
				shutdown_procs[nShutdownProcs].me = check->me;
            (*check->me) = shutdown_procs + nShutdownProcs;
				check->me = &shutdown_procs[nShutdownProcs].next;
            break;
			}
		}
      if( !check )
			LinkLast( shutdown_proc_schedule, PSHUTDOWN_PROC, shutdown_procs + nShutdownProcs );
		//lprintf( WIDE("first routine is %s(%d)")
	  //		 , shutdown_proc_schedule->func
		//		 , shutdown_proc_schedule->line );
	}
	nShutdownProcs++;
   //lprintf( WIDE("Total procs %d"), nProcs );
}

//id RunExits( void )
void InvokeExits( void )
{
// okay well since noone previously scheduled exits...
// this runs a prioritized list of exits - all within
// a single moment of exited-ness.
	PSHUTDOWN_PROC proc;
// shutdown is much easier than startup cause more
// procedures shouldn't be added as a property of shutdown.
   //bugBreak();
	while(
#ifdef __64__
			( proc = (PSHUTDOWN_PROC)LockedExchange64( (PV_64)&shutdown_proc_schedule, 0 ) ) != NULL
#else
			( proc = (PSHUTDOWN_PROC)LockedExchange( (PV_32)&shutdown_proc_schedule, 0 ) ) != NULL
#endif
		  )
	{
      PSHUTDOWN_PROC proclist = proc;
      // link list to myself..
		proc->me = &proclist;
		while( ( proc = proclist ) )
		{
#ifdef DEBUG_SHUTDOWN
			lprintf( WIDE("Exit Proc %s(%p)(%d) priority %d from %s(%d)...")
					 , proc->func
					 , proc->proc
					 , proc - shutdown_procs
                , proc->priority
					 , proc->file
					 , proc->line );
#endif
			if( proc->priority == 0 )
			{
				//atexit( proc->proc );
				//continue;
			}
			// don't release this stuff... memory might be one of the autoexiters.
			if( proc->proc
#ifndef __LINUX__
				&& !IsBadCodePtr( (FARPROC)proc->proc )
#endif
			  )
			{
#ifdef DEBUG_SHUTDOWN
				lprintf( "Dispatching..." );
#endif
				proc->proc();
			}
			// okay I have the whol elist... so...
			UnlinkThing( proc );
#ifdef DEBUG_SHUTDOWN
			lprintf( WIDE("Okay and that's done... next is %p %p"), proclist, shutdown_proc_schedule );
#endif
		}
      //shutdown_proc_schedule = proc;
	}
	//shutdown_proc_schedule = NULL;
}

void DispelDeadstart( void )
{
   shutdown_proc_schedule = NULL;
}

#ifdef __cplusplus

ROOT_ATEXIT(AutoRunExits)
{
   InvokeExits();
}

#endif


void SuspendDeadstart( void )
{
   bSuspend = 1;
}
void ResumeDeadstart( void )
{
	bSuspend = 0;
	if( bInitialDone )
		InvokeDeadstart();
}

SACK_DEADSTART_NAMESPACE_END
SACK_NAMESPACE

// linked into BAG to provide a common definition for function Exit()
// this then invokes an exit in the mainline program (if available)
EXPORT_METHOD	void BAG_Exit( int code )
{
#ifndef __cplusplus_cli
	InvokeExits();
#endif
#undef exit
   exit( code );
}

// legacy linking code - might still be usin this for linux...
int is_deadstart_complete( void )
{
	//extern _32 deadstart_complete;
#ifndef __STATIC_GLOBALS__
   if( deadstart_local_data )
		return bInitialDone;//deadstart_complete;
#endif
	return 0;
}

SACK_NAMESPACE_END
SACK_DEADSTART_NAMESPACE

LOGICAL IsRootDeadstartStarted( void )
{
#ifndef __STATIC_GLOBALS__
	if( deadstart_local_data )
		return bInitialStarted;
#endif
   return 0;
}



#if 0
int APIENTRY DllMain( HINSTANCE HInst, DWORD dwReason, UINT voidreason )
{
   return 1;
}
#endif

SACK_DEADSTART_NAMESPACE_END
