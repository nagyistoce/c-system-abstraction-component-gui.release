/*
 *  Crafted by James Buckeyne
 *
 *   (c) Freedom Collective 2000-2006++
 *
 *   created to provide standard memory allocation features.
 *   Release( Allocate(size) )
 *   Hold( pointer ); // must release a second time.
 *   if DEBUG, memory bounds checking enabled and enableable.
 *   if RELEASE standard memory includes no excessive operations
 *
 *  standardized to never use int. (was a clean port, however,
 *  inaccurate, knowing the conversions on calculations of pointers
 *  are handled by cast to int! )
 *
 * see also - include/sharemem.h
 *
 */

//   DEBUG_SYMBOLS
// had some problems with OpenSpace opening a shared region under win98
// Apparently if a create happens with a size of 0, the name of the region
// becomes unusable, until a reboot happens.
//#define DEBUG_OPEN_SPACE

// we're definatly not unicode compliant, sorry
//#undef UNICODE

// this variable controls whether allocate/release is logged.


#include <stddef.h>
#include <stdio.h>

#ifdef __LINUX__
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif


#define MEMORY_STRUCT_DEFINED
#define DEFINE_MEMORY_STRUCT
#include <stdhdrs.h>
#include <filedotnet.h>
#include <sack_types.h>
#include <logging.h>
#include <deadstart.h>
#include <sharemem.h>
#include <procreg.h>
#include "sharestruc.h"
#include <sqlgetoption.h>
#include <ctype.h>
#ifdef __cplusplus
namespace sack {
	namespace memory {
	
#endif


#undef _DEBUG

#if defined( __LINUX__ ) && !defined( __CYGWIN__ )
#define WINAPI
typedef _32 HINSTANCE;
#endif

// last entry in space tracking array will ALWAYS be
// another space tracking array (if used)
// (32 bytes)
typedef struct space_tracking_structure {
	PMEM pMem;
#ifdef _WIN32
	HANDLE  hFile;
	HANDLE  hMem;
#else
	struct {
		_32 bTemporary : 1;
	} flags;
	int hFile;
#endif
   	PTRSZVAL dwSmallSize;
   	DeclareLink( struct space_tracking_structure );
} SPACE, *PSPACE;

typedef struct space_pool_structure {
   DeclareLink( struct space_pool_structure );
	SPACE spaces[ (4096 - sizeof( struct space_pool_structure *)
							- sizeof( struct space_pool_structure **) )
											  / sizeof( SPACE )];
} SPACEPOOL, *PSPACEPOOL;

#define MAX_PER_BLOCK (4096 - sizeof( struct space_pool_structure *) \
	- sizeof( struct space_pool_structure **) )  \
	/ sizeof( SPACE )


#ifdef _WIN32
					//(0x10000 * 0x1000) //256 megs?
#define FILE_GRAN g.si.dwAllocationGranularity
#else
#define FILE_GRAN g.pagesize
#endif

struct global_memory_tag {
	_32 dwSystemCapacity; // basic OS block grabbed for allocation
//#ifdef _DEBUG
// may define one or the other of these but NOT both
	int bDisableDebug;
	int bDisableAutoCheck;
	int bLogCritical;
//#endif
	int nMinAllocateSize;
	int pagesize;
	int bLogAllocate;
	LOGICAL bInit;
	PSPACEPOOL pSpacePool;
#ifdef _WIN32
	SYSTEM_INFO si;
#endif
	int InAdding; // don't add our tracking to ourselves...
	_32 bMemInstanced; // set if anybody starts to DIG.
	PMEM pMemInstance;
};

#ifdef __STATIC__
static struct global_memory_tag global_memory_data = { 0x10000 * 0x08, 0, 0, 0, 0, 0, 0/*logging*/ };
#  define g (global_memory_data)
#  if 0
static struct global_memory_tag *global_memory_data;
#    define g (*global_memory_data)
static void CPROC InitGlobalData( POINTER p, _32 size )
{
	struct global_memory_tag *global = (struct global_memory_tag *)p;
	global->bLogAllocate = 0;
#ifdef UNDER_CE
	global->bDisableAutoCheck = 1;
#endif
#    ifndef _DEBUG
	global->bDisableDebug = 1;
	global->bDisableAutoCheck = 1;
#ifdef DEBUG_CRITICAL_SECTIONS
   global->bLogCritical = 1;
#endif
#    endif
	global->dwSystemCapacity = 0x10000 * 0x08;  // 512K ! 1 meg... or 16 :(
}
PRIORITY_PRELOAD( InitGlobal, GLOBAL_INIT_PRELOAD_PRIORITY-1 )
{
	SimpleRegisterAndCreateGlobalWithInit( global_memory_data, InitGlobalData );
}
#  endif

#else
#  ifdef _DEBUG
struct global_memory_tag global_memory_data = { 0x10000 * 0x08, 0, 1/*auto check*/
#    ifdef DEBUG_CRITICAL_SECTIONS
															 , 1
#    else
															 , 0
#    endif

															 , 0, 0, 0/*logging*/ };
// this one has memory logging enabled by default...
//struct global_memory_tag global_memory_data = { 0x10000 * 0x08, 0, 1, 0, 0, 0, 1 };
#  else
struct global_memory_tag global_memory_data = { 0x10000 * 0x08, 0, 1/*auto check*/, 0, 0, 0, 0/*logging*/ };
// this one has memory logging enabled by default...
//struct global_memory_tag global_memory_data = { 0x10000 * 0x08, 0, 1, 0, 0, 0, 1 };
#  endif
#define g global_memory_data
#endif

#define ODSEx(s,pFile,nLine) SystemLogFL( s DBG_RELAY )
//#define ODSEx(s,pFile,nLine) SystemLog( s )
#define ODS(s)  SystemLog(s)

#define MAGIC_SIZE sizeof( void* )

#ifdef __LINUX64__
#define BLOCK_TAG(pc)  (*(_64*)((pc)->byData + (pc)->dwSize - (pc)->dwPad ))
// so when we look at memory this stamp is 0123456789ABCDEF
#define BLOCK_TAG_ID 0xefcdab8967452301LL
#else
#define BLOCK_TAG(pc)  (*(_32*)((pc)->byData + (pc)->dwSize - (pc)->dwPad ))
// so when we look at memory this stamp is 12345678
#define BLOCK_TAG_ID 0x78563412L
#endif
// file/line info are at the very end of the physical block...
// block_tag is at the start of the padding...
#define BLOCK_FILE(pc) (*(CTEXTSTR*)((pc)->byData + (pc)->dwSize - MAGIC_SIZE*2))
#define BLOCK_LINE(pc) (*(int*)((pc)->byData + (pc)->dwSize - MAGIC_SIZE))

#ifndef _WIN32
#include <errno.h>
#endif

PRIORITY_PRELOAD( InitGlobal, DEFAULT_PRELOAD_PRIORITY-3 )
{
#ifndef __NO_OPTIONS__

#ifdef DEBUG_CRITICAL_SECTIONS
	g.bLogCritical = SACK_GetProfileIntEx( GetProgramName(), "SACK/Memory Library/Log critical sections", 0, TRUE );
#endif
	g.bLogAllocate = SACK_GetProfileIntEx( GetProgramName(), "SACK/Memory Library/Enable Logging", 0, TRUE );
	g.bDisableDebug = SACK_GetProfileIntEx( GetProgramName(), "SACK/Memory Library/Disable Debug"
#ifdef _DEBUG
													 , 0
#else
													 , 1
#endif
													 , TRUE
													);
#endif
}

#ifdef __LINUX__
#ifdef __ARM__
#define XCHG(p,val)  LockedExchange( p, val )
#else
inline _32 DoXchg( PV_32 p, _32 val ){  __asm__( WIDE("lock xchg (%2),%0"):"=a"(val):"0"(val),"c"(p) ); return val; }
#define XCHG(p,val)  DoXchg(p,val)
#endif
#else
#define XCHG(p,val)  LockedExchange( p, val )
#endif
//-------------------------------------------------------------------------
#if !defined( HAS_ASSEMBLY ) || defined( __CYGWIN__ )
 _32  LockedExchange ( PV_32 p, _32 val )
{
	// Windows only available - for linux platforms please consult
	// the assembly version should be consulted
#if defined( _WIN32 ) || defined( __WINDOWS__ ) || defined( WIN32 )
#  if !defined(_MSC_VER)
	return InterlockedExchange( (volatile LONG *)p, val );
#  else
	// windows wants this as a LONG not ULONG
	return InterlockedExchange( (volatile LONG *)p, val );
#  endif
#else
#  if defined( __LINUX__ ) || defined( __LINUX64__ ) 
//DoXchg( p, val );
#    if defined( __ARM__ )
	_32 result;
#       if __thumb____whatever
   _32 tmp;
// assembly courtesy of ....
//http://lab.etfto.gov.br/~ccm/GPL_WAP54G/tools-src/gnu-20010422/libstdc++-v3/config/cpu/arm/bits/atomicity.h
	__asm__ __volatile__ (
								 "ldr     %0, 4f \n\t"
								 "bx      %0 \n\t"
								 ".align 0 \n"
								 "4:\t"
								 ".word   0f \n\t"
								 ".code 32\n"
								 "0:\t"
								 "swp     %0, %3, [%2] \n\t"
								 "ldr     %1, 1f \n\t"
								 "bx      %1 \n"
								 "1:\t"
								 ".word   2f \n\t"
								 ".code 16 \n"
								 "2:\n"
								 : "=&l"(result), "=&r"(tmp)
								 : "r"(p), "r"(val)
								 : "memory");
	return result;
#       else
	__asm__ __volatile__("swp %0, %2 [%1];" \
								: "=&r" (result)        \
								: "r"(p), "r" (val)           \
								: "memory");
	return result;
#       endif
#    else /* not arm (intel?) */
	__asm__( WIDE("lock xchg (%2),%0"):"=a"(val):"0"(val),"c"(p) );
	return val;
#    endif
#  else /* some other system, not windows, not linux... */
	{
#warning compiling C fallback locked exchange.  This is NOT atomic, and needs to be
	// swp is the instruction....
		_32 prior = *p;
		*p = val;
		return prior;
	}
#  endif
#endif
}

 _64  LockedExchange64 ( PV_64 p, _64 val )
{
	// Windows only available - for linux platforms please consult
	// the assembly version should be consulted
#ifdef WIN32
#ifdef _MSC_VER
   _64 prior = InterlockedExchange( (PS_32)p, (S_32)val ) | InterlockedExchange( ((PS_32)p)+1, (S_32)(val >> 32) );
#else
   _64 prior = InterlockedExchange( (volatile LONG*)p, (S_32)val ) | InterlockedExchange( ((volatile LONG*)p)+1, (_32)(val >> 32) );
#endif
	return prior;
#else
	{
		// swp is the instruction....
      // going to have to set IRQ, PIRQ on arm...
		_64 prior = *p;
		*p = val;
		return prior;
	}
#endif
}

#ifdef __cplusplus
/*
 PTRSZVAL  LockedExchange ( PVPTRSZVAL p, PTRSZVAL val )
{
	// Windows only available - for linux platforms please consult
	// the assembly version should be consulted
#ifdef WIN32
#ifdef _MSC_VER
   _64 prior = InterlockedExchange( (PS_32)p, (S_32)val ) | InterlockedExchange( ((PS_32)p)+1, (S_32)(val >> 32) );
#else
   _64 prior = InterlockedExchange( p, (_32)val ) | InterlockedExchange( ((P_32)p)+1, (_32)(val >> 32) );
#endif
	return prior;
#else
	{
		// swp is the instruction....
      // going to have to set IRQ, PIRQ on arm...
		_64 prior = *p;
		*p = val;
		return prior;
	}
#endif
}
*/
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _MSC_VER
#ifndef UNDER_CE
#include <intrin.h>
#endif
#endif

 void  MemSet ( POINTER p, PTRSZVAL n, PTRSZVAL sz )
{
#if defined( _MSC_VER ) && !defined( __NO_WIN32API__ ) && !defined( UNDER_CE )
#if defined( _WIN64 )
	__stosq( (_64*)p, n, sz/8 );
	if( sz & 4 )
		(*(_32*)( ((PTRSZVAL)p) + sz - (sz&7) ) ) = n;
	if( sz & 2 )
		(*(_16*)( ((PTRSZVAL)p) + sz - (sz&3) ) ) = n;
	if( sz & 1 )
		(*(_8*)( ((PTRSZVAL)p) + sz - (sz&1) ) ) = n;
#else
	__stosd( (_32*)p, n, sz / 4 );
	if( sz & 2 )
		(*(_16*)( ((PTRSZVAL)p) + sz - (sz&3) ) ) = n;
	if( sz & 1 )
		(*(_8*)( ((PTRSZVAL)p) + sz - (sz&1) ) ) = n;
#endif
#else
   memset( p, n, sz );
#endif
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 void  MemCpy ( POINTER pTo, CPOINTER pFrom, PTRSZVAL sz )
{
#if defined( _MSC_VER ) && !defined( __NO_WIN32API__ )&& !defined( UNDER_CE )

#ifdef _WIN64
	__movsq( (_64*)pTo, (_64*)pFrom, sz/8 );
	if( sz & 4 )
		(*(_32*)( ((PTRSZVAL)pTo) + sz - (sz&7) ) ) = (*(_32*)( ((PTRSZVAL)pFrom) + sz - (sz&7) ) );
#else
	__movsd( (_32*)pTo, (_32*)pFrom, sz/4 );
#endif
	if( sz & 2 )
		(*(_16*)( ((PTRSZVAL)pTo) + sz - (sz&3) ) ) = (*(_16*)( ((PTRSZVAL)pFrom) + sz - (sz&3) ) );
	if( sz & 1 )
		(*(_8*)( ((PTRSZVAL)pTo) + sz - (sz&1) ) ) = (*(_8*)( ((PTRSZVAL)pFrom) + sz - (sz&1) ) );
#else
	memcpy( pTo, pFrom, sz );
#endif
}

 int  MemCmp ( CPOINTER pOne, CPOINTER pTwo, PTRSZVAL sz )
{
	return memcmp( pOne, pTwo, sz );
}
#else
#ifdef __STATIC__
extern _32  LockedExchange( PV_32 p, _32 val );
extern void MemSet( POINTER p, PTRSZVAL n, PTRSZVAL sz );
extern void MemCpy( POINTER to, CPOINTER from, PTRSZVAL sz );
extern int MemCmp( CPOINTER p1, CPOINTER p2, PTRSZVAL sz );
#else
  _32   LockedExchange ( PV_32 p, _32 val );
  void  MemSet ( POINTER p, PTRSZVAL n, PTRSZVAL sz );
  void  MemCpy ( POINTER to, CPOINTER from, PTRSZVAL sz );
  int  MemCmp ( CPOINTER to, CPOINTER from, PTRSZVAL sz );
#endif
#endif

TEXTSTR StrCpyEx( TEXTSTR s1, CTEXTSTR s2, int n )
{
	int x;
	for( x = 0; x < n && (s1[x]=s2[x]); x++ );
	s1[n-1] = 0;
	return s1;
}

#undef StrCpy
TEXTSTR StrCpy( TEXTSTR s1, CTEXTSTR s2 )
{
	int x;
	for( x = 0; (s1[x]=s2[x]); x++ );
	return s1;
}


CTEXTSTR StrChr( CTEXTSTR s1, TEXTCHAR c )
{
	CTEXTSTR p1 = s1;
	if( !p1 ) return NULL;
	while( p1[0] && p1[0] != c ) p1++;
	if( p1[0] )
		return p1;
	return NULL;
}

CTEXTSTR StrRChr( CTEXTSTR s1, TEXTCHAR c )
{
	CTEXTSTR  p1 = s1;
	if( !p1 ) return NULL;
	while( p1[0] ) p1++;
	while( p1 != s1 && p1[0] != c ) p1--;
	if( p1[0] )
		return p1;
	return NULL;
}

#ifdef __cplusplus
TEXTSTR StrChr( TEXTSTR s1, TEXTCHAR c )
{
	TEXTSTR p1 = s1;
	if( !p1 ) return NULL;
	while( p1[0] && p1[0] != c ) p1++;
	if( p1[0] )
		return p1;
	return NULL;
}

TEXTSTR StrRChr( TEXTSTR s1, TEXTCHAR c )
{
	TEXTSTR  p1 = s1;
	if( !p1 ) return NULL;
	while( p1[0] ) p1++;
	while( p1 != s1 && p1[0] != c ) p1--;
	if( p1[0] )
		return p1;
	return NULL;
}
#endif

CTEXTSTR StrCaseStr( CTEXTSTR s1, CTEXTSTR s2 )
{
	int match = 0;
	CTEXTSTR p1, p2, began_at;
	if( !s1 )
		return NULL;
	if( !s2 )
		return s1;
	began_at = NULL;
	p1 = s1;
	p2 = s2;
	while( p1[0] && p2[0] )
	{
		if( p1[0] == p2[0] ||
			(((p1[0] >='a' && p1[0] <='z' )?p1[0]-('a'-'A'):p1[0])
			 == ((p2[0] >='a' && p2[0] <='z' )?p2[0]-('a'-'A'):p2[0]) ))
		{
			if( !match )
			{
				match++;
				began_at = p1;
			}
			//p2++;
		}	
		else
		{
			if( began_at )
			{
				p1 = began_at;  // start at the beginning again..
				p2 = s2; // reset searching string.
            began_at = NULL;
			}
			match = 0;
		}
		p1++;
      if( match )
			p2++;
	}
	if( !p2[0] )
		return began_at;
	return NULL;
}

CTEXTSTR StrStr( CTEXTSTR s1, CTEXTSTR s2 )
{
	int match = 0;
	CTEXTSTR p1, p2, began_at;
	if( !s1 )
		return NULL;
	if( !s2 )
		return s1;
	began_at = NULL;
	p1 = s1;
	p2 = s2;
	while( p1[0] && p2[0] )
	{
		if( p1[0] == p2[0] )
		{
			if( !match )
			{
				match++;
				began_at = p1;
			}
			//p2++;
		}	
		else
		{
			if( began_at )
			{
				p1 = began_at;  // start at the beginning again..
				p2 = s2; // reset searching string.
            began_at = NULL;
			}
			match = 0;
		}
		p1++;
      if( match )
			p2++;
	}
	if( !p2[0] )
		return began_at;
	return NULL;
}
#ifdef __cplusplus
TEXTSTR StrStr( TEXTSTR s1, CTEXTSTR s2 )
{
	int match = 0;
	TEXTSTR p1, began_at;
	CTEXTSTR p2;
	if( !s1 )
		return NULL;
	if( !s2 )
		return s1;
	began_at = p1 = s1; // set began_at here too..
	p2 = s2;
	while( p1[0] && p2[0] )
	{
		if( p1[0] == p2[0] )
		{
			if( !match )
			{
				match++;
				began_at = p1;
			}
		}
		else
		{
			if( began_at )
			{
				p1 = began_at;  // start at the beginning again..
				p2 = s2; // reset searching string.
            began_at = NULL;
			}
			match = 0;
		}
		p1++;
      if( match )
			p2++;
	}
	if( !p2[0] )
		return began_at;
	return NULL;
}
#endif


//-------------------------------------------------------------------------
 _32  LockedIncrement ( P_32 p)
{
	 if(p)
		  return (*p)++;
	 return 0;
}
//-------------------------------------------------------------------------
 _32  LockedDecrement ( P_32 p )
{
	 if(p)
		  return (*p)--;
	 return 0;
}
//-------------------------------------------------------------------------

#ifdef DEBUG_CRITICAL_SECTIONS
#if 0
static void DumpSection( PCRITICALSECTION pcs )
{
	lprintf( WIDE("Critical Section.....") );
	lprintf( WIDE("------------------------------") );
	lprintf( WIDE("Update: %08x"), pcs->dwUpdating );
	lprintf( WIDE("Current Process: %16Lx"), pcs->dwThreadID );
	lprintf( WIDE("Next Process:    %16Lx"), pcs->dwThreadWaiting );
	lprintf( WIDE("Last update: %s(%d)"), pcs->pFile?pcs->pFile:"unknown", pcs->nLine );
}
#endif
#endif



#ifdef __cplusplus
}; // namespace memory {
	namespace timers { // begin timer namespace

#endif
 _32  CriticalSecOwners ( PCRITICALSECTION pcs )
{
   return pcs->dwLocks;
}

 S_32  EnterCriticalSecNoWaitEx ( PCRITICALSECTION pcs, THREAD_ID *prior DBG_PASS )
{
	//_64 tick, tick2;
	THREAD_ID dwCurProc;
	//static int nEntry;

	dwCurProc = GetMyThreadID();

#ifdef DEBUG_CRITICAL_SECTIONS
	if( g.bLogCritical > 0 && g.bLogCritical < 2 )
		_lprintf( DBG_RELAY )( " [%16Lx] Attempt enter critical Section %p %08lx"
									, dwCurProc
									, pcs
									, pcs->dwLocks );
#endif
	// need to aquire lock on section...
   // otherwise our old mechanism allowed an enter in another thread
   // to falsely identify the section as its own while the real owner
	// tried to exit...

	if( XCHG( &pcs->dwUpdating, 1 ) )
	{
#ifdef DEBUG_CRITICAL_SECTIONS
		pcs->bCollisions++;
		//lprintf( "Collision++" );
		//DumpSection( pcs );
#endif
		return -1;
	}
#ifdef DEBUG_CRITICAL_SECTIONS
	if( pcs->bCollisions > 1 )
	{
		if( g.bLogCritical > 0 && g.bLogCritical < 2 )
			_lprintf(DBG_RELAY)( WIDE("Section is was updating %") _32f WIDE(" cycles"), pcs->bCollisions );
		pcs->bCollisions = 0;
	}
	else
		if( g.bLogCritical > 0 && g.bLogCritical < 2 )
			_lprintf(DBG_RELAY)( "Locked....for enter" );
#endif

	if( !pcs->dwThreadID )
	{
      // section is unowned...
		if( pcs->dwThreadWaiting )
		{
         // someone was waiting for it...
			if( pcs->dwThreadWaiting != dwCurProc )
			{
#ifdef DEBUG_CRITICAL_SECTIONS
				lprintf( "waiter is not myself... someone else wanted to own this section (more recently) than me." );
#endif
				if( prior && !(*prior) )
				{
#ifdef DEBUG_CRITICAL_SECTIONS
					lprintf( "Inserting myself as waiter..." );
#endif
					(*prior) = pcs->dwThreadWaiting;
					pcs->dwThreadWaiting = dwCurProc;
					//pcs->dwUpdating = 0;
					//return 0;
					// actually need to continue as if I had just
               // claimed the section...
				}
			}
		}
	}


	if( !pcs->dwThreadID || dwCurProc == pcs->dwThreadID )
	{
      // is unowned, and there was a waiting thread...
		if( (!pcs->dwThreadID) && pcs->dwThreadWaiting )
		{
         // someone was waiting... is the last one waiting me?
			if( pcs->dwThreadWaiting != dwCurProc )
			{

#ifdef DEBUG_CRITICAL_SECTIONS
				lprintf( "Was woken up as wrong thread.. %016Lx %016Lx %016Lx", dwCurProc, pcs->dwThreadWaiting, prior?(*prior):0 );
#endif
				// wake the correct thread...
				if( prior && !( *prior ) )
				{
					(*prior) = pcs->dwThreadWaiting;
				}
#ifdef DEBUG_CRITICAL_SECTIONS
				else if( prior )
				{
               lprintf( "Already saved the prior waiter and are setting self as waiter." );
				}
#endif
				pcs->dwThreadWaiting = dwCurProc;

				pcs->dwUpdating = 0;
				WakeThreadID( pcs->dwThreadWaiting );
            Relinquish();
            return 0;
			}
			else
			{
#ifdef DEBUG_CRITICAL_SECTIONS
            lprintf( "Was woken up as correct thread.. %016Lx %016Lx %016Lx", dwCurProc, pcs->dwThreadWaiting, prior?(*prior):0 );
#endif
            // is me... unowned section, and prior waiter is me.
				if( prior && (*prior ) )
				{
#ifdef DEBUG_CRITICAL_SECTIONS
					lprintf( "Moving prior into waiting..." );
#endif
					pcs->dwThreadWaiting = *prior;
               *prior = 0;
				}
				else
				{
					pcs->dwThreadWaiting = 0;
				}
			}
		}
		else
		{
         // otherwise threadID (is me) or !dwThreadWaiting
		}
		// otherwise 1) I won the thread already... (threadID == me )
      // or 2) there was not someone waiting...
		pcs->dwLocks++;
#ifdef DEBUG_CRITICAL_SECTIONS
		if( g.bLogCritical > 0 && g.bLogCritical < 2 )
			lprintf( "Locks are %08lx", pcs->dwLocks );
		if( ( pcs->dwLocks & 0xFFFFF ) > 1 )
		{

			if( pFile != __FILE__ )
			{
				if( g.bLogCritical > 0 && g.bLogCritical < 2 )
					_xlprintf( 1 DBG_RELAY )("!!!!  %p  Multiple Double entery! %"_32fx, pcs, pcs->dwLocks );
			}
		}
		pcs->pFile = pFile;
		pcs->nLine = nLine;
#endif
		pcs->dwThreadID = dwCurProc;
		pcs->dwUpdating = 0;
#ifdef DEBUG_CRITICAL_SECTIONS
		if( g.bLogCritical > 0 && g.bLogCritical < 2 )
			lprintf( "Entered, and unlocked for entry" );
#endif
		//nEntry--;
		return 1;
	}
	else // if( pcs->dwThreadID ) ... and it's not me
	{
	 	//if( !(pcs->dwLocks & SECTION_LOGGED_WAIT) )
		{
			pcs->dwLocks |= SECTION_LOGGED_WAIT;
#ifdef DEBUG_CRITICAL_SECTIONS
			lprintf( WIDE("Waiting on critical section owned by %s(%d) %08lx."), (pcs->pFile)?(pcs->pFile):"Unknown", pcs->nLine, pcs->dwLocks );
#endif
		}
      // if the prior is wante to be saved...
      if( prior )
		{
			if( *prior )
			{
				if( pcs->dwThreadWaiting != dwCurProc )
				{
					// assume that someone else kept our waiting ID...
               // cause we're not the one waiting, and we have someone elses ID..
					// we are awake out of order..
					pcs->dwUpdating = 0;
               return 0;
				}
			}
			if( pcs->dwThreadWaiting != dwCurProc )
			{
				if( prior && !(*prior) )
				{
					*prior = pcs->dwThreadWaiting;
				}
				else if( prior && (*prior ) )
				{
					lprintf( WIDE( "prior was set already... no room to store dwThreadWaiting..." ) );
					DebugBreak();
				}
				pcs->dwThreadWaiting = dwCurProc;
			}
		}
		else
		{
			// else no prior... so don't set the dwthreadwaiting...
#ifdef DEBUG_CRITICAL_SECTIONS
			_lprintf(DBG_RELAY)( WIDE( "No prior... not setting wake ID" ) );
#endif
		}
      //else
		//	pcs->dwThreadWaiting = dwCurProc;
		pcs->dwUpdating = 0;
#ifdef DEBUG_CRITICAL_SECTIONS
		if( g.bLogCritical > 0 && g.bLogCritical < 2 )
			_lprintf(DBG_RELAY)( WIDE( "Unlocked... for enter" ) );
#endif
		//nEntry--;
	}
	//lprintf( WIDE("Enter section : %"PRIdFAST64"\n"),tick2-tick );
	return 0;
}

//-------------------------------------------------------------------------

static LOGICAL LeaveCriticalSecNoWakeEx( PCRITICALSECTION pcs DBG_PASS )
#define LeaveCriticalSecNoWake(pcs) LeaveCriticalSecNoWakeEx( pcs DBG_SRC )
{
	THREAD_ID dwCurProc = GetMyThreadID();
	while( XCHG( &pcs->dwUpdating, 1 ) )
		Relinquish();
#ifdef DEBUG_CRITICAL_SECTIONS
	if( g.bLogCritical > 0 && g.bLogCritical < 2 )
		_lprintf(DBG_RELAY)( "Locked %p for leaving...", pcs );
#endif
	if( !( pcs->dwLocks & ~SECTION_LOGGED_WAIT ) )
	{
#ifdef DEBUG_CRITICAL_SECTIONS
		if( g.bLogCritical > 0 && g.bLogCritical < 2 )
			lprintf( DBG_FILELINEFMT WIDE( "Leaving a blank critical section" ) DBG_RELAY );
#endif
		//while( 1 );
		pcs->dwUpdating = 0;
		return FALSE;
	}
#ifdef DEBUG_CRITICAL_SECTIONS
	//if( g.bLogCritical > 1 )
	// lprintf( DBG_FILELINEFMT WIDE( "Leaving %Lx %Lx %p" ) DBG_RELAY ,pcs->dwThreadID, dwCurProc, pcs );
#endif
	if( pcs->dwThreadID == dwCurProc )
	{
		pcs->dwLocks--;
		if( pcs->dwLocks & SECTION_LOGGED_WAIT )
		{
			if( !( pcs->dwLocks & ~(SECTION_LOGGED_WAIT) ) )
			{
#ifdef DEBUG_CRITICAL_SECTIONS
				pcs->pFile = pFile;
				pcs->nLine = nLine;
#endif
				pcs->dwLocks = 0;
				pcs->dwThreadID = 0;
				// better be 0 already...
				//pcs->dwThreadWaiting = 0;
				Relinquish(); // allow whoever was waiting to go now...
			}
		}
		else
		{
			if( !pcs->dwLocks )
				pcs->dwThreadID = 0;
		}
		// don't wake the prior (if there is one sleeping)
		// pcs->dwThreadID = 0;
	}
	else
	{
#ifdef DEBUG_CRITICAL_SECTIONS
		{
			_xlprintf( 0 DBG_RELAY )( WIDE("Sorry - you can't leave a section owned by %08Lx %08lx %s(%d)...")
											, pcs->dwThreadID
											, pcs->dwLocks
											, (pcs->pFile)?(pcs->pFile):WIDE( "Unknown" ), pcs->nLine );
         DebugBreak();
		}
#else
		lprintf( WIDE("Sorry - you can't leave a section you don't own...") );
#endif
		pcs->dwUpdating = 0;
		return FALSE;
	}
	// allow other locking threads immediate access to section
	// but I know when that happens - since the waiting process
	// will flag - SECTION_LOGGED_WAIT
	//Relinquish();
	pcs->dwUpdating = 0;
#ifdef DEBUG_CRITICAL_SECTIONS
	if( g.bLogCritical > 0 && g.bLogCritical < 2 )
		_lprintf(DBG_RELAY)( WIDE( "Unocked %p for leaving..." ), pcs );
#endif
	return TRUE;
}

//-------------------------------------------------------------------------

 void  InitializeCriticalSec ( PCRITICALSECTION pcs )
{
#ifdef DEBUG_CRITICAL_SECTIONS
	lprintf( WIDE( "CLEARING CRITICAL SECTION" ) );
#endif
	MemSet( pcs, 0, sizeof( CRITICALSECTION ) );
	return;
}

#ifdef __cplusplus
}; // namespace timers {
	namespace memory { // resume memory namespace
#endif

//-------------------------------------------------------------------------




#define BASE_MEMORY (POINTER)0x80000000
// golly allocating a WHOLE DOS computer to ourselves? how RUDE
#define SYSTEM_CAPACITY  g.dwSystemCapacity


#define CHUNK_SIZE ( offsetof( CHUNK, byData ) )
#define MEM_SIZE  ( offsetof( MEM, pRoot ) )

//-----------------------------------------------------------------

#ifdef _DEBUG
static _32 dwBlocks; // last values from getmemstats...
static _32 dwFreeBlocks;
static _32 dwAllocated;
static _32 dwFree;
#endif

//------------------------------------------------------------------------------------------------------
static void DoCloseSpace( PSPACE ps, int bFinal );
//------------------------------------------------------------------------------------------------------

// hmm this runs
PRIORITY_ATEXIT(ReleaseAllMemory,ATEXIT_PRIORITY_SHAREMEM)
{
#ifndef __LINUX__
	// actually, under linux, it releases /tmp/.shared files.
	//lprintf( WIDE( "No super significant reason to release all memory blocks?" ) );
	//lprintf( WIDE( "Short circuit on memory shutdown." ) );
	return;
#else
	// need to try and close /tmp/.shared region files...  so we only close
   // temporary spaces
	PSPACEPOOL psp;
	PSPACE ps;
	while( ( psp = g.pSpacePool ) )
	{
		int i;
		for( i = 0; i < (((int)(MAX_PER_BLOCK))-1); i++ )
		{
			ps = psp->spaces + i;
			if( ps->pMem )
			{
				/*
             * if we do this, then logging will attempt to possibly use memory which was allocated from this?
#ifdef _DEBUG
				if( !g.bDisableDebug )
				{
					lprintf( WIDE("Space: %p mem: %p-%p"), ps, ps->pMem, (P_8)ps->pMem + ps->dwSmallSize );
					lprintf( WIDE("Closing tracked space...") );
				}
#endif
*/
				if( ps->flags.bTemporary )
					DoCloseSpace( ps, TRUE );
			}
		}
		if( ( (*psp->me) = psp->next ) )
			psp->next->me = psp->me;
#ifdef _WIN32
		UnmapViewOfFile( ps->pMem );
		CloseHandle( ps->hMem );
		CloseHandle( ps->hFile );
#else
		//lprintf( WIDE("unmaping space tracking structure...") );
		munmap( ps, MAX_PER_BLOCK * sizeof( SPACE ) );
		//close( (int)ps->pMem );
		//if( ps->hFile >= 0 )
		//	close( ps->hFile );
#endif
	}
#endif
	g.bInit = FALSE;
}

//------------------------------------------------------------------------------------------------------

void InitSharedMemory( void )
{
	 if( !g.bInit )
	 {
	 // this would be really slick to do
	 // especially in the case where files have been used
	 // to back storage...
	 // so please do make releaseallmemory smarter and dlea
	 // only with closing those regions which have a file
       // backing, espcecially those that are temporary chickens.
		//atexit( ReleaseAllMemory );
#ifdef _WIN32
		GetSystemInfo( &g.si );
#else
		g.pagesize = getpagesize();
#endif
#ifdef VERBOSE_LOGGING
		if( !g.bDisableDebug )
			Log2( WIDE("CHUNK: %d  MEM:%d"), CHUNK_SIZE, MEM_SIZE );
#endif
		g.bInit = TRUE;  // onload was definatly a zero.
		{
			PTRSZVAL dwSize = sizeof( SPACEPOOL );
			g.pSpacePool = (PSPACEPOOL)OpenSpace( NULL, NULL, &dwSize );
			if( g.pSpacePool )
			{
				MemSet( g.pSpacePool, 0, dwSize );
				g.pSpacePool->me = &g.pSpacePool;
#ifdef VERBOSE_LOGGING
				if( !g.bDisableDebug )
					Log1( WIDE("Allocated Space pool %lu"), dwSize );
#endif
			}
		}
	}
	else
	{
#ifdef VERBOSE_LOGGING
		if( !g.bDisableDebug )
			ODS( WIDE("already initialized?") );
#endif
	}
}

//------------------------------------------------------------------------------------------------------
// private
static PSPACE AddSpace( PSPACE pAddAfter
#if defined( __WINDOWS__ ) || defined( __CYGWIN__ )
							 , HANDLE hFile
							 , HANDLE hMem
#else
							 , int hFile
							 , int hMem
#endif
							 , POINTER pMem, PTRSZVAL dwSize, int bLink )
{
	PSPACEPOOL psp;
	PSPACEPOOL _psp = NULL;
	PSPACE ps;
	int i;
	if( !g.pSpacePool || g.InAdding )
	{
#ifdef VERBOSE_LOGGING
		if( !g.bDisableDebug )
			Log2( WIDE("No space pool(%p) or InAdding(%d)"), g.pSpacePool, g.InAdding );
#endif
		return NULL;
	}
	g.InAdding = 1;
	//_ps = NULL;
	psp = g.pSpacePool;
Retry:
	do {
		ps = psp->spaces;
		for( i = 0; i < (((int)(MAX_PER_BLOCK))-1); i++ )
		{
			if( !ps[i].pMem )
			{
				ps += i;
				break;
			}
		}
		if( i == (MAX_PER_BLOCK-1) )
		{
			_psp = psp;
			psp = psp->next;
		}
		else
			break;
	} while( psp );

	if( !psp )
	{
		//DebugBreak(); // examine conditions for allocating new space block...
		dwSize = sizeof( SPACEPOOL );
		if( _psp )
		{
			psp = _psp->next = (PSPACEPOOL)OpenSpace( NULL, NULL, &dwSize );
			MemSet( psp, 0, dwSize );
			psp->me = &_psp->next;
		}
		goto Retry;
	}
	//Log7( WIDE("Managing space (s)%p (pm)%p (hf)%08") _32fx WIDE(" (hm)%08") _32fx WIDE(" (sz)%") _32f WIDE(" %08") _32fx WIDE("-%08") _32fx WIDE("")
	//				, ps, pMem, (_32)hFile, (_32)hMem, dwSize
	//				, (_32)pMem, ((_32)pMem + dwSize)
	//				);
	ps->pMem = (PMEM)pMem;

   // okay yes I made this line ugly.
	ps->hFile =
#ifdef _WIN32
		         (HANDLE)
#endif
		                  hFile;
#ifdef _WIN32
	ps->hMem = hMem;
#endif
	ps->dwSmallSize = dwSize;
	/*
	if( bLink )
	{
		while( AddAfter && AddAfter->next )
			AddAfter = AddAfter->next;
		//Log2( WIDE("Linked into space...%p after %p "), ps, AddAfter );
		if( AddAfter )
		{
			ps->me = &AddAfter->next;
			AddAfter->next = ps;
		}
  		ps->next = NULL;
	}
	*/
	g.InAdding = 0;
	return ps;
}

//------------------------------------------------------------------------------------------------------

PSPACE FindSpace( POINTER pMem )
{
	PSPACEPOOL psp;
	INDEX idx;
	for( psp = g.pSpacePool;psp; psp = psp->next)
		for( idx = 0; idx < MAX_PER_BLOCK; idx++ )
			if( psp->spaces[idx].pMem == pMem )
				return psp->spaces + idx;
	return NULL;
}

//------------------------------------------------------------------------------------------------------

static void DoCloseSpace( PSPACE ps, int bFinal )
{
	if( ps )
	{
		//Log( WIDE("Closing a space...") );
#ifdef _WIN32
		UnmapViewOfFile( ps->pMem );
		CloseHandle( ps->hMem );
		CloseHandle( ps->hFile );
#else
		munmap( ps->pMem, ps->dwSmallSize );
		if( ps->flags.bTemporary && (ps->hFile >= 0) )
		{
			if( bFinal )
			{
				char file[256];
				char fdname[64];
				snprintf( fdname, sizeof(fdname), WIDE("/proc/self/fd/%d"), (int)ps->hFile );
				file[readlink( fdname, file, sizeof( file ) )] = 0;
				remove( file );
			}
			close( (int)ps->hFile );
		}
#endif
		MemSet( ps, 0, sizeof( SPACE ) );
	}
}

//------------------------------------------------------------------------------------------------------

 void  CloseSpaceEx ( POINTER pMem, int bFinal )
{
	DoCloseSpace( FindSpace( pMem ), bFinal );
}

//------------------------------------------------------------------------------------------------------

 void  CloseSpace ( POINTER pMem )
{
	DoCloseSpace( FindSpace( pMem ), TRUE );
}

//------------------------------------------------------------------------------------------------------

 PTRSZVAL  GetSpaceSize ( POINTER pMem )
{
	PSPACE ps;
	ps = FindSpace( pMem );
	if( ps )
		return ps->dwSmallSize;
	return 0;
}

#if defined( __LINUX__ ) && !defined( __CYGWIN__ )
PTRSZVAL GetFileSize( int fd )
{
	 PTRSZVAL len = lseek( fd, 0, SEEK_END );
	 lseek( fd, 0, SEEK_SET );
	 return len;
}

#endif
//------------------------------------------------------------------------------------------------------
 POINTER  OpenSpaceExx ( CTEXTSTR pWhat, CTEXTSTR pWhere, PTRSZVAL address, PTRSZVAL *dwSize, P_32 bCreated )
{
	POINTER pMem = NULL;
	//static _32 bOpening;
	static CRITICALSECTION cs;
	int readonly = FALSE;
	if( !g.bInit )
	{
		 //ODS( WIDE("Doing Init") );
		 InitSharedMemory();
	}
	while( !EnterCriticalSecNoWait( &cs, NULL ) )
      Relinquish();
	//while( XCHG( &bOpening, 1 ) )
	//	Relinquish();
	{
#ifdef __LINUX__
      char *filename = NULL;
		int fd = -1;
      int bTemp = FALSE;
		int exists = FALSE;
		if( !pWhat && !pWhere)
		{
			pMem = mmap( 0, *dwSize
						  , PROT_READ|PROT_WRITE
						  , MAP_SHARED|MAP_ANONYMOUS
						  , 0, 0 );
			if( pMem == (POINTER)-1 )
			{
				lprintf( "Something bad about this region sized %p", *dwSize );
            	DebugBreak();
			}
			//lprintf( WIDE("Clearing anonymous mmap %") _32f WIDE(""), *dwSize );
			MemSet( pMem, 0, *dwSize );
		}
		else if( pWhere ) // name doesn't matter, same file cannot be called another name
		{
         filename = StrDup( pWhere );
		}
		else if( pWhat )
		{
			int len;
         	len = snprintf( NULL, 0, WIDE("/tmp/.shared.%s"), pWhat );
         	filename = (char*)Allocate( len + 1 );
			snprintf( filename, len+1, WIDE("/tmp/.shared.%s"), pWhat );
         	bTemp = TRUE;
		}


		if( !pMem )
		{
			mode_t prior;
			prior = umask( 0 );
			if( bCreated )
				(*bCreated) = 1;
			fd = open( filename, O_RDWR|O_CREAT|O_EXCL, 0600 );
			umask(prior);
			if( fd == -1 )
			{
			// if we didn't create the file...
			// then it can't be marked as temporary...
				bTemp = FALSE;
				if( GetLastError() == EEXIST )
				{
					exists = TRUE;
					fd = open( filename, O_RDWR );
					bTemp = FALSE;
					if( bCreated )
                  (*bCreated) = 0;
				}
				if( fd == -1 )
				{
					readonly = TRUE;
					fd = open( filename, O_RDONLY );
				}
				if( fd == -1 )
				{
					Log2( WIDE("Sorry - failed to open: %d %s")
						 , errno
						 , filename );
					LeaveCriticalSecNoWake( &cs );
				//bOpening = FALSE;
               if(filename)Release( filename );
					return NULL;
				}
			}
			if( exists )
			{
				if( GetFileSize( fd ) < (PTRSZVAL)*dwSize )
				{
				   // expands the file...
					ftruncate( fd, *dwSize );
				   //*dwSize = ( ( *dwSize + ( FILE_GRAN - 1 ) ) / FILE_GRAN ) * FILE_GRAN;
				}
				else
				{
				   // expands the size requested to that of the file...
					(*dwSize) = GetFileSize( fd );
				}
			}
			else
			{
				if( !*dwSize )
				{
				// can't create a 0 sized file this way.
					(*dwSize) = 1; // not zero.
					close( fd );
					unlink( filename );
								 //bOpening = FALSE;
					LeaveCriticalSecNoWake( &cs );
					if(filename)Release( filename );
					return NULL;
				}
				//*dwSize = ( ( *dwSize + ( FILE_GRAN - 1 ) ) / FILE_GRAN ) * FILE_GRAN;
				ftruncate( fd, *dwSize );
			}
			pMem = mmap( 0, *dwSize
						  , PROT_READ|(readonly?(0):PROT_WRITE)
							 , MAP_SHARED|((fd<0)?MAP_ANONYMOUS:0)
							  , fd, 0 );
			if( !exists && pMem )
			{
				MemSet( pMem, 0, *dwSize );
			}
		}
		if( pMem )
		{
			PSPACE ps = AddSpace( NULL, fd, 0, pMem, *dwSize, TRUE );
         if( ps )
				ps->flags.bTemporary = bTemp;
		}
		LeaveCriticalSecNoWake( &cs );
		if(filename)Release( filename );
		return pMem;

#elif defined( _WIN32 )
#ifndef UNDER_CE
		HANDLE hFile;
		HANDLE hMem = NULL;
		*dwSize = ( ( (*dwSize) + ( FILE_GRAN - 1 ) ) / FILE_GRAN ) * FILE_GRAN;
		if( !pWhat && !pWhere )
		{
			//lprintf( "ALLOCATE %Ld", (*dwSize)>>32, 0 );
			hMem = CreateFileMapping( INVALID_HANDLE_VALUE, NULL
											, PAGE_READWRITE
											 |SEC_COMMIT
											, (*dwSize)>>32 // dwSize is sometimes 64 bit... this should be harmless
											, (*dwSize) & (0xFFFFFFFF)
											, pWhat ); // which should be NULL... but is consistant
			if( !hMem )
			{
				//lprintf( "Failed to allocate pagefile memory?! %p %d", *dwSize, GetLastError() );

				{
					POINTER p = malloc( *dwSize );
					//lprintf(" but we could allocate it %p", p  );
					LeaveCriticalSecNoWake( &cs );
					return p;
				}
			}
			else
			{
			// created and this size is right...
				if( bCreated )
					(*bCreated) = TRUE;
			}
		}
		else if( pWhat )
		{
			hMem = OpenFileMapping( FILE_MAP_READ|FILE_MAP_WRITE
										 , FALSE
										 , pWhat );
			if( hMem )
			{
				if( bCreated )
					(*bCreated) = FALSE;
			}
			else
			{
#ifdef DEBUG_OPEN_SPACE
				lprintf( WIDE("Failed to open region named %s %d"), pWhat, GetLastError() );
#endif
				if( (*dwSize) == 0 )  // don't continue... we're expecting open-existing behavior
				{
               LeaveCriticalSecNoWake( &cs );
					return FALSE;
				}
			}
		}

		hFile = INVALID_HANDLE_VALUE;
      // I would have hmem here if the file was validly opened....
		if( !hMem )
		{
			hFile = CreateFile( pWhere, GENERIC_READ|GENERIC_WRITE
									,FILE_SHARE_READ|FILE_SHARE_WRITE//|FILE_SHARE_DELETE
									,NULL // default security
									,(dwSize&&(*dwSize)) ? OPEN_ALWAYS : OPEN_EXISTING
									,FILE_ATTRIBUTE_NORMAL //|FILE_ATTRIBUTE_TEMPORARY
									//| FILE_FLAG_WRITE_THROUGH
									//| FILE_FLAG_NO_BUFFERING
									// must access on sector bournds
									// must read complete sectors
									//| FILE_FLAG_DELETE_ON_CLOSE
									, NULL );
#ifdef DEBUG_OPEN_SPACE
			lprintf( WIDE("Create file %s result %d"), pWhere, hFile );
			lprintf( WIDE("File result is %ld (error %ld)"), hFile, GetLastError() );
#endif
			if( hFile == INVALID_HANDLE_VALUE )
			{
				readonly = 1;
				if( ( dwSize && (!(*dwSize )) ) && ( GetLastError() == ERROR_PATH_NOT_FOUND || GetLastError() == ERROR_FILE_NOT_FOUND ) )
				{
#ifdef DEBUG_OPEN_SPACE
					lprintf( WIDE("File did not exist, and we're not creating the file (0 size passed)") );
#endif
					LeaveCriticalSecNoWake( &cs );
					return NULL;
				}
				hFile = CreateFile( pWhere, GENERIC_READ
										,FILE_SHARE_READ//|FILE_SHARE_DELETE
										,NULL // default security
										,OPEN_ALWAYS
										,FILE_ATTRIBUTE_NORMAL //|FILE_ATTRIBUTE_TEMPORARY
										//| FILE_FLAG_WRITE_THROUGH
										//| FILE_FLAG_NO_BUFFERING
										// must access on sector bournds
										// must read complete sectors
										//| FILE_FLAG_DELETE_ON_CLOSE
										, NULL );
#ifdef DEBUG_OPEN_SPACE
				lprintf( WIDE("Create file %s result %d"), pWhere, hFile );
#endif
				if( hFile != INVALID_HANDLE_VALUE )
					SetLastError( ERROR_ALREADY_EXISTS ); // lie...
			}
			else
				SetLastError( ERROR_ALREADY_EXISTS ); // lie...

			if( hFile == INVALID_HANDLE_VALUE )
			{
				readonly = 0;
#ifdef DEBUG_OPEN_SPACE
				lprintf( WIDE("file is still invalid(alreadyexist?)... new size is %d %d on %p"), (*dwSize), FILE_GRAN, hFile );
#endif
				hMem = CreateFileMapping( hFile // is INVALID_HANDLE_VALUE, but is consistant
												, NULL
												, (readonly?PAGE_READONLY:PAGE_READWRITE)
												 /*|SEC_COMMIT|SEC_NOCACHE*/
												, 0, (*dwSize)
												, pWhat );
				if( hMem )
				{
					if( bCreated )
						(*bCreated) = 1;
					goto isokay;
				}
#ifdef DEBUG_OPEN_SPACE
				lprintf( WIDE("Sorry - Nothing good can happen with a filename like that...%s %d"), pWhat, GetLastError());
#endif
					  //bOpening = FALSE;
				LeaveCriticalSecNoWake( &cs );
				return NULL;
			}

			if( GetLastError() == ERROR_ALREADY_EXISTS )
			{
				LARGE_INTEGER lSize;
				GetFileSizeEx( hFile, &lSize );
			// mark status for memory... dunno why?
				// in theory this is a memory image of valid memory already...
#ifdef DEBUG_OPEN_SPACE
				lprintf( WIDE("Getting existing size of region...") );
#endif
				if( lSize.QuadPart < (*dwSize) )
				{
#ifdef DEBUG_OPEN_SPACE
					lprintf( WIDE("Expanding file to size requested.") );
#endif
					SetFilePointer( hFile, *dwSize, NULL, FILE_BEGIN );
					SetEndOfFile( hFile );
				}
				else
				{
#ifdef DEBUG_OPEN_SPACE
					lprintf( WIDE("Setting size to size of file (which was larger..") );
#endif
					(*dwSize) = lSize.QuadPart;
				}
				if( bCreated )
					(*bCreated) = 1;
			}
			else
			{
#ifdef DEBUG_OPEN_SPACE
				lprintf( WIDE("New file, setting size to requested %d"), *dwSize );
#endif
				SetFilePointer( hFile, *dwSize, NULL, FILE_BEGIN );
				SetEndOfFile( hFile );
				if( bCreated )
					(*bCreated) = 0;
			}
			//(*dwSize) = GetFileSize( hFile, NULL );
#ifdef DEBUG_OPEN_SPACE
			lprintf( WIDE("%s Readonly? %d  hFile %d"), pWhat, readonly, hFile );
#endif
			hMem = CreateFileMapping( hFile
											, NULL
											, (readonly?PAGE_READONLY:PAGE_READWRITE)
											 /*|SEC_COMMIT|SEC_NOCACHE*/
											, 0, 0
											, pWhat );
			if( !hMem )
			{
#ifdef DEBUG_OPEN_SPACE
				lprintf( WIDE("Create of mapping failed on object specified? %d %p"), GetLastError(), hFile );
#endif
				(*dwSize) = 1;
				CloseHandle( hFile );
				//bOpening = FALSE;
				LeaveCriticalSecNoWake( &cs );
				return NULL;
			}
		}
	isokay:
      /*
		if( !hMem )
		{
			pMem = VirtualAlloc( address, (*dwSize), 0 ,PAGE_READWRITE );
			if( !VirtualLock( pMem, (*dwSize ) ) )
            DebugBreak();
		}
		else
      */
		{
			pMem = MapViewOfFileEx( hMem
										 , FILE_MAP_READ| ((readonly)?(0):(FILE_MAP_WRITE))
										 , 0, 0  // offset high, low
										 , 0	  // size of file to map
										 , (POINTER)address ); // don't specify load location... irrelavent...
		}
	if( !pMem )
	{
#ifdef DEBUG_OPEN_SPACE
		Log1( WIDE("Create view of file for memory access failed at %p"), (POINTER)address );
#endif
		CloseHandle( hMem );
		if( hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );
		//bOpening = FALSE;
		LeaveCriticalSecNoWake( &cs );
		return NULL;
	}
	else
	{
		if( bCreated && !(*bCreated) && ((*dwSize) == 0) ) 
		{
			MEMORY_BASIC_INFORMATION meminfo;
			VirtualQuery( pMem, &meminfo, sizeof( meminfo ) );
			(*dwSize) = meminfo.RegionSize;
#ifdef DEBUG_OPEN_SPACE
			lprintf( WIDE("Fixup memory size to %ld %s:%s(reported by system on view opened)")
					 , *dwSize, pWhat?pWhat:"ANON", pWhere?pWhere:"ANON" );
#endif
		}
	}
	// store information about this
	// external to the space - do NOT
	// modify content of memory opened!
	AddSpace( NULL, hFile, hMem, pMem, *dwSize, TRUE );
					  //bOpening = FALSE;
	LeaveCriticalSecNoWake( &cs );
	return pMem;
#else
	if( bCreated )
      (*bCreated) = 1;
	return malloc( *dwSize );
#endif
#endif
	}
}

//------------------------------------------------------------------------------------------------------
#undef OpenSpaceEx
 POINTER  OpenSpaceEx ( CTEXTSTR pWhat, CTEXTSTR pWhere, PTRSZVAL address, PTRSZVAL *dwSize )
{
	_32 bCreated;
	return OpenSpaceExx( pWhat, pWhere, address, dwSize, &bCreated );
}

//------------------------------------------------------------------------------------------------------
#undef OpenSpace
 POINTER  OpenSpace ( CTEXTSTR pWhat, CTEXTSTR pWhere, PTRSZVAL *dwSize )
{
	return OpenSpaceEx( pWhat, pWhere, 0, dwSize );
}
//------------------------------------------------------------------------------------------------------

 int  InitHeap( PMEM pMem, PTRSZVAL dwSize )
{
	//pMem->dwSize = *dwSize - MEM_SIZE;
	// size of the PMEM block is all inclusive (from pMem(0) to pMem(dwSize))
	// do NOT need to substract the size of the tracking header
	// otherwise we would be working from &pMem->pRoot + dwSize
	if( pMem->dwSize )
	{
		if( pMem->dwHeapID != 0xbab1f1ea )
		{
			lprintf( WIDE("Memory has content, and is NOT a heap!") );
			return FALSE;
		}
		lprintf( WIDE("Memory was already initialized as a heap?") );
		return FALSE;
	}
	if( !FindSpace( pMem ) )
	{
		//lprintf( WIDE("space for heap has not been tracked yet....") );
		// a heap must be in the valid space pool.
		// it may not have come from a file, and will not have 
		// a file or memory handle.
		AddSpace( NULL, 0, 0, pMem, dwSize, TRUE );
	}
	pMem->dwSize = dwSize;
	pMem->dwHeapID = 0xbab1f1ea;
	pMem->pFirstFree = NULL;
	LinkThing( pMem->pFirstFree, pMem->pRoot );
	InitializeCriticalSec( &pMem->cs );
	pMem->pRoot[0].dwSize = dwSize - MEM_SIZE - CHUNK_SIZE;
	pMem->pRoot[0].dwPad = MAGIC_SIZE;
	pMem->pRoot[0].dwOwners = 0;
	pMem->pRoot[0].pRoot  = pMem;
	pMem->pRoot[0].pPrior = NULL;
#ifdef _DEBUG
	if( !g.bDisableDebug )
	{
#ifdef VERBOSE_LOGGING
		lprintf( WIDE("Initializing %p %d")
				, pMem->pRoot[0].byData
				, pMem->pRoot[0].dwSize );
#endif
		MemSet( pMem->pRoot[0].byData, 0x1BADCAFE, pMem->pRoot[0].dwSize );
#ifdef __LINUX64__
		BLOCK_TAG( pMem->pRoot ) = BLOCK_TAG_ID;
#else
		BLOCK_TAG( pMem->pRoot ) = BLOCK_TAG_ID;
#endif
	}
	{
		pMem->pRoot[0].dwPad += 2*MAGIC_SIZE;
		BLOCK_FILE( pMem->pRoot ) = _WIDE(__FILE__);
		BLOCK_LINE( pMem->pRoot ) = __LINE__;
	}
#endif
	return TRUE;
}

//------------------------------------------------------------------------------------------------------

PMEM DigSpace( TEXTSTR pWhat, TEXTSTR pWhere, PTRSZVAL *dwSize )
{
	 PMEM pMem = (PMEM)OpenSpace( pWhat, pWhere, dwSize );

	 if( !pMem )
	 {
		  TEXTCHAR byDebug[256];
		  // did reference BASE_MEMORY...
		  snprintf( byDebug, sizeof( byDebug), WIDE("Create view of file for memory access failed at ????") );
		  ODS( byDebug );
		  CloseSpace( (POINTER)pMem );
		  return NULL;
	 }
#ifdef VERBOSE_LOGGING
	 Log( WIDE("Go to init the heap...") );
#endif
	 pMem->dwSize = 0;
#if defined( USE_CUSTOM_ALLOCER )
	 InitHeap( pMem, *dwSize );
#endif
	 return pMem;
}

//------------------------------------------------------------------------------------------------------

int ExpandSpace( PMEM pHeap, PTRSZVAL dwAmount )
{
	PSPACE pspace = FindSpace( (POINTER)pHeap ), pnewspace;
	PMEM pExtend;
	//lprintf( WIDE("Expanding by %d %d"), dwAmount );
	pExtend = DigSpace( NULL, NULL, &dwAmount );
	if( !pExtend )
	{
		Log1( WIDE("Failed to expand space by %") _32f, dwAmount );
		return FALSE;
	}
	pnewspace = FindSpace( pExtend );
	if( pnewspace )
	{
		while( pspace && pspace->next )
			pspace = pspace->next;
		if( ( pspace->next = pnewspace ) )
		{
			pnewspace->me = &pspace->next;
		}
	}
	return TRUE;
}


//------------------------------------------------------------------------------------------------------


//static PMEM pMasterInstance;
//#ifdef USE_CUSTOM_ALLOCER

static PMEM GrabMemEx( PMEM pMem DBG_PASS )
#define GrabMem(m) GrabMemEx( m DBG_SRC )
{
	if( !pMem )
	{
		// use default heap...
		if( !XCHG( &g.bMemInstanced, TRUE ) )
		{
			PTRSZVAL MinSize = SYSTEM_CAPACITY;
			// generic internal memory, unnamed, unshared, unsaved
			g.pMemInstance = pMem = DigSpace( NULL, NULL, &MinSize );
			if( !pMem )
			{
				g.bMemInstanced = FALSE;
				ODS( WIDE("Failed to allocate memory - assuming fatailty at Allocation service level.") );
				return NULL;
			}
		}
		else
			return 0;
	}
	//lprintf( WIDE("grabbing memory %p"), pMem );
#ifdef DEBUG_CRITICAL_SECTIONS
	{
      int log = g.bLogCritical;
		g.bLogCritical = 0;
#endif
	while( EnterCriticalSecNoWaitEx( &pMem->cs, NULL DBG_RELAY ) <= 0 )
	{
#ifdef _DEBUG
		//if( !g.bDisableDebug )
		//	lprintf( WIDE("Waiting for lock in grabmem") );
#endif
		Relinquish();
	}
#ifdef DEBUG_CRITICAL_SECTIONS
	g.bLogCritical = log;
	}
#endif

	return pMem;
}

//------------------------------------------------------------------------------------------------------

static void DropMemEx( PMEM pMem DBG_PASS )
#define DropMem(m) DropMemEx( m DBG_SRC)
{
	if( !pMem )
		return;
   //lprintf( WIDE("dropping memory %p"), pMem );
#ifdef DEBUG_CRITICAL_SECTIONS
	{
      int log = g.bLogCritical;
		g.bLogCritical = 0;
#endif
	LeaveCriticalSecNoWakeEx( &pMem->cs DBG_RELAY );
#ifdef DEBUG_CRITICAL_SECTIONS
	g.bLogCritical = log;
	}
#endif
}

//------------------------------------------------------------------------------------------------------

POINTER HeapAllocateEx( PMEM pHeap, PTRSZVAL dwSize DBG_PASS )
{
#if !defined( USE_CUSTOM_ALLOCER )
	if( !pHeap )
	{
		PCHUNK pc = (PCHUNK)malloc( sizeof( CHUNK ) + dwSize );
		pc->dwOwners = 1;
		pc->dwSize = dwSize;
      if( g.bLogAllocate )
			_lprintf(DBG_RELAY)( "alloc %p(%p) %d", pc, pc->byData, dwSize );
		return pc->byData;
	}
	else
#endif
	{
		PHEAP_CHUNK pc;
		PMEM pMem, pCurMem = NULL;
		PSPACE pMemSpace;
		_32 dwPad = 0;
		//_lprintf(DBG_RELAY)( "..." );
#ifdef _DEBUG
		if( !g.bDisableAutoCheck )
			GetHeapMemStatsEx(pHeap, &dwFree,&dwAllocated,&dwBlocks,&dwFreeBlocks DBG_RELAY);
#endif
		if( !dwSize ) // no size is NO space!
		{
			return NULL;
		}
		// if memstats is used - memory could have been initialized there...
		// so wait til now to grab g.pMemInstance.
		if( !pHeap )
			pHeap = g.pMemInstance;
		pMem = GrabMem( pHeap );
#ifdef _WIN64
		dwSize += 7; // fix size to allocate at least _32s which
		dwSize &= 0xFFFFFFFFFFFFFFF8;
#else
		dwSize += 3; // fix size to allocate at least _32s which
		dwSize &= 0xFFFFFFFC;
#endif

#ifdef _DEBUG
		if( !g.bDisableDebug )
		{
			dwPad += MAGIC_SIZE;
			dwSize += MAGIC_SIZE;  // add a _32 at end to mark, and check for application overflow...
			if( !pMem || !( pMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
			{
				dwPad += MAGIC_SIZE*2;
				dwSize += MAGIC_SIZE*2; // pFile, nLine per block...
				//lprintf( WIDE("Adding 8 bytes to block size...") );
			}
		}
#endif

		// re-search for memory should step long back...
	search_for_free_memory:
		for( pc = NULL, pMemSpace = FindSpace( pMem ); !pc && pMemSpace; pMemSpace = pMemSpace->next )
		{
			// grab the new memory (might be old, is ok)
			GrabMem( (PMEM)pMemSpace->pMem );
			// then drop old memory, don't need that anymore.
			if( pCurMem ) // first time through, there is no current.
				DropMem( pCurMem );
			// then mark that this block is our current block.
			pCurMem = (PMEM)pMemSpace->pMem;
			//lprintf( WIDE("region %p is now owned."), pCurMem );

			for( pc = pCurMem->pFirstFree; pc; pc = pc->next )
			{
				//Log2( WIDE("Check %d vs %d"), pc->dwSize, dwSize );
				if( pc->dwSize >= dwSize ) // if free block size is big enough...
				{
					// split block
					if( ( pc->dwSize - dwSize ) <= ( CHUNK_SIZE + g.nMinAllocateSize ) ) // must allocate it all.
					{
						pc->dwPad = (_16)(dwPad + ( pc->dwSize - dwSize ));
						UnlinkThing( pc );
						pc->dwOwners = 1;
						break; // successful allocation....
					}
					else
					{
						PHEAP_CHUNK pNew;  // cleared, NEW, uninitialized block...
						PHEAP_CHUNK next;
						next = (PHEAP_CHUNK)( pc->byData + pc->dwSize );
						pNew = (PHEAP_CHUNK)(pc->byData + dwSize);
						pNew->dwPad = 0;
						pNew->dwSize = ( ( pc->dwSize - CHUNK_SIZE ) - dwSize );
						if( pNew->dwSize & 0x80000000 )
							DebugBreak();

						pc->dwPad = (_16)dwPad;
						pc->dwSize = dwSize; // set old size?  this can wait until we have the block.
						if( pc->dwSize & 0x80000000 )
							DebugBreak();

						if( (PTRSZVAL)next - (PTRSZVAL)pCurMem < (PTRSZVAL)pCurMem->dwSize )  // not beyond end of memory...
							next->pPrior = pNew;

						pNew->dwOwners = 0;
						pNew->pRoot = pc->pRoot;
						pNew->pPrior = pc;

						// copy link...
						if( ( pNew->next = pc->next ) )
							pNew->next->me = &pNew->next;
						*( pNew->me = pc->me ) = pNew;

						pc->dwOwners = 1;  // set owned block.
						break; // successful allocation....
					}
				}
			}
		}
		if( !pc )
		{
			if( dwSize < SYSTEM_CAPACITY )
			{
				if( ExpandSpace( pMem, SYSTEM_CAPACITY ) )
					goto search_for_free_memory;
			}
			else
			{
				// after 1 allocation, need a free chunk at end...
				// and let's just have a couple more to spaere.
				if( ExpandSpace( pMem, dwSize + (CHUNK_SIZE*4) + MEM_SIZE ) )
				{
					lprintf( WIDE("Creating a new expanded space...") );
					goto search_for_free_memory;
				}
			}
			DropMem( pCurMem );
			pCurMem = NULL;

#ifdef _DEBUG
			if( !g.bDisableDebug )
				ODS( WIDE("Remaining space in memory block is insufficient.  Please EXPAND block."));
#endif
			DropMem( pMem );
			return NULL;
		}
		//#if DBG_AVAILABLE
		if( g.bLogAllocate )
		{
			_xlprintf( 2 DBG_RELAY )( WIDE("Allocate : %p(%p) - %") _32f WIDE(" bytes") , pc->byData, pc, pc->dwSize );
		}
		//#endif

#if defined( _DEBUG ) //|| !defined( __NO_WIN32API__ )
		if( !g.bDisableDebug )
		{
			// set end of block tag(s).
			// without disabling memory entirely, blocks are
			// still tagged and trashed in debug mode.
			MemSet( ((PCHUNK)pc)->byData, 0xDEADBEEF, pc->dwSize );
			BLOCK_TAG(pc) = BLOCK_TAG_ID;
		}
		if( !(pMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
		{
			BLOCK_FILE(pc) = pFile;
			BLOCK_LINE(pc) = nLine;
		}
#endif
		DropMem( pCurMem );
		DropMem( pMem );
		return pc->byData;
	}

   return NULL;
}

//------------------------------------------------------------------------------------------------------
#undef AllocateEx
 POINTER  AllocateEx ( PTRSZVAL dwSize DBG_PASS )
{
	return HeapAllocateEx( g.pMemInstance, dwSize DBG_RELAY );
}

//------------------------------------------------------------------------------------------------------

 POINTER  HeapReallocateEx ( PMEM pHeap, POINTER source, PTRSZVAL size DBG_PASS )
{
	POINTER dest;
	PTRSZVAL min;

	dest = HeapAllocateEx( pHeap, size DBG_RELAY );
	if( source )
	{
		min = SizeOfMemBlock( source );
		if( size < min )
			min = size;
		MemCpy( dest, source, min );
		if( min < size )
			MemSet( ((P_8)dest) + min, 0, size - min );
		ReleaseEx( source DBG_RELAY );
	}
	else
		MemSet( dest, 0, size );


	return dest;
}

//------------------------------------------------------------------------------------------------------

 POINTER  HeapPreallocateEx ( PMEM pHeap, POINTER source, PTRSZVAL size DBG_PASS )
{
	POINTER dest;
	PTRSZVAL min;

	dest = HeapAllocateEx( pHeap, size DBG_RELAY );
	if( source )
	{
		min = SizeOfMemBlock( source );
		if( size < min )
			min = size;
		MemCpy( (P_8)dest + (size-min), source, min );
		if( min < size )
			MemSet( dest, 0, size - min );
		ReleaseEx( source DBG_RELAY );
	}
	else
		MemSet( dest, 0, size );
	return dest;
}

//------------------------------------------------------------------------------------------------------

 POINTER  HeapMoveEx ( PMEM pNewHeap, POINTER source DBG_PASS )
{
	return HeapReallocateEx( pNewHeap, source, SizeOfMemBlock( source ) DBG_RELAY );
}

//------------------------------------------------------------------------------------------------------

 POINTER  ReallocateEx ( POINTER source, PTRSZVAL size DBG_PASS )
{
	return HeapReallocateEx( g.pMemInstance, source, size DBG_RELAY );
}

//------------------------------------------------------------------------------------------------------

 POINTER  PreallocateEx ( POINTER source, PTRSZVAL size DBG_PASS )
{
	return HeapPreallocateEx( g.pMemInstance, source, size DBG_RELAY );
}

//------------------------------------------------------------------------------------------------------

#if defined( USE_CUSTOM_ALLOCER )

static void Bubble( PMEM pMem )
{
	// handle sorting free memory to be least signficant first...
	PCHUNK temp, next;
	PCHUNK *prior;
	prior = &pMem->pFirstFree;
	temp = *prior;
	if( !temp )
		return;
	next = temp->next;
	while( temp && next )
	{
		if( (PTRSZVAL)next < (PTRSZVAL)temp )
		{
			UnlinkThing( temp );
			UnlinkThing( next );
			LinkThing( *prior, next );
			LinkThing( next->next, temp );
			prior = &next->next;
			temp = *prior;
			next = temp->next;
		}
		else
		{
			prior = &temp->next;
			temp = *prior;
			if( temp->next == temp )
			{
				lprintf( WIDE("OOps this block is way bad... how'd that happen? %s(%d)"), BLOCK_FILE( temp ), BLOCK_LINE( temp ) );
				DebugBreak();
			}
			next = temp->next;
		}
	}
}
#endif
//------------------------------------------------------------------------------------------------------

//static void CollapsePrior( PCHUNK pThis )
//{
//
//}

//------------------------------------------------------------------------------------------------------


 PTRSZVAL  SizeOfMemBlock ( CPOINTER pData )
{
	if( pData )
	{
		register PCHUNK pc = (PCHUNK)(((PTRSIZEVAL)pData) - offsetof( CHUNK, byData ));
		return pc->dwSize
#if defined( USE_CUSTOM_ALLOCER )
			- pc->dwPad
#endif
			;
	}
	return 0;
}
//------------------------------------------------------------------------------------------------------

 POINTER  MemDupEx ( CPOINTER thing DBG_PASS )
{
	PTRSZVAL size = SizeOfMemBlock( thing );
	POINTER result;
	result = AllocateEx( size DBG_RELAY );
	MemCpy( result, thing, size );
	return result;
}

#undef MemDup
 POINTER  MemDup (CPOINTER thing )
{
	return MemDupEx( thing DBG_SRC );
}
//------------------------------------------------------------------------------------------------------

POINTER ReleaseEx ( POINTER pData DBG_PASS )
{
#if !defined( USE_CUSTOM_ALLOCER )
	if( pData )
	{
		if( !( ((PTRSZVAL)pData) & 0x3FF ) )
		{
			// system allocated blocks ( OpenSpace ) will be tracked as spaces...
			// and they will be aligned on large memory blocks (4096 probably)
			PSPACE ps = FindSpace( pData );
			if( ps )
			{
				DoCloseSpace( ps, TRUE );
				return NULL;
			}
		}
		// how to figure if it's a CHUNK or a HEAP_CHUNK?
		{
			// register PMEM pMem = (PMEM)(pData - offsetof( MEM, pRoot ));
			register PCHUNK pc = (PCHUNK)(((PTRSZVAL)pData) -
													offsetof( CHUNK, byData ));
			pc->dwOwners--;
			if( !pc->dwOwners )
			{
				if( g.bLogAllocate )
					_lprintf(DBG_RELAY)( "Release %p(%p)", pc, pc->byData );
				free( pc );
				return NULL;
			}
			return pc;
		}
	}
	return NULL;
#else
	{
		register PCHUNK pc = (PCHUNK)(((PTRSZVAL)pData) -
												offsetof( CHUNK, byData ));
		PMEM pMem, pCurMem;
		PSPACE pMemSpace;
		if( !pData ) // always safe to release nothing at all..
			return NULL;

		if( g.bLogAllocate )
			_lprintf(DBG_RELAY)( "Release %p(%p)", pc, pc->byData );
		// Allow a simple release() to close a shared memory file mapping
		// this is a slight performance hit for all deallocations
	{
		PSPACE ps = FindSpace( pData );
		if( ps )
		{
			DoCloseSpace( ps, TRUE );
			return NULL;
		}
	}
#ifdef _DEBUG
	if( !g.bDisableAutoCheck )
		GetHeapMemStatsEx(pc->pRoot, &dwFree,&dwAllocated,&dwBlocks,&dwFreeBlocks DBG_RELAY);
#endif

	pMem = GrabMem( pc->pRoot );
	if( !pMem )
	{
		_lprintf( DBG_RELAY )( WIDE("ERROR: Chunk to free does not reference a heap!") );
		DebugBreak();
	}
	pMemSpace = FindSpace( pMem );

	while( pMemSpace && ( ( pCurMem = (PMEM)pMemSpace->pMem ),
			 (	( (PTRSZVAL)pData < (PTRSZVAL)pCurMem )
			 ||  ( (PTRSZVAL)pData > ( (PTRSZVAL)pCurMem + pCurMem->dwSize ) ) )
			 )
		  )
	{
		Log( WIDE("ERROR: This block should have immediatly referenced it's correct heap!") );
		 pMemSpace = pMemSpace->next;
	}
	if( !pMemSpace )
	{
#ifdef _DEBUG
		TEXTCHAR byMsg[256];
		snprintf( byMsg, sizeof( byMsg ), WIDE("This Block is NOT within the managed heap! : %p")
						  , pData );
		ODSEx( byMsg, pFile, nLine );
#endif
  		DebugBreak();
		DropMem( pMem );
		 return NULL;
	}
	pCurMem = (PMEM)pMemSpace->pMem;
	if( g.bLogAllocate )
	{
#ifdef _DEBUG
		if( !g.bDisableDebug )
			_xlprintf( 2 DBG_RELAY)( WIDE("Release  : %p(%p) - %") _32f WIDE(" bytes %s(%d)"), pc->byData, pc, pc->dwSize, BLOCK_FILE(pc), BLOCK_LINE(pc) );
		else
			_xlprintf( 2 DBG_RELAY )( WIDE("Release  : %p(%p) - %") _32f WIDE(" bytes"), pc->byData, pc, pc->dwSize );
#endif
	}
	if( pData && pc )
	{
		if( !pc->dwOwners )
		{
#ifdef _DEBUG
			if( !g.bDisableDebug &&
				 !(pCurMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
				_xlprintf( 2
				          , BLOCK_FILE(pc)
						  , BLOCK_LINE(pc) 
						  )( WIDE("Block is already Free! %p ")
						  , pc );
			else
#endif
				// CRITICAL ERROR!
				_xlprintf( 2 DBG_RELAY)( WIDE("Block is already Free! %p "), pc );
			DropMem( pMem );
			return pData;
		}
#ifdef _DEBUG
		if( !g.bDisableDebug )
			if( BLOCK_TAG( pc ) != BLOCK_TAG_ID )
			{
				// if this tag is invalid, then probably the file/line
            // data in the block has alse been squished.
				//if( !(pCurMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
				//{
					//lprintf( WIDE("%s(%d): Application overflowed memory:%p")
					//		 , BLOCK_FILE(pc)
					//		 , BLOCK_LINE(pc)
					//		 , pc->byData
					//		 );
				//}
				//else
				{
					lprintf( WIDE("Application overflowed memory:%p"), pc->byData );
				}
				DebugBreak();
			}
#endif
		pc->dwOwners--;
		if( pc->dwOwners )
		{
#ifdef _DEBUG
			if( !(pCurMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
			{
				// store where it was released from
				BLOCK_FILE(pc) = pFile;
				BLOCK_LINE(pc) = nLine;
			}
#endif
			DropMem( pMem );
			return pData;
		}
		else
		{
			LOGICAL bCollapsed = FALSE;
			PCHUNK next, nextNext, pPrior;
			unsigned int nNext;
			// fill memory with a known value...
			// this will allow me to check usage after release....

#ifdef _DEBUG
			if( !(pCurMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
			{
				// store where it was released from
				BLOCK_FILE(pc) = pFile;
				BLOCK_LINE(pc) = nLine;
			}
			if( !g.bDisableDebug )
			{
				BLOCK_TAG(pc)=BLOCK_TAG_ID;
				MemSet( pc->byData, 0xFACEBEAD, pc->dwSize - pc->dwPad );
			}
#endif
			next = (PCHUNK)(pc->byData + pc->dwSize);
			if( (nNext = (PTRSZVAL)next - (PTRSZVAL)pCurMem) >= pCurMem->dwSize )
			{
				// if next is NOT within valid memory...
				next = NULL;
			}

			if( ( pPrior = pc->pPrior ) ) // is not root chunk...
			{
				if( !pPrior->dwOwners ) // prior physical is free
				{
					pPrior->dwSize += CHUNK_SIZE + pc->dwSize; // add this header plus size
#ifdef _DEBUG
					//if( bLogAllocate )
					{
						//lprintf( WIDE("Collapsing freed block with prior block...%p %p"), pc, pPrior );
					}
					if( !g.bDisableDebug )
					{
						pPrior->dwPad = MAGIC_SIZE;
						BLOCK_TAG( pPrior ) = BLOCK_TAG_ID;
					}
					else
#endif
						pPrior->dwPad = 0; // *** NEEDFILELINE ***
#ifdef _DEBUG
					if( !(pMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
					{
						pPrior->dwPad += 2*MAGIC_SIZE;
						BLOCK_FILE(pPrior) = pFile;
						BLOCK_LINE(pPrior) = nLine;
					}
#endif
					if( pPrior->dwSize & 0x80000000 )
						DebugBreak();
					pc = pPrior; // use prior block as base ....
					if( next )
						next->pPrior = pPrior;
					bCollapsed = TRUE;
				}
			}
			// begin checking NEXT physical memory block for conglomerating
			if( next )
			{
				if( !next->dwOwners )
				{
					pc->dwSize += CHUNK_SIZE + next->dwSize;
					if( bCollapsed )
					{
						// pc is already in free list...
						UnlinkThing( next );
					}
					else
					{
						// otherwise need to use next's link spot
						// for this pc...
						if( (pc->next = next->next) )
							pc->next->me = &pc->next;
						*( pc->me = next->me ) = pc;
						bCollapsed = TRUE;
					}
#ifdef _DEBUG
					//if( bLogAllocate )
					{
						//lprintf( WIDE("Collapsing freed block with next block...%p %p"), pc, next );
					}
					if( !g.bDisableDebug )
					{
						pc->dwPad = MAGIC_SIZE;
						BLOCK_TAG( pc ) = BLOCK_TAG_ID;
					}
					else
#endif
						pc->dwPad = 0; // *** NEEDFILELINE ***
#ifdef _DEBUG
					if( !(pMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
					{
						pc->dwPad += 2*MAGIC_SIZE;
						BLOCK_FILE(pc) = pFile;
						BLOCK_LINE(pc) = nLine;
					}
#endif

					if( pc->dwSize & 0x80000000 )
						DebugBreak();
					nextNext = (PCHUNK)(pc->byData + pc->dwSize );

					if( (((PTRSZVAL)nextNext) - ((PTRSZVAL)pCurMem)) < (PTRSZVAL)pCurMem->dwSize )
					{
						nextNext->pPrior = pc;
					}
				}
			}

			if( !bCollapsed ) // no block near this one was free...
			{
				LinkThing( pc->pRoot->pFirstFree, pc );
			}
		}
	}
	Bubble( pMem );
	DropMem( pMem );
#ifdef _DEBUG
	if( !g.bDisableAutoCheck )
		GetHeapMemStatsEx(pc->pRoot, &dwFree,&dwAllocated,&dwBlocks,&dwFreeBlocks DBG_RELAY);
#endif
	}
	return NULL;
#endif
}

//------------------------------------------------------------------------------------------------------

 POINTER  HoldEx ( POINTER pData DBG_PASS )
{
#if !defined( USE_CUSTOM_ALLOCER )
	PCHUNK pc = (PCHUNK)((char*)pData - CHUNK_SIZE);
	pc->dwOwners++;
	return pData;
#else
	{
	PCHUNK pc = (PCHUNK)((char*)pData - CHUNK_SIZE);
	PMEM pMem = GrabMem( pc->pRoot );

	if( g.bLogAllocate )
	{
		_xlprintf( 2 DBG_RELAY)( WIDE("Hold	  : %p - %") _32f WIDE(" bytes"),pc, pc->dwSize );
	}
	if( !pc->dwOwners )
	{
		lprintf( WIDE("Held block has already been released!  too late to hold it!") );
		DebugBreak();
		DropMem( pMem );
		return pData;
	}
	pc->dwOwners++;
	DropMem(pMem );
	return pData;
	}
#endif
}

//------------------------------------------------------------------------------------------------------

 POINTER  GetFirstUsedBlock ( PMEM pHeap )
{
	return pHeap->pRoot[0].byData;
}

//------------------------------------------------------------------------------------------------------

 void  DebugDumpHeapMemEx ( PMEM pHeap, LOGICAL bVerbose )
{
#if defined( USE_CUSTOM_ALLOCER )
	PCHUNK pc, _pc;
	int nTotalFree = 0;
	int nChunks = 0;
	int nTotalUsed = 0;
	TEXTCHAR byDebug[256];
	PSPACE pMemSpace;
	PMEM pMem = GrabMem( pHeap ), pCurMem;

	pc = pMem->pRoot;

	ODS(WIDE(" ------ Memory Dump ------- ") );
	{
		TEXTCHAR byDebug[256];
		snprintf( byDebug, sizeof( byDebug ), WIDE("FirstFree : %p"),
					pMem->pFirstFree );
		ODS( byDebug );
	}

	for( pc = NULL, pMemSpace = FindSpace( pMem ); pMemSpace; pMemSpace = pMemSpace->next )
	{
		pCurMem = (PMEM)pMemSpace->pMem;
		pc = pCurMem->pRoot; // current pChunk(pc)

		while( (((PTRSZVAL)pc) - ((PTRSZVAL)pCurMem)) < (PTRSZVAL)pCurMem->dwSize ) // while PC not off end of memory
		{
#ifndef __LINUX__
			Relinquish(); // allow debug log to work... (OutputDebugString() Win32, also network streams may require)
#endif
			nChunks++;
			if( !pc->dwOwners )
			{
				nTotalFree += pc->dwSize;
				if( bVerbose )
				{
					snprintf( byDebug, sizeof( byDebug ), WIDE("Free at %p size: %") _32f WIDE("(%") _32fx WIDE(") Prior:%p NF:%p"),
						 pc, pc->dwSize, pc->dwSize,
						 pc->pPrior,
						 pc->next );
				}
			}
			else
			{
				nTotalUsed += pc->dwSize;
				if( bVerbose )
				{
					snprintf( byDebug, sizeof( byDebug ), WIDE("Used at %p size: %") _32f WIDE("(%") _32fx WIDE(") Prior:%p"),
						 pc, pc->dwSize, pc->dwSize,
						 pc->pPrior );
				}
			}
			if( bVerbose )
			{
#ifdef _DEBUG
				if( /*!g.bDisableDebug && */ !(pCurMem->dwFlags & HEAP_FLAG_NO_DEBUG ) && pc->dwPad )
				{
					CTEXTSTR pFile =  !IsBadReadPtr( BLOCK_FILE(pc), 1 )
							?BLOCK_FILE(pc)
							:WIDE("Unknown");
					_32 nLine = BLOCK_LINE(pc);
					_xlprintf( 2 DBG_RELAY)( WIDE("%s"), byDebug );
				}
				else
#endif
					ODS( byDebug );
			}
			_pc = pc;
			pc = (PCHUNK)(pc->byData + pc->dwSize );
			if( pc == _pc )
			{
				ODS( WIDE("Next block is the current block...") );
				DebugBreak(); // broken memory chain
				break;
			}
		}
	}
	snprintf( byDebug, sizeof( byDebug ), WIDE("Total Free: %d  TotalUsed: %d  TotalChunks: %d TotalMemory:%lu"),
				nTotalFree, nTotalUsed, nChunks,
				(long unsigned)(nTotalFree + nTotalUsed + nChunks * CHUNK_SIZE) );
	ODS( byDebug );
	//Relinquish();  // see notes on above Relinquish.
	DropMem( pMem );
#endif
}

//------------------------------------------------------------------------------------------------------

 void  DebugDumpMemEx ( LOGICAL bVerbose )
{
	DebugDumpHeapMemEx( g.pMemInstance, bVerbose );
}

//------------------------------------------------------------------------------------------------------

 void  DebugDumpHeapMemFile ( PMEM pHeap, CTEXTSTR pFilename )
{
#if defined( USE_CUSTOM_ALLOCER )
	FILE *file;
	Fopen( file, pFilename, WIDE("wt") );
	if( file )
	{
		PCHUNK pc, _pc;
		PMEM pMem, pCurMem;
		PSPACE pMemSpace;
		int nTotalFree = 0;
		int nChunks = 0;
		int nTotalUsed = 0;
		TEXTCHAR byDebug[256];

		pMem = GrabMem( pHeap );

		fprintf( file, WIDE(" ------ Memory Dump ------- \n") );
		{
			TEXTCHAR byDebug[256];
			snprintf( byDebug, sizeof( byDebug ), WIDE("FirstFree : %p"),
						pMem->pFirstFree );
			fprintf( file, WIDE("%s\n"), byDebug );
		}

		for( pc = NULL, pMemSpace = FindSpace( pMem ); pMemSpace; pMemSpace = pMemSpace->next )
		{
			pCurMem = (PMEM)pMemSpace->pMem;
			pc = pCurMem->pRoot; // current pChunk(pc)

			while( (((PTRSZVAL)pc) - ((PTRSZVAL)pCurMem)) < (PTRSZVAL)pCurMem->dwSize ) // while PC not off end of memory
			{
				//Relinquish(); // allow debug log to work...
				nChunks++;
				if( !pc->dwOwners )
				{
					nTotalFree += pc->dwSize;
					snprintf( byDebug, sizeof(byDebug), WIDE("Free at %p size: %") _32f WIDE("(%") _32fx WIDE(") Prior:%p NF:%p"),
						 pc, pc->dwSize, pc->dwSize,
						 pc->pPrior,
						 pc->next );
				}
				else
				{
					nTotalUsed += pc->dwSize;
					snprintf( byDebug, sizeof(byDebug), WIDE("Used at %p size: %") _32f WIDE("(%") _32fx WIDE(") Prior:%p"),
						 pc, pc->dwSize, pc->dwSize,
						 pc->pPrior );
				}
#ifdef _DEBUG
				if( !g.bDisableDebug && !(pCurMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
				{
					CTEXTSTR pFile =  !IsBadReadPtr( BLOCK_FILE(pc), 1 )
							?BLOCK_FILE(pc)
							:WIDE("Unknown");
					fprintf( file, WIDE("%s(%d):%s\n"), pFile, BLOCK_LINE(pc), byDebug );
				}
				else
#endif
					fprintf( file, WIDE("%s\n"), byDebug );
				_pc = pc;
				pc = (PCHUNK)(pc->byData + pc->dwSize );
				if( pc == _pc )
				{
					DebugBreak(); // broken memory chain
					break;
				}
			}
		}
		fprintf( file, WIDE("--------------- FREE MEMORY LIST --------------------\n") );

		for( pc = NULL, pMemSpace = FindSpace( pMem ); pMemSpace; pMemSpace = pMemSpace->next )
		{
			pCurMem = (PMEM)pMemSpace->pMem;
			pc = pCurMem->pFirstFree; // current pChunk(pc)

			while( pc ) // while PC not off end of memory
			{
				snprintf( byDebug, sizeof(byDebug), WIDE("Free at %p size: %") _32f WIDE("(%") _32fx WIDE(") "),
				 			pc, pc->dwSize, pc->dwSize );

	#ifdef _DEBUG
				if( /*!g.bDisableDebug && */ !(pCurMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
				{
					CTEXTSTR pFile =  !IsBadReadPtr( BLOCK_FILE(pc), 1 )
							?BLOCK_FILE(pc)
							:WIDE("Unknown");
					fprintf( file, WIDE("%s(%d):%s\n"), pFile, BLOCK_LINE(pc), byDebug );
				}
				else
	#endif
					fprintf( file, WIDE("%s\n"), byDebug );
				pc = pc->next;
			}
		}
		snprintf( byDebug, sizeof(byDebug), WIDE("Total Free: %d  TotalUsed: %d  TotalChunks: %d TotalMemory:%lu"),
					nTotalFree, nTotalUsed, nChunks,
					(long unsigned)(nTotalFree + nTotalUsed + nChunks * CHUNK_SIZE) );
		fprintf( file, WIDE("%s\n"), byDebug );
		//Relinquish();
		DropMem( pMem );

 		fclose( file );
	}
#endif
}

//------------------------------------------------------------------------------------------------------
 void  DebugDumpMemFile ( CTEXTSTR pFilename )
{
	DebugDumpHeapMemFile( g.pMemInstance, pFilename );
}
//------------------------------------------------------------------------------------------------------

 LOGICAL  Defragment ( POINTER *ppMemory ) // returns true/false, updates pointer
{
	// this is broken... needs
	// to fixup BLOCK_TAG, BLOCK_FILE, etc...
#if 1
	return FALSE;
#else
	// pass an array of allocated memory... for all memory blocks in list,
	// check to see if they can be reallocated lower, and or just moved to
	// a memory space lower than they are now.
	PCHUNK pc, pPrior;
	PMEM pMem;
	if( !ppMemory || !*ppMemory)
		return FALSE;
	pc = (PCHUNK)(((PTRSIZEVAL)(*ppMemory)) - offsetof( CHUNK, byData ));
	pMem = GrabMem( pc->pRoot );

		// check if prior block is free... if so - then...
		// move this data down, and reallocate the freeness at the end
		// this reallocation may move the free next to another free, which
		// should be collapsed into this one...
	pPrior = pc->pPrior;
	if( ( pc->dwOwners == 1 ) && // not HELD by others... no way to update their pointers
		 pPrior &&
		 !pPrior->dwOwners )
	{
		CHUNK Free = *pPrior;
		CHUNK Allocated, *pNew;
		Allocated = *pc; // save this chunk...
		MemCpy( pPrior->byData, pc->byData, Allocated.dwSize );
		pNew = (PCHUNK)(pPrior->byData + Allocated.dwSize);
		pNew->dwSize = Free.dwSize;
		pNew->dwOwners = 0;
		pNew->pPrior = pPrior; // now pAllocated...
		pNew->pRoot = Free.pRoot;
		if( ( pNew->next = Free.next ) )
			pNew->next->me = &pNew->next;
		if( ( pNew->me = Free.me ) )
			(*pNew->me) = pNew;
#ifdef _DEBUG
		if( !(pMem->dwFlags & HEAP_FLAG_NO_DEBUG ) )
		{
			BLOCK_FILE(pNew) = BLOCK_FILE(&Free);
			BLOCK_LINE(pNew) = BLOCK_LINE(&Free);
		}
#endif
		pPrior->dwSize = Allocated.dwSize;
		pPrior->dwOwners = 1;
		pPrior->next = NULL;
		pPrior->me = NULL;
		// update NEXT NEXT real block...
		{
			PCHUNK next;
			next = (PCHUNK)( pNew->byData + pNew->dwSize );

			if( (((PTRSZVAL)next) - ((PTRSZVAL)pMem)) < (PTRSZVAL)pMem->dwSize )
			{
				if( !next->dwOwners ) // if next is free.....
				{
					// consolidate...
					if( (pNew->next = next->next) )
						pNew->next->me = &pNew->next;
					*( pNew->me = next->me ) = pNew;

					pNew->dwSize += next->dwSize + CHUNK_SIZE;
					next = (PCHUNK)( pNew->byData + pNew->dwSize );
					if( (_32)(((char *)next) - ((char *)pMem)) < pMem->dwSize )
					{
						next->pPrior = pNew;
					}
				}
				else
					next->pPrior = pNew;
			}
		}
		*ppMemory = pPrior->byData;
		DropMem( pMem );
		GetHeapMemStats( g.pMemInstance, NULL, NULL, NULL, NULL );
		return TRUE;
	}

	DropMem( pMem );
	return FALSE;
#endif
}

//------------------------------------------------------------------------------------------------------

 void  GetHeapMemStatsEx ( PMEM pHeap, _32 *pFree, _32 *pUsed, _32 *pChunks, _32 *pFreeChunks DBG_PASS )
{
#if defined( USE_CUSTOM_ALLOCER )
	int nFree = 0, nUsed = 0, nChunks = 0, nFreeChunks = 0, nSpaces = 0;
	PCHUNK pc, _pc;
	PMEM pMem;
	PSPACE pMemSpace;
	if( !pHeap )
		pHeap = g.pMemInstance;
	pMem = GrabMem( pHeap );
	pMemSpace = FindSpace( pMem );
	while( pMemSpace )
	{
		PMEM pMemCheck = ((PMEM)pMemSpace->pMem);
		pc = pMemCheck->pRoot;
		GrabMem( pMemCheck );
		while( (((PTRSZVAL)pc) - ((PTRSZVAL)pMemCheck)) < (PTRSZVAL)pMemCheck->dwSize ) // while PC not off end of memory
		{
			nChunks++;
			if( !pc->dwOwners )
			{
				nFree += pc->dwSize;
				nFreeChunks++;
			}
			else
			{
				nUsed += pc->dwSize;
#ifdef _DEBUG
				if( !g.bDisableDebug )
				{
					if( pc->dwSize > pMemCheck->dwSize )
					{
						Log1( WIDE("Memory block %p has a corrupt size."), pc->byData );
						DebugBreak();
					}
					else
					{
						if( BLOCK_TAG(pc) != BLOCK_TAG_ID )
						{
							Log1( WIDE("memory block: %p"), pc->byData );
							if( !(pMemCheck->dwFlags & HEAP_FLAG_NO_DEBUG ) )
							{
								ODSEx( WIDE("Application overflowed allocated memory."), BLOCK_FILE(pc), BLOCK_LINE(pc) );
							}
							else
								ODS( WIDE("Application overflowed allocated memory.") );

							DebugBreak();
						}
					}
				}
#endif
			}

			_pc = pc;
			pc = (PCHUNK)(pc->byData + pc->dwSize );
			if( (((PTRSZVAL)pc) - ((PTRSZVAL)pMemCheck)) < (PTRSZVAL)pMemCheck->dwSize  )
			{
				if( pc == _pc )
				{
					Log( WIDE("Current block is the same as the last block we checked!") );
					DebugBreak(); // broken memory chain
					break;
				}
				if( pc->pPrior != _pc )
				{
					Log4( WIDE("Block's prior is not the last block we checked! prior %p sz: %") _32f WIDE(" current: %p currentprior: %p")
						, _pc
						, _pc->dwSize
						, pc
						, pc->pPrior );
					DebugBreak();
					break;
				}
			}
		}
		_pc = NULL;
		pc = pMemCheck->pFirstFree;
		while( pc )
		{
			if( pc->dwOwners )
			{  // owned block is in free memory chain ! ?
				lprintf( WIDE("Owned block %p is in free memory chain!"), pc );
				DebugBreak();
				break;
			}
			_pc = pc;
			pc = pc->next;
		}
		nSpaces++;
		pMemSpace = pMemSpace->next;
		DropMem( pMemCheck );
	}
	DropMem( pMem );
	if( pFree )
		*pFree = nFree;
	if( pUsed )
		*pUsed = nUsed;
	if( pChunks )
		*pChunks = nChunks;
	if( pFreeChunks )
		*pFreeChunks = nFreeChunks;
#endif
}

//------------------------------------------------------------------------------------------------------

 void  GetMemStats ( _32 *pFree, _32 *pUsed, _32 *pChunks, _32 *pFreeChunks )
{
	GetHeapMemStats( g.pMemInstance, pFree, pUsed, pChunks, pFreeChunks );
}

//------------------------------------------------------------------------------------------------------
 int  SetAllocateLogging ( LOGICAL bTrueFalse )
{
   LOGICAL prior = g.bLogAllocate;
	g.bLogAllocate = bTrueFalse;
   return prior;
}

//------------------------------------------------------------------------------------------------------

 int  SetCriticalLogging ( LOGICAL bTrueFalse )
{
#ifdef _DEBUG
   int prior = g.bLogCritical;
	g.bLogCritical = bTrueFalse;
	return prior;
#else
   return 0;
#endif
}
//------------------------------------------------------------------------------------------------------

 int  SetAllocateDebug ( LOGICAL bDisable )
{
#ifdef _DEBUG
	int save = g.bDisableDebug;
	g.bDisableDebug = bDisable;
	g.bDisableAutoCheck = bDisable;
	return save;
#else
   return 1;
#endif
}

 int  SetManualAllocateCheck ( LOGICAL bDisable )
{
#ifdef _DEBUG
	int save = g.bDisableAutoCheck;
	g.bDisableAutoCheck = bDisable;
	return save;
#else
   return 1;
#endif
}

//------------------------------------------------------------------------------------------------------

 void  SetMinAllocate ( int nSize )
{
	g.nMinAllocateSize = nSize;
}

//------------------------------------------------------------------------------------------------------

 void  SetHeapUnit ( int dwSize )
{
	g.dwSystemCapacity = dwSize;
}

//------------------------------------------------------------------------------------------------------
// result in 0(equal), 1 above, or -1 below
// *r contains the position of difference
 int  CmpMem8 ( void *s1, void *s2, unsigned long n, unsigned long *r )
{
	register int t1, t2;
	_32 pos;
	{
		pos = 0;
		while( pos < n )
		{
			t1 = *(unsigned char*)s1;
			t2 = *(unsigned char*)s2;
			if( ( t1 ) == ( t2 ) )
			{
				(pos)++;
				s1 = (void*)(((PTRSZVAL)s1) + 1);
				s2 = (void*)(((PTRSZVAL)s2) + 1);
			}
			else if( t1 > t2 )
			{
				if( r )
					*r = pos;
				return 1;
			}
			else
				if( r )
					*r = pos;
				return -1;
		}
	}
	if( r )
		*r = pos;
	return 0;
}

//------------------------------------------------------------------------------------------------------
#undef GetHeapMemStats
 void  GetHeapMemStats ( PMEM pHeap, _32 *pFree, _32 *pUsed, _32 *pChunks, _32 *pFreeChunks )
{
	GetHeapMemStatsEx( pHeap, pFree, pUsed, pChunks, pFreeChunks DBG_SRC );
}

 TEXTSTR  StrDupEx ( CTEXTSTR original DBG_PASS )
{
	if( original )
	{
		PTRSZVAL len = (PTRSZVAL)StrLen( original ) + 1;
		TEXTCHAR *result;
		result = (TEXTCHAR*)AllocateEx( sizeof(TEXTCHAR)*len  DBG_RELAY );
		MemCpy( result, original, sizeof(TEXTCHAR)*len );
		return result;
	}
	return NULL;
}

size_t StrLen( CTEXTSTR s )
{
	size_t l;
	if( !s )
		return 0;
	for( l = 0; s[l];l++);
	return l;	
}

size_t CStrLen( char *s )
{
	size_t l;
	if( !s )
		return 0;
	for( l = 0; s[l];l++);
	return l;	
}


 char *  CStrDupEx ( CTEXTSTR original DBG_PASS )
{
	INDEX len;
	char *result;
	if( !original )
		return NULL;
	for( len = 0; original[len]; len++ ); if( len )
	result = (char*)AllocateEx( (len+1) * sizeof( result[0] ) DBG_RELAY );
	len = 0;
	while( ( result[len] = original[len] ) != 0 ) len++;
	return result;
}

 TEXTSTR  DupCStrEx ( const char * original DBG_PASS )
{
	INDEX len = 0;
	TEXTSTR result;
	if( !original )
		return NULL;
	while( original[len] ) len++;
	result = (TEXTSTR)AllocateEx( (len+1) * sizeof( result[0] )  DBG_RELAY );
	len = 0;
	while( ( result[len] = original[len] ) != 0 ) len++;
	return result;
}


 int  StrCmp ( CTEXTSTR s1, CTEXTSTR s2 )
{
	if( !s1 )
		if( s2 )
			return -1;
		else
			return 0;
	else
		if( !s2 )
			return 1;
	if( s1 == s2 )
      return 0; // ==0 is success.
	for( ;s1[0] && s2[0] && ( s1[0] == s2[0] ); s1++, s2++ );
	return s1[0] - s2[0];
}

 int  StrCaseCmp ( CTEXTSTR s1, CTEXTSTR s2 )
{
	if( !s1 )
		if( s2 )
			return -1;
		else
			return 0;
	else
		if( !s2 )
			return 1;
	if( s1 == s2 )
		return 0; // ==0 is success.
	for( ;s1[0] && s2[0] && (((s1[0] >='a' && s1[0] <='z' )?s1[0]-('a'-'A'):s1[0])
									 == ((s2[0] >='a' && s2[0] <='z' )?s2[0]-('a'-'A'):s2[0]) );
		  s1++, s2++ );
	return tolower(s1[0]) - tolower(s2[0]);
}

 int  StrCmpEx ( CTEXTSTR s1, CTEXTSTR s2, INDEX maxlen )
{
	if( !s1 )
		if( s2 )
			return -1;
		else
			return 0;
	else
		if( !s2 )
			return 1;
	if( s1 == s2 )
      return 0; // ==0 is success.
	for( ;s1[0] && s2[0] && ( s1[0] == s2[0] ) && maxlen; s1++, s2++, maxlen-- );
   if( maxlen )
		return s1[0] - s2[0];
   return 0;
}

 int  StrCaseCmpEx ( CTEXTSTR s1, CTEXTSTR s2, INDEX maxlen )
{
	if( !s1 )
		if( s2 )
			return -1;
		else
			return 0;
	else
		if( !s2 )
			return 1;
	if( s1 == s2 )
		return 0; // ==0 is success.
	for( ;s1[0] && s2[0] && (((s1[0] >='a' && s1[0] <='z' )?s1[0]-('a'-'A'):s1[0])
									 == ((s2[0] >='a' && s2[0] <='z' )?s2[0]-('a'-'A'):s2[0]) ) && maxlen;
		  s1++, s2++, maxlen-- );
   if( maxlen )
		return tolower(s1[0]) - tolower(s2[0]);
   return 0;
}

 int  StriCmp ( CTEXTSTR pOne, CTEXTSTR pTwo )
{
   return -1;
}

#ifdef __cplusplus

#ifdef _MSC_VER 
//>= 900
#include <crtdbg.h>
#include <new.h>


_CRT_ALLOC_HOOK prior_hook;

int allocHook(int allocType, void *userData, size_t size, int 
blockType, long requestNumber, const unsigned char *filename, int 
lineNumber)
{
	static int logging;
	if( logging )
		return TRUE;
	logging = 1;
	switch( allocType )
	{
	case _HOOK_ALLOC:
		lprintf( "CRT Alloc: %d bytes %s(%d)"
			, size 
			, filename, lineNumber
			);
		break;
	case _HOOK_REALLOC:
		lprintf( "CRT Realloc: %d bytes %s(%d)"
			, size 
			, filename, lineNumber
			);
		break;
	case _HOOK_FREE:
		lprintf( "CRT Free: %p[%d](%d) %s(%d)"
			, userData
			, userData
			, size
			, filename, lineNumber
			);
		break;
	default:
		DebugBreak();
	}
	logging = 0;
	if( prior_hook )
		return prior_hook( allocType, userData, size, blockType, requestNumber, filename, lineNumber );
	return TRUE;
}

//int handle_program_memory_depletion( size_t )
//{
   // Your code
//}


PRELOAD( ShareMemToVSAllocHook )
{
	 //_CRT_ALLOC_HOOK allocHook;
	 //allocHook = 0 ;
	/* this is about useless... the free doesn't report the correct address
	 * the allocate doesn't report the block
	 * the free doesn't reprot the size
	 * there is no way to relate what is freed with what is allocated
	 */
	//prior_hook = _CrtSetAllocHook(	 allocHook ); 
	//_set_new_handler( pn );
}
#endif

};//namespace sack {
};//	namespace memory {
	
#endif

