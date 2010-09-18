
// sample prototype to copy and paste new methods, replace @ with WORDS
/*
#define @( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), @, WIDE("server core/@"), WIDE(#name),name,void,( ), __LINE__)
*/

#define VideoLinkCommandServeMaster( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandServeMaster, WIDE("server core/CommandServeMaster"), ,name,void,( void ), __LINE__)

#define VideoLinkCommandServeDelegate( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandServeDelegate, WIDE("server core/CommandServeDelegate"), ,name,void,( void ), __LINE__)

#define VideoLinkCommandServeBData( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandServeBdata, WIDE("server core/CommandServeBdata"), ,name,void,( void ), __LINE__)

#define VideoLinkCommandConnectToMaster( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandConnectToMaster, WIDE("server core/CommandConnectToMaster"), ,name,void,( CTEXTSTR ), __LINE__)

#define VideoLinkCommandConnectToDelegate( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandConnectToDelegate, WIDE("server core/CommandConnectToDelegate"), ,name,void,( CTEXTSTR ), __LINE__)

#define VideoLinkCommandConnectToBData( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandConnectToBData, WIDE("server core/CommandConnectToBData"), ,name,void,( CTEXTSTR ), __LINE__)

#define VideoLinkCommandSetInputStreamMode( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandSetInputStreamMode, WIDE("server core/CommandSetInputStreamMode"), ,name,void,( int ), __LINE__)

#define VideoLinkCommandReset( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandReset, WIDE("server core/CommandReset"), ,name,void,( LOGICAL ), __LINE__)

#define VideoLinkCommandDisconnectBData( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandDisconnectBData, WIDE("server core/CommandDisconnectBData"), ,name,void,( ), __LINE__)


#define VIDEO_SERVER_PROC_PTR(type,name)  type (CPROC* name)

struct video_server_interface {
   VIDEO_SERVER_PROC_PTR( void, MarkTaskStarted )( void );
   VIDEO_SERVER_PROC_PTR( void, MarkTaskDone )( void );
};
#ifdef USES_VIDEO_SERVER_INTERFACE

#define MarkTaskStarted()   if(VideoServerInterface) VideoServerInterface->MarkTaskStarted()
#define MarkTaskDone()   if(VideoServerInterface) VideoServerInterface->MarkTaskDone()


#  ifndef DEFINES_VIDEO_SERVER_INTERFACE
extern 
#  endif
	 struct video_server_interface *VideoServerInterface
#endif
#ifdef __GCC__
	 __attribute__((visibility("hidden")))
#endif
	 ;

#  ifdef DEFINES_VIDEO_SERVER_INTERFACE
// this needs to be done before most modules can run their PRELOADS...so just move this one.
// somehow this ended up as 69 and 69 was also PRELOAD() priority... bad.
PRIORITY_PRELOAD( InitInterface, DEFAULT_PRELOAD_PRIORITY - 3)
{
	VideoServerInterface = (struct video_server_interface*)GetInterface( "Video Server" );
}

#  endif

