#ifndef NETWORK_HEADER_INCLUDED
#define NETWORK_HEADER_INCLUDED
#include "sack_types.h"
#include "loadsock.h"

#if !defined(__STATIC__) && !defined( __UNIX__ )
#ifdef NETWORK_SOURCE
#define NETWORK_PROC(type,name) EXPORT_METHOD type name
#else
#define NETWORK_PROC(type,name) IMPORT_METHOD type name
#endif
#else
#ifdef NETWORK_LIBRARY_SOURCE
#define NETWORK_PROC(type,name) type name
#else
#define NETWORK_PROC(type,name) extern type name
#endif
#endif

#ifdef __cplusplus
#define _NETWORK_NAMESPACE  namespace network {
#define _NETWORK_NAMESPACE_END }
#define _TCP_NAMESPACE  namespace tcp {
#define _TCP_NAMESPACE_END }
#define USE_TCP_NAMESPACE using namespace tcp;
#define _UDP_NAMESPACE  namespace udp {
#define _UDP_NAMESPACE_END }
#define USE_UDP_NAMESPACE using namespace tcp;
#else
#define _NETWORK_NAMESPACE  
#define _NETWORK_NAMESPACE_END
#define _TCP_NAMESPACE  
#define _TCP_NAMESPACE_END
#define _UDP_NAMESPACE  
#define _UDP_NAMESPACE_END
#define USE_TCP_NAMESPACE
#define USE_UDP_NAMESPACE

#endif

#define SACK_NETWORK_NAMESPACE  SACK_NAMESPACE _NETWORK_NAMESPACE
#define SACK_NETWORK_NAMESPACE_END _NETWORK_NAMESPACE_END SACK_NAMESPACE_END
#define SACK_NETWORK_TCP_NAMESPACE  SACK_NAMESPACE _NETWORK_NAMESPACE _TCP_NAMESPACE
#define SACK_NETWORK_TCP_NAMESPACE_END _TCP_NAMESPACE_END _NETWORK_NAMESPACE_END SACK_NAMESPACE_END
#define SACK_NETWORK_UDP_NAMESPACE  SACK_NAMESPACE _NETWORK_NAMESPACE _UDP_NAMESPACE
#define SACK_NETWORK_UDP_NAMESPACE_END _UDP_NAMESPACE_END _NETWORK_NAMESPACE_END SACK_NAMESPACE_END

SACK_NETWORK_NAMESPACE

//#ifndef CLIENT_DEFINED
typedef struct NetworkClient *PCLIENT;
//typedef struct Client
//{
//   unsigned char Private_Structure_information_here;
//}CLIENT, *PCLIENT;
//#endif


NETWORK_PROC( CTEXTSTR, GetSystemName )( void );

#define NETWORK_MESSAGE (WM_USER+512)
enum {
PING_FAILED,
PING_SENT,
NETWORK_STARTED,
CONNECTION_DELETED,
CONNECTION_ADDED,
CLIENT_CONNECTED,
};

//NETWORK_PROC( int, NetworkStartup )( void );
// results true if locked
// results false if not open when the lock was done.
NETWORK_PROC( PCLIENT, NetworkLockEx )( PCLIENT pc DBG_PASS );
NETWORK_PROC( void, NetworkUnlockEx )( PCLIENT pc DBG_PASS );
NETWORK_PROC( PCLIENT, NetworkLock )( PCLIENT pc );
NETWORK_PROC( void, NetworkUnlock )( PCLIENT pc );
#define NetworkLock(pc) NetworkLockEx( pc DBG_SRC )
#define NetworkUnlock(pc) NetworkUnlockEx( pc DBG_SRC )

typedef void (CPROC*cReadComplete)(PCLIENT, POINTER, int );
typedef void (CPROC*cReadCompleteEx)(PCLIENT, POINTER, int, SOCKADDR * );
typedef void (CPROC*cCloseCallback)(PCLIENT);
typedef void (CPROC*cWriteComplete)(PCLIENT );
typedef void (CPROC*cNotifyCallback)(PCLIENT server, PCLIENT newClient);
typedef void (CPROC*cConnectCallback)(PCLIENT, int);
typedef void (CPROC*cppReadComplete)(PTRSZVAL, POINTER, int );
typedef void (CPROC*cppReadCompleteEx)(PTRSZVAL,POINTER, int, SOCKADDR * );
typedef void (CPROC*cppCloseCallback)(PTRSZVAL);
typedef void (CPROC*cppWriteComplete)(PTRSZVAL );
typedef void (CPROC*cppNotifyCallback)(PCLIENT pListener, PCLIENT newClient);
typedef void (CPROC*cppConnectCallback)(PTRSZVAL, int);

NETWORK_PROC( void, SetNetworkWriteComplete )( PCLIENT, cWriteComplete );
#ifdef __cplusplus
NETWORK_PROC( void, SetCPPNetworkWriteComplete )( PCLIENT, cppWriteComplete, PTRSZVAL );
#endif
#define SetWriteCallback SetNetworkWriteComplete
NETWORK_PROC( void, SetNetworkReadComplete )( PCLIENT, cReadComplete );
#ifdef __cplusplus
NETWORK_PROC( void, SetCPPNetworkReadComplete )( PCLIENT, cppReadComplete, PTRSZVAL );
#endif
#define SetReadCallback SetNetworkReadComplete
NETWORK_PROC( void, SetNetworkCloseCallback )( PCLIENT, cCloseCallback );
#ifdef __cplusplus
NETWORK_PROC( void, SetCPPNetworkCloseCallback )( PCLIENT, cppCloseCallback, PTRSZVAL );
#endif
#define SetCloseCallback SetNetworkCloseCallback

 // wwords is BYTES and wClients=16 is defaulted to 16
#ifdef __LINUX__
NETWORK_PROC( LOGICAL, NetworkWait )(POINTER unused,_16 wClients,int wUserData);
#else
NETWORK_PROC( LOGICAL, NetworkWait )(HWND hWndNotify,_16 wClients,int wUserData);
#endif
#define NetworkStart() NetworkWait( NULL, 0, 0 )
NETWORK_PROC( LOGICAL, NetworkAlive )( void ); // returns true if network layer still active...
NETWORK_PROC( int, NetworkQuit )(void);
// preferred method is to call Idle(); if in doubt.
//NETWORK_PROC( int, ProcessNetworkMessages )( void );

// dwIP would be for 1.2.3.4  (0x01020304 - memory 04 03 02 01) - host order
// VERY RARE!
NETWORK_PROC( SOCKADDR *, CreateAddress_hton )( _32 dwIP,_16 nHisPort);
// dwIP would be for 1.2.3.4  (0x04030201 - memory 01 02 03 04) - network order
#ifndef __WINDOWS__
NETWORK_PROC( SOCKADDR *, CreateUnixAddress )( CTEXTSTR path );
#endif
NETWORK_PROC( SOCKADDR *, CreateAddress )( _32 dwIP,_16 nHisPort);
NETWORK_PROC( SOCKADDR *, SetAddressPort )( SOCKADDR *pAddr, _16 nDefaultPort );
NETWORK_PROC( SOCKADDR *, SetNonDefaultPort )( SOCKADDR *pAddr, _16 nDefaultPort );
/*
 * this is the preferred method to create an address
 * name may be "* / *" with a slash, then the address result will be a unix socket (if supported)
 * name may have an options ":port" port number associated, if there is no port, then the default
 * port is used.
 *
 */
NETWORK_PROC( SOCKADDR *, CreateSockAddress )( CTEXTSTR name, _16 nDefaultPort );
NETWORK_PROC( SOCKADDR *, CreateRemote )(CTEXTSTR lpName,_16 nHisPort);
NETWORK_PROC( SOCKADDR *, CreateLocal )(_16 nMyPort);
NETWORK_PROC( int, GetAddressParts )( SOCKADDR *pAddr, _32 *pdwIP, _16 *pwPort );
NETWORK_PROC( void, ReleaseAddress )(SOCKADDR *lpsaAddr); // release a socket resource that has been created by an above routine

NETWORK_PROC( LOGICAL, CompareAddress )(SOCKADDR *sa1, SOCKADDR *sa2 ); // result with TRUE if equal, else FALSE
NETWORK_PROC( SOCKADDR *, DuplicateAddress )( SOCKADDR *pAddr ); // return a copy of this address...

_TCP_NAMESPACE
#ifdef __cplusplus
NETWORK_PROC( PCLIENT, CPPOpenTCPListenerAddrEx )( SOCKADDR *, cppNotifyCallback NotifyCallback, PTRSZVAL psvConnect );
#endif
NETWORK_PROC( PCLIENT, OpenTCPListenerAddrEx )( SOCKADDR *, cNotifyCallback NotifyCallback );
#define OpenTCPListenerAddr( pAddr ) OpenTCPListenerAddrEx( paddr, NULL );
#ifdef __cplusplus
NETWORK_PROC( PCLIENT, CPPOpenTCPListenerEx )( _16 wPort, cppNotifyCallback NotifyCallback, PTRSZVAL psvConnect );
#endif
NETWORK_PROC( PCLIENT, OpenTCPListenerEx )( _16 wPort, cNotifyCallback NotifyCallback );
#define OpenTCPListener( wPort )    OpenTCPListenerEx( wPort, NULL )

#define OpenTCPServer OpenTCPListener
#define OpenTCPServerEx OpenTCPListenerEx
#define OpenTCPServerAddr OpenTCPListenerAddr
#define OpenTCPServerAddrEx OpenTCPListenerAddrEx


#ifdef __cplusplus
NETWORK_PROC( PCLIENT, CPPOpenTCPClientAddrExx )(SOCKADDR *lpAddr, 
                         cppReadComplete  pReadComplete, PTRSZVAL, 
                         cppCloseCallback CloseCallback, PTRSZVAL, 
                         cppWriteComplete WriteComplete, PTRSZVAL, 
																 cppConnectCallback pConnectComplete,  PTRSZVAL );
#endif
NETWORK_PROC( PCLIENT, OpenTCPClientAddrExx )(SOCKADDR *lpAddr, 
                         cReadComplete  pReadComplete,
                         cCloseCallback CloseCallback,
                         cWriteComplete WriteComplete,
                         cConnectCallback pConnectComplete );
#ifdef __cplusplus
NETWORK_PROC( PCLIENT, CPPOpenTCPClientAddrEx )(SOCKADDR *
								, cppReadComplete, PTRSZVAL 
                        , cppCloseCallback, PTRSZVAL 
															  , cppWriteComplete, PTRSZVAL  );
#endif
NETWORK_PROC( PCLIENT, OpenTCPClientAddrEx )(SOCKADDR *, cReadComplete,
                         cCloseCallback, cWriteComplete );

#ifdef __cplusplus
NETWORK_PROC( PCLIENT, CPPOpenTCPClientExx )(CTEXTSTR lpName,_16 wPort
                         , cppReadComplete  pReadComplete, PTRSZVAL
                         , cppCloseCallback CloseCallback, PTRSZVAL
                         , cppWriteComplete WriteComplete, PTRSZVAL
														  , cppConnectCallback pConnectComplete, PTRSZVAL );
#endif
NETWORK_PROC( PCLIENT, OpenTCPClientExx )(CTEXTSTR lpName,_16 wPort,
                         cReadComplete  pReadComplete,
                         cCloseCallback CloseCallback,
                         cWriteComplete WriteComplete,
														cConnectCallback pConnectComplete );
#define OpenTCPClient( name, port, read ) OpenTCPClientExx(name,port,read,NULL,NULL,NULL)
NETWORK_PROC( PCLIENT, OpenTCPClientEx )( CTEXTSTR, _16, cReadComplete,
                         cCloseCallback, cWriteComplete );

NETWORK_PROC( LOGICAL, TCPDrainEx )( PCLIENT pClient, int nLength, int bExact );
#define TCPDrain(c,l) TCPDrainEx( (c), (l), TRUE )

// this is - (TRUE)disable NAGLE or (FALSE)enable NAGLE
// (TRUE)nodelay (FALSE)packet gather delay
NETWORK_PROC( void, SetTCPNoDelay )( PCLIENT pClient, int bEnable );
NETWORK_PROC( void, SetClientKeepAlive)( PCLIENT pClient, int bEnable );

// --------------------
NETWORK_PROC( int, doReadExx )(PCLIENT lpClient, POINTER lpBuffer, int nBytes
										, LOGICAL bIsStream, LOGICAL bWait );
NETWORK_PROC( int, doReadEx )(PCLIENT lpClient,POINTER lpBuffer,int nBytes, LOGICAL bIsStream);
#define ReadStream(pc,pBuf,nSize) doReadExx( pc, pBuf, nSize, TRUE, FALSE )
#define doRead(pc,pBuf,nSize)     doReadExx(pc, pBuf, nSize, FALSE, FALSE )
#define ReadTCP ReadStream 
#define ReadTCPMsg doRead
#define WaitReadTCP(pc,buf,nSize)    doReadExx(pc,buf, nSize, TRUE, TRUE )
#define WaitReadTCPMsg(pc,buf,nSize) doReadExx(pc,buf, nSize, FALSE, TRUE )


NETWORK_PROC( LOGICAL, doTCPWriteExx )( PCLIENT lpClient
						, CPOINTER pInBuffer
						, int nInLen, int bLongBuffer 
                                   , int failpending
                                   DBG_PASS
                                  );
#define doTCPWriteEx( c,b,l,f1,f2) doTCPWriteExx( (c),(b),(l),(f1),(f2) DBG_SRC )
//NETWORK_PROC( LOGICAL doTCPWrite(PCLIENT lpClient,POINTER lpBuffer,int nLen, int bLongBuffer);
#define SendTCPEx( c,b,l,p) doTCPWriteExx( c,b,l,FALSE,p DBG_SRC)
#define SendTCP(c,b,l) doTCPWriteExx(c,b,l, FALSE, FALSE DBG_SRC)
#define SendTCPLong(c,b,l) doTCPWriteExx(c,b,l, TRUE, FALSE DBG_SRC)
_TCP_NAMESPACE_END


NETWORK_PROC( void, SetNetworkLong )(PCLIENT lpClient,int nLong,PTRSZVAL dwValue);
NETWORK_PROC( void, SetNetworkInt )(PCLIENT lpClient,int nLong, int value);
NETWORK_PROC( void, SetNetworkWord )(PCLIENT lpClient,int nLong,_16 wValue);
NETWORK_PROC( PTRSZVAL, GetNetworkLong )(PCLIENT lpClient,int nLong);
NETWORK_PROC( _32, GetNetworkInt )(PCLIENT lpClient,int nLong);
NETWORK_PROC( _16, GetNetworkWord )(PCLIENT lpClient,int nLong);

#define GNL_IP     (-1)   
#define GNL_PORT   (-4)
#define GNL_MYIP   (-3)
#define GNL_MYPORT (-2) // prior GNL_PORT was my port...
#define GNL_MAC_LOW (-5)
#define GNL_MAC_HIGH (-6)

NETWORK_PROC( int, GetMacAddress)(PCLIENT pc );

NETWORK_PROC( void, RemoveClientExx )(PCLIENT lpClient, LOGICAL bBlockNofity, LOGICAL bLinger DBG_PASS );
#define RemoveClientEx(c,b,l) RemoveClientExx(c,b,l DBG_SRC)
#define RemoveClient(c) RemoveClientEx(c, FALSE, FALSE )


_UDP_NAMESPACE
NETWORK_PROC( PCLIENT, ServeUDP )( CTEXTSTR pAddr, _16 wPort,
                  cReadCompleteEx pReadComplete,
                  cCloseCallback Close);
NETWORK_PROC( PCLIENT, ServeUDPAddr )( SOCKADDR *pAddr, 
                     cReadCompleteEx pReadComplete,
                     cCloseCallback Close);


NETWORK_PROC( PCLIENT, ConnectUDP )( CTEXTSTR , _16 ,
                    CTEXTSTR, _16,
                    cReadCompleteEx,
                    cCloseCallback );
NETWORK_PROC( PCLIENT, ConnectUDPAddr )( SOCKADDR *sa, 
                        SOCKADDR *saTo, 
                    cReadCompleteEx pReadComplete,
                    cCloseCallback Close );
NETWORK_PROC( LOGICAL, ReconnectUDP )( PCLIENT pc, CTEXTSTR pToAddr, _16 wPort );
NETWORK_PROC( LOGICAL, GuaranteeAddr )( PCLIENT pc, SOCKADDR *sa );
NETWORK_PROC( void, UDPEnableBroadcast )( PCLIENT pc, int bEnable );

NETWORK_PROC( LOGICAL, SendUDPEx )( PCLIENT pc, CPOINTER pBuf, int nSize, SOCKADDR *sa );
#define SendUDP(pc,pbuf,size) SendUDPEx( pc, pbuf, size, NULL )
NETWORK_PROC( int, doUDPRead )( PCLIENT pc, POINTER lpBuffer, int nBytes );
#define ReadUDP doUDPRead

NETWORK_PROC( void, DumpAddrEx )( CTEXTSTR name, SOCKADDR *sa DBG_PASS );
#define DumpAddr(n,sa) DumpAddrEx(n,sa DBG_SRC )
_UDP_NAMESPACE_END
USE_UDP_NAMESPACE

//----- PING.C ------
NETWORK_PROC( LOGICAL, DoPing )( CTEXTSTR pstrHost,
             int maxTTL, 
             _32 dwTime, 
             int nCount, 
             TEXTSTR pResult, 
             LOGICAL bRDNS,
             void (*ResultCallback)( _32 dwIP, CTEXTSTR name, int min, int max, int avg, int drop, int hops ) );
NETWORK_PROC( LOGICAL, DoPingEx )( CTEXTSTR pstrHost,
             int maxTTL, 
             _32 dwTime, 
             int nCount, 
             TEXTSTR pResult, 
             LOGICAL bRDNS,
											 void (*ResultCallback)( PTRSZVAL psv, _32 dwIP, CTEXTSTR name, int min, int max, int avg, int drop, int hops )
											, PTRSZVAL psv );

//----- WHOIS.C -----
NETWORK_PROC( LOGICAL, DoWhois )( CTEXTSTR pHost, CTEXTSTR pServer, TEXTSTR pResult );

#ifdef __cplusplus

typedef class network *PNETWORK;
typedef class network
{
	PCLIENT pc;
	int TCP;
	static void CPROC WrapTCPReadComplete( PTRSZVAL psv, POINTER buffer, int nSize );
	static void CPROC WrapUDPReadComplete( PTRSZVAL psv, POINTER buffer, int nSize, SOCKADDR *sa );
	static void CPROC WrapWriteComplete( PTRSZVAL psv );
	static void CPROC WrapClientConnectComplete( PTRSZVAL psv, int nError );
	static void CPROC WrapServerConnectComplete( PTRSZVAL psv, PCLIENT pcNew );
	static void CPROC WrapCloseCallback( PTRSZVAL psv );
   // notify == server (listen)
	static void CPROC SetNotify( PCLIENT pc, cppNotifyCallback, PTRSZVAL psv );
   // connect == client (connect)
   static void CPROC SetConnect( PCLIENT pc, cppConnectCallback, PTRSZVAL psv );
   static void CPROC SetRead( PCLIENT pc, cppReadComplete, PTRSZVAL psv );
   static void CPROC SetWrite( PCLIENT pc, cppWriteComplete, PTRSZVAL psv );
   static void CPROC SetClose( PCLIENT pc, cppCloseCallback, PTRSZVAL psv );
public:
	network() { NetworkStart(); pc = NULL; TCP = TRUE; };
	network( PCLIENT pc ) { NetworkStart(); this->pc = pc; TCP = TRUE; };
	network( network &cp ) { cp.pc = pc; cp.TCP = TCP; };
	~network() { if( pc ) RemoveClientEx( pc, TRUE, FALSE ); pc = NULL; };
	inline void MakeUDP( void ) { TCP = FALSE; }

	virtual void ReadComplete( POINTER buffer, int nSize ) {}
	virtual void ReadComplete( POINTER buffer, int nSize, SOCKADDR *sa ) {}
	virtual void WriteComplete( void ) {}
	virtual void ConnectComplete( int nError ) {}
	// received on the server listen object...
	virtual void ConnectComplete( class network &pNewClient ) {}
	virtual void CloseCallback( void ) {}

	inline int Connect( SOCKADDR *sa )
	{
		if( !pc )
		pc = CPPOpenTCPClientAddrExx( sa 
									, WrapTCPReadComplete
									, (PTRSZVAL)this 
									, WrapCloseCallback
									, (PTRSZVAL)this 
									, WrapWriteComplete
									, (PTRSZVAL)this 
									, WrapClientConnectComplete 
									, (PTRSZVAL)this 
									);
		return (int)(pc!=NULL);
	};
	inline int Connect( CTEXTSTR name, _16 port )
	{
		if( !pc )
		pc = CPPOpenTCPClientExx( name, port
									, WrapTCPReadComplete
									, (PTRSZVAL)this 
									, WrapCloseCallback
									, (PTRSZVAL)this 
									, WrapWriteComplete
									, (PTRSZVAL)this 
									, WrapClientConnectComplete 
									, (PTRSZVAL)this 
									);
		return (int)(pc!=NULL);
	};
	inline int Listen( SOCKADDR *sa )
	{
		if( !pc )
		{
			if( ( pc = CPPOpenTCPListenerAddrEx( sa
				                        , (cppNotifyCallback)WrapServerConnectComplete
												, (PTRSZVAL)this 
														)  ) )
			{
				SetRead( pc, (cppReadComplete)WrapTCPReadComplete, (PTRSZVAL)this );
				SetWrite( pc, (cppWriteComplete)WrapWriteComplete, (PTRSZVAL)this );
				SetClose( pc, network::WrapCloseCallback, (PTRSZVAL)this );
			}
		}
		return (int)(pc!=NULL);
	};
	inline int Listen( _16 port )
	{
		if( !pc )
		{
			if( ( pc = CPPOpenTCPListenerEx( port
			                      , (cppNotifyCallback)WrapServerConnectComplete
											 , (PTRSZVAL)this ) ) )
			{
				SetRead( pc, (cppReadComplete)WrapTCPReadComplete, (PTRSZVAL)this );
				SetWrite( pc, (cppWriteComplete)WrapWriteComplete, (PTRSZVAL)this );
				SetClose( pc, network::WrapCloseCallback, (PTRSZVAL)this );
			}
		}
		return (int)(pc!=NULL);
	};
	inline void Write( POINTER p, int size )
	{
		if( pc ) SendTCP( pc, p, size );
	};
	inline void WriteLong( POINTER p, int size )
	{
		if( pc ) SendTCPLong( pc, p, size );
	};
	inline void Read( POINTER p, int size )
	{
		if( pc ) ReadTCP( pc, p, size );
	};
	inline void ReadBlock( POINTER p, int size )
	{
		if( pc ) ReadTCPMsg( pc, p, size );
	};
	inline void SetLong( int l, _32 value )
	{
      if( pc ) SetNetworkLong( pc, l, value );
	}
	inline void SetNoDelay( LOGICAL bTrue )
	{
      if( pc ) SetTCPNoDelay( pc, bTrue );
	}
	inline void SetClientKeepAlive( LOGICAL bTrue )
	{
		if( pc ) sack::network::SetClientKeepAlive( pc, bTrue );
	}
	inline PTRSZVAL GetLong( int l )
	{
		if( pc ) 
			return GetNetworkLong( pc, l );
      	return 0;
	}
}NETWORK;

#endif
SACK_NETWORK_NAMESPACE_END
#ifdef __cplusplus
using namespace sack::network;
using namespace sack::network::tcp;
using namespace sack::network::udp;
#endif

#endif
//------------------------------------------------------------------
// $Log: network.h,v $
// Revision 1.36  2005/05/23 19:29:24  jim
// Added definition to support WaitReadTCP...
//
// Revision 1.35  2005/03/15 20:22:32  chrisd
// Declare NotifyCallback with meaningful parameters
//
// Revision 1.34  2005/03/15 20:14:15  panther
// Define a routine to build a PF_UNIX socket for unix... this can be used with TCP_ routines to open a unix socket instead of an IP socket.
//
// Revision 1.33  2004/09/29 00:49:47  d3x0r
// Added fancy wait for PSI frames which allows non-polling sleeping... Extended Idle() to result in meaningful information.
//
// Revision 1.32  2004/08/18 23:52:24  d3x0r
// Cleanups - also enhanced network init to expand if called with larger params.
//
// Revision 1.31  2004/07/28 16:47:18  jim
// added support for get address parts.
//
// Revision 1.31  2004/07/27 18:28:17  d3x0r
// Added definition for getaddressparts
//
// Revision 1.30  2004/01/26 23:47:20  d3x0r
// Misc edits.  Fixed filemon.  Export net startup, added def to edit frame
//
// Revision 1.29  2003/12/03 10:21:34  panther
// Tinkering with C++ networking
//
// Revision 1.28  2003/11/09 03:32:22  panther
// Added some address functions to set port and override default port
//
// Revision 1.27  2003/09/25 08:34:00  panther
// Restore callback defs to proper place
//
// Revision 1.26  2003/09/25 08:29:16  panther
// ...New test
//
// Revision 1.25  2003/09/25 00:22:35  panther
// Move cpp wrapper functions into network library
//
// Revision 1.24  2003/09/25 00:21:49  panther
// Move cpp wrapper functions into network library
//
// Revision 1.23  2003/09/24 15:10:54  panther
// Much mangling to extend C++ network interface...
//
// Revision 1.22  2003/09/24 02:26:02  panther
// Fix C++ methods, extend and correct.
//
// Revision 1.21  2003/07/29 09:27:14  panther
// Add Keep Alive option, enable use on proxy
//
// Revision 1.20  2003/07/24 16:56:41  panther
// Updates to expliclity define C procedure model for callbacks and assembly modules - incomplete
//
// Revision 1.19  2003/06/04 11:38:01  panther
// Define PACKED
//
// Revision 1.18  2003/03/25 08:38:11  panther
// Add logging
//
// Revision 1.17  2002/12/22 00:14:11  panther
// Cleanup function declarations and project defines.
//
// Revision 1.16  2002/11/24 21:37:40  panther
// Mods - network - fix server->accepted client method inheritance
// display - fix many things
// types - merge chagnes from verious places
// ping - make function result meaningful yes/no
// controls - fixes to handle lack of image structure
// display - fixes to handle moved image structure.
//
// Revision 1.16  2002/11/21 19:13:11  jim
// Added CreateAddress, CreateAddress_hton
//
// Revision 1.15  2002/07/25 12:59:02  panther
// Added logging, removed logging....
// Network: Added NetworkLock/NetworkUnlock
// Timers: Modified scheduling if the next timer delta was - how do you say -
// to fire again before now.
//
// Revision 1.14  2002/07/23 11:24:26  panther
// Added new function to TCP networking - option on write to disable
// queuing of pending data.
//
// Revision 1.13  2002/07/17 11:33:26  panther
// Added new function to tcp network - dotcpwriteex - allows option to NOT pend
// buffers.
//
// Revision 1.12  2002/07/15 08:34:07  panther
// Include function to set udp broadcast or not.
//
//
// $Log: network.h,v $
// Revision 1.36  2005/05/23 19:29:24  jim
// Added definition to support WaitReadTCP...
//
// Revision 1.35  2005/03/15 20:22:32  chrisd
// Declare NotifyCallback with meaningful parameters
//
// Revision 1.34  2005/03/15 20:14:15  panther
// Define a routine to build a PF_UNIX socket for unix... this can be used with TCP_ routines to open a unix socket instead of an IP socket.
//
// Revision 1.33  2004/09/29 00:49:47  d3x0r
// Added fancy wait for PSI frames which allows non-polling sleeping... Extended Idle() to result in meaningful information.
//
// Revision 1.32  2004/08/18 23:52:24  d3x0r
// Cleanups - also enhanced network init to expand if called with larger params.
//
// Revision 1.31  2004/07/28 16:47:18  jim
// added support for get address parts.
//
// Revision 1.31  2004/07/27 18:28:17  d3x0r
// Added definition for getaddressparts
//
// Revision 1.30  2004/01/26 23:47:20  d3x0r
// Misc edits.  Fixed filemon.  Export net startup, added def to edit frame
//
// Revision 1.29  2003/12/03 10:21:34  panther
// Tinkering with C++ networking
//
// Revision 1.28  2003/11/09 03:32:22  panther
// Added some address functions to set port and override default port
//
// Revision 1.27  2003/09/25 08:34:00  panther
// Restore callback defs to proper place
//
// Revision 1.26  2003/09/25 08:29:16  panther
// ...New test
//
// Revision 1.25  2003/09/25 00:22:35  panther
// Move cpp wrapper functions into network library
//
// Revision 1.24  2003/09/25 00:21:49  panther
// Move cpp wrapper functions into network library
//
// Revision 1.23  2003/09/24 15:10:54  panther
// Much mangling to extend C++ network interface...
//
// Revision 1.22  2003/09/24 02:26:02  panther
// Fix C++ methods, extend and correct.
//
// Revision 1.21  2003/07/29 09:27:14  panther
// Add Keep Alive option, enable use on proxy
//
// Revision 1.20  2003/07/24 16:56:41  panther
// Updates to expliclity define C procedure model for callbacks and assembly modules - incomplete
//
// Revision 1.19  2003/06/04 11:38:01  panther
// Define PACKED
//
// Revision 1.18  2003/03/25 08:38:11  panther
// Add logging
//

/* and then we could be really evil

#define send(s,b,x,t,blah)
#define recv
#define socket
#define getsockopt ?
#define heh yeah these have exact equivalents ....

*/
