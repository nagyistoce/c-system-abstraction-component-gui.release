
// sample prototype to copy and paste new methods, replace @ with WORDS
/*
#define @( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), @, WIDE("server core/@"), WIDE(#name),name,void,( ), __LINE__)
*/

#define VideoLinkCommandServeMaster( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandServeMaster, WIDE("server core/CommandServeMaster"), WIDE(#name),name,void,( void ), __LINE__)

#define VideoLinkCommandServeDelegate( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandServeDelegate, WIDE("server core/CommandServeDelegate"), WIDE(#name),name,void,( void ), __LINE__)

#define VideoLinkCommandServeBData( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandServeBdata, WIDE("server core/CommandServeBdata"), WIDE(#name),name,void,( void ), __LINE__)

#define VideoLinkCommandConnectToMaster( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandConnectToMaster, WIDE("server core/CommandConnectToMaster"), WIDE(#name),name,void,( CTEXTSTR ), __LINE__)

#define VideoLinkCommandConnectToDelegate( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandConnectToDelegate, WIDE("server core/CommandConnectToDelegate"), WIDE(#name),name,void,( CTEXTSTR ), __LINE__)

#define VideoLinkCommandConnectToBData( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandConnectToBData, WIDE("server core/CommandConnectToBData"), WIDE(#name),name,void,( CTEXTSTR ), __LINE__)

#define VideoLinkCommandSetInputStreamMode( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandSetInputStreamMode, WIDE("server core/CommandSetInputStreamMode"), WIDE(#name),name,void,( int ), __LINE__)

#define VideoLinkCommandReset( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandReset, WIDE("server core/CommandReset"), WIDE(#name),name,void,( LOGICAL ), __LINE__)

#define VideoLinkCommandDisconnectBData( name ) \
	__DefineRegistryMethod( WIDE( "video link" ), CommandDisconnectBData, WIDE("server core/CommandDisconnectBData"), WIDE(#name),name,void,( ), __LINE__)

