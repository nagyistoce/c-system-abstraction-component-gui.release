#ifndef ISSUE_POS_REGISTRY_SHORTCUTS_DEFINED
#define ISSUE_POS_REGISTRY_SHORTCUTS_DEFINED

#define InterShell_REGISTRY // symbol to test if this was included....
#define TASK_PREFIX "intershell"
#include <psi.h>
#include "intershell_button.h"
#include <configscript.h>
#ifndef POSTFIX
#define POSTFIX ""
#endif

//OnCreateMenuButton(name)(PMENU_BUTTON button ) { /*create button data*/ }
#define OnCreateMenuButton(name) \
	  DefineRegistryMethod(TASK_PREFIX,CreateMenuButton,"control",name POSTFIX,"button_create",PTRSZVAL,(PMENU_BUTTON))
#define OnCreateControl(name) \
	DefineRegistryMethod(TASK_PREFIX,CreateControl,"control",name,"control_create",PTRSZVAL,(PSI_CONTROL,S_32,S_32,_32,_32))
#define OnCreateListbox(name) \
	DefineRegistryMethod(TASK_PREFIX,CreateMenuListbox,"control",name,"listbox_create",PTRSZVAL,(PSI_CONTROL))

#define OnDestroyMenuButton(name) \
	  DefineRegistryMethod(TASK_PREFIX,DestroyMenuButton,"control",name,"button_destroy",void,(PTRSZVAL))
#define OnDestroyControl OnDestroyMenuButton
#define OnDestroyListbox OnDestroyMenuButton

#define OnGetControl(name) \
	DefineRegistryMethod(TASK_PREFIX,GetControl,"control",name,"get_control",PSI_CONTROL,(PTRSZVAL))
#define OnFixupControl(name) \
	  DefineRegistryMethod(TASK_PREFIX,FixupControl,"control",name,"control_fixup",void,(PTRSZVAL))
#define OnFixupMenuButton OnFixupControl

// things like clock can do a clock unlock
// things like lists can requery databases to show new items....
#define OnShowControl(name) \
	DefineRegistryMethod(TASK_PREFIX,FixupControl,"control",name,"show_control",void,(PTRSZVAL))
#define OnHideControl(name) \
	DefineRegistryMethod(TASK_PREFIX,FixupControl,"control",name,"hide_control",void,(PTRSZVAL))

#define OnLoadControl( name ) \
     DefineRegistryMethod(TASK_PREFIX,LoadControl,"control",name,"control_config",void,(PCONFIG_HANDLER,PTRSZVAL))
#define OnLoad( name ) \
     _DefineRegistryMethod(TASK_PREFIX,LoadControl,"control",name,"control_config",void,(XML_Parser,PTRSZVAL))

#define OnConfigureControl(name) \
	DefineRegistryMethod(TASK_PREFIX,ConfigureControl,"control",name,"control_edit",PTRSZVAL,(PTRSZVAL,PSI_CONTROL))
#define OnEditControl OnConfigureControl

#define OnEditBegin(name) \
	  DefineRegistryMethod(TASK_PREFIX,EditBegin,"control",name,"on_menu_edit_begin",void,(PTRSZVAL))
#define OnEditEnd(name) \
	  DefineRegistryMethod(TASK_PREFIX,EditEnd,"control",name,"on_menu_edit_end",void,(PTRSZVAL))

#define OnSelectListboxItem(name,othername) \
	DefineRegistrySubMethod(TASK_PREFIX,ListSelectionChanged,"control",name,"listbox_selection_changed",othername,void,(PTRSZVAL,PLISTITEM))
#define OnDoubleSelectListboxItem(name,othername) \
	DefineRegistrySubMethod(TASK_PREFIX,ListDoubleSelectionChanged,"control",name,"listbox_double_changed",othername,void,(PTRSZVAL,PLISTITEM))

/* 
 * this is depreicated, buttons shouldn't really HAVE to know the button they are... 
 *
 */
//#define OnSaveMenuButton(name)
//	  DefineRegistryMethod(TASK_PREFIX,SaveButton,"control",name,"button_save",void,(FILE*,PMENU_BUTTON,PTRSZVAL))
//#define OnSaveCommon OnSaveMenuButton
#define OnSave(name) \
	  _DefineRegistryMethod(TASK_PREFIX,SaveButton,"control",name,"control_save_xml",void,(genxWriter,PTRSZVAL))

#define OnSaveControl(name) \
	  DefineRegistryMethod(TASK_PREFIX,SaveButton,"control",name,"control_save",void,(FILE*,PTRSZVAL))

#define OnSaveControl(name) \
	DefineRegistryMethod(TASK_PREFIX,SaveButton,"control",name,"control_save",void,(FILE*,PTRSZVAL))
/* this method is depricated also, and will soon be obsolete.... perfer usage of OnSave and OnLoad for further development */
#define OnSaveMenuButton(name) \
	  DefineRegistryMethod(TASK_PREFIX,SaveButton,"control",name,"button_save",void,(FILE*,PMENU_BUTTON,PTRSZVAL))

// return TRUE/FALSE whether the control should be shown, else it remains hidden.
// this method is used both with MenuButton and Control.
// This is called after FixupButton(on MenuButtonOnly) and EndEdit( on Controls and MenuButtons )
// this is called during RestorePage(Full), which is used by ChangePage() after HidePage().
#define OnQueryShowControl( name )  \
	  DefineRegistryMethod(TASK_PREFIX,QueryShowControl,"control",name,"query can show",LOGICAL,(PTRSZVAL))

#define OnChangePage(name) \
	  DefineRegistryMethod(TASK_PREFIX,ChangePage,"common","change page",name"_on_change_page",int,(void))

#define OnLoadCommon( name )  \
	  DefineRegistryMethod(TASK_PREFIX,LoadCommon,"common","common_config",name"_on_load",void,(PCONFIG_HANDLER))

#define OnSaveCommon(name) \
	   DefineRegistryMethod(TASK_PREFIX,SaveCommon,"common","save common",name"_on_save_common",void,(FILE*))

// handler to catch event of when a page is saved.  The currnet ppage_data is passed as a convenience
#define OnSavePage(name) \
	   DefineRegistryMethod(TASK_PREFIX,SavePage,"common","save page",name"_on_save_page",void,(FILE*,PPAGE_DATA))
#define OnSaveXMLPage(name) \
	   DefineRegistryMethod(TASK_PREFIX,SavePage,"common","save xml page",name"_on_save_page",void,(genxWriter,PPAGE_DATA))

#define OnSelectUser(name) \
      DefineRegistryMethod(TASK_PREFIX,SelectUser,"common","select user",name"_on_select_user",void,(void))

// once selected, those buttons that show just selected
// users (in multi-select mode), buttons will invoke these methods
//  (pulltab and paper will then perform their issues...)
#define OnActivateUser(name) \
      DefineRegistryMethod(TASK_PREFIX,SelectUser,"common","activate user",name"_on_user_action",void,(void))

// invoked when all other initization is done, and before the main applciation actually runs and displays stuff.
//
#define OnFinishInit( name ) \
      DefineRegistryMethod(TASK_PREFIX,FinishInit,"common","finish init",name"_on_finish_init",void,(void))

// invoked when ALL initialzation is run, just after the menu is shown.  (tasks, first load)
#define OnFinishAllInit( name ) \
      DefineRegistryMethod(TASK_PREFIX,FinishAllInit,"common","finish all init",name"_on_finish_all_init",void,(void))

// passed the PSI_CONTROL of the menu canvas, used to give the control the parent to display its frame against.
// may also be used to get the current page.
#define OnGlobalPropertyEdit( name ) \
      DefineRegistryMethod(TASK_PREFIX,GlobalProperty,"common","global properties",name,void,(PSI_CONTROL))

#define OnInterShellShutdown( name ) \
      DefineRegistryMethod(TASK_PREFIX,InterShellShutdown,"common","intershell shutdown",name"_on_intershell_shutdown",void,(void))

#define OnDropAccept(name) \
	__DefineRegistryMethod(TASK_PREFIX,DropAccept,"common","Drop Accept",name"_on_drop_accept",LOGICAL,(CTEXTSTR,int,int),__LINE__)

//GETALL_REGISTERED( WIDE("issue_pos/common/common_config") )

#define GETALL_REGISTERED( root,type,args )	{          \
		CTEXTSTR name;                           \
		PCLASSROOT data = NULL;                     \
		for( name = GetFirstRegisteredName( root, &data );  \
			 name;                                           \
			  name = GetNextRegisteredName( &data ) )        \
		{ type (CPROC *f)args; f = GetRegisteredProcedure2(root,type,name,args);

#define ENDALL_REGISTERED()		} }


//-------------
// second generation (post documentation)

// Method invoked when a control is selected and a copy operation invoked.
// New method requires 'static <return type>' to be applied...
// static void OnCopyControl( "blah" )( PTRSZVAL psvYourControl );
#define OnCopyControl(name) \
	__DefineRegistryMethod(TASK_PREFIX,CopyControl,"control",name,"copy_control",void,(PTRSZVAL),__LINE__)

// Method invoked when a control is selected and a paste operation invoked.
// New method requires 'static <return type>' to be applied...
// static void OnPasteControl( "blah" )( PTRSZVAL psvYourControl );
#define OnPasteControl(name) \
	__DefineRegistryMethod(TASK_PREFIX,PasteControl,"control",name,"paste_control",void,(PTRSZVAL),__LINE__)

// Event handler invoked when a control is cloned (using existing interface)
// static void OnCloneControl( "blah" )( PTRSZVAL psvControlThatIsNewClone, PTRSZVAL psvControlThatIsBeingCloned )
#define OnCloneControl(name) \
	__DefineRegistryMethod(TASK_PREFIX,CloneControl,"control",name,"clone_control",void,(PTRSZVAL,PTRSZVAL),__LINE__)

#define OnLoadSecurityContext( name ) \
	__DefineRegistryMethod(TASK_PREFIX,LoadSecurityContext,"common","Load Security",name"_load_security",void,(PCONFIG_HANDLER),__LINE__)

#define OnSaveSecurityContext( name ) \
	__DefineRegistryMethod(TASK_PREFIX,SaveSecurityContext,"common","Save Security",name"_save_security",void,(FILE*,PMENU_BUTTON),__LINE__)

/* result INVALID_INDEX - premission denied
 result 0 - no context
 any other result is a result handle that is also closed when complete */
#define TestSecurityContext( name ) \
	__DefineRegistryMethod(TASK_PREFIX,TestSecurityContext,"common","Test Security",name"_test_security",PTRSZVAL,(PMENU_BUTTON),__LINE__)
#define EndSecurityContext( name ) \
	__DefineRegistryMethod(TASK_PREFIX,TestSecurityContext,"common","Close Security",name"_close_security",void,(PMENU_BUTTON,PTRSZVAL),__LINE__)

#define OnEditSecurityContext( name ) \
	__DefineRegistryMethod(TASK_PREFIX,EditSecurityContext,"common","Edit Security",name,void,(PMENU_BUTTON),__LINE__)

/* Intended use:edits properties regarding page security... OnPageChange return FALSE to disallow page change...*/
#define OnEditPageSecurityContext( name ) \
	__DefineRegistryMethod(TASK_PREFIX,EditSecurityContext,"common","global properties","Page Security:"name,void,(PPAGE_DATA),__LINE__)

#endif

