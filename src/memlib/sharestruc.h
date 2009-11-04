#if !defined( MEMORY_STRUCT_DEFINED ) || defined( DEFINE_MEMORY_STRUCT )
#define MEMORY_STRUCT_DEFINED
#include "sack_types.h"

#ifdef _DEBUG
#include <sharemem.h>
//  Define this symbol in SHAREMEM.H!
// if you define it here it will not work as expected...
//// defined in sharemem.h #define DEBUG_CRITICAL_SECTIONS
//// defined in sharemem.h #define LOG_DEBUG_CRITICAL_SECTIONS
#endif
#define _SHARED_MEMORY_LIBRARY

#ifdef __cplusplus
namespace sack {
	namespace timers {
#endif
// bit set on dwLocks when someone hit it and it was locked
#define SECTION_LOGGED_WAIT 0x80000000
// If you change this structure please change the public
// reference of this structure, and please, do hand-count
// the bytes to set there... so not include this file
// to get the size.  The size there should be the worst
// case - debug or release mode.
#ifdef NO_PRIVATE_DEF
struct critical_section_tag {
	volatile _32 dwUpdating; // this is set when entering or leaving (updating)...
   volatile _32 dwLocks;
	THREAD_ID dwThreadID; // windows upper 16 is process ID, lower is thread ID
   THREAD_ID dwThreadWaiting; // ID of thread waiting for this..
   //PDATAQUEUE pPriorWaiters;
#ifdef DEBUG_CRITICAL_SECTIONS
	_32 bCollisions ;
	CTEXTSTR pFile;
	_32  nLine;
#endif
};
typedef struct critical_section_tag CRITICALSECTION;
#endif
#ifdef __cplusplus
	};
};
#endif

#ifdef __cplusplus
namespace sack {
	namespace memory {
		using namespace sack::timers;
#endif
// pFile, nLine has been removed from this
// the references for this info are now
// stored at the end of the block
// after the 0x12345678 tag.
PREFIX_PACKED struct chunk_tag
{
	_16 dwOwners;            // if 0 - block is free
	_16 dwPad;   // extra bytes 4/12 typical, sometimes pad untill next.
	// which is < ( CHUNK_SIZE + nMinAllocate )
	// real size is then dwSize - dwPad.
   // this is actually where the end of block tag(s) should begin!
	_32 dwSize;  // limited to allocating 4 billion bytes...
	struct chunk_tag *pPrior;         // save some math backwards...
	struct memory_block_tag * pRoot;  // pointer to master allocation struct (pMEM)
   DeclareLink( struct chunk_tag );
	_8 byData[1]; // _8 is the smallest valid datatype could be _0
} PACKED;
typedef PREFIX_PACKED struct chunk_tag CHUNK, *PCHUNK;


// chunks allocated have no debug information.
#define HEAP_FLAG_NO_DEBUG 0x0001

struct memory_block_tag
{
	_32 dwSize;
   _32 dwHeapID; // unique value 0xbab1f1ea (baby flea);
	// lock between multiple processes/threads
	CRITICALSECTION cs;
	_32 dwFlags;
	PCHUNK pFirstFree;
	CHUNK pRoot[1];
};
typedef struct memory_block_tag MEM;

#ifdef __cplusplus
	};
};
#endif

#endif
// $Log: sharestruc.h,v $
// Revision 1.18  2005/06/22 23:13:58  jim
// Differentiate the normal logging of 'entered, left section' but leave in notable exception case logging when enabling critical section debugging.
//
// Revision 1.17  2005/05/12 20:59:49  jim
// Remove releaseallmemeory from atexit list... (should be added only for file-backed storage) also enhanced critical section debugging.
//
// Revision 1.16  2003/12/14 05:48:31  panther
// Disable critical section debugging
//
// Revision 1.15  2003/12/09 16:15:05  panther
// Option out DEBUG_CRIT if not debug
//
// Revision 1.14  2003/12/09 13:12:08  panther
// Initialize anonymous memory to 0
//
// Revision 1.13  2003/11/09 22:31:08  panther
// Modification to free memory linking - Be ready to rollback
//
// Revision 1.12  2003/10/22 01:58:23  panther
// Fixed critical sections yay! disabled critical section logging
//
// Revision 1.11  2003/10/21 08:59:57  panther
// Rework allocation - -put debug info at tail of block, introduce dwPad - wonderful things.
//
// Revision 1.10  2003/10/21 01:39:37  panther
// Fixed some issues with new perma-wait critical sections...
//
// Revision 1.9  2003/10/20 03:01:21  panther
// Fix getmythreadid - split depending if getpid returns ppid or pid.
// Fix memory allocator to init region correctly...
// fix initial status of found thred to reflect sleeping
// in /proc/#/status
//
// Revision 1.8  2003/10/20 00:04:21  panther
// Extend OpenSpace in SharedMem
// revise msgqueue operations to more resemble sysVipc msgq
//
// Revision 1.7  2003/10/19 03:53:35  panther
// Removed STRONG_DIAGNOSTIC and DEBUG_TRACKING options...
// in the last year and 3/4 I've not even botherd with em so, they're gone.
// Revised chunk, allow per heap disabling of debug info.  Every so often
// it's useful to debug an application level error to leave the debug
// checking in that is already present.  But, for purposes of shared memory
// and static memory, it is impossible to let debug and release variations
// have different format.  Need to retrofit ODS, ODSEx (originally short for
// OutputDebugString which is the WinNT api for debug info.
//
// Revision 1.6  2003/10/18 05:58:06  panther
// Fix new implemented leave critical no wake
//
// Revision 1.5  2003/10/17 00:56:05  panther
// Rework critical sections.
// Moved most of heart of these sections to timers.
// When waiting, sleep forever is used, waking only when
// released... This is preferred rather than continuous
// polling of section with a Relinquish.
// Things to test, event wakeup order uner linxu and windows.
// Even see if the wake ever happens.
// Wake should be able to occur across processes.
// Also begin implmeenting MessageQueue in containers....
// These work out of a common shared memory so multiple
// processes have access to this region.
//
// Revision 1.4  2003/03/25 08:45:55  panther
// Added CVS logging tag
//
