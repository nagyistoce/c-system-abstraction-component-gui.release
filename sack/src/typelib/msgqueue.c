/* 
 * Crafted by Jim Buckeyne
 * Resembles function of SYSV IPC Message Queueus, and handle event based, inter-process, shared
 * queue, message transport.
 *
 * (c)1999-2006++ Freedom Collective
 *
 */

#include <stdhdrs.h> // Sleep

#include <stddef.h> // offsetof

#include <sharemem.h>

#include <sack_types.h>
#include <timers.h>

#ifdef __LINUX__
#define SetLastError(n)  errno = n
#endif

#ifdef __cplusplus
namespace sack {
	namespace containers {
	namespace message {
		using namespace sack::memory;
		using namespace sack::timers; 	
		using namespace sack::logging;
#endif


#define DISABLE_MSGQUE_LOGGING
#define DISABLE_MSGQUE_LOGGING_DETAILED
static _32 _tmp, __tmp;

void _UpdatePos( _32 *tmp, _32 inc DBG_PASS )
{
	if( inc == 0 )
      DebugBreak();
	__tmp = _tmp;
	_tmp = (*tmp);
   (*tmp) += inc;
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
	_xlprintf(5 DBG_RELAY )( WIDE("updating position from %d,%d,%d by %d"), __tmp, _tmp, (*tmp), inc );
#endif
}

void _SetPos( _32 *tmp, _32 inc, _32 initial DBG_PASS )
{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
   if( initial )
		_xlprintf(5 DBG_RELAY )( WIDE("Setting position to %d"), inc );
   else
		_xlprintf(5 DBG_RELAY )( WIDE("Setting position from %d to %d"), (*tmp), inc );
#endif
	if( !initial )
	{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
		__tmp = _tmp;
		_tmp = (*tmp);
#endif
		(*tmp) = inc;
	}
	else
	{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
		__tmp = _tmp =
#endif
			(*tmp) = inc;
	}
}
#define UpdatePos(t,i) _UpdatePos( &(t), (i) DBG_SRC )
#define SetPos(t,i) _SetPos( &(t), (i), FALSE DBG_SRC )
#define SetPosI(t,i) _SetPos( &(t), (i), TRUE DBG_SRC )

//--------------------------------------------------------------------------
//  MSG Queue functions.
//--------------------------------------------------------------------------
typedef PREFIX_PACKED struct MsgInternalData {
	_32 length;
	_32 real_length; // size resulting in read...
   // space which we added ourselves...
} PACKED MSGCORE;

typedef PREFIX_PACKED struct MsgData
{
	union {
		MSGCORE msg;
		struct
		{
			_32 length;
			_32 real_length;
		};
	};
	// length  & 0x80000000 == after msg, return to head of queue
	// length == 0x80000000 == next is actually first in queue...
	// length & 0x40000000 message has already been received...
	// length & 0x20000000 message tag for request for specific message
	//   then length(low) points at next waiting.
	//   MsgID is the ID being waited for
   //   datalength is sizeof( THREAD_ID ) and data is MyThreadID()
	_32 MsgID;
	// ... end of this structure is
	// defined by length & 0x0FFFFFFC
} PACKED MSGDATA, *PMSGDATA;

typedef PREFIX_PACKED struct ThreadWaitMsgData
{
	union {
		MSGDATA msgdata;
		struct {
			union {
				MSGCORE msgcore;
				struct
				{
					_32 length;
					_32 real_length;
				};
			};
			_32 MsgID;
		};
	};
   THREAD_ID thread;
	// ... end of this structure is
	// defined by length & 0x0FFFFFFC
} PACKED THREADMSGDATA, *PTHREADMSGDATA;

typedef struct MsgDataQueue
{
   TEXTCHAR name[128];
   _32     Top;
   _32     Bottom;
	_32     Cnt;   // number of times this is open, huh?
   _32     waiter_top; // reference of first element in queue waiting for specific ID
   _32     waiter_bottom; // reference of first element in queue waiting for specific ID
	//THREAD_ID waiting;  // a thread waiting for any message...
	CRITICALSECTION cs;
	_32     Size;
	// this is a lot of people using this queue....
	// 1000 unique people waiting for a message....
   // this is fairly vast..
	struct {
		_32 msg; // and this is the message I wait for.
      THREAD_ID me;  // my ID - who I am that I am waiting...
	} waiters[1024];
   _8      data[1];
} MSGQUEUE, *PMSGQUEUE;

typedef struct MsgDataHandle
{
	PMSGQUEUE pmq;
	MsgQueueReadCallback Read;
	PTRSZVAL psvRead;
   DeclareLink( struct MsgDataHandle );
} MSGHANDLE;

//--------------------------------------------------------------------------


//--------------------------------------------------------------------------

// in this case size is the size of the queue, there
// is no expansion possible...
// this should be created such that it is at least 3 * largest message
 PMSGHANDLE  SackCreateMsgQueue ( CTEXTSTR name, PTRSZVAL size
													  , MsgQueueReadCallback Read
														, PTRSZVAL psvRead )
{
   	PMSGHANDLE pmh;
	PMSGQUEUE pmq;
	PTRSZVAL dwSize = size + sizeof( MSGQUEUE );
	_32 bCreated;
#ifdef __LINUX__
	TEXTCHAR tmpname[128];
   sprintf( tmpname, WIDE("/tmp/%s"), name );
#endif
	pmq = (PMSGQUEUE)OpenSpaceExx(
#ifdef __LINUX__
							 NULL
						  , tmpname
#else
							 name
						  , NULL
#endif
						  , 0
						  , &dwSize
							, &bCreated );
	if( !pmq )
		return NULL;
	pmh          = (PMSGHANDLE)Allocate( sizeof( MSGHANDLE ) );
	pmh->Read    = Read;
	pmh->psvRead = psvRead;
	pmh->pmq     = pmq;
	// now - how to see if result is new...
	// space is 0'd on create.
	// so if the second open results before the create
	// always increment this - otherwise the create open will
	// obliterate the second opener's presense.
	StrCpyEx( pmq->name, name?name:WIDE("Anonymous"),127 );
	pmq->name[127] = 0;
	pmq->Cnt++;
	if( bCreated )
	{
		pmq->Size = size;
	}
	return pmh;
}

//--------------------------------------------------------------------------

// in this case size is the size of the queue, there
// is no expansion possible...
// this should be created such that it is at least 3 * largest message
 PMSGHANDLE  SackOpenMsgQueue ( CTEXTSTR name
													 , MsgQueueReadCallback Read
													 , PTRSZVAL psvRead )
{
	PMSGHANDLE pmh;
	PMSGQUEUE pmq;
	PTRSZVAL dwSize = 0;
	_32 bCreated;
#ifdef __LINUX__
	char tmpname[128];
   sprintf( tmpname, WIDE("/tmp/%s"), name );
#endif
	pmq = (PMSGQUEUE)OpenSpaceExx(
#ifdef __LINUX__
							 NULL
						  , tmpname
#else
							 name
						  , NULL
#endif
						  , 0
						  , &dwSize
							, &bCreated );
	if( !pmq )
		return NULL;
	pmh          = (PMSGHANDLE)Allocate( sizeof( MSGHANDLE ) );
	pmh->Read    = Read;
	pmh->psvRead = psvRead;
	pmh->pmq     = pmq;
	// now - how to see if result is new...
	// space is 0'd on create.
	// so if the second open results before the create
	// always increment this - otherwise the create open will
	// obliterate the second opener's presense.
	StrCpyEx( pmq->name, name?name:WIDE("Anonymous"), sizeof( pmq->name ) );
	pmq->Cnt++;
	if( bCreated )
	{
      lprintf( WIDE("SackOpenMsgQueue should never result with a created queue!") );
      DebugBreak();
		//pmq->Size = size;
	}
   return pmh;
}

//--------------------------------------------------------------------------

 void  DeleteMsgQueueEx ( PMSGHANDLE *ppmh DBG_PASS )
{

	if( ppmh )
	{
		EnterCriticalSec( &(*ppmh)->pmq->cs );
		if( (*ppmh)->pmq )
		{
			int owners;
			owners = --(*ppmh)->pmq->Cnt;
			lprintf( WIDE("Remaining owners of queue: %d"), owners );
			CloseSpaceEx( (*ppmh)->pmq, (!owners) );
		}
		Release( *ppmh );
		*ppmh = NULL;
	}
}

#define DBG_SOURCE DBG_SRC
//--------------------------------------------------------------------------

#define ACTUAL_LEN_MASK           0x000FFFFF
#define MARK_END_OF_QUE           0x80000000
#define MARK_MESSAGE_ALREADY_READ 0x40000000
#define MARK_THREAD_WAITING       0x20000000
#define MESSAGE_SKIPABLE          0xc0000000

//--------------------------------------------------------------------------

#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
void DumpWaiterQueue( PMSGQUEUE pmq )
{
	INDEX tmp;
	tmp = pmq->waiter_bottom;
	while( tmp != pmq->waiter_top )
	{
		lprintf( WIDE("[%d] waiter sleeping is %016") _64fX WIDE(" for %") _32f WIDE("")
              , tmp
				 , pmq->waiters[tmp].me
				 , pmq->waiters[tmp].msg
				 );
		tmp++;
		if( tmp >= 1024 )
         tmp = 0;
	}
}
#endif
//--------------------------------------------------------------------------

static void CollapseWaiting( PMSGQUEUE pmq, _32 msg )
{
	//int nWoken = 0;

	S_32 tmp = pmq->waiter_bottom;
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
	lprintf( WIDE("before moving the waiters forward on %s... msg %ld (or me? %s)"), pmq->name, msg, msg?"no":"yes" );
	DumpWaiterQueue( pmq );
#endif
	// now walk tmp backwards...
	// and move entried before the threads woken forward...
	// end up with a new bottom always (if having awoken something)
	if( pmq->waiter_top != pmq->waiter_bottom )
	{
		//_32 last = 0;
		_32 found = 0;
		_32 marked = 0;
		S_32 next = pmq->waiter_top - 1;
		INDEX tmp_bottom = pmq->waiter_bottom;
		if( next < 0 )
			next = 1023;
		// start at the last used queue spot (tmp)
		tmp = next;
		tmp_bottom = pmq->waiter_bottom;
		while( 1 )
		{
			if( !pmq->waiters[next].me ||
				( msg &&
				 ( msg == pmq->waiters[next].msg ) ) )
			{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
				lprintf( WIDE("Skipping a next %d... "), next );
#endif
				if( !marked )
				{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
					lprintf( WIDE("marking tmp to copy into...") );
#endif
					tmp = next;
               marked = 1;
				}
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
				else
					lprintf( WIDE("Already marked... and moving every element..") );
#endif
				if( (++tmp_bottom) >= 1024 )
				{
					tmp_bottom = 0;
				}
            // no reason to move next if it's still NULL...
				//continue;
			}
			else
			{
            found = 1;
				if( marked )
				{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
					lprintf( WIDE("Marked something... and now we move next %d into %d"), next, tmp );
#endif
					pmq->waiters[tmp--] = pmq->waiters[next];
					if( tmp < 0 )
						tmp = 1023;
					//continue;
				}
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
				lprintf( WIDE("Next queue element is kept... set temp here.") );
#endif
			}
			// update next
			if( next == pmq->waiter_bottom )
            break;
         next--;
			if( next < 0 )
				next = 1023;
		}
		if( !found )
		{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
			lprintf( WIDE("Found nothing of interest.  Empty queue.") );
#endif
         pmq->waiter_bottom = pmq->waiter_top;
		}
		else if( marked )
		{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
			lprintf( WIDE("Moving final into last position, and updating bottom to %d"), tmp_bottom );
#endif
         pmq->waiters[tmp] = pmq->waiters[next];
			pmq->waiter_bottom = tmp_bottom;
		}
	}

#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
   lprintf( WIDE("And then after moving waiters forward....") );
	DumpWaiterQueue( pmq );
#endif
}

//--------------------------------------------------------------------------

static int ScanForWaiting( PMSGQUEUE pmq, _32 msg )
{
	int nWoken = 0;

	INDEX tmp = pmq->waiter_bottom;
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
   lprintf( WIDE("before Scanning waiting on %s"), pmq->name );
	DumpWaiterQueue( pmq );
#endif
	while( tmp != pmq->waiter_top )
	{
		if( pmq->waiters[tmp].me )
		{
			// if waiting for any message....
			// or waiting for the exact message... and it is that message
			// or waiting for any other message... and it's not the message...
			if( !pmq->waiters[tmp].msg ||
				( (msg & 0x80000000)
				 ? (pmq->waiters[tmp].msg != (msg & 0x7FFFFFFF))
				 : (pmq->waiters[tmp].msg == msg) ) )
			{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
				lprintf( WIDE("Wake thread %016Lx"), pmq->waiters[tmp].me );
#endif
				WakeThreadID( pmq->waiters[tmp].me );
				// reset the waiter ID... it's been
				// awoken, and is no longer waiting....
				pmq->waiters[tmp].me = 0;
				nWoken++;
				// go through all possible people who might wake up
				// because of this message... it's typically bad form
				// to have two or more processes watiing on the same ID...
			}
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
			else
			{
				lprintf( WIDE("Not waking thread %016Lx %08lx %08lx"), pmq->waiters[tmp].me
						 , msg, pmq->waiters[tmp].msg );
			}
#endif
		}
		tmp++;
		if( tmp >= 1024 )
         tmp = 0;
	}

	// now walk tmp backwards...
	// and move entried before the threads woken forward...
   // end up with a new bottom always (if having awoken something)
   if( nWoken )
	{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
		lprintf( WIDE("Scanning to delete messages that have been awoken.") );
#endif
      CollapseWaiting( pmq, 0 );
	}

	if( nWoken > 1 )
	{
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
		lprintf( WIDE("Woke %d threads as a result of message id %08lx"), nWoken, msg );
#endif
	}
   return 0;
}

//--------------------------------------------------------------------------

 int  EnqueMsgEx ( PMSGHANDLE pmh,  POINTER msg, PTRSZVAL size, _32 opts DBG_PASS )
{
	if( pmh )
	{
		PMSGQUEUE pmq = pmh->pmq;
		INDEX tmp;
#ifndef DISABLE_MSGQUE_LOGGING
		int bNoSpace = 0;
#endif
		_32 realsize = (( size + (sizeof( MSGDATA ) ) ) + 3 ) & 0x7FFFFFFC;
		if( !pmq )
		{
         // errno = ENOSPACE; // or something....
			return -1; // cannot create this - no idea how big.
		}
		if( ( size > ( pmq->Size >> 2 ) )
			||( realsize > ( pmq->Size >> 2 ) ) )
		{
         //errno = E2BIG;
			return -1; // message is too big for this queue...
		}
		// probably someday need an error variable of
		// some sort or another...
#ifndef DISABLE_MSGQUE_LOGGING
		_xlprintf(3 DBG_RELAY)( WIDE("Enque space left...raw: %d %d Avail: %d %d used: %d %d %d")
				 , pmq->Top, pmq->Bottom
				 , pmq->Bottom-pmq->Top, pmq->Size-pmq->Top + pmq->Bottom
				 , pmq->Top-pmq->Bottom, pmq->Size-pmq->Bottom + pmq->Top
				 , realsize );
		_xlprintf(3 DBG_RELAY)( WIDE("[%s] ENqueMessage [%p] %d len %d %08lx"), pmq->name, pmq, *(P_32)msg, size, *(P_32)(pmq->data + pmq->Bottom) );
      //LogBinary( pmq->data + pmq->Bottom, 32 );
#endif
		while( msg )
		{
			int nWaiting;
			PMSGDATA pStoreMsg;
			EnterCriticalSecEx( &pmq->cs DBG_SOURCE );
			pStoreMsg = (PMSGDATA)(pmq->data + pmq->Top);
			SetPosI( tmp, pmq->Top + realsize );
			if( tmp == (pmq->Size) )
			{
				// space is exactly what we need.
				pStoreMsg->real_length = size;
				pStoreMsg->length = realsize | MARK_END_OF_QUE | (( opts & MSGQUE_WAIT_ID )?MARK_THREAD_WAITING:0 );
#ifndef DISABLE_MSGQUE_LOGGING
				lprintf( WIDE("New tmp will be 0.") );
#endif
				SetPos( tmp, 0 );
			}
			else
			{
				if( tmp >= ( pmq->Size - sizeof( MSGDATA ) ) )
				{
					// okay - this message is too big to fit here...
					// going to have to store at start, or I suppose whenever the
					// queue has enough space...
#ifndef DISABLE_MSGQUE_LOGGING
					lprintf( WIDE("space left is not big enough for the message... %d %d %d %d"), pmq->Top, pmq->Bottom, pmq->Bottom-pmq->Top, pmq->Size-pmq->Top + pmq->Bottom );
#endif
					if( ( pmq->Bottom == 0 ) ||
						( pmq->Bottom <= realsize ) )
					{
						// Need to wait for some space...
						LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
						if( opts & MSGQUE_NOWAIT )
						{
							//errno = EAGAIN;
							return -1;
						}
#ifndef DISABLE_MSGQUE_LOGGING
						lprintf( WIDE("bottom isn't far enough away either. Waiting for space") );
#endif
						Relinquish(); // someone's gotta run and take their message.
						continue;
					}
#ifndef DISABLE_MSGQUE_LOGGING
					lprintf( WIDE("Setting step to origin in length, going to origin, setting data") );
#endif
					// 0 data length, marked end, just junk...
					// okay there, and it's deleted, so noone can read it
					// even if they want a zero byte message :)
					pStoreMsg->length = MARK_END_OF_QUE|MARK_MESSAGE_ALREADY_READ;
					// tmp needs to point to the next top.
					SetPos( tmp, realsize );
					pStoreMsg = (PMSGDATA)pmq->data;
				}
				else
				{
					// this is the size of this msg... we can store
					// and there IS room for another message header of
					// at least 0 bytes at the end.
					if( tmp > pmq->Bottom && pmq->Top < pmq->Bottom )
					{
						lprintf( WIDE("No room left in queue...") );
						LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
						Relinquish();
						continue; // try again from the top...
					}
				}
				pStoreMsg->real_length = size;
				pStoreMsg->length = realsize | (( opts & MSGQUE_WAIT_ID )?MARK_THREAD_WAITING:0 );
			}
			if( tmp == pmq->Bottom )
			{
#ifndef DISABLE_MSGQUE_LOGGING
				bNoSpace = 1;
				lprintf( WIDE("Head would collide with tail...") );
#endif
				LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
				if( opts & MSGQUE_NOWAIT )
				{
					//errno = EAGAIN;
					return -1;
				}
#ifndef DISABLE_MSGQUE_LOGGING
				lprintf( WIDE("Waiting for space") );
#endif
				Relinquish(); // someone's gotta run and take their message.
				continue;
			}
			else
			{
#ifndef DISABLE_MSGQUE_LOGGING
				if( bNoSpace )
					lprintf( WIDE("Okay there's space now...") );
#endif
			}
			MemCpy( &pStoreMsg->MsgID, msg, size + sizeof( pStoreMsg->MsgID ) );
#ifndef DISABLE_MSGQUE_LOGGING
			lprintf( WIDE("[%s] Stored message data..... at %d %d"), pmq->name, pmq->Top ,size );
			LogBinary( (POINTER)pStoreMsg, size + sizeof( pStoreMsg->MsgID ) + offsetof( MSGDATA, MsgID ) );
#endif
			msg = NULL;
#ifndef DISABLE_MSGQUE_LOGGING
			lprintf( WIDE("Update top to %d"),tmp );
#endif
			pmq->Top = tmp;
			if( !(opts & MSGQUE_WAIT_ID) )
			{
				// look for, and wake anyone waiting for this
				// type of message... or anyone waiting on any message
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
				lprintf( WIDE("not sending a wait, therefore scan for messages...") );
#endif
				nWaiting = ScanForWaiting( pmq, pStoreMsg->MsgID );
				LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
				if( nWaiting )
					Relinquish();
			}
			else
			{
				//lprintf( WIDE("Okay then we leave here?") );
				LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
			}
		}
		// return success
		return 0;
	}
	// errno = EINVAL;
	return -1; // fail if no pmh
}

	//--------------------------------------------------------------------------

 int  IsMsgQueueEmpty ( PMSGHANDLE pmh )
{
   PMSGQUEUE pmq = pmh->pmq;
   if( !pmq || ( pmq->Bottom == pmq->Top ) )
      return TRUE;
   return FALSE;
}

//--------------------------------------------------------------------------

// if this thread id known, you may change the MsgID
// being waited for, which will result in this waking up
// and reading for the new ID...
 int  DequeMsgEx ( PMSGHANDLE pmh, _32 *MsgID, POINTER result, PTRSZVAL size, _32 options DBG_PASS )
{
	PMSGQUEUE pmq = pmh->pmq;
	int p;
	int slept = 0;
	INDEX tmp
      , _tmp
	;
	INDEX _Bottom, _Top;

   //_64 tick, tick2;
	if( !pmq )
      return 0;
	// if there's a read routine, this should not be called.
   // instead the routine to handle
	p = 0;
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
	_xlprintf(3 DBG_RELAY)( WIDE("[%s] Enter dequeue... for %") _32f WIDE(""), pmq->name, MsgID?*MsgID:0 );
#endif
	EnterCriticalSecEx( &pmq->cs DBG_SOURCE );
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
	_xlprintf(3 DBG_RELAY)( WIDE("Deque space left... Top:%d Bottom:%d Avail: %d %d used: %d %d")
			 , pmq->Top, pmq->Bottom
			 , pmq->Bottom-pmq->Top, pmq->Size-pmq->Top + pmq->Bottom
			 , pmq->Top-pmq->Bottom, pmq->Size-pmq->Bottom + pmq->Top );
#endif
	_Bottom = INVALID_INDEX;
	_Top = INVALID_INDEX;
	while( !p && !slept )
	{
		PTHREADMSGDATA pThreadMsg = NULL;
		PMSGDATA pReadMsg;
		PMSGDATA pLastReadMsg;
		_tmp = tmp = pmq->Bottom;
		//lprintf( WIDE("tmp = %d"), tmp );
		if( !(options & MSGQUE_NOWAIT) )
		{
			_32 LastMsgID = *MsgID;
			// then here we must wait...
			// if the queue is empty, or we've already
			// checked the queue, go to sleep.
			while( ( ((_tmp = tmp),(tmp=pmq->Bottom)) == pmq->Top ||
					(pmq->Bottom == _Bottom &&
					pmq->Top == _Top )) && !slept )
			{
            //lprintf( WIDE("no message, waiting...") );
				{
					_32 tmp_top = pmq->waiter_top + 1;
					if( tmp_top >= 1024 )
						tmp_top = 0;

#if 0
					// do a scan to see if already waiting...
					// but since we're single process... there may be multipel threads
					// interacting here?  one on the server side reading, one on the client
					// but still only one ID for that queue should be waiting for
					// any specific message....
               // not sure why this doesn't work to avoid redundant wakeups
					tmp_top = pmq->waiter_bottom;
					while( tmp_top != pmq->waiter_top )
					{
						//lprintf( WIDE("Checking %d msg:%d"), tmp_top, pmq->waiters[tmp_top].msg );
						if( pmq->waiters[tmp_top].msg == *MsgID )
						{
                     //lprintf( WIDE("waiting... leave...") );
							break;
						}
						tmp_top++;
						if( tmp_top >= 1024 )
                     tmp_top = 0;
					}
#else
					tmp_top = pmq->waiter_top;
#endif
					// if waiter for message si already registered...
               // do not mark him.
					if( tmp_top == pmq->waiter_top )
					{
						tmp_top++;
						if( tmp_top >= 1024 )
							tmp_top = 0;
						if( tmp_top != pmq->waiter_bottom )
						{
							pmq->waiters[pmq->waiter_top].me = GetMyThreadID();
							pmq->waiters[pmq->waiter_top].msg = *MsgID;
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
							lprintf( WIDE("New waiter - waiting for %016") _64fX WIDE(" %") _32f WIDE("")
									 , pmq->waiters[pmq->waiter_top].me
									 , pmq->waiters[pmq->waiter_top].msg );
#endif
							pmq->waiter_top = tmp_top;
						}
						else
						{
							lprintf( WIDE("CRITICAL ERROR - No space to mark this process to wait.") );
							Relinquish();
							continue;
						}
					}
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
					else
						lprintf( WIDE("Already waiting...") );
               DumpWaiterQueue( pmq );
#endif
				}
				LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
				lprintf( WIDE("(left section) sleeping until message (%016Lx)")
						 , GetMyThreadID() );
#endif
				// if someone wakes wakeable sleep - either a> there's a new message
				// or b> someone wants to wakeup a process from Idle()... and we need to return
				slept = 1;
				WakeableSleep( SLEEP_FOREVER );
            // remove wait message...
#ifndef DISABLE_MSGQUE_LOGGING_DETAILED
				lprintf( WIDE("Re-enter critical section here...(%016Lx)")
						 , GetMyThreadID() );
#endif
				EnterCriticalSecEx( &pmq->cs DBG_SOURCE );
				CollapseWaiting( pmq, LastMsgID );
				if( (*MsgID) == INVALID_INDEX ) 
				{
					lprintf( WIDE( "Aborting waiting read..." ) );
					SetLastError( MSGQUE_ERROR_EABORT );
					break;
				}
				//pmq->waiting = prior;
			}
#ifndef DISABLE_MSGQUE_LOGGING
			//lprintf( WIDE("Fetching a message...") );
#endif
		}
		else
		{
         // if the queue is empty, then result now with no message.
			if( tmp == pmq->Top )
			{
#ifndef DISABLE_MSGQUE_LOGGING
				lprintf( WIDE("[%s] NOWAIT option selected... resulting NOMSG."), pmq->name );
#endif
				SetLastError( MSGQUE_ERROR_NOMSG );
	            LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
				return -1;
			}
		}
#ifndef DISABLE_MSGQUE_LOGGING
		_xlprintf( 1 DBG_RELAY )( WIDE("------- tmp = %d bottom=%d top = %d ------"), tmp, pmq->Bottom, pmq->Top );
#endif
		pLastReadMsg = NULL;

		while( tmp != pmq->Top )
		{
			// after returning a message, the next should be checked
			// 2 conditions - if the length == MARK_END_OF_QUE or
			// length & 0x40000000 then the next message needs to
			// be consumed until said condition is not set.
			pReadMsg = (PMSGDATA)(pmq->data + tmp);
#ifndef DISABLE_MSGQUE_LOGGING
			//lprintf( WIDE("Check for a message at %d (%08lx)"), tmp, pReadMsg->length );
         //LogBinary( (POINTER)pReadMsg, pReadMsg->real_length + sizeof( pReadMsg->MsgID ) + sizeof( MSGDATA ));
#endif
			if( pReadMsg->length & MARK_MESSAGE_ALREADY_READ )
			{
#ifndef DISABLE_MSGQUE_LOGGING
				//lprintf( WIDE("Message has already been read...") );
#endif
				if( tmp == pmq->Bottom )
				{
					if( pReadMsg->length & MARK_END_OF_QUE )
					{
						SetPos( tmp, 0 );
						pmq->Bottom = tmp;
					}
					else
					{
						UpdatePos( tmp, pReadMsg->length & ACTUAL_LEN_MASK );
                  pmq->Bottom = tmp;
					}
				}
				else
				{
					if( pReadMsg->length & MARK_END_OF_QUE )
						SetPos( tmp, 0 );
               else
						UpdatePos( tmp, pReadMsg->length & ACTUAL_LEN_MASK );
				}
				// skip messages already read. (and/or throw them out
				// by updating bottom, we remove them from the queue
				// with no further consideration.
				if( pLastReadMsg && pLastReadMsg->length & MARK_MESSAGE_ALREADY_READ )
					pLastReadMsg->length += pReadMsg->length & ACTUAL_LEN_MASK;
            else
					pLastReadMsg = pReadMsg;
            continue;
			}
         // just looped around to kill deleted messages.
			if( p )
            break;

			if( pReadMsg->length & MARK_THREAD_WAITING )
			{
#ifndef DISABLE_MSGQUE_LOGGING
				lprintf( WIDE("Is a thread waiting message...") );
#endif
				// skip these... don't care on read?
				// well maybe we care if this is the
				// wait message of me, in which case I can
				// clean it up. It's likely the first message
				// in the queue when I get awoke, it may be
				// early - but all other near messages will
				// likely also be thread wakes...
				if( ((PTHREADMSGDATA)pReadMsg)->thread == GetMyThreadID() )
				{
					// retest this current message as already read.
#ifndef DISABLE_MSGQUE_LOGGING
					lprintf( WIDE("And it's the message that denoted *I* was waiting... delete please") );
#endif
					if( tmp != pmq->Bottom )
					{
						PTHREADMSGDATA pTmpMsg = (PTHREADMSGDATA)(pmq->data + pmq->Bottom);
						if( pTmpMsg->length & MARK_THREAD_WAITING )
						{
							((PTHREADMSGDATA)pReadMsg)->thread = pTmpMsg->thread;
							((PTHREADMSGDATA)pReadMsg)->MsgID = pTmpMsg->MsgID;
							lprintf( WIDE("Mark message as having been read( should be a temporary wait message...") );
							pTmpMsg->length |= MARK_MESSAGE_ALREADY_READ;
						}
						else
						{
							lprintf( WIDE("First message in queue is not a thread wait?!") );
							pReadMsg->length |= MARK_MESSAGE_ALREADY_READ;
						}
					}
					else
					{
						lprintf( WIDE("Mark message as having been read( should be a temporary wait message...") );
						pReadMsg->length |= MARK_MESSAGE_ALREADY_READ;
					}
               // and now move forward still...
               UpdatePos( tmp, pReadMsg->length & ACTUAL_LEN_MASK );
               continue;
				}
				if( !pThreadMsg )
				{
					pThreadMsg = (PTHREADMSGDATA)pReadMsg;
				}
				else
				{
					// concatentate this new one in the old one.
					// assuming there's space...
				}
				UpdatePos( tmp, pReadMsg->length & ACTUAL_LEN_MASK );
				pLastReadMsg = NULL;
				continue;
			}
			if( !(*MsgID) || ( pReadMsg->MsgID == (*MsgID) ) )
			{
				if( size > ( pReadMsg->length & ACTUAL_LEN_MASK ) )
				{
					p = pReadMsg->real_length + sizeof( pReadMsg->MsgID );
					MemCpy( result
							, &pReadMsg->MsgID
							, p );
#ifndef DISABLE_MSGQUE_LOGGING
					lprintf( WIDE("DequeMessage [%p] %d len %d"), result, *(P_32)result, p+sizeof( MSGDATA ) );
					LogBinary( (POINTER)result, p );
#endif
					pReadMsg->length |= MARK_MESSAGE_ALREADY_READ;
				}
				else
				{
#ifdef __LINUX__
					errno = E2BIG;
#endif
					return -1;
				}
            //lprintf( WIDE("...") );
				continue; // end loop...
			}
			else if( *MsgID )
			{
#ifndef DISABLE_MSGQUE_LOGGING
				lprintf( WIDE("Looking for a message %d...at %d haven't found one yet."), *MsgID, tmp );
				LogBinary( (POINTER)pReadMsg, (pReadMsg->length + sizeof( MSGCORE )) & ACTUAL_LEN_MASK );
#endif
				if( pReadMsg->length & MARK_END_OF_QUE )
					SetPos( tmp, 0 );
				else
					UpdatePos( tmp, ( pReadMsg->length & ACTUAL_LEN_MASK ) );
				pLastReadMsg = NULL;
            continue;
			}
			// clear off this message...
			if( tmp == pmq->Bottom )
			{
				if( pReadMsg->length & MARK_END_OF_QUE )
					SetPos( tmp, pmq->Bottom = 0 );
				else
				{
					UpdatePos( tmp, (pReadMsg->length & ACTUAL_LEN_MASK) );
               pmq->Bottom = tmp;
				}
			}
		}

		if( !p )
		{
#ifndef DISABLE_MSGQUE_LOGGING
			lprintf( WIDE("No message found... looping...") );
#endif
			if( options & MSGQUE_NOWAIT )
			{
				lprintf( WIDE("Retunign - not looping...err uhh...") );
				SetLastError( MSGQUE_ERROR_NOMSG );
				LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
				return -1;
			}
			_Bottom = pmq->Bottom;
			_Top = pmq->Top;
		}
	}
#ifndef DISABLE_MSGQUE_LOGGING
	//lprintf( WIDE("Leaving section here...") );
#endif
	LeaveCriticalSecEx( &pmq->cs DBG_SOURCE );
#ifndef DISABLE_MSGQUE_LOGGING
	if( p >= sizeof( *MsgID ) )
	{
		//lprintf( WIDE("DEqueMessage [%p] %d len %d"), result, *(P_32)result, p-sizeof( MsgID ) );
		//LogBinary( result, p );
	}
#endif
	if( !p )
	{
		SetLastError( MSGQUE_ERROR_NOMSG );
		return -1;
	}
	return p - sizeof( MsgID );
}

#ifdef __cplusplus
}; //	namespace message {
}; // namespace containers {
}; //namespace sack {
#endif

