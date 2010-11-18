/*
 *  Crafted by James Buckeyne
 *
 *   (c) Freedom Collective 2000-2006++
 *
 *   Adds functionality of timers that run dispatched from a single thread
 *   timer delay is trackable to provide self adjusting reliable frequency dispatch.
 *   
 *   RemoveTimer( AddTimer( tick_frequency, timer_callback, user_data ) );
 *
 */


// this is a method replacement to use PIPEs instead of SEMAPHORES
// replacement code only affects linux.
//#define USE_PIPE_SEMS

// this is a cheat to get the critical section
// object... otherwise we'd have had circular
// linking reference between this and sharemem
// which would prefer to implement wakeablesleep
// for critical section waiting...
// must be included before memlib..

//#undef UNICODE
#define MEMORY_STRUCT_DEFINED
#define DEFINE_MEMORY_STRUCT

#define THREAD_STRUCTURE_DEFINED

#include <stdhdrs.h> // Sleep()
// sorry if this causes problems...
// maybe promote this include into stdhdrs when this fails to compile

#ifdef __WATCOMC__
// _beginthread
#undef exit
#undef getenv
// process.h redefines exit
#include <process.h>
#endif


#include <sack_types.h>
#include <deadstart.h>
#include <sharemem.h>
#include <idle.h>
#ifndef __NO_OPTIONS__
#include <sqlgetoption.h>
#endif
#define DO_LOGGING
#include <logging.h>

#include <timers.h>

#include "../memlib/sharestruc.h"
#ifdef __cplusplus 
namespace sack {
	namespace timers {
		using namespace sack::containers;
		using namespace sack::memory;
		using namespace sack::logging;
#endif


//#define LOG_CREATE_EVENT_OBJECT
//#define LOG_THREAD
//#define LOG_SLEEPS

// - define this to log when timers were delayed in scheduling...
//198#define LOG_LATENCY_LIGHT
//#define LOG_LATENCY

//#define LOG_INSERTS
//#define LOG_DISPATCH

struct timer_tag
{
// putting next as first thing in structure
   // allows me to reference also prior
	struct timer_tag *next;
	union {
		struct timer_tag **me;
		struct timer_tag *prior;
	};
	struct {
		BIT_FIELD bRescheduled : 1;
	} flags;
	_32 frequency;
	S_32 delta;
	_32 ID;
	void (CPROC*callback)(PTRSZVAL user);
	PTRSZVAL userdata;
#ifdef _DEBUG
	CTEXTSTR pFile;
   int nLine;
#endif
};
typedef struct timer_tag TIMER, *PTIMER;

#define MAXTIMERSPERSET 32
DeclareSet( TIMER );

struct threads_tag
{
	// these first two items MUST
	// be declared publically, and MUST be visible
	// to the thread created.
	PTRSZVAL param;
	PTRSZVAL (CPROC*proc)( struct threads_tag * );
	PTRSZVAL (CPROC*simple_proc)( POINTER );
	THREAD_ID thread_ident;
#ifdef _WIN32
	HANDLE hEvent;
	HANDLE hThread;
#else
#ifdef USE_PIPE_SEMS
   int pipe_ends[2]; // file handles that are the pipe's ends. 0=read 1=write
#endif
	int semaphore; // use this as a status of pipes if USE_PIPE_SEMS is used...
	pthread_t thread;
#endif
	struct {
		//BIT_FIELD bLock : 1;
		//BIT_FIELD bSleeping : 1;
		//BIT_FIELD bWakeWhileRunning : 1;
		BIT_FIELD bRemovedWhileRunning : 1;
		BIT_FIELD bLocal : 1;
		BIT_FIELD bReady : 1;
	} flags;
	//struct threads_tag *next, **me;
	CTEXTSTR pFile;
   _32 nLine;
};

typedef struct threads_tag THREAD;
#define MAXTHREADSPERSET 16
DeclareSet( THREAD );

struct thread_event
{
	CTEXTSTR name;
#ifdef _WIN32
   HANDLE hEvent;
#endif
};

typedef struct thread_event THREAD_EVENT;
typedef struct thread_event *PTHREAD_EVENT;

static struct {
	_32 timerID;
   PTIMERSET timer_pool;
	PTIMER timers;
	PTIMER add_timer; // this timer is scheduled to be added...
   PTIMER current_timer;
	struct {
		BIT_FIELD away_in_timer : 1;
		BIT_FIELD insert_while_away : 1;
		BIT_FIELD set_timer_signal : 1;
		BIT_FIELD bExited : 1;
		BIT_FIELD bLogCriticalSections : 1;
		BIT_FIELD bLogSleeps : 1;
	} flags;
	_32 del_timer; // this timer is scheduled to be removed...
	_32 tick_bias; // should somehow end up equating to sleep overhead...
	_32 last_tick; // last known time that a timer could have fired...
	_32 this_tick; // the current moment up to which we fire all timers.
	PTHREAD pTimerThread;
	PTHREADSET threadset;
	PTHREAD threads;
	_32 lock_timers;
   CRITICALSECTION cs_timer_change;
	//_32 pending_timer_change;
	_32 remove_timer;
	_32 CurrentTimerID;
	S_32 last_sleep;
#ifdef _DEBUG
// person that did that last wakethread...
	_32 nLineWake;
	CTEXTSTR pFileWake;
#endif
#define g global_timer_structure
	_32 lock_thread_create;
   // should be a short list... 10 maybe 15...
   PLIST thread_events;
} g = { 1000 };


#ifdef _WIN32
#else
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
//#include <sys/ipc.h>

	 // hmm wonder why this has to be defined....
	 // semtimedop is a wonderful wonderful thing...
	 // but yet /usr/include/sys/sem.h only defines it if
// __USE_GNU is defined....
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sys/sem.h>
#include <signal.h>

#endif

PRELOAD( ConfigureTimers )
{
#ifndef __NO_OPTIONS__
	g.flags.bLogCriticalSections = SACK_GetProfileInt( GetProgramName(), "SACK/Timers/Log Critical Sections", 0 );
   g.flags.bLogSleeps = SACK_GetProfileInt( GetProgramName(), "SACK/Timers/Log Sleeps", 0 );
#endif

}

//--------------------------------------------------------------------------
#ifdef __LINUX__
#ifdef __LINUX__
TIMER_PROC( _32, GetTickCount )( void )
{
	struct timeval time;
	gettimeofday( &time, 0 );
	return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

TIMER_PROC( _32, timeGetTime )( void )
{
	struct timeval time;
	gettimeofday( &time, 0 );
	return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

TIMER_PROC( void, Sleep )( _32 ms )
{
	(usleep((ms)*1000));
}
#endif
PTRSZVAL closesem( POINTER p, PTRSZVAL psv )
{
   PTHREAD thread = (PTHREAD)p;
#ifdef USE_PIPE_SEMS
	close( thread->pipe_ends[0] );
	close( thread->pipe_ends[1] );
	thread->pipe_ends[0] = -1;
	thread->pipe_ends[1] = -1;
   thread->semaphore = -1;
#else
	if( semctl( thread->semaphore, 0, IPC_RMID ) == -1 )
	{
		lprintf( "Error: %08x %s", thread->semaphore, strerror( errno ) );
	}
	thread->semaphore = -1;
#endif
   return 0;
}

// sharemem exit priority +1 (exit after everything else, except emmory)
PRIORITY_ATEXIT( CloseAllWakeups, ATEXIT_PRIORITY_TIMERS )
{
	//pid_t mypid = getppid();
// not sure if mypid is needed...
   g.flags.bExited = 1;
   lprintf( WIDE("Destroy thread semaphores...") );
	ForAllInSet( THREAD, g.threadset, closesem, (PTRSZVAL)0 );
	DeleteSet( (GENERICSET**)&g.threadset );
   g.pTimerThread = NULL;
   //g.threads = NULL;
   g.timers = NULL;
}
#endif
//--------------------------------------------------------------------------

static void InitWakeup( PTHREAD thread )
{
#ifdef _WIN32
	if( !thread->hEvent )
	{
		TEXTCHAR name[64];
		snprintf( name, sizeof(name), WIDE("Thread Signal:%08lX:%08lX"), (_32)(thread->thread_ident >> 32)
				 , (_32)(thread->thread_ident & 0xFFFFFFFF) );
#ifdef LOG_CREATE_EVENT_OBJECT
		lprintf( WIDE("Thread Event created is: %s everyone should use this..."), name );
#endif
		thread->hEvent = CreateEvent( NULL, TRUE, FALSE, name );
		//lprintf( WIDE("and is %d"), thread->hEvent );
	}
#else
#ifdef USE_PIPE_SEMS
   // store status of pipe() in semaphore... it's not really a semaphore..
	if( ( thread->semaphore = pipe( thread->pipe_ends ) )  == -1 )
	{
		lprintf( WIDE("Failed to get pipe! %d:%s"), errno, strerror( errno ) );
	}
#else
	thread->semaphore = semget( IPC_PRIVATE 
									  , 1, IPC_CREAT | 0600 );
	if( thread->semaphore == -1 )
	{
      // basically this can't really happen....
		if( errno ==  EEXIST )
		{
			thread->semaphore = semget( IPC_PRIVATE 
											  , 1, 0 );
			if( thread->semaphore == -1 )
				lprintf( WIDE("FAILED TO CREATE SEMAPHORE! : %d"), errno );
		}
		if( errno == ENOSPC )
		{
			lprintf( WIDE("Hmm Need to cleanup some semaphore objects!!!") );
		}
		else
			lprintf( WIDE("Failed to get semaphore! %d"), errno );
	}
	if( thread->semaphore != -1 )
	{
		//union semun ctl;
	   //ctl.val = 0;
      //lprintf( WIDE("Setting thread semaphore to 0 (locked).") );
		if( semctl( thread->semaphore, 0, SETVAL, 0 ) < 0 )
		{
			lprintf( WIDE("Errro setting semaphre value: %d"), errno );
		}
      //lprintf( WIDE("after semctl = %d %08lx"), semctl( thread->semaphore, 0, GETVAL ), thread->semaphore );
	}
#endif
#endif
}

//--------------------------------------------------------------------------


PTRSZVAL CPROC check_thread( POINTER p, PTRSZVAL psv )
{
	PTHREAD thread = (PTHREAD)p;
   THREAD_ID ID = *((THREAD_ID*)psv);
	if( thread->thread_ident == ID )
		return (PTRSZVAL)p;
   return 0;
}

static PTHREAD FindThread( THREAD_ID thread )
{
	PTHREAD check;
	while( LockedExchange( &g.lock_thread_create, 1 ) )
		Relinquish();
	check = (PTHREAD)ForAllInSet( THREAD, g.threadset, check_thread, (PTRSZVAL)&thread );
	if( !check )
	{
#ifdef _DEBUG
		//lprintf( DBG_FILELINEFMT "Failed to find the thread - so let's add it" );
#endif
		check = GetFromSet( THREAD, &g.threadset );
      //lprintf( WIDE("Get Thread %p"), check );
		//check = Allocate( sizeof( THREAD ) );
		MemSet( check, 0, sizeof( THREAD ) );
		check->thread_ident = thread;
		//check->proc = NULL;
		//check->param = 0;
#ifdef __LINUX__
      if( 0 )
		{
			char buf[128];
			char fname[128];
			sprintf( fname, WIDE("/proc/%") _32f WIDE("/status"), (_32)((thread >> 32) & 0x3FFFFFFF) );
			{
				FILE *status = fopen( fname, WIDE("rt") );
				if( status )
				{
					while( fgets( buf, 128, status ) )
					{
						if( strncmp( buf, WIDE("State:"),6 ) == 0 )
						{
							char *p = buf + 6;
							while( p[0] == ' ' || p[0] == '\t' ) p++;
							if( p[0] == 'S' )
							{
								//check->flags.bSleeping = 1;
							}
							break;
						}
					}
					fclose( status );
				}
				else
					lprintf( WIDE("Failed to open %s"), fname );
			}
		}
#endif
		InitWakeup( check );
      check->flags.bReady = 1;
		//if( ( check->next = g.threads ) )
		//	check->next->me = &check->next;
		//check->me = &g.threads;
		//g.threads = check;
	}
   g.lock_thread_create = 0;
	return check;
}

//--------------------------------------------------------------------------
#if 0
TIMER_PROC( int, TestWakeThread )( PTHREAD thread )
{
#ifdef _WIN32
	if( thread )
	{
		char name[64];
		HANDLE hEvent;
		snprintf( name, sizeof(name), WIDE("Thread Signal:%08X:%08X"), (_32)(thread->thread_ident >> 32)
				 , (_32)(thread->thread_ident & 0xFFFFFFFF) );
		//Log1( WIDE("Test Event created is: %s"), name );
		// this may test true in the event that the creator
		// is being awoken by someone, the creator dies, this
		// runs, and is able to open the event which is still
		// held by the waker.
		hEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, name );
		if( hEvent )
		{
         //lprintf( WIDE("event opened successfully...") );
			CloseHandle( hEvent );
			return TRUE;
		}
		else
		{
         lprintf( WIDE("Failed to open that event (test...)") );
		}
	}
	return FALSE;
#else
   // uhh linux needs a test...
   return TRUE;
#endif
}

TIMER_PROC( int, TestWakeThreadID )( THREAD_ID thread )
{
	PTHREAD pThread = FindThread( thread );
	return TestWakeThread( pThread );
}
#endif
//--------------------------------------------------------------------------

TIMER_PROC( void, WakeThreadEx )( PTHREAD thread DBG_PASS )
{
	if( !thread ) // can't wake nothing
	{
		_lprintf(DBG_RELAY)( WIDE("Failed to find thread to wake...") );
		return;
	}
	//_xlprintf( 0 DBG_RELAY )( WIDE("Waking a thread: %p"), thread );
	//while( thread->flags.bLock )
	//{
	//	Log( WIDE("Waiting for thread to go to sleep") );
	//	Relinquish();
	//}
	//if( thread->flags.bLocal && !thread->flags.bSleeping )
	//{
	//	//Log( WIDE("Waking thread which is already awake") );
	//  thread->flags.bWakeWhileRunning = 1;
	//  Relinquish(); // wake implies that we want the other thing to run.
	//  lprintf( DBG_FILELINEFMT "Thread is not sleeping... woke it before it slept" );
	//  return;
	//}
#ifdef _WIN32
	//	lprintf( WIDE("setting event.") );
	{
		PTHREAD_EVENT event;
		INDEX idx;
		TEXTCHAR name[64];
		snprintf( name, sizeof(name), WIDE("Thread Signal:%08lX:%08lX"), (_32)(thread->thread_ident >> 32)
				 , (_32)(thread->thread_ident & 0xFFFFFFFF));
		LIST_FORALL( g.thread_events, idx, PTHREAD_EVENT, event )
		{
			if( StrCmp( event->name, name ) == 0 )
				break;
		}
#ifdef LOG_CREATE_EVENT_OBJECT
		lprintf( WIDE("Event opened is: %s"), name );
#endif
		if( !event )
		{
			event = New( THREAD_EVENT );
			event->name = StrDup( name );
			event->hEvent = OpenEvent( EVENT_ALL_ACCESS /*EVENT_MODIFY_STATE */, FALSE, name );
			AddLink( &g.thread_events, event );
		}
		if( event->hEvent )
		{
			//lprintf( WIDE("event opened successfully... %d"), WaitForSingleObject( hEvent, 0 ) );
			if( !SetEvent( event->hEvent ) )
				 lprintf( WIDE("Set event FAILED..%d"), GetLastError() );
			Relinquish(); // may or may not execute other thread before this...
		}
		else
		{
			lprintf( WIDE("Failed to open that event! %d"), GetLastError() );
			// thread to wake is not ready to be
			// woken, does not exist, or some other
			// BAD problem.
		}
	}
#else
#ifdef USE_PIPE_SEMS
   if( thread->semaphore != -1 )
	{
		write( thread->pipe_ends[1], "G", 1 );
		Relinquish();
	}
#else
   if( thread->semaphore != -1 )
	{
		int stat;
      int val;
		struct sembuf semdo;
		semdo.sem_num = 0;
		semdo.sem_op = 1;
		semdo.sem_flg = 0;
      //_xlprintf( 1 DBG_RELAY )( WIDE("Resetting event on %08x %016Lx"), thread->semaphore, thread->thread_ident );
      //lprintf( WIDE("Before semval = %d %08lx"), semctl( thread->semaphore, 0, GETVAL ), thread->semaphore );
		stat = semop( thread->semaphore, &semdo, 1 );
		if( stat == -1 )
		{
			if( errno != ERANGE )
				lprintf( WIDE("semop error (wake) : %d"), errno );
		}
		//lprintf( WIDE("After semval = %d %08lx"), val = semctl( thread->semaphore, 0, GETVAL ), thread->semaphore );
		if( !val )
		{
         //DebugBreak();
         //lprintf( WIDE("Did we fail the semop?!") );
		}
		Relinquish(); // may or may not execute other thread before this...
	}
#endif
   if( thread->thread ) // thread creation might not be complete yet...
		pthread_kill( thread->thread, SIGUSR1 );
#endif
}

//#undef WakeThread
//TIMER_PROC( void, WakeThread )( PTHREAD thread )
//{
//	WakeThreadEx( thread DBG_SRC );
//}

//--------------------------------------------------------------------------

TIMER_PROC( void, WakeThreadIDEx )( THREAD_ID thread DBG_PASS )
{
	PTHREAD pThread = FindThread( thread );
#ifdef _DEBUG
	g.nLineWake = nLine;
	g.pFileWake = pFile;
#endif	
	WakeThreadEx( pThread DBG_RELAY );
}

//--------------------------------------------------------------------------
#undef WakeThreadID 
TIMER_PROC( void, WakeThreadID )( THREAD_ID thread )
{
	WakeThreadIDEx( thread DBG_SRC );
}

//--------------------------------------------------------------------------
#ifdef _NO_SEMTIMEDOP_
#ifndef _WIN32
static void CPROC TimerWake( PTRSZVAL psv )
{
	WakeThreadEx( (PTHREAD)psv DBG_SRC );
}
#endif
#endif
//--------------------------------------------------------------------------

TIMER_PROC( void, WakeableSleepEx )( _32 n DBG_PASS )
{
	PTHREAD pThread = FindThread( GetMyThreadID() );
	if( pThread )
	{
#ifdef _WIN32
		if( g.flags.bLogSleeps )
			_xlprintf(1 DBG_RELAY )( WIDE("About to sleep on %d Thread event created...%016llx"), pThread->hEvent, pThread->thread_ident );
		if( WaitForSingleObject( pThread->hEvent
									  , n==SLEEP_FOREVER?INFINITE:(n) ) != WAIT_TIMEOUT )
		{
#ifdef LOG_LATENCY
			_lprintf(DBG_RELAY)( WIDE("Woke up- reset event") );
#endif
			ResetEvent( pThread->hEvent );
			//if( n == SLEEP_FOREVER )
			//   DebugBreak();
		}
#ifdef LOG_LATENCY
		else
			_lprintf(DBG_RELAY)( WIDE("Timed out from %d"), n );
#endif
#else
		{
#ifdef _NO_SEMTIMEDOP_
			int nTimer = 0;
			if( n != SLEEP_FOREVER )
			{
				//lprintf( WIDE("Wakeable sleep in %ld"), n );
				nTimer = AddTimerEx( n, 0, TimerWake, (PTRSZVAL)pThread );
			}
#endif
         if( pThread->semaphore == -1 )
			{
            //lprintf( WIDE("Invalid semaphore...fixing?") );
            InitWakeup( pThread );
			}
         if( pThread->semaphore != -1 )
			{
#ifdef USE_PIPE_SEMS
#else
				struct sembuf semdo[2];
				semdo[0].sem_num = 0;
				semdo[0].sem_op = -1;
				semdo[0].sem_flg = 0;
#endif
				while(1)
				{
					int stat;
					//lprintf( WIDE("Lock on semop on semdo... %08x %016Lx"), pThread->semaphore, pThread->thread_ident );
					//lprintf( WIDE("Before semval = %d %08lx"), semctl( pThread->semaphore, 0, GETVAL ), pThread->semaphore );
					if( n != SLEEP_FOREVER )
					{
#ifdef USE_PIPE_SEMS
                  char buf;
                  stat = read( pThread->pipe_ends[0], &buf, 1 );
#else
# ifdef _NO_SEMTIMEDOP_
						stat = semop( pThread->semaphore, semdo, 1 );
# else
						struct timespec timeout;
						timeout.tv_nsec = ( n % 1000 ) * 1000000L;
						timeout.tv_sec = n / 1000;
						stat = semtimedop( pThread->semaphore, semdo, 1, &timeout );
# endif
#endif
					}
					else
					{
#ifdef USE_PIPE_SEMS
                  char buf;
                  stat = read( pThread->pipe_ends[0], &buf, 1 );
#else
						stat = semop( pThread->semaphore, semdo, 1 );
#endif
					}
#ifdef _NO_SEMTIMEDOP_
					if( nTimer )
					{
                  //lprintf( WIDE("Removing our wakeup timer....") );
						RemoveTimer( nTimer );
					}
#endif
					//lprintf( WIDE("After semval = %d %08lx"), semctl( pThread->semaphore, 0, GETVAL ), pThread->semaphore );
					//lprintf( WIDE("Lock passed.") );
					if( stat < 0 )
					{
						if( errno == EINTR )
						{
						//lprintf( WIDE("EINTR") );
                     break;
							//continue;
						}
						if( errno == EAGAIN )
						{
                     //lprintf( WIDE("EAGAIN?") );
						// timeout elapsed on semtimedop - or IPC_NOWAIT was specified
                     // but since it's not, it must be the timeout condition.
							break;
						}
						if( errno == EIDRM )
						{
                     lprintf( WIDE("Semaphore has been removed on us!?") );
                     pThread->semaphore = -1;
                     break;
						}
						if( errno == EINVAL )
						{
							lprintf( WIDE("Semaphore is no longer valid on this thread object... %d")
									 , pThread->semaphore );
                     // this probably means that it has gone away..
							pThread->semaphore = -1;
                     break;
						}
						lprintf( WIDE("stat from sempop on thread semaphore %p = %d (%d)")
								 , pThread
								 , stat
								 , stat<0?errno:0 );
						break;
					}
					else
					{
					// reset semaphore to nothing.... might
					// have been woken up MANY times.
						//lprintf( WIDE("Resetting our lock count from %d to 0....")
					//		 , semctl( pThread->semaphore, 0, GETVAL ));
#ifdef USE_PIPE_SEMS
                  // flush? empty the pipe?
#else
						semctl( pThread->semaphore, 0, SETVAL, 0 );
#endif
						break;
					}
				}
			}
			else
			{
				lprintf( WIDE("Still an invalid semaphore? Dang.") );
            fprintf( stderr, WIDE("Out of semaphores.") );
            BAG_Exit(0);
			}
		}
#endif
		//pThread->flags.bSleeping = 0;
	}
	else
	{
      lprintf( WIDE("You, as a thread, do not exist, sorry.") );
	}
}

#undef WakeableSleep
TIMER_PROC( void, WakeableSleep )( _32 n )
#define WakeableSleep(n) WakeableSleepEx(n DBG_SRC)
{
   WakeableSleepEx(n DBG_SRC);
}

//--------------------------------------------------------------------------

#ifdef __LINUX__
static void ContinueSignal( int sig )
{
	lprintf( "Sigusr1" );
}

// network is at GLOBAL_INIT_PRIORITY
PRIORITY_PRELOAD( IgnoreSignalContinue, GLOBAL_INIT_PRELOAD_PRIORITY-1 )
{
	lprintf( "register handler for sigusr1" );
   signal( SIGUSR1, ContinueSignal );
}

static void AlarmSignal( int sig )
{
	WakeThread( g.pTimerThread );
}

static void TimerWakeableSleep( _32 n )
{
	if( g.pTimerThread )
	{
		if( !g.flags.set_timer_signal )
		{
			signal( SIGALRM, AlarmSignal );
			g.flags.set_timer_signal = 1;
		}
		if( n != SLEEP_FOREVER )
		{
			struct itimerval val;
		//lprintf( WIDE("Wakeable sleep in %") _32f WIDE(""), n );
			val.it_value.tv_sec = n / 1000;
			val.it_value.tv_usec = (n % 1000) * 1000;
			val.it_interval.tv_sec = 0;
			val.it_interval.tv_usec = 0;
			setitimer( ITIMER_REAL, &val, NULL );
		}
		if( g.pTimerThread && g.pTimerThread->semaphore != -1 )
		{
#ifdef USE_PIPE_SEMS
			char buf;
			while( read( g.pTimerThread->pipe_ends[0], &buf, 1 ) < 0 )
			{
				if( !g.pTimerThread )
               return;
				if( errno == EIDRM )
				{
					g.pTimerThread->semaphore = -1; // closed.
					return;
				}
				//lprintf( WIDE("Before semval = %d"), semctl( g.pTimerThread->semaphore, 0, GETVAL ) );
				if( errno == EINTR )
				{
					//lprintf( WIDE("Before semval = %d"), semctl( g.pTimerThread->semaphore, 0, GETVAL ) );
					continue;
				}
				else
				{
					lprintf( WIDE("read failed: %d %08x"), errno, g.pTimerThread->semaphore );
					break;
				}
			}
#else
			struct sembuf semdo;
			semdo.sem_num = 0;
			semdo.sem_op = -1;
			semdo.sem_flg = 0;
			//lprintf( WIDE("Before semval = %d %08lx")
			//		 , semctl( g.pTimerThread->semaphore, 0, GETVAL )
			//		 , g.pTimerThread->semaphore );
			while( semop( g.pTimerThread->semaphore, &semdo, 1 ) < 0 )
			{
				if( !g.pTimerThread )
               return;
				if( errno == EIDRM )
				{
					g.pTimerThread->semaphore = -1; // closed.
					return;
				}
				//lprintf( WIDE("Before semval = %d"), semctl( g.pTimerThread->semaphore, 0, GETVAL ) );
				if( errno == EINTR )
				{
					//lprintf( WIDE("Before semval = %d"), semctl( g.pTimerThread->semaphore, 0, GETVAL ) );
					continue;
				}
				else
				{
					lprintf( WIDE("Semop failed: %d %08x"), errno, g.pTimerThread->semaphore );
					break;
				}
			}
#endif
			//lprintf( WIDE("After semval = %d %08lx")
			//		 , semctl( g.pTimerThread->semaphore, 0, GETVAL )
			//		 , g.pTimerThread->semaphore );
		}
	}
}

#endif

//--------------------------------------------------------------------------
PTRSZVAL CPROC ThreadProc( PTHREAD pThread );
// results if the timer

TIMER_PROC( int, IsThisThreadEx )( PTHREAD pThreadTest DBG_PASS )
{
	PTHREAD pThread = FindThread( GetMyThreadID() );
//   lprintf( WIDE("Found thread; %p is it %p?"), pThread, pThreadTest );
	if( pThread == pThreadTest )
		return TRUE;
   //lprintf( WIDE("Found thread; %p is not  %p?"), pThread, pThreadTest );
	return FALSE;
}

static int NotTimerThread( void )
{
	PTHREAD pThread = FindThread( GetMyThreadID() );
	if( pThread && ( pThread->proc == ThreadProc ) )
		 return FALSE;
	return TRUE;
}

//--------------------------------------------------------------------------

TIMER_PROC( void, UnmakeThread )( void )
{
	PTHREAD pThread = FindThread( GetMyThreadID() );
	if( pThread )
	{
#ifdef _WIN32
      //lprintf( WIDE("Unmaking thread event! on thread %016Lx"), pThread->thread_ident );
		CloseHandle( pThread->hEvent );
#else
      closesem( (POINTER)pThread, 0 );
#endif
      // unlink from g.threads list.
		//if( ( (*pThread->me)=pThread->next ) )
		//	pThread->next->me = pThread->me;
		DeleteFromSet( THREAD, &g.threadset, pThread ) /*Release( pThread )*/;
	}
}

//--------------------------------------------------------------------------
#ifdef __WATCOMC__
static void *ThreadWrapper( PTHREAD pThread )
#else
static PTRSZVAL CPROC ThreadWrapper( PTHREAD pThread )
#endif
{
	PTRSZVAL result = 0;
#ifdef __LINUX__
	lprintf( "register handler for sigusr1 (for thread)" );
	signal( SIGUSR1, ContinueSignal );
#endif
#ifdef _WIN32
	while( !pThread->hThread )
	{
		Log( WIDE("wait for main thread to process...") );
		Relinquish();
	}
#endif
	while( !pThread->flags.bReady )
      Relinquish();
	pThread->thread_ident = GetMyThreadID();
	InitWakeup( pThread );
#ifdef LOG_THREAD
	Log1( WIDE("Set thread ident: %016Lx"), pThread->thread_ident );
#endif
	if( pThread->proc )
		 result = pThread->proc( pThread );
	//lprintf( WIDE("%s(%d):Thread is exiting... "), pThread->pFile, pThread->nLine );
	UnmakeThread();
	//lprintf( WIDE("%s(%d):Thread is exiting... "), pThread->pFile, pThread->nLine );
#ifdef __WATCOMC__
	return (void*)result;
#else
	return result;
#endif
}

//--------------------------------------------------------------------------
#ifdef __WATCOMC__
static void *SimpleThreadWrapper( PTHREAD pThread )
#else
static PTRSZVAL CPROC SimpleThreadWrapper( PTHREAD pThread )
#endif
{
	PTRSZVAL result = 0;
#ifdef _WIN32
	while( !pThread->hThread )
	{
		Log( WIDE("wait for main thread to process...") );
		Relinquish();
	}
#endif
	while( !pThread->flags.bReady )
      Relinquish();
	pThread->thread_ident = GetMyThreadID();
	InitWakeup( pThread );
#ifdef LOG_THREAD
	Log1( WIDE("Set thread ident: %016Lx"), pThread->thread_ident );
#endif
	if( pThread->proc )
		 result = pThread->simple_proc( (POINTER)GetThreadParam( pThread ) );
	//lprintf( WIDE("%s(%d):Thread is exiting... "), pThread->pFile, pThread->nLine );
	UnmakeThread();
	//lprintf( WIDE("%s(%d):Thread is exiting... "), pThread->pFile, pThread->nLine );
#ifdef __WATCOMC__
	return (void*)result;
#else
	return result;
#endif
}

//--------------------------------------------------------------------------

TIMER_PROC( PTHREAD, MakeThread )( void )
{
	PTHREAD pThread;
	THREAD_ID thread_ident = GetMyThreadID();
	if( !(pThread = FindThread( thread_ident ) ) )
	{
		PTHREAD pThread;
		while( LockedExchange( &g.lock_thread_create, 1 ) )
			Relinquish();
		pThread = GetFromSet( THREAD, &g.threadset ); /*Allocate( sizeof( THREAD ) )*/;
		lprintf( WIDE("Get Thread %p"), pThread );
		MemSet( pThread, 0, sizeof( THREAD ) );
		pThread->flags.bLocal = TRUE;
		pThread->proc = NULL;
		pThread->param = 0;
		pThread->thread_ident = thread_ident;
		pThread->flags.bReady = 1;
		//if( ( pThread->next = g.threads ) )
		//	g.threads->me = &pThread->next;
		//pThread->me = &g.threads;
		//g.threads = pThread;

		InitWakeup( pThread );
		g.lock_thread_create = 0;
#ifdef LOG_THREAD
		Log3( WIDE("Created thread address: %p %"PRIxFAST64" at %p")
			 , pThread->proc, pThread->thread_ident, pThread );
#endif
	}
	return pThread;
}

THREAD_ID GetThreadID( PTHREAD thread )
{
	if( thread )
		return thread->thread_ident;
	return 0;
}
PTRSZVAL GetThreadParam( PTHREAD thread )
{
	if( thread )
		return thread->param;
	return 0;
}

//--------------------------------------------------------------------------

TIMER_PROC( PTHREAD, ThreadToEx )( PTRSZVAL (CPROC*proc)(PTHREAD), PTRSZVAL param DBG_PASS )
{
	int success;
	PTHREAD pThread;
	while( LockedExchange( &g.lock_thread_create, 1 ) )
		Relinquish();
	pThread = GetFromSet( THREAD, &g.threadset );
	/*AllocateEx( sizeof( THREAD ) DBG_RELAY );*/
#ifdef LOG_THREAD
	Log( WIDE("Creating a new thread... ") );
	lprintf( WIDE("New thread %p"), pThread );
#endif
	MemSet( pThread, 0, sizeof( THREAD ) );
	pThread->flags.bLocal = TRUE;
	pThread->proc = proc;
	pThread->param = param;
	pThread->thread_ident = 0;
#if DBG_AVAILABLE
	pThread->pFile = pFile;
	pThread->nLine = nLine;
#endif
   g.lock_thread_create = 0;
#ifdef LOG_THREAD
	Log( WIDE("Begin Create Thread") );
#endif
#ifdef _WIN32
#if defined( __WATCOMC__ ) || defined( __WATCOM_CPLUSPLUS__ )
	pThread->hThread = (HANDLE)_beginthread( (void(*)(void*))ThreadWrapper, 8192, pThread );
#else
	{
      DWORD dwJunk;
		pThread->hThread = CreateThread( NULL, 1024
												 , (LPTHREAD_START_ROUTINE)(ThreadWrapper)
												 , pThread
												 , 0
												 , &dwJunk );
	}
#endif
    success = (int)(pThread->hThread!=NULL);
#else
	 success = !pthread_create( &pThread->thread, NULL, (void*(*)(void*))ThreadWrapper, pThread );
#endif
    if( success )
	 {
		 // link into list... it's a valid thread
       // the system claims that it can start one.
		 //if( ( ( pThread->next = g.threads ) ) )
		 //   g.threads->me = &pThread->next;
		 //pThread->me = &g.threads;
		 //g.threads = pThread;
      pThread->flags.bReady = 1;
		 while( !pThread->thread_ident )
			Relinquish();
#ifdef LOG_THREAD
	   Log3( WIDE("Created thread address: %p %016Lx at %p")
	             , pThread->proc, pThread->thread_ident, pThread );
#endif
	}
	else
	{
      // unlink from g.threads list.
		DeleteFromSet( THREAD, &g.threadset, pThread ) /*Release( pThread )*/;
		pThread = NULL;
	}
	 return pThread;
}

//--------------------------------------------------------------------------

TIMER_PROC( PTHREAD, ThreadToSimpleEx )( PTRSZVAL (CPROC*proc)(POINTER), POINTER param DBG_PASS )
{
	int success;
	PTHREAD pThread;
	while( LockedExchange( &g.lock_thread_create, 1 ) )
		Relinquish();
	pThread = GetFromSet( THREAD, &g.threadset );
	/*AllocateEx( sizeof( THREAD ) DBG_RELAY );*/
#ifdef LOG_THREAD
	Log( WIDE("Creating a new thread... ") );
	lprintf( WIDE("New thread %p"), pThread );
#endif
	MemSet( pThread, 0, sizeof( THREAD ) );
	pThread->flags.bLocal = TRUE;
	pThread->simple_proc = proc;
	pThread->param = (PTRSZVAL)param;
	pThread->thread_ident = 0;
#if DBG_AVAILABLE
	pThread->pFile = pFile;
	pThread->nLine = nLine;
#endif
   g.lock_thread_create = 0;
#ifdef LOG_THREAD
	Log( WIDE("Begin Create Thread") );
#endif
#ifdef _WIN32
#if defined( __WATCOMC__ ) || defined( __WATCOM_CPLUSPLUS__ )
	pThread->hThread = (HANDLE)_beginthread( (void(*)(void*))SimpleThreadWrapper, 8192, pThread );
#else
	{
      DWORD dwJunk;
		pThread->hThread = CreateThread( NULL, 1024
												 , (LPTHREAD_START_ROUTINE)(SimpleThreadWrapper)
												 , pThread
												 , 0
												 , &dwJunk );
	}
#endif
    success = (int)(pThread->hThread!=NULL);
#else
	 success = !pthread_create( &pThread->thread, NULL, (void*(*)(void*))SimpleThreadWrapper, pThread );
#endif
    if( success )
	 {
		 // link into list... it's a valid thread
       // the system claims that it can start one.
		 //if( ( ( pThread->next = g.threads ) ) )
		 //   g.threads->me = &pThread->next;
		 //pThread->me = &g.threads;
		 //g.threads = pThread;
      pThread->flags.bReady = 1;
		 while( !pThread->thread_ident )
			Relinquish();
#ifdef LOG_THREAD
	   Log3( WIDE("Created thread address: %p %016Lx at %p")
	             , pThread->proc, pThread->thread_ident, pThread );
#endif
	}
	else
	{
      // unlink from g.threads list.
		DeleteFromSet( THREAD, &g.threadset, pThread ) /*Release( pThread )*/;
		pThread = NULL;
	}
	 return pThread;
}

//--------------------------------------------------------------------------

TIMER_PROC( void, EndThread )( PTHREAD thread )
{
#ifdef __LINUX__
	pthread_cancel( thread->thread );
#else
	TerminateThread( thread->hThread, 0xD1E );
   lprintf( WIDE("Killing thread...") );
	CloseHandle( thread->hEvent );
#endif
}

//--------------------------------------------------------------------------
static CRITICALSECTION csGrab;

static void DoInsertTimer( PTIMER timer )
{
	PTIMER check;
	EnterCriticalSec( &csGrab );
	if( !(check = g.timers) )
	{
#ifdef LOG_INSERTS
		Log( WIDE("First(only known) timer!") );
#endif
		// subtract already existing time... (ONLY if first timer)
		//timer->delta -= ( g.this_tick - g.last_tick );
		(*(timer->me = &g.timers))=timer;
#ifdef LOG_INSERTS
		Log( WIDE("Done with addition") );
#endif
		LeaveCriticalSec( &csGrab );
		return;
	}
	while( check )
	{
		// was previously <= which would schedule equal timers at the
		// head of the queue constantly.
#ifdef LOG_INSERTS
		lprintf( WIDE("Timer to store %d freq: %d delta: %d check delta: %d"), timer->ID, timer->frequency, timer->delta, check->delta );
#endif
		if( timer->delta < check->delta )
		{
			check->delta -= timer->delta;
#ifdef LOG_INSERTS
			Log3( WIDE("Storing before timer: %d delta %d next %d"), check->ID, timer->delta, check->delta );
#endif
			timer->next = check;
			(*(timer->me = check->me))=timer;
			check->me = &timer->next;
			break;
		}
		else
		{
			timer->delta -= check->delta;
		}
		if( !check->next )
		{
#ifdef LOG_INSERTS
			Log1( WIDE("Storing after last timer. Delta %d"), timer->delta );
#endif
			(*(timer->me = &check->next))=timer;
			break;
		}
		check = check->next;
	}
#ifdef LOG_INSERTS
	Log( WIDE("Done with addition") );
#endif
	if( !check )
		Log( WIDE("Fatal! Didn't add the timer!") );
	LeaveCriticalSec( &csGrab );
}

//--------------------------------------------------------------------------

static void  DoRemoveTimer( _32 timerID )
{
	EnterCriticalSec( &csGrab );
	{
	PTIMER timer = g.timers;
	while( timer )
	{
		if( timer->ID == timerID )
			break;
		timer = timer->next;
	}
	if( timer )
	{
      PTIMER tmp;
		if( ( tmp = ( (*timer->me) = timer->next ) ) )
		{
			// if I had a next - his refernece of thing that points at him is mine.
			tmp->delta += timer->delta;
			tmp->me = timer->me;
		}
      DeleteFromSet( TIMER, &g.timer_pool, timer );
	}
	}
	LeaveCriticalSec( &csGrab );
}

//--------------------------------------------------------------------------

static void InsertTimer( PTIMER timer )
{
	if( NotTimerThread() )
	{
		if( g.flags.away_in_timer )
		{ // if it's away - should be safe to add a new timer
			g.flags.insert_while_away = 1;
			// set that we're adding a timer while away
			if( g.flags.away_in_timer )
			{
				// if the thread is still away - we can add the timer...
#ifdef LOG_SLEEPS
				lprintf( "Timer is away, just add this new timer back in.." );
#endif
				DoInsertTimer( timer );
				g.flags.insert_while_away = 0;
				return;
			}
			// otherwise he came back before we set our addin
			// therefore it should be safe to schedule.
			g.flags.insert_while_away = 0;
		}
#ifdef LOG_INSERTS
		Log( WIDE("Inserting timer...to wait for change allow") );
#endif
											  // lockout multiple additions...
      EnterCriticalSec( &g.cs_timer_change );
#ifdef LOG_INSERTS
		Log( WIDE("Inserting timer...to wait for free add") );
#endif
		// don't add a timer while there's one being added...
		while( g.add_timer )
		{
			WakeThread(g.pTimerThread);
			//Relinquish();
		}
#ifdef LOG_INSERTS
		Log( WIDE("Inserting timer...setup dataa") );
#endif
		g.add_timer = timer;
      LeaveCriticalSec( &g.cs_timer_change );
		// it might be sleeping....
#ifdef LOG_INSERTS
		Log( WIDE("Inserting timer...wake and done") );
#endif

#ifdef LOG_SLEEPS
		lprintf( "Wake timer thread." );
#endif
		// wake this thread because it's current scheduled delta (ex 1000ms)
		// may put it's sleep beyond the frequency of this timer (ex 10ms)
		WakeThread(g.pTimerThread);
	}
	else
	{
		EnterCriticalSec( &csGrab );
		// have to assume that we're away in callback
		// in order to get here... there's no other way
		// for this routine to be called and BE the timer thread.
		// therefore - safe to add it.
		DoInsertTimer( timer );
#ifdef LOG_SLEEPS
		lprintf( "Insert timer not dispatched." );
#endif
		if( g.timers == timer )
		{
#ifdef LOG_SLEEPS
			lprintf( "Wake timer thread." );
#endif
			WakeThread(g.pTimerThread);
		}
		LeaveCriticalSec( &csGrab );
	}
}

//--------------------------------------------------------------------------

static PTIMER GrabTimer( PTIMER timer )
{
	// if a timer has been grabbed, it won't be grabbed...
	// but if a timer is being grabbed, it might get grabbed twice.
	EnterCriticalSec( &csGrab );
	if( timer && timer->me )
	{
		// the thing that points at me points at my next....
#ifdef LOG_INSERTS
		Log1( WIDE("Grab Timer: %d"), timer->ID );
#endif
		if( ( (*timer->me) = timer->next ) )
		{
			// if I had a next - his refernece of thing that points at him is mine.
			timer->next->me = timer->me;
		}
		timer->next = NULL;
		timer->me = NULL;
		LeaveCriticalSec( &csGrab );
		return timer;
	}
	LeaveCriticalSec( &csGrab );
	return NULL;
}

//--------------------------------------------------------------------------

static int CPROC ProcessTimers( PTRSZVAL psvForce )
{
	PTIMER timer;
	_32 newtick;
	if( g.flags.bExited )
      return -1;
	if( !psvForce && !IsThisThread( g.pTimerThread ) )
	{
		//Log( WIDE("Unknown thread attempting to process timers...") );
		return -1;
	}
#ifndef _WIN32
	//nice( -3 ); // allow ourselves a bit more priority...
#endif
	{
		// there are timers - and there's one which wants to be added...
		// if there's no timers - just sleep here...
		while( !g.add_timer && !g.timers )
		{
			if( !psvForce )
				return 1;
#ifdef LOG_SLEEPS
			lprintf( WIDE("Timer thread sleeping forever...") );
#endif
#ifdef __LINUX__
         if( g.pTimerThread )
				TimerWakeableSleep( SLEEP_FOREVER );
#else
			WakeableSleep( SLEEP_FOREVER );
#endif
			// had no timers - but NOW either we woke up by default...
			// OR - we go kicked awake - so mark the beginning of known time.
#ifdef LOG_LATENCY
			Log( WIDE("Re-synch first tick...") );
#endif
			g.last_tick = timeGetTime();//GetTickCount();
		}
		// add and delete new/old timers here... 
		// should be the next event after sleeping (low var-sleep top const-sleep)
		if( g.add_timer )
		{
#ifdef LOG_INSERTS
			Log( WIDE("Adding timer really...") );
#endif
			DoInsertTimer( g.add_timer );
			g.add_timer = NULL;
		}
		if( g.del_timer )
		{
#ifdef LOG_INSERTS
			Log( WIDE("Scheduled remove timer...") );
#endif
			DoRemoveTimer( g.del_timer );
			g.del_timer = 0;
		}
		// get the time now....
		newtick = g.this_tick = timeGetTime();//GetTickCount();
#ifdef LOG_LATENCY
		Log3( WIDE("total - Tick: %u Last: %u  delta: %u"), g.this_tick, g.last_tick, g.this_tick-g.last_tick );
#endif
		//if( g.timers )
		//	 delay_skew = g.this_tick-g.last_tick - g.timers->delta;
		// delay_skew = 0; // already chaotic...
		 //if( timers )
		//	Log1( WIDE("timer delta: %ud"), timers->delta );
      
		while( ( EnterCriticalSec( &csGrab ), timer = g.timers ) &&
				( (S_32)( newtick - g.last_tick ) >= timer->delta ) )
		{
#ifdef LOG_LATENCY
#ifdef _DEBUG
			_xlprintf( 1, timer->pFile, timer->nLine )( WIDE("Tick: %u Last: %u  delta: %u Timerdelta: %u")
					  , g.this_tick, g.last_tick, g.this_tick-g.last_tick, timer->delta );
#else
			lprintf( WIDE("Tick: %u Last: %u  delta: %u Timerdelta: %u")
					  , g.this_tick, g.last_tick, g.this_tick-g.last_tick, timer->delta );
#endif
#endif
         // also enters csGrab... should be ok.
			GrabTimer( timer );
         LeaveCriticalSec( &csGrab );
			g.last_tick += timer->delta;
			if( timer->callback )
			{
#ifdef _WIN32
#if PARANOID
				if( IsBadCodePtr( (FARPROC)timer->callback ) )
				{
					Log1( WIDE("Timer %d proc has been unloaded! kiling timer"), timer->ID );
					timer->frequency = 0;
				}
				else
#endif
#endif
				{
#ifdef LOG_DISPATCH
					Log1( WIDE("Dispatching timer %ld"), timer->ID );
#endif
					g.current_timer = timer;
					timer->flags.bRescheduled = 0;
					g.flags.away_in_timer = 1;
					g.CurrentTimerID = timer->ID;
					timer->callback( timer->userdata );
					g.flags.away_in_timer = 0;
					while( g.flags.insert_while_away )
					{
					// request for insert while away... allow it to
                  // get scheduled...
						Relinquish();
					}
					g.current_timer = NULL;
 				}
				// allow timers to be added while away in this
				// timer's callback... so wait for it to finish.
				// but do - clear away status so that ANOTHER
				// timer will be held waiting...
			}
			// reset timer to frequency here

				// if a VERY long time has elapsed, next timer occurs its
				//  frequency after now.  Otherwise we may NEVER get out
				//  of processing this timer.
				// this point should be optioned whether the timer is
				// a guaranteed tick, or whether it's sloppy.
				if( timer->frequency || timer->flags.bRescheduled )
				{
					if( timer->flags.bRescheduled )
					{
                  timer->flags.bRescheduled = 0;
					// delta will have been set for next run...
                  // therefore do not schedule it ourselves.
					}
					else
					{
						if( ( newtick - g.last_tick ) > timer->frequency )
						{
#ifdef LOG_LATENCY_LIGHT
							lprintf( WIDE("Timer used more time than its frequency.  Scheduling at 1 ms.") );
#endif
							timer->delta = ( newtick - g.last_tick ) + 1;
						}
						else
						{
#ifdef LOG_LATENCY_LIGHT
						// timer alwyas goes +1 frequency from its base tick.
                     lprintf( WIDE("Scheduling timer at 1 frequency.") );
#endif
							timer->delta = timer->frequency;
						}
					}
					DoInsertTimer( timer );
				}
				else
				{
#ifdef LOG_INSERTS
					Log( WIDE("Removing one shot timer.") );
#endif
					// was a one shot timer.
					DeleteFromSet( TIMER, &g.timer_pool, timer );
				}
		 }
		 LeaveCriticalSec( &csGrab );
		 if( timer )
		 {
#ifdef LOG_LATENCY
				lprintf( WIDE("Pending timer in: %d Sleeping %d (%d) [%d]")
					 , timer->delta
					 , timer->delta - (newtick-g.last_tick)
					 , timer->delta - (g.this_tick-g.last_tick)
					 , newtick - g.this_tick
					 );
#endif
				g.last_sleep = ( timer->delta - ( g.this_tick - g.last_tick ) );
				if( g.last_sleep < 0 )
				{
               lprintf( WIDE( "next pending sleep is %d" ), g.last_sleep );
					g.last_sleep = 1;
				}
#ifdef LOG_LATENCY
				Log1( WIDE("Sleeping %d"), g.last_sleep );
#endif
				if( !psvForce )
					return 1;
				if( g.last_sleep )
				{
#ifdef __LINUX__
				TimerWakeableSleep( g.last_sleep );
#else
#ifdef _DEBUG
				WakeableSleepEx( g.last_sleep, timer->pFile, timer->nLine );
#else
				WakeableSleepEx( g.last_sleep );
#endif
#endif
				}
				if( g.flags.bExited )
					return -1;
		 }
		 // else no timers - go back up to the top - where we sleep.
	}
	//Log( WIDE("Timer thread is exiting...") );
	return 1;
}


//--------------------------------------------------------------------------

PTRSZVAL CPROC ThreadProc( PTHREAD pThread )
{
	g.pTimerThread = pThread;
	AddIdleProc( ProcessTimers, (PTRSZVAL)0 );
#ifndef _WIN32
	nice( -3 ); // allow ourselves a bit more priority...
#endif
	//Log( WIDE("Permanently lock timers - indicates that thread is running...") );
	g.lock_timers = 1;
	//Log( WIDE("Get first tick") );
	g.last_tick = timeGetTime();//GetTickCount();
	while( ProcessTimers( 1 ) );
	Log( WIDE("Timer thread is exiting...") );
	return 0;
}

//--------------------------------------------------------------------------
#if 0
// this would really be a good thing to impelment someday.
static void *WatchdogProc( void *unused )
{
	// this checks the running status of the main thread(s)
// if there is a paused thread, then a new thread is created.
// yeah see dekware( syscore/nexus.c WakeAThread() )
	return 0;
}
#endif
//--------------------------------------------------------------------------

TIMER_PROC( _32, AddTimerExx )( _32 start, _32 frequency, TimerCallbackProc callback, PTRSZVAL user DBG_PASS )
{
	PTIMER timer = GetFromSet( TIMER, &g.timer_pool );
					 //timer = AllocateEx( sizeof( TIMER ) DBG_RELAY );
	MemSet( timer, 0, sizeof( TIMER ) );
	if( start && !frequency )
	{
		//"Creating one shot timer %d long", start );
	}
	timer->delta	 = (S_32)start; // first time for timer to fire... may be 0
	timer->frequency = frequency;
	timer->callback = callback;
	timer->ID = g.timerID++;
	timer->userdata = user;
#ifdef _DEBUG
	timer->pFile = pFile;
	timer->nLine = nLine;
#endif
	if( !g.pTimerThread )
	{
		 //Log( WIDE("Starting \"a\" timer thread!!!!" ) );
		 if( !( ThreadTo( ThreadProc, 0 ) ) )
		 {
				//Log1( WIDE("Failed to start timer ThreadProc... %d"), GetLastError() );
				return 0;
		 }
		 while( !g.lock_timers )
				Relinquish();
		 //Log1( WIDE("Thread started successfully? %d"), GetLastError() );

		 // make sure that the thread is running, and had setup its
		 // locks, and tick reference
	}
   //_xlprintf(1 DBG_RELAY)( WIDE("Inserting newly created timer.") );
	InsertTimer( timer );
	// don't need to sighup here, cause we MUST have permission
	// from the idle thread to add the timer, which means we issue it
	// a sighup to make it wake up and allow us to post.
#ifdef LOG_INSERTS
	_lprintf( DBG_RELAY )( "Resulting timer ID: %d", timer->ID );
#endif
	return timer->ID;
}

#undef AddTimerEx
TIMER_PROC( _32, AddTimerEx )( _32 start, _32 frequency, void (CPROC*callback)(PTRSZVAL user), PTRSZVAL user )
{
	return AddTimerExx( start, frequency, callback, user DBG_SRC );
}

//--------------------------------------------------------------------------

TIMER_PROC( void, RemoveTimer )( _32 ID )
{
	// Lockout multiple changes at a time...
	if( !NotTimerThread() && // IS timer thread..
		( ID != g.CurrentTimerID ) ) // and not in THIS timer...
	{
		// is timer thread itself... safe to remove the timer....
		DoRemoveTimer( ID );
		return;
	}
	EnterCriticalSec( &g.cs_timer_change );
	// only allow one delete at a time...
	while( g.del_timer )
	{
		if( !NotTimerThread() ) // IS timer thread...
		{
			if( g.del_timer != g.CurrentTimerID )
			{
				DoRemoveTimer( g.del_timer );
				g.del_timer = 0;
			}
			if( ID != g.CurrentTimerID )
			{
				DoRemoveTimer( ID );
				return;
			}
		}
		else
			Relinquish();
	}
	// now how to set del_timer to a valid timer?!
	g.del_timer = ID;
	LeaveCriticalSec( &g.cs_timer_change );
	if( NotTimerThread() )
	{
		//Log( WIDE("waking timer thread to indicate deletion...") );
		WakeThread( g.pTimerThread );
	}
}

//--------------------------------------------------------------------------

static void InternalRescheduleTimerEx( PTIMER timer, _32 delay )
{
	if( timer )
	{
		PTIMER bGrabbed = GrabTimer( timer );
      timer->flags.bRescheduled = 1;
		timer->delta = (S_32)delay;  // should never pass a negative value here, but delta can be negative.
#ifdef LOG_SLEEPS
		lprintf( "Reschedule at %d  %p", timer->delta, bGrabbed );
#endif
		if( bGrabbed )
		{
         //lprintf( WIDE("Rescheduling timer...") );
			DoInsertTimer( timer );
			if( timer == g.timers )
			{
#ifdef LOG_SLEEPS
				lprintf( "We cheated to insert - so create a wake." );
#endif
				WakeThread( g.pTimerThread );
			}
		}
	}
}

//--------------------------------------------------------------------------

// should lock this...
TIMER_PROC( void, RescheduleTimerEx )( _32 ID, _32 delay )
{
	PTIMER timer;
	EnterCriticalSec( &csGrab );
	if( !ID )
	{
      timer =g.current_timer;
	}
	else
	{
		timer = g.timers;
		while( timer && timer->ID != ID )
			timer = timer->next;

		if( !timer )
		{
			// this timer is not part of the list if it's
         // dispatched and we get here (timer itself rescheduling itself)
			if( g.current_timer && g.current_timer->ID == ID )
				timer = g.current_timer;
		}
	}
	InternalRescheduleTimerEx( timer, delay );
	LeaveCriticalSec( &csGrab );
}

//--------------------------------------------------------------------------

TIMER_PROC( void, RescheduleTimer )( _32 ID )
{
	PTIMER timer = g.timers;
	EnterCriticalSec( &csGrab );
	while( timer && timer->ID != ID )
		timer = timer->next;
	if( !timer )
	{
		if( g.current_timer && g.current_timer->ID == ID )
         timer = g.current_timer;
	}
	if( timer )
	{
		InternalRescheduleTimerEx( timer, timer->frequency );
	}
	LeaveCriticalSec( &csGrab );
}

//--------------------------------------------------------------------------

TIMER_PROC( void, ChangeTimerEx )( _32 ID, _32 initial, _32 frequency )
{
	PTIMER timer = g.timers;
	while( timer && timer->ID != ID )
		timer = timer->next;
	if( timer )
	{
		timer->frequency = frequency;
		InternalRescheduleTimerEx( timer, initial );
	}
}

//--------------------------------------------------------------------------

TIMER_PROC( LOGICAL, EnterCriticalSecEx )( PCRITICALSECTION pcs DBG_PASS )
{
	int d;
	THREAD_ID prior = 0;
#ifdef _DEBUG
   _32 curtick = timeGetTime();//GetTickCount();
#endif
	if( g.flags.bLogCriticalSections )
		_lprintf( DBG_RELAY )( "Enter critical section %p %"_64fx, pcs, GetMyThreadID() );
	do
	{
		d=EnterCriticalSecNoWaitEx( pcs, &prior DBG_RELAY );
		if( d < 0 )
		{
			// lock critical failed - couldn't update.
			Relinquish();
#ifdef _DEBUG
			if( ( curtick+2000) < timeGetTime() )//GetTickCount() )
			{
				xlprintf(1)( WIDE( "Timeout during critical section wait for lock.  No lock should take more than 1 task cycle %ld %ld" ), curtick, timeGetTime() );//GetTickCount() );
				DebugBreak();
				return FALSE;
			}
			curtick = timeGetTime();//GetTickCount();
#endif
		}
		else if( d == 0 )
		{
			if( pcs->dwThreadID )
			{
				if( g.flags.bLogCriticalSections )
					lprintf( WIDE("Failed to enter section... sleeping...") );
				WakeableSleepEx( SLEEP_FOREVER DBG_RELAY );
				if( g.flags.bLogCriticalSections )
					lprintf( WIDE("Allowed to retry section entry, woken up...") );
#ifdef _DEBUG
				curtick = timeGetTime();//GetTickCount();
#endif
			}
			else
			{
				if( g.flags.bLogCriticalSections )
					lprintf( WIDE("Lock Released while we logged?") );
			}
		}
		// after waking up, this will re-aquire a lock, and
		// set the prior waiting ID into the criticalsection
		// this will then wake that process when this lock is left.
	}
	while( (d <= 0) );
	return TRUE;
}
//-------------------------------------------------------------------------

TIMER_PROC( LOGICAL, LeaveCriticalSecEx )( PCRITICALSECTION pcs DBG_PASS )
{
	THREAD_ID dwCurProc = GetMyThreadID();
#ifdef _DEBUG
   _32 curtick = timeGetTime();//GetTickCount();
#endif
	if( g.flags.bLogCriticalSections )
		_xlprintf( LOG_NOISE DBG_RELAY )( "Begin leave critical section %p %"_64fx, pcs, GetMyThreadID() );
	while( LockedExchange( &pcs->dwUpdating, 1 ) 
#ifdef _DEBUG
			&& ( (curtick+2000) > timeGetTime() )//GetTickCount() )
#endif
	)
	{
		if( g.flags.bLogCriticalSections )
			_lprintf( DBG_RELAY )( "On leave - section is updating, wait..." );
		Relinquish();
	}
#ifdef _DEBUG
	if( (curtick+2000) < timeGetTime() )//GetTickCount() )
	{
		lprintf( WIDE( "Timeout during critical section wait for lock.  No lock should take more than 1 task cycle" ) );
		DebugBreak(); 
		return FALSE;
	}
#endif
	if( !( pcs->dwLocks & ~SECTION_LOGGED_WAIT ) )
	{
		if( g.flags.bLogCriticalSections )
			_lprintf( DBG_RELAY )( "Leaving a blank critical section" );
		pcs->dwUpdating = 0;
		return FALSE;
	}

	if( pcs->dwThreadID == dwCurProc )
	{
#ifdef DEBUG_CRITICAL_SECTIONS
		pcs->pFile = pFile;
		pcs->nLine = nLine;
#endif
		pcs->dwLocks--;
		if( g.flags.bLogCriticalSections )
			lprintf( "Remaining locks... %08" _32fx, pcs->dwLocks );
		if( !( pcs->dwLocks & ~(SECTION_LOGGED_WAIT) ) )
		{
			pcs->dwLocks = 0;
			pcs->dwThreadID = 0;
			// wake the prior (if there is one sleeping)
			if( pcs->dwLocks & SECTION_LOGGED_WAIT)
				DebugBreak();

			if( pcs->dwThreadWaiting )
			{
				THREAD_ID wake = pcs->dwThreadWaiting;
				//pcs->dwThreadWaiting = 0;
				pcs->dwUpdating = 0;
				if( g.flags.bLogCriticalSections )
					_lprintf( DBG_RELAY )( "%8LxWaking a thread which is waiting...", wake );
				// don't clear waiting... so the proper thread can
				// allow itself to claim section...
				WakeThreadIDEx( wake DBG_RELAY);
			}
			else
				pcs->dwUpdating = 0;
		}
		else
			pcs->dwUpdating = 0;
	}
	else
	{
		if( g.flags.bLogCriticalSections )
		{
			_lprintf( DBG_RELAY )( WIDE("Sorry - you can't leave a section owned by %016Lx locks:%08lx" )
#ifdef DEBUG_CRITICAL_SECTIONS
										 WIDE(  "%s(%d)...")
#endif
										, pcs->dwThreadID
										, pcs->dwLocks
#ifdef DEBUG_CRITICAL_SECTIONS
										, (pcs->pFile)?(pcs->pFile):"Unknown", pcs->nLine
#endif
										);
		}
		pcs->dwUpdating = 0;
		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------------------

TIMER_PROC( void, DeleteCriticalSec )( PCRITICALSECTION pcs )
{
   // ya I don't have anything to do here...
	return;
}

#ifdef __cplusplus 
};//	namespace timers {
};//namespace sack {
#endif
//--------------------------------------------------------------------------

// $Log: timers.c,v $
// Revision 1.140  2005/06/22 23:13:51  jim
// Differentiate the normal logging of 'entered, left section' but leave in notable exception case logging when enabling critical section debugging.
//
// Revision 1.139  2005/06/08 22:25:27  jim
// Remove noisy loging
//
// Revision 1.138  2005/05/26 21:35:03  jim
// Restore debug line for debug-critical-sections that shows when sections are entered...
//
// Revision 1.137  2005/05/17 18:38:18  jim
// Remove shutdown logging... causes problems since it seems the C library has shutdown IO access in a previous atexit routine
//
// Revision 1.136  2005/05/17 01:18:30  jim
// Close semaphore on thread unmake.
//
// Revision 1.135  2005/05/16 23:21:03  jim
// Replace commented logging with compile-optioned logging.
//
// Revision 1.134  2005/05/16 19:07:11  jim
// Extend wakeable sleep to know the originator of the sleep.  Removed some noisy logging messages for windows.
//
// Revision 1.133  2005/05/16 17:22:03  jim
// Modify shutdown prioirty
//
// Revision 1.132  2005/05/13 23:51:51  jim
// Massive changes to enhance ATEXIT, implement a PRIORITY_ATEXIT, and be happy.
//
// Revision 1.131  2005/05/12 20:59:05  jim
// Remove noisy logging even from debug-logging
//
// Revision 1.130  2005/05/10 22:53:50  jim
// Remove extra unused function
//
// Revision 1.129  2005/05/10 22:43:05  jim
// Add compile-option to not use semtimedop - which apparently is a bleeding edge feature.
//
// Revision 1.128  2005/05/05 19:04:39  jim
// Remove another dangling reference to timers - which causes a crash when exiting.
//
// Revision 1.127  2005/05/03 21:52:56  jim
// Oops left a comiple-time debug option on... and removed a couple noisy logging statements.
//
// Revision 1.126  2005/05/03 21:45:22  jim
// Implement semtimedop (hope it's there) Remove some logging comments... fix some compiler warnings...
//
// Revision 1.125  2005/04/20 00:25:11  jim
// Cleaning up logging statements - vast performance increase.
//
// Revision 1.124  2005/04/19 22:49:30  jim
// Looks like the display module technology nearly works... at least exits graceful are handled somewhat gracefully.
//
// Revision 1.123  2005/04/18 15:52:51  jim
// Much debugging, better protection on semaphore operations under linux.  Maybe even cleanup correctly now.
//
// Revision 1.122  2005/03/30 03:26:37  panther
// Checkpoint on stabilizing display projects, and the exiting thereof
//
// Revision 1.121  2005/03/29 22:28:29  panther
// Take care of closing our memory spaces which we might use later.... should also destroy active timers.
//
// Revision 1.120  2005/03/23 01:55:36  panther
// Order of forallinset callback was backwards
//
// Revision 1.119  2005/03/16 23:25:52  panther
// Duh sorry wrong symbol for threadID
//
// Revision 1.118  2005/03/16 22:30:25  panther
// On MakeThread check to see if the thread refernece already exists and return that one if it does.
//
// Revision 1.117  2005/03/15 18:42:30  panther
// Thread waits for main to queue it, main waits for thread to set its ID, thread positively active when done.
//
// Revision 1.116  2005/02/23 13:01:35  panther
// Fix scrollbar definition.  Also update vc projects
//
// Revision 1.115  2005/02/09 22:47:27  panther
// oops - double linked myslef into my list (self circularly)
//
// Revision 1.114  2005/02/09 22:24:58  panther
// Pull thread resources from a set also...
//
// Revision 1.113  2005/02/04 19:28:41  panther
// Check currently dispatched timer also when looking for a reschedule
//
// Revision 1.112  2005/01/27 07:10:51  panther
// Windows cleaned.
//
// Revision 1.111  2005/01/27 07:10:03  panther
// Linux cleaned.
//
// Revision 1.110  2005/01/26 06:50:09  panther
// Use a TIMERSET instead of dynamic allocate/release...
//
// Revision 1.109  2005/01/25 10:23:41  panther
// Made several functions static, trimmed logging, commented about watchdog
//
// Revision 1.108  2005/01/23 11:28:24  panther
// Thread ID modification broke timer...
//
// 400 lines of logging removed... version 1.109?