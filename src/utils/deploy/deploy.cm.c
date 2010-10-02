#include <sack_types.h>
#define SUFFER_WITH_NO_SNPRINTF
#include <final_types.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif


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
#ifdef _MSC_VER
	fopen_s( &out, tmp, "wt" );
#else
	out = fopen( tmp, "wt" );
#endif
	if( out )
	{
		int c;
		for( c = 0; path[c]; c++ )
			if( path[c] == '\\' ) path[c] = '/';
#ifdef WIN32
		SetRegistryItem( HKEY_LOCAL_MACHINE, "SOFTWARE", "\\Freedom Collective\\SACK", "Install_Dir", REG_SZ, (BYTE*)path, strlen(path));
		if(0)
		{
			FILE *out2;
			snprintf( tmp, sizeof( tmp ), "%s/CMakePackage", path );
#ifdef _MSC_VER
			fopen_s( &out2, "%s/MakeShortcut.vbs", "wt" );
#else
			out2 = fopen( "%s/MakeShortcut.vbs", "wt" );
#endif
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
		{
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
		}

		fprintf( out, "set( SACK_BASE %s )\n", path );
		fprintf( out, "set( SACK_INCLUDE_DIR $""{SACK_BASE}/include/SACK )\n" );
		fprintf( out, "set( SACK_BAG_PLUSPLUS @SACK_BAG_PLUSPLUS@ )\n" );
		fprintf( out, "set( SACK_LIBRARIES sack_bag $""{SACK_BAG_PLUSPLUS} )\n" );
		fprintf( out, "set( SACK_LIBRARY_DIR $""{SACK_BASE}/$""{CMAKE_BUILD_TYPE}/lib )\n" );
		fprintf( out, "\n" );
		fprintf( out, "if( MSVC )\n" );
		fprintf( out, "set( SUPPORTS_PARALLEL_BUILD_TYPE 1 )\n" );
		fprintf( out, "endif( MSVC )\n" );
		fprintf( out, "\n" );
		fprintf( out, "set(  CMAKE_CXX_FLAGS_DEBUG \"$""{CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG\" )\n" );
		fprintf( out, "set(  CMAKE_CXX_FLAGS_RELWITHDEBINFO \"$""{CMAKE_CXX_FLAGS_RELWITHDEBINFO} -D_DEBUG\" )\n" );
		fprintf( out, "set(  CMAKE_C_FLAGS_DEBUG \"$""{CMAKE_C_FLAGS_DEBUG} -D_DEBUG\" )\n" );
		fprintf( out, "set(  CMAKE_C_FLAGS_RELWITHDEBINFO \"$""{CMAKE_C_FLAGS_RELWITHDEBINFO} -D_DEBUG\" )\n" );
		fprintf( out, "\n" );
		fprintf( out, "  if( $""{CMAKE_COMPILER_IS_GNUCC} )\n" );
		fprintf( out, "  SET( FIRST_GCC_LIBRARY_SOURCE $""{SACK_BASE}/src/sack/deadstart_list.c )\n" );
		fprintf( out, "  SET( FIRST_GCC_PROGRAM_SOURCE $""{SACK_BASE}/src/sack/deadstart_list.c )\n" );
		fprintf( out, "  SET( LAST_GCC_LIBRARY_SOURCE $""{SACK_BASE}/src/sack/deadstart_lib.c $""{SACK_BASE}/src/sack/deadstart_end.c )\n" );
		fprintf( out, "  SET( LAST_GCC_PROGRAM_SOURCE $""{SACK_BASE}/src/sack/deadstart_lib.c $""{SACK_BASE}/src/sack/deadstart_prog.c $""{SACK_BASE}/src/sack/deadstart_end.c )\n" );
		fprintf( out, "  endif()\n" );
		fprintf( out, "\n" );
		fprintf( out, "if( MSVC OR WATCOM )\n" );
		fprintf( out, "  SET( LAST_GCC_PROGRAM_SOURCE $""{SACK_BASE}/src/sack/deadstart_prog.c )\n" );
		fprintf( out, "endif( MSVC OR WATCOM )\n" );
		fprintf( out, "\n" );
		fprintf( out, "SET( DATA_INSTALL_PREFIX resources )\n" );
		fprintf( out, "include( $""{SACK_BASE}/${CMAKE_BUILD_TYPE}/DefaultInstall.cmake )\n" );
		fprintf( out, "macro( INSTALL_SACK dest )\n" );
		fprintf( out, "\n" );
		fprintf( out, "if(SUPPORTS_PARALLEL_BUILD_TYPE)\n" );
		fprintf( out, "install( FILES $""{SACK_BASE}/\\$""{CMAKE_INSTALL_CONFIG_NAME}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}${SACK_BAG_PLUSPLUS}${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION $""{dest} )\n" );
		fprintf( out, "install( FILES $""{SACK_BASE}/\\$""{CMAKE_INSTALL_CONFIG_NAME}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}sack_bag${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION $""{dest} )\n" );
		fprintf( out, "install( FILES $""{SACK_BASE}/\\$""{CMAKE_INSTALL_CONFIG_NAME}/bin/edit_options${CMAKE_EXECUTABLE_SUFFIX} DESTINATION $""{dest} )\n" );
		fprintf( out, "else(SUPPORTS_PARALLEL_BUILD_TYPE)\n" );
		fprintf( out, "install( FILES $""{SACK_BASE}/$""{CMAKE_BUILD_TYPE}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}${SACK_BAG_PLUSPLUS}${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION $""{dest} )\n" );
		fprintf( out, "install( FILES $""{SACK_BASE}/$""{CMAKE_BUILD_TYPE}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}sack_bag${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION $""{dest} )\n" );
		fprintf( out, "install( FILES $""{SACK_BASE}/$""{CMAKE_BUILD_TYPE}/bin/edit_options${CMAKE_EXECUTABLE_SUFFIX} DESTINATION $""{dest} )\n" );
		fprintf( out, "endif(SUPPORTS_PARALLEL_BUILD_TYPE)\n" );
		fprintf( out, "ENDMACRO( INSTALL_SACK )\n" );
		fprintf( out, "\n" );

		//fprintf( out, "IF(CMAKE_BUILD_TPYE_INITIALIZED_TO_DEFAULT)\n" );

		fprintf( out, "set(CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\" CACHE STRING \"Set build type\")\n" );
		fprintf( out, "set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)\n" );
                
		//fprintf( out, "ENDIF(CMAKE_BUILD_TPYE_INITIALIZED_TO_DEFAULT)\n" );

		fprintf( out, "\n" );

		fclose( out );
	}
	return 0;
}
