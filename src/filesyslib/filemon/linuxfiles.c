//#include <windows.h>
#include <stdhdrs.h>
#ifdef __LINUX__
// fcntl, open
//#include <unistd.h>
//#include <fcntl.h>
#endif
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef __cplusplus
#include <unistd.h>
#include <fcntl.h>
#else
#include <linux/fcntl.h>
#endif
#include <signal.h>
#include <dirent.h>


#include <string.h>
#include <stdio.h>

#include <network.h>
#include <timers.h>
#include <filesys.h>

#include <sack_types.h>
#include <sharemem.h>
#define DO_LOGGING
#include <logging.h>


#define INVALID_HANDLE_VALUE -1
#define SCAN_DELAY 1500

#include "monitor.h"

FILEMON_NAMESPACE

//-------------------------------------------------------------------------

void ScanDirectory( PMONITOR monitor )
{
    DIR *dir;
    dir = opendir( monitor->directory );
    if( dir )
    {
        struct dirent *dirent;
        while( ( dirent = readdir( dir ) ) )
        {
				PCHANGECALLBACK Change;
				if( monitor->flags.bLogFilesFound )
					Log1( WIDE("Found file %s"), dirent->d_name );
				for( Change = monitor->ChangeHandlers; Change; Change = Change->next )
				{
					if( !Change->mask ||
                   CompareMask( Change->mask
										, dirent->d_name
										, FALSE ) )
						if( monitor->flags.bLogFilesFound )
                     Log( WIDE("And the mask matched.") );
                  AddMonitoredFile( Change, dirent->d_name );
				}
        }
        closedir( dir );
    }
    return;
}

//-------------------------------------------------------------------------

static int Monitoring;

// perhaps these two shall be grouped?
// then I can watch multiple directories and maintain
// their names...
PMONITOR Monitors;

//-------------------------------------------------------------------------

PMONITOR CreateMonitor( int fdMon, char *directory )
{
    PMONITOR monitor = (PMONITOR)Allocate( sizeof( MONITOR ) );
    MemSet( monitor, 0, sizeof( MONITOR ) );
    monitor->fdMon = fdMon;
	 strcpy( monitor->directory, directory );
 //   strcpy( monitor->mask, mask );
    monitor->next = Monitors;
    if( Monitors )
        Monitors->me = &monitor->next;
    monitor->me = &Monitors;
    Monitors = monitor;
    return monitor;
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( void, EndMonitor )( PMONITOR monitor )
{
	if( monitor->flags.bDispatched )
	{
		monitor->flags.bEnd = 1;
      return;
	}
	if( monitor->flags.bClosing )
	{
		//Log( WIDE("Monitor already closing...") );
      return;
	}
   EnterCriticalSec( &monitor->cs );
	if( monitor )
	{
		if( monitor->timer )
			RemoveTimer( monitor->timer );
		// clear signals on this handle?
	// wouldn't just closing it be enough!?!?!
	// if I include <fcntl.h> then <linux/fcntl.h> is broken
	// or vice-versa.  Please suffer with no definition on these lines.

		fcntl( monitor->fdMon, F_SETSIG, 0 );
		fcntl( monitor->fdMon, F_NOTIFY, 0 );
		Log1( WIDE("Closing monitor handle: %d"), monitor->fdMon );
		close( monitor->fdMon );
      UnlinkThing( monitor );
		Release( monitor );
		if( !Monitors )
		{
			signal( SIGRTMIN, SIG_IGN );
		}
	}
}

//-------------------------------------------------------------------------

static void handler( int sig, siginfo_t *si, void *data )
{
    if( si )
    {
        PMONITOR cur = Monitors;
        for( cur = Monitors; cur; cur = cur->next )
        {
            if( si->si_fd == cur->fdMon )
            {
                Log1( WIDE("Setting to scan due to event.. %s"), cur->directory );
					 if( !cur->DoScanTime )
						 cur->DoScanTime = GetTickCount() - 1;
                break;
            }
        }
        if( !cur )
        {
           Log1( WIDE("Signal on handle which did not exist?! %d Invoking failure scanall!"), si->si_fd );
           close( si->si_fd );
	        for( cur = Monitors; cur; cur = cur->next )
   	     {
				  if( !cur->DoScanTime )
					  cur->DoScanTime = GetTickCount() - 1;
           }
        }
    }
}

//-------------------------------------------------------------------------

FILEMONITOR_PROC( PMONITOR, MonitorFiles )( CTEXTSTR dirname, int scan_delay )
{
	if( !Monitors )
	{
		struct sigaction act;
		act.sa_sigaction = handler;
		sigemptyset( &act.sa_mask );
		act.sa_flags = SA_SIGINFO;
		if( sigaction( SIGRTMIN, &act, NULL ) != 0 )
		{
			Log( WIDE("Failed to set signal handler") );
			return NULL;
		}
	}

	{
		int fdMon;
		Log2( WIDE("Attempting to monitor:(%d) %s"), getpid(), dirname );
	// if I include <fcntl.h> then <linux/fcntl.h> is broken
	// or vice-versa.  Please suffer with no definition on these lines.
		fdMon = open( dirname, O_RDONLY );
		if( fdMon >= 0 )
		{
			PMONITOR monitor;
			fcntl( fdMon, F_SETSIG, SIGRTMIN );
			fcntl( fdMon, F_NOTIFY, DN_CREATE|DN_DELETE|DN_RENAME|DN_MODIFY|DN_MULTISHOT );
			monitor = CreateMonitor( fdMon, (char*)dirname );
			Monitoring = 1;
			//monitor->Client = Client;
			monitor->DoScanTime = GetTickCount() - 1; // do first scan NOW
			monitor->timer = AddTimerEx( 0, SCAN_DELAY/2, (void(*)(PTRSZVAL))ScanTimer, (PTRSZVAL)monitor );
			Log2( WIDE("Created timer: %") _32f WIDE(" Monitor handle: %d"), monitor->timer, monitor->fdMon );
			return monitor;
		}
		else
			Log( WIDE("Failed to open directory to monitor") );
	}
	return NULL;
}

FILEMON_NAMESPACE_END

//-------------------------------------------------------------------
// Branched from bingocore.  Also console barbingo/relay (a little more robust)
// $Log: linuxfiles.c,v $
// Revision 1.12  2005/01/27 07:25:47  panther
// Linux - well as clean as it can be with libC sucking.
//
// Revision 1.11  2004/01/20 04:28:56  d3x0r
// Fix stupid copy from windows mistake.
//
// Revision 1.10  2004/01/16 16:57:48  d3x0r
// Added logging, and an extern anable per monitor for file logging
//
// Revision 1.9  2003/12/03 16:27:09  panther
// Fixup for linux build
//
// Revision 1.8  2003/11/09 22:30:09  panther
// Stablity fixes... don't End while scanning for instance
//
// Revision 1.7  2003/11/04 11:41:27  panther
// On directory event set scan now, each file will be updated itself.
//
// Revision 1.6  2003/11/04 09:20:49  panther
// Modify call method to watch directory and supply seperate masks per change routine
//
// Revision 1.5  2003/11/04 02:08:17  panther
// Reduced messages, investigated close.  If EndMonitor is called, the thread will be awoken and killed
//
// Revision 1.4  2003/11/04 01:46:43  panther
// Remove some unused members, remove tree function
//
// Revision 1.3  2003/11/04 01:32:21  panther
// Fixed some compat issued with linux scanner
//
// Revision 1.2  2003/11/04 01:28:51  panther
// Fixed most holes with the windows change monitoring system
//
// Revision 1.1  2003/11/03 23:01:43  panther
// Initial commit of librarized filemonitor
//
//
