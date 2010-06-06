#include <stdhdrs.h>

#include <sharemem.h>
#include <filesys.h>
#include <vidlib/vidstruc.h>

int main( int argc, char **argv )
{
   POINTER mem_lock;
   _32 size = 0;
	TEXTCHAR *myname = StrDup( pathrchr( argv[0] ) );
	TEXTCHAR *endname;
   char lockname[256];
	if( myname )
		myname++;
	else
		myname = argv[0];
	endname = (TEXTCHAR*)StrChr( myname, '.' );
	if( endname )
		endname[0] = 0;

	snprintf( lockname, sizeof( lockname ), "%s.instance.lock", myname );
   //lprintf( "Checking lock %s", lockname );
	mem_lock = OpenSpace( lockname
		, NULL
		//, WIDE("memory.delete")
		, &size );
	if( mem_lock )
	{
#ifdef __WINDOWS__
		PRENDER_INTERFACE pri = GetDisplayInterface();
		PVIDEO video = (PVIDEO)mem_lock;
		if( video->hWndOutput )
		{
			ForceDisplayFocus( video );
			//SetForegroundWindow( video->hWndOutput );
			/*
			 {
			 INPUT keys[4];
			 keys[0].type = INPUT_KEYBOARD;
			 keys[0].ki.wVk = VK_MENU;
			 keys[0].ki.wScan = 56;
			 keys[0].ki.dwFlags = 0;
			 keys[0].ki.dwExtraInfo = 0;
			 keys[0].ki.time = 0;

			 keys[1].type = INPUT_KEYBOARD;
			 keys[1].ki.wVk = VK_F4;
			 keys[1].ki.wScan = 56;
			 keys[1].ki.dwFlags = 0;
			 keys[1].ki.dwExtraInfo = 0;
			 keys[0].ki.time = 0;

			 keys[2].type = INPUT_KEYBOARD;
			 keys[2].ki.wVk = VK_F4;
			 keys[2].ki.wScan = 56;
			 keys[2].ki.dwFlags = KEYEVENTF_KEYUP;
			 keys[2].ki.dwExtraInfo = 0;
			 keys[0].ki.time = 0;

			 keys[3].type = INPUT_KEYBOARD;
			 keys[3].ki.wVk = VK_MENU;
			 keys[3].ki.wScan = 56;
			 keys[3].ki.dwFlags = KEYEVENTF_KEYUP;
			 keys[3].ki.dwExtraInfo = 0;
			 keys[0].ki.time = 0;
			 }
			 */

			keybd_event( VK_MENU, 56, 0, 0 );
			keybd_event( VK_F4, 62, 0, 0 );
			keybd_event( VK_F4, 62, KEYEVENTF_KEYUP, 0 );
			keybd_event( VK_MENU, 56, KEYEVENTF_KEYUP, 0 );

#define WM_EXIT_PLEASE 0xd1e
			//if( !SendMessage( video->hWndOutput, WM_QUERYENDSESSION, 0, 0 ) )
			//   lprintf( "Failed to post queyendsession." );
		}
		else
		{
         // region found, but no content...
		}
#endif
	}
	else
      lprintf( "lock region not found." );
   return 0;
}
