#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#define DEFINE_DEFAULT_RENDER_INTERFACE
// define the render interface first.

#include <stdhdrs.h>
#include <system.h>
#include <idle.h>
#ifdef WIN32
#ifndef UNDER_CE
#include <io.h> // unlink
#endif
// need PVIDEO internals, so we can generate kind-close keystrokes
#include <vidlib/vidstruc.h>
#endif

#include <psi.h>
#include <psi/shadewell.h>
#include "widgets/include/banner.h"
#include <filesys.h>
#include <text_label.h> // InterShell substitution proc
#include "../intershell_export.h"
#include "../intershell_registry.h"
#include "tasks.h"

static struct {
	PLOAD_TASK tasklist;
	PLIST autoload;
	PLOAD_TASK shell;
	PLOAD_TASK power_shell;
	PLOAD_TASK power_shell_ise;
	PLOAD_TASK windows_shell;
   //PSI_CONTROL frame; // this should be the same as the global frame (to hide when launching task)
	struct {
		_32 bExit : 4; // this needs to be set someday... it comes from intershell_main
		_32 wait_for_caller : 1;
	} flags;
   PLIST tasks_that_hid_main_canvas;
	CTEXTSTR more_path, less_path; // append and prepend path values... (append at end, insert at start of PATH environment)
} l;

enum {
	EDIT_TASK_LAUNCH_X = 2000
	  , EDIT_TASK_LAUNCH_Y
     , LISTBOX_AUTO_TASKS
	  , BUTTON_EDIT_TASK_PROPERTIES
	  , BUTTON_CREATE_AUTO_TASK
	  , CHECKBOX_RESTART
	  , CHECKBOX_ONE_TIME_LAUNCH
	  , CHECKBOX_EXCLUSIVE
	  , CHECKBOX_LAUNCH_CALLER_READY
     , CHECKBOX_CALLER_WAIT
     , CHECKBOX_CAPTURE_OUTPUT // dos prompt, get the input and do something with it...
	  , LISTBOX_ALLOW_RUN_ON
	  , LISTBOX_DISALLOW_RUN_ON
	  , EDIT_SYSTEM_NAME
	  , BTN_ADD_SYSTEM
     , BTN_REMOVE_SYSTEM
	  , TXT_TASK_NAME
	  , TXT_TASK_PATH
	  , TXT_TASK_ARGS
     , EDIT_TASK_FRIENDLY_NAME
	  , BUTTON_DESTROY_AUTO_TASK
	  , CHECKBOX_BACKGROUND
     , CHECKBOX_HIDE_CANVAS
};

LOGICAL MainCanvasStillHidden( void )
{
	LOGICAL yes = FALSE;
	INDEX idx;
	POINTER task;
	LIST_FORALL( l.tasks_that_hid_main_canvas, idx, POINTER, task )
	{
      yes = TRUE;
	}
	lprintf( WIDE("Still hidden is %s"), yes?WIDE("yes"):WIDE("no") );
   return yes;
}


#ifdef WIN32
#ifndef UNDER_CE
DEVMODE devmode; // the original mode that this as called with
/* Utility routine for SetResolution */
static void SetWithFindMode( LPDEVMODE mode, int bRestoreOnCrash )
{
	DEVMODE current;
	DEVMODE check;
	DEVMODE best;
	INDEX idx;
	// going to code something that compilers hate
	// will have usage of an undefined thing.
	// it is defined by the time it is read. Assured.
	int best_set = 0;
	for( idx = 0;
		 EnumDisplaySettings( NULL /*EnumDisplaySettings */
								  , idx
									//ENUM_REGISTRY_SETTINGS
								  , &check );
		  idx++
		)
	{

		lprintf( WIDE("Found mode: %d %dx%d %d @%d")
				 , idx
				 , check.dmPelsWidth
				 , check.dmPelsHeight
				 , check.dmBitsPerPel
				 , check.dmDisplayFrequency
				 );

		if( !idx )
			current = check;
		if( idx )
		{
			// current and check should both be valid
			if( ( check.dmPelsWidth == mode->dmPelsWidth )
				&& (check.dmPelsHeight == mode->dmPelsHeight ) )
			{
				if( best_set )
				{
					if( best.dmBitsPerPel < check.dmBitsPerPel )
					{
						if( check.dmDisplayFrequency == mode->dmDisplayFrequency ||
							check.dmDisplayFrequency == 0 ||
						   check.dmDisplayFrequency == 1
						  )
						{
							lprintf( WIDE(" ---- Updating best to current ---- ") );
							best = check;
						}
					}
				}
				else
				{
					lprintf( WIDE(" ---- Updating best to ccheck ---- ") );
					best = check;
					best_set = 1;
				}
			}
		}
	}
	{
		int n;
		for( n = 0; n < 3; n++ )
		{
			_32 flags;
			_32 result;
			switch( n )
			{
			case 0:
				flags = bRestoreOnCrash?CDS_FULLSCREEN:0;
				break;
			case 1:
				flags = 0;
				break;
			case 2:
				flags = CDS_UPDATEREGISTRY | CDS_GLOBAL;
				break;


			}
			if ( !best_set || (result=ChangeDisplaySettings(&best
																		  , flags // on program exit/original mode is restored.
																		  )) != DISP_CHANGE_SUCCESSFUL ) {
				if( best_set && ( result == DISP_CHANGE_RESTART ) )
				{
               //system( WIDE("rebootnow.exe") );
					//SimpleMessageBox( NULL, WIDE("Restart?"), WIDE("Must RESTART for Resolution change to apply :(") );
					lprintf( WIDE("Result indicates Forced Restart to change modes.") );
					break;
				}
				mode->dmBitsPerPel = 32;
				lprintf( WIDE("Last failure %d %d"), result, GetLastError() );
				if ( (result=ChangeDisplaySettings(mode
															 , flags // on program exit/original mode is restored.
															 )) != DISP_CHANGE_SUCCESSFUL ) {
					if( result == DISP_CHANGE_RESTART )
					{
						//SimpleMessageBox( NULL, WIDE("Restart?"), WIDE("Must RESTART for Resolution change to apply :(") );
						lprintf( WIDE("Result indicates Forced Restart to change modes.") );
                  break;
					}
					mode->dmBitsPerPel = 24;
					lprintf( WIDE("Last failure %d %d"), result, GetLastError() );
					if ( (result=ChangeDisplaySettings(mode
																 , flags // on program exit/original mode is restored.
																 )) != DISP_CHANGE_SUCCESSFUL ) {
						if( result == DISP_CHANGE_RESTART )
						{
							//SimpleMessageBox( NULL, WIDE("Restart?"), WIDE("Must RESTART for Resolution change to apply :(") );
							lprintf( WIDE("Result indicates Forced Restart to change modes.") );
							break;
						}
						mode->dmBitsPerPel = 16;
						lprintf( WIDE("Last failure %d %d"), result, GetLastError() );
						if ( (result=ChangeDisplaySettings(mode
														, flags // on program exit/original mode is restored.
																	 )) != DISP_CHANGE_SUCCESSFUL ) {

							if( result == DISP_CHANGE_RESTART )
							{
								//SimpleMessageBox( NULL, WIDE("Restart?"), WIDE("Must RESTART for Resolution change to apply :(") );
								lprintf( WIDE("Result indicates Forced Restart to change modes.") );
								break;
							}
							//char msg[256];
							lprintf( WIDE(WIDE("Failed to change resolution to %d by %d (16,24 or 32 bit) %d %d"))
									 , mode->dmPelsWidth
									 , mode->dmPelsHeight
									 , result
									 , GetLastError() );
							//MessageBox( NULL, msg
							//			 , WIDE(WIDE("Resolution Failed")), MB_OK );
						}
						else
						{
							lprintf( WIDE("Success setting 16 bit %d %d")
									 , mode->dmPelsWidth
									 , mode->dmPelsHeight );
							break;
						}
					}
					else
					{
						lprintf( WIDE("Success setting 24 bit %d %d")
								 , mode->dmPelsWidth
								 , mode->dmPelsHeight );
						break;
					}
				}
				else
				{
					lprintf( WIDE("Success setting 32 bit %d %d")
							 , mode->dmPelsWidth
							 , mode->dmPelsHeight );
					break;
				}
			}
			else
			{
				lprintf( WIDE("Success setting enumerated bestfit %d %d")
						 , mode->dmPelsWidth
						 , mode->dmPelsHeight );
				break;
			}
		}
	}
}
#endif
#endif

void SetResolution( PLOAD_TASK task, _32 w, _32 h )
{
#ifndef UNDER_CE
#ifdef WIN32
	DEVMODE settings;
	PPAGE_DATA page;
	page = ShellGetCurrentPage();
	if( page )
	{
		if( task )
		{
         lprintf( WIDE("Adding task that hides the frame.") );
			AddLink( &l.tasks_that_hid_main_canvas, task );
		}
      InterShell_Hide();
	}
	else
      lprintf( WIDE("Failed to get current page?!") );
	{
		//_32 w, h;
      devmode.dmSize = sizeof( devmode );
      EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &devmode );
		//GetDisplaySize( &w, &h );
		{
			FILE *file = sack_fopen( 2, WIDE("last.resolution"), WIDE("wb") );
			if( file )
			{
				fwrite( &devmode, 1, sizeof( devmode ), file );
				fclose( file );
			}
		}
	}
	settings = devmode;
   /*
	memset(&settings, 0, sizeof(DEVMODE));
	settings.dmSize = sizeof(DEVMODE);
	settings.dmBitsPerPel = 32; //video->format->BitsPerPixel;
	*/
	settings.dmPelsWidth = w;
	settings.dmPelsHeight = h;
	settings.dmBitsPerPel = 32; //video->format->BitsPerPixel;
	settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

   SetWithFindMode( &settings, TRUE /* we do want to restore on rpogram crahs */);

#endif
#endif
}
void ResetResolution( PLOAD_TASK task )
{
   lprintf( WIDE("RESET RESOLUTION") );
#ifndef UNDER_CE
#ifdef WIN32
   if( task )
	{
		INDEX idx = FindLink( &l.tasks_that_hid_main_canvas, task );
		if( idx != INVALID_INDEX )
			SetLink( &l.tasks_that_hid_main_canvas, idx, NULL );
	}
	if( MainCanvasStillHidden() )
		return;
	if( !task || task->flags.bLaunchAt )
	{
		DEVMODE oldmode;
		FILE *file = sack_fopen( 2, WIDE("last.resolution"), WIDE("rb") );
		if( file )
		{
			fread( &oldmode, 1, sizeof( oldmode ), file );
			fclose( file );
			SetWithFindMode( &oldmode, TRUE );
#ifndef UNDER_CE
			sack_unlink( 2, WIDE("last.resolution") );
#endif
		}
		else
		{
			if ( ChangeDisplaySettings( NULL
											  , 0 //CDS_FULLSCREEN // on program exit/original mode is restored.
											  ) == DISP_CHANGE_SUCCESSFUL ) {

				lprintf( WIDE("Success Reset Resolution") );
			}
			else
	            lprintf( WIDE("Fail reset resolution") );
		}
		//EnableCommonUpdates( page->frame, TRUE );
		Sleep( 250 ); // give resolution a little time to settle...
	}
	InterShell_DisablePageUpdate( FALSE );
	lprintf( WIDE("Calling InterShell_Reveal...") );
	InterShell_Reveal();
#endif
#endif
}


PRELOAD( RegisterTaskControls )
{
	{
		_32 w, h;
		{
			FILE *file = sack_fopen( 2, WIDE("last.resolution"), WIDE("rb") );
			if( file )
			{
				fread( &w, 1, sizeof( w ), file );
				fread( &h, 1, sizeof( h ), file );
            fclose( file );
				SetResolution( NULL, w, h );
#ifndef UNDER_CE
            unlink( WIDE("last.resolution") );
#endif
			}
		}
	}
   EasyRegisterResource( WIDE("InterShell/tasks"), TXT_TASK_NAME, EDIT_FIELD_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), TXT_TASK_PATH, EDIT_FIELD_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), TXT_TASK_ARGS, EDIT_FIELD_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), EDIT_TASK_LAUNCH_X, EDIT_FIELD_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), EDIT_TASK_LAUNCH_Y, EDIT_FIELD_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), LISTBOX_AUTO_TASKS          , LISTBOX_CONTROL_NAME );
	EasyRegisterResource( WIDE("InterShell/tasks"), BUTTON_EDIT_TASK_PROPERTIES , NORMAL_BUTTON_NAME );
	EasyRegisterResource( WIDE("InterShell/tasks"), BUTTON_CREATE_AUTO_TASK     , NORMAL_BUTTON_NAME );
	EasyRegisterResource( WIDE("InterShell/tasks"), BUTTON_DESTROY_AUTO_TASK     , NORMAL_BUTTON_NAME );

   EasyRegisterResource( WIDE("InterShell/tasks"), CHECKBOX_RESTART            , RADIO_BUTTON_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), CHECKBOX_EXCLUSIVE          , RADIO_BUTTON_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), CHECKBOX_BACKGROUND         , RADIO_BUTTON_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), CHECKBOX_LAUNCH_CALLER_READY, RADIO_BUTTON_NAME );
	EasyRegisterResource( WIDE("InterShell/tasks"), CHECKBOX_CALLER_WAIT        , RADIO_BUTTON_NAME );
	EasyRegisterResource( WIDE("InterShell/tasks"), CHECKBOX_ONE_TIME_LAUNCH    , RADIO_BUTTON_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), CHECKBOX_CAPTURE_OUTPUT     , RADIO_BUTTON_NAME );

	EasyRegisterResource( WIDE("InterShell/tasks"), LISTBOX_ALLOW_RUN_ON    , LISTBOX_CONTROL_NAME );
	EasyRegisterResource( WIDE("InterShell/tasks"), EDIT_SYSTEM_NAME    , EDIT_FIELD_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), BTN_ADD_SYSTEM     , NORMAL_BUTTON_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), BTN_REMOVE_SYSTEM     , NORMAL_BUTTON_NAME );
   EasyRegisterResource( WIDE("InterShell/tasks"), EDIT_TASK_FRIENDLY_NAME, EDIT_FIELD_NAME );

	{
		l.shell = CreateTask( NULL );
		l.shell->flags.bNonExclusive = 1;
		StrCpyEx( l.shell->pName, WIDE("Command Shell"), sizeof( l.shell->pName ) );
		StrCpyEx( l.shell->pTask, WIDE("cmd.exe"), sizeof( l.shell->pTask ) );
		StrCpyEx( l.shell->pPath, WIDE("."), sizeof( l.shell->pPath ) );
	}
	{
		l.power_shell = CreateTask( NULL );
		l.power_shell->flags.bNonExclusive = 1;
		StrCpyEx( l.power_shell->pName, WIDE("Power Shell"), sizeof( l.shell->pName ) );
		StrCpyEx( l.power_shell->pTask, WIDE("%SystemRoot%\\System32\\WindowsPowerShell\\v1.0\\PowerShell.exe"), sizeof( l.shell->pTask ) );
		StrCpyEx( l.power_shell->pPath, WIDE("."), sizeof( l.shell->pPath ) );
	}
	{
		l.power_shell_ise = CreateTask( NULL );
		l.power_shell_ise->flags.bNonExclusive = 1;
		StrCpyEx( l.power_shell_ise->pName, WIDE("Power Shell ISE"), sizeof( l.shell->pName ) );
		StrCpyEx( l.power_shell_ise->pTask, WIDE("%SystemRoot%\\System32\\WindowsPowerShell\\v1.0\\PowerShell_ise.exe"), sizeof( l.shell->pTask ) );
		StrCpyEx( l.power_shell_ise->pPath, WIDE("."), sizeof( l.shell->pPath ) );
	}
	{
		l.windows_shell = CreateTask( NULL );
		l.windows_shell->flags.bNonExclusive = 1;
		StrCpyEx( l.windows_shell->pName, WIDE("Explorer"), sizeof( l.shell->pName ) );
		StrCpyEx( l.windows_shell->pTask, WIDE("explorer.exe"), sizeof( l.shell->pTask ) );
		StrCpyEx( l.windows_shell->pPath, WIDE("."), sizeof( l.shell->pPath ) );
	}


#if 0
#define EasyAlias( x, y )   \
	RegisterClassAlias( WIDE("psi/resources/intershell/tasks/")y WIDE("/")WIDE(#x), WIDE("psi/resources/application/")y WIDE("/") WIDE(#x) )
   // migration path...
   EasyAlias( CHECKBOX_RESTART            , RADIO_BUTTON_NAME );
   EasyAlias( CHECKBOX_EXCLUSIVE          , RADIO_BUTTON_NAME );
   EasyAlias( CHECKBOX_LAUNCH_CALLER_READY, RADIO_BUTTON_NAME );
	EasyAlias( CHECKBOX_CALLER_WAIT        , RADIO_BUTTON_NAME );
	EasyAlias( CHECKBOX_ONE_TIME_LAUNCH    , RADIO_BUTTON_NAME );
#endif
}

//---------------------------------------------------------------------------

PLOAD_TASK CPROC CreateTask( PMENU_BUTTON button )
{
	PLOAD_TASK task = New( LOAD_TASK );
	MemSet( task, 0, sizeof( LOAD_TASK ) );
	StrCpyEx( task->pPath, WIDE("."), sizeof( task->pPath )/sizeof(TEXTCHAR) );
	task->spawns = CreateList();
	LinkThing( l.tasklist, task );
   //task->button = button;
   return task;
}

OnCreateMenuButton( WIDE("Task") )( PMENU_BUTTON button )
{
	PLOAD_TASK task = New( LOAD_TASK );
	MemSet( task, 0, sizeof( LOAD_TASK ) );
	StrCpyEx( task->pPath, WIDE("."), sizeof( task->pPath )/sizeof(TEXTCHAR) );
	task->spawns = CreateList();
	task->flags.bButton = 1;
	LinkThing( l.tasklist, task );
	task->button = button;
	InterShell_SetButtonStyle( button, WIDE("bicolor square") );
	return (PTRSZVAL)task;
}
//---------------------------------------------------------------------------

OnDestroyMenuButton( WIDE("Task") )( PTRSZVAL psv )
{
	// destory button... destroy associated task information...
}

//---------------------------------------------------------------------------

void SetTaskArguments( PLOAD_TASK pTask, char *args )
{
	int argc;
	char **pp;
	pp = pTask->pArgs;
	while( pp && pp[0] )
	{
		Release( pp[0] );
		pp++;
	}
	Release( pTask->pArgs );

	ParseIntoArgs( args, &argc, &pTask->pArgs );
	// insert a TEXTSTR pointer so we can include the task name in the args... prebuilt for launching.
	pTask->pArgs = (TEXTSTR*)Preallocate( pTask->pArgs, SizeOfMemBlock( pTask->pArgs ) + sizeof( TEXTSTR ) );
	pTask->pArgs[0] = StrDup( pTask->pTask );
	return;
}

//---------------------------------------------------------------------------

char *GetTaskName( PLOAD_TASK pTask )
{
	return pTask->pName;
}

//---------------------------------------------------------------------------

char *GetTaskArgs( PLOAD_TASK pTask )
{
	static char args[4096];
	int len = 0, n;
	args[0] = 0;
	// arg[0] should be the same as program name...
	for( n = 1; pTask->pArgs && pTask->pArgs[n]; n++ )
	{
		if( StrChr( pTask->pArgs[n], ' ' ) )
			len += snprintf( args + len, sizeof( args ) - len * sizeof( TEXTCHAR ), WIDE("%s\"%s\""), n>1?WIDE(" "):WIDE(""), pTask->pArgs[n] );
		else
			len += snprintf( args + len, sizeof( args ) - len * sizeof( TEXTCHAR ), WIDE("%s%s"), n>1?WIDE(" "):WIDE(""), pTask->pArgs[n] );
	}
	return args;
}

//---------------------------------------------------------------------------

void SetTaskName( PLOAD_TASK pTask, char *p )
{
	StrCpyEx( pTask->pName, p, sizeof( pTask->pName ) );
	if( pTask->button )
		InterShell_SetButtonText( pTask->button, p );
}

//---------------------------------------------------------------------------

static void CPROC AddSystemAllow( PTRSZVAL psv, PSI_CONTROL pc_button )
{
	char buffer[256];
	GetControlText( GetNearControl( pc_button, EDIT_SYSTEM_NAME ), buffer, sizeof( buffer ) );
	AddListItem( GetNearControl( pc_button, LISTBOX_ALLOW_RUN_ON ), buffer );

}

//---------------------------------------------------------------------------

static void CPROC RemoveSystemAllow( PTRSZVAL psv, PSI_CONTROL pc_button )
{
	PLISTITEM pli;
	PSI_CONTROL list;
	list = GetNearControl( pc_button, LISTBOX_ALLOW_RUN_ON );
	pli = GetSelectedItem( list );
	if( pli )
	{
      DeleteListItem( list, pli );
	}
}

//---------------------------------------------------------------------------




void EditTaskProperties( PTRSZVAL psv, PSI_CONTROL parent_frame, LOGICAL bVisual )
{
   PLOAD_TASK pTask = (PLOAD_TASK)psv;
	PCOMMON frame = LoadXMLFrame( bVisual?WIDE("menu.task.isframe"):WIDE("task.isframe") );
   int created = 0;
	int okay = 0;
	int done = 0;
	char menuname[256];
	{
		pTask->button = InterShell_GetCurrentButton();
		SetCommonButtons( frame, &done, &okay );
      if( bVisual )
			SetCommonButtonControls( frame );

		SetControlText( GetControl( frame, EDIT_TASK_FRIENDLY_NAME ), GetTaskName( pTask ) );
		SetControlText( GetControl( frame, TXT_TASK_NAME ), pTask->pTask );
		SetControlText( GetControl( frame, TXT_TASK_PATH ), pTask->pPath );
		SetControlText( GetControl( frame, TXT_TASK_ARGS ), GetTaskArgs( pTask ) );
      snprintf( menuname, sizeof(menuname), WIDE("%ld"), pTask->launch_width );
      SetControlText( GetControl( frame, EDIT_TASK_LAUNCH_X ), menuname );
      snprintf( menuname, sizeof(menuname), WIDE("%ld"), pTask->launch_height );
		SetControlText( GetControl( frame, EDIT_TASK_LAUNCH_Y ), menuname );
		SetCheckState( GetControl( frame, CHECKBOX_RESTART ), pTask->flags.bRestart );
		SetCheckState( GetControl( frame, CHECKBOX_EXCLUSIVE ), !pTask->flags.bNonExclusive );
		SetCheckState( GetControl( frame, CHECKBOX_BACKGROUND ), pTask->flags.bBackground );
		SetCheckState( GetControl( frame, CHECKBOX_LAUNCH_CALLER_READY ), pTask->flags.bLaunchWhenCallerUp );
		SetCheckState( GetControl( frame, CHECKBOX_ONE_TIME_LAUNCH ), pTask->flags.bOneLaunch );
		SetCheckState( GetControl( frame, CHECKBOX_CAPTURE_OUTPUT ), pTask->flags.bCaptureOutput );
		SetCheckState( GetControl( frame, CHECKBOX_HIDE_CANVAS ), pTask->flags.bHideCanvas );
      
		{
			PSI_CONTROL list;
			list = GetControl( frame, LISTBOX_ALLOW_RUN_ON );
			if( list )
			{
				INDEX idx;
				CTEXTSTR system;
				LIST_FORALL( pTask->allowed_run_on, idx, CTEXTSTR, system )
				{
               AddListItem( list, system );
				}
			}
			SetButtonPushMethod( GetControl( frame, BTN_ADD_SYSTEM ), AddSystemAllow, (PTRSZVAL)pTask );
         SetButtonPushMethod( GetControl( frame, BTN_REMOVE_SYSTEM ), RemoveSystemAllow, (PTRSZVAL)pTask );
		}
	}
	DisplayFrameOver( frame, parent_frame );
   EditFrame( frame, TRUE );
	CommonWait( frame );
   lprintf( WIDE("Wait complete... %d %d"), okay, done );
	if( okay )
	{
      char args[256];
		// Get info from dialog...
		GetControlText( GetControl( frame, TXT_TASK_NAME ), pTask->pTask, sizeof( pTask->pTask ) );
		GetControlText( GetControl( frame, TXT_TASK_PATH ), pTask->pPath, sizeof( pTask->pPath ) );
		GetControlText( GetControl( frame, EDIT_TASK_FRIENDLY_NAME ), menuname, sizeof( menuname ) );
		SetTaskName( pTask, menuname );
		GetControlText( GetControl( frame, EDIT_TASK_LAUNCH_X ), args, sizeof( args ) );
		pTask->launch_width = atoi( args );
      GetControlText( GetControl( frame, EDIT_TASK_LAUNCH_Y ), args, sizeof( args ) );
		pTask->launch_height = atoi( args );
		if( pTask->launch_width && pTask->launch_height )
         pTask->flags.bLaunchAt = 1;
		{
			PSI_CONTROL checkbox;
			checkbox = GetControl( frame, CHECKBOX_RESTART );
         if( checkbox ) pTask->flags.bRestart = GetCheckState( checkbox );
			checkbox = GetControl( frame, CHECKBOX_EXCLUSIVE );
         if( checkbox ) pTask->flags.bNonExclusive = !GetCheckState( checkbox );
			checkbox = GetControl( frame, CHECKBOX_BACKGROUND );
         if( checkbox ) pTask->flags.bBackground = GetCheckState( checkbox );
			checkbox = GetControl( frame, CHECKBOX_LAUNCH_CALLER_READY );
         if( checkbox ) pTask->flags.bLaunchWhenCallerUp = GetCheckState( checkbox );
			checkbox = GetControl( frame, CHECKBOX_ONE_TIME_LAUNCH );
         if( checkbox ) pTask->flags.bOneLaunch = GetCheckState( checkbox );
			checkbox = GetControl( frame, CHECKBOX_CAPTURE_OUTPUT );
         if( checkbox ) pTask->flags.bCaptureOutput = GetCheckState( checkbox );
			checkbox = GetControl( frame, CHECKBOX_HIDE_CANVAS );
         if( checkbox ) pTask->flags.bHideCanvas = GetCheckState( checkbox );
			{
				PSI_CONTROL list;
				list = GetControl( frame, LISTBOX_ALLOW_RUN_ON );
				if( list )
				{
					INDEX idx;
               PLISTITEM pli;
					TEXTSTR system;
					LIST_FORALL( pTask->allowed_run_on, idx, TEXTSTR, system )
					{
						Release( system );
                  SetLink( &pTask->allowed_run_on, idx, NULL );
					}
					for( idx = 0; pli = GetNthItem( list, idx ); idx++ )
					{
						char buffer[256];
                  INDEX idx2;
						GetListItemText( pli, buffer, sizeof( buffer ) );
						LIST_FORALL( pTask->allowed_run_on, idx2, TEXTSTR, system )
						{
                     //lprintf( "Compare [%s] vs [%s] ...", system, buffer );
							if( CompareMask( system, buffer, FALSE ) )
							{
                        //lprintf( "success..." );
								break;
							}
							//else
                     //   lprintf( "failure..." );
						}
						if( !system )
						{
                     AddLink( &pTask->allowed_run_on, StrDup( buffer ) );
						}

					}
				}
			}
		}
		GetControlText( GetControl( frame, TXT_TASK_ARGS )
						  , args, sizeof( args ) );
		SetTaskArguments( pTask, args );
		if( bVisual )
			GetCommonButtonControls( frame );
	}
	else
	{
		if( created )
         DestroyTask( &pTask );
	}
	DestroyFrame( &frame );
}

OnEditControl( WIDE("Task") )( PTRSZVAL psv, PSI_CONTROL parent_frame )
{
	EditTaskProperties( psv, parent_frame, TRUE );
   return psv;
}


//---------------------------------------------------------------------------

void DestroyTask( PLOAD_TASK *ppTask )
{
	if( ppTask && *ppTask )
	{
		PLOAD_TASK pTask = *ppTask;
		{
         int tried = 0;
			POINTER spawned;
			INDEX idx;
		retry:
			LIST_FORALL( pTask->spawns, idx, POINTER, spawned )
			{
				if( spawned )
               break;
			}
			if( spawned )
			{
				pTask->flags.bDestroy = TRUE;
            if( !tried )
				{
					INDEX idx;
					PTASK_INFO task;
					LIST_FORALL( pTask->spawns, idx, PTASK_INFO, task )
					{
						// should get the termination callback which will take this
						// instance out of this list... which then
                  // will enable us to continue;
						TerminateProgram( task );
					}
               tried = 1;
               goto retry;
				}

            // do still remove the task...
				DeleteLink( &l.autoload, pTask );
            // and unlink it.
				UnlinkThing( pTask );
            return;
			}
		}
		DeleteList( &pTask->spawns );
      if( pTask->pArgs )
		{
			char **pp;
			pp = pTask->pArgs;
			while( pp && pp[0] )
			{
				Release( pp[0] );
				pp++;
			}
			Release( pTask->pArgs );
         pTask->pArgs = NULL;
		}
      DeleteLink( &l.autoload, pTask );
      UnlinkThing( pTask );
		Release( pTask );
      *ppTask = NULL;
	}
}

//---------------------------------------------------------------------------

void CPROC HandleTaskOutput( PTRSZVAL psvTaskInfo, PTASK_INFO task, CTEXTSTR buffer, _32 size )
{
	PLOAD_TASK pTask = (PLOAD_TASK)psvTaskInfo;
	SystemLog( pTask->pTask );
	SystemLog( buffer );
}

//---------------------------------------------------------------------------
// forward declaration, cause the task may re-spawn within task ended
void CPROC TaskEnded( PTRSZVAL psv, PTASK_INFO task_ended );
//---------------------------------------------------------------------------

void RunATask( PLOAD_TASK pTask, int bWaitInRoutine )
{
	//PLOAD_TASK pTask = (PLOAD_TASK)psv;
	PTASK_INFO task;
	INDEX idx;
	// if task flag set as exclusive...
	if( pTask->flags.bDisallowedRun )
	{
		if( !pTask->flags.bAllowedRun )
			return;
	}
	// else if allowed, okay
	// if not allowed and not disallowed, okay

	if( !pTask->flags.bNonExclusive || pTask->flags.bOneLaunch )
	{
		LIST_FORALL( pTask->spawns, idx, PTASK_INFO, task )
		{
			if( pTask->flags.bOneLaunch )
			{
            //lprintf( WIDE("Task is already spawned, leave.") );
				return;
			}
			lprintf( WIDE("Re-hide frame - tasks still running.") );
			// shouldn't be showing to launch anything...
         InterShell_Hide();
			return;
		}
		lprintf( WIDE("Launching task %s"), pTask->pTask );
		if( pTask->flags.bLaunchAt )
		{
         SetResolution( pTask, pTask->launch_width, pTask->launch_height );
		}
	}
	lprintf( WIDE("Launching program... %s in %s"), pTask->pTask, pTask->pPath );
	pTask->last_lauch_time = timeGetTime();
	pTask->launch_count++;
	{
		char buffer1[256];
		char buffer2[256];
		CTEXTSTR taskname = StrDup( InterShell_TranslateLabelText( NULL, buffer1, sizeof( buffer1 ), pTask->pTask ) );
		CTEXTSTR path = StrDup( InterShell_TranslateLabelText( NULL, buffer2, sizeof( buffer2 ), pTask->pPath ) );
		TEXTSTR *args;
      if( pTask->pArgs )
		{
			int n;
         args = pTask->pArgs;
			for( n = 0; args && args[n]; n++ ); // just count.
			args = NewArray( TEXTSTR, (n+1) );
			for( n = 0; pTask->pArgs[n]; n++ )
				args[n] = StrDup( InterShell_TranslateLabelText( NULL, buffer1, sizeof( buffer1 ), pTask->pArgs[n] ) );
         args[n] = pTask->pArgs[n]; // copy NULL too.
		}
		else
         args = NULL;
		pTask->flags.bStarting = 1;
#ifndef UNDER_CE
		if( pTask->flags.bCaptureOutput )
		{
			task = LaunchPeerProgram( taskname
											, path
											, (PCTEXTSTR)args
											, HandleTaskOutput
											, TaskEnded
											, (PTRSZVAL)pTask );
		}
		else
#endif
			task = LaunchProgramEx( taskname, path, (PCTEXTSTR)args, TaskEnded, (PTRSZVAL)pTask );
		Release( (POINTER)taskname );
		Release( (POINTER)path );
      if( args )
		{
			int n;
			for( n = 0; args[n]; n++ )
            Release( (POINTER)args[n] );
			Release( (POINTER)args );
		}
	}
   lprintf( WIDE("Result is %p"), task );

	if( task )
	{
		AddLink( &pTask->spawns, (POINTER)task );
		pTask->flags.bStarting = 0; // okay to allow ended to check now...

		InterShell_SetButtonColors( pTask->button
								  , COLOR_IGNORE
								  , COLOR_IGNORE
								  , pTask->highlight_color
								  , COLOR_IGNORE );

		if( !pTask->flags.bNonExclusive )
		{
			lprintf( WIDE("Hiding display...") );
			if( !pTask->flags.bCaptureOutput || pTask->flags.bHideCanvas )
			{
				AddLink( &l.tasks_that_hid_main_canvas, pTask );
            InterShell_Hide();
			}
			// Wait here until task ends.
			// no real reason to wait anymore, there is an event thhat happens?
			if( bWaitInRoutine )
			{
				lprintf( "Waiting..." );
				do
				{
					LIST_FORALL( pTask->spawns, idx, PTASK_INFO, task )
					{
						// there is a task spawned...
						if( !Idle() )
						{
							pTask->waiting_thread = MakeThread();
							WakeableSleep( 2500 );
						}
						else
                     WakeableSleep( 100 );
						break;
					}
				} while( task );
			}
			else
            lprintf( WIDE("Skipping in-button wait...") );
		}
	}
	else
	{
		pTask->flags.bStarting = 0; // not starting anymore...
		ResetResolution( pTask );
	}
}

//---------------------------------------------------------------------------

void CPROC TaskEnded( PTRSZVAL psv, PTASK_INFO task_ended )
{
	INDEX idx;
	int marked = FALSE;
	PTASK_INFO task;
	PLOAD_TASK pTask = (PLOAD_TASK)psv;
	PLOAD_TASK tmp;
	if( !pTask )
		return;
   // don't check ended while starting...
	while( pTask->flags.bStarting )
      Relinquish();
	lprintf( WIDE("%s ended - refocus menu..."), pTask->pName );
	for( tmp = l.tasklist; (!marked) && tmp; tmp = tmp->next )
	{
      //lprintf( "looking at task %p...", tmp );
		if( tmp->flags.bLaunchAt || (!tmp->flags.bNonExclusive) )
		{
			LIST_FORALL( tmp->spawns, idx, PTASK_INFO, task )
			{
				//lprintf( "looking at task instance %p task %p... for %p", tmp, task, task_ended );
				if( task_ended == task )
					continue;

				marked = TRUE;
				break;
			}
		}
	}

	LIST_FORALL( pTask->spawns, idx, PTASK_INFO, task )
	{
      //lprintf( WIDE("looking at task %p task %p"), pTask, task );
		if( task_ended == task )
		{
			//DebugBreak();
			lprintf( WIDE("Restore frame") );
			SetLink( &pTask->spawns, idx, NULL );
			// reset resolution (if applicable)
			{
				if( !marked )
					ResetResolution( pTask );
			}
         /* destroy the task... was still running when we destroyed its button */
			if( pTask->flags.bDestroy )
			{
				DestroyTask( &pTask );
            return;
			}

			if( pTask->waiting_thread )
			{
				WakeThread( pTask->waiting_thread );
            pTask->waiting_thread = NULL;
			}
			// get out now, no reason to do much of anything here...
         // especially if we're shutting down.
			if( l.flags.bExit == 2 )
			{
            lprintf( WIDE("Shutting down... why do we crash after this?!") );
				return;
			}
			// exclusive task runs hiding the menu... and only runs once.
         // this task exiting is fair to reveal common, else, don't blink.
			if( pTask &&
				pTask->flags.bRestart &&
				( l.flags.bExit != 2 ) )
			{
				if( ( pTask->last_lauch_time + 2000 ) > timeGetTime() )
				{
					lprintf( WIDE("Task spawning too fast, disabling auto spawn.") );
               pTask->flags.bRestart = 0;
				}
            else
					RunATask( pTask, InterShell_IsButtonVirtual( pTask->button ) );
			}
			if( pTask->flags.bButton )
			{
				if( ((PLOAD_TASK)psv)->button )
				{
					InterShell_SetButtonColors( ((PLOAD_TASK)psv)->button, COLOR_IGNORE, COLOR_IGNORE, pTask->highlight_normal_color, COLOR_IGNORE );
					UpdateButton( ((PLOAD_TASK)psv)->button );
				}
			}
         break;
		}
	}
	if( !task )
	{
      lprintf( WIDE("Failed to find task which ended.") );
	}
 
   lprintf( WIDE("and task is done...") );
}

//---------------------------------------------------------------------------

// should get auto innited to button proc...
OnKeyPressEvent(  WIDE("Task") )( PTRSZVAL psv )
{
	//PLOAD_TASK pTask = (PLOAD_TASK)psv;
	RunATask( (PLOAD_TASK)psv, InterShell_IsButtonVirtual( ((PLOAD_TASK)psv)->button ) );
	if( ((PLOAD_TASK)psv)->button )
	{
		UpdateButton( ((PLOAD_TASK)psv)->button );
	}
}


//---------------------------------------------------------------------------

static void KillSpawnedPrograms( void )
{
	PLOAD_TASK tasks = l.tasklist;
   // need to kill autolaunched things too...
	while( tasks )
	{
		INDEX idx;
		PTASK_INFO task;
		LIST_FORALL( tasks->spawns, idx, PTASK_INFO, task )
		{
         LOGICAL closed = FALSE;
			tasks->flags.bRestart = 0;
#ifdef WIN32
			{
				TEXTCHAR progname[256];
				TEXTCHAR buffer[256];
				TEXTSTR p;
            LOGICAL bIcon = FALSE;
				HWND hWnd;
				CTEXTSTR fullname = InterShell_TranslateLabelText( NULL, buffer, sizeof( buffer ), tasks->pTask );
				CTEXTSTR filename = pathrchr( fullname );
				if( filename )
					filename++;
				else
               filename = fullname;
				snprintf( progname, sizeof( progname ), WIDE("AlertAgentIcon:%s"), filename );
				if( ( hWnd = FindWindow( WIDE("AlertAgentIcon"), progname ) )
					||( ( p = strrchr( progname, '.' ) )
					  , (p)?p[0]=0:0
					  , hWnd = FindWindow( WIDE("AlertAgentIcon"), progname ) ) )
				{
               bIcon = TRUE;
               lprintf( WIDE("Found by alert tray icon... closing.") );
					SendMessage( hWnd, WM_COMMAND, /*MNU_EXIT*/1000, 0 );
				}
            if( bIcon )
				{
               HWND still_here;
					_32 TickDelay = timeGetTime() + 250;
					// give it a little time before just killing it.
					while( ( still_here = FindWindow( WIDE("AlertAgentIcon"), progname ) ) &&
							( TickDelay > timeGetTime() ) )
						Relinquish();
					if( !still_here )
                  closed = TRUE;
				}

            if( !closed )
				{

					TEXTSTR name = StrDup( InterShell_TranslateLabelText( NULL, buffer, sizeof( buffer ), tasks->pTask ) );
					CTEXTSTR basename = name;
					TEXTSTR ext;
					ext = (TEXTSTR)StrCaseStr( basename, WIDE(".exe") );
					if( ext )
						ext[0] = 0;
					ext = (TEXTSTR)pathrchr( basename );
					if( ext )
						basename = ext + 1;
               lprintf( WIDE("Attempting to find [%s]"), basename );
					snprintf( progname, sizeof( progname ), WIDE("%s.instance.lock"), basename );
					{
						POINTER mem_lock;
						PTRSZVAL size = 0;
						mem_lock = OpenSpace( progname
												  , NULL
													//, WIDE("memory.delete")
												  , &size );
						if( mem_lock )
						{
							PVIDEO video = (PVIDEO)mem_lock;
							ForceDisplayFocus( video );
							keybd_event( VK_MENU, 56, 0, 0 );
							keybd_event( VK_F4, 62, 0, 0 );
							keybd_event( VK_F4, 62, KEYEVENTF_KEYUP, 0 );
							keybd_event( VK_MENU, 56, KEYEVENTF_KEYUP, 0 );
							closed = TRUE;
						}
					}
					Release( name );
				}
			}
#endif
			if( !closed )
			{
				TerminateProgram( task );
			}
		}
      tasks = NextLink( tasks );
	}
   // did as good a job as we can...
   l.tasklist = NULL;
}


OnInterShellShutdown( WIDE("DOKillSpawnedPrograms") )(void)
{
	Banner2NoWait( WIDE("Ending Tasks...") );
	l.flags.bExit = 2; // magic number indicating we're quitting for sure.
	KillSpawnedPrograms();
	Banner2NoWait( WIDE("Ended Task....") );
}

//---------------------------------------------------------------------------
PTRSZVAL CPROC WaitForCallerThread( PTHREAD thread );

int LaunchAutoTasks( int bCaller )
{
   int launched = 0;
	INDEX idx;
	PLOAD_TASK task;
#ifdef WIN32
	if( bCaller && !l.flags.wait_for_caller )
	{
		FILE *file;
		// we're not waiitng for caller in a banner-type mode...
	// therefore we need to launch these ourselves...
		if( ( file = sack_fopen( -1, WIDE("f:/config.sys"), WIDE("rb") ) ) )
		{
			/* is okay. */
			fclose( file );
		}
		else
		{
			ThreadTo( WaitForCallerThread, 0 );
			return 0; // bCaller mode tasks - and caller is not up.
		}
	}
#endif
	LIST_FORALL( l.autoload, idx, PLOAD_TASK, task )
	{
		if( ( bCaller && ( task->flags.bLaunchWhenCallerUp ) )
			|| ( !bCaller && !( task->flags.bLaunchWhenCallerUp ) ) )
		{
			launched = 1;
			RunATask( task, !task->flags.bNonExclusive&&!task->flags.bBackground );
		}
	}
   return launched;
}

#ifdef _WIN32
PTRSZVAL CPROC WaitForCallerThread( PTHREAD thread )
{
	static int bWaiting;
	PBANNER banner = NULL;
	if( bWaiting )
		return 0;
   bWaiting = 1;
   if( GetThreadParam( thread ) )
		CreateBanner2Ex( NULL, &banner, WIDE("Waiting for caller..."), BANNER_TOP|BANNER_NOWAIT|BANNER_DEAD, 0 );
   //SetBannerOptions( banner, BANNER_TOP, 0 );
	while( 1 )
	{
		FILE *file;
		if( ( file = sack_fopen( 0, WIDE("f:\\config.sys"), WIDE("rb") ) ) )
		{
			fclose( file );
			lprintf( WIDE("Launching caller auto tasks.") );
			LaunchAutoTasks( 1 );
			break;
		}
		WakeableSleep( 1000 );
	}
	if( GetThreadParam( thread ) )
		RemoveBanner2Ex( &banner DBG_SRC );
   bWaiting = 0;
   return 0;
}
#else
PTRSZVAL CPROC WaitForCallerThread( PTHREAD thread )
{
   // shrug - in a linux world, how do we know?
   return 0;
}
#endif

OnFinishAllInit( WIDE("tasks") )( void )
{
	//if( LaunchAutoTasks( 0 ) )
   //   RemoveBannerEx( NULL DBG_SRC );
	LaunchAutoTasks( 0 );
   if( l.flags.wait_for_caller )
		ThreadTo( WaitForCallerThread, 1 );
	else
	{
		//if( LaunchAutoTasks( 1 ) )
		//	RemoveBannerEx( NULL DBG_SRC );
		LaunchAutoTasks( 1 );
	}
	{
		PLOAD_TASK task;
		for( task = l.tasklist; task; task = NextThing( task ) )
		{
			if( task->button )
				InterShell_GetButtonColors( task->button, NULL, NULL, &task->highlight_normal_color, &task->highlight_color );
		}
	}
}

void CPROC PressDosKey( PTRSZVAL psv, _32 key )
{
	static _32 _tick, tick;
	static int reset = 0;
	static int reset2 = 0;
	static int reset3 = 0;
	static int reset4 = 0;
	if( _tick < ( ( tick = timeGetTime() ) - 2000 ) )
	{
      reset4 = 0;
      reset3 = 0;
      reset2 = 0;
      reset = 0;
		_tick = tick;
	}
	//lprintf( WIDE("Got a %c at %ld reset=%d"), psv, tick - _tick, reset );
	switch( reset )
	{
	case 0:
		if( psv == 'D' )
			reset++;
		break;
	case 1:
		if( psv == 'O' )
			reset++;
		break;
	case 2:
		if( psv == 'S' )
		{
			if( l.shell )
				RunATask( l.shell, 0 );
			reset = 0;
		}
		break;

	}
	switch( reset3 )
	{
	case 0:
		if( psv == 'W' )
			reset3++;
		break;
	case 1:
		if( psv == 'I' )
			reset3++;
		break;
	case 2:
		if( psv == 'N' )
		{
			if( l.windows_shell )
				RunATask( l.windows_shell, 0 );
			reset3 = 0;
		}
		break;
	}
	switch( reset2 )
	{
	case 0:
		if( psv == 'P' )
			reset2++;
		break;
	case 1:
		if( psv == 'W' )
			reset2++;
		break;
	case 2:
		if( psv == 'S' )
		{
			if( l.power_shell )
				RunATask( l.power_shell, 0 );
			reset2 = 0;
		}
		break;

	}
	switch( reset4 )
	{
	case 0:
		if( psv == 'P' )
			reset4++;
		break;
	case 1:
		if( psv == 'S' )
			reset4++;
		break;
	case 2:
		if( psv == 'I' )
		{
			if( l.power_shell_ise )
				RunATask( l.power_shell_ise, 0 );
			reset4 = 0;
		}
		break;

	}

}

OnFinishInit( WIDE("TasksShellKeys") )( void )
//PRELOAD( SetTaskKeys )
{
	BindEventToKey( NULL, KEY_D, KEY_MOD_ALT, PressDosKey, (PTRSZVAL)'D' );
	BindEventToKey( NULL, KEY_O, KEY_MOD_ALT, PressDosKey, (PTRSZVAL)'O' );
	BindEventToKey( NULL, KEY_S, KEY_MOD_ALT, PressDosKey, (PTRSZVAL)'S' );
	BindEventToKey( NULL, KEY_P, KEY_MOD_ALT, PressDosKey, (PTRSZVAL)'P' );
	BindEventToKey( NULL, KEY_W, KEY_MOD_ALT, PressDosKey, (PTRSZVAL)'W' );
	BindEventToKey( NULL, KEY_I, KEY_MOD_ALT, PressDosKey, (PTRSZVAL)'I' );
	BindEventToKey( NULL, KEY_N, KEY_MOD_ALT, PressDosKey, (PTRSZVAL)'N' );

}

//---------------------------------------------------------------------------


#define PSV_PARAM   PLOAD_TASK pTask;                       \
	   pTask = (PLOAD_TASK)psv;


PTRSZVAL CPROC ConfigSetTaskName( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, text );
   PSV_PARAM;
	if( pTask )
	{
		SetTaskName( pTask, text );
      //InterShell_SetButtonText( pTask->button, GetTaskName( pTask ) );
	}
   return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetTaskPath( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, text );
	PSV_PARAM;
   //lprintf( "Setting path on task %p", psv );
	if( pTask )
		StrCpyEx( pTask->pPath, text, sizeof( pTask->pPath ) );
   return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetTaskTask( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, text );
   PSV_PARAM;
	if( pTask )
		StrCpyEx( pTask->pTask, text, sizeof( pTask->pTask ) );
   return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetTaskArgs( PTRSZVAL psv, arg_list args )
{
	PARAM( args, char *, text );
   PSV_PARAM;
	if( pTask )
		SetTaskArguments( pTask, text );
   //strcpy( pTask->name, text );
   return psv;
}

PTRSZVAL CPROC SetTaskRestart( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bRestart );
   PSV_PARAM;
	if( pTask )
      pTask->flags.bRestart = bRestart;
   return psv;
}

PTRSZVAL CPROC SetTaskExclusive( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bNonExclusive );
   PSV_PARAM;
	if( pTask )
      pTask->flags.bNonExclusive = bNonExclusive;
   return psv;
}
PTRSZVAL CPROC SetTaskBackground( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bBackground );
   PSV_PARAM;
	if( pTask )
      pTask->flags.bBackground = bBackground;
   return psv;
}

PTRSZVAL CPROC SetTaskCapture( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bCaptureOutput );
   PSV_PARAM;
	if( pTask )
      pTask->flags.bCaptureOutput = bCaptureOutput;
   return psv;
}

PTRSZVAL CPROC SetTaskHide( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bHideCanvas );
   PSV_PARAM;
	if( pTask )
      pTask->flags.bHideCanvas = bHideCanvas;
   return psv;
}

PTRSZVAL CPROC SetTaskOneTime( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, bOneLaunch );
   PSV_PARAM;
	if( pTask )
      pTask->flags.bOneLaunch = bOneLaunch;
   return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetLaunchResolution( PTRSZVAL psv, arg_list args )
{
	PARAM( args, S_64, width );
	PARAM( args, S_64, height );
	PSV_PARAM;
	if( pTask )
	{
		if( width && height )
		{
			pTask->launch_width = (int)width;
			pTask->launch_height = (int)height;
			pTask->flags.bLaunchAt = 1;
		}
	}
	return psv;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC SetTaskSecurity( PTRSZVAL psv, arg_list args )
{
	//PARAM( args, char *, text );
	//if( stristr( text, WIDE("something") )
	//  )
	//{
	//}
	return psv;
}

PTRSZVAL CPROC SetTaskRunOn( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, system );
	CTEXTSTR my_system = InterShell_GetSystemName();
	PSV_PARAM;
	AddLink( &pTask->allowed_run_on, StrDup( system ) );
   //lprintf( "Compare %s vs %s", system, my_system );
	if( CompareMask( system, my_system, FALSE ) )
	{
      //lprintf( "Task allowed..." );
      // at least one matched...
      pTask->flags.bAllowedRun = 1;
	}
	else
	{
      //lprintf( "task disallowed..." );
		// at least one didn't match... if not AllowedRun && Disallowed, don't run
      // else if( !allowed and !disallowed) run
      pTask->flags.bDisallowedRun = 1;
	}
   return psv;
}

PTRSZVAL CPROC BeginButtonTaskInfo( PTRSZVAL psv, arg_list args )
{
	//BeginSubConfiguration( NULL, WIDE("Task Done") );
   return psv;
}


void AddTaskConfigs( PCONFIG_HANDLER pch )
{
   //lprintf( WIDE("Adding configuration handling for a task....") );
	AddConfigurationMethod( pch, WIDE("name=%m"), ConfigSetTaskName );
	AddConfigurationMethod( pch, WIDE("path=%m"), SetTaskPath );
	AddConfigurationMethod( pch, WIDE("program=%m"), SetTaskTask );
	AddConfigurationMethod( pch, WIDE("args=%m"), SetTaskArgs );
   AddConfigurationMethod( pch, WIDE("security=%m"), SetTaskSecurity );
	AddConfigurationMethod( pch, WIDE("Launch at %i by %i"), SetLaunchResolution );
	AddConfigurationMethod( pch, WIDE("restart %b"), SetTaskRestart );
	AddConfigurationMethod( pch, WIDE("one time %b"), SetTaskOneTime );
	AddConfigurationMethod( pch, WIDE("non-exclusive %b"), SetTaskExclusive );
	AddConfigurationMethod( pch, WIDE("background %b"), SetTaskBackground );
   AddConfigurationMethod( pch, WIDE("Capture task output?%b" ), SetTaskCapture );
	AddConfigurationMethod( pch, WIDE("Force Hide Display?%b" ), SetTaskHide );
	AddConfigurationMethod( pch, WIDE("Run task On %m" ), SetTaskRunOn );

}

PTRSZVAL  CPROC FinishConfigTask( PTRSZVAL psv, arg_list args )
{
	// just return NULL here, so there's no object to process
   // EndConfig?  how does the pushed state recover?
   return 0;
}

/* place holder for common subconfiguration start. */
OnLoadControl( WIDE("TaskInfo") )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
   lprintf( WIDE("Begin sub for task...") );
   AddTaskConfigs( pch );
}

OnLoadControl( WIDE("Task") )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
   //lprintf( "Begin sub for task..." );
   AddTaskConfigs( pch );
	//BeginSubConfiguration( WIDE( "TaskInfo" ), WIDE("Task Done") );
}

OnInterShellShutdown( WIDE("Task") )( void )
{
	KillSpawnedPrograms();
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC CreateNewAutoTask( PTRSZVAL psv, arg_list args )
{
	PLOAD_TASK pTask = CreateTask(NULL);
	pTask->flags.bNonExclusive = 1;
   pTask->flags.bAutoLaunch = 1;
	AddLink( &l.autoload, pTask );
   //l.flags.bTask = 1;
   BeginSubConfiguration( WIDE("TaskInfo"), WIDE("Task Done") );
	return (PTRSZVAL)pTask;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC CreateShellCommand( PTRSZVAL psv, arg_list args )
{
	PLOAD_TASK pTask = CreateTask(NULL);
	pTask->flags.bNonExclusive = 1;
   l.shell = pTask;
   //l.flags.bTask = 1;
   BeginSubConfiguration( WIDE("TaskInfo"), WIDE("Task Done") );
	return (PTRSZVAL)pTask;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC CreateWinShellCommand( PTRSZVAL psv, arg_list args )
{
	PLOAD_TASK pTask = CreateTask(NULL);
	pTask->flags.bNonExclusive = 1;
   l.windows_shell = pTask;
   //l.flags.bTask = 1;
   BeginSubConfiguration( WIDE("TaskInfo"), WIDE("Task Done") );
	return (PTRSZVAL)pTask;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC CreatePowerShellCommand( PTRSZVAL psv, arg_list args )
{
	PLOAD_TASK pTask = CreateTask(NULL);
	pTask->flags.bNonExclusive = 1;
   l.power_shell = pTask;
   //l.flags.bTask = 1;
   BeginSubConfiguration( WIDE("TaskInfo"), WIDE("Task Done") );
	return (PTRSZVAL)pTask;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC CreatePowerShellISECommand( PTRSZVAL psv, arg_list args )
{
	PLOAD_TASK pTask = CreateTask(NULL);
	pTask->flags.bNonExclusive = 1;
   l.power_shell_ise = pTask;
   //l.flags.bTask = 1;
   BeginSubConfiguration( WIDE("TaskInfo"), WIDE("Task Done") );
	return (PTRSZVAL)pTask;
}

//---------------------------------------------------------------------------

PTRSZVAL CPROC CreateNewAutoCallerTask( PTRSZVAL psv, arg_list args )
{
	PLOAD_TASK pTask = CreateTask(NULL);
   pTask->flags.bLaunchWhenCallerUp = 1;
   pTask->flags.bAutoLaunch = 1;
   pTask->flags.bNonExclusive = 1;
	AddLink( &l.autoload, pTask );
	//l.flags.bTask = 1;
   // at this point how do I get the thing?
	//AddTaskConfigs( ... );
   BeginSubConfiguration( WIDE("TaskInfo"), WIDE("Task Done") );
	return (PTRSZVAL)pTask;
}

PTRSZVAL CPROC SetWaitForCaller( PTRSZVAL psv, arg_list args )
{
	PARAM( args, LOGICAL, yes_no );
   l.flags.wait_for_caller = yes_no;
   return psv;
}

PTRSZVAL CPROC AddAdditionalPath( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, path );
#ifdef HAVE_ENVIRONMENT
	OSALOT_AppendEnvironmentVariable( WIDE( "PATH" ), path );
#endif
   l.more_path = StrDup( path );
   return psv;
}

PTRSZVAL CPROC AddPrependPath( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, path );
#ifdef HAVE_ENVIRONMENT
	OSALOT_PrependEnvironmentVariable( WIDE( "PATH" ), path );
#endif
   l.less_path = StrDup( path );
   return psv;
}

OnLoadCommon( WIDE("Tasks") )( PCONFIG_HANDLER pch )
{
   /* standard tasks, these will get task_info prefix to compliment task done suffix */
   AddConfigurationMethod( pch, WIDE("auto caller task"), CreateNewAutoCallerTask );
	AddConfigurationMethod( pch, WIDE("auto task"), CreateNewAutoTask );
	AddConfigurationMethod( pch, WIDE("Command Shell"), CreateShellCommand );
	AddConfigurationMethod( pch, WIDE("Windows Shell"), CreateWinShellCommand );
	AddConfigurationMethod( pch, WIDE("Power Shell"), CreatePowerShellCommand );
	AddConfigurationMethod( pch, WIDE("Power Shell ISE"), CreatePowerShellISECommand );
	//AddConfigurationMethod( pch, WIDE("{task_info}" ), BeginButtonTaskInfo );

	AddConfigurationMethod( pch, WIDE("<path more=\"%m\"}"), AddAdditionalPath );
	AddConfigurationMethod( pch, WIDE("<path less=\"%m\">"), AddPrependPath );
   AddConfigurationMethod( pch, WIDE("wait for caller? %b"), SetWaitForCaller );
}


//---------------------------------------------------------------------------

char *GetTaskSecurity( PLOAD_TASK pTask )
{
	static char args[256];
	snprintf( args, sizeof( args ), WIDE("none") );
   return args;
}

static void DumpTask( FILE *file, PLOAD_TASK pTask, int sub )
{
   //PLOAD_TASK pTask = (PLOAD_TASK)psv;
	if( pTask )
	{
		CTEXTSTR p;
		if( ( p = EscapeMenuString( GetTaskName( pTask ) ) ) )
			fprintf( file, WIDE("name=%s\n"), p );
		if( ( p = EscapeMenuString( pTask->pPath ) ) )
			fprintf( file, WIDE("path=%s\n"), p );
		if( ( p =  EscapeMenuString( pTask->pTask ) ) )
			fprintf( file, WIDE("program=%s\n"), p );
		if( ( p =  EscapeMenuString( GetTaskArgs( pTask ) ) ) )
			fprintf( file, WIDE("args=%s\n"), p );
		if( pTask->flags.bLaunchAt )
			fprintf( file, WIDE("launch at %d by %d\n"), pTask->launch_width, pTask->launch_height );
		else
			fprintf( file, WIDE("launch at 0 by 0\n"), pTask->launch_width, pTask->launch_height );

		//if( !pTask->flags.bButton )
		fprintf( file, WIDE("restart %s\n"), pTask->flags.bRestart?WIDE("Yes"):WIDE("No") );
		fprintf( file, WIDE("non-exclusive %s\n"), pTask->flags.bNonExclusive?WIDE("Yes"):WIDE("No") );
		fprintf( file, WIDE("background %s\n"), pTask->flags.bBackground?WIDE("Yes"):WIDE("No") );
		fprintf( file, WIDE("one time %s\n"), pTask->flags.bOneLaunch?WIDE("Yes"):WIDE("No") );
		fprintf( file, WIDE("Capture task output?%s\n" ), pTask->flags.bCaptureOutput?WIDE("Yes"):WIDE("No") );
		fprintf( file, WIDE("Force Hide Display?%s\n" ), pTask->flags.bHideCanvas?WIDE("Yes"):WIDE("No") );
		{
			INDEX idx;
         CTEXTSTR sysname;
			LIST_FORALL( pTask->allowed_run_on, idx, CTEXTSTR, sysname )
				fprintf( file, WIDE("Run task on %s\n" ), sysname );
		}
		//if( pTask->flags.bButton )
		fprintf( file, WIDE("security=%s\n"), GetTaskSecurity( pTask ) );
		//if( pTask->pImage && pTask->pImage[0] )
		//	fprintf( file, WIDE("image=%s\n"), EscapeMenuString( pTask->pImage ) );
      if( sub )
			fprintf( file, WIDE( "Task Done\n" ) );
	}
}

OnSaveControl( WIDE("Task") )( FILE *file, PTRSZVAL psv )
{
   DumpTask( file, (PLOAD_TASK)psv, 0 );
}

OnSaveCommon( WIDE("Tasks") )( FILE *file )
{
		int bWroteAutoCaller = FALSE;
		int bWroteAuto = FALSE;
		if( l.shell )
		{
			fprintf( file, WIDE("\n\nCommand Shell\n") );
         DumpTask( file, l.shell, 1 );
			fprintf( file, WIDE("\n\n") );
		}
		if( l.power_shell )
		{
			fprintf( file, WIDE("\n\nPower Shell\n") );
         DumpTask( file, l.power_shell, 1 );
			fprintf( file, WIDE("\n\n") );
		}
		if( l.power_shell_ise )
		{
			fprintf( file, WIDE("\n\nPower Shell ISE\n") );
         DumpTask( file, l.power_shell_ise, 1 );
			fprintf( file, WIDE("\n\n") );
		}
		if( l.windows_shell )
		{
			fprintf( file, WIDE("\n\nWindows Shell\n") );
         DumpTask( file, l.windows_shell, 1 );
			fprintf( file, WIDE("\n\n") );
		}
		if( l.autoload )
		{
			INDEX idx;
         PLOAD_TASK pTask;
			LIST_FORALL( l.autoload, idx, PLOAD_TASK, pTask )
			{
				if( pTask->flags.bLaunchWhenCallerUp )
				{
               bWroteAutoCaller = TRUE;
					fprintf( file, WIDE("auto caller task\n") );
				}
				else
				{
               bWroteAuto = TRUE;
					fprintf( file, WIDE("auto task\n") );
				}
				DumpTask( file, pTask, 1 );
				fprintf( file, WIDE("\n\n") );
			}
		}
		if( !bWroteAutoCaller )
		{
			fprintf( file, WIDE("#auto caller task\n") );
			fprintf( file, WIDE("#  define a standard task here with program=, path=, and optional args=\n") );
			fprintf( file, WIDE("#  auto caller tasks run when the caller comes up - good time for synctime.\n") );
			fprintf( file, WIDE("\n\n") );
		}
		if( !bWroteAuto )
		{
			fprintf( file, WIDE("#auto task\n") );
			fprintf( file, WIDE("#  define a standard task here with program=, path=, and optional args=\n") );
			fprintf( file, WIDE("#  auto tasks run as soon before the menu displays.\n") );
			fprintf( file, WIDE("\n\n") );
		}
      if( l.more_path )
			fprintf( file, WIDE( "<path more=\"%s\">\n" ), l.more_path );
		if( l.less_path )
			fprintf( file, WIDE( "<path less=\"%s\">\n" ), l.less_path );
		fprintf( file, WIDE( "wait for caller? %s\n\n" ), l.flags.wait_for_caller?WIDE("yes"):WIDE("no") );

}

//-------------------------------------------------------------------------------
// property methods for tasks which are auto load/invisible.


void CPROC EditAutoTaskProperties( PTRSZVAL psv, PSI_CONTROL button )
{
	PLISTITEM pli = GetSelectedItem( GetNearControl( button, LISTBOX_AUTO_TASKS ) );
	if( pli )
	{
      char buf[256];
		PLOAD_TASK task = (PLOAD_TASK)GetItemData( pli );
		EditTaskProperties( (PTRSZVAL)task, button, FALSE );
		snprintf( buf, sizeof( buf ), WIDE("%s%s%s")
				  , GetTaskName( task )
				  , task->flags.bLaunchWhenCallerUp?WIDE("[CALLER]"):WIDE("")
				  , task->flags.bRestart?WIDE("[RESTART]"):WIDE("") );
		SetItemText( pli, buf );
	}
}

void CPROC CreateAutoTaskProperties( PTRSZVAL psv, PSI_CONTROL button )
{
	PLOAD_TASK task;
	task = CreateTask( NULL );
	EditTaskProperties( (PTRSZVAL)task, button, FALSE );
	// validate task, and perhaps destroy it?
	if( !task->pName[0] )
		StrCpy( task->pName, WIDE("NO PROGRAM") );
	{
      char buf[256];
		snprintf( buf, sizeof( buf ), WIDE("%s%s%s")
				  , GetTaskName( task )
				  , task->flags.bLaunchWhenCallerUp?WIDE("[CALLER]"):WIDE("")
				  , task->flags.bRestart?WIDE("[RESTART]"):WIDE("") );
		SetItemData( AddListItem( GetNearControl( button, LISTBOX_AUTO_TASKS ), buf ), (PTRSZVAL)task );
	}
   AddLink( &l.autoload, task );
}

void CPROC DestroyAutoTaskProperties( PTRSZVAL psv, PSI_CONTROL button )
{
   PSI_CONTROL list;
	PLISTITEM pli = GetSelectedItem( list = GetNearControl( button, LISTBOX_AUTO_TASKS ) );
	if( pli )
	{
		PLOAD_TASK task = (PLOAD_TASK)GetItemData( pli );
		DestroyTask( &task );
		DeleteListItem( list, pli );
	}
}


OnGlobalPropertyEdit( WIDE("Tasks") )( PSI_CONTROL parent )
{
	PSI_CONTROL frame = LoadXMLFrame( WIDE("CommonTaskProperties.isFrame") );
	if( frame )
	{
		int okay = 0;
		int done = 0;
		SetCommonButtons( frame, &done, &okay );
		SetButtonPushMethod( GetControl( frame, BUTTON_EDIT_TASK_PROPERTIES ), EditAutoTaskProperties, 0 );
		SetButtonPushMethod( GetControl( frame, BUTTON_CREATE_AUTO_TASK ), CreateAutoTaskProperties, 0 );
		SetButtonPushMethod( GetControl( frame, BUTTON_DESTROY_AUTO_TASK ), DestroyAutoTaskProperties, 0 );
		{
			PSI_CONTROL list = GetControl( frame, LISTBOX_AUTO_TASKS );
			if( list )
			{
				PLOAD_TASK task;
            INDEX idx;
				char buf[256];
				SetItemData( AddListItem( list, WIDE("Command Shell") ), (PTRSZVAL)l.shell );
#ifdef WIN32
				SetItemData( AddListItem( list, WIDE("Explorer") ), (PTRSZVAL)l.windows_shell );
				SetItemData( AddListItem( list, WIDE("Power Shell") ), (PTRSZVAL)l.power_shell );
				SetItemData( AddListItem( list, WIDE("Power Shell ISE") ), (PTRSZVAL)l.power_shell_ise );
#endif
				LIST_FORALL( l.autoload, idx, PLOAD_TASK, task )
				{
					snprintf( buf, sizeof( buf ), WIDE("%s%s%s")
							  , GetTaskName( task )
							  , task->flags.bLaunchWhenCallerUp?WIDE("[CALLER]"):WIDE("")
							  , task->flags.bRestart?WIDE("[RESTART]"):WIDE("") );
               SetItemData( AddListItem( list, buf ), (PTRSZVAL)task );
				}
			}
         SetCheckState( GetControl( frame, CHECKBOX_CALLER_WAIT ), l.flags.wait_for_caller );
		}
		DisplayFrameOver( frame, parent );
		CommonWait( frame );
		if( okay )
		{
			// command shell properties
			// wait for caller flag
			// auto caller tasks
			// auto tasks
			{
            l.flags.wait_for_caller = GetCheckState( GetControl( frame, CHECKBOX_CALLER_WAIT ) );
			}
		}
      DestroyFrame( &frame );
	}
}

static void OnCloneControl( WIDE("Task") )( PTRSZVAL psvNew, PTRSZVAL psvOriginal )
{
   PLOAD_TASK pNewTask = (PLOAD_TASK)psvNew;
	PLOAD_TASK pOriginalTask = (PLOAD_TASK)psvOriginal;
	pNewTask[0] = pOriginalTask[0];
	pNewTask->button = NULL; // don't know the button we're cloning...
	pNewTask->spawns = NULL; // this button hasn't launched anything yet.
	pNewTask->allowed_run_on = NULL;
   pNewTask->pArgs = NULL; // don't have args yet... so don't release anything when setting args.
   SetTaskArguments( pNewTask, GetTaskArgs( pOriginalTask ) );
	{
		POINTER p;
		INDEX idx;
		LIST_FORALL( pOriginalTask->allowed_run_on, idx, POINTER, p )
		{
         AddLink( &pNewTask->allowed_run_on, p );
		}
	}
}

struct resolution_button
{
	int width, height;
   PMENU_BUTTON button;
};

OnKeyPressEvent( WIDE("Task Util/Set Resolution") )( PTRSZVAL psv )
{
	struct resolution_button *resbut = (struct resolution_button *)psv;
   if( resbut->width && resbut->height )
		SetResolution( NULL, resbut->width, resbut->height );
	else
      ResetResolution( NULL );
}

OnCreateMenuButton( WIDE("Task Util/Set Resolution") )( PMENU_BUTTON button )
{
	struct resolution_button *resbut = New( struct resolution_button );
	resbut->width = 0;
	resbut->height = 0;
   resbut->button = button;
   return (PTRSZVAL)resbut;
}

OnEditControl( WIDE("Task Util/Set Resolution") )( PTRSZVAL psv, PSI_CONTROL parent_frame )
{
	PCOMMON frame = LoadXMLFrame( WIDE( "task.resolution.isframe" ) );
	int okay = 0;
	int done = 0;
   struct resolution_button *resbut = (struct resolution_button *)psv;
	if( frame )
	{
      char buffer[15];
		SetCommonButtons( frame, &done, &okay );
      snprintf( buffer, sizeof( buffer ), WIDE("%ld"), resbut->width );
      SetControlText( GetControl( frame, EDIT_TASK_LAUNCH_X ), buffer );
      snprintf( buffer, sizeof( buffer ), WIDE("%ld"), resbut->height );
      SetControlText( GetControl( frame, EDIT_TASK_LAUNCH_Y ), buffer );
      DisplayFrameOver( frame, parent_frame );
		CommonWait( frame );
		if( okay )
		{
			GetControlText( GetControl( frame, EDIT_TASK_LAUNCH_X ), buffer, sizeof( buffer ) );
         resbut->width = atoi( buffer );
         GetControlText( GetControl( frame, EDIT_TASK_LAUNCH_Y ), buffer, sizeof( buffer ) );
         resbut->height = atoi( buffer );
		}
      DestroyFrame( &frame );
	}
   return psv;
}

OnSaveControl( WIDE("Task Util/Set Resolution") )( FILE *file, PTRSZVAL psv )
{
	struct resolution_button *resbut = (struct resolution_button *)psv;
	fprintf( file, WIDE("launch at %d by %d\n"), resbut->width, resbut->height );
}

PTRSZVAL CPROC SetLaunchResolution2( PTRSZVAL psv, arg_list args )
{
	PARAM( args, S_64, width );
	PARAM( args, S_64, height );
	struct resolution_button *resbut = (struct resolution_button *)psv;
	if( resbut )
	{
		resbut->width = (int)width;
		resbut->height = (int)height;
	}
	return psv;
}



OnLoadControl( WIDE("Task Util/Set Resolution") )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
	AddConfigurationMethod( pch, WIDE("Launch at %i by %i"), SetLaunchResolution2 );
}


static LOGICAL OnDropAccept( WIDE("Add Task Button") )( CTEXTSTR file, int x, int y )
{
	if( StrCaseStr( file, WIDE(".exe") )
		||StrCaseStr( file, WIDE(".bat") )
		||StrCaseStr( file, WIDE(".com") )
		||StrCaseStr( file, WIDE(".cmd") ) )
	{
		PTRSZVAL psv = InterShell_CreateControl( WIDE("Task"), x, y, 5, 3 );
		PLOAD_TASK pTask = (PLOAD_TASK)psv;
		if( pTask )
		{
			int argc;
			char **argv;
			char *pathend;
         char *ext;
			ParseIntoArgs( (char*)file, &argc, &argv );
			pathend = (char*)pathrchr( argv[0] );
			if( pathend )
				pathend[0] = 0;
			else
            pathend = argv[0];
			StrCpyEx( pTask->pTask, pathend+1, 256 );
			ext = strrchr( pathend+1, '.' );
			if( ext )
				ext[0] = 0;

         SetTaskName( pTask, pathend+1 );
			StrCpyEx( pTask->pPath, argv[0], 256 );

			pTask->pArgs = argv;


		}
      return 1;
	}
   return 0;
}

#ifdef __WATCOMC__
PUBLIC( void, ExportThis )( void )
{
}
#endif
