#ifndef SOURCE_PSI2
#define SOURCE_PSI2
#endif
#ifndef __CONTROLS_DEFINED__
#define __CONTROLS_DEFINED__

#define MK_PSI_VERSION(ma,mi)  (((ma)<<8)|(mi))
#define PSI_VERSION            MK_PSI_VERSION(1,1)
#define REQUIRE_PSI(ma,mi)    ( PSI_VERSION >= MK_PSI_VERSION(ma,mi) )

//---------------------------------------------------------------
// PSI Version comments
//   1.1 )
//     - added PSI_VERSION so that after this required features
//       may be commented out...
//     - added fonts to common structure - controls and frames may define
//       frames an apply a scale factor to itself and or its children..
//
#include <stdhdrs.h>
#include <image.h>
#include <render.h>

#include <menuflags.h>
#include <fractions.h>

#include <psi/namespaces.h>

PSI_NAMESPACE

#ifdef BCC16
#ifdef PSI_SOURCE
#define PSI_PROC(type,name) type STDPROC _export name
#else
#define PSI_PROC(type,name) type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef PSI_SOURCE
#define PSI_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define PSI_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#else
#ifdef PSI_SOURCE
#define PSI_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define PSI_PROC(type,name) extern type CPROC name
#endif
#endif
#endif

// Control callback functions NEED to be declared in the same source as what
// created the control/frame...
#ifdef SOURCE_PSI2
#define CONTROL_PROC( name,_args )  \
	PSI_PROC( PSI_CONTROL, Get##name##PropertyPage )( PSI_CONTROL pc ); \
	PSI_PROC( void, Apply##name##PropertyPage )( PSI_CONTROL pc, PSI_CONTROL page ); \
   PSI_PROC( int, Config##name)( PSI_CONTROL );  \
	/*PSI_PROC( PSI_CONTROL, Make##name )( PSI_CONTROL pFrame, int attr */ \
	/*, int x, int y, int w, int h*/ \
	/*, PTRSZVAL nID, ... ); */
#define CAPTIONED_CONTROL_PROC( name,_args )  \
	PSI_PROC( PSI_CONTROL, Get##name##PropertyPage )( PSI_CONTROL pc ); \
	PSI_PROC( void, Apply##name##PropertyPage )( PSI_CONTROL pc, PSI_CONTROL page ); \
   PSI_PROC( int, Config##name)( PSI_CONTROL );  \
	/*PSI_PROC( PSI_CONTROL, Make##name )( PSI_CONTROL pFrame, int attr */\
	/*, int x, int y, int w, int h  */\
	/*, PTRSZVAL nID, CTEXTSTR caption, ... ); */
#else
#define CONTROL_PROC( name,_args )  \
	PSI_PROC( PSI_CONTROL, Get##name##PropertyPage )( PSI_CONTROL pc ); \
	PSI_PROC( void, Apply##name##PropertyPage )( PSI_CONTROL pc, PSI_CONTROL page ); \
   PSI_PROC( PSI_CONTROL, Config##name)( PSI_CONTROL );  \
	/*PSI_PROC( PSI_CONTROL, Make##name )( PSI_CONTROL pFrame, int attr*/ \
	/*, int x, int y, int w, int h*/ \
	/*, PTRSZVAL nID, ... );*/
#define CAPTIONED_CONTROL_PROC( name,_args )  \
	PSI_PROC( PSI_CONTROL, Get##name##PropertyPage )( PSI_CONTROL pc ); \
	PSI_PROC( void, Apply##name##PropertyPage )( PSI_CONTROL pc, PSI_CONTROL page ); \
   PSI_PROC( PSI_CONTROL, Config##name)( PSI_CONTROL );  \
	/*PSI_PROC( PSI_CONTROL, Make##name )( PSI_CONTROL pFrame, int attr */\
	/*, int x, int y, int w, int h */\
	/*, PTRSZVAL nID, ... );*/
#endif

// hmm upon loading the thing from disk, need to query the callbacks of the application to
// request the correct ptrszvals...
enum {
    MSG_ControlInit = MSG_EventUser
     , MSG_ButtonDraw
    , MSG_ButtonClick
};

enum {
    COMMON_PROC_ANY
     , COMMON_PROC_DRAW
     , COMMON_PROC_MOUSE
     , COMMON_PROC_KEY
};

#define RegisterControl(name)  do { extern CTEXTSTR  ControlMakeProcName_##name, ControlLoadProcName_##name; \
    RegisterControlProcName( ControlMakeProcName_##name \
    , (POINTER)Make##name        \
    , ControlLoadProcName_##name \
    , (POINTER)Load##name );     \
} while(0)

#define RegisterFrame(name)       RegisterControlProcName( FrameProcName_##name \
    , (POINTER)Make##name  \
    , (POINTER)Load##name )

//PSI_PROC( int, RegisterControlProcName )( CTEXTSTR name, POINTER MakeProc, POINTER LoadProc );


#ifndef CONTROL_SOURCE
#define MKPFRAME(hVid) (((PTRSZVAL)(hVid))|1)
//typedef POINTER PSI_CONTROL;
// any remaining code should reference PSI_CONTROL
#define PCOMMON PSI_CONTROL
#define PCONTROL PSI_CONTROL
#endif
typedef struct common_control_frame *PSI_CONTROL;

#define COMMON_BUTTON_WIDTH 55
#define COMMON_BUTTON_HEIGHT 19
#define COMMON_BUTTON_PAD 5

// normal case needs to be 0 - but this is the thickest - go figure.
#define BORDER_NORMAL            0
#define BORDER_NONE              3
#define BORDER_THIN              1
#define BORDER_THINNER           2
#define BORDER_DENT              4
#define BORDER_THIN_DENT         5
#define BORDER_THICK_DENT        6
#define BORDER_TYPE           0x0f // 16 different frame types standard...

#define BORDER_INVERT         0x80
#define BORDER_CAPTION        0x40
#define BORDER_NOCAPTION      0x20
#define BORDER_INVERT_THINNER (BORDER_THINNER|BORDER_INVERT)
#define BORDER_INVERT_THIN    (BORDER_THIN|BORDER_INVERT)
#define BORDER_BUMP           (BORDER_DENT|BORDER_INVERT)

#define BORDER_NOMOVE         0x0100 // not really a border really...
#define BORDER_CLOSE          0x0200 // well okay maybe these are border traits
#define BORDER_RESIZABLE      0x0400 // can resize this window with a mouse
#define BORDER_WITHIN         0x0800 // frame is on the surface of parent...

#define BORDER_WANTMOUSE      0x1000 // frame surface desires any unclaimed mouse calls
#define BORDER_EXCLUSIVE      0x2000 // frame wants exclusive application input.
// by default controls are scalable.
#define BORDER_FRAME          0x4000 // marks controls which were done with 'create frame', and without BORDER_WITHIN
#define BORDER_FIXED          0x8000 // scale does not apply to coordinates... otherwise it will be...

#define BORDER_NO_EXTRA_INIT        0x010000 // control is private to psi library(used for scrollbars in listboxes, etc) and as such does not call 'extra init'

// these are the indexes for base color
#define HIGHLIGHT           0
#define NORMAL              1
#define SHADE               2
#define SHADOW              3
#define TEXTCOLOR           4
#define CAPTION             5
#define CAPTIONTEXTCOLOR    6
#define INACTIVECAPTION    7
#define INACTIVECAPTIONTEXTCOLOR 8
#define SELECT_BACK         9
#define SELECT_TEXT         10
#define EDIT_BACKGROUND     11
#define EDIT_TEXT           12
#define SCROLLBAR_BACK     13

// these IDs are used to designate default control IDs for
// buttons...
#define TXT_STATIC -1
#ifndef IDOK
#define IDOK BTN_OKAY
#endif
#ifndef IDCANCEL
#define IDCANCEL BTN_CANCEL
#endif
#ifdef __LINUX__
#ifndef PSI_SOURCE
#define BTN_OKAY   1
#define BTN_CANCEL 2
#endif
#endif

// enumeration for control->nType                    
//enum {
#define	CONTROL_FRAME  0// master level control framing...
#define	CONTROL_FRAME_NAME  WIDE("Frame")// master level control framing...
#define  UNDEFINED_CONTROL  1// returns a default control to user - type 1
#define  UNDEFINED_CONTROL_NAME  WIDE("Undefined")// returns a default control to user - type 1
#define  CONTROL_SUB_FRAME 2
#define  CONTROL_SUB_FRAME_NAME WIDE("SubFrame")
#define  STATIC_TEXT 3
#define  STATIC_TEXT_NAME WIDE("TextControl")
#define  NORMAL_BUTTON 4
#define  NORMAL_BUTTON_NAME WIDE("Button")
#define  CUSTOM_BUTTON 5
#define  CUSTOM_BUTTON_NAME WIDE("CustomDrawnButton")
#define  IMAGE_BUTTON 6
#define  IMAGE_BUTTON_NAME WIDE("ImageButton")
#define  RADIO_BUTTON 7// also subtype radio button
#define  RADIO_BUTTON_NAME WIDE("CheckButton")// also subtype radio button
#define  EDIT_FIELD 8
#define  EDIT_FIELD_NAME WIDE("EditControl")
#define  SLIDER_CONTROL 9
#define  SLIDER_CONTROL_NAME WIDE("Slider")
#define  LISTBOX_CONTROL 10
#define  LISTBOX_CONTROL_NAME WIDE("ListBox")
#define  SCROLLBAR_CONTROL 11
#define  SCROLLBAR_CONTROL_NAME WIDE("ScrollBar")
#define  GRIDBOX_CONTROL  12 // TBI (to be implemented)
#define  GRIDBOX_CONTROL_NAME  WIDE("Gridbox") // TBI (to be implemented)
#define  CONSOLE_CONTROL  13 // TBI (to be implemented)
#define  CONSOLE_CONTROL_NAME  WIDE("Console") // TBI (to be implemented)
#define  SHEET_CONTROL    14
#define  SHEET_CONTROL_NAME    WIDE("SheetControl")
#define  COMBOBOX_CONTROL 15
#define  COMBOBOX_CONTROL_NAME WIDE("Combo Box")

#define  BUILTIN_CONTROL_COUNT 16 // last known builtin control...
#define  USER_CONTROL   128 // should be sufficiently high as to not conflict with common controls


//};

_MENU_NAMESPACE
typedef struct menuitem_tag *PMENUITEM;
typedef struct menu_tag *PMENU;

#ifndef MENU_DRIVER_SOURCE
typedef struct draw_popup_item_tag 
{
    PTRSZVAL psvUser; // ID param of append menu item
    struct {
        _32 selected : 1;
        _32 checked  : 1;
    } flags;
    union {
        struct {
            _32 width, height;
        } measure;
        struct {
            S_32 x, y;
            _32 width, height;
            Image image;
        } draw;
    };
} DRAWPOPUPITEM, *PDRAWPOPUPITEM;

#endif
_MENU_NAMESPACE_END
USE_MENU_NAMESPACE

//-------- Initialize colors to current windows colors -----------
PSI_PROC( PRENDER_INTERFACE, SetControlInterface)( PRENDER_INTERFACE DisplayInterface );
PSI_PROC( PIMAGE_INTERFACE, SetControlImageInterface )( PIMAGE_INTERFACE DisplayInterface );

PSI_PROC( void, AlignBaseToWindows)( void );
// see indexes above.
PSI_PROC( void, SetBaseColor )( INDEX idx, CDATA c );
PSI_PROC( CDATA, GetBaseColor )( INDEX idx );

//-------- Frame and generic control functions --------------
#ifdef CONTROL_SOURCE
#define MKPFRAME(hvideo) ((PSI_CONTROL)(((PTRSZVAL)(hvideo))|1))
#endif
PSI_PROC( void, SetCommonBorderEx )( PSI_CONTROL pc, _32 BorderType DBG_PASS);
#define SetCommonBorder(pc,b) SetCommonBorderEx(pc,b DBG_SRC)
//PSI_PROC( void, SetCommonBorder )( PSI_CONTROL pc, _32 BorderType );
PSI_PROC( void, SetDrawBorder )( PSI_CONTROL pc ); // update to current border type drawing.
PSI_PROC( PSI_CONTROL, CreateFrame)( CTEXTSTR caption, int x, int y
										, int w, int h
										, _32 BorderFlags
											  , PSI_CONTROL hAbove );

// 1) causes all updates to be done in video thread, otherwise selecting opengl context fails.
// 2) ...
PSI_PROC( void, EnableControlOpenGL )( PSI_CONTROL pc );
//PSI_PROC( void, SetFrameDraw )( PSI_CONTROL pc, void (CPROC*OwnerDraw)(PSI_CONTROL pc) );
//PSI_PROC( void, SetFrameMouse )( PSI_CONTROL pc, void (CPROC*OwnerMouse)(PSI_CONTROL pc, S_32 x, S_32 y, _32 b) );


// Control Init Proc is called each time a control is created
// a control may be created either with a 'make' routine
// or by loading a dialog resource.
#ifdef SOURCE_PSI2
typedef int (CPROC*ControlInitProc)( PSI_CONTROL, va_list );
#else
typedef int (CPROC*ControlInitProc)( PTRSZVAL, PSI_CONTROL, _32 ID );
#endif
typedef int (CPROC*FrameInitProc)( PTRSZVAL, PSI_CONTROL, _32 ID );
PSI_PROC( void, SetFrameInit )( PSI_CONTROL, ControlInitProc, PTRSZVAL );
PSI_PROC( CTEXTSTR, GetControlTypeName)( PSI_CONTROL pc );
// internal routine now exposed... results in a frame from a given
// renderer - a more stable solution than MKPFRAME which
// would require MUCH work to implement all checks everywhere...
PSI_PROC( PSI_CONTROL, CreateFrameFromRenderer )( CTEXTSTR caption
                                                         , _32 BorderTypeFlags
														 , PRENDERER pActImg );
PSI_PROC( PSI_CONTROL, AttachFrameToRenderer )( PSI_CONTROL pcf, PRENDERER pActImg );
// any control on a frame may be passed, and
// the top level
PSI_PROC( PRENDERER, GetFrameRenderer )( PSI_CONTROL );
PSI_PROC( PSI_CONTROL, GetFrameFromRenderer )( PRENDERER renderer );
PSI_PROC( void, GetPhysicalCoordinate )( PSI_CONTROL relative_to, S_32 *_x, S_32 *_y, int include_surface );


//PSI_PROC( void, DestroyFrameEx)( PSI_CONTROL pf DBG_PASS );
#ifdef SOURCE_PSI2
#define DestroyFrame(pf) DestroyCommonEx( pf DBG_SRC )
#else
#define DestroyFrame(pf) DestroyControlEx( pf DBG_SRC )
#endif
PSI_PROC( int, SaveFrame )( PSI_CONTROL pFrame, CTEXTSTR file );
_XML_NAMESPACE

PSI_PROC( void, SetFrameInitProc )( PSI_CONTROL pFrame, ControlInitProc InitProc, PTRSZVAL psvInit );
PSI_PROC( int, SaveXMLFrame )( PSI_CONTROL frame, CTEXTSTR file );

// results with the frame and all controls created
// whatever extra init needs to be done... needs to be done
PSI_PROC( PSI_CONTROL, LoadXMLFrameEx )( CTEXTSTR file DBG_PASS ); // if parent, use DisplayFrameOver()
PSI_PROC( PSI_CONTROL, ParseXMLFrameEx )( POINTER buffer, _32 size DBG_PASS );
PSI_PROC( PSI_CONTROL, LoadXMLFrameOverEx )( PSI_CONTROL frame, CTEXTSTR file DBG_PASS );
#define LoadXMLFrameOver(parent,file) LoadXMLFrameOverEx( parent,file DBG_SRC )
#define LoadXMLFrame(file) LoadXMLFrameEx( file DBG_SRC )
#define ParseXMLFrame(p,s) ParseXMLFrameEx( (p),(s) DBG_SRC )
_XML_NAMESPACE_END
USE_XML_NAMESPACE


PSI_PROC( PSI_CONTROL, LoadFrameFromMemory )( POINTER info, _32 size, PSI_CONTROL hAbove, FrameInitProc InitProc, PTRSZVAL psv  );
PSI_PROC( PSI_CONTROL, LoadFrameFromFile )( FILE *in, PSI_CONTROL hAbove, FrameInitProc InitProc, PTRSZVAL psv  );
PSI_PROC( PSI_CONTROL, LoadFrame )( CTEXTSTR file, PSI_CONTROL hAbove, FrameInitProc InitProc, PTRSZVAL psv );
_PROP_NAMESPACE
PSI_PROC( void, EditFrame )( PSI_CONTROL pf, int bEnable );
_PROP_NAMESPACE_END
PSI_PROC( void, GetFramePosition )( PSI_CONTROL pf, int *x, int *y );
PSI_PROC( void, GetFrameSize )( PSI_CONTROL pf, int *w, int *h );

// results in the total width (left and right) of the frame
PSI_PROC( int, FrameBorderX )( _32 BorderFlags );
// results in left offset of the surface within the frame...
PSI_PROC( int, FrameBorderXOfs )( _32 BorderFlags );
// results in the total height (top and bottom) of frame (and caption)
PSI_PROC( int, FrameBorderY )( PSI_CONTROL pc, _32 BorderFlags, CTEXTSTR caption );
// results in top offset of the surface within the frame...
PSI_PROC( int, FrameBorderYOfs )( PSI_CONTROL pc, _32 BorderFlags, CTEXTSTR caption );

PSI_PROC( int, CaptionHeight )( PSI_CONTROL pf, CTEXTSTR text );


PSI_PROC( void, DisplayFrameOverOn )( PSI_CONTROL pc, PSI_CONTROL over, PRENDERER pActImg );
// stacks the physical display behind this other frame... 
PSI_PROC( void, DisplayFrameUnder )( PSI_CONTROL pc, PSI_CONTROL under );

PSI_PROC( void, DisplayFrameOver )( PSI_CONTROL pc, PSI_CONTROL over );
PSI_PROC( void, DisplayFrameOn )( PSI_CONTROL pc, PRENDERER pActImg );
PSI_PROC( void, DisplayFrame)( PSI_CONTROL pf );
PSI_PROC( void, HideFrame )( PSI_CONTROL pf );
PSI_PROC( void, HideCommon )( PSI_CONTROL pf );
PSI_PROC( LOGICAL, IsControlHidden )( PSI_CONTROL pc );

PSI_PROC( void, RevealCommonEx )( PSI_CONTROL pf DBG_PASS );
#define RevealCommon(pc) RevealCommonEx(pc DBG_SRC );

PSI_PROC( void, SizeCommon)( PSI_CONTROL pf, _32 w, _32 h );
#define SizeControl(c,x,y) SizeCommon((PSI_CONTROL)c,x,y)
#define SizeFrame(c,x,y) SizeCommon((PSI_CONTROL)c,x,y)
PSI_PROC( void, SizeCommonRel)( PSI_CONTROL pf, _32 w, _32 h );
#define SizeControlRel(c,x,y) SizeCommonRel((PSI_CONTROL)c,x,y)
#define SizeFrameRel(c,x,y) SizeCommonRel((PSI_CONTROL)c,x,y)
PSI_PROC( void, MoveCommon)( PSI_CONTROL pf, S_32 x, S_32 y );
#define MoveControl(c,x,y) MoveCommon((PSI_CONTROL)c,x,y)
#define MoveFrame(c,x,y) MoveCommon((PSI_CONTROL)c,x,y)
PSI_PROC( void, MoveCommonRel)( PSI_CONTROL pf, S_32 x, S_32 y );
#define MoveControlRel(c,x,y) MoveCommonRel((PSI_CONTROL)c,x,y)
#define MoveFrameRel(c,x,y) MoveCommonRel((PSI_CONTROL)c,x,y)
PSI_PROC( void, MoveSizeCommon)( PSI_CONTROL pf, S_32 x, S_32 y, _32 width, _32 height );
#define MoveSizeControl(c,x,y,w,h) MoveSizeCommon((PSI_CONTROL)c,x,y,w,h)
#define MoveSizeFrame(c,x,y,w,h) MoveSizeCommon((PSI_CONTROL)c,x,y,w,h)
PSI_PROC( void, MoveSizeCommonRel)( PSI_CONTROL pf, S_32 x, S_32 y, _32 width, _32 height );
#define MoveSizeControlRel(c,x,y,w,h) MoveSizeCommonRel((PSI_CONTROL)c,x,y,w,h)
#define MoveSizeFrameRel(c,x,y,w,h) MoveSizeCommonRel((PSI_CONTROL)c,x,y,w,h)


PSI_PROC( PSI_CONTROL, GetControl )( PSI_CONTROL pContainer, int ID );
#ifdef PSI_SOURCE
//#define GetControl(pc,id) GetControl( &((pc)->common),id)
#define GetControl(pc,id) GetControl( (PSI_CONTROL)(pc),id)
#endif
//PSI_PROC( PSI_CONTROL, GetControl)( PSI_CONTROL pf, int ID );
PSI_PROC( PTRSZVAL, GetCommonUserData )( PSI_CONTROL pf );
#define GetFrameUserData(pf) GetCommonUserData( (PSI_CONTROL)pf )
PSI_PROC( void, SetCommonUserData )( PSI_CONTROL pf, PTRSZVAL psv );
#define SetFrameUserData(pf,d) SetCommonUserData( (PSI_CONTROL)pf,d )


// do the draw to the display...
PSI_PROC( void, UpdateFrameEx )( PSI_CONTROL pf
                                      , int x, int y
										 , int w, int h DBG_PASS );
#define UpdateFrame(pf,x,y,w,h) UpdateFrameEx(pf,x,y,w,h DBG_SRC )

_MOUSE_NAMESPACE
PSI_PROC( void, ReleaseCommonUse )( PSI_CONTROL pc );
PSI_PROC( void, SetFrameMousePosition )( PSI_CONTROL frame, int x, int y );
PSI_PROC( void, CaptureCommonMouse )( PSI_CONTROL pc, LOGICAL bCapture );
_MOUSE_NAMESPACE_END
USE_MOUSE_NAMESPACE

PSI_PROC( Font, GetCommonFontEx )( PSI_CONTROL pc DBG_PASS );
#define GetCommonFont(pc) GetCommonFontEx( pc DBG_SRC )
#define GetFrameFont(pf) GetCommonFont((PSI_CONTROL)pf)
PSI_PROC( void, SetCommonFont )( PSI_CONTROL pc, Font font );
#define SetFrameFont(pf,font) SetCommonFont((PSI_CONTROL)pf,font)

// setting scale of this control immediately scales all contained
// controls, but the control itself remains at it's current size.
PSI_PROC( void, SetCommonScale )( PSI_CONTROL pc, PFRACTION scale_x, PFRACTION scale_y );
// use scale_x and scale_y to scale a, b, results are done in a, b
void ScaleCoords( PSI_CONTROL pc, PS_32 a, PS_32 b );

// bOwn sets the ownership of mouse events to a control, where it remains
// until released.  Some other control has no way to steal it.
PSI_PROC( void, OwnCommonMouse)( PSI_CONTROL pc, int bOwn );

//PSI_PROC void SetDefaultOkayID( PSI_CONTROL pFrame, int nID );
//PSI_PROC void SetDefaultCancelID( PSI_CONTROL pFrame, int nID );

//-------- Generic control functions --------------
PSI_PROC( PSI_CONTROL, GetCommonParent )( PSI_CONTROL pc );
PSI_PROC( PSI_CONTROL, GetFrame)( PSI_CONTROL pc );
#define GetFrame(c) GetFrame((PSI_CONTROL)(c))
PSI_PROC( PSI_CONTROL, GetNearControl)( PSI_CONTROL pc, int ID );
PSI_PROC( void, GetCommonTextEx)( PSI_CONTROL pc, TEXTSTR  buffer, int buflen, int bCString );
#define GetControlTextEx(pc,b,len,str) GetCommonTextEx(pc,b,len,str)
#define GetControlText( pc, buffer, buflen ) GetCommonTextEx( (PSI_CONTROL)(pc), buffer, buflen, FALSE )
#define GetFrameText( pc, buffer, buflen ) GetCommonTextEx( (PSI_CONTROL)(pc), buffer, buflen, FALSE )

PSI_PROC( void, SetCommonText )( PSI_CONTROL pc, CTEXTSTR text );
PSI_PROC( void, SetControlText )( PSI_CONTROL pc, CTEXTSTR text );
PSI_PROC( void, SetFrameText )( PSI_CONTROL pc, CTEXTSTR text );

// set focus to this control,
// it's container which needs to be updated
// is discoverable from the control itself.
PSI_PROC( void, SetCommonFocus)( PSI_CONTROL pc );
PSI_PROC( void, EnableControl)( PSI_CONTROL pc, int bEnable );
PSI_PROC( int, IsControlFocused )( PSI_CONTROL pc );
PSI_PROC( int, IsControlEnabled)( PSI_CONTROL pc );
/*


PSI_PROC( PSI_CONTROL, CreateCommonExx)( PSI_CONTROL pContainer
											  , CTEXTSTR pTypeName
											  , _32 nType
											  , int x, int y
											  , int w, int h
											  , _32 nID
											  , CTEXTSTR caption
											  , _32 ExtraBorderType
											  , PTEXT parameters
											  //, va_list args
												DBG_PASS );
#define CreateCommonEx(pc,nt,x,y,w,h,id,caption) CreateCommonExx(pc,NULL,nt,x,y,w,h,id,caption,0,NULL DBG_SRC)
#define CreateCommon(pc,nt,x,y,w,h,id,caption) CreateCommonExx(pc,NULL,nt,x,y,w,h,id,caption,0,NULL DBG_SRC)
*/

#undef ControlType
PSI_PROC( INDEX, ControlType)( PSI_CONTROL pc );

PSI_PROC( PSI_CONTROL, MakeControl )( PSI_CONTROL pContainer
										  , _32 nType
										  , int x, int y
										  , int w, int h
										  , _32 nID
										  //, ...
										  );

// init is called with an extra parameter on the stack
// works as long as we guarantee C stack call basis...
// the register_control structure allows this override.
PSI_PROC( PSI_CONTROL, MakeControlParam )( PSI_CONTROL pContainer
													  , _32 nType
													  , int x, int y
													  , int w, int h
													  , _32 nID
													  , POINTER param
													  );

// MakePrivateControl passes BORDER_NO_EXTRA_INIT...
PSI_PROC( PSI_CONTROL, MakePrivateControl )( PSI_CONTROL pContainer
													, _32 nType
													, int x, int y
													, int w, int h
													, _32 nID
													//, ...
													);
// MakePrivateControl passes BORDER_NO_EXTRA_INIT...
PSI_PROC( PSI_CONTROL, MakePrivateNamedControl )( PSI_CONTROL pContainer
													, CTEXTSTR pType
													, int x, int y
													, int w, int h
													, _32 nID
													);
PSI_PROC( PSI_CONTROL, MakeCaptionedControl )( PSI_CONTROL pContainer
													  , _32 nType
													  , int x, int y
													  , int w, int h
													  , _32 nID
													  , CTEXTSTR caption
													  //, ...
													  );
PSI_PROC( PSI_CONTROL, VMakeCaptionedControl )( PSI_CONTROL pContainer
														, _32 nType
														, int x, int y
														, int w, int h
														, _32 nID
														, CTEXTSTR caption
														//, va_list args
														);

PSI_PROC( PSI_CONTROL, MakeNamedControl )( PSI_CONTROL pContainer
												 , CTEXTSTR pType
												 , int x, int y
												 , int w, int h
												 , _32 nID
												 //, ...
												 );
PSI_PROC( PCOMMON, MakeNamedCaptionedControlByName )( PCOMMON pContainer
																	 , CTEXTSTR pType
																	 , int x, int y
																	 , int w, int h
																	 , CTEXTSTR pIDName
                                                    , _32 nID // also pass this (if known)
																	 , CTEXTSTR caption
																	 );
PSI_PROC( PSI_CONTROL, MakeNamedCaptionedControl )( PSI_CONTROL pContainer
															 , CTEXTSTR pType
															 , int x, int y
															 , int w, int h
															 , _32 nID
															 , CTEXTSTR caption
															 //, ...
															 );
PSI_PROC( PSI_CONTROL, VMakeControl )( PSI_CONTROL pContainer
											, _32 nType
											, int x, int y
											, int w, int h
											, _32 nID
											//, va_list args
											);

/*
 depricated
 PSI_PROC( PSI_CONTROL, CreateControl)( PSI_CONTROL pFrame
 , int nID
 , int x, int y
 , int w, int h
 , int BorderType
 , int extra );
 */

PSI_PROC( Image,GetControlSurface)( PSI_CONTROL pc );
// result with an image pointer, will sue the image passed
// as prior_image to copy into (resizing if nessecary), if prior_image is NULL
// then a new Image will be returned.  If the surface has not been
// marked as parent_cleaned, then NULL results, as no Original image is
// available.  The image passed as a destination for the surface copy is
// not released, it is resized, and copied into.  THe result may still be NULL
// the image will still be the last valid copy of the surface.
PSI_PROC( Image, CopyOriginalSurface )( PSI_CONTROL pc, Image prior_image );
// this allows the application to toggle the transparency
// characteristic of a control.  If a control is transparent, then it behaves
// automatically as one should using CopyOriginalSurface and restoring that surface
// before doing the draw.  The application does not need to concern itself
// with restoring the prior image, but it must also assume that the entire surface
// has been destroyed, and partial updates are not possible.
PSI_PROC( void, SetCommonTransparent )( PSI_CONTROL pc, LOGICAL bTransparent );

PSI_PROC( void, OrphanCommonEx )( PSI_CONTROL pc, LOGICAL bDraw );
PSI_PROC( void, OrphanCommon )( PSI_CONTROL pc );
#define OrphanFrame(pf) OrphanCommonEx((PSI_CONTROL)pf, FALSE)
#define OrphanControl(pc) OrphanCommonEx((PSI_CONTROL)pc, FALSE)
#define OrphanControlEx(pc,d) OrphanCommonEx((PSI_CONTROL)pc, d)
PSI_PROC( void, AdoptCommonEx )( PSI_CONTROL pFoster, PSI_CONTROL pElder, PSI_CONTROL pOrphan, LOGICAL bDraw );
PSI_PROC( void, AdoptCommon )( PSI_CONTROL pFoster, PSI_CONTROL pElder, PSI_CONTROL pOrphan );
#define AdoptFrame(pff,pfe,pfo) AdoptCommonEx((PSI_CONTROL)pff,(PSI_CONTROL)pfe,(PSI_CONTROL)pfo, TRUE)
#define AdoptControl(pcf,pce,pco) AdoptCommonEx((PSI_CONTROL)pcf,(PSI_CONTROL)pce,(PSI_CONTROL)pco, TRUE)
#define AdoptControlEx(pcf,pce,pco,d) AdoptCommonEx((PSI_CONTROL)pcf,(PSI_CONTROL)pce,(PSI_CONTROL)pco, d)

PSI_PROC( void, SetCommonDraw )( PSI_CONTROL pf, int (CPROC*Draw)( PSI_CONTROL pc ) );
PSI_PROC( void, SetCommonKey )( PSI_CONTROL pf, int (CPROC*Key)(PSI_CONTROL,_32) );
PSI_PROC( void, SetCommonMouse)( PSI_CONTROL pc, int (CPROC*MouseMethod)(PSI_CONTROL, S_32 x, S_32 y, _32 b ) );
PSI_PROC( void, AddCommonDraw )( PSI_CONTROL pf, int (CPROC*Draw)( PSI_CONTROL pc ) );
PSI_PROC( void, AddCommonKey )( PSI_CONTROL pf, int (CPROC*Key)(PSI_CONTROL,_32) );
PSI_PROC( void, AddCommonMouse)( PSI_CONTROL pc, int (CPROC*MouseMethod)(PSI_CONTROL, S_32 x, S_32 y, _32 b ) );

PSI_PROC( void, SetCommonAcceptDroppedFiles)( PSI_CONTROL pc, void (CPROC*AcceptDroppedFilesMethod)(PSI_CONTROL, CTEXTSTR file, S_32 x, S_32 y ) );
PSI_PROC( void, AddCommonAcceptDroppedFiles)( PSI_CONTROL pc, void (CPROC*AcceptDroppedFilesMethod)(PSI_CONTROL, CTEXTSTR file, S_32 x, S_32 y ) );

PSI_PROC( void, SetCommonSave)( PSI_CONTROL pc, void (CPROC*)(int PSI_CONTROL) );
#define SetControlSave(pc,mm)   SetCommonSave((PSI_CONTROL)pc,(void (CPROC*)(int, PSI_CONTROL))mm)
#define SetFrameSave(pc,mm)     SetCommonSave((PSI_CONTROL)pc,(void (CPROC*)(int, PSI_CONTROL))mm)
PSI_PROC( void, SetCommonLoad)( PSI_CONTROL pc, void (CPROC*)(int PSI_CONTROL) );
#define SetControlLoad(pc,mm)   SetCommonLoad((PSI_CONTROL)pc,(void (CPROC*)(int, PSI_CONTROL))mm)
#define SetFrameLoad(pc,mm)     SetCommonLoad((PSI_CONTROL)pc,(void (CPROC*)(int, PSI_CONTROL))mm)

// ---
// restore background restores the prior background of the control
// so that semi-opaque controls can draw over the correct surface.
PSI_PROC( void, RestoreBackground )( PSI_CONTROL pc, P_IMAGE_RECTANGLE r );
// --
// output to the physical surface the rectangle of the control's surface specified.
PSI_PROC( void, UpdateSomeControls )( PSI_CONTROL pc, P_IMAGE_RECTANGLE pRect );

PSI_PROC( void, SetUpdateRegionEx )( PSI_CONTROL pc, S_32 rx, S_32 ry, _32 rw, _32 rh DBG_PASS );
#define SetUpdateRegion(pc,x,y,w,h) SetUpdateRegionEx( pc,x,y,w,h DBG_SRC )


PSI_PROC( void, EnableCommonUpdates )( PSI_CONTROL frame, int bEnable );
#define EnableFrameUpdates(pf,e) EnableCommonUpdates( (PSI_CONTROL)pf, e )
#define EnableControlUpdates(pc,e) EnableCommonUpdates( (PSI_CONTROL)pc, e )

//PSI_PROC void SetControlKey( PSI_CONTROL pc, void (*KeyMethod)( PSI_CONTROL pc, int key ) );
PSI_PROC( void, UpdateCommonEx )( PSI_CONTROL pc, int bDraw DBG_PASS );
PSI_PROC( void, SmudgeCommonAreaEx )( PSI_CONTROL pc, P_IMAGE_RECTANGLE rect DBG_PASS );
#define SmudgeCommonArea( pc, area ) SmudgeCommonAreaEx( pc, area DBG_SRC )
PSI_PROC( void, SmudgeCommonEx )( PSI_CONTROL pc DBG_PASS );
#define SmudgeCommon(pc) SmudgeCommonEx( pc DBG_SRC )
//#ifdef SOURCE_PSI2
#define UpdateCommon(pc) SmudgeCommon(pc)
//#else
//#define UpdateCommon(pc) UpdateCommonEx(pc,TRUE DBG_SRC)
//#endif
//#define UpdateControlEx(pc,draw) UpdateCommonEx( (PSI_CONTROL)pc, draw )
//#define UpdateFrameEx(pc,draw)   UpdateCommonEx( (PSI_CONTROL)pc, draw )

PSI_PROC( void, UpdateControlEx)( PSI_CONTROL pc DBG_PASS );
#define UpdateControl(pc) UpdateControlEx( pc DBG_SRC )
PSI_PROC( int, GetControlID)( PSI_CONTROL pc );
PSI_PROC( void, SetControlID )( PSI_CONTROL pc, int ID );

PSI_PROC( void, DestroyCommonEx)( PSI_CONTROL *ppc DBG_PASS);
#define DestroyCommon(ppc) DestroyCommonEx(ppc DBG_SRC )
PSI_PROC( void, DestroyControlEx)( PSI_CONTROL pc DBG_PASS);
#define DestroyControl(pc) DestroyControlEx( pc DBG_SRC )
PSI_PROC( void, SetNoFocus)( PSI_CONTROL pc );
PSI_PROC( void *, ControlExtraData)( PSI_CONTROL pc );
_PROP_NAMESPACE
PSI_PROC( int, EditControlProperties )( PSI_CONTROL control );
PSI_PROC( int, EditFrameProperties )( PSI_CONTROL frame, S_32 x, S_32 y );
_PROP_NAMESPACE_END
USE_PROP_NAMESPACE

//------ General Utilities ------------
// adds OK and Cancel buttons based off the 
// bottom right of the frame, and when pressed set
// either done (cancel) or okay(OK) ... 
// could do done&ok or just done - but we're bein cheesey
PSI_PROC( void, AddCommonButtonsEx)( PSI_CONTROL pf
                                , int *done, CTEXTSTR donetext
                                , int *okay, CTEXTSTR okaytext );
PSI_PROC( void, AddCommonButtons)( PSI_CONTROL pf, int *done, int *okay );
PSI_PROC( void, SetCommonButtons)( PCOMMON pf, int *pdone, int *pokay );
PSI_PROC( void, InitCommonButton )( PSI_CONTROL pc, int *value );

PSI_PROC( void, CommonLoop)( int *done, int *okay ); // perhaps give a callback for within the loop?
PSI_PROC( void, CommonWait)( PSI_CONTROL pf ); // perhaps give a callback for within the loop?
PSI_PROC( void, CommonWaitEndEdit)( PSI_CONTROL *pf ); // a frame in edit mode, once edit mode done, continue
PSI_PROC( void, ProcessControlMessages)(void);
//------ BUTTONS ------------
_BUTTON_NAMESPACE
#define BUTTON_NO_BORDER 0x0001
//CONTROL_DEFINE( Button );
typedef void (CPROC *ButtonPushMethod)(PTRSZVAL,PSI_CONTROL);
typedef void (CPROC*ButtonDrawMethod)(PTRSZVAL psv, PSI_CONTROL pc);
CONTROL_PROC(Button,(CTEXTSTR,void (CPROC*PushMethod)(PTRSZVAL psv, PSI_CONTROL pc)
						  , PTRSZVAL Data));

// this method invokes the button push method...
PSI_PROC( void, InvokeButton )( PSI_CONTROL pc );
PSI_PROC( void, GetButtonPushMethod )( PSI_CONTROL pc, ButtonPushMethod *method, PTRSZVAL *psv );
PSI_PROC( PSI_CONTROL, SetButtonPushMethod )( PSI_CONTROL pc, ButtonPushMethod method, PTRSZVAL psv );
PSI_PROC( PSI_CONTROL, SetButtonAttributes )( PSI_CONTROL pc, int attr ); // BUTTON_ flags...
PSI_PROC( PSI_CONTROL, SetButtonDrawMethod )( PSI_CONTROL pc, ButtonDrawMethod method, PTRSZVAL psv );
// drop a - attr is a user private thing...
#define MakeButton(f,x,y,w,h,id,c,a,p,d) SetButtonAttributes( SetButtonPushMethod( MakeCaptionedControl(f,NORMAL_BUTTON,x,y,w,h,id,c), p, d ), a )

//CONTROL_PROC( ImageButton, (Image pImage
//									, void (CPROC*PushMethod)(PTRSZVAL psv, PSI_CONTROL pc)
//									, PTRSZVAL Data) );
PCONTROL PSIMakeImageButton( PCOMMON parent, int x, int y, int w, int h, _32 ID
							 , Image pImage
							 , void (CPROC*PushMethod)(PTRSZVAL psv, PCONTROL pc)
							 , PTRSZVAL Data );
// drop a - attr is a user private thing...
#define MakeImageButton(f,x,y,w,h,id,c,a,p,d) SetButtonPushMethod( SetButtonAttributes( SetButtonImage( MakeControl(f,IMAGE_BUTTON,x,y,w,h,id),c),a),p,d)
PSI_PROC( PSI_CONTROL, SetButtonImage )( PSI_CONTROL pc, Image image );

#define MakeCustomDrawnButton(f,x,y,w,h,id,a,dr,p,d) SetButtonPushMethod( SetButtonDrawMethod( SetButtonAttributes( MakeControl(f,CUSTOM_BUTTON,x,y,w,h,id),a ), dr, d ), p,d)


PSI_PROC( void, PressButton)( PSI_CONTROL pc, int bPressed );
PSI_PROC( void, SetButtonFont)( PSI_CONTROL pc, Font font );
PSI_PROC( int, IsButtonPressed)( PSI_CONTROL pc );
PSI_PROC( PSI_CONTROL, SetButtonGroupID )(PSI_CONTROL pc, int nGroupID );
PSI_PROC( PSI_CONTROL, SetButtonCheckedProc )( PSI_CONTROL pc
														, void (CPROC*CheckProc)(PTRSZVAL psv, PSI_CONTROL pc)
														, PTRSZVAL psv );
#define RADIO_CALL_ALL 0
#define RADIO_CALL_CHECKED   1
#define RADIO_CALL_UNCHECKED 2
#define RADIO_CALL_CHANGED   (RADIO_CALL_CHECKED|RADIO_CALL_UNCHECKED)

#define MakeRadioButton(f,x,y,w,h,id,t,gr,a,p,d) SetCheckButtonHandler( SetButtonGroupID( SetButtonAttributes( MakeCaptionedControl(f,RADIO_BUTTON,x,y,w,h,id,t), a ), gr ), p, d )
//CONTROL_PROC( RadioButton, (_32 GroupID, CTEXTSTR text
//                                , void (CPROC*CheckProc)(PTRSZVAL psv, PSI_CONTROL pc)
//              , PTRSZVAL psv) );

// check buttons are radio buttons with GroupID == 0
//CONTROL_PROC( CheckButton, (CTEXTSTR text
//                        , void (CPROC*CheckProc)(PTRSZVAL psv, PSI_CONTROL pc)
//									, PTRSZVAL psv) );
//PSI_CONTROL PSIMakeCheckButton( PSI_CONTROL parent, int x, int y, int w, int h, _32 ID
//                           , _32 attr
//									, CTEXTSTR text
//									, void (CPROC*CheckProc)(PTRSZVAL psv, PSI_CONTROL pc)
//									, PTRSZVAL psv );
#define MakeCheckButton(f,x,y,w,h,id,t,a,p,d) SetCheckButtonHandler( SetButtonAttributes( SetButtonGroupID( MakeCaptionedControl(f,RADIO_BUTTON,x,y,w,h,id,t),0),a),p,d)
PSI_PROC( PSI_CONTROL, SetRadioButtonGroup )( PSI_CONTROL, int group_id );
PSI_PROC( PSI_CONTROL, SetCheckButtonHandler )( PSI_CONTROL
															 , void (CPROC*CheckProc)(PTRSZVAL psv, PSI_CONTROL pc)
															 , PTRSZVAL psv );
PSI_PROC( int, GetCheckState)( PSI_CONTROL pc );
PSI_PROC( void, SetCheckState)( PSI_CONTROL pc, int nState );
// set the button's background color...
PSI_PROC( void, SetButtonColor )( PSI_CONTROL pc, CDATA color );

_BUTTON_NAMESPACE_END
USE_BUTTON_NAMESPACE

//------ Static Text -----------
_TEXT_NAMESPACE
#define TEXT_NORMAL     0x00
#define TEXT_VERTICAL   0x01
#define TEXT_CENTER     0x02
#define TEXT_RIGHT      0x04
#define TEXT_FRAME_BUMP 0x10
#define TEXT_FRAME_DENT 0x20
#define MakeTextControl( pf,x,y,w,h,id,text,flags ) SetTextControlAttributes( MakeCaptionedControl( pf, STATIC_TEXT, x, y, w,h, id, text ), flags )
PSI_PROC( void, SetControlAlignment )( PCOMMON pc, int align );

// offset is a pixel specifier... +/- amount from it's normal position.
// returns if any text remains visible (true is visible, false is no longer visible, allowing
// upper level application to reset to 0 offset.
PSI_PROC( LOGICAL, SetControlTextOffset )( PCOMMON pc, int offset );
PSI_PROC( LOGICAL, GetControlTextOffsetMinMax )( PCOMMON pc, int *min_offset, int *max_offset );

CAPTIONED_CONTROL_PROC( TextControl, (void) );
PSI_PROC( PSI_CONTROL, SetTextControlAttributes )( PSI_CONTROL pc, int flags );
PSI_PROC( void, SetTextControlColors )( PSI_CONTROL pc, CDATA fore, CDATA back );
_TEXT_NAMESPACE_END
USE_TEXT_NAMESPACE

//------- Edit Control ---------
_EDIT_NAMESPACE
#define EDIT_READONLY 0x01
#define EDIT_PASSWORD 0x02
CAPTIONED_CONTROL_PROC( EditControl, (CTEXTSTR text) );
PSI_PROC( void, TypeIntoEditControl )( PSI_CONTROL control, CTEXTSTR text );

#define MakeEditControl(f,x,y,w,h,id,t,a) SetEditControlPassword( SetEditControlReadOnly( MakeCaptionedControl( f,EDIT_FIELD,x,y,w,h,id,t ) \
	, ( a & EDIT_READONLY)?TRUE:FALSE )   \
	, ( a & EDIT_PASSWORD)?TRUE:FALSE )
PSI_PROC( PSI_CONTROL, SetEditControlReadOnly )( PSI_CONTROL pc, LOGICAL bReadOnly );
PSI_PROC( PSI_CONTROL, SetEditControlPassword )( PSI_CONTROL pc, LOGICAL bPassword );
//PSI_PROC( void, SetEditAttributes )( pc, a );
#define SetEditFont SetCommonFont
//PSI_PROC( void, SetEditFont )( PSI_CONTROL pc, Font font );
// Use GetControlText/SetControlText
_EDIT_NAMESPACE_END
USE_EDIT_NAMESPACE

//------- Slider Control --------
_SLIDER_NAMESPACE
#define SLIDER_HORIZ 1
#define SLIDER_VERT  0
#define MakeSlider(pf,x,y,w,h,nID,opt,updateproc,updateval) SetSliderOptions( SetSliderUpdateHandler( MakeControl(pf,SLIDER_CONTROL,x,y,w,h,nID ), updateproc,updateval ), opt)
//CONTROL_PROC( Slider, (
typedef void (CPROC*SliderUpdateProc)(PTRSZVAL psv, PSI_CONTROL pc, int val);
PSI_PROC( void, SetSliderValues)( PSI_CONTROL pc, int min, int current, int max );
PSI_PROC( PSI_CONTROL, SetSliderOptions )( PSI_CONTROL pc, int opts );
PSI_PROC( PSI_CONTROL, SetSliderUpdateHandler )( PSI_CONTROL pc, SliderUpdateProc, PTRSZVAL psvUser );
_SLIDER_NAMESPACE_END
USE_SLIDER_NAMESPACE

//------- Font Control --------
_FONTS_NAMESPACE
#ifndef FONT_RENDER_SOURCE
// types of data which may result...
//typedef struct font_data_tag *PFONTDATA;
//typedef struct render_font_data_tag *PRENDER_FONTDATA;
#endif

// common dialog to get a font which is then available for all
// Image text operations (PutString, PutCharacter)
// result font selection data can be resulted in the area referenced by
// the pointer, and size pointer...

// actual work done here for pUpdateFontFor(NULL) ...
// if pUpdateFontFor is not null, an apply button will appear, allowing the actual
// control to be updated to the chosen font.

// scale_width, height magically apply... and are saved in the
// font data structure for re-rendering... if a font is rendered here
// the same exact font result will be acheived using RenderScaledFont( pdata )
PSI_PROC( Font, PickScaledFontWithUpdate )( S_32 x, S_32 y
														, PFRACTION width_scale
														, PFRACTION height_scale
														, P_32 pFontDataSize
														 // resulting parameters for the data and size of data
														 // which may be passe dto RenderFontData
														, POINTER *pFontData
														, PCOMMON pAbove
														, void (CPROC *UpdateFont)( PTRSZVAL psv, Font font )
														, PTRSZVAL psvUpdate );

PSI_PROC( Font, PickFontWithUpdate )( S_32 x, S_32 y
										  , P_32 pFontDataSize
											// resulting parameters for the data and size of data
											// which may be passe dto RenderFontData
										  , POINTER *pFontData
										  , PCOMMON pAbove
										  , void (CPROC *UpdateFont)( PTRSZVAL psv, Font font )
												, PTRSZVAL psvUpdate );
// pick font for uses pickfontwithupdate where the update procedure
// sets the font on a control.
PSI_PROC( Font, PickFontFor )( S_32 x, S_32 y
								  , P_32 pFontDataSize
									// resulting parameters for the data and size of data
                           // which may be passe dto RenderFontData
								  , POINTER *pFontData
									  , PSI_CONTROL pAbove
									  , PSI_CONTROL pUpdateFontFor );

PSI_PROC( Font, PickFont)( S_32 x, S_32 y
                                  , P_32 size, POINTER *pFontData
								 , PSI_CONTROL pAbove );
#define PickScaledFont( x,y,ws,hs,size,fd,pa) PickScaledFontWithUpdate( x,y,ws,hs,size,fd,pa,NULL,0)

// this can take the resulting data from a pick font operation
// and result in a font... concerns at the moment are for cases
// of trying to use the same  string on different systems (different font
// locations) and getting a same (even similar?) result
//PSI_PROC( Font, RenderFont)( POINTER data, _32 flags ); // flags ? override?
//PSI_PROC( Font, RenderFontData )( PFONTDATA pfd );
//PSI_PROC( Font, RenderFontFileEx )( CTEXTSTR file, _32 width, _32 height, _32 flags, P_32 size, POINTER *pFontData );
//PSI_PROC( Font, RenderFontFile )( CTEXTSTR file, _32 width, _32 height, _32 flags );
//#define RenderFontFile(file,w,h,flags) RenderFontFileEx(file,w,h,flags,NULL,NULL)
//PSI_PROC( void, DestroyFont)( Font *font );
// takes an already rendered font, and writes it to a file.
// at the moment this will not work with display services....
PSI_PROC( void, DumpFontFile )( CTEXTSTR name, Font font );
PSI_PROC( void, DumpFrameContents )( PCOMMON pc );

_FONTS_NAMESPACE_END
USE_FONTS_NAMESPACE

//------- ListBox Control --------
_LISTBOX_NAMESPACE
typedef struct listitem_tag *PLISTITEM;
#define LISTOPT_TREE   2
#define LISTOPT_SORT   1
CONTROL_PROC( ListBox, (void) );
#define MakeListBox(pf,x,y,w,h,nID,opt) SetListboxIsTree( MakeControl(pf,LISTBOX_CONTROL,x,y,w,h,nID), (opt & LISTOPT_TREE)?TRUE:FALSE )
PSI_PROC( PSI_CONTROL, SetListboxIsTree )( PSI_CONTROL pc, int bTree );
#define LISTBOX_SORT_NORMAL 1
#define LISTBOX_SORT_DISABLE 0
PSI_PROC( PSI_CONTROL, SetListboxSort )( PSI_CONTROL pc, int bSortTrue ); // may someday add SORT_INVERSE?
PSI_PROC( PSI_CONTROL, SetListboxMultiSelect )( PSI_CONTROL, int bEnable );
PSI_PROC( PSI_CONTROL, SetListboxMultiSelectEx )( PSI_CONTROL, int bEnable, int bLazy );
PSI_PROC( int, GetListboxMultiSelectEx )( PSI_CONTROL, int *multi, int *lazy );
PSI_PROC( int, GetListboxMultiSelect )( PSI_CONTROL ); // returns only multiselect option, not lazy with multselect
PSI_PROC( void, ResetList)( PSI_CONTROL pc );
// put an item at end of list.
PSI_PROC( PLISTITEM, AddListItem)( PSI_CONTROL pc, const CTEXTSTR text );
PSI_PROC( PLISTITEM, AddListItemEx)( PSI_CONTROL pc, int nLevel, CTEXTSTR text );
// put an item after a known item... NULL to add at head of list.
PSI_PROC( PLISTITEM, InsertListItem)( PSI_CONTROL pc, PLISTITEM pAfter, CTEXTSTR text );
PSI_PROC( PLISTITEM, InsertListItemEx)( PSI_CONTROL pc, PLISTITEM pAfter, int nLevel, CTEXTSTR text );
PSI_PROC( void, DeleteListItem)( PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC( PLISTITEM, SetItemData)( PLISTITEM hli, PTRSZVAL psv );
PSI_PROC( PTRSZVAL, GetItemData)( PLISTITEM hli );
PSI_PROC( void, GetListItemText)( PLISTITEM hli, TEXTSTR buffer, int bufsize );
/* depreicated, use GetListItemText instead, please */
PSI_PROC( void, GetItemText)( PLISTITEM hli, int bufsize, TEXTSTR buffer );
#define GetItemText(hli,bufsize,buf) GetListItemText(hli,buf,bufsize)
PSI_PROC( void, SetItemText )( PLISTITEM hli, CTEXTSTR buffer );
PSI_PROC( PLISTITEM, GetSelectedItem)( PSI_CONTROL pc );
PSI_PROC( void, ClearSelectedItems )( PSI_CONTROL plb );

PSI_PROC( void, SetSelectedItem)( PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC( void, SetItemSelected)( PSI_CONTROL pc, PLISTITEM hli, int bSelect );
PSI_PROC( void, SetCurrentItem)( PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC( PLISTITEM, FindListItem)( PSI_CONTROL pc, CTEXTSTR text );
PSI_PROC( PLISTITEM, GetNthTreeItem )( PSI_CONTROL pc, PLISTITEM pli, int level, int idx );
PSI_PROC( PLISTITEM, GetNthItem )( PSI_CONTROL pc, int idx );
typedef void (CPROC*SelectionChanged )( PTRSZVAL psvUser, PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC( void, SetSelChangeHandler)( PSI_CONTROL pc, SelectionChanged proc, PTRSZVAL psvUser );
typedef void (CPROC*DoubleClicker)( PTRSZVAL psvUser, PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC( void, SetDoubleClickHandler)( PSI_CONTROL pc, DoubleClicker proc, PTRSZVAL psvUser );
// if bopened - branch is being expanded, else it is being closed (collapsed)
typedef void (CPROC*ListItemOpened)( PTRSZVAL psvUser, PSI_CONTROL pc, PLISTITEM hli, LOGICAL bOpened );
PSI_PROC( void, SetListItemOpenHandler )( PSI_CONTROL pc, ListItemOpened proc, PTRSZVAL psvUser );
// returns the prior state of disabledness
PSI_PROC( int, DisableUpdateListBox )( PSI_CONTROL pc, LOGICAL bDisable );
// on right click down,up this proc is triggered...
PSI_PROC( void, SetItemContextMenu )( PLISTITEM pli, PMENU pMenu, void (CPROC*MenuProc)(PTRSZVAL, PLISTITEM, _32 menuopt ), PTRSZVAL psv );
PSI_PROC( int, OpenListItem )( PLISTITEM pli, int bOpen );
PSI_PROC( void, SetListBoxTabStops )( PSI_CONTROL pc, int nStops, int *pStops );
PSI_PROC( void, EnumListItems )( PSI_CONTROL pc
										 , PLISTITEM pliStart
										 , void (CPROC *HandleListItem )(PTRSZVAL,PSI_CONTROL,PLISTITEM)
										 , PTRSZVAL psv );
PSI_PROC( void, EnumSelectedListItems )( PCONTROL pc
													, PLISTITEM pliStart
													, void (CPROC *HandleListItem )(PTRSZVAL,PCOMMON,PLISTITEM)
													, PTRSZVAL psv );
PSI_PROC( PSI_CONTROL, GetItemListbox )( PLISTITEM pli );

PSI_PROC( void, MoveListItemEx )( PSI_CONTROL pc, PLISTITEM pli, int level_direction, int direction );
PSI_PROC( void, MoveListItem )( PSI_CONTROL pc, PLISTITEM pli, int direction );

_LISTBOX_NAMESPACE_END
USE_LISTBOX_NAMESPACE

//------- GridBox Control --------
#ifdef __LINUX__
typedef PTRSZVAL HGRIDITEM;
PSI_PROC(PSI_CONTROL, MakeGridBox)( PSI_CONTROL pf, int options, int x, int y, int w, int h,
                                 int viewport_x, int viewport_y, int total_x, int total_y,
                                 int row_thickness, int column_thickness, PTRSZVAL nID );
#endif
//------- Popup Menus ------------
// popup interface.... these are mimics of windows... 
// and at some point should internally alias to popup code - 
//    if I ever get it back

_MENU_NAMESPACE



PSI_PROC( PMENU, CreatePopup)( void );
PSI_PROC( void, DestroyPopup)( PMENU pm );
PSI_PROC( void, ResetPopup)( PMENU pm );
// get sub-menu data...
PSI_PROC( void *,GetPopupData)( PMENU pm, int item );
PSI_PROC( PMENUITEM, AppendPopupItem)( PMENU pm, int type, PTRSZVAL dwID, CPOINTER pData );
PSI_PROC( PMENUITEM, CheckPopupItem)( PMENU pm, _32 dwID, _32 state );
PSI_PROC( PMENUITEM, DeletePopupItem)( PMENU pm, _32 dwID, _32 state );
PSI_PROC( int, TrackPopup)( PMENU hMenuSub, PSI_CONTROL parent );
_MENU_NAMESPACE_END
USE_MENU_NAMESPACE

//------- File Selector Control --------
// these are basic basic file selection dialogs... 
	// the concept is valid, and they should be common like controls...
	// types are tab sepeared list of default extensions to open.
	// returns TRUE if the filename is selected and the result buffer is filled.
   // returns FALSE if the filename selection is canceled, result is not modified.
	// if bcreate is used, then the filename does not HAVE to exist..
   // bCreate may also be read as !bMustExist
PSI_PROC( int, PSI_PickFile)( PSI_CONTROL parent, CTEXTSTR basepath, CTEXTSTR types, TEXTSTR result, _32 result_len, int bCreate );
PSI_PROC( int, PSI_OpenFile)( CTEXTSTR basepath, CTEXTSTR types, TEXTSTR result );
// this may be used for save I think....
//PSI_PROC( int, PSI_OpenFileEx)( PSI_CONTROL parent, CTEXTSTR basepath, CTEXTSTR types, CTEXTSTR result, _32 result_len, int Create );

//------- Scroll Control --------
#define SCROLL_HORIZONTAL 1
#define SCROLL_VERITCAL   0
_SCROLLBAR_NAMESPACE
   // types of UpdateProc and for MoveScrollBar
#define UPD_1UP       0
#define UPD_1DOWN     1
#define UPD_RANGEUP   2
#define UPD_RANGEDOWN 3
#define UPD_THUMBTO   4

PSI_PROC( void, SetScrollParams)( PSI_CONTROL pc, int min, int cur, int range, int max );
CONTROL_PROC( ScrollBar, (_32 attr) );
PSI_PROC( void, SetScrollUpdateMethod)( PSI_CONTROL pc
                    , void (CPROC*UpdateProc)(PTRSZVAL psv, int type, int current)
                                                  , PTRSZVAL data );
PSI_PROC( void, MoveScrollBar )( PSI_CONTROL pc, int type );
_SCROLLBAR_NAMESPACE_END
USE_SCROLLBAR_NAMESPACE

//------- Misc Controls (and widgets) --------
_SHEETS_NAMESPACE
#define MakeSheetControl(c,x,y,w,h,id) MakeControl(c,SHEET_CONTROL,x,y,w,h,id)
//CONTROL_PROC( SheetControl, (void) );
PSI_PROC( void, AddSheet )( PSI_CONTROL pControl, PSI_CONTROL contents );
PSI_PROC( int, RemoveSheet )( PSI_CONTROL pControl, _32 ID );
PSI_PROC( PSI_CONTROL, GetSheet )( PSI_CONTROL pControl, _32 ID );
PSI_PROC( PSI_CONTROL, GetSheetControl )( PSI_CONTROL pControl, _32 IDSheet, _32 IDControl );
PSI_PROC( PSI_CONTROL, GetCurrentSheet )( PSI_CONTROL pControl );
PSI_PROC( void, SetCurrentSheet )( PSI_CONTROL pControl, _32 ID );
PSI_PROC( int, GetSheetSize )( PSI_CONTROL pControl, _32 *width, _32 *height );
PSI_PROC( void, DisableSheet )( PSI_CONTROL pControl, _32 ID, LOGICAL bDisable );

// Tab images are sliced and diced across the vertical center of the image
// the center is then spread as wide as the caption requires.
// set default tabs for the sheet control itself
PSI_PROC( void, SetTabImages )( PSI_CONTROL pControl, Image active, Image inactive );
// set tab images on a per-sheet basis, overriding the defaults specified.
PSI_PROC( void, SetSheetTabImages )( PSI_CONTROL pControl, _32 ID, Image active, Image inactive );
// with the ability to set the image for the tab, suppose it would be
// wise to set the sheet's text color on the tab...
// Initial tabs are black and white, with inverse black/white text...
PSI_PROC( void, SetTabTextColors )( PSI_CONTROL pControl, CDATA cActive, CDATA cInactive );
PSI_PROC( void, SetSheetTabTextColors )( PSI_CONTROL pControl, _32 ID, CDATA cActive, CDATA cInactive );
_SHEETS_NAMESPACE_END
USE_SHEETS_NAMESPACE


//------- Misc Controls (and widgets) --------
	PSI_PROC( void, SimpleMessageBox )( PSI_CONTROL parent, CTEXTSTR title, CTEXTSTR content );
// result is the address of a user buffer to read into, reslen is the size of that buffer.
// question is put above the question... pAbove is the window which this one should be placed above (lock-stacked)
PSI_PROC( int, SimpleUserQuery )( TEXTSTR result, int reslen, CTEXTSTR question, PCOMMON pAbove );

PSI_PROC( void, RegisterResource )( CTEXTSTR appname, CTEXTSTR resource_name, int ID, int resource_name_range, CTEXTSTR type_name );
// assuming one uses a
#define SimpleRegisterResource( name, typename ) RegisterResource( WIDE("application"), WIDE(#name), name, 1, typename );
#define EasyRegisterResource( domain, name, typename ) RegisterResource( domain, WIDE(#name), name, 1, typename );
#define EasyRegisterResourceRange( domain, name, range, typename ) RegisterResource( domain, WIDE(#name), name, range, typename );
#define SimpleRegisterAppResource( name, typename, class ) RegisterResource( WIDE("application/") class, WIDE(#name), name, 1, typename );

//------- INI Prompt for option library (static)
#ifdef __STATIC__
PSI_PROC( int, _SQLPromptINIValue )(
												CTEXTSTR lpszSection,
												CTEXTSTR lpszEntry,
												CTEXTSTR lpszDefault,
												TEXTSTR lpszReturnBuffer,
												int cbReturnBuffer,
												CTEXTSTR filename
											  );
#endif



#define PSI_PROC_PTR( type, name ) type (CPROC*name)
typedef struct control_interface_tag
{

PSI_PROC_PTR( PRENDER_INTERFACE, SetControlInterface)( PRENDER_INTERFACE DisplayInterface );
PSI_PROC_PTR( PIMAGE_INTERFACE, SetControlImageInterface )( PIMAGE_INTERFACE DisplayInterface );

PSI_PROC_PTR( void, AlignBaseToWindows)( void );
// see indexes above.
PSI_PROC_PTR( void, SetBaseColor )( INDEX idx, CDATA c );
PSI_PROC_PTR( CDATA, GetBaseColor )( INDEX idx );

//-------- Frame and generic control functions --------------
PSI_PROC_PTR( PSI_CONTROL, CreateFrame)( CTEXTSTR caption, int x, int y
                                        , int w, int h
                                        , _32 BorderFlags
                                        , PSI_CONTROL hAbove );

PSI_PROC_PTR( PSI_CONTROL, CreateFrameFromRenderer )( CTEXTSTR caption
                                                         , _32 BorderTypeFlags
                                                         , PRENDERER pActImg );
PSI_PROC_PTR( void, DestroyFrameEx)( PSI_CONTROL pf DBG_PASS );

PSI_PROC_PTR( int, FrameBorderX )( _32 flags );
PSI_PROC_PTR( int, FrameBorderXOfs )( _32 flags );
PSI_PROC_PTR( int, FrameBorderY )( _32 flags, CTEXTSTR caption );
PSI_PROC_PTR( int, FrameBorderYOfs )( _32 flags, CTEXTSTR caption );
PSI_PROC_PTR( void, DisplayFrame)( PSI_CONTROL pf );
PSI_PROC_PTR( void, SizeCommon)( PSI_CONTROL pf, _32 w, _32 h );
PSI_PROC_PTR( void, SizeCommonRel)( PSI_CONTROL pf, _32 w, _32 h );
PSI_PROC_PTR( void, MoveCommon)( PSI_CONTROL pf, S_32 x, S_32 y );
PSI_PROC_PTR( void, MoveCommonRel)( PSI_CONTROL pf, S_32 x, S_32 y );
PSI_PROC_PTR( void, MoveSizeCommon)( PSI_CONTROL pf, S_32 x, S_32 y, _32 w, _32 h );
PSI_PROC_PTR( void, MoveSizeCommonRel)( PSI_CONTROL pf, S_32 x, S_32 y, _32 w, _32 h );
PSI_PROC_PTR( PSI_CONTROL, GetControl)( PSI_CONTROL pf, int ID );
PSI_PROC_PTR( PTRSZVAL, GetFrameUserData )( PSI_CONTROL pf );
PSI_PROC_PTR( void, SetFrameUserData )( PSI_CONTROL pf, PTRSZVAL psv );
PSI_PROC_PTR( void, UpdateFrame )( PSI_CONTROL pf
                                            , int x, int y
                                            , int w, int h );
PSI_PROC_PTR( void, SetFrameMousePosition )( PSI_CONTROL frame, int x, int y );

//PSI_PROC_PTR void SetDefaultOkayID( PSI_CONTROL pFrame, int nID );
//PSI_PROC_PTR void SetDefaultCancelID( PSI_CONTROL pFrame, int nID );

//-------- Generic control functions --------------
PSI_PROC_PTR( PSI_CONTROL, GetFrame)( PSI_CONTROL pc );
PSI_PROC_PTR( PSI_CONTROL, GetNearControl)( PSI_CONTROL pc, int ID );
PSI_PROC_PTR( void, GetControlTextEx)( PSI_CONTROL pc, TEXTSTR buffer, int buflen, int bCString );
PSI_PROC_PTR( void, SetControlText)( PSI_CONTROL pc, CTEXTSTR text );
PSI_PROC_PTR( void, SetControlFocus)( PSI_CONTROL pf, PSI_CONTROL pc );
PSI_PROC_PTR( void, EnableControl)( PSI_CONTROL pc, int bEnable );
PSI_PROC_PTR( int, IsControlEnabled)( PSI_CONTROL pc );
PSI_PROC_PTR( Image,GetControlSurface)( PSI_CONTROL pc );
PSI_PROC_PTR( void, SetCommonDraw )( PSI_CONTROL pf, void (CPROC*Draw)( PTRSZVAL, PSI_CONTROL pc ), PTRSZVAL psv );
PSI_PROC_PTR( void, SetCommonKey )( PSI_CONTROL pf, void (CPROC*Key)(PTRSZVAL,_32), PTRSZVAL psv );
PSI_PROC_PTR( void, SetCommonMouse)( PSI_CONTROL pc, void (CPROC*MouseMethod)(PTRSZVAL, S_32 x, S_32 y, _32 b ),PTRSZVAL psv );
//PSI_PROC_PTR void SetControlKey( PSI_CONTROL pc, void (*KeyMethod)( PSI_CONTROL pc, int key ) );
PSI_PROC_PTR( void, UpdateControlEx)( PSI_CONTROL pc DBG_PASS);
PSI_PROC_PTR( int, GetControlID)( PSI_CONTROL pc );

PSI_PROC_PTR( void, DestroyControlEx)( PSI_CONTROL pc DBG_PASS );
PSI_PROC_PTR( void, SetNoFocus)( PSI_CONTROL pc );
PSI_PROC_PTR( void*, ControlExtraData)( PSI_CONTROL pc );
PSI_PROC_PTR( void, OrphanCommon )( PSI_CONTROL pc );
PSI_PROC_PTR( void, AdoptCommon )( PSI_CONTROL pFoster, PSI_CONTROL pElder, PSI_CONTROL pOrphan );

//------ General Utilities ------------
// adds OK and Cancel buttons based off the 
// bottom right of the frame, and when pressed set
// either done (cancel) or okay(OK) ... 
// could do done&ok or just done - but we're bein cheesey
PSI_PROC_PTR( void, AddCommonButtonsEx)( PSI_CONTROL pf
                                , int *done, CTEXTSTR donetext
                                , int *okay, CTEXTSTR okaytext );
PSI_PROC_PTR( void, AddCommonButtons)( PSI_CONTROL pf, int *done, int *okay );

PSI_PROC_PTR( void, CommonLoop)( int *done, int *okay ); // perhaps give a callback for within the loop?
PSI_PROC_PTR( void, ProcessControlMessages)(void);
//------ BUTTONS ------------
PSI_PROC_PTR( void, PressButton)( PSI_CONTROL pc, int bPressed );
PSI_PROC_PTR( int, IsButtonPressed)( PSI_CONTROL pc );

PSI_PROC_PTR( int, GetCheckState)( PSI_CONTROL pc );
PSI_PROC_PTR( void, SetCheckState)( PSI_CONTROL pc, int nState );

//------ Static Text -----------
//#define MakeTextControl( pf,x,y,w,h,id,text,flags ) MakeCaptionedControl( pf, STATIC_TEXT, x, y, w,h, id, text, flags )
//PSI_PROC_PTR( PSI_CONTROL, MakeTextControl)( PSI_CONTROL pf, int flags, int x, int y, int w, int h
//                        , PTRSZVAL nID, ... );
PSI_PROC_PTR( void, SetTextControlColors )( PSI_CONTROL pc, CDATA fore, CDATA back );

//------- Edit Control ---------
// Use GetControlText/SetControlText

//------- Slider Control --------
//PSI_PROC_PTR( PSI_CONTROL, MakeSlider)( PSI_CONTROL pf, int flags, int x, int y, int w, int h, PTRSZVAL nID, ... );
PSI_PROC_PTR( void, SetSliderValues)( PSI_CONTROL pc, int min, int current, int max );

//------- Color Control --------
// common dialog to get a color... returns TRUE if *result is set
// if result is NULL color is not returned, but still can set presets...
PSI_PROC_PTR( int, PickColor)( CDATA *result, CDATA original, PSI_CONTROL pAbove );

//------- Font Control --------
PSI_PROC_PTR( Font, PickFont)( S_32 x, S_32 y
                                  , P_32 size, POINTER *pFontData
                                  , PSI_CONTROL pAbove );
PSI_PROC_PTR( Font, RenderFont)( POINTER data, _32 flags );
//PSI_PROC_PTR( Font, RenderFontFile )( CTEXTSTR file, _32 width, _32 height, _32 flags );

//------- ListBox Control --------

//#define MakeListBox(pf,x,y,w,h,nID,opt) SetListboxTree( MakeControl(pf,LISTBOX_CONTROL,x,y,w,h,nID,opt), (opt & LISTOPT_TREE)?TRUE:FALSE )
//PSI_PROC_PTR( PSI_CONTROL, MakeListBox)( PSI_CONTROL pf, int options, int x, int y, int w, int h, PTRSZVAL nID, ... );

PSI_PROC_PTR( void, ResetList)( PSI_CONTROL pc );
PSI_PROC_PTR( PLISTITEM, InsertListItem)( PSI_CONTROL pc, PLISTITEM prior, CTEXTSTR text );
PSI_PROC_PTR( PLISTITEM, InsertListItemEx)( PSI_CONTROL pc, PLISTITEM prior, int nLevel, CTEXTSTR text );
PSI_PROC_PTR( PLISTITEM, AddListItem)( PSI_CONTROL pc, const CTEXTSTR text );
PSI_PROC_PTR( PLISTITEM, AddListItemEx)( PSI_CONTROL pc, int nLevel, const CTEXTSTR text );
PSI_PROC_PTR( void, DeleteListItem)( PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC_PTR( void, SetItemData)( PLISTITEM hli, PTRSZVAL psv );
PSI_PROC_PTR( PTRSZVAL, GetItemData)( PLISTITEM hli );
PSI_PROC_PTR( void, GetItemText)( PLISTITEM hli, TEXTSTR buffer, int bufsize );
PSI_PROC_PTR( PLISTITEM, GetSelectedItem)( PSI_CONTROL pc );
PSI_PROC_PTR( void, SetSelectedItem)( PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC_PTR( void, SetCurrentItem)( PSI_CONTROL pc, PLISTITEM hli );
PSI_PROC_PTR( PLISTITEM, FindListItem)( PSI_CONTROL pc, CTEXTSTR text );
PSI_PROC_PTR( PLISTITEM, GetNthItem )( PSI_CONTROL pc, int idx );


PSI_PROC_PTR( void, SetSelChangeHandler)( PSI_CONTROL pc, SelectionChanged proc, PTRSZVAL psvUser );
PSI_PROC_PTR( void, SetDoubleClickHandler)( PSI_CONTROL pc, DoubleClicker proc, PTRSZVAL psvUser );

//------- GridBox Control --------
#ifdef __LINUX__
PSI_PROC_PTR(PSI_CONTROL, MakeGridBox)( PSI_CONTROL pf, int options, int x, int y, int w, int h,
                                 int viewport_x, int viewport_y, int total_x, int total_y,
                                 int row_thickness, int column_thickness, PTRSZVAL nID );
#endif
//------- Popup Menus ------------
// popup interface.... these are mimics of windows... 
PSI_PROC_PTR( PMENU, CreatePopup)( void );
PSI_PROC_PTR( void, DestroyPopup)( PMENU pm );
// get sub-menu data...
PSI_PROC_PTR( void *,GetPopupData)( PMENU pm, int item );
PSI_PROC_PTR( PMENUITEM, AppendPopupItem)( PMENU pm, int type, PTRSZVAL dwID, CPOINTER pData );
PSI_PROC_PTR( PMENUITEM, CheckPopupItem)( PMENU pm, _32 dwID, _32 state );
PSI_PROC_PTR( PMENUITEM, DeletePopupItem)( PMENU pm, _32 dwID, _32 state );
PSI_PROC_PTR( int, TrackPopup)( PMENU hMenuSub, PSI_CONTROL parent );

//------- Color Control --------
// these are basic basic file selection dialogs... 
// the concept is valid, and they should be common like controls...
PSI_PROC_PTR( int, PSI_OpenFile)( CTEXTSTR basepath, CTEXTSTR types, CTEXTSTR result );
// this may be used for save I think....
PSI_PROC_PTR( int, PSI_OpenFileEx)( CTEXTSTR basepath, CTEXTSTR types, CTEXTSTR result, int Create );

//------- Scroll Control --------
PSI_PROC_PTR( void, SetScrollParams)( PSI_CONTROL pc, int min, int cur, int range, int max );
PSI_PROC_PTR( PSI_CONTROL, MakeScrollBar)( PSI_CONTROL pf, int x, int y, int w, int h, PTRSZVAL nID, int flags );
PSI_PROC_PTR( void, SetScrollUpdateMethod)( PSI_CONTROL pc
                    , void (CPROC*UpdateProc)(PTRSZVAL psv, int type, int current)
                    , PTRSZVAL data );
PSI_PROC_PTR( void, MoveScrollBar )( PSI_CONTROL pc, int type );

//------- Misc Controls (and widgets) --------
PSI_PROC_PTR( void, SimpleMessageBox )( PSI_CONTROL parent, CTEXTSTR title, CTEXTSTR content );

//------- new things go here...
PSI_PROC_PTR( void, HideFrame )( PSI_CONTROL pf );
PSI_PROC_PTR( void, UpdateCommonEx )( PSI_CONTROL pc, int bDraw );
} CONTROL_INTERFACE, *PCONTROL_INTERFACE;

PCONTROL_INTERFACE GetControlInterface( void );
void DropControlInterface( void );

#ifdef USE_CONTROL_INTERFACE

#define ALIAS_WRAPPER(name)   ( (USE_CONTROL_INTERFACE)->(name))

#define SetControlInterface         ALIAS_WRAPPER(SetControlInterface)
#define SetControlImageInterface    ALIAS_WRAPPER(SetControlImageInterface)

#define AlignBaseToWindows          ALIAS_WRAPPER(AlignBaseToWindows)
// see indexes above.
#define SetBaseColor                ALIAS_WRAPPER(SetBaseColor)
#define GetBaseColor                ALIAS_WRAPPER(GetBaseColor)

//-------- Frame and generic control functions --------------
#define CreateFrame                 ALIAS_WRAPPER(CreateFrame)

                           
#define DestroyFrameEx                ALIAS_WRAPPER(DestroyFrameEx)
#define DisplayFrame                ALIAS_WRAPPER(DisplayFrame)
#define HideFrame                   ALIAS_WRAPPER(HideFrame)
#define SizeFrame                   ALIAS_WRAPPER(SizeFrame)
#define MoveFrame                   ALIAS_WRAPPER(MoveFrame)
#define MoveSizeFrame               ALIAS_WRAPPER(MoveSizeFrame)
#define SizeFrameRel                ALIAS_WRAPPER(SizeFrameRel)
#define MoveFrameRel                ALIAS_WRAPPER(MoveFrameRel)
#define MoveSizeFrameRel            ALIAS_WRAPPER(MoveSizeFrameRel)
#define GetControl                  ALIAS_WRAPPER(GetControl)
#define UpdateFrameEx                 ALIAS_WRAPPER(UpdateFrameEx)
#define SetFrameMousePosition       ALIAS_WRAPPED(SetFrameMousePosition)
//PSI_PROC_PTR void SetDefaultOkayID( PSI_CONTROL pFrame, int nID );
//PSI_PROC_PTR void SetDefaultCancelID( PSI_CONTROL pFrame, int nID );

//-------- Generic control functions --------------
#define GetFrame                    ALIAS_WRAPPER(GetFrame)
#define GetNearControl              ALIAS_WRAPPER(GetNearControl)
#define GetControlTextEx            ALIAS_WRAPPER(GetControlTextEx)
#define SetControlText              ALIAS_WRAPPER(SetControlText)
#define SetControlFocus             ALIAS_WRAPPER(SetControlFocus)
#define EnableControl               ALIAS_WRAPPER(EnableControl)
#define IsControlEnabled            ALIAS_WRAPPER(IsControlEnabled)
#define GetControlSurface           ALIAS_WRAPPER(GetControlSurface)
#define SetFrameDraw                ALIAS_WRAPPER(SetFrameDraw)
#define SetFrameMouse               ALIAS_WRAPPER(SetFrameMouse)
#define SetFrameKey                 ALIAS_WRAPPER(SetFrameKey)
#define SetControlDraw              ALIAS_WRAPPER(SetControlDraw)
#define SetControlMouse             ALIAS_WRAPPER(SetControlMouse)
#define UpdateControl               ALIAS_WRAPPER(UpdateControl)
#define UpdateControlEx             ALIAS_WRAPPER(UpdateControlEx)
#define GetControlID                ALIAS_WRAPPER(GetControlID)

#define DestroyControlEx            ALIAS_WRAPPER(DestroyControlEx)
#define SetNoFocus                  ALIAS_WRAPPER(SetNoFocus)
#define ControlExtraData            ALIAS_WRAPPER(ControlExtraData)
#define OrphanCommon                ALIAS_WRAPPER(OrphanCommon)
#define AdoptCommon                 ALIAS_WRAPPER(AdoptCommon)
//------ General Utilities ------------
#define AddCommonButtonsEx          ALIAS_WRAPPER(AddCommonButtonsEx)
#define AddCommonButtons            ALIAS_WRAPPER(AddCommonButtons)

#define CommonLoop                  ALIAS_WRAPPER(CommonLoop)
#define ProcessControlMessages      ALIAS_WRAPPER(ProcessControlMessages)
//------ BUTTONS ------------
#define MakeButton                  ALIAS_WRAPPER(MakeButton)
#define MakeImageButton             ALIAS_WRAPPER(MakeImageButton)
#define MakeCustomDrawnButton       ALIAS_WRAPPER(MakeCustomDrawnButton)
#define PressButton                 ALIAS_WRAPPER(PressButton)
#define IsButtonPressed             ALIAS_WRAPPER(IsButtonPressed)

#define MakeCheckButton             ALIAS_WRAPPER(MakeCheckButton)
#define MakeRadioButton             ALIAS_WRAPPER(MakeRadioButton)
#define GetCheckState               ALIAS_WRAPPER(GetCheckState)
#define SetCheckState               ALIAS_WRAPPER(SetCheckState)

//------ Static Text -----------
#define MakeTextControl             ALIAS_WRAPPER(MakeTextControl)
#define SetTextControlColors        ALIAS_WRAPPER(SetTextControlColors)
//------- Edit Control ---------
#define MakeEditControl             ALIAS_WRAPPER(MakeEditControl)
// Use GetContrcolText/SetControlText

//------- Slider Control --------
#define MakeSlider                  ALIAS_WRAPPER(MakeSlider)
#define SetSliderValues             ALIAS_WRAPPER(SetSliderValues)

//------- Color Control --------
// common dialog to get a color... returns TRUE if *result is set
// if result is NULL color is not returned, but still can set presets...
#define PickColor                   ALIAS_WRAPPER(PickColor)
//------- Font Control --------
#define PickFont                    ALIAS_WRAPPER(PickFont)
#define RenderFont                  ALIAS_WRAPPER(RenderFont)

//------- ListBox Control --------
#define MakeListBox                 ALIAS_WRAPPER(MakeListBox)

                           
#define ResetList                   ALIAS_WRAPPER(ResetList)
#define InsertListItem              ALIAS_WRAPPER(InsertListItem)
#define InsertListItemEx            ALIAS_WRAPPER(InsertListItemEx)
#define AddListItem                 ALIAS_WRAPPER(AddListItem)
#define AddListItemEx               ALIAS_WRAPPER(AddListItemEx)
#define DeleteListItem              ALIAS_WRAPPER(DeleteListItem)
#define SetItemData                 ALIAS_WRAPPER(SetItemData)
#define GetItemData                 ALIAS_WRAPPER(GetItemData)
#define GetItemText                 ALIAS_WRAPPER(GetItemText)
#define GetSelectedItem             ALIAS_WRAPPER(GetSelectedItem)
#define SetSelectedItem             ALIAS_WRAPPER(SetSelectedItem)
#define SetCurrentItem              ALIAS_WRAPPER(SetCurrentItem)
#define FindListItem                ALIAS_WRAPPER(FindListItem)
#define GetNthItem                  ALIAS_WRAPPER(GetNthItem)
#define SetSelChangeHandler         ALIAS_WRAPPER(SetSelChangeHandler )
#define SetDoubleClickHandler       ALIAS_WRAPPER(SetDoubleClickHandler)
//------- GridBox Control --------
#define MakeGridBox                 ALIAS_WRAPPER(MakeGridBox)
    //------- Popup Menus ------------
// popup interface.... these are mimics of windows... 
#define CreatePopup                 ALIAS_WRAPPER(CreatePopup)
#define DestroyPopup                ALIAS_WRAPPER(DestroyPopup)
#define GetPopupData                ALIAS_WRAPPER(GetPopupData)
#define AppendPopupItem             ALIAS_WRAPPER(AppendPopupItem)
#define CheckPopupItem              ALIAS_WRAPPER(CheckPopupItem)
#define DeletePopupItem             ALIAS_WRAPPER(DeletePopupItem)
#define TrackPopup                  ALIAS_WRAPPER(TrackPopup)
//------- Color Control --------
// these are basic basic file selection dialogs... 
// the concept is valid, and they should be common like controls...
#define PSI_OpenFile                ALIAS_WRAPPER(PSI_OpenFile)
// this may be used for save I think....
#define PSI_OpenFileEx              ALIAS_WRAPPER(PSI_OpenFileEx)
//------- Scroll Control --------
#define SetScrollParams             ALIAS_WRAPPER(SetScrollParams)
#define MakeScrollBar               ALIAS_WRAPPER(MakeScrollBar)
#define SetScrollUpdateMethod       ALIAS_WRAPPER(SetScrollUpdateMethod)

#endif


#ifdef BASE_CONTROL_MESSAGE_ID
// need to define BASE_IMAGE_MESSAGE_ID before hand to determine what the base message is.
#define MSG_ID(method)  ( ( offsetof( struct control_interface_tag, method ) / sizeof( void (CPROC*)(void) ) ) + BASE_CONTROL_MESSAGE_ID + MSG_EventUser )

#define MSG_SetControlInterface         MSG_ID(SetControlInterface)
#define MSG_SetControlImageInterface    MSG_ID(SetControlImageInterface)

#define MSG_AlignBaseToWindows          MSG_ID(AlignBaseToWindows)
// see indexes above.
#define MSG_SetBaseColor                MSG_ID(SetBaseColor)
#define MSG_GetBaseColor                MSG_ID(GetBaseColor)

//-------- Frame and generic control functions --------------
#define MSG_CreateFrame                 MSG_ID(CreateFrame)

                           
#define MSG_DestroyFrameEx                MSG_ID(DestroyFrameEx)
#define MSG_DisplayFrame                MSG_ID(DisplayFrame)
#define MSG_HideFrame                   MSG_ID(HideFrame)
#define MSG_SizeCommon                   MSG_ID(SizeCommon)
#define MSG_MoveCommon                   MSG_ID(MoveCommon)
#define MSG_MoveSizeCommon               MSG_ID(MoveSizeCommon)
#define MSG_SizeCommonRel                   MSG_ID(SizeCommonRel)
#define MSG_MoveCommonRel                   MSG_ID(MoveCommonRel)
#define MSG_MoveSizeCommonRel               MSG_ID(MoveSizeCommonRel)
#define MSG_GetControl                  MSG_ID(GetControl)
#define MSG_UpdateFrame                 MSG_ID(UpdateFrame)
#define MSG_SetFrameMousePosition       MSG_ID(SetFrameMousePosition)
//PSI_PROC_PTR void SetDefaultOkayID( PSI_CONTROL pFrame, int nID );
//PSI_PROC_PTR void SetDefaultCancelID( PSI_CONTROL pFrame, int nID );

//-------- Generic control functions --------------
#define MSG_GetFrame                    MSG_ID(GetFrame)
#define MSG_GetNearControl              MSG_ID(GetNearControl)
#define MSG_GetControlTextEx            MSG_ID(GetControlTextEx)
#define MSG_SetControlText              MSG_ID(SetControlText)
#define MSG_SetControlFocus             MSG_ID(SetControlFocus)
#define MSG_EnableControl               MSG_ID(EnableControl)
#define MSG_IsControlEnabled            MSG_ID(IsControlEnabled)
#define MSG_GetControlSurface           MSG_ID(GetControlSurface)
#define MSG_SetFrameDraw                MSG_ID(SetFrameDraw)
#define MSG_SetFrameMouse               MSG_ID(SetFrameMouse)
#define MSG_SetFrameKey                 MSG_ID(SetFrameKey)
#define MSG_SetControlDraw              MSG_ID(SetControlDraw)
#define MSG_SetControlMouse             MSG_ID(SetControlMouse)
#define MSG_UpdateControl               MSG_ID(UpdateControl)
#define MSG_UpdateControlEx             MSG_ID(UpdateControlEx)
#define MSG_GetControlID                MSG_ID(GetControlID)

#define MSG_DestroyControlEx            MSG_ID(DestroyControlEx)
#define MSG_SetNoFocus                  MSG_ID(SetNoFocus)
#define MSG_ControlExtraData            MSG_ID(ControlExtraData)
#define MSG_OrphanCommon                MSG_ID(OrphanCommon)
#define MSG_AdoptCommon                 MSG_ID(AdoptCommon)

//------ General Utilities ------------
#define MSG_AddCommonButtonsEx          MSG_ID(AddCommonButtonsEx)
#define MSG_AddCommonButtons            MSG_ID(AddCommonButtons)

#define MSG_CommonLoop                  MSG_ID(CommonLoop)
#define MSG_ProcessControlMessages      MSG_ID(ProcessControlMessages)
//------ BUTTONS ------------
#define MSG_MakeButton                  MSG_ID(MakeButton)
#define MSG_MakeImageButton             MSG_ID(MakeImageButton)
#define MSG_MakeCustomDrawnButton       MSG_ID(MakeCustomDrawnButton)
#define MSG_PressButton                 MSG_ID(PressButton)
#define MSG_IsButtonPressed             MSG_ID(IsButtonPressed)

#define MSG_MakeCheckButton             MSG_ID(MakeCheckButton)
#define MSG_MakeRadioButton             MSG_ID(MakeRadioButton)
#define MSG_GetCheckState               MSG_ID(GetCheckState)
#define MSG_SetCheckState               MSG_ID(SetCheckState)

//------ Static Text -----------
#define MSG_MakeTextControl             MSG_ID(MakeTextControl)
#define MSG_SetTextControlColors        MSG_ID(SetTextControlColors)

//------- Edit Control ---------
#define MSG_MakeEditControl             MSG_ID(MakeEditControl)
// Use GetContrcolText/SetControlText

//------- Slider Control --------
#define MSG_MakeSlider                  MSG_ID(MakeSlider)
#define MSG_SetSliderValues             MSG_ID(SetSliderValues)

//------- Color Control --------
// common dialog to get a color... returns TRUE if *result is set
// if result is NULL color is not returned, but still can set presets...
#define MSG_PickColor                   MSG_ID(PickColor)
//------- Font Control --------
#define MSG_PickFont                    MSG_ID(PickFont)
#define MSG_RenderFont                  MSG_ID(RenderFont)

//------- ListBox Control --------
#define MSG_MakeListBox                 MSG_ID(MakeListBox)

                           
#define MSG_ResetList                   MSG_ID(ResetList)
#define MSG_InsertListItem              MSG_ID(InsertListItem)
#define MSG_InsertListItemEx            MSG_ID(InsertListItemEx)
#define MSG_AddListItem                 MSG_ID(AddListItem)
#define MSG_AddListItemEx               MSG_ID(AddListItemEx)
#define MSG_DeleteListItem              MSG_ID(DeleteListItem)
#define MSG_SetItemData                 MSG_ID(SetItemData)
#define MSG_GetItemData                 MSG_ID(GetItemData)
#define MSG_GetItemText                 MSG_ID(GetItemText)
#define MSG_GetSelectedItem             MSG_ID(GetSelectedItem)
#define MSG_SetSelectedItem             MSG_ID(SetSelectedItem)
#define MSG_SetCurrentItem              MSG_ID(SetCurrentItem)
#define MSG_FindListItem                MSG_ID(FindListItem)
#define MSG_GetNthItem                  MSG_ID(GetNthItem)
#define MSG_SetSelChangeHandler         MSG_ID(SetSelChangeHandler)
#define MSG_SetDoubleClickHandler       MSG_ID(SetDoubleClickHandler)
//------- GridBox Control --------
#define MSG_MakeGridBox                 MSG_ID(MakeGridBox)
    //------- Popup Menus ------------
// popup interface.... these are mimics of windows... 
#define MSG_CreatePopup                 MSG_ID(CreatePopup)
#define MSG_DestroyPopup                MSG_ID(DestroyPopup)
#define MSG_GetPopupData                MSG_ID(GetPopupData)
#define MSG_AppendPopupItem             MSG_ID(AppendPopupItem)
#define MSG_CheckPopupItem              MSG_ID(CheckPopupItem)
#define MSG_DeletePopupItem             MSG_ID(DeletePopupItem)
#define MSG_TrackPopup                  MSG_ID(TrackPopup)
//------- Color Control --------
// these are basic basic file selection dialogs... 
// the concept is valid, and they should be common like controls...
#define MSG_PSI_OpenFile                MSG_ID(PSI_OpenFile)
// this may be used for save I think....
#define MSG_PSI_OpenFileEx              MSG_ID(PSI_OpenFileEx)
//------- Scroll Control --------
#define MSG_SetScrollParams             MSG_ID(SetScrollParams)
#define MSG_MakeScrollBar               MSG_ID(MakeScrollBar)
#define MSG_SetScrollUpdateMethod       MSG_ID(SetScrollUpdateMethod)

#endif

#define GetFrameSurface GetControlSurface

PSI_NAMESPACE_END

USE_PSI_NAMESPACE

#include <psi/edit.h>
#include <psi/buttons.h>
#include <psi/slider.h>
#include <psi/controls.h>
#include <psi/shadewell.h>


#endif
