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
	fopen_s( &out, "CMakePackage", "wt" );
#else
	out = fopen( "CMakePackage", "wt" );
#endif
	if( out )
	{
		int c;
		for( c = 0; path[c]; c++ )
			if( path[c] == '\\' ) path[c] = '/';
#ifdef WIN32
		SetRegistryItem( HKEY_LOCAL_MACHINE, "SOFTWARE", "\\Freedom Collective\\${CMAKE_PROJECT_NAME}", "Install_Dir", REG_SZ, (BYTE*)path, strlen(path));
#endif
		fprintf( out, "GET_FILENAME_COMPONENT(SACK_SDK_ROOT_PATH \"[HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Freedom Collective\\\\SACK;Install_Dir]\" ABSOLUTE CACHE)\n" );
		fprintf( out, "include( $""{SACK_SDK_ROOT_PATH}/CMakePackage)\n" );

		//fprintf( out, "GET_FILENAME_COMPONENT(INTERSHELL_SDK_ROOT_PATH \"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Freedom Collective\\${CMAKE_PROJECT_NAME};Install_Dir]\" ABSOLUTE CACHE)\n" );

		fprintf( out, "set( INTERSHELL_BASE %s )\n", path );
		fprintf( out, "set( INTERSHELL_INCLUDE_DIR $""{INTERSHELL_BASE}/include/sack )\n" );
		fprintf( out, "set( INTERSHELL_LIBRARIES sack_widgets )\n" );
		fprintf( out, "set( INTERSHELL_LIBRARY_DIR $""{INTERSHELL_BASE}/lib )\n" );

		fprintf( out, "\n" );
		fprintf( out, "FILE(GLOB InterShell_Binaries \"$""{INTERSHELL_BASE}/bin/*.*\" )\n" );
		fprintf( out, "FILE(GLOB InterShell_Plugins \"$""{INTERSHELL_BASE}/bin/plugins/*\" )\n" );
		fprintf( out, "FILE(GLOB InterShell_Resources_fonts \"$""{INTERSHELL_BASE}/resources/fonts/*\" )\n" );
		fprintf( out, "FILE(GLOB InterShell_Resources_frames \"$""{INTERSHELL_BASE}/resources/frames/*\" )\n" );
		fprintf( out, "FILE(GLOB InterShell_Resources_images \"$""{INTERSHELL_BASE}/resources/images/*\" )\n" );
		fprintf( out, "\n" );
		fprintf( out, "macro( INSTALL_INTERSHELL dest )\n" );
		fprintf( out, "install( FILES $""{InterShell_Binaries} DESTINATION $""{dest}/bin )\n" );
		fprintf( out, "install( FILES $""{InterShell_Plugins} DESTINATION $""{dest}/bin/plugins )\n" );
		fprintf( out, "install( FILES $""{InterShell_Resources_fonts} DESTINATION $""{dest}/Resources/fonts )\n" );
		fprintf( out, "install( FILES $""{InterShell_Resources_frames} DESTINATION $""{dest}/Resources/frames )\n" );
		fprintf( out, "install( FILES $""{InterShell_Resources_images} DESTINATION $""{dest}/Resources/images )\n" );
		fprintf( out, "ENDMACRO( INSTALL_INTERSHELL )\n" );
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
