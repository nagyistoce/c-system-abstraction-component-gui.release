#include <stdhdrs.h>
#include <configscript.h>
#include <network.h>
#include <controls.h>
enum {
	LISTBOX_ITEMS = 1700,
	EDIT_SITE_NAME,
	EDIT_SYSTEM_NAME,
	EDIT_SYSTEM_ADDRESS,
	EDIT_MYSQL_SERVER,
   EDIT_SYSTEM_LOCAL_ADDRESS,
	CHECKBOX_IS_MYSQL_SERVER,
	CHECKBOX_USES_BINGODAY

};

struct site_info
{
	CTEXTSTR name;
	CTEXTSTR hostname;
	int hosts_sql;
	CTEXTSTR mysql_server;

   CTEXTSTR address;
   CTEXTSTR local_address;
};

struct map_file
{
	CTEXTSTR name;
	TEXTSTR filename;
};

static struct {
	PLIST maps; // list of struct map_file
	int nMaps;

	PLIST sites; // list of struct site_info

	struct map_file *selected_map;
	struct site_info *selected_site;

   int bingoday;
} l;

PRELOAD( RegisterResources )
{
   EasyRegisterResource( "Video Server/Editor", EDIT_SITE_NAME, EDIT_FIELD_NAME );
   EasyRegisterResource( "Video Server/Editor", EDIT_SYSTEM_NAME, EDIT_FIELD_NAME );
   EasyRegisterResource( "Video Server/Editor", EDIT_SYSTEM_ADDRESS, EDIT_FIELD_NAME );
   EasyRegisterResource( "Video Server/Editor", EDIT_SYSTEM_LOCAL_ADDRESS, EDIT_FIELD_NAME );
   EasyRegisterResource( "Video Server/Editor", EDIT_MYSQL_SERVER, EDIT_FIELD_NAME );
   EasyRegisterResource( "Video Server/Editor", LISTBOX_ITEMS, LISTBOX_CONTROL_NAME );
	EasyRegisterResource( "Video Server/Editor", CHECKBOX_IS_MYSQL_SERVER, RADIO_BUTTON_NAME );
	EasyRegisterResource( "Video Server/Editor", CHECKBOX_USES_BINGODAY, RADIO_BUTTON_NAME );

}


void CPROC FoundMap( PTRSZVAL psv, CTEXTSTR name, int flags )
{
	struct map_file *file;
   TEXTCHAR *start, *end;
	file = New( struct map_file );

	file->filename = StrDup( name );
	start = (TEXTCHAR*)pathrchr( file->filename );
	if( start )
		start++;
	else
		start = (TEXTCHAR*)file->filename;
	end = (TEXTCHAR*)StrRChr( start, '.' );
	if( !end )
		end = start + strlen( start );
	file->name = NewArray( TEXTCHAR, (end - start) + 1);
	snprintf( (TEXTCHAR*)file->name, (end-start)+1, "%*.*s", end-start, end-start, start );
   l.nMaps++;
	AddLink( &l.maps, file );
   return ;
}

void CPROC ButtonCreateMap( PTRSZVAL psv, PSI_CONTROL pc )
{
	TEXTCHAR tmp[256];
	if( SimpleUserQuery( tmp, sizeof( tmp ), "Enter new video map name", NULL ) )
	{
		TEXTCHAR tmp2[256];
		TEXTCHAR filename[256];
		struct map_file *map = New( struct map_file );
		GetFileGroupText( GetFileGroup( "Configuration Maps", "../resources" ), tmp2, sizeof( tmp2 ) );
		snprintf( filename, sizeof( filename ), "%s/%s.Map", tmp2, tmp );
		MemSet( map, 0, sizeof( struct map_file ) );
		map->filename = StrDup( filename );
		map->name = StrDup( tmp );
		AddLink( &l.maps, map );
		SetItemData( AddListItem( GetNearControl( pc, LISTBOX_ITEMS ), tmp ), (PTRSZVAL)map );
	}
}


void SelectMap( void )
{
	void *info = NULL;
	TEXTCHAR tmp[256];
	while( ScanFiles( GetFileGroupText( GetFileGroup( "Configuration Maps", "../resources" ), tmp, sizeof( tmp ) ), "*.Map", &info, FoundMap, 0, 0 ) );

   //if( l.nMaps > 1 )
	{
		int okay = 0;
		int done = 0;
		PSI_CONTROL frame = CreateFrame( "Select Site Map", 120, 120, 480, 320, BORDER_NORMAL, NULL );
		PSI_CONTROL listbox = MakeNamedControl( frame, LISTBOX_CONTROL_NAME, 5, 5, 470, 285, LISTBOX_ITEMS );
		AddCommonButtons( frame, &done, &okay );
		{
			struct map_file *map;
			INDEX idx;
			LIST_FORALL( l.maps, idx, struct map_file *, map )
			{
            SetItemData( AddListItem( listbox, map->name ), (PTRSZVAL)map );
			}
		}
		{
			int x;
			x = 480-(COMMON_BUTTON_WIDTH+COMMON_BUTTON_PAD)*3;

			MakeButton( frame
						 , x , 320-(COMMON_BUTTON_PAD+COMMON_BUTTON_HEIGHT)
						 , COMMON_BUTTON_WIDTH, COMMON_BUTTON_HEIGHT
						 , 0, "Create", 0, ButtonCreateMap, (PTRSZVAL)frame );
		}
		DisplayFrame( frame );
		CommonWait( frame );
		if( okay )
		{
			PLISTITEM pli = GetSelectedItem( listbox );
			PTRSZVAL psv_map = GetItemData( pli );
         l.selected_map = (struct map_file *)psv_map;
		}
      DestroyFrame( &frame );
	}
	//else
	//	l.selected_map = (struct map_file*)GetLink( &l.maps, 0 );
}

void EditSite( PLISTITEM pli )
{
	TEXTCHAR tmp[256];
   TEXTCHAR frame_file[256];
	GetFileGroupText( GetFileGroup( "Configuration Maps", "../resources" ), tmp, sizeof( tmp ) );
	snprintf( frame_file, sizeof( frame_file ), "edit_video_link_site.Frame", tmp );
	{
		PSI_CONTROL frame = LoadXMLFrame( "edit_video_link_site.Frame" );
		struct site_info *site = (struct site_info *)GetItemData( pli );
		if( frame )
		{
			int okay = 0;
			int done = 0;
			{
				SetControlText( GetControl( frame, EDIT_SITE_NAME ), site->name );
				SetControlText( GetControl( frame, EDIT_SYSTEM_NAME ), site->hostname );
				SetControlText( GetControl( frame, EDIT_SYSTEM_ADDRESS ), site->address );
				SetControlText( GetControl( frame, EDIT_SYSTEM_LOCAL_ADDRESS ), site->local_address );
				SetControlText( GetControl( frame, EDIT_MYSQL_SERVER ), site->mysql_server );
            SetCheckState( GetControl( frame, CHECKBOX_IS_MYSQL_SERVER ), site->hosts_sql );
            SetCheckState( GetControl( frame, CHECKBOX_USES_BINGODAY ), l.bingoday );

			}

			SetCommonButtons( frame, &done, &okay );
			DisplayFrame( frame );
			CommonWait( frame );
			if( okay )
			{
				TEXTCHAR tmp[256];
				GetControlText( GetControl( frame, EDIT_SYSTEM_NAME ), tmp, sizeof( tmp ) );
				site->hostname = StrDup( tmp );
				GetControlText( GetControl( frame, EDIT_SYSTEM_ADDRESS ), tmp, sizeof( tmp )  );
				site->address = StrDup( tmp );
				GetControlText( GetControl( frame, EDIT_SYSTEM_LOCAL_ADDRESS ), tmp, sizeof( tmp )  );
				site->local_address = StrDup( tmp );
				GetControlText( GetControl( frame, EDIT_MYSQL_SERVER ), tmp, sizeof( tmp )  );
            site->mysql_server = StrDup( tmp );
            site->hosts_sql = GetCheckState( GetControl( frame, CHECKBOX_IS_MYSQL_SERVER ) );
            l.bingoday = GetCheckState( GetControl( frame, CHECKBOX_USES_BINGODAY ) );

			}
			DestroyFrame( &frame );
		}
	}
}

void CPROC ButtonEdit( PTRSZVAL psv, PSI_CONTROL pc )
{
   PSI_CONTROL frame = (PSI_CONTROL)psv;
	PLISTITEM pli = GetSelectedItem( GetControl( frame, LISTBOX_ITEMS  ) );
   if( pli )
		EditSite( pli );
}

void CPROC ButtonCreate( PTRSZVAL psv, PSI_CONTROL pc )
{
   TEXTCHAR tmp[256];
	if( SimpleUserQuery( tmp, sizeof( tmp ), "Enter new site name", NULL ) )
	{
		struct site_info *site = New( struct site_info );
      MemSet( site, 0, sizeof( struct site_info ) );
		site->name = StrDup( tmp );
		AddLink( &l.sites, site );
      SetItemData( AddListItem( GetNearControl( pc, LISTBOX_ITEMS ), tmp ), (PTRSZVAL)site );
	}
}
void CPROC ButtonDelete( PTRSZVAL psv, PSI_CONTROL pc )
{
   PSI_CONTROL frame = (PSI_CONTROL)psv;
	PLISTITEM pli = GetSelectedItem( GetControl( frame, LISTBOX_ITEMS  ) );
	if( pli )
	{
		struct site_info *site = (struct site_info *)GetItemData( pli );
		DeleteLink( &l.sites, site );
      DeleteListItem( GetControl( frame, LISTBOX_ITEMS ), pli );
	}
}

void SelectSite( void )
{

	{
		int okay = 0;
		int done = 0;
		PSI_CONTROL frame = CreateFrame( "Select Site", 100, 100, 480, 320, BORDER_NORMAL, NULL );
		PSI_CONTROL listbox = MakeNamedControl( frame, LISTBOX_CONTROL_NAME, 5, 5, 470, 285, LISTBOX_ITEMS );
		AddCommonButtons( frame, &done, &okay );
		{
			int x;
			x = 480-(COMMON_BUTTON_WIDTH+COMMON_BUTTON_PAD)*3;

			MakeButton( frame
						 , x , 320-(COMMON_BUTTON_PAD+COMMON_BUTTON_HEIGHT)
						 , COMMON_BUTTON_WIDTH, COMMON_BUTTON_HEIGHT
						 , 0, "Edit", 0, ButtonEdit, (PTRSZVAL)frame );
			x = 480-(COMMON_BUTTON_WIDTH+COMMON_BUTTON_PAD)*4;

			MakeButton( frame
						 , x , 320-(COMMON_BUTTON_PAD+COMMON_BUTTON_HEIGHT)
						 , COMMON_BUTTON_WIDTH, COMMON_BUTTON_HEIGHT
						 , 0, "Create", 0, ButtonCreate, (PTRSZVAL)frame );
			x = 480-(COMMON_BUTTON_WIDTH+COMMON_BUTTON_PAD)*5;

			MakeButton( frame
						 , x , 320-(COMMON_BUTTON_PAD+COMMON_BUTTON_HEIGHT)
						 , COMMON_BUTTON_WIDTH, COMMON_BUTTON_HEIGHT
						 , 0, "Delete", 0, ButtonDelete, (PTRSZVAL)frame );
		}
		{
			struct site_info *site;
			INDEX idx;
			LIST_FORALL( l.sites, idx, struct site_info *, site )
			{
            SetItemData( AddListItem( listbox, site->name ), (PTRSZVAL)site );
			}
		}
		DisplayFrame( frame );
		CommonWait( frame );
		if( okay )
		{
			PLISTITEM pli = GetSelectedItem( listbox );
			PTRSZVAL psv_site = GetItemData( pli );
         l.selected_site = (struct site_info *)1;//psv_site;
		}
      DestroyFrame( &frame );
	}
}

PTRSZVAL CPROC AddSite( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	struct site_info *site = New( struct site_info );
   MemSet( site, 0, sizeof( struct site_info ) );
   site->name = StrDup( name );
   AddLink( &l.sites, site );
   return (PTRSZVAL)site;
}

PTRSZVAL CPROC SetSiteHostname( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	struct site_info *site = (struct site_info*)psv;
   site->hostname = StrDup( name );
   return psv;
}

PTRSZVAL CPROC SetSiteSQLServer( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	struct site_info *site = (struct site_info*)psv;
   site->mysql_server = StrDup( name );
   return psv;
}

PTRSZVAL CPROC SetSiteAddress( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	struct site_info *site = (struct site_info*)psv;
   site->address = StrDup( name );
   return psv;
}

PTRSZVAL CPROC SetSiteLocalAddress( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	struct site_info *site = (struct site_info*)psv;
   site->local_address = StrDup( name );
   return psv;
}

PTRSZVAL CPROC SetSiteSQL( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, yes_no );
	struct site_info *site = (struct site_info*)psv;
	site->hosts_sql = yes_no;
	return psv;
}

PTRSZVAL CPROC SetBingodayOption( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, yes_no );
	struct site_info *site = (struct site_info*)psv;
	l.bingoday = yes_no;
	return psv;
}
void ReadMap( void )
{
	PCONFIG_HANDLER pch = CreateConfigurationHandler();
	AddConfigurationMethod( pch, "site %m", AddSite );
	AddConfigurationMethod( pch, "hostname %m", SetSiteHostname );
	AddConfigurationMethod( pch, "serves Mysql?%b", SetSiteSQL );
	AddConfigurationMethod( pch, "Mysql Server %m", SetSiteSQLServer );
	AddConfigurationMethod( pch, "address %m", SetSiteAddress );
	AddConfigurationMethod( pch, "local address %m", SetSiteLocalAddress );
	AddConfigurationMethod( pch, "Uses Bingoday?%b", SetBingodayOption );
	ProcessConfigurationFile( pch, l.selected_map->filename, 0 );
	DestroyConfigurationHandler( pch );
}


void UpdateConfiguration( void )
{
	// based on selected map and site, set hostname, and ODBC connection parameters.
	FILE *file = sack_fopen( 0, l.selected_map->filename, "wt" );
	{
		INDEX idx;
		struct site_info *site;
		fprintf( file, "# site file defines locations of servers\n" );
		fprintf( file, "#   site <name> \n" );
		fprintf( file, "#   Hostname <name> - sets the hostname of the system, also maps in 'hosts'\n" );
		fprintf( file, "#   address <ip> - defines the IP address of the system\n" );
		fprintf( file, "#   serves MySQL <yes/no> - if the sql server runs on this node... then others know where also\n" );
		fprintf( file, "#   MySQL Server - if not running the server, this may indicate an external server from here - implies services for others\n" );
		fprintf( file, "\n\n" );
		fprintf( file, "Uses Bingoday?%s", l.bingoday?"Yes":"No" );
		fprintf( file, "\n\n" );
		LIST_FORALL( l.sites, idx, struct site_info *, site )
		{
			fprintf( file, "site %s\n", site->name );
			if( site->hostname ) fprintf( file, "Hostname %s\n", site->hostname );
			if( site->address ) fprintf( file, "address %s\n", site->address );
			fprintf( file, "serves MySQL %s\n", site->hosts_sql?"Yes":"No" );
			if( site->mysql_server ) fprintf( file, "MySQL Server %s\n", site->mysql_server );
         fprintf( file, "\n\n" );
		}
	}

}

#ifdef _MSC_VER
int APIENTRY WinMain( HINSTANCE a, HINSTANCE b, LPSTR c, int d )
#else
int main( void )
#endif
{

	SelectMap();
	if( !l.selected_map )
      return 0;
	ReadMap();
	SelectSite();

	if( l.selected_site )
	{
		TEXTCHAR filename[256];
		int n;
		UpdateConfiguration();
		for( n = 0; l.selected_map->filename[n]; n++ )
			if( l.selected_map->filename[n] == '/' )
				l.selected_map->filename[n] = '\\';
		snprintf( filename, sizeof( filename ), "Explorer /select,%s", l.selected_map->filename );
		lprintf( "command [%s]", filename );
		system( filename );
	}
	return 0;
}

