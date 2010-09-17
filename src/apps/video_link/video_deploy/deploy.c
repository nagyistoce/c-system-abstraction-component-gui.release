#include <stdhdrs.h>
#include <configscript.h>
#include <controls.h>
enum {
	LISTBOX_ITEMS,

};

struct site_info
{
	CTEXTSTR name;
	CTEXTSTR hostname;
	int hosts_sql;
	CTEXTSTR sql_server;
   CTEXTSTR address;

};

struct map_file
{
	CTEXTSTR name;
   CTEXTSTR filename;
};

static struct {
	PLIST maps; // list of struct map_file
	int nMaps;

	PLIST sites; // list of struct map_file

	struct map_file *selected_map;
   struct site_info *selected_site;
} l;




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

void SelectMap( void )
{
   void *info = NULL;
	while( ScanFiles( ".", "*.Map", &info, FoundMap, 0, 0 ) );

   if( l.nMaps > 1 )
	{
		int okay = 0;
      int done = 0;
		PSI_CONTROL frame = CreateFrame( "Select Site Map", -1, -1, 480, 320, BORDER_NORMAL, NULL );
		PSI_CONTROL listbox = MakeNamedControl( frame, LISTBOX_CONTROL_NAME, 5, 5, 470, 295, 0 );
		AddCommonButtons( frame, &done, &okay );
		{
			struct map_file *map;
			INDEX idx;
			LIST_FORALL( l.maps, idx, struct map_file *, map )
			{
            SetItemData( AddListItem( listbox, map->name ), (PTRSZVAL)map );
			}
		}
		DisplayFrame( frame );
		CommonWait( frame );
		if( okay )
		{
			PLISTITEM pli = GetSelectedItem( listbox );
			PTRSZVAL psv_map = GetItemData( pli );
         l.selected_map = (struct map_file *)psv_map;
		}
	}
	else
      l.selected_map = (struct map_file*)GetLink( &l.maps, 0 );
}



void SelectSite( void )
{

	{
		int okay = 0;
      int done = 0;
		PSI_CONTROL frame = CreateFrame( "Select Site", -1, -1, 480, 320, BORDER_NORMAL, NULL );
		PSI_CONTROL listbox = MakeNamedControl( frame, LISTBOX_CONTROL_NAME, 5, 5, 470, 295, 0 );
		AddCommonButtons( frame, &done, &okay );
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
         l.selected_site = (struct site_info *)psv_site;
		}
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

PTRSZVAL CPROC SetSiteSQL( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, yes_no );
	struct site_info *site = (struct site_info*)psv;
   site->hosts_sql = yes_no;
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
	ProcessConfigurationFile( pch, l.selected_map->filename, 0 );
   DestroyConfigurationHandler( pch );
}

void UpdateConfiguration( void )
{
	// based on selected map and site, set hostname, and ODBC connection parameters.
	if( StrCaseCmp( GetSystemName(), l.selected_site->hostname ) )
	{
		lprintf( "hostname needs update" );

	}



}

int main( void )
{
	SelectMap();
	if( !l.selected_map )
      return 0;
	ReadMap();
	SelectSite();

   if( l.selected_site )
		UpdateConfiguration();

   return 0;
}
