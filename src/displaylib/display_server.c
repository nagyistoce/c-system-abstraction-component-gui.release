#include <stdhdrs.h>
#include <sharemem.h>
#include <timers.h>
#include <msgclient.h>
#ifdef __WINDOWS__
#include <systray.h>
#endif
#include "ghdr.h"
//#include <render.h>


typedef struct display_tracking_tag {
	PRENDERER hDisplay;
	_32 pid;
	PTRSZVAL psvClientClose; // for events this is a client side value to mark the display
	PTRSZVAL psvClientRedraw; // for events this is a client side value to mark the display
	PTRSZVAL psvClientMouse; // for events this is a client side value to mark the display
	PTRSZVAL psvClientLoseFocus; // for events this is a client side value to mark the display
	PTRSZVAL psvClientKeyboard; // for events this is a client side value to mark the display
	PTRSZVAL psvClientGeneral; // for events this is a client side value to mark the display
} DISPLAY_TRACKING, *PDISPLAY_TRACKING;

#define MAXDISPLAY_TRACKINGSPERSET 32
DeclareSet( DISPLAY_TRACKING );

typedef struct local_tag
{
#ifdef STANDALONE_SERVICE
	struct {
		_32 bFailed : 1;
	} flags;
#endif
	_32 MsgBase;
	// future change - for validation purposes this should be like
	// a PLIST pDisplays - then the index from the client can
// be validated for range and PID of creature.
   PDISPLAY_TRACKINGSET pDisplays;
	//PDISPLAY_TRACKING pDisplays;
} LOCAL;

static LOCAL l;

static int CPROC ServerSetApplicationTitle( _32 *params, _32 length
											  , _32 *result, _32 *result_length )

{
    lprintf( WIDE("SetApplicationTitle: %s"), (CTEXTSTR)params );
    *result_length = INVALID_INDEX;
    return TRUE;
}

static int CPROC ServerSetRendererTitle( _32 *params, _32 length
											  , _32 *result, _32 *result_length )

{
    lprintf( WIDE("SetRendererTitle: %s"), (CTEXTSTR)(params + 1) );
    *result_length = INVALID_INDEX;
    return TRUE;
}

static PTRSZVAL CPROC CloseIfPid( void *member, PTRSZVAL pid )
{
	PDISPLAY_TRACKING display = (PDISPLAY_TRACKING)member;
	Log3( WIDE("Checking to see if display %p %ld is owned by %ld")
		 , display->hDisplay, display->pid, pid );
	if( display->pid == pid )
	{
	// get this display out of the list
	// this will stop any events
	// from going backwards... err ...
		display->pid = 0;
		//if( ( (*display->me) = display->next ) )
		//	display->next->me = display->me;
									// make sure we don't end up trying to make
									// client messages to noone.
		//Log( WIDE("Setting NULL close handler") );
		SetCloseHandler( display->hDisplay, NULL, 0 );
		//Log1( WIDE("Closing display %p"), display->hDisplay );
		CloseDisplay( display->hDisplay );
		DeleteFromSet( DISPLAY_TRACKING, &l.pDisplays, display );
									//Release( display );
	}
   return 0;
}


static int CPROC DisplayClientClosed( _32 *params, _32 length
										  , _32 *result, _32 *result_length )
{
	ForAllInSet( DISPLAY_TRACKING, l.pDisplays, CloseIfPid, params[-1] );
   *result_length = 0;
   return TRUE;
}

static int CPROC ServerDisplayUnload( _32 *params, _32 length
										, _32 *result, _32 *result_length )
{
// no data is passed...
// only if final client is exiting...
   //_32 client = params[-1];
	extern void CPROC EndDisplay( void );
	// calls SDL_Quit .. uhmm...
   // should release all known data also...
	EndDisplay();
   *result_length = 0;
   return TRUE;
}

static int CPROC ServerGetDisplaySize( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length )
{
	GetDisplaySize( result, result+1 );
   *result_length = 8;
   return TRUE;
}

static int CPROC ServerSetDisplaySize( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length )
{
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerOpenDisplaySizedAt( _32 *params, _32 param_length
												, _32 *result, _32 *result_length )
{
	//PDISPLAY_TRACKING display = Allocate( sizeof( DISPLAY_TRACKING ) );
	PDISPLAY_TRACKING display = GetFromSet( DISPLAY_TRACKING, &l.pDisplays );
	//if( ( display->next = l.pDisplays ) )
   //   l.pDisplays->me = &display->next;
	//l.pDisplays = display;
   display->pid = params[-1];  // this is the pid of the client.
	display->hDisplay = OpenDisplaySizedAt( params[0], params[1], params[2], params[3], params[4] );
	//Log5( WIDE("Resulting display: %p %d %d %d %d")
	//	 , display->hDisplay
	//	 ,  display->hDisplay->common.x
	//	 , display->hDisplay->common.y
	//	 , display->hDisplay->common.width
	//	 , display->hDisplay->common.height
	//	 );
	result[0] = GetMemberIndex( DISPLAY_TRACKING, &l.pDisplays, display );
   result[1] = display->hDisplay->common.x;
   result[2] = display->hDisplay->common.y;
   result[3] = display->hDisplay->common.width;
   result[4] = display->hDisplay->common.height;
   *result_length = 20;
   return TRUE;
}


PDISPLAY_TRACKING GetTrackedDisplay( _32 ID )
{
	if( ID == 0xFACEBEAD )
	{
      lprintf( WIDE("Application is requesting a display from released memory") );
      return NULL;
	}
	if( ID == 0xDEADBEEF )
	{
      lprintf( WIDE("Application is requesting a display from uninitialized memory") );
      return NULL;
	}
	if( ID == INVALID_INDEX )
	{
		lprintf( WIDE("Application is requesting an invalid display handle") );
      return NULL;
	}
   return GetUsedSetMember( DISPLAY_TRACKING, &l.pDisplays, ID );
}

static int CPROC ServerOpenDisplayAboveSizedAt( _32 *params, _32 param_length
													  , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetFromSet( DISPLAY_TRACKING, &l.pDisplays );
	PDISPLAY_TRACKING over_display = GetTrackedDisplay( params[5] );;
	//if( ( display->next = l.pDisplays ) )
   //   l.pDisplays->me = &display->next;
	//display->me = &l.pDisplays;
	//l.pDisplays = display;
   display->pid = params[-1];  // this is the pid of the client.
	display->hDisplay = OpenDisplayAboveSizedAt( params[0], params[1], params[2], params[3], params[4], over_display?over_display->hDisplay:NULL );
	//Log5( WIDE("Resulting display: %p %d %d %d %d")
	//	 , display->hDisplay
	//	 ,  display->hDisplay->common.x
	//	 , display->hDisplay->common.y
	//	 , display->hDisplay->common.width
	//	 , display->hDisplay->common.height
	//	 );
   result[0] = GetMemberIndex( DISPLAY_TRACKING, &l.pDisplays, display );
   result[1] = display->hDisplay->common.x;
   result[2] = display->hDisplay->common.y;
   result[3] = display->hDisplay->common.width;
   result[4] = display->hDisplay->common.height;
   *result_length = 20;
   return TRUE;
}

static int CPROC ServerCloseDisplay( _32 *params, _32 param_length
										, _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay(  params[0] );
	// stop tracking this display - so that it will not timeout to the client.
									  //  Log( WIDE("Client closing a display!!!!!!") );
	if( display )
	{
		if( display->psvClientClose )
			SetCloseHandler( display->hDisplay, NULL, 0 );
		if( display->psvClientRedraw )
			SetRedrawHandler( display->hDisplay, NULL, 0 );
		if( display->psvClientMouse )
			SetMouseHandler( display->hDisplay, NULL, 0 );
		if( display->psvClientLoseFocus )
			SetLoseFocusHandler( display->hDisplay, NULL, 0 );
		if( display->psvClientKeyboard )
			SetKeyboardHandler( display->hDisplay, NULL, 0 );
		if( display->psvClientGeneral )
			SetDefaultHandler( display->hDisplay, NULL, 0 );
		CloseDisplay( display->hDisplay );
	//Log( WIDE("Found the display - closing tracking structure.") );
		DeleteFromSet( DISPLAY_TRACKING, &l.pDisplays, display );
	}
	else
	{
      lprintf( WIDE("Already closed.") );
	}
	//Release( display );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerUpdateDisplayPortionEx( _32 *params, _32 param_length
												  , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );
	if( display )
	{
#ifdef _DEBUG
		UpdateDisplayPortionEx( display->hDisplay, params[1], params[2], params[3], params[4], (CTEXTSTR)params+6, params[5]  );
#else
		UpdateDisplayPortionEx( display->hDisplay, params[1], params[2], params[3], params[4]);
#endif
	}
   *result_length = 0;
   return TRUE;
}

static int CPROC ServerUpdateDisplay( _32 *params, _32 param_length
										 , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay(  params[0] );;
   if( display )
		UpdateDisplay( display->hDisplay );
   *result_length = 0;
   return TRUE;
}

static int CPROC ServerClearDisplay( _32 *params, _32 param_length
										, _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay(  params[0] );;
   if( display )
		ClearDisplay( display->hDisplay );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerMoveDisplay( _32 *params, _32 param_length
									  , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
   if( display )
		MoveDisplay( display->hDisplay, params[1], params[2] );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerMoveDisplayRel( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
   if( display )
		MoveDisplayRel( display->hDisplay, params[1], params[2] );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerSizeDisplay( _32 *params, _32 param_length
									  , _32 *result, _32 *result_length )
{
    PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
    lprintf( WIDE("Client request resize of %p(%p) %d,%d")
            , (POINTER)params[0]
            , display
            , params[1]
            , params[2] );
    if( display )
        SizeDisplay( display->hDisplay, params[1], params[2] );
    *result_length = INVALID_INDEX;
    return TRUE;
}

static int CPROC ServerSizeDisplayRel( _32 *params, _32 param_length
									    , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
   if( display )
		SizeDisplayRel( display->hDisplay, params[1], params[2] );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerMoveSizeDisplayRel( _32 *params, _32 param_length
									  , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );
   if( display )
		MoveSizeDisplayRel( display->hDisplay
								, params[1], params[2]
								, params[3], params[4] );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerMoveSizeDisplay( _32 *params, _32 param_length
									  , _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
   if( display )
		MoveSizeDisplay( display->hDisplay
							, params[1], params[2]
							, params[3], params[4] );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerPutDisplayAbove( _32 *params, _32 param_length
											, _32 *result, _32 *result_length )
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
	PDISPLAY_TRACKING display2 = GetTrackedDisplay( params[1] );;
   PutDisplayAbove( display->hDisplay, display2->hDisplay );
   *result_length = INVALID_INDEX;
   return TRUE;
}

RENDER_NAMESPACE
    _32 CPROC AddTrackedImage( _32 pid, Image image, _32 psv );
RENDER_NAMESPACE_END

static int CPROC ServerGetDisplayImage( _32 *params, _32 param_length
											, _32 *result, _32 *result_length )
{
    PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
    PPANEL panel = display->hDisplay;
    _32 image = AddTrackedImage( params[-1]
										 , panel->common.RealImage
										 , 0 );
    Image _image = panel->common.RealImage;
    Log1( WIDE("Resulting image: %p"), image );
    result[0] = _image->x;
    result[1] = _image->y;
    result[2] = _image->width;
    result[3] = _image->height;
    result[4] = (_32)image;
    *result_length = 20;
    return TRUE;
}

static int CPROC ServerGetMousePosition( _32 *params, _32 param_length
											 , _32 *result, _32 *result_length )
{
	GetMousePosition( (PS_32)result, (PS_32)(result + 1) );
	*result_length = 8;
   return TRUE;
}

static int CPROC ServerGetMouseState( _32 *params, _32 param_length
											 , _32 *result, _32 *result_length )
{
	GetMouseState( (PS_32)result, (PS_32)(result + 1), (P_32)(result + 2) );
	*result_length = 12;
   return TRUE;
}


//#define ParamLength( first, last ) ( ((char*)((&(last))+1)) - ((char*)(&(first))) )

static int CPROC CPROC MouseMethod( PTRSZVAL psvUser, S_32 x, S_32 y, _32 b )
{
	PDISPLAY_TRACKING display = (PDISPLAY_TRACKING)psvUser;
	psvUser = display->psvClientMouse;
   // rebias the x and y to screen coordinates to send to client...
	x += display->hDisplay->common.x;
   y += display->hDisplay->common.y;
	//lprintf( WIDE("Sending event to %d %p %ld %ld %ld"), display->pid, (POINTER)psvUser, x, y, b );
   if( b & MK_OBUTTON )
		SendMultiServiceEvent( display->pid
								   , l.MsgBase + MSG_MouseMethod
							 	   , 2
							 	   , (P_32)&psvUser, ParamLength( psvUser, b )
								  , (P_32)display->hDisplay->KeyboardState
								        , sizeof( display->hDisplay->KeyboardState )
									);
   else
		SendServiceEvent( display->pid
							 , l.MsgBase + MSG_MouseMethod
						    , (P_32)&psvUser, ParamLength( psvUser, b )
							 );
	return TRUE;
}

static int CPROC ServerSetMouseHandler( _32 *params, _32 param_length
								  , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display =
		GetTrackedDisplay( params[0] );

	//Log1( WIDE("Setting mouse method :%p"), (POINTER)params[1] );
	SetMouseHandler( (PRENDERER)display->hDisplay
						, MouseMethod
						, (PTRSZVAL)display );
   display->psvClientMouse = (PTRSZVAL)params[1];
	*result_length = INVALID_INDEX;
   return TRUE;
}

static void CPROC CloseMethod( PTRSZVAL psvUser )
{
	PDISPLAY_TRACKING display = (PDISPLAY_TRACKING)psvUser;
   psvUser = display->psvClientClose;
	SendServiceEvent( display->pid, l.MsgBase + MSG_CloseMethod
						 , (P_32)&psvUser, ParamLength( psvUser, psvUser ) );
}


static int CPROC ServerSetCloseHandler( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display =
		GetTrackedDisplay( params[0] );
									  //PDISPLAY_TRACKING display;
	if( display )
	{
		SetCloseHandler( display->hDisplay, CloseMethod, (PTRSZVAL)display );
		display->psvClientClose = (PTRSZVAL)params[1];
	}
   *result_length = INVALID_INDEX;
   return TRUE;
}

static void CPROC RedrawMethod( PTRSZVAL psvUser, PRENDERER psvSelf )
{
	PDISPLAY_TRACKING display = (PDISPLAY_TRACKING)psvUser;
#ifdef DEBUG_REDRAW
	lprintf( WIDE("Generating redraw event to client : %p %p %ld %d %d %d %d")
       , (POINTER)display
		 , (POINTER)display->psvClientRedraw
       , display->pid
		 , psvSelf->common.RealImage->x
		 , psvSelf->common.RealImage->y
		 , psvSelf->common.RealImage->width
		 , psvSelf->common.RealImage->height
			 );
#endif
	psvUser = display->psvClientRedraw;
	SendMultiServiceEvent( display->pid, l.MsgBase + MSG_RedrawMethod
								, 3
								, (P_32)&psvUser, sizeof( psvUser )
								, (P_32)&psvSelf, sizeof( psvSelf )
								, psvSelf->common.RealImage, sizeof( IMAGE_RECTANGLE )
								);
	// and then we have to wait for a responce that the client
   // has finished processing this event.
}


static int CPROC ServerSetRedrawHandler( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display =
		GetTrackedDisplay( params[0] );
	if( display )
	{
	//PDISPLAY_TRACKING display;
   //display = FindDisplay( (PRENDERER)params[0] );
	display->psvClientRedraw = (PTRSZVAL)params[1];
   //Log1( WIDE("Set redraw method (data=%p)"), display );
	SetRedrawHandler( display->hDisplay
						 , RedrawMethod
						 , (PTRSZVAL)display );
	}
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC KeyboardMethod( PTRSZVAL psvUser, _32 keycode )
{
	PDISPLAY_TRACKING display = (PDISPLAY_TRACKING)psvUser;
	//Log1( WIDE("Generating key event: %08lx"), keycode );
	psvUser = display->psvClientKeyboard;
	SendMultiServiceEvent( display->pid, l.MsgBase + MSG_KeyMethod
								, 2
								, (P_32)&psvUser, ParamLength( psvUser, keycode )
                              , (P_32)display->hDisplay->KeyboardState, sizeof( display->hDisplay->KeyboardState ) );
        lprintf( "Need to transact this I guess to get the real result." );
        return 1;
}


static int CPROC ServerSetKeyboardHandler( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display =
		GetTrackedDisplay( params[0] );
	if( display )
	{
   //PDISPLAY_TRACKING display;
	SetKeyboardHandler( display->hDisplay, KeyboardMethod
							, (PTRSZVAL)display );
	display->psvClientKeyboard = (PTRSZVAL)params[1];
	}
   *result_length = INVALID_INDEX;
   return TRUE;
}

static void CPROC LoseFocusMethod( PTRSZVAL psvUser, PRENDERER pGain )
{
	PDISPLAY_TRACKING display = (PDISPLAY_TRACKING)psvUser;
	psvUser = display->psvClientLoseFocus;

   //Log3( WIDE("Dispatch focus event %d %p %p"), display->pid, psvUser, pGain );
	SendServiceEvent( display->pid, l.MsgBase + MSG_LoseFocusMethod
						 , (P_32)&psvUser, ParamLength( psvUser, pGain ) );
}


static int CPROC ServerSetLoseFocusHandler( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display =
		GetTrackedDisplay( params[0] );
	if( display )
	{
	display->psvClientLoseFocus = (PTRSZVAL)params[1];
	//Log1( WIDE("Enabling lose focus handler to event dispatch. %p")
	//	 , params[1] );
	SetLoseFocusHandler( display->hDisplay, LoseFocusMethod
							 , (PTRSZVAL)display );
	}
	//Log2( WIDE("Enabling lose focus handler to event dispatch. %p %p")
	//	 , params[1]
	//	 , display );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static void CPROC GeneralMethod( PTRSZVAL psvUser, PRENDERER image, PACTIVEMESSAGE msg )
{
	PDISPLAY_TRACKING display = (PDISPLAY_TRACKING)psvUser;
	psvUser = display->psvClientGeneral;
   // needs a multi-meeds type call - to handle the active message content.
	SendMultiServiceEvent( display->pid, l.MsgBase + MSG_GeneralMethod
                        , 1
								, (P_32)&psvUser, ParamLength( psvUser, image ) );
}


static int CPROC ServerSetGeneralHandler( _32 *params, _32 param_length
										  , _32 *result, _32 *result_length)
{
   //PDISPLAY_TRACKING display;
	PDISPLAY_TRACKING display =
		GetTrackedDisplay( params[0] );
	if( display )
	{
		SetDefaultHandler( display->hDisplay, GeneralMethod, (PTRSZVAL)(display ) );
		display->psvClientGeneral = (PTRSZVAL)params[1];
	}
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerSetMousePosition( _32 *params, _32 param_length
											, _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );
   if( display )
   //Log3( WIDE("SetMousePosition %p %d,%d"), params[0], params[1], params[2] );
	SetMousePosition( display->hDisplay, params[1], params[2] );
   //Log( WIDE("Returned...") );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerOwnMouse( _32 *params, _32 param_length
								 , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display = GetTrackedDisplay(  params[0] );
	if( display )
		OwnMouse( display->hDisplay, params[1] );
	*result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerBeginCalibrate( _32 *params, _32 param_length
										 , _32 *result, _32 *result_length)
{
	BeginCalibration( ((int*)params)[0] );
   *result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerSyncRender( _32 *params, _32 param_length
									, _32 *result, _32 *result_length)
{
	// should do something to validate
	// that all operations ssuch aas move mouse, updaate display
	// etc are complete (queued to yet another thread
	PDISPLAY_TRACKING display = GetTrackedDisplay(  params[0] );
	// update all dirty panels, which are now marked clean
   // and are displayed.
	if( display )
	{
		lprintf( WIDE("SyncRender") );
		UpdateDisplay( display->hDisplay );
#ifdef __LINUX__
		{
			SDL_Event event;
			while( SDL_PeepEvents( &event, 1, SDL_PEEKEVENT, 0xFFFFFFFF ) )
				Relinquish();
		}
#endif
	}
	*result_length = 0;
   return TRUE;
}

static int CPROC ServerOkaySyncRender( _32 *params, _32 param_length
									, _32 *result, _32 *result_length)
{
	// should do something to validate
	// that all operations ssuch aas move mouse, updaate display
	// etc are complete (queued to yet another thread
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );
	// update all dirty panels, which are now marked clean
   // and are displayed.
	if( !display )
	{
	   // return failure to sync...
	   *result_length = INVALID_INDEX;
		return FALSE;
	}
   //lprintf( WIDE("------------- SyncRender") );
   UpdateDisplay( display->hDisplay );
#ifdef __LINUX__
	{
      SDL_Event event;
		while( SDL_PeepEvents( &event, 1, SDL_PEEKEVENT, 0xFFFFFFFF ) )
			Relinquish();
	}
#endif
	*result_length = INVALID_INDEX;
   return TRUE;
}


static int CPROC ServerHideDisplay( _32 *params, _32 param_length
								  , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );
   if( display )
		HideDisplay( display->hDisplay );
	*result_length = INVALID_INDEX;
   return TRUE;
}

static int CPROC ServerRestoreDisplay( _32 *params, _32 param_length
								  , _32 *result, _32 *result_length)
{
	PDISPLAY_TRACKING display = GetTrackedDisplay( params[0] );;
   if( display )
		RestoreDisplay( display->hDisplay );
	*result_length = INVALID_INDEX;
   return TRUE;
}


static int CPROC DoNothing( _32 *params, _32 param_length
						  , _32 *result, _32 *result_length)
{
	*result_length = 0;
   return FALSE;
}

static int CPROC ServerIsTouchDisplay( _32 *params, _32 param_length
												 , _32 *result, _32 *result_length)
{
   result[0] = IsTouchDisplay();
	*result_length = 4;
   return FALSE;
}

#define NUM_FUNCTIONS (sizeof(MyMessageHandlerTable)/sizeof( server_function))

// this table MUST be in the same order as RENDER_INTERFACE (struct render_interface_tag)
// the message IDs are based on the offset  of the same function names within this struct
static SERVER_FUNCTION MyMessageHandlerTable[] = {
#ifdef GCC
#ifndef __cplusplus
    [MSG_ServiceUnload] =
#endif
#endif
		ServerFunctionEntry( DisplayClientClosed ),
		// these NULLS would be better skipped with C99 array init - please do look
		// that up and implement here.
#ifdef GCC
#ifndef __cplusplus
                [MSG_ServiceLoad] =
#endif
#endif
		{0},
		{0},
		{0},
#ifdef GCC
#ifndef __cplusplus
                [MSG_EventUser] =
#endif
#endif
		ServerFunctionEntry( DoNothing )

, ServerFunctionEntry( ServerSetApplicationTitle )
, ServerFunctionEntry( DoNothing )
, ServerFunctionEntry( ServerGetDisplaySize )
, ServerFunctionEntry( ServerSetDisplaySize )
, ServerFunctionEntry( DoNothing ) // this is ProcessDisplayMessages() - will never be activated client-side.
, ServerFunctionEntry( ServerOpenDisplaySizedAt )
, ServerFunctionEntry( ServerOpenDisplayAboveSizedAt )
, ServerFunctionEntry( ServerCloseDisplay )
, ServerFunctionEntry( ServerUpdateDisplayPortionEx )
, ServerFunctionEntry( ServerUpdateDisplay )
, ServerFunctionEntry( ServerClearDisplay )
, ServerFunctionEntry( DoNothing )
, ServerFunctionEntry( ServerMoveDisplay )
, ServerFunctionEntry( ServerMoveDisplayRel )
, ServerFunctionEntry( ServerSizeDisplay )
, ServerFunctionEntry( ServerSizeDisplayRel )
, ServerFunctionEntry( ServerMoveSizeDisplayRel )
, ServerFunctionEntry( ServerPutDisplayAbove )
, ServerFunctionEntry( ServerGetDisplayImage )
, ServerFunctionEntry( ServerSetCloseHandler )
, ServerFunctionEntry( ServerSetMouseHandler )
, ServerFunctionEntry( ServerSetRedrawHandler )
, ServerFunctionEntry( ServerSetKeyboardHandler )
, ServerFunctionEntry( ServerSetLoseFocusHandler )
, ServerFunctionEntry( ServerSetGeneralHandler )
, ServerFunctionEntry( ServerGetMousePosition )
, ServerFunctionEntry( ServerSetMousePosition ) // ServerSetMousePosition
// actually these should be fowarded here (with no responce)
// then we can provide the override locally to forward events
// to the appropriate pid.
, ServerFunctionEntry( DoNothing ) // ServerHasFocus
, ServerFunctionEntry( DoNothing )  // send active message
, ServerFunctionEntry( DoNothing )  // create active message
, ServerFunctionEntry( DoNothing ) // get key text
, ServerFunctionEntry( DoNothing ) // IsKeyDown
, ServerFunctionEntry( DoNothing ) // KeyDown
, ServerFunctionEntry( DoNothing ) // DisplayIsValid
																, ServerFunctionEntry( ServerOwnMouse )
																, ServerFunctionEntry( ServerBeginCalibrate )
																, ServerFunctionEntry( ServerSyncRender )
																, ServerFunctionEntry( DoNothing ) // EnableGL
																, ServerFunctionEntry( DoNothing ) // SetGLDisplay
																, ServerFunctionEntry( ServerMoveSizeDisplay ) // MoveSizeDisplay
							 , ServerFunctionEntry( DoNothing ) // MakeTopMost
							 , ServerFunctionEntry( ServerHideDisplay )
							 , ServerFunctionEntry( ServerRestoreDisplay )
							 , ServerFunctionEntry( DoNothing ) // force focus
							 , ServerFunctionEntry( DoNothing ) // ForceDIsplayFront
							 , ServerFunctionEntry( DoNothing ) // ForceDisplayBack
                      , ServerFunctionEntry( DoNothing ) // BindKey ) // client side only
                      , ServerFunctionEntry( DoNothing ) // UnBindKey ) // client side only
                      , ServerFunctionEntry( DoNothing ) // IsTopmost ) //
																 , ServerFunctionEntry( ServerOkaySyncRender )
																 , ServerFunctionEntry( ServerIsTouchDisplay )
																 , ServerFunctionEntry( ServerGetMouseState )
, ServerFunctionEntry( ServerSetRendererTitle )






};

#ifndef STANDALONE_SERVICE
static int CPROC GetDisplayFunctionTable( server_function_table *table, int *entries, _32 MsgBase )
{
	*table = MyMessageHandlerTable;
	*entries = NUM_FUNCTIONS;
	// good as a on_entry procedure.
	l.MsgBase = MsgBase;
   Log( WIDE("Resulting load to server...") );
   return InitMemory();
}
#endif

PRELOAD( RegisterDisplayService )
{
#ifdef STANDALONE_SERVICE
	lprintf( WIDE("Register display service.") );
	l.MsgBase = RegisterService( WIDE("display"), MyMessageHandlerTable, NUM_FUNCTIONS );
	if( !l.MsgBase )
	{
      printf( WIDE("Failed to register display service...\n") );
		l.flags.bFailed = 1;
      exit(0);
      return;
	}
#else
	//RegisterFunction( WIDE("system/interfaces/msg_service"), GetDisplayFunctionTable
	//					 , WIDE("int"), WIDE("display"), WIDE("(server_function_table*,int*,_32)") );
	{
		extern LOGICAL ImageRegisteredOkay( void );
		if( ImageRegisteredOkay() )
		{
			//RegisterClassAlias( WIDE("system/interfaces/real_image"), WIDE("system/interfaces/image") );
			//RegisterClassAlias( WIDE("system/interfaces/display"), WIDE("system/interfaces/render") );
		   //DumpRegisteredNames();
			lprintf( WIDE("Leaving display_service...") );
		}
		else
			printf( WIDE("Please run the message server service first...\n") );
	}
#endif
}

#ifdef __LINUX__
int IsDisplayLoadedOkay( void )
{
   return !l.flags.bFailed;
}
#endif

//#endif
// $Log: display_server.c,v $
// Revision 1.65  2005/06/24 15:11:46  jim
// Merge with branch DisplayPerformance, also a fix for watcom compilation
//
// Revision 1.64.2.2  2005/06/22 23:12:10  jim
// checkpoint...
//
// Revision 1.64.2.1  2005/06/22 19:49:32  jim
// Fix (somehow) the display update issue...
//
// Revision 1.64  2005/06/06 16:23:59  jim
// Attach default systray icon to service under windows.
//
// Revision 1.45  2005/06/05 05:27:01  d3x0r
// Make default runmode windows for display_service under ... windows... add default icon handler which gives us exit.
//
// Revision 1.44  2005/05/31 21:57:00  d3x0r
// Progress optimizing redraws... still have several issues...
//
// Revision 1.43  2005/05/25 16:50:12  d3x0r
// Synch with working repository.
//
// Revision 1.62  2005/05/25 15:51:49  jim
// Removed breakpoint
//
// Revision 1.61  2005/05/23 21:57:57  jim
// Updates to newest message service.  Also abstraction of images to indexes instead of PIMAGE pointers in messages...
//
// Revision 1.60  2005/05/18 23:45:17  jim
// Fix font transport... begin validation code for valid display things given from the client... ... need to do more checking all over.
//
// Revision 1.59  2005/05/17 18:35:56  jim
// Fix disconnection of a client.  Remove noisy logging.
//
// Revision 1.58  2005/05/16 17:20:05  jim
// Remove noiys logging. Fix the result of disconnects so no data is transferred.
//
// Revision 1.57  2005/05/13 00:38:18  jim
// Update CPROC specifier for service functions for watcom build.
//
// Revision 1.56  2005/05/12 23:08:17  jim
// Remove much noisy logging.
//
// Revision 1.55  2005/05/12 21:04:42  jim
// Fixed several conflicts that resulted in various deadlocks.  Also endeavored to clean all warnings.
//
// Revision 1.54  2005/05/06 21:35:46  jim
// Add support to make display service a stand along service also.
//
// Revision 1.53  2005/04/25 17:33:55  jim
// Disallow redundant updates from applications while in a dispatched redraw - which will itself result in a display update.
//
// Revision 1.52  2005/04/25 16:08:00  jim
// On sync actually do an output... also removed a couple noisy messages.
//
// Revision 1.51  2005/04/20 23:46:45  jim
// Fix some minor compiler issues with watcom
//
// Revision 1.50  2005/04/19 20:58:53  jim
// Localize local glboal service data.
//
// Revision 1.49  2005/04/19 20:58:08  jim
// Localize local glboal service data.
//
// Revision 1.48  2005/04/19 20:49:02  jim
// Abstract display render service to indexes through the message service
//
// Revision 1.47  2005/04/18 15:53:45  jim
// Fix some linking issues with service.
//
// Revision 1.46  2005/03/23 09:02:42  panther
// Make getdisplayfunctable private
//
// Revision 1.45  2005/03/18 20:21:10  panther
// Fix log4 warnign
//
// Revision 1.44  2005/03/17 02:23:53  panther
// Checkpoint - working on message server abstraction interface... some of this seems to work quite well, some of this is still broken very badly...
//
// Revision 1.43  2005/03/16 22:28:26  panther
// Updates for extended interface definitions... Fixes some issues with client/server display usage...
//
// Revision 1.42  2005/01/24 21:37:27  panther
// Fix client/server linkages, export registrations for procreg
//
// Revision 1.41  2003/12/04 10:40:51  panther
// Add to sync a more definitive sync
//
// Revision 1.40  2003/10/28 01:14:34  panther
// many changes to implement msgsvr on windows.  Even to get displaylib service to build, there's all sorts of errors in inconsistant definitions...
//
// Revision 1.39  2003/09/29 13:19:27  panther
// Fix MSG_RestoreDisplay, Implement client/server hooks for hide/restore display
//
// Revision 1.38  2003/09/21 20:32:41  panther
// Sheet control nearly functional.
// More logging on Orphan/Adoption path.
//
// Revision 1.37  2003/09/21 11:49:04  panther
// Fix service refernce counting for unload.  Fix Making sub images hidden.
// Fix Linking of services on server side....
// cleanup some comments...
//
// Revision 1.36  2003/09/18 07:42:47  panther
// Changes all across the board...image support, display support, controls editable in psi...
//
// Revision 1.35  2003/08/27 15:25:17  panther
// Implement key strokes through mouse callbacks.  And provide image/render sync point method.  Also assign update display portion to be sync'd
//
// Revision 1.34  2003/08/13 16:10:24  panther
// Fix debug transport of UpdateDisplayPortionEx
//
// Revision 1.33  2003/08/01 07:56:12  panther
// Commit changes for logging...
//
// Revision 1.32  2003/06/24 11:44:37  panther
// Okay disable compiling service under windows... apparently compiles under windows now - not sure about usability
//
// Revision 1.31  2003/06/15 22:34:44  panther
// Added idle routine handler... for registering StandardNetworkProc, ProcessDIsplayMessages etc...
//
// Revision 1.30  2003/04/24 16:32:50  panther
// Initial calibration support in display (incomplete)...
// mods to support more features in controls... (returns set interface)
// Added close service support to display_server and display_image_server
//
// Revision 1.29  2003/03/31 22:56:14  panther
// extend makesystem to basically support distclean.  Handle project makes better.
//
// Revision 1.28  2003/03/31 10:55:09  panther
// Removed much logging.  Fixed closing, introduced locks to spacetree
//
// Revision 1.27  2003/03/31 01:11:28  panther
// Tweaks to work better under service application
//
// Revision 1.26  2003/03/29 04:12:10  panther
// Debugging draw/focus mathods
//
// Revision 1.25  2003/03/27 15:26:12  panther
// Add Service Unloading in message service layer - still need a service notification message
//
// Revision 1.24  2003/03/27 11:49:40  panther
// Implement OwnMouse available to client applications
//
// Revision 1.23  2003/03/27 10:50:59  panther
// Display - enable resize that works.  Image - remove hline failed message.  Display - Removed some logging messages.
//
// Revision 1.22  2003/03/26 01:48:50  panther
// Enable key repeat.  Remove noisy key messages since they seem to be working
//
// Revision 1.21  2003/03/26 00:58:11  panther
// Implement SizeDisplayRel and MoveSizeDisplayrel.  Bias mouse coordinates to screen coords between client and server
//
// Revision 1.20  2003/03/25 15:23:25  panther
// Implement GetMousePosition...
//
// Revision 1.19  2003/03/25 08:45:50  panther
// Added CVS logging tag
//
