#include <sack_types.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#if defined( _WIN32 ) && !defined( __TURBOC__ )
#include <stdhdrs.h> // windows lean_and_mean
#ifndef UNDER_CE
#include <io.h>  // findfirst,findnext, fileinfo
#endif
#else
#include <dirent.h> // opendir etc..
#include <sys/stat.h>
#endif
#include <filesys.h>

FILESYS_NAMESPACE

#define MAX_PATH_NAME 512

// DEBUG_COMPARE 1 == full debug
// DEBUG_COMPARE 2 == quieter debug
#ifdef _DEBUG
#define DEBUG_COMPARE 3
#else
#define DEBUG_COMPARE 999
#endif
//--------------------------------------------------------------------------

 int  CompareMask ( CTEXTSTR mask, CTEXTSTR name, int keepcase )
{
	int m = 0, n = 0;
	int anymatch;
	int wasanymatch, wasmaskmatch;
	int matchone;
	TEXTCHAR namech, maskch;
	if( !mask )
		return 1;
	if( !name )
		return 0;
try_mask:
	anymatch = 0;
	wasanymatch = 0;
	wasmaskmatch = 0;
	matchone = 0;
#if ( DEBUG_COMPARE < 3 )
	lprintf( WIDE("Check %s vs %s"), mask + m, name );
#endif
	do
	{
		if( mask[m] == '\t' || mask[m] == '|' )
		{
			//Log1( WIDE("Found mask seperator - skipping to next mask :%s"), name );
			n = 0;
			m++;
         continue;
		}
		while( mask[m] == '*' )
		{
			anymatch = 1;
			m++;
		}
		while( mask[m] == '?' )
		{
#if ( DEBUG_COMPARE < 2 )
         //Log( WIDE("Match any one character") );
#endif
			matchone++;
         m++;
		}
		if( !keepcase && name[n]>= 'a' && name[n] <= 'z' )
			namech = name[n] - ('a' - 'A');
		else
			namech = name[n];
		if( !keepcase && mask[m]>= 'a' && mask[m] <= 'z' )
			maskch = mask[m] - ('a' - 'A');
		else
			maskch = mask[m];
      if( matchone )
		{
			matchone--;
			n++;
		}
		else if(
#if ( DEBUG_COMPARE < 2 )
				  lprintf( WIDE("Check %c == %c?"), maskch, namech ),
#endif
				  maskch == namech )
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" yes.") );
#endif
		 	if( anymatch )
		 	{
			 	wasanymatch = n;
			 	wasmaskmatch = m;
			 	anymatch = 0;
			}
		 	n++;
	 		m++;
		}
		else if(
#if ( DEBUG_COMPARE < 2 )
				  lprintf( WIDE(" no. Any match?") ),
#endif
				  anymatch )
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" yes"));
#endif
			n++;
		}
		else if(
#if ( DEBUG_COMPARE < 2 )
				  lprintf( WIDE(" No. wasanymatch?") ),
#endif
				  wasanymatch )
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" yes. reset to anymatch.") );
#endif
			n = wasanymatch;
			m = wasmaskmatch;
			anymatch = 1;
			n++;
		}
		else
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" No. match failed.") );
#endif
			break;
		}
	}while( name[n] );
	// 0 or more match a *
   // so auto match remaining *
	while( mask[m] && mask[m] == '*' )
      m++;
#if ( DEBUG_COMPARE < 3 )
	lprintf( WIDE("Skipping to next mask") );
#endif
	if( mask[m] &&
	    ( mask[m] != '\t' &&
		   mask[m] != '|' ) )
	{
		int mask_m = m;
		while( mask[m] )
		{
			if( mask[m] == '\t' || mask[m] == '|' )
			{
				n = 0;
				m++;
				break;
			}
			m++;
		}
		if( mask[m] )
			goto try_mask;
      m = mask_m;
	}
   //lprintf( WIDE("Result: %d %c %d"), matchone, mask[m], name[n] );
	// match ???? will not match abc 
	// a??? abc not match
	if( !matchone && (!mask[m] || mask[m] == '\t' || mask[m] == '|' ) && !name[n] )
		return 1;
	return  0;
}

//--------------------------------------------------------------------------
//---------------------------------------------------------------------------

typedef struct result_buffer
{
	TEXTSTR buffer;
	int len;
   int result_len;
} RESULT_BUFFER, *PRESULT_BUFFER;

static void CPROC MatchFile( PTRSZVAL psvUser, CTEXTSTR name, int flags )
{
   PRESULT_BUFFER buffer = (PRESULT_BUFFER)psvUser;
   buffer->result_len = snprintf( buffer->buffer, buffer->len, WIDE("%s"), name );
}

 int  GetMatchingFileName ( CTEXTSTR filemask, int flags, TEXTSTR pResult, int nResult )
{
	void *info = NULL;
	RESULT_BUFFER result_buf;
	result_buf.buffer = pResult;
	result_buf.len = nResult;
	result_buf.result_len = 0;
	// may need a while loop here...
   // but I'm just going to result the first matching anyhow.
   while( ScanFiles( NULL, filemask, &info, MatchFile, flags, (PTRSZVAL)&result_buf ) );
   return result_buf.result_len;
}

//---------------------------------------------------------------------------

#if defined( _WIN32 ) && !defined( __TURBOC__ )

#ifdef UNDER_CE
#define finddata_t WIN32_FIND_DATA
#define findfirst FindFirstFile
#define findnext  FindNextFile
#define findclose FindClose
#else
#  ifdef UNICODE
#define finddata_t _wfinddata_t
#define findfirst _wfindfirst
#define findnext  _wfindnext
#define findclose _findclose
#  else
#define finddata_t _finddata_t
#define findfirst _findfirst
#define findnext  _findnext
#define findclose _findclose
#  endif
#endif


typedef struct myfinddata {
# ifdef _MSC_VER
#define HANDLECAST (HANDLE)
	HANDLE
# else
#define HANDLECAST
	int 
# endif
		handle;
# ifdef UNDER_CE
	WIN32_FIND_DATA fd;
# else
#  ifdef UNICODE
	struct _wfinddata_t fd;
#  else
   struct finddata_t fd;
#  endif
# endif
   TEXTCHAR buffer[MAX_PATH_NAME];
} MFD, *PMFD;

#define findhandle(pInfo) ( ((PMFD)(*pInfo))->handle)
#define finddata(pInfo) ( &((PMFD)(*pInfo))->fd)
#define findbuffer(pInfo) ( ((PMFD)(*pInfo))->buffer)


 int  ScanFiles ( CTEXTSTR base
           , CTEXTSTR mask
           , void **pInfo
           , void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
           , int flags 
           , PTRSZVAL psvUser )
{
	int sendflags;
	TEXTCHAR basename[256];
	if( base )
		StrCpy( basename, base );
	else
	{
		CTEXTSTR p = pathrchr( mask );
		if( p )
		{
			StrCpyEx( basename, mask, p - mask + 1 );
			mask = p + 1;
		}
		else
			StrCpyEx( basename, WIDE("."), sizeof( basename ) / sizeof( TEXTCHAR ) );
	}
	if( !*pInfo )
	{
		TEXTCHAR findmask[256];
		snprintf( findmask, sizeof(findmask), WIDE("%s/*"), basename );
		*pInfo = Allocate( sizeof( MFD ) );
		findhandle(pInfo) = findfirst( findmask, finddata(pInfo) );
		if( findhandle(pInfo) == HANDLECAST -1 )
		{
			findclose( findhandle(pInfo) );
			Release( *pInfo );
			(*pInfo) = NULL;
			return 0;
		}
	}
	else
	{
	getnext:
		if( findnext( findhandle(pInfo), finddata( pInfo ) ) )
		{
			findclose( findhandle(pInfo) );
			Release( *pInfo );
			(*pInfo) = NULL;
			return 0;
		}
	}
#ifdef UNDER_CE
	if( !strcmp( WIDE("."), finddata(pInfo)->cFileName ) ||
       !strcmp( WIDE(".."), finddata(pInfo)->cFileName ) )
#else
   if( !strcmp( WIDE("."), finddata(pInfo)->name ) ||
       !strcmp( WIDE(".."), finddata(pInfo)->name ) )
#endif
   	goto getnext;
	if( flags & SFF_NAMEONLY )
#ifdef UNDER_CE
		strncpy( findbuffer( pInfo ), finddata(pInfo)->cFileName, MAX_PATH_NAME );
#else
		strncpy( findbuffer( pInfo ), finddata(pInfo)->name, MAX_PATH_NAME );
#endif
   else 
#ifdef UNDER_CE
		snprintf( findbuffer( pInfo ), MAX_PATH_NAME, WIDE("%s/%s"), basename, finddata(pInfo)->cFileName );
#else
	snprintf( findbuffer( pInfo ), MAX_PATH_NAME, WIDE("%s/%s"), basename, finddata(pInfo)->name );
#endif
	findbuffer( pInfo )[MAX_PATH_NAME-1] = 0; // force nul termination...
   if( ( flags & (SFF_DIRECTORIES|SFF_SUBCURSE) )
#ifdef UNDER_CE
       && finddata(pInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
#else
       && finddata(pInfo)->attrib & _A_SUBDIR )
#endif
   {
		if( flags & SFF_DIRECTORIES )
		{
			if( Process != NULL )
				Process( psvUser, findbuffer( pInfo ), SFF_DIRECTORY );
         return 1;
		}
		if( flags & SFF_NAMEONLY ) // if nameonly - have to rebuild the correct name.
#ifdef UNDER_CE
			snprintf( findbuffer( pInfo ), MAX_PATH_NAME, WIDE("%s/%s"), basename, finddata(pInfo)->cFileName );
#else
			snprintf( findbuffer( pInfo ), MAX_PATH_NAME, WIDE("%s/%s"), basename, finddata(pInfo)->name );
#endif
		if( flags & SFF_SUBCURSE  )
   	{
			void *data = NULL;
	   	while( ScanFiles( findbuffer(pInfo), mask, &data, Process, flags, psvUser ) );
	   }
   	goto getnext;
	}
	if( ( sendflags = SFF_DIRECTORY, ( ( flags & SFF_DIRECTORIES ) 
#ifdef UNDER_CE
			&& ( finddata(pInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) )
#else
			&& ( finddata(pInfo)->attrib & _A_SUBDIR ) ) )
#endif
|| ( sendflags = 0, CompareMask( mask
#ifdef UNDER_CE
										 , finddata(pInfo)->cFileName
#else
										 , finddata(pInfo)->name
#endif
												  // yes this is silly - but it's correct...
												 , (flags & SFF_IGNORECASE)?0:0 ) ) )
   { 
		if( Process != NULL )
			Process( psvUser, findbuffer( pInfo ), sendflags );
	   return 1;
	}
	return 1;
}

//---------------------------------------------------------------------------

 void  ScanDrives ( void (CPROC*Process)(PTRSZVAL user, CTEXTSTR letter, int flags)
									, PTRSZVAL user )
{
#ifdef UNDER_CE
	Process( user, WIDE(""), SFF_DRIVE );
#else
	_32 drives;
	int i;
	drives = GetLogicalDrives();
	for( i = 0; i < 26; i++ )
	{
		TEXTCHAR name[2];
		name[1] = 0;
		if( drives & ( 1 << i ) ) 
		{
			name[0] = 'A' + i;
			if( Process )
				Process( user, name, SFF_DRIVE );
		}
	}
#endif

}

#else

//---------------------------------------------------------------------------
typedef struct myfinddata {
	int handle;
   //struct finddata_t fd;
   char buffer[MAX_PATH_NAME];
} MFD, *PMFD;

//---------------------------------------------------------------------------
 int  ScanFiles ( CTEXTSTR base
           , CTEXTSTR mask
           , void **pInfo
           , void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
           , int flags
           , PTRSZVAL psvUser )
{
	DIR *dir;
   char name[256];
	struct dirent *de;
	char basename[256];
	// need to dup base - it might be in read-only space.
	if( !*pInfo )
	{
		if( base )
			strcpy( basename, base );
		else
		{
			CTEXTSTR p = pathrchr( mask );
			if( p )
			{
				strncpy( basename, mask, p - mask );
				basename[p-mask] = 0;
				mask = p + 1;
			}
			else
				strcpy( basename, WIDE(".") );
		}
		//lprintf( WIDE("Check base: %s"), basename  );
		dir = opendir( basename );
		*pInfo = (void*)dir;
	}
	else
	{
      dir = (DIR*)*pInfo;
	}
	{
		char *p;
	// result from pathrchr is within basename
	// it's result si technically a CTEXTSTR since
	// that is what is passed to pathrchr
		if( ( p = (char*)pathrchr( basename ) ) )
		{
			if( !p[1] )
			{
				p[0] = 0;
			}
		}
	}
   if( dir )
		while( ( de = readdir( dir ) ) )
		{
			struct stat filestat;
			//lprintf( WIDE("Check: %s"), de->d_name );
			// should figure a way to check file vs mask...
			if( !strcmp( WIDE("."), de->d_name ) ||
				!strcmp( WIDE(".."), de->d_name ) )
				continue;
			sprintf( name, WIDE("%s/%s"), basename, de->d_name );
#ifdef BCC32
			if( stat( name, &filestat ) == -1 )
				continue;
#else
			if( lstat( name, &filestat ) == -1 )
			{
				//lprintf( WIDE("We got problems with stat! (%s)"), name );
				continue;
			}
			if( S_ISLNK( filestat.st_mode ) )
			{
				//lprintf( WIDE("A link: %s"), name );
				// not following links...
				continue;
			}
#endif
			if( S_ISDIR(filestat.st_mode) )
			{
				if( flags & SFF_SUBCURSE )
				{
					void *data = NULL;
					if( S_ISBLK( filestat.st_mode ) ||
						S_ISCHR( filestat.st_mode ) )
                  continue;
		   		if( flags & SFF_DIRECTORIES ) 
						if( Process )
							Process( psvUser
							       , (flags & SFF_NAMEONLY)?de->d_name:name
							       , SFF_DIRECTORY );
					while( ScanFiles( name, mask, &data, Process, flags, psvUser ) );
				}
				continue;
			}
			if( CompareMask( mask, de->d_name, (flags & SFF_IGNORECASE)?0:1 ) )
			{
				if( Process )
					Process( psvUser
					       , (flags & SFF_NAMEONLY)?de->d_name:name
					       , 0 );
				return 1;
			}
		}
	closedir( dir );
	*pInfo = NULL;
	return 0;
}

//---------------------------------------------------------------------------


 void  ScanDrives ( void(*Process)(PTRSZVAL user, CTEXTSTR letter, int flags)
                                 , PTRSZVAL psv )
{

}
//---------------------------------------------------------------------------
#endif

FILESYS_NAMESPACE_END

// $Log: filescan.c,v $
// Revision 1.28  2005/03/26 02:49:53  panther
// Well the windows filename simile matcher seems to work
//
// Revision 1.27  2005/01/27 07:25:47  panther
// Linux - well as clean as it can be with libC sucking.
//
// Revision 1.26  2003/12/18 11:19:52  panther
// Fix debug logging/check issue when doing RElease
//
// Revision 1.25  2003/12/16 23:09:46  panther
// Fix mask test for 'FILE*' matching 'FILE'
//
// Revision 1.24  2003/11/04 11:39:37  panther
// Use | or \t to seperate masks
//
// Revision 1.23  2003/09/26 14:20:41  panther
// PSI DumpFontFile, fix hide/restore display
//
// Revision 1.22  2003/08/21 14:48:45  panther
// remove makefile warning
//
// Revision 1.21  2003/04/21 20:02:31  panther
// Support option to return file name only
//
// Revision 1.20  2003/04/07 15:25:38  panther
// Close find handle
//
// Revision 1.19  2003/04/06 09:55:08  panther
// Export compare mask
//
// Revision 1.18  2003/03/25 08:45:50  panther
// Added CVS logging tag
//
