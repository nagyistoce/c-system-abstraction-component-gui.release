
#define DISABLE_DEBUG_REGISTER_AND_DISPATCH
#if 0
#define LIBRARY_DEADSTART

#include "deadstart.c"
#endif

#if defined( GCC )
#ifndef __cplusplus
#include <stdhdrs.h>
#include <deadstart.h>

static void RegisterStartups( void ) __attribute__((constructor));
//PRIORITY_PRELOAD( RunStartups, 25 )
// This becomes the only true contstructor...
// this is loaded in the main program, and not in a library
// this ensures that the libraries registration (if any)
// is definatly done to the main application
//(the one place for doing the work)
#ifndef LIBRARY_DEADSTART
void ctor_RunStartups( void ) __attribute__((constructor));
#endif


static int Registered;
// this one is used when the library loads.  (there is only one of these.)
// and constructors are run every time a library is loaded....
// I wonder whose fault that is....
void RegisterStartups( void )
{
#define paste(a,b) a##b
#define paste2(a,b) paste(a,b)
#define DeclareList(n) paste2(n,TARGET_LABEL)
   extern struct rt_init DeclareList( begin_deadstart_ );
   extern struct rt_init DeclareList( end_deadstart_ );
	struct rt_init *begin = &DeclareList( begin_deadstart_ );
	struct rt_init *end = &DeclareList( end_deadstart_ );
	struct rt_init *current;
#ifdef __NO_BAG__
   printf( "Not doing deadstarts\n" );
	return;
#endif
   Registered=1;
   //cygwin_dll_init();
	if( begin[0].scheduled )
      return;
	if( (begin+1) < end )
	{
#ifdef __CYGWIN__
		void (*MyRegisterPriorityStartupProc)( void (*proc)(void), CTEXTSTR func,int priority, CTEXTSTR file,int line );
		char myname[256];
      HMODULE mod;
      GetModuleFileName(NULL,myname,sizeof(myname));
		mod = LoadLibrary( myname );GetModuleFileName(NULL,myname,sizeof(myname));
		MyRegisterPriorityStartupProc = (void(*)( void(*)(void),CTEXTSTR,int,CTEXTSTR,int))GetProcAddress( mod, "RegisterPriorityStartupProc" );
#ifdef DEBUG_CYGWIN_START
		fprintf( stderr, "mod is %p proc  is %p %s\n", mod, MyRegisterPriorityStartupProc, TARGETNAME );
#endif
		if( MyRegisterPriorityStartupProc )
		{
#endif
		for( current = begin + 1; current < end; current++ )
		{
			if( !current[0].scheduled )
			{
#ifndef  DISABLE_DEBUG_REGISTER_AND_DISPATCH
				lprintf( WIDE("Register %d %s@%s(%d)"), current->priority, current->funcname, current->file, current->line );
#endif
#ifdef __CYGWIN__
#ifdef DEBUG_CYGWIN_START
				fprintf( stderr, WIDE("Register %d %s@%s(%d)\n"), current->priority, current->funcname, current->file, current->line );
#endif
				MyRegisterPriorityStartupProc( current->routine, current->funcname, current->priority, current->file, current->line );
#else
				RegisterPriorityStartupProc( current->routine, current->funcname, current->priority, NULL, current->file, current->line );
#endif
            current[0].scheduled = 1;
			}
			else
			{
#ifndef  DISABLE_DEBUG_REGISTER_AND_DISPATCH
				lprintf( WIDE("Not Register(already did this once) %d %s@%s(%d)"), current->priority, current->funcname, current->file, current->line );
#endif
			}
#ifdef __CYGWIN__
		}
#endif
		}
	}
   // should be setup in such a way that this ignores all external invokations until the core app runs.
	//InvokeDeadstart();
}
#endif
#endif
