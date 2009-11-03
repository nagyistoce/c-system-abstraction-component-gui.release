#include <stdhdrs.h>
#include <logging.h>
#include <sharemem.h>
#include <deadstart.h>
#include "run.h"

#ifdef MILK_PROGRAM
#define MODE 0
#define LOAD_LIBNAME "milk.core"
#endif
#ifdef INTERSHELL_PROGRAM
#define MODE 0
#define LOAD_LIBNAME "InterShell.core"
#endif


#if (MODE==0)
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow )
//int main( int argc, char **argv )
{

	char **argv;
	int argc;
	{
		char *args = GetCommandLine();
		char  *p;
		char **pp;
		char quote = 0;
		{
			int count = 0;
			int lastchar;
			lastchar = ' '; // auto continue spaces...
			//lprintf( WIDE("Got args: %s"), args );
			p = args;
			while( p && p[0] )
			{
				if( quote && p[0] == quote )
				{
					p++; // okay next
					count++;
					quote = 0;
					lastchar = ' ';
				}
				else if( !quote && lastchar != ' ' && p[0] == ' ' ) // and there's a space
					count++;
				if( !quote && ( p[0] == '\"' || p[0] == '\"' ) )
					quote = p[0];

				lastchar = p[0];
				p++;
			}
			count++; // complete this argument
			if( count )
			{
				char *start;
				lastchar = ' '; // auto continue spaces...
				pp = argv = NewArray(  char *, ( count + 2 ) );
				p = args;
				quote = 0;
				count = 0; // reset the counter, used to store args.
				start = p;
				while( p[0] )
				{
					//lprintf( WIDE("check character %c %c"), lastchar, p[0] );
					if( quote && p[0] == quote )
					{
						p[0]=0;
						p++;
						pp[count++] = StrDup( start );
						start = NULL;
						quote = 0;
					}
					else if( !quote && lastchar != ' ' && p[0] == ' ' ) // and there's a space
					{
						p[0] = 0;
						pp[count++] = StrDup( start );
						start = NULL;
						p[0] = ' ';
					}
					else if( lastchar == ' ' && p[0] != ' ' )
					{
						if( !start )
							start = p;
					}
					if( p[0] == '\"' || p[0] == '\"' )
					{
						quote = p[0];
						start = p+1;
					}
					lastchar = p[0] ;
					p++;
				}
				//lprintf( WIDE("Setting arg %d to %s"), count, start );
				if( start )
					pp[count++] = StrDup( start );
				pp[count] = NULL;

			}
			argc = count;
		}
	}

#else
int main( int argc, char **argv )
{
	//CTEXTSTR lpCmdLine = GetCommandLine();
	//char *cmdline, *p, *outline;
	//char *argline;

#endif
	{
   int arg_offset = 1;
	generic_function hModule;
	MainFunction Main;
	BeginFunction Begin;
	StartFunction Start;
   CTEXTSTR libname;
	char *cmdline, *p, *outline;
#ifdef MEMORY_DEBUG_LOG
   // define memory_debug to enable memory logging at the pre-first-load level.
	SetAllocateLogging( TRUE );
#endif
   //char *argline;
	p = GetCommandLine();
   // find a space
	while( p[0] && p[0] != ' ' ) p++;
   // skip over extra spaces
	while( p[0] && p[0] == ' ' ) p++;

   //allocate the outline...
	outline = cmdline = (char*)Allocate( strlen( p ) + 1 );
	while( p[0] && p[0] != ' ' )
	{
		outline[0] = p[0];
		outline++;
		p++;
	}
	outline[0] = 0;

	hModule = LoadFunction( libname = cmdline, NULL );

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
   if( hModule )
	{
      InvokeDeadstart();
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
