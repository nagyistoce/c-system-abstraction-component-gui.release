
#include <stdhdrs.h>
#include <sack_types.h>
#include <string.h>
#include <filesys.h>
#include <logging.h>
#ifdef __LINUX__
#include <time.h>
#include <sys/stat.h>
#endif
//-----------------------------------------------------------------------

FILESYS_NAMESPACE

 CTEXTSTR  pathrchr ( CTEXTSTR path )
{
	CTEXTSTR end1, end2;
	end1 = strrchr( path, '\\' );
	end2 = strrchr( path, '/' );
	if( end1 > end2 )
		return end1;
   return end2;
}

#ifdef __cplusplus
 TEXTSTR  pathrchr ( TEXTSTR path )
{
	TEXTSTR end1, end2;
	end1 = strrchr( path, '\\' );
	end2 = strrchr( path, '/' );
	if( end1 > end2 )
		return end1;
   return end2;
}
#endif

//-----------------------------------------------------------------------

 CTEXTSTR  pathchr ( CTEXTSTR path )
{
	CTEXTSTR end1, end2;
	end1 = strchr( path, (int)'\\' );
	end2 = strchr( path, (int)'/' );
	if( end1 && end2 )
	{
		if( end1 < end2 )
			return end1;
      return end2;
	}
	else if( end1 )
		return end1;
	else if( end2 )
      return end2;
   return NULL;
}

//-----------------------------------------------------------------------

TEXTSTR GetCurrentPath( TEXTSTR path, int len )
{
	if( !path )
		return 0;
#ifndef UNDER_CE
#ifdef _WIN32
	GetCurrentDirectory( len, path );
#else
	getcwd( path, len );
#endif
#endif
	return path;
}

#ifndef _WIN32
static void convert( P_64 outtime, time_t *time )
{
#warning convert time function is incomplete.
	*outtime = *time;
}
#endif

//-----------------------------------------------------------------------

_64 GetTimeAsFileTime ( void )
{
#if defined( __LINUX__ )
	struct timeval tmp;
	struct timezone tz;
	FILETIME result;
	gettimeofday( &tmp, &tz );
	result = ( tmp.tv_usec * 10LL ) + ( tmp.tv_sec * 1000LL * 1000LL * 10LL );
	return result;
#else
	SYSTEMTIME st;
	FILETIME result;
	GetLocalTime( &st );
	SystemTimeToFileTime( &st, &result );
	return *(_64*)&result;
#endif
}

 _64  GetFileWriteTime( CTEXTSTR name ) // last modification time.
{
#ifdef _WIN32
	HANDLE hFile = CreateFile( name
                                  , 0 // device access?
                                  , FILE_SHARE_READ|FILE_SHARE_WRITE
                                  , NULL
                                  , OPEN_EXISTING
                                  , 0
                                  , NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		FILETIME filetime;
		//_64 realtime;
		GetFileTime( hFile, NULL, NULL, &filetime );
		CloseHandle( hFile );
		//realtime = *(_64*)&filetime;
		//realtime *= 100; // nano seconds?
		return *(_64*)&filetime;
	}
	return 0;
#else
	struct stat statbuf;
	_64 realtime;
	stat( name, &statbuf );	
	convert( &realtime, &statbuf.st_mtime );
	return realtime;
#endif	
	return 0;
}

//-----------------------------------------------------------------------

 LOGICAL  SetFileWriteTime( CTEXTSTR name, _64 filetime ) // last modification time.
{
#ifdef _WIN32
	HANDLE hFile = CreateFile( name
                                  , GENERIC_WRITE // device access?
                                  , FILE_SHARE_READ|FILE_SHARE_WRITE
                                  , NULL
                                  , OPEN_EXISTING
                                  , 0
                                  , NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		//_64 realtime;
		SetFileTime( hFile, NULL, NULL, (CONST FILETIME*)&filetime );
		CloseHandle( hFile );
		//realtime = *(_64*)&filetime;
	   //realtime *= 100; // nano seconds?
      return TRUE;
	}
	return FALSE;
#else
	struct stat statbuf;
	_64 realtime;
	stat( name, &statbuf );	
	convert( &realtime, &statbuf.st_mtime );
	return realtime;
#endif	
	return 0;
}

//-----------------------------------------------------------------------

 int  IsPath ( CTEXTSTR path )
{
	
	if( !path )
		return 0;
#ifdef _WIN32
	{
		DWORD dwResult;
		dwResult = GetFileAttributes( path );
		if( dwResult == 0xFFFFFFFF )
			return 0;
		if( dwResult & FILE_ATTRIBUTE_DIRECTORY ) 
			return 1;
		return 0;
	}
#else
        {
	struct stat statbuf;
	stat( path, &statbuf );
        return S_ISDIR( statbuf.st_mode );
        }
#endif
}

//-----------------------------------------------------------------------

 int  MakePath ( CTEXTSTR path )
{
	if( !path )
		return 0;
#ifdef _WIN32
	return CreateDirectory( path, NULL );
#else
	return !mkdir( path, -1 ); // make directory with full umask permissions
#endif
}

//-----------------------------------------------------------------------

int  SetCurrentPath ( CTEXTSTR path )
{
	int status = 1;
	if( !path )
		return 0;
#ifndef UNDER_CE
#  ifdef _WIN32
	status = SetCurrentDirectory( path );
#  else
	status = !chdir( path );
#  endif
	if( status )
	{
		TEXTCHAR tmp[256];
		path = GetCurrentPath( tmp, sizeof( tmp ) );
		SetDefaultFilePath( path );
	}
	else
	{
		TEXTCHAR tmp[256];
		lprintf( "Failed to change to [%s](%d) from %s", path, GetLastError(), GetCurrentPath( tmp, sizeof( tmp ) ) );
	}
#endif

	return status;
}

int IsAbsolutePath( CTEXTSTR path )
{
#ifdef WIN32
	if( ( path[0] && path[1] && path[2] ) &&
		 ( ( ( ( path[0] >= 'a' && path[0] <= 'z' )
			  || ( path[0] >= 'A' && path[0] <= 'Z' ) )
			&& ( path[1] == ':' )
			&& ( path[2] == '/' || path[2] == '\\' ) )
		  || ( path[0] == '/' && path[1] == '/' )
		  || ( path[0] == '\\' && path[1] == '\\' ) )
      )
      return TRUE;
#else
	if( path[0] == '/' || path[0] == '\\' )
      return TRUE;
#endif
   return FALSE;
}


FILESYS_NAMESPACE_END


//-----------------------------------------------------------------------
// $Log: pathops.c,v $
// Revision 1.14  2005/05/06 18:15:24  jim
// Add ability to set file write time.
//
// Revision 1.13  2004/05/27 21:37:39  d3x0r
// Syncpoint.
//
// Revision 1.12  2004/05/06 08:13:46  d3x0r
// Apply repsiective const changes to pathchr, pathrchr
//
// Revision 1.11  2004/01/12 08:42:16  panther
// Fix return type of pathchr
//
// Revision 1.10  2004/01/07 09:46:37  panther
// Fix pathops to handle const char * decl.  Disable logging
//
// Revision 1.9  2003/08/12 12:14:01  panther
// ...
//
// Revision 1.8  2003/04/21 20:02:31  panther
// Support option to return file name only
//
// Revision 1.7  2003/01/31 16:23:24  panther
// Cleaned for visual studio warnings
//
// Revision 1.6  2003/01/28 02:24:43  panther
// Fixes to network - common timer for network pause... minor updates which should have been commited already
//
// Revision 1.5  2002/08/16 21:41:11  panther
// Updated to compile under linux, also updated to new method of declaring
// public externals.
//
// Revision 1.4  2002/07/26 09:16:55  panther
// Added IsPath, CreatePath, SetCurrentPath, GetFileWriteTime
// Modified GetCurrentPath
//
//