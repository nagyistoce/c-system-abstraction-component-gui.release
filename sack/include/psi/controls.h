
#include <deadstart.h>

#define CUSTOM_CONTROL_DRAW( name, args ) PUBLIC( int, name )args;   \
	PRELOAD( name##_Init ) { SimpleRegisterMethod( PSI_ROOT_REGISTRY WIDE("/control/Undefined/Draw") \
	                                        , name, WIDE("int"), _WIDE(#name), WIDE("(PCOMMON)") ); } \
	int CPROC name args


#define CUSTOM_CONTROL_MOUSE( name, args ) PUBLIC( int, name )args;   \
	PRELOAD( name##_Init ) { SimpleRegisterMethod( PSI_ROOT_REGISTRY WIDE("/control/Undefined/Mouse") \
	                                        , name, WIDE("int"), _WIDE(#name), WIDE("(PCOMMON, S_32, S_32, _32)") ); } \
	int CPROC name args

#define CUSTOM_CONTROL_KEY( name, args ) PUBLIC( int, name )args;   \
	PRELOAD( name##_Init ) { SimpleRegisterMethod( PSI_ROOT_REGISTRY WIDE("/control/Undefined/Key") \
	                                        , name, WIDE("int"), _WIDE(#name), WIDE("(PCOMMON, _32)") ); } \
	int CPROC name args
