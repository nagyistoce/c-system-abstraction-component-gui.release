
#include <deadstart.h>
#define SLIDER_UPDATE( name, args ) PUBLIC( void, name )args;   \
	PRELOAD( name##_Init ) { SimpleRegisterMethod( WIDE("psi/control/Slider/Update") \
	                                        , name, WIDE("void"), _WIDE(#name), WIDE("(PTRSZVAL,PCONTROL,int)") ); } \
	void CPROC name args



