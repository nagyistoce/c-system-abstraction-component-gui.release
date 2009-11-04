#define BAG_EXIT_DEFINED
#include <stdhdrs.h>
#include <sack_types.h>
#include <deadstart.h>

SACK_NAMESPACE

// linked into BAG to provide a common definition for function Exit()
// this then invokes an exit in the mainline program (if available)
#ifndef __STATIC__
DYNAMIC_EXPORT
#endif
	void BAG_Exit( int code )
{
#ifndef __cplusplus_cli
	InvokeExits();
#endif
#undef exit
   exit( code );
}

SACK_NAMESPACE_END

