#define USES_MILK_INTERFACE

#include <milk_export.h>
#include <milk_button.h>
#include <milk_registry.h>
#include <sqlstub.h>
#include <filesys.h>
#include <widgets/banner.h>

#ifdef WIN32
#define MEDIA_ROOT_PATH "c:/ftn3000/etc/images/"
#else
#define MEDIA_ROOT_PATH "/storage/media/"
#endif

//TXT_MEDIA_NAME
enum{  LISTBOX_MEDIA_AVAILABLE = 4510
	 , LISTBOX_MEDIA_SELECTED
	 , BTN_ADD_AVAILABLE_TO_SELECTED_MEDIA
	 , BTN_SELECTED_UP
	 , BTN_SELECTED_DOWN
};

PRELOAD( InitControllerSelectMedia )
{
	EasyRegisterResource( "Video Link/Controller Select Media" , LISTBOX_MEDIA_AVAILABLE, LISTBOX_CONTROL_NAME );
	EasyRegisterResource( "Video Link/Controller Select Media" , LISTBOX_MEDIA_SELECTED, LISTBOX_CONTROL_NAME );
	EasyRegisterResource( "Video Link/Controller Select Media" , BTN_ADD_AVAILABLE_TO_SELECTED_MEDIA, NORMAL_BUTTON_NAME );
	EasyRegisterResource( "Video Link/Controller Select Media" , BTN_SELECTED_UP, NORMAL_BUTTON_NAME );
	EasyRegisterResource( "Video Link/Controller Select Media" , BTN_SELECTED_DOWN, NORMAL_BUTTON_NAME );

}



typedef struct media_info
{
	TEXTCHAR name[128];
	_32 order;
	_32 id;
} MEDIA_INFO, *PMEDIA_INFO;
typedef struct button_info_for_select_media
{
	struct {
		_32 bOn:1;
	} flags;
	PMENU_BUTTON button;
	PVARIABLE label_var;
	CTEXTSTR label_str;

} BUTTON_SELECT_MEDIA_INFO, *PBUTTON_SELECT_MEDIA_INFO;

static struct select_media_local_tag{
	//tried to avoid using a local structure of global scope, but
	//rather do it this way than constantly querying for the PSI_CONTROL
	//of the following controls:
	PSI_CONTROL listboxAvailable, listboxSelected;
	_32 countSelected; // well, necessary to be static to count the media elements in the selected listbox

}selectmedialocal;

void AddAvailableToSelected( void )
{
	PMEDIA_INFO mda = NULL;
	PMEDIA_INFO m = New( MEDIA_INFO);//yes, needs a new iteration.

	mda = ( PMEDIA_INFO )GetItemData( GetSelectedItem( selectmedialocal.listboxAvailable ) );
	if( mda )
	{
		m->order = selectmedialocal.countSelected++;
		m->id = mda->id;
		strcpy( m->name, mda->name );
		SetItemData( AddListItem( selectmedialocal.listboxSelected , m->name ) , (PTRSZVAL)m );
	}
}

void MoveSelectedSelection( int direction , PLISTITEM pli )
{
	MoveListItem( selectmedialocal.listboxSelected, pli , direction );
}

#define UP -1
#define DOWN 1
void CPROC ButtonMoveSelectedSelectionUp( PTRSZVAL psv, PSI_CONTROL button )
{
	if( selectmedialocal.countSelected )
	{
		PMEDIA_INFO mda = NULL;
		PLISTITEM pli;
		mda = ( PMEDIA_INFO )GetItemData( pli = GetSelectedItem( selectmedialocal.listboxSelected  ) );
		if( mda->order  ) //greater than zero?
		{
			PMEDIA_INFO m = (PMEDIA_INFO)GetItemData( GetNthItem( selectmedialocal.listboxSelected , ( mda->order - 1 ) ) );
			m->order++;
			mda->order--;
			MoveSelectedSelection( UP , pli );
		}
	}

}

void CPROC ButtonMoveSelectedSelectionDown( PTRSZVAL psv, PSI_CONTROL button )
{
	if( selectmedialocal.countSelected )
	{
		PMEDIA_INFO mda = NULL;
		PLISTITEM pli;
		mda = ( PMEDIA_INFO )GetItemData( pli = GetSelectedItem( selectmedialocal.listboxSelected  ) );
		if( mda->order < selectmedialocal.countSelected )
		{
			PMEDIA_INFO m = (PMEDIA_INFO)GetItemData(
																  GetNthItem( selectmedialocal.listboxSelected
																				, ( mda->order + 1 ) )
																 );
			if( !m )
			{
				lprintf("Bogus! Could not find PMEDIA_INFO from GetItemData for ( mda->order + 1 ) of %lu.  Returning."
						 , ( mda->order + 1 )
						 );
				return;
			}
			m->order--;
			mda->order++;
			MoveSelectedSelection( DOWN , pli );
		}
	}
}



void CPROC ButtonAddAvailableToSelected( PTRSZVAL psv, PSI_CONTROL button )
{
	AddAvailableToSelected();
}

void CPROC DoubleClickedAvailable( PTRSZVAL psv, PSI_CONTROL listboxAvailable, PLISTITEM hli )
{
	AddAvailableToSelected();
}

void CPROC DoubleClickedSelected( PTRSZVAL psv, PSI_CONTROL listboxSelected, PLISTITEM hli )
{
	_32 x = 0;
	if( selectmedialocal.countSelected )
	{
		PMEDIA_INFO m =( PMEDIA_INFO )GetItemData( hli );
		_32 oldorder = x = m->order;
		DoSQLCommandf( WIDE( "DELETE FROM media_playlist WHERE media_playlist_order=%lu")
						 , x
						 );
		//was this the last in the list? if not, then...
		if( x )
		{
			// get the next one in the list, and every one thereafter.
			for( x++ ; x < selectmedialocal.countSelected ; x++ )
			{
				PMEDIA_INFO mda = (PMEDIA_INFO)GetItemData( GetNthItem( selectmedialocal.listboxSelected ,  x ) );
				oldorder = mda->order--;
				//lprintf("oldorder is now %lu mda->order is now %lu", oldorder, mda->order);
				if( !( DoSQLCommandf( WIDE( "UPDATE media_playlist SET media_playlist_order=%lu WHERE media_playlist_order=%lu" )
										  , mda->order
										  , oldorder
										  )
					  )
				  )
				{
					CTEXTSTR errorresult;
					char * buf = Allocate(63);
					char * p;
					GetSQLError( &errorresult );
					p = strchr( errorresult, '\'' );
					MemCpy( buf, p, 62 );
					lprintf("Looks like a SQL error of: %s", buf );
					BannerMessage( buf );
				}
			}
		}
		DeleteListItem( selectmedialocal.listboxSelected , hli );
		selectmedialocal.countSelected--;
	}
}


OnKeyPressEvent(WIDE( "Video Link/Controller Select Media" ) )( PTRSZVAL psv )
{
	PBUTTON_SELECT_MEDIA_INFO pButtonInfo = ( PBUTTON_SELECT_MEDIA_INFO  )psv;
	PSI_CONTROL parent = GetFrame( pButtonInfo->button );
	PSI_CONTROL frame = NULL;
	int okay = 0, done = 0;

	if( frame = LoadXMLFrame( "Select_Media_For_Link_Controller.Frame" ) )
	{
		SetCommonButtons( frame, &done, &okay );
		SetCommonButtonControls( frame );

		selectmedialocal.listboxAvailable = GetControl( frame, LISTBOX_MEDIA_AVAILABLE );
		if( selectmedialocal.listboxAvailable )
		{
			INDEX idx;
			CTEXTSTR *results;
			PMEDIA_INFO mda;
			PLIST listAvailable = NULL;//, listSelected = NULL;
			PSI_CONTROL button = GetControl( frame, BTN_ADD_AVAILABLE_TO_SELECTED_MEDIA );

			if( button )
				SetButtonPushMethod( button , ButtonAddAvailableToSelected, psv );
			SetDoubleClickHandler( selectmedialocal.listboxAvailable, DoubleClickedAvailable, psv );

			for( DoSQLRecordQuery( WIDE( "SELECT media_id, media_name FROM media WHERE deleted=0"), NULL, &results, NULL )
				 ; results
				  ; GetSQLRecord( &results )
				)
			{
				PMEDIA_INFO media = New( MEDIA_INFO );
				//				snprintf( media->name, (sizeof(media->name)),"%s", results[1] );
				strcpy( media->name, results[1] );
				media->order = 0;
				media->id = atoi( results[0] );
				AddLink( &listAvailable, media );
			}

			LIST_FORALL( listAvailable, idx, PMEDIA_INFO , mda )
			{
				SetItemData( AddListItem( selectmedialocal.listboxAvailable , mda->name ) , (PTRSZVAL)mda );
			}
		}

		selectmedialocal.listboxSelected = GetControl( frame, LISTBOX_MEDIA_SELECTED );
		if( selectmedialocal.listboxSelected )
		{
			CTEXTSTR *results;
			PSI_CONTROL button1 = GetControl( frame, BTN_SELECTED_UP );
			PSI_CONTROL button2 = GetControl( frame, BTN_SELECTED_DOWN );
			if( button1 )
				SetButtonPushMethod( button1 , ButtonMoveSelectedSelectionUp, psv );
			if( button2 )
				SetButtonPushMethod( button2 , ButtonMoveSelectedSelectionDown, psv );
			SetDoubleClickHandler( selectmedialocal.listboxSelected, DoubleClickedSelected, psv );

			for( DoSQLRecordQuery ( WIDE("SELECT media_playlist.media_playlist_order,media_playlist.media_id,media.media_name FROM media, media_playlist where media.media_id=media_playlist.media_id ORDER BY media_playlist.media_playlist_order ASC"), NULL, &results, NULL )
				 ; results
				  ; GetSQLRecord( &results )
				)
			{
				PMEDIA_INFO m = New( MEDIA_INFO);//yes, needs a new iteration.
				m->order = atoi( results[0] );
				m->id = atoi( results[1] );
				strcpy( m->name, results[2] );
				SetItemData( AddListItem( selectmedialocal.listboxSelected , m->name ) , (PTRSZVAL)m );
				selectmedialocal.countSelected++;
			}
		}

		DisplayFrameOver( frame, parent );
		//		EditFrame( frame, TRUE );
		CommonWait( frame );
	}

	if( okay )
	{
		INDEX idx = 0;
		PMEDIA_INFO media;
		PLISTITEM pli;

		GetCommonButtonControls( frame );
		while( pli = GetNthItem( selectmedialocal.listboxSelected , idx) )
		{
			media = ( PMEDIA_INFO )GetItemData( pli );
			lprintf("[%d] %s  %lu  order:%lu", idx, media->name, media->id, media->order);
			if( !( DoSQLCommandf( WIDE( "REPLACE INTO media_playlist SET media_id=%lu,media_playlist_order=%lu" )
									  , media->id
									  , media->order
									  )
				  )
			  )
			{
				CTEXTSTR errorresult;
				char * buf = Allocate(63);
				char * p;
				GetSQLError( &errorresult );
				p = strchr( errorresult, '\'' );
				MemCpy( buf, p, 62 );
				lprintf("Looks like a SQL error of: %s", buf );
				BannerMessage( buf );
			}
			idx++;
		}
	}
	if( frame )
		DestroyFrame( &frame );



}
OnCreateMenuButton( WIDE( "Video Link/Controller Select Media" ) )( PMENU_BUTTON button )
{
	PBUTTON_SELECT_MEDIA_INFO pButtonInfo = Allocate( sizeof( BUTTON_SELECT_MEDIA_INFO  ) );
	//	pButtonInfo->button = button;
	MILK_SetButtonText( pButtonInfo->button = button, "Select_Media");

	return (PTRSZVAL)pButtonInfo;
}

