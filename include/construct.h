#include <sack_types.h>

#if !defined(__STATIC__) && !defined( __UNIX__ )
#ifdef CONSTRUCT_SOURCE
#define CONSTRUCT_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define CONSTRUCT_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#else
#ifdef VIDEO_LIBRARY_SOURCE 
#define CONSTRUCT_PROC(type,name) type CPROC name
#else
#define CONSTRUCT_PROC(type,name) extern type CPROC name
#endif
#endif

#ifdef __cplusplus
#define _TASK_NAMESPACE namespace task {
#define _CONSTRUCT_NAMESPACE namespace construct {
#define _TASK_NAMESPACE_END }
#define _CONSTRUCT_NAMESPACE_END }

#else
#define _TASK_NAMESPACE 
#define _CONSTRUCT_NAMESPACE 
#define _TASK_NAMESPACE_END
#define _CONSTRUCT_NAMESPACE_END
#endif

#define CONSTRUCT_NAMESPACE SACK_NAMESPACE _TASK_NAMESPACE _CONSTRUCT_NAMESPACE
#define CONSTRUCT_NAMESPACE_END _CONSTRUCT_NAMESPACE_END _TASK_NAMESPACE_END SACK_NAMESPACE_END

CONSTRUCT_NAMESPACE

CONSTRUCT_PROC( void, LoadComplete )( void );

CONSTRUCT_NAMESPACE_END

#ifdef __cplusplus
	using namespace sack::task::construct;
#endif

