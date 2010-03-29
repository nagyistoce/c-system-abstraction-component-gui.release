#ifndef KEY_STRUCTURE_DEFINED

RENDER_NAMESPACE

#ifndef RENDER_INTERFACE_INCLUDED
typedef void (CPROC *KeyTriggerHandler)(PTRSZVAL,_32 keycode);
typedef struct KeyDefine *PKEYDEFINE;
#endif
//#if !defined( DISPLAY_SOURCE ) && !defined( DISPLAY_SERVICE ) && !defined( DISPLAY_CLIENT )
#define KEY_STRUCTURE_DEFINED
typedef struct keybind_tag { // overrides to default definitions
   struct {
		int bFunction:1;
		int bRelease:1;
		int bAll:1; // application wants presses and releases..
   } flags;
		struct key_triggers{
			KeyTriggerHandler trigger;
			KeyTriggerHandler extended_key_trigger;
		}data;
	PTRSZVAL psv;
   PTRSZVAL extended_key_psv;
} KEYBIND, *PKEYBIND;

typedef struct KeyDefine {
   // names which keys may be also called if the key table is dumped
   char *name1;
   // names which keys may be also called if the key table is dumped
   char *name2;
   int flags;
   KEYBIND mod[8];
} KEYDEFINE;


RENDER_NAMESPACE_END


#endif
