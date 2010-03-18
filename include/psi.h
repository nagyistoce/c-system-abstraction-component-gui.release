/*
 * Crafted by: Jim Buckeyne
 *
 * Purpose: Provide a well defined, concise structure for
 *   describing controls.  The only methods that are well
 *   supported are create, destroy, draw, mouse, and keyboard
 *
 * Also- the method for extracting class private data
 *  from an abstract PSI_CONTROL handle is provided, called
 *  ValidatedControlData()
 *
 * A registration function that hooks into deadstart loading
 * is available so that registrations are complete by the time the
 * application starts in main.
 *
 * (c)Freedom Collective, Jim Buckeyne 2006; SACK Collection.
 *
 */



#ifndef PSI_STUFF_DEFINED
#define PSI_STUFF_DEFINED
#include <procreg.h>
#include <controls.h>

PSI_NAMESPACE

//---------------------------------------------------------------------------

typedef struct ControlRegistration_tag {
	CTEXTSTR name;
	struct {
		struct {
			_32 width, height;
		}
#ifdef __cplusplus
	stuff
#endif
		;
		_32 extra; // default width, height for right-click creation.
		_32 default_border;
		//CTEXTSTR master_config;
      //struct ControlRegistration_tag *pMasterConfig;
	}
#ifdef __cplusplus
	stuff
#endif
	;
	// set to initial values...
	// initial values
	int (CPROC*init)( PSI_CONTROL );
	int (CPROC*load)( PSI_CONTROL , PTEXT parameters );
	int (CPROC*draw)( PSI_CONTROL  );
	int (CPROC*mouse)( PSI_CONTROL , S_32 x, S_32 y, _32 b );
	int (CPROC*key)( PSI_CONTROL , _32 );
	void (CPROC*destroy)( PSI_CONTROL  );
	PSI_CONTROL (CPROC*prop_page)( PSI_CONTROL pc );
	void (CPROC*apply_prop)( PSI_CONTROL pc, PSI_CONTROL frame );
	void (CPROC*save)( PSI_CONTROL pc, PVARTEXT pvt );
	void (CPROC*AddedControl)(PSI_CONTROL me, PSI_CONTROL pcAdding );
	void (CPROC*CaptionChanged)( PSI_CONTROL pc );
	// bFocused will be true if control gains focus
	// if !bFocused, control is losing focus.
	// 1> with current focus is told it will lose focus (!bFocus).
	//   a> control may allow focus loss (return !FALSE) (goto 2)
	//   b> control may reject focus loss, which will force it to remain (return FALSE)
   // 1.5 > current focus reference of the container is cleared
	// 2> newly focused control is told it now has focus.
	//    a> it may accept the focus (return TRUE/!FALSE)
	//    b> it may reject the focus - and the focus will be left nowhere.
   // 2.5 > the current focus reference of the container is set
	int (CPROC*FocusChanged)( PSI_CONTROL pc, LOGICAL bFocused );

	// bstart is TRUE when the position starts changing
	// and is false when the change is done... this allows
   // a critical section to be entered and left around the resize.
   void (CPROC*PositionChanging)( PSI_CONTROL pc, LOGICAL bStart );

	// result data - uninitialized
	_32 TypeID; // this is filled in by the registrar (handler of registration)
} CONTROL_REGISTRATION, *PCONTROL_REGISTRATION;

#define LinePaste(a,b) a##b
#define LinePaste2(a,b) LinePaste(a,b)
#define SYMVAL(a) a
#define EasyRegisterControl( name, extra ) static CONTROL_REGISTRATION LinePaste2(ControlType,SYMVAL(__LINE__))= { name, { 32, 32, extra, BORDER_THINNER } }; PRELOAD( SimpleRegisterControl ){ DoRegisterControl( &LinePaste2(ControlType,SYMVAL(__LINE__)) ); } static P_32 _MyControlID = &LinePaste2(ControlType,SYMVAL(__LINE__)).TypeID;
#define EasyRegisterControlEx( name, extra, reg_name ) static CONTROL_REGISTRATION reg_name= { name, { 32, 32, extra, BORDER_THINNER } }; PRELOAD( SimpleRegisterControl##reg_name ){ DoRegisterControl( &reg_name ); } static P_32 _MyControlID##reg_name = &reg_name.TypeID;
#define EasyRegisterControlWithBorder( name, extra, border_flags ) static CONTROL_REGISTRATION LinePaste2(ControlType,SYMVAL(__LINE__))= { name, { 32, 32, extra, border_flags} }; PRELOAD( SimpleRegisterControl ){ DoRegisterControl( &LinePaste2(ControlType,SYMVAL(__LINE__)) ); } static P_32 _MyControlID = &LinePaste2(ControlType,SYMVAL(__LINE__)).TypeID;
#define EasyRegisterControlWithBorderEx( name, extra, border_flags, reg_name ) static CONTROL_REGISTRATION reg_name= { name, { 32, 32, extra, border_flags} }; PRELOAD( SimpleRegisterControl##reg_name ){ DoRegisterControl( &reg_name ); } static P_32 _MyControlID##reg_name = &reg_name.TypeID;
#define MyControlID (_MyControlID[0])
#define MyControlIDEx(n) (_MyControlID##n[0])
#define MyValidatedControlData( type, result, pc ) ValidatedControlData( type, MyControlID, result, pc )
#define MyValidatedControlDataEx( type, reg_name, result, pc ) ValidatedControlData( type, MyControlIDEx(reg_name), result, pc )

//#define CONTROL_METHODS(draw,mouse,key,destroy) {{"draw",draw},{"mouse",mouse},{"key",key},{"destroy",destroy}}
//---------------------------------------------------------------------------

// please fill out the required forms, submit them once and only once.
// see form definition above, be sure to sign them.
	// in the future (after now), expansion can be handled by observing the size
	// of the registration entry.  at sizeof(registration) - 4 is always the
   // type ID result of this registration...
	PSI_PROC( int, DoRegisterControl )( PCONTROL_REGISTRATION pcr, int sizeof_registration );
#define DoRegisterControl(pcr) DoRegisterControl( pcr, sizeof(*pcr) )
//PSI_PROC( int, DoRegisterSubcontrol )( PSUBCONTROL_REGISTRATION pcr );

#define ControlData(type,common) ((common)?(*((type*)(common))):NULL)
#define SetControlData(type,common,pf) (*((type*)(common))) = (type)(pf)
#define ValidatedControlData(type,id,result,com) type result = (((com)&&(ControlType(com)==(id)))?ControlData(type,com):NULL)

#define OnHideCommon(name) \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),HideCommon,WIDE("control"),name WIDE("/hide_control"),PASTE(name,WIDE("hide_control")),void,(PSI_CONTROL),__LINE__)
#define OnRevealCommon(name) \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),RevealCommon,WIDE("control"),name WIDE("/reveal_control"),WIDE("reveal_control"),void,(PSI_CONTROL),__LINE__)

#define OnCreateCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("init"),int,(PSI_CONTROL), __LINE__)
#define OnDrawCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("draw"),int,(PSI_CONTROL), __LINE__)
#define OnMouseCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("mouse"),int,(PSI_CONTROL,S_32,S_32,_32), __LINE__)
#define OnKeyCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("key"),int,(PSI_CONTROL,_32), __LINE__)
#define OnMoveCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("position_changing"),void,(PSI_CONTROL,LOGICAL), __LINE__)
#define OnSizeCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("resize"),void,(PSI_CONTROL,LOGICAL), __LINE__)
#define OnMotionCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("some_parents_position_changing"),void,(PSI_CONTROL,LOGICAL), __LINE__)

#define OnDestroyCommon(name)  \
	__DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),unused_name,WIDE("control"),name WIDE("/rtti"),WIDE("destroy"),void,(PSI_CONTROL), __LINE__)

// return a frame page to the caller for display.
#define OnPropertyEdit(name) \
	DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),PropertyEditControl,WIDE("control"),name WIDE("/rtti"),WIDE("get_property_page"),PSI_CONTROL,(PSI_CONTROL))

#define OnCommonFocus(name) \
	DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),FocusChanged,WIDE("control"),name WIDE("/rtti"),WIDE("focus_changed"),void,(PSI_CONTROL,LOGICAL))

// The frame edit mode has begun, and controls are given an
// opportunity to make life good for themselves and those around
// them - such as sheet controls displaying sheets seperatly.
#define OnEditFrame(name) \
	DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),FrameEditControl,WIDE("control"),name WIDE("/rtti"),WIDE("begin_frame_edit"),void,(PSI_CONTROL))
#define OnEditFrameDone(name) \
	DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),FrameEditDoneControl,WIDE("control"),name WIDE("/rtti"),WIDE("end_frame_edit"),void,(PSI_CONTROL))
// somet
//#define OnFrameEdit(name)
//	DefineRegistryMethod(WIDE("psi"),FrameEditControl,"common",name,"begin_edit",void,(void))

// on okay - read your information for ( your control, from the frame )
#define OnPropertyEditOkay(name) \
	DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),PropertyEditOkayControl,WIDE("control"),name WIDE("/rtti"),WIDE("read_property_page"),void,(PSI_CONTROL,PSI_CONTROL))

// on cancel return void ( your_control, the sheet your resulted to get_property_page
#define OnPropertyEditCancel(name) \
	DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),PropertyEditCancelControl,WIDE("control"),name WIDE("/rtti"),WIDE("abort_property_page"),void,(PSI_CONTROL,PSI_CONTROL))

// some controls may change their appearance and drawing characteristics based on
// having their properties edited.  This is done after either read or abort is done, also
// after the container dialog is destroyed, thereby indicating that any reference to the frame
// you created is now gone, unless you did magic.
#define OnPropertyEditDone( name )  \
	DefineRegistryMethodP(PSI_PRELOAD_PRIORITY,WIDE("psi"),PropertyEditDoneControl,WIDE("control"),name WIDE("/rtti"),WIDE("done_property_page"),void,(PSI_CONTROL))

// just a passing thought.
//#define OnEditFrameBegin( name )
//	DefineRegistryMethod(WIDE("psi"),EditFrameBegin,"common",name,"frame_edit_begin",void,(PSI_CONTROL))

// and here we can use that fancy declare method thing
// to register the appropriate named things LOL!
//


PSI_NAMESPACE_END

#endif
