/* Includes the system platform as required or appropriate. If
   under a linux system, include appropriate basic linux type
   headers, if under windows pull "windows.h".
   
   
   
   Includes the MOST stuff here ( a full windows.h parse is many
   many lines of code.)                                          */

#include <sack_types.h>

/* A macro to build a wide character string of __FILE__ */
#define _WIDE__FILE__(n) WIDE(n)
#define WIDE__FILE__ _WIDE__FILE__(__FILE__)

#ifndef STANDARD_HEADERS_INCLUDED
/* multiple inclusion protection symbol */
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
// just in case windows.h also fails after undef WIN32
// these will be the correct order for primitives we require.
#include <excpt.h>
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
//#error blah
#include <wingdi.h>
#include <wtypes.h>
#include <winuser.h>
#undef WIN32
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
#if defined( HAVE_ENVIRONMENT )
#define getenv(name)       OSALOT_GetEnvironmentVariable(name)
#define setenv(name,val)   SetEnvironmentVariable(name,val)
#endif
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

#if !defined( UNDER_CE ) ||defined( ELTANIN)
#include <fcntl.h>
#include <io.h>
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
/* A symbol used to cause a debugger to break at a certain
   point. Sometimes dynamicly loaded plugins can be hard to set
   the breakpoint in the debugger, so it becomes easier to
   recompile with a breakpoint in the right place.
   Example
   <code lang="c++">
   DebugBreak();
   </code>                                                      */
#define DebugBreak()  asm("int $3\n" )
#endif

// moved into timers - please linnk vs timers to get Sleep...
//#define Sleep(n) (usleep((n)*1000))
#define Relinquish() sched_yield()
#define GetLastError() (S_32)errno
/* return with a THREAD_ID that is a unique, universally
   identifier for the thread for inter process communication. */
#define GetCurrentProcessId() ((_32)getpid())
#define GetCurrentThreadId() ((_32)getpid())

/* Define a min(a,b) macro when the compiler lacks it. */
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifdef __LINUX64__
//typedef unsigned long HANDLE;
#else
//typedef void *HANDLE;
#endif

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
#include <string.h>
#ifdef __CYGWIN__
#include <errno.h> // provided by -lgcc
// lots of things end up including 'setjmp.h' which lacks sigset_t defined here.
#include <sys/types.h>
// lots of things end up including 'setjmp.h' which lacks sigset_t defined here.
#include <sys/signal.h>

#endif

#include <logging.h>

// GetTickCount() and Sleep(n) Are typically considered to be defined by including stdhdrs...
#include <timers.h>


#ifndef MAXPATH
// windef.h has MAX_PATH
# define MAXPATH MAX_PATH
#endif

#ifndef PATH_MAX
// sometimes PATH_MAX is what's used, well it's should be MAXPATH which is MAX_PATH
# define PATH_MAX MAXPATH
#endif


#include <final_types.h>

#endif
