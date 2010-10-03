
#include <stdhdrs.h>
#include <system.h>
#include <procreg.h>
#include "link_events.h"

#include "ianson/video_ctrl.h"

static void VideoLinkCommandServeMaster( "MUX Process" )( void )
{
   video_ctrl_pip( 2, 1 );
}

static void VideoLinkCommandServeDelegate( "MUX Process" )( void )
{
   video_ctrl_pip( 2, 1 );
}

static void VideoLinkCommandConnectToMaster( "MUX Process" )( CTEXTSTR unused )
{
   video_ctrl_display( 4 );
}

static void VideoLinkCommandConnectToDelegate( "MUX Process" )( CTEXTSTR unused )
{
   video_ctrl_display( 4 );
}

static void VideoLinkCommandReset( "MUX Process" )( LOGICAL soft_reset )
{
   video_ctrl_display( 1 );
}

#ifdef __WATCOMC__
PUBLIC( void, MustExportOneFunction )( void )
{
}
#endif


