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
/* Header multiple inclusion protection symbol. */
#define FILESYSTEM_UTILS_DEFINED
#include <stdhdrs.h>

#if !defined( UNDER_CE ) ||defined( ELTANIN)
#include <fcntl.h>
#include <io.h>
#endif

/* uhmm in legacy usage this was not CPROC, but was unspecified */
#define FILESYS_API

#ifdef FILESYSTEM_LIBRARY_SOURCE
#define FILESYS_PROC EXPORT_METHOD
#else
/* define the method that file system and file monitor use for
   library linkage.                                            */
#define FILESYS_PROC IMPORT_METHOD
#endif

#ifdef __cplusplus
/* defined the file system partial namespace (under
   SACK_NAMESPACE probably)                         */
#define _FILESYS_NAMESPACE  namespace filesys {
/* Define the ending symbol for file system namespace. */
#define _FILESYS_NAMESPACE_END }
/* Defined the namespace of file montior utilities. File monitor
   provides event notification based on file system changes.     */
#define _FILEMON_NAMESPACE  namespace monitor {
/* Define the end symbol for file monitor namespace. */
#define _FILEMON_NAMESPACE_END }
#else
#define _FILESYS_NAMESPACE 
#define _FILESYS_NAMESPACE_END
#define _FILEMON_NAMESPACE 
#define _FILEMON_NAMESPACE_END
#endif
/* define the file system namespace end. */
#define FILESYS_NAMESPACE_END _FILESYS_NAMESPACE_END SACK_NAMESPACE_END 
/* define the file system namespace. */
#define FILESYS_NAMESPACE SACK_NAMESPACE _FILESYS_NAMESPACE 
/* Define end file monitor namespace. */
#define FILEMON_NAMESPACE_END _FILEMON_NAMESPACE_END _FILESYS_NAMESPACE_END SACK_NAMESPACE_END 
/* Defines the file montior namespace when compiling C++. */
#define FILEMON_NAMESPACE SACK_NAMESPACE _FILESYS_NAMESPACE _FILEMON_NAMESPACE

/* It's called System Abstraction Component Kit - that is almost
   what it is, self descriptively, but that's not what it
   entirely. It's more appropriately just a sack-o-goodies. A
   grab-bag of tidbits. The basic aggregation of group of
   objects in sack is bag. Bag contains everything that is not
   graphic.
   
   It abstracts loading external libraries (.dll and .so shared
   objects) so applications can be unbothered by platform
   details.
   
   It abstracts system features like threads into a consistent
   API.
   
   It abstracts system shared memory allocation and management,
   does have a custom allocation and release method, but
   currently falls back to using malloc and free.
   
   It abstracts access to system file system browsing; getting
   the contents of directories.
   
   It abstracts creating external processes, and the use of
   standard input-output methods to communicate with those
   tasks.
   
   It has a custom timer implementation using a thread and
   posting timer events as calls to user callbacks.
   
   It contains a variety of structures to manage data from
   lists, stack and queues to complex linked list of slab
   allocated structures, and management for text as words and
   phrases.
   
   It contains an event based networking system, using an
   external thread to coordinate network socket events.
   
   It contains methods to work with images.
   
   It contains methods to display images on a screen. This is
   also a system abstraction point, since to put a display out
   under Linux and Windows is entirely different.
   
   It contains a control system based on the above images and
   display, managing events to controls as user event callbacks.
   
   It contains a method of registering values, code, and even
   structures, and methods to browse and invoke registered code,
   create registered structures, and get back registered values.
   
   It contains a SQL abstraction that boils SQL communication to
   basically 2 methods, Commands and Queries.
   
   It provides application logging features, for debug, and
   crash diagnostics. This is basically a single command
   'lprintf'.
   
   It provides a vector and matrix library for 3D and even 4D
   graphics projects.
   
   
   
   Beyond that - it uses libpng, jpeg, and freetype for dealing
   with images. These are included in this one package for
   windows where they are not system libraries. Internal are up
   to date as of march 2010. Also includes sqlite. All of these
   \version compile as C++ replacing extern "C"{} with an
   appropriate namespace XXX {}.
   Example
   The current build system is CMake. Previously I had a bunch
   of makefiles, but then required gnu make to compile with open
   watcom, with cmake, the correct makefiles appropriate for
   each package is generated. Visual studio support is a little
   lacking in cmake. So installed output should be compatible
   with cmake find XXX.
   <code>
   PROJECT( new_project )
   ADD_EXECTUABLE( my_program my_source.c )
   LINK_TARGET_LIBRARIES( my_program ${SACK_LIBRARY} )
   </code>
   
   The problem with this... depends on the mode of sack being
   built against... maybe I should have a few families of
   compile-options and link libraries.
   Remarks
   If sack is built 'monolithic' then everything that is any
   sort of library is compiled together.
   
   If sack is not built monolithic, then it produces
     * \  bag &#45; everything that requires no system display,
       it is all the logic components for lists, SQL, processes,
       threads, networking
       * ODBC is currently linked here, so unixodbc or odbc32
         will be required as appropriate. Considering moving SQL
         entirely to a seperate module, but the core library can take
         advantage options, which may require SQL.
     * \  bag.external &#45; external libraries zlib, libpng,
       jpeg, freetype.
     * bag.sqlite3.external &#45; external sqlite library, with
       sqlite interface structure. This could be split to be just
       the sqlite interface, and link to a system sqlite library.
     * bag.image &#45; library that provides an image namespace
       interface.
     * bag.video &#45; library that provides a render namespace
       interface. Windows system only. Provides OpenGL support for
       displays also.
     * bag.display.image &#45; a client library that
       communicates to a remote image namespace interface.
     * bag.image.video &#45; a client library that communicates
       to a remote render namespace interface.
     * bag.display &#45; a host library that provides window
       management for render interface, which allows the application
       multliple windows. bag.display was built against SDL, and SDL
       only supplies a single display surface for an application, so
       popup dialogs and menus needed to be tracked internally.
       Bag.display can be loaded as a display service, and shared
       between multiple applications. SDL Can provide OpenGL support
       for render interface also. This can be mounted against
       bag.video also, for developing window support. This has
       fallen by the wayside... and really a display interface
       should be provided that can just open X displays directly to
       copy <link sack::image::Image, Images> to.
     * bag.psi &#45; provides the PSI namespace.                      */
FILESYS_NAMESPACE

	enum ScanFileFlags {

SFF_SUBCURSE    = 1, // go into subdirectories
SFF_DIRECTORIES = 2, // return directory names also
SFF_NAMEONLY    = 4, // don't concatenate base with filename to result.
SFF_IGNORECASE  = 8 // when matching filename - do not match case.
	};

 // flags sent to Process when called with a matching name
enum ScanFileProcessFlags{
SFF_DIRECTORY  = 1, // is a directory...
		SFF_DRIVE      = 2, // this is a drive...
};

/* \ \ 
   Parameters
   mask :      This is the mask used to compare 
   name :      this is the name to compare against using the mask.
   keepcase :  if TRUE, must match case also.
   
   Returns
   TRUE if name is matched by mask. Otherwise returns FALSE.
   Example
   <code lang="c++">
   if( CompareMask( "*.exe", "program.exe", FALSE ) )
   {
       // then program.exe is matched by the mask.
   }
   </code>
   Remarks
   The mask support standard 'globbing' characters.
   
   ? matches one character
   
   \* matches 0 or more characters
   
   otherwise the literal character must match, unless comparing
   case insensitive, in which case 'A' == 'a' also.                */
FILESYS_PROC  int FILESYS_API  CompareMask ( CTEXTSTR mask, CTEXTSTR name, int keepcase );

// ScanFiles usage:
//   base - base path to scan
//   mask - file mask to process if NULL or "*" is everything "*.*" must contain a .
//   pInfo is a pointer to a void* - this pointer is used to maintain
//        internal information... 
//   Process is called with the full name of any matching files
//   subcurse is a flag - set to go into all subdirectories looking for files.
// There is no way to abort the scan... 
FILESYS_PROC  int FILESYS_API  ScanFiles ( CTEXTSTR base
           , CTEXTSTR mask
           , void **pInfo
           , void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
           , int flags 
           , PTRSZVAL psvUser );
FILESYS_PROC  void FILESYS_API  ScanDrives ( void (CPROC *Process)(PTRSZVAL user, CTEXTSTR letter, int flags)
										  , PTRSZVAL user );
// result is length of name filled into pResult if pResult == NULL && nResult = 0
// the result will the be length of the name matching the file.
FILESYS_PROC  int FILESYS_API  GetMatchingFileName ( CTEXTSTR filemask, int flags, TEXTSTR pResult, int nResult );

// searches a path for the last '/' or '\'
FILESYS_PROC  CTEXTSTR FILESYS_API  pathrchr ( CTEXTSTR path );
#ifdef __cplusplus
FILESYS_PROC  TEXTSTR FILESYS_API  pathrchr ( TEXTSTR path );
#endif
// searches a path for the first '/' or '\'
FILESYS_PROC  CTEXTSTR FILESYS_API  pathchr ( CTEXTSTR path );

// returns pointer passed (if it worked?)
FILESYS_PROC  TEXTSTR FILESYS_API  GetCurrentPath ( TEXTSTR path, int buffer_len );
FILESYS_PROC  int FILESYS_API  SetCurrentPath ( CTEXTSTR path );
/* Creates a directory. If parent peices of the directory do not
   exist, those parts are created also.
   
   
   Example
   <code lang="c#">
   MakePath( "c:\\where\\I'm/going/to/store/data" ); 
   </code>                                                       */
FILESYS_PROC  int FILESYS_API  MakePath ( CTEXTSTR path );
/* A boolean result function whether a specified name is a
   directory or not. (if not, assumes it's a file).
   
   
   Example
   <code lang="c#">
   if( IsPath( "c:/windows" ) )
   {
       // if yes, then c:\\windows is a directory.
   }
   </code>                                                 */
FILESYS_PROC  int FILESYS_API  IsPath ( CTEXTSTR path );


FILESYS_PROC  _64 FILESYS_API  GetFileWriteTime ( CTEXTSTR name );
FILESYS_PROC  _64 FILESYS_API  GetTimeAsFileTime ( void );
FILESYS_PROC  LOGICAL FILESYS_API  SetFileWriteTime( CTEXTSTR name, _64 filetime ); 


FILESYS_PROC  void FILESYS_API  SetDefaultFilePath ( CTEXTSTR path );
FILESYS_PROC  int FILESYS_API  SetGroupFilePath ( CTEXTSTR group, CTEXTSTR path );
FILESYS_PROC  TEXTSTR FILESYS_API  sack_prepend_path ( int group, CTEXTSTR filename );


/* This is a new feature added for supporting systems without a
   current file location. This gets an integer ID of a group of
   files by name.
   
   the name 'default' is used to specify files to go into the
   'current working directory'
   
   
   Parameters
   groupname :     name of the group
   default_path :  the path of the group, if the name is not
                   found.
   
   Returns
   the ID of a file group.
   Example
   <code lang="c++">
   int group = GetFileGroup( "fonts", "./fonts" );
   </code>                                                      */
FILESYS_PROC  int FILESYS_API  GetFileGroup ( CTEXTSTR groupname, CTEXTSTR default_path );

/* \Returns the size of the file.
   
   
   Parameters
   name :  name of the file to get information about
   
   Returns
   \Returns the size of the file. or -1 if the file did not
   exist.                                                   */
FILESYS_PROC  _32 FILESYS_API  GetSizeofFile ( TEXTCHAR *name, P_32 unused );
/* An extended function, which returns a _64 bit time
   appropriate for the current platform. This is meant to
   replace 'stat'. It can get all commonly checked attributes of
   a file.
   
   
   Parameters
   name :              name of the file to get information about
   lpCreationTime :    pointer to a FILETIME type to get creation
                       time. can be NULL.
   lpLastAccessTime :  pointer to a FILETIME type to get access
                       time. can be NULL.
   lpLastWriteTime :   pointer to a FILETIME type to get write
                       time. can be NULL.
   IsDirectory :       pointer to a LOGICAL to receive indicator
                       whether the file was a directory. can be
                       NULL.
   
   Returns
   \Returns the size of the file. or -1 if the file did not
   exist.                                                         */
FILESYS_PROC  _32 FILESYS_API  GetFileTimeAndSize ( CTEXTSTR name
													, LPFILETIME lpCreationTime
													,  LPFILETIME lpLastAccessTime
													,  LPFILETIME lpLastWriteTime
													, int *IsDirectory
													);


// can use 0 as filegroup default - single 'current working directory'
#define _NO_OLDNAMES
//#ifdef UNDER_CE
# ifndef O_RDONLY


#define O_RDONLY       0x0000  /* open for reading only */
#define O_WRONLY       0x0001  /* open for writing only */
#define O_RDWR         0x0002  /* open for reading and writing */
#define O_APPEND       0x0008  /* writes done at eof */

#define O_CREAT        0x0100  /* create and open file */
#define O_TRUNC        0x0200  /* open and truncate */
#define O_EXCL         0x0400  /* open only if file doesn't already exist */


#  ifndef S_IRUSR
#define S_IRUSR 1
#define S_IWUSR 2
#  endif
//# endif
#endif

FILESYS_PROC  HANDLE FILESYS_API  sack_open ( int group, CTEXTSTR filename, int opts, ... );
FILESYS_PROC  HANDLE FILESYS_API  sack_openfile ( int group, CTEXTSTR filename, OFSTRUCT *of, int flags );
FILESYS_PROC  HANDLE FILESYS_API  sack_creat ( int group, CTEXTSTR file, int opts, ... );
FILESYS_PROC  int FILESYS_API  sack_close ( HANDLE file_handle );
FILESYS_PROC  int FILESYS_API  sack_lseek ( HANDLE file_handle, int pos, int whence );
FILESYS_PROC  int FILESYS_API  sack_read ( HANDLE file_handle, CPOINTER buffer, int size );
FILESYS_PROC  int FILESYS_API  sack_write ( HANDLE file_handle, CPOINTER buffer, int size );

FILESYS_PROC  int FILESYS_API  sack_iopen ( int group, CTEXTSTR filename, int opts, ... );
FILESYS_PROC  int FILESYS_API  sack_iopenfile ( int group, CTEXTSTR filename, OFSTRUCT *of, int flags );
FILESYS_PROC  int FILESYS_API  sack_icreat ( int group, CTEXTSTR file, int opts, ... );
FILESYS_PROC  int FILESYS_API  sack_iclose ( int file_handle );
FILESYS_PROC  int FILESYS_API  sack_ilseek ( int file_handle, int pos, int whence );
FILESYS_PROC  int FILESYS_API  sack_iread ( int file_handle, CPOINTER buffer, int size );
FILESYS_PROC  int FILESYS_API  sack_iwrite ( int file_handle, CPOINTER buffer, int size );

FILESYS_PROC  FILE* FILESYS_API  sack_fopen ( int group, CTEXTSTR filename, CTEXTSTR opts );
FILESYS_PROC  int FILESYS_API  sack_fclose ( FILE *file_file );
FILESYS_PROC  int FILESYS_API  sack_fseek ( FILE *file_file, int pos, int whence );
FILESYS_PROC  int FILESYS_API  sack_fread ( CPOINTER buffer, int size, int count,FILE *file_file );
FILESYS_PROC  int FILESYS_API  sack_fwrite ( CPOINTER buffer, int size, int count,FILE *file_file );

FILESYS_PROC  int FILESYS_API  sack_unlink ( CTEXTSTR filename );
FILESYS_PROC  int FILESYS_API  sack_rename ( CTEXTSTR file_source, CTEXTSTR new_name );

#if !defined( SACK_BAG_EXPORTS ) && !defined( BAG_EXTERNALS )
#define open(a,...) sack_iopen(0,a,##__VA_ARGS__)
#define _lopen(a,...) sack_open(0,a,##__VA_ARGS__)
#define lseek(a,b,c) sack_ilseek(a,b,c)
#define _llseek(a,b,c) sack_lseek(a,b,c)

#define HFILE HANDLE
#undef HFILE_ERROR
#define HFILE_ERROR INVALID_HANDLE_VALUE

#define creat(a,...)  sack_icreat( 0,a,##__VA_ARGS__ )
#define close(a)  sack_iclose(a)
#define OpenFile(a,b,c) sack_openfile(0,a,b,c)
#define _lclose(a)  sack_close(a)
#define read(a,b,c) sack_iread(a,b,c)
#define write(a,b,c) sack_iwrite(a,b,c)
#define _lread(a,b,c) sack_read(a,b,c)
#define _lwrite(a,b,c) sack_write(a,b,c)
#define _lcreat(a,b) sack_creat(0,a,b)

#define remove(a)   sack_unlink(a)
#define unlink(a)   sack_unlink(a)
#endif

#if UNICODE
#define fprintf fwprintf
#define fputs fputws
#endif


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
