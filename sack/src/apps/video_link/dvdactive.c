#define USES_MILK_INTERFACE

#include <milk_export.h>
#include <milk_button.h>
#include <milk_registry.h>
#include <sqlstub.h>

#include "statebuttons.h"

enum{ LISTBOX_HALL_NUMBER =4581  // 4510 starts importmedia.c //4541 starts mediaactive.c
	 , LISTBOX_MEDIA_NAMES
	 , CHECKBOX_ALL_HALLS_ON
	 , CHECKBOX_ALL_HALLS_OFF
};

PSI_CONTROL cka,ckb;//weak, i know.  i'm sorry.

typedef struct hall_description_info{
	char name[256];
	_32 id;
}HALL_DVD_ACTIVE_INFO, *PHALL_DVD_ACTIVE_INFO;



typedef struct button_info
{
	struct {
		_32 bDVDActive : 1;
		_32 _bDVDActive : 1;
		_32 bOn:1;
	} flags;
	PHALL_DVD_ACTIVE_INFO pHall;
	PMENU_BUTTON button;
	PVARIABLE label_var;
	CTEXTSTR label_str;
} BUTTON_DVD_ACTIVE_INFO, *PBUTTON_DVD_ACTIVE_INFO;


static struct dvdactive_local_tag{
	PLIST buttons; //of type PBUTTON_DVD_ACTIVE_INFO.  Could have just made one button, and made it global.
	PLIST halls;
}dvdactivelocal;

PRELOAD( InitDVDActiveSource )
{

	EasyRegisterResource( "Controller DVD Active" , LISTBOX_HALL_NUMBER, LISTBOX_CONTROL_NAME );
	EasyRegisterResource( "Controller DVD Active" , CHECKBOX_ALL_HALLS_ON, RADIO_BUTTON_NAME );
	EasyRegisterResource( "Controller DVD Active" , CHECKBOX_ALL_HALLS_OFF, RADIO_BUTTON_NAME );
}

void DVDActiveStateChangedEach(_32 which, _32 how )
{
	INDEX idx;
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo;
	lprintf("DVDActiveStateChangedEach for %u becoming %u " , which , how );
	LIST_FORALL( dvdactivelocal.buttons, idx, PBUTTON_DVD_ACTIVE_INFO  , pDVDActiveButtonInfo )
	{
		if( pDVDActiveButtonInfo->pHall->id == which )
		{
			pDVDActiveButtonInfo->flags._bDVDActive = pDVDActiveButtonInfo->flags.bDVDActive;
			pDVDActiveButtonInfo->flags.bDVDActive = how;
			UpdateButton(  pDVDActiveButtonInfo->button );
			if( pDVDActiveButtonInfo->flags.bDVDActive )
				pDVDActiveButtonInfo->label_str = StrDup( "Playing" );
			else
				pDVDActiveButtonInfo->label_str = StrDup( "Ready" );

			LabelVariableChanged( pDVDActiveButtonInfo->label_var );
			break;
		}
	}
}

void DVDActiveStateChanged(void)
{
	INDEX idx;
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo;
	lprintf("DVDActiveStateChanged");
	LIST_FORALL( dvdactivelocal.buttons, idx, PBUTTON_DVD_ACTIVE_INFO  , pDVDActiveButtonInfo )
	{
		lprintf("Updating for hall %lu", pDVDActiveButtonInfo->pHall->id );
		UpdateButton(  pDVDActiveButtonInfo->button );
	}
}

OnShowControl( WIDE( "Controller DVD Active" ) )( PTRSZVAL psv )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = (PBUTTON_DVD_ACTIVE_INFO)psv;
	MILK_SetButtonColors( (PMENU_BUTTON)pDVDActiveButtonInfo->button, BASE_COLOR_WHITE, BASE_COLOR_ORANGE
							  , pDVDActiveButtonInfo->flags.bDVDActive?BASE_COLOR_GREEN:BASE_COLOR_BLACK, COLOR_IGNORE );
}

void CPROC ResetOfTheAllHallButtonColors(PTRSZVAL psv )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = (PBUTTON_DVD_ACTIVE_INFO)psv;
	lprintf("ResetOfTheAllHallButtonColors!");
	pDVDActiveButtonInfo->flags.bDVDActive = 0;// ...and...ZERO!
	UpdateButton(  pDVDActiveButtonInfo->button );

}


OnKeyPressEvent(WIDE( "Controller DVD Active" ) )( PTRSZVAL psv )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = (PBUTTON_DVD_ACTIVE_INFO)psv;

	//   lprintf("Got a keypress event for %lu" , pDVDActiveButtonInfo->pHall->id );
	if( !( AllowLinkStateKeypress()))
		return;

	if( pDVDActiveButtonInfo->pHall->id )
        {
            lprintf( "g.odbc is never set.  This will use default connection (may be thread conflict)."  );
		SQLCommandf( g.odbc, "update link_hall_state set media_active=0,dvd_active=%u where hall_id=%u"
						 , ( pDVDActiveButtonInfo->flags.bDVDActive = 1 - pDVDActiveButtonInfo->flags.bDVDActive )// ...and...InVeRt!
						 , pDVDActiveButtonInfo->pHall->id
						 );

		/*		lprintf("Commanded: pDVDActiveButtonInfo->flags.bDVDActive %lu pDVDActiveButtonInfo->pHall->id %lu"
		 , pDVDActiveButtonInfo->flags.bDVDActive
		 , pDVDActiveButtonInfo->pHall->id
		 );
		 */
		pDVDActiveButtonInfo->label_str = StrDup( "Waiting..." );
		LabelVariableChanged( pDVDActiveButtonInfo->label_var );
	}
	else
	{
		//		lprintf("oh, this is a global button. > %lu < ") , pDVDActiveButtonInfo->flags.bOn;
		pDVDActiveButtonInfo->flags.bDVDActive = 1; //...and...ONE!
            lprintf( "g.odbc is never set.  This will use default connection (may be thread conflict)."  );
		SQLCommandf( g.odbc, "update link_hall_state set media_active=0,dvd_active=%u"
						 , pDVDActiveButtonInfo->flags.bOn
						 );
		AddTimerEx(5000, 0, ResetOfTheAllHallButtonColors, (PTRSZVAL)pDVDActiveButtonInfo );
	}


	UpdateButton(  pDVDActiveButtonInfo->button );
}

OnCreateMenuButton( WIDE( "Controller DVD Active" ) )( PMENU_BUTTON button )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = Allocate( sizeof( BUTTON_DVD_ACTIVE_INFO ) );
	pDVDActiveButtonInfo->button = button;
	pDVDActiveButtonInfo->flags.bDVDActive = 0;
	pDVDActiveButtonInfo->pHall = New( HALL_DVD_ACTIVE_INFO );
	pDVDActiveButtonInfo->pHall->id = 0;
	AddLink( &dvdactivelocal.buttons, pDVDActiveButtonInfo );
	return (PTRSZVAL)pDVDActiveButtonInfo;
}

OnDestroyMenuButton( WIDE( "Controller DVD Active" ) )(PTRSZVAL psv )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo  = (PBUTTON_DVD_ACTIVE_INFO)psv;
	DeleteLink( &dvdactivelocal.buttons, pDVDActiveButtonInfo );
}
static PTRSZVAL CPROC RestoreHallID( PTRSZVAL psv, arg_list args )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = (PBUTTON_DVD_ACTIVE_INFO)psv;
	CTEXTSTR *results = NULL;
	int count = 0;
	int x ;
	PARAM( args, S_64, hall_id );

	if( !pDVDActiveButtonInfo->pHall->id )
	{
		if( hall_id > 0 )
		{
			char buf[512];
			x = hall_id;
			lprintf("Searching for %ld" , x );
			snprintf( buf, (sizeof(buf))
					  , "select location.packed_name, link_hall_state.dvd_active from location, link_hall_state where link_hall_state.hall_id = %ld AND location.id = %ld "
					  , x
					  , x
					  );
			lprintf("buf is now %s", buf);
            lprintf( "g.odbc is never set.  This will use default connection (may be thread conflict)."  );
			if( SQLRecordQuery( g.odbc, buf, &count, &results , NULL) )
				/*
				 if( SQLRecordQueryf( g.odbc, NULL, &results, NULL
				 //										, WIDE("select location.packed_name, link_hall_state.dvd_active from location, link_hall_state where link_hall_state.hall_id=%d AND location.id=%d")
				 , "select location.packed_name, link_hall_state.dvd_active from location, link_hall_state where link_hall_state.hall_id=%ld AND location.id=%ld"
				 , hall_id
				 , hall_id
				 ) )
				 */			{
					 if( results )
					 {
						 char buffer[64];
						 lprintf("Found %s whose dvd_active is %s", results[0], results[1] );
						 pDVDActiveButtonInfo->pHall->id = (_32)hall_id;
						 pDVDActiveButtonInfo->flags.bDVDActive=( ( atoi(results[1]) >=1 )?1:0) ;
						 MILK_SetButtonText( pDVDActiveButtonInfo->button, results[0]);
						 snprintf( buffer, (sizeof(buffer)), "<DVD:%s>", results[0] );
						 pDVDActiveButtonInfo->label_var = CreateLabelVariable( buffer, LABEL_TYPE_STRING, &pDVDActiveButtonInfo->label_str  );
						 pDVDActiveButtonInfo->label_str = StrDup( " . . . . . . " );
						 UpdateButton( pDVDActiveButtonInfo->button );
					 }
					 else
						 lprintf("No REsults?!");
				 }
			else
			{
				CTEXTSTR *results = NULL;
				lprintf("Some strangeness from ODBC");
            lprintf( "g.odbc is never set.  This will use default connection (may be thread conflict)."  );
				FetchSQLError(g.odbc, results);
				lprintf("GetSQLError was %s", results );
			}
		}
		else if( hall_id < 0 )
		{
			lprintf("oh this is a global ON button.  this one will set all halls in the database, not just one.");
			pDVDActiveButtonInfo->flags.bDVDActive= 0;
			pDVDActiveButtonInfo->pHall->id = 0;
			pDVDActiveButtonInfo->flags.bOn=1;
			MILK_SetButtonText( pDVDActiveButtonInfo->button, "All_Halls_ON");
			UpdateButton( pDVDActiveButtonInfo->button );

		}
		else //oh, it's zero.
		{
			lprintf("oh this is a global OFF button.  this one will set all halls in the database, not just one.");
			pDVDActiveButtonInfo->flags.bDVDActive= 0;
			pDVDActiveButtonInfo->pHall->id = 0;
			pDVDActiveButtonInfo->flags.bOn=0;
			MILK_SetButtonText( pDVDActiveButtonInfo->button, "All_Halls_OFF");
			UpdateButton( pDVDActiveButtonInfo->button );

		}

		AddLink( &dvdactivelocal.buttons, pDVDActiveButtonInfo );// Will there be more than one button at a time?  Probably not, but the alternative here is to make the button global.  This seems more extensible.
	}
	else
		lprintf("Uh, RestoreHallID called with %d but pDVDActiveButtonInfo->hall_id was already %lu.  Now what?", hall_id, pDVDActiveButtonInfo->pHall->id );

	return psv;
}
static void CPROC HallSelected( PTRSZVAL psv, PSI_CONTROL list, PLISTITEM pli )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = (PBUTTON_DVD_ACTIVE_INFO)psv;
	PHALL_INFO pHall = ( PHALL_INFO )GetItemData( pli );

	pDVDActiveButtonInfo->pHall->id = pHall->hall_id;
	strcpy( pDVDActiveButtonInfo->pHall->name , pHall->name );
	lprintf("[HallSelected] pDVDActiveButtonInfo->name is now %s and pDVDActiveButtonInfo->hall_id is now %u"
			 , pDVDActiveButtonInfo->pHall->name
			 , pDVDActiveButtonInfo->pHall->id
			 );
	SetCheckState( cka , 0 );
	SetCheckState( ckb , 0 );

}
OnEditControl( "Controller DVD Active" )( PTRSZVAL psv, PSI_CONTROL parent )
{
	int okay = 0, done = 0;
	PSI_CONTROL frame;
	PSI_CONTROL listHalls = NULL;
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = (PBUTTON_DVD_ACTIVE_INFO)psv;

	frame = LoadXMLFrame( "Edit_DVD_Active_For_Link_Controller.Frame" );
	if( !frame )
	{
		// popuplate default, editing was canceled
	}
	if( frame )
	{
		SetCommonButtons( frame, &done, &okay );
		SetCommonButtonControls( frame );

		//if there is no pHall->id at this point, this is probably an "all halls" button
		if( !pDVDActiveButtonInfo->pHall->id )
		{
			if( pDVDActiveButtonInfo->flags.bOn )
			{
				SetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_ON ), 1 );
				SetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_OFF ), 0 );
			}
			else
			{
				SetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_ON ), 0 );
				SetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_OFF ), 1 );
			}
		}
		else
		{
			SetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_ON ), 0 );
			SetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_OFF ), 0 );
		}

		listHalls = GetControl( frame, LISTBOX_HALL_NUMBER );
		if( listHalls )
		{
			PLISTITEM pli;
			INDEX idx;
			PHALL_INFO pHall;

			LIST_FORALL( g.halls, idx, PHALL_INFO, pHall )
			{
				SetItemData( pli = AddListItem( listHalls , pHall->name ) , (PTRSZVAL)pHall );
				if( pDVDActiveButtonInfo->pHall->id && ( pDVDActiveButtonInfo->pHall->id == pHall->hall_id ) )
					SetSelectedItem( listHalls, pli );
			}
			SetSelChangeHandler( listHalls, HallSelected, (PTRSZVAL)pDVDActiveButtonInfo );
		}
		DisplayFrameOver( frame, parent );
		EditFrame( frame, TRUE );
		CommonWait( frame );
	}

	if( okay )
	{
		GetCommonButtonControls( frame );

		if( GetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_ON ) ) )
		{
			lprintf("oh, this button is an all halls ON button?  Ok.");
			pDVDActiveButtonInfo->pHall->id = 0;
			pDVDActiveButtonInfo->flags.bOn = 1;
			strcpy( pDVDActiveButtonInfo->pHall->name , "All_Halls_ON");
		}
		else if( GetCheckState( GetControl( frame, CHECKBOX_ALL_HALLS_OFF ) ) )
		{
			lprintf("oh, this button is an all halls OFF button?  Ok.");
			pDVDActiveButtonInfo->pHall->id = 0;
			pDVDActiveButtonInfo->flags.bOn = 0;
			strcpy( pDVDActiveButtonInfo->pHall->name , "All_Halls_OFF");
		}
		else
		{
			// get currently selected hall listbox item, save in buton_info structure
			if( listHalls )
			{
				PHALL_INFO pHall = ( PHALL_INFO )GetItemData( GetSelectedItem( listHalls ) );
				if( pHall && ( pDVDActiveButtonInfo->pHall->id != pHall->hall_id ) )
				{
					pDVDActiveButtonInfo->pHall->id = pHall->hall_id;
					strcpy( pDVDActiveButtonInfo->pHall->name , pHall->name );
				}
			}
		}
		lprintf("pDVDActiveButtonInfo->hall_id is now %u for %s", pDVDActiveButtonInfo->pHall->id , pDVDActiveButtonInfo->pHall->name );
		MILK_SetButtonText( pDVDActiveButtonInfo->button, pDVDActiveButtonInfo->pHall->name );

	}
	if( frame )
		DestroyFrame( &frame );
	return psv;
}

OnLoadControl( WIDE( "Controller DVD Active" ) )( PCONFIG_HANDLER pch, PTRSZVAL unused )
{
	AddConfigurationMethod( pch, "Hall ID %i", RestoreHallID );
	// load these before processing the configuration file...
}

OnSaveControl( WIDE("Controller DVD Active") )( FILE *file, PTRSZVAL psv )
{
	PBUTTON_DVD_ACTIVE_INFO pDVDActiveButtonInfo = (PBUTTON_DVD_ACTIVE_INFO)psv;
	if( pDVDActiveButtonInfo )
	{
		if( pDVDActiveButtonInfo->pHall->id)
			fprintf( file, WIDE("Hall ID %d\n"), pDVDActiveButtonInfo->pHall->id );
		else if( pDVDActiveButtonInfo->flags.bOn )
			fprintf( file, WIDE("Hall ID -1\n")); //global ON
		else
			fprintf( file, WIDE("Hall ID 0\n"));//global OFF

	}
}

