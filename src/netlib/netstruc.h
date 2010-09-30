#include <stdhdrs.h>
#include <sack_types.h>
#include <loadsock.h>
#include <sharemem.h> // critical section
#include <timers.h>
#include <network.h>


// debugging flag for socket creation/closing
//#define LOG_SOCKET_CREATION

// there were some messages regarding the close sequence of sockets
// they were left open... so developers might track why sockets were closing...
// these should be probably be re-enabled and be controlled with a runtime option flag.
//#define LOG_DEBUG_CLOSING

// started using this symbol more in the later days of disabling logging...
//#define VERBOSE_DEBUG
//#define LOG_STARTUP_SHUTDOWN
// Define this symbol to use Log( ghLog, WIDE("") ) to log pending
// write status...
//#define LOG_PENDING
// for windows - this will log all FD_XXXX notifications processed...
//#define LOG_NOTICES
//#define LOG_CLIENT_LISTS

//TODO: modify the client struct to contain the MAC addr
#ifndef __LINUX__
#define USE_WSA_EVENTS
#endif

#ifndef CLIENT_DEFINED
#define CLIENT_DEFINED

SACK_NETWORK_NAMESPACE

#define MAGIC_SOCKADDR_LENGTH sizeof(SOCKADDR_IN)< 256?256:sizeof( SOCKADDR_IN)

// this might have to be like sock_addr_len_t
#define SOCKADDR_LENGTH(sa) ( (int)*(PTRSZVAL*)( ( (PTRSZVAL)(sa) ) - sizeof(PTRSZVAL) ) )
#define SET_SOCKADDR_LENGTH(sa,size) ( ( *(PTRSZVAL*)( ( (PTRSZVAL)(sa) ) - sizeof(PTRSZVAL) ) ) = size )

// used by the network thread dispatched network layer messages...
#define SOCKMSG_UDP (WM_USER+1)  // messages for UDP use this window Message
#define SOCKMSG_TCP (WM_USER+2)  // Messages for TCP use this Window Message
#define SOCKMSG_CLOSE (WM_USER+3) // Message for Network layer shutdown.

// not sure if this is used anywhere....
#define HOSTNAME_LEN 50      // maximum length of a host's text name...

typedef struct PendingBuffer
{
   _32 dwAvail;                // number of bytes to be read yet
   _32 dwUsed;                 // Number of bytes already read.
   _32 dwLastRead;             // number of bytes received on last read.
   struct {
      int  bStream:1;    // is a stream request...
      int  bDynBuffer:1; // lpBuffer was malloced...
	}s;
	union {
		CPOINTER c;              // Buffer Pointer.
		POINTER p;
	} buffer;
   struct PendingBuffer *lpNext; // Next Pending Message to be handled
}PendingBuffer;

#define CF_UDP           0x0001
#define CF_TCP           0x0000  // no flag... is NOT UDP....

#define CF_LISTEN        0x0002
#define CF_CONNECT       0x0000

#define CF_WRITEPENDING  0x0004 // some write is left hanging to output

#define CF_READPENDING   0x0008 // set if buffers have been set by a read
#define CF_READREADY     0x0010 // set if next read to pend should recv also
#define CF_READWAITING   0x8000 // set if reading application is waiting in-line for result.

#define CF_CONNECTED     0x0020 // set when FD_CONNECT is issued...
#define CF_CONNECTERROR  0x0040
#define CF_CONNECTING    0x0080

#define CF_TOCLOSE       0x0100

#define CF_WRITEISPENDED 0x0200

#define CF_CLOSING       0x0400
#define CF_DRAINING      0x0800

#define CF_CLOSED        0x1000 // closed, handled everything except releasing the socket.
#define CF_ACTIVE        0x2000
#define CF_AVAILABLE     0x4000


#define CF_CPPCONNECT       0x010000
// server/client is implied in usage....
// much like Read, ReadEX are implied in TCP/UDP usage...
//#define CF_CPPSERVERCONNECT 0x010000
//#define CF_CPPCLIENTCONNECT 0x020000
#define CF_CPPREAD          0x020000
#define CF_CPPCLOSE         0x040000
#define CF_CPPWRITE         0x080000
#define CF_CALLBACKTYPES ( CF_CPPCONNECT|CF_CPPREAD|CF_CPPCLOSE|CF_CPPWRITE )
#define CF_STATEFLAGS (CF_ACTIVE|CF_AVAILABLE|CF_CLOSED)

#define CF_WANTS_GLOBAL_LOCK 0x10000000

struct NetworkClient
{
	SOCKADDR *saClient;  //Dest Address
	SOCKADDR *saSource;  //Local Address of this port ...
	SOCKADDR *saLastClient; // use this for UDP recvfrom
	_8     hwClient[6];
	_8     hwSource[6];
	//  ServeUDP( WIDE("SourceIP"), SourcePort );
	// 		saSource w/ no Dest - read is a connect...
	//  ConnectUDP( WIDE("DestIP"), DestPort );
	//     saClient is DestIP
	//		saSource and implied source...
	//     USE TCP to locate MY Address?
	//     bind(UDP) results in?
	//     connect(UDP) results in?

	SOCKET      Socket;
	_32         dwFlags; // CF_
	_8        *lpUserData;

	union {
		void (CPROC*ClientConnected)( struct NetworkClient *old, struct NetworkClient *newclient ); // new incoming client.
		void (CPROC*ThisConnected)(struct NetworkClient *me, int nStatus );
		void (CPROC*CPPClientConnected)( PTRSZVAL psv, struct NetworkClient *newclient ); // new incoming client.
		void (CPROC*CPPThisConnected)( PTRSZVAL psv, int nStatus );
	}connect;
	PTRSZVAL psvConnect;
	union {
		void (CPROC*CloseCallback)(struct NetworkClient *);
		void (CPROC*CPPCloseCallback)(PTRSZVAL psv);
	} close;
	PTRSZVAL psvClose;
	union {
		void (CPROC*ReadComplete)(struct NetworkClient *, POINTER, int );
		void (CPROC*CPPReadComplete)(PTRSZVAL psv, POINTER, int );
		void (CPROC*ReadCompleteEx)(struct NetworkClient *, POINTER, int, SOCKADDR *);
		void (CPROC*CPPReadCompleteEx)(PTRSZVAL psv, POINTER, int, SOCKADDR *);
	}read;
	PTRSZVAL psvRead;
	union {
		void (CPROC*WriteComplete)( struct NetworkClient * );
		void (CPROC*CPPWriteComplete)( PTRSZVAL psv );
	}write;
	PTRSZVAL psvWrite;

	LOGICAL 	      bWriteComplete; // set during bWriteComplete Notify...

	LOGICAL        bDraining;    // byte sink functions.... JAB:980202
	LOGICAL        bDrainExact;  // length does not matter - read until one empty read.
	int            nDrainLength;
#if defined( USE_WSA_EVENTS )
   WSAEVENT event;
#endif
	CRITICALSECTION csLock;      // per client lock.
	PTHREAD pWaiting; // Thread which is waiting for a result...
	PendingBuffer RecvPending, FirstWritePending; // current incoming buffer
	PendingBuffer *lpFirstPending,*lpLastPending; // outgoing buffers
	_32    LastEvent; // GetTickCount() of last event...
   DeclareLink( struct NetworkClient );
};
typedef struct NetworkClient CLIENT;

#ifdef MAIN_PROGRAM
#define LOCATION
#else
#define LOCATION extern
#endif

//LOCATION CRITICALSECTION csNetwork;

#define MAX_NETCLIENTS  g.nMaxClients

typedef struct client_slab_tag {
	_32 count;
   CLIENT client[1];
} CLIENT_SLAB, *PCLIENT_SLAB;

// global network data goes here...
LOCATION struct network_global_data{
   int     nMaxClients;
	int     nUserData;     // number of longs.
   P_8     pUserData;
   PLIST   ClientSlabs;
   LOGICAL bLog;
   LOGICAL bQuit;
   PTHREAD pThread;
	PCLIENT AvailableClients;
	PCLIENT ActiveClients;
	PCLIENT ClosedClients;
	CRITICALSECTION csNetwork;
	_32 uNetworkPauseTimer;
   _32 uPendingTimer;
#ifndef __LINUX__
	HWND ghWndNetwork;
#endif
	CTEXTSTR system_name;
#ifdef WIN32
   int nProtos;
	WSAPROTOCOL_INFO *pProtos;

	INDEX tcp_protocol;
	INDEX udp_protocol;
	INDEX tcp_protocolv6;
	INDEX udp_protocolv6;
#endif
#if defined( USE_WSA_EVENTS )
   WSAEVENT event;
#endif
	_32 dwReadTimeout;
	_32 dwConnectTimeout;
   PLIST addresses;
}
*global_network_data; // aka 'g'

#define g (*global_network_data)

#ifdef _WIN32
#ifndef errno
#define errno WSAGetLastError()
#endif
#else
#endif

//---------------------------------------------------------------------
// routines exported from the core for use in external modules
PCLIENT GetFreeNetworkClientEx( DBG_VOIDPASS );
#define GetFreeNetworkClient() GetFreeNetworkClientEx( DBG_VOIDSRC )

_UDP_NAMESPACE
int FinishUDPRead( PCLIENT pc );
_UDP_NAMESPACE_END

#ifdef __WINDOWS__
	// errors started arrising because of faulty driver stacks.
	// spontaneous 10106 errors in socket require migration to winsock2.
   // socket is opened specifically by protocol descriptor...
SOCKET OpenSocket( LOGICAL v4, LOGICAL bStream, LOGICAL bRaw );
int SystemCheck( void );
#endif


void TerminateClosedClientEx( PCLIENT pc DBG_PASS );
#define TerminateClosedClient(pc) TerminateClosedClientEx(pc DBG_SRC)
void InternalRemoveClientExx(PCLIENT lpClient, LOGICAL bBlockNofity, LOGICAL bLinger DBG_PASS );
#define InternalRemoveClientEx(c,b,l) InternalRemoveClientExx(c,b,l DBG_SRC)
#define InternalRemoveClient(c) InternalRemoveClientEx(c, FALSE, FALSE )

SOCKADDR *AllocAddr( void );

#define CLIENT_DEFINED

#define IsValid(S)   ((S)!=INVALID_SOCKET)   //spv:980303
#define IsInvalid(S) ((S)==INVALID_SOCKET)   //spv:980303

SACK_NETWORK_NAMESPACE_END

#endif

// $Log: netstruc.h,v $
// Revision 1.19  2005/05/23 17:27:02  jim
// Add some flags and data to support wait mode read on TCP sockets.
//
// Revision 1.18  2004/08/18 23:53:07  d3x0r
// Cleanups - also enhanced network init to expand if called with larger params.
//
// Revision 1.17  2003/09/25 00:21:21  panther
// Move cpp wrapper functions into network library
//
// Revision 1.16  2003/09/24 22:32:54  panther
// Update for latest type procs...
//
// Revision 1.15  2003/09/24 15:10:54  panther
// Much mangling to extend C++ network interface...
//
// Revision 1.14  2003/07/24 22:50:10  panther
// Updates to make watcom happier
//
// Revision 1.13  2003/03/25 08:45:55  panther
// Added CVS logging tag
//
