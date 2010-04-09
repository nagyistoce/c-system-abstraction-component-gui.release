#ifndef MESSAGE_SERVICE_PROTOCOL
#define MESSAGE_SERVICE_PROTOCOL

#include <stddef.h>
#ifdef __cplusplus
using namespace sack;
#endif

#ifdef __cplusplus
#define _MSG_NAMESPACE  namespace msg {
#define _PROTOCOL_NAMESPACE namespace protocol {
#define MSGPROTOCOL_NAMESPACE namespace sack { _MSG_NAMESPACE _PROTOCOL_NAMESPACE
#define MSGPROTOCOL_NAMESPACE_END }} }
#else
#define _MSG_NAMESPACE
#define _PROTOCOL_NAMESPACE 
#define MSGPROTOCOL_NAMESPACE
#define MSGPROTOCOL_NAMESPACE_END 
#endif

SACK_NAMESPACE
	/* This namespace contains an implmentation of inter process
	   communications using a set of message queues which result
	   from 'msgget' 'msgsnd' and 'msgrcv'. This are services
	   available under a linux kernel. Reimplemented a version to
	   service for windows. This is really a client/service
	   registration and message routing system, it is not the
	   message queue itself. See <link sack::containers::message, message>
	   for the queue implementation (again, under linux, does not
	   use this custom queue).
	   
	   
	   See Also
	   RegisterService
	   
	   LoadService                                                         */
	_MSG_NAMESPACE
/* Defines structures and methods for receiving and sending
	   messages. Also defines some utility macros for referencing
		message ID from a user interface structure.                */
	_PROTOCOL_NAMESPACE

#define MSGQ_ID_BASE WIDE("Srvr")

// this is a fun thing, in order to use it,
// undefine MyInterface, and define your own to your
// library's interface structure name (the tag of the structure)
#define MSG_ID(method)  ( ( offsetof( struct MyInterface, _##method ) / sizeof( void(*)(void) ) ) + BASE_MESSAGE_ID + MSG_EventUser )
#define MSG_OFFSET(method)  ( ( offsetof( struct MyInterface, _##method ) / sizeof( void(*)(void) ) ) + MSG_EventUser )
#define INTERFACE_METHOD(type,name) type (CPROC*_##name)

// this is the techincal type of SYSV IPC MSGQueues
#define MSGIDTYPE long

// this will determine the length of parameter list
// based on the first and last parameters.
#define ParamLength( first, last ) ( ((PTRSZVAL)((&(last))+1)) - ((PTRSZVAL)(&(first))) )

typedef PREFIX_PACKED struct buffer_len_tag {
	CPOINTER buffer;
	_32 len;
} PACKED BUFFER_LENGTH_PAIR;



// Dispach Pending - particularly display mouse event messages
//                   needed to be accumulated before being dispatched
//                   this event is generated when no more messages
//                   have been received.
#define MSG_EventDispatchPending   0
#define MSG_DispatchPending   MSG_EventDispatchPending

// these are event message definitions.
// server events come through their function table, clients
// register an event handler... these are low numbered since
// they are guaranteed from the client/server respectively.

// Mate ended - for the client, this means that the server
//              has become defunct.  For the server, this
//              means that a client is no longer present.
//              also issued when a client volentarily leaves
//              which in effect is the same as being discovered gone.
//    param[0] = Process ID of client disconnecting
//  result_length = INVALID_INDEX - NO RESULT DATA, PLEASE!
#define MSG_MateEnded         MSG_ServiceUnload
#define MSG_ServiceUnload     0
//#define MSG_ServiceClose    MSG_ServiceUnload
//#define MSG_ServiceUnload        MSG_MateEnded

// finally - needed to define a way for the service
// to actually know when a client connects... so that
// it may validate commands as being froma good source.
// also, a multiple service server may want this to know which
// service is being loaded.
//     params + 0 = text string of the service to load
//  on return result[1] is the number of messages this routine
//  expects.
//     result[0] is the number of events this service may generate
#define MSG_MateStarted      1
#define MSG_ServiceLoad      MSG_MateStarted

// Service is about to be unloaded - here's a final chance to
// cleanup before being yanked from existance.
// Last reference to the service is now gone, going to do the unload.
#define MSG_UndefinedMessage2      2

// no defined mesasage fo this
#define MSG_UndefinedMessage3       3
// Other messages may be filled in here...

// skip a couple messages so we don't have to recompile everything
// very soon...
#define MSG_EventUser       MSG_UserServiceMessages
#define MSG_UserServiceMessages 4

enum server_event_messages {
	// these messages are sent to client's event channel
	// within the space of core service requests (0-256?)
	// it's on top of client event user - cause the library
	// may also receive client_disconnect/connect messages
   //
	MSG_SERVICE_DATA = MSG_EventUser
      , MSG_SERVICE_NOMORE // end of list - zero or more MSG_SERVICE_DATA mesasges will preceed this.
};

enum server_failure_messages {
	CLIENT_UNKNOWN
, MESSAGE_UNKNOWN
, MESSAGE_INVALID // sending server(sourced) messages to server
, SERVICE_UNKNOWN // could not find a service for the message.
, UNABLE_TO_LOAD
};

enum service_messages {
  INVALID_MESSAGE  = 0 // no message ID 0 ever.
, SERVER_FAILURE   = 0x80000000 // server responce to clients - failure
      // failure may result for the above reasons.
, SERVER_SUCCESS   = 0x40000000 // server responce to clients - success
, SERVER_NEED_TIME = 0x20000000 // server needs more time to complete...
, CLIENT_LOAD_SERVICE = 1 // client requests a service (load by name)
, CLIENT_UNLOAD_SERVICE // client no longer needs a service (unload msgbase)
, CLIENT_CONNECT       // new client wants to connect
, CLIENT_DISCONNECT    // client disconnects (no responce)
							 , RU_ALIVE             // server/client message to other requesting status
							 , IM_ALIVE             // server/client message to other responding status
							 , CLIENT_REGISTER_SERVICE // client register service (name, serivces, callback table.)
                      , CLIENT_LIST_SERVICES // client requests a list of services (optional param partial filter?)
                      , IM_TARDY   // Service needs more time, and passes back a millisecond delay-reset
};

#define LOWEST_BASE_MESSAGE 0x100

// server functions will return TRUE if no failure
// server functions will return FALSE on failure
// FAILURE or SUCCESS will be returned to the client,
//   along with any result data set.
// native mode (unspecified... one would assume
// stack passing, but the world is bizarre and these are
// probably passed by registers.

typedef int (CPROC *server_message_handler)( _32 SourceRouteID, _32 MsgID
														 , _32 *params, _32 param_length
														 , _32 *result, _32 *result_length );
typedef int (CPROC *server_message_handler_ex)( PTRSZVAL psv
															 , _32 SourceRouteID, _32 MsgID
															 , _32 *params, _32 param_length
															 , _32 *result, _32 *result_length );
// params[-1] == Source Process ID
// params[-2] == Source Route ID
typedef int (CPROC *server_function)( _32 *params, _32 param_length
										 , _32 *result, _32 *result_length );

typedef struct server_function_entry_tag{
#ifdef _DEBUG
   CTEXTSTR name;
#endif
   server_function function;
} SERVER_FUNCTION;

#ifdef _DEBUG
#define ServerFunctionEntry(name) { #name, name }
#else
#define ServerFunctionEntry(name) { name }
#endif

typedef SERVER_FUNCTION *server_function_table;

// MsgID will be < MSG_EventUser if it's a system message...
// MsgID will be service msgBase + Remote ID...
//    so the remote needs to specify a unique base... so ...
//    entries must still be used...
typedef int (CPROC*EventHandlerFunction)(_32 MsgID, _32*params, _32 paramlen);
typedef int (CPROC*EventHandlerFunctionEx)(_32 SourceID, _32 MsgID, _32*params, _32 paramlen);
typedef int (CPROC*EventHandlerFunctionExx)( PTRSZVAL psv, _32 SourceID, _32 MsgID, _32*params, _32 paramlen);

// result of EventHandlerFunction shall be one fo the following values...
//   EVENT_HANDLED
// 0 - no futher action required
//   EVENT_WAIT_DISPATCH
// 1 - when no further events are available, please send event_dispatched.
//     this Event was handled by an internal queuing for later processing.
enum EventResult {
 EVENT_HANDLED = 0,
 EVENT_WAIT_DISPATCH = 1
};

MSGPROTOCOL_NAMESPACE_END
#ifdef __cplusplus
using namespace sack::msg::protocol;
#endif

#endif
// $Log: msgprotocol.h,v $
// Revision 1.14  2005/06/30 18:32:41  jimq
// Define extended event handler type.
//
// Revision 1.12  2005/06/30 13:22:44  d3x0r
// Attempt to define preload, atexit methods for msvc.  Fix deadstart loading to be more protected.
//
// Revision 1.11  2005/06/20 09:41:59  d3x0r
// define some event handler result codes
//
// Revision 1.10  2005/05/25 16:50:09  d3x0r
// Synch with working repository.
//
// Revision 1.12  2005/05/23 21:58:29  jim
// Restore prior old dispatch event ID
//
// Revision 1.11  2005/05/23 19:29:11  jim
// Improved registeration of services to allow a specification of a message-proc style handler.... Also worked on straightening out MSG_ and CLIENT_ message IDs...
//
// Revision 1.10  2005/05/12 21:12:50  jim
// Merge process_service_branch into trunk development.
//
// Revision 1.9.2.4  2005/05/23 19:26:10  jim
// Improved registeration of services to allow a specification of a message-proc style handler.... Also worked on straightening out MSG_ and CLIENT_ message IDs...
//
// Revision 1.9.2.3  2005/05/12 21:01:58  jim
// Fix prototype of server function to be CPROC
//
// Revision 1.9.2.2  2005/05/06 23:01:04  jim
// Don't redundantly define getpid_returns_ppid - since it always does now - SUCK\!
//
// Revision 1.9.2.1  2005/05/02 17:01:08  jim
// Nearly works... time to move over to linux... still need some cleanup on exits... and dead clients.
//
// Revision 1.9  2004/06/21 07:47:36  d3x0r
// Checkpoint - make system rouding out nicly.
//
// Revision 1.8  2003/09/21 11:49:04  panther
// Fix service refernce counting for unload.  Fix Making sub images hidden.
// Fix Linking of services on server side....
// cleanup some comments...
//
// Revision 1.7  2003/09/19 14:52:40  panther
// Added new procedure name registry
//
// Revision 1.6  2003/03/27 15:36:38  panther
// Changes were done to limit client messages to server - but all SERVER-CLIENT messages were filtered too... Define LOWEST_BASE_MESSAGE
//
// Revision 1.5  2003/03/25 08:38:11  panther
// Add logging
//
