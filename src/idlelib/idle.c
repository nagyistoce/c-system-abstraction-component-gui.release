#include <stdhdrs.h>
#include <sack_types.h>
#include <sharemem.h>
#include <timers.h>
#include <idle.h>

#include <procreg.h>
#include <deadstart.h>

#ifdef __cplusplus
namespace sack {
	namespace timers {
		using namespace sack::memory;
#endif
      typedef struct idle_proc_tag IDLEPROC;
      typedef struct idle_proc_tag *PIDLEPROC;
struct idle_proc_tag
{
	struct {
		_32 bDispatched : 1;
		_32 bRemove : 1;
	} flags;
	// return -1 if not the correct thread
	// to handle this callback
	// return 0 if no messages were processed
   // return 1 if messages were processed
	int (CPROC*function)(PTRSZVAL);
	PTRSZVAL data;
	THREAD_ID thread;
	//PDATAQUEUE threads;
   PIDLEPROC similar; // same function references go here - for multiple thread entries...
   DeclareLink( struct idle_proc_tag );
};

static PIDLEPROC *registered_idle_procs;
#define procs (*registered_idle_procs)
PRIORITY_PRELOAD( InitGlobal, OSALOT_PRELOAD_PRIORITY )
{
   SimpleRegisterAndCreateGlobal( registered_idle_procs );
}
//PLIST pIdleProcs;
//PLIST pIdleData;

IDLE_PROC( void, AddIdleProc )( int (CPROC*Proc)( PTRSZVAL psv ), PTRSZVAL psvUser )
{
	PIDLEPROC proc = NULL;
	if( !registered_idle_procs )
		SimpleRegisterAndCreateGlobal( registered_idle_procs );

	for( proc = procs; proc; proc = proc->next )
	{
		if( Proc == proc->function )
		{
			PIDLEPROC newproc = (PIDLEPROC)Allocate( sizeof( IDLEPROC ) );
			newproc->thread = 0;
			//newproc->threads = CreateDataQueue( sizeof( THREAD_ID ) );
			newproc->flags.bDispatched = 0;
			newproc->flags.bRemove = 0;
			newproc->function = Proc;
			newproc->data = psvUser;
         newproc->similar = NULL;
         LinkLast( proc->similar, PIDLEPROC, newproc );
			break;
		}
	}
	// if the function is not already registered as an idle proc, register it.
	if( !proc )
	{
		proc = (PIDLEPROC)Allocate( sizeof( IDLEPROC ) );
		proc->thread = 0;
      //proc->threads = CreateDataQueue( sizeof( THREAD_ID ) );
		proc->flags.bDispatched = 0;
		proc->flags.bRemove = 0;
		proc->function = Proc;
		proc->data = psvUser;
      proc->similar = NULL;
		LinkThing( procs, proc );
	}
}

IDLE_PROC( int, RemoveIdleProc )( int (CPROC*Proc)(PTRSZVAL psv ) )
{
	PIDLEPROC check_proc;
	for( check_proc = procs; check_proc; check_proc = check_proc->next )
	{
		if( Proc == check_proc->function )
		{
			if( !check_proc->flags.bDispatched )
			{
				UnlinkThing( check_proc );
				if( check_proc->similar )
               LinkThing( check_proc->similar, procs );
				Release( check_proc );
			}
			else
			{
				check_proc->flags.bRemove = 1;
			}
			break;
		}
	}
	return 0;
}

IDLE_PROC( int, IdleEx )( DBG_VOIDPASS )
{
	THREAD_ID me = GetMyThreadID();
	int success = 0;
	PIDLEPROC proc;
	for( proc = procs; proc;  )
	{
		PIDLEPROC check;
		for( check = proc; check; check = check->similar )
		{
			check->flags.bDispatched = 1;
			//lprintf( "attempt proc %p in %Lx  procthread=%Lx", check, GetThreadID( MakeThread() ), check->thread );
			//if( !check->thread || ( check->thread == me ) )
         // sometimes... a function belongs to multiple threads...
				if( check->function( check->data ) != -1 )
				{
					check->thread = me;
					success = 1;
				}
			check->flags.bDispatched = 0;
			if( check->flags.bRemove )
			{
				UnlinkThing( check );
				if( check->similar && check == proc )
               LinkThing( check->similar, procs );
				Release( proc );
				proc = procs;
            break;
			}
			else
			{
				//if( check->thread == me )
				{
					proc = proc->next;
					break;
				}
			}
		}
			if( check == NULL )
            proc = proc->next;
	}
	//_lprintf( DBG_AVAILABLE, WIDE("Is Going idle.") DBG_RELAY );
	Relinquish();
	//_lprintf( DBG_AVAILABLE, WIDE("Is back from idle.") DBG_RELAY );
	return success;
}

#undef Idle
IDLE_PROC( int, Idle )( void )
{
   return IdleEx( DBG_VOIDSRC );
}

IDLE_PROC( int, IdleForEx )( _32 dwMilliseconds DBG_PASS )
{
	_32 dwStart = GetTickCount();
	while( ( dwStart + dwMilliseconds ) > GetTickCount() )
	{
		if( !IdleEx( DBG_VOIDRELAY ) )
		{
         // sleeping... cause ew didn't do any idle procs...
         WakeableSleep( dwMilliseconds );
		}
	}
   return 0;
}

#undef IdleFor
IDLE_PROC( int, IdleFor )( _32 dwMilliseconds )
{
   return IdleForEx( dwMilliseconds DBG_SRC );
}


#ifdef __cplusplus
}; //namespace sack {
}; //	namespace idle {
#endif

//---------------------------------------------------
// $Log: idle.c,v $
// Revision 1.17  2005/01/27 07:08:19  panther
// Linux cleaned.
//
// Revision 1.16  2004/10/02 06:16:16  d3x0r
// Initialize new thread member of idle proc.
//
// Revision 1.15  2004/09/29 00:49:48  d3x0r
// Added fancy wait for PSI frames which allows non-polling sleeping... Extended Idle() to result in meaningful information.
//
// Revision 1.14  2004/05/24 17:44:13  d3x0r
// Okay really really fix types - other was adding loggin
//
// Revision 1.13  2004/05/24 17:42:49  d3x0r
// Fixed idle return types
//
// Revision 1.12  2003/11/10 03:54:57  panther
// Init proc to work right...
//
// Revision 1.11  2003/11/09 22:35:06  panther
// Attempt to fix removal of idleproc allocated memory. Standardize linked list methods
//
// Revision 1.10  2003/10/26 01:58:37  panther
// Hmm amazing that this didn't crash and burn!
//
// Revision 1.9  2003/10/21 00:21:47  panther
// Export CloseSpaceEx (sharemem.h).  Unwind circular dependancy for idle.
//
// Revision 1.8  2003/09/18 07:53:20  panther
// Added to idle - IdleFor - which sleeps for a time, calling idle procs
//
// Revision 1.7  2003/07/24 22:50:37  panther
// Updates to make watcom happier
//
// Revision 1.6  2003/07/22 15:28:54  panther
// Added cvs logging
//
//
