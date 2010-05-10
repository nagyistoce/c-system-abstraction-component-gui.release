typedef struct display_tag *PPANEL;

#include <image.h>
#include <vidlib/vidstruc.h>
#include <render.h>



#ifdef DISPLAY_CLIENT
#define g global_display_client_data
typedef struct global_tag {
	struct {
		_32 connected : 1;
		_32 disconnected : 1;
		_32 redraw_dispatched : 1;
	} flags;
	_32 MsgBase;
	pid_t MessageSource;
	PLIST pDisplayList;  // list of displays which are open (probably very short)
   PKEYDEFINE KeyDefs;

} GLOBAL;
#elif defined( DISPLAY_IMAGE_CLIENT )
#define g global_image_client_data
typedef struct my_font_tag {
	Font font; // font as the service knows it.
   Font data; // font data as the service would kknow it.
} MYFONT, *PMYFONT;

typedef struct global_tag {
	struct {
		_32 connected : 1;
		_32 disconnected : 1;
	} flags;
	_32 MsgBase;

   PMYFONT DefaultFont;     // actual value returned from the service
	PMYFONT DefaultFontData; // actual data contained in the service

	pid_t MessageSource;
   PLIST pDisplayList;  // list of displays which are open (probably very short)
} GLOBAL;
#endif

#ifndef GLOBAL_STRUCTURE_DEFINED
extern
#endif
GLOBAL g;


// this is what the local client knows about
// displays, including things like the callbacks
// since on the server side it knows nothing about
// the client's program space.
typedef struct my_image_tag {
	ImageData;       // our relavent info about the image...
	Image RealImage; // handle passed from server
	IMAGE_RECTANGLE auxrect;
} *PMyImage, MyImage;

typedef struct display_tag {
	PRENDERER hDisplay; // handle this display represents.
	// need to keep this so that when we destroy it we can destroy
	// the image - this keeps the burden of knowing what's valid to
	// destroy to a mimimum... but if the application keeps the handle
	// and insists on using it, then of course that's an application fault.
	struct {
		_32 mouse_pending : 1;
		_32 draw_pending : 1;
		_32 redraw_dispatched : 1;
	} flags;
	PMyImage DisplayImage;  // this has to be updated whenever the server's is
	                        // on event callbacks - move, resize, etc.

	S_32 x, y;
	_32 width, height;

	CloseCallback CloseMethod;
	PTRSZVAL CloseData;
	struct {
		S_32 x, y;
      _32 b;
	} mouse_pending;
	MouseCallback MouseMethod;
	PTRSZVAL MouseData;
	// every move/size is accompanied by a redraw
   // but a redraw may happen without a resize/move
	RedrawCallback RedrawMethod;
	PTRSZVAL RedrawData;
	KeyProc KeyMethod;
	PTRSZVAL KeyData;
	LoseFocusCallback LoseFocusMethod;
	PTRSZVAL LoseFocusData;
	GeneralCallback GeneralMethod;
	PTRSZVAL GeneralData;

   KEYDEFINE KeyDefs[256];
	FLAGSET( KeyboardState, 512 );
	FLAGSET( KeyboardMetaState, 256 ); //
	_32 KeyDownTime[256];
} DISPLAY, *PDISPLAY;



// LoadImageFile can be done here also, just load the compressed
// image data into a buffer, ship the entire buffer to the server,
// who is then responsible for decoompressing the image annd returning
// a valid image if any.  Then we can release any local buffers.

// This module is all the functions of displaylib native linking methods

// $Log: client.h,v $
// Revision 1.17  2005/05/17 18:35:56  jim
// Fix disconnection of a client.  Remove noisy logging.
//
// Revision 1.16  2005/05/12 21:04:42  jim
// Fixed several conflicts that resulted in various deadlocks.  Also endeavored to clean all warnings.
//
// Revision 1.15  2005/04/25 17:33:55  jim
// Disallow redundant updates from applications while in a dispatched redraw - which will itself result in a display update.
//
// Revision 1.14  2005/03/16 22:28:26  panther
// Updates for extended interface definitions... Fixes some issues with client/server display usage...
//
// Revision 1.13  2004/08/11 12:00:27  d3x0r
// Migrate to new, more common keystruct...
//
// Revision 1.12  2004/06/16 21:13:10  d3x0r
// Cleanups for keybind duplication between here and vidlib... that's all been cleaned...
//
// Revision 1.11  2004/06/16 10:27:23  d3x0r
// Added key events to display library...
//
// Revision 1.10  2003/09/08 12:57:23  panther
// Added comentary about structure usage
//
// Revision 1.9  2003/08/27 15:25:16  panther
// Implement key strokes through mouse callbacks.  And provide image/render sync point method.  Also assign update display portion to be sync'd
//
// Revision 1.8  2003/03/27 10:50:59  panther
// Display - enable resize that works.  Image - remove hline failed message.  Display - Removed some logging messages.
//
// Revision 1.7  2003/03/25 08:45:50  panther
// Added CVS logging tag
//
