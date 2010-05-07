
#include <sack_types.h>
#include <deadstart.h>
#include <pssql.h>
#include "intershell_local.h"
#include "intershell_registry.h"
#include "menu_real_button.h"
#include "resource.h"
#include "fonts.h"


/*
 * Menu Buttons extended security attributes.
 *
 *  When a button is pressed the flag bSecurity is check, and if set,
 *  then a security context by permission_context table index.
 *  The program_id will be set for
 *
 */

PRELOAD( UpdateSecurityTables )
{
	//CheckODBCTable( LoginHistory );
	{
      // logout any prior instances...
	}
}

#if 0
void InitLoginFrame( PSI_CONTROL frame, int *done, int *okay )
{
	PSI_CONTROL list = GetNearControl( frame, LST_SELECT_USER );
	PSI_CONTROL password = GetNearControl( frame, EDT_PASSWORD );
	if( list )
	{
		CTEXTSTR *result;
      //SetSelChangeHandler( list, SelectCurrentUser, 0 );
		for( DoSQLRecordQueryf( NULL, &result, NULL
									 , WIDE("select concat(last_name,', ',first_name,'(',staff_id,')') as name,pui.user_id ")
									  WIDE(" from permission_user_info as pui")
									  WIDE(" join permission_user as pu on pu.user_id = pui.user_id")
									  WIDE(" join permission_set as ps on ps.permission_group_id = pu.permission_group_id")
									  WIDE(" join permission_tokens as pt on pt.permission_id = ps.permission_id")
									  WIDE(" where terminate=0")
									  WIDE(" and terminate=0 and pt.name='login/logout'")
                             WIDE(" group by pui.user_id")
									  WIDE(" order by name") );
			  result;
			  GetSQLRecord( &result ) )
		{
         SetItemData( AddListItem( list, result[0] ), atoi( result[1] ) );
		}
	}
   if( password )
		SetEditControlPassword( password, TRUE );
   SetCommonButtons( frame, done, okay );
}
#endif

INDEX ReadLoginFrame( PSI_CONTROL frame, PMENU_BUTTON button )
{
	//PSI_CONTROL list = GetNearControl( frame, LST_SELECT_USER );
	PSI_CONTROL password = GetNearControl( frame, EDT_PASSWORD );
	TEXTCHAR p[256];
	INDEX iLogin;
	GetControlText( password, p, sizeof( p ) );
#if 0
	dialog.user_id = (_32)GetItemData( GetSelectedItem( GetNearControl( frame, LST_SELECT_USER ) ) );
	iLogin = UserLogin( dialog.user_id, p, button->security_context );
	if( iLogin == INVALID_INDEX )
	{
      BannerMessage( WIDE("Invalid username or password.") );
	}
#endif
   return iLogin;
}

//INDEX


/* result with either INVALID_INDEX - permission denied
 or 0 - no token created
 or some other value that is passed to CloseSecurityContext when the button press is complete
 */
PTRSZVAL CreateSecurityContext( PMENU_BUTTON button )
{
#if 0
	if( 1 )
	{
      int okay = 0;
		int done = 0;
		{
			PSI_CONTROL frame = LoadXMLFrame( WIDE("User_Login_Screen.isFrame") );
			if( frame )
			{
				Font *ppFont;
				ppFont = UseAFont( "User Login Font" );
				if( !ppFont )
				{
					ppFont = CreateAFont( "User Login Font", NULL, NULL, 0 );
					if( ppFont )
						font = (*ppFont );
					else
						font = NULL;
				}
				else
					font = (*ppFont );
				SetCommonFont( frame, font );
				InitLoginFrame( frame, &done, &okay );
				{
					int w, h;
					_32 dw, dh;
					GetFrameSize( frame, &w, &h );
					GetPageSize( &dw, &dh );
					if( dw > w && dh > h )
						MoveFrame( frame, ( dw - w ) / 2, ( dh - h ) / 2 );
					else if( dw > w )
						MoveFrame( frame, ( dw - w ) / 2, 0 );
					else if( dh > h )
						MoveFrame( frame, 0, ( dh - h ) / 2 );
					else
						MoveFrame( frame, 0, 0 );
				}
				DisplayFrame( frame );
				//EditFrame( frame, TRUE );
				CommonWait( frame );
				if( okay )
				{
					pg.iCurrentLogin = ReadLoginFrame( frame );
					if( pg.iCurrentLogin == INVALID_INDEX )
						okay = FALSE;
				}
				DestroyFrame( &frame );
			}
		}
		return okay;
	}
#endif
      // add additional security plugin stuff...
      if( button )
		{
         PTRSZVAL psv_context;
			CTEXTSTR name;
			PCLASSROOT data = NULL;
			for( name = GetFirstRegisteredName( TASK_PREFIX WIDE( "/common/Test Security" ), &data );
				 name;
				  name = GetNextRegisteredName( &data ) )
			{
				PTRSZVAL (CPROC*f)(PMENU_BUTTON);
				//snprintf( rootname, sizeof( rootname ), TASK_PREFIX WIDE( "/common/save common/%s" ), name );
				f = GetRegisteredProcedure2( (CTEXTSTR)data, PTRSZVAL, name, (PMENU_BUTTON) );
				if( f )
				{
					psv_context = f( button );
					if( psv_context )
					{
                  /* should probably record on per-plugin basis this value...... */
						return psv_context;
					}
				}
			}
		}
   return 0;
}

void CloseSecurityContext( PMENU_BUTTON button, PTRSZVAL psvSecurity )
{
#if 0
	if( button->SecurityContext
#endif
      if( button )
		{
			//PTRSZVAL psv_context;
			CTEXTSTR name;
			PCLASSROOT data = NULL;
			for( name = GetFirstRegisteredName( TASK_PREFIX WIDE( "/common/Close Security" ), &data );
				 name;
				  name = GetNextRegisteredName( &data ) )
			{
				void (CPROC*f)(PMENU_BUTTON,PTRSZVAL);
				//snprintf( rootname, sizeof( rootname ), TASK_PREFIX WIDE( "/common/save common/%s" ), name );
				f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PMENU_BUTTON,PTRSZVAL) );
				if( f )
				{
					f( button, psvSecurity );
				}
			}
		}
}

void CPROC SelectEditSecurity( PTRSZVAL psv, PSI_CONTROL listbox, PLISTITEM pli )
{
	{
		CTEXTSTR name;
		//TEXTCHAR invoke[256];
		void (CPROC*f)(PMENU_BUTTON);
		name = (CTEXTSTR)GetItemData( pli );
		f = GetRegisteredProcedure2( (CTEXTSTR)WIDE( "intershell/common/Edit Security" ), void, name, (PMENU_BUTTON) );
		if( f )
			f( (PMENU_BUTTON)psv );
	}

}


void CPROC EditSecurity( PTRSZVAL psv, PSI_CONTROL button )
{
	PSI_CONTROL pc_list;
   PLISTITEM pli;
	pc_list = GetNearControl( button, LISTBOX_SECURITY_MODULE );
	pli = GetSelectedItem( pc_list );
	if( pli )
	{
      SelectEditSecurity( psv, NULL, pli );
	}
	else
	{
      SimpleMessageBox( button, WIDE( "No selected security module" ), WIDE( "No Selection" ) );
	}
}


void CPROC EditSecurityNoList( PTRSZVAL psv, PSI_CONTROL button )
{
	SimpleMessageBox( button, WIDE( "No listbox to select security module" ), WIDE( "NO SECURITY LIST" ) );

}

