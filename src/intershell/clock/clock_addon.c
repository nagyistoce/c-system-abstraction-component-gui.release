#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#include <stdhdrs.h> //DebugBreak()
#include "resource.h"

#include <controls.h>
#include <psi/clock.h>

#include "intershell_export.h"
#include "intershell_registry.h"

//---------------------------------------------------------------------------
USE_PSI_CLOCK_NAMESPACE

enum {
	CHECKBOX_ANALOG = 4000
	  , CHECKBOX_DATE
	  , CHECKBOX_SINGLE_LINE
	  , CHECKBOX_DAY_OF_WEEK
	  , EDIT_BACKGROUND_IMAGE
     , EDIT_ANALOG_IMAGE
};

typedef struct clock_info_tag
{
	struct {
		BIT_FIELD bAnalog : 1;  // we want to set analog, first show will enable
		BIT_FIELD bSetAnalog : 1; // did we already set analog?
		BIT_FIELD bDate : 1;
		BIT_FIELD bDayOfWeek : 1;
		BIT_FIELD bSingleLine : 1;
	} flags;

	CDATA backcolor, color;

	Font *font;

   /* this control may be destroyed and recreated based on other options */
	PSI_CONTROL control;
   TEXTSTR image_name;
	Image image;
   TEXTSTR analog_image_name;
   struct clock_image_thing image_desc;
} CLOCK_INFO, *PCLOCK_INFO;

PRELOAD( RegisterExtraClockConfig )
{
   EasyRegisterResource( "InterShell/" TARGETNAME, CHECKBOX_DATE, RADIO_BUTTON_NAME );
   EasyRegisterResource( "InterShell/" TARGETNAME, CHECKBOX_DAY_OF_WEEK, RADIO_BUTTON_NAME );
   EasyRegisterResource( "InterShell/" TARGETNAME, CHECKBOX_ANALOG, RADIO_BUTTON_NAME );
   EasyRegisterResource( "InterShell/" TARGETNAME, CHECKBOX_DATE, RADIO_BUTTON_NAME );
   EasyRegisterResource( "InterShell/" TARGETNAME, EDIT_BACKGROUND_IMAGE, EDIT_FIELD_NAME );
   EasyRegisterResource( "InterShell/" TARGETNAME, EDIT_ANALOG_IMAGE, EDIT_FIELD_NAME );
}

OnCreateControl("Clock")
/*PTRSZVAL CPROC CreateClock*/( PSI_CONTROL frame, S_32 x, S_32 y, _32 w, _32 h )
{
	PCLOCK_INFO info = New( CLOCK_INFO );
   MemSet( info, 0, sizeof( *info ) );
	info->color = BASE_COLOR_WHITE;

	info->control = MakeNamedControl( frame
											  , WIDE("Basic Clock Widget")
											  , x
											  , y
											  , w
											  , h
											  , -1
											  );

   // none of these are accurate values, they are just default WHITE and nothing.
	InterShell_SetButtonColors( NULL, info->color, info->backcolor, 0, 0 );
	SetClockColor( info->control, info->color );
	SetClockBackColor( info->control, info->backcolor );
	// need to supply extra information about the image, location of hands and face in image
   // and the spots...
	//MakeClockAnalog( info->control );
	info->font = InterShell_GetCurrentButtonFont();
	if( info->font )
		SetCommonFont( info->control, (*info->font ) );
	// the result of this will be hidden...
	return (PTRSZVAL)info;
}

OnShowControl( "Clock" )( PTRSZVAL psv )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	StartClock( info->control );
}

OnConfigureControl( WIDE("Clock") )( PTRSZVAL psv, PSI_CONTROL parent_frame )
//PTRSZVAL CPROC ConfigureClock( PTRSZVAL psv )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	{
		PCOMMON frame = NULL; 
		int okay = 0;
		int done = 0;
		if( !frame )
		{
			PCONTROL pc;
         frame = LoadXMLFrameOver( parent_frame, WIDE("Clock_Properties.isFrame") );
			//frame = CreateFrame( WIDE("Clock Properties"), 0, 0, 420, 250, 0, NULL );
			if( frame )
			{
				//MakeTextControl( frame, 5, 15, 120, 18, TXT_STATIC, WIDE("Text Color"), 0 );
				//EnableColorWellPick( MakeColorWell( frame, 130, 15, 18, 18, CLR_TEXT_COLOR, info->color ), TRUE );
				SetCommonButtonControls( frame );
				SetCheckState( GetControl( frame, CHECKBOX_ANALOG ), info->flags.bAnalog );
            SetControlText( GetControl( frame, EDIT_ANALOG_IMAGE ), info->analog_image_name );
            SetControlText( GetControl( frame, EDIT_BACKGROUND_IMAGE ), info->image_name );
				//EnableColorWellPick( SetColorWell( GetControl( frame, CLR_TEXT_COLOR), page_label->color ), TRUE );
            //SetButtonPushMethod( GetControl( frame, CHECKBOX_ANALOG ), ChangeClockStyle );
				SetCommonButtons( frame, &done, &okay );
				DisplayFrameOver( frame, parent_frame );
				CommonWait( frame );
				if( okay )
				{
               char buffer[256];
					GetCommonButtonControls( frame );
					info->font = InterShell_GetCurrentButtonFont();
					if( info->font )
                  SetCommonFont( info->control, (*info->font ) );
					info->color = GetColorFromWell( GetControl( frame, CLR_TEXT_COLOR ) );
					info->backcolor = GetColorFromWell( GetControl( frame, CLR_BACKGROUND ) );
					{
						GetControlText( GetControl( frame, EDIT_BACKGROUND_IMAGE ), buffer, sizeof( buffer ) );
						if( info->image_name )
							Release( info->image_name );
						info->image_name = StrDup( buffer );
					}
					{
						GetControlText( GetControl( frame, EDIT_ANALOG_IMAGE ), buffer, sizeof( buffer ) );
						if( info->analog_image_name )
							Release( info->analog_image_name );
						info->analog_image_name = StrDup( buffer );
					}
					SetClockColor( info->control, info->color );
					SetClockBackColor( info->control, info->backcolor );
               info->flags.bAnalog = GetCheckState( GetControl( frame, CHECKBOX_ANALOG ) );
               info->flags.bDate = GetCheckState( GetControl( frame, CHECKBOX_DATE ) );
               info->flags.bSingleLine = GetCheckState( GetControl( frame, CHECKBOX_SINGLE_LINE ) );
               info->flags.bDayOfWeek = GetCheckState( GetControl( frame, CHECKBOX_DAY_OF_WEEK ) );
               if( info->image )
						SetClockBackImage( info->control, info->image );
				}
            DestroyFrame( &frame );
			}
		}
	}
   return psv;
}


OnSaveControl( WIDE( "Clock" ) )( FILE *file,PTRSZVAL psv )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	fprintf( file, WIDE("Clock color=$%02lX%02lX%02lX%02lX\n")
			 , AlphaVal( info->color )
			 , RedVal( info->color )
			 , GreenVal( info->color )
			 , BlueVal( info->color )
			 );
	fprintf( file, WIDE("Clock back color=$%02lX%02lX%02lX%02lX\n")
			 , AlphaVal( info->backcolor )
			 , RedVal( info->backcolor )
			 , GreenVal( info->backcolor )
			 , BlueVal( info->backcolor )
			 );
	fprintf( file, WIDE("Clock background image=%s\n" ), info->image_name?info->image_name:"" );
	fprintf( file, WIDE("Clock is analog?%s\n"), info->flags.bAnalog?"Yes":"No" );

	fprintf( file, WIDE("Clock analog image=%s\n" ), info->analog_image_name?info->analog_image_name:"images/Clock.png" );
	{
		TEXTSTR out;
      EncodeBinaryConfig( &out, &info->image_desc, sizeof( info->image_desc ) );
		fprintf( file, WIDE("Clock analog description=%s\n" ), out );
		Release( out );
	}

	InterShell_SaveCommonButtonParameters( file );

}


static PTRSZVAL CPROC ReloadClockColor( PTRSZVAL psv, arg_list args )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	PARAM( args, CDATA, color );
   info->color = color;
   return psv;
}

static PTRSZVAL CPROC ReloadClockBackColor( PTRSZVAL psv, arg_list args )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	PARAM( args, CDATA, color );
   info->backcolor = color;
   return psv;
}

static PTRSZVAL CPROC SetClockAnalog( PTRSZVAL psv, arg_list args )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	PARAM( args, LOGICAL, bAnalog );
	info->flags.bAnalog = bAnalog;
   return psv;
}
static PTRSZVAL CPROC SetClockAnalogImage( PTRSZVAL psv, arg_list args )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	PARAM( args, CTEXTSTR, name );
	info->analog_image_name = StrDup( name );

   return psv;
}
static PTRSZVAL CPROC SetClockBackgroundImage( PTRSZVAL psv, arg_list args )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	PARAM( args, CTEXTSTR, name );
	info->image_name = StrDup( name );

   return psv;
}
static PTRSZVAL CPROC SetClockAnalogImageDesc( PTRSZVAL psv, arg_list args )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	PARAM( args, struct clock_image_thing *, desc );
	PARAM( args, _32, size );
	if( size == sizeof( struct clock_image_thing ) )
	{
		info->image_desc = (*desc);
	}
	else
      DebugBreak();
   return psv;
}

OnLoadControl( WIDE( "Clock" ) )( PCONFIG_HANDLER pch, PTRSZVAL psv )
{
   AddConfigurationMethod( pch, WIDE("Clock color=%c"), ReloadClockColor );
	AddConfigurationMethod( pch, WIDE("Clock back color=%c"), ReloadClockBackColor );
	AddConfigurationMethod( pch, WIDE("Clock is analog?%b"), SetClockAnalog );
   AddConfigurationMethod( pch, WIDE("Clock analog image=%m" ), SetClockAnalogImage );
   AddConfigurationMethod( pch, WIDE("Clock background image=%m" ), SetClockBackgroundImage );
   AddConfigurationMethod( pch, WIDE("Clock analog description=%b" ), SetClockAnalogImageDesc );
}

OnFixupControl( WIDE("Clock") )(  PTRSZVAL psv )
//void CPROC FixupClock(  PTRSZVAL psv )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	if( info )
	{
		SetClockColor( info->control, info->color );
		SetClockBackColor( info->control, info->backcolor );
		InterShell_SetButtonColors( NULL, info->color, info->backcolor, 0, 0 );
	}
}

OnGetControl( WIDE("Clock") )( PTRSZVAL psv )
//PSI_CONTROL CPROC GetClockControl( PTRSZVAL psv )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
   return info->control;
}

OnEditEnd( WIDE("Clock") )(PTRSZVAL psv )
//void CPROC ResumeClock( PTRSZVAL psv)
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	lprintf( "Break." );
	StartClock( info->control );
}

OnEditBegin( WIDE("Clock") )( PTRSZVAL psv )
//void CPROC PauseClock( PTRSZVAL psv)
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	//HideCommon( info->control );
	StopClock( info->control );
}

OnQueryShowControl( WIDE("Clock") )( PTRSZVAL psv )
{
	PCLOCK_INFO info = (PCLOCK_INFO)psv;
	if( info->flags.bAnalog )
	{
		MakeClockAnalogEx( info->control, info->analog_image_name, &info->image_desc );
	}
	else
		MakeClockAnalogEx( info->control, NULL, NULL ); // hrm this should turn off the analog feature...
	return TRUE;
}

//---------------------------------------------------------------------------
