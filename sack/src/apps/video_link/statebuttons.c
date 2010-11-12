/*
 *  statbuttons.c
 *  Copyright: FortuNet, Inc.  2006
 *  Product:  BingLink      Project: alpha2server   Version: Alpha2
 *  Contributors:    Jim Buckeyne
 *  The real functions with hooks into the InterShell-based GUI.
 *
 */


//#define USE_KEYPRESSGUARD 1
//#define USE_RESETGUARD 1

#define USES_MILK_INTERFACE
#define DEFINES_MILK_INTERFACE
#define DEFINE_DEFAULT_IMAGE_INTERFACE
#define DEFINE_STATEBUTTONS_GLOBAL 1
#include <timers.h>
#include <pssql.h>
#include <psi.h>
#include <configscript.h>
#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#include <InterShell/InterShell_registry.h>
#include <InterShell/InterShell_button.h>
#include <InterShell/InterShell_export.h>
#include <widgets/banner.h>
#include <pssql.h>
#include <sqlgetoption.h>

#include "statebuttons.h"
#include "db.h"

#ifndef BASE_COLOR_PURPLE
#define BASE_COLOR_PURPLE 0xfff1769d
#endif

enum Local_control_ids{
	COLOR_NO_MASTER = 2000
							 , COLOR_MASTER
                      , COLOR_PARTICIPANT
							 , COLOR_DELEGATE
							 , BUTTON_CREATE_HALL
							 , TEXT_DSN
							 , LST_HALL
                      , LST_BUTTON_MODES
};

enum button_type { UNINITIALIZED
                 , SELECT_HOST
					  , ENABLE_LINK
					  , ENABLE_DELEGATE
					  , DISABLE_LINK
                 , DISABLE_MASTER_HOST
					  , PROHIBITED_MODE
					  , DISABLE_DELEGATE
                 , ENABLE_OLDSTYLE
                 , DISCONNECT_OLDSTYLE
					  , MAX_BUTTON_TYPE
};

// yay visual studio is still lacking c99 initializers...
#ifdef _MSC_VER
#define ABSOLUTE_EQUATE(n)   
#else
#define ABSOLUTE_EQUATE(n)  [n]=
#endif

char *button_modes[] = { ABSOLUTE_EQUATE( UNINITIALIZED )"Undefined"
							  , ABSOLUTE_EQUATE( SELECT_HOST )"Select Host(Master)"
							  , ABSOLUTE_EQUATE( ENABLE_LINK ) "Enable Link"
							  , ABSOLUTE_EQUATE( ENABLE_DELEGATE ) "Enable Delegate Host"
							  , ABSOLUTE_EQUATE( DISABLE_LINK ) "Disable Link"
							  , ABSOLUTE_EQUATE( DISABLE_MASTER_HOST ) "Disable Host(Master)"
							  , ABSOLUTE_EQUATE( PROHIBITED_MODE ) "Prohibited Mode"
							  , ABSOLUTE_EQUATE( DISABLE_DELEGATE ) "Disable Delegated Host"
							  , ABSOLUTE_EQUATE( ENABLE_OLDSTYLE ) "Enable Old Style"
                       , ABSOLUTE_EQUATE( DISCONNECT_OLDSTYLE ) "Disconnect Old Style"
};

typedef struct button_info
{
	struct {
		_32 bUpdateButton : 1;
	} flags;
   enum button_type type;
	struct hall_info *hall; // when this is set, also add PBUTTON_INFO to hall->buttons;
   // if this previously was a differnet hall, ,DeleteLink( button->hall->buttons, this_button );

	PMENU_BUTTON button;
} BUTTON_INFO, *PBUTTON_INFO;


static struct {
	struct {
		_32 bCheckedState : 1;
		_32 bOpenedDatabase : 1;
		_32 bUseDate : 1;
	} flags;
	PTHREAD hall_state_monitor;
   INDEX master_hall_id, delegated_master_hall_id;
	INDEX _master_hall_id, _delegated_master_hall_id;
	PODBC odbc;
	TEXTSTR pDSN;
	CRITICALSECTION cs_sql;
	struct {
      CDATA no_master;
		CDATA master;
		CDATA participant;
      CDATA delegate;
	} color;
	PVARIABLE host_mode_var;
} l;

PRELOAD( RegsisterLocalIDs )
{
   l.pDSN = StrDup( "vsrvr" );
   SimpleRegisterResource( LST_HALL, "Listbox" );
   SimpleRegisterResource( LST_BUTTON_MODES, "Listbox" );
   SimpleRegisterResource( COLOR_NO_MASTER, "Color Well" );
   SimpleRegisterResource( COLOR_MASTER, "Color Well" );
   SimpleRegisterResource( COLOR_PARTICIPANT, "Color Well" );
	SimpleRegisterResource( COLOR_DELEGATE, "Color Well" );
	SimpleRegisterResource( BUTTON_CREATE_HALL, NORMAL_BUTTON_NAME );
	SimpleRegisterResource( TEXT_DSN, EDIT_FIELD_NAME );
	l.flags.bUseDate = SACK_GetProfileInt( GetProgramName(), "Use bingoday for link state (else use 0)", 0 );


}




CTEXTSTR LinkAliveText( PTRSZVAL psvUser )
{
	PHALL_INFO hall = (PHALL_INFO)psvUser;
	if( hall->seconds_ago_alive < 60 )
	{
		//seconds
		snprintf( hall->second_alive_buffer, sizeof( hall->second_alive_buffer ), "%d Second%s", hall->seconds_ago_alive, (hall->seconds_ago_alive==1)?"":"s" );
	}
	else if( hall->seconds_ago_alive < 3600 )
	{
		// min:sec
		snprintf( hall->second_alive_buffer, sizeof( hall->second_alive_buffer )
				  , "%d:%02d Minutes"
				  , hall->seconds_ago_alive / 60
				  , hall->seconds_ago_alive % 60
				  );
	}
	else if( hall->seconds_ago_alive < 86400 )
	{
		// hour:min
		snprintf( hall->second_alive_buffer, sizeof( hall->second_alive_buffer )
				  , "%d:%02d:%02d Hours"
				  , ( hall->seconds_ago_alive / 3600 )
				  , ( hall->seconds_ago_alive / 60 ) % 60
				  , ( hall->seconds_ago_alive ) % 60
				  );
	}
	else
	{
      // day hour
		snprintf( hall->second_alive_buffer, sizeof( hall->second_alive_buffer ), "%d day%s %02d:%02d:%02d"
				  , hall->seconds_ago_alive/86400, ((hall->seconds_ago_alive/86400)==1)?"":"s"
				  , (hall->seconds_ago_alive/3600)%24
				  , (hall->seconds_ago_alive/60)%60
				  , (hall->seconds_ago_alive)%60
				  );
	}
   return hall->second_alive_buffer;
}


CTEXTSTR LinkStateText( PTRSZVAL psvUser )
{
	PHALL_INFO hall = (PHALL_INFO)psvUser;

#ifdef USE_B_STATE_READ
	if( !hall->flags.bStateRead )
	{
		return "Unknown";
	}
#endif
	if( hall )
	{
		if( hall->flags.bDisabled )
		{
			if( hall->flags.bEnabled )
				return "Prohibited, Enabled";
			if( hall->flags.bMaster )
				return "Prohibited, Master";
			if( hall->flags.bDelegate )
				return "Prohibited, Guest Host";
			if( hall->flags.bParticipating )
            return "Prohibited, Participant";
         return "Prohibited";
		}
		if( hall->flags.bEnabled )
		{
			if( hall->flags.bLaunching )
			{
            return "Loading...";
			}
			if( hall->flags.bDelegate )
			{
				return "Guest Host";
			}
			if( hall->flags.bMaster )
			{
            lprintf("I guess I'm master ( %s )" , hall->name);
				return "Hosting";
			}
			if( hall->flags.bParticipating )
			{
				return "Participating";
			}
         return "Enabled...";
		}
		else
		{
			if( hall->flags.bLaunching )
			{
            return "Loading...";
			}
			if( hall->flags.bDelegate )
			{
				return "Disconnecting... Guest host";
			}
			if( hall->flags.bMaster )
			{
				return "Disconnecting... Master";
			}
			if( hall->flags.bParticipating )
			{
				return "Disconnecting... Participant";
			}
         return "Disconnected";
		}
	}
   return "Invalid Hall";
}

void OpenDatabase( void )
{
	if( l.flags.bOpenedDatabase )
		return;
	if( !l.pDSN )
		l.odbc = ConnectToDatabase( "MySQL" );
   else
		l.odbc = ConnectToDatabase( l.pDSN );
	SetSQLLoggingDisable( l.odbc, !SACK_GetProfileInt( GetProgramName(), "Log SQL", 0 ) );

   if( l.odbc )
	{
		CTEXTSTR *results;
      CheckMyTables( l.odbc );
		for( SQLRecordQueryf( l.odbc, NULL, &results, NULL, "select id,packed_name from location" );
			 results;
			  FetchSQLRecord( l.odbc, &results ) )
		{
			lprintf( "Adding hall %s=%s", results[0], results[1] );
			if( ( results[0]!=NULL ) && ( results[1] !=NULL ) )
			{
				char buffer[64];
				PHALL_INFO hall = New( HALL_INFO );
				MemSet( hall, 0, sizeof( HALL_INFO ) );
				//hall->buttons = NULL;
				hall->name = StrDup( results[1] );
				hall->hall_id = atoi( results[0] );
				snprintf( buffer, sizeof( buffer ), "<Hall State:%s>", hall->name );
				hall->label_var = CreateLabelVariableEx( buffer, LABEL_TYPE_PROC_EX, LinkStateText, (PTRSZVAL)hall );
				snprintf( buffer, sizeof( buffer ), "<Hall Last Alive:%s>", hall->name );
				hall->label_alive_var = CreateLabelVariableEx( buffer, LABEL_TYPE_PROC_EX, LinkAliveText, (PTRSZVAL)hall );
				lprintf("%s for %s", hall->name, buffer );

				AddLink( &g.halls, hall );
			}
			else
				xlprintf(LOG_NOISE)("Bogus~!");
		}
	}
	else
	{
		Banner2Message( "Failed to open Link State ODBC!\n"
						  "Check Link State ODBC DSN setting in config\n" );
	}
   l.flags.bOpenedDatabase = 1;
}

PHALL_INFO GetHall( _32 ID )
{
   PHALL_INFO hall;
	INDEX idx;
	LIST_FORALL( g.halls, idx, PHALL_INFO, hall )
		if( hall->hall_id == ID )
         break;
   return hall;
}

PHALL_INFO GetHallByName( char *name )
{
   PHALL_INFO hall;
	INDEX idx;
	lprintf( "seekingAdding hall %s", name );
	LIST_FORALL( g.halls, idx, PHALL_INFO, hall )
      if( hall->name )
			if( StrCaseCmp( hall->name, name ) == 0 )
			{
            lprintf( "found." );
				break;
			}
	if( !hall )
      lprintf( " NOT found" );
   return hall;
}

PTRSZVAL CPROC CheckStateThread( PTHREAD thread )
{
   int changed_master_state = 0;
	int changed_states = 0;
   int first = 1;
	CTEXTSTR *results;
	LOGICAL bResetInProgress = FALSE;

	l.hall_state_monitor = MakeThread();

	while( 1 )
	{
		if( !l.odbc )
		{
			xlprintf( LOG_ERROR )( "Database connection did not exist, does not work." );
         continue;
         //return;
		}
      bResetInProgress=FALSE;
      EnterCriticalSec( &l.cs_sql );
		changed_states = 0;
		changed_master_state = 0;
		if( SQLRecordQueryf( l.odbc, NULL, &results, NULL, "select master_hall_id,delegated_master_hall_id from link_state where bingoday=%s"
								 , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0" ) && results )
		{
			l.master_hall_id = atoi( results[0] );
			l.delegated_master_hall_id = atoi( results[1] );
			if( l._master_hall_id != l.master_hall_id ||
				l._delegated_master_hall_id != l.delegated_master_hall_id )
			{
				changed_master_state = 1;
			}
         l._master_hall_id = l.master_hall_id;
         l._delegated_master_hall_id = l.delegated_master_hall_id;
		}
		for( SQLRecordQueryf( l.odbc, NULL, &results, NULL, "select hall_id,enabled,master_ready,delegate_ready,participating,prohibited,task_launched,dvd_active,media_active from link_hall_state" );
			 results;
			  FetchSQLRecord( l.odbc, &results ) )
		{
			_32 hall_id = atoi( results[0] );
			PHALL_INFO hall = GetHall( hall_id );
			if( hall )
			{
				hall->flags.bEnabled = atoi( results[1] );
				hall->flags.bMaster = atoi( results[2] );
				hall->flags.bDelegate = atoi( results[3] );
				hall->flags.bParticipating = atoi( results[4] );
				hall->flags.bDisabled = atoi( results[5] );
				hall->flags.bLaunching = atoi( results[6] );
				hall->flags.bDVDActive  = atoi( results[7] );
				hall->flags.bMediaActive  = atoi( results[8] );
#ifdef NOISY_FLAGS_LOGGING
				lprintf( "flags for %u : %d %d %d %d %d %d %d %d" //hey, wait a tick.  aren't these _32's?
						 , hall_id
						 , hall->flags.bEnabled
						 , hall->flags.bMaster
						 , hall->flags.bDelegate
						 , hall->flags.bParticipating
						 , hall->flags.bDisabled
						 , hall->flags.bLaunching
						 , hall->flags.bDVDActive
						 , hall->flags.bMediaActive
						 );
				lprintf( "_flags for %u  : %d %d %d %d %d %d %d %d"
						 , hall_id
						 , hall->flags._bEnabled
						 , hall->flags._bMaster
						 , hall->flags._bDelegate
						 , hall->flags._bParticipating
						 , hall->flags._bDisabled
						 , hall->flags._bLaunching
						 , hall->flags._bDVDActive
						 , hall->flags._bMediaActive
						 );
#endif
				if( changed_master_state
					|| ( hall->flags.bEnabled != hall->flags._bEnabled )
					|| ( hall->flags.bDisabled != hall->flags._bDisabled )
               || ( hall->flags.bMaster != hall->flags._bMaster )
               || ( hall->flags.bDelegate != hall->flags._bDelegate )
					|| ( hall->flags.bParticipating != hall->flags._bParticipating )
					|| ( hall->flags.bLaunching != hall->flags._bLaunching )
				  )
				{
					INDEX idx;
					PBUTTON_INFO button;
					LIST_FORALL( hall->buttons, idx, PBUTTON_INFO, button )
					{
                  button->flags.bUpdateButton = 1;
					}
               hall->flags.bStateChanged = 1;
//					if( hall->flags.bMaster )//just some minor troubleshooting
					{
						changed_states = 1;
					}
				}
				if( !hall->flags.bStateRead )
				{
					hall->flags.bStateChanged = 1;
               hall->flags.bStateRead = 1;
				}
            hall->flags._bEnabled = hall->flags.bEnabled;
            hall->flags._bMaster = hall->flags.bMaster;
            hall->flags._bDelegate = hall->flags.bDelegate;
            hall->flags._bParticipating = hall->flags.bParticipating;
            hall->flags._bDisabled = hall->flags.bDisabled;
				hall->flags._bLaunching = hall->flags.bLaunching;
				if( hall->flags.bDVDActive != hall->flags._bDVDActive )
				{
					hall->flags._bDVDActive = hall->flags.bDVDActive;
               lprintf( "Need to fire an event here - but isn't this for the server to dispatch?" );
					//DVDActiveStateChangedEach( hall->hall_id , hall->flags.bDVDActive );
				}
				if( hall->flags.bMediaActive != hall->flags._bMediaActive )
				{
					hall->flags._bMediaActive = hall->flags.bMediaActive;
               lprintf( "Need to fire an event here - but isn't this for the server to dispatch?" );
					//MediaActiveStateChangedEach( hall->hall_id , hall->flags.bMediaActive );
				}
#ifdef USE_RESETGUARD
				if( g.resetguard.pressed )
				{
					bResetInProgress =  bResetInProgress ||  hall->flags.bEnabled;
					if( hall->flags.bEnabled )
						lprintf("Still waiting for %u to be un-bEnabled");
				}
#endif
			}
		}
		if( !first && changed_states )
		{
         LabelVariableChanged( l.host_mode_var );
		}
		first = 0;
      l.flags.bCheckedState = 1;

		for( SQLRecordQueryf( l.odbc, NULL, &results, NULL, "select hall_id,timestampdiff( second, last_alive, now() ) from link_alive" );
			 results;
			  FetchSQLRecord( l.odbc, &results ) )
		{
			PHALL_INFO hall = GetHall( atoi( results[0] ) );
			if( hall )
			{
				_32 seconds = atoi( results[1] );
				if( seconds != hall->seconds_ago_alive )
				{
					hall->seconds_ago_alive = atoi( results[1] );
					LabelVariableChanged( hall->label_alive_var );
				}
			}
		}

		LeaveCriticalSec( &l.cs_sql );
		{
			INDEX idx_hall;
			PHALL_INFO hall;
			LIST_FORALL( g.halls, idx_hall, PHALL_INFO, hall )
			{
				INDEX idx;
				PBUTTON_INFO button;

				if( hall->flags.bStateChanged )
				{
					LabelVariableChanged( hall->label_var );
               hall->flags.bStateChanged = 0;
				}
				LIST_FORALL( hall->buttons, idx, PBUTTON_INFO, button )
				{
					if( button->flags.bUpdateButton )
					{
						button->flags.bUpdateButton = 0;
//						lprintf( "Update %p", button->button );
						UpdateButton( button->button );
					}
				}
			}
		}

#ifdef USE_RESETGUARD
		if( g.resetguard.pressed )
		{
			if( ( ( GetTickCount() - g.resetguard.pressed ) > 2000 ) && (!bResetInProgress) )
			{
				lprintf("Looks like we're all reset here,  setting g.resetguard.pressed  to zero, was %lu"
						 , g.resetguard.pressed
						 );
				g.resetguard.pressed = 0;
				if( g.resetguard.btn )
					UpdateButton( g.resetguard.btn );
				else
               lprintf("Hey, where is g.resetguard.btn?");

			}
			else
			{
				lprintf("Hm. g.resetguard.pressed is %lu and bResetInProgress is %u"
						 , g.resetguard.pressed
						 , bResetInProgress
						 );
			}
		}
#endif



		// this really could be in it's own timer, but this is arguably efficient enough.
#ifdef USE_KEYPRESSGUARD
      if( g.keypressguard.flags.remind )
		{
			_64 t = GetTickCount() ;
			if( ( ( t - g.keypressguard.pressed ) / 1000 ) < g.keypressguard.period )
			{
				if( !g.keypressguard.flags.reminding )
				{
					g.keypressguard.label_str = StrDup( "Please Wait..." );
					LabelVariableChanged( g.keypressguard.label_var );
					g.keypressguard.flags.reminding = 1;
				}
				else
				{
					switch( g.keypressguard.flags.ucReminded )
					{
					case 4:
						{
							g.keypressguard.label_str = StrDup( "Please Wait..." );
						}
                  break;
					case 3:
						{
							g.keypressguard.label_str = StrDup( "...Please Wait" );
						}
                  break;
					case 2:
						{
							g.keypressguard.label_str = StrDup( "it...Please Wa" );
						}
                  break;
					case 1:
						{
							g.keypressguard.label_str = StrDup( " Wait...Please" );
						}
                  break;
					case 0:
					default:
						{
							g.keypressguard.label_str = StrDup( "ase Wait...Ple" );
						}
                  break;
					}
					( g.keypressguard.flags.ucReminded >= 4 ) ? ( g.keypressguard.flags.ucReminded = 0 ) : ( g.keypressguard.flags.ucReminded++ );
					LabelVariableChanged( g.keypressguard.label_var );

				}
			}
			else
			{
				if( g.keypressguard.flags.reminding )
				{
					g.keypressguard.label_str = StrDup( " " );
					LabelVariableChanged( g.keypressguard.label_var );
					g.keypressguard.flags.reminding = g.keypressguard.flags.remind = 0;
				}

			}
		}
#endif

		WakeableSleep( 1000 );
	}
   return 0;
}

void CPROC CheckState( PTRSZVAL psv, char *extra )
{
   lprintf( "wakeup" );
	WakeThread( l.hall_state_monitor );
}

CTEXTSTR GetHostSelectMessage( void )
{
	if( l.master_hall_id )
	{
		return "Enable Participants";
	}
	else
	{
		return "Select Host";
	}
}

OnFinishInit("Enable Participant")( void )
{
	INDEX i_hall;
	PHALL_INFO hall;
	INDEX i_button;
	PBUTTON_INFO button;

	if( !l.odbc )
		OpenDatabase();

	{
		INDEX idx;
		LIST_FORALL( g.halls, idx, PHALL_INFO, hall )
		{
			// create halls we know about.
			SQLCommandf( l.odbc, "insert ignore into link_hall_state (hall_id) values (%ld)"
							 , hall->hall_id );
		}

	}
#ifdef USE_RESETGUARD
	//	g.resetguard.pressed = GetTickCount();
#endif

	ThreadTo( CheckStateThread, 0 );
	while( !l.hall_state_monitor )
		Relinquish();

	l.host_mode_var = CreateLabelVariable( "Host Mode Select", LABEL_TYPE_PROC, GetHostSelectMessage );

	LIST_FORALL( g.halls, i_hall, PHALL_INFO, hall )
	{
		LIST_FORALL( hall->buttons, i_button, PBUTTON_INFO, button )
		{
         //FixupButton( button->button );
		}
	}
}

void DumpHallInfo( PHALL_INFO hall )
{
	if( hall )
	{
      lprintf( "hall    %s", hall->name );
		lprintf( "enabled %d", hall->flags.bEnabled );
		lprintf( "partici %d", hall->flags.bParticipating );
		lprintf( "master  %d", hall->flags.bMaster );
		lprintf( "delega  %d", hall->flags.bDelegate );
		lprintf( "launch  %d", hall->flags.bLaunching );
		lprintf( "dvd_ac  %d", hall->flags.bDVDActive );
		lprintf( "media_  %d", hall->flags.bMediaActive );
	}
	else
      lprintf( "NO HALL" );
}

OnShowControl( "Enable Participant" )( PTRSZVAL psv )
{
	PBUTTON_INFO button_info = (PBUTTON_INFO)psv;
	CDATA foreground;
#ifdef NOISY_FLAGS_LOGGING
	lprintf( "Opportunity to set the color here... l.master_hall_id is %u "
          , l.master_hall_id
			 );
	DumpHallInfo( button_info->hall );
#endif
	if( !l.master_hall_id )
	{
      if( button_info->hall )
			foreground = l.color.no_master;
		else
		{
#ifdef NOISY_FLAGS_LOGGING
			lprintf("COLOR_IGNORE is %08X"
					 , COLOR_IGNORE
                 );
#endif
			foreground = COLOR_IGNORE;
		}
	}
	else if( button_info->hall )
	{
#ifdef NOISY_FLAGS_LOGGING
		lprintf(" button_info->hall->hall_id is %u vs l.master_hall_id %u"
				 , button_info->hall->hall_id
				 , l.master_hall_id
				 );
#endif
		if( button_info->hall->hall_id == l.master_hall_id )
		{
#ifdef NOISY_FLAGS_LOGGING
			lprintf(" l.color.master is %08X"
					 , l.color.master
                 );
#endif
			foreground = l.color.master;
		}
		else
		{
#ifdef NOISY_FLAGS_LOGGING
			lprintf(" l.color.participant is %08X"
					 , l.color.participant
					 );
#endif
			foreground = l.color.participant;
		}
	}
	else
	{
#ifdef NOISY_FLAGS_LOGGING
		lprintf("else COLOR_IGNORE? %08X"
				 , COLOR_IGNORE
				 );
#endif
		foreground = COLOR_IGNORE;
	}

	switch( button_info->type )
	{
	case DISABLE_MASTER_HOST:
#ifdef USE_RESETGUARD
		lprintf("DISABLE_MASTER_HOST g.resetguard.pressed  is %lu", g.resetguard.pressed  );
		if( !g.resetguard.pressed )
		{
			InterShell_SetButtonColor( button_info->button
								 , 0xFF77967C
								 , 0xFF005700
									 );
		}
		else
		{
			InterShell_SetButtonColor( button_info->button
								 , 0xFFDFA0B0
								 , 0xFF555555
									 );
		}
#endif
         break;

	case ENABLE_LINK:
		if( !button_info->hall )
         return;
      /*
		if( !l.master_hall_id )
		{
         if( button_info->hall->flags.bDisabled )
				InterShell_SetButtonColor( button_info->button, AColor( 0, 0, 0, 32 ), AColor( 0, 0, 0, 255 ) );
			else if( button_info->hall->flags.bEnabled )
            // something like brown color
				InterShell_SetButtonColor( button_info->button, foreground, 0xFFA6753A );
         else
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
		}
 		else if( !button_info->hall )
         InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_PURPLE );

		else if( button_info->hall->hall_id == l.master_hall_id )
		{
         if( button_info->hall->flags.bEnabled )
				InterShell_SetButtonColor( button_info->button, foreground, AColor( 0, 0, 255, 120 ) );
		}
		else
		*/
		if( button_info->hall->flags.bEnabled )
		{

#ifdef NOISY_FLAGS_LOGGING
			lprintf( " enabled... Enabled %u Master %u Participating %u Disabled %u Launching %u DVDActive %u MediaActive %u"
					 , button_info->hall->flags.bEnabled
					 , button_info->hall->flags.bMaster
					 , button_info->hall->flags.bParticipating
					 , button_info->hall->flags.bDisabled
					 , button_info->hall->flags.bLaunching
					 , button_info->hall->flags.bDVDActive
					 , button_info->hall->flags.bMediaActive
					 );
#endif
			if( button_info->hall->flags.bDisabled )
			{
				InterShell_SetButtonColor( button_info->button, AColor( 0, 0, 0, 32 ), AColor( 0, 0, 0, 255 ) );
			}
			else if( button_info->hall->flags.bLaunching )
			{
				InterShell_SetButtonColor( button_info->button, foreground, Color( 210, 210, 0 ) );
			}
			else if( button_info->hall->flags.bParticipating )
			{
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_LIGHTGREEN );
#if USE_KEYPRESSGUARD
				if( g.keypressguard.period )
					g.keypressguard.pressed = ( GetTickCount() - ( ( g.keypressguard.period - 1 ) * 1000 ) );
#endif
			}
			else if( button_info->hall->flags.bMaster )
			{
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_LIGHTBLUE );
#if USE_KEYPRESSGUARD
				if( g.keypressguard.period )
					g.keypressguard.pressed = ( GetTickCount() - ( ( g.keypressguard.period - 1 ) * 1000 ) );
#endif

			}
                        else if( button_info->hall->flags.bDelegate )
                        {
                            InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_MAGENTA );
                        }
                        else
                        {
                            InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_RED );
                        }

		}
		else
		{
			if( button_info->hall->flags.bDisabled )
			{
				InterShell_SetButtonColor( button_info->button, AColor( 0, 0, 0, 32 ), AColor( 0, 0, 0, 255 ) );
			}
			else if( button_info->hall->flags.bLaunching )
			{
				InterShell_SetButtonColor( button_info->button, foreground, ColorAverage( BASE_COLOR_BLACK, Color( 210, 210, 0 ), 33, 100 ) );
			}
			else if( button_info->hall->flags.bParticipating )
			{
				InterShell_SetButtonColor( button_info->button, foreground, ColorAverage( BASE_COLOR_BLACK, BASE_COLOR_LIGHTGREEN, 33, 100 ) );
			}
			else if( button_info->hall->flags.bMaster )
			{
				InterShell_SetButtonColor( button_info->button, foreground, ColorAverage( BASE_COLOR_BLACK, BASE_COLOR_LIGHTBLUE, 33, 100 ) );
			}
			else if( button_info->hall->flags.bDelegate )
			{
				InterShell_SetButtonColor( button_info->button, foreground, ColorAverage( BASE_COLOR_BLACK, BASE_COLOR_MAGENTA, 33, 100 ) );
			}
			else
			{
				InterShell_SetButtonColor( button_info->button, foreground, ColorAverage( BASE_COLOR_BLACK, BASE_COLOR_RED, 33, 100 ) );
			}
			//InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
		}
		break;
	case ENABLE_DELEGATE:
		if( l.delegated_master_hall_id )
		{
			if( !button_info->hall )
            InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_PURPLE );
			else if( l.delegated_master_hall_id == button_info->hall->hall_id )
			{
				if( button_info->hall->flags.bDelegate )
               InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_LIGHTGREEN );
				else if( button_info->hall->flags.bLaunching )
					InterShell_SetButtonColor( button_info->button, foreground, Color( 210, 210, 0 ) );
				else
               InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_RED );
			}
         else
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
		}
		else
		{
			InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
		}
      break;
	case SELECT_HOST:
		if( l.master_hall_id )
		{
			if( !button_info->hall )
            InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_PURPLE );
			else if( l.master_hall_id == button_info->hall->hall_id )
			{
				if( button_info->hall->flags.bMaster )
					InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_LIGHTGREEN );
				else if( button_info->hall->flags.bLaunching )
					InterShell_SetButtonColor( button_info->button, foreground, Color( 210, 210, 0 ) );
				else
               InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_RED );

			}
			else
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );

		}
		else
		{
			if( !button_info->hall )
            InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_PURPLE );
			else if( button_info->hall && button_info->hall->flags.bDisabled )
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_ORANGE );
         else
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
		}
      break;
	case ENABLE_OLDSTYLE:
		InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
      break;
	case DISCONNECT_OLDSTYLE:
		InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
      break;
	case PROHIBITED_MODE:
		if( !button_info->hall )
			InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_PURPLE );
		else if( button_info->hall->hall_id != l.master_hall_id )
		{
			if( button_info->hall->flags.bDisabled )
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_LIGHTRED );
			else
				InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_BLACK );
		}
		else
		{
			InterShell_SetButtonColor( button_info->button, foreground, BASE_COLOR_ORANGE );
		}
      break;
	}
	if( button_info->hall )
	{
//      lprintf("Let's LabelVariableChanged in OnShowControl for %s", button_info->hall->name );
		LabelVariableChanged( button_info->hall->label_var );
	}

}

OnQueryShowControl( "Enable Participant" )( PTRSZVAL psv )
{
	PBUTTON_INFO button_info = (PBUTTON_INFO)psv;
//   lprintf( "consider visibility of %s", button_modes[button_info->type] );
	switch( button_info->type )
	{
	case DISABLE_DELEGATE:
		if( l.delegated_master_hall_id )
			return TRUE;
      return FALSE;
	case ENABLE_DELEGATE:
		if( !l.master_hall_id )
			return FALSE;
		if( button_info->hall )
		{
			if( button_info->hall->flags.bDisabled || !button_info->hall->flags.bEnabled )
				return FALSE;
			if( button_info->hall->hall_id == l.master_hall_id )
				return FALSE;
			if( l.delegated_master_hall_id &&
				( l.delegated_master_hall_id != button_info->hall->hall_id ) )
				return FALSE;
		}
      break;
	case DISABLE_MASTER_HOST:
		// always show this... cause this button
		// also serves as a 'master reset' button ... that is it
      // resets prohibited participation states.
		//if( !l.master_hall_id )
		//	return FALSE;
      return TRUE;
      break;
	case ENABLE_LINK:
      return TRUE;
		if( !l.master_hall_id )
			return FALSE;
		if( button_info->hall )
		{
			if( button_info->hall->flags.bDisabled )
				return FALSE;
		}
		break;
	case SELECT_HOST:
		if( button_info->hall )
		{
			lprintf( "master ID is %ld and I am %ld", l.master_hall_id, button_info->hall->hall_id );
			if( l.master_hall_id && button_info->hall->hall_id != l.master_hall_id )
				return FALSE;
		}
		break;
	}
   return TRUE;
}

void GenerateEvent( void )
{
	l.flags.bCheckedState = 0;
   lprintf( "wakeup" );
	WakeThread( l.hall_state_monitor );
	//BARD_IssueSimpleEvent( "extern video state change:check video state" );
	while( !l.flags.bCheckedState )
		Relinquish();
}
LOGICAL AllowLinkStateKeypress( void )
{
#ifdef USE_KEYPRESSGUARD
	// specifying zero or less than zero in "Seconds between keypresses is valid,
	// just means keypressguard is ignored.  Also, if this line is never translated
	// from the config file (aka doesn't exist) this value will be zero, ignoring
	// the keypressguard.
	if( g.keypressguard.period )
	{
		_64 t;
		t = GetTickCount();
		//  Either this is the first keypress or a validly-timed keypress
		if( ( !g.keypressguard.pressed ) ||
			( ( ( t - g.keypressguard.pressed ) / 1000 ) >= g.keypressguard.period )
		  )
		{
			g.keypressguard.pressed = t;
         return TRUE;
		}
		// or this is a keypress soon after the last keypress
		else
		{
//			lprintf("Too Soon for a keypress. Wait for %u more seconds. Returning.", ( g.keypressguard.period - ( ( t - g.keypressguard.pressed ) / 1000 ) ) );
			g.keypressguard.flags.remind = TRUE;
			return FALSE;
		}
	}
	else
      return TRUE;
#else
   return TRUE;
#endif

}
OnKeyPressEvent( "Enable Participant" )( PTRSZVAL psv )
{
   int bEvent = 0;
	//PTRSZVAL psv = *(PTRSZVAL*)ppsv;
	PBUTTON_INFO button_info = (PBUTTON_INFO)psv;

	if( !( AllowLinkStateKeypress()))
      return;

	if( !l.odbc )
		return; // get out of here.


   EnterCriticalSec( &l.cs_sql );
	switch( button_info->type )
	{
	case SELECT_HOST:
		if( button_info->hall )
		{
         // if prohibited, don't allow this operation.
			if( !button_info->hall->flags.bDisabled )
			{
				SQLCommandf( l.odbc, "replace into link_state (master_hall_id,bingoday) select %ld,%s"
							  , button_info->hall->hall_id
							  , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0"
							  );
            bEvent = 1;
			}
		};
		break;
	case ENABLE_OLDSTYLE:
		if( button_info->hall)
		{
			if( !button_info->hall->flags.bDisabled )
			{
				SQLCommandf( l.odbc, "replace into link_state (master_hall_id,bingoday) select %ld,%s"
							  , button_info->hall->hall_id
							  , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0"
							 );
			}
			SQLCommandf( l.odbc, "update link_hall_state set enabled=1" );
			bEvent = 1;
		}
      break;
	case DISCONNECT_OLDSTYLE:
		SQLCommandf( l.odbc, "replace into link_state (master_hall_id,delegated_master_hall_id,bingoday) select 0,0,%s"
					  , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0"
					  );
		SQLCommandf( l.odbc, "replace into link_hall_state(hall_id,enabled,master_ready,delegate_ready,participating,prohibited,task_launched,dvd_active,media_active)"
						"select hall_id,0,0,0,0,0,0,0,0 from link_hall_state" );
      // this should be safe?
		//SQLCommandf( l.odbc, "delete from link_hall_state" );
		bEvent = 1;
      break;
	case DISABLE_MASTER_HOST:
		{
			SQLCommandf( l.odbc, l.flags.bUseDate
							?"replace into link_state (master_hall_id,bingoday) select 0,%s"
							:"replace into link_state (master_hall_id) select 0"
						  , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0"
						  );
#ifdef THE_WAY_IT_USED_TO_BE1
			SQLCommandf( l.odbc, "update link_hall_state set prohibited=0" );
#endif
			SQLCommandf( l.odbc, "update link_hall_state set prohibited=0,enabled=0" );
			{
				INDEX idx;
				PHALL_INFO hall;
				LIST_FORALL( g.halls, idx, PHALL_INFO, hall )
				{
					LabelVariableChanged( hall->label_var );
				}
			}
#ifdef USE_KEYPRESSGUARD
			g.keypressguard.flags.remind=1;
#endif

#ifdef USE_RESETGUARD
			g.resetguard.pressed = GetTickCount();
			UpdateButton( g.resetguard.btn=button_info->button );
			lprintf("Clear!");
#endif
			bEvent = 1;
		}
		break;
	case ENABLE_LINK:
		if( button_info->hall && !l.master_hall_id )
		{
			INDEX idx;
			PHALL_INFO hall;
			LIST_FORALL( g.halls, idx, PHALL_INFO, hall )
			{
				if( hall->flags.bMaster )
               break;
			}
			if( hall )
			{
				Banner2Message( "Sorry, You must wait for all halls to be disabled.\nPlease try again in a few seconds." );
				break;
			}
			SQLCommandf( l.odbc, "replace into link_state (master_hall_id,bingoday) select %ld,%s"
						  , button_info->hall->hall_id
						  , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0"
						  );
			SQLCommandf( l.odbc, "update link_hall_state set enabled=1 where hall_id=%ld"
						  , button_info->hall->hall_id
						  );
			bEvent = 1;
		}
		if( button_info->hall && (button_info->hall->hall_id != l.master_hall_id) )
		{
			SQLCommandf( l.odbc, "update link_hall_state set enabled=%ld where hall_id=%ld"
							 , !button_info->hall->flags.bEnabled
							 , button_info->hall->hall_id );
			bEvent = 1;

		};

      if( button_info->hall )
			LabelVariableChanged( button_info->hall->label_var );

		break;
	case DISABLE_LINK:
		if( button_info->hall )
		{
			SQLCommandf( l.odbc, "update link_hall_state set enabled=%ld where hall_id=%ld"
						  , !button_info->hall->flags.bEnabled
						  , button_info->hall->hall_id );
			bEvent = 1;
		};
		break;
	case ENABLE_DELEGATE:
		if( button_info->hall )
		{
			if( !button_info->hall->flags.bDisabled )
			{
				SQLCommandf( l.odbc, "replace into link_state (delegated_master_hall_id,bingoday) select %ld,%s"
							  , button_info->hall->hall_id
							  , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0"
							  );
				bEvent = 1;
			}
		};
      break;
	case DISABLE_DELEGATE:
		{
			SQLCommandf( l.odbc, "replace into link_state (delegated_master_hall_id,bingoday) select 0,%s"
						  , l.flags.bUseDate?GetSQLOffsetDate( l.odbc, "Video Server" ):"0"
						  );
			bEvent = 1;
		}
      break;
	case PROHIBITED_MODE:
		if( button_info->hall )
		{
			if( button_info->hall->hall_id != l.master_hall_id )
			{
				SQLCommandf( l.odbc, "update link_hall_state set prohibited=1-prohibited where hall_id=%ld"
								 , button_info->hall->hall_id
								 );
				bEvent = 1;
			}
		}
		break;
	default:
		break;
	}
	LeaveCriticalSec( &l.cs_sql );
	if( bEvent )
      GenerateEvent();

}


OnCreateMenuButton( WIDE("Enable Participant") )( PMENU_BUTTON button )
{
	PBUTTON_INFO button_info = Allocate( sizeof( *button_info ) );
	button_info->type = UNINITIALIZED;
	button_info->button = button;
	button_info->hall = NULL;
	InterShell_SetButtonStyle( button, "bicolor square" );
	InterShell_SetButtonColors( button, BASE_COLOR_WHITE, BASE_COLOR_BLUE, BASE_COLOR_GREEN, BASE_COLOR_YELLOW );
	//InterShell_SetButtonText( button, "Button..." );
	return (PTRSZVAL)button_info;
}


void CPROC HallSelected( PTRSZVAL psv, PSI_CONTROL list, PLISTITEM pli )
{
	//PHALL_INFO hall = (PHALL_INFO)GetItemData( pli );

}

void CPROC CreateNewHall( PTRSZVAL psv, PSI_CONTROL button )
{
	char result[256];
	if( SimpleUserQuery( result, sizeof( result ), "Enter a new hall name", button ) )
	{
		PHALL_INFO hall = Allocate( sizeof( HALL_INFO ) );
		MemSet( hall, 0, sizeof( HALL_INFO ) );
		//hall->buttons = NULL;
		hall->name = StrDup( result );
		hall->hall_id = ReadNameTableExx( result, "location", "id", "packed_name", TRUE );
		AddLink( &g.halls, hall );
		SetItemData( AddListItem( GetNearControl( button, LST_HALL ), hall->name ), (PTRSZVAL)hall );
	}
}

OnEditControl( "Enable Participant" )( PTRSZVAL psv, PSI_CONTROL parent )
{
	int okay = 0, done = 0;
	PSI_CONTROL frame;
	PSI_CONTROL list = NULL;
	PSI_CONTROL list2 = NULL;

	PBUTTON_INFO button = (PBUTTON_INFO)psv;
	frame = LoadXMLFrame( "Edit_Link_Button.isFrame" );
	if( !frame )
	{
		// popuplate default, editing was canceled
	}
	if( frame )
	{
		SetCommonButtons( frame, &done, &okay );
		SetCommonButtonControls( frame );
		// a listbox should be filled with all halls, which can be selected for the button.
		{
			PHALL_INFO hall;
			INDEX idx;
         SetControlText( GetControl( frame, TEXT_DSN ), l.pDSN );
			list = GetControl( frame, LST_HALL );
			if( list )
			{
				LIST_FORALL( g.halls, idx, PHALL_INFO, hall )
				{
               PLISTITEM pli;
					SetItemData( pli = AddListItem( list, hall->name ), (PTRSZVAL)hall );
					if( button->hall == hall )
                  SetSelectedItem( list, pli );
				}
			}
			list2 = GetControl( frame, LST_BUTTON_MODES );
			if( list2 )
			{
				int i;
				for( i = 0; i < MAX_BUTTON_TYPE; i++ )
				{
               PLISTITEM pli;
					pli = AddListItem( list2, button_modes[i] );
               SetItemData( pli, i );
					if( i == button->type )
					{
                  SetSelectedItem( list2, pli );
					}
				}
			}
			SetSelChangeHandler( list, HallSelected, 0 );
         EnableColorWellPick( SetColorWell( GetControl( frame, COLOR_NO_MASTER ), l.color.no_master ), TRUE );
         EnableColorWellPick( SetColorWell( GetControl( frame, COLOR_MASTER ), l.color.master ), TRUE );
         EnableColorWellPick( SetColorWell( GetControl( frame, COLOR_PARTICIPANT ), l.color.participant ), TRUE );
			EnableColorWellPick( SetColorWell( GetControl( frame, COLOR_DELEGATE ), l.color.delegate ), TRUE );
         SetButtonPushMethod( GetControl( frame, BUTTON_CREATE_HALL ), CreateNewHall, 0 );
		}

	}
	DisplayFrameOver( frame, parent );
	EditFrame( frame, TRUE );
	CommonWait( frame );

	if( okay )
	{
		TEXTCHAR buffer[64];
		GetControlText( GetControl( frame, TEXT_DSN ), buffer, sizeof( buffer ));
		if( StrCaseCmp( buffer, l.pDSN ) )
		{
         // dsn changed, close old connection.
         Release( l.pDSN );
			l.pDSN = StrDup( buffer );
			CloseDatabase( l.odbc );
         l.odbc = NULL;
		}

		GetCommonButtonControls( frame );
		// get currently selected hall listbox item, save in buton_info structure
		if( list )
		{
			button->hall = (PHALL_INFO)GetItemData( GetSelectedItem( list ) );
		}
		if( list2 )
		{
         button->type = (_8)GetItemData( GetSelectedItem( list2 ) );
		}
      l.color.master = GetColorFromWell( GetControl( frame, COLOR_MASTER ) );
      l.color.no_master = GetColorFromWell( GetControl( frame, COLOR_NO_MASTER ) );
      l.color.participant = GetColorFromWell( GetControl( frame, COLOR_PARTICIPANT ) );
      l.color.delegate = GetColorFromWell( GetControl( frame, COLOR_DELEGATE ) );

	}
	DestroyFrame( &frame );

   // option to change value here?
   return psv;
}

OnSaveControl( WIDE("Enable Participant") )( FILE *file, PTRSZVAL psv )
{
	PBUTTON_INFO pButtonInfo = (PBUTTON_INFO)psv;
	if( pButtonInfo )
	{
		fprintf( file, WIDE("button mode=%s\n"), button_modes[pButtonInfo->type] );
		if( pButtonInfo->hall )
		{
			fprintf( file, WIDE("hall name=%s\n"), pButtonInfo->hall->name );
		}
      else
			fprintf( file, WIDE("hall name=\n") );

	}
//	fprintf( file, WIDE("# button data here...\n"));
}


static PTRSZVAL CPROC RestoreHallID( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, hall_id );
	PBUTTON_INFO pButtonInfo = (PBUTTON_INFO)psv;
	if( ( pButtonInfo->hall = GetHallByName( hall_id ) ) )
		AddLink( &pButtonInfo->hall->buttons, pButtonInfo );
   return psv;
}

static PTRSZVAL CPROC RestoreButtonType( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, type );
	PBUTTON_INFO pButtonInfo = (PBUTTON_INFO)psv;
	{
		INDEX idx;
		for( idx = 0; idx < MAX_BUTTON_TYPE; idx++ )
		{
			if( StrCaseCmp( type, button_modes[idx] )== 0 )
            break;
		}
      if( idx < MAX_BUTTON_TYPE )
			pButtonInfo->type = idx;
	}

   return psv;

}
#ifdef USE_KEYPRESSGUARD
// specifying zero or less than zero in "Seconds between keypresses" is valid,
// just means keypressguard is ignored.  Also, if this line is never translated
// from the config file (aka doesn't exist) this value will be zero, ignoring
// the keypressguard.

PTRSZVAL CPROC RestoreKeypressGuardPeriod( PTRSZVAL psv, arg_list args )
{
	PARAM( args, S_64, period );
	if( period > 0 )
	{
		g.keypressguard.period = (_64) period;
		g.keypressguard.label_var = CreateLabelVariable( "KeypressGuard", LABEL_TYPE_STRING, &g.keypressguard.label_str  );
		g.keypressguard.label_str = StrDup( " " );
	}
	else
      g.keypressguard.period = 0;
   return psv;
}
#endif

PTRSZVAL CPROC LoadColor( PTRSZVAL psv, arg_list args )
{
   PARAM( args, CTEXTSTR, name );
	PARAM( args, CDATA, color );
	if( strcmp( name, "Master" ) == 0 )
      l.color.master = color;
	if( strcmp( name, "No Master" ) == 0 )
      l.color.no_master = color;
	if( strcmp( name, "Participant" ) == 0 )
      l.color.participant = color;
	if( strcmp( name, "Delegate" ) == 0 )
		l.color.delegate = color;
   return psv;
}

OnLoadControl( WIDE("Enable Participant") )( PCONFIG_HANDLER pch, PTRSZVAL unused )
{
   /* common config will have already been processed */
	OpenDatabase();
	AddConfigurationMethod( pch, "hall name=%w", RestoreHallID );
   AddConfigurationMethod( pch, "button mode=%m", RestoreButtonType );
}

static PTRSZVAL CPROC SetDSN( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, DSN );
	if( strlen( DSN ) )
	{
		if( l.pDSN )
         Release( l.pDSN );
		l.pDSN = StrDup( DSN );
	}
   return psv;
}


OnLoadCommon( WIDE( "Link State Buttons" ) )( PCONFIG_HANDLER pch )
{
	AddConfigurationMethod( pch, "Link State ODBC DSN=%m", SetDSN );
	AddConfigurationMethod( pch, "Link %m Color %c", LoadColor );

#ifdef USE_KEYPRESSGUARD
	AddConfigurationMethod( pch, "Seconds Between Keypresses %i", RestoreKeypressGuardPeriod );
#endif
	// load these before processing the configuration file...
}

OnSaveCommon( WIDE( "Link State Buttons" ) )( FILE *file )
{
	if( l.pDSN )
		fprintf( file, "Link State ODBC DSN=%s\n", l.pDSN );
   else
		fprintf( file, "Link State ODBC DSN=vsrvr\n" );
	fprintf( file, "Link Master Color $%08"_32fX"\n", l.color.master );
	fprintf( file, "Link No Master Color $%08"_32fX"\n", l.color.no_master );
	fprintf( file, "Link Participant Color $%08"_32fX"\n", l.color.participant );
	fprintf( file, "Link Delegate Color $%08"_32fX"\n", l.color.delegate );
#ifdef USE_KEYPRESSGUARD
	fprintf( file, "Seconds Between Keypresses %u\n", g.keypressguard.period );
#endif
}

