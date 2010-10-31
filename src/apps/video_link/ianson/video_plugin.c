#include <stdhdrs.h>

#include "../src/InterShell/intershell_registry.h"
#include "../link_events.h"


static void VideoLinkCommandServeMaster( "Update Video Mux" )( void )
{
   lprintf( "Set video to master mode." );
}

static void VideoLinkCommandServeDelegate( "Update Video Mux" )( void )
{
   lprintf( "Set video to master(delegate) mode." );
}

static void VideoLinkCommandConnectToMaster( "Update Video Mux" )( CTEXTSTR remote )
{
   lprintf( "Set video to show-link mode." );
}

static void VideoLinkCommandConnectToDelegate( "Update Video Mux" )( CTEXTSTR remote )
{
   lprintf( "Set video to show-link mode." );
}

static void VideoLinkCommandReset( "Update Video Mux" )( LOGICAL forced )
{
   lprintf( "Set video to local mode." );
}


