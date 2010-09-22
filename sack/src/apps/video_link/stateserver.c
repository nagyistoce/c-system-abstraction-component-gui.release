/*
 *  stateserver.c
 *  Copyright:   2006
 *  Product:  BingLink      Project: alpha2server   Version: Alpha2
 *  Contributors:    Jim Buckeyne, Christopher Green
 *  Checks the state of the database and responds appropriately.
 *
 */

#define DECLARE_GLOBAL
#define DECLARE_LOCAL
#include "server.h"

#include "link_events.h"
#include <systray.h>
//#include <bard.h>

#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#include <InterShell/intershell_export.h>
#include <InterShell/intershell_registry.h>
#include "db.h"

#define VIDEOADDRFIELD "address"
#define VIDEOADDRTABLE "systems"

// some locally-global variables. yes, it's dirty cheese.
typedef struct noise_tag{

	_32 bHolyCow:1;
	_32 bExcuseMe:1;
	_32 bGetOuttaHere:1;
	_32 bBeingGreen:1;
	_32 bWhoaTaskLaunched:1;
}NOISESTRUCT;

NOISESTRUCT noise;

#ifdef __WINDOWS__
#define MEDIA_ROOT_PATH "c:/ftn3000/etc/images/"
#else
#define MEDIA_ROOT_PATH "/storage/media/"
#endif

typedef struct playlist_tag{
	_32 hall_id;
	TEXTCHAR name[1024];
	TEXTCHAR simplename[1024];
} PLAYLIST, *PPLAYLIST;
//Visit http://hanna.pyxidis.org/tech/m3u.html for extended m3u description.

void UpdateLinkButtons( void );

void ClearError( void )
{
	ShellSetCurrentPage( "first" );
}

void DogastrophicErrorExx( int level, char *page, char * message DBG_PASS)
#define DogastrophicError(n, page) DogastrophicErrorExx(n, page, "Error." DBG_SRC)
#define DogastrophicErrorEx(n,page,m) DogastrophicErrorExx(n,page,m DBG_SRC)
{

	switch( level )
	{

	case 1:
		{
			_xlprintf(LOG_ALWAYS DBG_RELAY)("\n\n\n                    This is a failure condition. \n%s\n             I'm sorry it didn't work out.  \n\n             Goodbye."
													 , message
													 );
			break;
		}
	default:
		{
			_xlprintf(LOG_ALWAYS DBG_RELAY)("\n\n\nDogastrophicError: %s  \n\nGoodbye.\n\n\n"
													 , message
													 );
			break;
		}
	}
	fprintf( stderr
			 , "\n\n\n                    This is a failure condition. \n%s\n             I'm sorry it didn't work out.  \n\n             Goodbye."
			 , message );

	//Sleep(5000);
	//exit(0);
	ShellSetCurrentPage( page );
   WakeableSleep( 5000 );
}

void DogastrophicErrorEx_(int level DBG_PASS )
{

	DogastrophicErrorExx( level , "Generic Error", "Error." DBG_RELAY );
}

void CPROC DoTaskOutput( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR text, _32 length )
{
   lprintf( "%s", text );
}


PBINGHALL IsMasterReady( void )
{
	PBINGHALL pHall;
	INDEX idx;
	LIST_FORALL( l.pHallList, idx, PBINGHALL, pHall )
	{
		if( l.current_state.LinkState.delegated_master_hall_id )
		{
			if( pHall->LinkHallState.hall_id == l.current_state.LinkState.delegated_master_hall_id )
			{
				if( pHall->LinkHallState.delegate_ready )
					return pHall;
				else
					return NULL;
			}
		}
		else
		{
			if( pHall->LinkHallState.hall_id == l.current_state.LinkState.master_hall_id )
			{
				if( pHall->LinkHallState.master_ready )
				{
					return pHall;
				}
				else
					return NULL;
			}
		}
	}
	return NULL;
}

#define DefineInvokeMethod(name,return_type,args,i_args,...) return_type Invoke##name i_args \
{                                                                     \
	CTEXTSTR result;                                                    \
	PCLASSROOT data = NULL;                                                 \
	lprintf( "Invoking %s", #name );                                        \
	for( result = GetFirstRegisteredName( WIDE( "video link/server core/Command" #name ), &data ); \
		 result;                                                                              \
		  result = GetNextRegisteredName( &data ) )                                            \
	{                                                                                          \
		return_type (CPROC*f)args;                                                                    \
		{                                                                                      \
			PCLASSROOT root = GetClassRootEx( data, result );                                      \
			CTEXTSTR file = GetRegisteredValue( (CTEXTSTR)root, "Source File" );                  \
			int line = (int)(PTRSZVAL)GetRegisteredValueEx( (CTEXTSTR)root, "Source Line", TRUE ); \
			lprintf( "invoking %s handler at %s in %s(%d)", #name, result, file, line );           \
		}                                                                                          \
		f = GetRegisteredProcedure2( data, return_type, result, args );                                \
		if( f )                                                                                    \
	f(__VA_ARGS__);                                                                          \
   else lprintf( "no function" ); \
	}                                                                                               \
}

#define Mark(name,flag_name,sql_col_name,state,important) \
static void CPROC Mark##name( void )            \
{                                               \
	l.pMyHall->LinkHallState.flag_name = state;      \
	if( !g.flags.bReadOnly )                     \
	{                                            \
	SQLCommandf( g.odbc,"UPDATE link_hall_state SET "sql_col_name"="#state" WHERE hall_id=%d", l.pMyHall->LinkHallState.hall_id ); \
   if( important ) InvokeStateChanged(important);                                                                              \
	}                                                                                                                         \
}

// event specific for event-peer-notification module
DefineInvokeMethod( StateChanged, void, (CTEXTSTR), (CTEXTSTR message), message );


Mark( MasterServing, master_ready, "master_ready", 1, "MASTER_READY" );
Mark( DelegateServing, delegate_ready, "delegate_ready", 1, "DELEGATE_READY" );
Mark( Participating, participating, "participating", 1, "PARTICIPANT_READY" );
Mark( MasterEnded, master_ready, "master_ready", 0, "MASTER_DONE" );
Mark( DelegateEnded, delegate_ready, "delegate_ready", 0, "DELEGATE_DONE" );
Mark( ParticipantEnded, participating, "participating", 0, "PARTICIPANT_DONE" );
Mark( TaskStarting, task_launched, "task_launched", 1, 0 );
Mark( TaskDone, task_launched, "task_launched", 0, 0 );


static void CPROC StateChanged( void )
{
   WakeThread( l.check_state_thread );
}


struct video_server_interface VideoServerInterface = { MarkTaskStarting, MarkTaskDone
																	  , MarkMasterServing, MarkDelegateServing
																	  , MarkParticipating
																	  , MarkMasterEnded, MarkDelegateEnded
																	  , MarkParticipantEnded
                                                     , StateChanged

 };

static POINTER CPROC GetVideoServerInterface( void )
{
   return (POINTER)&VideoServerInterface;
}


int IsOldMasterReady( void )
{
	PBINGHALL pHall;
	INDEX idx;
	LIST_FORALL( l.pHallList, idx, PBINGHALL, pHall )
	{
		if( pHall->LinkHallState.master_ready )
			return TRUE;
	}
	return FALSE;
}

PBINGHALL CreateAHall( INDEX myid, int hall_id )
{
	CTEXTSTR *sequelresults = NULL; //yes a pointer
	PBINGHALL pBingHall = NULL;
   PushSQLQueryEx(g.odbc);
	for (   SQLRecordQueryf( g.odbc, NULL, &sequelresults, NULL, "SELECT id,packed_name,address_bdata,address_video FROM location where id=%d", hall_id );
		  sequelresults ;
			  FetchSQLRecord( g.odbc, &sequelresults )
		 )
	{
		if( ( !sequelresults[1] ) ||
			( !sequelresults[1][0] )
		  )
		{
			continue;
		}
		{
			_32 buflen = ( strlen(  sequelresults[1] ) + strlen( "bdata." ) + 1 );//adding one for the null
			TEXTSTR pChar = NewArray( TEXTCHAR, buflen ); // anfassen - freigabe
			pBingHall = New( BINGHALL );
			MemSet( pBingHall, 0, sizeof( BINGHALL ) );

			pBingHall->LinkHallState.hall_id = atoi( sequelresults[0] );
			pBingHall->stIdentity.szSiteName  =  StrDup( sequelresults[1] );
			if( pBingHall->stIdentity.szSiteName )
				xlprintf(LOG_ADVISORY)("I am officially %s ( %u )", pBingHall->stIdentity.szSiteName ,pBingHall->LinkHallState.hall_id );
			lprintf( "I am [%s] [%s] %d %d", l.hall_name, pBingHall->stIdentity.szSiteName, myid, pBingHall->LinkHallState.hall_id );
			if( (l.hall_name &&
				  ( ( strcasecmp( pBingHall->stIdentity.szSiteName, l.hall_name ) ) == 0 ) )||
				( ( !l.hall_name) &&
				 ( myid == pBingHall->LinkHallState.hall_id ) )
			  )
			{
				lprintf("Assigning pBingHall to l.pMyHall ");
				l.pMyHall = pBingHall;
			}
			if( sequelresults[2] )
			{
				pBingHall->stIdentity.szBdataAddr = StrDup( sequelresults[2] );
				xlprintf(LOG_NOISE)("%s is BdataAddr"
										 , pBingHall->stIdentity.szBdataAddr
										 );
			}
			else
			{
				snprintf(pChar, buflen
						  , "bdata.%s"
						  , pBingHall->stIdentity.szSiteName
						  );
				pBingHall->stIdentity.szBdataAddr = StrDup( pChar );
				xlprintf(LOG_NOISE)("BOO! No BdataAddr!  Using %s"
										 , pBingHall->stIdentity.szBdataAddr
										 );
				if( !SQLCommandf( g.odbc,"UPDATE location SET address_bdata=\'%s\' WHERE id=%u"
										, pBingHall->stIdentity.szBdataAddr
										, pBingHall->LinkHallState.hall_id
										)
				  )
				{
					DogastrophicErrorEx( 0, "Update Failed", "Cannot get to location table? check sql.log.  Bailing.");
				}
				else
				{
					xlprintf(LOG_NOISE)("made a new entry into location for %s"
											 , pBingHall->stIdentity.szBdataAddr
											 );
				}
			}
			if( sequelresults[3] )
			{
				pBingHall->stIdentity.szVideoAddr = StrDup( sequelresults[3] );
				xlprintf(LOG_NOISE)("%s is VideoAddr"
										 , pBingHall->stIdentity.szVideoAddr
										 );
			}
			else
			{
				snprintf(pChar, buflen
						  , "vsrvr.%s"
						  , pBingHall->stIdentity.szSiteName
						  );
				pBingHall->stIdentity.szVideoAddr = StrDup( pChar );
				xlprintf(LOG_NOISE)("BOO! No VideoAddr!  Using %s"
										 , pBingHall->stIdentity.szVideoAddr
										 );
				if( !SQLCommandf( g.odbc,"UPDATE location SET address_video=\'%s\' WHERE id=%u"
										, pBingHall->stIdentity.szVideoAddr
										, pBingHall->LinkHallState.hall_id
										)
				  )
				{
					DogastrophicErrorEx( 0, "Update Failed", "Cannot get to location table? check sql.log.  Bailing.");
				}
				else
				{
					xlprintf(LOG_NOISE)("made a new entry into location for %s"
											 , pBingHall->stIdentity.szVideoAddr
											 );
				}
			}

			pBingHall->LinkHallState.flags.uiBdataFailures = 0;

			AddLink( &l.pHallList, pBingHall );
		}
	}
	PopODBCEx( g.odbc );
   return pBingHall;
}



//-------------------------------------------------------------------------------
//    RudelyInterrupted
//    Returns TRUE if the master is NOT ENABLED or IS PROHIBITED.
//    This is a sanity check to determine if the master was enabled
//    at one time but now is not because it immediately changed states.
//-------------------------------------------------------------------------------
int RudelyInterrupted( void )
{
	PBINGHALL pHall;
	INDEX idx;
	INDEX lookfor = l.current_state.LinkState.master_hall_id;
	LIST_FORALL( l.pHallList, idx, PBINGHALL, pHall )
	{
		if( pHall->LinkHallState.hall_id == lookfor )
			break;
	}
	if( !pHall )
	{
		if( !noise.bExcuseMe )
		{
			pHall = CreateAHall( INVALID_INDEX, lookfor );
			if( !pHall )
			{
				xlprintf(LOG_NOISE)( "EXCUSE ME? What the ... is going on around here? Failed to find  %d.  Is there no master at all?", lookfor );
				noise.bExcuseMe = TRUE;
			}
		}

		return 0;
	}

	if( ( !pHall->LinkHallState.enabled ) || ( pHall->LinkHallState.prohibited ) )
	{
		if( noise.bExcuseMe )
		{
			xlprintf(LOG_NOISE)( "No more EXCUSE ME? Hall enabled %u prohibited %u "
									 , pHall->LinkHallState.enabled
									 , pHall->LinkHallState.prohibited
									 );
			noise.bExcuseMe = FALSE;
		}
		return 1;
	}
	else
		return 0;

}

// this method should enable hosting bdata as available.

DefineInvokeMethod( StartMedia, void, (void), (void) );
DefineInvokeMethod( StopMedia, void, (void), (void) );
DefineInvokeMethod( Announcement, void, (CTEXTSTR), (CTEXTSTR filename), filename );
DefineInvokeMethod( StopPromotions, void, (void), (void) );
DefineInvokeMethod( PlayPromotions, void, (void), (void) );

// bdata tracks only master events... begin service proxy for bdata
DefineInvokeMethod( ServeBData, void, (void), (void) );
// disconnect event to bdata links
DefineInvokeMethod( DisconnectBData, void, (void), (void) );

// begin serving master steram
DefineInvokeMethod( ServeMaster, void, (void), (void) );
// begin serving delegate stream
DefineInvokeMethod( ServeDelegate, void, (void), (void) );
// directed connect - connect display to master
DefineInvokeMethod( ConnectToMaster, void, (CTEXTSTR), (CTEXTSTR host_address), host_address );
// directed connect - connect display to delegate
DefineInvokeMethod( ConnectToDelegate, void, (CTEXTSTR), (CTEXTSTR host_address), host_address );

// reconfigures the input camera mux.
DefineInvokeMethod( SetInput, void, (int), (int mode), mode );

// passed variable is 'forced' or 'initial state' it is issued once when the program start
// and then again for disconnect states.
DefineInvokeMethod( Reset, void, (LOGICAL), (LOGICAL forced), forced );


/*
static void OnLinkedCardVerify( "video state server" )( LOGICAL bEnable )
{
	if( l.pMyHall->LinkHallState.master_ready || l.pMyHall->LinkHallState.delegate_ready )
	{
		InvokeSetInput( bEnable );
	}
}
*/
//-----------------------------------------------------------------
// int PollCurrentState( void )
//   A standardized way of populating the state structure within
//   the static local structure.
//
//------------------------------------------------------------------
void PollCurrentState( void )
{
	CTEXTSTR *sequelresults;//yes, a pointer.
	_32 count = 0; // What if the database becomes disconnected?   This count will help figure that out.

	if( !SQLRecordQueryf( g.odbc, NULL
							 , &sequelresults
							 , NULL
							 , "SELECT master_hall_id,delegated_master_hall_id,controller_hall_id from link_state where bingoday=%s"
							 , g.flags.bUseBingoDay?GetSQLOffsetDate( g.odbc, "Video Link" ):"0"
							 )
	  )
	{
		xlprintf(LOG_ERRORS)( "Error reading from database, connection open? %s", IsSQLOpen( g.odbc )?"YES":"NO" );
      return;
	}
	{
		if( sequelresults )
		{
         int new_delegate = atoi (sequelresults[1]);
			l.current_state.LinkState.master_hall_id = atoi (sequelresults[0]) ;
			if( new_delegate != l.current_state.LinkState.delegated_master_hall_id )
			{
				l.current_state.LinkState.delegated_master_hall_id = new_delegate;
            UpdateLinkButtons();
			}
			l.current_state.LinkState.controller_hall_id = atoi (sequelresults[2]) ;
		}
		else
		{
			while( !SQLCommandf( g.odbc,"insert into link_state (bingoday,master_hall_id,delegated_master_hall_id,controller_hall_id) "
									  "values (%s,%"_32f",%"_32f",%"_32f")"
									 , g.flags.bUseBingoDay?"curdate()":"0"
									 , l.current_state.LinkState.master_hall_id
									 , l.current_state.LinkState.delegated_master_hall_id
									 , l.current_state.LinkState.controller_hall_id
									 ) )
			{
				DogastrophicErrorEx( 0, "Insert Failed", "Check sql.log. Can't get to link_state? Bailing");
			}
			xlprintf(LOG_ERRORS)( "no halls in link_state table, but inserted link_state for master_hall_id %u at least.", l.current_state.LinkState.master_hall_id );
		}
	}

	for( SQLRecordQueryf( g.odbc,NULL
								 , &sequelresults
								 , NULL
								 , "SELECT hall_id,participating,enabled,master_ready,delegate_ready,task_launched,announcing,reset_state,announcement,prohibited,dvd_active,media_active from link_hall_state"
								 ) ;
		  sequelresults;
		  FetchSQLRecord( g.odbc, &sequelresults )
		)
	{
		PBINGHALL pHall;
		INDEX idx;
		INDEX lookfor = atoi( sequelresults[0] );
		CTEXTSTR *results = sequelresults + 1;

		count++;// What if the database becomes disconnected?   This count will help figure that out.

		LIST_FORALL( l.pHallList, idx, PBINGHALL, pHall )
		{
			if( pHall->LinkHallState.hall_id == lookfor )
				break;
		}
		if( !pHall )
		{
			continue;
		}
		//assign the resultant to relevant variables in our static local structure.
		pHall->LinkHallState.participating = atoi( results[0] );
		{
			int newval = atoi( results[1] );
			if( newval != pHall->LinkHallState.enabled )
			{
				pHall->LinkHallState.enabled = newval;
            if( pHall->LinkHallState.hall_id == l.pMyHall->LinkHallState.hall_id )
					UpdateLinkButtons();
			}
		}
		pHall->LinkHallState.master_ready = atoi( results[2] );
		pHall->LinkHallState.delegate_ready = atoi( results[3] );
		pHall->LinkHallState.task_launched = atoi( results[4] );
		pHall->LinkHallState.bSoundPlaying = atoi( results[5] );
		pHall->LinkHallState.reset_state = atoi( results[6] );
		strcpy(pHall->LinkHallState.announcement, results[7] );
		pHall->LinkHallState.prohibited = atoi( results[8] );
		pHall->LinkHallState.dvd_active = atoi( results[9] );
		pHall->LinkHallState.media_active = atoi( results[10] );

	}

	if( !count )
	{
		xlprintf(LOG_ALWAYS)( "Can't get to link_hall_state?  In PollCurrentState, the DoSQLRecordQueryf failed to return any results\n\tmeaning either the database cannot be contacted, as in the case of a disconnected LAN cable,\n\tor there are no valid entries in the link_hall_state table.\n\tRegardless, this is a failure condition.  Please check sql log for any error conditions.  \n\tSince the database cannot be relied upon at this time, there is no point in continuing.\n\tSo, Bailing.  Sorry it didn't work out.");
	}
	else
	{
		if( !g.flags.bReadOnly )
			if( l.pMyHall )
				SQLCommandf( g.odbc,"replace into link_alive (hall_id) values (%d)", l.pMyHall->LinkHallState.hall_id );
	}
}

void ReadOnlyReset( void )
{
	// this is one-shot protected invokation....

	if( l.pMyHall->LinkHallState.flags.bParticipating )
	{
		lprintf( "no longer pariticpating. (readonly state)" );
		l.pMyHall->LinkHallState.flags.bParticipating = 0;
	}
	if( l.pMyHall->LinkHallState.flags.bHosting )
	{
 		lprintf( "no longer hosting. (readonly state)" );
		l.pMyHall->LinkHallState.flags.bHosting = 0;
	}
}

int ForceLocalModeEx( LOGICAL soft_reset DBG_PASS )
#define ForceLocalMode(onoff) ForceLocalModeEx(onoff DBG_SRC)
{
	if( g.flags.bReadOnly )
	{
		// these shouldn't reset public state... it's just reading.
      _lprintf(DBG_RELAY)( "... ");
		InvokeReset( !soft_reset );
	}
	else
	{
		LOGICAL forced = !soft_reset;
		int retval;
		//CTEXTSTR sequelresult = NULL;
		_xlprintf(LOG_ADVISORY DBG_RELAY)("Entered ForceLocalMode( %s )"
													, soft_reset?"TRUE":"FALSE"
													);
		//  	if( !forced && l.pMyHall->LinkHallState.task_launched )
		if( !forced && l.pMyHall->LinkHallState.reset_state )
		{
			xlprintf(LOG_NOISE)( "Reset state was set, ForceLocalMode clearing reset.(2)" );
			l.pMyHall->LinkHallState.reset_state = 0;
		}

		// set these flags immediately for internal reference....

		l.pMyHall->LinkHallState.task_launched =
			l.pMyHall->LinkHallState.master_ready =
			l.pMyHall->LinkHallState.delegate_ready =
			l.pMyHall->LinkHallState.reset_state =
			l.pMyHall->LinkHallState.participating = 0;


		if( !( SQLCommandf( g.odbc,"UPDATE link_hall_state SET reset_state=0,task_launched=0,master_ready=0,delegate_ready=0,participating=0 WHERE hall_id=%u"
								, l.pMyHall->LinkHallState.hall_id
								)
			  ))
		{
			DogastrophicErrorEx( 0, "Update Failed", "Can't get to link_hall_state ? ! ? ! Check sql log.  Bailing.");
		}
		// this needs to be a force always condition almost...
		else
		{
      _lprintf(DBG_RELAY)( "... ");
			InvokeReset( forced );
		}
	}
	return TRUE;
}

//--- void PostLoadInit( void ) ---------------------------------------
/*  The main purpose of this function is to populate the hall list for
 *  later reference and the secondary function is to determine which hall
 *  is the local hall.
 */
void PostLoadInit( void )
{
   INDEX hallid = INVALID_INDEX;
	l.pHallList = CreateList();
	EmptyList( &l.pHallList );
	//tagHallName
   do
	{
		CTEXTSTR *sequelresults = NULL; //yes a pointer
		//This is error checking to ensure my entry is in the location and link_hall_state tables.
		{
			CTEXTSTR sqlrslt = NULL;
			char szGottenHostName[128];
			CTEXTSTR pszHost = NULL;

			if( !l.hall_name )//tagHallName If no hallname was specified when the application was launched, fill it in.
			{
				{
					l.hall_name = GetSystemName();
					xlprintf(LOG_ALWAYS)("Assigned %s as l.hall_name because no name was passed on the command line.", l.hall_name );
				}
			}

			pszHost = l.hall_name;

			SQLQueryf( g.odbc,&sqlrslt
							  , "SELECT id FROM location WHERE packed_name=\'%s\'"
							  , pszHost
						);

			if( !sqlrslt )
			{
				//oops, I don't exist?
				while( !SQLCommandf(g.odbc,"INSERT INTO location (packed_name,name) VALUES (\'%s\','\%s\')"
										 , pszHost
										 , pszHost
										 )
				  )
				{
					DogastrophicErrorEx( 0, "Insert Failed", "Hey! What's wrong with location table?? Could not insert robustly.  Check sql.log. Bailing." );
               WakeableSleep( 5000 );
				}

				{
					hallid = FetchLastInsertID( g.odbc, "location", "id" );
					xlprintf(LOG_ALWAYS)("Warning.  Could not find %s in location table.  Inserted as record %d, and wrote the ini file."
											  , pszHost
                                    , hallid
											  );
				}
			}
			else
			{
				hallid = strtoul( sqlrslt, NULL, 0 );
				xlprintf(LOG_NOISE)("joy. my hallid is now %u", hallid);
			}

			sqlrslt = NULL;

			SQLQueryf( g.odbc,&sqlrslt
						  , "SELECT id FROM link_hall_state WHERE hall_id=%u"
						  , hallid
						  );
			if( !sqlrslt )
			{
				//oops, again I don't exist.  must be a freshly created database.
				if( !SQLCommandf(g.odbc,"INSERT INTO link_hall_state (hall_id) VALUES (%u)"
										, hallid
										)
				  )
				{
					DogastrophicErrorEx( 0, "Insert Failed", "Hey! What's wrong with link_hall_state table?? Could not insert robustly.  Check sql.log. Bailing." );
				}
				else
				{
               hallid = FetchLastInsertID( g.odbc, "link_hall_state", "id" );
					xlprintf(LOG_ALWAYS)("Warning.  Could not find %s in link_hall_state table.  Inserted as record %d"
											  , pszHost
											  , hallid
											  );
				}
			}
		}
		//This was error checking to ensure my entry is in the location and link_hall_state tables.
		for( SQLRecordQueryf( g.odbc,NULL, &sequelresults, NULL, "select id from location" );
			 sequelresults;
			  FetchSQLRecord( g.odbc,&sequelresults ) )
		{
			int ID = atoi( sequelresults[0] );
         CreateAHall( hallid, ID );
		}
		if( !l.pMyHall )
		{
			DogastrophicErrorEx( 0, "Get My Hall Failed", "Failed to setup the pointer to my current hall info.");
			//exit(1);
		}
	} while( !l.pMyHall );


	xlprintf(LOG_ADVISORY)("l.hall_id is now %d"
								 , l.pMyHall->LinkHallState.hall_id
								 );

}

void ProcessBdataConfig( void )
{
	//  	xlprintf(LOG_ADVISORY)("Welcome to ProcessBDataConfig. By the way, l.pMyHall->LinkHallState.task_launched is %d"
	//                        , l.pMyHall->LinkHallState.task_launched
	//  							 );
	// when the video is ready, configure ports...
#if 0
	if( l.pMyHall->LinkHallState.task_launched )
	{
		// was launched with ianson commands before...
		if(! noise.bGetOuttaHere )
		{
			xlprintf(LOG_ADVISORY)("Hey! YOu can't ProcessBdataConfig when task_(still_)launched! One warning here, pay attention.  Now Get outta here. returning");
			noise.bGetOuttaHere = TRUE;
		}
		return;
	}
	else
	{
		noise.bGetOuttaHere = FALSE;
	}
#endif

	if( l.pMyHall->LinkHallState.master_ready )
	{
		//xlprintf(LOG_NOISE)( "master host ready...connected:%d", l.pMyHall->LinkHallState.flags.bConnected );
		// on the first pass, reset to local, first pass at master ready, and not connected
		// otherwise we're telling the host to host or the remotes to reset.

		if( ! l.pMyHall->LinkHallState.flags.bConnected  )
		{
			l.pMyHall->LinkHallState.flags.bConnected = 1;
			if( !l.pMyHall->LinkHallState.flags.bHostAssigned )
			{
				InvokeServeBData();
				l.pMyHall->LinkHallState.flags.bHostAssigned = 1;
			}
		}//end of if( l.pMyHall->LinkHallState.flags.bConnected )
	}//end of if( l.pMyHall->LinkHallState.master_ready )
	else //   if( l.pMyHall->LinkHallState.master_ready )
	{
		// not master_ready, therefore reset my box? WELL, not exactly.  Just show bConnected as zero for now, right?
		// right, let's not cause so much trouble without checking our facts here first.
		//  are we connected?  did the master_hall_id in fact, change? or are we just returning from delegated_master?
		//xlprintf( LOG_NOISE)("master host not ready...connected:%d", l.pMyHall->LinkHallState.flags.bConnected );

		if ( ( l.pMyHall->LinkHallState.flags.bConnected ) &&
			 ( (l.current_state.LinkState.master_hall_id != l.pMyHall->LinkHallState.hall_id )
			  || ( (!l.pMyHall->LinkHallState.enabled)
					&& ( l.current_state.LinkState.master_hall_id == l.pMyHall->LinkHallState.hall_id ) )
			 )
			)
		{
			if ( noise.bBeingGreen )
			{
				xlprintf(LOG_NOISE)( "easy being green. First time I've noticed that master is no longer me, and I am connected, disconnecting bdata" );
				noise.bBeingGreen = FALSE;
			}

			InvokeDisconnectBData();
			l.pMyHall->LinkHallState.flags.bConnected = 0;
			lprintf( "..." );
		}
		else
		{
			if( ! noise.bBeingGreen )
			{
				xlprintf(LOG_NOISE)("It ain't easy being green. l.pMyHall->LinkHallState.master_ready is %u l.pMyHall->LinkHallState.flags.bConnected is %d, l.current_state.LinkState.master_hall_id is %u  l.pMyHall->LinkHallState.hall_id is %u  so doing nothing here."
										 , l.pMyHall->LinkHallState.master_ready
										 , l.pMyHall->LinkHallState.flags.bConnected
										 , l.current_state.LinkState.master_hall_id
										 , l.pMyHall->LinkHallState.hall_id
										 );
				noise.bBeingGreen = TRUE;
			}
		}
	}
}

void ShopBlob( PPLAYLIST pPlaylist )
{
	char name[1024];
	FILE *p2TheFile = NULL;

	lprintf("Does %s exist?", pPlaylist->name );
	if( !( ( p2TheFile =  fopen( pPlaylist->name, "r" ) ) ) )
	{
		CTEXTSTR pBlob = NULL;
		TEXTSTR pMedia = NULL;
		lprintf(" No, %s does not exist.  Can it be downloaded from the database?"
				 , pPlaylist->name
				 );
		if( SQLQueryf( g.odbc,&pBlob
							, "SELECT content FROM media WHERE media_name=\'%s\' AND deleted=0"
							, pPlaylist->simplename
							) )
		{
			_32 s = strlen( pBlob );
			pMedia = RevertEscapeBinary( pBlob , &s );
			if( pMedia )
			{
				FILE * pFile = fopen( name, "wb" );
				if( pFile )
				{
					size_t count = strlen( pMedia ), w;
					if(  ( w = fwrite( pMedia, sizeof( pMedia[0] ), count , pFile  ) ) == count )
					{
						lprintf("Successfully wrote %u bytes for %s "
								 , count
								 , pPlaylist->simplename
								 );
					}
					else
					{
						lprintf("Supposed to write %u bytes for %s, but wrote %u bytes instead.  Hope it works out. Now what?"
								 , count
								 , pPlaylist->simplename
								 , w
								 );
					}
					fclose( pFile );
				}
				else
				{
					lprintf("Could not open %s?  Now what?", pPlaylist->simplename );
				}

			}
			else
			{
				lprintf("pMedia is null?  Now what?");
			}
		}
		else
		{
			lprintf("No, for some reason, %s cannot be downloaded from the database.  This is not serious enough to bail, but needs to be fixed."
					 , pPlaylist->name
					 );
		}
	}
	else
	{
		fclose( p2TheFile );
		lprintf(" Yes, %s exists ", pPlaylist->name );
	}
}

void WritePlaylistNamesToFile( PLIST list , FILE * pFile )
{
	INDEX idx;
	PPLAYLIST pPlaylist;
	int i;

	LIST_FORALL( list, idx, PPLAYLIST, pPlaylist )
	{
		i = fwrite( pPlaylist->name, ( strlen(pPlaylist->name)  ), 1 , pFile );
		lprintf(" Wrote %s into the playlist", pPlaylist->name );
		ShopBlob( pPlaylist ); //any reason why I cannot have two files open for writing at the same time?
	}

}
void GetMediaList( _32 hall_id )
{
	_32 alliter=0, myiter=0;
	CTEXTSTR *results;
	PLIST listMediaForAll = NULL;
	PLIST listMediaForMe = NULL;
	FILE *file;
	PPLAYLIST pPlaylist;
	INDEX idx;

	lprintf("GetMediaList.");
	for( SQLRecordQueryf( g.odbc,NULL, &results, NULL
								 , "SELECT media_playlist.hall_id,media.media_name "
								  "FROM media,media_playlist "
								  "WHERE ( media_playlist.hall_id=0  "
								  "OR media_playlist.hall_id=%lu ) "
								  "AND media_playlist.media_id=media.media_id "
								  "ORDER BY media_playlist.media_playlist_order ASC"
								 , hall_id ) ;
		  results  ;
		  FetchSQLRecord( g.odbc, &results ) )
	{
		PPLAYLIST p = New( PLAYLIST );
		p->hall_id = atoi( results[0] );
		strcpy( pPlaylist->simplename, results[1]);
		snprintf( p->name , (sizeof ( p->name ) ), "%s%s\n", MEDIA_ROOT_PATH , results[1] );

		if( !p->hall_id )
		{
			lprintf("For everyone: %s %s" , results[0], results[1] );
			AddLink( &listMediaForAll, p );
			alliter++;
		}
		else
		{
			lprintf("For someone specific: %s %s" , results[0], results[1] );
			AddLink( &listMediaForMe, p );
			myiter++;
		}
	}
	lprintf( "alliter is %u myiter is %u", alliter, myiter );
	if ( ( file = fopen( MEDIA_ROOT_PATH"medialist.m3u", WIDE("wt") ) ) )
	{
		// So, if my hall_id is specified in media_playlist, myiter will be
		// non-zero.  I want that stuff even though there is global stuff.
		if( myiter )
		{
			WritePlaylistNamesToFile( listMediaForMe, file );
		}
		else if( alliter )
		{
			WritePlaylistNamesToFile( listMediaForAll, file );
		}
		fclose( file );
	}
	else
	{
		lprintf("could not open up %s", MEDIA_ROOT_PATH"medialist.m3u" );
	}

	//a little obvious...but necessary.  Regardless of how much is specified, get rid of it.
	LIST_FORALL( listMediaForMe, idx, PPLAYLIST, pPlaylist )
	{
		Release( pPlaylist );
	}
	LIST_FORALL( listMediaForAll, idx, PPLAYLIST, pPlaylist )
	{
		Release( pPlaylist );
	}
}

void ProcessMedia( void )
{
			//The DVD Promo
			if( l.pMyHall->LinkHallState.dvd_active )
			{
				if( !l.current_state.LinkHallState.dvd_active )
				{
					InvokePlayPromotions();
					l.current_state.LinkHallState.dvd_active = 1;
				}
				else
				{
					//lprintf("l.current_state.LinkHallState.dvd_active is already active, so not calling ScriptCommunication playpromo");
				}
			}
			else if( !l.pMyHall->LinkHallState.dvd_active )
			{
				if( l.current_state.LinkHallState.dvd_active )
				{
					InvokeStopPromotions();
					l.current_state.LinkHallState.dvd_active = 0;
				}
				else
				{
					//lprintf("l.current_state.LinkHallState.dvd_active is already inactive, so not calling ScriptCommunication killpromo");
				}
			}
			//The DVD Promo
			//The media list
			if( l.pMyHall->LinkHallState.media_active )
			{
				if( !l.current_state.LinkHallState.media_active )
				{
					InvokeStartMedia();
					l.current_state.LinkHallState.media_active = 1;
				}
				else
				{
					//lprintf("l.current_state.LinkHallState.media_active is already active, so not calling GetMediaList");
				}
			}
			else if( !l.pMyHall->LinkHallState.media_active )
			{
				if( l.current_state.LinkHallState.media_active )
				{
					InvokeStopMedia();
					l.current_state.LinkHallState.media_active = 0;
				}
				else
				{
					//lprintf("l.current_state.LinkHallState.dvd_active is already inactive, so not calling ScriptCommunication killpromo");
				}
			}

					//---Checking for Announcements ---//
#ifdef __LINUX__
					// ok, the result value of strncasecmp is zero ONLY if the two strings are equal, otherwise it's a non-zero number.
					if( ( strncasecmp( l.pMyHall->LinkHallState.announcement, "NONE", 4 ) ) == 0)
#else
						if(  strnicmp( l.pMyHall->LinkHallState.announcement, "NONE", 4 ) == 0)
#endif  // stupid little windows compiler fix.  i guess wcc version < 1.5 doesn't support strncasecmp.
						{
						}
						else
						{
							//  oops, the comparison came back nonzero (announcement is NONE or none), resulting in an else condition.
							if( !l.pMyHall->LinkHallState.task_launched
								&& !l.pMyHall->LinkHallState.bSoundPlaying
							  )
							{
								char buf[100];

								xlprintf(LOG_ADVISORY)("Yay! Got a sound %s to be played."
															 , l.pMyHall->LinkHallState.announcement
															 );
								snprintf( buf, (sizeof(buf)), "%s/sounds/%s"
										  ,  g.MyLoadPath
										  , l.pMyHall->LinkHallState.announcement
										  );
								InvokeAnnouncement( buf );
								SQLCommandf( g.odbc,"UPDATE link_hall_state SET announcement='NONE' WHERE hall_id=%d"
											  , l.pMyHall->LinkHallState.hall_id // ;-) that's me!
											  );
							}
							else
							{
								xlprintf(LOG_ALWAYS)("Hey! Sound is already playing!");
							}
						}//end of sounds rountine
					//---End of Checking for Announcements ---//
}

/*  void CPROC ServerCheckStateThread( PTHREAD pThread )
 *  This is the main function of this application. Its purpose is to
 *  launch video, audio, or control bdata as identified by state within
 *  the appropriate database tables.  A five second (while) loop is
 *  executed. Every five seconds, PollCurrentState is called, resulting in
 *  the MySQL database being queried for state.
 *
 */
PTRSZVAL CPROC ServerCheckStateThread( PTHREAD pThread )
{
	LOGICAL bVerySensitive = FALSE;

	xlprintf(LOG_ADVISORY)("Welcome to ServerCheckStateThread");
	l.check_state_thread = pThread;

	lprintf("Entered the main routine. l.hall_name is %s"
			 , l.hall_name
			 );

	// after all init is done, this clears all states in the database
	// and forces definitively that we are RESET.
	ForceLocalMode(FALSE);  // ;)

	xlprintf(LOG_NOISE)("...  Entering ServerCheckStateThread's main loop ...");
	while(1)
	{
		PollCurrentState();

		// process bdata configuration setup (connect and host, disconnect, reset)
		if( ( !l.current_state.LinkState.delegated_master_hall_id ) &&
			( l.current_state.LinkState.master_hall_id == l.pMyHall->LinkHallState.hall_id )
		  )
		{
         // no delegate master, and master hall ID is me.
			ProcessBdataConfig();
		}

		// check states related to video control...
		if( 1 )
		{
			//The media list
         ProcessMedia();

			if( l.pMyHall->LinkHallState.reset_state )
			{
				//"Eject, EJECT, EJECT!!"
				xlprintf(LOG_ADVISORY)("Yeehaw! Reset by way ForceLocalMode( TRUE )! Let's GO!");
				// convention would indicate that reset_state should also be reset internally here
				// however, this is needed by forcelocalmode to actually be FORCED
				// therefore, we leave the reset of this internal bit to ForceLocalMode routine.
				if( !SQLCommandf( g.odbc,"update link_hall_state set reset_state=0 where hall_id=%d", l.pMyHall->LinkHallState.hall_id ) )
					DogastrophicError(1, "Update Failed" );
				ForceLocalMode( TRUE );
			}

			// The most important state is "enabled".
			// Either I am *not* enabled or I am prohibited.  Same difference for now.
			else if( ( !l.pMyHall->LinkHallState.enabled ) || ( l.pMyHall->LinkHallState.prohibited ) )
			{
				// since all fo these get cleared when doing a force local mode,
				// a second pass through here will not retrigger the script.
				if(   ( l.pMyHall->LinkHallState.master_ready   )
					|| ( l.pMyHall->LinkHallState.delegate_ready )
					|| ( l.pMyHall->LinkHallState.participating )
					|| ( l.pMyHall->LinkHallState.flags.bParticipating )
				  )
				{
               lprintf( "Exisitng state needs to be cleared, we're no longer enabled." );
					if(  !( l.pMyHall->LinkHallState.task_launched ) )
					{
						xlprintf(LOG_ADVISORY)( "Forcing local mode because not enabled or prohibited... one time.");
						ForceLocalMode(TRUE);
					}
					else
					{
						xlprintf(LOG_ALWAYS)("[PU][PEE EWE] WHOA!  l.pMyHall->LinkHallState.task_launched is %u!  Better not call ForceLocalMode(TRUE) for now... come back later. For the record, %s%s%s is set", l.pMyHall->LinkHallState.task_launched, ( l.pMyHall->LinkHallState.master_ready   )?"MASTER READY "  :" ", ( l.pMyHall->LinkHallState.delegate_ready )?"DELEGATE READY ":"", ( l.pMyHall->LinkHallState.participating  )?"PARTICIPATING " :"");
					}
				}
			}// if (! enabled or prohibited )
			//on the ohter hand,  I am enabled AND I am not prohibited....
			else if( ( l.pMyHall->LinkHallState.enabled  ) && ( !l.pMyHall->LinkHallState.prohibited) )
			{
				if( RudelyInterrupted () ) // make sure the master is enabled and not prohibited...that is, not rudely interrupted.
				{
					if( !bVerySensitive )
					{
						xlprintf(LOG_NOISE)("EXCUSE ME! Where did the master go? Master is RudelyInterrupted(). That means that I'm enabled/not prohibited, but the master is not. ForceLocalMode ( TRUE ) then. Hrmph." );
						bVerySensitive = TRUE;
						if( !g.flags.bReadOnly )
							ForceLocalMode(TRUE);
					}
					else
					{
						xlprintf(LOG_NOISE)("Hrmph. The master continues to be NOT ENABLED or PROHIBITED.  I'm still waiting.... I called ForceLocalMode a while back, and the master is prohibited or not enabled.");
					}
				}
				else
				{
					//  Well, the master (finally) is ENABLED and NOT PROHIBITED.  It's about time.
					bVerySensitive = FALSE;
				}

				{
					// Well, well, what have we here?  Once in a great while, usually at the beginning of
					// the application, master_hall_id is set to zero.  When the master takes over and
					// starts to broadcast, it transitions so that current != last.  But what happens
					// during that rare instance when someone changes master?  Bad, real bad. What now?

					if( ( l.current_state.LinkState.master_hall_id != l.last_state.LinkState.master_hall_id ) )
					{
						//"Changes In Master"
						// new master, reset myself.
						//lprintf("new master, reset myself.");
						if( ForceLocalMode( TRUE ) )
						{
							xlprintf(LOG_NOISE)("[GRU][GEE ARE YOU] l.last_state.LinkState.master_hall_id was %u"
													 , l.last_state.LinkState.master_hall_id
													 );
							l.last_state.LinkState.master_hall_id = l.current_state.LinkState.master_hall_id;
							xlprintf(LOG_NOISE)("[GRU][GEE ARE YOU] l.last_state.LinkState.master_hall_id is now %u"
													 , l.last_state.LinkState.master_hall_id
													 );
						}
						else
						{
							xlprintf(LOG_ALWAYS)("[GRU][GEE ARE YOU] Uh, oh.  Why did ForceLocalMode(TRUE) return FALSE???");
						}
					}

					// Ok, what happens when the delegated_master_hall_id changes?  Supposed to be set to zero
					// especially at the beginning of the application, and when delegated_master relinquishes back
					// to master (as in, after verification of a player-called bingo is completed ).  For now,
					// since it went from zero to something  or something to zero, just KillAllTasks but don't
					// ForceLocalMode because pretty soon (and I mean *really* soon) master_ready will be set
					// making everybody look at the master once again.
					//"Changes In Delegate Master"
					if( ( l.current_state.LinkState.delegated_master_hall_id != l.last_state.LinkState.delegated_master_hall_id )  &&
						! ( l.pMyHall->LinkHallState.task_launched )
					  )
					{
						if( l.pMyHall->LinkHallState.master_ready )

						l.last_state.LinkState.delegated_master_hall_id = l.current_state.LinkState.delegated_master_hall_id;
						xlprintf(LOG_ALWAYS)( "New delegate master... killing tasks. (USED TO BE KILL ALL TASKS... NEED LAUNCH CLOSED!)" );

					}


					// What happens when the delegated_master_id is me all of a sudden?  I'm not ready, so let's get it done.
					if( ( l.current_state.LinkState.delegated_master_hall_id == l.pMyHall->LinkHallState.hall_id ) &&
						( !(l.pMyHall->LinkHallState.delegate_ready  ) )
					  )
					{
						if( !l.pMyHall->LinkHallState.task_launched ) //guard
						{
							InvokeServeDelegate();
						}
					}
					// Am I (l.hall_id) the master? Has "master_ready" been set?
					else if( (! l.current_state.LinkState.delegated_master_hall_id )  &&
							  ( l.current_state.LinkState.master_hall_id == l.pMyHall->LinkHallState.hall_id ) &&
							  ! ( l.pMyHall->LinkHallState.master_ready  )
							 )
					{
						if( !l.pMyHall->LinkHallState.task_launched ) //guard
						{
							lprintf( "-------This is the good one.---------------" );
							InvokeServeMaster();
						}
					}// else if
					// Am I (l.hall_id) the master? Has "master_ready" been set?
					else if( ( l.current_state.LinkState.delegated_master_hall_id )  &&
							  ( l.current_state.LinkState.delegated_master_hall_id != l.pMyHall->LinkHallState.hall_id ) &&
							  ( l.pMyHall->LinkHallState.delegate_ready  )&&
							  ! ( l.pMyHall->LinkHallState.participating )
							 )
					{
						if( !l.pMyHall->LinkHallState.task_launched ) //guard
						{
							PBINGHALL pMasterHall;
							lprintf( "-------This is the good one.---------------" );
							if( ( pMasterHall = IsMasterReady() ) )
							{
								InvokeConnectToDelegate( pMasterHall->stIdentity.szVideoAddr );
							}
						}
					}// else if

					//"Where did everybody go?"
					// Ok, what's going on here?  Why is no master_ or delegate_ready and I'm not even participating?
					else if(! ( l.pMyHall->LinkHallState.master_ready )   &&
							  ! ( l.pMyHall->LinkHallState.delegate_ready ) &&
							  ! ( l.pMyHall->LinkHallState.participating )
							 )
					{
						//start the participant mode based on the current master, as long as the current master is ready....
						PBINGHALL pMasterHall;
						if( ( pMasterHall = IsMasterReady() ) )
						{
							if( !l.pMyHall->LinkHallState.task_launched ) //guard
							{
								InvokeConnectToMaster( pMasterHall->stIdentity.szVideoAddr );
							}
						}
					}//yet another else if
				}// if (!l.pMyHall->LinkHallState.prohibited)
			}// if( ( l.pMyHall->LinkHallState.enabled == 1 ) )
		}// if ( DoSQLRecordQueryf
		WakeableSleep(g.poll_delay);
	}// while(1)
   return 0;
}//void CPROC ServerCheckStateThread( PTHREAD pThread )



void CPROC KickTimerProc( PTRSZVAL psv, char *extradata )
{
	//  	xlprintf(LOG_NOISE)( "Event received! :) It's address is %p and had extra data of --> %s <---"
	//  							 , psv
	//  							 , extradata
	//  							 );
	WakeThread( l.check_state_thread );
}

void CommonInit( void )
{
	// the path where the original executable is found. (use this for where scripts go)
	RegisterInterface( "Video Server", GetVideoServerInterface, NULL );
	g.poll_delay = 10000;
	g.flags.bUseBingoDay = SACK_GetProfileInt( GetProgramName(), "Use bingoday for link state (else use 0)", 0 );
	g.flags.bReadOnly = SACK_GetProfileInt( GetProgramName(), "Readonly state tracking", 1 );

	g.MyLoadPath = GetProgramPath();
	{
		char dsn[256];
		SACK_GetProfileString( GetProgramName(), "Video State Server DSN", "vsrvr", dsn, sizeof( dsn ) );
		// specify this as a required connection.
		// this will not allow SQL statements to pass against a connectoin that is not open
      // they can still fail syntactically or because data doesn't exist..
		g.odbc = ConnectToDatabaseEx( dsn, TRUE );
		SetSQLLoggingDisable( g.odbc, !SACK_GetProfileInt( GetProgramName(), "Log SQL", 0 ) );
		SACK_GetProfileString( GetProgramName(), "My Hall Name", GetSystemName(), dsn, sizeof( dsn ) );
      l.hall_name = StrDup( dsn );
		
	}
	CheckMyTables( g.odbc );
	{

		{
			INDEX hallid = INVALID_INDEX;
			l.pHallList = CreateList();
			EmptyList( &l.pHallList );
			//tagHallName
			do
			{
				CTEXTSTR *sequelresults = NULL; //yes a pointer
				//This is error checking to ensure my entry is in the location and link_hall_state tables.
				{
					CTEXTSTR sqlrslt = NULL;
					char szGottenHostName[128];
					CTEXTSTR pszHost = NULL;

					if( !l.hall_name )//tagHallName If no hallname was specified when the application was launched, fill it in.
					{
						{
							l.hall_name = GetSystemName();
							xlprintf(LOG_ALWAYS)("Assigned %s as l.hall_name because no name was passed on the command line.", l.hall_name );
						}
					}

					pszHost = l.hall_name;

					SQLQueryf( g.odbc,&sqlrslt
								, "SELECT id FROM location WHERE packed_name=\'%s\'"
								, pszHost
								);

					if( !sqlrslt )
					{
						//oops, I don't exist?
						while( !SQLCommandf(g.odbc,"INSERT INTO location (packed_name,name) VALUES (\'%s\','\%s\')"
												 , pszHost
												 , pszHost
												 )
							  )
						{
							DogastrophicErrorEx( 0, "Insert Failed", "Hey! What's wrong with location table?? Could not insert robustly.  Check sql.log. Bailing." );
							WakeableSleep( 5000 );
						}

						{
							hallid = FetchLastInsertID( g.odbc, "location", "id" );
							xlprintf(LOG_ALWAYS)("Warning.  Could not find %s in location table.  Inserted as record %d, and wrote the ini file."
													  , pszHost
													  , hallid
													  );
						}
					}
					else
					{
						hallid = strtoul( sqlrslt, NULL, 0 );
						xlprintf(LOG_NOISE)("joy. my hallid is now %u", hallid);
					}

					sqlrslt = NULL;

					SQLQueryf( g.odbc,&sqlrslt
								, "SELECT id FROM link_hall_state WHERE hall_id=%u"
								, hallid
								);
					if( !sqlrslt )
					{
						//oops, again I don't exist.  must be a freshly created database.
						if( !SQLCommandf(g.odbc,"INSERT INTO link_hall_state (hall_id) VALUES (%u)"
											 , hallid
											 )
						  )
						{
							DogastrophicErrorEx( 0, "Insert Failed", "Hey! What's wrong with link_hall_state table?? Could not insert robustly.  Check sql.log. Bailing." );
						}
						else
						{
							xlprintf(LOG_ALWAYS)("Warning.  Could not find %s in link_hall_state table.  Inserted as record %d"
													  , pszHost
													  , hallid
													  );
						}
					}
				}
				//This was error checking to ensure my entry is in the location and link_hall_state tables.
				for( SQLRecordQueryf( g.odbc,NULL, &sequelresults, NULL, "select id from location" );
					 sequelresults;
					  FetchSQLRecord( g.odbc,&sequelresults ) )
				{
					int ID = atoi( sequelresults[0] );
					CreateAHall( hallid, ID );
				}
				if( !l.pMyHall )
				{
					DogastrophicErrorEx( 0, "Get My Hall Failed", "Failed to setup the pointer to my current hall info.");
					//exit(1);
				}
			} while( !l.pMyHall );


			xlprintf(LOG_ADVISORY)("l.hall_id is now %d"
										 , l.pMyHall->LinkHallState.hall_id
										 );

		}

		//	xlprintf(LOG_ADVISORY)("Welcome to void InitProc(void)" );
		if( !g.flags.bReadOnly )
		{
			if(! (SQLCommandf(g.odbc,"UPDATE link_hall_state SET announcement='NONE'" ) ) )
			{
				DogastrophicErrorEx( 0, "Update Failed", "Cannot get to link_hall_state? check sql.log.  Bailing.");
			}
		}
		strcpy( l.command.szExtendedScriptSupportPath, "." );

		noise.bExcuseMe =
			noise.bHolyCow =
			noise.bGetOuttaHere =
			noise.bBeingGreen =
			noise.bWhoaTaskLaunched =
			l.pMyHall->LinkHallState.flags.bHostAssigned  =
			l.pMyHall->LinkHallState.flags.bConnected = 0;

	}
}

void UpdateLinkButtons( void )
{
   PMENU_BUTTON button;
	INDEX idx;
	LIST_FORALL( l.buttons, idx, PMENU_BUTTON, button )
	{
      UpdateButton( button );
	}
}

OnShowControl( WIDE(  "Video Link/Join-Unjoin"  ) )( PTRSZVAL psv )
{
	if( l.pMyHall )
	{
		LOGICAL yesno = l.pMyHall->LinkHallState.enabled;
		InterShell_SetButtonHighlight( (PMENU_BUTTON)psv, yesno );
	}
	else
      lprintf( "Database connection not configured...\nor is currently disconnected." );
}

OnKeyPressEvent( WIDE( "Video Link/Join-Unjoin" ) )( PTRSZVAL psv )
{
	if( l.pMyHall )
	{
		LOGICAL yesno = !l.pMyHall->LinkHallState.enabled;
		SQLCommandf( g.odbc, "update link_hall_state set enabled=%d where hall_id=%d", yesno, l.pMyHall->LinkHallState.hall_id );
		InterShell_SetButtonHighlight( (PMENU_BUTTON)psv, yesno );
	}
	else
      lprintf( "Database connection not configured...\nor is currently disconnected." );
}

OnCreateMenuButton( WIDE( "Video Link/Join-Unjoin" ) )( PMENU_BUTTON menubutton )
{
	InterShell_SetButtonStyle( menubutton, WIDE(  "bicolor square"  ) );
	InterShell_SetButtonColors( menubutton, BASE_COLOR_WHITE, BASE_COLOR_BROWN
							  , BASE_COLOR_BLACK, BASE_COLOR_LIGHTGREEN );
   AddLink( &l.buttons, menubutton );
   return (PTRSZVAL)menubutton;
}

OnShowControl( WIDE(  "Video Link/Claim Delegate"  ) )( PTRSZVAL psv )
{
	if( l.pMyHall )
	{
		LOGICAL yesno = (l.current_state.LinkState.delegated_master_hall_id == l.pMyHall->LinkHallState.hall_id );
		InterShell_SetButtonHighlight( (PMENU_BUTTON)psv, yesno );
	}
	else
      lprintf( "Database connection not configured...\nor is currently disconnected." );
}

OnKeyPressEvent( WIDE( "Video Link/Claim Delegate" ) )( PTRSZVAL psv )
{
	if( l.pMyHall )
	{
		LOGICAL yesno = (l.current_state.LinkState.delegated_master_hall_id == l.pMyHall->LinkHallState.hall_id );
		if( yesno )
			SQLCommandf( g.odbc, "update link_state set delegated_master_hall_id=%d", 0 );
		else
			SQLCommandf( g.odbc, "update link_state set delegated_master_hall_id=%d", l.pMyHall->LinkHallState.hall_id );

		InterShell_SetButtonHighlight( (PMENU_BUTTON)psv, yesno );
	}
	else
      lprintf( "Database connection not configured...\nor is currently disconnected." );
}

OnCreateMenuButton( WIDE( "Video Link/Claim Delegate" ) )( PMENU_BUTTON menubutton )
{
	InterShell_SetButtonStyle( menubutton, WIDE(  "bicolor square"  ) );
   InterShell_SetButtonText( menubutton, "Link_Verify" );
	InterShell_SetButtonColors( menubutton, BASE_COLOR_WHITE, BASE_COLOR_NICE_ORANGE
							  , BASE_COLOR_BLACK, BASE_COLOR_LIGHTGREEN );
   AddLink( &l.buttons, menubutton );
   return (PTRSZVAL)menubutton;
}


static void CPROC LoadAPlugin( PTRSZVAL psv, CTEXTSTR name, int flags )
{
	lprintf( "Loading Video Server plugin : %s", name );
	LoadFunction( name, NULL );
}

static void LoadVideoPlugins( void )
{
	void *info = NULL;
#ifdef WIN32
	char myname[256];
	char *my_path_end;
	HMODULE mod;
	//GetModuleFileName(NULL,myname,sizeof(myname));
	lprintf( "Loading %s", TARGETNAME );
	mod = LoadLibrary( TARGETNAME );
	lprintf( "result %p", mod );
	GetModuleFileName( mod,myname,sizeof(myname));
	lprintf( "Full name %s", myname );
	//GetModuleFileName( TARGETNAME, myname, sizeof( myname ) );
	my_path_end = (char*)pathrchr( myname );
	if( my_path_end )
	{
		my_path_end[0] = 0;
		while( ScanFiles( myname, "*.vplug", &info, LoadAPlugin, 0, 0 ) );
	}
	else
#endif
		while( ScanFiles( GetProgramPath(), "*.vplug", &info, LoadAPlugin, 0, 0 ) );
}



#ifdef INTERSHELL_PLUGIN
static PTRSZVAL CPROC DoCommonInit( PTHREAD thread )
{
	CommonInit();
   LoadVideoPlugins();
   return ServerCheckStateThread( thread );
}
#endif

static void CPROC MyDumpNames( void )
{
   DumpRegisteredNames();
}

#ifdef INTERSHELL_PLUGIN
PRELOAD( MyInitHook )
{
   ThreadTo( DoCommonInit, 0 );
}
#else

int main(int argc, char **argv, char **env)
{
   CommonInit();
	LoadVideoPlugins();
	RegisterIcon( NULL );
   AddSystrayMenuFunction( "Dump Names", MyDumpNames );
	ThreadTo( ServerCheckStateThread, 0);

	while( !g.flags.bExit )
	{
		WakeableSleep(50000);
	}
   UnregisterIcon( );
	return 0;
}
#endif
