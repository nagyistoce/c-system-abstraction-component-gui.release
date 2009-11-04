/*
 *  Created By Jim Buckeyne
 *
 *  Purpose:
 *    Provides some cross platform/library functionatlity for
 *  filesystem activities.
 *  - File dates, times, stuff like that
 *  - make paths, change paths
 *  - path parsing (like strchr, strrchr, but looking for closest / or \)
 *  - scan a directory for a set of files... using a recursive callback method
 */

#ifndef FILESYSTEM_UTILS_DEFINED
#define FILESYSTEM_UTILS_DEFINED
#include <sack_types.h>

#if defined( BCC16 )
#if !defined(__STATIC__)
#define FILESYS_PROC(type,name) type _far _pascal _export name
#else
#define FILESYS_PROC(type,name) type _far _pascal name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef FILESYSTEM_LIBRARY_SOURCE
#define FILESYS_PROC(type,name) EXPORT_METHOD type name
#else
#define FILESYS_PROC(type,name) IMPORT_METHOD type name
#endif
#else
#ifdef FILESYSTEM_LIBRARY_SOURCE
#define FILESYS_PROC(type,name) type name
#else
#define FILESYS_PROC(type,name) extern type name
#endif
#endif
#endif

#ifdef __cplusplus
#define _FILESYS_NAMESPACE  namespace filesys {
#define _FILESYS_NAMESPACE_END }
#define _FILEMON_NAMESPACE  namespace monitor {
#define _FILEMON_NAMESPACE_END }
#else
#define _FILESYS_NAMESPACE 
#define _FILESYS_NAMESPACE_END
#define _FILEMON_NAMESPACE 
#define _FILEMON_NAMESPACE_END
#endif
#define FILESYS_NAMESPACE_END _FILESYS_NAMESPACE_END SACK_NAMESPACE_END 
#define FILESYS_NAMESPACE SACK_NAMESPACE _FILESYS_NAMESPACE 
#define FILEMON_NAMESPACE_END _FILEMON_NAMESPACE_END _FILESYS_NAMESPACE_END SACK_NAMESPACE_END 
#define FILEMON_NAMESPACE SACK_NAMESPACE _FILESYS_NAMESPACE _FILEMON_NAMESPACE

FILESYS_NAMESPACE

#define SFF_SUBCURSE    1 // go into subdirectories
#define SFF_DIRECTORIES 2 // return directory names also
#define SFF_NAMEONLY    4 // don't concatenate base with filename to result.
#define SFF_IGNORECASE  8 // when matching filename - do not match case.

// flags sent to Process when called with a matching name
#define SFF_DIRECTORY   1 // is a directory...
#define SFF_DRIVE       2 // this is a drive...

FILESYS_PROC( int, CompareMask )( CTEXTSTR mask, CTEXTSTR name, int keepcase );

// ScanFiles usage:
//   base - base path to scan
//   mask - file mask to process if NULL or "*" is everything "*.*" must contain a .
//   pInfo is a pointer to a void* - this pointer is used to maintain
//        internal information... 
//   Process is called with the full name of any matching files
//   subcurse is a flag - set to go into all subdirectories looking for files.
// There is no way to abort the scan... 
FILESYS_PROC( int, ScanFiles )( CTEXTSTR base
           , CTEXTSTR mask
           , void **pInfo
           , void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
           , int flags 
           , PTRSZVAL psvUser );
FILESYS_PROC( void, ScanDrives )( void (CPROC *Process)(PTRSZVAL user, CTEXTSTR letter, int flags)
										  , PTRSZVAL user );
// result is length of name filled into pResult if pResult == NULL && nResult = 0
// the result will the be length of the name matching the file.
FILESYS_PROC( int, GetMatchingFileName )( CTEXTSTR filemask, int flags, TEXTSTR pResult, int nResult );

// searches a path for the last '/' or '\'
FILESYS_PROC( CTEXTSTR, pathrchr )( CTEXTSTR path );
#ifdef __cplusplus
FILESYS_PROC( TEXTSTR, pathrchr )( TEXTSTR path );
#endif
// searches a path for the first '/' or '\'
FILESYS_PROC( CTEXTSTR, pathchr )( CTEXTSTR path );

// returns pointer passed (if it worked?)
FILESYS_PROC( TEXTSTR, GetCurrentPath )( TEXTSTR path, int buffer_len );
FILESYS_PROC( int, SetCurrentPath )( CTEXTSTR path );
FILESYS_PROC( int, MakePath )( CTEXTSTR path );
FILESYS_PROC( int, IsPath )( CTEXTSTR path );


FILESYS_PROC( _64, GetFileWriteTime )( CTEXTSTR name ); // last modification time.
FILESYS_PROC( LOGICAL, SetFileWriteTime)( CTEXTSTR name, _64 filetime ); // last modification time.


#ifdef __LINUX__
#define SYSPATHCHAR WIDE("/")
#else
#define SYSPATHCHAR WIDE("\\")
#endif

FILESYS_NAMESPACE_END
#ifdef __cplusplus
using namespace sack::filesys;
#endif

#endif

//-------------------------------------------------------------------
// $Log: filesys.h,v $
// Revision 1.21  2005/05/06 18:15:33  jim
// Add ability to set file write time.
//
// Revision 1.20  2005/03/26 02:50:53  panther
// Define a fancier function to get similar filenames
//
// Revision 1.19  2004/05/27 21:37:38  d3x0r
// Syncpoint.
//
// Revision 1.18  2004/05/06 08:11:51  d3x0r
// It's bad form to return the const char * cause there's too many others that take char * inut..
//
// Revision 1.17  2004/01/06 08:17:25  panther
// const char * on pathchar funcs
//
// Revision 1.16  2003/08/21 14:54:47  panther
// Update callback definitions
//
// Revision 1.15  2003/08/08 11:16:56  panther
// Define SYSPATHCHAR based on system
//
// Revision 1.14  2003/04/21 19:59:43  panther
// Option to return filename only
//
// Revision 1.13  2003/04/02 16:01:21  panther
// Export CompareMask
//
// Revision 1.12  2003/03/25 08:38:11  panther
// Add logging
//
// Revision 1.11  2003/02/21 16:28:13  panther
// Added user flag to scanfiles
//
// Revision 1.10  2002/08/21 19:20:12  panther
// Minor updates removing a define check unneeded, and set NULL on accesses
// to LIST_FORALL
//
// Revision 1.9  2002/08/12 22:23:08  panther
// Added support for 16 bit borland compiles.
//
//
// $Log: filesys.h,v $
// Revision 1.21  2005/05/06 18:15:33  jim
// Add ability to set file write time.
//
// Revision 1.20  2005/03/26 02:50:53  panther
// Define a fancier function to get similar filenames
//
// Revision 1.19  2004/05/27 21:37:38  d3x0r
// Syncpoint.
//
// Revision 1.18  2004/05/06 08:11:51  d3x0r
// It's bad form to return the const char * cause there's too many others that take char * inut..
//
// Revision 1.17  2004/01/06 08:17:25  panther
// const char * on pathchar funcs
//
// Revision 1.16  2003/08/21 14:54:47  panther
// Update callback definitions
//
// Revision 1.15  2003/08/08 11:16:56  panther
// Define SYSPATHCHAR based on system
//
// Revision 1.14  2003/04/21 19:59:43  panther
// Option to return filename only
//
// Revision 1.13  2003/04/02 16:01:21  panther
// Export CompareMask
//
// Revision 1.12  2003/03/25 08:38:11  panther
// Add logging
//
