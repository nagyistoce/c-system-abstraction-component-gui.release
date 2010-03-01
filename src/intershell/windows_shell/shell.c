
#include <stdhdrs.h>
#include "../intershell_registry.h"
#include "../widgets/include/buttons.h"


// file associations can be done via
//   administrative privilege cmd.exe
//
//C:\Windows\system32>assoc .autoconfigbackup2=intershellbackup
//.autoconfigbackup2=intershellbackup
//
//C:\Windows\system32>assoc .autoconfigbackup3=intershellbackup
//.autoconfigbackup3=intershellbackup
//
//C:\Windows\system32>ftype intershellbackup=c:\ftn3000\bin32\intershell\intershell.exe -restore -SQL %1
//intershellbackup=c:\ftn3000\bin32\intershell\intershell.exe -restore -SQL %1


//C:\Windows\system32>ftype /?
//Displays or modifies file types used in file extension associations
//
//FTYPE [fileType[=[openCommandString]]]
//
//  fileType  Specifies the file type to examine or change
//  openCommandString Specifies the open command to use when launching files
//                    of this type.
//
//Type FTYPE without parameters to display the current file types that
//have open command strings defined.  FTYPE is invoked with just a file
//type, it displays the current open command string for that file type.
//Specify nothing for the open command string and the FTYPE command will
//delete the open command string for the file type.  Within an open
//command string %0 or %1 are substituted with the file name being
//launched through the assocation.  %* gets all the parameters and %2
//gets the 1st parameter, %3 the second, etc.  %~n gets all the remaining
//parameters starting with the nth parameter, where n may be between 2 and 9,
//inclusive.  For example:
//
//    ASSOC .pl=PerlScript
//    FTYPE PerlScript=perl.exe %1 %*
//
//would allow you to invoke a Perl script as follows:
//
//    script.pl 1 2 3
//
//If you want to eliminate the need to type the extensions, then do the
//following:
//
//    set PATHEXT=.pl;%PATHEXT%
//
//and the script could be invoked as follows:
//
//    script 1 2 3
//
//
//
//C:\Windows\system32>ftype intershellbackup=c:\ftn3000\bin32\intershell\intershell.exe -restore -SQL %1
//intershellbackup=c:\ftn3000\bin32\intershell\intershell.exe -restore -SQL %1
//





PRELOAD( AllowWindowsShell )
{
	// Credit to Nicolas Escuder for figuring out how to *Hide the pretteh XP Logon screen*!
	// Credit also to GeoShell 4.11.8 changelog entry
	lprintf( "Credit to Nicolas Escuder for figuring out how to *Hide the pretteh XP Logon screen*!" );
   lprintf( "Credit also to GeoShell version 4.11.8 changelog entry..." );
	// Code borrowed from LiteStep (give it back when we're done using it ^.^).
	{
		HANDLE hLogonEvent = OpenEvent( EVENT_MODIFY_STATE, 1, "Global\\msgina: ShellReadyEvent" );    // also: "Global\msgina: ReturnToWelcome"

		if( !hLogonEvent )
		{
			lprintf( "Error : %d", GetLastError() );
			hLogonEvent = OpenEvent( EVENT_MODIFY_STATE, 1, "msgina: ShellReadyEvent" );    // also: "Global\msgina: ReturnToWelcome"
         if( !hLogonEvent )
				lprintf( "Error : %d", GetLastError() );
		}
		SetEvent( hLogonEvent );
		CloseHandle( hLogonEvent );
	}

	{
		/*
		 * configure InterShell as a sendto handler for certain file types...
		 * configure InterShell as the handler for .config.# files
		 *
		 *  HKEY_CLASSES_ROOT/.config.1
		 *    Content Type=text/plain text/xml application/xml
		 *    PerceivedType
       *   ~/OpenWithList
       *   ~/OpenWithProgids
       *   ~/PersisentHandler

       */
	}
}

OnKeyPressEvent( "Windows Logoff" )( PTRSZVAL psv )
{
	HANDLE hLogonEvent = OpenEvent( EVENT_MODIFY_STATE, 1, "Global\\msgina: ReturnToWelcome" );
	if( !hLogonEvent )
      hLogonEvent = OpenEvent( EVENT_MODIFY_STATE, 1, "msgina: ReturnToWelcome" );
	SetEvent( hLogonEvent );
	CloseHandle( hLogonEvent );
}

OnCreateMenuButton( "Windows Logoff" )( PMENU_BUTTON button )
{
   return 1;
}

#if 0
/* this might be nice to have auto populated start menu
 * based on the original window system stuff... should be easy enough to do...
 */
OnKeyPressEvent( "Windows Logoff" )( PTRSZVAL psv )
{
	PMENU menu;
	menu = CreatePopup();
   /*
    * if( is98 )
	 * BuildMenuItemsPopup( menu, "/users/all users/start menu" );
    * if( isXP )
    * BuildMenuItemsPopup( menu, "/documents and settings/all users/start menu" );
    * if( isVista )
	 * BuildMenuItemsPopup( menu, "/????" );
    */
}

OnCreateMenuButton( "Windows->Start" )( PMENU_BUTTON button )
{
   return 1;
}
#endif

#ifdef __WATCOMC__
PUBLIC( void, ExportThis_POINTS_DOWN )()
{
}
#endif
