/*
 *  Crafted by Jim Buckeyne
 * 
 *  (c)2001-2006++ Freedom Collective
 *
 *  Provide API interface for timers, critical sections
 *  and other thread things.
 *
 */

#ifndef TIMERS_DEFINED
#define TIMERS_DEFINED

#include <sack_types.h>
#include <sharemem.h>

#ifdef __cplusplus
/* define a timer library namespace in C++. */
#define TIMER_NAMESPACE SACK_NAMESPACE namespace timers {
/* define a timer library namespace in C++ end. */
#define TIMER_NAMESPACE_END } SACK_NAMESPACE_END
#else
#define TIMER_NAMESPACE 
#define TIMER_NAMESPACE_END
#endif

TIMER_NAMESPACE


#ifdef TIMER_SOURCE 
#define TIMER_PROC(type,name) EXPORT_METHOD type name
#else
#define TIMER_PROC(type,name) IMPORT_METHOD type name
#endif

#ifdef __LINUX__
TIMER_PROC( _32, GetTickCount )( void );
TIMER_PROC( void, Sleep )( _32 ms );
#endif

TIMER_PROC( _32, AddTimerExx )( _32 start, _32 frequency
					, void (CPROC*callback)(PTRSZVAL user)
					, PTRSZVAL user DBG_PASS);
#define AddTimerEx( s,f,c,u ) AddTimerExx( (s),(f),(c),(u) DBG_SRC )
#define AddTimer( f, c, u ) AddTimerExx( (f), (f), (c), (u) DBG_SRC)

TIMER_PROC( void, RemoveTimer )( _32 timer );
TIMER_PROC( void, RescheduleTimerEx )( _32 timer, _32 delay );
// default delay to reschedule == timer frequency
TIMER_PROC( void, RescheduleTimer )( _32 timer );
TIMER_PROC( void, ChangeTimerEx )( _32 ID, _32 initial, _32 frequency );
#define ChangeTimer( ID, Freq ) ChangeTimerEx( ID, Freq, Freq )

// if a timer is dispatched and needs to wait - please link with
// idlelib, and call Idle();

typedef struct threads_tag THREAD, *PTHREAD;

typedef PTRSZVAL (CPROC*THREAD_PROC)( PTHREAD );


TIMER_PROC( PTHREAD, ThreadToEx )( THREAD_PROC proc, PTRSZVAL param DBG_PASS );
#define ThreadTo(proc,param) ThreadToEx( proc,param DBG_SRC )
TIMER_PROC( PTHREAD, MakeThread )( void );
TIMER_PROC( void, UnmakeThread )( void );
TIMER_PROC( PTRSZVAL, GetThreadParam )( PTHREAD thread );
TIMER_PROC( THREAD_ID, GetThreadID )( PTHREAD thread );

#define SLEEP_FOREVER 0xFFFFFFFF
TIMER_PROC( void, WakeableSleepEx )( _32 milliseconds DBG_PASS );
TIMER_PROC( void, WakeableSleep )( _32 milliseconds );
#define WakeableSleep(n) WakeableSleepEx(n DBG_SRC )

TIMER_PROC( void, WakeThreadIDEx )( THREAD_ID thread DBG_PASS );
TIMER_PROC( void, WakeThreadEx )( PTHREAD thread DBG_PASS );

#define WakeThreadID(thread) WakeThreadIDEx( thread DBG_SRC )
#define WakeThread(t) WakeThreadEx(t DBG_SRC )

TIMER_PROC( int, TestWakeThreadID )( THREAD_ID thread );
TIMER_PROC( int, TestWakeThread )( PTHREAD thread );

//TIMER_PROC( void, WakeThread )( PTHREAD thread );

TIMER_PROC( void, EndThread )( PTHREAD thread );
TIMER_PROC( int, IsThisThreadEx )( PTHREAD pThreadTest DBG_PASS );
#define IsThisThread(thread) IsThisThreadEx(thread DBG_SRC)

TIMER_PROC( LOGICAL, EnterCriticalSecEx )( PCRITICALSECTION pcs DBG_PASS );
#define EnterCriticalSec( pcs ) EnterCriticalSecEx( (pcs) DBG_SRC )
TIMER_PROC( LOGICAL, LeaveCriticalSecEx )( PCRITICALSECTION pcs DBG_PASS );
#define LeaveCriticalSec( pcs ) LeaveCriticalSecEx( (pcs) DBG_SRC )
TIMER_PROC( void, DeleteCriticalSec )( PCRITICALSECTION pcs );

TIMER_NAMESPACE_END
#ifdef __cplusplus
using namespace sack::timers;
#endif

#endif
// $Log: timers.h,v $
// Revision 1.37  2005/05/16 19:06:58  jim
// Extend wakeable sleep to know the originator of the sleep.
//
// Revision 1.36  2004/09/29 16:42:51  d3x0r
// fixed queues a bit - added a test wait function for timers/threads
//
// Revision 1.35  2004/07/07 15:33:54  d3x0r
// Cleaned c++ warnings, bad headers, fixed make system, fixed reallocate...
//
// Revision 1.34  2004/05/02 02:04:16  d3x0r
// Begin border exclusive option, define PushMethod explicitly, fix LaunchProgram in timers.h
//
// Revision 1.33  2003/12/10 15:38:25  panther
// Move Sleep and GetTickCount to real code
//
// Revision 1.32  2003/11/02 00:31:47  panther
// Added debuginfo pass to wakethread
//
// Revision 1.31  2003/10/24 14:59:21  panther
// Added Load/Unload Function for system shared library abstraction
//
// Revision 1.30  2003/10/17 00:56:04  panther
// Rework critical sections.
// Moved most of heart of these sections to timers.
// When waiting, sleep forever is used, waking only when
// released... This is preferred rather than continuous
// polling of section with a Relinquish.
// Things to test, event wakeup order uner linxu and windows.
// Even see if the wake ever happens.
// Wake should be able to occur across processes.
// Also begin implmeenting MessageQueue in containers....
// These work out of a common shared memory so multiple
// processes have access to this region.
//
// Revision 1.29  2003/09/21 04:03:30  panther
// Build thread ID with pthread_self and getgid
//
// Revision 1.28  2003/07/29 10:41:25  panther
// Predefine struct threads_tag to avoid warning
//
// Revision 1.27  2003/07/24 22:49:20  panther
// Define callback procs as CDECL
//
// Revision 1.26  2003/07/24 16:56:41  panther
// Updates to expliclity define C procedure model for callbacks and assembly modules - incomplete
//
// Revision 1.25  2003/07/22 15:33:19  panther
// Added comment about idle()
//
// Revision 1.24  2003/04/03 10:10:20  panther
// Add file/line debugging to addtimer
//
// Revision 1.23  2003/03/27 13:47:14  panther
// Immplement a EndThread
//
// Revision 1.22  2003/03/25 08:38:11  panther
// Add logging
//
