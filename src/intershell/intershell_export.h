#ifndef InterShell_EXPORT
#define InterShell_EXPORT
#include <sack_types.h>
#include <genxml/genx.h>
#include <configscript.h>
#define _DEFINE_INTERFACE

//!defined(__STATIC__) &&
#    define INTERSHELL_PROC_PTR(type,name)  type (CPROC* name)

#if !defined(__LINUX__)
#ifdef INTERSHELL_SOURCE
#define INTERSHELL_PROC(type,name) type CPROC name
#else
#define INTERSHELL_PROC(type,name) extern type CPROC name
#endif
#else
#ifdef INTERSHELL_SOURCE
#define INTERSHELL_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define INTERSHELL_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#endif

//#include "intershell_local.h"
#include "intershell_button.h"

typedef struct page_data_tag PAGE_DATA, *PPAGE_DATA;


// pabel label is actually just a text label thing... 
typedef struct page_label *PPAGE_LABEL;
typedef struct font_preset_tag *PFONT_PRESET;


//-------------------------------------------------------
//   Dynamic variable declaration - used within button/text_label contexts...
//-------------------------------------------------------
//
// PVARIABLE types are used in Other/Text Label
//   a %<varname> matches the name exactly
//   a revision needs to be done to offer a different variable sepearateor that can
//   imply sub function... suppose a simple xml stealing of <varname> will work
//   implying that optional paramters may be <function param1=value1 ...>
// CreateLabelVariable returns a value that may be used in...
// LabelVariableChanged
//  to cause the update of all boxes which may or may not be visible at the time, and
//  will cause approrpiate refresh
//

typedef struct variable_tag *PVARIABLE;

/* moved from text_label.h
 these values need to be exposed to peer modules
 */
enum label_variable_types {
   /* POINTER data should be the address of a CTEXSTR (a pointer to the pointer of a string )*/
	LABEL_TYPE_STRING
      /* POINTER data should be the address of an integer, changing the integer will reflect in the output*/
								  , LABEL_TYPE_INT
                          /* POINTER data is the address of a routine which takes (void) parameters and returns a CTEXTSTR*/
								  , LABEL_TYPE_PROC
								  /* POINTER data is the address of a routine which takes a (PTRSZVAL) and returns a CTEXTSTR */
                          /* PTRSZVAL psv is user data to pass to the procedure */
								  , LABEL_TYPE_PROC_EX

								  /* POINTER data is the address of a routine which takes a (PTRSZVAL) and returns a CTEXTSTR */
								  /* PTRSZVAL psv is user data to pass to the procedure */
                          /* routine also gets the control the text is contained on? */
								  , LABEL_TYPE_PROC_CONTROL_EX
                          /* POINTER data is a pointer to a simple string, the value is copied and used on the control */
	, LABEL_TYPE_CONST_STRING
};
typedef CTEXTSTR  *label_string_value;
typedef _32       *label_int_value;
typedef CTEXTSTR (*label_gettextproc)(void);
typedef CTEXTSTR (*label_gettextproc_ex)(PTRSZVAL);
typedef CTEXTSTR (*label_gettextproc_control)(PTRSZVAL, PMENU_BUTTON);

#ifdef _DEFINE_INTERFACE
struct intershell_interface {

// magically knows which button we're editing at the moment.
// intended to only be used during OnEditControl() Event Handler
INTERSHELL_PROC_PTR( void, GetCommonButtonControls )( PSI_CONTROL frame );
// magically knows which button we're editing at the moment.
// intended to only be used during OnEditControl() Event Handler
INTERSHELL_PROC_PTR( void, SetCommonButtonControls )( PSI_CONTROL frame );

// wake up menu processing... there's a flag that was restart that this thinks
// it might want...
INTERSHELL_PROC_PTR( void, RestartMenu )( PTRSZVAL psv, _32 keycode );
INTERSHELL_PROC_PTR( void, ResumeMenu )( PTRSZVAL psv, _32 keycode );


// a zero (0) passed as a primary/secondary or tertiary color indicates no change. (err or disable)
#define COLOR_DISABLE 0x00010000 // okay transparent level 1 red is disable key. - cause that's such a useful color alone.
#define COLOR_IGNORE  0x00000000
INTERSHELL_PROC_PTR( void, InterShell_GetButtonColors )( PMENU_BUTTON button
													, CDATA *cText
													, CDATA *cBackground1
													, CDATA *ring_color
													, CDATA *highlight_ring_color );
INTERSHELL_PROC_PTR( void, InterShell_SetButtonColors )( PMENU_BUTTON button, CDATA cText, CDATA cBackground1, CDATA cBackground2, CDATA cBackground3 );
INTERSHELL_PROC_PTR( void, InterShell_SetButtonColor )( PMENU_BUTTON button, CDATA primary, CDATA secondary );
INTERSHELL_PROC_PTR( void, InterShell_SetButtonText )( PMENU_BUTTON button, CTEXTSTR text );
INTERSHELL_PROC_PTR( void, InterShell_GetButtonText )( PMENU_BUTTON button, TEXTSTR text, int text_buf_len );
INTERSHELL_PROC_PTR( void, InterShell_SetButtonImage )( PMENU_BUTTON button, CTEXTSTR name );
#ifndef __NO_ANIMATION__
//INTERSHELL_PROC_PTR( void, InterShell_SetButtonAnimation )( PMENU_BUTTON button, CTEXTSTR name );
#endif
INTERSHELL_PROC_PTR( Image, InterShell_CommonImageLoad )( CTEXTSTR name );
INTERSHELL_PROC_PTR( void, InterShell_CommonImageUnloadByName )( CTEXTSTR name );
INTERSHELL_PROC_PTR( void, InterShell_CommonImageUnloadByImage )( Image unload );
/*
 *  For InterShell_SetButtonImageAlpha.....
 *    0 is no alpha change.
 *    alpha < 0 increases transparency
 *    alpha > 0 increases opacity.
 *    Max value is +/-255
 */
INTERSHELL_PROC_PTR( void, InterShell_SetButtonImageAlpha )( PMENU_BUTTON button, S_16 alpha );


// return if the button is just virtual (part of a macro)
INTERSHELL_PROC_PTR( LOGICAL, InterShell_IsButtonVirtual )( PMENU_BUTTON button );

INTERSHELL_PROC_PTR( void, InterShell_SetButtonFont )( PMENU_BUTTON button, Font *font );
// THis function returns the font of the current button being edited...
// this result can be used for controls that are not really buttons to get the common
// properties of the font being used for this control...
INTERSHELL_PROC_PTR( Font*, InterShell_GetCurrentButtonFont )( void );
INTERSHELL_PROC_PTR( void, InterShell_SetButtonStyle )( PMENU_BUTTON button, char *style );
INTERSHELL_PROC_PTR( void, InterShell_SaveCommonButtonParameters )( FILE *file );
INTERSHELL_PROC_PTR( CTEXTSTR, InterShell_GetSystemName )( void );
//INTERSHELL_PROC_PTR( void, UpdateButtonEx )( PMENU_BUTTON button, int bEndingEdit );
INTERSHELL_PROC_PTR( void, UpdateButtonExx )( PMENU_BUTTON button, int bEndingEdit DBG_PASS );
#define UpdateButtonEx( button, edit ) UpdateButtonExx( button, edit DBG_SRC )
//#define UpdateButtonEx( button, edit ) UpdateButtonExx( button, edit DBG_SRC )
#define UpdateButton(button) UpdateButtonEx( button, TRUE )

// fixup button has been depricated for external usage
//  please use UpdateButton instead. (which does also invoke fixup)
// UpdateButton
//INTERSHELL_PROC_PTR( void, FixupButtonEx )( PMENU_BUTTON button DBG_PASS);
#define FixupButton(b) FixupButtonEx((b) DBG_SRC)

//---------------------------------------
// pages controls here...
//
INTERSHELL_PROC_PTR(PPAGE_DATA, ShellGetCurrentPageEx)( PSI_CONTROL pc_canvas_or_control_in_canvas);
INTERSHELL_PROC_PTR(PPAGE_DATA, ShellGetCurrentPage)( void );
// pass pc NULL defaults internally to using the main frame surface.  The page
// name returns the current page of that name.
INTERSHELL_PROC_PTR(PPAGE_DATA, ShellGetNamedPage)( PSI_CONTROL pc, CTEXTSTR pagename );
// special names
// start, next, prior are keywords that imply direct
// page stacking.
INTERSHELL_PROC_PTR( int, ShellSetCurrentPage )( CTEXTSTR name );
INTERSHELL_PROC_PTR( int, ShellSetCurrentPageEx )( PSI_CONTROL pc, CTEXTSTR name );

// a call will push the current page on a stack
// which will be returned to if returncurrentpage is used.
// clear page list will flush the stack cause
// there is such a temptation to call to all pages, providing
// near inifinite page recall (back back back)
INTERSHELL_PROC_PTR( int, ShellCallSetCurrentPage )( CTEXTSTR name );
INTERSHELL_PROC_PTR( int, ShellCallSetCurrentPageEx )( PSI_CONTROL pc_canvas, CTEXTSTR name );

INTERSHELL_PROC_PTR( void, ShellReturnCurrentPage )( void );
INTERSHELL_PROC_PTR( void, ClearPageList )( void );
// disable updates on the page, disable updating of buttons...
INTERSHELL_PROC_PTR( void, InterShell_DisablePageUpdate )( LOGICAL bDisable );
INTERSHELL_PROC_PTR( void, RestoreCurrentPage )( PSI_CONTROL pc_canvas );
INTERSHELL_PROC_PTR( void, HidePageExx )( PSI_CONTROL pc_canvas DBG_PASS);
#define HidePageEx2(page) HidePageExx( page DBG_SRC )



//---------------------------------------
// this sets a one time flag on a button which disables
// the auto page change which may be assigned to a button.
//   tend/issue/perform these types of verbs may fail, and this is the only
//   sort of thing at the moment that happens... perhaps renaming this to
// button_abort_function could be done?
INTERSHELL_PROC_PTR( void, InterShell_DisableButtonPageChange )( PMENU_BUTTON button );

INTERSHELL_PROC_PTR( PVARIABLE, CreateLabelVariable )( CTEXTSTR name, enum label_variable_types type, CPOINTER data );
INTERSHELL_PROC_PTR( PVARIABLE, CreateLabelVariableEx )( CTEXTSTR name, enum label_variable_types type, CPOINTER data, PTRSZVAL psv );
// pass NULL to update all labels, otherwise, one can pass the result of a CreateLableVariable
// to update only text labels using that variable.
INTERSHELL_PROC_PTR( void, LabelVariableChanged )( PVARIABLE ); // update any labels which are using this variable.
INTERSHELL_PROC_PTR( void, LabelVariablesChanged )( PLIST ); // update any labels which are using this variable. list of PVARIABLE types

// local export to allow exxternal plugins to control whether the main display is showing...
//  (specially for Tasks at this time... when an exclusive task is launched, display is hidden)
INTERSHELL_PROC_PTR( void, InterShell_Hide )( void );
INTERSHELL_PROC_PTR( void, InterShell_Reveal )( void );


//----------------------------------------------------------
//
INTERSHELL_PROC_PTR( void, GetPageSize )( P_32 width, P_32 height );

//-----------------------------------------------------
// layout members which have a position x, y, font, text and color of their own
// may be created on buttons.  They are displayed below the lense/ridge[up/down] and above the background.
INTERSHELL_PROC_PTR( void, SetButtonTextField )( PMENU_BUTTON pKey, PTEXT_PLACEMENT pField, char *text );
INTERSHELL_PROC_PTR( PTEXT_PLACEMENT, AddButtonLayout )( PMENU_BUTTON pKey, int x, int y, Font *font, CDATA color, _32 flags );


//-----------------------------------------------------
// this provides low level access to a button, let the programmer beware.
INTERSHELL_PROC_PTR( PSI_CONTROL, InterShell_GetButtonControl )( PMENU_BUTTON button );

// result in substituted text from variables registered for InterShell
// if called from a context that does not have PPAGE_LABEL, pass NULL
INTERSHELL_PROC_PTR( CTEXTSTR, InterShell_GetLabelText )( PPAGE_LABEL label, CTEXTSTR variable );
// use of this is preferred, otherwise thread conflicts will destroy the buffer.
INTERSHELL_PROC_PTR( CTEXTSTR, InterShell_TranslateLabelText )( PPAGE_LABEL label, TEXTSTR output, int buffer_len, CTEXTSTR variable );

/* actual worker function for InterShell_GetLabelText - but suport dispatch to bProcControlEx*/
INTERSHELL_PROC_PTR( CTEXTSTR, InterShell_GetControlLabelText )( PMENU_BUTTON button, PPAGE_LABEL label, CTEXTSTR variable );

//-----------------------------------------------------

//-------- FONTS -----------------------------------------------------


INTERSHELL_PROC_PTR( Font *, SelectAFont )( PSI_CONTROL parent, CTEXTSTR *default_name );
INTERSHELL_PROC_PTR( Font *, UseAFont )( CTEXTSTR name );

// depricated - used for forward migration...
INTERSHELL_PROC_PTR( Font *, CreateAFont )( CTEXTSTR name, Font font, POINTER data, _32 datalen );

INTERSHELL_PROC_PTR( void, BeginCanvasConfiguration )( PSI_CONTROL pc_canvas );
INTERSHELL_PROC_PTR( void, SaveCanvasConfiguration )( FILE *file, PSI_CONTROL pc_canvas );
INTERSHELL_PROC_PTR( void, SaveCanvasConfiguration_XML )( genxWriter w, PSI_CONTROL pc_canvas );
INTERSHELL_PROC_PTR( PCONFIG_HANDLER, InterShell_GetCurrentConfigHandler )( void );

//PTRSZVAL GetButtonExtension( struct menu_button_tag * button );

//void AddCommonButtonConfig( PCONFIG_HANDLER pch, struct menu_button_tag * button );
//void DumpCommonButton( FILE *file, struct menu_button_tag * button );

//PTRSZVAL GetButtonExtension( struct menu_button_tag * button );


// BeginSubConfiguration....
//   colntrol_type_name is a InterShell widget path/name for the type of
//   other info to save... the method for setting additional configuration methods
//   is invoked by thisname.
//   Then end_type_name is the last string which will close the subconfiguration.
INTERSHELL_PROC_PTR( LOGICAL, BeginSubConfiguration )( char *control_type_name, const char *end_type_name );
INTERSHELL_PROC_PTR( CTEXTSTR, EscapeMenuString )( CTEXTSTR string );
INTERSHELL_PROC_PTR( PMENU_BUTTON, InterShell_GetCurrentLoadingControl )( void );


INTERSHELL_PROC_PTR( Font*, InterShell_GetButtonFont )( PMENU_BUTTON pc );
INTERSHELL_PROC_PTR( CTEXTSTR, InterShell_GetButtonFontName )( PMENU_BUTTON pc );
INTERSHELL_PROC_PTR( PMENU_BUTTON, InterShell_GetCurrentButton )( void );
INTERSHELL_PROC_PTR( void, InterShell_SetButtonFontName )( PMENU_BUTTON button, CTEXTSTR name );

// return the physical button associate with this button (might be a macro element... and it might want to update the outer button's status.)
INTERSHELL_PROC_PTR( PMENU_BUTTON, InterShell_GetPhysicalButton )( PMENU_BUTTON button );

INTERSHELL_PROC_PTR( void, InterShell_SetButtonHighlight )( PMENU_BUTTON button, LOGICAL bEnable );

INTERSHELL_PROC_PTR( PTRSZVAL,  InterShell_CreateControl )( CTEXTSTR type, int x, int y, int w, int h );

};  //struct intershell_interface {

#endif

#ifdef INTERSHELL_SOURCE
// magically knows which button we're editing at the moment.
// intended to only be used during OnEditControl() Event Handler
INTERSHELL_PROC( void, GetCommonButtonControls )( PSI_CONTROL frame );
// magically knows which button we're editing at the moment.
// intended to only be used during OnEditControl() Event Handler
INTERSHELL_PROC( void, SetCommonButtonControls )( PSI_CONTROL frame );

// wake up menu processing... there's a flag that was restart that this thinks
// it might want...
INTERSHELL_PROC( void, RestartMenu )( PTRSZVAL psv, _32 keycode );
INTERSHELL_PROC( void, ResumeMenu )( PTRSZVAL psv, _32 keycode );


// a zero (0) passed as a primary/secondary or tertiary color indicates no change. (err or disable)
#define COLOR_DISABLE 0x00010000 // okay transparent level 1 red is disable key. - cause that's such a useful color alone.
#define COLOR_IGNORE  0x00000000
INTERSHELL_PROC( void, InterShell_GetButtonColors )( PMENU_BUTTON button
													, CDATA *cText
													, CDATA *cBackground1
													, CDATA *ring_color
													, CDATA *highlight_ring_color );
INTERSHELL_PROC( void, InterShell_SetButtonColors )( PMENU_BUTTON button, CDATA cText, CDATA cBackground1, CDATA cBackground2, CDATA cBackground3 );
INTERSHELL_PROC( void, InterShell_SetButtonColor )( PMENU_BUTTON button, CDATA primary, CDATA secondary );
INTERSHELL_PROC( void, InterShell_SetButtonText )( PMENU_BUTTON button, CTEXTSTR text );
INTERSHELL_PROC( void, InterShell_GetButtonText )( PMENU_BUTTON button, TEXTSTR text, int text_buf_len );
INTERSHELL_PROC( void, InterShell_SetButtonImage )( PMENU_BUTTON button, CTEXTSTR name );
INTERSHELL_PROC( void, InterShell_SetButtonHighlight )( PMENU_BUTTON button, LOGICAL bEnable );
#ifndef __NO_ANIMATION__
//INTERSHELL_PROC( void, InterShell_SetButtonAnimation )( PMENU_BUTTON button, CTEXTSTR name );
#endif
INTERSHELL_PROC( Image, InterShell_CommonImageLoad )( CTEXTSTR name );
INTERSHELL_PROC( void, InterShell_CommonImageUnloadByName )( CTEXTSTR name );
INTERSHELL_PROC( void, InterShell_CommonImageUnloadByImage )( Image unload );
/*
 *  For InterShell_SetButtonImageAlpha.....
 *    0 is no alpha change.
 *    alpha < 0 increases transparency
 *    alpha > 0 increases opacity.
 *    Max value is +/-255
 */
INTERSHELL_PROC( void, InterShell_SetButtonImageAlpha )( PMENU_BUTTON button, S_16 alpha );


// return if the button is just virtual (part of a macro)
INTERSHELL_PROC( LOGICAL, InterShell_IsButtonVirtual )( PMENU_BUTTON button );

INTERSHELL_PROC( PMENU_BUTTON, InterShell_GetPhysicalButton )( PMENU_BUTTON button );

INTERSHELL_PROC( void, InterShell_SetButtonFont )( PMENU_BUTTON button, Font *font );
// THis function returns the font of the current button being edited...
// this result can be used for controls that are not really buttons to get the common
// properties of the font being used for this control...
INTERSHELL_PROC( void, InterShell_SetButtonStyle )( PMENU_BUTTON button, char *style );
INTERSHELL_PROC( void, InterShell_SaveCommonButtonParameters )( FILE *file );
INTERSHELL_PROC( CTEXTSTR, InterShell_GetSystemName )( void );
//INTERSHELL_PROC( void, UpdateButtonEx )( PMENU_BUTTON button, int bEndingEdit );
INTERSHELL_PROC( void, UpdateButtonExx )( PMENU_BUTTON button, int bEndingEdit DBG_PASS );
#define UpdateButtonEx( button, edit ) UpdateButtonExx( button, edit DBG_SRC )
//#define UpdateButtonEx( button, edit ) UpdateButtonExx( button, edit DBG_SRC )
#define UpdateButton(button) UpdateButtonEx( button, TRUE )

// fixup button has been depricated for external usage
//  please use UpdateButton instead. (which does also invoke fixup)
// UpdateButton
//INTERSHELL_PROC( void, FixupButtonEx )( PMENU_BUTTON button DBG_PASS);
#define FixupButton(b) FixupButtonEx((b) DBG_SRC)

//---------------------------------------
// pages controls here...
//
INTERSHELL_PROC(PPAGE_DATA, ShellGetCurrentPageEx)( PSI_CONTROL pc_canvas_or_control_in_canvas);
INTERSHELL_PROC(PPAGE_DATA, ShellGetCurrentPage)( void );
// pass pc NULL defaults internally to using the main frame surface.  The page
// name returns the current page of that name.
INTERSHELL_PROC(PPAGE_DATA, ShellGetNamedPage)( PSI_CONTROL pc, CTEXTSTR pagename );
// special names
// start, next, prior are keywords that imply direct
// page stacking.
INTERSHELL_PROC( int, ShellSetCurrentPage )( CTEXTSTR name );
INTERSHELL_PROC( int, ShellSetCurrentPageEx )( PSI_CONTROL pc, CTEXTSTR name );

// a call will push the current page on a stack
// which will be returned to if returncurrentpage is used.
// clear page list will flush the stack cause
// there is such a temptation to call to all pages, providing
// near inifinite page recall (back back back)
INTERSHELL_PROC( int, ShellCallSetCurrentPage )( CTEXTSTR name );
INTERSHELL_PROC( int, ShellCallSetCurrentPageEx )( PSI_CONTROL pc_canvas, CTEXTSTR name );

INTERSHELL_PROC( void, ShellReturnCurrentPage )( void );
INTERSHELL_PROC( void, ClearPageList )( void );
// disable updates on the page, disable updating of buttons...
INTERSHELL_PROC( void, InterShell_DisablePageUpdate )( LOGICAL bDisable );
INTERSHELL_PROC( void, RestoreCurrentPage )( PSI_CONTROL pc_canvas );
INTERSHELL_PROC( void, HidePageExx )( PSI_CONTROL pc_canvas DBG_PASS);



//---------------------------------------
// this sets a one time flag on a button which disables
// the auto page change which may be assigned to a button.
//   tend/issue/perform these types of verbs may fail, and this is the only
//   sort of thing at the moment that happens... perhaps renaming this to
// button_abort_function could be done?
INTERSHELL_PROC( void, InterShell_DisableButtonPageChange )( PMENU_BUTTON button );

INTERSHELL_PROC( PVARIABLE, CreateLabelVariable )( CTEXTSTR name, enum label_variable_types type, CPOINTER data );
INTERSHELL_PROC( PVARIABLE, CreateLabelVariableEx )( CTEXTSTR name, enum label_variable_types type, CPOINTER data, PTRSZVAL psv );
// pass NULL to update all labels, otherwise, one can pass the result of a CreateLableVariable
// to update only text labels using that variable.
INTERSHELL_PROC( void, LabelVariableChanged )( PVARIABLE ); // update any labels which are using this variable.
INTERSHELL_PROC( void, LabelVariablesChanged )( PLIST ); // update any labels which are using this variable. list of PVARIABLE types

// local export to allow exxternal plugins to control whether the main display is showing...
//  (specially for Tasks at this time... when an exclusive task is launched, display is hidden)
INTERSHELL_PROC( void, InterShell_Hide )( void );
INTERSHELL_PROC( void, InterShell_Reveal )( void );


//----------------------------------------------------------
//
INTERSHELL_PROC( void, GetPageSize )( P_32 width, P_32 height );

//-----------------------------------------------------
// layout members which have a position x, y, font, text and color of their own
// may be created on buttons.  They are displayed below the lense/ridge[up/down] and above the background.
INTERSHELL_PROC( void, SetButtonTextField )( PMENU_BUTTON pKey, PTEXT_PLACEMENT pField, char *text );
INTERSHELL_PROC( PTEXT_PLACEMENT, AddButtonLayout )( PMENU_BUTTON pKey, int x, int y, Font *font, CDATA color, _32 flags );


//-----------------------------------------------------
// this provides low level access to a button, let the programmer beware.
INTERSHELL_PROC( PSI_CONTROL, InterShell_GetButtonControl )( PMENU_BUTTON button );

//---------------------------------------------------
// text_label.h
INTERSHELL_PROC( CTEXTSTR, InterShell_GetLabelText )( PPAGE_LABEL label, CTEXTSTR variable );
// use of this is preferred, otherwise thread conflicts will destroy the buffer.
INTERSHELL_PROC( CTEXTSTR, InterShell_TranslateLabelText )( PPAGE_LABEL label, TEXTSTR output, int buffer_len, CTEXTSTR variable );

/* actual worker function for InterShell_GetLabelText - but suport dispatch to bProcControlEx*/
INTERSHELL_PROC( CTEXTSTR, InterShell_GetControlLabelText )( PMENU_BUTTON button, PPAGE_LABEL label, CTEXTSTR variable );

//  ---- FONTS ------
INTERSHELL_PROC( Font *, SelectAFont )( PSI_CONTROL parent, CTEXTSTR *default_name );
INTERSHELL_PROC( Font *, UseAFont )( CTEXTSTR name );

// depricated - used for forward migration...
INTERSHELL_PROC( Font *, CreateAFont )( CTEXTSTR name, Font font, POINTER data, _32 datalen );

// ----- LOAD SAVE Stuff--------------------------------------
INTERSHELL_PROC( void, BeginCanvasConfiguration )( PSI_CONTROL pc_canvas );
INTERSHELL_PROC( void, SaveCanvasConfiguration )( FILE *file, PSI_CONTROL pc_canvas );
INTERSHELL_PROC( void, SaveCanvasConfiguration_XML )( genxWriter w, PSI_CONTROL pc_canvas );
INTERSHELL_PROC( PCONFIG_HANDLER, InterShell_GetCurrentConfigHandler )( void );
INTERSHELL_PROC( PMENU_BUTTON, InterShell_GetCurrentLoadingControl )( void );

//PTRSZVAL GetButtonExtension( struct menu_button_tag * button );

//void AddCommonButtonConfig( PCONFIG_HANDLER pch, struct menu_button_tag * button );
//void DumpCommonButton( FILE *file, struct menu_button_tag * button );

//PTRSZVAL GetButtonExtension( struct menu_button_tag * button );


// BeginSubConfiguration....
//   colntrol_type_name is a InterShell widget path/name for the type of
//   other info to save... the method for setting additional configuration methods
//   is invoked by thisname.
//   Then end_type_name is the last string which will close the subconfiguration.
INTERSHELL_PROC( LOGICAL, BeginSubConfiguration )( char *control_type_name, const char *end_type_name );
INTERSHELL_PROC( CTEXTSTR, EscapeMenuString )( CTEXTSTR string );

INTERSHELL_PROC( PTRSZVAL,  InterShell_CreateControl )( CTEXTSTR type, int x, int y, int w, int h );



#endif

#ifdef USES_INTERSHELL_INTERFACE
#  ifndef DEFINES_INTERSHELL_INTERFACE
extern 
#  endif
	 struct intershell_interface *InterShell
#ifdef __GCC__
	 __attribute__((visibility("hidden")))
#endif
	 ;

#  ifdef DEFINES_INTERSHELL_INTERFACE
// this needs to be done before most modules can run their PRELOADS...so just move this one.
// somehow this ended up as 69 and 69 was also PRELOAD() priority... bad.
PRIORITY_PRELOAD( InitInterShellInterface, DEFAULT_PRELOAD_PRIORITY - 3)
{
	InterShell = (struct intershell_interface*)GetInterface( "intershell" );
}

#  endif

#endif

#ifndef INTERSHELL_SOURCE
#define InterShell_CreateControl                                ( !InterShell )?0:InterShell->InterShell_CreateControl
#define  GetCommonButtonControls                               if( InterShell )InterShell->GetCommonButtonControls 
#define  SetCommonButtonControls							   if( InterShell )InterShell->SetCommonButtonControls 
#define  RestartMenu										   if( InterShell )InterShell->RestartMenu 
#define  ResumeMenu											   if( InterShell )InterShell->ResumeMenu 
#define  InterShell_GetButtonColors								   if( InterShell )InterShell->InterShell_GetButtonColors 
#define  InterShell_SetButtonColors								   if( InterShell )InterShell->InterShell_SetButtonColors 
#define  InterShell_SetButtonColor								   if( InterShell )InterShell->InterShell_SetButtonColor 
#define  InterShell_SetButtonText									   if( InterShell )InterShell->InterShell_SetButtonText 
#define  InterShell_GetButtonText									   if( InterShell )InterShell->InterShell_GetButtonText 
#define  InterShell_SetButtonImage								   if( InterShell )InterShell->InterShell_SetButtonImage 
#define  InterShell_SetButtonAnimation							   if( InterShell )InterShell->InterShell_SetButtonAnimation 
#define  InterShell_CommonImageLoad								   if( InterShell )InterShell->InterShell_CommonImageLoad 
#define  InterShell_CommonImageUnloadByName						   if( InterShell )InterShell->InterShell_CommonImageUnloadByName 
#define  InterShell_CommonImageUnloadByImage						   if( InterShell )InterShell->InterShell_CommonImageUnloadByImage 
#define  InterShell_SetButtonImageAlpha							   if( InterShell )InterShell->InterShell_SetButtonImageAlpha 
#define  InterShell_IsButtonVirtual								   ( !InterShell )?NULL:InterShell->InterShell_IsButtonVirtual
#define  InterShell_SetButtonFont									   if( InterShell )InterShell->InterShell_SetButtonFont 
#define  InterShell_GetCurrentButtonFont							   ( !InterShell )?NULL:InterShell->InterShell_GetCurrentButtonFont
#define  InterShell_SetButtonStyle								   if( InterShell )InterShell->InterShell_SetButtonStyle 
#define  InterShell_SaveCommonButtonParameters					   if( InterShell )InterShell->InterShell_SaveCommonButtonParameters 
#define  InterShell_GetSystemName									   ( !InterShell )?"NoInterShell":InterShell->InterShell_GetSystemName
#define  UpdateButtonExx									   if( InterShell )InterShell->UpdateButtonExx 
#define  ShellGetCurrentPageEx(x)								   (( !InterShell )?(PPAGE_DATA)NULL:InterShell->ShellGetCurrentPageEx(x))
#define  ShellGetCurrentPage()								   (( !InterShell )?NULL:InterShell->ShellGetCurrentPage())
#define  ShellGetNamedPage									   ( !InterShell )?NULL:InterShell->ShellGetNamedPage
#define  ShellSetCurrentPage								   if( InterShell )InterShell->ShellSetCurrentPage
#define  ShellSetCurrentPageEx								   if( InterShell )InterShell->ShellSetCurrentPageEx 
#define  ShellCallSetCurrentPage							   ( InterShell )?FALSE:InterShell->ShellCallSetCurrentPage
#define  ShellCallSetCurrentPageEx							   if( InterShell )InterShell->ShellCallSetCurrentPageEx 
#define  ShellReturnCurrentPage								   if( InterShell )InterShell->ShellReturnCurrentPage 
#define  ClearPageList										   if( InterShell )InterShell->ClearPageList 
#define  InterShell_DisablePageUpdate								   if( InterShell )InterShell->InterShell_DisablePageUpdate 
#define  RestoreCurrentPage									   if( InterShell )InterShell->RestoreCurrentPage 
#define  HidePageExx											   if( InterShell )InterShell->HidePageExx
#define  InterShell_DisableButtonPageChange						   if( InterShell )InterShell->InterShell_DisableButtonPageChange 
#define  CreateLabelVariable								  ( !InterShell )?NULL:InterShell->CreateLabelVariable
#define  CreateLabelVariableEx								   ( !InterShell )?NULL:InterShell->CreateLabelVariableEx
#define  LabelVariableChanged								   if( InterShell )InterShell->LabelVariableChanged 
#define  LabelVariablesChanged								   if( InterShell )InterShell->LabelVariablesChanged 
#define  InterShell_Hide											   if( InterShell )InterShell->InterShell_Hide 
#define  InterShell_Reveal										   if( InterShell )InterShell->InterShell_Reveal 
#define  GetPageSize										   if( InterShell )InterShell->GetPageSize 
#define  SetButtonTextField									   if( InterShell )InterShell->SetButtonTextField 
#define  AddButtonLayout									   ( !InterShell )?NULL:InterShell->AddButtonLayout
#define  InterShell_GetButtonControl								   ( !InterShell )?NULL:InterShell->InterShell_GetButtonControl
#define  InterShell_GetPhysicalButton								   ( !InterShell )?NULL:InterShell->InterShell_GetPhysicalButton
#define  InterShell_GetLabelText								   if( InterShell )InterShell->InterShell_GetLabelText 
#define  InterShell_TranslateLabelText								   ( !InterShell )?NULL:InterShell->InterShell_TranslateLabelText
#define  InterShell_GetControlLabelText								   if( InterShell )InterShell->InterShell_GetControlLabelText 
#define  BeginCanvasConfiguration								   if( InterShell )InterShell->BeginCanvasConfiguration 
#define  SaveCanvasConfiguration								   if( InterShell )InterShell->SaveCanvasConfiguration 
#define  SaveCanvasConfiguration_XML								   if( InterShell )InterShell->SaveCanvasConfiguration_XML 
#define  InterShell_GetCurrentConfigHandler								   if( InterShell )InterShell->InterShell_GetCurrentConfigHandler 
#define  InterShell_GetCurrentLoadingControl								   ( !InterShell )?NULL:InterShell->InterShell_GetCurrentLoadingControl
#define  BeginSubConfiguration								   if( InterShell )InterShell->BeginSubConfiguration 
#define  EscapeMenuString								   ( !InterShell )?NULL:InterShell->EscapeMenuString

#define  InterShell_SetButtonHighlight(a,b)     if(InterShell)InterShell->InterShell_SetButtonHighlight(a,b)
#define  SelectAFont								   ( !InterShell )?NULL:InterShell->SelectAFont
#define  UseAFont								   ( !InterShell )?NULL:InterShell->UseAFont
#define  CreateAFont								   ( !InterShell )?NULL:InterShell->CreateAFont


#define InterShell_GetButtonFont         ( !InterShell )?NULL:InterShell->InterShell_GetButtonFont
#define InterShell_GetButtonFontName  ( !InterShell )?NULL:InterShell->InterShell_GetButtonFontName
#define InterShell_SetButtonFontName  if( InterShell )InterShell->InterShell_SetButtonFontName
#define InterShell_GetCurrentButton  ( !InterShell )?NULL:InterShell->InterShell_GetCurrentButton
#endif



#ifndef HidePageEx
#define HidePageEx(page) HidePageExx( page DBG_SRC )
#endif


#endif
