#define IS_DEADSTART
#include <stdhdrs.h>
#include <sack_types.h>

SACK_NAMESPACE



#ifdef __WINDOWS__


PUBLIC( LOGICAL, is_deadstart_complete )( void )
{
	TEXTCHAR myname[256];
	HMODULE mod;
	GetModuleFileName( NULL, myname, sizeof( myname ) );
	mod=LoadLibrary(myname);
	if(mod)
	{
		_32 *rsp;
		((rsp=(_32*)(GetProcAddress( mod, "deadstart_complete"))));
      if( !rsp )
			((rsp=(_32*)(GetProcAddress( mod, "_deadstart_complete"))));
		if(rsp)
		{
			_32 res = (*rsp);
			FreeLibrary( mod );
			return res;
		}
		else
		{
			lprintf( WIDE("deadstart code in main application does not support 'complete'") );
			return FALSE;
		}
	}
	return FALSE;

}

#else
int is_deadstart_complete( void )
{
   //extern _32 deadstart_complete;
	return 1;//deadstart_complete;
}
#endif

SACK_NAMESPACE_END

