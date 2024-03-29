#ifndef FILE_MONITOR_LIBRARY_DEFINED
#define FILE_MONITOR_LIBRARY_DEFINED

#ifdef BCC16
#ifdef FILEMONITOR_SOURCE
#define FILEMONITOR_PROC(type,name) type STDPROC _export name
#else
#define FILEMONITOR_PROC(type,name) type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef FILEMONITOR_SOURCE
#define FILEMONITOR_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define FILEMONITOR_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#else
#ifdef FILEMONITOR_SOURCE
#define FILEMONITOR_PROC(type,name) type CPROC name
#else
#define FILEMONITOR_PROC(type,name) extern type CPROC name
#endif
#endif
#endif

// filemon will require system features, so pull stdhdrs instead of
// just sack_tyeps
#include <stdhdrs.h>

#include <filesys.h>  // for namespace nesting/definition

FILEMON_NAMESPACE

typedef struct monitor_tag *PMONITOR;
typedef struct filechangecallback_tag *PCHANGEHANDLER;
typedef struct filemonitor_tag *PFILEMON;

// bTree monitors this path and all sub-paths...
// (in theory)
// mask is a file mask to match - supports DOS style * and ?
FILEMONITOR_PROC(PMONITOR, MonitorFiles )( CTEXTSTR directory, int scan_delay );
// end a single monitor....
FILEMONITOR_PROC( void, EndMonitor )( PMONITOR monitor );
// close all file monitors....
FILEMONITOR_PROC(void, EndMonitorFiles )( void );

// Log files read in the directory for this monitor.  Log matches per handler
FILEMONITOR_PROC( void, SetFileLogging )( PMONITOR monitor, int enable );

// return TRUE if okay to get next, return FALSE to
// stop processing and wait until next...
typedef int (CPROC *CHANGEHANDLER)(PTRSZVAL psv
											 , CTEXTSTR filepath
											 , int bDeleted);

FILEMONITOR_PROC( PCHANGEHANDLER, AddFileChangeCallback )( PMONITOR monitor
                                               , CTEXTSTR mask
															  , CHANGEHANDLER HandleChange
															  , PTRSZVAL psv );

typedef int (CPROC *EXTENDEDCHANGEHANDLER)( PTRSZVAL psv
														, CTEXTSTR filepath
														, _64 size
														, _64 time
														, LOGICAL bCreated // file has just now been created.
                                          , LOGICAL bDirectory // it's a directory (add another monitor?)
														, LOGICAL bDeleted); // file was just now deleted.

FILEMONITOR_PROC( PCHANGEHANDLER, AddExtendedFileChangeCallback )( PMONITOR monitor
																		, CTEXTSTR mask
																		, EXTENDEDCHANGEHANDLER HandleChange
																		, PTRSZVAL psv );


FILEMONITOR_PROC( PFILEMON, AddMonitoredFile )( PCHANGEHANDLER Change, CTEXTSTR name );
FILEMONITOR_PROC( void, EverybodyScan )( void );
FILEMONITOR_PROC( void, MonitorForgetAll )( PMONITOR monitor );
FILEMONITOR_PROC( void, SetFMonitorForceScanTime )( PMONITOR monitor, _32 delay );

// returns 0 if no changed were pending, else returns number
// of changes dispatched (not nessecarily handled)
FILEMONITOR_PROC( int, DispatchChanges )( PMONITOR monitor );


FILEMON_NAMESPACE_END
#ifdef __cplusplus
using namespace sack::filesys::monitor;
#endif


#endif
//----------------------------------------------------------------------
//
// $Log: filemon.h,v $
// Revision 1.6  2005/02/23 13:01:35  panther
// Fix scrollbar definition.  Also update vc projects
//
// Revision 1.5  2004/12/01 23:15:58  panther
// Extend file monitor to make forced scan timeout settable by the application
//
// Revision 1.4  2004/01/16 17:07:08  d3x0r
// Header updates...
//
// Revision 1.3  2003/11/09 22:31:36  panther
// Add extended monitor registration
//
// Revision 1.2  2003/11/04 11:39:15  panther
// Modified interface to monitor files
//
// Revision 1.1  2003/11/03 23:01:49  panther
// Initial commit of librarized filemonitor
//
