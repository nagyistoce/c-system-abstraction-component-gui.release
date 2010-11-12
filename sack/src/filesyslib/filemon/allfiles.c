#define PRIMARY_FILEMON_SOURCE
#include <stdhdrs.h>
#include <deadstart.h>
#include <sqlgetoption.h>
#ifdef WIN32
//#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/stat.h>
//#include <linux/fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#  ifndef O_BINARY
#    define O_BINARY 0
#  endif
//#include <windunce.h>
#endif

#if defined( WIN32 ) && !defined( S_IFDIR )
#include <sys/stat.h>
#endif

#include <time.h>

#include <signal.h>


#include <string.h>
#include <stdio.h>

#include <stdhdrs.h>

#include <sack_types.h>
#include <sharemem.h>
#include <filesys.h>
#include <logging.h>

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif

#include "monitor.h"

FILEMON_NAMESPACE

#ifdef __cplusplus
using namespace sack::containers::queue;
#endif
//-------------------------------------------------------------------------


static void Init( void )
{
	if( !l.flags.bInit )
	{
#ifndef __NO_OPTIONS__
      l.flags.bLog = SACK_GetProfileIntEx( "SACK/File Monitor", "Enable Logging", 0, TRUE );
#endif
      l.flags.bInit = 1;
	}
}

PRELOAD( InitFileMon)
{
	Init();
}

int CPROC CompareName( PTRSZVAL psv1, PTRSZVAL psv2 )
{
	CTEXTSTR s1 = (CTEXTSTR)psv1;
	CTEXTSTR s2 = (CTEXTSTR)psv2;
   return StrCmp( s1, s2 );
}

//-------------------------------------------------------------------------

void CloseFileMonitor( PCHANGEHANDLER Change, PFILEMON filemon )
{
    if( !Change || !filemon )
        return;
	 //Log1( WIDE("Closing %p"), filemon );
    RemoveBinaryNode( Change->filelist, (POINTER)filemon, (PTRSZVAL)filemon->filename );
	 //DeleteLink( &Change->filelist, filemon );
	 if( Change->currentchange == filemon )
       Change->currentchange = NULL;
    Release( filemon );
}

//-------------------------------------------------------------------------

void CloseFileMonitors( PMONITOR monitor )
{
	PFILEMON filemon;
   //INDEX idx;
	PCHANGEHANDLER Change;
	if( !monitor )
		return;
	for( Change = monitor->ChangeHandlers; Change; Change = Change->next )
	{
		for( filemon = (PFILEMON)GetLeastNode(Change->filelist); filemon; filemon = (PFILEMON)GetGreaterNode(Change->filelist) )
		{
			CloseFileMonitor( Change, filemon );
		}
		Change->currentchange = NULL;
		DeleteLinkQueue( &Change->PendingChanges );
		DestroyBinaryTree( Change->filelist );
      Change->filelist = NULL;
      //DeleteList( &Change->filelist );
	}
	if( l.flags.bLog ) Log( WIDE("Closed all") );
}


//-------------------------------------------------------------------------

int IsDirectory( CTEXTSTR name )
{
#ifdef __cplusplus_cli
	System::String^ tmp = gcnew System::String(name);	
	if( System::IO::Directory::Exists( tmp ) )		
		return 1;
	return 0;
#else
#ifdef WIN32
	{
		_32 dwAttr = GetFileAttributes( name );
		if( dwAttr == -1 ) // uncertainty about what it really is, return ti's not a directory
         return 0;
		if( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
			return 1;
      return 0;
	}
#else
	struct stat statbuf;
	if( stat( name, &statbuf ) >= 0 && statbuf.st_mode & S_IFDIR )
		return 1;
	return 0;
#endif
#endif
}

//-------------------------------------------------------------------------

PFILEMON WatchingFile( PCHANGEHANDLER monitor, CTEXTSTR name )
{
	PFILEMON filemon;
	//INDEX idx;
	// this check could be moved
	// but then I'd have to track a filemonitor structure
	// for each directory... well this is fairly efficient
   // so I'm not significantly worried at this moment...
	if( name[0] == '.' && ( strcmp( name, WIDE(".") ) == 0 ||
		strcmp( name, WIDE("..") ) == 0 ) )
	{
      if( l.flags.bLog ) Log1( WIDE("%s is a root file path"), name );
		return (PFILEMON)2; // claim we already know about these  - stops actual updates
	}
	filemon = (PFILEMON)FindInBinaryTree( monitor->filelist, (PTRSZVAL)name );
	if( !filemon && IsDirectory( name ) )
	{
		if( l.flags.bLog ) Log1( WIDE("%s is a directory - probably skipping.."), name );
		return (PFILEMON)1;
	}
	return filemon;
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( PFILEMON, AddMonitoredFile )( PCHANGEHANDLER Change, CTEXTSTR name )
{
	PFILEMON newfile;
	PMONITOR monitor = Change->monitor;
   //_64 tick = GetCPUTick();
	if( !(newfile=WatchingFile( Change, name ) ) )
	{
		int pos;
		CTEXTSTR base;
		base = monitor->directory;
		newfile = (PFILEMON)Allocate( sizeof( FILEMON ) );
		//Log2( WIDE("Making a new file (not watched): %s/%s"), base, name );
		snprintf( newfile->name, sizeof(newfile->name), WIDE("%s/"), base );
      pos = strlen( newfile->name );
		newfile->filename = newfile->name + pos;
		StrCpyEx( newfile->filename, name, (sizeof( newfile->name )/sizeof(TEXTCHAR)) - pos );
#ifdef __cplusplus_cli
		newfile->lastmodifiedtime = gcnew System::DateTime();
#endif
		newfile->flags.bScanned         = 0;
		newfile->flags.bToDelete        = 0;
		newfile->flags.bDirectory       = 0;
      newfile->flags.bPending         = 0;
		newfile->flags.bCreated         = Change->flags.bInitial?0:1;
		if( l.flags.bLog ) lprintf( WIDE("New file %s is marked as created? %s"), name, newfile->flags.bCreated?"yes":"no" );
#ifdef WIN32
		(*(_64*)&newfile->lastmodifiedtime) = 0;
#else
		newfile->lastmodifiedtime = 0;
#endif
		newfile->lastknownsize    = 0;
      if( l.flags.bLog ) lprintf( WIDE("Setting monitor do scan time - new file monitor") );
		monitor->DoScanTime =
			newfile->ScannedAt = timeGetTime() + monitor->scan_delay;
		AddBinaryNode( Change->filelist, newfile, (PTRSZVAL)newfile->filename );
      BalanceBinaryTree( Change->filelist );
//      AddLink( &Change->filelist, newfile );
	}
	else
	{
		if( newfile != (PFILEMON)1 && // iS NOT a directory name
			newfile != (PFILEMON)2 )  // iS NOT . or .. directories
		{
         // IS a file.
			newfile->flags.bScanned = FALSE; // okay need to get new information about this.
         newfile->flags.bToDelete = FALSE; // present - not deleted.
		}
	}
	//{
	//	_64 tick2 = GetCPUTick();
   //   lprintf( "Delta %Ld (%d)", tick2-tick, ConvertTickToMicrosecond( tick2-tick ) );
	//}
	return newfile;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC ScanFile( PTRSZVAL psv, INDEX idx, POINTER *item )
{
	PCHANGEHANDLER Change = (PCHANGEHANDLER)psv;
	PFILEMON filemon = (PFILEMON)(*item);
#ifdef __cplusplus_cli
	_64 dwSize, dwBigSize;
#else
	_32 dwSize;
#ifdef WIN32
#else
	struct stat statbuf;
#endif
#endif
	// if this file is already pending a change
	// do not re-enque... if it's changing THIS often
	// such that a idle time has elapsed between changes
	// and just when we start to queue it for event dispatch
	// it changes AGAIN ... whatever... we don't like these
	// kind of logging stream files - but they'll work....
	// eventually a free scan will happen even if we end up
	// losing the very last change announcement... but
	// even then ... blah it'll all be good... the clock
	// mechanism may get extra wait states, but the notice will
   // be moved forward...
	if( filemon->flags.bPending )
	{
		if( l.flags.bLog ) lprintf( WIDE("%s Already pending.... do not scan yet."), filemon->name );
		return 0;
	}
	if( !filemon->flags.bScanned )
	{
#ifdef __cplusplus_cli
		System::DateTime lastmodified;
		System::String ^filemonname = gcnew System::String( filemon->name );
		System::IO::FileInfo ^info= gcnew System::IO::FileInfo( filemonname );
		//dwSize == System::IO::FileInfo::Length::get();
#else
		FILETIME lastmodified;
#endif
		filemon->flags.bScanned = TRUE;
		if( filemon->flags.bDirectory )
			dwSize = 0xFFFFFFFF;
		else
		{
#ifdef __cplusplus_cli
			dwSize = info->Length;
#else
			dwSize = GetFileTimeAndSize( filemon->name, NULL, NULL, &lastmodified, NULL );
#endif
		}
		//lprintf( WIDE("File change stats: %s(%s) %lu %lu, %lu %lu, %s")
		//		 , filemon->name
		//		 , filemon->filename
		//		 , dwSize
		//		 , filemon->lastknownsize
		 //  	 , statbuf.st_mtime, filemon->lastmodifiedtime
		 //  	 , filemon->flags.bToDelete?"delete":"" );
		if( dwSize != filemon->lastknownsize
#ifdef __cplusplus_cli
			|| filemon->lastmodifiedtime->CompareTo( (lastmodified = info->LastWriteTime ) )
#else
			|| (*(_64*)&lastmodified) != (*(_64*)&filemon->lastmodifiedtime)
#endif
			|| filemon->flags.bToDelete
		  )
		{
			filemon->lastknownsize = dwSize;
#ifdef __cplusplus_cli
			filemon->lastmodifiedtime = lastmodified;
#else
#ifdef WIN32
			(*(_64*)&filemon->lastmodifiedtime) = (*(_64*)&lastmodified);
#else
			filemon->lastmodifiedtime = statbuf.st_mtime;
#endif
#endif
			if( !filemon->flags.bToDelete && !filemon->flags.bCreated )
			{
				if( dwSize == 0xFFFFFFFF )
				{
					//Log1( WIDE("Directory %s changed"), filemon->name );
				}
				else
				{
					//Log1( WIDE("File %s changed"), filemon->name );
				}
				filemon->ScannedAt = timeGetTime() + Change->monitor->scan_delay;
				if( l.flags.bLog ) lprintf( WIDE("file changed Setting monitor do scan time - new file monitor") );
				Change->monitor->DoScanTime = filemon->ScannedAt;
				//Log( WIDE("A file changed... setting file's change time at now plus delay") );
				//filemon->lastknownsize = dwSize;
				//filemon->lastmodifiedtime = statbuf.st_mtime;
            return 0;
			}
			else
			{
            // if created or deleted, immediatly pend the change.
				if( l.flags.bLog ) Log1( WIDE("Enque (pend) filemon for a change...%s"), filemon->name );
            filemon->flags.bPending = 1;
				EnqueLink( &Change->PendingChanges, filemon );
			}
		}
	}
	if( !Change->flags.bInitial &&
	    !filemon->flags.bToDelete &&
		 filemon->ScannedAt &&
		 filemon->ScannedAt < timeGetTime() )
	{
		//Log(" File didn't change - but it did before..." );
		if( l.flags.bLog ) Log1( WIDE("Enque (pend) filemon for a change...%s"), filemon->name );
      filemon->flags.bPending = 1;
		EnqueLink( &Change->PendingChanges, filemon );
	}
	return 0;
}


//-------------------------------------------------------------------------

static void ScanMonitorFiles( PMONITOR monitor )
{
	PCHANGEHANDLER Change;
	for( Change = monitor->ChangeHandlers; Change; Change = Change->next )
	{
      PFILEMON filemon;
		for( filemon = (PFILEMON)GetLeastNode( Change->filelist )
			 ; filemon
			  ; filemon = (PFILEMON)GetGreaterNode( Change->filelist ) )
		{
			ScanFile( (PTRSZVAL)Change, 0, (POINTER*)&filemon );
		}
		//while( ForAllLinks( &Change->filelist, ScanFile, (PTRSZVAL)Change ) );
	}
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( void, MonitorForgetAll )( PMONITOR monitor )
{
	PCHANGEHANDLER Change;
	EnterCriticalSec( &monitor->cs );
	for( Change = monitor->ChangeHandlers; Change; Change = Change->next )
	{
		PFILEMON filemon;
		//INDEX idx;
      for( filemon = (PFILEMON)GetLeastNode( Change->filelist ); filemon; filemon = (PFILEMON)GetGreaterNode( Change->filelist ) )
		//LIST_FORALL( Change->filelist, idx, PFILEMON, filemon )
		{
			if( !filemon->flags.bPending )
			{
				filemon->flags.bScanned = FALSE;
				filemon->flags.bToDelete = 1;
				filemon->lastknownsize = 0;
#ifdef WIN32
				(*(_64*)&filemon->lastmodifiedtime) = 0;
#else
				filemon->lastmodifiedtime = 0;
#endif
			}
		}
	}
	if( l.flags.bLog ) lprintf( WIDE("Setting monitor do scan time - forget all files") );

	monitor->DoScanTime = timeGetTime() - 1;
	LeaveCriticalSec( &monitor->cs );
}

//-------------------------------------------------------------------------

static void DeleteAll( PMONITOR monitor )
{
	// called while in critical section
	PCHANGEHANDLER Change;
	for( Change = monitor->ChangeHandlers; Change; Change = Change->next )
	{
		PFILEMON filemon;
		//INDEX idx;
		for( filemon = (PFILEMON)GetLeastNode( Change->filelist ); filemon; filemon = (PFILEMON)GetGreaterNode( Change->filelist ) )
			//LIST_FORALL( Change->filelist, idx, PFILEMON, filemon )
		{
			// if a change is already pending, do not mark it
         // deleted... the dispatch might happen just then.
			if( !filemon->flags.bPending )
			{
				//Log1( WIDE("Marking delete: %s"), filemon->filename );
				filemon->flags.bScanned = 0;
				filemon->flags.bToDelete = 1;
				// if the file has already been in this list once,
				// it is no longer created, otherwise it will not be here,
				// but will be added in the next stage of scanning as created.
				// created must then immediatly pend that change.
				if( !Change->flags.bInitial )
				{
					filemon->flags.bCreated = 0;
					//lprintf( WIDE("File %s is no longer created."), filemon->name );
				}
			}
		}
	}
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( int, DispatchChanges )( PMONITOR monitor )
{
	int changed = 0;
	PCHANGEHANDLER Change;
	if( monitor->flags.bClosing )
      return 0;
	EnterCriticalSec( &monitor->cs );
   monitor->flags.bDispatched = 1;
	monitor->flags.bUserInterruptedChanges = 0;
	for( Change = monitor->ChangeHandlers; Change; Change = Change->next )
	{
		while( ( Change->currentchange = (PFILEMON)DequeLink( &Change->PendingChanges ) ) )
		{
			TEXTCHAR fullname[256];
			changed++;
         Change->currentchange->ScannedAt = 0;
			snprintf( fullname, sizeof( fullname ), WIDE("%s") SYSPATHCHAR WIDE("%s"), monitor->directory, Change->currentchange->filename );
			if( l.flags.bLog ) lprintf( WIDE("Report %s for a change (%s,%s)")
					 , fullname
					 , Change->currentchange->flags.bToDelete?"deleted":""
					 , Change->currentchange->flags.bCreated?"created":""
					 );
			if( Change->HandleChange )
			{
				if( Change->flags.bExtended )
				{
					if( !Change->HandleChangeEx( Change->psv
														, fullname
														, Change->currentchange->lastknownsize
#ifdef __cplusplus_cli
														, Change->currentchange->lastmodifiedtime->Ticks
#else
														, (*(_64*)&Change->currentchange->lastmodifiedtime)
#endif
														, Change->currentchange->flags.bCreated
														, Change->currentchange->flags.bDirectory
														, Change->currentchange->flags.bToDelete ) )
					{
						monitor->flags.bUserInterruptedChanges = 1;
						break;
					}
				}
				else
				{
					if( !Change->HandleChange( Change->psv
													 , fullname
													 , Change->currentchange->flags.bToDelete ) )
					{
						monitor->flags.bUserInterruptedChanges = 1;
						break;
					}
				}
			}
			if( Change->currentchange->flags.bToDelete )
				CloseFileMonitor( Change, Change->currentchange );
			else
			{
				Change->currentchange->flags.bScanned = FALSE;
				if( l.flags.bLog ) lprintf( WIDE("Done dispatching that change... no longer pending.") );
				Change->currentchange->flags.bPending = 0;
			}
		}
		if( changed && !Change->currentchange )
		{
         // no further changes expected.
			if( Change->flags.bExtended )
				Change->HandleChangeEx( Change->psv, NULL, 0, 0, 0, 0, 0 );
			else
				Change->HandleChange( Change->psv, NULL, 0 );
		}
      //lprintf( WIDE("----------- NOW Files can be mared with CREATE ---------------") );
		Change->flags.bInitial = 0;
	}
	monitor->flags.bDispatched = 0;
	LeaveCriticalSec( &monitor->cs );
	if( monitor->flags.bEnd )
      EndMonitor( monitor );
	return changed;
}

//-------------------------------------------------------------------------

void EndMonitorFiles( void )
{
   extern PMONITOR Monitors;
	while( Monitors )
		EndMonitor( Monitors );
}

//-------------------------------------------------------------------------


void DoScan( PMONITOR monitor )
{
	// in critical section...
   //lprintf( WIDE("Tick...%s %d %d"), monitor->directory, timeGetTime(), monitor->DoScanTime );
	if( monitor->DoScanTime && ( timeGetTime() > monitor->DoScanTime ) ) 
	{
      //Log( WIDE("Doing a scan...") );
		monitor->DoScanTime = 0;
      monitor->flags.bUserInterruptedChanges = 0;
      //Log( WIDE("Delete all on monitor...") );
		DeleteAll( monitor ); // mark everything deleted
      //Log( WIDE("Scan directory...") );
		ScanDirectory( monitor ); // look for all files which match mask(s)
      //Log( WIDE("Scan files for status changes...") );
		ScanMonitorFiles( monitor ); // check for changed size/time
      //Log( WIDE("Dispatch Changes... ") );
		if( !monitor->flags.bUserInterruptedChanges )
		{
         if( monitor->DoScanTime < timeGetTime() )
				DispatchChanges( monitor ); // announce any modified files..
			else
			{
				if( l.flags.bLog ) lprintf( WIDE("Changes require further delay... %") _32f WIDE(""), monitor->DoScanTime - timeGetTime() );
			}
		}
		else
		{
			if( l.flags.bLog ) Log( WIDE("Can't report auto update - user interuppted...") );
		}
   }
}

//-------------------------------------------------------------------------

void CPROC ScanTimer( PTRSZVAL monitor )
{
   if( EnterCriticalSecNoWait( &((PMONITOR)monitor)->cs, NULL ) == 1 )
	{
      ((PMONITOR)monitor)->flags.bScanning = 1;
		DoScan( (PMONITOR)monitor );
      ((PMONITOR)monitor)->flags.bScanning = 0;
		if( ((PMONITOR)monitor)->flags.bEnd )
			EndMonitor( ((PMONITOR)monitor) );
		else
			LeaveCriticalSec( &((PMONITOR)monitor)->cs );
	}
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( void, EverybodyScan )( void )
{
   extern PMONITOR Monitors;
	PMONITOR monitor;
	for( monitor = Monitors; monitor; monitor = monitor->next )
	{
		MonitorForgetAll( monitor );
	}
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( PCHANGEHANDLER, AddExtendedFileChangeCallback )( PMONITOR monitor
																		, CTEXTSTR mask
																		, EXTENDEDCHANGEHANDLER HandleChange
																		, PTRSZVAL psv )
{
	if( monitor && HandleChange )
	{
		PCHANGECALLBACK Change = (PCHANGECALLBACK)Allocate( sizeof( CHANGECALLBACK ) );
      EnterCriticalSec( &monitor->cs );
		Change->mask           = StrDup( mask );
      Change->currentchange  = NULL;
		Change->PendingChanges = NULL;
      Change->filelist       = CreateBinaryTreeExtended( BT_OPT_NODUPLICATES
																		 , CompareName
																		 , NULL
                                                       DBG_SRC);
		Change->psv            = psv;
		Change->flags.bExtended = 1;
      Change->flags.bInitial = 1; // created flag set on file monitors is cleared
		Change->HandleChangeEx = HandleChange;
      Change->monitor        = monitor;
		LinkThing( monitor->ChangeHandlers, Change );
      if( l.flags.bLog ) Log1( WIDE("Setting scan time to %d"), timeGetTime() - 1 );
		monitor->DoScanTime = timeGetTime() + monitor->scan_delay;
      LeaveCriticalSec( &monitor->cs );
      return Change;
	}
   return NULL;
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( PCHANGEHANDLER, AddFileChangeCallback )( PMONITOR monitor
                                              , CTEXTSTR mask
															 , CHANGEHANDLER HandleChange
															 , PTRSZVAL psv )
{
	if( monitor && HandleChange )
	{
		PCHANGECALLBACK Change = (PCHANGECALLBACK)Allocate( sizeof( CHANGECALLBACK ) );
      EnterCriticalSec( &monitor->cs );
		Change->mask           = StrDup( mask );
      Change->currentchange  = NULL;
		Change->PendingChanges = NULL;
		Change->filelist       = CreateBinaryTreeExtended( BT_OPT_NODUPLICATES
																		 , CompareName
																		 , NULL
                                                       DBG_SRC
                                                       );
		Change->psv            = psv;
      Change->flags.bExtended = 0;
      Change->flags.bInitial = 1; // created flag set on file monitors is cleared
		Change->HandleChange   = HandleChange;
      Change->monitor        = monitor;
		LinkThing( monitor->ChangeHandlers, Change );
      if( l.flags.bLog ) Log1( WIDE("Setting scan time to %d"), timeGetTime() - 1 );
		monitor->DoScanTime = timeGetTime() + monitor->scan_delay;
      LeaveCriticalSec( &monitor->cs );
      return Change;
	}
   return NULL;
}

//-------------------------------------------------------------------

FILEMONITOR_PROC( void, SetFileLogging )( PMONITOR monitor, int enable )
{
   monitor->flags.bLogFilesFound = 1;
}

FILEMONITOR_PROC( void, SetFMonitorForceScanTime )( PMONITOR monitor, _32 delay )
{
	if( monitor )
      monitor->free_scan_delay = delay;
}

FILEMON_NAMESPACE_END

//-------------------------------------------------------------------
// Branched from bingocore.  Also console barbingo/relay (a little more robust)
// $Log: allfiles.c,v $
// Revision 1.23  2005/03/27 01:52:11  panther
// Remove noisy logging line aobut created file markings
//
// Revision 1.22  2005/03/03 19:51:46  panther
// Okay create events are not doubled with an update event now... opertaions can happen out of order however, a delete and create may happen in reverse order.
//
// Revision 1.21  2005/03/02 21:17:04  panther
// fix for handling deleted files and new pending flag... created gets notified - but there's some reduncancies on update/create
//
// Revision 1.20  2005/02/28 07:07:05  panther
// Attempt to add on create callback to extended method... added logging.
//
// Revision 1.19  2005/02/23 13:01:35  panther
// Fix scrollbar definition.  Also update vc projects
//
// Revision 1.18  2005/01/27 07:25:47  panther
// Linux - well as clean as it can be with libC sucking.
//
// Revision 1.17  2004/12/01 23:15:58  panther
// Extend file monitor to make forced scan timeout settable by the application
//
// Revision 1.16  2004/10/25 10:39:58  d3x0r
// Linux compilation cleaning requirements...
//
// Revision 1.15  2004/04/29 19:52:05  d3x0r
// See prior comment.
//
// Revision 1.16  2004/04/29 19:52:29  jim
// Fix issue double enquing deleted file notifications
//
// Revision 1.15  2004/04/27 18:36:59  jim
// Remove noisy logging
//
// Revision 1.14  2004/01/16 16:57:48  d3x0r
// Added logging, and an extern anable per monitor for file logging
//
// Revision 1.13  2004/01/12 08:43:06  panther
// Fixed CVS comment in source
//
// Revision 1.12  2004/01/12 08:42:16  panther
// (cut prior)
// Removed logging for deleted file, stat change of a file.
//
// Revision 1.11  2003/12/16 23:09:46  panther
// Fix mask test for 'FILE*' matching 'FILE'
//
// Revision 1.10  2003/12/08 10:32:32  dave
//  define O_BINARY and dont' use windunce
//
// Revision 1.9  2003/12/08 03:27:05  panther
// Fix linux/fcntl conflict
//
// Revision 1.8  2003/12/03 16:27:02  panther
// Reorder routines to avoid warnings, include stdhdrs
//
// Revision 1.7  2003/11/10 03:22:14  panther
// Added some logging ... gotta figure out this timer stuff
//
// Revision 1.6  2003/11/09 22:30:09  panther
// Stablity fixes... don't End while scanning for instance
//
// Revision 1.5  2003/11/04 11:40:00  panther
// Mark per file updates - a directory may continually get updates
//
// Revision 1.4  2003/11/04 09:20:49  panther
// Modify call method to watch directory and supply seperate masks per change routine
//
// Revision 1.3  2003/11/04 01:46:43  panther
// Remove some unused members, remove tree function
//
// Revision 1.2  2003/11/04 01:28:51  panther
// Fixed most holes with the windows change monitoring system
//
// Revision 1.1  2003/11/03 23:01:43  panther
// Initial commit of librarized filemonitor
//
//
