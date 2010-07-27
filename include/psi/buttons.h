#ifndef BUTTON_METHODS_DEFINED
#define BUTTON_METHODS_DEFINED
//#include <deadstart.h>
// could probably do something like make a dummy declaration
// and assign a opinter to this one to validate that args are really
// of the correct type, it's pretty easy to cheat here...

#define BUTTON_CLICK( name, args ) PUBLIC( void, name )args;   \
	PRELOAD( name##_Init ) { SimpleRegisterMethod( WIDE("psi/methods/Button/Click") \
	                                        , name, WIDE("void"), _WIDE(#name), WIDE("(PTRSZVAL,PCOMMON)") ); } \
	void CPROC name args

#define BUTTON_DRAW( name, args ) PUBLIC( void, name )args;   \
	PRELOAD( name##_Init ) { SimpleRegisterMethod( WIDE("psi/methods/Button/Draw") \
	                                        , name, WIDE("void"), _WIDE(#name), WIDE("(PTRSZVAL,PCOMMON)") ); } \
	void CPROC name args

#define BUTTON_CHECK( name, args ) PUBLIC( void, name )args;   \
	PRELOAD( name##_Init ) { SimpleRegisterMethod( WIDE("psi/methods/Button/Check") \
	                                        , name, WIDE("void"), _WIDE(#name), WIDE("(PTRSZVAL,PCOMMON)") ); } \
	void CPROC name args




#endif

