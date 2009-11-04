#define l local_video_common

typedef struct vidlib_local_tag
{
	struct {
		//-------- these should be removed! and moved to comments to save
      // string space! (and related logging statements these protect should be eliminated.)
		BIT_FIELD bLogRegister : 1;
		BIT_FIELD bLogFocus : 1;
		BIT_FIELD bLogWrites : 1; // log when surfaces are written to real space
		BIT_FIELD bThreadCreated : 1;
		BIT_FIELD bPostedInvalidate : 1;
		BIT_FIELD bLogKeyEvent : 1;
		BIT_FIELD bWhatever : 1;
		BIT_FIELD bLayeredWindowDefault : 1;
		BIT_FIELD mouse_on : 1;
		BIT_FIELD bOptimizeHide : 1;
      //---------- see comment above
	} flags;
   PRENDERER mouse_last_vid;
   int mouse_b, mouse_y, mouse_x;
   int _mouse_b, _mouse_y, _mouse_x;

   int WindowBorder_X, WindowBorder_Y;

   ATOM aClass;      // keep reference of window class....
   ATOM aClass2;      // keep reference of window class.... (opengl minimal)

   HANDLE hWndInstance;
   int bCreatedhWndInstance;

// thread synchronization variables...
	unsigned char bThreadRunning, bExitThread;
	PLIST pActiveList;
	PLIST pInactiveList;
	PLIST threads;
	PTHREAD actual_thread; // this is the one that creates windows surfaces...
	DWORD dwThreadID;  // thread that receives events from windows queues...
	DWORD dwEventThreadID; // thread that handles dispatch to application
	VIDEO hDialogVid[16];
	int nControlVid;
	const TEXTCHAR *gpTitle;
	PVIDEO hVideoPool;      // linked list of active windows
	PVIDEO hVidFocused;
	PVIDEO hVidCore; // common, invisible surface behind all windows (application container)
	PVIDEO hCaptured;
	// kbd.key == KeyboardState
	KEYBOARD kbd;
	_32 dwMsgBase;
	//_32 pid;
	//char KeyboardState[256];   // export for key procs to reference...
	PLIST keyhooks;
	PLIST ll_keyhooks;
   CRITICALSECTION csList;
	//HHOOK hKeyHook;
#ifdef __WINDOWS__
	BOOL (WINAPI *UpdateLayeredWindow)(HWND,HDC,POINT*,SIZE*,HDC,POINT*,COLORREF,BLENDFUNCTION*,DWORD);
	BOOL (WINAPI *UpdateLayeredWindowIndirect )(HWND hWnd, const UPDATELAYEREDWINDOWINFO *pULWInfo);
#endif
   _32 last_mouse_update; // last tick the mouse moved.
} LOCAL;



#ifndef VIDLIB_MAIN
extern
#endif
 LOCAL l;
