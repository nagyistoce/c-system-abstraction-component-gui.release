// find and replace ABCDEFG with desired header name...
#ifndef ABCDEFG_DEFINED
#define ABCDEFG_DEFINED
#include <sack_types.h>

#ifdef BCC16
#if defined( ABCDEFG_SOURCE )
#define ABCDEFG_PROC(type,name) type STDPROC _export name
#else
#define ABCDEFG_PROC(type,name) type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#if defined( ABCDEFG_SOURCE )
#define ABCDEFG_PROC(type,name) __declspec(dllexport) type CPROC name
#define ABCDEFG_PROC_PTR(type,name) __declspec(dllexport) type (*CPROC name)
#define ABCDEFG_DATA(type,name) __declspec(dllexport) type name
#else
#define ABCDEFG_PROC(type,name) __declspec(dllimport) type CPROC name
#define ABCDEFG_PROC_PTR(type,name) __declspec(dllimport) type (*CPROC name)
#define ABCDEFG_DATA(type,name) __declspec(import) type name
#endif
#else
#if defined( ABCDEFG_SOURCE )
#define ABCDEFG_PROC(type,name) type CPROC name
#define ABCDEFG_PROC_PTR(type,name) type (*CPROC name)
#define ABCDEFG_DATA(type,name) type name
#else
#define ABCDEFG_PROC(type,name) extern type CPROC name
#define ABCDEFG_PROC_PTR(type,name) extern type (*CPROC name)
#define ABCDEFG_DATA(type,name) extern type name
#endif
#endif
#endif

#define FUTGETPR_PROC(type,name) __declspec(dllexport) type CPROC name
#define FUTGETPR_DATA(type,name) __declspec(dllexport) type name
#else
#define FUTGETPR_PROC(type,name) __declspec(dllimport) type CPROC name
#define FUTGETPR_DATA(type,name) __declspec(dllimport) type name
#endif
#else
#ifdef FUTGETPR_SOURCE
#define FUTGETPR_PROC(type,name) type CPROC name
#define FUTGETPR_DATA(type,name) type name
#else
#define FUTGETPR_PROC(type,name) extern type CPROC name
#define FUTGETPR_DATA(type,name) extern type name


#endif
