/* This contains the methods to use the base container types
   defined in sack_types.h.                                  */
#ifndef LINKSTUFF
#define LINKSTUFF

#include <sack_types.h>


	SACK_NAMESPACE
	_CONTAINER_NAMESPACE

#    define TYPELIB_CALLTYPE CPROC
#  if defined( _TYPELIBRARY_SOURCE_STEAL )
#    define TYPELIB_PROC type TYPELIB_CALLTYPE name type CPROC name
#  elif defined( _TYPELIBRARY_SOURCE )
#    define TYPELIB_PROC EXPORT_METHOD
#  else
#    define TYPELIB_PROC IMPORT_METHOD
#  endif

// These were designated for basic content containers...
// original implementation basically just used TEXT
//PDATA CreateData( _32 size );
//PDATA CreateDataFromText( CTEXTSTR pData );
//PDATA ExpandData( PDATA pData, _32 amount );
//PDATA DuplicateData( PDATA pData );

//void ReleaseData( PDATA pd );
_LINKLIST_NAMESPACE

//--------------------------------------------------------
TYPELIB_PROC  PLIST TYPELIB_CALLTYPE        CreateListEx   ( DBG_VOIDPASS );
TYPELIB_PROC  PLIST TYPELIB_CALLTYPE        DeleteListEx   ( PLIST *plist DBG_PASS );
/* See <link AddLink>.
   
   
   
   See <link DBG_PASS>. */
TYPELIB_PROC  PLIST TYPELIB_CALLTYPE        AddLinkEx      ( PLIST *pList, POINTER p DBG_PASS );
TYPELIB_PROC  PLIST TYPELIB_CALLTYPE        SetLinkEx      ( PLIST *pList, INDEX idx, POINTER p DBG_PASS );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      GetLink        ( PLIST *pList, INDEX idx );
TYPELIB_PROC  POINTER* TYPELIB_CALLTYPE     GetLinkAddress ( PLIST *pList, INDEX idx );
TYPELIB_PROC  INDEX TYPELIB_CALLTYPE        FindLink       ( PLIST *pList, POINTER value );
TYPELIB_PROC  LOGICAL TYPELIB_CALLTYPE      DeleteLink     ( PLIST *pList, POINTER value );
TYPELIB_PROC  void TYPELIB_CALLTYPE         EmptyList      ( PLIST *pList );

#ifdef __cplusplus
/* This was a basic attempt to make list into a C++ class. I
   gave up doing this sort of thing afterwards after realizing
   the methods of a library and these static methods for a class
   aren't much different.                                        */
typedef class iList
{
	PLIST list;
	INDEX idx;
public:
	inline iList() { list = CreateListEx( DBG_VOIDSRC ); }
	inline ~iList() { DeleteListEx( &list DBG_SRC ); }
	inline iList &operator+=( POINTER &p ){ AddLinkEx( &list, p DBG_SRC ); return *this; }
	inline void add( POINTER p ) { AddLinkEx( &list, p DBG_SRC ); }
	inline POINTER first( void ) { POINTER p; for( idx = 0, p = NULL;list && (idx < list->Cnt) && (( p = GetLink( &list, idx ) )!=0); idx++ ); return p; }
	inline POINTER next( void ) { POINTER p; for( idx++;list && (( p = GetLink( &list, idx ) )!=0) && idx < list->Cnt; idx++ ); return p; }
	inline POINTER get(INDEX idx) { return GetLink( &list, idx ); }
} *piList;
#endif

// address of the thing...
typedef PTRSZVAL (CPROC *ForProc)( PTRSZVAL user, INDEX idx, POINTER *item );
// if the callback function returns non 0 - then the looping is aborted,
// and the value is returned... the user value is passed to the callback.
TYPELIB_PROC  PTRSZVAL TYPELIB_CALLTYPE     ForAllLinks    ( PLIST *pList, ForProc func, PTRSZVAL user ); 

/* This is a iterator which can be used to check each member in
   a PLIST.
   Parameters
   list :     List to iterate through
   index :    variable to use to index the list
   type :     type of the elements stored in the list (for C++)
   pointer :  variable used to get the current member of the
              list.
   
   Example
   <code lang="c++">
   POINTER p;  // the pointer to receive the list member pointer (should be a user type)
   INDEX idx; // indexer
   PLIST pList; // some list.
   
   LIST_FORALL( pList, idx, POINTER, p )
   {
       // p will never be NULL here.
       // each link stored in the list is set to p here..
   
       // this is a way to remove this item from the list...
       SetLink( &amp;pList, idx, NULL );
   
       if( some condition )
          break;
   }
   
   </code>
   Another example that uses data and searches..
   <code lang="c++">
   PLIST pList = NULL;
   INDEX idx;
   CTEXTSTR string;
   
   AddLink( &amp;pList, (POINTER)"hello" );
   </code>
   <code>
   AddLink( &amp;pList, (POINTER)"world" );
   
   LITS_FORALL( pList, idx, CTEXTSTR, string )
   {
       if( strcmp( string, "hello" ) == 0 )
           break;
   }
   // here 'string' will be NULL if not found, else will be what was found
   </code>
   Remarks
   This initializes the parameters passed to the macro so that
   if the list is NULL or empty, then p will be set to NULL. If
   there are no non-nulll members in the list, p will be set to
   NULL. If you break in the loop, like in the case of searching
   the list for something, then p will be non-null at the end of
   the loop.
                                                                                         */
#define LIST_FORALL( l, i, t, v )  if(((v)=(t)NULL),(l))            \
                                            for( ((i)=0); ((i) < ((l)->Cnt))? \
                                        (((v)=(t)(PTRSZVAL)(l)->pNode[i]),1):(((v)=(t)NULL),0); (i)++ )  if( v )
/* This can be used to continue iterating through a list after a
   LIST_FORALL has been interrupted.
   Parameters
   list :     \Description
   index :    index variable for stepping through the list
   type :     type of the members in the list.
   pointer :  variable name to use to store the the current list
              element.
   
   Example
   <code lang="c++">
   PLIST pList = NULL;
   CTEXTSTR p;
   INDEX idx;
   
   </code>
   <code>
   AddLink( &amp;pList, "this" );
   AddLink( &amp;pList, "is" );
   AddLink( &amp;pList, "a" );
   AddLink( &amp;pList, "test" );
   
   LIST_FORALL( pList, idx, CTEXTSTR, p )
   {
       if( strcmp( p, "is" ) == 0 )
           break;
   }
   LIST_NEXTALL( pList, idx, CTEXTSTR, p )
   {
       printf( "remaining element : %s", p );
   }
   </code>
   <code lang="c++">
   j 
   </code>                                                       */
#define LIST_NEXTALL( l, i, t, v )  if(l)            \
    for( ++(i),((v)=(t)NULL); ((i) < ((l)->Cnt))? \
    (((v)=(t)(l)->pNode[i]),1):(((v)=(t)NULL),0); (i)++ )  if( v )
/* <combine sack::containers::list::CreateListEx@DBG_VOIDPASS>
   
   \ \                                                         */

#define CreateList()       ( CreateListEx( DBG_VOIDSRC ) )
/* <combine sack::containers::list::DeleteListEx@PLIST *plist>
   
   \ \                                                         */
#define DeleteList(p)      ( DeleteListEx( (p) DBG_SRC ) )
/* Adds a pointer to a user object to a list.
   Example
   <code lang="c++">
   
   // the list can be initialized to NULL,
   // it does not have to be assigned the result of a CreateList().
   // this allows the list to only be allocated if it is used.
   PLIST list = NULL;
   
   AddLink( &amp;list, (POINTER)user_pointer );
   
   
   {
       POINTER p; // this should be USER_DATA_TYPE *p;
       INDEX idx; // just a generic counter.
       LIST_FORALL( list, idx, POINTER, p )
       {
           // for each item in the list, p will be not null.
           if( p-\>something == some_other_thing )
               break;
       }
       // p will be NULL if the list is empty
       // p will be NULL if the LIST_FORALL loop completes to termination.
       // p will be not NULL if the LIST_FORALL loop executed a 'break;'
   }
   </code>                                                                 */
#define AddLink(p,v)       ( AddLinkEx( (p),((POINTER)(v)) DBG_SRC ) )
/* <combine sack::containers::list::SetLinkEx@PLIST *@INDEX@POINTER p>
   
   \ \                                                                 */
#define SetLink(p,i,v)     ( SetLinkEx( (p),(i),((POINTER)(v)) DBG_SRC ) )

#ifdef __cplusplus
	};//		namespace list;
#endif
//--------------------------------------------------------
_DATALIST_NAMESPACE

TYPELIB_PROC  PDATALIST TYPELIB_CALLTYPE  CreateDataListEx ( PTRSZVAL nSize DBG_PASS );
TYPELIB_PROC  void TYPELIB_CALLTYPE       DeleteDataListEx ( PDATALIST *ppdl DBG_PASS );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE    SetDataItemEx ( PDATALIST *ppdl, INDEX idx, POINTER data DBG_PASS );
/* Adds an item to a DataList.
   Example
   <code lang="c++">
   PDATALIST datalist = CreateDataList();
   
   struct my_struct {
       _32 my_data;
   }
   
   struct my_struct my_item;
   my_item.my_data = 0;
   
   
   AddDataItem( &amp;datalist, &amp;my_item );
   
   </code>                                     */
#define AddDataItem(list,data) (((list)&&(*(list)))?SetDataItemEx((list),(*list)->Cnt,data DBG_SRC ):NULL)
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE    SetDataItemEx ( PDATALIST *ppdl, INDEX idx, POINTER data DBG_PASS );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE    GetDataItem ( PDATALIST *ppdl, INDEX idx );
TYPELIB_PROC  void TYPELIB_CALLTYPE       DeleteDataItem ( PDATALIST *ppdl, INDEX idx );
TYPELIB_PROC  void TYPELIB_CALLTYPE       EmptyDataList ( PDATALIST *ppdl );

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
/* Destroy a DataList.
   Example
   <code>
   PDATALIST datalist = CreateDataList( 4 );
   DeleteDataList( &amp;datalist );
   </code>
   Parameters
   ppDataList :  pointer to the PDATALIST.   */
#define DeleteDataList(p)  ( DeleteDataListEx( (p) DBG_SRC ) )
#define SetDataItem(p,i,v) ( SetDataItemEx( (p),(i),(v) DBG_SRC ) )

   _DATALIST_NAMESPACE_END

//--------------------------------------------------------

#ifdef __cplusplus
		namespace link_stack {
#endif
TYPELIB_PROC  PLINKSTACK TYPELIB_CALLTYPE   CreateLinkStackEx( DBG_VOIDPASS );
         // creates a link stack with maximum entries - any extra entries are pushed off the bottom into NULL
TYPELIB_PROC  PLINKSTACK TYPELIB_CALLTYPE      CreateLinkStackLimitedEx        ( int max_entries  DBG_PASS );
#define CreateLinkStackLimited(n) CreateLinkStackLimitedEx(n DBG_SRC)
TYPELIB_PROC  void TYPELIB_CALLTYPE         DeleteLinkStackEx( PLINKSTACK *pls DBG_PASS);
TYPELIB_PROC  PLINKSTACK TYPELIB_CALLTYPE   PushLinkEx       ( PLINKSTACK *pls, POINTER p DBG_PASS);
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PopLink          ( PLINKSTACK *pls );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PeekLink         ( PLINKSTACK *pls );
// thought about adding these, but decided on creating a limited stack instead.
//TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      StackLength      ( PLINKSTACK *pls );
//TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PopLinkEx        ( PLINKSTACK *pls, int position );

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

TYPELIB_PROC  PDATASTACK TYPELIB_CALLTYPE   CreateDataStackEx( INDEX size DBG_PASS ); // sizeof data elements...
TYPELIB_PROC  void TYPELIB_CALLTYPE         DeleteDataStackEx( PDATASTACK *pds DBG_PASS);
TYPELIB_PROC  PDATASTACK TYPELIB_CALLTYPE   PushDataEx     ( PDATASTACK *pds, POINTER pdata DBG_PASS );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PopData        ( PDATASTACK *pds );
TYPELIB_PROC  void TYPELIB_CALLTYPE         EmptyDataStack ( PDATASTACK *pds );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PeekData       ( PDATASTACK *pds ); // keeps data on stack (can be used)
// Incrementing Item moves progressivly down the stack
// final(invalid) stack, and/or empty stack will return NULL;
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PeekDataEx     ( PDATASTACK *pds, INDEX Item ); // keeps data on stack (can be used)
                                          
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

/* Creates a <link LinkQueue>. In debug mode, gets passed the
   current source and file so it can blame the user for the
   allocation.                                                */
TYPELIB_PROC  PLINKQUEUE TYPELIB_CALLTYPE   CreateLinkQueueEx( DBG_VOIDPASS );
TYPELIB_PROC  void TYPELIB_CALLTYPE         DeleteLinkQueueEx( PLINKQUEUE *pplq DBG_PASS );
TYPELIB_PROC  PLINKQUEUE TYPELIB_CALLTYPE   EnqueLinkEx      ( PLINKQUEUE *pplq, POINTER link DBG_PASS );
TYPELIB_PROC  PLINKQUEUE TYPELIB_CALLTYPE   PrequeLinkEx      ( PLINKQUEUE *pplq, POINTER link DBG_PASS );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      DequeLink        ( PLINKQUEUE *pplq );
TYPELIB_PROC  LOGICAL TYPELIB_CALLTYPE      IsQueueEmpty     ( PLINKQUEUE *pplq );
TYPELIB_PROC  INDEX TYPELIB_CALLTYPE        GetQueueLength   ( PLINKQUEUE plq );
// get a PLINKQUEUE element at index
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PeekQueueEx    ( PLINKQUEUE plq, INDEX idx );
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE      PeekQueue    ( PLINKQUEUE plq );


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
TYPELIB_PROC  PDATAQUEUE TYPELIB_CALLTYPE   CreateDataQueueEx( INDEX size DBG_PASS );
TYPELIB_PROC  PDATAQUEUE TYPELIB_CALLTYPE   CreateLargeDataQueueEx( INDEX size, INDEX entries, INDEX expand DBG_PASS );
TYPELIB_PROC  void TYPELIB_CALLTYPE         DeleteDataQueueEx( PDATAQUEUE *pplq DBG_PASS );
TYPELIB_PROC  PDATAQUEUE TYPELIB_CALLTYPE   EnqueDataEx      ( PDATAQUEUE *pplq, POINTER Data DBG_PASS );
TYPELIB_PROC  PDATAQUEUE TYPELIB_CALLTYPE   PrequeDataEx      ( PDATAQUEUE *pplq, POINTER Data DBG_PASS );
TYPELIB_PROC  LOGICAL TYPELIB_CALLTYPE      DequeData        ( PDATAQUEUE *pplq, POINTER Data );
TYPELIB_PROC  LOGICAL TYPELIB_CALLTYPE      UnqueData        ( PDATAQUEUE *pplq, POINTER Data );
TYPELIB_PROC  LOGICAL TYPELIB_CALLTYPE      IsDataQueueEmpty ( PDATAQUEUE *pplq );
TYPELIB_PROC  void TYPELIB_CALLTYPE         EmptyDataQueue ( PDATAQUEUE *pplq );
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
TYPELIB_PROC  LOGICAL TYPELIB_CALLTYPE  PeekDataQueueEx    ( PDATAQUEUE *pplq, POINTER ResultBuffer, INDEX idx );
#define PeekDataQueueEx( q, type, result, idx ) PeekDataQueueEx( q, (POINTER)result, idx )
/*
 * Result buffer is filled with the last element, and the result is true, otherwise the return
 * value is FALSE, and the data was not filled in.
 */
TYPELIB_PROC  LOGICAL TYPELIB_CALLTYPE  PeekDataQueue    ( PDATAQUEUE *pplq, POINTER ResultBuffer );
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
typedef void (CPROC *MsgQueueReadCallback)( PTRSZVAL psv, CPOINTER p, PTRSZVAL sz );
TYPELIB_PROC  PMSGHANDLE TYPELIB_CALLTYPE  SackCreateMsgQueue ( CTEXTSTR name, PTRSZVAL size
                                                      , MsgQueueReadCallback Read
                                                      , PTRSZVAL psvRead );
TYPELIB_PROC  PMSGHANDLE TYPELIB_CALLTYPE  SackOpenMsgQueue ( CTEXTSTR name
													 , MsgQueueReadCallback Read
													 , PTRSZVAL psvRead );
TYPELIB_PROC  void TYPELIB_CALLTYPE  DeleteMsgQueue ( PMSGHANDLE **ppmh );

#define MSGQUE_NOWAIT 0x0001 // if enque, fail send, return immediate on fail
                             // if deque, fail no msg ready to get...
#define MSGQUE_EXCEPT 0x0002 // read any msg BUT MsgID
#define MSGQUE_WAIT_ID 0x0004 // enque this message... it is a task ID which is waiting.

/* Error result if there is no message to read. (GetLastError()
   after peekmsg or readmsg returns -1)                         */
#define MSGQUE_ERROR_NOMSG 1
/* Error result if the message to read is bigger than the buffer
   passed to read the message.                                   */
#define MSGQUE_ERROR_E2BIG 2
/* Error result. Unexpected error (queue head/tail out of
   bounds)                                                */
#define MSGQUE_ERROR_EABORT 5
// result is the size of the message, or 0 if no message.
// -1 if some other error?
TYPELIB_PROC  int TYPELIB_CALLTYPE  DequeMsgEx ( PMSGHANDLE pmh, _32 *MsgID, POINTER buffer, PTRSZVAL msgsize, _32 options DBG_PASS );
/* Receives a message from the message queue.
   Parameters
   Message Queue :  PMSGHANDLE to read from
   Message ID * :   a Pointer to the message ID to read. Updated
                    with the message ID from the queue.
   buffer :         buffer to read message into
   buffer length :  length of the buffer to read
   options :        extra options for the read
   
   Return Value List
   \-1 :  Error
   0 :    No Message to read
   \>0 :  size of message read.
   
   Returns
   \ \                                                           */
#define DequeMsg(q,b,s,i,o) DequeMsgEx(q,b,s,i,o DBG_SRC )
TYPELIB_PROC  int TYPELIB_CALLTYPE  PeekMsgEx ( PMSGHANDLE pmh, _32 MsgID, POINTER buffer, PTRSZVAL msgsize, _32 options DBG_PASS );
/* Just peek at the next message.
   Parameters
   queue :        The PMSGHANDLE queue to read.
   MsgID :        what message to read. 0 is read any message.
   buffer :       where to read the message data into.
   buffer_size :  the length of the message buffer.
   options :      Options controlling the read
   
   Returns
   \-1 on error
   
   0 if no message
   
   length of the message read                                  */
#define PeekMsg(q,b,s,i,o) PeekMsgEx(q,b,s,i,o DBG_SRC )
TYPELIB_PROC  int TYPELIB_CALLTYPE  EnqueMsgEx ( PMSGHANDLE pmh, POINTER buffer, PTRSZVAL msgsize, _32 options DBG_PASS );
/* Add a message to the queue.
   Parameters
   Message Queue :  PMSGQUEUE to write to.
   Buffer :         pointer to the message to send. THe MSgID is
                    the first part of the message buffer.
   Buffer Length :  how long the message to send is
   Options :        Extra options for send
   
   Return Value List
   \-1 :  Error
   \>0 :  bytes of message sent                                  */
#define EnqueMsg(q,b,s,o) EnqueMsgEx(q,b,s,o DBG_SRC )
TYPELIB_PROC  int TYPELIB_CALLTYPE  IsMsgQueueEmpty ( PMSGHANDLE pmh );

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

/* Hard coded 32 bit division for getting word index. (x\>\>5) */
#define UNIT_USED_IDX(n)   ((n) >> 5)
/* Hard coded 32 bit division for getting bit index. (x &amp;
   0x1f)                                                      */
#define UNIT_USED_MASK(n)  (1 << ((n) &0x1f))

/* A macro for use by internal code that marks a member of a set
   as used.
   Parameters
   set :    pointer to a genericset
   index :  item to mark used.                                   */
#define SetUsed(set,n)   ((((set)->bUsed[UNIT_USED_IDX(n)]) |= UNIT_USED_MASK(n)), (++(set)->nUsed) )
/* A macro for use by internal code that marks a member of a set
   as available.
   Parameters
   set :    pointer to a genericset
   index :  item to mark available.                              */
#define ClearUsed(set,n) ((((set)->bUsed[UNIT_USED_IDX(n)]) &= ~UNIT_USED_MASK(n)), (--(set)->nUsed) )
/* A macro for use by internal code that tests a whole set of
   bits for used. (32 bits, can check to see if any in 32 is
   free)
   Parameters
   set :    pointer to a genericset
   index :  index of an one in the set of 32 being tested.
   
   Returns
   0 if not all are used.
   
   1 if all in this block of bits are used.                   */
#define AllUsed(set,n)   (((set)->bUsed[UNIT_USED_IDX(n)]) == 0xFFFFFFFF )
/* A macro for use by internal code that tests a member of a set
   as used.
   Parameters
   set :    pointer to a genericset
   index :  item to test used.
   
   Returns
   not zero if is used, otherwise is free.                       */
#define IsUsed(set,n)    (((set)->bUsed[UNIT_USED_IDX(n)]) & UNIT_USED_MASK(n) )


#ifdef __cplusplus
#define CPP_(n)
/* A macro which is used to emit code in C++ mode... */
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

/* This represents the basic generic set structure. Addtional
   data is allocated at the end of this strcture to fit the bit
   array that maps usage of the set, and for the set size of
   elements.
   
   
   Remarks
   \ \ 
   Summary
   Generic sets are good for tracking lots of tiny structures.
   
   
   
   They track slabs of X structures at a time. They allocate a
   slab of X structures with an array of X bits indicating
   whether a node is used or not. The structure overall has how
   many are used, so once full, a block can be quickly checked
   whether there is anything free. Then when checking a block
   that might have room, the availablility is checked 32 bits at
   a time, until a free spot is found.
   
   Sets of 1024 members of x,y coordinates for example are good
   for this sort of storage. the points are often static, once
   loaded they all exist until none of them do. This storage has
   gross deletion methods too, quickly evaporate all allocated
   chunks. Storing tiny chunks in a slab is more efficient
   because every allocation method has some sort of tracking
   associated with it - an overhead of having it. Plus, when
   operating on sets of data, a single solid slab of exatly the
   structures you are working with is more efficient to cache.
   
   
   Example
   <code lang="c++">
   struct treenode_tag {
       _32 treenode_data;  // abitrary structure data
   };
   typedef struct treenode_tag TREENODE;
   
   \#define MAXTREENODESPERSET 256
   DeclareSet( TREENODE );
   </code>
   The important part of the prior code is the last two lines.
   
   \#define MAX\<your type name\>SPERSET \<how many\>
   
   This defines how many of your structure are kept per set
   block.
   
   The DeclareSet( type ) declares a typedefed structure called
   'struct type##set_tag', 'name##SET', and '*P##name##SET'; in
   the above case, it would be 'struct TREENODEset_tag',
   'TREENODESET', and 'PTREENODESET'.
   
   
   
   Then to actually use the set...
   <code lang="c#">
   // declare a set pointer with one of the magic names.
   PTREENODESET nodeset = NULL;
   // get a node from the set.
   TREENODE *node = GetFromSet( TREENODE, nodeset );
   
   </code>
   
   Notice there is no CreateSet, getting a set member will
   create the set as required. Many operations may expend the
   set, except for GetUsedSetMember which will only result with
   \members that are definatly in the set. Accesses to the set
   are all prefixed by the type name the set was created with,
   'TREENODE' in this example.
   <code lang="c++">
   DeleteFromSet( TREENODE, nodeset, node );
   node = GetFromSet( TREENODE, nodeset );
   
   {
      int index = GetMemberIndex( TREENODE, nodeset, node );
   }
   </code>
   
   The accessor macros take care of expanding several parameters
   that require sizeof structure expansion.                      */
typedef struct genericset_tag {
	// wow might be nice to have some flags...
	// first flag - bSetSet - meaning that this is a set of sets of
	// the type specified...
	struct genericset_tag *next;
	/* This is the pointer that's pointing at the pointer pointing
	   to me. (did you get that?) See <link DeclareLink>.          */
	struct genericset_tag **me;
	_32 nUsed;
	_32 nBias; // hmm if I change this here? we're hozed... so.. we'll do it anyhow :) evil - recompile please
	_32 bUsed[1]; // after this p * unit must be computed
} GENERICSET, *PGENERICSET;

/* \ \ 
   Parameters
   pSet :      pointer to a generic set
   nMember :   index of the member
   setsize :   number of elements in each block
   unitsize :  set block
   maxcnt :    max elements per set block       */
TYPELIB_PROC  POINTER  TYPELIB_CALLTYPE GetFromSetEx( GENERICSET **pSet, int setsize, int unitsize, int maxcnt DBG_PASS );
/* <combine sack::containers::sets::GetFromSetEx@GENERICSET **@int@int@int maxcnt>
   
   \ \                                                                             */
#define GetFromSeta(ps, ss, us, max) GetFromSetEx( (ps), (ss), (us), (max) DBG_SRC )
/* <combine sack::containers::sets::GetFromSetEx@GENERICSET **@int@int@int maxcnt>
   
   \ \ 
   Parameters
   name :  name of type the set contains.
   pSet :  pointer to a set to get an element from.                                */
#define GetFromSet( name, pset ) (P##name)GetFromSeta( (GENERICSET**)(pset), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET )

/* \ \ 
   Parameters
   pSet :      pointer to a generic set
   nMember :   index of the member
   setsize :   number of elements in each block
   unitsize :  set block
   maxcnt :    max elements per set block       */
TYPELIB_PROC  PGENERICSET  TYPELIB_CALLTYPE GetFromSetPoolEx( GENERICSET **pSetSet
													 , int setsetsize, int setunitsize, int setmaxcnt
													 , GENERICSET **pSet
													 , int setsize, int unitsize, int maxcnt DBG_PASS );
/* <combine sack::containers::sets::GetFromSetPoolEx@GENERICSET **@int@int@int@GENERICSET **@int@int@int maxcnt>
   
   \ \                                                                                                           */
#define GetFromSetPoola(pl, sss, sus, smax, ps, ss, us, max) GetFromSetPoolEx( (pl), (sss), (sus), (smax), (ps), (ss), (us), (max) DBG_SRC )
/* <combine sack::containers::sets::GetFromSetPoolEx@GENERICSET **@int@int@int@GENERICSET **@int@int@int maxcnt>
   
   \ \                                                                                                           */
#define GetFromSetPool( name, pool, pset ) (P##name)GetFromSetPoola( (GENERICSET**)(pool)    \
	, sizeof( name##SETSET ), sizeof( name##SET ), MAX##name##SETSPERSET\
	, (GENERICSET**)(pset), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET )

/* \ \ 
   Parameters
   pSet :      pointer to a generic set
   nMember :   index of the member
   setsize :   number of elements in each block
   unitsize :  set block
   maxcnt :    max elements per set block       */
TYPELIB_PROC  POINTER  TYPELIB_CALLTYPE GetSetMemberEx( GENERICSET **pSet, INDEX nMember, int setsize, int unitsize, int maxcnt DBG_PASS );
/* <combine sack::containers::sets::GetSetMemberEx@GENERICSET **@INDEX@int@int@int maxcnt>
   
   \ \                                                                                     */
#define GetSetMembera(ps, member, ss, us, max) (GetSetMemberEx( (ps), (member), (ss), (us), (max) DBG_SRC ))
/* <combine sack::containers::sets::GetSetMemberEx@GENERICSET **@INDEX@int@int@int maxcnt>
   
   \ \                                                                                     */
#define GetSetMember( name, pset, member ) ((P##name)GetSetMembera( (GENERICSET**)(pset), (member), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET ))

/* \ \ 
   Parameters
   pSet :      pointer to a generic set
   nMember :   index of the member
   setsize :   number of elements in each block
   unitsize :  set block
   maxcnt :    max elements per set block       */
TYPELIB_PROC  POINTER  TYPELIB_CALLTYPE GetUsedSetMemberEx( GENERICSET **pSet, INDEX nMember, int setsize, int unitsize, int maxcnt DBG_PASS );
/* <combine sack::containers::sets::GetUsedSetMemberEx@GENERICSET **@INDEX@int@int@int maxcnt>
   
   \ \                                                                                         */
#define GetUsedSetMembera(ps, member, ss, us, max) (GetUsedSetMemberEx( (ps), (member), (ss), (us), (max) DBG_SRC ))
/* <combine sack::containers::sets::GetUsedSetMemberEx@GENERICSET **@INDEX@int@int@int maxcnt>
   
   \ \                                                                                         */
#define GetUsedSetMember( name, pset, member ) ((P##name)GetUsedSetMembera( (GENERICSET**)(pset), (member), sizeof( name##SET ), sizeof( name ), MAX##name##SPERSET ))

TYPELIB_PROC  INDEX TYPELIB_CALLTYPE  GetMemberIndex(GENERICSET **set, POINTER unit, int unitsize, int max );
/* <combine sack::containers::sets::GetMemberIndex>
   
   \ \                                              */
#define GetMemberIndex(name,set,member) GetMemberIndex( (GENERICSET**)set, member, sizeof( name ), MAX##name##SPERSET )

/* <combine sack::containers::sets::GetMemberIndex@GENERICSET **@POINTER@int@int>
   
   \ \                                                                            */
#define GetIndexFromSet( name, pset ) GetMemberIndex( name, pset, GetFromSet( name, pset ) )

/* \ \ 
   Parameters
   pSet :      pointer to a generic set
   nMember :   index of the member
   setsize :   number of elements in each block
   unitsize :  set block
   maxcnt :    max elements per set block       */
TYPELIB_PROC  void TYPELIB_CALLTYPE  DeleteFromSetExx( GENERICSET *set, POINTER unit, int unitsize, int max DBG_PASS );
/* <combine sack::containers::sets::DeleteFromSetExx@GENERICSET *@POINTER@int@int max>
   
   \ \                                                                                 */
#define DeleteFromSetEx( name, set, member, xx ) DeleteFromSetExx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET DBG_SRC )
/* <combine sack::containers::sets::DeleteFromSetExx@GENERICSET *@POINTER@int@int max>
   
   \ \                                                                                 */
#define DeleteFromSet( name, set, member ) DeleteFromSetExx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET DBG_SRC )

/* Marks a member in a set as usable.
   Parameters
   set :       pointer to a genericset pointer
   iMember :   index of member to delete
   unitsize :  (filled by macro) size of element in set
   max :       (filled by macro) size of a block of elements. */
TYPELIB_PROC  void TYPELIB_CALLTYPE  DeleteSetMemberEx( GENERICSET *set, INDEX iMember, PTRSZVAL unitsize, INDEX max );
/* <combine sack::containers::sets::DeleteSetMemberEx@GENERICSET *@INDEX@PTRSZVAL@INDEX>
   
   \ \                                                                                   */
#define DeleteSetMember( name, set, member ) DeleteSetMemberEx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET )

/* This function can check to see if a pointer is a valid
   element from a set.
   Parameters
   set :       pointer to a set to check
   unit :      pointer to an element from the set
   unitsize :  size of element structures in the set.
   max :       count of structures per set block
   
   Returns
   TRUE if unit is in the set, else FALSE.                */
TYPELIB_PROC  int TYPELIB_CALLTYPE  MemberValidInSetEx( GENERICSET *set, POINTER unit, int unitsize, int max );
/* <combine sack::containers::sets::MemberValidInSetEx@GENERICSET *@POINTER@int@int>
   
   \ \                                                                               */
#define MemberValidInSet( name, set, member ) MemberValidInSetEx( (GENERICSET*)set, member, sizeof( name ), MAX##name##SPERSET )

TYPELIB_PROC  int TYPELIB_CALLTYPE  CountUsedInSet( GENERICSET *set, int max );
/* <combine sack::containers::sets::CountUsedInSet>
   
   \ \                                              */
#define CountUsedInSet( name, set ) CountUsedInSet( (GENERICSET*)set, MAX##name##SPERSET )

TYPELIB_PROC  POINTER * TYPELIB_CALLTYPE GetLinearSetArray( GENERICSET *pSet, int *pCount, int unitsize, int max );
/* <combine sack::containers::sets::GetLinearSetArray>
   
   \ \                                                 */
#define GetLinearSetArray( name, set, pCount ) GetLinearSetArray( (GENERICSET*)set, pCount, sizeof( name ), MAX##name##SPERSET )

/* Returned the index of an item in a linear array returned from
   a set.
   Parameters
   pArray :      pointer to an array which has been returned from
                 the set
   nArraySize :  size fo the array
   unit :        pointer to an element in the array
   
   Returns
   Index of the unit in the array, INVALID_INDEX if not in the
   array.                                                         */
TYPELIB_PROC  int TYPELIB_CALLTYPE  FindInArray( POINTER *pArray, int nArraySize, POINTER unit );

/* Delete all allocated slabs.
   Parameters
   ppSet :  pointer to a generic set pointer to delete. */
TYPELIB_PROC  void TYPELIB_CALLTYPE  DeleteSet( GENERICSET **ppSet );
/* <combine sack::containers::sets::DeleteSet@GENERICSET **>
   
   \ \                                                       */
#define DeleteSetEx( name, ppset ) { name##SET **delete_me = ppset; DeleteSet( (GENERICSET**)delete_me ); }

typedef PTRSZVAL (CPROC *FAISCallback)(void*,PTRSZVAL);
/* \ \ 
   Parameters
   pSet :      poiner to a set
   unitsize :  size of elements in the array
   max :       count of elements per set block
   f :         user callback function to call for each element in
               the set
   psv :       user data passed to the user callback when it is
               invoked for a member of the set.
   
   Returns
   If the user callback returns 0, the loop continues. If the
   user callback returns non zero then the looping through the
   set ends, and that result is returned.                         */
TYPELIB_PROC  PTRSZVAL TYPELIB_CALLTYPE  _ForAllInSet( GENERICSET *pSet, int unitsize, int max, FAISCallback f, PTRSZVAL psv );

typedef PTRSZVAL (CPROC *FESMCallback)(INDEX,PTRSZVAL);
TYPELIB_PROC  PTRSZVAL TYPELIB_CALLTYPE  ForEachSetMember ( GENERICSET *pSet, int unitsize, int max, FESMCallback f, PTRSZVAL psv );


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

/* <combine sack::containers::sets::_ForAllInSet@GENERICSET *@int@int@FAISCallback@PTRSZVAL>
   
   \ \                                                                                       */
#define ForAllInSet(name, pset,f,psv) _ForAllInSet( (GENERICSET*)(pset), sizeof( name ), MAX##name##SPERSET, (f), (psv) )
/* <combine sack::containers::sets::ForEachSetMember@GENERICSET *@int@int@FESMCallback@PTRSZVAL>
   
   \ \                                                                                           */
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
	enum FORMAT_OPS {
      /* this segment clears to the end of the line.  Its content is then added to the output */
		FORMAT_OP_CLEAR_END_OF_LINE = 1
        ,FORMAT_OP_CLEAR_START_OF_LINE/* clear from the current cursor to the start of line */
						  
                   ,/* clear the current line */
						  FORMAT_OP_CLEAR_LINE
						 ,/* clear to the end of the page from this line */
						  FORMAT_OP_CLEAR_END_OF_PAGE
                   ,/* clear from this line to the start of the page */
						  FORMAT_OP_CLEAR_START_OF_PAGE
						 ,/* clear the entire vieable page (pushes all content to history)
                    set cursor home */
						  FORMAT_OP_CLEAR_PAGE 
						 ,/* sets option to not show text at all until next color. */
						  FORMAT_OP_CONCEAL
                   ,/* background is how many to delete. */
						  FORMAT_OP_DELETE_CHARS 
                   ,/* format.x, y are start/end of region -1,-1 clears. */
						  FORMAT_OP_SET_SCROLL_REGION 
                   ,/* this works as a transaction... */
						  FORMAT_OP_GET_CURSOR 
						 ,/* responce to getcursor... */
						  FORMAT_OP_SET_CURSOR
						 ,/* clear page, home page... result in page break... */
						  FORMAT_OP_PAGE_BREAK
						 ,/* break between paragraphs - kinda same as lines...
						  since lines are as long as possible... */
						 FORMAT_OP_PARAGRAPH_BREAK
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
   /* bit-packed flags indicating the type of format information that is applied to this segment.*/
	struct {
		// extended operation from enumeration above...
		// might shrink if more attributes are desired...
		// if many more are needed, one might consider
      // adding FONT!
     /* this segment uses the prior foreground, not its own. */
		_32 prior_foreground : 1;
     /* this segment uses the prior background, not its own. */
		_32 prior_background : 1;
     /* this segment uses the default foreground, not its own. */
		_32 default_foreground : 1;
      /* this segment uses the default background, not its own. */
		_32 default_background : 1;
      /* the foreground color of this segment (0-16 standard console text [ANSI text]) */
		_32 foreground : 4;
      /* the background color of this segment (0-16 standard console text [ANSI text]) */
		_32 background : 4;
      /* a bit indicating the text should blink if supported */
		_32 blink : 1;
      /* a bit indicating the foreground and background color should be reversed */
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
      // text is drawn wide (printer kinda font?)
		_32 wide : 1;
       // this is pretty common......
		_32 italic : 1;
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
		// 0 = default alignment 1 = left, 2 = center 3 = right
		// 0 is not set, the flag set in the lower 32 bit flags
		// is not needed any longer.... anything non zero
		// is that operation to apply.
		_32 bAlign:2;
      /* format op indicates one of the enum FORMAT_OPS applies to this segment */
		_32 format_op : 7;

	} flags;
	// if x,y are valid segment will have TF_POSFORMAT set...
	union {
		/* Coordinate information attached to a text segment. */
		/* Positioning specification of this text segment. with
		   basically 0 format options, position is used.
		   
		   Position represents the distance from this segment to the
		   prior segment in count of tabs and spaces.
		   
		   coords specifies an x,y coordinate location for the segment.
		   
		   
		   
		   Usage of this union is dependant on <link text::format_info_tag::flags@1::format_op, format_op>. */
		struct {
         // Signed coordinate of this segment on a text display.  May be relative depending on format_op.
			S_16 x;
         // Signed coordinate of this segment on a text display.  May be relative depending on format_op.
			S_16 y; 
		} coords;
		/* Defines the distance from the prior segment in count of tabs
		   and spaces (mostly count of spaces).                         */
		struct {
         _16 tabs;   // tabs preceed spaces....
			_16 spaces; // not sure what else to put with this...
		};
	} position;
} FORMAT, *PFORMAT;


#define IGNORE_CURSOR_POS -16384 // special coordinate which is NO coordinate

/* test flag, format has position data */
#define TF_FORMATPOS (TF_FORMATABS|TF_FORMATREL|TF_FORMATEX)

/* these flags are used in PTEXT.flags member
 applications may use these flags to group expressions
 will affect the BuildLine but is not generated by library.
( TF_QUOTE, TF_SQUOTE, TF_BRACKET, TF_BRACE, TF_PAREN, and TF_TAG).
*/
enum TextFlags {
 TF_STATIC    = 0x00000001,   // declared in program data.... do NOT release

 TF_QUOTE     = 0x00000002,   // double quoted string segment " "
 TF_SQUOTE    = 0x00000004,   // single quoted string ' '
 TF_BRACKET   = 0x00000008,   // bracketed expression []
 TF_BRACE     = 0x00000010,   // braced expression {}
 TF_PAREN     = 0x00000020,   // parenthised expression ()
 TF_TAG       = 0x00000040,   // HTML tag like expression &lt;&gt;

 TF_FORMATEX  = 0x00000080,   // foreground is FORMAT_OP
 TF_FORMATREL = 0x00000100,   // x,y position used (relative)
 TF_INDIRECT  = 0x00000200,   // size field extually points at PTEXT
 TF_FORMATABS = 0x00000800,   // format position is x/y - else space count
 TF_COMPLETE  = 0x00001000,   // set during burst for last segment...
 TF_BINARY    = 0x00002000,   // set for non-text variable
 TF_DEEP      = 0x00004000,   // on release release indrect also...
 TF_NORETURN  = 0x00008000,   // set on first segment to send to omit lead \r\n

// these values used originally for ODBC query construction....
// these values are flags stored on the indirect following a value
// label...
// Low bound of value...
  TF_LOWER     = 0x00010000,
// these values used originally for ODBC query construction....
// these values are flags stored on the indirect following a value
// label...
  // Upper bound of a value...
  TF_UPPER     = 0x00020000,
// these values used originally for ODBC query construction....
// these values are flags stored on the indirect following a value
// label...
// boundry may be ON this value...
 TF_EQUAL     = 0x00040000,
 
 TF_TEMP      = 0x00080000,   // this segment is not a permanent part (SubstToken)
 TF_APPLICATION = 0x00100000,  // this is something special do not treat as text indirect.
};
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

/* Description
   A Text segment, it is based on DataBlock that has a length
   and an addtional region at the end of the structure which
   contains the text of the segment. Segments may have
   formatting attributes. Segments may be linked to other
   segments in a NEXTLINE/PRIORLINE. Segments may have indirect
   content, which may represent phrases. Sets of segments may
   represent sentence diagrams. A Pointer to a <link text::TEXT, TEXT>
   type.
   
   TEXT is a type I created to provide a variety of functions.
   One particular application was a common language processor,
   and I created the TEXT structure to store elements which are
   described by language. Sentences are words, and phases. A
   phrase is a set of words, but sometimes a word is a phrase.
   
   
   
   (sentence) = ( word ) ... (phrase ) ...
   
   (phrase) = (word)...
   
   
   
   hmm.. how to describe this.
   
   <code lang="c++">
   PTEXT phrase = NULL;
   SegAppend( phrase, SegCreateFromText( "Test" ) );
   </code>
   <code>
   SegAppend( phrase, SegCreateFromText( "Test" ) );
   SegAppend( phrase, SegCreateFromText( "Test" ) );
   </code>
   
   PTEXT segments point at other segments. A list of segments is
   a sentence. Segments can have information encoded on them
   that remove text from them. For instance, \< and \> tags
   might be removed around a phrase and stored as an attribute
   of the segment. A segment with such an attribute could be an
   indirect segment that points at a list of words which are the
   phrases in the tag.
   
   <code lang="c++">
   
   a map of two segments, and their content...
   
       (segment with TF_TAG) -\> (segment with TF_TAG)
             |                        |
             \+ - ("html")             + - (body) -\> (background="#000000")
   
   
   would actually expand to
      \<html\>\<body background="#000000"\>
   
   </code>
   See Also
   SegCreate
   
   burst
   
   TextParse
   
   SegAppend
   
   SegSubst
   
   SegSplit
   
   SegGrab
   
   SegDelete
   
   LineRelease
   
   BuildLine
   
   
   
   and also.....
   
   PVARTEXT                                                                  */
typedef struct text_segment_tag
{
	// then here I could overlap with pEnt .bshadow, bmacro, btext ?
   _32 flags;  
	/* This points to the next segment in the sentence or phrase. NULL
	   if at the end of the line.                                      */
		struct text_segment_tag *Next;
	/* This points to the prior segment in the sentence or phrase. (NULL
	   if at the first segment)                                          */
		struct text_segment_tag *Prior;
	/* format is 64 bits.
      it's two 32 bit bitfields (position, expression)
	 valid if TF_FORMAT is set... */
	FORMAT format; 
   /* A description of the data stored here.  It is compatible with a DATABLOCk.... */
   struct {
	   /* unsigned size; size is sometimes a pointer value...
                  this means bad thing when we change platforms... Or not, since we went to PTRSZVAL which is big enough for a pointer. */
		PTRSZVAL size;
		/* the data of the test segment 
		 beginning of var data - this is created size+sizeof(TEXT) */
	   	TEXTCHAR  data[1]; 
	} data; 
} TEXT, *PTEXT;

//
// PTEXT DumpText( PTEXT somestring )
//    PTExT (single data segment with full description \r in text)
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  DumpText ( PTEXT text );
//SegCreateFromText( ".." );
// Burst, SegAppend, SegGrab
// segments are ment to be lines, the meaninful tag "TF_NORETURN" means it's part of the prior line.

//--------------------------------------------------------------------------


#define HAS_WHITESPACE(pText) ( (pText)->format.position.spaces || (pText)->format.position.tabs )

/* A convenient macro to go from one segment in a line of text
   to the next segment.                                        */
#define NEXTLINE(line)   ((PTEXT)(((PTEXT)line)?(((PTEXT)line)->Next):(NULL)))
/* A convenient macro to go from one segment in a line of text
   to the prior segment.                                       */
#define PRIORLINE(line)  ((PTEXT)(((PTEXT)line)?(((PTEXT)line)->Prior):(NULL)))

/* Link one PTEXT segment to another. Sets one half of the links
   appropriate for usage.
   
   
   Example
   This example sets the prior pointer of 'word' to 'line'.
   <code>
   PTEXT line;
   PTEXT word;
   SETPRIORLINE( word, line );
   </code>                                                       */
#define SETPRIORLINE(line,p) ((line)?(((line)->Prior) = (PTEXT)(p)):0)
/* Link one PTEXT segment to another. Sets one half of the links
   appropriate for usage.
   
   
   Example
   This example sets the next pointer of 'line' to 'word'.
   <code lang="c#">
   PTEXT line;
   PTEXT word;
   SETNEXTLINE( line, word );
   </code>                                                       */
#define SETNEXTLINE(line,p)  ((line)?(((line)->Next ) = (PTEXT)(p)):0)

/* Sets a pointer to PTEXT to the first text segment in the
   list.                                                    */
#define SetStart(line)     for(; line && PRIORLINE(line);line=PRIORLINE(line))
/* Sets a PTEXT to the last segment that it points to.
   Parameters
   line :  segment in the line to move to the end of.
   
   Remarks
   Updates the variable passed to point to the last segment. */
#define SetEnd(line)      for(; line && NEXTLINE(line); line=NEXTLINE(line))
// might also check to see if pseg is an indirect - setting this size would be BAD
#define SetTextSize(pseg, sz ) ((pseg)?((pseg)->data.size = (sz )):0)
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  GetIndirect(PTEXT segment );

TYPELIB_PROC  _32 TYPELIB_CALLTYPE  GetTextFlags( PTEXT segment );
TYPELIB_PROC  INDEX TYPELIB_CALLTYPE  GetTextSize( PTEXT segment );
TYPELIB_PROC  TEXTSTR TYPELIB_CALLTYPE  GetText( PTEXT segment );
// by registering for TF_APPLICTION is set on the segment
// and flags anded with the segment flags match, the
// function is called.... the result is the actual
// segment of this - since a TF_APPLICATION is also
// TF_INDIRECT - using the size to point to some application
// defined structure instead of a PTEXT structure.
TYPELIB_PROC  void TYPELIB_CALLTYPE  RegisterTextExtension ( _32 flags, PTEXT(CPROC*)(PTRSZVAL,POINTER), PTRSZVAL );
// similar to GetIndirect - but results in the literal pointer
// instead of the text that the application may have registered to result with.
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetApplicationPointer ( PTEXT text );
/* Used to set the content of a segment to some application
   defined value. This allows a users application to store
   chunks of data in lists of text. These external chunks are
   handled like other words.
   
   
   Parameters
   text :  this is the text segment to set application data on
   p :     this is a pointer to application data               */
TYPELIB_PROC  void TYPELIB_CALLTYPE  SetApplicationPointer ( PTEXT text, POINTER p);


/* Set segment's indirect data.
   Parameters
   segment :  pointer to a TEXT segment to set the indirect content
              of.
   data :     pointer to a PTEXT to be referenced indirectly.       */
#define SetIndirect(Seg,Where)  ( (Seg)->data.size = ((PTRSZVAL)(Where)-(PTRSZVAL)NULL) )

		/* these return 1 for more(l1&gt;l2) -1 for (l1&lt;l2) and 0 for match.
       */
TYPELIB_PROC  int TYPELIB_CALLTYPE  SameText ( PTEXT l1, PTEXT l2 );
TYPELIB_PROC  int TYPELIB_CALLTYPE  LikeText ( PTEXT l1, PTEXT l2 );
/* Compares if text is like a C string. Case Sensitive.
   
   <b>Returns</b>
   
   TRUE if they are alike.
   
   FALSE if they are different.
   
   <b>Parameters</b>                                    */
TYPELIB_PROC  int TYPELIB_CALLTYPE  TextIs  ( PTEXT pText, CTEXTSTR text );
/* Compares if text is like a C string. Case insensitive (like).
   Returns
   TRUE if they are alike.
   
   FALSE if they are different.
   Parameters
   pText :  PTEXT segment to compare
   text :   C string buffer to compare against                   */
TYPELIB_PROC  int TYPELIB_CALLTYPE  TextLike  ( PTEXT pText, CTEXTSTR text );

//#define SameText( l1, l2 )  ( strcmp( GetText(l1), GetText(l2) ) )
#define textmin(a,b) ( (((a)>0)&&((b)>0))?(((a)<(b))?(a):(b)):(((a)>0)?(a):((b)>0)?(b):0) )
#ifdef __LINUX__
/* windows went with stricmp() and strnicmp(), whereas linux
   went with strcasecmp() and strncasecmp()                  */
#define strnicmp strncasecmp
/* windows went with stricmp() and strnicmp(), whereas linux
   went with strcasecmp() and strncasecmp()                  */
#define stricmp strcasecmp
#endif
//#define LikeText( l1, l2 )  ( strnicmp( GetText(l1), GetText(l2), textmin( GetTextSize(l1),
//                                                                        GetTextSize(l2) ) ) )
//#define TextIs(text,string) ( !stricmp( GetText(text), string ) )
//#define TextLike(text,string) ( !stricmp( GetText(text), string ) )

TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegCreateEx( S_32 nSize DBG_PASS );
/* Create a PTEXT with specified number of character capacity.
   
   
   Example
   <code lang="c#">
   PTEXT text = SegCreate( 10 ); 
   </code>                                                     */
#define SegCreate(s) SegCreateEx(s DBG_SRC)
/* \ \ 
   See Also
   <link DBG_PASS>
   
   <link SegCreateFromText> */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegCreateFromTextEx( CTEXTSTR text DBG_PASS );
/* Creates a PTEXT segment from a string.
   Example
   <code lang="c++">
   PTEXT line = SegCreateFromText( "Around the world in a day." );
   </code>                                                         */
#define SegCreateFromText(t) SegCreateFromTextEx(t DBG_SRC)
/* \ \ 
   See Also
   <link DBG_PASS>
   
   <link SegCreateIndirect> */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegCreateIndirectEx( PTEXT pText DBG_PASS );
/* Creates a text segment that refers to the parameter
   indirectly. The new segment is not really a clone, but a
   reference of the original PTEXT.
   
   
   Example
   <code lang="c#">
   PTEXT phrase = SegCreateIndirect( SegAppend( SegCreateFromText( "Hello" )
                                              , SegCreateFromText( "World" ) ) );
   </code>
   The resulting phrase is a single segment with no prior or
   next, but its content is "HelloWorld" if it was passed to
   buildline... it's go the content of the two text segments
   linked together, but not in its buffer. It is actually a 0
   length buffer for a TEXT segment.
                                                                                  */
#define SegCreateIndirect(t) SegCreateIndirectEx(t DBG_SRC)

/* \ \ 
   See Also
   <link DBG_PASS>
   
   <link SegDuplicate> */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegDuplicateEx( PTEXT pText DBG_PASS);
/* This duplicates a specific segment. It duplicates the first
   segment of a string. If the segment has indirect data, then
   the first segment of the indirect data is duplicated.       */
#define SegDuplicate(pt) SegDuplicateEx( pt DBG_SRC )
/* Duplicates a linked list of segments.
   
   Duplicates the structure of a line. The resulting line is an
   exact duplicate of the input line. All segments linked in
   exactly the same sorts of ways.
   Parameters
   line :  list of segments to duplicate                        */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  LineDuplicateEx( PTEXT pText DBG_PASS );
/* <combine sack::containers::text::LineDuplicateEx@PTEXT pText>
   
   \ \                                                           */
#define LineDuplicate(pt) LineDuplicateEx(pt DBG_SRC )
/* \ \ 
   See Also
   <link DBG_PASS>
   
   <link TextDuplicate> */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  TextDuplicateEx( PTEXT pText, int bSingle DBG_PASS );
/* Duplicate the whole string of text to another string with
   exactly the same content.                                 */
#define TextDuplicate(pt,s) TextDuplicateEx(pt,s DBG_SRC )

/* \ \ 
   See Also
   <link DBG_PASS>
   
   <link SegCreateFromInt> */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegCreateFromIntEx( int value DBG_PASS );
/* Creates a text segment from a 64 bit integer.
   Example
   <code>
   PTEXT number = SegCreateFromInt( 3314 );
   </code>                                       */
#define SegCreateFromInt(v) SegCreateFromIntEx( v DBG_SRC )
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegCreateFrom_64Ex( S_64 value DBG_PASS );
/* Create a text segment from a _64 bit value. (long long int) */
#define SegCreateFrom_64(v) SegCreateFrom_64Ex( v DBG_SRC )
/* \ \ 
   See Also
   <link DBG_PASS>
   
   <link SegCreateFromFloat> */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegCreateFromFloatEx( float value DBG_PASS );
/* Creates a text segment from a floating point value. Probably
   uses something like '%g' to format output. Fairly limited.
   
   
   Example
   <code lang="c++">
   PTEXT short_PI = SegCreateFromFloat( 3.14 );
   </code>                                                      */
#define SegCreateFromFloat(v) SegCreateFromFloatEx( v DBG_SRC )

                   
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegAppend   ( PTEXT source, PTEXT other );
/* Inserts a segment before another segment.
   Parameters
   what :    what to insert into the list
   before :  insert the segments before this segment
   
   Returns
   The parameter 'what'.                             */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegInsert   ( PTEXT what, PTEXT before );

TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegExpandEx (PTEXT source, int nSize DBG_PASS );  /* \ \ 
                                                                                           See Also
                                                                                           <link DBG_PASS>
                                                                                           
                                                                                           <link SegExpand> */
/* This expands a segment by a number of characters.
   
   
   Parameters
   PTEXT :  the segment to expand
   int :    count of character to expand by
   
   Returns
   A pointer to a new segment that is bigger, but has the same
   existing content.                                           */
/* Expands a text segment by a certain amount. */
#define SegExpand(s,n) SegExpandEx( s,n DBG_SRC );

TYPELIB_PROC  void TYPELIB_CALLTYPE   LineReleaseEx (PTEXT line DBG_PASS );
/* Release a line of text.
   
   A line may be a single segment.
   
   This is the proper way to dispose of PTEXT segments.
   
   Any segment in the line may be passed, the first segment is
   found, and then all segments in the line are deleted.       */
#define LineRelease(l) LineReleaseEx(l DBG_SRC )

/* \ \ 
   
   <b>See Also</b>
   
   <link DBG_PASS>
   
   <link SegRelease> */
TYPELIB_PROC  void TYPELIB_CALLTYPE  SegReleaseEx( PTEXT seg DBG_PASS );
/* Release a single segment. UNSAFE. Does not respect that it is
   in a list.
   See Also
   <link LineRelease>                                            */
#define SegRelease(l) SegReleaseEx(l DBG_SRC )

TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegConcatEx   (PTEXT output,PTEXT input,S_32 offset,S_32 length DBG_PASS);
/* <combine sack::containers::text::SegConcatEx@PTEXT@PTEXT@S_32@S_32 length>
   
   looks like it takes a peice of one segment and appends it to
   another....
   
   Needs More research to document correctly and exemplify.                   */
#define SegConcat(out,in,ofs,len) SegConcatEx(out,in,ofs,len DBG_SRC)

/* Removes a segment from a list of segments. Links what was
   prior and what was after together. Sets both next and prior
   of the segment unlinked to NULL.
   
   
   Example
   <code lang="c++">
   SegUnlink( segment );
   </code>
   Returns
   The segment passed.                                         */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegUnlink   (PTEXT segment);
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegBreak    (PTEXT segment);
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegDelete   (PTEXT segment); /* Removes a segment from a list. It also releases the segment.
                                                                      Example
                                                                      <code lang="c#">
                                                                      
                                                                      SegDelete( segment );
                                                                      </code>
                                                                      
                                                                      the result is NULL;                                          */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegGrab     (PTEXT segment); /* removes segment from any list it might be in, returns
                                                                      segment.                                              */
/* This substitutes a text segment 'this' with 'that.
   
   The segment may be substituted by a list of segments. (Replace
   the word 'the' with ('this','little','apple') ).               */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegSubst    ( PTEXT _this, PTEXT that );

/* \ \ 
   See Also
   <link DBG_PASS>
   
   <link SegSplit> */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  SegSplitEx( PTEXT *pLine, int nPos DBG_PASS);
/* Split a PTEXT segment.
   
   
   Example
   \ \ 
   <code lang="c++">
   
   PTEXT result = SegSplit( &amp;old_string, 5 );
   
   </code>
   Returns
   PTEXT new_string;
   Remarks
   the old string segment is split at the position indicated. The
   pointer to the old segment is modified to point to now two
   segments linked dynamically, each part of the segment after
   the split. If the index is beyond the bounds of the segment,
   the segment remains unmodified.                                */
#define SegSplit(line,pos) SegSplitEx( line, pos DBG_SRC )

TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  FlattenLine ( PTEXT pLine );
TYPELIB_PROC  S_64 TYPELIB_CALLTYPE  IntCreateFromSeg( PTEXT pText );
TYPELIB_PROC  S_64 TYPELIB_CALLTYPE  IntCreateFromText( CTEXTSTR p );

TYPELIB_PROC  double TYPELIB_CALLTYPE  FloatCreateFromSeg( PTEXT pText );
TYPELIB_PROC  double TYPELIB_CALLTYPE  FloatCreateFromText( CTEXTSTR p, CTEXTSTR *pp );

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
/* Tests a PTEXT segment to see if it might be a floating point
   number.                                                      */
#define IsFltNumber(p, pflt) IsSegAnyNumberEx( &(p), pflt, NULL, NULL, 0 )
/* Tests the content of a PTEXT to see if it might be a number.
   Parameters
   ppText :       pointer to PTEXT to check
   pfNumber :     pointer to double to get result of number it's
                  a float
   piNumber :     pointer to a signed 64 bit value to get the
                  \result if it's not a float.
   pbIsInt :      point to a integer \- receives boolean result
                  if the segment was an integer is TRUE else it's
                  a double.
   bUseAllSegs :  if TRUE, use all the segments starting with the
                  first, and update the pointer to the next
                  stgment. If false, use only the first segment. if
                  uses all segments, it must also use ALL
                  segments to get the number.
   
   Returns
   0 if not a number or fails.
   
   1 if a valid conversion took place.                              */
TYPELIB_PROC  int TYPELIB_CALLTYPE  IsSegAnyNumberEx ( PTEXT *ppText, double *pfNumber, S_64 *piNumber, int *pbIsInt, int bUseAllSegs );
/* <combine sack::containers::text::IsSegAnyNumberEx@PTEXT *@double *@S_64 *@int *@int>
   
   \ \                                                                                  */
#define IsSegAnyNumber(pptext, pfNum, piNum, pbIsInt) IsSegAnyNumberEx( pptext, pfNum, piNum, pbIsInt, 0 )



TYPELIB_PROC  _32 TYPELIB_CALLTYPE  GetSegmentSpaceEx ( PTEXT segment, int position, int nTabs, INDEX *tabs);
TYPELIB_PROC  _32 TYPELIB_CALLTYPE  GetSegmentSpace ( PTEXT segment, int position, int nTabSize );
TYPELIB_PROC  _32 TYPELIB_CALLTYPE  GetSegmentLengthEx ( PTEXT segment, int position, int nTabs, INDEX *tabs );
TYPELIB_PROC  _32 TYPELIB_CALLTYPE  GetSegmentLength ( PTEXT segment, int position, int nTabSize );

TYPELIB_PROC  _32 TYPELIB_CALLTYPE  LineLengthExEx( PTEXT pt, _32 bSingle, INDEX nTabsize, PTEXT pEOL );
TYPELIB_PROC  _32 TYPELIB_CALLTYPE  LineLengthExx( PTEXT pt, _32 bSingle,PTEXT pEOL );
#define LineLengthExx(pt,single,eol) LineLengthExEx( pt,single,8,eol)
/* \ \ 
   Parameters
   Text segment :  PTEXT line or segment to get the length of
   single :        boolean, if set then only a single segment is
                   measured, otherwise all segments from this to
                   the end are measured.                         */
#define LineLengthEx(pt,single) LineLengthExx( pt,single,NULL)
/* Computes the length of characters in a line, if all segments
   in the line are flattened into a single word.                */
#define LineLength(pt) LineLengthEx( pt, FALSE )
/* Collapses an indirect segment or a while list of segments
   into a single segment with content expanded. When passed to
   things like TextParse and Burst, segments have their
   positioning encoded to counters for tabs and spaces; the
   segment itself contains only text without whitespace. Buildline
   expands these segments into their plain text representation.
   Parameters
   pt :        pointer to a PTEXT segment. 
   bSingle :   if TRUE, build only the first segment. If the
               segment is indirect, builds entire content of
               indirect.
   nTabsize :  how wide tabs are. When written into a line, tabs
               are written as spaces. (maybe if 0, tabs are
               emitted directly?)
   pEOL :      the segment to use to represent an end of line. Often
               this is a SegCreate(0) segment.                       */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  BuildLineExEx( PTEXT pt, int bSingle, INDEX nTabsize, PTEXT pEOL DBG_PASS );
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  BuildLineExx( PTEXT pt, int bSingle, PTEXT pEOL DBG_PASS );
//#define BuildLineEx(b,pt,single) BuildLineEx(b,pt,single DBG_SRC )
#define BuildLineExx(from,single,eol) BuildLineExEx( from,single,8,NULL DBG_SRC )
/* <combine sack::containers::text::BuildLineExEx@PTEXT@int@INDEX@PTEXT pEOL>
   
   \ \                                                                        */
#define BuildLineEx(from,single) BuildLineExEx( from,single,8,NULL DBG_SRC )
/* Flattens all segments in a line to a single segment result. */
#define BuildLine(from) BuildLineExEx( from, FALSE,8,NULL DBG_SRC )

//
// text parse - more generic flavor of burst.
//
//static CTEXTSTR normal_punctuation=WIDE("\'\"\\({[<>]}):@%/,;!?=*&$^~#`");
// filter_to_space WIDE(" \t")
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  TextParse ( PTEXT input, CTEXTSTR punctuation, CTEXTSTR filter_tospace, int bTabs, int bSpaces  DBG_PASS );
/* normal_punctuation=WIDE("'"\\({[\<\>]}):@%/,;!?=*&amp;$^~#`");
   
   Process a line of PTEXT into another line of PTEXT, but with
   words parsed as appropriate for common language.
   Parameters
   input\ :  pointer to a list of PTEXT segments to parse.
   
   Remarks
   Burst is a simple method of breaking a sentence into its word
   and phrase parts. It collapses space and tabs before words
   into the word. Any space representation is space preceeding
   the word. Sentences are also broken on any punctuation.
   "({[\<\>]})'";;.,/?\\!@#$%^&amp;*=" for instances. + and - are
   treated specially if they prefix numbers, otherwise they are
   also punctuation. Also groups of '.' like '...' are kept
   together. if the '.' is in a number, it is stored as part of
   the number. Otherwise a '.' used in an abbreviation like P.S.
   will be a '.' with 0 spaces followed by a segment also with 0
   spaces. (unless it's the lsat one)
   
   so initials are encoded badly.
   
   
   Bugs
   There is an exploit in the parser such that . followed by a
   number will cause fail to break into seperate words. This is
   used by configuration scripts to write binary blocks, and
   read them back in, having the block parsed into a segment
   correctly.
   See Also
   <link sack::containers::text::TextParse@PTEXT@CTEXTSTR@CTEXTSTR@int@int bSpaces, TextParse> */

TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  burstEx( PTEXT input DBG_PASS);
/* <combine sack::containers::text::burstEx@PTEXT input>
   
   \ \                                                   */
#define burst( input ) burstEx( (input) DBG_SRC )

/* Compares a couple lists of text segments.
   Parameters
   pt1 :      pointer to a phrase
   single1 :  use only the first word, not the whole phrase
   pt2 :      pointer to a phrase
   single2 :  use only the first segment, not the whole phrase
   bExact :   if FALSE, match case insensitive, otherwise match
              exact case.                                       */
TYPELIB_PROC  int TYPELIB_CALLTYPE  CompareStrings( PTEXT pt1, int single1
                            , PTEXT pt2, int single2
                            , int bExact );


/* This removes indirect segments, replacing them with their
   indirect content.
   Parameters
   pLine :  pointer to a PTEXT segment list to flatten.      */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  FlattenLine ( PTEXT pLine );

#define FORALLTEXT(start,var)  for(var=start;var; var=NEXTLINE(var))

TYPELIB_PROC  char * TYPELIB_CALLTYPE  WcharConvert ( const wchar_t *wch );

//--------------------------------------------------------------------------

/* This is a string collector type.  It has an interface to be able to vtprintf( vartext, "format string", ... ); which appends the specified string to the collected text.
  Example
   PVARTEXT pvt = VarTextCreate();
   vtprintf( pvt, "hello world!" );
   {
      PTEXT text = VarTextGet( pvt );
	  printf( "Text is : %s(%d)", GetText( text ), GetTextSize( text ) );
	  LineRelease( text );
   }
   VarTextDestroy( &pvt );
   */
typedef struct vartext_tag *PVARTEXT;

TYPELIB_PROC  PVARTEXT TYPELIB_CALLTYPE  VarTextCreateExEx ( _32 initial, _32 expand DBG_PASS );
/* <combine sack::containers::text::VarTextCreateExEx@_32@_32 expand>
   
   \ \                                                                */
#define VarTextCreateExx(i,e) VarTextCreateExEx(i,e DBG_SRC )
TYPELIB_PROC  PVARTEXT TYPELIB_CALLTYPE  VarTextCreateEx ( DBG_VOIDPASS );
/* The simplest, most general way to create a PVARTEXT
   collector. The most extended vartext creator allows
   specification of how long the initial buffer is, and how much
   the buffer expands by when required. This was added to
   optimize building HUGE SQL queries, working withing 100k
   buffers that expanded by 50k at a time was a lot less
   operations than expanding 32 bytes or something at a time.    */
#define VarTextCreate() VarTextCreateEx( DBG_VOIDSRC )
TYPELIB_PROC  void TYPELIB_CALLTYPE  VarTextDestroyEx ( PVARTEXT* DBG_PASS );
/* Destroy a VarText collector. */
#define VarTextDestroy(pvt) VarTextDestroyEx( pvt DBG_SRC )

TYPELIB_PROC  void TYPELIB_CALLTYPE  VarTextInitEx( PVARTEXT pvt DBG_PASS);
/* Probably should not be exported. Initializes a VARTEXT
   structure to prepare it for subsequent VarText operations. */
#define VarTextInit(pvt) VarTextInitEx( (pvt) DBG_SRC )
/* Empties a PVARTEXT structure.
   Parameters
   pvt :  PVARTEXT to empty.     */
TYPELIB_PROC  void TYPELIB_CALLTYPE  VarTextEmptyEx( PVARTEXT pvt DBG_PASS);
/* <combine sack::containers::text::VarTextEmptyEx@PVARTEXT pvt>
   
   \ \                                                           */
#define VarTextEmpty(pvt) VarTextEmptyEx( (pvt) DBG_SRC )
TYPELIB_PROC  void TYPELIB_CALLTYPE  VarTextAddCharacterEx( PVARTEXT pvt, TEXTCHAR c DBG_PASS );
/* Adds a single character to a PVARTEXT collector.
   
   
   Example
   <code lang="c++">
   PVARTEXT pvt = VarTextCreate();
   VarTextAddCharacter( pvt, 'a' );
   </code>                                          */
#define VarTextAddCharacter(pvt,c) VarTextAddCharacterEx( (pvt),(c) DBG_SRC )
/* Commits the currently collected text to segment, and adds the
   segment to the internal line accumulator.                     
		 returns true if any data was added...
       move any collected text to commit... */
TYPELIB_PROC  int TYPELIB_CALLTYPE  VarTextEndEx( PVARTEXT pvt DBG_PASS ); 
/* <combine sack::containers::text::VarTextEndEx@PVARTEXT pvt>
   
   \ \                                                         */
#define VarTextEnd(pvt) VarTextEndEx( (pvt) DBG_SRC )
TYPELIB_PROC  int TYPELIB_CALLTYPE  VarTextLength( PVARTEXT pvt );
/* Gets the text segment built in the VarText. The PVARTEXT is
   set to empty. Clears the collector.
   Parameters
   pvt :  PVARTEXT to get text from.                           */
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  VarTextGetEx( PVARTEXT pvt DBG_PASS );
/* <combine sack::containers::text::VarTextGetEx@PVARTEXT pvt>
   
   \ \                                                         */
#define VarTextGet(pvt) VarTextGetEx( (pvt) DBG_SRC )
TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  VarTextPeekEx ( PVARTEXT pvt DBG_PASS );
/* \Returns the PTEXT that is currently in a PVARTEXT. It does
   not alter the contents of the PVARTEXT. Do not LineRelease
   this peeked value.                                          */
#define VarTextPeek(pvt) VarTextPeekEx( (pvt) DBG_SRC )
TYPELIB_PROC  void TYPELIB_CALLTYPE  VarTextExpandEx( PVARTEXT pvt, int size DBG_PASS );
/* Add a specified number of characters to the amount of space
   in the VARTEXT collector.                                   */
#define VarTextExpand(pvt, sz) VarTextExpandEx( (pvt), (sz) DBG_SRC )

//TYPELIB_PROC  int vtprintfEx( PVARTEXT pvt DBG_PASS TYPELIB_CALLTYPE  CTEXTSTR format, ... ;
// note - don't include format - MUST have at least one parameter passed to ...
//#define vtprintf(pvt, ...) vtprintfEx( (pvt) DBG_SRC, __VA_ARGS__ )

TYPELIB_PROC  int TYPELIB_CALLTYPE  vtprintfEx( PVARTEXT pvt, CTEXTSTR format, ... );
/* <combine sack::containers::text::vtprintfEx@PVARTEXT@CTEXTSTR@...>
   
   Note                                                               */
#define vtprintf vtprintfEx

TYPELIB_PROC  int TYPELIB_CALLTYPE  vvtprintf( PVARTEXT pvt, CTEXTSTR format, va_list args );

//--------------------------------------------------------------------------
// extended command entry stuff... handles editing buffers with insert/overwrite/copy/paste/etc...

typedef struct user_input_buffer_tag {
// -------------------- custom cmd buffer extension 
   int nHistory;  // position counter for pulling history
   PLINKQUEUE InputHistory;  // a link queue which contains the prior lines of text entered for commands.
   int   bRecallBegin; // set to TRUE when nHistory has wrapped...

   /* A exchange-lock variable for controlling access to the
      \history (so things aren't being read from it while it is
      scrolling old data out).                                  */
   _32   CollectionBufferLock;
   INDEX CollectionIndex;  // used to store index.. for insert type operations...
   int   CollectionInsert; // flag for whether we are inserting or overwriting
	PTEXT CollectionBuffer; // used to store partial from GatherLine
	void (CPROC*CollectedEvent)( PTRSZVAL psv, PTEXT text ); // called when a buffer is complete.
   PTRSZVAL psvCollectedEvent;  // passed to the event callback when a line is completed
} USER_INPUT_BUFFER, *PUSER_INPUT_BUFFER;

TYPELIB_PROC  PUSER_INPUT_BUFFER TYPELIB_CALLTYPE  CreateUserInputBuffer ( void );

TYPELIB_PROC  void TYPELIB_CALLTYPE  DestroyUserInputBuffer ( PUSER_INPUT_BUFFER *pci );

// negative with SEEK_SET is SEEK_END -nPos
enum CommandPositionOps {
	// defined that the x,y position in the segment should be used for absolute positioning.
   // can also be SEEK_SET
 COMMAND_POS_SET = 0,
 // defined that the x,y position in the segment should be used for relative positioning.
 // can also be SEEK_CUR
 COMMAND_POS_CUR = 1
};
/* Updates the current input position, for things like input,
   etc. Some external process indicates where in the line to set
   the cursor position.                                          */
TYPELIB_PROC  int TYPELIB_CALLTYPE  SetUserInputPosition ( PUSER_INPUT_BUFFER pci, int nPos, int whence );

// bInsert < 0 toggle insert.  bInsert == 0 clear isnert(set overwrite) else
// set insert (clear overwrite )
TYPELIB_PROC  void TYPELIB_CALLTYPE  SetUserInputInsert ( PUSER_INPUT_BUFFER pci, int bInsert );
TYPELIB_PROC  void TYPELIB_CALLTYPE  SetUserInputInsert ( PUSER_INPUT_BUFFER pci, int bInsert );

TYPELIB_PROC  void TYPELIB_CALLTYPE  RecallUserInput ( PUSER_INPUT_BUFFER pci, int bUp );
TYPELIB_PROC  void TYPELIB_CALLTYPE  EnqueUserInputHistory ( PUSER_INPUT_BUFFER pci, PTEXT pHistory );

TYPELIB_PROC  PTEXT TYPELIB_CALLTYPE  GatherUserInput ( PUSER_INPUT_BUFFER pci, PTEXT stroke );

#ifdef __cplusplus
}; //namespace text {
#endif


//--------------------------------------------------------------------------
#ifdef __cplusplus
	namespace BinaryTree {
#endif
/* This type defines a specific node in the tree. It is entirely
   private, and is a useless definition.                         */
typedef struct treenode_tag *PTREENODE;
/* Defines a Binary Tree.
   See Also
   <link CreateBinaryTree> */
typedef struct treeroot_tag *PTREEROOT;

/* This option may be passed to extended CreateBinaryTree
   methods to disallow adding of duplicates. Otherwise
   duplicates will be added; they will be added to the side of
   the node with the same value that has less children. Trees
   are created by default without this option, allowing the
   addition of duplicates.
   
   
   Example
   <code lang="c++">
   
   PTREEROOT = <link CreateBinaryTreeExtended>( BT_OPT_NODUPLICATES, NULL, NULL DBG_SRC );
   
   </code>                                                                                 */
#define BT_OPT_NODUPLICATES 1

/* Generic Compare is the type declaration for the callback routine for user custom comparisons.  
  This routine should return -1 if new is less than old, it should return 1 if new is more than old, and it 
  should return 0 if new and old are the same key. */
typedef int (CPROC *GenericCompare)( PTRSZVAL oldnode,PTRSZVAL newnode );
/* Signature for the user callback passed to CreateBinaryTreeEx
   that will be called for each node removed from the binary
   list.                                                        */
typedef void (CPROC *GenericDestroy)( POINTER user, PTRSZVAL key);

/* when adding a node if Compare is NULL the default method of a
   basic unsigned integer compare on the key value is done. if
   Compare is specified the specified key value of the orginal
   node (old) and of the new node (new) is added. Result of
   compare should be ( \<0 (lesser)) ( 0 (equal)) ( \>0
   (greater))
   
   
   Example
   <code lang="c++">
   int CPROC MyGenericCompare( PTRSZVAL oldnode,PTRSZVAL newnode )
   {
   </code>
   <code>
      if(oldnode\>newnode)
          return 1;
      else if(oldnode\<newnode)
          return -1;
      else return 0;
   
   </code>
   <code lang="c++">
      return (oldnode\>newnode)? 1
             \:(oldnode\<newnode)? -1
             \:0;
   }
   void CPROC MyGenericDestroy(POINTER user, PTRSZVAL key)
   {
      // do something custom with your user data and or key value
   }
   
   PTREEROOT tree = CreateBinaryTreeExtended( 0 // BT_OPT_NODUPLICATES
                                            , MyGenericCompare
                                            , MyGenericDestroy
                                            <link DBG_PASS, DBG_SRC> );
   
   </code>
   
   See Also
   <link CreateBinaryTreeExx>
   
   <link CreateBinaryTreeEx>
   
   <link CreateBinaryTree>                                               */
TYPELIB_PROC  PTREEROOT TYPELIB_CALLTYPE  CreateBinaryTreeExtended( _32 flags
															, GenericCompare Compare
															, GenericDestroy Destroy DBG_PASS);
/* This is the simpler case of <link CreateBinaryTreeExtended>,
   which does not make you pass DBG_SRC.
   
   
   
   
   Example
   <code lang="c++">
   
   PTREEROOT tree = CreateBinaryTreeExx( BT_OPT_NODUPLICATES, NULL, NULL );
   </code>                                                                  */
#define CreateBinaryTreeExx(flags,compare,destroy) CreateBinaryTreeExtended(flags,compare,destroy DBG_SRC)
/* Creates a binary tree, allowing specification of comparison
   and destruction routines.
   
   
   Example
   <code lang="c++">
   
   PTREEROOT tree = CreateBinaryTreeEx( <link CreateBinaryTreeExtended, MyGenericCompare>, <link CreateBinaryTreeExtended, MyGenericDestroy> );
   
   </code>                                                                                                                                      */
#define CreateBinaryTreeEx(compare,destroy) CreateBinaryTreeExx( 0, compare, destroy )

/* This is the simplest way to create a binary tree.
   
   The default compare routine treats 'key' as an integer value
   that is compared against other for lesser/greater condition.
   
   This tree also allows duplicates to be added.
   
   
   Example
   <code lang="c++">
   
   PTREEROOT tree = CreateBinaryTree();
   
   </code>                                                      */
#define CreateBinaryTree() CreateBinaryTreeEx( NULL, NULL )

/* \ \ 
   Example
   <code lang="c++">
   
   PTREEROOT tree = CreateBinaryTree();
   
   DestroyBinaryTree( tree );
   tree = NULL;
   
   </code>                              */
TYPELIB_PROC  void TYPELIB_CALLTYPE  DestroyBinaryTree( PTREEROOT root );

/* Balances a binary tree. If data is added to a binary list in
   a linear way (from least to most), the tree can become
   unbalanced, and all be on the left or right side of data. This
   routine can analyze branches and perform rotations so that
   the tree can be discretely rebalanced.
   
   
   Example
   <code lang="c++">
   <link PTREEROOT> tree;
   
   // <link AddBinaryNode>...
   BalanceBinaryTree( tree );
   </code>                                                        */
TYPELIB_PROC  void TYPELIB_CALLTYPE  BalanceBinaryTree( PTREEROOT root ); 

/* \ \ 
   See Also
   <link AddBinaryNode>
   
   <link DBG_PASS>
                        */
TYPELIB_PROC  int TYPELIB_CALLTYPE  AddBinaryNodeEx( PTREEROOT root
                                    , POINTER userdata
											  , PTRSZVAL key DBG_PASS );
/* Adds a user pointer identified by key to a binary list.
   
   
   See Also
   <link BinaryTree::CreateBinaryTree, CreateBinaryTree>
   Example
   <code lang="c++">
   
   PTREEROOT tree = CreateBinaryTree();
   
   PTRSZVAL key = 1;
   POINTER data = NewArray( TEXTCHAR, 32 );
   
   AddBinaryNode( tree, data, key );
   
   </code>
   Parameters
   root :  PTREEROOT binary tree instance.
   data :  POINTER to some user object.
   key :   PTRSZVAL a integer type which can be used to identify
           the data. (used to compare in the tree).<p /><p />If
           the user has specified a custom comparison routine in
           an extended CreateBinaryTree(), then this value might
           be a pointer to some other data. Often the thing used
           to key into a binary tree is a <link CTEXTSTR>.
   Returns
   The tree may be created with <link BT_OPT_NODUPLICATES>, in
   which case this will result FALSE if the key is found
   duplicated in the list. Otherwise this returns TRUE. if the
   root parameter is NULL, the result is FALSE.                  */
#define AddBinaryNode(r,u,k) AddBinaryNodeEx((r),(u),(k) DBG_SRC )
//TYPELIB_PROC  int TYPELIB_CALLTYPE  AddBinaryNode( PTREEROOT root
//                                    , POINTER userdata
//                                    , PTRSZVAL key );

TYPELIB_PROC  void TYPELIB_CALLTYPE  RemoveBinaryNode( PTREEROOT root, POINTER use, PTRSZVAL key );

/* Search in a binary tree for the specified key.
   Returns
   user data POINTER if found, else NULL.
   
   
   Example
   <code lang="c++">
   
   PTREEROOT tree;
   
   void f( void )
   {
      POINTER mydata = FindInBinaryTree( tree, 5 );
      if( mydata )
      {
          // found '5' as the key in the tree
      }
   }
   </code>                                          */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  FindInBinaryTree( PTREEROOT root, PTRSZVAL key );


// result of fuzzy routine is 0 = match.  100 = inexact match
// 1 = no match, actual may be larger
// -1 = no match, actual may be lesser
// 100 = inexact match- checks nodes near for better match.
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  LocateInBinaryTree( PTREEROOT root, PTRSZVAL key
														, int (CPROC*fuzzy)( PTRSZVAL psv, PTRSZVAL node_key ) );


/* During FindInBinaryTree and LocateInBinaryTree, the last
   found result is stored. This function allows deletion of that
   node.
   
   
   Example
   <code lang="c++">
   FindInBinaryTree( tree, 5 );
   RemoveLastFoundNode( tree );
   </code>                                                       */
TYPELIB_PROC  void TYPELIB_CALLTYPE  RemoveLastFoundNode(PTREEROOT root );
/* Removes the currently browsed node from the tree.
   
   
   See Also
   <link GetChildNode>                               */
TYPELIB_PROC  void TYPELIB_CALLTYPE  RemoveCurrentNode(PTREEROOT root );

/* Basically this is meant to dump to a log, if the print
   function is passed as NULL, then the tree's contents are
   dumped to the log. It dumps a very cryptic log of how all
   nodes in the tree are arranged. But by allowing the user to
   provide a method to log his data and key, the logging is more
   meaningful based on the application. The basic code for
   managing trees and nodes works....
   
   
   Example
   <code>
   
   int ForEachNode( POINTER user, PTRSZVAL key )
   {
       // return not 1 to dump to log the internal tree structure
       return 0; // probably did own logging here, so don't log tree internal
   }
   
   <link PTREEROOT> tree;
   
   void f( void )
   {
       DumpTree( tree, ForEachNode );
   }
   
   </code>                                                                    */
TYPELIB_PROC  void TYPELIB_CALLTYPE  DumpTree( PTREEROOT root 
                          , int (*Dump)( POINTER user, PTRSZVAL key ) );


/* See Also
   <link GetChildNode> */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetLeastNode( PTREEROOT root );
/* See Also
   <link GetChildNode> */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetGreatestNode( PTREEROOT root );
/* See Also
   <link GetChildNode> */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetLesserNode( PTREEROOT root );
/* See Also
   <link GetChildNode> */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetGreaterNode( PTREEROOT root );
/* \Returns the node that is set as 'current' in the tree. There
   is a cursor within the tree that can be used for browsing.
   See Also
   <link GetChildNode>                                           */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetCurrentNode( PTREEROOT root );

/* This sets the current node cursor to the root of the node.
   See Also
   <link GetChildNode>                                        */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetRootNode( PTREEROOT root );
/* See Also
   <link GetChildNode> */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetParentNode( PTREEROOT root );
/* While browsing the tree after a find operation move to the
   next child node, direction 0 is lesser direction !0 is
   greater.
   
   
   
   Binary Trees have a 'current' cursor. These operations may be
   used to browse the tree.
   
   
   Example
   \ \ 
   <code>
   
   // this assumes you have a tree, and it's fairly populated, then this demonstrates
   // all steps of browsing.
   
   POINTER my_data;
   
   // go to the 'leftmost' least node. (as determined by the compare callback)
   my_data = GetLeastNode( tree );
   
   // go to the 'rightmost' greatest node. (as determined by the compare callback)
   my_data = GetGreatestNode( tree );
   
   // move to the node that is less than the current node.  (move to the 'left')
   my_data = GetLesserNode( tree );
   
   // move to the node that is greater than the current node.  (move to the 'right')
   my_data = GetGreaterNode( tree );
   
   // follow the tree to the left down from here
   my_data = GetChildNode( tree, 0 );
   
   // follow the tree to the right down from here
   my_data = GetChildNode( tree, 1 );
   
   // follow the tree up to the node above the current one.
   //  (the one who's lesser or greater points at this)
   my_data = GetParentNode( tree );
   
   // this is probably the least useful, but someone clever might find a trick for it
   // Move back to the node we were just at.
   //  (makes the current the prior, and moves to what the prior was,
   //     but then it's just back and forth between the last two; it's not a stack ).
   my_data = GetPriorNode( tree );
   
   </code>
   
   A more practical example...
   <code lang="c++">
   
   POINTER my_data;
   for( my_data = GetLeastNode( tree );
        my_data;
        my_data = GetGreaterNode( tree ) )
   {
        // browse the tree from least to most.
   }
   
   </code>                                                                            */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetChildNode( PTREEROOT root, int direction );
/* See Also
   <link GetChildNode> */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  GetPriorNode( PTREEROOT root );

/* \Returns the total number of nodes in the tree.
   
   
   Example
   <code lang="c++">
   _32 total_nodes = GetNodeCount(tree);
   </code>                                         */
TYPELIB_PROC  _32 TYPELIB_CALLTYPE  GetNodeCount ( PTREEROOT root );

TYPELIB_PROC  PTREEROOT TYPELIB_CALLTYPE  ShadowBinaryTree( PTREEROOT root ); // returns a shadow of the original.

#ifdef __cplusplus
	}; //namespace BinaryTree {
#endif

//--------------------------------------------------------------------------

#ifdef __cplusplus
namespace family {
#endif
typedef struct familyroot_tag *PFAMILYTREE;
/* <unfinished>
   
   Incomplete Work in progress (maybe) */
TYPELIB_PROC  PFAMILYTREE TYPELIB_CALLTYPE  CreateFamilyTree ( int (CPROC *Compare)(PTRSZVAL key1, PTRSZVAL key2)
															, void (CPROC *Destroy)(POINTER user, PTRSZVAL key) );
/* <unfinished>
   
   Incomplete, Family tree was never completed. */
TYPELIB_PROC  POINTER TYPELIB_CALLTYPE  FamilyTreeFindChild ( PFAMILYTREE root
														  , PTRSZVAL psvKey );
/* <unfinished>
   
   Incomplete, Family tree was never completed. */
TYPELIB_PROC  void TYPELIB_CALLTYPE  FamilyTreeReset ( PFAMILYTREE *option_tree );
/* <unfinished>
   
   Incomplete Work in progress (maybe) */
TYPELIB_PROC  void TYPELIB_CALLTYPE  FamilyTreeAddChild ( PFAMILYTREE *root, POINTER userdata, PTRSZVAL key );
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
using namespace sack::containers::data_list;
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
//
// Revision 1.39  2003/03/25 08:38:11  panther
// Add logging
//
