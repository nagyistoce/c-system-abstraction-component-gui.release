#include <stdhdrs.h>
#include <stdio.h>
#include <sack_types.h>
#include <filesys.h>
#include <sharemem.h>

#include "pcopy.h"

extern GLOBAL g;
static PFILESOURCE copytree;

PFILESOURCE CreateFileSource( CTEXTSTR name )
{
	PFILESOURCE pfs = (PFILESOURCE)Allocate( sizeof( FILESOURCE ) );
	MemSet( pfs, 0, sizeof( FILESOURCE ) );
	pfs->name = StrDup( name );
	return pfs;
}

PFILESOURCE FindFileSource( PFILESOURCE pfs, CTEXTSTR name )
{
	PFILESOURCE pFound = NULL;
	while( pfs )
	{
		if( StrCaseCmp( name, pfs->name ) == 0 )
			pFound = pfs;
		if( !pFound ) pFound = FindFileSource( pfs->children, name );
		if( pFound ) break;
		pfs = pfs->next;
	}
	return pFound;
}

PFILESOURCE AddDependCopy( PFILESOURCE pfs, CTEXTSTR name )
{
	PFILESOURCE pfsNew;
	if( g.flags.bVerbose )
	{
		printf( WIDE("Adding %s as dependant of %s\n"), name, pfs->name );
	}
	if( !(pfsNew = FindFileSource( copytree, name ) ) )
	{
		pfsNew = CreateFileSource( name );
		pfsNew->next = pfs->children;
		pfs->children = pfsNew;
	}
	return pfsNew;
}

void CPROC DoScanFile( PTRSZVAL psv, CTEXTSTR name, int flags )
{
	PFILESOURCE pfs = CreateFileSource( name );
	pfs->next = copytree;
	copytree = pfs;
	ScanFile( copytree );
}

void AddFileCopy( CTEXTSTR name )
{
	void *p = NULL;
	if( pathrchr( name ) )
	{
		TEXTSTR tmp = StrDup( name );
		TEXTSTR x = (TEXTSTR)pathrchr( tmp );
		//lprintf( "..." );
		x[0] = 0;
		//printf( "Old path: %s\n%s\n", OSALOT_GetEnvironmentVariable( "PATH" ), tmp );
		if( !StrStr( OSALOT_GetEnvironmentVariable( "PATH" ), tmp ) )
		{
			if( !( pathrchr( tmp ) == tmp || tmp[1] == ':' ) )
			{
				TEXTSTR path = (TEXTSTR)OSALOT_GetEnvironmentVariable( "MY_WORK_PATH" );
				int len;
#ifdef __WINDOWS__
#define PATHCHAR "\\"
#else
#define PATHCHAR "/"
#endif
				TEXTSTR real_tmp = NewArray( TEXTCHAR, len = strlen(path) + strlen( tmp ) + 2 );
				snprintf( real_tmp, len, "%s"PATHCHAR"%s", path, tmp );
				Release( tmp );
				while( path = strstr( real_tmp, ".." ) )
				{
					TEXTSTR prior;
					path[-1] = 0;
					prior = (TEXTSTR)pathrchr( real_tmp );
					if( !prior )
					{
						path[-1] = PATHCHAR[0];
						break;
					}
               MemCpy( prior, path+2, strlen( path ) - 1 );
				}
            tmp = real_tmp;
				if( StrStr( OSALOT_GetEnvironmentVariable( "PATH" ), tmp ) )
				{
               goto skip_path;
				}
			}
#ifdef __WINDOWS__
			x[0] = ';';
#else
			x[0] = ':';
#endif
			x[1] = 0;
			OSALOT_PrependEnvironmentVariable( "PATH", tmp );
         Release( tmp );
			//printf( "New path: %s\n", OSALOT_GetEnvironmentVariable( "PATH" ) );
			fflush( stdout );
		}
	}
skip_path:
   while( ScanFiles( NULL, name, &p, DoScanFile, 0, 0 ) );
   //return pfs;
}


void DoScanFileCopyTree( PFILESOURCE pfs )
{
	while( pfs )
	{
		if( !pfs->flags.bScanned && !pfs->flags.bInvalid )
		{
			ScanFile( pfs );
			if( pfs->flags.bSystem )
            ; //fprintf( stderr, WIDE("Skipped system library %s\n"), pfs->name );
			else if( pfs->flags.bExternal )
            ; //fprintf( stderr, WIDE("Scanned extern library %s\n"), pfs->name );

		}
		DoScanFileCopyTree( pfs->children );
		pfs = pfs->next;
	}
}

void ScanFileCopyTree( void )
{
	DoScanFileCopyTree( copytree );
}

void copy( char *src, char *dst )
{
	static _8 buffer[4096];
	FILE *in, *out;
	_64 filetime;
	_64 filetime_dest;

	filetime = GetFileWriteTime( src );
	filetime_dest = GetFileWriteTime( dst );

	if( filetime <= filetime_dest )
      return;
	in = sack_fopen( 0, src, WIDE("rb") );
	if( in )
		out = sack_fopen( 0, dst, WIDE("wb") );
	else
		out = NULL;
	if( in && out )
	{
		int len;
		while( len = fread( buffer, 1, sizeof( buffer ), in ) )
			fwrite( buffer, 1, len, out );
	}
	if( in )
		fclose( in );
	if( out )
		fclose( out );
	g.copied++;
	SetFileWriteTime( dst, filetime );
}

void DoCopyFileCopyTree( PFILESOURCE pfs, CTEXTSTR dest )
{
	char fname[256];
	while( pfs )
	{
		const char *name;
		name = pathrchr( pfs->name );
		if( !name )
			name = pfs->name;
		snprintf( fname, sizeof( fname ), WIDE("%s/%s"), dest, name );
		//if( !IsFile( fname ) )
		{
         // only copy if the file is new...
			if( pfs->flags.bScanned && !pfs->flags.bSystem )
				copy( pfs->name, fname );
		}
		DoCopyFileCopyTree( pfs->children, dest );
      pfs = pfs->next;
	}
}

void CopyFileCopyTree( CTEXTSTR dest )
{
   DoScanFileCopyTree( copytree );
   DoCopyFileCopyTree( copytree, dest );

}

