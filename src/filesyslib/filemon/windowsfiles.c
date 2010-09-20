#include <stdhdrs.h>
#include <time.h> // gmtime
// --- unix like files --
#include <io.h>
#include <fcntl.h>
// ----------------------
#include <string.h>
#include <stdio.h>

#include <network.h>

#include <sack_types.h>
#include <logging.h>
#include <sharemem.h>
#include <timers.h>
#include <filesys.h>
#include "monitor.h"

FILEMON_NAMESPACE

//-------------------------------------------------------------------------

void ScanDirectory( PMONITOR monitor )
{
	HANDLE hFile;
	TEXTCHAR match[256];
	WIN32_FIND_DATA FindFileData;
	snprintf( match, sizeof(match), WIDE("%s/*.*"), monitor->directory );
	hFile = FindFirstFile( match, &FindFileData );
	if( l.flags.bLog ) lprintf( "Scan directory: %s", match );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		do
		{
			if(!( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ))
			{
				PCHANGECALLBACK Change;
				if( monitor->flags.bLogFilesFound )
					if( l.flags.bLog ) Log1( WIDE("Found file %s"), FindFileData.cFileName );
 				for( Change = monitor->ChangeHandlers; Change; Change = Change->next )
				{
					if( !Change->mask ||
                   CompareMask( Change->mask
										, FindFileData.cFileName
										, FALSE ) )
					{
						if( monitor->flags.bLogFilesFound )
							if( l.flags.bLog ) Log( WIDE("And the mask matched.") );
						AddMonitoredFile( Change, FindFileData.cFileName );
					}
				}
			}
		} while( FindNextFile( hFile, &FindFileData ) );
		FindClose( hFile );
	}
	else
		if( l.flags.bLog ) Log( WIDE("FindFirstFile returned an invalid find handle") );
	if( l.flags.bLog ) lprintf( "Scanned directory: %s", match );
}


//-------------------------------------------------------------------------

PMONITOR Monitors;

//-------------------------------------------------------------------------

FILEMONITOR_PROC( void, EndMonitor )( PMONITOR monitor )
{
	if( monitor->flags.bDispatched || monitor->flags.bScanning )
	{
		monitor->flags.bEnd = 1;
      return;
	}
	if( monitor->flags.bClosing )
	{
		if( l.flags.bLog ) Log( WIDE("Monitor already closing...") );
      return;
	}
	EnterCriticalSec( &monitor->cs );
   monitor->flags.bClosing = 1;
	//Log1( WIDE("Closing the monitor on %s and killing thread...")
	//    , monitor->directory );
	if( monitor->hChange != INVALID_HANDLE_VALUE )
		FindCloseChangeNotification( monitor->hChange );
	if( monitor->pThread )
	{
		EndThread( monitor->pThread );
	}
	//else
	//	Log( WIDE("Thread already left...") );
   CloseFileMonitors( monitor );
	RemoveTimer( monitor->timer );
	UnlinkThing( monitor );
   LeaveCriticalSec( &monitor->cs );
   Release( monitor );
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC MonitorFileThread( PTHREAD pThread )
{
   PMONITOR monitor = (PMONITOR)GetThreadParam( pThread );
	DWORD dwResult;
	monitor->DoScanTime = timeGetTime() - 1; // scan next tick
	monitor->hChange = FindFirstChangeNotification( monitor->directory
																 , FALSE
																 , FILE_NOTIFY_CHANGE_FILE_NAME
																  | FILE_NOTIFY_CHANGE_SIZE
																  | FILE_NOTIFY_CHANGE_LAST_WRITE
																 );
   if( l.flags.bLog ) lprintf( WIDE("Opened handle %d on %s"), monitor->hChange, monitor->directory );
    if( monitor->hChange == INVALID_HANDLE_VALUE )
	 {
		 TEXTCHAR msg[128];
		 snprintf( msg, sizeof(msg), WIDE("Cannot monitor directory: %s"), monitor->directory );
		 // probably log something like we didn't have a good directory?
		 // this needs to be a visible failure - likely a
		 // configuration error...
       // and being in windows this is allowed?
		 MessageBox( NULL, msg, WIDE("Monitor Failed"), MB_OK );
    }
    else do
	 {
        //Log( WIDE("Waiting for a change...") );
        dwResult = WaitForSingleObject( monitor->hChange, monitor->free_scan_delay /*900000*/ /*3600000*/ );
        if( dwResult == WAIT_ABANDONED )
		  {
           if( l.flags.bLog ) lprintf( WIDE("Wait abandoned - have to leave...") );
			  //MessageBox( NULL, WIDE("Wait returned bad error"), WIDE("Monitor Failed"), MB_OK );
           break;
        }
        if( dwResult == WAIT_OBJECT_0 || dwResult == WAIT_TIMEOUT )
        {
            if( !FindNextChangeNotification( monitor->hChange ) )
				{
               if( l.flags.bLog ) lprintf( WIDE("Find next change failed...%d %s"), GetLastError(), monitor->directory );
                // bad things happened
					//MessageBox( NULL, WIDE("Find change notification failed"), WIDE("Monitor Failed"), MB_OK );
               monitor->hChange = INVALID_HANDLE_VALUE;
               break;
				}
            if( !monitor->DoScanTime )
					monitor->DoScanTime = timeGetTime() - 1;
        }
        //else if( dwResult == WAIT_TIMEOUT )
		  //{
			  // if multiple objects are going to be consolidated
			  // to reduce the threadcount, then an extra event
			  // object needs to be added so that the wait may
			  // be woken up and the new handles added to
           // what's being watched...
        //}
        // and we'll never have dwResult == WAIT_TIMEOUT
	 } while( 1 );
    if( l.flags.bLog ) Log( WIDE("Leaving the thread...") );
	 EndMonitor( monitor );
    monitor->pThread = NULL;
    return 0; // something....
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( PMONITOR, MonitorFiles )( CTEXTSTR directory, int scan_delay )
{
    PMONITOR monitor;
	 if( !scan_delay )
       scan_delay = 1000;
    if( l.flags.bLog ) Log1( WIDE("Going to start monitoring changes to: %s"), directory );
    monitor = (PMONITOR)Allocate( sizeof( MONITOR ) );
    MemSet( monitor, 0, sizeof( MONITOR ) );
	 StrCpyEx( monitor->directory, directory, sizeof( monitor->directory )/sizeof(TEXTCHAR) );
	 //strcpy( monitor->mask, mask );
	 monitor->scan_delay = scan_delay;
    monitor->free_scan_delay = 5000;
    LinkThing( Monitors, monitor );
    monitor->pThread = ThreadTo( MonitorFileThread,
												(PTRSZVAL)monitor );
    if( !monitor->pThread )
    {
        MessageBox( NULL, WIDE("Error spawning file monitoring thread!"), WIDE("Fatal Error"), MB_OK );
        EndMonitor( monitor );
        return 0;
	 }
    if( l.flags.bLog ) Log1( WIDE("Adding timer %d"), scan_delay / 3 );
	 monitor->timer = AddTimer( scan_delay/3, ScanTimer, (PTRSZVAL)monitor );
    return monitor;
}

FILEMON_NAMESPACE_END

//-------------------------------------------------------------------
// Branched from bingocore.  Also console barbingo/relay (a little more robust)
// $Log: windowsfiles.c,v $
// Revision 1.13  2004/12/01 23:15:58  panther
// Extend file monitor to make forced scan timeout settable by the application
//
// Revision 1.12  2004/02/04 16:49:36  d3x0r
// Shorted inactivity timer - 15 minutes from 1 hour
//
// Revision 1.11  2004/01/26 23:47:24  d3x0r
// Misc edits.  Fixed filemon.  Export net startup, added def to edit frame
//
// Revision 1.10  2004/01/16 16:57:48  d3x0r
// Added logging, and an extern anable per monitor for file logging
//
// Revision 1.9  2004/01/07 09:46:18  panther
// Cleanup warnings, missing prototypes...
//
// Revision 1.8  2003/11/09 22:30:09  panther
// Stablity fixes... don't End while scanning for instance
//
// Revision 1.7  2003/11/04 11:41:27  panther
// On directory event set scan now, each file will be updated itself.
//
// Revision 1.6  2003/11/04 11:40:00  panther
// Mark per file updates - a directory may continually get updates
//
// Revision 1.5  2003/11/04 09:20:49  panther
// Modify call method to watch directory and supply seperate masks per change routine
//
// Revision 1.4  2003/11/04 02:08:17  panther
// Reduced messages, investigated close.  If EndMonitor is called, the thread will be awoken and killed
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
