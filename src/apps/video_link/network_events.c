#include <stdhdrs.h>
#include <network.h>
#include <deadstart.h>
#include <procreg.h>

#define USES_VIDEO_SERVER_INTERFACE
#define DEFINES_VIDEO_SERVER_INTERFACE
#include "link_events.h"

struct network_notifier
{
	PCLIENT pc;
   PLIST peer_addresses;
}

static struct video_network_events
{
   PLIST hosts;
} video_network_events;

#define l video_network_events

static void CPROC PacketInput( PCLIENT pc, POINTER buffer, int length, SOCKADDR *sa_from )
{
	if( !buffer )
	{
		buffer = Allocate( 1000 );
	}
	else
	{
      StateChanged();
	}
	ReadUDP( pc, buffer, 1000 );
}


PRELOAD( InitNetworkService )
{
	TEXTCHAR tmp[256];
	TEXTCHAR tmp2[256];
   PCLIENT pc_host;
	int iface = 1;
	int default_port = SACK_GetProfileInt( GetProgramName(), "Video Server/Service Events/Default Port", 5722 );
	do
	{
      SOCKADDR *host;
		snprintf( tmp, sizeof( tmp ), "Video Server/Service Events/interface %d", iface );
		SACK_GetProfileString( GetProgramName(), tmp, "", tmp2, sizeof( tmp2 ) );
		host = CreateSockAddress( tmp2, default_port );
		if( !host )
			lprintf( "Failed to convert [%s] at %s to a interface address", tmp2, tmp );
		else
		{
			pc_host = ServeUDPAddr( host, PacketInput, NULL );
			if( !pc_host )
				lprintf( "Failed to bind to [%s] default port %d at %s", tmp2, default_port, tmp );
			else
			{
				int isend;
				do
				{
               SOCK_ADDR *remote;
					snprintf( tmp, sizeof( tmp ), "Video Server/Service Events/interface %d/Send To %d", iface, isend );
					SACK_GetProfileString( GetProgramName(), tmp, "", tmp3, sizeof( tmp3 ) );
					remote = CreateSockAddress( tmp2, default_port );
					if( !remote )
					{
                  lprintf( "Failed to convert [%s] to an address (%s)", tmp3, tmp );
					}
					else
                  AddLink( &l.

				}
            while( tmp3[0] );
				AddLink( &l.sockets, pc_host );
			}
		}
      isend++;
	}
	while( tmp2[0] );
}

static void VideoLinkCommandStateChanged( "network_event" )( void )
{
	INDEX idx;
	SOCKADDR *addr;
	LIST_FORALL( l.peer_address_list, idx, SOCKADDR*, addr )
	{
      SendUDP( l.pc_listen, "STATE CHANGE", 12, addr );
	}
}



