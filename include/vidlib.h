//#pragma message ( WIDE("include vidlib") )

#ifndef VIDLIB2_INCLUDED
#define VIDLIB2_INCLUDED

#include "keybrd.h" // required for keyboard support ever....
#include <sack_types.h>
#include <render.h>

#ifdef _WIN32
#include <windows.h>
#endif


#define WM_RUALIVE 5000 // lparam = pointer to alive variable expected to set true

#if !defined(__STATIC__) && !defined( __UNIX__ )
#ifdef VIDEO_LIBRARY_SOURCE 
#define VIDEO_PROC EXPORT_METHOD
#else
#define VIDEO_PROC __declspec(dllimport)
#endif
#else
#ifdef VIDEO_LIBRARY_SOURCE 
#define VIDEO_PROC
#else
#define VIDEO_PROC extern
#endif
#endif


#ifdef __cplusplus
#define VIDLIB_NAMESPACE RENDER_NAMESPACE namespace vidlib {
#define VIDLIB_NAMESPACE_END } RENDER_NAMESPACE_END
#else
#define VIDLIB_NAMESPACE
#define VIDLIB_NAMESPACE_END
#endif

VIDLIB_NAMESPACE
//#include <actimg.h>

#ifndef VIDEO_STRUCTURE_DEFINED
//typedef PTRSZVAL PVIDEO;
#endif

// window class = "VideoOutputClass"
//VIDEO_PROC int RegisterVideoOutput( void );  // can be called to use this as a control
#define WD_HVIDEO   0   // WindowData_HVIDEO

// in case "VideoOutputClass" was used as a control in a dialog...
#define GetVideoHandle( hWndDialog, nControl ) ((HVIDEO)(GetWindowLong( GetDlgItem( hWndDialog, nControl ), 0 )))

// sometimes needed when using existing system knowledge
//RENDER_PROC( PTRSZVAL, GetNativeHandle )( PRENDERER hVideo );
//#ifndef DISPLAY_SOURCE
//VIDEO_PROC ImageFile *GetDisplayImage( HVIDEO hVideo );
//#endif
RENDER_PROC( PKEYBOARD, GetDisplayKeyboard )( PRENDERER hVideo );
//VIDEO_PROC void GetDisplayPosition( HVIDEO hVid, S_32 *x, S_32 *y );

//VIDEO_PROC int ProcessMessages( void ); // returns true while messages were procesed.

#ifndef _WIN32
#define HICON int
#endif

//VIDEO_PROC void PutVideoAbove( HVIDEO hVideo, HVIDEO hAbove ); // put hvideo above 'above'


VIDLIB_NAMESPACE_END
#ifdef __cplusplus
	using namespace sack::image::render;
using namespace sack::image::render::vidlib;
#endif
#endif
// $Log: vidlib.h,v $
// Revision 1.28  2003/07/25 11:59:25  panther
// Define packed_prefix for watcom structure
//
// Revision 1.27  2003/03/25 23:36:46  panther
// phase out more functions
//
// Revision 1.26  2003/03/25 08:38:11  panther
// Add logging
//
