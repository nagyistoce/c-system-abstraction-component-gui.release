
#ifndef SHARED_MEM_DEFINED
#define SHARED_MEM_DEFINED

#include <sack_types.h>

#ifdef BCC16
#ifdef MEM_LIBRARY_SOURCE
#define MEM_PROC(type,name) type STDPROC _export name
#else
#define MEM_PROC(type,name) type STDPROC name
#endif
#else
#    ifdef MEM_LIBRARY_SOURCE
#      define MEM_PROC(type,name) EXPORT_METHOD type CPROC name
#    else
#      define MEM_PROC(type,name) IMPORT_METHOD type CPROC name
#    endif
#endif


#ifdef __cplusplus
namespace sack {
namespace timers {
#endif
   // enables file/line monitoring of sections and a lot of debuglogging
//#define DEBUG_CRITICAL_SECTIONS
   /* this symbol controls the logging in timers.c... (higher level interface to NoWait primatives)*/
//#define LOG_DEBUG_CRITICAL_SECTIONS

//#if !defined( MEMORY_STRUCT_DEFINED )
typedef struct critical_section_tag {
	_32 dwUpdating; // this is set when entering or leaving (updating)...
	_32 dwLocks;
	THREAD_ID dwThreadID; // windows upper 16 is process ID, lower is thread ID
	THREAD_ID dwThreadWaiting; // ID of thread waiting for this..
#ifdef DEBUG_CRITICAL_SECTIONS
	_32 bCollisions ;
	CTEXTSTR pFile;
	_32  nLine;
#endif
} CRITICALSECTION;
//#endif

typedef struct critical_section_tag *PCRITICALSECTION;
MEM_PROC( S_32, EnterCriticalSecNoWaitEx )( PCRITICALSECTION pcs, THREAD_ID *prior DBG_PASS );
#define EnterCriticalSecNoWait( pcs,prior ) EnterCriticalSecNoWaitEx( (pcs),(prior) DBG_SRC )
MEM_PROC( void, InitializeCriticalSec )( PCRITICALSECTION pcs );
MEM_PROC( _32, CriticalSecOwners )( PCRITICALSECTION pcs );

#ifdef __cplusplus
}; // namespace timers
}; // namespace sack
using namespace sack::timers;
#endif

#ifdef __cplusplus
namespace sack {
namespace memory {
//#define __mem_ns__ sack::memory::
#endif
typedef struct memory_block_tag* PMEM;

// what is an abstract name for the memory mapping handle...
// where is a filename for the filebacking of the shared memory
// DigSpace( WIDE("Picture Memory"), WIDE("Picture.mem"), 100000 );

// raw shared file view...
MEM_PROC( POINTER, OpenSpace )( CTEXTSTR pWhat, CTEXTSTR pWhere, _32 *dwSize );

// an option to specify a requested address would be MOST handy...
//MEM_PROC( POINTER, OpenSpaceEx )( TEXTSTR pWhat, TEXTSTR pWhere, _32 address, _32 *dwSize );
MEM_PROC( POINTER, OpenSpaceExx )( CTEXTSTR pWhat, CTEXTSTR pWhere, _32 address, P_32 dwSize, P_32 bCreated );
#define OpenSpaceEx( what,where,address,psize) OpenSpaceExx( what,where,address,psize,NULL )

MEM_PROC( void, CloseSpace )( POINTER pMem );
MEM_PROC( void, CloseSpaceEx )( POINTER pMem, int bFinal );
MEM_PROC( _32, GetSpaceSize )( POINTER pMem );

// even if pMem is just a POINTER returned from OpenSpace
// this will create a valid heap pointer.
// will result TRUE if a valid heap is present
// will result FALSE if heap is not able to init (has content)
MEM_PROC( int, InitHeap)( PMEM pMem, _32 dwSize );

// not sure about this one - perhaps with custom heaps
// we DEFINATLY need to disallow auto-additions
//MEM_PROC( void, AddMemoryHeap )( POINTER pMem, LOGICAL bInit );


MEM_PROC( void, DebugDumpHeapMemEx )( PMEM pHeap, LOGICAL bVerbose );
#define DebugDumpHeapMem(h)     DebugDumpMemEx( (h), TRUE )

MEM_PROC( void, DebugDumpMemEx )( LOGICAL bVerbose );
#define DebugDumpMem()     DebugDumpMemEx( TRUE )
MEM_PROC( void, DebugDumpHeapMemFile )( PMEM pHeap, CTEXTSTR pFilename );
MEM_PROC( void, DebugDumpMemFile )( CTEXTSTR pFilename );

#ifdef GCC
MEM_PROC( POINTER, HeapAllocateEx )( PMEM pHeap, _32 nSize DBG_PASS ) __attribute__((malloc));
MEM_PROC( POINTER, AllocateEx )( _32 nSize DBG_PASS ) __attribute__((malloc));
#else
MEM_PROC( POINTER, HeapAllocateEx )( PMEM pHeap, _32 nSize DBG_PASS );
MEM_PROC( POINTER, AllocateEx )( _32 nSize DBG_PASS );
#endif

#define New(type) ((type*)Allocate(sizeof(type)))
#define Renew(type,p,sz) ((type*)Reallocate(p, sizeof(type)*sz))
// an advantage of C, can define extra space at end of structure
// which is allowed to carry extra data, which is unknown by other code
// room for exploits rock.
#define NewPlus(type,extra) ((type*)Allocate(sizeof(type)+(extra)))
#define NewArray(type,count) ((type*)Allocate(sizeof(type)*(count)))
// will invoke some sort of registered initializer
#define NewObject(type) ((type*)FancyAllocate(sizeof(type),#type DBG_SRC))
//#define New(type) ((type*)Allocate(sizeof(type)))
#ifdef __cplusplus
#define Delete(type,thing) for(type tmp=thing;Release((POINTER)(tmp)),0;)
#else
#define Delete(type,thing) (Release((POINTER)(thing)))
#endif
#define HeapAllocate(heap, n) HeapAllocateEx( (heap), (_32)(n) DBG_SRC )
#define Allocate( n ) HeapAllocateEx( (PMEM)0, (n) DBG_SRC )
//MEM_PROC( POINTER, AllocateEx )( _32 nSize DBG_PASS );
//#define Allocate(n) AllocateEx(n DBG_SRC )
MEM_PROC( POINTER, GetFirstUsedBlock )( PMEM pHeap );

MEM_PROC( POINTER, ReleaseEx )( POINTER pData DBG_PASS ) ;
#define Release(p) ReleaseEx( (p) DBG_SRC )

MEM_PROC( POINTER, HoldEx )( POINTER pData DBG_PASS  );
#define Hold(p) HoldEx(p DBG_SRC )

MEM_PROC( POINTER, HeapReallocateEx )( PMEM pHeap, POINTER source, _32 size DBG_PASS );
#define HeapReallocate(heap,p,sz) HeapReallocateEx( (heap),(p),(sz) DBG_SRC )
MEM_PROC( POINTER, ReallocateEx )( POINTER source, _32 size DBG_PASS );
#define Reallocate(p,sz) ReallocateEx( (p),(sz) DBG_SRC )

MEM_PROC( POINTER, HeapPreallocateEx )( PMEM pHeap, POINTER source, _32 size DBG_PASS );
#define HeapPreallocate(heap,p,sz) HeapPreallocateEx( (heap),(p),(sz) DBG_SRC )
MEM_PROC( POINTER, PreallocateEx )( POINTER source, _32 size DBG_PASS );
#define Preallocate(p,sz) PreallocateEx( (p),(sz) DBG_SRC )

MEM_PROC( POINTER, HeapMoveEx )( PMEM pNewHeap, POINTER source DBG_PASS );
#define HeapMove(h,s) HeapMoveEx( (h), (s) DBG_SRC )

MEM_PROC( _32, SizeOfMemBlock )( CPOINTER pData );

MEM_PROC( LOGICAL, Defragment )( POINTER *ppMemory );

MEM_PROC( void, GetHeapMemStatsEx )( PMEM pHeap, _32 *pFree, _32 *pUsed, _32 *pChunks, _32 *pFreeChunks DBG_PASS );
#define GetHeapMemStats(h,f,u,c,fc) GetHeapMemStatsEx( h,f,u,c,fc DBG_SRC )
//MEM_PROC( void, GetHeapMemStats )( PMEM pHeap, _32 *pFree, _32 *pUsed, _32 *pChunks, _32 *pFreeChunks );
MEM_PROC( void, GetMemStats )( _32 *pFree, _32 *pUsed, _32 *pChunks, _32 *pFreeChunks );

MEM_PROC( int, SetAllocateLogging )( LOGICAL bTrueFalse );
/* disables storing file/line, also disables auto GetMemStats checking*/
MEM_PROC( int, SetAllocateDebug )( LOGICAL bDisable );
/* disables auto GemMemStats on every allocate/release/Hold */
MEM_PROC( int, SetManualAllocateCheck )( LOGICAL bDisable );
// returns the prior state of logging...
MEM_PROC( int, SetCriticalLogging )( LOGICAL bTrueFalse );
MEM_PROC( void, SetMinAllocate )( int nSize );
MEM_PROC( void, SetHeapUnit )( int dwSize );
MEM_PROC( void, DisableHeapDebug)( PMEM pHeap );


#ifdef __cplusplus
MEM_PROC( PTRSZVAL, LockedExchange64 )( PVPTRSZVAL p, PTRSZVAL val );
#endif
MEM_PROC( _64, LockedExchange64 )( PV_64 p, _64 val );
MEM_PROC( _32, LockedIncrement )( P_32 p );
MEM_PROC( _32, LockedDecrement )( P_32 p );

#ifdef __cplusplus
extern "C" {
#endif

MEM_PROC( _32, LockedExchange )( PV_32 p, _32 val );
MEM_PROC( void, MemSet )( POINTER p, _32 n, _32 sz );
#define _memset_ MemSet
MEM_PROC( void, MemCpy )( POINTER pTo, CPOINTER pFrom, _32 sz );
#define _memcpy_ MemCpy
MEM_PROC( int, MemCmp )( CPOINTER pOne, CPOINTER pTwo, _32 sz );

#ifdef __cplusplus
}; // extern "C"
#endif

MEM_PROC( int, StrCmp )( CTEXTSTR pOne, CTEXTSTR pTwo );
MEM_PROC( int, StrCaseCmp )( CTEXTSTR s1, CTEXTSTR s2 );
MEM_PROC( int, StrCaseCmpEx )( CTEXTSTR s1, CTEXTSTR s2, INDEX maxlen );
MEM_PROC( CTEXTSTR, StrChr )( CTEXTSTR s1, TEXTCHAR c );
MEM_PROC( TEXTSTR, StrCpyEx )( TEXTSTR s1, CTEXTSTR s2, int n );
MEM_PROC( TEXTSTR, StrCpy )( TEXTSTR s1, CTEXTSTR s2 );
MEM_PROC( size_t, StrLen )( CTEXTSTR s );
MEM_PROC( size_t, CStrLen )( char *s );


MEM_PROC( CTEXTSTR, StrRChr )( CTEXTSTR s1, TEXTCHAR c );

#ifdef __cplusplus
MEM_PROC( TEXTSTR, StrChr )( TEXTSTR s1, TEXTCHAR c );
MEM_PROC( TEXTSTR, StrRChr )( TEXTSTR s1, TEXTCHAR c );
MEM_PROC( int, StrCmp )( const char * s1, CTEXTSTR s2 );
//MEM_PROC( int, StrCmp )( char * s1, CTEXTSTR s2 );
#endif

MEM_PROC( int, StrCmpEx )( CTEXTSTR s1, CTEXTSTR s2, INDEX maxlen );
MEM_PROC( CTEXTSTR, StrStr )( CTEXTSTR s1, CTEXTSTR s2 );
#ifdef __cplusplus
MEM_PROC( TEXTSTR, StrStr )( TEXTSTR s1, CTEXTSTR s2 );
#endif
MEM_PROC( CTEXTSTR, StrCaseStr )( CTEXTSTR s1, CTEXTSTR s2 );

#define memnop(mem,sz,comment)
MEM_PROC( POINTER, MemDupEx )( CPOINTER thing DBG_PASS );
#define MemDup(thing) MemDupEx(thing DBG_SRC )

MEM_PROC( TEXTSTR, StrDupEx )( CTEXTSTR original DBG_PASS );
MEM_PROC( char * , CStrDupEx )( CTEXTSTR original DBG_PASS );
MEM_PROC( TEXTSTR, DupCStrEx )( const char * original DBG_PASS );
#define StrDup(o) StrDupEx( (o) DBG_SRC )
#define CStrDup(o) CStrDupEx( (o) DBG_SRC )
#define DupCStr(o) DupCStrEx( (o) DBG_SRC )

/*
// patch for spider which attempted to use ShareMem but FAILED
#ifndef _SHARED_MEMORY_LIBRARY
#define Allocate(n) malloc(n)
#undef Release
#define Release(n)  free(n)
#endif
*/

//------------------------------------------------------------------------

#ifndef TRANSPORT_STRUCTURE_DEFINED
typedef PTRSZVAL PTRANSPORT_QUEUE;
struct transport_queue_tag { _8 private_data_here; };

#endif

#if 0
MEM_PROC( struct transport_queue_tag *, CreateQueue )( int size );
MEM_PROC( int, EnqueMessage )( struct transport_queue_tag *queue, POINTER msg, int size );
MEM_PROC( int, DequeMessage )( struct transport_queue_tag *queue, POINTER msg, int *size );
MEM_PROC( int, PequeMessage )( struct transport_queue_tag *queue, POINTER *msg, int *size );
#endif


//------------------------------------------------------------------------

#ifdef __cplusplus

#ifdef __cplusplus
// this is what they made 'using namespace' for
//#define AllocateEx __mem_ns__ AllocateEx
//#define HeapAllocateEx __mem_ns__ HeapAllocateEx
//#define ReleaseEx __mem_ns__ ReleaseEx
//#define MemSet __mem_ns__ MemSet
//#define MemCpy __mem_ns__ MemCpy
#endif

}; // namespace memory
}; // namespace sack
using namespace sack::memory;

#include <stddef.h>

#ifdef _DEBUG
/*
inline void operator delete( void * p )
{ Release( p ); }
#ifdef DELETE_HANDLES_OPTIONAL_ARGS
inline void operator delete (void * p DBG_PASS )
{ ReleaseEx( p DBG_RELAY ); }
#define delete delete( DBG_VOIDSRC )
#endif
//#define deleteEx(file,line) delete(file,line)
#ifdef USE_SACK_ALLOCER
inline void * operator new( size_t size DBG_PASS )
{ return AllocateEx( (_32)size DBG_RELAY ); }
static void * operator new[]( size_t size DBG_PASS )
{ return AllocateEx( (_32)size DBG_RELAY ); }
#define new new( DBG_VOIDSRC )
#define newEx(file,line) new(file,line)
#endif
*/
// common names - sometimes in conflict when declaring 
// other functions... AND - release is a common 
// component of iComObject 
//#undef Allocate
//#undef Release

// Hmm wonder where this conflicted....
//#undef LineDuplicate

#else
#ifdef USE_SACK_ALLOCER
inline void * operator new(size_t size)
{ return AllocateEx( size ); }
inline void operator delete (void * p)
{ ReleaseEx( p ); }
#endif
#endif

#endif


#endif
// $Log: sharemem.h,v $
// Revision 1.38  2005/05/12 21:00:22  jim
// Expand critical section size ot excess of worst case...
//
// Revision 1.37  2004/12/19 15:44:57  panther
// Extended set methods to interact with raw index numbers
//
// Revision 1.36  2004/12/05 15:32:06  panther
// Some minor cleanups fixed a couple memory leaks
//
// Revision 1.35  2004/06/03 10:55:47  d3x0r
// Add a CPROC strcmp
//
// Revision 1.34  2004/06/01 07:07:39  d3x0r
// Fix custom drawn button no-border option
//
// Revision 1.33  2003/12/08 03:49:09  panther
// Make some more types/defs for eltanin building
//
// Revision 1.32  2003/10/22 01:58:23  panther
// Fixed critical sections yay! disabled critical section logging
//
// Revision 1.31  2003/10/21 00:21:36  panther
// Export CloseSpaceEx (sharemem.h).  Unwind circular dependancy for idle.
//
// Revision 1.30  2003/10/20 03:01:21  panther
// Fix getmythreadid - split depending if getpid returns ppid or pid.
// Fix memory allocator to init region correctly...
// fix initial status of found thred to reflect sleeping
// in /proc/#/status
//
// Revision 1.29  2003/10/20 00:04:21  panther
// Extend OpenSpace in SharedMem
// revise msgqueue operations to more resemble sysVipc msgq
//
// Revision 1.28  2003/10/18 04:45:32  panther
// Preallocate was misspelled...
//
// Revision 1.27  2003/10/17 00:56:04  panther
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
// Revision 1.26  2003/10/16 00:12:00  panther
// Fix configscript arg building.  Added Preallocate which like Reallocate resizes memmory, but puts the old memory at the end of the allcoated space.
//
// Revision 1.25  2003/09/24 02:26:02  panther
// Fix C++ methods, extend and correct.
//
// Revision 1.24  2003/07/24 16:56:41  panther
// Updates to expliclity define C procedure model for callbacks and assembly modules - incomplete
//
// Revision 1.23  2003/05/18 19:30:56  panther
// hmm use pretteir names for StrDup
//
// Revision 1.22  2003/05/13 09:12:37  panther
// Update types to be more protected for memlib
//
// Revision 1.21  2003/04/27 01:25:45  panther
// Don't disable allocate, release in header
//
// Revision 1.20  2003/04/24 00:03:49  panther
// Added ColorAverage to image... Fixed a couple macros
//
// Revision 1.19  2003/04/21 19:59:43  panther
// Option to return filename only
//
// Revision 1.18  2003/03/25 08:38:11  panther
// Add logging
//
