#ifndef __KEYPAD_WIDGET_DEFINED__
#define __KEYPAD_WIDGET_DEFINED__
#include <controls.h>

#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef KEYPAD_SOURCE
#define KEYPAD_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define KEYPAD_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#else
#ifdef KEYPAD_SOURCE
#define KEYPAD_PROC(type,name) type name
#else
#define KEYPAD_PROC(type,name) extern type name
#endif
#endif

//-- keypad.c --------------------

#ifndef KEYPAD_STRUCTURE_DEFINED
typedef PSI_CONTROL PKEYPAD;
#endif

#define KEYPAD_FLAG_DISPLAY 1
#define KEYPAD_FLAG_PASSWORD 2 // 2 and 1
#define KEYPAD_FLAG_ENTRY 4
#define KEYPAD_FLAG_ALPHANUM 8

KEYPAD_PROC( void, SetNewKeypadFlags )( int newflags );
// if you call SetNewKeypadFlags with the desired flags, and
// then invoke a normal MakeNamedControl( WIDE("Keypad Control") ) you will
// get the desired results - though not thread safe...
KEYPAD_PROC(PSI_CONTROL, MakeKeypad )( PCOMMON parent
												 , S_32 x, S_32 y, _32 w, _32 h, _32 ID, _32 flags
												 , CTEXTSTR accumulator_name );
KEYPAD_PROC(void, SetKeypadAccumulator )( PSI_CONTROL keypad, char *accumulator_name );
KEYPAD_PROC(S_64, GetKeyedValue )( PSI_CONTROL keypad );
KEYPAD_PROC(int, GetKeyedText )( PSI_CONTROL keypad, TEXTSTR buffer, int buffersize );
KEYPAD_PROC(void, ClearKeyedEntry )( PSI_CONTROL keypad );
KEYPAD_PROC( void, ClearKeyedEntryOnNextKey )( PSI_CONTROL pc );

KEYPAD_PROC(int, WaitForKeypadResult) ( PSI_CONTROL keypad );
KEYPAD_PROC( void, CancelKeypadWait )( PSI_CONTROL keypad );
//KEYPAD_PROC(void, HideKeypad )( PSI_CONTROL keypad );
//KEYPAD_PROC(void, ShowKeypad )( PSI_CONTROL keypad );
#define HideKeypad HideCommon
#define ShowKeypad RevealCommon

//KEYPAD_PROC(PCOMMON, GetKeypadCommon )( PSI_CONTROL keypad );
#define GetKeypadCommon(keypad) (keypad)
//KEYPAD_PROC(PSI_CONTROL, GetKeypad )( PCOMMON pc );
#define GetKeypad(pc) (pc)

KEYPAD_PROC( void, SetKeypadEnterEvent )( PCOMMON pc, void (CPROC *event)(PTRSZVAL,PSI_CONTROL), PTRSZVAL psv );
KEYPAD_PROC( PSI_CONTROL, MakeKeypadHotkey )( PSI_CONTROL frame
														  , S_32 x
														  , S_32 y
														  , _32 w
														  , _32 h
														  , char *keypad
														  );
KEYPAD_PROC( void, KeyIntoKeypad )( PSI_CONTROL keypad, _64 value );
KEYPAD_PROC( void, KeypadInvertValue )( PSI_CONTROL keypad );
#endif
