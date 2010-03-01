#include <stdhdrs.h>
#include <logging.h>
#include <sharemem.h>
#include <deadstart.h>
#include "run.h"

#ifndef LOAD_LIBNAME
#ifdef MILK_PROGRAM
#define MODE 0
#define LOAD_LIBNAME "milk.core"
#endif
#ifdef INTERSHELL_PROGRAM
#define MODE 0
#define LOAD_LIBNAME "InterShell.core.dll"
#endif
#endif


#if (MODE==0)
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow )
//int main( int argc, char **argv )
{

	char **argv;
	int argc;
	ParseIntoArgs( GetCommandLine(), &argc, &argv );

#else
int main( int argc, char **argv )
{
#endif
	{
   int arg_offset = 1;
	generic_function hModule = NULL;
	MainFunction Main;
	BeginFunction Begin;
	StartFunction Start;
	CTEXTSTR libname;
#ifdef MEMORY_DEBUG_LOG
   // define memory_debug to enable memory logging at the pre-first-load level.
	SetAllocateLogging( TRUE );
#endif
#ifndef LOAD_LIBNAME
	if( argc > 1 )
	{
		hModule = LoadFunction( libname = argv[1], NULL );
		if( hModule )
         arg_offset++;
	}
#endif

	if( !hModule )
	{
#ifdef LOAD_LIBNAME
		hModule = LoadFunction( libname = LOAD_LIBNAME, NULL );
		if( !hModule )
		{
			lprintf( "error: (%ld)%s"
					 , GetLastError()
					 , strerror(GetLastError()) );
			return 0;
		}
		else
			arg_offset = 0;
#else
		lprintf( "strerror(This is NOT right... what's the GetStrError?): (%ld)%s"
				 , GetLastError()
				 , strerror(GetLastError()) );
		return 0;
#endif
	}
	{
		// look through command line, and while -L options exist, use thsoe to load more libraries
      // then pass the final remainer to the proc (if used)
	}
	{

		Main = (MainFunction)LoadFunction( libname, "_Main" );
		if( !Main )
			Main = (MainFunction)LoadFunction( libname, "Main" );
		if( !Main )
			Main = (MainFunction)LoadFunction( libname, "Main_" );
		if( Main )
			Main( argc-arg_offset, argv+arg_offset, MODE );
		else
		{
			Begin = (BeginFunction)LoadFunction( libname, "_Begin" );
			if( !Begin )
				Begin = (BeginFunction)LoadFunction( libname, "Begin" );
			if( !Begin )
				Begin = (BeginFunction)LoadFunction( libname, "Begin_" );
			if( Begin )
			{
				int xlen, ofs, arg;
				char *x;
				for( arg = arg_offset, xlen = 0; arg < argc; arg++, xlen += snprintf( NULL, 0, "%s%s", arg?" ":"", argv[arg] ) );
				x = (char*)malloc( ++xlen );
				for( arg = arg_offset, ofs = 0; arg < argc; arg++, ofs += snprintf( x + ofs, xlen - ofs, "%s%s", arg?" ":"", argv[arg] ) );
				Begin( x, MODE ); // pass console defined in Makefile
				free( x );
			}
			else
			{
				Start = (StartFunction)LoadFunction( libname, "_Start" );
				if( !Start )
					Start = (StartFunction)LoadFunction( libname, "Start" );
				if( !Start )
					Start = (StartFunction)LoadFunction( libname, "Start_" );
				if( Start )
					Start( );
			}
		}
	}
	}
	return 0;
}
// $Log: runwin.c,v $
// Revision 1.6  2003/08/01 00:58:34  panther
// Fix loader for other alternate entry proc
//
// Revision 1.5  2003/03/25 08:59:03  panther
// Added CVS logging
//
