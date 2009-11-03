
#include <sack_types.h>

#ifdef __WATCOMC__
// returns the size passed.
_32 __cdecl PushArgument( _32 size, ... );
#define PushArgument(a) PushArgument( sizeof(a), (a) )
_32 __cdecl PopArguments( _32 size );	
#else
// returns the size passed.
_32 PushArgument( _32 size, ... );
#define PushArgument(a) PushArgument( sizeof(a), (a) )
_32 PopArguments( _32 size );	
#endif


void CallProcedure( PTRSZVAL *psv, PCONFIG_ELEMENT pce );

// $Log: vargs.h,v $
// Revision 1.5  2003/03/25 08:45:50  panther
// Added CVS logging tag
//
