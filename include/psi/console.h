
#ifndef PSI_CONSOLE_INTERFACE_DEFINED
#define PSI_CONSOLE_INTERFACE_DEFINED

#ifdef BCC16
#ifdef PSI_CONSOLE_SOURCE
#define PSI_CONSOLE_PROC(type,name) type STDPROC _export name
#else
#define PSI_CONSOLE_PROC(type,name) type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef PSI_CONSOLE_SOURCE
#define PSI_CONSOLE_PROC(type,name) __declspec(dllexport) type CPROC name
#else
#define PSI_CONSOLE_PROC(type,name) __declspec(dllimport) type CPROC name
#endif
#else
#ifdef PSI_CONSOLE_SOURCE
#define PSI_CONSOLE_PROC(type,name) type CPROC name
#else
#define PSI_CONSOLE_PROC(type,name) extern type CPROC name
#endif
#endif
#endif

#ifdef __cplusplus
#define PSI_CONSOLE_NAMESPACE PSI_NAMESPACE namespace console {
#define PSI_CONSOLE_NAMESPACE_END } PSI_NAMESPACE_END
#define USE_PSI_CONSOLE_NAMESPACE using namespace sack::psi::console
#else
#define PSI_CONSOLE_NAMESPACE 
#define PSI_CONSOLE_NAMESPACE_END 
#define USE_PSI_CONSOLE_NAMESPACE
#endif



#include <controls.h>

PSI_CONSOLE_NAMESPACE
PSI_CONSOLE_PROC( void, PSIConsoleLoadFile )( PSI_CONTROL pc, CTEXTSTR filename );
PSI_CONSOLE_PROC( void, PSIConsoleSaveFile )( PSI_CONTROL pc, CTEXTSTR filename );

PSI_CONSOLE_PROC( int, vpcprintf )( PSI_CONTROL pc, CTEXTSTR format, va_list args );
PSI_CONSOLE_PROC( int, pcprintf )( PSI_CONTROL pc, CTEXTSTR format, ... );
PSI_CONSOLE_PROC( int, PSIConsoleOutput )( PSI_CONTROL pc, PTEXT lines );
PSI_CONSOLE_NAMESPACE_END;

USE_PSI_CONSOLE_NAMESPACE;

#endif
