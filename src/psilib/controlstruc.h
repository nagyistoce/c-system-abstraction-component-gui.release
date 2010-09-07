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
// Revision 1.72  2005/05/30 11:56:36  d3x0r
// various fixes... working on psilib update optimization... various stabilitizations... also extending msgsvr functionality.
//
// Revision 1.71  2005/05/25 16:50:18  d3x0r
// Synch with working repository.
//
// Revision 1.84  2005/04/25 17:32:39  jim
// Use AddWait instead of AddUse - AddWait does not dispatch redraw events...Also our refresh events are redundant-ated still.
//
// Revision 1.83  2005/03/30 11:36:38  panther
// Remove a lot of debugging messages...
//
// Revision 1.82  2005/03/22 12:41:58  panther
// Wow this transparency thing is going to rock! :) It was much closer than I had originally thought.  Need a new class of controls though to support click-masks.... oh yeah and buttons which have roundable scaleable edged based off of a dot/circle
//
// Revision 1.81  2005/03/13 23:34:35  panther
// Focus and mouse capture issues resolved for windows libraries... need to tinker with this same function within Linux.
//
// Revision 1.80  2005/03/12 23:31:21  panther
// Edit controls nearly works... have some issues with those dang popups.
//
// Revision 1.79  2005/03/06 11:21:40  panther
// Mouse works really good now, need to fix scrollbars next.  Also sheet control dialogs still have funk and need to be de-funked no wait they are defunct
//
// Revision 1.78  2005/03/04 19:07:32  panther
// Define SetItemText
//
// Revision 1.77  2005/02/28 22:31:45  panther
// Okay this should work for a moment... modified adding/setting NULL methods for controls
//
// Revision 1.76  2005/02/24 22:33:14  panther
// Begin modifications to allow multiple draw/key and mouse routings to be attached to a control - sub/super-classing ability
//
// Revision 1.75  2004/12/16 10:32:45  panther
// Scroll scrollbar to font... handle rendering the top and bottom buttons better... next to restore function to scrollbars.
//
// Revision 1.74  2004/12/16 06:53:30  panther
// Minor updates for edit controls
//
// Revision 1.73  2004/12/04 01:57:44  panther
// Don't destroy a child common control if it is in use - do unlink it and pretend we did most of the work.
//
// Revision 1.72  2004/12/02 08:49:13  panther
// Duh - fix check button drawing... should offer the option to attach the border to the frame of the control
//
// Revision 1.71  2004/11/29 11:29:53  panther
// Minor code cleanups, investigate incompatible display driver
//
// Revision 1.70  2004/11/05 02:34:51  d3x0r
// Minor mods...
//
// Revision 1.69  2004/10/24 20:09:46  d3x0r
// Sync to psilib2... stable enough to call it mainstream.
//
// Revision 1.13  2004/10/22 09:23:32  d3x0r
// Caption scaling correct.... all is going more well... need to test the verification thing tomowrrow...
//
// Revision 1.12  2004/10/21 16:45:51  d3x0r
// Updaes to dialog handling... still ahve  aproblem with caption resize
//
// Revision 1.11  2004/10/13 11:13:53  d3x0r
// Looks like this is cleaning up very nicely... couple more rough edges and it'll be good to go.
//
// Revision 1.10  2004/10/12 08:10:51  d3x0r
// checkpoint... frames are controls, and anything can be displayed...
//
// Revision 1.9  2004/10/09 00:30:26  d3x0r
// Progress... better drawings - hide and obscure seems to not work yet...
//
// Revision 1.8  2004/10/08 13:07:42  d3x0r
// Okay beginning to look a lot like PRO-GRESS
//
// Revision 1.7  2004/10/07 04:37:16  d3x0r
// Okay palette and listbox seem to nearly work... controls draw, now about that mouse... looks like my prior way of cheating is harder to step away from than I thought.
//
// Revision 1.6  2004/10/06 10:38:47  d3x0r
// Frames are created once again...
//
// Revision 1.5  2004/10/06 09:52:16  d3x0r
// checkpoint... total conversion... now how does it work?
//
// Revision 1.4  2004/10/05 00:58:27  d3x0r
// Checkpoint.
//
// Revision 1.3  2004/10/05 00:20:29  d3x0r
// Break out these rather meaty parts from controls.c
//
// Revision 1.2  2004/09/27 20:44:28  d3x0r
// Sweeping changes only a couple modules left...
//
// Revision 1.1  2004/09/19 19:22:30  d3x0r
// Begin version 2 psilib...
//
// Revision 1.72  2004/09/18 00:13:46  d3x0r
// checkpoint psi... hate this broken thing I did... but I should have done it from the start...
//
// Revision 1.71  2004/09/17 16:18:29  d3x0r
// ...
//
// Revision 1.70  2004/09/17 02:51:21  d3x0r
// checkpoint
//
// Revision 1.69  2004/09/16 03:48:43  d3x0r
// Once again I talk myself into fucking the world!
//
// Revision 1.68  2004/09/15 16:12:42  d3x0r
// Tear apart controls, commons and frames...
//
// Revision 1.67  2004/09/13 09:12:40  d3x0r
// Simplify procregsitration, standardize registration, cleanups...
//
// Revision 1.66  2004/09/09 00:54:20  d3x0r
// Compiles...
//
// Revision 1.65  2004/09/07 07:05:46  d3x0r
// Stablized up to palette dialog, which is internal... may require recompile binary upgrade may not work.
//
// Revision 1.64  2004/09/07 01:13:01  d3x0r
// Checkpoint - really really tempting to break all existing code....
//
// Revision 1.63  2004/09/06 23:38:56  d3x0r
// This control thing... I think I took a wrong turn somewhere there...
//
// Revision 1.62  2004/09/06 19:28:09  d3x0r
// Checkpoint... buttons are nearly ready to generically instance...
//
// Revision 1.61  2004/09/04 18:49:48  d3x0r
// Changes to support scaling and font selection of dialogs
//
// Revision 1.60  2004/09/03 14:43:48  d3x0r
// flexible frame reactions to font changes...
//
// Revision 1.59  2004/09/02 22:01:33  d3x0r
// Extended PSI Controls to have fonts on frames, and some controls to have private fonts.
//
// Revision 1.58  2004/08/29 18:52:12  d3x0r
// Yeah it's still broke.... need to do some work with hide/show child.
//
// Revision 1.57  2004/08/25 08:44:52  d3x0r
// Portability changes for MSVC... Updated projects, all projects build up to PSI, no display...
//
// Revision 1.56  2004/08/24 11:15:15  d3x0r
// Checkpoint Visual studio mods.
//
// Revision 1.55  2004/08/17 02:34:11  d3x0r
// begin implementation of lockout on controls...
//
// Revision 1.54  2004/06/01 21:54:13  d3x0r
// Fix definitions of functions
//
// Revision 1.53  2004/05/28 17:11:52  d3x0r
// Just clean clean build, distclean... make system...
//
// Revision 1.52  2004/05/27 23:39:16  d3x0r
// fix CONTROL_INIT_EX def for gcc
//
// Revision 1.51  2004/05/27 08:16:48  d3x0r
// Removed noisy intro logging.
//
// Revision 1.50  2004/05/27 00:08:11  d3x0r
// Checkpoint - whatever.
//
// Revision 1.49  2004/05/26 02:06:49  d3x0r
// Begin making registration a bit more efficient
//
// Revision 1.48  2004/05/24 21:05:54  d3x0r
// Checkpoint - all builds.
//
// Revision 1.47  2004/05/24 16:03:36  d3x0r
// Begin using registered data type
//
// Revision 1.46  2004/05/23 09:50:44  d3x0r
// Updates to extend dynamic edit dialogs.
//
// Revision 1.45  2004/05/22 00:39:57  d3x0r
// Lots of progress on dynamic editing of frames.
//
// Revision 1.46  2004/05/22 00:42:20  jim
// Score - specific property pages will work also.
//
// Revision 1.45  2004/05/21 18:12:59  jim
// Checkpoint, need to add registered functions to link to.
//
// Revision 1.44  2004/04/12 10:49:53  d3x0r
// checkpoint
//
// Revision 1.43  2003/11/23 08:42:20  panther
// Added option for frames to request always getting mouse messages
//
// Revision 1.42  2003/10/12 02:47:05  panther
// Cleaned up most var-arg stack abuse ARM seems to work.
//
// Revision 1.41  2003/10/06 19:04:13  panther
// Phase one lockdown to avoid destroy while in use
//
// Revision 1.40  2003/10/06 16:46:41  panther
// Begin scheduling flags for destruction
//
// Revision 1.39  2003/09/28 21:52:51  panther
// Include stdio.h to support FILE*
//
// Revision 1.38  2003/09/21 11:49:04  panther
// Fix service refernce counting for unload.  Fix Making sub images hidden.
// Fix Linking of services on server side....
// cleanup some comments...
//
// Revision 1.37  2003/09/18 12:14:49  panther
// MergeRectangle Added.  Seems Control edit near done (fixing move/size errors)
//
// Revision 1.36  2003/09/18 08:43:16  panther
// Move editcontrolprops public...
//
// Revision 1.35  2003/09/18 07:42:48  panther
// Changes all across the board...image support, display support, controls editable in psi...
//
// Revision 1.34  2003/09/15 17:06:37  panther
// Fixed to image, display, controls, support user defined clipping , nearly clearing correct portions of frame when clearing hotspots...
//
// Revision 1.33  2003/09/15 01:02:25  panther
// Well most of the work recovered... but still need partial update
//
// Revision 1.32  2003/09/13 17:06:29  panther
// Okay - and now we use stdargs... ugly kinda but okay...
//
// Revision 1.31  2003/09/13 11:33:42  panther
// Checkpoint dialog edit
//
// Revision 1.30  2003/09/11 22:07:11  panther
// Figure out that ## var arg constructs work with watcom, also minor mods... probably incoming conflicts.
//
// Revision 1.29  2003/09/11 16:55:01  panther
// Okay and progress on Load/Save, looks a little cumbersome, but perhaps usable...
//
// Revision 1.28  2003/09/11 14:14:31  panther
// Rough cut save and load code for frames...
//
// Revision 1.27  2003/09/11 13:09:25  panther
// Looks like we maintained integrety while overhauling the Make/Create/Init/Config interface for controls
//
// Revision 1.26  2003/07/24 23:47:22  panther
// 3rd pass visit of CPROC(cdecl) updates for callbacks/interfaces
//
// Revision 1.25  2003/05/01 19:18:15  panther
// broken - but will fix
//
// Revision 1.24  2003/05/01 18:55:52  panther
// Create control for subgframes, extra params
//
// Revision 1.23  2003/03/30 19:40:14  panther
// Encapsulate pick color data better.
//
// Revision 1.22  2003/03/28 22:37:28  panther
// Move control/frame focus var to common.
//
// Revision 1.21  2003/03/26 00:35:17  panther
// Handle Resizable borders! yay!
//
// Revision 1.20  2003/03/25 08:45:56  panther
// Added CVS logging tag
//
