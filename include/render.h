// this shouldprobably be interlocked with
//  display.h or vidlib.h(video.h)

#ifndef RENDER_INTERFACE_INCLUDED
#define RENDER_INTERFACE_INCLUDED

#include <sack_types.h>
#include <keybrd.h>
#include <image.h>
#include <msgprotocol.h>

#ifndef SECOND_RENDER_LEVEL
#define SECOND_RENDER_LEVEL
#define PASTE(sym,name) name
#else
#define PASTE2(sym,name) sym##name
#define PASTE(sym,name) PASTE2(sym,name)
#endif

#  ifdef BCC16
#     ifdef RENDER_LIBRARY_SOURCE 
#        define RENDER_PROC(type,name) type STDPROC _export PASTE(SECOND_RENDER_LEVEL,name)
#     else
#        define RENDER_PROC(type,name) extern type STDPROC PASTE(SECOND_RENDER_LEVEL,name)
#     endif
#  else
#     if !defined(__STATIC__) && !defined(__UNIX__)
#        ifdef RENDER_LIBRARY_SOURCE 
#           define RENDER_PROC(type,name) EXPORT_METHOD type CPROC PASTE(SECOND_RENDER_LEVEL,name)
#           define RENDER_DATA(type,name) EXPORT_METHOD type name
#        else
#           define RENDER_PROC(type,name) IMPORT_METHOD type CPROC PASTE(SECOND_RENDER_LEVEL,name)
#           define RENDER_DATA(type,name) IMPORT_METHOD type name
#        endif
#     else
#        ifdef RENDER_LIBRARY_SOURCE 
#           define RENDER_PROC(type,name) type CPROC PASTE(SECOND_RENDER_LEVEL,name)
#           define RENDER_DATA(type,name) type name
#        else
#           define RENDER_PROC(type,name) extern type CPROC PASTE(SECOND_RENDER_LEVEL,name)
#           define RENDER_DATA(type,name) extern type name
#        endif
#     endif
#  endif


#ifndef PSPRITE_METHOD
#define PSPRITE_METHOD PSPRITE_METHOD
//RENDER_NAMESPACE
	typedef struct sprite_method_tag *PSPRITE_METHOD;
//END_RENDER_NAMESPACE
#endif

#ifdef __cplusplus
#define RENDER_NAMESPACE namespace sack { namespace image { namespace render {
#define RENDER_NAMESPACE_END }}}
#else
#define RENDER_NAMESPACE 
#define RENDER_NAMESPACE_END

//extern "C" {
#endif
RENDER_NAMESPACE

#ifndef PRENDERER
	typedef struct HVIDEO_tag *PRENDERER;
typedef struct HVIDEO_tag RENDERER;
#endif


// Message IDs 0-99 are reserved for
// very core level messages.
// Message IDs 100-999 are for general purpose window input/output
// Message ID 1000+ Usable by applications to transport messages via
//                  the image's default message loop.
enum active_msg_id {
   ACTIVE_MSG_PING    // Message ID 0 - contains a active image to respond to
   , ACTIVE_MSG_PONG    // Message ID 0 - contains a active image to respond to
   , ACTIVE_MSG_MOUSE = 100
   , ACTIVE_MSG_GAIN_FOCUS
   , ACTIVE_MSG_LOSE_FOCUS
   , ACTIVE_MSG_DRAG
   , ACTIVE_MSG_KEY
   , ACTIVE_MSG_DRAW
   , ACTIVE_MSG_CREATE
   , ACTIVE_MSG_DESTROY

   , ACTIVE_MSG_USER = 1000
};

typedef struct {
   enum active_msg_id ID;
   _32  size; // the size of the cargo potion of the message. (mostly data.raw)
   union {
  //--------------------
      struct {
         PRENDERER respondto; 
      } ping;
  //--------------------
      struct {
         int x, y, b;
      } mouse;
  //--------------------
      struct {
         PRENDERER lose;
      } gain_focus;
  //--------------------
      struct {
         PRENDERER gain;
      } lose_focus;
  //--------------------
      struct {
         _8 no_informaiton;
      } draw;
  //--------------------
      struct {
         _8 no_informaiton;
      } close;
  //--------------------
      struct {
         _8 no_informaiton;
      } create;
  //--------------------
      struct {
         _8 no_informaiton;
      } destroy;     
  //--------------------
      struct {
         _32 key;
      } key;
  //--------------------
      _8 raw[1];
   } data;
} ACTIVEMESSAGE, *PACTIVEMESSAGE;

// Event Message ID's CANNOT be 0
// Message Event ID (base+0) is when the
// server teriminates, and ALL client resources
// are lost.
// Message Event ID (base+1) is when the
// final message has been received, and any
// pending events collected should be dispatched.

enum {
 MSG_CloseMethod = MSG_EventUser
 ,MSG_RedrawMethod    
 ,MSG_MouseMethod     
 ,MSG_LoseFocusMethod 
 ,MSG_KeyMethod       
 ,MSG_GeneralMethod   
 ,MSG_RedrawFractureMethod
 ,MSG_ThreadEventPost // used by 'display' renderer.... internal events are posted via a queue
};

typedef void (CPROC*CloseCallback)( PTRSZVAL psvUser );
typedef void (CPROC*RedrawCallback)( PTRSZVAL psvUser, PRENDERER self );
// would be 'wise' to retun 0 if ignored, 1 if observed (perhaps not used), but NOT ignored.
typedef int  (CPROC*MouseCallback)( PTRSZVAL psvUser, S_32 x, S_32 y, _32 b );
typedef void (CPROC*LoseFocusCallback)( PTRSZVAL dwUser, PRENDERER pGain );
// without a keyproc, you will still get key notification in the mousecallback
// if KeyProc returns 0 or is NULL, then bound keys are checked... otherwise
// priority is given to controls with focus that handle keys.
typedef int (CPROC*KeyProc)( PTRSZVAL dwUser, _32 keycode );
// without any other proc, you will get a general callback message.
typedef void (CPROC*GeneralCallback)( PTRSZVAL psvUser
                                , PRENDERER image
                                     , PACTIVEMESSAGE msg );
typedef void (CPROC*RenderReadCallback)(PTRSZVAL psvUser, PRENDERER pRenderer, TEXTSTR buffer, INDEX len );
// called before redraw callback to update the background on the scene...
typedef void (CPROC*_3DUpdateCallback)( PTRSZVAL psvUser );

//----------------------------------------------------------
//   Mouse Button definitions
//----------------------------------------------------------
// the prefix of these may either be interpreted as MAKE - as in
// a make/break state of a switch.  Or may be interpreted as
// MouseKey.... such as KB_ once upon a time stood for KeyBoard,
// and not Keebler as some may have suspected.
#ifndef MK_LBUTTON 
#define MK_LBUTTON 0x01
#endif
#ifndef MK_RBUTTON 
#define MK_RBUTTON 0x02
#endif
#ifndef MK_MBUTTON
#define MK_MBUTTON 0x10
#endif
// mask to test to see if some button (physical mouse, not logical)
// is currently pressed...
#define MK_SOMEBUTTON       (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)
#define MAKE_SOMEBUTTONS(b)     ((b)&(MK_SOMEBUTTON))
// test a button variable to see if no buttons are currently pressed
// NOBUTTON, NOBUTTONS may be confusing, consider renaming these....
#define MAKE_NOBUTTONS(b)     ( !((b) & MK_SOMEBUTTON ) )
// break of some button
#define BREAK_NEWBUTTON(b,_b) ((((b)^(_b))&(_b))&MK_SOMEBUTTON)
// make of some button
#define MAKE_NEWBUTTON(b,_b) ((((b)^(_b))&(b))&MK_SOMEBUTTON)
// test current b vs prior _b to see if the  last button pressed is
// now not pressed...
#define BREAK_LASTBUTTON(b,_b)  ( BREAK_NEWBUTTON(b,_b) && MAKE_NOBUTTONS(b) )
// test current b vs prior _b to see if there is now some button pressed
// when previously there were no buttons pressed...
#define MAKE_FIRSTBUTTON(b,_b) ( MAKE_NEWBUTTON(b,_b) && MAKE_NOBUTTONS(_b) )
// these button states may reflect the current
// control, alt, shift key states.  There may be further
// definitions (meta?) And as of the writing of this comment
// these states may not be counted on, if you care about these
// please do validate that the code gives them to you all the way
// from the initial mouse message through all layers to the final
// application handler.
#ifndef MK_CONTROL
#define MK_CONTROL 0x08
#endif
#ifndef MK_ALT
#define MK_ALT 0x20
#endif
#ifndef MK_SHIFT
#define MK_SHIFT 0x40
#endif

#define MK_SCROLL_DOWN  0x100
#define MK_SCROLL_UP    0x200
#define MK_SCROLL_LEFT  0x400
#define MK_SCROLL_RIGHT 0x800

#ifndef MK_NO_BUTTON
// used to indicate that there is
// no known button information available.  The mouse
// event which triggered this was beyond the realm of
// this mouse handler, but it is entitled to know that
// it now knows nothing.
#define MK_NO_BUTTON 0xFFFFFFFF
#endif

// this bit will NEVER NEVER NEVER be set
// for ANY reason whatsoever.
#define MK_INVALIDBUTTON 0x80000000

// One or more other buttons were pressed.  These
// buttons are available by querying the keyboard state.
#define MK_OBUTTON 0x80 // any other button (keyboard)

//----------------------------------------------------------

#define PANEL_ATTRIBUTE_ALPHA    0x10000
#define PANEL_ATTRIBUTE_HOLEY    0x20000
// focus on this window excludes any of it's parent/sibling panels
// from being able to focus.
#define PANEL_ATTRIBUTE_EXCLUSIVE 0x40000

// child attribute affects the child is contained within this parent
#define PANEL_ATTRIBUTE_INTERNAL  0x88000

    RENDER_PROC( int , InitDisplay) (void); // does not HAVE to be called but may

	 // this generates a mouse event though the mouse system directly
    // there is no queuing, and the mouse is completed before returning.
    RENDER_PROC( void, GenerateMouseRaw)( S_32 x, S_32 y, _32 b );
	 RENDER_PROC( void, GenerateMouseDeltaRaw )( S_32 x, S_32 y, _32 b );

    RENDER_PROC( void , SetApplicationTitle) (const TEXTCHAR *title );
    RENDER_PROC( void , SetRendererTitle) ( PRENDERER render, const TEXTCHAR *title );
    RENDER_PROC( void , SetApplicationIcon)  (Image Icon); //
    // these would be better named ScreenSize
    RENDER_PROC( void , GetDisplaySize)      ( _32 *width, _32 *height );
    RENDER_PROC( void , SetDisplaySize)      ( _32 width, _32 height );

    // Preferred method is to call Idle() or IdleFor(n)
    //RENDER_PROC( int , ProcessDisplayMessages)      (void);

#ifdef __WINDOWS__
    RENDER_PROC (void, EnableLoggingOutput)( LOGICAL bEnable );

	 RENDER_PROC (PRENDERER, MakeDisplayFrom) (HWND hWnd);
#endif
    // open the window as layered - allowing full transparency.
#define DISPLAY_ATTRIBUTE_LAYERED 0x0100
    // window will not be in alt-tab list
#define DISPLAY_ATTRIBUTE_CHILD 0x0200
    // set to WS_EX_TRANSPARENT - all mouse is passed, regardless of alpha/shape
#define DISPLAY_ATTRIBUTE_NO_MOUSE 0x0400
#define DISPLAY_ATTRIBUTE_NO_AUTO_FOCUS 0x0800

    RENDER_PROC( PRENDERER, OpenDisplaySizedAt)     ( _32 attributes, _32 width, _32 height, S_32 x, S_32 y );
    RENDER_PROC( PRENDERER, OpenDisplayAboveSizedAt)( _32 attributes, _32 width, _32 height, S_32 x, S_32 y, PRENDERER above );
    RENDER_PROC( PRENDERER, OpenDisplayAboveUnderSizedAt)( _32 attributes, _32 width, _32 height, S_32 x, S_32 y, PRENDERER above, PRENDERER under );
	 RENDER_PROC( void, SetDisplayFade )( PRENDERER hVideo, int level );

    RENDER_PROC( void         , CloseDisplay) ( PRENDERER );

    RENDER_PROC( void , UpdateDisplayPortionEx) ( PRENDERER, S_32 x, S_32 y, _32 width, _32 height DBG_PASS );
#define UpdateDisplayPortion(r,x,y,w,h) UpdateDisplayPortionEx(r,x,y,w,h DBG_SRC )
	 RENDER_PROC( void , UpdateDisplayEx)        ( PRENDERER DBG_PASS );
#define UpdateDisplay(v) UpdateDisplayEx( v DBG_SRC )
                             
    RENDER_PROC( void , ClearDisplay)         ( PRENDERER ); // ClearTo(0), Update
   
    RENDER_PROC( void, GetDisplayPosition)   ( PRENDERER, S_32 *x, S_32 *y, _32 *width, _32 *height );
    RENDER_PROC( void , MoveDisplay)          ( PRENDERER, S_32 x, S_32 y );
    RENDER_PROC( void , MoveDisplayRel)       ( PRENDERER, S_32 delx, S_32 dely );
    RENDER_PROC( void , SizeDisplay)          ( PRENDERER, _32 w, _32 h );
    RENDER_PROC( void , SizeDisplayRel)       ( PRENDERER, S_32 delw, S_32 delh );
   RENDER_PROC( void, MoveSizeDisplay )( PRENDERER hVideo
                                        , S_32 x, S_32 y
                                        , S_32 w, S_32 h );
   RENDER_PROC( void, MoveSizeDisplayRel )( PRENDERER hVideo
                                        , S_32 delx, S_32 dely
                                        , S_32 delw, S_32 delh );
    RENDER_PROC( void , PutDisplayAbove)      ( PRENDERER, PRENDERER ); // this that - put this above that
	 RENDER_PROC (void, PutDisplayIn) (PRENDERER hVideo, PRENDERER hContainer); // put this in container

    RENDER_PROC( Image , GetDisplayImage)     ( PRENDERER );

    RENDER_PROC( void , SetCloseHandler)      ( PRENDERER, CloseCallback, PTRSZVAL );
    RENDER_PROC( void , SetMouseHandler)      ( PRENDERER, MouseCallback, PTRSZVAL );
	 RENDER_PROC( void , SetRedrawHandler)     ( PRENDERER, RedrawCallback, PTRSZVAL );
	 // call this to call the callback registered. as appropriate.  Said callback
    // should never be directly called by application.
    RENDER_PROC( void, Redraw )( PRENDERER hVideo );


    RENDER_PROC( void , SetKeyboardHandler)   ( PRENDERER, KeyProc, PTRSZVAL );
    RENDER_PROC( void , SetLoseFocusHandler)  ( PRENDERER, LoseFocusCallback, PTRSZVAL );
    RENDER_PROC( void, SetRenderReadCallback )( PRENDERER pRenderer, RenderReadCallback callback, PTRSZVAL psv );
    RENDER_PROC( void , SetDefaultHandler)    ( PRENDERER, GeneralCallback, PTRSZVAL );

    RENDER_PROC( void , GetMouseState )        ( S_32 *x, S_32 *y, _32 *b );
    RENDER_PROC( void , GetMousePosition)     ( S_32 *x, S_32 *y );
    RENDER_PROC( void , SetMousePosition)     ( PRENDERER, S_32 x, S_32 y );

    RENDER_PROC( LOGICAL , HasFocus)          ( PRENDERER );

    RENDER_PROC( int, SendActiveMessage)     ( PRENDERER dest, PACTIVEMESSAGE msg );
    RENDER_PROC( PACTIVEMESSAGE , CreateActiveMessage) ( int ID, int size, ... );

    RENDER_PROC( char, GetKeyText)             ( int key );
    RENDER_PROC( _32, IsKeyDown )              ( PRENDERER display, int key );
    RENDER_PROC( _32, KeyDown )                ( PRENDERER display, int key );
    RENDER_PROC( LOGICAL, DisplayIsValid )     ( PRENDERER display );
    RENDER_PROC( void, OwnMouseEx )            ( PRENDERER display, _32 bOwn DBG_PASS );
    RENDER_PROC( int, BeginCalibration )       ( _32 points );
    RENDER_PROC( void, SyncRender )            ( PRENDERER display );
	 RENDER_PROC( int, EnableOpenGL )           ( PRENDERER hVideo );
	 RENDER_PROC( int, EnableOpenGLView )           ( PRENDERER hVideo, int x, int y, int w, int h );
    // results with the fracture ID for SetActiveGLDisplay
    RENDER_PROC( int, SetActiveGLDisplayView )     ( PRENDERER hDisplay, int nFracture );
	 RENDER_PROC( int, SetActiveGLDisplay )     ( PRENDERER hDisplay );
	 // makes an opengl texture type from an image...
	 // uhmm not sure what else we can do about this...
	 // but there are possibly issues such as power of 2 limitations
	 // that may mean this has to return some sort of structure
	 // that can contain the actual image within a virtual blankness
	 // and or parts of the image ... depending on how the image can be
	 // broken up to make valid opengl images -- opengl 2.0 - rumor has it
    // power of 2 textures are no longer required.
	 RENDER_PROC( int, LoadGLImage )( Image image, int *result );

RENDER_PROC( void, MakeTopmost )( PRENDERER hVideo );
RENDER_PROC (void, MakeAbsoluteTopmost) (PRENDERER hVideo);
RENDER_PROC( int, IsTopmost )( PRENDERER hVideo );
RENDER_PROC( void, HideDisplay )( PRENDERER hVideo );
RENDER_PROC( void, RestoreDisplay )( PRENDERER hVideo );
RENDER_PROC( LOGICAL, IsDisplayHidden )( PRENDERER video );

// set focus to display, no events are generated if display already
// has the focus.
RENDER_PROC( void, ForceDisplayFocus )( PRENDERER display );

// display set as topmost within it's group (normal/bottommost/topmost)
RENDER_PROC( void, ForceDisplayFront )( PRENDERER display );
// display is force back one layer... or forced to bottom?
// alt-n pushed the display to the back... alt-tab is different...
RENDER_PROC( void, ForceDisplayBack )( PRENDERER display );

// if a readcallback is enabled, then this will be no-wait, and one
// will expect to receive the read data in the callback.  Otherwise
// this will return any data which is present already, also non wait.
// Returns length read, INVALID_INDEX if no data read.
//
// If there IS a read callback, return will be 1 if there was no
// previous read queued, and 0 if there was already a read pending
// there may be one and only one read queued (for now)  In either case
// if the read could not be queued, it will be 0..
//
// If READLINE is true - then the result of the read will be a completed line.
// if there is no line present, and no callback defined, this will return INVALID_INDEX characters...
// 0 characters is a \n only (in line mode)
// 0 will be returned for no characters in non line mode...
//
// it will not have the end of line terminator (as generated by a non-bound enter key)
// I keep thinking there must be some kinda block mode read one can do, but no, uhh
// no, there's no way to get the user to put in X characters exactly....?
RENDER_PROC( _32, ReadDisplayEx )( PRENDERER pRenderer, TEXTSTR buffer, _32 maxlen, LOGICAL bReadLine );
#define ReadDisplay(r,b,len)      ReadDisplayEx(r,b,len,FALSE)
#define ReadDisplayLine(r,b,len)  ReadDisplayEx(r,b,len,TRUE)



#ifndef KEY_STRUCTURE_DEFINED
typedef void (CPROC*KeyTriggerHandler)(PTRSZVAL,_32 keycode);
typedef struct KeyDefine *PKEYDEFINE;
#endif
RENDER_PROC( PKEYDEFINE, CreateKeyBinder )( void );
RENDER_PROC( void, DestroyKeyBinder )( PKEYDEFINE pKeyDef );
RENDER_PROC( int, HandleKeyEvents )( PKEYDEFINE KeyDefs, _32 keycode );

RENDER_PROC( int, BindEventToKeyEx )( PKEYDEFINE KeyDefs, _32 scancode, _32 modifier, KeyTriggerHandler trigger, PTRSZVAL psv );
RENDER_PROC( int, BindEventToKey )( PRENDERER pRenderer, _32 scancode, _32 modifier, KeyTriggerHandler trigger, PTRSZVAL psv );
RENDER_PROC( int, UnbindKey )( PRENDERER pRenderer, _32 scancode, _32 modifier );

RENDER_PROC( int, IsTouchDisplay )( void );

RENDER_PROC( PSPRITE_METHOD, EnableSpriteMethod )(PRENDERER render, void(CPROC*RenderSprites)(PTRSZVAL psv, PRENDERER renderer, S_32 x, S_32 y, _32 w, _32 h ), PTRSZVAL psv );

typedef void (CPROC*dropped_file_acceptor)(PTRSZVAL psv, CTEXTSTR filename, S_32 x, S_32 y );
RENDER_PROC( void, WinShell_AcceptDroppedFiles )( PRENDERER renderer, dropped_file_acceptor f, PTRSZVAL psvUser );

RENDER_PROC (void, DisableMouseOnIdle) (PRENDERER hVideo, LOGICAL bEnable );
RENDER_PROC( void, SetDisplayNoMouse )( PRENDERER hVideo, int bNoMouse );

#ifdef __WINDOWS__
	RENDER_PROC( HWND, GetNativeHandle )( PRENDERER video );
#endif

#define OwnMouse(d,o) OwnMouseEx( d, o DBG_SRC )
    //IsKeyDown
    //KeyDown
    //KeyDouble
    //GetKeyText
#define RENDER_PROC_PTR(type,name) type  (CPROC*_##name)
typedef struct render_interface_tag
    {
       RENDER_PROC_PTR( int , InitDisplay) (void); // does not HAVE to be called but may

       RENDER_PROC_PTR( void , SetApplicationTitle) (const TEXTCHAR *title );
       RENDER_PROC_PTR( void , SetApplicationIcon)  (Image Icon); //
    RENDER_PROC_PTR( void , GetDisplaySize)      ( _32 *width, _32 *height );
    RENDER_PROC_PTR( void , SetDisplaySize)      ( _32 width, _32 height );
    RENDER_PROC_PTR( int , ProcessDisplayMessages)      (void);

    RENDER_PROC_PTR( PRENDERER , OpenDisplaySizedAt)     ( _32 attributes, _32 width, _32 height, S_32 x, S_32 y );
    RENDER_PROC_PTR( PRENDERER , OpenDisplayAboveSizedAt)( _32 attributes, _32 width, _32 height, S_32 x, S_32 y, PRENDERER above );
    RENDER_PROC_PTR( void        , CloseDisplay) ( PRENDERER );

    RENDER_PROC_PTR( void, UpdateDisplayPortionEx) ( PRENDERER, S_32 x, S_32 y, _32 width, _32 height DBG_PASS );
    RENDER_PROC_PTR( void, UpdateDisplayEx)        ( PRENDERER DBG_PASS);
                             
    RENDER_PROC_PTR( void, ClearDisplay)         ( PRENDERER ); // ClearTo(0), Update
   
    RENDER_PROC_PTR( void, GetDisplayPosition)   ( PRENDERER, S_32 *x, S_32 *y, _32 *width, _32 *height );
    RENDER_PROC_PTR( void, MoveDisplay)          ( PRENDERER, S_32 x, S_32 y );
    RENDER_PROC_PTR( void, MoveDisplayRel)       ( PRENDERER, S_32 delx, S_32 dely );
    RENDER_PROC_PTR( void, SizeDisplay)          ( PRENDERER, _32 w, _32 h );
    RENDER_PROC_PTR( void, SizeDisplayRel)       ( PRENDERER, S_32 delw, S_32 delh );
    RENDER_PROC_PTR( void, MoveSizeDisplayRel )  ( PRENDERER hVideo
                                                 , S_32 delx, S_32 dely
                                                 , S_32 delw, S_32 delh );
    RENDER_PROC_PTR( void, PutDisplayAbove)      ( PRENDERER, PRENDERER ); // this that - put this above that
 
    RENDER_PROC_PTR( Image, GetDisplayImage)     ( PRENDERER );

    RENDER_PROC_PTR( void, SetCloseHandler)      ( PRENDERER, CloseCallback, PTRSZVAL );
    RENDER_PROC_PTR( void, SetMouseHandler)      ( PRENDERER, MouseCallback, PTRSZVAL );
    RENDER_PROC_PTR( void, SetRedrawHandler)     ( PRENDERER, RedrawCallback, PTRSZVAL );
    RENDER_PROC_PTR( void, SetKeyboardHandler)   ( PRENDERER, KeyProc, PTRSZVAL );
    RENDER_PROC_PTR( void, SetLoseFocusHandler)  ( PRENDERER, LoseFocusCallback, PTRSZVAL );
    RENDER_PROC_PTR( void, SetDefaultHandler)    ( PRENDERER, GeneralCallback, PTRSZVAL );

    RENDER_PROC_PTR( void, GetMousePosition)     ( S_32 *x, S_32 *y );
    RENDER_PROC_PTR( void, SetMousePosition)     ( PRENDERER, S_32 x, S_32 y );

    RENDER_PROC_PTR( LOGICAL, HasFocus)          ( PRENDERER );

    RENDER_PROC_PTR( int, SendActiveMessage)     ( PRENDERER dest, PACTIVEMESSAGE msg );
    RENDER_PROC_PTR( PACTIVEMESSAGE , CreateActiveMessage) ( int ID, int size, ... );

    RENDER_PROC_PTR( char, GetKeyText)           ( int key );
    RENDER_PROC_PTR( _32, IsKeyDown )        ( PRENDERER display, int key );
    RENDER_PROC_PTR( _32, KeyDown )         ( PRENDERER display, int key );
    RENDER_PROC_PTR( LOGICAL, DisplayIsValid )  ( PRENDERER display );
    // own==0 release else mouse owned.
    RENDER_PROC_PTR( void, OwnMouseEx )            ( PRENDERER display, _32 Own DBG_PASS);
    RENDER_PROC_PTR( int, BeginCalibration )       ( _32 points );
    RENDER_PROC_PTR( void, SyncRender )            ( PRENDERER pDisplay );
    RENDER_PROC_PTR( int, EnableOpenGL )           ( PRENDERER hVideo );
    RENDER_PROC_PTR( int, SetActiveGLDisplay )     ( PRENDERER hDisplay );
    //IsKeyDown
    //KeyDown
    //KeyDouble
    //GetKeyText
   RENDER_PROC_PTR( void, MoveSizeDisplay )( PRENDERER hVideo
                                        , S_32 x, S_32 y
                                        , S_32 w, S_32 h );
   RENDER_PROC_PTR( void, MakeTopmost )    ( PRENDERER hVideo );
   RENDER_PROC_PTR( void, HideDisplay )      ( PRENDERER hVideo );
   RENDER_PROC_PTR( void, RestoreDisplay )   ( PRENDERER hVideo );

	RENDER_PROC_PTR( void, ForceDisplayFocus )( PRENDERER display );
	RENDER_PROC_PTR( void, ForceDisplayFront )( PRENDERER display );
	RENDER_PROC_PTR( void, ForceDisplayBack )( PRENDERER display );

	RENDER_PROC_PTR( int, BindEventToKey )( PRENDERER pRenderer, _32 scancode, _32 modifier, KeyTriggerHandler trigger, PTRSZVAL psv );
	RENDER_PROC_PTR( int, UnbindKey )( PRENDERER pRenderer, _32 scancode, _32 modifier );
	RENDER_PROC_PTR( int, IsTopmost )( PRENDERER hVideo );
	RENDER_PROC_PTR( void, OkaySyncRender )            ( void );
   RENDER_PROC_PTR( int, IsTouchDisplay )( void );
	RENDER_PROC_PTR( void , GetMouseState )        ( S_32 *x, S_32 *y, _32 *b );
	RENDER_PROC_PTR ( PSPRITE_METHOD, EnableSpriteMethod )(PRENDERER render, void(CPROC*RenderSprites)(PTRSZVAL psv, PRENDERER renderer, S_32 x, S_32 y, _32 w, _32 h ), PTRSZVAL psv );
	RENDER_PROC_PTR( void, WinShell_AcceptDroppedFiles )( PRENDERER renderer, dropped_file_acceptor f, PTRSZVAL psvUser );
	RENDER_PROC_PTR(void, PutDisplayIn) (PRENDERER hVideo, PRENDERER hContainer);
#ifdef __WINDOWS__
	RENDER_PROC_PTR (PRENDERER, MakeDisplayFrom) (HWND hWnd) ;
#endif
	RENDER_PROC_PTR( void , SetRendererTitle) ( PRENDERER render, const TEXTCHAR *title );
	RENDER_PROC_PTR (void, DisableMouseOnIdle) (PRENDERER hVideo, LOGICAL bEnable );
	RENDER_PROC_PTR( PRENDERER, OpenDisplayAboveUnderSizedAt)( _32 attributes, _32 width, _32 height, S_32 x, S_32 y, PRENDERER above, PRENDERER under );
	RENDER_PROC_PTR( void, SetDisplayNoMouse )( PRENDERER hVideo, int bNoMouse );
	RENDER_PROC_PTR( void, Redraw )( PRENDERER hVideo );
	RENDER_PROC_PTR(void, MakeAbsoluteTopmost) (PRENDERER hVideo);
	RENDER_PROC_PTR( void, SetDisplayFade )( PRENDERER hVideo, int level );
	RENDER_PROC_PTR( LOGICAL, IsDisplayHidden )( PRENDERER video );
#ifdef __WINDOWS__
	RENDER_PROC_PTR( HWND, GetNativeHandle )( PRENDERER video );
#endif

} *PRENDER_INTERFACE, RENDER_INTERFACE;

//RENDER_PROC( PRENDER_INTERFACE, GetDisplayInterface )( void );
//RENDER_PROC( void, DropDisplayInterface )( void );

#include <procreg.h>
#define GetDisplayInterface() (PRENDER_INTERFACE)GetInterface( WIDE("render") )
#define DropDisplayInterface(x) DropInterface( WIDE("render"), x )

#ifdef DEFINE_DEFAULT_RENDER_INTERFACE
#define USE_RENDER_INTERFACE GetDisplayInterface()
#endif

#ifdef FORCE_NO_INTERFACE
#undef USE_RENDER_INTERFACE
#endif

#ifdef USE_RENDER_INTERFACE
typedef int check_this_variable;
// these methods are provided for backwards compatibility
// these should not be used - but rather use the interface defined below
// (the ones not prefixed by ActImage_ - except for ActImage_Init, which
// may(should) be called before any other function.
#define REND_PROC_ALIAS(name) ((USE_RENDER_INTERFACE)->_##name)
#define REND_PROC_ALIAS_VOID(name) if(USE_RENDER_INTERFACE)(USE_RENDER_INTERFACE)->_##name

#define SetApplicationTitle       REND_PROC_ALIAS(SetApplicationTitle)
#define SetRendererTitle       REND_PROC_ALIAS(SetRendererTitle)
#define SetApplicationIcon        REND_PROC_ALIAS(SetApplicationIcon)
#define GetDisplaySize            REND_PROC_ALIAS(GetDisplaySize)
#define SetDisplaySize            REND_PROC_ALIAS(SetDisplaySize)
#define GetDisplayPosition        REND_PROC_ALIAS(GetDisplayPosition)
#define ProcessDisplayMessages    REND_PROC_ALIAS(ProcessDisplayMessages)

#define MakeDisplayFrom        REND_PROC_ALIAS(MakeDisplayFrom)
#define OpenDisplaySizedAt        REND_PROC_ALIAS(OpenDisplaySizedAt)
#define OpenDisplayAboveSizedAt   REND_PROC_ALIAS(OpenDisplayAboveSizedAt)
#define OpenDisplayAboveUnderSizedAt   REND_PROC_ALIAS(OpenDisplayAboveUnderSizedAt)
#define CloseDisplay              REND_PROC_ALIAS(CloseDisplay)
#define UpdateDisplayPortionEx    REND_PROC_ALIAS(UpdateDisplayPortionEx)
#define UpdateDisplayEx             REND_PROC_ALIAS(UpdateDisplayEx)
#define ClearDisplay              REND_PROC_ALIAS(ClearDisplay)
#define SetMousePosition          REND_PROC_ALIAS(SetMousePosition)
#define GetMousePosition          REND_PROC_ALIAS(GetMousePosition)
#define GetMouseState          REND_PROC_ALIAS(GetMouseState)
#define EnableSpriteMethod          REND_PROC_ALIAS(EnableSpriteMethod)
#define WinShell_AcceptDroppedFiles REND_PROC_ALIAS(WinShell_AcceptDroppedFiles)
#define MoveDisplay               REND_PROC_ALIAS(MoveDisplay)
#define MoveDisplayRel            REND_PROC_ALIAS(MoveDisplayRel)
#define SizeDisplay               REND_PROC_ALIAS(SizeDisplay)
#define Redraw               REND_PROC_ALIAS(Redraw)
#define SizeDisplayRel            REND_PROC_ALIAS(SizeDisplayRel)
#define MoveSizeDisplay        REND_PROC_ALIAS(MoveSizeDisplay)
#define MoveSizeDisplayRel        REND_PROC_ALIAS(MoveSizeDisplayRel)
#define PutDisplayAbove           REND_PROC_ALIAS(PutDisplayAbove)
#define PutDisplayIn           REND_PROC_ALIAS(PutDisplayIn)
#define GetDisplayImage           REND_PROC_ALIAS(GetDisplayImage)

#define SetCloseHandler           REND_PROC_ALIAS(SetCloseHandler)
#define SetMouseHandler           REND_PROC_ALIAS(SetMouseHandler)
#define SetRedrawHandler          REND_PROC_ALIAS(SetRedrawHandler)
#define SetKeyboardHandler        REND_PROC_ALIAS(SetKeyboardHandler)
#define SetLoseFocusHandler       REND_PROC_ALIAS(SetLoseFocusHandler)
#define SetDefaultHandler         REND_PROC_ALIAS(SetDefaultHandler)

#define GetKeyText                REND_PROC_ALIAS(GetKeyText)
#define HasFocus                  REND_PROC_ALIAS(HasFocus)

#define CreateMessage             REND_PROC_ALIAS(CreateMessage)
#define SendActiveMessage         REND_PROC_ALIAS(SendActiveMessage)
#define IsKeyDown                 REND_PROC_ALIAS(IsKeyDown)
#define KeyDown                   REND_PROC_ALIAS(KeyDown)
#define DisplayIsValid            REND_PROC_ALIAS(DisplayIsValid)
#define OwnMouseEx                REND_PROC_ALIAS(OwnMouseEx)
#define BeginCalibration          REND_PROC_ALIAS(BeginCalibration)
#define SyncRender                REND_PROC_ALIAS(SyncRender)
#define OkaySyncRender                REND_PROC_ALIAS(OkaySyncRender)
#define EnableOpenGL              REND_PROC_ALIAS(EnableOpenGL)
#define SetActiveGLDisplay        REND_PROC_ALIAS(SetActiveGLDisplay )
#define HideDisplay               REND_PROC_ALIAS(HideDisplay)
#define IsDisplayHidden               REND_PROC_ALIAS(IsDisplayHidden)
#define GetNativeHandle             REND_PROC_ALIAS(GetNativeHandle)
#define RestoreDisplay             REND_PROC_ALIAS(RestoreDisplay)
#define MakeTopmost               REND_PROC_ALIAS_VOID(MakeTopmost)
#define MakeAbsoluteTopmost               REND_PROC_ALIAS_VOID(MakeAbsoluteTopmost)
#define IsTopmost               REND_PROC_ALIAS(IsTopmost)
#define SetDisplayFade               REND_PROC_ALIAS(SetDisplayFade)

#define ForceDisplayFocus         REND_PROC_ALIAS(ForceDisplayFocus)
#define ForceDisplayFront       REND_PROC_ALIAS(ForceDisplayFront)
#define ForceDisplayBack          REND_PROC_ALIAS(ForceDisplayBack)
#define BindEventToKey          REND_PROC_ALIAS(BindEventToKey)
#define UnbindKey               REND_PROC_ALIAS(UnbindKey)
#define IsTouchDisplay          REND_PROC_ALIAS(IsTouchDisplay)
#define DisableMouseOnIdle      REND_PROC_ALIAS(DisableMouseOnIdle )
#define SetDisplayNoMouse      REND_PROC_ALIAS(SetDisplayNoMouse )

#endif

#define OpenDisplay(a)            OpenDisplaySizedAt(a,-1,-1,-1,-1)
#define OpenDisplaySized(a,w,h)   OpenDisplaySizedAt(a,w,h,-1,-1)
#define OpenDisplayAbove(p,a)            OpenDisplayAboveSizedAt(p,-1,-1,-1,-1,a)
#define OpenDisplayAboveSized(p,a,w,h)   OpenDisplayAboveSizedAt(p,w,h,-1,-1,a)
#define OpenDisplayUnderSizedAt(p,a,w,h,x,y) OpenDisplayAboveUnderSizedAt(a,w,h,-1,-1,NULL,p) 

#ifdef DEFINE_RENDER_PROTOCOL
#include <stddef.h>
// need to define BASE_RENDER_MESSAGE_ID before including this.
//#define MSG_ID(method)  ( ( offsetof( struct render_interface_tag, _##method ) / sizeof( void(*)(void) ) ) + BASE_RENDER_MESSAGE_ID + MSG_EventUser )

#define MSG_DisplayClientClose        MSG_ID(DisplayClientClose)
#define MSG_SetApplicationTitle       MSG_ID(SetApplicationTitle)
#define MSG_SetRendererTitle       MSG_ID(SetRendererTitle)
#define MSG_SetApplicationIcon        MSG_ID(SetApplicationTitle)
#define MSG_GetDisplaySize            MSG_ID(GetDisplaySize)
#define MSG_SetDisplaySize            MSG_ID(SetDisplaySize)
#define MSG_GetDisplayPosition        MSG_ID(GetDisplayPosition)
//, #define MSG_ProcessDisplayMessage
#define MSG_OpenDisplaySizedAt        MSG_ID(OpenDisplaySizedAt)
#define MSG_OpenDisplayAboveSizedAt   MSG_ID(OpenDisplayAboveSizedAt)
#define MSG_CloseDisplay              MSG_ID(CloseDisplay)
#define MSG_UpdateDisplayPortionEx    MSG_ID(UpdateDisplayPortionEx)
#define MSG_UpdateDisplay             MSG_ID(UpdateDisplayEx)
#define MSG_ClearDisplay              MSG_ID(ClearDisplay)
#define MSG_SetMousePosition          MSG_ID(SetMousePosition)
#define MSG_GetMousePosition          MSG_ID(GetMousePosition)
#define MSG_GetMouseState             MSG_ID(GetMouseState )
#define MSG_Redraw               MSG_ID(Redraw)

#define MSG_EnableSpriteMethod             MSG_ID(EnableSpriteMethod )
#define MSG_WinShell_AcceptDroppedFiles    MSG_ID(WinShell_AcceptDroppedFiles )
#define MSG_MoveDisplay               MSG_ID(MoveDisplay)
#define MSG_MoveDisplayRel            MSG_ID(MoveDisplayRel)
#define MSG_SizeDisplay               MSG_ID(SizeDisplay)
#define MSG_SizeDisplayRel            MSG_ID(SizeDisplayRel)
#define MSG_MoveSizeDisplay           MSG_ID(MoveSizeDisplay)
#define MSG_MoveSizeDisplayRel        MSG_ID(MoveSizeDisplayRel)
#define MSG_PutDisplayAbove           MSG_ID(PutDisplayAbove)
#define MSG_GetDisplayImage           MSG_ID(GetDisplayImage)
#define MSG_SetCloseHandler           MSG_ID(SetCloseHandler)
#define MSG_SetMouseHandler           MSG_ID(SetMouseHandler)
#define MSG_SetRedrawHandler          MSG_ID(SetRedrawHandler)
#define MSG_SetKeyboardHandler        MSG_ID(SetKeyboardHandler)
#define MSG_SetLoseFocusHandler       MSG_ID(SetLoseFocusHandler)
#define MSG_SetDefaultHandler         MSG_ID(SetDefaultHandler)
// -- all other handlers - client side only
#define MSG_HasFocus                  MSG_ID(HasFocus)
#define MSG_SendActiveMessage         MSG_ID(SendActiveMessage)
#define MSG_GetKeyText                MSG_ID(GetKeyText)
#define MSG_IsKeyDown                 MSG_ID(IsKeyDown)
#define MSG_KeyDown                   MSG_ID(KeyDown)
#define MSG_DisplayIsValid            MSG_ID(DisplayIsValid)
#define MSG_OwnMouseEx                 MSG_ID(OwnMouseEx)
#define MSG_BeginCalibration           MSG_ID(BeginCalibration)
#define MSG_SyncRender                 MSG_ID(SyncRender)
#define MSG_OkaySyncRender                 MSG_ID(OkaySyncRender)
#define MSG_EnableOpenGL               MSG_ID(EnableOpenGL)
#define MSG_SetActiveGLDisplay         MSG_ID(SetActiveGLDisplay)
#define MSG_HideDisplay               MSG_ID(HideDisplay)
#define MSG_IsDisplayHidden               MSG_ID(IsDisplayHidden)
#define MSG_RestoreDisplay             MSG_ID(RestoreDisplay)
#define MSG_MakeTopmost               MSG_ID(MakeTopmost)
#define MSG_BindEventToKey          MSG_ID(BindEventToKey)
#define MSG_UnbindKey               MSG_ID(UnbindKey)
#define MSG_IsTouchDisplay          MSG_ID(IsTouchDisplay )
#define MSG_GetNativeHandle             MSG_ID(GetNativeHandle)
#endif

RENDER_NAMESPACE_END
#ifdef __cplusplus
	using namespace sack::image::render;
#endif

#endif

// : $
// $Log: render.h,v $
// Revision 1.48  2005/05/25 16:50:09  d3x0r
// Synch with working repository.
//
// Revision 1.50  2005/05/12 21:00:47  jim
// Fix types for begin calibration.  Also Added OkaySyncRender which is a void responce SyncRender
//
// Revision 1.49  2005/04/25 16:07:32  jim
// Update definition of SyncRender to specify the renderer also
//
// Revision 1.48  2005/03/28 09:44:12  panther
// Use single surface to project surround-o-vision.  This btw has the benefit of uniform output.
//
// Revision 1.47  2004/12/05 15:32:06  panther
// Some minor cleanups fixed a couple memory leaks
//
// Revision 1.46  2004/10/13 18:52:53  d3x0r
// Export defniition of movesizedisplay
//
// Revision 1.45  2004/10/03 01:26:07  d3x0r
// Checkpoint - cleaning, stabilizings...
//
// Revision 1.44  2004/10/02 19:49:54  d3x0r
// Fix logging... trying to track down multiple update display issues.... keys are queued, events are locally queued...
//
// Revision 1.43  2004/09/29 00:49:47  d3x0r
// Added fancy wait for PSI frames which allows non-polling sleeping... Extended Idle() to result in meaningful information.
//
// Revision 1.42  2004/09/02 10:22:52  d3x0r
// tweaks for linux build
//
// Revision 1.41  2004/09/01 03:27:19  d3x0r
// Control updates video display issues?  Image blot message go away...
//
// Revision 1.40  2004/08/11 11:41:06  d3x0r
// Begin seperation of key and render
//
// Revision 1.39  2004/06/21 07:47:36  d3x0r
// Checkpoint - make system rouding out nicly.
//
// Revision 1.38  2004/06/14 13:05:35  d3x0r
// Add bind key events to interface.
//
// Revision 1.37  2004/06/14 10:55:09  d3x0r
// Oops defined an alias wrong...
//
// Revision 1.36  2004/06/14 10:46:28  d3x0r
// Define force focus and stacking operations for render panels...
//
// Revision 1.35  2004/06/03 11:16:12  d3x0r
// Update to newer interface names, default interface to dynamic function, else have to link to a library.
//
// Revision 1.34  2004/06/01 21:53:43  d3x0r
// Fix PUBLIC dfeinitions from Windoze-centric to system nonspecified
//
// Revision 1.33  2004/06/01 05:58:32  d3x0r
// Include procreg instead of interface.h
//
// Revision 1.32  2004/05/03 06:15:22  d3x0r
// Define buffered render read
//
// Revision 1.31  2004/05/02 05:44:51  d3x0r
// Implement  BindEventToKey and UnbindKey
//
// Revision 1.30  2004/04/26 09:47:25  d3x0r
// Cleanup some C++ problems, and standard C issues even...
//
// Revision 1.29  2004/03/26 17:12:09  d3x0r
// Attempt to provide a default interface method
//
// Revision 1.28  2004/03/04 01:09:47  d3x0r
// Modifications to force slashes to wlib.  Updates to Interfaces to be gotten from common interface manager.
//
// Revision 1.27  2004/01/11 23:10:38  panther
// Include keyboard to avoid windows errors
//
// Revision 1.26  2003/09/29 13:19:27  panther
// Fix MSG_RestoreDisplay, Implement client/server hooks for hide/restore display
//
// Revision 1.25  2003/09/26 14:20:41  panther
// PSI DumpFontFile, fix hide/restore display
//
// Revision 1.24  2003/09/18 07:42:47  panther
// Changes all across the board...image support, display support, controls editable in psi...
//
// Revision 1.23  2003/09/01 20:04:37  panther
// Added OpenGL Interface to windows video lib, Modified RCOORD comparison
//
// Revision 1.22  2003/08/27 16:05:09  panther
// Define image and renderer sync functions
//
// Revision 1.21  2003/08/01 07:56:12  panther
// Commit changes for logging...
//
// Revision 1.20  2003/07/25 11:59:25  panther
// Define packed_prefix for watcom structure
//
// Revision 1.19  2003/07/24 16:56:41  panther
// Updates to expliclity define C procedure model for callbacks and assembly modules - incomplete
//
// Revision 1.18  2003/06/15 22:35:15  panther
// Define begincalibration entry
//
// Revision 1.17  2003/04/24 00:03:49  panther
// Added ColorAverage to image... Fixed a couple macros
//
// Revision 1.16  2003/03/30 21:38:54  panther
// Fix MSG_ definitions.  Fix lack of anonymous unions
//
// Revision 1.15  2003/03/29 22:51:59  panther
// New render/image layering ability.  Added support to Display for WIN32 usage (native not SDL)
//
// Revision 1.14  2003/03/27 15:36:38  panther
// Changes were done to limit client messages to server - but all SERVER-CLIENT messages were filtered too... Define LOWEST_BASE_MESSAGE
//
// Revision 1.13  2003/03/27 11:49:40  panther
// Implement OwnMouse available to client applications
//
// Revision 1.12  2003/03/25 23:36:34  panther
// Added SizeDisplayRel and MoveSizeDisplayRel.
//
// Revision 1.11  2003/03/25 11:39:44  panther
// Expose OpenDisplay() simple macros, dispatch to SDL thread WarpMouse Events
//
// Revision 1.10  2003/03/25 08:38:11  panther
// Add logging
//
