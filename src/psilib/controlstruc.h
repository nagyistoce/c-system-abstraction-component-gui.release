#ifndef CONTROL_SOURCE
#define CONTROL_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <procreg.h>
#include <fractions.h>
#include <controls.h>
#include "global.h"
//#include <vidlib.h>

//#define DEBUG_BORDER_DRAWING
//#define QUICK_DEBUG_BORDER_FLAGS

PSI_NAMESPACE
// define this to prevent multiple definition to application 
// viewpoint...
//---------------------------------------------------------------------------

// default_width, default_height,
#define CONTROL_PROC_DEF( controltype, type, name, _args )                  \
	int CPROC Init##name( PSI_CONTROL pControl );  \
	int CPROC Config##name( PSI_CONTROL pc ) { return Init##name(pc); } \
   int CPROC Init##name( PSI_CONTROL pc )

#define CAPTIONED_CONTROL_PROC_DEF( controltype, type, name, _args )                  \
	int CPROC Init##name( PSI_CONTROL pControl );  \
	int CPROC Config##name( PSI_CONTROL pc ) { return Init##name(pc); } \
   int CPROC Init##name( PSI_CONTROL pc )


#define CONTROL_PROC_DEF_EX( controltype, type, name, _args) \
   int CPROC Init##name( PSI_CONTROL );  \
	int CPROC Config##name( PSI_CONTROL pc ) { return Init##name(pc); } \
	int CPROC Init##name( PSI_CONTROL pc )

#define ARG( type, name ) PARAM( args, type, name )
#define FP_ARG( type, name, funcargs ) FP_PARAM( args, type, name, funcargs )


#ifdef GCC
#define CONTROL_INIT(name) CONTROL_PROPERTIES( name )(PSI_CONTROL pControl)      \
{ return NULL; } CONTROL_PROPERTIES_APPLY( name )(PSI_CONTROL pc,PSI_CONTROL page) { ; }    \
   int CPROC Init##name ( PTRSZVAL psv, PSI_CONTROL pControl, _32 ID )

#define CONTROL_INIT_EX(name)  \
   int CPROC Init##name ( PTRSZVAL psv, PSI_CONTROL pControl, _32 ID )
//#error Need to figure out how to register control Inits

#else
#define CONTROL_INIT(name)  CONTROL_PROPERTIES( name )(PSI_CONTROL pc)      \
{ return NULL; } CONTROL_PROPERTIES_APPLY( name )(PSI_CONTROL pc,PSI_CONTROL page) { ; }   \
   int CPROC Init##name ( PSI_CONTROL pc, va_list args )

#define CONTROL_INIT_EX(name)  \
   int CPROC Init##name ( PSI_CONTROL pc, va_list args )
//#error Need to figure out how to register control Inits
#endif



// size of the property dialog page...
#define PROP_WIDTH 540
#define PROP_HEIGHT 240
#define PROP_PAD 5

/* \Internal event callback definition. Request an additional
   page to add to the control property edit dialog.           */
typedef PSI_CONTROL (CPROC*GetControlPropSheet)( PSI_CONTROL );
#define CONTROL_PROPERTIES( name )  OnPropertyEdit( TOSTR(name) )

/* \Internal defintion of the callback to invoke when a property
   sheet is requested to be applied when editing the control.    */
typedef void (CPROC*ApplyControlPropSheet)( PSI_CONTROL, PSI_CONTROL );
#define CONTROL_PROPERTIES_APPLY( name )  OnPropertyEditOkay( TOSTR(name) )
/* Tells a control that the edit process is done with the
   property sheet requested.
   
   \Internal event callback definition.                   */
typedef void (CPROC*DoneControlPropSheet)( PSI_CONTROL );
#define CONTROL_PROPERTIES_DONE( name )  OnPropertyEditDone( TOSTR(name) )

//typedef struct subclass_control_tag {
//	PCONTROL_REGISTRATION methods;
//};

//#define ControlType(pc) ((pc)->nType)

enum HotspotLocations {
   SPOT_NONE // 0 = no spot locked...
    , SPOT_TOP_LEFT
    , SPOT_TOP
    , SPOT_TOP_RIGHT
    , SPOT_LEFT
    , SPOT_CENTER
    , SPOT_RIGHT
    , SPOT_BOTTOM_LEFT
    , SPOT_BOTTOM
    , SPOT_BOTTOM_RIGHT
};

/* \Internal event callback definition. Draw border, this
   usually pointing to an internal function, but may be used for
   a control to draw a custom border.                            */
typedef void (CPROC*_DrawBorder)        ( struct common_control_frame * );
/* \Internal event callback definition. This is called when the
   control needs to draw itself. This happens when SmudgeCommon
   is called on the control or on a parent of the control.      */
typedef int (CPROC*__DrawThySelf)       ( struct common_control_frame * );
/* \Internal event callback definition. A mouse event is
   happening over the control.                           */
typedef int (CPROC*__MouseMethod)       ( struct common_control_frame *, S_32 x, S_32 y, _32 b );
/* \Internal event callback definition. A key has been pressed. */
typedef int (CPROC*__KeyProc)           ( struct common_control_frame *, _32 );
/* \Internal event callback definition. The caption of a control
   is changing (Edit control uses this).                         */
typedef void (CPROC*_CaptionChanged)    ( struct common_control_frame * );
/* \Internal event callback definition. Destruction of the
   control is in progress. Allow control to free internal
   resources.                                              */
typedef void (CPROC*_Destroy)           ( struct common_control_frame * );
/* \Internal event callback definition.
   
   A control has been added to this control. */
typedef void (CPROC*_AddedControl)      ( struct common_control_frame *, struct common_control_frame *pcAdding );
/* \Internal event callback definition. The focus of a control
   is changing.                                                */
typedef void (CPROC*_ChangeFocus)       ( struct common_control_frame *, LOGICAL bFocused );
/* \Internal event callback definition. Called when a control is
	   being resized. Width or height changing.                      */
typedef void (CPROC*_Resize)            ( struct common_control_frame *, LOGICAL bSizing );
/* \Internal event callback definition. Called when the
   control's position (x,y) is changing.                */
typedef void (CPROC*_PosChanging)       ( struct common_control_frame *, LOGICAL bMoving );	
/* \Internal event callback definition. Triggered when edit on a
   frame is started.                                             */
typedef void (CPROC*_BeginEdit)         ( struct common_control_frame * );
/* \Internal event callback definition. Ending control editing. */
typedef void (CPROC*_EndEdit)           ( struct common_control_frame * );
/* \Internal event callback definition. A file has been dropped
   on the control.                                              */
typedef void (CPROC*_AcceptDroppedFiles)( struct common_control_frame *, CTEXTSTR filename, S_32 x, S_32 y );


#define DeclMethod( name ) int n##name; _##name *name
#define DeclSingleMethod( name ) _##name name
// right now - all these methods evolved from a void
// function, therefore, this needs to invoke all key/draw methods not just
// until 'processed'
#define InvokeDrawMethod(pc,name,args)  if( (pc) && (pc)->name ) { int n; for( n = 0; (pc) && n < (pc)->n##name; n++ ) if( (pc)->name[n] ) /*if(*/(pc)->draw_result |= (pc)->name[n]args /*)*/ /*break*/; }
#define InvokeMethod(pc,name,args)  if( (pc) && (pc)->name ) { int n; for( n = 0; (pc) && n < (pc)->n##name; n++ ) if( (pc)->name[n] ) /*if(*/(pc)->name[n]args /*)*/ /*break*/; }
#define InvokeResultingMethod(result,pc,name,args)  if( (pc) && (pc)->name ) { int n; for( n = 0; (pc) && n < (pc)->n##name; n++ ) if( (pc)->name[n] ) if( (result)=(pc)->name[n]args ) break; }
#define InvokeSingleMethod(pc,name,args)  if( (pc)->name ) { (pc)->name args; }

struct edit_state_tag {

//DOM-IGNORE-BEGIN
   PSI_CONTROL pCurrent;
   // so we can restore keystrokes to the control
   // and/or relay unused keys to the control...
   // perhaps should override draw this way
   // some controls may take a long time to draw while 
   // sizing... although we probably do want to see
   // their behavior at the new size....
	// but THIS definatly so we can process arrow keys...
   //void (CPROC*PriorKeyProc)( PTRSZVAL psv, _32 );
	//PTRSZVAL psvPriorKey;
   DeclMethod( _KeyProc );
   //void (CPROC*_PriorKeyProc)( PSI_CONTROL pc, _32 );

   IMAGE_POINT hotspot[9];
   IMAGE_RECTANGLE bound;
   IMAGE_POINT bias; // pCurrent upper left corner kinda on the master frame
	S_32 _x, _y; // marked x, y when the hotspot was grabbed...

   // should also do a cumulative change delta off
	// this to lock the mouse in position...
	S_32 delxaccum, delyaccum;

   struct {
      _32 bActive : 1; // edit state is active.
      _32 fLocked : 4; // which spot it's locked on...
      _32 bDragging : 1;
      _32 bSizing : 1; // any sizing flag set
      _32 bSizing_left  : 1;
      _32 bSizing_right : 1;
      _32 bSizing_top   : 1;
      _32 bSizing_bottom: 1;
      _32 bFrameWasResizable : 1;
      _32 bHotSpotsActive : 1;
   } flags;
	_32 BorderType;
//DOM-IGNORE-END
};
typedef struct edit_state_tag EDIT_STATE;
typedef struct edit_state_tag *PEDIT_STATE;

struct physical_device_interface
{
//DOM-IGNORE-BEGIN
   PRENDERER pActImg; // any control can have a physical renderer...
   struct common_control_frame * common; // need this to easily back track...
   struct device_flags {
      _32 bDragging : 1; // frame is being moved
      _32 bSizing       : 1; // flags for when frame is sizable
      _32 bSizing_left  : 1;
      _32 bSizing_right : 1;
      _32 bSizing_top   : 1;
      _32 bSizing_bottom: 1;
      _32 bCurrentOwns : 1; // pCurrent is also owner of the mouse (button was clicked, and never released)
		_32 bNoUpdate : 1; // don't call update function...
		_32 bCaptured : 1; // frame owns mouse, control behaving as frame wants all mouse events.
		_32 bApplicationOwned : 1; // current owns was set by application, do not auto disown.
   }flags;
   EDIT_STATE EditState;
   //PRENDERER pActImg;
   //PTRSZVAL psvUser; // user data...
   int _x, _y;
   _32 _b; // last button state...
   // these two buttons override controls which have the ID BTN_OKAY, BTN_CANCEL
   int nIDDefaultOK;
   int nIDDefaultCancel;
   // when has mouse is set, this is set
   // as a quick computation of the coordinate
	// bias.
	struct {
		struct {
			_32 bias_is_surface : 1;
		} flags;
		S_32 x, y;
	} CurrentBias;
   //Image original_surface;
   struct common_control_frame * pCurrent; // Current control which has the mouse within it...(or owns mouse)
	struct common_control_frame * pFocus;   // keyboard goes here...
   // this is now added as a draw callback method
	//void (CPROC*OwnerDraw)(struct common_control_frame * pc);
   // this is now added as a mouse callback method
	//void (CPROC*OwnerMouse)(struct common_control_frame * pc, S_32 x, S_32 y, _32 b);
   // this is unused yet...
   //int (CPROC*InitControl)(PTRSZVAL, struct common_control_frame *, _32);// match ControlInitProc(controls.h)
	PTRSZVAL psvInit;
//DOM-IGNORE-END
};
typedef struct physical_device_interface PHYSICAL_DEVICE;
typedef struct physical_device_interface*PPHYSICAL_DEVICE;




typedef struct common_button_data {
//DOM-IGNORE-BEGIN
	PTHREAD thread;
   int *okay_value;
	int *done_value;
	struct button_flags {
		_32 bWaitOnEdit : 1;
	} flags;
//DOM-IGNORE-END
} COMMON_BUTTON_DATA, *PCOMMON_BUTTON_DATA;


//DOM-IGNORE-BEGIN

typedef struct common_control_frame
{
	// this is the one place allowed for
	// an application to store data.
	// once upon a time, I followed windows and
	// allowed set/get network long, which allowed
	// multiple values to be kept...
	// but that doesn't help anything, it just allows for
	// bad coding, and useless calls to fetch values
   // from the object...
	POINTER pUser;
   // the user may also set a DWORD value associated with a control.
	PTRSZVAL psvUser;

	// this is the numeric type ID of the control.  Although, now
   // controls are mostly tracked with their name.
   int nType; 
   /* Name of the type this control is. Even if a control is
      created by numeric Type ID, it still gets its name from the
      procedure registry.                                         */
	CTEXTSTR pTypeName;
   // unique control ID ....
	int nID;
   // this is the text ID registered...
   CTEXTSTR pIDName; 
	//----------------
	// the data above this point may be known by
	// external sources...

	/* just the data portion of the control */
	Image Surface;
	Image OriginalSurface; // just the data portion of the control

	/* flags that affect a control's behavior or state.
	                                                    */
	/* <combine sack::psi::common_control_frame::flags@1>
	   
	   \ \                                                */
	struct {
      /* Control is currently keyboard focused. */
		_32 bFocused : 1;
      // destroyed - and at next opportunity will be...
		_32 bDestroy : 1; 
      // set when a size op begins to void draw done during size
		_32 bSizing : 1; 
      // used to make Frame more 'Pop' Up...
		_32 bInitial : 1; 
      // set to disable updates
		_32 bNoUpdate : 1;
      // this control was explicitly set hidden.. don't unhide.
		_32 bHiddenParent : 1; 
      // can't see it, can't touch it.
		_32 bHidden : 1; 
      // scale currently applies.
		_32 bScaled : 1; 
		/* control gets no keyboard focus. */
		_32 bNoFocus:1;
      // greyed out state?
		_32 bDisable : 1; 
       // 0 = default alignment 1 = left, 2 = center 3 = right
		_32 bAlign:2;
      // draw veritcal instead of horizontal
		_32 bVertical:1;
       // draw opposite/upside down from normal
		_32 bInvert:1;
      // needs DrawThySelf called...
		_32 bDirty : 1;
      // DrawThySelf has been called...
		_32 bCleaning : 1;
      // only need to update the control's frame... (focus change)
		_32 bDirtyBorder : 1;
      // parent drew, therefore this needs to draw, and it's an initial draw.
		_32 bParentCleaned : 1;
      // saves it's original surface and restores it before invoking the control draw.
		_32 bTransparent : 1; 

		/* Adopted children are not automatically saved in XML files. */
		_32 bAdoptedChild : 1;
      // children were cleaned by an internal update... don't draw again.
		_32 children_cleaned : 1;
      // no extra init, and no save, this is a support control created for a master control
		_32 private_control : 1; 
		/* control has been temporarily displaced from its parent
		   control.                                               */
			_32 detached : 1;
         // edit mode enabled visibility of this window and opened it.
			_32 auto_opened : 1;
         // first time this is being cleaned (during the course of refresh this could be called many times)
			_32 bFirstCleaning : 1;
         // frame was loaded from XML, and desires that EditFrame not be enablable.
		_32 bNoEdit : 1; 
		/* Edit has been enabled on the control. */
			_32 bEditSet : 1;
         // this came from the XML file.
			_32 bEditLoaded : 1;
         // an update event is already being done, bail on this and all children?
			_32 bUpdating : 1;
         // enable only real draw events in video thread.  (post invalidate only)
			_32 bOpenGL : 1;
         // needs DrawThySelf called... // collect these, so a master level draw can be done down. only if a control or it's child is dirty.
			_32 bChildDirty : 1;
         // there is a frame caption update (with flush to display) which needs to be locked....
			_32 bRestoring : 1;
         // got at least one frame redraw event (focus happens before first draw)
		_32 bShown : 1; 
		/* Set when resized by a mouse drag, causes a dirty state. */
			_32 bResizedDirty : 1;
         // during control update the effective surface region was set while away.
		_32 bUpdateRegionSet : 1; 
	} flags;


	/* Information about the caption of a control. */
	/* <combine sack::psi::common_control_frame::caption@1>
	   
	   \ \                                                  */
	struct {
		/* This is the font that applies to the current control. If it
		   is NULL, then it uses the parent controls' font. If that's
		   null, it keeps searching up until it finds a font or results
		   NULL and uses the default internal font.                     */
		Font font;
		/* This is actually a PFONTDATA.
		                                 */
		POINTER fontdata;
      /* Length of the PFONTDATA. */
      _32 fontdatalen;
      /* Text of the control's caption. */
      PTEXT text;
	} caption;


    // original size the control was created at (before the border is applied)
   IMAGE_RECTANGLE original_rect;
   /* the scalex that is applied to creation of controls within
      this control. Modifies the width of a control and X
      positioning.                                              */
		FRACTION scalex;
   /* the scaley that is applied to creation of controls within
      this control. Modifies the height of a control and Y
      positioning.                                              */
		FRACTION scaley;
      // the actual rect of the control...
   IMAGE_RECTANGLE rect;  
	/* This is the rectangle that describes where the surface of the
	   control is relative to is outside position.                   */
		IMAGE_RECTANGLE surface_rect;
      // size and position of detachment.
   IMAGE_RECTANGLE detached_at; 
   /* this is the output device that the control is being rendered
      to.                                                          */
	PPHYSICAL_DEVICE device;
   // includes border/caption of control
   Image Window; 
 // actively processing - only when decremented to 0 do we destroy...
	_32 InUse;
   // fake counter to allow ReleaseCommonUse to work.
   _32 NotInUse; 
	// waitinig for a responce... when both inuse and inwait become 0 destroy can happy.
   // otherwise when inuse reduces to 0, draw events are dispatched.
	_32 InWait;
	/* This is a pointer to the prior control in a frame. */
	/* pointer to the first child control in this one. */
	/* pointer to the next control within this control's parent. */
	/* pointer to the control that contains this control. */
	struct common_control_frame *child, *parent, *next, *prior;
   // maybe I can get pointers to this....

	_32 BorderType;
   // also declare a method above of the same name...
	int draw_result;

   DeclMethod( _DrawThySelf );
   // also declare a method above of the same name...
   DeclMethod( _MouseMethod );
   // also declare a method above of the same name...
   DeclMethod( _KeyProc );
   // also declare a method above of the same name...
   DeclSingleMethod( DrawBorder );
   // also declare a method above of the same name...
   DeclSingleMethod( CaptionChanged );
   // also declare a method above of the same name...
   DeclSingleMethod( Destroy );
   // also declare a method above of the same name...
   DeclSingleMethod( AddedControl );
   // also declare a method above of the same name...
	DeclSingleMethod( ChangeFocus );
   // also declare a method above of the same name...
	DeclSingleMethod( Resize );
	//DeclSingleMethod( PosChanging );
   DeclSingleMethod( BeginEdit );
   DeclSingleMethod( EndEdit );
   DeclMethod( AcceptDroppedFiles );
	/* Pointer to common button data. Common buttons are the Okay
	   and Cancel buttons that are commonly on dialogs.           */
		COMMON_BUTTON_DATA pCommonButtonData;
      // invalidating an arbitrary rect, this is the intersection of the parent's dirty rect on this
		IMAGE_RECTANGLE dirty_rect;   
      // during update this may be set, and should be used for the update region insted of control surface
	IMAGE_RECTANGLE update_rect;  
   /* A copy of the name that the frame was loaded from or saved
      to. For subsequent save when the control is edited.        */
		CTEXTSTR save_name;
      CDATA *basecolors;
   int nExtra; // size above common required...
} FR_CT_COMMON, *PCONTROL;
//DOM-IGNORE-END

//---------------------------------------------------------------------------

// each control has itself a draw border method.
//void CPROC DrawBorder( PTRSZVAL psv, PSI_CONTROL pc );
// check box uses these... ???
void CPROC DrawThinFrame( PSI_CONTROL pc );
void CPROC DrawThinnerFrame( PSI_CONTROL pc );
void CPROC DrawThinFrameInverted( PSI_CONTROL pc );
void CPROC DrawThinnerFrameInverted( PSI_CONTROL pc );
void CPROC DrawThinFrameImage( Image pc );
/* Draw a 2 pixel frame around a control.
   Parameters
   pc :  COntrol to draw a thinner frame on;. */
void CPROC DrawThinnerFrameImage( PSI_CONTROL pc, Image image );
void CPROC DrawThinFrameInvertedImage( PSI_CONTROL pc, Image image );
void CPROC DrawThinnerFrameInvertedImage( PSI_CONTROL pc, Image image );

void GetCurrentDisplaySurface( PPHYSICAL_DEVICE device );
_MOUSE_NAMESPACE
/* This is an internal routine which sets the hotspot drawing
   coordiantes. Prepares for drawing, but doesn't draw.       */
void SetupHotSpots( PEDIT_STATE pEditState );
/* Routine in mouse space which draws hotspots on the frame
   indicating areas that can be manipulated on a control. Otherwise
   controls are fully active, and you can use them as you are
   developing. Hotspots are drawn in WHITE unless the mouse is
   captured by one, then the spot is RED.
   
   
   Parameters
   pf :  frame being edited.
   pe :  pointer to the current edit state containing information
         like the currently active control for editing on a frame.  */
void DrawHotSpotsEx( PSI_CONTROL pf, PEDIT_STATE pEditState DBG_PASS );
/* <combine sack::psi::_mouse::DrawHotSpotsEx@PSI_CONTROL@PEDIT_STATE pEditState>
   
   \ \                                                                        */
#define DrawHotSpots(pf,pe) DrawHotSpotsEx(pf,pe DBG_SRC)
//void DrawHotSpots( PSI_CONTROL pf, PEDIT_STATE pEditState );
_MOUSE_NAMESPACE_END
void SmudgeSomeControls( PSI_CONTROL pc, P_IMAGE_RECTANGLE pRect );
void DetachFrameFromRenderer( PSI_CONTROL pc );
void IntelligentFrameUpdateAllDirtyControls( PSI_CONTROL pc DBG_PASS );

_BUTTON_NAMESPACE
//	void InvokeButton( PSI_CONTROL pc );
_BUTTON_NAMESPACE_END

// dir 0 here only... in case we removed ourself from focusable
// dir -1 go backwards
// dir 1 go forwards
#define FFF_HERE      0
#define FFF_FORWARD   1
#define FFF_BACKWARD -1
void FixFrameFocus( PPHYSICAL_DEVICE pf, int dir );

_MOUSE_NAMESPACE
	/* Specifies symbols for which default control to press -
	   default accept or default cancel.                      */
	enum MouseInvokeType {
 INV_OKAY   = 0, /* Invoke Button OK. */
 
 INV_CANCEL = 1 /* Invoke Button Cancel. */
 
	};
/* Invokes a default button on a frame.
   Parameters
   pc :    frame to invoke the event on
   type :  type of Event from MouseInvokeType. */
int InvokeDefault( PSI_CONTROL pc, int type );

/* Add a usage counter to a control. Controls in use have redraw
   events blocked.                                               */
void AddUseEx( PSI_CONTROL pc DBG_PASS);
/* <combine sack::psi::_mouse::AddUseEx@PSI_CONTROL pc>
   
   \ \                                              */
#define AddUse( pc ) AddUseEx( pc DBG_SRC )

/* Removes a use added by AddUse. COntrols in use cannot update. */
void DeleteUseEx( PSI_CONTROL *pc DBG_PASS );
/* <combine sack::psi::_mouse::DeleteUseEx@PSI_CONTROL *pc>
   
   \ \                                                  */
#define DeleteUse(pc) DeleteUseEx( &pc DBG_SRC )

/* Adds a wait to a control. This prevents drawing while the
   system is out of a drawable state.                        */
void AddWaitEx( PSI_CONTROL pc DBG_PASS);
/* <combine sack::psi::_mouse::AddWaitEx@PSI_CONTROL pc>
   
   \ \                                               */
#define AddWait( pc ) AddWaitEx( pc DBG_SRC )

/* Removes a wait added by AddWait */
void DeleteWaitEx( PSI_CONTROL *pc DBG_PASS );
#define DeleteWait(pc) DeleteWaitEx( &pc DBG_SRC ) /* <combine sack::psi::_mouse::DeleteWaitEx@PSI_CONTROL *pc>
                                                      
                                                      \ \                                                   */

PPHYSICAL_DEVICE OpenPhysicalDevice( PSI_CONTROL pc, PSI_CONTROL over, PRENDERER pActImg, PSI_CONTROL under );
void TryLoadingFrameImage( void );
Image CopyOriginalSurfaceEx( PCONTROL pc, Image use_image DBG_PASS );
#define CopyOriginalSurface(pc,i) CopyOriginalSurfaceEx(pc,i DBG_SRC )


_MOUSE_NAMESPACE_END
USE_MOUSE_NAMESPACE
#define PCOMMON PSI_CONTROL

PSI_NAMESPACE_END

#include <controls.h>

#endif

// $Log: controlstruc.h,v $
// Revision 1.73  2005/07/08 00:45:47  d3x0r
// Define NotInUse in control structure.
//
// Revision 1.20  2003/03/25 08:45:56  panther
// Added CVS logging tag
//
