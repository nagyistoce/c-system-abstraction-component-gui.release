//### debugging break of compilation, if this is on, I'm sorry.
//#define FIX_BROKEN_TYPECASTS
#ifdef FIX_BROKEN_TYPECASTS
#warning haha err
#endif

#ifdef _MSC_VER
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#ifndef __WINDOWS__
#define __WINDOWS__
#endif
#endif

#ifdef __cplusplus_cli
// these things define a type called 'Byte' 
// which causes confusion... so don't include vcclr for those guys.
#ifdef SACK_BAG_EXPORTS
// maybe only do this while building sack_bag project itself...
#if !defined( ZCONF_H ) \
	&& !defined( __FT2_BUILD_GENERIC_H__ ) \
	&& !defined( ZUTIL_H ) \
	&& !defined( SQLITE_PRIVATE ) \
	&& !defined( NETSERVICE_SOURCE ) \
	&& !defined( LIBRARY_DEF )
#include <vcclr.h>
using namespace System;
#endif
#endif
#endif

#ifdef SACK_BAG_EXPORTS
// exports don't really matter with CLI compilation.
#  ifndef BAG
#    ifdef SACK_COM_OBJECT
//#include "stdafx.h"
#      ifndef UNICODE
//#error WHY WASN'T UNICODE DONE?!
#        define UNICODE
#      endif
#    endif
#    ifdef UNICODE
#      define XML_UNICODE_WCHAR_T
#      define XML_UNICODE
#    endif
#ifndef TARGETNAME
#  define TARGETNAME "sack_bag.dll" //$(TargetFileName)
#endif
#ifndef __cplusplus_cli 
// cli mode, we use this directly, and build the exports in sack_bag.dll directly
#else
#define LIBRARY_DEADSTART
#endif
#define MEM_LIBRARY_SOURCE
#define SYSLOG_SOURCE
#define _TYPELIBRARY_SOURCE
#define TIMER_SOURCE
#define IDLE_SOURCE
#define CLIENTMSG_SOURCE
#define PROCREG_SOURCE
#define FRACTION_SOURCE
#define NETWORK_SOURCE
#define CONFIGURATION_LIBRARY_SOURCE
#define FILESYSTEM_LIBRARY_SOURCE
#define SYSTEM_SOURCE
#define FILEMONITOR_SOURCE
#define VECTOR_LIBRARY_SOURCE
#define SHA1_SOURCE
#define JPEG_SOURCE
#define __PNG_LIBRARY_SOURCE__
#define GENX_SOURCE
#define SEXPAT_SOURCE
#define CONSTRUCT_SOURCE
#define SQLPROXY_LIBRARY_SOURCE
#define FREETYPE_SOURCE
#define FT2_BUILD_LIBRARY
//MSVC#define WIN32
#define BAG

//MSVC//#define WIN32
//MSVC//#define _DEBUG
//MSVC#define _WINDOWS
//MSVC#define _USRDLL
#define SQLGETOPTION_SOURCE
#define BAGIMAGE_EXPORTS
#define IMAGE_LIBRARY_SOURCE
#define SYSTRAY_LIBRARAY

#define PSI_SOURCE
#define SOURCE_PSI2

#define VIDEO_LIBRARY_SOURCE
#define RENDER_LIBRARY_SOURCE
#define MNG_BUILD_DLL
#ifndef __NO_WIN32API__
#define _OPENGL_ENABLED
#endif
#define SQLITE_SOURCE
// define a type that is a public name struct type... 
// good thing that typedef and struct were split
// during the process of port to /clr option.
//#define PUBLIC_TYPE public
#else
//#define PUBLIC_TYPE
#ifdef __cplusplus_CLR
#include <vcclr.h>
using namespace System;
#endif

#ifdef __CYGWIN__
#include <wchar.h> // wchar for X_16 definition
#endif
#ifdef _MSC_VER
#include <sys/stat.h>
#endif


#endif
#endif

#ifdef SACK_BAG_DEFINED
// use this to override default export of 'image' and 'render' interfaces registered.
#define SACK_BAG_EXPORTS
#endif



#ifndef MY_TYPES_INCLUDED
#define MY_TYPES_INCLUDED
// include this before anything else
// thereby allowing us to redefine exit()
#include <limits.h> // CHAR_BIT
#include <stdlib.h>
#include <stdarg.h> // typelib requires this
#include <string.h> // typelib requires this
#if !defined( __WINDOWS__ ) && !defined( _WIN32 )
#include <dlfcn.h>
#endif
#define LOG_LIBRARY_ENTER 

#if defined( _MSC_VER )
// disable pointer conversion warnings - wish I could disable this
// according to types...
#pragma warning( disable:4312; disable:4311 )
// disable deprication warnings of snprintf, et al.
#pragma warning( disable:4996 ) 
#define EMPTY_STRUCT struct { char nothing[]; }
#endif
#if defined( __WATCOMC__ )
#define EMPTY_STRUCT char
#endif

#ifdef __cplusplus
//#error CPLUSPLUS! yay.
// could also consider defining 'SACK_NAMESPACE' as 'extern "C"' {' and '..._END' as '}'
#define SACK_NAMESPACE namespace sack {
#define SACK_NAMESPACE_END }
#define _CONTAINER_NAMESPACE namespace containers {
#define _CONTAINER_NAMESPACE_END }
#define _LINKLIST_NAMESPACE namespace list {
#define _LINKLIST_NAMESPACE_END }
#else
#define SACK_NAMESPACE
#define SACK_NAMESPACE_END
#define _CONTAINER_NAMESPACE
#define _CONTAINER_NAMESPACE_END
#define _LINKLIST_NAMESPACE
#define _LINKLIST_NAMESPACE_END
#endif

#define SACK_CONTAINER_NAMESPACE SACK_NAMESPACE _CONTAINER_NAMESPACE
#define SACK_CONTAINER_NAMESPACE_END _CONTAINER_NAMESPACE_END SACK_NAMESPACE_END
#define SACK_CONTAINER_LINKLIST_NAMESPACE SACK_CONTAINER_NAMESPACE _LISTLIST_NAMESPACE
#define SACK_CONTAINER_LINKLIST_NAMESPACE_END _LISTLIST_NAMESPACE_END SACK_CONTAINER_NAMESPACE

// this symbols is defined to enforce
// the C Procedure standard - using a stack, and resulting
// in EDX:EAX etc...
#define CPROC

#ifdef SACK_BAG_EXPORTS
# ifdef BUILD_GLUE
#  define EXPORT_METHOD [DllImport(LibName)] public
# else
#  ifdef __cplusplus_cli
#   define EXPORT_METHOD __declspec(dllexport)
#   define IMPORT_METHOD __declspec(dllimport)
#  define LITERAL_LIB_EXPORT_METHOD __declspec(dllexport)
#  define LITERAL_LIB_IMPORT_METHOD extern
//__declspec(dllimport)
#  else
#   define EXPORT_METHOD __declspec(dllexport)
#   define IMPORT_METHOD __declspec(dllimport)
#  define LITERAL_LIB_EXPORT_METHOD __declspec(dllexport)
#  define LITERAL_LIB_IMPORT_METHOD __declspec(dllimport)
#  endif
# endif
#else
# if ( !defined( __STATIC__ ) && defined( __WINDOWS__ ) && !defined( __cplusplus_cli) )
#  define EXPORT_METHOD __declspec(dllexport)
#  define IMPORT_METHOD __declspec(dllimport)
#  define LITERAL_LIB_EXPORT_METHOD __declspec(dllexport)
#  define LITERAL_LIB_IMPORT_METHOD __declspec(dllimport)
# else
#  if defined( __LINUX__ ) || defined( __STATIC__ ) && !defined( __cplusplus_cli )
#    define EXPORT_METHOD
#    define IMPORT_METHOD extern
#    define LITERAL_LIB_EXPORT_METHOD 
#    define LITERAL_LIB_IMPORT_METHOD extern
#  else
#    define EXPORT_METHOD __declspec(dllexport)
#    define IMPORT_METHOD __declspec(dllimport)
#    define LITERAL_LIB_EXPORT_METHOD __declspec(dllexport)
#    define LITERAL_LIB_IMPORT_METHOD __declspec(dllimport)
#  endif
# endif
#endif
// used when the keword specifying a structure is packed
// needs to prefix the struct keyword.
#define PREFIX_PACKED 

#define my_offsetof( ppstruc, member ) ((PTRSZVAL)&((*ppstruc)->member)) - ((PTRSZVAL)(*ppstruc))

#ifndef WIN32
#ifdef _WIN32
#define WIN32 _WIN32
#endif
#endif

// okay move this out, tired of recompiling the world... only those things that use
// deadstart will include PRELOAD() functionality (for now)
//#include <deadstart.h>

SACK_NAMESPACE


#ifndef __LINUX__
typedef int pid_t;
#endif

#ifdef BCC16
#define __inline__
#define MAINPROC(type,name)     type _pascal name
// winproc is intended for use at libmain/wep/winmain...
#define WINPROC(type,name)      type _far _pascal _export name
// callbackproc is for things like timers, dlgprocs, wndprocs...
#define CALLBACKPROC(type,name) type _far _pascal _export name
#define PUBLIC(type,name)       type STDPROC _export name
#ifdef LOG_LIBRARY_ENTER
#define LIBMAIN() WINPROC(int, LibMain)(HINSTANCE hInstance, WORD wDataSeg, WORD wHeapSize, LPSTR lpCmdLine ) \
		{ /*Log( WIDE("Library Enter") );*//* here would be if dwReason == process_attach */ {
#define LIBEXIT() } /* end if */ } /*endproc*/ \
	   int STDPROC WEP(int nSystemExit )  { /*Log( WIDE("Library Exit")*/ );
#else
#define LIBMAIN() WINPROC(int, LibMain)(HINSTANCE hInstance, WORD wDataSeg, WORD wHeapSize, LPSTR lpCmdLine ) \
		{ /* here would be if dwReason == process_attach */ {
#define LIBEXIT() } /* end if */ } /*endproc*/ \
	   int STDPROC WEP(int nSystemExit )  { 
#endif
#define LIBMAIN_END()  }

// should use this define for all local defines...
// this will allow one place to modify ALL _pascal or not to _pascal decls.
#define STDPROC _far _pascal

#endif

#if defined( __LCC__ ) || defined( _MSC_VER ) || defined(__DMC__) || defined( __WATCOMC__ )
#ifdef __WATCOMC__
#undef CPROC
#define CPROC __cdecl
#define STDPROC __cdecl
#ifndef __WATCOMC__
// watcom windef.h headers define this
#define STDCALL _stdcall
#endif
#if __WATCOMC__ >= 1280
// watcom windef.h headers no longer define this.
#define STDCALL __stdcall
#endif
#undef PREFIX_PACKED
#define PREFIX_PACKED _Packed
#else
#undef CPROC
//#error blah
#define CPROC __cdecl
#define STDPROC
#define STDCALL _stdcall
#endif
#ifndef MAXPATH
#define MAXPATH MAX_PATH
#endif
#define far 
#define huge
#define near
#define _far
#define _huge
#define _near
#define __far
#ifndef FAR
#define FAR
#endif
//#define HUGE
//#ifndef NEAR
//#define NEAR
//#endif
#define _fastcall
#define DYNAMIC_EXPORT EXPORT_METHOD
#define DYNAMIC_IMPORT __declspec(dllimport)
#ifdef __cplusplus
#ifdef __cplusplus_cli
#define PUBLIC(type,name) extern "C" LITERAL_LIB_EXPORT_METHOD type CPROC name
#else
//#error what the hell!?
// okay Public functions are meant to be loaded with LoadFuncion( "library", "function name" );
#define PUBLIC(type,name) extern "C" LITERAL_LIB_EXPORT_METHOD type CPROC name
#endif
#else
#define PUBLIC(type,name) LITERAL_LIB_EXPORT_METHOD type CPROC name
#endif
#define MAINPROC(type,name)  type WINAPI name
#define WINPROC(type,name)   type WINAPI name
#define CALLBACKPROC(type,name) type CALLBACK name

#if defined( __WATCOMC__ )
#define LIBMAIN()   static int __LibMain( HINSTANCE ); PRELOAD( LibraryInitializer ) { \
	__LibMain( GetModuleHandle(TARGETNAME) );   } \
	static int __LibMain( HINSTANCE hInstance ) { /*Log( WIDE("Library Enter") );*/
#define LIBEXIT() } static int LibExit( void ); ATEXIT( LiraryUninitializer ) { LibExit(); } int LibExit(void) { /*Log( WIDE("Library Exit") );*/
#define LIBMAIN_END() }
#else
#ifdef TARGETNAME
#define LIBMAIN()   static int __LibMain( HINSTANCE ); PRELOAD( LibraryInitializer ) { \
	__LibMain( GetModuleHandle(TARGETNAME) );   } \
	static int __LibMain( HINSTANCE hInstance ) { /*Log( WIDE("Library Enter") );*/
#else
#define LIBMAIN()   static int __LibMain( HINSTANCE ); PRELOAD( LibraryInitializer ) { \
	__LibMain( GetModuleHandle (NULL) );   } \
	static int __LibMain( HINSTANCE hInstance ) { /*Log( WIDE("Library Enter") );*/
#endif
#define LIBEXIT() } static int LibExit( void ); ATEXIT( LiraryUninitializer ) { LibExit(); } int LibExit(void) { /*Log( WIDE("Library Exit") );*/
#define LIBMAIN_END() }
#if 0
#define LIBMAIN() EXPORT_METHOD WINPROC(int, DllMain)(HINSTANCE hInstance, DWORD dwReason, void *unused ) \
		{ if( dwReason == DLL_PROCESS_ATTACH ) {\
			/*Log( WIDE("Library Enter") );*//* here would be if dwReason == process_attach */
#define LIBEXIT() } /* end if */ if( dwReason == DLL_PROCESS_DETACH ) {  \
	   									 /*Log( WIDE("Library Exit") );*/
#define LIBMAIN_END()  } return 1; }
#endif
#endif
#define PACKED
#endif

#if defined( __LINUX__ ) || defined( __CYGWIN__ )
#define STDPROC
#define STDCALL // for IsBadCodePtr which isn't a linux function...
#ifndef __CYGWIN__
#define WINAPI
#define PASCAL
#endif
#define CALLBACKPROC( type, name ) type name
#define DYNAMIC_EXPORT
#define DYNAMIC_IMPORT extern
#define PUBLIC(type,name) type name
#define LIBMAIN() static int LibraryEntrance( void ) __attribute__((constructor)); static int LibraryEntrance( void )
#define LIBEXIT() static int LibraryExit( void )     __attribute__((destructor)); static int LibraryExit( void )
#define LIBMAIN_END()
#define FAR
#define NEAR
//#define HUGE
#define far
#define near
#define huge
#define PACKED __attribute__((packed))
#endif

#if defined( BCC32 )
#define far 
#define huge
#define near
#define _far
#define _huge
#define _near
#define __inline__
#define MAINPROC(type,name)     type _pascal name
// winproc is intended for use at libmain/wep/winmain...
#define WINPROC(type,name)      EXPORT_METHOD type _pascal name
// callbackproc is for things like timers, dlgprocs, wndprocs...
#define CALLBACKPROC(type,name) EXPORT_METHOD type _stdcall name
#define STDCALL _stdcall
#define PUBLIC(type,name)        type STDPROC name
#ifdef __STATIC__
#define LIBMAIN() static WINPROC(int, LibMain)(HINSTANCE hInstance, DWORD dwReason, void *unused ) \
		{ if( dwReason == DLL_PROCESS_ATTACH ) {\
			/*Log( WIDE("Library Enter") );*//* here would be if dwReason == process_attach */
#define LIBEXIT() } /* end if */ if( dwReason == DLL_PROCESS_DETACH ) {  \
	  									 /*Log( WIDE("Library Exit") );*/
#define LIBMAIN_END()  } return 1; }
#else
#define LIBMAIN() WINPROC(int, LibMain)(HINSTANCE hInstance, DWORD dwReason, void *unused ) \
		{ if( dwReason == DLL_PROCESS_ATTACH ) {\
			/*Log( WIDE("Library Enter") );*//* here would be if dwReason == process_attach */
#define LIBEXIT() } /* end if */ if( dwReason == DLL_PROCESS_DETACH ) {  \
	   									 /*Log( WIDE("Library Exit") );*/
#define LIBMAIN_END()  } return 1; }
#endif

// should use this define for all local defines...
// this will allow one place to modify ALL _pascal or not to _pascal decls.
#define STDPROC _pascal
#define PACKED
#endif

#define TOCHR(n) #n[0]
#define TOSTR(n) WIDE(#n)
#define STRSYM(n) TOSTR(n)
#define FILELINE  TEXT(__FILE__) WIDE("(") TEXT(STRSYM(__LINE__))WIDE(") : ")
#if defined( _MSC_VER ) || defined( __PPCCPP__ )
#define pragnote(msg) message( FILELINE msg )
#define pragnoteonly(msg) message( msg )
#else
#define pragnote(msg) msg
#define pragnoteonly(msg) msg
#endif

#define FILELINE_SRC         , (CTEXTSTR)_WIDE(__FILE__), __LINE__
#define FILELINE_VOIDSRC     (CTEXTSTR)_WIDE(__FILE__), __LINE__ 
#define FILELINE_LEADSRC     (CTEXTSTR)_WIDE(__FILE__), __LINE__, 
#define FILELINE_VOIDPASS    CTEXTSTR pFile, _32 nLine
#define FILELINE_LEADPASS    CTEXTSTR pFile, _32 nLine, 
#define FILELINE_PASS        , CTEXTSTR pFile, _32 nLine
#define FILELINE_RELAY       , pFile, nLine
#define FILELINE_VOIDRELAY   pFile, nLine
#define FILELINE_FILELINEFMT WIDE("%s(%") _32f WIDE("): ")
#define FILELINE_NULL        , NULL, 0
#define FILELINE_VOIDNULL    NULL, 0
#define FILELINE_VARSRC       CTEXTSTR pFile = _WIDE(__FILE__); _32 nLine = __LINE__

// this is for passing FILE, LINE information to allocate
// useful during DEBUG phases only...

// drop out these debug relay paramters for managed code...
// we're going to have the full call frame managed and known...
#if defined( _DEBUG ) //&& !defined( __NO_WIN32API__ ) 
	// these DBG_ formats are commented out from duplication in sharemem.h
#if defined( __LINUX__ ) && !defined( __PPCCPP__ )
//#warning "Setting DBG_PASS and DBG_FORWARD to work."
#else
//#pragma pragnoteonly("Setting DBG_PASS and DBG_FORWARD to work" )
#endif
#define DBG_AVAILABLE   1
#define DBG_SRC         FILELINE_SRC
#define DBG_VOIDSRC     FILELINE_VOIDSRC
#define DBG_LEADSRC     FILELINE_LEADSRC
#define DBG_VOIDPASS    FILELINE_VOIDPASS
#define DBG_LEADPASS    FILELINE_LEADPASS
#define DBG_PASS        FILELINE_PASS
#define DBG_RELAY       FILELINE_RELAY
#define DBG_VOIDRELAY   FILELINE_VOIDRELAY
#define DBG_FILELINEFMT FILELINE_FILELINEFMT
#define DBG_VARSRC      FILELINE_VARSRC
#else
#if defined( __LINUX__ ) && !defined( __PPCCPP__ )
//#warning "Setting DBG_PASS and DBG_FORWARD to be ignored."
#else
//#pragma pragnoteonly("Setting DBG_PASS and DBG_FORWARD to be ignored" )
#endif
#define DBG_AVAILABLE   0
#define DBG_SRC 
#define DBG_VOIDSRC     
#define DBG_LEADSRC
#define DBG_VOIDPASS    void
#define DBG_LEADPASS   
#define DBG_PASS
#define DBG_RELAY
#define DBG_VOIDRELAY
#define DBG_FILELINEFMT
#define DBG_VARSRC      
#endif


	// cannot declare _0 since that overloads the
	// vector library definition for origin (0,0,0,0,...)
//typedef void             _0; // totally unusable to declare 0 size things.
#ifdef __NO_WIN32API__
#define P_0 void*
#ifdef __cplusplus_cli
// long is 32, int is 64
#define _32 unsigned long
#else
#define _32 unsigned int
#endif
#define  _8   unsigned char      
#define  P_8   _8               *
#define  _16   unsigned short    
#define  P_16   _16              *
#define  P_32  _32             *
#define  PC_32  const _32      *
#define  S_8 signed   char     
#define  PS_8 S_8             *
#define  S_16 signed   short   
#define  PS_16 S_16           *
#define  S_32 signed   long    
#define  PS_32 S_32           *
#define  X_8 char              
#define  PX_8 char            *
#else
typedef void             *P_0;
typedef unsigned char      _8;
typedef _8               *P_8;
typedef unsigned short    _16;
typedef _16             *P_16;
#if defined( __LINUX64__ )
typedef unsigned int      _32;
#elif defined( __WATCOMC__ ) || (1)
typedef unsigned long      _32;
#endif
typedef _32             *P_32;
typedef volatile _32             *PV_32;
typedef const _32      *PC_32;
typedef signed   char     S_8;
typedef S_8             *PS_8;
typedef signed   short   S_16;
typedef S_16           *PS_16;
#ifdef __LINUX64__
typedef signed   int     S_32;
#else
typedef signed   long    S_32;
#endif
typedef S_32           *PS_32;
typedef char              X_8;
typedef char            *PX_8;
#endif
/*
 * several compilers are rather picky about the types of data
 * used for bit field declaration, therefore this type
 * should be used instead of _32
 */
typedef unsigned int  BIT_FIELD;
// have to do this on a per structure basis - otherwise
// any included headers with structures to use will get FUCKED
#ifndef PACKED
#warning NO PREVIOUS deintion of PACKED...
#define PACKED
#endif

#if defined( BCC16 )
#pragma warning _64 bit types are not supported... using _32
typedef unsigned long _64;
typedef long S_64;
#elif defined( GCC ) || defined( __LCC__ ) || defined( __WATCOMC__ ) || defined( __DMC__ )
typedef unsigned  long long   _64;
typedef signed  long long   S_64;
#elif defined( BCC32 ) || defined( __BORLAND__ ) || defined( _MSC_VER )
typedef unsigned  __int64  _64;
typedef signed  __int64      S_64;
#else
#error unknown declaration for _64 type.
#endif
typedef _64              *P_64;
typedef S_64          *PS_64;
#if defined( BCC16 ) 
//#pragma warning "Setting PTRSZVAL to 32 bits... pointers are this size?"
#else 
# if defined( __LCC__ ) || defined( GCC ) && !defined( __PPCCPP__ )
//#  warning "Setting PTRSZVAL to 32 bits... pointers are this size?"
# else
//#  pragma pragnoteonly("Setting PTRSZVAL to 32 bits... pointers are this size?" )
# endif 
#endif
#if defined( __LINUX64__ )
typedef _64             PTRSIZEVAL;
typedef _64             PTRSZVAL;
#else
typedef _32             PTRSIZEVAL;
typedef _32             PTRSZVAL;
#endif

typedef volatile _64  *PV_64;
typedef volatile PTRSZVAL        *PVPTRSZVAL;

typedef _32             INDEX;
#define INVALID_INDEX ((INDEX)-1) 

#ifdef __CYGWIN__
typedef unsigned short wchar_t;
#endif
// may consider changing this to P_16 for unicode...
typedef wchar_t X_16;
typedef wchar_t *PX_16;
#if defined( UNICODE ) || defined( SACK_COM_OBJECT )
//should also consider revisiting code that was updated for TEXTCHAR to char conversion methods...
#ifdef _MSC_VER
#define NULTERM __nullterminated
#else
#define NULTERM
#endif
#define WIDE(s)  L##s
#define _WIDE(s)  WIDE(s)
#define cWIDE(s)  s
#define _cWIDE(s)  cWIDE(s)
typedef NULTERM          const X_16      *CTEXTSTR; // constant text string content
typedef NULTERM          CTEXTSTR        *PCTEXTSTR; // pointer to constant text string content
typedef NULTERM          PX_16            TEXTSTR;  
typedef X_16             TEXTCHAR;

#else
#define WIDE(s)   s 
#define _WIDE(s)  s
#define cWIDE(s)   s 
#define _cWIDE(s)  s
typedef const X_8     *CTEXTSTR; // constant text string content
typedef PX_8            TEXTSTR;
#if defined( __LINUX__ ) && defined( __cplusplus )
typedef TEXTSTR const *PCTEXTSTR; // pointer to constant text string content
#else
// char const *const *
typedef CTEXTSTR const *PCTEXTSTR;
#endif
typedef X_8             TEXTCHAR;
#endif

//typedef enum { FALSE, TRUE } LOGICAL; // smallest information
#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif
typedef _32 LOGICAL;

typedef P_0 POINTER;
typedef const void *CPOINTER;

//------------------------------------------------------
// formatting macro defintions for [vsf]printf output of the above types

#ifdef __LINUX64__
#define _32f   WIDE("u")
#define _32fx   WIDE("x")
#define _32fX   WIDE("X")
#define _32fs   WIDE("d")
#define PTRSZVALfs WIDE("llu")
#define PTRSZVALfx WIDE("llx")
#define c_32f   "u"
#define c_32fx   "x"
#define c_32fX   "X"
#define c_32fs   "d"
#define cPTRSZVALfs "llu"
#define cPTRSZVALfx "llx"
#else
#define _32f   WIDE("lu")
#define _32fx   WIDE("lx")
#define _32fX   WIDE("lX")
#define _32fs   WIDE("ld")
#define PTRSZVALfs WIDE("lu")
#define PTRSZVALfx WIDE("lx")
#define c_32f   "lu"
#define c_32fx   "lx"
#define c_32fX   "lX"
#define c_32fs   "ld"
#define cPTRSZVALfs "lu"
#define cPTRSZVALfx "lx"
#endif

#define PTRSZVALf WIDE("p")

#if defined( __WATCOMC__ ) //|| defined( _MSC_VER )
#define _64f    WIDE("Lu")
#define _64fx   WIDE("Lx")
#define _64fX   WIDE("LX")
#define _64fs   WIDE("Ld")
#else
#define _64f    WIDE("llu")
#define _64fx   WIDE("llx")
#define _64fX   WIDE("llX")
#define _64fs   WIDE("lld")
#endif



// This should be for several years a
// sufficiently large type to represent
// threads and processes.
typedef _64 THREAD_ID;
#define GetMyThreadIDNL GetMyThreadID
#if defined( _WIN32 ) || defined( __CYGWIN__ )
#define GetMyThreadID()  ( (( ((_64)GetCurrentProcessId()) << 32 ) | ( (_64)GetCurrentThreadId() ) ) )
#else
// this is now always the case
// it's a safer solution anyhow...
#ifndef GETPID_RETURNS_PPID
#define GETPID_RETURNS_PPID
#endif
#ifdef GETPID_RETURNS_PPID
#define GetMyThreadID()  (( ((_64)getpid()) << 32 ) | ( (_64)((pthread_self())) ) )
#else
#define GetMyThreadID()  (( ((_64)getppid()) << 32 ) | ( (_64)(getpid()|0x40000000)) )
#endif
#endif

//#error blah
// general macros for linking lists using
#define DeclareLink( type )  type *next;type **me

#define RelinkThing( root, node )   \
	((( node->me && ( (*node->me)=node->next ) )?  \
	node->next->me = node->me:0),(node->next = NULL),(node->me = NULL),node), \
	((( node->next = root )?        \
	(root->me = &node->next):0),  \
	(node->me = &root),             \
	(root = node) )

#define LinkThing( root, node )     \
		((( (node)->next = (root) )?        \
	(((root)->me) = &((node)->next)):0),  \
	(((node)->me) = &(root)),             \
	((root) = (node)) )

#define LinkLast( root, type, node ) if( node ) do { if( !root ) \
	{ root = node; (node)->me=&root; } \
	else { type tmp; \
	for( tmp = root; tmp->next; tmp = tmp->next ); \
	tmp->next = (node); \
	(node)->me = &tmp->next; \
	} } while (0)


// put 'Thing' after 'node'
#define LinkThingAfter( node, thing ) \
	( ( (thing)&&(node))   \
	?(((((thing)->next = (node)->next))?((node)->next->me = &(thing)->next):0) \
	 ,((thing)->me = &(node)->next), ((node)->next = thing))  \
	:((node)=(thing)) )


//
// put 'Thing' before 'node'... so (*node->me) = thing
#define LinkThingBefore( node, thing ) \
	{  \
thing->next = (*node->me);\
	(*node->me) = thing;    \
thing->me = node->me;       \
node->me = &thing->next;     \
}

#define UnlinkThing( node )                      \
	((( (node) && (node)->me && ( (*(node)->me)=(node)->next ) )?  \
	(node)->next->me = (node)->me:0),((node)->next = NULL),((node)->me = NULL),(node))

// this has two expressions duplicated...
// but in being so safe in this expression,
// the self-circular link needs to be duplicated.
// GrabThing is used for nodes which are circularly bound
#define GrabThing( node )    \
	((node)?(((node)->me)?(((*(node)->me)=(node)->next)? \
	((node)->next->me=(node)->me),((node)->me=&(node)->next):NULL):((node)->me=&(node)->next)):NULL)

#define NextLink(node) ((node)?(node)->next:NULL)
// everything else is called a thing... should probably migrate to using this...
#define NextThing(node) ((node)?(node)->next:NULL)
//#ifndef FALSE
//#define FALSE 0
//#endif

//#ifndef TRUE
//#define TRUE (!FALSE)
//#endif


#define FLAGSETTYPE _32
#define FLAGTYPEBITS(t) (sizeof(t)*CHAR_BIT)
#define FLAGROUND(t) (FLAGTYPEBITS(t)-1)
#define FLAGTYPE_INDEX(t,n)  (((n)+FLAGROUND(t))/FLAGTYPEBITS(t))
#define FLAGSETSIZE(t,n) (FLAGTYPE_INDEX(t,n) * sizeof( FLAGSETTYPE ) )
// declare a set of flags...
#define FLAGSET(v,n)   FLAGSETTYPE (v)[((n)+FLAGROUND(FLAGSETTYPE))/FLAGTYPEBITS(FLAGSETTYPE)]
// set a single flag index
#define SETFLAG(v,n)   ( (v)[(n)/FLAGTYPEBITS((v)[0])] |= 1 << ( (n) & FLAGROUND((v)[0]) ))
// clear a single flag index
#define RESETFLAG(v,n) ( (v)[(n)/FLAGTYPEBITS((v)[0])] &= ~( 1 << ( (n) & FLAGROUND((v)[0]) ) ) )
// test if a flags is set
#define TESTFLAG(v,n)  ( (v)[(n)/FLAGTYPEBITS((v)[0])] & ( 1 << ( (n) & FLAGROUND((v)[0]) ) ) )
// reverse a flag from 1 to 0 and vice versa
#define TOGGLEFLAG(v,n)   ( (v)[(n)/FLAGTYPEBITS((v)[0])] ^= 1 << ( (n) & FLAGROUND((v)[0]) ))

// 32 bits max for range on mask
#define MASK_MAX_LENGTH 32
#define MASKSET_READTYPE _32 // gives a 32 bit mask possible from flagset..
#define MASKSETTYPE _8  // gives byte index...
#define MASKTYPEBITS(t) (sizeof(t)*CHAR_BIT)
#define MASK_MAX_TYPEBITS(t) (sizeof(t)*CHAR_BIT)
#define MASKROUND(t) (MASKTYPEBITS(t)-1)
#define MASK_MAX_ROUND() (MASK_MAX_TYPEBITS(MASKSET_READTYPE)-1)
#define MASKTYPE_INDEX(t,n)  (((n)+MASKROUND(t))/MASKTYPEBITS(t))
#define MASKSETSIZE(t,n,range) (MASKTYPE_INDEX(t,n))
// declare a set of flags...

#define MASK_TOP_MASK_VAL(length,val) ((val)&( (0xFFFFFFFFUL) >> (32-(length)) ))
#define MASK_TOP_MASK(length) ( (0xFFFFFFFFUL) >> (32-(length)) )
#define MASK_MASK(n,length)   (MASK_TOP_MASK(length) << (((n)*(length))&0x7) )
// masks value with the mask size, then applies that mask back to the correct word indexing
#define MASK_MASK_VAL(n,length,val)   (MASK_TOP_MASK_VAL(length,val) << (((n)*(length))&0x7) )

#define MASKSET(v,n,r)  MASKSETTYPE  (v)[(((n)*(r))+MASK_MAX_ROUND())/MASKTYPEBITS(MASKSETTYPE)]; const int v##_mask_size = r;
// set a single flag index
#define SETMASK(v,n,val)    (((MASKSET_READTYPE*)((v)+((n)*(v##_mask_size))/MASKTYPEBITS((v)[0])))[0] =    \
( ((MASKSET_READTYPE*)((v)+((n)*(v##_mask_size))/MASKTYPEBITS(_8)))[0]                                 \
 & (~(MASK_MASK(n,v##_mask_size))) )                                                                           \
	| MASK_MASK_VAL(n,v##_mask_size,val) )
// clear a single flag index
#define GETMASK(v,n)  ( ( ((MASKSET_READTYPE*)((v)+((n)*(v##_mask_size))/MASKTYPEBITS((v)[0])))[0]                                 \
 & MASK_MASK(n,v##_mask_size) )                                                                           \
	>> (((n)*(v##_mask_size))&0x7))



_CONTAINER_NAMESPACE
#define DECLDATA(name,sz) struct {PTRSZVAL size; TEXTCHAR data[sz];} name

// Hmm - this can be done with MemLib alone...
// although this library is not nessecarily part of that?
// and it's not nessecarily allocated.
typedef struct SimpleDataBlock {
   PTRSZVAL size;// unsigned size; size is sometimes a pointer value...
                 // this means bad thing when we change platforms...
#ifdef _MSC_VER
#pragma warning (disable:4200)
#endif
   _8  data[
#ifndef __cplusplus 
   1
#endif
   ]; // beginning of var data - this is created size+sizeof(VPA)
#ifdef _MSC_VER
#pragma warning (default:4200)
#endif
} DATA, *PDATA;

_LINKLIST_NAMESPACE

typedef struct LinkBlock
{
   _32     Cnt;
   _32     Lock;
   POINTER pNode[1];
} LIST, *PLIST;

_LINKLIST_NAMESPACE_END

#ifdef __cplusplus
using namespace sack::containers::list;
#endif
typedef struct DataBlock  DATALIST, *PDATALIST;

struct DataBlock
{
	_32     Cnt;
	_32     Avail;
	_32     Lock;
	_32     Size;
	_8      data[1];
};

typedef struct LinkStack
{
   _32     Top;
   _32     Cnt;
	_32     Lock;  // thread interlock using InterlockedExchange semaphore
   _32     Max;
   POINTER pNode[1];
} LINKSTACK, *PLINKSTACK;

typedef struct DataListStack
{
   _32     Top; // next avail...
   _32     Cnt;
   _32     Lock;  // thread interlock using InterlockedExchange semaphore
   _32     Size;
   _8      data[1];
} DATASTACK, *PDATASTACK;

typedef struct LinkQueue
{
   _32     Top;
   _32     Bottom;
   _32     Cnt;
   _32     Lock;  // thread interlock using InterlockedExchange semaphore
   POINTER pNode[2]; // need two to have distinct empty/full conditions
} LINKQUEUE, *PLINKQUEUE;

typedef struct DataQueue
{
   _32     Top;
   _32     Bottom;
   _32     Cnt;
   _32     Lock;  // thread interlock using InterlockedExchange semaphore
   _32     Size;
   _32     ExpandBy;
   _8      data[1];
} DATAQUEUE, *PDATAQUEUE;

_CONTAINER_NAMESPACE_END
SACK_NAMESPACE_END

#include <typelib.h> // general functions for using basic types

#ifdef LOG_LIBRARY_ENTER
#include <logging.h>
#endif

// may consider changing this to P_16 for unicode...
#ifdef UNICODE
#ifndef NO_UNICODE_C
#define strrchr          wcsrchr  
#define strchr           wcschr
#define strncpy          wcsncpy
#define strcpy           wcscpy
#define strcmp           wcscmp

#define strlen           wcslen
#define stricmp          wcsicmp
#define strnicmp         wcsnicmp
#define stat             _wstat
#ifdef _MSC_VER
#ifndef __cplusplus_cli
#error "okay - no  we don't like unicode please set multibyte charset..."
#define sprintf          _swprintf
#define vsnprintf        _vsnwprintf
#define snprintf         _snwprintf
#endif
#endif
#endif
#else
#ifdef _MSC_VER
#define snprintf _snprintf
#ifndef __cplusplus_cli
#if ( _MSC_VER < 1500 )
// myabe this is required for older stuff... I keep trying to remove these...
#define	vsnprintf _vsnprintf
#endif
#endif
#endif
#endif





SACK_NAMESPACE

#ifndef IS_DEADSTART
// this is always statically linked with libraries, so they may contact their
// core executable to know when it's done loading everyone else also...
#  ifdef __cplusplus
extern "C" 
#  endif
#  if defined( __WINDOWS__ ) && !defined( __STATIC__ )
#    ifdef __NO_WIN32API__ 
// DllImportAttribute ?
#    else
__declspec(dllimport)
#    endif
#  else
#ifndef __cplusplus
extern
#endif
#  endif
LOGICAL
#  if defined( __WATCOMC__ )
__cdecl
#  endif
is_deadstart_complete( void );
#endif

//DYNAMIC_EXPORT void Exit( int code );
//#define exit(n) Exit(n)
#ifdef BAG_EXIT_DEFINED
EXPORT_METHOD
#else
#  ifndef BAG
#    ifndef __STATIC__
	IMPORT_METHOD
#    endif
#  else
extern
#  endif
#endif
	void BAG_Exit( int code );
#define exit(n) BAG_Exit(n)

SACK_NAMESPACE_END // namespace sack {

// this should become common to all libraries and programs...
#include <construct.h> // pronounced 'kahn-struct'


#ifdef __cplusplus
using namespace sack;
using namespace sack::containers;
#endif


#endif



