
#ifndef MONITOR_HEADER_INCLUDED
#define MONITOR_HEADER_INCLUDED

#ifndef _WIN32
#define MAX_PATH 256 // protocol limit...
#endif

#include <stdhdrs.h>
#include <sack_types.h>
#include <timers.h>
#include <filemon.h>
#include <sys/types.h> // time_t

FILEMON_NAMESPACE

typedef struct filemonitor_tag {
    TEXTCHAR   name[320]; // MAX_PATH is inconsistantly defined
	TEXTSTR  filename; // just the filename in the name
	struct {
		_32   bDirectory : 1;
		_32   bToDelete : 1;
		_32   bScanned : 1; // if set - don't requeue, set when added to the changed queue, and cleared when done...
		_32   bCreated : 1;
		_32   bPending : 1; // set when enqueued in pending list, and cleared after event dispatched.
	} flags;
	 // if GetTickCount() - ScannedAt > monitor->scan_delay queue for change
	_32 ScannedAt; // time this file was scanned
#ifdef __cplusplus_cli
	gcroot<System::DateTime^> lastmodifiedtime;
#else
	time_t lastmodifiedtime;
#endif
    _64    lastknownsize;
    CRITICALSECTION cs;
} FILEMON;

typedef struct mytime_tag {
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} MYTIME, *PMYTIME;

typedef struct filechangecallback_tag
{
   CTEXTSTR mask;
	PFILEMON currentchange;
	PLINKQUEUE PendingChanges;
	PTREEROOT filelist;
	struct {
		_32 bExtended : 1;
		_32 bInitial : 1; // first time files are scanned, they are not created.
	} flags;
	struct monitor_tag *monitor;
	union {
		CHANGEHANDLER HandleChange;
		EXTENDEDCHANGEHANDLER HandleChangeEx;
	};
   PTRSZVAL psv;
   DeclareLink( struct filechangecallback_tag );
} CHANGECALLBACK, *PCHANGECALLBACK;

typedef struct monitor_tag
{
	CRITICALSECTION cs;
	PTHREAD pThread;
	struct {
		_32 bDispatched : 1;
		_32 bEnd : 1;
		_32 bClosing : 1;
		_32 bUserInterruptedChanges : 1;
		_32 bScanning : 1;
		_32 bLogFilesFound : 1;
	} flags;
#ifdef _WIN32
    HANDLE hChange;
#else
    int fdMon;
#endif
    // this includes updates to directories and files
	 TEXTCHAR directory[MAX_PATH];
    _32 DoScanTime;
    _32 timer;
	 _32 scan_delay;
    _32 free_scan_delay;
	 PCHANGECALLBACK ChangeHandlers;
    DeclareLink( struct monitor_tag );
} MONITOR;

#ifndef PRIMARY_FILEMON_SOURCE
extern
#endif
struct local_filemon {
	struct {
		BIT_FIELD bInit : 1;
		BIT_FIELD bLog : 1;
	} flags;
} local_filemon;
#define l local_filemon


void CPROC ScanTimer( PTRSZVAL monitor );

PFILEMON NewFile( PCHANGEHANDLER, CTEXTSTR name );
void CloseFileMonitors( PMONITOR monitor );
void ScanDirectory( PMONITOR monitor );

FILEMON_NAMESPACE_END


#endif
//-------------------------------------------------------------------
// Branched from bingocore.  Also console barbingo/relay (a little more robust)
// $Log: monitor.h,v $
// Revision 1.11  2005/02/23 13:01:35  panther
// Fix scrollbar definition.  Also update vc projects
//
// Revision 1.10  2004/12/01 23:15:58  panther
// Extend file monitor to make forced scan timeout settable by the application
//
// Revision 1.9  2004/01/16 16:57:48  d3x0r
// Added logging, and an extern anable per monitor for file logging
//
// Revision 1.8  2004/01/07 09:46:18  panther
// Cleanup warnings, missing prototypes...
//
// Revision 1.7  2003/11/09 22:30:09  panther
// Stablity fixes... don't End while scanning for instance
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
