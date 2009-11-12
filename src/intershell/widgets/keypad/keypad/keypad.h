#ifndef KEYPAD_ISP_PUBLIC_DEFINED
#define KEYPAD_ISP_PUBLIC_DEFINED 

#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef KEYPAD_ISP_SOURCE
#define KEYPAD_ISP_PROC(type,name) __declspec(dllexport) type CPROC name
#else
#define KEYPAD_ISP_PROC(type,name) __declspec(dllimport) type CPROC name
#endif
#else
#ifdef KEYPAD_ISP_SOURCE
#define KEYPAD_ISP_PROC(type,name) type CPROC name
#else
#define KEYPAD_ISP_PROC(type,name) extern type CPROC name
#endif
#endif

#include "../../include/keypad.h"

KEYPAD_ISP_PROC( void, CreateKeypadType )( CTEXTSTR name );
KEYPAD_ISP_PROC( PSI_CONTROL, GetPOSKeypad )( void );


#define OnKeypadEnter(name) \
	  DefineRegistryMethod(TASK_PREFIX,KeypadEnter,"common","keypad enter",name"_on_keypad_enter",void,(PSI_CONTROL))

#define OnKeypadEnterType(name, typename) \
	  DefineRegistryMethod(TASK_PREFIX,KeypadTypeEnter,"common/"typename,"keypad enter",name"_on_keypad_enter",void,(PSI_CONTROL))


#endif
