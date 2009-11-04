#ifndef DEADSTART_DEFINED
#define DEADSTART_DEFINED

#ifdef __WINDOWS__
//#include <stdhdrs.h>
#endif
#include <sack_types.h>
#include <typelib.h> // leach, assuming this will be compiled with this part at least.

#ifdef TYPELIB_SOURCE
#define DEADSTART_SOURCE
#endif

#ifdef __cplusplus
#define USE_SACK_DEADSTART_NAMESPACE using namespace sack::app::deadstart;
#define SACK_DEADSTART_NAMESPACE   SACK_NAMESPACE namespace app { namespace deadstart {
#define SACK_DEADSTART_NAMESPACE_END    } } SACK_NAMESPACE_END
#else
#define USE_SACK_DEADSTART_NAMESPACE 
#define SACK_DEADSTART_NAMESPACE
#define SACK_DEADSTART_NAMESPACE_END
#endif

#ifdef DEADSTART_SOURCE
#define DEADSTART_PROC TYPELIB_PROC
#else
#define DEADSTART_PROC TYPELIB_PROC
#endif

SACK_DEADSTART_NAMESPACE


   // this is just a global space initializer (shared, named region, allows static link plugins to share information)
#define CONFIG_SCRIPT_PRELOAD_PRIORITY    (SQL_PRELOAD_PRIORITY-1)
   // this is just a global space initializer (shared, named region, allows static link plugins to share information)
#define SQL_PRELOAD_PRIORITY    (SYSLOG_PRELOAD_PRIORITY-1)
#define SYSLOG_PRELOAD_PRIORITY 35
   // global_init_preload_priority-1 is used by sharemem.. memory needs init before it can register itself
#define GLOBAL_INIT_PRELOAD_PRIORITY 37

#define OSALOT_PRELOAD_PRIORITY (CONFIG_SCRIPT_PRELOAD_PRIORITY-1) // OS A[bstraction] L[ayer] O[n] T[op] - system lib

#define NAMESPACE_PRELOAD_PRIORITY 39
   // image_preload MUST be fter Namespce preload (anything that uses RegisterAndCreateGlobal)
#define IMAGE_PRELOAD_PRIORITY  45 // should init this before vidlib (which needs image?)
#define VIDLIB_PRELOAD_PRIORITY 46
#define PSI_PRELOAD_PRIORITY    47

// need to open the queues and threads before the service server can begin...
#define MESSAGE_CLIENT_PRELOAD_PRIORITY 65
#define MESSAGE_SERVICE_PRELOAD_PRIORITY 66
#define DEFAULT_PRELOAD_PRIORITY (DEADSTART_PRELOAD_PRIORITY-1)
#define DEADSTART_PRELOAD_PRIORITY 70

#ifdef __WATCOMC__
#define default_name_for_deadstart_runner "RunDeadstart_"
#endif
#ifdef _MSC_VER
#define default_name_for_deadstart_runner "_RunDeadstart"
#endif
#if defined( __CYGWIN__ )
#define default_name_for_deadstart_runner "RunDeadstart"
#endif


// proc, proc_name, priority DEADSTART_PRIORTY_,unused to hack in self reference of static symbol
// this will trick most compilers.
// uses a compiler-native function (not cproc)
DEADSTART_PROC( void, RegisterPriorityStartupProc)( void(*)(void), CTEXTSTR,int,void* unused, CTEXTSTR,int);
DEADSTART_PROC( void, RegisterPriorityShutdownProc)( void(*)(void), CTEXTSTR,int,void* unused, CTEXTSTR,int);
DEADSTART_PROC( void, SuspendDeadstart )( void );
DEADSTART_PROC( void, InvokeDeadstart )(void);
DEADSTART_PROC( void, InvokeExits )(void);
DEADSTART_PROC( void, MarkRootDeadstartComplete )( void );

#ifdef __LINUX__
// call this after a fork().  Otherwise, it will falsely invoke shutdown when it exits.
DEADSTART_PROC( void, DispelDeadstart )( void );
#endif

#if defined( __cplusplus) 

#define PRIORITY_PRELOAD(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {    \
	RegisterPriorityStartupProc( name,#name,priority,(void*)this,__FILE__,__LINE__ );\
	  }  \
	} do_schedul_##name;     \
	static void name(void)
#define MAGIC_PRIORITY_PRELOAD(name,priority) static void name(void); \
   static class schedule_##name {   \
	  public:schedule_##name() {  \
	name();  \
	  }  \
	} do_schedul_##name;     \
	static void name(void)
#define ATEXIT_PRIORITY(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {    \
	RegisterPriorityShutdownProc( name,#name,priority,(void*)this,__FILE__,__LINE__ );\
	  }  \
	} do_schedul_##name;     \
	static void name(void)
#define PRIORITY_ATEXIT(name,priority) static void name(void); \
   static class shutdown_##name {   \
	public:shutdown_##name() {    \
   RegisterPriorityShutdownProc( name,#name,priority,(void*)this,__FILE__,__LINE__ );\
	/*name(); / * call on destructor of static object.*/ \
	  }  \
	} do_shutdown_##name;     \
	static void name(void)


#define PRELOAD(name) PRIORITY_PRELOAD(name,DEFAULT_PRELOAD_PRIORITY)
#define ATEXIT(name)      PRIORITY_ATEXIT(name,ATEXIT_PRIORITY_DEFAULT)
#define ROOT_ATEXIT(name) ATEXIT_PRIORITY(name,ATEXIT_PRIORITY_ROOT)

//------------------------------------------------------------------------------------
// Win32 Watcom
//------------------------------------------------------------------------------------
#elif defined( __WATCOMC__ )
#pragma off (check_stack)
#if defined( _MSC_VER )
#error both watcom and MSC_VER?!?!?!
#endif
/* code taken from openwatcom/bld/watcom/h/rtinit.h */
typedef unsigned char   __type_rtp;
typedef unsigned short  __type_pad;
typedef void(*__type_rtn ) ( void );
#ifdef __cplusplus
#pragma pack(1)
#else
#pragma pack(1)
#endif
struct rt_init // structure placed in XI/YI segment
{
    __type_rtp  rtn_type; // - near=0/far=1 routine indication
                          //   also used when walking table to flag
                          //   completed entries
    __type_rtp  priority; // - priority (0-highest 255-lowest)
    __type_rtn  rtn;      // - routine
};
#pragma pack()
/* end code taken from openwatcom/bld/watcom/h/rtinit.h */


/* in the main program is a routine which is referenced in dynamic mode...
 need to redo this macro in the case of static linking...

 */

//------------------------------------------------------------------------------------
//  WIN32 basic methods... on wait
//------------------------------------------------------------------------------------


#ifdef _legacy_reverse_link_app_
#define InvokeDeadstart() do {                                              \
	TEXTCHAR myname[256];HMODULE mod;GetModuleFileName( NULL, (LPSTR)myname, sizeof( myname ) );\
	mod=LoadLibrary((LPSTR)myname);if(mod){        \
   void(*rsp)(void); \
	if((rsp=((void(*)(void))(GetProcAddress( mod, default_name_for_deadstart_runner))))){rsp();}else{lprintf( WIDE("Hey failed to get proc %d"), GetLastError() );}\
	FreeLibrary( mod); }} while(0)
#endif

//------------------------------------------------------------------------------------
// MSVC Starup/atexit registration methods...
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
// watcom
//------------------------------------------------------------------------------------
#ifdef __WATCOMC__
//void RegisterStartupProc( void (*proc)(void) );


#ifdef __cplusplus
#else
#define PRIORITY_PRELOAD(name,priority) static void schedule_##name(void); static void name(void); \
	static struct rt_init __based(__segname("XI")) name##_ctor_label={0,(DEADSTART_PRELOAD_PRIORITY-1),schedule_##name}; \
	static void schedule_##name(void) {                 \
	RegisterPriorityStartupProc( name,#name,priority,&name##_ctor_label,__FILE__,__LINE__ );\
	}                                       \
	static void name(void)
#define ATEXIT_PRIORITY(name,priority) static void schedule_exit_##name(void); static void name(void); \
	static struct rt_init __based(__segname("XI")) name##_dtor_label={0,69,schedule_exit_##name}; \
	static void schedule_exit_##name(void) {                                              \
	RegisterPriorityShutdownProc( name,#name,priority,&name##_dtor_label,__FILE__,__LINE__ );\
	}                                       \
	static void name(void)
#endif
// syslog runs preload at priority 65
// message service runs preload priority 66
// deadstart itself tries to run at priority 70 (after all others have registered)
#define PRELOAD(name) PRIORITY_PRELOAD(name,DEFAULT_PRELOAD_PRIORITY)


// this is a special case macro used in client.c
// perhaps all PRIORITY_ATEXIT routines should use this
// this enables cleaning up things that require threads to be
// active under windows... (message disconnect)
// however this routine is only triggered in windows by calling
// BAG_Exit(nn) which is aliased to replace exit(n) automatically

#ifdef __cplusplus
#else
#define PRIORITY_ATEXIT(name,priority) ATEXIT_PRIORITY( name,priority)
/*
static void name(void); static void name##_x_(void);\
	static struct rt_init __based(__segname("YI")) name##_dtor_label={0,priority,name##_x_}; \
	static void name##_x_(void) { char myname[256];myname[0]=*(CTEXTSTR)&name##_dtor_label;GetModuleFileName(NULL,myname,sizeof(myname));name(); } \
	static void name(void)
  */
#endif
#define ROOT_ATEXIT(name) ATEXIT_PRIORITY(name,ATEXIT_PRIORITY_ROOT)
#define ATEXIT(name)      PRIORITY_ATEXIT(name,ATEXIT_PRIORITY_DEFAULT)
// if priority_atexit is used with priority 0 - the proc is scheduled into
// atexit, and exit() is then invoked.
//#define PRIORITY_ATEXIT(name,priority) ATEXIT_PRIORITY(name,priority )

#endif
//------------------------------------------------------------------------------------
// Linux
//------------------------------------------------------------------------------------
#elif defined( __LINUX__ )

/* code taken from openwatcom/bld/watcom/h/rtinit.h */
typedef unsigned char   __type_rtp;
typedef void(*__type_rtn ) ( void );
struct rt_init // structure placed in XI/YI segment
{
#ifdef __cplusplus
	//rt_init( int _rtn_type ) { rt_init::rtn_type = _rtn_type; }
	/*rt_init( int _priority, CTEXTSTR name, __type_rtn rtn, CTEXTSTR _file, int _line )
	{
		rtn_type = 0;
		scheduled = 0;
		priority = priority;
		file = _file;
		line = _line;
      routine = rtn;
		}
      */
#endif
    __type_rtp  rtn_type; // - near=0/far=1 routine indication
                          //   also used when walking table to flag
                          //   completed entries
    __type_rtp  scheduled; // has this been scheduled? (0 if no)
    __type_rtp  priority; // - priority (0-highest 255-lowest)
#if defined( __LINUX64__ ) ||defined( __arm__ )
#define INIT_PADDING ,{0}
	 char padding[1]; // need this otherwise it's 23 bytes and that'll be bad.
#else
#define INIT_PADDING
#endif
	 int line; // 32 bits in 64 bits....
// this ends up being nicely aligned for 64 bit platforms
// specially with the packed attributes
	 __type_rtn  routine;      // - routine (rtn)
	 CTEXTSTR file;
	 CTEXTSTR funcname;
	 struct rt_init *junk;
#ifdef __LINUX64__
    // this provides padding - inter-object segments are packed
    // to 32 bytes...
	 struct rt_init *junk2[3];
#endif
} __attribute__((packed));

#define JUNKINIT(name) ,&name##_ctor_label

#ifdef __cplusplus
#define RTINIT_STATIC 
#else
#define RTINIT_STATIC static
#endif

#ifdef __cplusplus
#define PRIORITY_PRELOAD(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {    \
	        RegisterPriorityStartupProc( name,#name,priority,this,__FILE__,__LINE__ );\
    }\
	} do_schedul_##name;     \
	static void name(void)
#define MAGIC_PRIORITY_PRELOAD(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {  name();  \
	  }  \
	} do_schedul_##name;     \
	static void name(void)
#define ATEXIT_PRIORITY(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {    \
        RegisterPriorityShutdownProc( name,#name,priority,this,__FILE__,__LINE__ ); \
      }\
	} do_schedul_##name;     \
	static void name(void)
#define PRIORITY_ATEXIT ATEXIT_PRIORITY
#else // this is gcc, compiling as a C file...
#define ATEXIT_PRIORITY PRIORITY_ATEXIT
#define PRIORITY_PRELOAD(name,pr) static void name(void); \
	RTINIT_STATIC struct rt_init name##_ctor_label \
	  __attribute__((section("deadstart_list"))) \
	={0,0,pr INIT_PADDING    \
	 ,__LINE__,name         \
	 ,__FILE__        \
	,#name        \
	JUNKINIT(name)}; \
	static void name(void)

typedef void(*atexit_priority_proc)(void (*)(void),CTEXTSTR,int,CTEXTSTR,int);
#define PRIORITY_ATEXIT(name,priority) static void name(void); static void atexit##name(void) __attribute__((constructor));  \
void atexit##name(void)                                                  \
{                                                                        \
	RegisterPriorityShutdownProc(name,#name,priority,NULL,__FILE__,__LINE__);                          \
}                                                                          \
void name(void)
#endif

#define ATEXIT(name) PRIORITY_ATEXIT( name,ATEXIT_PRIORITY_DEFAULT )
#define ROOT_ATEXIT(name) static void name(void) __attribute__((destructor)); \
   static void name(void)

#define PRELOAD(name) PRIORITY_PRELOAD(name,DEFAULT_PRELOAD_PRIORITY)



//------------------------------------------------------------------------------------
// CYGWIN (-mno-cygwin)
//------------------------------------------------------------------------------------
#elif defined( __CYGWIN__ )

/* code taken from openwatcom/bld/watcom/h/rtinit.h */
typedef unsigned char   __type_rtp;
typedef void(*__type_rtn ) ( void );
struct rt_init // structure placed in XI/YI segment
{
#ifdef __cplusplus
	//rt_init( int _rtn_type ) { rt_init::rtn_type = _rtn_type; }
	/*rt_init( int _priority, CTEXTSTR name, __type_rtn rtn, CTEXTSTR _file, int _line )
	{
		rtn_type = 0;
		scheduled = 0;
		priority = priority;
		file = _file;
		line = _line;
      routine = rtn;
		}
      */
#endif
    __type_rtp  rtn_type; // - near=0/far=1 routine indication
                          //   also used when walking table to flag
                          //   completed entries
    __type_rtp  scheduled; // has this been scheduled? (0 if no)
    __type_rtp  priority; // - priority (0-highest 255-lowest)
#if defined( __LINUX64__ ) ||defined( __arm__ ) || defined( __CYGWIN__ )
#define INIT_PADDING ,{0}
	 char padding[1]; // need this otherwise it's 23 bytes and that'll be bad.
#else
#define INIT_PADDING
#endif
	 int line; // 32 bits in 64 bits....
// this ends up being nicely aligned for 64 bit platforms
// specially with the packed attributes
	 __type_rtn  routine;      // - routine (rtn)
	 CTEXTSTR file;
	 CTEXTSTR funcname;
	 struct rt_init *junk;
#ifdef __LINUX64__
    // this provides padding - inter-object segments are packed
    // to 32 bytes...
	 struct rt_init *junk2[3];
#endif
} __attribute__((packed));

#define JUNKINIT(name) ,&name##_ctor_label

#ifdef __cplusplus
#define RTINIT_STATIC 
#else
#define RTINIT_STATIC static
#endif

#ifdef __cplusplus
#define PRIORITY_PRELOAD(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {    \
	static char myname[256];HMODULE mod;if(myname[0])return;myname[0]='a';GetModuleFileName( NULL, myname, sizeof( myname ) );\
	mod=LoadLibrary(myname);DebugBreak();if(mod){\
   typedef void (*x)(void);void(*rsp)( x,const CTEXTSTR,int,const CTEXTSTR,int); \
	if((rsp=((void(*)(void(*)(void),const CTEXTSTR,int,const CTEXTSTR,int))(GetProcAddress( mod, WIDE("RegisterPriorityStartupProc"))))))\
	{rsp( name,#name,priority,__FILE__,__LINE__ );}}\
     FreeLibrary( mod); \
    }\
	} do_schedul_##name;     \
	static void name(void)
#define MAGIC_PRIORITY_PRELOAD(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {  name();  \
	  }  \
	} do_schedul_##name;     \
	static void name(void)
#define ATEXIT_PRIORITY(name,priority) static void name(void); \
   static class schedule_##name {   \
     public:schedule_##name() {    \
	static char myname[256];HMODULE mod;DebugBreak();if(myname[0])return;myname[0]='a';GetModuleFileName( NULL, myname, sizeof( myname ) );\
	mod=LoadLibrary(myname);if(mod){\
   typedef void (*x)(void);void(*rsp)( x,const CTEXTSTR,int,const CTEXTSTR,int); \
	if((rsp=((void(*)(void(*)(void),const CTEXTSTR,int,const CTEXTSTR,int))(GetProcAddress( mod, WIDE("RegisterPriorityShutdownProc"))))))\
	{rsp( name,#name,priority,__FILE__,__LINE__ );}}\
     FreeLibrary( mod); \
      }\
	} do_schedul_##name;     \
	static void name(void)

#else

typedef void(*atexit_priority_proc)(void (*)(void),CTEXTSTR,int,CTEXTSTR,int);
#define ATEXIT_PRIORITY(name,priority) static void name(void); static void atexit##name(void) __attribute__((constructor));  \
	void atexit_failed##name(void(*f)(void),int i,CTEXTSTR s1,CTEXTSTR s2,int n) { lprintf( WIDE("Failed to load atexit_priority registerar from core program.") );} \
void atexit##name(void)                                                  \
{                                                                        \
	static char myname[256];HMODULE mod;if(myname[0])return;myname[0]='a';GetModuleFileName( NULL, myname, sizeof( myname ) );\
	mod=LoadLibrary(myname);if(mod){\
   typedef void (*x)(void);void(*rsp)( x,const CTEXTSTR,int,const CTEXTSTR,int); \
	if((rsp=((void(*)(void(*)(void),const CTEXTSTR,int,const CTEXTSTR,int))(GetProcAddress( mod, WIDE("RegisterPriorityShutdownProc"))))))\
	 {rsp( name,#name,priority,__FILE__,__LINE__ );}\
	 else atexit_failed##name(name,priority,#name,__FILE__,__LINE__);        \
	}\
     FreeLibrary( mod); \
	}             \
void name( void) \

#define PRIORITY_PRELOAD(name,pr) static void name(void); \
	RTINIT_STATIC struct rt_init name##_ctor_label \
	  __attribute__((section("deadstart_list"))) \
	={0,0,pr INIT_PADDING    \
	 ,__LINE__,name         \
	 ,__FILE__        \
	,#name        \
	JUNKINIT(name)}; \
	static void name(void)
#endif

#define ATEXIT(name)      ATEXIT_PRIORITY(name,ATEXIT_PRIORITY_DEFAULT)
#define PRIORITY_ATEXIT ATEXIT_PRIORITY

#define ROOT_ATEXIT(name) static void name(void) __attribute__((destructor)); \
   static void name(void)

#define PRELOAD(name) PRIORITY_PRELOAD(name,DEFAULT_PRELOAD_PRIORITY)

#ifdef __old_deadstart
#define InvokeDeadstart() do {  \
	TEXTCHAR myname[256];HMODULE mod;GetModuleFileName( NULL, (LPSTR)myname, sizeof( myname ) );\
	mod=LoadLibrary((LPSTR)myname);if(mod){        \
   void(*rsp)(void); \
	if((rsp=((void(*)(void))(GetProcAddress( mod, default_name_for_deadstart_runner))))){rsp();}else{lprintf( WIDE("Hey failed to get proc %d"), GetLastError() );}\
	FreeLibrary( mod);  } \
	} while(0)
#endif

#ifdef __old_deadstart
#define DispelDeadstart() do {  \
	void *hMe = dlopen(NULL, RTLD_LAZY );   \
	if(hMe){ void (*f)(void)=(void(*)(void))dlsym( hMe,"ClearDeadstarts" ); if(f)f(); dlclose(hMe); }\
	} while(0)
#endif

//------------------------------------------------------------------------------------
// WIN32 MSVC
//------------------------------------------------------------------------------------
#elif defined( _MSC_VER )

//#define PRELOAD(name) __declspec(allocate(".CRT$XCAA")) void CPROC name(void)

//#pragma section(".CRT$XCA",long,read)
//#pragma section(".CRT$XCZ",long,read)

// put init in both C startup and C++ startup list...
// looks like only one or the other is invoked, not both?


/////// also the variables to be put into these segments
#if defined( __cplusplus_cli )
#if defined( __cplusplus_cli )
#define LOG_ERROR(n) System::Console::WriteLine( gcnew System::String(n) + gcnew System::String( myname) ) )
#else
#define LOG_ERROR(n) SystemLog( n )
#endif


#else
#define _STARTSEG_ ".CRT$XIZ"
#define _STARTSEG2_ ".CRT$XCZ"
#define _ENDSEG_ ".CRT$YIZ"

#pragma data_seg(".CRT$XIA")
#pragma data_seg(".CRT$XIZ")
#pragma data_seg(".CRT$XCZ")
#pragma data_seg(".CRT$YCZ")
#pragma data_seg(".CRT$YIZ")
#pragma data_seg()

#define pastejunk_(a,b) a##b
#define pastejunk(a,b) pastejunk_(a,b)

#define PRIORITY_PRELOAD(name,priority) static void name(void); \
	static int schedule_##name(void) {                 \
	RegisterPriorityStartupProc( name,#name,priority,0/*chance to use label to reference*/,__FILE__,__LINE__ );\
	return 0; \
	}                                       \
	/*static __declspec(allocate(_STARTSEG_)) void (CPROC*pointer_##name)(void) = schedule_##name;*/ \
	/*static */__declspec(allocate(_STARTSEG_)) void (CPROC*pastejunk(unique_name,pastejunk( x_##name,__LINE__)))(void) = (void(CPROC*)(void))schedule_##name; \
	static void name(void)

/*
#define ATEXIT_PRIORITY(name,priority) static void schedule_exit_##name(void); static void name(void); \
	static struct rt_init __based(__segname("XI")) name##_dtor_label={0,69,schedule_exit_##name}; \
	static void schedule_exit_##name(void) {                                              \
	RegisterPriorityShutdownProc( name,#name,priority,&name##_dtor_label,__FILE__,__LINE__ );\
	}                                       \
*/

#define ROOT_ATEXIT(name) static void name(void); \
	__declspec(allocate(_ENDSEG_)) static void (*f##name)(void)=name; \
   static void name(void)
#define ATEXIT(name) static void name(void); \
	__declspec(allocate(_ENDSEG_)) static void (*ae##name)(void)=name; \
   static void name(void)

typedef void(*atexit_priority_proc)(void (*)(void),int,CTEXTSTR,CTEXTSTR,int);
#define PRIORITY_ATEXIT(name,priority) static void name(void); \
	static void (atexit##name)(void);\
   __declspec(allocate(_ENDSEG_)) static void (*atexit##name##ptr)(void)=atexit##name;;  \
 	void atexit_failed##name(void(*f)(void),int i,CTEXTSTR s1,CTEXTSTR s2,int n) { lprintf( WIDE("Failed to load atexit_priority registerar from core program.") );} \
void atexit##name(void)                                                  \
{                                                                        \
	atexit_priority_proc f;                                               \
   HMODULE hMod;                                                         \
	/*lprintf( WIDE("Do pr_atexit %s"), #name ); */                             \
	f=(atexit_priority_proc)GetProcAddress( hMod = LoadLibrary( NULL )    \
										  , WIDE("RegisterShutdownProc") );              \
	if(f) f(name,priority,#name,__FILE__,__LINE__);                          \
	else atexit_failed##name(name,priority,#name,__FILE__,__LINE__);        \
   FreeLibrary(hMod); /*optional*/                                         \
}                                                                          \
void name(void)
#define ATEXIT_PRIORITY(name,priority) PRIORITY_ATEXIT(name,priority)
#endif
#ifdef __cplusplus_cli
#define InvokeDeadstart() do {                                              \
	TEXTCHAR myname[256];HMODULE mod; \
	mod=LoadLibrary("sack_bag.dll");if(mod){        \
   void(*rsp)(void); \
	if((rsp=((void(*)(void))(GetProcAddress( mod, "RunDeadstart"))))){rsp();}else{lprintf( WIDE("Hey failed to get proc %d"), GetLastError() );}\
	FreeLibrary( mod); }} while(0)
#else
#endif
#define PRELOAD(name) PRIORITY_PRELOAD(name,DEFAULT_PRELOAD_PRIORITY)

//extern _32 deadstart_complete;
//#define DEADSTART_LINK _32 *deadstart_link_couple = &deadstart_complete; // make sure we reference this symbol
//#pragma data_seg(".CRT$XCAA")
//extern void __cdecl __security_init_cookie(void);
//static _CRTALLOC(".CRT$XCAA") _PVFV init_cookie = __security_init_cookie;
//#pragma data_seg()

//------------------------------------------------------------------------------------
// UNDEFINED
//------------------------------------------------------------------------------------
#else
#error "there's nothing I can do to wrap PRELOAD() or ATEXIT()!"
#define PRELOAD(name)
#endif

#include <exit_priorities.h>

SACK_DEADSTART_NAMESPACE_END
USE_SACK_DEADSTART_NAMESPACE
#endif
