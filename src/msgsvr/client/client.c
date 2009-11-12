//#define ENABLE_GENERAL_USEFUL_DEBUGGING
#ifdef ENABLE_GENERAL_USEFUL_DEBUGGING
//#define DEBUG_THREAD
#define LOG_LOCAL_EVENT
//#define DEBUG_RU_ALIVE_CHECK
//#define DEBUG_HANDLER_LOCATE
// event messages need to be enabled to log event message data...
#define DEBUG_DATA_XFER
//#define NO_LOGGING
/// show event messages...
#define DEBUG_EVENTS
#define DEBUG_OUTEVENTS
/// attempt to show the friendly name for messages handled
#define LOG_HANDLED_MESSAGES
//#define DEBUG_MESSAGE_BASE_ID
#define _DEBUG_RECEIVE_DISPATCH_
//#define DEBUG_THREADS
//#define DEBUG_MSGQ_OPEN
#endif

#ifdef __LINUX__
#include <unistd.h>
#include <time.h>
// ipc sysv msgque (msgget,msgsnd,msgrcv)
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#define MSGTYPE (struct msgbuf*)
#else
#define MSGTYPE
#endif
#include <msgclient.h>
#include <msgserver.h>
#include <procreg.h>
#include <stdhdrs.h>
#include <sharemem.h>
#include <logging.h>
#include <timers.h>
#include <idle.h>

MSGCLIENT_NAMESPACE

#define MSG_DEFAULT_RESULT_BUFFER_MAX (sizeof( _32 ) * 2048)

#ifdef  _DEBUG
#define DEFAULT_TIMEOUT 300000 // standard transaction timout
#else
#define DEFAULT_TIMEOUT 500 // standard transaction timout
#endif


#ifdef _WIN32
#define MSGQ_TYPE PMSGHANDLE
#else
#define MSGQ_TYPE int
#endif

typedef PREFIX_PACKED struct message_header_tag
{
// internal message information
// transportable across messsage queues or networks

	_32 msgid;
	_32 sourceid;
// #define MSGDATA( msghdr ) ( (&msghdr.sourceid)+1 )
} PACKED MSGHDR;

typedef PREFIX_PACKED struct full_message_header_tag
{
	MSGIDTYPE targetid; // target process ID
	MSGHDR hdr;
#define QMSGDATA( qmsghdr ) ((_32*)( (&qmsghdr.hdr)+1 ))
	//_32 data[];
} PACKED QMSG;

// these are client structures
// services which are loaded result in the creation
// of a handler device.
typedef struct ServiceEventHandler_tag EVENTHANDLER, *PEVENTHANDLER;
struct ServiceEventHandler_tag
{
	DeclareLink( EVENTHANDLER );
	struct {
		_32 dispatched : 1;
		_32 notify_if_dispatched : 1;
		_32 destroyed : 1;
		_32 local_service : 1;
		_32 waiting_for_responce : 1;
		_32 responce_received : 1;
		_32 bCheckedResponce : 1;
	} flags;
	// when receiving messages from the application...
	// this is the event base which it will give me...
	// it is also the unique routing ID
	// my list will be built from the sum of all services
	// connected to, and the number of MsgCount they claim to have..
	_32 MyMsgBase;
	_32 MsgBase;
	THREAD_ID EventID;
	// destination address of this service's
	// messages...
	_32 ServiceID;
	_32 MsgCountEvents;
	_32 MsgCount; // number of messages service serves...
	EventHandlerFunction Handler;
	EventHandlerFunctionEx HandlerEx;
	EventHandlerFunctionExx HandlerExx;
   PTRSZVAL psv;
	// this handler's events come from this queue.
	MSGQ_TYPE msgq_events;
	CRITICALSECTION csMsgTransact;
	// timeout on this local handler's reception of a responce.
	_32 wait_for_responce;
	_32 LastMsgID; // last outbound resquest - which is waiting for a responce
	_32 bias;  // my_message_base + his_message_base
	_32 *MessageID;
	POINTER msg;
	_32 *len;
	_32 last_check_tick;
	TEXTCHAR servicename[1];
};

// registration of a service results in a new
// CLIENT_SERVICE structure.  Messages are received
// from clients, and dispatched.. ..
// Just like clients, services also support events...
typedef struct service_tag
{
	struct {
		BIT_FIELD bRegistered : 1;
		BIT_FIELD bFailed : 1;
		BIT_FIELD connected : 1;
		BIT_FIELD bMasterServer : 1;
		BIT_FIELD bWaitingInReceive : 1;
		BIT_FIELD bClosed : 1;
	} flags;
	GetServiceFunctionTable GetFunctionTable;
	server_function_table functions;
	server_message_handler handler;
	server_message_handler_ex handler_ex;
   PTRSZVAL psv;
	_32 first_message_index;
	_32 entries;
	_32 references;
	TEXTCHAR *name;
	PTHREAD thread;
	//_32 pid_me; // these pids should be phased out, and given an ID from msgsvr.

	PQMSG recv;
	PQMSG result;
	DeclareLink( struct service_tag );
} CLIENT_SERVICE, *PCLIENT_SERVICE;


// PEVENTHANDLER creation also results in the creation of one of these...
// this allows the ProbeClientAlive to work correctly...
// then on the server side, when the service is loaded, one of these is
// created... allowing the server to monitor client connectivity.
typedef struct service_client_tag
{
	_32 pid; // process only? no thread?
	_32 last_time_received;
	struct {
		BIT_FIELD valid : 1;
		BIT_FIELD error : 1; // uhmm - something like got a message from this but it wasn't known
		BIT_FIELD status_queried : 1;
		BIT_FIELD is_service : 1; // created to track connection from client to service...
	} flags;
	PLIST services;
	PEVENTHANDLER handler;
	DeclareLink( struct service_client_tag );
} SERVICE_CLIENT, *PSERVICE_CLIENT;
#ifdef _WIN32
#define pid_t _32
#endif


#define MAXBUFFER_LENGTH_PAIRSPERSET 256
/* use a small local pool of these instead of allocate/release */
/* this high-reuse area just causes fragmentation */
DeclareSet( BUFFER_LENGTH_PAIR );

typedef struct global_tag
{
	struct {
		BIT_FIELD message_responce_handler_ready : 1;
		BIT_FIELD message_handler_ready : 1;
		BIT_FIELD failed : 1;
		BIT_FIELD events_ready : 1;
		BIT_FIELD local_events_ready : 1;
		BIT_FIELD disconnected : 1;
		//BIT_FIELD connected : 1;
		BIT_FIELD handling_client_message : 1;
		BIT_FIELD bAliveThreadStarted : 1;
		// enabled when my_message_id is resolved...
		BIT_FIELD connected : 1;
		BIT_FIELD bMasterServer : 1;
		BIT_FIELD bServiceHandlerStarted : 1; // thread for service handling...
		BIT_FIELD found_server : 1; // set this when a valid server was found - dont' reset master id 
	} flags;
	// incrementing counter for loaded services...
	_32 nMsgCounter;
	_32 my_message_id;
	//_32 pid_me_events;
	//_32 master_server_pid; // this server is addressable at pid 1.
	PLIST pSleepers;
	PTHREAD pThread;			// handles messages on msgq_in (responce from service)
	PTHREAD pMessageThread;  // handles messages on msgq_out (request for service)
	PTHREAD pEventThread;	 // handles messages to msgq_event (client side events)
	PTHREAD pLocalEventThread; // handles messages on msgq_local_event (client only internal events)
	PEVENTHANDLER pHandlers;
	PCLIENT_SERVICE services;
	PSERVICE_CLIENT clients;
	_32 wait_for_message;
	//_32 wait_for_responce;
	_32 responce_received;
	EVENTHANDLER _handler; // a static handler to cover communication with this library itself.
	// this is the one waiting for a register service ack...
	// I won't have the ServiceID
	CRITICALSECTION csLoading;
	PEVENTHANDLER pLoadingService;
	CRITICALSECTION csMsgTransact;

	MSGQ_TYPE msgq_in;	// id + 1 (thread 0)
	MSGQ_TYPE msgq_out;  // id + 0 (thread 0)
	MSGQ_TYPE msgq_event;// id + 2 (thread 1)
	MSGQ_TYPE msgq_local;// id + 3 (thread 2 (or 1 if only local)
#if defined( _WIN32 ) || defined( USE_SACK_MSGQ )
#define IPC_NOWAIT MSGQUE_NOWAIT

#ifndef ENOMSG
#define ENOMSG MSGQUE_ERROR_NOMSG
#endif

#ifndef EINTR
#define EINTR -1
#endif

#ifndef EIDRM
#define EIDRM -2
#endif

#ifndef EINVAL
#define EINVAL -3
#endif

#ifndef E2BIG
#define E2BIG -4
#endif

#ifndef EABORT
#define EABORT MSGQUE_ERROR_EABORT
#endif

#ifdef DEBUG_DATA_XFER
#  define msgsnd( q,msg,len,opt) ( _lprintf(DBG_RELAY)( WIDE("Send Message...") ), LogBinary( (P_8)msg, (len)+4  ), EnqueMsg( q,(msg),(len),(opt)) )
#else
#  define msgsnd EnqueMsg
#endif
	#define msgrcv(q,m,sz,id,o) DequeMsg((PMSGHANDLE)q,&id,m,sz,o)
	#define MSGQSIZE 32768
	#define IPC_CREAT  1
	#define IPC_EXCL	2
#define msgget(name,n,opts) ( (opts) & IPC_CREAT )		\
	? ( ( (opts) & IPC_EXCL)									  \
	  ? ( OpenMsgQueue( name, NULL, 0 )						\
		 ? MSGFAIL													 \
		 : CreateMsgQueue( name, MSGQSIZE, NULL, 0 ))	  \
	  : CreateMsgQueue( name, MSGQSIZE, NULL, 0 ))		 \
	: OpenMsgQueue( name, NULL, 0 )
	//#define msgget(name,n,opts ) CreateMsgQueue( name, MSGQSIZE, NULL, 0 )
	#define msgctl(n,o,f) 
	#define MSGFAIL NULL
#else
#ifdef DEBUG_DATA_XFER
	//#define msgsnd( q,msg,len,opt) ( _xlprintf(1 DBG_RELAY)( WIDE("Send Message...") ), LogBinary( (POINTER)msg, (len)+4  ), msgsnd( q,(msg),(len),(opt)) )
	#define msgsnd( q,msg,len,opt) ( lprintf( WIDE("Send Message...") ), LogBinary( (POINTER)msg, (len)+4  ), msgsnd( q,(msg),(len),(opt)) )
#endif
	#define msgget( name,n,opts) msgget( n,opts )
	#define MSGFAIL -1
#endif
	PREFIX_PACKED struct {
		MSGIDTYPE msgid;
		_32 data[2048];
	} PACKED MessageIn; // 8192 bytes
	S_32 MessageLen;

	PREFIX_PACKED struct {
		MSGIDTYPE msgid;
		_32 data[2048]; // 8192 bytes
	} PACKED MessageOut;
	BUFFER_LENGTH_PAIRSET buffer_length_set;
	int pending;
} GLOBAL;

#ifdef __STATIC__
#define g (*global_msgclient)
static GLOBAL *global_msgclient;

#ifdef __WATCOM_CPLUSPLUS__
#pragma initialize 65
#endif
PRIORITY_PRELOAD( LoadMsgClientGlobal, MESSAGE_CLIENT_PRELOAD_PRIORITY )
{
	SimpleRegisterAndCreateGlobal( global_msgclient );
}
#else
#define g (global_msgclient)
static GLOBAL global_msgclient;
#endif


//--------------------------------------------------------------------

int SendInMultiMessageEx( _32 routeID, _32 MsgID, _32 parts, BUFFER_LENGTH_PAIR *pairs DBG_PASS)
{
	CPOINTER msg;
	_32 len, ofs, param;
	// shouldn't use MessageOut probably.. ..
	// protect msgout against multiple people.. ..
	EnterCriticalSec( &g.csMsgTransact );
	g.MessageOut.msgid = routeID;
	g.MessageOut.data[0] = MsgID;
	g.MessageOut.data[1] = g.my_message_id;
	ofs = sizeof( QMSG );
	for( param = 0; param < parts; param++ )
	{
		msg = pairs[param].buffer;
		len = pairs[param].len;
		if( len + ofs > 8192 )
		{
		// wow - this is a BIG message - lets see - what can we do?
			errno = E2BIG;
			lprintf( WIDE("Length of message is too big to transport...%") _32f WIDE(" (len %") _32f WIDE(" ofs %") _32f WIDE(")"), len + ofs, len, ofs );
			LeaveCriticalSec( &g.csMsgTransact );
			return FALSE;
		}
		if( msg && len )
		{
			//Log3( WIDE("Adding %d bytes at %d: %08x "), len, ofs, ((P_32)msg)[0] );
			MemCpy( ((char*)&g.MessageOut) + ofs, msg, len );
			ofs += len;
		}
	}
	{
		int stat;
	// send to application inbound queue..
#ifdef DEBUG_OUTEVENTS
		lprintf( WIDE("Sending result to application...") );
		LogBinary( (POINTER)&g.MessageOut, ofs );

#endif
		stat = msgsnd( g.msgq_in, MSGTYPE &g.MessageOut, ofs - sizeof( MSGIDTYPE ), 0 );
		LeaveCriticalSec( &g.csMsgTransact );
		return !stat;
	}
}
int SendInMultiMessage( _32 routeID, _32 MsgID, _32 parts, BUFFER_LENGTH_PAIR *pairs )
#define SendInMultiMessage(r,m,parts,pairs) SendInMultiMessageEx(r,m,parts,pairs DBG_SRC )
{
	return SendInMultiMessage( routeID, MsgID, parts, pairs);
}

//--------------------------------------------------------------------

int SendInMessage( _32 routeID, _32 MsgID, POINTER buffer, _32 len )
{
	BUFFER_LENGTH_PAIR pair;
	pair.buffer = buffer;
	pair.len = len;
	return SendInMultiMessage( routeID, MsgID, 1, (BUFFER_LENGTH_PAIR*)&pair );
}

//--------------------------------------------------------------------

#ifdef _DEBUG_RECEIVE_DISPATCH_
int metamsgrcv( MSGQ_TYPE q, POINTER p, int len, _32 id, int opt DBG_PASS )
{
	int stat;
	_xlprintf(1 DBG_RELAY)( WIDE("*** Read Message %d"), id);
	stat = msgrcv( q,MSGTYPE p,len,id,opt );
#undef msgrcv
	#define msgrcv(q,p,l,i,o) metamsgrcv(q,p,l,i,o DBG_SRC)
	_xlprintf(1 DBG_RELAY)( WIDE("*** Got message %d"), stat );
	return stat;
}
#endif

static int PrivateSendDirectedServerMultiMessageEx( _32 DestID
																, _32 MessageID, _32 buffers
																, BUFFER_LENGTH_PAIR *pairs
																 DBG_PASS )
#define PrivateSendDirectedServerMultiMessage(d,m,bu,p) PrivateSendDirectedServerMultiMessageEx(d,m,bu,p DBG_SRC )
{
	CPOINTER msg;
	_32 len, ofs, param;
	if( g.flags.disconnected )
	{
		lprintf( WIDE("Have already disconnected from server... no further communication possible.") );
		return TRUE;
	}
	g.MessageOut.msgid = DestID;
	g.MessageOut.data[0] = MessageID;
	g.MessageOut.data[1] = g.my_message_id;
	// don't know len at this point.. .. ..
	//if( len > 8188 )
	//{
		// wow - this is a BIG message - lets see - what can we do?
		//errno = E2BIG;
		//lprintf( WIDE("Lenght of message is too big to transport...") );
		//return FALSE;
	//}
	ofs = sizeof( QMSG );
	//Log1( WIDE("Adding %d params"), buffers );
	for( param = 0; param < buffers; param++ )
	{
		msg = pairs[param].buffer;
		len = pairs[param].len;
		if( len + ofs > 8192 )
		{
		// wow - this is a BIG message - lets see - what can we do?
			errno = E2BIG;
			lprintf( WIDE("Length of message is too big to transport...%") _32f WIDE(" (len %") _32f WIDE(" ofs %") _32f WIDE(")"), len + ofs, len, ofs );
			return FALSE;
		}
		if( msg && len )
		{
			//Log3( WIDE("Adding %d bytes at %d: %08x "), len, ofs, ((P_32)msg)[0] );
			MemCpy( ((char*)&g.MessageOut) + ofs, msg, len );
			ofs += len;
		}
	}
	// subtract 4 from the offset (the msg_id is not counted)
	//Log2( WIDE("Sent %d  (%d) bytes"), g.MessageOut[0], ofs - sizeof( MSGIDTYPE ) );
	// 0 success, non zero failure - return notted state
				  //lprintf( WIDE("Send Message. %08lX"), *(_32*)g.MessageOut );
	//_xlprintf( 1 DBG_RELAY )( "blah." );
	return !msgsnd( g.msgq_out, MSGTYPE &g.MessageOut, ofs - sizeof( MSGIDTYPE ), 0 );

}

static PEVENTHANDLER PrivateSendServerMultiMessageEx( _32 *MessageID, _32 *bias, _32 buffers
																  , BUFFER_LENGTH_PAIR *pairs DBG_PASS )
#define PrivateSendServerMultiMessage(mi,bi,bu,pairs) PrivateSendServerMultiMessageEx(mi,bi,bu,pairs DBG_SRC)
{
	PEVENTHANDLER handler;
	_32 MsgID = (*MessageID);
	_32 RouteID;
	// these messages are KNOWN, and VALIDATED
	// before sending to the server.
	// a LOW message ID may ALSO be the result
	// of APPLICATION PROGRAMMER ERROR
	// these message IDs should only be generated and used
	// via methods of the MsgClient library (this code)
	if( ( MsgID == CLIENT_CONNECT )
		|| ( MsgID == CLIENT_REGISTER_SERVICE )
		|| ( MsgID == MSG_ServiceLoad )
		|| ( MsgID == CLIENT_REGISTER_SERVICE )
		|| ( MsgID == CLIENT_LIST_SERVICES ) )
	{
		// special case message .v...
		// this is sent to process '1'
		// for which there will be one and only one
		// service listing, and it will be the
		// master-coordinator service.
		if( bias ) (*bias) = 0;
		RouteID = 1;
	}
	{
		//lprintf( "Message ID %08lx", MsgID );
		if( MsgID >= LOWEST_BASE_MESSAGE )
		{
			for( handler = g.pHandlers; handler; handler = handler->next )
			{
				//lprintf( WIDE("Is %d within %ld to %ld?? "), MsgID, handler->MyMsgBase, handler->MyMsgBase + handler->MsgCount );
				if( handler->MyMsgBase <= MsgID &&
					( handler->MyMsgBase + handler->MsgCount )  > MsgID )
				{
					// rebias message to server's view of the world.
					//lprintf( WIDE("rebias message %ld by %ld into %ld"),
					//		  MsgID
					//		 , handler->MyMsgBase
					//		  , handler->MsgBase );
					//_xlprintf(1 DBG_RELAY)( WIDE("Entering section, indicating a wait for message...%p"), handler );
					if( handler->flags.waiting_for_responce )
					{
						lprintf( WIDE("Already waiting... why are we requing?") );
						//DebugBreak();
					}
					EnterCriticalSec( &handler->csMsgTransact );
#ifdef DEBUG_DATA_XFER
					lprintf( WIDE("Set wait...") );
#endif
					handler->flags.waiting_for_responce = 1;
					handler->LastMsgID = MsgID;
					if( MsgID == RU_ALIVE ) // one fo the few exceptions for mated responce
						handler->LastMsgID = IM_ALIVE;

					MsgID -= handler->MyMsgBase;
					MsgID += handler->MsgBase;
					if( bias )
						(*bias) = handler->MyMsgBase - handler->MsgBase;
					//handler->bias = handler->MyMsgBase - handler->MsgBase;
					//lprintf( WIDE("bias = %ld"), handler->bias );
					//lprintf( WIDE("Resulting handler is some open connection") );
					break;
				}
			}
			if( !handler )
			{
            DebugBreak();
				Log( WIDE("Client attempting to send an invalid message ID") );
				errno = EINVAL;
				return NULL;
			}
			else
			{
				RouteID = handler->ServiceID;
			}
		}
		else
		{
			if( !RouteID )
			{
				lprintf( WIDE("Route ID will be %") _32f WIDE(""), RouteID );
				lprintf( WIDE("This value is probably 0 at this point... and means that the message has been dropped.") );
				return NULL;
			}
			else
			{
				if( g.pLoadingService && ( MsgID == MSG_ServiceLoad ) )
					handler = g.pLoadingService;
				else
				{
#ifdef DEBUG_HANDLER_LOCATE
					lprintf( WIDE("Resulting handler is the global core message handler") );
#endif
					handler = &g._handler;
				}
#ifdef DEBUG_HANDLER_LOCATE
				lprintf( WIDE("set handler's bias to 0?") );
#endif
				handler->bias = 0;
				handler->LastMsgID = MsgID;
			}
 		}
	}
	(*MessageID) = MsgID;
	if( PrivateSendDirectedServerMultiMessageEx( RouteID
														  , MsgID
														  , buffers, pairs DBG_RELAY ) )
	{
		return handler;
	}
	return NULL;
}
//--------------------------------------------------------------------

CLIENTMSG_PROC(int, SendMultiServiceEventPairsEx)( _32 pid, _32 event
															  , _32 parts
															  , BUFFER_LENGTH_PAIR *pairs
															  DBG_PASS
															  )
#define SendMultiServiceEventPairs(p,ev,par,pair) SendMultiServiceEventPairsEx(p,ev,par,pair DBG_SRC)
{
	static struct { QMSG msg;
		_32 data[2048-sizeof(QMSG)];
	}msg;
	static CRITICALSECTION cs;
	P_8 msgbuf;
	_32 sendlen = 0;
#ifdef DEBUG_MESSAGE_BASE_ID
	//DBG_VARSRC;
#endif
	EnterCriticalSec( &cs );
	if( event >= LOWEST_BASE_MESSAGE )
	{
		msg.msg.hdr.msgid = event;
	}
	else
	{
	// internal events cannot be PID based?
#ifdef DEBUG_MESSAGE_BASE_ID
		lprintf( WIDE("Received a message ID %d < %d, and a PID of %d also?")
				 , event, LOWEST_BASE_MESSAGE, pid );
#endif
		if( pid )
		{
			// this means that the msgsvr is sending a message back
			// to the client API layer, and is not really intended for the
			// application.  HOWEVER, this may also be the range of
			// application error, so we should tightly check valid messages...
			if( ( event != MSG_SERVICE_DATA )
				&& ( event != MSG_SERVICE_NOMORE ) )
			{
				lprintf( WIDE("Received a message ID %") _32f WIDE(" < %d, and a PID of %") _32f WIDE(" also?")
						 , event, LOWEST_BASE_MESSAGE, pid );
				DebugBreak();
			}
		}
		msg.msg.hdr.msgid = event;
#ifdef DEBUG_MESSAGE_BASE_ID
		lprintf( WIDE("event is lower than lowest_base_message...") );
#endif
	}
	msg.msg.targetid = pid;
	msg.msg.hdr.sourceid = g.my_message_id;
	msgbuf = (P_8)msg.data;
	while( parts )
	{
		if( pairs->buffer && pairs->len )
		{
			MemCpy( msgbuf + sendlen, pairs->buffer, pairs->len );
			sendlen += pairs->len;
			pairs++;
		}
		parts--;
	}
						// outgoing que for this handler.
#if defined( DEBUG_EVENTS ) 
	_lprintf(DBG_RELAY)( WIDE("Send Event...") );
#if defined( DEBUG_DATA_XFER )
	LogBinary( &msg, sendlen + sizeof( MSGHDR ) + sizeof( MSGIDTYPE ) );
#endif
#endif
	if( !msg.msg.targetid )
		msg.msg.targetid = g.my_message_id;
	{
		int status;
		status = msgsnd( pid?g.msgq_event:g.msgq_local, MSGTYPE &msg
							, sendlen + sizeof( MSGHDR ), 0 );
		LeaveCriticalSec( &cs );
		return !status;
	}

}

#ifdef _DEBUG
static struct {
	CTEXTSTR pFile;
	int nLine;
}nextsmmse;
#endif
#undef SendMultiServiceEvent
CLIENTMSG_PROC(int, SendMultiServiceEvent)( _32 pid, _32 event
								 , _32 parts
								 , ... )
{
	int status;
	BUFFER_LENGTH_PAIR *pairs = NewArray( BUFFER_LENGTH_PAIR, parts );
	_32 n;
	va_list args;
	va_start( args, parts );
	for( n = 0; n < parts; n++ )
	{
		pairs[n].buffer = va_arg( args, POINTER );
		pairs[n].len = va_arg( args, _32 );
	}
#ifdef _DEBUG
	status = SendMultiServiceEventPairsEx( pid, event, parts, pairs, nextsmmse.pFile, nextsmmse.nLine );
#else
	status = SendMultiServiceEventPairsEx( pid, event, parts, pairs );
#endif
	Release( pairs );
	return status;
}
CLIENTMSG_PROC(SendMultiServiceEventProto, SendMultiServiceEventEx)( DBG_VOIDPASS )
{
#ifdef _DEBUG
	nextsmmse.pFile = pFile;
	nextsmmse.nLine = nLine;
#endif
	return SendMultiServiceEvent;
}

CLIENTMSG_PROC( int, SendServerMultiMessage )( _32 MessageID, _32 buffers, ... )
{
	BUFFER_LENGTH_PAIR *pairs = (BUFFER_LENGTH_PAIR*)Allocate( sizeof( BUFFER_LENGTH_PAIR ) * buffers );
	_32 n;
	va_list args;
	PEVENTHANDLER handler;
	va_start( args, buffers );
	for( n = 0; n < buffers; n++ )
	{
		pairs[n].buffer = va_arg( args, POINTER );
		pairs[n].len = va_arg( args, _32 );
	}
	{
		handler = PrivateSendServerMultiMessage( &MessageID, NULL, buffers, pairs );
		if( handler )
		{
			handler->flags.waiting_for_responce = 0;
			LeaveCriticalSec( &handler->csMsgTransact );
		}
	}
	Release( pairs );
	return ( handler != NULL );
}

CLIENTMSG_PROC( int, SendRoutedServerMultiMessage )( _32 RouteID, _32 MessageID, _32 buffers, ... )
{
	int status;
	BUFFER_LENGTH_PAIR *pairs = (BUFFER_LENGTH_PAIR*)Allocate( sizeof( BUFFER_LENGTH_PAIR ) * buffers );
	_32 n;
	va_list args;
	va_start( args, buffers );
	for( n = 0; n < buffers; n++ )
	{
		pairs[n].buffer = va_arg( args, POINTER );
		pairs[n].len = va_arg( args, _32 );
	}
	status = PrivateSendDirectedServerMultiMessage( RouteID, MessageID, buffers, pairs );
	Release( pairs );
	return status;
}

CLIENTMSG_PROC( int, SendRoutedServerMessage )( _32 RouteID, _32 MessageID, POINTER buffer, _32 len )
{
	int status;
	BUFFER_LENGTH_PAIR pair;
	pair.buffer = buffer;
	pair.len = len;
	status = PrivateSendDirectedServerMultiMessage( RouteID, MessageID, 1, &pair );
	return status;
}

CLIENTMSG_PROC( int, SendServerMessage )( _32 MessageID, POINTER msg, _32 len )
{
	BUFFER_LENGTH_PAIR pair;
	PEVENTHANDLER handler;
	pair.buffer = msg;
	pair.len = len;
	handler = PrivateSendServerMultiMessage( &MessageID, NULL, 1, &pair );
	if( handler )
	{
		handler->flags.waiting_for_responce = 0;
		LeaveCriticalSec( &handler->csMsgTransact );
	}
	return ( handler != NULL );
}

//--------------------------------------------------------------------

static PSERVICE_CLIENT FindClient( _32 pid )
{
	PSERVICE_CLIENT client = g.clients;
	while( client )
	{
		if( client->pid == pid )
		{
			break;
		}
		client = client->next;
	}
	return client;
}

//--------------------------------------------------------------------

static int GetAMessageEx( MSGQ_TYPE msgq, CTEXTSTR q, int flags DBG_PASS )
#define GetAMessage(m,f) GetAMessageEx(m,#m,f DBG_SRC)
{
	//int bLog = 0;
	if( IsThisThread( g.pThread ) )
	{
		int logged = 0;
		_32 get_tired_of_waiting_at = GetTickCount() + 200;
		while( g.responce_received )
		{
			Relinquish(); // someone hasn't gotten their responce in yet...
			if( get_tired_of_waiting_at < GetTickCount() )
			{
				lprintf( "Dropping a responce on the ground. %s:%08" _32fx, q, g.MessageIn.msgid );
				g.responce_received = 0;
				
				//DebugBreak();
				//break;
			}
			if( !logged )
			{
				//lprintf( WIDE("Someone's message hasn't been read yet...") );
				logged = 1;
			}
		}
		//if( logged )
		//	lprintf( WIDE("They finally got their message...") );
		do
		{
			//if( bLog )
			//lprintf( WIDE("Attempt to recieve for g.pid_me:%08lx %ld"), g.my_message_id, msgq );
			//lprintf( "vvv" );
			g.MessageLen = msgrcv( msgq, MSGTYPE &g.MessageIn, 8192, g.my_message_id, flags );
			//lprintf( "^^^" );
			//lprintf( WIDE("Got a receive...") );
			if( ( g.MessageLen == (-((S_32)sizeof(MSGIDTYPE))) ) ) // retry
			{
				lprintf( WIDE("Recieved -4 message (no data?!) no message, should have been -1, ENOMSG") );
				g.MessageIn.data[0] = RU_ALIVE;
				continue; // continue on do-while checks while condition - gofigure.
			}
			if( g.MessageLen == -1 )
			{
				if( errno == ENOMSG )
				{
					lprintf( WIDE("No message... nowait was set?") );
					return 0;
				}
				else if( errno == EIDRM )
				{
					lprintf( WIDE("No message... Message queue removed") );
					return -1;
				}
				else
				{
					if( errno == EINTR ){
						//bLog = 1;
						//lprintf( WIDE("Error Interrupt - that's okay...") );
					}
					else if( errno == EINVAL )
					{
						lprintf( WIDE("msgrecv on q %d is invalid! open it. or what is %") _32f WIDE("(%08") _32fx WIDE(") or %08d")
								 , msgq, g.my_message_id, g.my_message_id, flags );
					}
					else
					{
						xlprintf( LOG_ALWAYS )( WIDE("msgrcv resulted in error: %d"), errno );
					}
					g.MessageIn.data[0] = RU_ALIVE; // loop back around.
				}
			}
			else
			{
#ifdef DEBUG_DATA_XFER
				lprintf( WIDE("Received Mesage.... "));
				LogBinary( (P_8)&g.MessageIn, g.MessageLen + sizeof( MSGIDTYPE ) );
#endif
				//Log2( WIDE("Read message to %d (%08x)"), g.pid_me, g.MessageIn.data[0] );
				if( g.MessageIn.data[0] == IM_TARDY )
				{
					PEVENTHANDLER handler;
					lprintf( WIDE("Server wants to extend timout to %") _32f WIDE(""), g.MessageIn.data[2] );
					for( handler = g.pHandlers; handler; handler = handler->next )
						if( handler->ServiceID == g.MessageIn.data[1] )
						{
							handler->wait_for_responce = g.MessageIn.data[2] + GetTickCount();
							break;
						}
					if( !handler )
					{
						lprintf( WIDE("A service announced it was going to be tardy to someone who was not talking to it!") );
						DebugBreak();
					}
					// okay continue in a do_while will execute the while condition
					// also, much like while(){} will...
					g.MessageIn.data[0] = RU_ALIVE;
					continue;
				}
				else if( g.MessageIn.data[0] == RU_ALIVE )
				{
					QMSG Msg;
#ifdef DEBUG_MESSAGE_BASE_ID
					DBG_VARSRC;
#endif
					//Log( WIDE("Got RU_ALIVE am responding  AM ALIVE!!") );
					Msg.targetid = g.MessageIn.data[1];
					Msg.hdr.msgid = IM_ALIVE;
					Msg.hdr.sourceid = g.my_message_id;
					msgsnd( g.msgq_in, MSGTYPE &Msg, sizeof(MSGHDR), 0 );
				}
				else if( g.MessageIn.data[0] == IM_ALIVE )
				{
					PSERVICE_CLIENT client = FindClient( g.MessageIn.data[1] );
#ifdef DEBUG_RU_ALIVE_CHECK
					lprintf( WIDE("Got message IM_ALIVE from client... %") _32f WIDE(""), g.MessageIn.data[1] );
#endif
					if( client )
					{
						//lprintf( WIDE("Updating client %p with current time...allowing him to requery.."), client );
						if( client->flags.status_queried )
						{
							client->flags.status_queried = 0;
							// fake a get-next-message... if status_queried is NOT set
							// then, this is a request of ProbeClientAlive.
							g.MessageIn.data[0] = RU_ALIVE;
						}
						client->last_time_received = GetTickCount();
					}
					// go back to top and get another message...
					// application can care about this...
					// maybe should check...
					//g.MessageIn.data[0] = RU_ALIVE;
				}
			}
		}
		while( !g.flags.disconnected && g.MessageIn.data[0] == RU_ALIVE );
																		//lprintf( WIDE("Responce received...%08lX"), g.MessageIn.data[0] );
		if( !g.flags.disconnected )
		{
			int cnt = 0;
			INDEX idx;
			PTHREAD thread;
			g.responce_received = 1; // set to enable application to get it's message...
									  //lprintf( WIDE("waking sleepers.") );
			LIST_FORALL( g.pSleepers, idx, PTHREAD, thread )
			{
				cnt++;
			//lprintf( WIDE("Wake thread waiting for responces...%016Lx"), thread->ThreadID );
				WakeThread( thread );
			}
			if( !cnt )
			{
			//lprintf( WIDE("FATALITY - received responce from service, and noone was waiting for it!") );
			//lprintf( WIDE("No Sleepers woken - maybe - they haven't gotten around to sleeping yet?") );
			}
		}
	}
	else
	{
		//lprintf( WIDE("Not the message thread... exiting quietly... %d %p %d %Ld")
		//	, g.my_message_id
		//	 , g.pThread
		//	 , getpid()
		//		, g.pThread->ThreadID
		//	);
		return 2;
	}
	return 1;
}

typedef struct queue_read_state
{
	MSGQ_TYPE msgq;
	struct msgbuf *buffer;
	// on result size is size of message.
	_32 size; // 8192?
	_32 read_message;
	_32 flags;
} QUEUE_STATE, *PQUEUE_STATE;
// g.msgq_event
// g.buffer

//--------------------------------------------------------------------

static int HandleEvents( MSGQ_TYPE msgq, PQMSG MessageEvent, int initial_flags )
{
	int receive_flags = initial_flags;
	int receive_count = 0;
	while( 1 )
	{
		S_32 MessageLen;
#ifdef DEBUG_EVENTS
		lprintf( WIDE("Reading eventqueue... my_message_id = %d"), g.my_message_id );
#endif
			//lprintf( "vvv" );
		MessageLen = msgrcv( msgq
								 , MSGTYPE MessageEvent, 8192
								 , g.my_message_id
								 , receive_flags );
		//lprintf( "^^^" );
		if( (MessageLen+ sizeof( MSGIDTYPE )) == 0 )
		{
			lprintf( WIDE("Recieved -4 message (no data?!) no message, should have been -1, ENOMSG") );
		}
		else if( MessageLen == -1 )
		{
			//Log( WIDE("Failed a message...") );
			if( errno == ENOMSG )
			{
				//Log( WIDE("No message...") );
				if( receive_count )
				{
					PEVENTHANDLER pLastHandler;
					PEVENTHANDLER pHandler = g.pHandlers;
#ifdef DEBUG_EVENTS
					lprintf( WIDE("Dispatch dispatch_pending..") );
#endif
					while( pHandler )
					{
						pHandler->flags.dispatched = 1;
						if( pHandler->flags.notify_if_dispatched )
						{
							//lprintf( WIDE("Okay one had something pending...") );
							if( pHandler->Handler )
								pHandler->Handler( MSG_EventDispatchPending, NULL, 0 );
							else if( pHandler->HandlerEx )
								pHandler->HandlerEx( 0, MSG_EventDispatchPending, NULL, 0 );
							else if( pHandler->HandlerExx )
								pHandler->HandlerExx( pHandler->psv, 0, MSG_EventDispatchPending, NULL, 0 );
							// okay did that... now clear status.
							pHandler->flags.notify_if_dispatched = 0;
						}
						pLastHandler = pHandler;
						pHandler = pHandler->next;
						pLastHandler->flags.dispatched = 0;
						//lprintf( WIDE("next handler please...") );
					}
				}
				receive_flags = 0; // re-enable pause.
				//lprintf( WIDE("Done reading...") );
				break; // done;
			}
			else if( errno == EIDRM )
			{
				Log( WIDE("Queue was removed.") );
				g.flags.events_ready = 0;
				return -1;
			}
			else
			{
				if( errno != EINTR )
					xlprintf( LOG_ALWAYS )( WIDE("msgrcv resulted in error: %d"), errno );
				//else
				//	Log( WIDE("EINTR received.") );
				break;
			}
		}
		else
		{
			PEVENTHANDLER pHandler = g.pHandlers;
			receive_flags = IPC_NOWAIT;
			receive_count++;
#if defined( DEBUG_EVENTS )
			lprintf( WIDE("Received Event Mesage.... "));
#if defined( DEBUG_DATA_XFER )
			LogBinary( (P_8)MessageEvent, MessageLen + sizeof( MSGHDR ) );
#endif
#endif
			//Log( WIDE("Got an event message...") );
			if( MessageEvent->hdr.msgid < LOWEST_BASE_MESSAGE )
			{
				// core server events...
				// MessageEvent->hdr.sourceid == client_id (client_route_id)
				switch( MessageEvent->hdr.msgid )
				{
				case MSG_SERVICE_DATA:
					{
						PREFIX_PACKED struct msg {
							PLIST *list;
							// so what i there's no reference to this?! OMG!
                     // I still need to have it defined!
							_32 service_id;
							TEXTCHAR newname; // actually is the first element of an array of characters with nul terminator.
						} PACKED *data_msg = (struct msg*)((&MessageEvent->hdr)+1);
						// ahh - someone requested the list of services...
						// client-application API...
						// add to the list... what list?
						PLIST *list = data_msg->list;
						// MessageEvent[3] for this message is client_id
						// which may be used to directly contact the client.
						//lprintf( WIDE("Adding service %ld called %s to list.."), MessageEvent[4], MessageEvent + 5 );
						// SetLink( data_msg->list, data_msg->service_id, data_msg->newname );
						AddLink( list
								 , StrDup( &data_msg->newname ) );
					}
					break;
				case MSG_SERVICE_NOMORE:
					{
						// ahh - someone requested the list of services...
						// client-application API...
						// this is the END of the list
						PREFIX_PACKED struct msg {
							int *bDone;
							PTHREAD pThread;
						} PACKED *data_msg = (struct msg*)((&MessageEvent->hdr)+1);
						//int *bDone = *(int**)((&MessageEvent->hdr)+1);
						//PTHREAD pThread = *(PTHREAD*)((int*)((&MessageEvent->hdr)+1)+1);
						(*data_msg->bDone) = 1;
						WakeThread( data_msg->pThread );
					}
					break;
				}
			}
			else for( ; pHandler; pHandler = pHandler->next )
			{
				_32 Msg;
#ifdef DEBUG_EVENTS
				lprintf( WIDE("Finding handler for %ld (%ld)-%d %p (from %lx to %lx)")
						 , MessageEvent->hdr.msgid
						 , MessageEvent->hdr.msgid - pHandler->MsgBase
						 , pHandler->MsgCountEvents
						 , pHandler->Handler
						 , (_32*)((&MessageEvent->hdr)+1)
						 , pHandler->ServiceID );
#endif
				//if( !pHandler->ServiceID )
				//	pHandler->ServiceID = g.my_message_id;
				if( pHandler->ServiceID != MessageEvent->hdr.sourceid )
				{
					// if it's not from this handler's server... try the next.
					continue;
				}
				Msg = MessageEvent->hdr.msgid - pHandler->MsgBase;
				//lprintf( WIDE("Msg now %d base %d %d"), Msg, pHandler->MsgBase, pHandler->MsgCountEvents );
				if(( pHandler->Handler // have a handler
					 || pHandler->HandlerEx // have a fancier handler....
                || pHandler->HandlerExx ) // or an even fancier handler...
				  && !( Msg & 0x80000000 ) // not negative result (msg IS 32 bits)
				  && ( Msg < pHandler->MsgCountEvents ) ) // in range of handler
				{
					int result_yesno;
#ifdef DEBUG_EVENTS
					lprintf( WIDE("Dispatch event message to handler...") );
					LogBinary( (POINTER)((&MessageEvent->hdr)+1), MessageLen - sizeof(MSGHDR) );
#endif
					pHandler->flags.dispatched = 1;
					if( pHandler->Handler )
					{
						//lprintf( WIDE("small handler") );
						result_yesno = pHandler->Handler( Msg, (_32*)((&MessageEvent->hdr)+1), MessageLen - sizeof( MSGHDR ) );
					}
					else if( pHandler->HandlerEx )
					{
						//lprintf( WIDE("ex handler...%d"), Msg );
						result_yesno = pHandler->HandlerEx( MessageEvent->hdr.sourceid
															 , Msg
															 , (_32*)((&MessageEvent->hdr)+1)
															 , MessageLen - sizeof( MSGHDR ) );
					}
					else if( pHandler->HandlerExx )
					{
						//lprintf( WIDE("ex handler...%d"), Msg );
						result_yesno = pHandler->HandlerExx( pHandler->psv
																	  , MessageEvent->hdr.sourceid
																	  , Msg
																	  , (_32*)((&MessageEvent->hdr)+1)
																	  , MessageLen - sizeof( MSGHDR ) );
					}
					if( result_yesno & EVENT_WAIT_DISPATCH )
					{
						//lprintf( WIDE("Setting status to send dispatch_events...") );
						pHandler->flags.notify_if_dispatched = 1;
					}
					pHandler->flags.dispatched = 0;
					break;
				}
			}
		}
	}
	if( receive_count )
		return 1;
	return 0;
}

//--------------------------------------------------------------------

#ifndef __WINDOWS__
static void ResumeSignal( int signal )
{
	//lprintf( WIDE("Got a resume signal.... resuming uhmm some thread.") );
	//lprintf( WIDE("Uhmm and then pending should be 0?") );
	g.pending = 0;
}
#endif

//--------------------------------------------------------------------

static PTRSZVAL CPROC HandleEventMessages( PTHREAD thread )
{
	g.pEventThread = thread;
	g.flags.events_ready = TRUE;
#ifdef DEBUG_THREADS
	lprintf( WIDE("threadID: %Lx %lx"), GetThreadID( thread ), (unsigned long)(GetThreadID( thread ) & 0xFFFFFFFF) );
#endif
	//g.my_message_id = g.my_message_id; //(_32)( thread->ThreadID & 0xFFFFFFFF );
	while( !g.flags.disconnected )
	{
		int r;
		if( thread == g.pEventThread )
		{
			static _32 MessageEvent[2048]; // 8192 bytes
#ifdef DEBUG_EVENTS
			lprintf( WIDE("Reading event...") );
#endif
			if( ( r = HandleEvents( g.msgq_event, (PQMSG)MessageEvent, 0 ) ) < 0 )
			{
				Log( WIDE("EventHandler has reported a fatal error condition.") );
				break;
			}
#ifdef DEBUG_EVENTS
			lprintf( WIDE("Read event...") );
#endif
		}
		else if( r == 2 )
		{
			Log( WIDE("Thread has been restarted.") );
			// don't clear ready or main event flag
			// things.
			return 0;
		}
	}
	lprintf( WIDE("Done with this event thread - BAD! ") );
	g.flags.events_ready = FALSE;
	g.pEventThread = NULL;
	return 0;
}

//--------------------------------------------------------------------

static PTRSZVAL CPROC HandleLocalEventMessages( PTHREAD thread )
{
	g.pLocalEventThread = thread;
	g.flags.local_events_ready = TRUE;
	//g.my_message_id = getpid(); //(_32)( thread->ThreadID & 0xFFFFFFFF );
	while( !g.flags.disconnected )
	{
		int r;
		if( thread == g.pLocalEventThread )
		{
			// thread local storage :)
			static int levels;
			static _32 *pBuffer;
			static _32 MessageEvent[2048]; // 8192 bytes
			if( !levels )
				pBuffer = MessageEvent;
			else
				pBuffer = (_32*)Allocate( sizeof( MessageEvent ) );
			levels++;
			//lprintf( WIDE("---- GET A LOCAL EVENT!") );
			if( ( r = HandleEvents( g.msgq_local, (PQMSG)pBuffer, 0 ) ) < 0 )
			{
				Log( WIDE("EventHandler has reported a fatal error condition.") );
				break;
			}
			levels--;
			if( levels )
				Release( pBuffer );
		}
		else if( r == 2 )
		{
			Log( WIDE("Thread has been restarted.") );
			// don't clear ready or main event flag
			// things.
			return 0;
		}
	}
	g.flags.local_events_ready = FALSE;
	g.pLocalEventThread = NULL;
	return 0;
}

//--------------------------------------------------------------------

CLIENTMSG_PROC( int, ProcessClientMessages )( PTRSZVAL unused )
{
	static _32 MessageBuffer[2048];
	if( IsThisThread( g.pEventThread ) )
	{
		lprintf( WIDE("External handle event messages...") );
		return HandleEvents( g.msgq_event, (PQMSG)MessageBuffer, 0/*IPC_NOWAIT*/ );
	}
	if( IsThisThread( g.pLocalEventThread ) )
	{
#ifdef LOG_LOCAL_EVENT
		lprintf( WIDE("External handle local event messages...") );
#endif
		// if this is thE thread... chances are someone can wake it up
		// and it is allowed to go to sleep.  This thread is indeed wakable
		// by normal measures.
		return HandleEvents( g.msgq_local, (PQMSG)MessageBuffer, 0/*IPC_NOWAIT*/ );
	}
	return -1;
}

//--------------------------------------------------------------------


static PTRSZVAL CPROC HandleMessages( PTHREAD thread )
{
	g.pThread = thread;
#ifdef DEBUG_THREADS
	lprintf( WIDE("threadID: %lx"), g.my_message_id );
#endif

	g.flags.message_responce_handler_ready = TRUE;
	while( !g.flags.disconnected )
	{
		int r;
		//Log( WIDE("enter read a message...") );
		if( ( r = GetAMessage( g.msgq_in, 0 ) ) < 0 )
		{
			Log( WIDE("thread is exiting...") );
			g.flags.message_responce_handler_ready = FALSE;
			break;
		}
		if( r == 2 )
		{
			Log( WIDE("THIS thread is no longer THE thread!?!?!?!?!?!") );
			break;
		}
	}
	g.pThread = NULL;
	return 0;
}


//--------------------------------------------------------------------

static PSERVICE_CLIENT AddClient( _32 pid )
{
	{
		PSERVICE_CLIENT client = FindClient( pid );
		if( client )
		{
		//Log( WIDE("Client has reconnected?!?!?!") );
		// reconnect is done when requesting a service from
		// a server that supplies one or more services itself...
			// suppose we can just let him continue...
			return client;
		}
	}

	{
		PSERVICE_CLIENT client = (PSERVICE_CLIENT)Allocate( sizeof( SERVICE_CLIENT ) );
		MemSet( client, 0, sizeof( SERVICE_CLIENT ) );
		client->pid = pid;
		client->last_time_received = GetTickCount();
		client->flags.valid = 1;
		LinkThing( g.clients, client );
		g.clients = client;
		//Log( WIDE("Added client...") );
		return client;
	}
}

//--------------------------------------------------------------------

CLIENTMSG_PROC( void, HandleServerMessageEx )( PMSGHDR msg DBG_PASS )
{
	PREFIX_PACKED struct {
		QMSG msg;
		_32 first_index;
		_32 functions;
		_32 events;
		_32 zero1;
		_32 zero2;
		_32 message_source_id;
	} PACKED Responce;
	//_32 Responce[9];
	Responce.msg.targetid = msg->sourceid;
	Responce.msg.hdr.sourceid = g.my_message_id;
	Responce.message_source_id = g.my_message_id; // need this in message content also, cause source app can't get it otherwise.
	switch( msg->msgid )
	{
	case RU_ALIVE:
		{
			QMSG ImAlive;
#ifdef DEBUG_MESSAGE_BASE_ID
			DBG_VARSRC;
#endif
			ImAlive.targetid = msg->sourceid;
			ImAlive.hdr.msgid = IM_ALIVE;
			ImAlive.hdr.sourceid = g.my_message_id;
			msgsnd( g.msgq_in, MSGTYPE &ImAlive, sizeof(MSGHDR), 0 );
		}
		break;
	case MSG_ServiceLoad:
		{
			PCLIENT_SERVICE service = g.services;
			for( service = g.services; service; service = service->next )
			{
				// msg[0] == message ID
				// msg[1] == originating process
				// msg+2 == event name begin
				//lprintf( WIDE("is %s === %s?") , service->name, msg + 2 );
				if( strcmp( service->name, (TEXTCHAR*)((&msg->sourceid)+1) ) == 0 )
				{
					//lprintf( WIDE("Found the service...%s"), service->name );
					break;
				}
			}
			if( service )
			{
				PSERVICE_CLIENT client;
#ifdef DEBUG_MESSAGE_BASE_ID
				DBG_VARSRC;
#endif
				_32 resultbuf[2];
				_32 resultlen = 8;
				int success;
				//lprintf( WIDE("Hmm need to respond to load_service... the fact we got here really indicates that we are loaded....") );
				client = AddClient( msg->sourceid );
				AddLink( &client->services, service );
				Responce.msg.hdr.msgid = MSG_ServiceLoad|SERVER_SUCCESS;
				Responce.first_index = service->first_message_index;
				//lprintf( WIDE("Message event count is set to server message count (%ld) hard coded!"), service->entries );
				Responce.functions = service->entries;
				Responce.events = service->entries; // or something...
				//lprintf( WIDE("Need to result with THREAD_ID of event handler...") );
				Responce.zero1 = 0;
				Responce.zero2 = 0;
				// this is who messages should be sent to...
				// this messages was forwarded by the message server
				// and the client doens't know yet...
				success = FALSE;
				if( service->handler || service->handler_ex )
				{
					// service shall respond with the number of message id's it expexts
					// and the number of event IDs of events that it will generate
					// result[0] = nMyMaxMessageCount;
					// result[1] = nMyMaxEventGeneratedCount;
               if( service->handler )
						success = service->handler( msg->sourceid, MSG_ServiceLoad
														  , ((&msg->sourceid) + 1), 1
														  , resultbuf, &resultlen );
               if( service->handler_ex )
						success = service->handler_ex( service->psv
															  , msg->sourceid, MSG_ServiceLoad
															  , ((&msg->sourceid) + 1), 1
															  , resultbuf, &resultlen );
					//if( resultlen > 0 )
					Responce.functions = service->entries;
					//else
					//{
					//	service->entries =
					//		Responce.functions = 1;
					//}

					if( resultlen > 4 )
						// if 0 is result, fake 256 events
						Responce.events = resultbuf[1]?resultbuf[1]:256;
					else
						Responce.events = service->entries;
					lprintf( WIDE("Message event count is set to server message count (%") _32f WIDE(") event count (%") _32f WIDE(")!")
							 , Responce.functions
							 , Responce.events );
				}
				// didn't have handler or handler returned failure
				// attempt to invoke the serviceload functions
				if( !success )
				{
					lprintf( WIDE("There was a logging message here, but I don't know what msg+2 was supposed to be?") );
					//lprintf( WIDE("Dispatch event ID %s"), msg+2 );
					if( service->functions && service->functions[MSG_ServiceLoad].function )
					{
						success = service->functions[MSG_ServiceLoad].function( ((&msg->sourceid) + 1), 1
																								, resultbuf, &resultlen );
						Responce.events = service->entries;
					}
					else
					{
						// no function, no handler, respond on behalf of the service...
						// but it doesn't have any internal tracking of this
						// client... should probably not generate mate ended messages either?
						if( !service->handler )
							success = TRUE;
					}
				}
				if( !success )
					Responce.msg.hdr.msgid = MSG_ServiceLoad|SERVER_FAILURE;
				//lprintf( WIDE("Responce to pid: %d"), Responce[0] );
				if( msgsnd( g.msgq_in, MSGTYPE &Responce, sizeof( Responce ) - sizeof( MSGIDTYPE ), 0 ) < 0 )
					lprintf( WIDE("Error with sending message to msgq_out: %d"), errno );
				//else
				//	lprintf( WIDE("Success with sending message to msgq_out: %d"), errno );
			}
			else
			{
#ifdef DEBUG_MESSAGE_BASE_ID
				DBG_VARSRC;
#endif
				Responce.msg.hdr.msgid = MSG_ServiceLoad|SERVER_FAILURE;
				lprintf( WIDE("Failed to find the desired service (%s) for the client to connect to..."), (char*)((&msg->sourceid) + 1) );
				if( msgsnd( g.msgq_in, MSGTYPE &Responce, sizeof(MSGHDR), 0 ) < 0 )
					lprintf( WIDE("Error with sending message to msgq_out: %d"), errno );
				//else
				//	lprintf( WIDE("Success with sending message to msgq_out: %d"), errno );
			}
		}
		break;
	}
}

CLIENTMSG_PROC( void, HandleServerMessage )( PMSGHDR msg )
{
	HandleServerMessageEx( msg DBG_SRC );
}


//--------------------------------------------------------------------

int CPROC DoHandleServiceMessages( PTRSZVAL psvService )
//#define DoHandleServiceMessages(p) DoHandleServiceMessagesEx(p DBG_SRC)
{
	PCLIENT_SERVICE pService = (PCLIENT_SERVICE)psvService;

	S_32 length;
	//_32 recv[2048];
	//_32 result[2048];
	_32 result_length = INVALID_INDEX; // default to ... 
	if( !IsThisThread( pService->thread ) )
		return -1;
	// can't use getpid cause we really want the threadID
	// also this value may NOT be negative.

	// consume any outstanding messages... so we don't get confused
	// someone may have requested things for this service...
			//lprintf( "vvv" );
	while( msgrcv( g.msgq_out
					 , MSGTYPE pService->recv
					 , 8192
					 , g.my_message_id
					 , IPC_NOWAIT ) > 0 )
	{
		//lprintf( WIDE("Dropping a message...") );
	}
			//lprintf( "^^^" );
	while( !g.flags.disconnected )
	{
		//lprintf( WIDE("service is waiting for messages to %08lx"), g.my_message_id );
		// receiving from the 'out' queue which is commands TO a service.
			//lprintf( "vvv" );
		pService->flags.bWaitingInReceive = 1;

		length = msgrcv( g.msgq_out
							, MSGTYPE pService->recv
							, 8192
							, g.my_message_id
							, 0 );
		if( pService->flags.bClosed )
		{
			break;
		}
		pService->flags.bWaitingInReceive = 0;
			//lprintf( "^^^" );
		if( length < 0 )
		{
			if( errno == EINTR ) // got a signal - ignore and try again.
				continue;
			if( errno == EIDRM )
			{
				Log( WIDE("Server ended.") );
				break;
			}
			if( errno == EINVAL )
			{
				Log( WIDE("Queues Closed?") );
				g.flags.disconnected = 1;
				break;
			}
#if defined( _WIN32 ) || defined( USE_SACK_MSGQ )
			if( errno == EABORT )
			{
				Log( WIDE( "Server Read Abort." ) );
				break;
			}
#endif
			Log1( WIDE("msgrecv error: %d"), errno );
			continue;
		}
#ifdef DEBUG_DATA_XFER
		else
		{
			lprintf( WIDE("Received Mesage.... g.msgq_out"));
			LogBinary( (P_8)pService->recv, length + sizeof( MSGIDTYPE ) );
		}
#endif
		if( pService->recv->hdr.sourceid & 0xF0000000 )
		{
			// message is a responce from someone else...
		}
		else
		{
		//Log3( WIDE("Got a message: %08x from %08x length %d"), recv[0], recv[1], length - sizeof( MSGIDTYPE ) );
			//if( pService->recv[1] < 0x100 )
			//{
			// this is a message to the server itself.
			// such as 'load_service...'
			//	HandleServerMessage( pService->recv + 1 );
			//}
			//else
			{
            int handled = FALSE;
				PCLIENT_SERVICE service;
				for( service = g.services; service; service = service->next )
				{
					_32 msgid = pService->recv->hdr.msgid;
					if( msgid == MSG_ServiceLoad )
					{
						if( ( g.my_message_id == *((&pService->recv->hdr.sourceid)+1) ) )
						{
						// Load to myself...
							HandleServerMessage( &pService->recv->hdr );
							break;
						}
					}
#ifdef DEBUG_MESSAGE_BASE_ID
					lprintf( WIDE("service base %ld(+%ld) and this is from %s")
							 , service->first_message_index
                       , service->entries
							 , ( g.my_message_id == pService->recv->hdr.sourceid )?"myself":"someone else" );
#endif
					msgid -= service->first_message_index;
					if( !(msgid & 0x80000000) &&	// subtraction resulted in 'negative'
						( msgid < service->entries ) /*
					  && ( g.my_message_id != pService->recv[2] )*/ )  // within the range of the service
					{
					//g.flags.handling_client_message = 1;
					//Log( WIDE("Found a service this message is for...") );
						{
							{
								int result_okay = 0;
								if( service->handler_ex )
								{
#if defined( LOG_HANDLED_MESSAGES )
									lprintf( WIDE("Got a service message to handler: %08lx length %ld")
											 , pService->recv->hdr.sourceid
											 , length - sizeof(MSGHDR) );
#endif
									result_length = MSG_DEFAULT_RESULT_BUFFER_MAX;
                           handled = TRUE;
									result_okay = service->handler_ex( service->psv
																				, pService->recv->hdr.sourceid
																				, msgid
																				, QMSGDATA(pService->recv[0]), length - sizeof(MSGHDR)
																				, QMSGDATA(pService->result[0]), &result_length ) ;
								}
								if( service->handler )
								{
#if defined( LOG_HANDLED_MESSAGES )
									lprintf( WIDE("Got a service message to handler: %08lx length %ld")
											 , pService->recv->hdr.sourceid
											 , length - sizeof(MSGHDR) );
#endif
									result_length = MSG_DEFAULT_RESULT_BUFFER_MAX;
                           handled = TRUE;
									result_okay = service->handler( pService->recv->hdr.sourceid
																			, msgid
																			, QMSGDATA(pService->recv[0]), length - sizeof(MSGHDR)
																			, QMSGDATA(pService->result[0]), &result_length ) ;
								}
								if( !result_okay )
								{
									if( service->functions
									  && service->functions[msgid].function )
									{
										//result_length = 4096; // maximum responce buffer...
#if defined( LOG_HANDLED_MESSAGES )
										lprintf( WIDE("Got a service : (%d)%s from %08lx length %ld")
												 , msgid
												 , service->functions[msgid].name
												 , pService->recv[2]
												 , length - sizeof(MSGHDR) );
#endif
										result_length = 0; // safer default. although uninformative.
                           handled = TRUE;
										result_okay = service->functions[msgid].function( (_32*)(pService->recv + 1)
																										, length - sizeof(MSGHDR)
																										, ((&pService->result->hdr.sourceid)+1)
																										, (_32*)&result_length );
									}
									else if( service->functions )
									{
#if defined( LOG_HANDLED_MESSAGES )
                              DebugBreak();
										lprintf( "didn't have a function for 0x%lx (%ld) or %s" 
												 , msgid
												 , msgid
												 , service->functions[msgid].name );
#endif
										result_okay = 0;
										result_length = INVALID_INDEX;
									}
									else
									{
										DebugBreak();
										result_okay = 0;
										result_length = 0;
									}
								}
								if( result_okay )
									pService->result->hdr.msgid = pService->recv->hdr.msgid | SERVER_SUCCESS;
								else
									pService->result->hdr.msgid = pService->recv->hdr.msgid | SERVER_FAILURE;

								// a key result value to indicate there is
								// no responce to be sent to the client.
								if( result_length != INVALID_INDEX )
								{
#ifdef DEBUG_MESSAGE_BASE_ID
									DBG_VARSRC;
#endif
									//Log3( WIDE("Result %ld length: %ld Result First dword: %08x ")
									//	 , pService->recv[0], result_length, result->hdr.msgid );
									pService->result->targetid = pService->recv->hdr.sourceid;
									pService->result->hdr.sourceid = g.my_message_id;
									// +sizeof(MSGHDR) in the following line is +12 -4 (4 is the msgbuf content)
#ifdef DEBUG_OUTEVENTS
		//lprintf( WIDE("Sending result to application...") );
		//LogBinary( (POINTER)pService->result, result_length + sizeof( QMSG ) );

#endif
									{
#ifdef DEBUG_DATA_XFER
										DBG_VARSRC;
#endif
										msgsnd( g.msgq_in, MSGTYPE pService->result, result_length + sizeof(MSGHDR), 0 );
									}
								}
								else
								{
									//Log( WIDE("No responce sent") );
								}
								break;
							}
						}
						break;
					}
				}
				// the master server actually registers a service
				// which is able to handle base 0 messages...
				// other processes need to fake it to handle these generically
				if( !service )
				{
					if( pService->recv->hdr.msgid < 0x100 )
					{
						// this is a message to the server itself.
						// such as 'load_service...'
						HandleServerMessage( &pService->recv->hdr );
					}
				}
			}
#if defined(_DEBUG) && defined( LOG_HANDLED_MESSAGES )
			Log( WIDE("Message complete...") );
#endif
			g.flags.handling_client_message = 0;
		}
	}
	pService->flags.bWaitingInReceive = 0;

	return 0;
}

//--------------------------------------------------------------------

void DoRegisterService( PCLIENT_SERVICE pService )
{
	_32 MsgID;
	_32 MsgInfo[7];
	_32 MsgLen = 20; // expent MsgBase = 0, EventMessgaeCount = 1

				  // if I'm the master service, I don't very well have to
				  // register with myself do I?
	if( g.flags.bMasterServer )
	{
		pService->flags.bRegistered = 1;
	}
	if( !pService->flags.bMasterServer && !pService->flags.bFailed )
	{
		//lprintf( WIDE("Transacting a message....") );
		if( !TransactServerMultiMessage( CLIENT_REGISTER_SERVICE, 2
												 , &MsgID, MsgInfo, &MsgLen
												 , &pService->first_message_index, sizeof( pService->first_message_index )
												 , pService->name, strlen( pService->name ) + 1 // include NUL
												 ) )
		{
			pService->flags.bFailed = 1;
			lprintf( WIDE("registration failed.") );
		}
		else
		{
			//lprintf( WIDE("MsgID is %lx and should be %lx? "),MsgID , ( CLIENT_REGISTER_SERVICE | SERVER_SUCCESS ) );
			if( MsgID != ( CLIENT_REGISTER_SERVICE | SERVER_SUCCESS ) )
			{
				pService->flags.bFailed = 1;
				lprintf( WIDE("registration failed.") );
			}
		}
		pService->flags.bRegistered = 1;
	}
}

//--------------------------------------------------------------------

static PTRSZVAL CPROC HandleServiceMessages( PTHREAD thread )
{
	PCLIENT_SERVICE service = (PCLIENT_SERVICE)GetThreadParam( thread );
	if( service )
	{
		//g.pMessageThread = thread;
		service->thread = thread;
		//service->my_message_id = getpid(); //thread->ThreadID & 0x7FFFFFFFUL; /*(_32)getpid()*/;
		//lprintf( WIDE("Regsitering service with main msgsvr") );
		DoRegisterService( service );
		//lprintf( WIDE("Entering service message dispatch...") );
		while( !g.flags.disconnected && !service->flags.bClosed )
			DoHandleServiceMessages( (PTRSZVAL)service );
		service->thread = NULL;
		//g.pMessageThread = NULL;
	}
	else
	{
		// this special case server does not have a service handler....
	}

	return 0;
}

//--------------------------------------------------------------------

void ResumeThreads( void )
{
#ifndef _WIN32
	_32 tick;
	if( g.pThread )
	{
		lprintf( WIDE("Resume Service") );
		tick = GetTickCount();
		g.pending = 1;
		pthread_kill( ( GetThreadID( g.pThread ) & 0xFFFFFFFF ), SIGUSR2 );
		while( ((tick+10)>GetTickCount()) && g.pending ) Relinquish();
	}
	if( g.pMessageThread )
	{
		lprintf( WIDE("Resume Responce") );
		tick = GetTickCount();
		g.pending = 1;
		pthread_kill( ( GetThreadID( g.pMessageThread ) & 0xFFFFFFFF ), SIGUSR2 );
		while( ((tick+10)>GetTickCount()) && g.pending ) Relinquish();
	}

	if( g.pEventThread )
	{
		lprintf( WIDE("Resume event") );
		tick = GetTickCount();
		g.pending = 1;
		pthread_kill( ( GetThreadID( g.pEventThread ) & 0xFFFFFFFF ), SIGUSR2 );
		while( ((tick+10)>GetTickCount()) && g.pending ) Relinquish();
	}
	if( g.pLocalEventThread )
	{
		lprintf( WIDE("Resume local event") );
		tick = GetTickCount();
		g.pending = 1;
		pthread_kill( ( GetThreadID( g.pLocalEventThread ) & 0xFFFFFFFF ), SIGUSR2 );
		while( ((tick+10)>GetTickCount()) && g.pending ) Relinquish();
	}
#else
	if( g.pThread )
		WakeThread( g.pThread );
	if( g.pMessageThread )
		WakeThread( g.pMessageThread );
	if( g.pEventThread )
		WakeThread( g.pEventThread );
	if( g.pLocalEventThread )
		WakeThread( g.pLocalEventThread );
#endif
}

//--------------------------------------------------------------------

void CloseMessageQueues( void )
{
	g.flags.disconnected = TRUE;
	if( g.flags.bMasterServer )
	{
		// master server closing, removes all id's
		// removing the message queue should be enough
		// to wake each of these threads...
		if( g.msgq_in != MSGFAIL )
		{
			msgctl( g.msgq_in, IPC_RMID, NULL );
			g.msgq_in = 0;
		}
		if( g.msgq_out != MSGFAIL )
		{
			msgctl( g.msgq_out, IPC_RMID, NULL );
			g.msgq_out = 0;
		}
		if( g.msgq_event != MSGFAIL )
		{
			msgctl( g.msgq_event, IPC_RMID, NULL );
			g.msgq_event = 0;
		}
		if( g.msgq_local != MSGFAIL )
		{
			msgctl( g.msgq_local, IPC_RMID, NULL );
			g.msgq_local = 0;
		}
	}
	else
	{
		// we have to wake up everyone, so they can realize we're disconnected
		// and leave...
		// then we wait some short time for everyone to exit...
		ResumeThreads();
	}
	{
		_32 attempts = 0;
		_32 time;
		g.msgq_in = 0;
		g.msgq_out = 0;
		g.msgq_event = 0;
		g.msgq_local = 0;
		do
		{
			time = GetTickCount();
			while( ((time+100)>GetTickCount()) && g.pThread ) Relinquish();
			time = GetTickCount();
			while( ((time+100)>GetTickCount()) && g.pMessageThread ) Relinquish();
			time = GetTickCount();
			while( ((time+100)>GetTickCount()) && g.pEventThread ) Relinquish();
			time = GetTickCount();
			while( ((time+100)>GetTickCount()) && g.pLocalEventThread ) Relinquish();
			if( g.pThread || g.pMessageThread || g.pEventThread || g.pLocalEventThread )
			{
				attempts++;
				lprintf( WIDE("Threads are not exiting... %") _32f WIDE(" times"), attempts );
				if( attempts < 10 )
					continue; // skips
			}
		} while( 0 );
	}
	// re-establish our communication ID if we
	// end up with more work to do...
	g.flags.connected = 0;
}

//--------------------------------------------------------------------

static void DisconnectClient(void)
{
	static int bDone;
	PEVENTHANDLER pHandler;
	if( bDone )
		return;
	bDone = 1;

	//lprintf( WIDE("Disconnect all clients... %Lx"), GetMyThreadID() );
	while( ( pHandler = g.pHandlers ) )
	{
		//lprintf( WIDE("Unloading a service...") );
		UnloadService( pHandler->MyMsgBase );
	}
	//lprintf( WIDE("Okay all registered services are gone.") );
	// no real purpose in this....
	// well perhaps... but eh...
	// if( !master server )
	//SendServerMessage( CLIENT_DISCONNECT, NULL, 0 );
	CloseMessageQueues();

}

PRELOAD( thing )
{
// under linux atexit() registered procs run before
// destructors.
	//atexit( DisconnectClient );
}

ATEXIT_PRIORITY( _DisconnectClient, ATEXIT_PRIORITY_MSGCLIENT )
{
	DisconnectClient();
}

//--------------------------------------------------------------------

int RegisterWithMasterService( void )
{
	if( g.my_message_id )
		return TRUE;
	if( g.flags.bMasterServer )
	{
	// I'm already done here....
		// I am what I am, and that's it.
		g.flags.connected = 1;
	}

	if( !g.flags.connected )
	{
		_32 Result;
		_32 msg[2];
		_32 msglen = sizeof( msg );
		lprintf( WIDE("Connecting first time to service server...") );
		if( !TransactServerMessageExx( CLIENT_CONNECT, NULL, 0
											  , &Result, msg, &msglen
											  , 100 DBG_SRC )
		  || Result != (CLIENT_CONNECT|SERVER_SUCCESS) )
		{
			Log( WIDE("Failed CLIENT_CONNECT") );
			g.flags.failed = TRUE;
			// I see no purpose for this other than troubleshooting
			// RegisterWithMaster is called well after this should
			// have been set...
			//g.flags.message_responce_handler_ready = TRUE;
			return FALSE;
		}
		else
		{
		// result and pid in message received from server are trashed...
		//g.master_server_pid = msg[0];
		// modify my_message_id - this is now the
		// ID of messages which will be sent
		// and where repsonces will be returned
		// this therefore means that
			lprintf( WIDE("Okay now I have the ID I am communcating on...") );
			g.flags.connected = 1;
			g.my_message_id = msg[0];
			//lprintf( WIDE("Have changed my_message_id and now we need to wake all receivers...") );
			// this causes them to re-queue their requests with the new
			// flag... although the windows implementation passes the address
			// of this variable, so next message will wake this thread, however,
			// if the message was already posted, still will have to wake them
			// so they can scan for new-ready.
			ResumeThreads();
		}
	}
	return TRUE;
}

//--------------------------------------------------------------------

int InitMessageService( void )
{
	return TRUE;
}

//--------------------------------------------------------------------

static MSGQ_TYPE OpenQueueEx( TEXTCHAR *name, int key, int flags DBG_PASS )
#ifdef _WIN32
#define OpenQueue(n,k,f) OpenQueueEx(n,0,f DBG_SRC)
#else
#define OpenQueue(n,k,f)  OpenQueueEx( n,k,f DBG_SRC )
#endif
{
	MSGQ_TYPE queue;
	if( g.flags.bMasterServer )
	{
		queue = msgget( name, key, IPC_CREAT|IPC_EXCL|0666 );
		if( queue == MSGFAIL )
		{
			lprintf( WIDE("Failed to create message Q for \"%s\":%s for") DBG_FILELINEFMT, name, strerror(errno) DBG_RELAY );
			queue = msgget( name, key, 0 );
			if( queue == MSGFAIL )
			{

				//perror( WIDE("Failed to open message Q") );
			}
			else
			{
				lprintf( WIDE("Removing message queue id for %s"), name );
				msgctl( queue, IPC_RMID, NULL );
				queue = msgget( name, key, IPC_CREAT|IPC_EXCL|0666 );
				if( queue == MSGFAIL )
				{
					lprintf( WIDE("Failed to open message Q for \"%s\":%s"), name, strerror(errno) );
				}
			}
		}
	}
	else
	{
		queue = msgget( name, key, flags|0666 );
		if( queue == MSGFAIL )
		{
#ifdef DEBUG_MSGQ_OPEN
			lprintf( WIDE("Failed to create message Q for \"%s\":%s for ") DBG_FILELINEFMT, name, strerror(errno) DBG_RELAY );
#endif
			//lprintf( WIDE("Failed to open message Q for \")%s\":%s", name, strerror(errno) );
		}
	}
	return queue;
}


static int _InitMessageService( int local )
{
#ifdef __LINUX__
	key_t key, key2, key3, key4;
	signal( SIGUSR2, ResumeSignal ); // ignore this ...
#endif

	// key and key2 are reversed from the server - so my out is his in
	// and his inis my out.
	// we do funny things here since we switch in/out vs server.
#ifdef __LINUX__
	key = *(long*)MSGQ_ID_BASE; // server input, client output
	key2 = key + 1;  // server output, client input
	key3 = key + 2;  // pid-addressed events (all ways)
	key4 = key + 3;  // pid-addressed events (all ways)
#endif
	// until connected, our message handler ID
	// is my pid.  Then, once connected, we listen
	// for messages with the ID which was granted by the message
	// service.
	// maybe this could be declared to be '2'
	// and then before the client-connect is done, attempt
	// to send to process 2... if a responce is given, someone
	// else is currently registering with the message server
	// and we need to wait.

	// not failed... attempting to re-connect
	g.flags.failed = 0;
	if( !local )
	{
		if( g.flags.disconnected )
		{
			lprintf( WIDE("Previously we had closed all communication... allowing re-open.") );
			g.flags.disconnected = 0;
			g.my_message_id = 0; // reset this... so we re-request for new path...
		}
	}
	if( g.flags.bMasterServer )
	{
		if( g.my_message_id == 1 ) 
			return TRUE;
		lprintf( WIDE("Setting master server ID Wake all others, so they can request correct PID messages") );
		if( !g.flags.found_server )
		{
			g.my_message_id = 1;
			// need to wake up the other receiver thread thing
			// cause my message ID is now 1...
			ResumeThreads();
		}
	}
	else
	{
		if( !g.nMsgCounter )
			g.nMsgCounter = LOWEST_BASE_MESSAGE;
		if( !g.flags.connected )
		{
#ifdef __LINUX__
			g.my_message_id = getpid(); //pService->thread->ThreadID & 0x7FFFFFFFUL; /*(_32)getpid()*/;
#else
			//if( !local )
			{
			g.my_message_id = GetCurrentProcessId();
				//DebugBreak(); // please get a message id here...
			}
#endif
		}
	}
	if( !local && !g.msgq_in && !g.flags.message_responce_handler_ready )
	{
#ifdef DEBUG_MSGQ_OPEN
		lprintf( WIDE("opening message queue? %d %d %d")
				 , local, g.msgq_in, g.flags.message_responce_handler_ready );
#endif
		g.msgq_in = OpenQueue( MSGQ_ID_BASE WIDE("1"), key2, 0 );
#ifdef DEBUG_MSGQ_OPEN
		lprintf( WIDE("Result msgq_in = %ld"), g.msgq_in );
#endif
		if( g.msgq_in == MSGFAIL )
		{
			g.msgq_in = 0;
			return FALSE;
		}
#ifdef DEBUG_THREADS
		lprintf( WIDE("Creating thread to handle responces...") );
#endif
		AddIdleProc( ProcessClientMessages, 0 );
		ThreadTo( HandleMessages, 0 );
		while( !g.flags.message_responce_handler_ready )
			Relinquish();
	}
	if( !local && !g.msgq_out && !g.flags.message_handler_ready )
	{
		g.msgq_out = OpenQueue( MSGQ_ID_BASE WIDE("0"), key, 0 );
		if( g.msgq_out == MSGFAIL )
		{
			g.msgq_out = 0;
			return FALSE;
		}
		// just allow this thread to be created later...
		// need to open the Queue... but that's about it.
#ifdef DEBUG_THREADS
		//lprintf( WIDE("Creating thread to handle messages...") );
#endif
		//ThreadTo( HandleServiceMessages, 0 );
		//while( !g.flags.message_handler_ready )
		//	Relinquish();
	}
	if( !local && !g.msgq_event && !g.flags.events_ready )
	{
		g.msgq_event = OpenQueue( MSGQ_ID_BASE WIDE("2"), key3, 0 );
		if( g.msgq_event == MSGFAIL )
		{
			g.msgq_event = 0;
			return FALSE;
		}
#ifdef DEBUG_THREADS
		lprintf( WIDE("Creating thread to handle events...") );
#endif
		ThreadTo( HandleEventMessages, 0 );
		while( !g.flags.events_ready )
			Relinquish();
	}
	if( local && !g.msgq_local && !g.flags.local_events_ready )
	{
	// huh - how can I open this shm under linux?
		g.msgq_local = OpenQueue( NULL, key4, IPC_CREAT );
		if( g.msgq_local == MSGFAIL )
		{
			g.msgq_local = 0;
			return FALSE;
		}
#ifdef DEBUG_THREADS
		lprintf( WIDE("Creating thread to handle local events...") );
#endif
		AddIdleProc( ProcessClientMessages, 0 );
		ThreadTo( HandleLocalEventMessages, 0 );
		while( !g.flags.local_events_ready )
			Relinquish();
	}
	// right now our PID is the message ID
	// and after this we will have our correct message ID...
	g._handler.ServiceID = 1;
	g._handler.MyMsgBase = 0;
	g._handler.MsgBase = 0;
	if( !local && !RegisterWithMasterService() )
	{
		CloseMessageQueues();
		lprintf( WIDE("Resetting message queues so we re-open in correct config.") );
		g.msgq_in = 0;
		g.msgq_out = 0;
		g.msgq_event = 0;
		g.msgq_local = 0;

		// reset this ID... allowing us to reinitialize this...
		g.my_message_id = 0;
		//g.flags.failed = 1;
		return FALSE;
	}
	if( !local )
		g.flags.found_server = 1;

	if( g.flags.failed )
	{
		//g.flags.initialized = FALSE;
		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------------

static int ReceiveServerMessageEx( PEVENTHANDLER handler
											 /*, _32 *MessageID
											 , POINTER msg
											 , _32 *len */ DBG_PASS )
{
	if( handler == g.pLoadingService && g.pLoadingService && ( (g.MessageIn.data[0]&0xFFFFFFF) == (MSG_ServiceLoad) ) )
	{
		lprintf( WIDE("Loading service responce... setup the service ID for future com") );
		g.pLoadingService->ServiceID = g.MessageIn.data[1];
		handler = g.pLoadingService;
		g.pLoadingService = NULL;
		if( handler->MessageID )
			(*handler->MessageID) = g.MessageIn.data[0] + handler->bias;
	}

	if( g.MessageIn.data[1] == 1 )
	{
		if( handler != &g._handler )
		{
			// result received from some other handler (probably something like
			//lprintf( WIDE("message from core service ... wrong handler?") );
			if( g.MessageIn.data[0] & SERVER_FAILURE )
			{
				// eat this message...
				//return 0;
			}
			//else
			//	return 1;
			//DebugBreak();
		}
		if( handler->MessageID )
			(*handler->MessageID) = g.MessageIn.data[0] + handler->bias;
  //  	handler = &g._handler;
	}

	if( handler->ServiceID != g.MessageIn.data[1] )
	{
		//lprintf( WIDE("%ld and %ld "), handler->ServiceID, g.MessageIn.data[1] );
		// this handler is not for this message responce...
		//DebugBreak();

		//lprintf( WIDE("this handler is not THE handler!") );
		return 1;
	}
	else
	{
		//lprintf( WIDE("All is well, check message ID %p"), handler );
		if( handler->MessageID )
			(*handler->MessageID) = g.MessageIn.data[0] + handler->bias;

		if( handler->LastMsgID != ( (g.MessageIn.data[0] + handler->bias)& 0xFFFFFFF ) )
		{
			LogBinary( (P_8)&g.MessageIn, g.MessageLen );
			lprintf( WIDE("len was %") _32f, g.MessageLen );
			lprintf( WIDE("Message is for this guy - but isn't the right ID! %") _32f WIDE(" %") _32f WIDE(" %") _32f WIDE("")
					 , handler->LastMsgID, (g.MessageIn.data[0]+ handler->bias) & 0xFFFFFFF, handler->bias );
			//DebugBreak();
			return 1;
		}
	}

	if( handler->msg && handler->len )
	{
		g.MessageLen -= sizeof(MSGHDR);  // subtract message ID and message source from it.
		if( (S_32)(*handler->len) < g.MessageLen )
		{
			Log2( WIDE("Cutting out possible data to the application - should provide a failure! %") _32f WIDE(" expected %") _32fs WIDE(" returned"), (*handler->len), g.MessageLen );
			g.MessageLen = (*handler->len);
		}
		MemCpy( handler->msg, g.MessageIn.data + 2, g.MessageLen );
		(*handler->len) = g.MessageLen;
	}
	else
	{
	// maybe it was just interested in the header...
		// which will be the source ID and the message ID
		if( g.MessageLen - sizeof(MSGHDR) )
		{
			LogBinary( (P_8)&g.MessageIn, g.MessageLen + sizeof( QMSG ) );
			SystemLogEx( WIDE("Server returned result data which the client did not get") DBG_RELAY );
		}
	}
	return 0;
}

//--------------------------------------------------------------------

static int WaitReceiveServerMsg ( PEVENTHANDLER handler
										  , _32 MsgOut
										  , _32 *MsgIn
										  , POINTER BufferIn
										  , _32 *LengthIn )
{
	if( MsgIn )
	{
		int received;
		int IsThread = IsThisThread( g.pThread );
		/*
		if( !IsThread )
			IsThread = IsThisThread( g.pLocalEventThread );
		if( !IsThread )
		IsThread = IsThisThread( g.pEventThread );
		*/
		//lprintf( WIDE("waiting for cmd result") );
#ifdef DEBUG_THREAD
		lprintf( WIDE("This thread? %s"), IsThread?"Yes":"No" );
#endif
		handler->MessageID = MsgIn;
		handler->msg = BufferIn;
		handler->len = LengthIn;
		// therefore, please do relinquish one cycle...
		Relinquish();
		//Log( WIDE("To wait for a responce") );
						 //if( !handler->flags.responce_received )
		handler->flags.bCheckedResponce = 0;
		do
		{
		// this library is totally serialized for one transaction
		// at a time, from multiple threads.
			received = 0;
			while( ( handler->flags.bCheckedResponce ||
					  !g.responce_received ) &&
					(
#ifdef DEBUG_DATA_XFER
#ifdef _DEBUG
					 (lprintf( WIDE("Compare %") _32f WIDE(" vs %") _32f WIDE(" (=%") _32fs WIDE(") (positive keep waiting)")
								, handler->wait_for_responce
								, GetTickCount()
								, handler->wait_for_responce - GetTickCount()  ) ),
#endif
#endif
					( handler->wait_for_responce > GetTickCount() )) ) // wait for a responce
			{
				received = 1;
			// check for responces...
			// will return immediate if is not this thread which
			// is supposed to be there...
				//lprintf( WIDE("getting or waiting for... a message...") );
				if( IsThread )
				{
					lprintf( WIDE("Get message (might be my thread") );
					if( GetAMessage( g.msgq_in, IPC_NOWAIT ) == 2 )
					{
						Log( WIDE("Okay - won't check for messages anymore - just wait...") );
						IsThread = 0;
					}
				}
				if( !IsThread )
				{
					PTHREAD pThread;
					if( g.responce_received && !handler->flags.bCheckedResponce )
					{
						goto dont_sleep;
					}
					AddLink( &g.pSleepers, pThread = MakeThread() );
					if( g.responce_received && !handler->flags.bCheckedResponce )
					{
						DeleteLink( &g.pSleepers, pThread );
						goto dont_sleep;
					}
					handler->flags.bCheckedResponce = 0;
					//lprintf( WIDE("Going to sleep for %ld %016Lx"), handler->wait_for_responce - GetTickCount(), pThread->ThreadID );
					WakeableSleep( handler->wait_for_responce - GetTickCount() );
					//lprintf( WIDE("AWAKE!") );
					DeleteLink( &g.pSleepers, pThread );
				}
				else
				{
					Relinquish();
				}
			}
			if( !handler->flags.bCheckedResponce )
				received = 1;

			//lprintf( WIDE("When we finished this loop still was waiting %ld"), handler->wait_for_responce - GetTickCount() );
			//else
			{
				//lprintf( WIDE("Excellent... the responce is back before I could sleep!") );
			}
			if( !g.responce_received )
			{
				Log( WIDE("Responce timeout!") );
				handler->flags.waiting_for_responce = 0;
				LeaveCriticalSec( &handler->csMsgTransact );
				return FALSE; // DONE - fail! abort!
			}
			else
			{
				//Log( WIDE("Result to application... ") );
			}
		dont_sleep: ;
			handler->flags.bCheckedResponce = 1;
			//lprintf( WIDE("Read message...") );
		}
		while( received && ReceiveServerMessageEx( handler DBG_SRC ) );
		if( received )
		{
			handler->flags.waiting_for_responce = 0;

			//Log2( WIDE("Got responce: %08x %d long"), *MsgIn, LengthIn?*LengthIn:-1 );
			if( ( *MsgIn & 0x0FFFFFFF ) != ( (handler->LastMsgID) & 0x0FFFFFFF ) )
			{
				lprintf( WIDE("Mismatched server responce to client message: %")_32f WIDE(" to %")_32f WIDE(" (%")_32f WIDE(")")
						 , *MsgIn & 0x0FFFFFFF
						 , handler->LastMsgID & 0x0FFFFFFF
						 , handler->bias);
			}
			g.responce_received = 0; // allow more responces to be received.
		}
	}
	if( handler )
	{
		//lprintf( WIDE("cleanup...%p"), handler );
		handler->flags.waiting_for_responce = 0;
		LeaveCriticalSec( &handler->csMsgTransact );
	}
	//lprintf( WIDE("done waiting...") );
	return TRUE;
}

// returns FALSE on timeout, else success.
// this is used by msg.core.dll - used for forwarding messages
// to real handlers...
CLIENTMSG_PROC( int, TransactRoutedServerMultiMessageEx )( _32 RouteID
																			, _32 MsgOut, _32 buffers
																			, _32 *MsgIn
																			, POINTER BufferIn, _32 *LengthIn
																			, _32 timeout
																			// buffer starts arg list, length is
																			// not used, but is here for demonstration
																			, ... )
{
	BUFFER_LENGTH_PAIR *pairs = (BUFFER_LENGTH_PAIR*)Allocate( sizeof( BUFFER_LENGTH_PAIR ) * buffers );
	_32 n;
	va_list args;
	PEVENTHANDLER handler;
	for( handler = g.pHandlers; handler; handler = handler->next )
		if( handler->ServiceID == RouteID )
			break;
	if( !handler )
		if( g._handler.ServiceID == RouteID )
		{
			lprintf( WIDE("Using global handler...") );
			handler = &g._handler;
		}
	// can send the message, but may not get another responce
	// until the first is done...
	if( MsgIn || BufferIn )
	{
		if( !handler )
		{
			lprintf( WIDE("We have no business being here... no loadservice has been made to this service!") );
			return 0;
		}
		lprintf( WIDE("Enter %p"), handler );
		EnterCriticalSec( &handler->csMsgTransact );
		switch( MsgOut )
		{
		case RU_ALIVE:
			lprintf( WIDE("Lying about message to expect") );
			handler->LastMsgID = IM_ALIVE;
			break;
		default:
			lprintf( WIDE("set last msgID") );
			handler->LastMsgID = MsgOut;
			break;
		}
		if( ( handler->MessageID = MsgIn ) )
			(*MsgIn) = handler->LastMsgID;
		handler->wait_for_responce = GetTickCount() + (timeout?timeout:DEFAULT_TIMEOUT);
	}
	//lprintf( WIDE("transact message...") );
	va_start( args, timeout );
	for( n = 0; n < buffers; n++ )
	{
		pairs[n].buffer = va_arg( args, POINTER );
		pairs[n].len = va_arg( args, _32 );
	}
	if( !( PrivateSendDirectedServerMultiMessage( RouteID, MsgOut, buffers
															  , pairs ) ) )
	{
		Release( pairs );
		if( handler )
		{
			handler->flags.waiting_for_responce = 0;
			LeaveCriticalSec( &handler->csMsgTransact );
		}
 		return FALSE;
	}
	Release( pairs );
	lprintf( WIDE("Entering wait after serving a message...") );
	if( MsgIn || (BufferIn && LengthIn) )
		return WaitReceiveServerMsg( handler
											, MsgOut
											, MsgIn
											, BufferIn
											, LengthIn );
	else
	{
		handler->flags.waiting_for_responce = 0;
		LeaveCriticalSec( &handler->csMsgTransact );
		return TRUE;
	}
}

struct {
	CTEXTSTR pFile;
	int nLine;
}next_transact;

#undef TransactServerMultiMessage
CLIENTMSG_PROC( int, TransactServerMultiMessage )( _32 MsgOut, _32 buffers
										, _32 *MsgIn, POINTER BufferIn, _32 *LengthIn
										 // buffer starts arg list, length is
										 // not used, but is here for demonstration
										, ... )
{
	BUFFER_LENGTH_PAIR *pairs = (BUFFER_LENGTH_PAIR*)Allocate( sizeof( BUFFER_LENGTH_PAIR ) * buffers );
	_32 n;
	va_list args;
	int stat;
	PEVENTHANDLER handler;
	_32 bias;
	va_start( args, LengthIn );
	for( n = 0; n < buffers; n++ )
	{
		pairs[n].buffer = va_arg( args, CPOINTER );
		pairs[n].len = va_arg( args, _32 );
	}
	//if( MsgOut == 0x23f )
	//	DebugBreak();
	lprintf( "%s(%d):Sending message...", next_transact.pFile, next_transact.nLine );
#ifdef _DEBUG
	if( !( handler = PrivateSendServerMultiMessageEx( &MsgOut, &bias, buffers
																	, pairs
																	, next_transact.pFile, next_transact.nLine
																	) ) )
#else
	if( !( handler = PrivateSendServerMultiMessageEx( &MsgOut, &bias, buffers
																	, pairs
																	
																	) ) )
#endif
	{
		//lprintf( WIDE("Leaving...") );
		//handler->flags.wait_for_responce = 0;
		//LeaveCriticalSec( &handler->csMsgTransact );
		Release( pairs );
		return FALSE;
	}

	next_transact.pFile = "no File";
	Release( pairs );
	handler->bias = bias;
	handler->wait_for_responce = GetTickCount() + (DEFAULT_TIMEOUT);
	//lprintf( WIDE("waiting... %p"), handler );
	if( MsgIn || (BufferIn && LengthIn) )
		stat = WaitReceiveServerMsg( handler
											, MsgOut
											, MsgIn
											, BufferIn
											, LengthIn );
	else
	{
		handler->flags.waiting_for_responce = 0;
		LeaveCriticalSec( &handler->csMsgTransact );
		stat = TRUE;
	}
	//lprintf( WIDE("Done %p %d"),handler, stat );
	return stat;

}

CLIENTMSG_PROC( TSMMProto
										 // buffer starts arg list, length is
                               // not used, but is here for demonstration
									,
 TransactServerMultiMessageExEx )( DBG_VOIDPASS )
{
#ifdef _DEBUG
	next_transact.pFile = pFile;
	next_transact.nLine = nLine;
#endif
	return TransactServerMultiMessage;
}


CLIENTMSG_PROC( int, TransactServerMultiMessageEx )( _32 MsgOut, _32 buffers
																	, _32 *MsgIn, POINTER BufferIn, _32 *LengthIn
																	, _32 timeout
										 // buffer starts arg list, length is
										 // not used, but is here for demonstration
										, ... )
{
	BUFFER_LENGTH_PAIR *pairs = (BUFFER_LENGTH_PAIR*)Allocate( sizeof( BUFFER_LENGTH_PAIR ) * buffers );
	_32 n;
	va_list args;
	int stat;
	PEVENTHANDLER handler;
	_32 bias;
	va_start( args, timeout );
	for( n = 0; n < buffers; n++ )
	{
		pairs[n].buffer = va_arg( args, POINTER );
		pairs[n].len = va_arg( args, _32 );
	}
	if( !( handler = PrivateSendServerMultiMessage( &MsgOut, &bias, buffers
												 , pairs ) ) )
	{
		Release( pairs );
		handler->flags.waiting_for_responce = 0;
		LeaveCriticalSec( &handler->csMsgTransact );
		return FALSE;
	}
	Release( pairs );
	handler->bias = bias;
	handler->wait_for_responce = GetTickCount() + (timeout?timeout:DEFAULT_TIMEOUT);
	if( MsgIn || (BufferIn && LengthIn) )
		stat = WaitReceiveServerMsg( handler
											, MsgOut
											, MsgIn
											, BufferIn
											, LengthIn );
	else
	{
		handler->flags.waiting_for_responce = 0;
		LeaveCriticalSec( &handler->csMsgTransact );
		stat = TRUE;
	}
	return stat;

}

//--------------------------------------------------------------------

CLIENTMSG_PROC( int, TransactServerMessageEx)( _32 MsgOut, CPOINTER BufferOut, _32 LengthOut
						  , _32 *MsgIn, POINTER BufferIn, _32 *LengthIn DBG_PASS )
{
	return TransactServerMultiMessage( MsgOut, 1, MsgIn, BufferIn, LengthIn
												, BufferOut, LengthOut );
}

//--------------------------------------------------------------------

CLIENTMSG_PROC( int, TransactServerMessageExx)( _32 MsgOut, CPOINTER BufferOut, _32 LengthOut
															 , _32 *MsgIn, POINTER BufferIn, _32 *LengthIn
															  , _32 timeout DBG_PASS )
{
	return TransactServerMultiMessageEx( MsgOut, 1, MsgIn, BufferIn, LengthIn, timeout
												  , BufferOut, LengthOut );
}

//--------------------------------------------------------------------

CLIENTMSG_PROC( int, ProbeClientAlive )( _32 RouteID )
{
	_32 Responce;
	if( RouteID == g.my_message_id )
	{
		lprintf( WIDE("Yes, I, myself, am alive...") );
		return TRUE;
	}
	lprintf( WIDE("Hmm is client %")_32f WIDE(" alive?"), RouteID );
	{
		PEVENTHANDLER handler;
		for( handler = g.pHandlers; handler; handler = handler->next )
			if( handler->ServiceID == RouteID )
				break;
		if( !handler )
		{
			// has a nul name.
			handler = (PEVENTHANDLER)Allocate( sizeof( EVENTHANDLER ) );
			MemSet( handler, 0, sizeof( EVENTHANDLER ) );
			handler->ServiceID = RouteID;
			LinkThing( g.pHandlers, handler );
			lprintf( WIDE("Created a HANDLER to coordinate probe alive request..") );
		}
	}
	if( TransactRoutedServerMultiMessageEx( RouteID, RU_ALIVE, 0
													  , &Responce, NULL, NULL
#ifdef DEBUG_DATA_XFER
														// if logging data xfer - we need more time
													  , 250
#else
													  , 250 // 10 millisecond timeout... should be more than generous.
#endif
													  , NULL, NULL ) &&
		Responce == ( IM_ALIVE ) )
	{
		lprintf( WIDE("Yes.") );
		return TRUE;
	}
	lprintf( WIDE("No.") );
	return FALSE;
}

//--------------------------------------------------------------------

static void EndClient( PSERVICE_CLIENT client )
{
	// call the service termination function.
	PCLIENT_SERVICE service;
	INDEX idx;
	// for all services inform it that the client is defunct.
	Log( WIDE("Ending client (from death)") );
	if( client->flags.is_service )
	{
		lprintf( WIDE("Service is gone! please tell client...") );
		if( client->handler->Handler )
			client->handler->Handler( MSG_MateEnded, NULL, 0 );
		if( client->handler->HandlerEx )
			client->handler->HandlerEx( client->handler->ServiceID, MSG_MateEnded, NULL, 0 );
		if( client->handler->HandlerExx )
			client->handler->HandlerExx( client->handler->psv, client->handler->ServiceID, MSG_MateEnded, NULL, 0 );
		UnlinkThing( client->handler );
		Release( client->handler );
	}
	else
	{
		LIST_FORALL( client->services, idx, PCLIENT_SERVICE, service )
		{
			//lprintf( WIDE("Client had service... %p"), service );
			if( service->handler )
			{
				service->handler( client->pid, MSG_ServiceUnload
									 , NULL, 0
									 , NULL, NULL );
			}
			else if( service->handler_ex )
			{
				service->handler_ex( service->psv
										 , client->pid, MSG_ServiceUnload
										 , NULL, 0
										 , NULL, NULL );
			}
			else if( service->functions && service->functions[MSG_ServiceUnload].function )
			{
				_32 resultbuf[1];
				_32 resultlen = 4;
				service->functions[MSG_ServiceUnload].function( (&client->pid) + 1, 0
																			 , resultbuf, &resultlen );
			}
			//Log( WIDE("Ending service on client...") );
			// Use ungloadservice to signal server services of client loss...
			//if( !UnloadService( service->first_message_index, client->pid ) )
			//	Log( WIDE("Somehow unloading a known service failed...") );
		}
		DeleteList( &client->services );
		{
			static _32 msg[2048];
			S_32 len;
			//lprintf( "vvv" );
			while( (len=msgrcv( g.msgq_out, MSGTYPE msg, 8192, client->pid, IPC_NOWAIT )) >= 0 )
				//	errno != ENOMSG )
				lprintf( WIDE("Flushed a message to dead client(%") _32f WIDE(",%") _32f WIDE(") from output (%08") _32fx WIDE(":%") _32fs WIDE(" bytes)")
						 , client->pid
						 , msg[0]
						 , msg[1]
						 , len );
			//lprintf( "^^^" );
			//lprintf( "vvv" );
			while( msgrcv( g.msgq_event, MSGTYPE msg, 8192, client->pid, IPC_NOWAIT ) >= 0 ||
					errno != ENOMSG )
				Log( WIDE("Flushed a message to dead client from event") );
			//lprintf( "^^^" );
		}
	}
	// delete the client.
	Log( WIDE("Finally unlinking the client...") );
	UnlinkThing( client );
	Release( client );
}
//--------------------------------------------------------------------

static void CPROC MonitorClientActive( PTRSZVAL psv )
{
	// for all clients connected send an alive probe to see
	// if we can free their resources.
	PSERVICE_CLIENT client, next = g.clients;
	//const char *pFile = __FILE__;
	//int nLine = __LINE__;
#ifdef DEBUG_RU_ALIVE_CHECK
	Log( WIDE("Checking client alive") );
#endif
	// if am handling a message don't check alivenmess..
	if( !g.flags.handling_client_message )
	{
		while( ( client = next ) )
		{
			next = client->next;
#ifdef DEBUG_RU_ALIVE_CHECK
			lprintf( WIDE("Client %d(%p) last received %d ms ago"), client->pid, client, GetTickCount() - client->last_time_received );
#endif
			if( ( client->last_time_received + CLIENT_TIMEOUT ) < GetTickCount() )
			{
				Log( WIDE("Client has been silent +") STRSYM(CLIENT_TIMEOUT) WIDE("ms - he's dead. (maybe he unloaded and we forgot to forget him?!)") );
				EndClient( client );
			}
			else if( ( client->last_time_received + (CLIENT_TIMEOUT/2) ) <  GetTickCount() )
			{
				if( !client->flags.status_queried )
				{
					QMSG msg;
					//_32 msg[3];
#ifdef DEBUG_DATA_XFER
					DBG_VARSRC;
#endif

					//Log1( WIDE("Asking if client %d is alive"), client->pid );
					msg.targetid = client->pid;
					msg.hdr.msgid = RU_ALIVE;
					msg.hdr.sourceid = g.my_message_id;
					client->flags.status_queried = 1;
#ifdef DEBUG_RU_ALIVE_CHECK
					lprintf( "Ask RU_ALIVE..." );
#endif
					msgsnd( g.msgq_in, MSGTYPE &msg, sizeof( msg ) - sizeof( MSGIDTYPE ), 0 );
				}
				else
				{
#ifdef DEBUG_RU_ALIVE_CHECK
					lprintf( "client has been queried for alivness..." );
#endif
				}
			}
			else
			{
				// hmm maybe a shorter timeout can happen...
			}
		}
	}
#if 0
	{
		PEVENTHANDLER pHandler, pNextHandler = g.pHandlers;
		while( pHandler = pNextHandler )
		{
			lprintf( "Check event handler (Service) %p", pHandler );
			pNextHandler = pHandler->next;
			{
				_32 tick;
				if( ( pHandler->last_check_tick + (CLIENT_TIMEOUT) ) < ( tick = GetTickCount() ) )
				{
					pHandler->last_check_tick = tick;
					if( !ProbeClientAlive( pHandler->ServiceID ) )
					{
						lprintf( "Service is gone! please tell client..." );
						if( pHandler->Handler )
							pHandler->Handler( MSG_MateEnded, NULL, 0 );
						if( pHandler->HandlerEx )
							pHandler->HandlerEx( pHandler->ServiceID, MSG_MateEnded, NULL, 0 );
						if( pHandler->HandlerExx )
							pHandler->HandlerExx( pHandler->psv, pHandler->ServiceID, MSG_MateEnded, NULL, 0 );
						UnlinkThing( pHandler );
						Release( pHandler );
						// generate disconnect message to client(myself)
						continue;
					}
					else
						lprintf( "Service is okay..." );
				}
			}
		}
	}
#endif

}

//--------------------------------------------------------------------

static _32 _LoadService( CTEXTSTR service
							  , EventHandlerFunction EventHandler
							  , EventHandlerFunctionEx EventHandlerEx
							  , EventHandlerFunctionExx EventHandlerExx
							  , PTRSZVAL psv
							  )
{
	_32 MsgID;
	_32 MsgInfo[8];
	_32 MsgLen = 24; // expect MsgBase = 0, EventMessgaeCount = 1
	PEVENTHANDLER pHandler;
	pHandler = g.pHandlers;
	MsgInfo[0] = 0; // this is never ever valid.
	// can check now if some other part of this has loaded
	// this service.
				  // reset this status...

	if( !_InitMessageService( service?FALSE:TRUE ) )
	{
#ifdef DEBUG_MSGQ_OPEN
		lprintf( WIDE("Load of %s message service failed."), service );
#endif
		return INVALID_INDEX;
	}
	if( service )
	{
		if( !g.flags.bAliveThreadStarted )
		{
			// this timer monitors ALL clients for inactivity
			// it will probe them with RU_ALIVE messages
			// to which they must respond otherwise be termintated.
			g.flags.bAliveThreadStarted = 1;
			AddTimer( CLIENT_TIMEOUT/4, MonitorClientActive, 0 );
			// each service gets 1 thread to handle their own
			// messages... services do not have 'events' generated
			// to them.
		}
#if 0
		// always query for service, don't short cut... ?
		while( pHandler )
		{
			// only one connection to any given service name
			// may be maintained.  The service itself is resulted...
			if( !strcmp( pHandler->servicename, service ) )
				return pHandler->MyMsgBase;
			pHandler = pHandler->next;
		}
#endif
		EnterCriticalSec( &g.csLoading );
		pHandler = (PEVENTHANDLER)Allocate( (_32)(sizeof( EVENTHANDLER ) + strlen( service )) );
		MemSet( pHandler, 0, sizeof( EVENTHANDLER ) );
		strcpy( pHandler->servicename, service );
		//lprintf( WIDE("Allocating local structure which manages our connection to this service...") );
 
		pHandler->MyMsgBase = g.nMsgCounter;
		g.pLoadingService = pHandler;
		//MsgInfo[0] = (_32)(g.pEventThread->ThreadID);
		//MsgInfo[1] = (_32)(g.pEventThread->ThreadID >> 32);
		// MsgInfo is used both on the send and receives the
		// responce from the service...
		// LoadService goes to the msgsvr and requests the
		// location of the service.
		if( !TransactRoutedServerMultiMessageEx( 1,
															 MSG_ServiceLoad, 1
															, &MsgID, MsgInfo, &MsgLen
															, 250 /* short timeout */
															 //, MsgInfo, 8
															, service, strlen( service ) + 1 // include NUL
															) )
		{
			Log( WIDE("Transact message timeout.") );
			Release( pHandler );
			LeaveCriticalSec( &g.csLoading );
			return INVALID_INDEX;
		}
		if( MsgID != (MSG_ServiceLoad|SERVER_SUCCESS) )
		{
			lprintf( WIDE("Server reports it failed to load [%s] (%08") _32fx WIDE("!=%08x)")
					 , service
					 , MsgID
					 , MSG_ServiceLoad|SERVER_SUCCESS );
			Release( pHandler );
			LeaveCriticalSec( &g.csLoading );
			return INVALID_INDEX;		}
		// uncorrectable anymore.
		//if( MsgLen == 16 )
		//{
		//	lprintf( WIDE("Old server load service responce... lacks the PID of the event handler.") );
		//	MsgInfo[3] = 0;
		//	MsgInfo[4] = 0;
		//	MsgInfo[5] = 0;
		//	MsgLen = 20;
		//}
		if( MsgLen != 24 )
		{
			lprintf( WIDE("Server responce was the wrong length!!! %") _32f WIDE(" expecting %d"), MsgLen, 24 );
			Release( pHandler );
			LeaveCriticalSec( &g.csLoading );
			return INVALID_INDEX;
		}
	}
	else
	{
		pHandler = (PEVENTHANDLER)Allocate( (_32)( sizeof( EVENTHANDLER ) + strlen( WIDE("local_events") ) ) );
		MemSet( pHandler, 0, sizeof( EVENTHANDLER ) );
		pHandler->ServiceID = g.my_message_id;
		strcpy( pHandler->servicename, WIDE("local_events") );
		//lprintf( WIDE("opening local only service... we're making up numbers here.") );
		MsgInfo[0] = g.nMsgCounter; // msgbase 0.
		MsgInfo[1] = (_32)256; // all events.
		MsgInfo[2] = 256;
		if( g.pLocalEventThread )
		{
			MsgInfo[3] = (_32)(GetThreadID( g.pLocalEventThread ));
			MsgInfo[4] = (_32)(GetThreadID( g.pLocalEventThread ) >> 32);
		}
		else
		{
			lprintf( WIDE("Event message system has not started correctly...") );
			Release( pHandler );
			LeaveCriticalSec( &g.csLoading );
			return INVALID_INDEX;
		}
	}

	// EVENTHANDLER is the outbound structure to idenfity
	// the service information which messages go where...
	{
		//pHandler = Allocate( sizeof( EVENTHANDLER ) + strlen( service?service:"local_events" ) );
		//strcpy( pHandler->servicename, service?service:"local_events" );
		//lprintf( WIDE("Allocating local structure which manages our connection to this service...") );
		pHandler->flags.destroyed = 0;
		pHandler->flags.dispatched = 0;

		pHandler->MyMsgBase = g.nMsgCounter;
		//lprintf( WIDE("My Message base abstraction was %") _32f WIDE(""), g.nMsgCounter );
		g.nMsgCounter += MsgInfo[2];
		//lprintf( WIDE("My Message base abstraction is now %") _32f WIDE(""), g.nMsgCounter );
		pHandler->MsgBase = MsgInfo[0];
		pHandler->MsgCountEvents = MsgInfo[2];
		pHandler->MsgCount = MsgInfo[1];
		pHandler->Handler = EventHandler;
		pHandler->HandlerEx = EventHandlerEx;
		pHandler->HandlerExx = EventHandlerExx;
      pHandler->psv = psv;
		// thread ID to wake for events? or to probe?
		// thread ID unused.
		pHandler->EventID = ((_64)MsgInfo[3]) | (((_64)MsgInfo[4]) << 32 );
		if( service )
		{
			pHandler->flags.local_service = 0;
			pHandler->ServiceID = MsgInfo[5]; // magic place where source ID is..
			pHandler->msgq_events = g.msgq_event;
		}
		else
		{
			pHandler->flags.local_service = 1;
			pHandler->ServiceID = g.my_message_id;
			pHandler->msgq_events = g.msgq_local;
		}
		LinkThing( g.pHandlers, pHandler );
		if( service )
		{
			PSERVICE_CLIENT pClient = AddClient( pHandler->ServiceID ); // hang this on the list of services to check...
			pClient->flags.is_service = 1;
			pClient->handler = pHandler;
		}
		LeaveCriticalSec( &g.csLoading );
	}
	return pHandler->MyMsgBase;
}

//--------------------------------------------------------------------

CLIENTMSG_PROC( _32, LoadService)( CTEXTSTR service, EventHandlerFunction EventHandler )
{
	return _LoadService( service, EventHandler, NULL, NULL, 0 );
}

//--------------------------------------------------------------------

CLIENTMSG_PROC( _32, LoadServiceEx)( CTEXTSTR service, EventHandlerFunctionEx EventHandlerEx )
{
	return _LoadService( service, NULL, EventHandlerEx, NULL, 0 );
}

//--------------------------------------------------------------------

CLIENTMSG_PROC( _32, LoadServiceExx)( CTEXTSTR service, EventHandlerFunctionExx EventHandlerEx, PTRSZVAL psv )
{
	return _LoadService( service, NULL, NULL, EventHandlerEx, psv );
}

//--------------------------------------------------------------------

#undef RegisterServiceEx
CLIENTMSG_PROC( LOGICAL, RegisterServiceEx )( TEXTCHAR *name
														  , server_function_table functions
														  , int entries
														  , server_message_handler handler
														)
{
   return RegisterServiceExx( name, functions, entries, handler, NULL, 0 );
}

CLIENTMSG_PROC( LOGICAL, RegisterServiceExx )( TEXTCHAR *name
															, server_function_table functions
															, int entries
															, server_message_handler handler
															, server_message_handler_ex handler_ex
															, PTRSZVAL psv
															)
{
	if( !name )
		g.flags.bMasterServer = 1;
	if( !_InitMessageService( FALSE ) )
	{
		lprintf( WIDE("Initization of %s message service failed."), name );
		return 0;
	}
	else
	{
		//static int nBaseMsg;
		PCLIENT_SERVICE pService = (PCLIENT_SERVICE)Allocate( sizeof( CLIENT_SERVICE ) );
		pService->recv = (PQMSG)Allocate( MSG_DEFAULT_RESULT_BUFFER_MAX );
		pService->result = (PQMSG)Allocate( MSG_DEFAULT_RESULT_BUFFER_MAX );
		pService->flags.bRegistered = 0;
		pService->flags.bFailed = 0;
		pService->flags.connected = 0;
		pService->flags.bClosed = 0;
		pService->flags.bWaitingInReceive = 0;

		// setup service message base.
		if( !g.nMsgCounter )
			g.nMsgCounter = LOWEST_BASE_MESSAGE;
		if( !name )
		{
			pService->flags.bMasterServer = 1;
			pService->first_message_index = 0; // except the first 4 messages are slightly different than a generic service.
			pService->name = StrDup( WIDE("Master Server") );
		}
		else
		{
			pService->flags.bMasterServer = 0;
			pService->first_message_index = g.nMsgCounter;
#ifdef DEBUG_MESSAGE_BASE_ID
			lprintf( WIDE("Next message base is %") _32f WIDE(""), g.nMsgCounter );
#endif
			g.nMsgCounter += entries + 32;
#ifdef DEBUG_MESSAGE_BASE_ID
			lprintf( WIDE("Next message base is %") _32f WIDE(""), g.nMsgCounter );
#endif
			pService->name = StrDup( name );
		}
		pService->handler_ex = handler_ex;
		pService->handler = handler;
		pService->functions = functions;
		pService->entries = entries?entries:256;
		pService->references = 0;

		if( !g.flags.bAliveThreadStarted )
		{
			// this timer monitors ALL clients for inactivity
			// it will probe them with RU_ALIVE messages
			// to which they must respond otherwise be termintated.
			g.flags.bAliveThreadStarted = 1;
			AddTimer( CLIENT_TIMEOUT/4, MonitorClientActive, 0 );
			// each service gets 1 thread to handle their own
			// messages... services do not have 'events' generated
			// to them.
		}
		if( !g.flags.bServiceHandlerStarted )
		{
			ThreadTo( HandleServiceMessages, (PTRSZVAL)pService );
			g.flags.bServiceHandlerStarted = 1;
		}
		else
		{
			pService->thread = g.pMessageThread;
			DoRegisterService( pService );
		}
		while( !pService->flags.bRegistered )
			Relinquish();

		if( pService->flags.bFailed )
		{
			pService->flags.bClosed = 1;
			pService->recv->targetid = INVALID_INDEX;
			while( pService->flags.bWaitingInReceive )
			{
				pService->recv->targetid = INVALID_INDEX;
				if( pService->thread )
					WakeThread( pService->thread );
				else
					break;
				//Relinquish();
			}
			while( pService->thread )
			{
				Relinquish();
				WakeThread( pService->thread );
			}
			Release( pService->name );
			Release( pService );
			return FALSE;
		}
		if( pService )
		{
			LinkThing( g.services, pService );
			return pService->first_message_index;
		}
	}
	return 0;
}
	//--------------------------------------------------------------------
#undef RegisterService
CLIENTMSG_PROC( LOGICAL, RegisterService )( TEXTCHAR *name
														  , server_function_table functions
														  , int entries
														)
{
	return RegisterServiceEx( name, functions, entries, NULL );
}

	//--------------------------------------------------------------------

CLIENTMSG_PROC( void, UnloadService )( _32 MyMsgBase )
{
	PEVENTHANDLER pHandler;
	pHandler = g.pHandlers;
	while( pHandler )
	{
		if( pHandler->MyMsgBase == MyMsgBase )
			break;
		pHandler = pHandler->next;
	}
	if( pHandler )
	{
		_32 Responce;
		//lprintf( WIDE("Unload service: %s"), pHandler->servicename );
		if( pHandler->flags.local_service )
		{
			//lprintf( WIDE("Local service... resulting quick success...") );
			Responce = (MSG_ServiceUnload+pHandler->MsgBase)|SERVER_SUCCESS;
		}
		else
		{
			//lprintf( WIDE("Requesting message %d from %d "), MSG_ServiceUnload , pHandler->MsgBase );
			Responce = ((MSG_ServiceUnload+pHandler->MsgBase)|SERVER_SUCCESS);
			if( !TransactServerMessage( MSG_ServiceUnload + pHandler->MyMsgBase
											  , &pHandler->MsgBase
											  , sizeof( pHandler->MsgBase ) // include NUL
											  , &Responce/*NULL*/, NULL, NULL ) )
			{
				lprintf( WIDE("Transaction to ServiceUnload failed...") );
			}
			else if( Responce != ((MSG_ServiceUnload+pHandler->MsgBase)|SERVER_SUCCESS) )
			{
				lprintf( WIDE("Server reports it failed to unload the service %08") _32fx WIDE(" %08") _32fx WIDE("")
						 , Responce, ((MSG_ServiceUnload+pHandler->MsgBase)|SERVER_SUCCESS) );
			// no matter what the result, this must still release this
			// resource....
			//return;
			}
			while( pHandler->flags.dispatched )
			{
				Relinquish();
			}
		}

		UnlinkThing( pHandler );

		//lprintf( WIDE("Release? wow release hangs forever?") );
		//Release( pHandler );
		if( 0 && !g.pHandlers )
		{
			Log( WIDE("No more services loaded - killing threads, disconnecting") );
			if( g.pLocalEventThread )
			{
				EndThread( g.pLocalEventThread );
				// wake up the thread...
			}
			if( g.pEventThread )
				EndThread( g.pEventThread );
			if( g.pThread )
				EndThread( g.pThread );

			CloseMessageQueues();
			g.flags.events_ready = 0;
			g.flags.local_events_ready = 0;
			g.flags.failed = 0;
			g.flags.message_handler_ready = 0;
			g.flags.message_responce_handler_ready = 0;
		}
		//Log( WIDE("Done unloading services...") );
		return;
	}
	Log( WIDE("Service was already Unloaded!?!?!?!?!?") );
}

//-------------------------------------------------------------

CLIENTMSG_PROC( int, SendOutMessageEx )( PQMSG buffer, int len DBG_PASS )
{
#ifdef DEBUG_MESSAGE_BASE_ID
	//DBG_VARSRC;
#endif
	int stat;
	if( ( stat = msgsnd( g.msgq_out, MSGTYPE (buffer), len, 0 ) ) < 0 )
	{
		lprintf( WIDE("Error sending message: %s"), strerror(errno) );
	}
	return stat;
}
CLIENTMSG_PROC( int, SendOutMessage )( PQMSG buffer, int len )
{
	return SendOutMessageEx(buffer,len DBG_SRC);
}

//-------------------------------------------------------------

CLIENTMSG_PROC( void, SetMasterServer )( void )
{
	g.flags.bMasterServer = 1;
}

//-------------------------------------------------------------

CLIENTMSG_PROC( void, DumpServiceList )(void )
{
	PLIST list = NULL;
	int bDone = 0;
	PREFIX_PACKED struct {
		PLIST *ppList;
		int *pbDone;
		PTHREAD me;
	} PACKED mydata;
	mydata.ppList = &list;
	mydata.pbDone = &bDone;
	if( !_InitMessageService( FALSE ) )
	{
		lprintf( WIDE("Initization of public message participation failed, cannot query service master") );
		return;
	}
	mydata.me = MakeThread();
	lprintf( WIDE("Sending message to server ... list services...") );
	LogBinary( (P_8)&mydata, sizeof(mydata) );
	SendServerMessage( CLIENT_LIST_SERVICES, &mydata, sizeof(mydata) );
	// wait for end of list...
	while( !bDone )
	{
		WakeableSleep( 5000 );
		if( !bDone )
		{
			lprintf( WIDE("Treading water, but I think I'm stuck here forever...") );
		}
	}
	{
		INDEX idx;
		char *service;
		LIST_FORALL( list, idx, char *, service )
		{
			// ID of service MAY be available... but is not yet through
			// thsi interface...
			lprintf( WIDE("Available Service: %s"), service );
			Release( service );
		}
		lprintf( WIDE("End of service list.") );
		DeleteList( &list );
	}
}

//-------------------------------------------------------------

CLIENTMSG_PROC( void, GetServiceList )( PLIST *list )
{
	int bDone = 0;
	PREFIX_PACKED struct {
		PLIST *ppList;
		int *pbDone;
		PTHREAD me;
	} PACKED mydata;
	mydata.ppList = list;
	mydata.pbDone = &bDone;
	if( !_InitMessageService( FALSE ) )
	{
		lprintf( WIDE("Initization of public message participation failed, cannot query service master") );
		return;
	}
	mydata.me = MakeThread();
	lprintf( WIDE("Sending message to server ... list services...") );
	SendServerMessage( CLIENT_LIST_SERVICES, &mydata, sizeof(mydata) );
	// wait for end of list...
	while( !bDone )
	{
		WakeableSleep( 5000 );
		if( !bDone )
		{
			lprintf( WIDE("Treading water, but I think I'm stuck here forever...") );
		}
	}
	/*
	{
		INDEX idx;
		char *service;
		LIST_FORALL( list, idx, char *, service )
		{
			// ID of service MAY be available... but is not yet through
			// thsi interface...
			lprintf( WIDE("Available Service: %s"), service );
		}
		lprintf( WIDE("End of service list.") );
		}
		*/
}

MSGCLIENT_NAMESPACE_END

//-------------------------------------------------------------
