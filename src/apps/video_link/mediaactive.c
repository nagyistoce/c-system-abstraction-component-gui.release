#define USES_MILK_INTERFACE
#include <milk_export.h>
#include <milk_button.h>
#include <milk_registry.h>
#include <sqlstub.h>
#include <filesys.h>
#include <widgets/banner.h>

#include "statebuttons.h"

#ifdef __WINDOWS__
#define MEDIA_ROOT_PATH "c:/ftn3000/etc/images/"
#else
#define MEDIA_ROOT_PATH "/storage/media/"
#endif

enum{ LISTBOX_HALL_NUMBER =4541  // 4510 starts importmedia.c //4581 starts dvdactive
	 , LISTBOX_MEDIA_NAMES
	 , CHECKBOX_ALL_HALLS_ON
	 , CHECKBOX_ALL_HALLS_OFF
};

typedef struct hall_description_info{
	char name[256];
	_32 id;
}HALL_MEDIA_ACTIVE_INFO, *PHALL_MEDIA_ACTIVE_INFO;

typedef struct button_info
{
	struct {
		_32 bMediaActive : 1;
		_32 _bMediaActive : 1;
		_32 bOn:1;
	} flags;
	PHALL_MEDIA_ACTIVE_INFO pHall;
	PMENU_BUTTON button;
	PVARIABLE label_var;
	CTEXTSTR label_str;
} BUTTON_MEDIA_ACTIVE_INFO, *PBUTTON_MEDIA_ACTIVE_INFO;


static struct mediaactive_local_tag{
	PLIST buttons; //of type PBUTTON_MEDIA_ACTIVE_INFO.  Could have just made one button, and made it global.
	//PLIST halls;
}mediaactivelocal;

PRELOAD( InitMediaActiveSource )
{

	EasyRegisterResource( "Controller Media Active" , LISTBOX_HALL_NUMBER, LISTBOX_CONTROL_NAME );
	EasyRegisterResource( "Controller Media Active" , CHECKBOX_ALL_HALLS_ON, RADIO_BUTTON_NAME );
	EasyRegisterResource( "Controller Media Active" , CHECKBOX_ALL_HALLS_OFF, RADIO_BUTTON_NAME );
}

void MediaActiveStateChangedEach(_32 which, _32 how )
{
	INDEX idx;
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo;
	lprintf("MediaActiveStateChangedEach for %u becoming %u " , which , how );
	LIST_FORALL( mediaactivelocal.buttons, idx, PBUTTON_MEDIA_ACTIVE_INFO  , pButtonInfo )
	{
		if( pButtonInfo->pHall->id == which )
		{
			pButtonInfo->flags._bMediaActive = pButtonInfo->flags.bMediaActive;
			pButtonInfo->flags.bMediaActive = how;
			UpdateButton(  pButtonInfo->button );
			if( pButtonInfo->flags.bMediaActive )
				pButtonInfo->label_str = StrDup( "Playing" );
			else
				pButtonInfo->label_str = StrDup( "Ready" );

			LabelVariableChanged( pButtonInfo->label_var );
			break;
		}
	}
}

void MediaActiveStateChanged(void)
{
	INDEX idx;
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo;
	lprintf("MediaActiveStateChanged");
	LIST_FORALL( mediaactivelocal.buttons, idx, PBUTTON_MEDIA_ACTIVE_INFO  , pButtonInfo )
	{
		lprintf("Updating for hall %lu", pButtonInfo->pHall->id );
		UpdateButton(  pButtonInfo->button );
	}
}

OnShowControl( WIDE( "Controller Media Active" ) )( PTRSZVAL psv )
{
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo = (PBUTTON_MEDIA_ACTIVE_INFO)psv;
	MILK_SetButtonColors( (PMENU_BUTTON)pButtonInfo->button, BASE_COLOR_WHITE, BASE_COLOR_ORANGE
							  , pButtonInfo->flags.bMediaActive?BASE_COLOR_GREEN:BASE_COLOR_BLACK, COLOR_IGNORE );
}


OnKeyPressEvent(WIDE( "Controller Media Active" ) )( PTRSZVAL psv )
{
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo = (PBUTTON_MEDIA_ACTIVE_INFO)psv;

	//   lprintf("Got a keypress event for %lu" , pButtonInfo->pHall->id );
	if( !( AllowLinkStateKeypress()))
		return;

	if( pButtonInfo->pHall->id )
	{
		DoSQLCommandf( "update link_hall_state set dvd_active=0,media_active=%u where hall_id=%u"
						 , ( pButtonInfo->flags.bMediaActive = 1 - pButtonInfo->flags.bMediaActive )// ...and...InVeRt!
						 , pButtonInfo->pHall->id
						 );

		/*		lprintf("Commanded: pButtonInfo->flags.bMediaActive %lu pButtonInfo->pHall->id %lu"
		 , pButtonInfo->flags.bMediaActive
		 , pButtonInfo->pHall->id
		 );
		 */
		pButtonInfo->label_str = StrDup( "Waiting..." );
		LabelVariableChanged( pButtonInfo->label_var );
	}
	else
	{
		//		lprintf("oh, this is a global button. > %lu < ") , pButtonInfo->flags.bOn;
		pButtonInfo->flags.bMediaActive = 1; //...and...ONE!
		DoSQLCommandf( "update link_hall_state set dvd_active=0,media_active=%u"
						 , pButtonInfo->flags.bOn
						 );
		pButtonInfo->flags.bMediaActive = 0;// ...and...ZERO!
	}


	UpdateButton(  pButtonInfo->button );
}

OnCreateMenuButton( WIDE( "Controller Media Active" ) )( PMENU_BUTTON button )
{
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo = Allocate( sizeof( BUTTON_MEDIA_ACTIVE_INFO ) );
	pButtonInfo->button = button;
	pButtonInfo->flags.bMediaActive = 0;
	pButtonInfo->pHall = New( HALL_MEDIA_ACTIVE_INFO );
	pButtonInfo->pHall->id = 0;
	AddLink( &mediaactivelocal.buttons, pButtonInfo );
	return (PTRSZVAL)pButtonInfo;
}

OnDestroyMenuButton( WIDE( "Controller Media Active" ) )(PTRSZVAL psv )
{
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo  = (PBUTTON_MEDIA_ACTIVE_INFO)psv;
	DeleteLink( &mediaactivelocal.buttons, pButtonInfo );
}


static PTRSZVAL CPROC RestoreHallID( PTRSZVAL psv, arg_list args )
{
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo = (PBUTTON_MEDIA_ACTIVE_INFO)psv;
	CTEXTSTR *results = NULL;
	int count = 0;
	int x ;
	PARAM( args, S_64, hall_id );

	if( !pButtonInfo->pHall->id )
	{
		if( hall_id > 0 )
		{
			char buf[512];
			x = hall_id;
			lprintf("Searching for %ld" , x );
			snprintf( buf, (sizeof(buf))
					  , "select location.packed_name, link_hall_state.media_active from location, link_hall_state where link_hall_state.hall_id = %ld AND location.id = %ld "
					  , x
					  , x
					  );
			lprintf("buf is now %s", buf);
			if( DoSQLRecordQuery( buf, &count, &results , NULL) )
				/*
				 if( DoSQLRecordQueryf( NULL, &results, NULL
				 //										, WIDE("select location.packed_name, link_hall_state.dvd_active from location, link_hall_state where link_hall_state.hall_id=%d AND location.id=%d")
				 , "select location.packed_name, link_hall_state.dvd_active from location, link_hall_state where link_hall_state.hall_id=%ld AND location.id=%ld"
				 , hall_id
				 , hall_id
				 ) )
				 */			{
					 if( results )
					 {
						 char buffer[64];
						 lprintf("Found %s whose media_active is %s", results[0], results[1] );
						 pButtonInfo->pHall->id = (_32)hall_id;
						 pButtonInfo->flags.bMediaActive=( ( atoi(results[1]) >=1 )?1:0) ;
						 MILK_SetButtonText( pButtonInfo->button, results[0]);
						 snprintf( buffer, (sizeof(buffer)), "<Media:%s>", results[0] );
						 pButtonInfo->label_var = CreateLabelVariable( buffer, LABEL_TYPE_STRING, &pButtonInfo->label_str  );
						 pButtonInfo->label_str = StrDup( " . . . . . . " );
						 UpdateButton( pButtonInfo->button );
					 }
					 else
						 lprintf("No REsults?!");
				 }
			else
			{
				CTEXTSTR *results = NULL;
				lprintf("Some strangeness from ODBC");
				GetSQLError(results);
				lprintf("GetSQLError was %s", results );
			}
		}
		else if( hall_id < 0 )
		{
			lprintf("oh this is a global ON button.  this one will set all halls in the database, not just one.");
			pButtonInfo->flags.bMediaActive= 0;
			pButtonInfo->pHall->id = 0;
			pButtonInfo->flags.bOn=1;
			MILK_SetButtonText( pButtonInfo->button, "All_Halls_ON");
			UpdateButton( pButtonInfo->button );

		}
		else //oh, it's zero.
		{
			lprintf("oh this is a global OFF button.  this one will set all halls in the database, not just one.");
			pButtonInfo->flags.bMediaActive= 0;
			pButtonInfo->pHall->id = 0;
			pButtonInfo->flags.bOn=0;
			MILK_SetButtonText( pButtonInfo->button, "All_Halls_OFF");
			UpdateButton( pButtonInfo->button );

		}

		AddLink( &mediaactivelocal.buttons, pButtonInfo );// Will there be more than one button at a time?  Probably not, but the alternative here is to make the button global.  This seems more extensible.
	}
	else
		lprintf("Uh, RestoreHallID called with %d but pButtonInfo->hall_id was already %lu.  Now what?", hall_id, pButtonInfo->pHall->id );

	return psv;
}
/*
static void CPROC HallSelected( PTRSZVAL psv, PSI_CONTROL list, PLISTITEM pli )
{
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo = (PBUTTON_MEDIA_ACTIVE_INFO)psv;
	PHALL_INFO pHall = ( PHALL_INFO )GetItemData( pli );

	pButtonInfo->pHall->id = pHall->hall_id;
	strcpy( pButtonInfo->pHall->name , pHall->name );
	lprintf("[HallSelected] pButtonInfo->name is now %s and pButtonInfo->hall_id is now %u"
			 , pButtonInfo->pHall->name
			 , pButtonInfo->pHall->id
			 );

}
*/

OnLoadControl( WIDE( "Controller Media Active" ) )( PCONFIG_HANDLER pch, PTRSZVAL unused )
{
	AddConfigurationMethod( pch, "Hall ID %i", RestoreHallID );
	// load these before processing the configuration file...
}

OnSaveControl( WIDE("Controller Media Active") )( FILE *file, PTRSZVAL psv )
{
	PBUTTON_MEDIA_ACTIVE_INFO pButtonInfo = (PBUTTON_MEDIA_ACTIVE_INFO)psv;
	if( pButtonInfo )
	{
		if( pButtonInfo->pHall->id)
			fprintf( file, WIDE("Hall ID %d\n"), pButtonInfo->pHall->id );
		else if( pButtonInfo->flags.bOn )
			fprintf( file, WIDE("Hall ID -1\n")); //global ON
		else
			fprintf( file, WIDE("Hall ID 0\n"));//global OFF

	}
}

