#include <final_types.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
int SetRegistryItem( HKEY hRoot, char *pPrefix,
                     char *pProduct, char *pKey, 
                     DWORD dwType,
                     const BYTE *pValue, int nSize )
{
   char szString[512];
   char *pszString = szString;
   DWORD dwStatus;
   HKEY hTemp;

   if( pProduct )
      snprintf( szString, sizeof( szString ),"%s%s", pPrefix, pProduct );
   else
      snprintf( szString, sizeof( szString ),"%s", pPrefix );

   dwStatus = RegOpenKeyEx( hRoot,
                            pszString, 0, 
                            KEY_WRITE, &hTemp );
   if( dwStatus == ERROR_FILE_NOT_FOUND )
   {
      DWORD dwDisposition;
      dwStatus = RegCreateKeyEx( hRoot, 
                                 pszString, 0
                             , ""
                             , REG_OPTION_NON_VOLATILE
                             , KEY_WRITE
                             , NULL
                             , &hTemp
                             , &dwDisposition);
      if( dwDisposition == REG_OPENED_EXISTING_KEY )
         fprintf( stderr, "Failed to open, then could open???" );
      if( dwStatus )   // ERROR_SUCCESS == 0 
         return FALSE; 
   }
   if( (dwStatus == ERROR_SUCCESS) && hTemp )
   {
      dwStatus = RegSetValueEx(hTemp, pKey, 0
                                , dwType
                                , pValue, nSize );
      RegCloseKey( hTemp );
      if( dwStatus == ERROR_SUCCESS )
      {
         return TRUE;
      }
   }
   return FALSE;
}
#endif

int main( int argc, char **argv )
{
	char *path = strdup( argv[0] );
	FILE *out;
   char tmp[256];
	char *last1 = strrchr( path, '/' );
	char *last2 = strrchr( path, '\\' );
   char *last;
	if( last1 )
		if( last2 )
			if( last1 > last2 )
				last = last1;
			else
				last = last2;
		else
			last = last1;
	else
		if( last2 )
			last = last2;
		else
			last = NULL;
	if( last )
		last[0] = 0;
	else
      path = ".";
   snprintf( tmp, sizeof( tmp ), "%s/CMakePackage", path );
	out = fopen( "CMakePackage", "wt" );
	if( out )
	{
		int c;
		for( c = 0; path[c]; c++ )
         if( path[c] == '\\' ) path[c] = '/';
#ifdef WIN32
		SetRegistryItem( HKEY_LOCAL_MACHINE, "SOFTWARE", "\\SACK", "Install_Dir", REG_SZ, path, strlen(path));
      if(0)
		{
         FILE *out2;
			snprintf( tmp, sizeof( tmp ), "%s/CMakePackage", path );
			out2 = fopen( "%s/MakeShortcut.vbs", "wt" );
			if( out2 )
			{
				fprintf( out2, "set WshShell = WScript.CreateObject(\"WScript.Shell\" )\n" );
				fprintf( out2, "strDesktop = WshShell.SpecialFolders(\"AllUsersDesktop\" )\n" );
				fprintf( out2, "set oShellLink = WshShell.CreateShortcut(strDesktop & \"\\shortcut name.lnk\" )\n" );
				fprintf( out2, "oShellLink.TargetPath = \"c:\\application folder\\application.exe\"\n" );
				fprintf( out2, "oShellLink.WindowStyle = 1\n" );
				fprintf( out2, "oShellLink.IconLocation = \"c:\\application folder\\application.ico\"\n" );
				fprintf( out2, "oShellLink.Description = \"Shortcut Script\"\n" );
				fprintf( out2, "oShellLink.WorkingDirectory = \"c:\\application folder\"\n" );
				fprintf( out2, "oShellLink.Save\n" );
				fclose( out2 );
				system( tmp );
			}
		}
#endif
      fprintf( out, "set( SACK_BASE %s )\n", path );
		fprintf( out, "set( SACK_INCLUDE_DIR ${SACK_BASE}/include/sack )\n" );
#ifdef __WATCOMC__
		fprintf( out, "set( SACK_LIBRARIES sack_bag sack_bag_pp )\n" );
#else
		fprintf( out, "set( SACK_LIBRARIES sack_bag sack_bag++ )\n" );
#endif
      fprintf( out, "set( SACK_LIBRARY_DIR ${SACK_BASE}/lib )\n" );
      fprintf( out, "\n" );
      fprintf( out, "  if( ${CMAKE_COMPILER_IS_GNUCC} )\n" );
      fprintf( out, "  SET( FIRST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/sack/deadstart_list.c )\n" );
      fprintf( out, "  SET( FIRST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/sack/deadstart_list.c )\n" );
      fprintf( out, "  SET( LAST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/sack/deadstart_lib.c ${SACK_BASE}/src/sack/deadstart_end.c )\n" );
      fprintf( out, "  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/sack/deadstart_lib.c ${SACK_BASE}/src/sack/deadstart_prog.c ${SACK_BASE}/src/sack/deadstart_end.c )\n" );
		fprintf( out, "  endif()\n" );
      fprintf( out, "\n" );
		fprintf( out, "if( ${MSVC}${WATCOM} )\n" );
      fprintf( out, "  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/sack/deadstart_prog.c )\n" );
		fprintf( out, "endif()\n" );
      fprintf( out, "\n" );
      fprintf( out, "SET( DATA_INSTALL_PREFIX resources )\n" );
		fprintf( out, "include( ${SACK_BASE}/DefaultInstall.cmake )\n" );

      fclose( out );
	}
   return 0;
}
