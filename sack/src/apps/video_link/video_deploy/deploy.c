#include <stdhdrs.h>
#include <configscript.h>
#include <network.h>
#include <controls.h>
#include <sqlgetoption.h>

#include <rcomlib.h>


enum {
	LISTBOX_ITEMS,

};

struct site_info
{
	CTEXTSTR name;
	CTEXTSTR hostname;
	CTEXTSTR host_address;
	int hosts_sql;
	CTEXTSTR mysql_server;

   CTEXTSTR address;
   CTEXTSTR expected_address;
   CTEXTSTR mac_address;
   CTEXTSTR local_address;

};

struct map_file
{
	CTEXTSTR name;
	CTEXTSTR filename;
};

static struct {
	PLIST maps; // list of struct map_file
	int nMaps;

	PLIST sites; // list of struct site_info

	struct map_file *selected_map;
	struct site_info *selected_site;
   int bingoday;

	struct site_info *sql_site;

   // this should be copied to the default for the controller.
	CTEXTSTR controller_config_name;
   int controller;

   CTEXTSTR driver_name;

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
   TEXTCHAR tmp[256];
	while( ScanFiles( GetFileGroupText( GetFileGroup( "Configuration Maps", "../resources" ), tmp, sizeof( tmp ) ), "*.Map", &info, FoundMap, 0, 0 ) );

   if( l.nMaps > 1 )
	{
		int okay = 0;
      int done = 0;
		PSI_CONTROL frame = CreateFrame( "Select Site Map", -1, -1, 480, 320, BORDER_NORMAL, NULL );
		PSI_CONTROL listbox = MakeNamedControl( frame, LISTBOX_CONTROL_NAME, 5, 5, 470, 285, 0 );
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
      DestroyFrame( &frame );
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
		PSI_CONTROL listbox = MakeNamedControl( frame, LISTBOX_CONTROL_NAME, 5, 5, 470, 285, 0 );
		AddCommonButtons( frame, &done, &okay );
		{
			struct site_info *site;
			INDEX idx;
			LIST_FORALL( l.sites, idx, struct site_info *, site )
			{
				PLISTITEM pli = SetItemData( AddListItem( listbox, site->name ), (PTRSZVAL)site );
				if( ( StrCaseCmp( site->name, GetSystemName() ) == 0 )
               || ( StrCaseCmp( site->hostname, GetSystemName() ) == 0 ) )
				{
               SetSelectedItem( listbox, pli );
				}
			}
         AddListItem( listbox, "Link Controller" );
		}
		DisplayFrame( frame );
		CommonWait( frame );
		if( okay )
		{
			PLISTITEM pli = GetSelectedItem( listbox );
			PTRSZVAL psv_site = GetItemData( pli );
         l.selected_site = (struct site_info *)psv_site;
         l.controller = (!psv_site);
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

PTRSZVAL CPROC SetSiteExpectedAddress( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	struct site_info *site = (struct site_info*)psv;
   site->expected_address = StrDup( name );
   return psv;
}


PTRSZVAL CPROC SetControllerConfigName( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
   l.controller_config_name = StrDup( name );
   return psv;
}

PTRSZVAL CPROC SetSiteMacAddress( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	struct site_info *site = (struct site_info*)psv;
   site->mac_address = StrDup( name );
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
   AddConfigurationMethod( pch, "expected address %m", SetSiteExpectedAddress );
   AddConfigurationMethod( pch, "mac address %m", SetSiteMacAddress );
	AddConfigurationMethod( pch, "local address %m", SetSiteLocalAddress );
	AddConfigurationMethod( pch, "Uses Bingoday?%b", SetBingodayOption );
   AddConfigurationMethod( pch, "controller config=%m", SetControllerConfigName );
	ProcessConfigurationFile( pch, l.selected_map->filename, 0 );
   DestroyConfigurationHandler( pch );
}

void UpdateHostsFile( void )
{
	CTEXTSTR root = OSALOT_GetEnvironmentVariable( "SystemRoot" );
	TEXTCHAR tmp[256];
	FILE *file;
	snprintf( tmp, sizeof( tmp ), "%s/system32/drivers/etc/hosts", root );
	file = sack_fopen( 0, tmp, "wt" );
	fprintf( file, "# Copyright (c) 1993-2009 Microsoft Corp.\n" );
	fprintf( file, "#\n" );
	fprintf( file, "# This is a sample HOSTS file used by Microsoft TCP/IP for Windows.\n" );
	fprintf( file, "#\n" );
	fprintf( file, "# This file contains the mappings of IP addresses to host names. Each\n" );
	fprintf( file, "# entry should be kept on an individual line. The IP address should\n" );
	fprintf( file, "# be placed in the first column followed by the corresponding host name.\n" );
	fprintf( file, "# The IP address and the host name should be separated by at least one\n" );
	fprintf( file, "# space.\n" );
	fprintf( file, "#\n" );
	fprintf( file, "# Additionally, comments (such as these) may be inserted on individual\n" );
	fprintf( file, "# lines or following the machine name denoted by a '#' symbol.\n" );
	fprintf( file, "#\n" );
	fprintf( file, "# For example:\n" );
	fprintf( file, "#\n" );
	fprintf( file, "#      102.54.94.97     rhino.acme.com          # source server\n" );
	fprintf( file, "#       38.25.63.10     x.acme.com              # x client host\n" );
	fprintf( file, "\n" );
	fprintf( file, "# localhost name resolution is handled within DNS itself.\n" );
	fprintf( file, "#	127.0.0.1       localhost\n" );
	fprintf( file, "#	::1             localhost\n" );
	fprintf( file, "\n" );
	{
		INDEX idx;
		struct site_info *site;

      l.sql_site = NULL;
		LIST_FORALL( l.sites, idx, struct site_info *, site )
		{
			if( site->hosts_sql || ( site->mysql_server && site->mysql_server[0] ) )
			{
				if( l.sql_site )
               ; // misconfiguration
				l.sql_site = site;
			}
			fprintf( file, "%s %s %s vsrvr.%s bdata.%s\n"
					 , site->host_address
					 , site->name
					 , site->hostname
					 , site->hostname
					 , site->hostname
					 );
		}
	}
	fclose( file );

}

void UpdateMySQL( void )
{
   FILE *file;
	system( "mysql.proxy.service uninstall" );
	if( l.selected_site != l.sql_site )
	{
		file = sack_fopen( 0, "mysql.proxy.service.conf", "wt" );
		if( file )
		{
			fprintf( file, "MySQL: 3306 %s:3306\n", l.sql_site->address );
		}
		fclose( file );
		system( "mysql.proxy.service install" );
	}
	if( l.selected_site == l.sql_site )
	{
		if( !l.sql_site->hosts_sql )
		{
			file = sack_fopen( 0, "mysql.proxy.service.conf", "wt" );
			if( file )
			{
				fprintf( file, "MySQL: 3306 %s:3306\n", l.sql_site->mysql_server );
			}
			fclose( file );
			system( "mysql.proxy.service install" );
		}
		else
		{
			lprintf( "Should try and setup and start mysql?  This server should have a mysql service" );
         system( "mysql/bin/mysqld install" );
         system( "net start MySQL" );
		}
	}

}

void UpdateEventMap( void)
{
	INDEX idx;
	int iSend;
	struct site_info *site;

	TEXTCHAR tmp_send[256];


	//LIST_FORALL( l.sites, idx, struct site_info *, site )
	{
		SACK_WriteProfileString( "video_link_server", "Video Server/Service Events/interface 1", l.selected_site->address );
		SACK_WriteProfileString( "video_link_server", "Video Server/Service Events/interface 2", l.selected_site->local_address );
	}
	iSend = 1;
	LIST_FORALL( l.sites, idx, struct site_info *, site )
	{
		if( site != l.selected_site )
		{
			snprintf( tmp_send, sizeof( tmp_send ), "Video Server/Service Events/interface 1/Send To %d", iSend );
			iSend++;
			SACK_WriteProfileString( "video_link_server", tmp_send, site->address );

		}
	}
	snprintf( tmp_send, sizeof( tmp_send ), "Video Server/Service Events/interface 1/Send To %d", iSend );
	SACK_WriteProfileString( "video_link_server", tmp_send, "" );

	snprintf( tmp_send, sizeof( tmp_send ), "Video Server/Service Events/interface 2/Send To 1" );
	SACK_WriteProfileString( "video_link_server", tmp_send, "172.17.255.255" );
	snprintf( tmp_send, sizeof( tmp_send ), "Video Server/Service Events/interface 2/Send To 2" );
	SACK_WriteProfileString( "video_link_server", tmp_send, "" );

}

void UpdateConfiguration( void )
{
	// based on selected map and site, set hostname, and ODBC connection parameters.
	if( StrCaseCmp( GetSystemName(), l.selected_site->hostname ) )
	{
		char cmd[256];
      snprintf( cmd, sizeof( cmd ), "ClearShell setcomputer.ps1 %s", l.selected_site->hostname );
      system( cmd );
		lprintf( "hostname needs update - we'll need a reboot too" );
	}

	UpdateHostsFile();

	SACK_WriteProfileInt( "video_link_server", "Use bingoday for link state (else use 0)", l.bingoday );
	SACK_WriteProfileString( "video_link_server", "My Hall Name", l.selected_site->name );


	UpdateMySQL();

	UpdateEventMap();

}


static void copy( char *src, char *dst )
{
	{
		static _8 buffer[4096];
		FILE *in, *out;
		_64 filetime;
		_64 filetime_dest;

		filetime = GetFileWriteTime( src );
		filetime_dest = GetFileWriteTime( dst );

		if( filetime <= filetime_dest )
			return;
		in = sack_fopen( 0, src, WIDE("rb") );
		if( in )
			out = sack_fopen( 0, dst, WIDE("wb") );
		else
			out = NULL;
		if( in && out )
		{
			int len;
			while( len = fread( buffer, 1, sizeof( buffer ), in ) )
				fwrite( buffer, 1, len, out );
		}
		if( in )
			fclose( in );
		if( out )
			fclose( out );
		SetFileWriteTime( dst, filetime );
	}
}

void FinishCopying( void )
{
   copy( "proxy.service.exe", "proxy_bdata.exe" );
	copy( "proxy.service.exe", "mysql.proxy.service.exe" );
	{
		CTEXTSTR args[] = { "video.ini", "/vlc/config", "vlc_path", "c:/tools/vlc-1.1.4", NULL };
		LaunchProgram( "SetOption", NULL, args );
	}
	{
		CTEXTSTR args[] = { "-ext", "-dir", "install/mysql-5.4-minimal.zip", NULL };
		LaunchProgram( "install/pkzip25", NULL, args );
	}
	{
		CTEXTSTR args[] = { "x", "install/vlc-1.1.4-win32.7z", NULL };
		LaunchProgram( "install/7za x", NULL, args );
	}
}


static int CPROC FindDriver( PTRSZVAL psv, CTEXTSTR key )
{
	if( StrCaseStr( key, "mysql" ) )
	{
		TEXTCHAR *driver = (TEXTCHAR*)psv;
      StrCpy( driver, key );
      return 1;
	}
   return 0;
}

void UpdateODBC( void )
{
	TEXTCHAR name[256];
	snprintf( name, sizeof( name ), "not-set" );
	SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "SERVER", REG_SZ, name, StrLen( name ) );
}

void CheckOdbc( void )
{
	// this will take some work to do properly...
   // need to enumerate the list of drivers first and pick something like mysql to get driver path
	TEXTCHAR name[256];
	TEXTCHAR value[256];
   TEXTCHAR driver[256];
   TEXTCHAR driver_val[256];

   driver[0] = 0;
	ProcessRegistryKeys( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbcinst.ini", FindDriver, (PTRSZVAL)driver );

	if( !driver[0] )
	{
		MessageBox( NULL, "Failed to locate MySQL ODBC driver - did you install the connector?", "ODBC Failure", MB_OK );
      return;
	}

	GetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbcinst.ini\\", driver, "Driver", REG_SZ, driver_val, sizeof( driver_val ) );


	if( !GetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "@", REG_SZ, name, sizeof( name ) ) )
	{
      snprintf( name, sizeof( name ), "Video Link Server" );
		SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "DESCRIPTION", REG_SZ, "Video Link Server", StrLen( name ) );
      snprintf( name, sizeof( name ), "video_server" );
		SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "DATABASE", REG_SZ, name, StrLen( name ) );
      snprintf( name, sizeof( name ), driver_val );
		SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "DRIVER", REG_SZ, name, StrLen( name ) );
      snprintf( name, sizeof( name ), "3306" );
		SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "PORT", REG_SZ, name, StrLen( name ) );
      snprintf( name, sizeof( name ), "dkc408a1f" );
		SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "PWD", REG_SZ, name, StrLen( name ) );

      snprintf( name, sizeof( name ), "not-set" );
		SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "SERVER", REG_SZ, name, StrLen( name ) );

      snprintf( name, sizeof( name ), "fortunet" );
		SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "vsrvr", "UID", REG_SZ, name, StrLen( name ) );
      SetRegistryItem( HKEY_LOCAL_MACHINE, "Software\\ODBC\\odbc.ini\\", "ODBC Data Sources", "vsrvr", REG_SZ, driver, StrLen( driver ) );
	}
}

#ifdef _MSC_VER
int APIENTRY WinMain( HINSTANCE a, HINSTANCE b, LPSTR c, int d )
#else
int main( void )
#endif
{
   CheckOdbc();
   FinishCopying();
	SelectMap();
	if( !l.selected_map )
      return 0;
	ReadMap();
	SelectSite();

	if( l.selected_site )
		UpdateConfiguration();
	else if( l.controller )
	{
      // this system is only a controller - setup the link control configuration....
	}
	else
		SimpleMessageBox( NULL, "Configuration Canceled", "System setup may not be complete." );
	return 0;
}

