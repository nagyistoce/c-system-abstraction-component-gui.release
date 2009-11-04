
struct common_transport_tag;
struct queue_msg_tag;
struct transport_queue_tag;

#define TRANSPORT_STRUCTURE_DEFINED

#include <sharemem.h>

// so we need a reliable, portable, process event notification method.
// Signals are fun.  But Windows doesn't really do signals, event objects
// seemed to be rather fast.  If the process waiting is of similar priority
// then a simple sched yield would work.  If it's quietly waiting 
// event notification, and may be processing other works... then a thread
// per client must be used.  I used a thread itself and a callback 
// mechanism.


typedef struct common_transport_tag
{     	
	_32 size;
	CRITICALSECTION lock;
	_64 creator_process;
	_16 open;
	void (*InputCallback)( PTRSZVAL psv
								, struct common_transport_tag *transport );
	PTRSZVAL psvInput;
	void (*Connect)( PTRSZVAL psv
	               , struct common_transport_tag *transport 
	               , _64 process_identifer );
	PTRSZVAL psvConnect;
} COMMON;

typedef struct queue_msg_tag 
{
	_32 size;
	_8 data[1];
} QUEUE_MSG, *PQUEUE_MSG;

typedef struct transport_queue_tag
{
#ifdef HAVE_ANONYMOUS_STRUCTURES
	COMMON;
#else
#endif
	_32 head;
	_32 tail;
	_8 data[1]; // data for the queue goes here.
} TRANSPORT_QUEUE, *PTRANSPORT_QUEUE; 


MEM_PROC( PTRANSPORT_QUEUE, CreateQueue )( char *name, _32 *size )
{
	PTRANSPORT_QUEUE queue = OpenSpace( NULL
										, name
										, sizeof( TRANSPORT_QUEUE ) + size );
	MemSet( queue, 0, size );
	queue->size = size;
	queue->
	return queue;
}

MEM_PROC( int, EnqueMessage )( PTRANSPORT_QUEUE queue, POINTER msg, _32 size )
{
	if( queue )
	{
		_64 Process = ( (_64)GetProcessID() << 32 ) | (_64)GetThreadID();

	}
	return 0;
}

MEM_PROC( POINTER, DequeMessage )( PTRANSPORT_QUEUE queue )
{
	if( queue )
	{
		_64 Process = ( (_64)GetProcessID() << 32 ) | (_64)GetThreadID();

	}
	return 0;
}

MEM_PROC( POINTER, PequeMessage )( PTRANSPORT_QUEUE queue )
{
	if( queue )
	{
		_64 Process = ( (_64)GetProcessID() << 32 ) | (_64)GetThreadID();

	}
	return 0;
}
// $Log: $
