#ifndef LINKSTUFF
#define LINKSTUFF

#include <sack_types.h>

//#include <spacetree.h>

SACK_CONTAINER_NAMESPACE

#ifdef BCC16
#ifdef _TYPELIBRARY_SOURCE
#define TYPELIB_PROC(type,name) type STDPROC _export name
#else
#define TYPELIB_PROC(type,name) extern type STDPROC name
#endif
#else
#  if defined( _TYPELIBRARY_SOURCE_STEAL )
#    define TYPELIB_PROC(type,name) type CPROC name
#  elif defined( _TYPELIBRARY_SOURCE )
#    define TYPELIB_PROC(type,name) EXPORT_METHOD type CPROC name
#  else
#    define TYPELIB_PROC(type,name) IMPORT_METHOD type CPROC name
#  endif
#endif

// These were designated for basic content containers...
// original implementation basically just used TEXT
//PDATA CreateData( _32 size );
//PDATA CreateDataFromText( CTEXTSTR pData );
//PDATA ExpandData( PDATA pData, _32 amount );
//PDATA DuplicateData( PDATA pData );

//void ReleaseData( PDATA pd );
_LINKLIST_NAMESPACE

//--------------------------------------------------------
TYPELIB_PROC( PLIST,       CreateListEx   )( DBG_VOIDPASS );
TYPELIB_PROC( PLIST,       DeleteListEx   )( PLIST *plist DBG_PASS );
TYPELIB_PROC( PLIST,       AddLinkEx      )( PLIST *pList, POINTER p DBG_PASS );
TYPELIB_PROC( PLIST,       SetLinkEx      )( PLIST *pList, INDEX idx, POINTER p DBG_PASS );
TYPELIB_PROC( POINTER,     GetLink        )( PLIST *pList, INDEX idx );
TYPELIB_PROC( POINTER*,    GetLinkAddress )( PLIST *pList, INDEX idx );
TYPELIB_PROC( INDEX,       FindLink       )( PLIST *pList, POINTER value );
TYPELIB_PROC( LOGICAL,     DeleteLink     )( PLIST *pList, POINTER value );
TYPELIB_PROC( void,        EmptyList      )( PLIST *pList );

#ifdef __cplusplus
typedef class iList
{
	PLIST list;
	INDEX idx;
public:
	inline iList() { list = CreateListEx( DBG_VOIDSRC ); }
	inline ~iList() { DeleteListEx( &list DBG_SRC ); }
	inline iList &operator+=( POINTER &p ){ AddLinkEx( &list, p DBG_SRC ); return *this; }
	inline void add( POINTER p ) { AddLinkEx( &list, p DBG_SRC ); }
	inline POINTER first( void ) { POINTER p; for( idx = 0, p = NULL;list && (idx < list->Cnt) && !( p = GetLink( &list, idx ) ); idx++ ); return p; }
	inline POINTER next( void ) { POINTER p; for( idx++;list && !( p = GetLink( &list, idx ) ) && idx < list->Cnt; idx++ ); return p; }
	inline POINTER get(INDEX idx) { return GetLink( &list, idx ); }
} *piList;
#endif

// address of the thing...
typedef PTRSZVAL (CPROC *ForProc)( PTRSZVAL user, INDEX idx, POINTER *item );
// if the callback function returns non 0 - then the looping is aborted,
// and the value is returned... the user value is passed to the callback.
TYPELIB_PROC( PTRSZVAL,    ForAllLinks    )( PLIST *pList, ForProc func, PTRSZVAL user ); 

#define LIST_FORALL( l, i, t, v )  if(((v)=(t)NULL),(l))            \
                                            for( ((i)=0); ((i) < ((l)->Cnt))? \
                                        (((v)=(t)(PTRSZVAL)(l)->pNode[i]),1):(((v)=(t)NULL),0); (i)++ )  if( v )
#define LIST_NEXTALL( l, i, t, v )  if(l)            \
    for( ++(i),((v)=(t)NULL); ((i) < ((l)->Cnt))? \
    (((v)=(t)(l)->pNode[i]),1):(((v)=(t)NULL),0); (i)++ )  if( v )
// removing this will cause some code to be uncomfortably modified...
// but as with all bad things - they must be removed.
//#define LIST_ENDFORALL()

#define CreateList()       ( CreateListEx( DBG_VOIDSRC ) )
#define DeleteList(p)      ( DeleteListEx( (p) DBG_SRC ) )
#define AddLink(p,v)       ( AddLinkEx( (p),((POINTER)(v)) DBG_SRC ) )
#define SetLink(p,i,v)     ( SetLinkEx( (p),(i),((POINTER)(v)) DBG_SRC ) )

#ifdef __cplusplus
	};//		namespace list;
#endif
//--------------------------------------------------------

TYPELIB_PROC( PDATALIST, CreateDataListEx )( PTRSZVAL nSize DBG_PASS );
TYPELIB_PROC( void,      DeleteDataListEx )( PDATALIST *ppdl DBG_PASS );
TYPELIB_PROC( POINTER,   SetDataItemEx )( PDATALIST *ppdl, INDEX idx, POINTER data DBG_PASS );
#define AddDataItem(list,data) (((list)&&(*(list)))?SetDataItemEx((list),(*list)->Cnt,data DBG_SRC ):NULL)
TYPELIB_PROC( POINTER,   SetDataItemEx )( PDATALIST *ppdl, INDEX idx, POINTER data DBG_PASS );
TYPELIB_PROC( POINTER,   GetDataItem )( PDATALIST *ppdl, INDEX idx );
TYPELIB_PROC( void,      DeleteDataItem )( PDATALIST *ppdl, INDEX idx );
TYPELIB_PROC( void,      EmptyDataList )( PDATALIST *ppdl );

#define DATA_FORALL( l, i, t, v )  if(((v)=(t)NULL),(l)&&((l)->Cnt != INVALID_INDEX))   \
	for( ((i)=0);                         \
	(((i) < (l)->Cnt)                                    \
         ?(((v)=(t)((l)->data + (PTRSZVAL)(((l)->Size) * (i)))),1)   \
	      :(((v)=(t)NULL),0))&&(v); (i)++ )
/*
#define DATA_FORALL( l, i, t, v )  if(((v)=(t)NULL),(l)&&((l)->Cnt != INVALID_INDEX))   \
	for( ((i)=0);                         \
	((i) < (l)->Cnt)                                    \
         ?(((v)=(t)((l)->data + (PTRSZVAL)(((l)->Size) * (i)))),1)   \
	      :(((v)=(t)NULL),0)); (i)++ )
*/
#define DATA_NEXTALL( l, i, t, v )  if(((v)=(t)NULL),(l))   \
	for( ((i)++);                         \
	((i) < (l)->Cnt)                                    \
         ?((v)=(t)((l)->data + (((l)->Size) * (i))))   \
	      :(((v)=(t)NULL),0); (i)++ )

#define CreateDataList(sz) ( CreateDataListEx( (sz) DBG_SRC ) )
#define DeleteDataList(p)  ( DeleteDataListEx( (p) DBG_SRC ) )
#define SetDataItem(p,i,v) ( SetDataItemEx( (p),(i),(v) DBG_SRC ) )

//--------------------------------------------------------

#ifdef __cplusplus
		namespace link_stack {
#endif
TYPELIB_PROC( PLINKSTACK,  CreateLinkStackEx)( DBG_VOIDPASS );
         // creates a link stack with maximum entries - any extra entries are pushed off the bottom into NULL
TYPELIB_PROC( PLINKSTACK,     CreateLinkStackLimitedEx        )( int max_entries  DBG_PASS );
#define CreateLinkStackLimited(n) CreateLinkStackLimitedEx(n DBG_SRC)
TYPELIB_PROC( void,        DeleteLinkStackEx)( PLINKSTACK *pls DBG_PASS);
TYPELIB_PROC( PLINKSTACK,  PushLinkEx       )( PLINKSTACK *pls, POINTER p DBG_PASS);
TYPELIB_PROC( POINTER,     PopLink          )( PLINKSTACK *pls );
TYPELIB_PROC( POINTER,     PeekLink         )( PLINKSTACK *pls );
// thought about adding these, but decided on creating a limited stack instead.
//TYPELIB_PROC( POINTER,     StackLength      )( PLINKSTACK *pls );
//TYPELIB_PROC( POINTER,     PopLinkEx        )( PLINKSTACK *pls, int position );

#define CreateLinkStack()  CreateLinkStackEx( DBG_VOIDSRC ) 
#define DeleteLinkStack(p) DeleteLinkStackEx((p) DBG_SRC)
#define PushLink(p, v)     PushLinkEx((p),(v) DBG_SRC)
#ifdef __cplusplus
		};//		namespace link_stack {
#endif

//--------------------------------------------------------
#ifdef __cplusplus
		namespace data_stack {
#endif

TYPELIB_PROC( PDATASTACK,  CreateDataStackEx)( INDEX size DBG_PASS ); // sizeof data elements...
TYPELIB_PROC( void,        DeleteDataStackEx)( PDATASTACK *pds DBG_PASS);
TYPELIB_PROC( PDATASTACK,  PushDataEx     )( PDATASTACK *pds, POINTER pdata DBG_PASS );
TYPELIB_PROC( POINTER,     PopData        )( PDATASTACK *pds );
TYPELIB_PROC( void,        EmptyDataStack )( PDATASTACK *pds );
TYPELIB_PROC( POINTER,     PeekData       )( PDATASTACK *pds ); // keeps data on stack (can be used)
// Incrementing Item moves progressivly down the stack
// final(invalid) stack, and/or empty stack will return NULL;
TYPELIB_PROC( POINTER,     PeekDataEx     )( PDATASTACK *pds, INDEX Item ); // keeps data on stack (can be used)
                                          
#define CreateDataStack(size) CreateDataStackEx( size DBG_SRC )
#define DeleteDataStack(p) DeleteDataStackEx((p) DBG_SRC)
#define PushData(pds,p) PushDataEx(pds,p DBG_SRC )
#ifdef __cplusplus
		} //		namespace data_stack {
#endif

//--------------------------------------------------------
#ifdef __cplusplus
		namespace queue {
#endif

TYPELIB_PROC( PLINKQUEUE,  CreateLinkQueueEx)( DBG_VOIDPASS );
TYPELIB_PROC( void,        DeleteLinkQueueEx)( PLINKQUEUE *pplq DBG_PASS );
TYPELIB_PROC( PLINKQUEUE,  EnqueLinkEx      )( PLINKQUEUE *pplq, POINTER link DBG_PASS );
TYPELIB_PROC( PLINKQUEUE,  PrequeLinkEx      )( PLINKQUEUE *pplq, POINTER link DBG_PASS );
TYPELIB_PROC( POINTER,     DequeLink        )( PLINKQUEUE *pplq );
TYPELIB_PROC( LOGICAL,     IsQueueEmpty     )( PLINKQUEUE *pplq );
TYPELIB_PROC( INDEX,       GetQueueLength   )( PLINKQUEUE plq );
// get a PLINKQUEUE element at index
TYPELIB_PROC( POINTER,     PeekQueueEx    )( PLINKQUEUE plq, INDEX idx );
TYPELIB_PROC( POINTER,     PeekQueue    )( PLINKQUEUE plq );


#define     CreateLinkQueue()     CreateLinkQueueEx( DBG_VOIDSRC )
#define     PrequeLink(pplq,link) PrequeLinkEx( pplq, link DBG_SRC )
#define     DeleteLinkQueue(pplq) DeleteLinkQueueEx( pplq DBG_SRC )
#define     EnqueLink(pplq, link) EnqueLinkEx( pplq, link DBG_SRC )
#ifdef __cplusplus
		}//		namespace queue {
#endif

//--------------------------------------------------------

#ifdef __cplusplus
		namespace data_queue {
#endif
TYPELIB_PROC( PDATAQUEUE,  CreateDataQueueEx)( INDEX size DBG_PASS );
TYPELIB_PROC( PDATAQUEUE,  CreateLargeDataQueueEx)( INDEX size, INDEX entries, INDEX expand DBG_PASS );
TYPELIB_PROC( void,        DeleteDataQueueEx)( PDATAQUEUE *pplq DBG_PASS );
TYPELIB_PROC( PDATAQUEUE,  EnqueDataEx      )( PDATAQUEUE *pplq, POINTER Data DBG_PASS );
TYPELIB_PROC( PDATAQUEUE,  PrequeDataEx      )( PDATAQUEUE *pplq, POINTER Data DBG_PASS );
TYPELIB_PROC( LOGICAL,     DequeData        )( PDATAQUEUE *pplq, POINTER Data );
TYPELIB_PROC( LOGICAL,     IsDataQueueEmpty )( PDATAQUEUE *pplq );
TYPELIB_PROC( void,        EmptyDataQueue )( PDATAQUEUE *pplq );
/*
 * get a PDATAQUEUE element at index
 * result buffer is a pointer to the type of structure expected to be
 * stored within this.  The buffer result is a copy of the data stored in the queue.
 * This enforces that data stored in the list is immutable.
 * Also on the basic DequeData function, after resulting, if the pointer to the
 * data within the queue were returned, it could become invalid immediatly after
 * returning by having another enque happen which overwrites that position in the buffer.
 * One could, in theory, set a flag in the queue that a deque was done, and not update the
 * bottom until that flag is encountered while within DequeData again...
 * the pointer to the data in the queue may also not be returned because the queue may be
 * reallocated and moved.
 */
TYPELIB_PROC( LOGICAL, PeekDataQueueEx    )( PDATAQUEUE *pplq, POINTER ResultBuffer, INDEX idx );
#define PeekDataQueueEx( q, type, result, idx ) PeekDataQueueEx( q, (POINTER)result, idx )
/*
 * Result buffer is filled with the last element, and the result is true, otherwise the return
 * value is FALSE, and the data was not filled in.
 */
TYPELIB_PROC( LOGICAL, PeekDataQueue    )( PDATAQUEUE *pplq, POINTER ResultBuffer );
#define PeekDataQueue( q, type, result ) PeekDataQueue( q, (POINTER)result )

#define     CreateDataQueue(size)     CreateDataQueueEx( size DBG_SRC )
#define     CreateLargeDataQueue(size,entries)     CreateLargeDataQueueEx( size,entries, 0 DBG_SRC )
#define     DeleteDataQueue(pplq) DeleteDataQueueEx( pplq DBG_SRC )
#define     EnqueData(pplq, Data) EnqueDataEx( pplq, Data DBG_SRC )
#define     PrequeData(pplq, Data) PrequeDataEx( pplq, Data DBG_SRC )
#ifdef __cplusplus
		}//		namespace data_queue {
#endif

//---------------------------------------------------------------------------

#ifdef __cplusplus
namespace message {
#endif

typedef struct MsgDataHandle *PMSGHANDLE;
//typedef struct MsgDataQueue *PMSGQUEUE;

// messages sent - the first dword of them must be
// a message ID.
typedef void (CPROC *MsgQueueReadCallback)( PTRSZVAL psv, CPOINTER p, _32 sz );
TYPELIB_PROC( PMSGHANDLE, CreateMsgQueue )( CTEXTSTR name, _32 size
                                                      , MsgQueueReadCallback Read
                                                      , PTRSZVAL psvRead );
TYPELIB_PROC( PMSGHANDLE, OpenMsgQueue )( CTEXTSTR name
													 , MsgQueueReadCallback Read
													 , PTRSZVAL psvRead );
TYPELIB_PROC( void, DeleteMsgQueue )( PMSGHANDLE **ppmh );

#define MSGQUE_NOWAIT 0x0001 // if enque, fail send, return immediate on fail
                             // if deque, fail no msg ready to get...
#define MSGQUE_EXCEPT 0x0002 // read any msg BUT MsgID
#define MSGQUE_WAIT_ID 0x0004 // enque this message... it is a task ID which is waiting.

#define MSGQUE_ERROR_NOMSG 1
#define MSGQUE_ERROR_E2BIG 2
#define MSGQUE_ERROR_EABORT 5
// result is the size of the message, or 0 if no message.
// -1 if some other error?
TYPELIB_PROC( int, DequeMsgEx )( PMSGHANDLE pmh, _32 *MsgID, POINTER buffer, _32 msgsize, _32 options DBG_PASS );
#define DequeMsg(q,b,s,i,o) DequeMsgEx(q,b,s,i,o DBG_SRC )
TYPELIB_PROC( int, PeekMsgEx )( PMSGHANDLE pmh, _32 MsgID, POINTER buffer, _32 msgsize, _32 options DBG_PASS );
#define PeekMsg(q,b,s,i,o) PeekMsgEx(q,b,s,i,o DBG_SRC )
TYPELIB_PROC( int, EnqueMsgEx )( PMSGHANDLE pmh, POINTER buffer, _32 msgsize, _32 options DBG_PASS );
#define EnqueMsg(q,b,s,o) EnqueMsgEx(q,b,s,o DBG_SRC )
TYPELIB_PROC( int, IsMsgQueueEmpty )( PMSGHANDLE pmh );

#ifdef __cplusplus
}; //namespace message {
namespace sets {
#endif

//---------------------------------------------------------------------------
// Set type
//   Usage:
//      typedef struct name_tag { } <name>;
//      #define MAX<name>SPERSET
//      DeclareSet( <name> );
//    Should alias GetFromset, DeleteFromSet, CountUsedInSet, GetLinearSetArray
//       etc so that the type name is reflected there
//       another good place where #define defining defines is good.
//---------------------------------------------------------------------------

#define UNIT_USED_IDX(n)   ((n) >> 5)
#define UNIT_USED_MASK(n)  (1 << ((n) &0x1f))

#define SetUsed(set,n)   ((((set)->bUsed[UNIT_USED_IDX(n)]) |= UNIT_USED_MASK(n)), (++(set)->nUsed) )
#define ClearUsed(set,n) ((((set)->bUsed[UNIT_USED_IDX(n)]) &= ~UNIT_USED_MASK(n)), (--(set)->nUsed) )
#define AllUsed(set,n)   (((set)->bUsed[UNIT_USED_IDX(n)]) == 0xFFFFFFFF )
#define IsUsed(set,n)    (((set)->bUsed[UNIT_USED_IDX(n)]) & UNIT_USED_MASK(n) )


#ifdef __cplusplus
#define CPP_(n)
//#define CPP_(n) n
#else
#define CPP_(n)
#endif

// requires a symbol of MAX<insert name>SPERSET to declare max size...
#if 1 //ndef __cplusplus
#define SizeOfSet(size,count)  (sizeof(POINTER)*2+sizeof(int)+sizeof( _32[((count)+31)/32] ) + ((size)*(count)))
#define DeclareSet( name )  typedef struct name##set_tag {   \
	struct name##set_tag *next, *prior;                      \
	int nUsed;                                               \
	int nBias;                                               \
	_32 bUsed[(MAX##name##SPERSET + 31 ) / 32];              \
	name p[MAX##name##SPERSET];                           \
	CPP_(int forall(PTRSZVAL(CPROC*f)(void*,PTRSZVAL),PTRSZVAL psv) {if( this ) return _ForAllInSet( (struct genericset_tag*)this, sizeof(name), MAX##name##SPERSET, f, psv ); else return 0; }) \
	CPP_(name##set_tag() { next = NULL;prior = NULL;nUsed = 0; nBias = 0; MemSet( bUsed, 0, sizeof( bUsed ) ); MemSet( p, 0, sizeof( p ) );} )\
	} name##SET, *P##name##SET

#define DeclareClassSet( name ) typedef struct name##set_tag {   \
	struct name##set_tag *next, *prior;                      \
	int nUsed;                                               \
	int nBias;                                               \
	_32 bUsed[(MAX##name##SPERSET + 31 ) / 32];              \
	class name p[MAX##name##SPERSET];                        \
	CPP_(int forall(PTRSZVAL(CPROC*)(void*f,PTRSZVAL),PTRSZVAL psv) {if( this ) return _ForAllInSet( (struct genericset_tag*)this, sizeof(class name), MAX##name##SPERSET, f, psv ); else return 0; }) \
	} name##SET, *P##name##SET
#endif

typedef struct genericset_tag {
	// wow might be nice to have some flags...
	// first flag - bSetSet - meaning that this is a set of sets of
	// the type specified...
	struct genericset_tag *next;
	struct genericset_tag **me;
	_32 nUsed;
	_32 nBias; // hmm if I change this here? we're hozed... so.. we'll do it anyhow :) evil - recompile please
	_32 bUsed[1]; // after this p * unit must be computed
} GENERICSET, *PGENERICSET;

TYPELIB_PROC( POINTER ,GetFromSetEx)( GENERICSET **pSet, int setsize, int unitsize, int maxcnt DBG_PASS );
#define GetFromSeta(ps, ss, us, max) GetFromSetEx( (ps), (ss), (us), (max) DBG_SRC )
#define GetFromSet( name, pset ) (P##name)GetFromSeta( (GENERICSET**)(pset), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET )

TYPELIB_PROC( PGENERICSET ,GetFromSetPoolEx)( GENERICSET **pSetSet
													 , int setsetsize, int setunitsize, int setmaxcnt
													 , GENERICSET **pSet
													 , int setsize, int unitsize, int maxcnt DBG_PASS );
#define GetFromSetPoola(pl, sss, sus, smax, ps, ss, us, max) GetFromSetPoolEx( (pl), (sss), (sus), (smax), (ps), (ss), (us), (max) DBG_SRC )
#define GetFromSetPool( name, pool, pset ) (P##name)GetFromSetPoola( (GENERICSET**)(pool)    \
	, sizeof( name##SETSET ), sizeof( name##SET ), MAX##name##SETSPERSET\
	, (GENERICSET**)(pset), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET )

TYPELIB_PROC( POINTER ,GetSetMemberEx)( GENERICSET **pSet, INDEX nMember, int setsize, int unitsize, int maxcnt DBG_PASS );
#define GetSetMembera(ps, member, ss, us, max) (GetSetMemberEx( (ps), (member), (ss), (us), (max) DBG_SRC ))
#define GetSetMember( name, pset, member ) ((P##name)GetSetMembera( (GENERICSET**)(pset), (member), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET ))

TYPELIB_PROC( POINTER ,GetUsedSetMemberEx)( GENERICSET **pSet, INDEX nMember, int setsize, int unitsize, int maxcnt DBG_PASS );
#define GetUsedSetMembera(ps, member, ss, us, max) (GetUsedSetMemberEx( (ps), (member), (ss), (us), (max) DBG_SRC ))
#define GetUsedSetMember( name, pset, member ) ((P##name)GetUsedSetMembera( (GENERICSET**)(pset), (member), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET ))

TYPELIB_PROC( INDEX, GetMemberIndex)(GENERICSET **set, POINTER unit, int unitsize, int max );
#define GetMemberIndex(name,set,member) GetMemberIndex( (GENERICSET**)set, member, sizeof( name ), MAX##name##SPERSET )

#define GetIndexFromSet( name, pset ) GetMemberIndex( name, pset, GetFromSet( name, pset ) )

TYPELIB_PROC( void, DeleteFromSetExx)( GENERICSET *set, POINTER unit, int unitsize, int max DBG_PASS );
#define DeleteFromSetEx( name, set, member, xx ) DeleteFromSetExx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET DBG_SRC )
#define DeleteFromSet( name, set, member ) DeleteFromSetExx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET DBG_SRC )

TYPELIB_PROC( void, DeleteSetMemberEx)( GENERICSET *set, INDEX iMember, PTRSZVAL unitsize, INDEX max );
#define DeleteSetMember( name, set, member ) DeleteSetMemberEx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET )

TYPELIB_PROC( int, MemberValidInSetEx)( GENERICSET *set, POINTER unit, int unitsize, int max );
#define MemberValidInSet( name, set, member ) MemberValidInSetEx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET )

TYPELIB_PROC( int, CountUsedInSet)( GENERICSET *set, int max );
#define CountUsedInSet( name, set ) CountUsedInSet( (GENERICSET*)set, MAX##name##SPERSET )

TYPELIB_PROC( POINTER *,GetLinearSetArray)( GENERICSET *pSet, int *pCount, int unitsize, int max );
#define GetLinearSetArray( name, set, pCount ) GetLinearSetArray( (GENERICSET*)set, pCount, sizeof( name ), MAX##name##SPERSET )

TYPELIB_PROC( int, FindInArray)( POINTER *pArray, int nArraySize, POINTER unit );

TYPELIB_PROC( void, DeleteSet)( GENERICSET **ppSet );
#define DeleteSetEx( name, ppset ) { name##SET **delete_me = ppset; DeleteSet( (GENERICSET**)delete_me ); }

typedef PTRSZVAL (CPROC *FAISCallback)(void*,PTRSZVAL);
TYPELIB_PROC( PTRSZVAL, _ForAllInSet)( GENERICSET *pSet, int unitsize, int max, FAISCallback f, PTRSZVAL psv );

typedef PTRSZVAL (CPROC *FESMCallback)(INDEX,PTRSZVAL);
TYPELIB_PROC( PTRSZVAL, ForEachSetMember )( GENERICSET *pSet, int unitsize, int max, FESMCallback f, PTRSZVAL psv );


#if 0 //def __cplusplus

#define DeclareSet(name)                                \
	struct name##set_tag {               \
	_32 set_size;                             \
	_32 element_size;                         \
	_32 element_cnt;                          \
	PGENERICSET pool;                        \
	name##set_tag() {                        \
	element_size = sizeof( name );             \
	element_cnt = MAX##name##SPERSET;          \
	set_size = (element_size * element_cnt )+ ((((element_cnt + 31 )/ 32 )- 1 ) * 4) + sizeof( GENERICSET ); \
	pool = NULL;                               \
	}    \
	~name##set_tag() { DeleteSet( &pool ); } \
	P##name grab() { return (P##name)GetFromSetEx( &pool, set_size, element_size, element_cnt DBG_SRC ); } \
	P##name grab(INDEX member) { return (P##name)GetSetMemberEx( &pool, member, set_size, element_size, element_cnt DBG_SRC ); } \
	P##name get(INDEX member) { return (this)?(P##name)GetUsedSetMemberEx( &pool, member, set_size, element_size, element_cnt DBG_SRC ):(NULL); } \
	void drop( P##name member ) { DeleteFromSetEx( pool, (POINTER)member, element_size, element_cnt ); } \
	int valid( P##name member ) { return MemberValidInSetEx( pool, (POINTER)member, element_size, element_cnt ); } \
	PTRSZVAL forall( FAISCallback f, PTRSZVAL psv ) { if( this ) return _ForAllInSet( pool, element_size, element_cnt, f, psv ); else return 0; } \
	};       \
	typedef struct name##set_tag *P##name##SET, name##SET;        

#define ForAllInSet(name, pset,f,psv) _ForAllInSet( (GENERICSET*)(pset), sizeof( name ), MAX##name##SPERSET, (f), (psv) )
#else

#define ForAllInSet(name, pset,f,psv) _ForAllInSet( (GENERICSET*)(pset), sizeof( name ), MAX##name##SPERSET, (f), (psv) )
#define ForEachSetMember(name,pset,f,psv) ForEachSetMember( (GENERICSET*)(pset),sizeof(name),MAX##name##SPERSET, (f), (psv) )
#endif


//---------------------------------------------------------------------------

#ifdef __cplusplus
}; // namespace sets

namespace text {
#endif

// this defines more esoteric formatting notions...
// these data blocks will be zero sized, and ahve the TF_FORMATEX 
// bit set.

//#define DEFAULT_COLOR 0xF7
//#define PRIOR_COLOR 0xF6 // this does not change the color....

// these enumerated ops put in the foreground field of a format
// with a flag of TF_FORMATEX will cause the specified operation
// to be carried out on a display (not files) or generated into
// the appropriate sequence (ansi out encode)
// -- correction
//  this is encoded into its own field for the format
// size, due to machine optimization, 16 bits were free
// this was expanded and used for all information
// a segment may contain extended op, color, attributes,
// and text, everything short of a font for it...
//  - not sure how to address that issue... there's
// certainly modifications to current font... italic for
// instance..
enum {
	FORMAT_OP_CLEAR_END_OF_LINE = 1
	  , FORMAT_OP_CLEAR_START_OF_LINE
	  , FORMAT_OP_CLEAR_LINE
	  , FORMAT_OP_CLEAR_END_OF_PAGE
	  , FORMAT_OP_CLEAR_START_OF_PAGE
	  , FORMAT_OP_CLEAR_PAGE // set cursor home
	  //, FORMAT_OP_POSITION_DELTA // position information is a delta cursor
	  , FORMAT_OP_CONCEAL // sets option to not show text at all until next color.
	  , FORMAT_OP_DELETE_CHARS // background is how many to delete.
	  , FORMAT_OP_SET_SCROLL_REGION // format.x, y are start/end of region -1,-1 clears.
	  , FORMAT_OP_GET_CURSOR // this works as a transaction...
	  , FORMAT_OP_SET_CURSOR // responce to getcursor...

	  , FORMAT_OP_PAGE_BREAK // clear page, home page... result in page break...
	  , FORMAT_OP_PARAGRAPH_BREAK // break between paragraphs - kinda same as lines...
	  // since lines are as long as possible...
};

//typedef struct text_color_tag { _32 color: 8; } TEXTCOLOR;

// this was a 32 bit structure, but 8 fore, 8 back
// 8 x, 8 y failed for positioning...

// extended position, added more information
// reduced color, 16 colors is really all that there
// are... 4 bits... added bits for extended formatting
// like blink, bold, wide, high
// foreground/background  values will be
// sufficient... they retain full informaiton
//
typedef struct format_info_tag
{
	struct {
		// extended operation from enumeration above...
		// might shrink if more attributes are desired...
		// if many more are needed, one might consider
      // adding FONT!
		_32 prior_foreground : 1;
		_32 prior_background : 1;
		_32 default_foreground : 1;
		_32 default_background : 1;
		_32 foreground : 4;
		_32 background : 4;
		_32 blink : 1;
		_32 reverse : 1;
		// usually highly is bolder, perhaps it's
      // a highlighter effect and changes the background
		_32 highlight : 1;
		// this is double height modifications to the font...
		_32 tall : 1; 
      // this is thicker characters...
		_32 bold : 1;
      // draw a line under the text...
		_32 underline : 1;
		// strike through - if able, draw a line right
		// through the middle of the text... maybe
		// it's a wiggly scribble line?  maybe that
      // could be extended again?
		_32 strike : 1;
		_32 wide : 1; // text is drawn wide (printer kinda font?)
		_32 italic : 1; // this is pretty common......
		// --
		// these flags are free, but since we already have text segments
		// and I'm bringing in consoles, perhaps we should consider using
		// this to describe captions, but provide the api layer for CTEXTSTR 
		// --
		// position data remains constant.
		// text is mounted at the top/left of the
		// first character... (unless center, then
		// the position specifies the middle of the text
		// draw vertical instead of horizontal
		_32 bVertical:1;
		// draw opposite/upside down from normal
		// vertical/down, right/left upside down if not centered
		// if centered, the text pivots around position.
		_32 bInvert:1;
		_32 bAlign:2;  // 0 = default alignment 1 = left, 2 = center 3 = right
		// 0 is not set, the flag set in the lower 32 bit flags
		// is not needed any longer.... anything non zero
      // is that operation to apply.
		_32 format_op : 7;

	} flags;
	// if x,y are valid segment will have TF_POSFORMAT set...
	union {
		struct {
			S_16 x;
			S_16 y; // need more than 8 bits for displays even now...
		} coords;
		struct {
         _16 tabs;   // tabs preceed spaces....
			_16 spaces; // not sure what else to put with this...
		};
	} position;
} FORMAT, *PFORMAT;


#define IGNORE_CURSOR_POS -16384 // special coordinate which is NO coordinate

#define TF_STATIC    0x00000001   // declared in program data.... do NOT release

#define TF_FORMATPOS (TF_FORMATABS|TF_FORMATREL|TF_FORMATEX)
// applications may use these flags to group expressions
// will affect the BuildLine but is not generated by library.
#define TF_QUOTE     0x00000002   // double quoted string segment " "
#define TF_SQUOTE    0x00000004   // single quoted string ' '
#define TF_BRACKET   0x00000008   // bracketed expression []
#define TF_BRACE     0x00000010   // braced expression {}
#define TF_PAREN     0x00000020   // parenthised expression ()
#define TF_TAG       0x00000040   // HTML tag like expression <>

#define TF_FORMATEX  0x00000080   // foreground is FORMAT_OP
#define TF_FORMATREL 0x00000100   // x,y position used (relative)
#define TF_INDIRECT  0x00000200   // size field extually points at PTEXT
#define TF_FORMATABS 0x00000800   // format position is x/y - else space count
#define TF_COMPLETE  0x00001000   // set during burst for last segment...
#define TF_BINARY    0x00002000   // set for non-text variable
#define TF_DEEP      0x00004000   // on release release indrect also...
#define TF_NORETURN  0x00008000   // set on first segment to send to omit lead \r\n

// these values used originally for ODBC query construction....
// these values are flags stored on the indirect following a value
// label...
#define TF_LOWER     0x00010000   // Low bound of value...
#define TF_UPPER     0x00020000   // Upper bound of a value...
#define TF_EQUAL     0x00040000   // boundry may be ON this value...

#define TF_TEMP      0x00080000   // this segment is not a permanent part (SubstToken)
#define TF_APPLICATION 0x00100000  // this is something special do not treat as text indirect.

//--------------------------------------------------------------------------

// flag combinatoin which represents actual data is present even with 0 size
// extended format operations (position, ops) are also considered data.
#define IS_DATA_FLAGS (TF_QUOTE|TF_SQUOTE|TF_BRACKET|TF_BRACE|\
                              TF_PAREN|TF_TAG|TF_FORMATEX|TF_FORMATABS|TF_FORMATREL)

#define DECLTEXTSZTYPE( name, size ) struct { \
   _32 flags; \
   struct text_segment_tag *Next, *Prior; \
   FORMAT format; \
   DECLDATA(data, size); \
} name 

#define DECLTEXTSZ( name, size ) DECLTEXTSZTYPE( name,(size) ) \
	= { TF_STATIC, NULL, NULL, {{1,1}} }

#define DEFTEXT(str) {TF_STATIC,NULL,NULL,{{1,1}},{sizeof(str)-1,str}}
#define DECLTEXT(name, str) static DECLTEXTSZTYPE( name, sizeof(str) ) = DEFTEXT(str)

typedef struct text_segment_tag
{
   _32 flags;  // then here I could overlap with pEnt .bshadow, bmacro, btext ?
	struct text_segment_tag *Next, *Prior;
	// format is 64 bits.
   // it's two 32 bit bitfields (position, expression)
	FORMAT format; // valid if TF_FORMAT is set...
   struct {
		PTRSZVAL size;// unsigned size; size is sometimes a pointer value...
                 // this means bad thing when we change platforms...
#ifdef _MSC_VER
#pragma warning (disable:4200)
#endif
	   TEXTCHAR  data[
#if !defined( __cplusplus ) || defined( __WATCOMC__ ) || defined( GCC )
		   			1
#endif
						   ]; // beginning of var data - this is created size+sizeof(VPA)
#ifdef _MSC_VER
#pragma warning (default:4200)
#endif
	} data; // must be last since var character data is included
} TEXT, *PTEXT;

//
// PTEXT DumpText( PTEXT somestring )
//    PTExT (single data segment with full description \r in text)
TYPELIB_PROC( PTEXT, DumpText) ( PTEXT text );
//SegCreateFromText( ".." );
// Burst, SegAppend, SegGrab
// segments are ment to be lines, the meaninful tag "TF_NORETURN" means it's part of the prior line.

//--------------------------------------------------------------------------

#ifndef _TYPELIBRARY_SOURCE
#if defined (_WIN32 ) && !defined( __STATIC__ )
IMPORT_METHOD PTEXT newline;
IMPORT_METHOD PTEXT blank;
#else
extern TEXT newline;
extern TEXT blank;
#endif
#endif


#define HAS_WHITESPACE(pText) ( (pText)->format.position.spaces || (pText)->format.position.tabs )

#define NEXTLINE(line)   ((PTEXT)(((PTEXT)line)?(((PTEXT)line)->Next):(NULL)))
#define PRIORLINE(line)  ((PTEXT)(((PTEXT)line)?(((PTEXT)line)->Prior):(NULL)))

#define SETPRIORLINE(line,p) ((line)?(((line)->Prior) = (PTEXT)(p)):0)
#define SETNEXTLINE(line,p)  ((line)?(((line)->Next ) = (PTEXT)(p)):0)

#define SetStart(line)     for(; line && PRIORLINE(line);line=PRIORLINE(line))
#define SetEnd(line)      for(; line && NEXTLINE(line); line=NEXTLINE(line))
// might also check to see if pseg is an indirect - setting this size would be BAD
#define SetTextSize(pseg, sz ) ((pseg)?((pseg)->data.size = (sz )):0)
TYPELIB_PROC( PTEXT, GetIndirect)(PTEXT segment );

TYPELIB_PROC( _32, GetTextFlags)( PTEXT segment );
TYPELIB_PROC( INDEX, GetTextSize)( PTEXT segment );
TYPELIB_PROC( TEXTSTR, GetText)( PTEXT segment );
// by registering for TF_APPLICTION is set on the segment
// and flags anded with the segment flags match, the
// function is called.... the result is the actual
// segment of this - since a TF_APPLICATION is also
// TF_INDIRECT - using the size to point to some application
// defined structure instead of a PTEXT structure.
TYPELIB_PROC( void, RegisterTextExtension )( _32 flags, PTEXT(CPROC*)(PTRSZVAL,POINTER), PTRSZVAL );
// similar to GetIndirect - but results in the literal pointer
// instead of the text that the application may have registered to result with.
TYPELIB_PROC( POINTER, GetApplicationPointer )( PTEXT text );
TYPELIB_PROC( void, SetApplicationPointer )( PTEXT text, POINTER p);


#define SetIndirect(Seg,Where)  ( (Seg)->data.size = ((PTRSZVAL)(Where)-(PTRSZVAL)NULL) )

// these return 1 for more(l1>l2) -1 for (l1<l2) and 0 for match.
TYPELIB_PROC( int, SameText )( PTEXT l1, PTEXT l2 );
TYPELIB_PROC( int, LikeText )( PTEXT l1, PTEXT l2 );
// these return TRUE if match else FALSE.
TYPELIB_PROC( int, TextIs ) ( PTEXT pText, CTEXTSTR text );
TYPELIB_PROC( int, TextLike ) ( PTEXT pText, CTEXTSTR text );

//#define SameText( l1, l2 )  ( strcmp( GetText(l1), GetText(l2) ) )
#define textmin(a,b) ( (((a)>0)&&((b)>0))?(((a)<(b))?(a):(b)):(((a)>0)?(a):((b)>0)?(b):0) )
#ifdef __LINUX__
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif
//#define LikeText( l1, l2 )  ( strnicmp( GetText(l1), GetText(l2), textmin( GetTextSize(l1),
//                                                                        GetTextSize(l2) ) ) )
//#define TextIs(text,string) ( !stricmp( GetText(text), string ) )
//#define TextLike(text,string) ( !stricmp( GetText(text), string ) )

TYPELIB_PROC( PTEXT, SegCreateEx)( S_32 nSize DBG_PASS );
#define SegCreate(s) SegCreateEx(s DBG_SRC)
TYPELIB_PROC( PTEXT, SegCreateFromTextEx)( CTEXTSTR text DBG_PASS );
#define SegCreateFromText(t) SegCreateFromTextEx(t DBG_SRC)
TYPELIB_PROC( PTEXT, SegCreateIndirectEx)( PTEXT pText DBG_PASS );
#define SegCreateIndirect(t) SegCreateIndirectEx(t DBG_SRC)

TYPELIB_PROC( PTEXT, SegDuplicateEx)( PTEXT pText DBG_PASS);
#define SegDuplicate(pt) SegDuplicateEx( pt DBG_SRC )
TYPELIB_PROC( PTEXT, LineDuplicateEx)( PTEXT pText DBG_PASS );
#define LineDuplicate(pt) LineDuplicateEx(pt DBG_SRC )
TYPELIB_PROC( PTEXT, TextDuplicateEx)( PTEXT pText, int bSingle DBG_PASS );
#define TextDuplicate(pt,s) TextDuplicateEx(pt,s DBG_SRC )

TYPELIB_PROC( PTEXT, SegCreateFromIntEx)( int value DBG_PASS );
#define SegCreateFromInt(v) SegCreateFromIntEx( v DBG_SRC )
TYPELIB_PROC( PTEXT, SegCreateFrom_64Ex)( S_64 value DBG_PASS );
#define SegCreateFrom_64(v) SegCreateFrom_64Ex( v DBG_SRC )
TYPELIB_PROC( PTEXT, SegCreateFromFloatEx)( float value DBG_PASS );
#define SegCreateFromFloat(v) SegCreateFromFloatEx( v DBG_SRC )

                   
TYPELIB_PROC( PTEXT, SegAppend   )( PTEXT source, PTEXT other );
TYPELIB_PROC( PTEXT, SegInsert   )( PTEXT what, PTEXT before );

TYPELIB_PROC( PTEXT, SegExpandEx )(PTEXT source, int nSize DBG_PASS );  // add last node... blank space.
#define SegExpand(s,n) SegExpandEx( s,n DBG_SRC );

TYPELIB_PROC( void,  LineReleaseEx )(PTEXT line DBG_PASS );
#define LineRelease(l) LineReleaseEx(l DBG_SRC )

TYPELIB_PROC( void, SegReleaseEx)( PTEXT seg DBG_PASS );
#define SegRelease(l) SegReleaseEx(l DBG_SRC )

TYPELIB_PROC( PTEXT, SegConcatEx   )(PTEXT output,PTEXT input,S_32 offset,S_32 length DBG_PASS);
#define SegConcat(out,in,ofs,len) SegConcatEx(out,in,ofs,len DBG_SRC)

TYPELIB_PROC( PTEXT, SegUnlink   )(PTEXT segment);
TYPELIB_PROC( PTEXT, SegBreak    )(PTEXT segment);
TYPELIB_PROC( PTEXT, SegDelete   )(PTEXT segment); // removes seg from list, deletes seg.
TYPELIB_PROC( PTEXT, SegGrab     )(PTEXT segment); // removes seg from list, returns seg.
TYPELIB_PROC( PTEXT, SegSubst    )( PTEXT _this, PTEXT that );

TYPELIB_PROC( PTEXT, SegSplitEx)( PTEXT *pLine, int nPos DBG_PASS);
#define SegSplit(line,pos) SegSplitEx( line, pos DBG_SRC )

TYPELIB_PROC( PTEXT, FlattenLine )( PTEXT pLine );
TYPELIB_PROC( S_64, IntCreateFromSeg)( PTEXT pText );
TYPELIB_PROC( S_64, IntCreateFromText)( CTEXTSTR p );

TYPELIB_PROC( double, FloatCreateFromSeg)( PTEXT pText );
TYPELIB_PROC( double, FloatCreateFromText)( CTEXTSTR p, CTEXTSTR *pp );

//
// IsSegAnyNumber returns 0 if no, 1 if is int, 2 if is float
//   if pfNumber or piNumber are available then the text pointer
//   will be updated to the next segment after what was used to resolve
//   the number.
//   bUseAllSegs is for testing pTexts which are indirect, such that
//      only all segments within the indirect segment will result valid.
//   pfNumber and piNumber may be passed as NULL, and the function can still
// be used to determine ifnumber
//   the number resulting in the values pointed to will be filled in
//    with (*pfNumber)=FltCreateFromSeg(p) (or Int as appropriate)
//
//#define IsNumber(p) IsSegAnyNumberEx( &(p), NULL, NULL, NULL, 0 )
#define IsIntNumber(p, pint) IsSegAnyNumberEx( &(p), NULL, pint, NULL, 0 )
#define IsFltNumber(p, pflt) IsSegAnyNumberEx( &(p), pflt, NULL, NULL, 0 )
TYPELIB_PROC( int, IsSegAnyNumberEx )( PTEXT *ppText, double *pfNumber, S_64 *piNumber, int *pbIsInt, int bUseAllSegs );
#define IsSegAnyNumber(pptext, pfNum, piNum, pbIsInt) IsSegAnyNumberEx( pptext, pfNum, piNum, pbIsInt, 0 )



TYPELIB_PROC( _32, GetSegmentSpaceEx )( PTEXT segment, int position, int nTabs, INDEX *tabs);
TYPELIB_PROC( _32, GetSegmentSpace )( PTEXT segment, int position, int nTabSize );
TYPELIB_PROC( _32, GetSegmentLengthEx )( PTEXT segment, int position, int nTabs, INDEX *tabs );
TYPELIB_PROC( _32, GetSegmentLength )( PTEXT segment, int position, int nTabSize );

TYPELIB_PROC( _32, LineLengthExEx)( PTEXT pt, _32 bSingle, INDEX nTabsize, PTEXT pEOL );
TYPELIB_PROC( _32, LineLengthExx)( PTEXT pt, _32 bSingle,PTEXT pEOL );
#define LineLengthExx(pt,single,eol) LineLengthExEx( pt,single,8,eol)
#define LineLengthEx(pt,single) LineLengthExx( pt,single,NULL)
#define LineLength(pt) LineLengthEx( pt, FALSE )
TYPELIB_PROC( PTEXT, BuildLineExEx)( PTEXT pt, int bSingle, INDEX nTabsize, PTEXT pEOL DBG_PASS );
TYPELIB_PROC( PTEXT, BuildLineExx)( PTEXT pt, int bSingle, PTEXT pEOL DBG_PASS );
//#define BuildLineEx(b,pt,single) BuildLineEx(b,pt,single DBG_SRC )
#define BuildLineExx(from,single,eol) BuildLineExEx( from,single,8,NULL DBG_SRC )
#define BuildLineEx(from,single) BuildLineExEx( from,single,8,NULL DBG_SRC )
#define BuildLine(from) BuildLineExEx( from, FALSE,8,NULL DBG_SRC )

//
// text parse - more generic flavor of burst.
//
//static CTEXTSTR normal_punctuation=WIDE("\'\"\\({[<>]}):@%/,;!?=*&$^~#`");
// filter_to_space WIDE(" \t")
TYPELIB_PROC( PTEXT, TextParse )( PTEXT input, CTEXTSTR punctuation, CTEXTSTR filter_tospace, int bTabs, int bSpaces  DBG_PASS );
//static CTEXTSTR normal_punctuation=WIDE("\'\"\\({[<>]}):@%/,;!?=*&$^~#`");

TYPELIB_PROC( PTEXT, burstEx)( PTEXT input DBG_PASS);
#define burst( input ) burstEx( (input) DBG_SRC )

TYPELIB_PROC( int, CompareStrings)( PTEXT pt1, int single1
                            , PTEXT pt2, int single2
                            , int bExact );


TYPELIB_PROC( PTEXT, FlattenLine )( PTEXT pLine );

#define FORALLTEXT(start,var)  for(var=start;var; var=NEXTLINE(var))

TYPELIB_PROC( char *, WcharConvert )( const wchar_t *wch );

//--------------------------------------------------------------------------

typedef struct vartext_tag *PVARTEXT;

TYPELIB_PROC( PVARTEXT, VarTextCreateExEx )( _32 initial, _32 expand DBG_PASS );
#define VarTextCreateExx(i,e) VarTextCreateExEx(i,e DBG_SRC );
TYPELIB_PROC( PVARTEXT, VarTextCreateEx )( DBG_VOIDPASS );
#define VarTextCreate() VarTextCreateEx( DBG_VOIDSRC )
TYPELIB_PROC( void, VarTextDestroyEx )( PVARTEXT* DBG_PASS );
#define VarTextDestroy(pvt) VarTextDestroyEx( pvt DBG_SRC )

TYPELIB_PROC( void, VarTextInitEx)( PVARTEXT pvt DBG_PASS);
#define VarTextInit(pvt) VarTextInitEx( (pvt) DBG_SRC )
TYPELIB_PROC( void, VarTextEmptyEx)( PVARTEXT pvt DBG_PASS);
#define VarTextEmpty(pvt) VarTextEmptyEx( (pvt) DBG_SRC )
TYPELIB_PROC( void, VarTextAddCharacterEx)( PVARTEXT pvt, TEXTCHAR c DBG_PASS );
#define VarTextAddCharacter(pvt,c) VarTextAddCharacterEx( (pvt),(c) DBG_SRC )
// returns true if any data was added...
TYPELIB_PROC( int, VarTextEndEx)( PVARTEXT pvt DBG_PASS ); // move any collected text to commit...
#define VarTextEnd(pvt) VarTextEndEx( (pvt) DBG_SRC )
TYPELIB_PROC( int, VarTextLength)( PVARTEXT pvt );
TYPELIB_PROC( PTEXT, VarTextGetEx)( PVARTEXT pvt DBG_PASS );
#define VarTextGet(pvt) VarTextGetEx( (pvt) DBG_SRC )
TYPELIB_PROC( PTEXT, VarTextPeekEx )( PVARTEXT pvt DBG_PASS );
#define VarTextPeek(pvt) VarTextPeekEx( (pvt) DBG_SRC )
TYPELIB_PROC( void, VarTextExpandEx)( PVARTEXT pvt, int size DBG_PASS );
#define VarTextExpand(pvt, sz) VarTextExpandEx( (pvt), (sz) DBG_SRC )

//TYPELIB_PROC( int vtprintfEx( PVARTEXT pvt DBG_PASS, CTEXTSTR format, ... );
// note - don't include format - MUST have at least one parameter passed to ...
//#define vtprintf(pvt, ...) vtprintfEx( (pvt) DBG_SRC, __VA_ARGS__ )

TYPELIB_PROC( int, vtprintfEx)( PVARTEXT pvt, CTEXTSTR format, ... );
// note - don't include format - MUST have at least one parameter passed to ...
#define vtprintf vtprintfEx

TYPELIB_PROC( int, vvtprintf)( PVARTEXT pvt, CTEXTSTR format, va_list args );

//--------------------------------------------------------------------------
// extended command entry stuff... handles editing buffers with insert/overwrite/copy/paste/etc...

typedef struct user_input_buffer_tag {
// -------------------- custom cmd buffer extension 
   int nHistory;  // position counter for pulling history
   PLINKQUEUE InputHistory;
   int   bRecallBegin; // set to TRUE when nHistory has wrapped...

   _32   CollectionBufferLock;
   INDEX CollectionIndex;  // used to store index.. for insert type operations...
   int   CollectionInsert; // flag for whether we are inserting or overwriting
	PTEXT CollectionBuffer; // used to store partial from GatherLine
} USER_INPUT_BUFFER, *PUSER_INPUT_BUFFER;

TYPELIB_PROC( PUSER_INPUT_BUFFER, CreateUserInputBuffer )( void );

TYPELIB_PROC( void, DestroyUserInputBuffer )( PUSER_INPUT_BUFFER *pci );

// negative with SEEK_SET is SEEK_END -nPos
#define COMMAND_POS_SET SEEK_SET
#define COMMAND_POS_CUR SEEK_CUR
TYPELIB_PROC( int, SetUserInputPosition )( PUSER_INPUT_BUFFER pci, int nPos, int whence );

// bInsert < 0 toggle insert.  bInsert == 0 clear isnert(set overwrite) else
// set insert (clear overwrite )
TYPELIB_PROC( void, SetUserInputInsert )( PUSER_INPUT_BUFFER pci, int bInsert );
TYPELIB_PROC( void, SetUserInputInsert )( PUSER_INPUT_BUFFER pci, int bInsert );

TYPELIB_PROC( void, RecallUserInput )( PUSER_INPUT_BUFFER pci, int bUp );
TYPELIB_PROC( void, EnqueUserInputHistory )( PUSER_INPUT_BUFFER pci, PTEXT pHistory );

TYPELIB_PROC( PTEXT, GatherUserInput )( PUSER_INPUT_BUFFER pci, PTEXT stroke );

#ifdef __cplusplus
}; //namespace text {
#endif


//--------------------------------------------------------------------------
#ifdef __cplusplus
	namespace BinaryTree {
#endif
typedef struct treenode_tag *PTREENODE;
typedef struct treeroot_tag *PTREEROOT;

#define BT_OPT_NODUPLICATES 1

typedef int (CPROC *GenericCompare)( PTRSZVAL oldnode,PTRSZVAL newnode );
typedef void (CPROC *GenericDestroy)(POINTER user, PTRSZVAL key);
// when adding a node if Compare is NULL the default method of
// a basic unsigned integer compare on the key value is done.
// if Compare is specified the specified key value of the orginal node (old)
// and of the new node (new) is added.  
// Result of compare should be ( <0 (lesser)) ( 0 (equal)) ( >0 (greater))
TYPELIB_PROC( PTREEROOT, CreateBinaryTreeExtended)( _32 flags
															, GenericCompare Compare
															, GenericDestroy Destroy DBG_PASS);
#define CreateBinaryTreeExx(flags,compare,destroy) CreateBinaryTreeExtended(flags,compare,destroy DBG_SRC)
#define CreateBinaryTreeEx(compare,destroy) CreateBinaryTreeExx( 0, compare, destroy )
//TYPELIB_PROC( PTREEROOT, CreateBinaryTreeEx)( int (*Compare)(PTRSZVAL oldnode,PTRSZVAL newnode )
//                            , void (*Destroy)(PTRSZVAL user, PTRSZVAL key) );

#define CreateBinaryTree() CreateBinaryTreeEx( NULL, NULL )

TYPELIB_PROC( void, DestroyBinaryTree)( PTREEROOT root );

TYPELIB_PROC( void, BalanceBinaryTree)( PTREEROOT root ); 

TYPELIB_PROC( int, AddBinaryNodeEx)( PTREEROOT root
                                    , POINTER userdata
											  , PTRSZVAL key DBG_PASS );
#define AddBinaryNode(r,u,k) AddBinaryNodeEx((r),(u),(k) DBG_SRC )
//TYPELIB_PROC( int, AddBinaryNode)( PTREEROOT root
//                                    , POINTER userdata
//                                    , PTRSZVAL key );

TYPELIB_PROC( void, RemoveBinaryNode)( PTREEROOT root, POINTER use, PTRSZVAL key );

TYPELIB_PROC( POINTER, FindInBinaryTree)( PTREEROOT root, PTRSZVAL key );


// result of fuzzy routine is 0 = match.  100 = inexact match
// 1 = no match, actual may be larger
// -1 = no match, actual may be lesser
// 100 = inexact match- checks nodes near for better match.
TYPELIB_PROC( POINTER, LocateInBinaryTree)( PTREEROOT root, PTRSZVAL key
														, int (CPROC*fuzzy)( PTRSZVAL psv, PTRSZVAL node_key ) );


TYPELIB_PROC( void, RemoveLastFoundNode)(PTREEROOT root );
TYPELIB_PROC( void, RemoveCurrentNode)(PTREEROOT root );

TYPELIB_PROC( void, DumpTree)( PTREEROOT root 
                          , int (*Dump)( POINTER user, PTRSZVAL key ) );


TYPELIB_PROC( POINTER, GetLeastNode)( PTREEROOT root );
TYPELIB_PROC( POINTER, GetGreatestNode)( PTREEROOT root );
TYPELIB_PROC( POINTER, GetLesserNode)( PTREEROOT root );
TYPELIB_PROC( POINTER, GetGreaterNode)( PTREEROOT root );
TYPELIB_PROC( POINTER, GetCurrentNode)( PTREEROOT root );

TYPELIB_PROC( POINTER, GetRootNode)( PTREEROOT root );
TYPELIB_PROC( POINTER, GetParentNode)( PTREEROOT root );
TYPELIB_PROC( POINTER, GetChildNode)( PTREEROOT root, int direction );
TYPELIB_PROC( POINTER, GetPriorNode)( PTREEROOT root );

TYPELIB_PROC( _32, GetNodeCount )( PTREEROOT root );

TYPELIB_PROC( PTREEROOT, ShadowBinaryTree)( PTREEROOT root ); // returns a shadow of the original.

#ifdef __cplusplus
	}; //namespace BinaryTree {
#endif

//--------------------------------------------------------------------------

#ifdef __cplusplus
namespace family {
#endif
typedef struct familyroot_tag *PFAMILYTREE;
TYPELIB_PROC( PFAMILYTREE, CreateFamilyTree )( int (CPROC *Compare)(PTRSZVAL key1, PTRSZVAL key2)
															, void (CPROC *Destroy)(POINTER user, PTRSZVAL key) );
TYPELIB_PROC( POINTER, FamilyTreeFindChild )( PFAMILYTREE root
														  , PTRSZVAL psvKey );
TYPELIB_PROC( void, FamilyTreeReset )( PFAMILYTREE *option_tree );
TYPELIB_PROC( void, FamilyTreeAddChild )( PFAMILYTREE *root, POINTER userdata, PTRSZVAL key );
#ifdef __cplusplus
}; //namespace family {
#endif

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
#ifdef __cplusplus
//} // extern "c" 
}; // namespace containers
}; // namespace sack
using namespace sack::containers::link_stack;
using namespace sack::containers::data_stack;
using namespace sack::containers::data_queue;
using namespace sack::containers::queue;
using namespace sack::containers::BinaryTree;
using namespace sack::containers::text;
using namespace sack::containers::message;
using namespace sack::containers::sets;
using namespace sack::containers::family;
using namespace sack::containers;
#else
// should 'class'ify these things....
#endif

#ifndef _TYPELIBRARY_SOURCE
//#undef TYPELIB_PROC // we don't need this symbol after having built the right prototypes
#endif

#endif

// $Log: typelib.h,v $
// Revision 1.99  2005/07/10 23:56:25  d3x0r
// Fix types for C++...
//
// Revision 1.98  2005/07/05 23:46:09  d3x0r
// Fixes for c++ lameness, and c compatiblity
//
// Revision 1.104  2005/07/05 23:49:45  jim
// Blah more silly fixes to get around C++ lameness.
//
// Revision 1.103  2005/07/05 23:30:27  jim
// Fix set references... C doesn't allow struct ___ to become a name ____
//
// Revision 1.102  2005/07/05 22:20:06  jim
// Compat fixes for c++ and class usage of containers... some protection fixes for failing to load deadstart register
//
// Revision 1.97  2005/07/05 18:10:06  d3x0r
// Fixes for C++ compilation, fixed sets
//
// Revision 1.96  2005/07/01 07:40:00  d3x0r
// Disrelate getfromset from struct - might be a class...
//
// Revision 1.95  2005/06/30 13:22:44  d3x0r
// Attempt to define preload, atexit methods for msvc.  Fix deadstart loading to be more protected.
//
// Revision 1.94  2005/05/25 16:50:09  d3x0r
// Synch with working repository.
//
// Revision 1.101  2005/05/18 21:19:22  jim
// Define a method which will only get a valid set member from a set.
//
// Revision 1.100  2005/05/16 23:18:36  jim
// Allocate the correct amount of space for the message queue - it's a MSGQUEUE not a DATAQUEUE.  Also implement DequeMessage() in such a way that the waited for message ID can change.
//
// Revision 1.99  2005/02/09 22:38:54  panther
// allow timers library to steal sets code....
//
// Revision 1.98  2005/02/04 19:34:12  panther
// Added definition to test for leading whitespace
//
// Revision 1.97  2005/02/04 19:25:54  panther
// Added iterator for sets that's a little different
//
// Revision 1.96  2005/01/10 21:43:42  panther
// Unix-centralize makefiles, also modify set container handling of getmember index
//
// Revision 1.95  2004/12/22 20:15:51  panther
// Parnthise getsetmember for expression usage
//
// Revision 1.94  2004/12/19 15:44:57  panther
// Extended set methods to interact with raw index numbers
//
// Revision 1.93  2004/12/02 10:27:14  panther
// Updates for linux stripped environment build
//
// Revision 1.92  2004/11/05 02:34:41  d3x0r
// Minor mods...
//
// Revision 1.91  2004/09/29 00:49:47  d3x0r
// Added fancy wait for PSI frames which allows non-polling sleeping... Extended Idle() to result in meaningful information.
//
// Revision 1.90  2004/09/24 09:02:12  d3x0r
// Fix the braces around text declarations
//
// Revision 1.89  2004/09/17 16:18:00  d3x0r
// ...
//
// Revision 1.88  2004/09/15 16:12:12  d3x0r
// First - tear apart text object... added many many formatting options to it.
//
// Revision 1.87  2004/07/07 15:33:54  d3x0r
// Cleaned c++ warnings, bad headers, fixed make system, fixed reallocate...
//
// Revision 1.86  2004/06/17 01:28:43  d3x0r
// Tweaks to get relase build to build.
//
// Revision 1.85  2004/06/16 04:37:50  d3x0r
// Add extra parens to clean warnings, and extra initializers for extended union
//
// Revision 1.84  2004/06/16 03:02:44  d3x0r
// checkpoint
//
// Revision 1.83  2004/06/12 09:16:49  d3x0r
// Added flatten line to common text operations. define ignore cursor value
//
// Revision 1.82  2004/06/12 09:12:52  d3x0r
// Added FORMAT_OP, and extended format to include extended info
//
// Revision 1.81  2004/06/07 17:01:55  d3x0r
// Remove position flag...
//
// Revision 1.80  2004/06/07 10:58:27  d3x0r
// add flattenline
//
// Revision 1.79  2004/06/07 10:55:35  d3x0r
// add tab handling as sepearte type of space, extend linelength and buildline, and provide extended methods of calculating spacing
//
// Revision 1.78  2004/06/03 10:59:58  d3x0r
// Fix types passed to createfamilytree
//
// Revision 1.77  2004/06/03 10:56:01  d3x0r
// okay so family tree does exist...
//
// Revision 1.76  2004/05/24 16:41:12  d3x0r
// Add PeekQueue and GetQUeueLength
//
// Revision 1.75  2004/05/04 04:15:28  d3x0r
// remove LogN, soon to consider removing LogX
//
// Revision 1.74  2004/04/26 09:47:25  d3x0r
// Cleanup some C++ problems, and standard C issues even...
//
// Revision 1.73  2004/04/26 09:30:22  d3x0r
// Fix declarations to allow c++ compile
//
// Revision 1.72  2004/04/12 12:07:43  d3x0r
// Added family tree container - parent, child, elder, younger type relations
//
// Revision 1.71  2004/04/06 16:17:30  d3x0r
// Implement text comparison macros as functions for segfault protection
//
// Revision 1.70  2004/03/06 08:35:29  d3x0r
// format mods
//
// Revision 1.69  2004/03/04 01:09:47  d3x0r
// Modifications to force slashes to wlib.  Updates to Interfaces to be gotten from common interface manager.
//
// Revision 1.68  2004/02/14 01:19:00  d3x0r
// Extensions of Set structure in containers, C++ interface extension
//
// Revision 1.67  2004/02/10 19:03:03  d3x0r
// Fix termination on class traversion of plist
//
// Revision 1.66  2004/02/08 23:33:15  d3x0r
// Add a iList class for c++, public access to building parameter va_lists
//
// Revision 1.65  2004/01/29 10:11:50  d3x0r
// extended format for text type
//
// Revision 1.64  2003/11/28 19:37:42  panther
// Fix adoptcontrol, orphancontrol
//
// Revision 1.63  2003/11/10 01:53:49  panther
// Fix vartextcreateex call
//
// Revision 1.62  2003/11/03 15:51:18  panther
// Add some functionality to VarText, abstract data content
//
// Revision 1.61  2003/10/27 17:38:37  panther
// Define some errno abstractions - incomplete... but needed for msgsvr port
//
// Revision 1.60  2003/10/26 23:38:20  panther
// Modify createmsgqueue
//
// Revision 1.59  2003/10/24 14:51:01  panther
// Modify remove functions binary tree
//
// Revision 1.58  2003/10/20 00:04:21  panther
// Extend OpenSpace in SharedMem
// revise msgqueue operations to more resemble sysVipc msgq
//
// Revision 1.57  2003/08/20 08:07:12  panther
// some fixes to blot scaled... fixed to makefiles test projects... fixes to export containters lib funcs
//
// Revision 1.56  2003/08/08 07:50:34  panther
// Fix LIST_FORALL to clear variable to NULL at end of list.
//
// Revision 1.55  2003/08/02 17:40:09  panther
// Fix format on x,y position to be signed again - watcom is wrong either way
//
// Revision 1.54  2003/08/01 07:57:39  panther
// Fix project builds
//
// Revision 1.53  2003/07/27 14:22:26  panther
// Define forallinset callback as cproc
//
// Revision 1.52  2003/07/25 10:21:57  panther
// Fix callback for foralllinks
//
// Revision 1.51  2003/05/20 18:30:16  panther
// New functions - create/destroy vartext
//
// Revision 1.50  2003/04/27 01:24:17  panther
// Add AddDataItem, DATA_FORALL
//
// Revision 1.49  2003/04/21 11:46:52  panther
// Ug - forgot a commit somewhere... return pointer at set data item
//
// Revision 1.48  2003/04/21 08:12:27  panther
// Hmm lost change - destroydatalist to deletedatalist
//
// Revision 1.47  2003/04/13 22:14:06  panther
// Added extended format operation
//
// Revision 1.46  2003/04/12 20:52:46  panther
// Added new type contrainer - data list.
//
// Revision 1.45  2003/04/08 07:01:22  panther
// Fix cleanup issue (popups) Added another text type EX FORMAT OP
// Fixed a dangling segsplit in text.c
//
// Revision 1.44  2003/04/06 23:24:12  panther
// Define another FORMAT_OP and redefine SegSplit
//
// Revision 1.43  2003/04/06 09:57:02  panther
// Remove unused FORMAT_OP, make coords signed, add FORMATPOS define
//
// Revision 1.42  2003/04/02 06:45:37  panther
// Define flags for handling positioning in TEXT subsystem
//
// Revision 1.41  2003/03/30 21:16:05  panther
// Added EX functions to pass application source to DataStack allocations
//
// Revision 1.40  2003/03/26 07:23:53  panther
// Include buildline end of line option
//
// Revision 1.39  2003/03/25 08:38:11  panther
// Add logging
//
