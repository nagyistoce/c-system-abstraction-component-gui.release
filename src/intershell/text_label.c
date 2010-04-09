
//#define PAPER_SUPPORT_LIBRARY
#include <stdhdrs.h>
#include "intershell_local.h"
#include "intershell_registry.h"
#include "resource.h"

//#include "papers.h"
//#include "users.h"

#include "fonts.h"

INTERSHELL_NAMESPACE

#define TEXT_LABEL_NAME WIDE("Text Label")



struct page_label {
	PSI_CONTROL control;
   PMENU_BUTTON button;
	struct {
		BIT_FIELD bCenter : 1;
		BIT_FIELD bRight : 1;
		/* these will be fun to implement... */
		BIT_FIELD bVertical : 1;
		BIT_FIELD bInverted : 1;
		BIT_FIELD bScroll : 1;
	} flags;
	CDATA color;
	Font *font;
  	Font *new_font; // temporary variable until changes are okayed
  	CTEXTSTR fontname; // used to communicate with font preset subsystem
	//POINTER fontdata;
  	//_32 fontdatalen;
	PPAGE_DATA page;
	TEXTCHAR *label_text; // allowed to override the real title...
	int offset;
	int min_offset;
	int max_offset;
	// need to figure out how to maintain this information
	CTEXTSTR preset_name;
   TEXTSTR last_text; // the last value set as the label content.
};

enum { BTN_VARNAME = 1322
	  , BTN_VARVAL };

PRELOAD( AliasPageTitle )
{
	EasyRegisterResource( WIDE( "intershell/text" ), BTN_VARNAME, EDIT_FIELD_NAME );
	EasyRegisterResource( WIDE( "intershell/text" ), BTN_VARVAL, EDIT_FIELD_NAME );
	// hack for ancient stuff...
   // browse names maybe shouldnt' return aliases (at an option?)
	//RegisterClassAlias( TASK_PREFIX"/control/"TEXT_LABEL_NAME
	//						 , TASK_PREFIX"/control/" "page/Show Title"
	//						);
}

typedef struct variable_tag VARIABLE;
struct variable_tag
{
	TEXTCHAR *name;
#define BITFIELD _32
	struct {
		BITFIELD bString : 1;
		BITFIELD bInt : 1;
		BITFIELD bProc : 1;
		BITFIELD bProcEx : 1;
		BITFIELD bProcControlEx : 1;
		BITFIELD bConstString : 1;
	} flags;
	union {
		CTEXTSTR *string_value;
		CTEXTSTR string_const_value;
		_32 *int_value;
      label_gettextproc proc;
		label_gettextproc_ex proc_ex;
      label_gettextproc_control proc_control_ex;
	} data;
   PTRSZVAL psvUserData; // passed to data.proc_ex
	TEXTCHAR tmpval[32];
   PLIST references; // PPAGE_LABELs which reference this variable...
};

static struct label_local
{
	PLIST labels;

} l;

CTEXTSTR GetPageTitle(void);

/*
#define NUM_VARIABLES ( sizeof( variables ) / sizeof( variables[0] ) )
VARIABLE variables[] = { { "Current User", .flags={1,0},.data={.string_value=&paper_global_data.pCurrentUserName } }
							  , { "Current Session", .flags={0,1},.data={.int_value=&paper_global_data.current_session } }
							  , { "Current Page", .flags={.bProc=1},.data={.proc=GetPageTitle } }
							  , { "Current Selected User Name", .flags={.bString=1},.data={.string_value=&user_local.name } }
							  , { "Current Selected User StaffID", .flags={.bString=1},.data={.string_value=&user_local.staff_id} }
//							  , { "Current Current User Total", .flags={.bProc=1},.data={.proc=GetPageTitle } }

};
*/
static PLIST extern_variables;


#undef CreateLabelVariableEx
PVARIABLE CreateLabelVariableEx( CTEXTSTR name, enum label_variable_types type, CPOINTER data, PTRSZVAL psv )
{
	if( name && name[0] )
	{
		PVARIABLE newvar = New( VARIABLE );
		MemSet( newvar, 0, sizeof( *newvar ) );
		newvar->name = StrDup( name );
		switch( type )
		{
		case LABEL_TYPE_STRING:
			newvar->flags.bString = 1;
			newvar->data.string_value = (CTEXTSTR*)data;
			break;
		case LABEL_TYPE_CONST_STRING:
			newvar->flags.bConstString = 1;
			newvar->data.string_const_value = StrDup((TEXTSTR)data);
			break;
		case LABEL_TYPE_INT:
			newvar->flags.bInt = 1;
			newvar->data.int_value = (_32*)data;
			break;
		case LABEL_TYPE_PROC:
			newvar->flags.bProc = 1;
			newvar->data.proc = (label_gettextproc)data;
			break;
		case LABEL_TYPE_PROC_EX:
			newvar->flags.bProcEx = 1;
			newvar->data.proc_ex = (label_gettextproc_ex)data;
			newvar->psvUserData = psv;
			break;
		}
		AddLink( &extern_variables, newvar );
		return newvar;
	}
	return NULL;
}

PVARIABLE CreateLabelVariable( CTEXTSTR name, enum label_variable_types type, CPOINTER data )
{
	if( type == LABEL_TYPE_PROC_EX )
	{
		xlprintf( LOG_ALWAYS )( WIDE( "Cannot Register an EX Proc tyep label with CreateLabelVariable!" ) );
		DebugBreak();
	}
   return CreateLabelVariableEx( name, type, data, 0 );
}

CTEXTSTR InterShell_GetLabelText( PPAGE_LABEL label, CTEXTSTR variable )
{
   return InterShell_GetControlLabelText( NULL, label, variable );
}

// somehow, variables need to get udpate events...
// which can then update the text labels referencing said variable.

//OnUpdateVariable( WIDE("Current User Total") )( TEXTCHAR *variable )
//{
//}

void CPROC ScrollingLabelUpdate( PTRSZVAL psv )
{
   PPAGE_LABEL label;
   INDEX idx;
	LIST_FORALL( l.labels, idx, PPAGE_LABEL, label )
	{
		if( label->flags.bScroll )
		{
         label->offset -= 1;
			if( !SetControlTextOffset( label->control, label->offset ) )
			{
            label->offset = label->max_offset;
				SetControlTextOffset( label->control, label->offset );
			}
		}
	}
}

PRELOAD( PreconfigureVariables )
{
	CreateLabelVariable( WIDE( "Current Page" ), LABEL_TYPE_PROC, (POINTER)GetPageTitle );
	AddTimer( 50, ScrollingLabelUpdate, 0 );
}


CTEXTSTR GetPageTitle( void )
{
	PPAGE_DATA page = ShellGetCurrentPage();
   return page->title?page->title:WIDE( "DEFAULT PAGE" );
}

PVARIABLE FindVariableByName( CTEXTSTR variable )
{
	PVARIABLE var;
	INDEX nVar;
	if( !variable )
      return NULL; // can't have a NULL variable.
	LIST_FORALL( extern_variables, nVar, PVARIABLE, var )
	{
		//for( nVar = 0; nVar < NUM_VARIABLES; nVar++ )
		if( var->name )
		{
			int varnamelen = strlen( var->name );
			if( strnicmp( variable, var->name, varnamelen ) == 0 )
			{
				break;
			}
		}
	}
   return var;
}

CTEXTSTR InterShell_GetControlLabelText( PMENU_BUTTON button, PPAGE_LABEL label, CTEXTSTR variable )
{
	static TEXTCHAR output[256];
	int nOutput = 0;
	while( variable && variable[0] )
	{
		if( variable[0] == '%' )
		{
			PVARIABLE var = FindVariableByName( variable + 1);
         if( var )
			{
				if( var->flags.bString )
				{
					nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
									 , WIDE("%s"), (*var->data.string_value) );
				}
				else if( var->flags.bConstString )
				{
					nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
											 , WIDE("%s"), (var->data.string_const_value?var->data.string_const_value:WIDE("")) );
				}
				else if( var->flags.bInt )
				{
					nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
									 , WIDE("%ld"), (*var->data.int_value) );
				}
				else if( var->flags.bProc )
				{
					//lprintf( "Calling external function to get value..." );
					nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
											 , WIDE("%s"), var->data.proc() );
					//lprintf( "New output is [%s]", output );
				}
				else if( var->flags.bProcControlEx )
				{
					//lprintf( "Calling external function to get value..." );
					nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
											 , WIDE("%s"), var->data.proc_control_ex(var->psvUserData,button) );
					//lprintf( "New output is [%s]", output );
				}
				else if( var->flags.bProcEx )
				{
					//lprintf( "Calling external extended function to get value..." );
					nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
											 , WIDE("%s"), var->data.proc_ex( var->psvUserData ) );
					//lprintf( "New output is [%s]", output );
				}
				else
				{
					nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
									 , WIDE("%%%s"), var->name );
				}
				if( label )
				{
					if( FindLink( &var->references, label ) == INVALID_INDEX )
						AddLink( &var->references, label );
				}
            variable += strlen( var->name ); //varnamelen;
			}
			if( !var )
			{
				static int n;
            n++;
				nOutput += snprintf( output + nOutput, sizeof( output ) - nOutput - 1
								 , WIDE("[bad variable(%d)]"), n );
			}
		}
		else
		{
			if( nOutput < 255 )
			{
				output[nOutput] = variable[0];
				nOutput++;
			}
		}
      variable++;
	}
   output[nOutput] = 0;
   return output;
}

CTEXTSTR InterShell_TranslateLabelText( PPAGE_LABEL label, TEXTSTR output, int buffer_len, CTEXTSTR variable )
{
	int nOutput = 0;
	while( variable && variable[0] )
	{
		if( variable[0] == '%' )
		{
         PVARIABLE var = FindVariableByName( variable + 1 );
			//for( nVar = 0; nVar < NUM_VARIABLES; nVar++ )
			if( var )
			{
				{
					if( var->flags.bString )
					{
						nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
										 , WIDE("%s"), (*var->data.string_value) );
					}
					else if( var->flags.bConstString )
					{
						nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
										 , WIDE("%s"), (var->data.string_const_value?var->data.string_const_value:WIDE("")) );
					}
					else if( var->flags.bInt )
					{
						nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
										 , WIDE("%ld"), (*var->data.int_value) );
					}
					else if( var->flags.bProc )
					{
                  //lprintf( "Calling external function to get value..." );
                  nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
												 , WIDE("%s"), var->data.proc() );
						//lprintf( "New output is [%s]", output );
					}
					else
					{
						nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
										 , WIDE("%%%s"), var->name );
					}
					if( label )
					{
						if( FindLink( &var->references, label ) == INVALID_INDEX )
							AddLink( &var->references, label );
					}
               variable += strlen( var->name ); //varnamelen;
				}
			}
			else
			{
				CTEXTSTR env=StrChr( variable + 1, '%' );
				if( env )
				{
					CTEXTSTR env_var;
					TEXTSTR tmp = NewArray( TEXTCHAR, env - variable );
					StrCpyEx( tmp, variable+1, (env-variable-1) );
					tmp[(env-variable)-1] = 0;
#ifdef HAVE_ENVIRONMENT
					env_var = OSALOT_GetEnvironmentVariable( tmp );
					if( env_var )
					{
						nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
												 , WIDE( "%s" ), env_var );
						variable += (env-variable);
					}
					else
#endif
						nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
												 , WIDE("[bad variable]") );
				}
				else
					nOutput += snprintf( output + nOutput, buffer_len - nOutput - 1
											 , WIDE("[bad variable]") );
			}
		}
		else
		{
			if( nOutput < 255 )
			{
				output[nOutput] = variable[0];
				nOutput++;
			}
		}
      variable++;
	}
   output[nOutput] = 0;
   return output;
}

void LabelVariableChanged( PVARIABLE variable) // update any labels which are using this variable.
{
	PVARIABLE var;
	INDEX nVar;
	LIST_FORALL( extern_variables, nVar, PVARIABLE, var )
	{
		if( ( !variable ) || ( var == variable ) )
		{
			INDEX idx;
			PPAGE_LABEL label;
			LIST_FORALL( var->references, idx, PPAGE_LABEL, label )
			{
            CTEXTSTR tmp = InterShell_GetControlLabelText( label->button, label, label->label_text );
				//lprintf("Got one.");
				if( label->last_text && StrCmp( label->last_text, tmp ) == 0 )
				{
               continue;
				}
				else
				{
					Release( label->last_text );
					label->last_text = StrDup( tmp );
				}
				SetControlText( label->control, InterShell_GetControlLabelText( label->button, label, label->label_text ) );
            GetControlTextOffsetMinMax( label->control, &label->min_offset, &label->max_offset );
				//SmudgeCommon( label->control );
			}
		}
	}

}

void LabelVariablesChanged( PLIST variables) // update any labels which are using this variable.
{
	PVARIABLE var;
	INDEX idx;
	LIST_FORALL( variables, idx, PVARIABLE, var )
      LabelVariableChanged( var );
}


OnDestroyControl( TEXT_LABEL_NAME )( PTRSZVAL psv )
{
   PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PVARIABLE var;
	INDEX nVar;
	LIST_FORALL( extern_variables, nVar, PVARIABLE, var )
	{
      DeleteLink( &var->references, (POINTER)psv );
	}
   DeleteLink( &l.labels, title );
	Release( title->label_text );
   DestroyCommon( &title->control );
   Release( title );

}

OnCreateControl( TEXT_LABEL_NAME )( PSI_CONTROL frame, S_32 x, S_32 y, _32 w, _32 h )
//PTRSZVAL CPROC CreatePageTitle( S_32 x, S_32 y, _32 w, _32 h )
{
	PPAGE_LABEL title = New( struct page_label );
	MemSet( title, 0, sizeof( *title ) );
	title->page = ShellGetCurrentPage();
	title->label_text = NULL;
	title->control = MakeControl( frame, STATIC_TEXT, x, y, w, h, -1 );
	title->button = NULL;
   title->last_text = NULL;
	SetCommonBorder( title->control, BORDER_FIXED|BORDER_NONE );
	//SetControlAlignment( title->control, 2 );
	SetTextControlColors( title->control, BASE_COLOR_WHITE, 0 );
   SetCommonTransparent( title->control, TRUE );
	SetControlText( title->control, InterShell_GetControlLabelText( title->button, title, title->label_text ) );
	GetControlTextOffsetMinMax( title->control, &title->min_offset, &title->max_offset );
   AddLink( &l.labels, title );
   return (PTRSZVAL)title;
}

OnGetControl( TEXT_LABEL_NAME )( PTRSZVAL psv )
//PSI_CONTROL CPROC GetTitleControl( PTRSZVAL psv )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
   if( title )
		return title->control;
	else
      DebugBreak();
      return NULL;
}

OnSaveControl( TEXT_LABEL_NAME )( FILE *file,PTRSZVAL psv )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	fprintf( file, WIDE("color=$%02X%02X%02X%02X\n")
			 , AlphaVal( title->color )
			 , RedVal( title->color )
			 , GreenVal( title->color )
			 , BlueVal( title->color )
			 );
	fprintf( file, WIDE( "align center?%s\n" ), title->flags.bCenter?WIDE( "on" ):WIDE( "off" ) );
	fprintf( file, WIDE( "align right?%s\n" ), title->flags.bRight?WIDE( "on" ):WIDE( "off" ) );
	fprintf( file, WIDE( "align scroll?%s\n" ), title->flags.bScroll?WIDE( "on" ):WIDE( "off" ) );
	fprintf( file, WIDE( "align vertical?%s\n" ), title->flags.bVertical?WIDE( "on" ):WIDE( "off" ) );
	fprintf( file, WIDE( "align inverted?%s\n" ), title->flags.bInverted?WIDE( "on" ):WIDE( "off" ) );
	if( title->preset_name )
	{
		fprintf( file, WIDE("font name=%s\n"),title->preset_name );
	}
	if( title->label_text )
		fprintf( file, WIDE("label=%s\n"), title->label_text );
}

static PTRSZVAL CPROC SetTitleLabel( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, TEXTCHAR *, label );
   title->label_text = StrDup( label );
   return psv;
}

static PTRSZVAL CPROC SetTitleColor( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, CDATA, color );
   title->color = color;
   return psv;
}

static PTRSZVAL CPROC SetTitleCenter( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, LOGICAL, center );
   title->flags.bCenter = center;
   return psv;
}

static PTRSZVAL CPROC SetTitleVertical( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, LOGICAL, vertical );
   title->flags.bVertical = vertical;
   return psv;
}

static PTRSZVAL CPROC SetTitleInverted( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, LOGICAL, inverted );
   title->flags.bInverted = inverted;
   return psv;
}

static PTRSZVAL CPROC SetTitleRight( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, LOGICAL, right );
   title->flags.bRight = right;
   return psv;
}

static PTRSZVAL CPROC SetTitleScrollRightLeft( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, LOGICAL, scroll );
   title->flags.bScroll = scroll;
   return psv;
}

static PTRSZVAL CPROC SetTitleFontByName( PTRSZVAL psv, arg_list args )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	PARAM( args, TEXTCHAR *, name );
	title->font = UseAFont( name );
   title->preset_name = StrDup( name );
	return psv;
}

OnLoadControl( TEXT_LABEL_NAME )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
	AddConfigurationMethod( pch, WIDE("color=%c"), SetTitleColor );
   AddConfigurationMethod( pch, WIDE("font name=%m"), SetTitleFontByName );
   AddConfigurationMethod( pch, WIDE("label=%m"), SetTitleLabel );
   AddConfigurationMethod( pch, WIDE("align center?%b"), SetTitleCenter );
   AddConfigurationMethod( pch, WIDE("align scroll?%b"), SetTitleScrollRightLeft );
   AddConfigurationMethod( pch, WIDE("align right?%b"), SetTitleRight );
   AddConfigurationMethod( pch, WIDE("align vertical?%b"), SetTitleVertical );
   AddConfigurationMethod( pch, WIDE("align inverted?%b"), SetTitleInverted );
}

OnQueryShowControl( TEXT_LABEL_NAME )( PTRSZVAL psv )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	SetControlText( title->control, InterShell_GetControlLabelText( title->button, title, title->label_text ) );
	GetControlTextOffsetMinMax( title->control, &title->min_offset, &title->max_offset );
   return TRUE;
}

OnShowControl( TEXT_LABEL_NAME )( PTRSZVAL psv )
{
	PPAGE_LABEL title = (PPAGE_LABEL)psv;
	SetCommonFont( title->control, (title->font?(*title->font):NULL) );
	SetTextControlColors( title->control, title->color, 0 );
	SetControlText( title->control, InterShell_GetControlLabelText( title->button, title, title->label_text ) );
	GetControlTextOffsetMinMax( title->control, &title->min_offset, &title->max_offset );
   if( title->flags.bCenter )
		SetControlAlignment( title->control, TEXT_CENTER );
	else if( title->flags.bRight )
		SetControlAlignment( title->control, TEXT_RIGHT );
	else
		SetControlAlignment( title->control, TEXT_NORMAL );

	//SmudgeCommon( title->control );
}

OnSelectUser( TEXT_LABEL_NAME )( void )
{
	PVARIABLE var;
	INDEX nVar;
	LIST_FORALL( extern_variables, nVar, PVARIABLE, var )
	//for( nVar = 0; nVar < NUM_VARIABLES; nVar++ )
	{
		INDEX idx;
		PPAGE_LABEL label;
		LIST_FORALL( var->references, idx, PPAGE_LABEL, label )
		{
			SetControlText( label->control, InterShell_GetControlLabelText( label->button, label, label->label_text ) );
			GetControlTextOffsetMinMax( label->control, &label->min_offset, &label->max_offset );

		}
	}
}


typedef struct page_changer_dialog_struct
{
	//PPAGE_CHANGER page_changer;
   int *pOkay, *pDone;
} PAGE_DIALOG, *PPAGE_DIALOG;


static void CPROC PickLabelFont( PTRSZVAL psv, PSI_CONTROL pc )
{
	PPAGE_LABEL page_label = (PPAGE_LABEL)psv;
	Font *font = SelectAFont( GetFrame( pc )
									, &page_label->preset_name
									);
	if( font )
	{
		page_label->new_font = font;
	}
}


void CPROC VariableChanged( PTRSZVAL psv, PSI_CONTROL pc, PLISTITEM pli )
{
	PSI_CONTROL pc_text = (PSI_CONTROL)psv;
	if( pc_text )
	{
		TEXTCHAR buffer[256];
		GetItemText( pli, sizeof(buffer)-1, buffer+1 );
		buffer[0] = '%';
		TypeIntoEditControl( pc_text, buffer );
	}

}

void CPROC FillVariableList( PSI_CONTROL frame )
{
	PSI_CONTROL list = GetControl( frame, LST_VARIABLES );
	if( list )
	{
		PVARIABLE var;
		INDEX i;
		LIST_FORALL( extern_variables, i, PVARIABLE, var )
			//for( i = 0; i < NUM_VARIABLES; i++ )
		{
			AddListItem( list, var->name );
		}
		SetSelChangeHandler( list, VariableChanged, (PTRSZVAL)GetControl( frame, TXT_CONTROL_TEXT ) );
	}
}

OnEditControl( TEXT_LABEL_NAME )( PTRSZVAL psv, PSI_CONTROL parent_frame )
{
	PPAGE_LABEL page_label = (PPAGE_LABEL)psv;
	if( page_label )
	{
      PSI_CONTROL frame;
		int okay = 0;
		int done = 0;
		// psv may be passed as NULL, and therefore there was no task assicated with this
		// button before.... the button is blank, and this is an initial creation of a button of this type.
		// basically this should call (psv=CreatePaper(button)) to create a blank button, and then launch
		// the config, and return the button created.
		//PPAPER_INFO issue = button->paper;
		frame = LoadXMLFrame( WIDE("page_label_property.isframe") );
		if( frame )
		{
			//could figure out a way to register methods under
			// the filename of the property thing being loaded
			// for future use...
			{ // init frame which was loaded..
				SetCommonButtons( frame, &done, &okay );
            page_label->new_font = NULL;
				//EnableColorWellPick( SetColorWell( GetControl( frame, CLR_BACKGROUND), page_label->button->color ), TRUE );
				EnableColorWellPick( SetColorWell( GetControl( frame, CLR_TEXT_COLOR), page_label->color ), TRUE );
				//EnableColorWellPick( SetColorWell( GetControl( frame, CLR_RING_BACKGROUND), page_label->button->secondary_color ), TRUE );
            SetCheckState( GetControl( frame, CHECKBOX_LABEL_CENTER ), page_label->flags.bCenter );
            SetCheckState( GetControl( frame, CHECKBOX_LABEL_SCROLL ), page_label->flags.bScroll );
            SetCheckState( GetControl( frame, CHECKBOX_LABEL_RIGHT ), page_label->flags.bRight );
				SetButtonPushMethod( GetControl( frame, BTN_PICKFONT ), PickLabelFont, psv );
				SetCommonText( GetControl( frame, TXT_CONTROL_TEXT ), page_label->label_text );
				{
					PSI_CONTROL list = GetControl( frame, LST_VARIABLES );
					if( list )
					{
						PVARIABLE var;
                  INDEX i;
                  LIST_FORALL( extern_variables, i, PVARIABLE, var )
						//for( i = 0; i < NUM_VARIABLES; i++ )
						{
							AddListItem( list, var->name );
						}
                  SetSelChangeHandler( list, VariableChanged, (PTRSZVAL)GetControl( frame, TXT_CONTROL_TEXT ) );
					}
				}
			}
			DisplayFrameOver( frame, parent_frame );

			EditFrame( frame, TRUE );
			//edit frame must be done after the frame has a physical surface...
			// it's the surface itself that allows the editing...
			CommonWait( frame );
			if( okay )
			{
				// blah get the resuslts...
            if( page_label->new_font )
					page_label->font = page_label->new_font;
				page_label->color = GetColorFromWell( GetControl( frame, CLR_TEXT_COLOR ) );
				page_label->flags.bCenter = GetCheckState( GetControl( frame, CHECKBOX_LABEL_CENTER ) );
            if( !page_label->flags.bCenter )
					page_label->flags.bRight = GetCheckState( GetControl( frame, CHECKBOX_LABEL_RIGHT ) );
				else
					page_label->flags.bRight = 0;
            page_label->flags.bScroll = GetCheckState( GetControl( frame, CHECKBOX_LABEL_SCROLL ) );

				{
					TEXTCHAR buffer[128];
					int i,o;
					for( i = o = 0; buffer[i]; i++,o++ )
					{
						if( buffer[o] == '\\' )
						{
							switch( buffer[o+1] )
							{
							case 'n':
                        i++;
                        buffer[o] = '\n';
                        break;
							}
						}
					}
					GetControlText( GetControl( frame, TXT_CONTROL_TEXT ), buffer, sizeof( buffer ) );
					if( page_label->label_text )
						Release( page_label->label_text );
					page_label->label_text = StrDup( buffer );
				}
			}
         DestroyFrame( &frame );
		}
	}
   return psv;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Begin button method that can set variable text...
//  useful for setting in macros to indicate current task mode? or perhaps
//  status messages to indicate what needs to be done now?
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct button_set_text {
	TEXTSTR varname;
   TEXTSTR newval;
};
typedef struct button_set_text SETVAR;
typedef struct button_set_text *PSETVAR;

void SetVariable( CTEXTSTR name, CTEXTSTR value )
{
	PVARIABLE pVar = FindVariableByName( name );
	if( !pVar )
      pVar = CreateLabelVariableEx( name, LABEL_TYPE_CONST_STRING, value, 0 );
	else if( pVar->flags.bConstString )
	{
      /* this isn't const data... or better not be... */
		Release( (POINTER)pVar->data.string_const_value );
		if( value )
			pVar->data.string_const_value = StrDup( value );
		else
			pVar->data.string_const_value = NULL;
		LabelVariableChanged( pVar );
	}
	else
		lprintf( WIDE( "Attempt to set a variable that is not direct text, cannot override routines...name:%s newval:%s" )
				 , name, value );
}

OnKeyPressEvent( WIDE( "text/Set Variable" ) )( PTRSZVAL psv )
{
	PSETVAR pSetVar = (PSETVAR)psv;
   SetVariable( pSetVar->varname, pSetVar->newval );

   //return 1;
}

OnCreateMenuButton( WIDE( "text/Set Variable" ) )( PMENU_BUTTON button )
{
	PSETVAR pSetVar = New( SETVAR );
   pSetVar->varname = NULL;
   pSetVar->newval = NULL;
	InterShell_SetButtonStyle( button, WIDE( "bicolor square" ) );

   return (PTRSZVAL)pSetVar;
}

OnConfigureControl( WIDE( "text/Set Variable" ) )( PTRSZVAL psv, PSI_CONTROL parent )
{
	PSETVAR pSetVar = (PSETVAR)psv;
	PSI_CONTROL frame;
	frame = LoadXMLFrameOver( parent, WIDE( "configure_text_setvar_button.isframe" ) );
	if( frame )
	{
		int okay = 0;
		int done = 0;
		SetControlText( GetControl( frame, BTN_VARNAME ), pSetVar->varname );
		SetControlText( GetControl( frame, BTN_VARVAL ), pSetVar->newval );
      FillVariableList( frame );
		SetCommonButtons( frame, &done, &okay );
      DisplayFrameOver( frame, parent );
		CommonWait( frame );
		if( okay )
		{
         TEXTCHAR buffer[256];
			GetControlText( GetControl( frame, BTN_VARNAME ), buffer, sizeof( buffer )  );
			if( ( pSetVar->varname && strcmp( pSetVar->varname, buffer ) ) || ( !pSetVar->varname && buffer[0] ) )
			{
				Release( pSetVar->varname );
				pSetVar->varname = StrDup( buffer );
			}
			GetControlText( GetControl( frame, BTN_VARVAL ), buffer, sizeof( buffer ) );
			if( ( pSetVar->newval && strcmp( pSetVar->newval, buffer ) ) || ( !pSetVar->newval && buffer[0] ) )
			{
				Release( pSetVar->newval );
				pSetVar->newval = StrDup( buffer );
			}
		}
		{
			PVARIABLE pVar = FindVariableByName( pSetVar->varname );
			if( !pVar )
				pVar = CreateLabelVariableEx( pSetVar->varname, LABEL_TYPE_CONST_STRING, pSetVar->newval, 0 );
		}
      DestroyFrame( &frame );
	}
   return psv;
}

OnSaveControl( WIDE( "text/Set Variable" ) )( FILE *file, PTRSZVAL psv )
{
	PSETVAR pSetVar = (PSETVAR)psv;
   fprintf( file, WIDE( "set variable text name=%s\n" ), EscapeMenuString( pSetVar->varname ) );
   fprintf( file, WIDE( "set variable text value=%s\n" ), EscapeMenuString( pSetVar->newval ) );
}

static void OnCloneControl( WIDE( "text/Set Variable" ) )( PTRSZVAL psvNew, PTRSZVAL psvOld )
{
	PSETVAR pSetVarNew = (PSETVAR)psvNew;
	PSETVAR pSetVarOld = (PSETVAR)psvOld;
   pSetVarNew->varname = StrDup( pSetVarOld->varname );
   pSetVarNew->newval = StrDup( pSetVarOld->newval );
}

static PTRSZVAL CPROC SetVariableVariableName( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	PSETVAR pSetVar = (PSETVAR)psv;
	pSetVar->varname = StrDup( name );
	{
		PVARIABLE pVar = FindVariableByName( pSetVar->varname );
		if( !pVar )
			pVar = CreateLabelVariableEx( pSetVar->varname, LABEL_TYPE_CONST_STRING, NULL, 0 );
 	}
   return psv;
}

static PTRSZVAL CPROC SetVariableVariableValue( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, name );
	PSETVAR pSetVar = (PSETVAR)psv;
	pSetVar->newval = StrDup( name );
   return psv;
}

OnLoadControl( WIDE( "text/Set Variable" ) )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
   AddConfigurationMethod( pch,  WIDE( "set variable text name=%m" ), SetVariableVariableName );
   AddConfigurationMethod( pch,  WIDE( "set variable text value=%m" ), SetVariableVariableValue );
}

INTERSHELL_NAMESPACE_END

