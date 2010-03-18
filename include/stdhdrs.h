
#include <sack_types.h>

#define _WIDE__FILE__(n) WIDE(n)
#define WIDE__FILE__ _WIDE__FILE__(__FILE__)

#ifndef STANDARD_HEADERS_INCLUDED
#define STANDARD_HEADERS_INCLUDED 
#include <stdlib.h>
#include <stddef.h>
#ifdef __GNUC__
//#include <sys/types.h> // off64_t, ftello64
#endif
#include <stdio.h>
#if _MSC_VER > 100000
#include <stdint.h>
#endif

// apparently we don't use this anymore...
//#include <process.h> // also has a getenv defined..

#ifndef WINVER
#define WINVER 0x0501
#endif

#if !defined(__LINUX__) && !defined(__UNIX__)
#  ifndef STRICT
#    define STRICT
#  endif
#define WIN32_LEAN_AND_MEAN

// #define NOGDICAPMASKS             // CC_*, LC_*, PC_*, CP_*, TC_*, RC_                          
// #define NOVIRTUALKEYCODES         // VK_*                                                       
// #define NOWINMESSAGES             // WM_*, EM_*, LB_*, CB_*                                     
// #define NOWINSTYLES               // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*                      
// #define NOSYSMETRICS              // SM_*                                                       
// #define NOMENUS                   // MF_*                                                       
// #define NOICONS                   // IDI_*                                                      
// #define NOKEYSTATES               // MK_*                                                       
// #define NOSYSCOMMANDS             // SC_*                                                       
// #define NORASTEROPS               // Binary and Tertiary raster ops                             
// #define NOSHOWWINDOW              // SW_*                                                       
#define OEMRESOURCE               // OEM Resource values                                        
// #define NOATOM                    // Atom Manager routines                                      
#  ifndef _INCLUDE_CLIPBOARD
#    define NOCLIPBOARD               // Clipboard routines
#  endif
// #define NOCOLOR                   // Screen colors                                              
// #define NOCTLMGR                  // Control and Dialog routines                                
//(spv) #define NODRAWTEXT                // DrawText() and DT_*                                        

// #define NOGDI                     // All GDI defines and routines                               
// #define NOKERNEL                  // All KERNEL defines and routines                            
// #define NOUSER                    // All USER defines and routines
#ifndef _ARM_
#  ifndef _INCLUDE_NLS
#    define NONLS                     // All NLS defines and routines                               
#  endif
#endif
// #define NOMB                      // MB_* and MessageBox()                                      
#define NOMEMMGR                  // GMEM_*, LMEM_*, GHND, LHND, associated routines            
#define NOMETAFILE                // typedef METAFILEPICT                                       
// #define NOMINMAX                  // Macros min(a,b) and max(a,b)                               
// #define NOMSG                     // typedef MSG and associated routines                        
// #define NOOPENFILE                // OpenFile(), OemToAnsi, AnsiToOem, and OF_*                 
// #define NOSCROLL                  // SB_* and scrolling routines                                
#define NOSERVICE                 // All Service Controller routines, SERVICE_ equates, etc.    
//#define NOSOUND                   // Sound driver routines                                      
// #define NOTEXTMETRIC              // typedef TEXTMETRIC and associated routines                 
// #define NOWH                      // SetWindowsHook and WH_*                                    
// #define NOWINOFFSETS              // GWL_*, GCL_*, associated routines                          
// #define NOCOMM                    // COMM driver routines                                       
#define NOKANJI                   // Kanji support stuff.                                       
// #define NOHELP                    // Help engine interface.                                     
#define NOPROFILER                // Profiler interface.                                        
//#define NODEFERWINDOWPOS          // DeferWindowPos routines                                    
#define NOMCX                     // Modem Configuration Extensions                             

#  ifdef _MSC_VER
#    ifndef _WIN32_WINDOWS
// needed at least this for what - updatelayeredwindow?
#      define _WIN32_WINDOWS 0x0401
#    endif
#  endif

// INCLUDE WINDOWS.H
#  ifdef __WATCOMC__
#undef _WINDOWS_
#  endif

#  ifdef UNDER_CE
// just in case windows.h also fails after undef __WINDOWS__
// these will be the correct order for primitives we require.
#include <excpt.h>
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
//#error blah
#include <wingdi.h>
#include <wtypes.h>
#include <winuser.h>
#undef __WINDOWS__
#  endif

#include <windows.h>

#include <windowsx.h>
// we like timeGetTime() instead of GetTickCount()
#include <mmsystem.h>

#ifdef NEED_V4W
#include <vfw.h>
#endif
//#  ifdef __cplusplus
//// including these first will help with the Release() macro redefinition.
//#    include <objbase.h>
//#    include <ocidl.h>
//#  endif

// incldue this first so we avoid a conflict.
// hopefully this comes from sack system?
#include <system.h>
#define getenv(name)       OSALOT_GetEnvironmentVariable(name)
#define setenv(name,val)   SetEnvironmentVariable(name,val)
#define Relinquish()       Sleep(0)
//#pragma pragnoteonly("GetFunctionAddress is lazy and has no library cleanup - needs to be a lib func")
//#define GetFunctionAddress( lib, proc ) GetProcAddress( LoadLibrary( lib ), (proc) )


#  ifdef __cplusplus_cli
# include <vcclr.h>
# define DebugBreak() System::Console::WriteLine( /*lprintf( */gcnew System::String( WIDE__FILE__ WIDE("(") STRSYM(__LINE__) WIDE(") Would DebugBreak here...") ) );
//typedef unsigned int HANDLE;
//typedef unsigned int HMODULE;
//typedef unsigned int HWND;
//typedef unsigned int HRC;
//typedef unsigned int HMENU;
//typedef unsigned int HICON;
//typedef unsigned int HINSTANCE;
#  endif

#ifndef UNDER_CE
#include <fcntl.h>
#endif

# include <filedotnet.h>

#else // ifdef unix/linux
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sack_types.h>
#include <sys/time.h>
#include <errno.h>
# include <filedotnet.h>
#if defined( __ARM__ )
#define DebugBreak()
#else
#define DebugBreak()  asm("int $3\n" )
#endif

// moved into timers - please linnk vs timers to get Sleep...
//#define Sleep(n) (usleep((n)*1000))
#define Relinquish() sched_yield()
#define GetLastError() (S_32)errno
#define GetCurrentProcessId() ((_32)getpid())
#define GetCurrentThreadId() ((_32)getpid())

/*
 // moved into timers - please linnk vs timers to get time...
static unsigned long GetTickCount( void )
{
   struct timeval time;
   gettimeofday( &time, 0 );
   return (time.tv_sec * 1000) + (time.tv_usec / 1000);
	}
   */
/*
#define GetTickCount() ( struct timeval time;  gettimeofday( &time, 0 ),\
    ( (time.tv_sec * 1000) + (time.tv_usec / 1000) )\
    )
    */
//#define GetTickCount() ( time(NULL) * 1000 )
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifdef __LINUX64__
//typedef unsigned long HANDLE;
#else
//typedef void *HANDLE;
#endif

/*
#ifndef __PPCCPP__
#warning GetFunctionAddress is lazy and has no library cleanup - needs to be a lib func
#else
#pragma pragnoteonly( WIDE("GetFunctionAddress is lazy and has no library cleanup - needs to be a lib func") )
#endif
#define GetFunctionAddress( lib, proc ) dlsym( dlopen( lib, RTLD_NOW ), (proc) )
*/
#endif
#if defined( _MSC_VER )|| defined(__LCC__) || defined( __WATCOMC__ ) || defined( __GNUC__ )
//#ifndef __cplusplus_cli
#include "loadsock.h"
//#endif
#include <malloc.h>               // _heapmin() included here
#else
//#include "loadsock.h"
#endif
//#include <stdlib.h>
#include <stdarg.h>
//#include <stdio.h>
#include <string.h>
#ifdef __CYGWIN__
#include <errno.h> // provided by -lgcc
// lots of things end up including 'setjmp.h' which lacks sigset_t defined here.
#include <sys/types.h>
// lots of things end up including 'setjmp.h' which lacks sigset_t defined here.
#include <sys/signal.h>

#endif

// GetTickCount() and Sleep(n) Are typically considered to be defined by including stdhdrs...
#include <timers.h>

#ifdef NEED_TICK_LOGGING
#ifdef __LCC__
#ifdef _DEBUG
#include <logging.h>
#include <intrinsics.h>
static long long __tick_mark_time;
static long long __tick_mark_now;
static long long __tick_mark_calibrate;

#define TickCalibrate() {                          \
   long dwTime = GetTickCount() + 1000;            \
   long long _tick_count = _rdtsc();               \
   while( dwTime > GetTickCount() ) Sleep(0);        \
   _tick_count = ( _rdtsc() - _tick_count );       \
   Log1( WIDE("count: %ld"), _tick_count );            \
   /*_tick_count = _tick_count / 1000;  */             \
   __tick_mark_calibrate = _tick_count / 1000000;  \
   }

#define TickMark() ( __tick_mark_time = _rdtsc() )
#define TickLog(msg) { if( !__tick_mark_calibrate) { TickCalibrate(); TickMark(); } else {\
	__tick_mark_now = _rdtsc();     \
	Log3( WIDE("%s(%d): ") msg "Delta %lld", __FILE__, __LINE__, \
		(long)((__tick_mark_now - __tick_mark_time )/__tick_mark_calibrate) ); \
	__tick_mark_time = __tick_mark_now; \
	} }

   //long long start = _rdtsc();
   //long long cur, del;
// forgot how to declare a long long constant...


#else // not debug - don't log ticking
#define TickMark()
#define TickLog(msg)
#define TickCalibrate() 
#endif
#else // not lcc - don't use _rdtsc();
#define TickMark()
#define TickLog(msg)
#define TickCalibrate()
#endif
#endif

#endif

#ifdef FIX_BROKEN_TYPECASTS
#define int short
// int is the first mortal sin.
#endif


#include <final_types.h>

