#ifndef LOADSAVE_EVOMENU_DEFINED
#define LOADSAVE_EVOMENU_DEFINED
#include "intershell_local.h"
#include <../genx/genx.h>
#include <configscript.h>

INTERSHELL_NAMESPACE

void LoadButtonConfig( PSI_CONTROL pc_canvas, TEXTSTR filename );
//void LoadButtonConfig( void );
void SaveButtonConfig( PSI_CONTROL pc_canvas, TEXTSTR filename );
// for saving sub-canvases...
// or for controls to save a canavas control...
#if 0
InterShell_PROC( void, BeginCanvasConfiguration )( PSI_CONTROL pc_canvas );
InterShell_PROC( void, SaveCanvasConfiguration )( FILE *file, PSI_CONTROL pc_canvas );
InterShell_PROC( void, SaveCanvasConfiguration_XML )( genxWriter w, PSI_CONTROL pc_canvas );
InterShell_PROC( PCONFIG_HANDLER, InterShell_GetCurrentConfigHandler )( void );

//PTRSZVAL GetButtonExtension( struct menu_button_tag * button );

//void AddCommonButtonConfig( PCONFIG_HANDLER pch, struct menu_button_tag * button );
//void DumpCommonButton( FILE *file, struct menu_button_tag * button );

//PTRSZVAL GetButtonExtension( struct menu_button_tag * button );


// BeginSubConfiguration....
//   colntrol_type_name is a InterShell widget path/name for the type of
//   other info to save... the method for setting additional configuration methods
//   is invoked by thisname.
//   Then end_type_name is the last string which will close the subconfiguration.
InterShell_PROC( LOGICAL, BeginSubConfiguration )( TEXTCHAR *control_type_name, const TEXTCHAR *end_type_name );
InterShell_PROC( CTEXTSTR, EscapeMenuString )( CTEXTSTR string );
#endif

/* used by macros.c when loading the startup macro which is a button, but not really quite a button. */
void SetCurrentLoadingButton( PMENU_BUTTON button );

INTERSHELL_NAMESPACE_END

#endif
