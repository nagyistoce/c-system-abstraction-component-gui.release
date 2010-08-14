//comment
#include <controls.h>

#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#include "../intershell_registry.h"
#include "../intershell_export.h"

struct day_selector_info
{
	struct {
		BIT_FIELD visible : 1;
	} flags;
	int day;
   int update;
   PMENU_BUTTON button;
};

static struct local_calendar_info
{
	int current_index;
   int current_update;

	int current_month;
	int current_year;
	int current_day;
	int first_date;
   int days;
	PVARIABLE pCurrentDate;
   PLIST variables;
   TEXTSTR current_date; // 03/12/1999
   TEXTSTR current_month_var; // march
   TEXTSTR current_month_number; // 3
   TEXTSTR current_month_number2; // 03
   TEXTSTR current_year_var; // 1999
   TEXTSTR current_month_year; // mar, 1999
	TEXTSTR current_date_long; // march 12, 1999
	TEXTSTR current_date_extra_long; // thursday march 12, 1999
	TEXTSTR current_sql_date; // thursday march 12, 1999

   PLIST controls;
} l;


static TEXTSTR months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
static int day_map[] = {31,28,31,30,31,30,31,31,30,31,30,31};

static int get_month_days( int month, int year )
{
	if( month == 2 )
		return day_map[1] + (((year %4)==0)?((year%100)==0)?((year%400)==0)?1:0:1:0);
   return day_map[month-1];
}

static int GetStartDow( int mo, int year )
{
	SYSTEMTIME st;
	FILETIME ft;
	memset( &st, 0, sizeof( st ) );
	st.wYear = year;
	st.wMonth = mo;
	st.wDay = 1;
	SystemTimeToFileTime( &st, &ft );
	FileTimeToSystemTime( &ft, &st );
   return st.wDayOfWeek;
}


static void UpdateCalendar( LOGICAL bSetVariablesOnly )
{
#define SetVariable(v,f,...) 	snprintf( tmp, sizeof( tmp ), f,##__VA_ARGS__ ); \
	if( v ) Release( v );                                        \
	v = StrDup( tmp );

	TEXTCHAR tmp[64];
	l.first_date = GetStartDow( l.current_month, l.current_year );
	l.days = get_month_days( l.current_month, l.current_year );
   SetVariable( l.current_date, "%02d/%02d/%04d", l.current_day, l.current_month, l.current_year );
   SetVariable( l.current_sql_date, "%04d%02d%02d",l.current_year, l.current_month, l.current_day );
	SetVariable( l.current_month_var, "%s", l.current_month?months[l.current_month-1]:"<NAM>" );
   SetVariable( l.current_month_number, "%d", l.current_month );
	SetVariable( l.current_month_number2, "%02d", l.current_month );
	SetVariable( l.current_year_var, "%d", l.current_year );
	SetVariable( l.current_month_year, "%s,%d", l.current_month?months[l.current_month-1]:"<NAM>", l.current_year );
	SetVariable( l.current_date_long, "%s %d, %d", l.current_month?months[l.current_month-1]:"<NAM>", l.current_day, l.current_year );
	SetVariable( l.current_date_extra_long, "<dow> %s %d, %d", l.current_month?months[l.current_month-1]:"<NAM>", l.current_day, l.current_year );

	if( !bSetVariablesOnly )
	{
      InterShell_DisablePageUpdate( TRUE );
		LabelVariablesChanged( l.variables );
		l.current_update++;
      l.current_index = 0;
		{
			INDEX idx;
			PMENU_BUTTON button;
			LIST_FORALL( l.controls, idx, PMENU_BUTTON, button )
			{
            UpdateButton( button );
			}
		}
		//ShellSetCurrentPage( "here" );
		InterShell_DisablePageUpdate( FALSE );
	}
}

static void ResetDate( void )
{
   SYSTEMTIME st;
	GetLocalTime( &st );
	l.current_month = st.wMonth;
	l.current_day = st.wDay;
	l.current_year = st.wYear;
}

PRELOAD( InitCalendar )
{
	AddLink( &l.variables, CreateLabelVariable( "<Current Date>", LABEL_TYPE_STRING, &l.current_date ) );
	AddLink( &l.variables, CreateLabelVariable( "<Current Month>", LABEL_TYPE_STRING, &l.current_month_number ) );
	AddLink( &l.variables, CreateLabelVariable( "<Current Month2>", LABEL_TYPE_STRING, &l.current_month_number2 ) );
	AddLink( &l.variables, CreateLabelVariable( "<Current Month Name>", LABEL_TYPE_STRING, &l.current_month_var ) );
	AddLink( &l.variables, CreateLabelVariable( "<Current Year>", LABEL_TYPE_STRING, &l.current_year_var ) );
	AddLink( &l.variables, CreateLabelVariable( "<Current Month,Year>", LABEL_TYPE_STRING, &l.current_month_year ) );
	AddLink( &l.variables, CreateLabelVariable( "<Current Date Long>", LABEL_TYPE_STRING, &l.current_date_long ) );
	AddLink( &l.variables, CreateLabelVariable( "<Current Date Extra Long>", LABEL_TYPE_STRING, &l.current_date_extra_long ) );
   ResetDate();
	UpdateCalendar( TRUE );
}

OnChangePage( "Calendar" )( void )
{
   lprintf( "Empty List" );
   EmptyList( &l.controls );
	l.current_index = 0;
	l.first_date = GetStartDow( l.current_month, l.current_year );
	l.days = get_month_days( l.current_month, l.current_year );
   l.current_update++;
   return TRUE;
}

OnQueryShowControl( "Calendar/Date Selector" )( PTRSZVAL psv )
{
	struct day_selector_info *pDay = (struct day_selector_info*)psv;
	if( pDay->update != l.current_update )
	{
      lprintf( "new update" );
		pDay->update = l.current_update;
	}
	else
	{
      lprintf( "Short return" );
      return pDay->flags.visible;
	}
   AddLink( &l.controls, pDay->button );
   lprintf( "%p %d %d %d", psv, l.current_index, l.first_date, l.days );
	if( l.current_index < l.first_date )
	{
		l.current_index++;
      pDay->flags.visible = 0;
      return FALSE;
	}
	if( l.current_index >= (l.first_date + l.days) )
	{
      l.current_index++;
      pDay->flags.visible = 0;
      return FALSE;
	}
	{
		struct day_selector_info *pDay = (struct day_selector_info*)psv;
		char tmp[32];
      pDay->day = (l.current_index - l.first_date) + 1;
      snprintf( tmp, sizeof( tmp ), "%d", pDay->day );
		InterShell_SetButtonText( pDay->button, tmp );

      l.current_index++;
      pDay->flags.visible = 1;
	}
   return TRUE;
}

OnKeyPressEvent( "Calendar/Date Selector" )( PTRSZVAL psv )
{
   struct day_selector_info *pDay = (struct day_selector_info*)psv;
   l.current_day = pDay->day;
	UpdateCalendar( FALSE );

}

OnCreateMenuButton( "Calendar/Date Selector" )( PMENU_BUTTON button )
{
	struct day_selector_info *pDay = New(struct day_selector_info);
   MemSet( pDay, 0, sizeof( struct day_selector_info ) );
   pDay->button = button;
   return (PTRSZVAL)pDay;
}

OnKeyPressEvent( "Calendar/Previous Month" )( PTRSZVAL button )
{
	l.current_month--;
	if( l.current_month < 1 )
	{
		l.current_month += 12;
      l.current_year--;
	}
   UpdateCalendar( FALSE );
}

OnCreateMenuButton( "Calendar/Previous Month" )( PMENU_BUTTON button )
{
   InterShell_SetButtonText( button, "Previous_Month" );
   return (PTRSZVAL)button;
}

OnKeyPressEvent( "Calendar/Next Month" )( PTRSZVAL button )
{
	l.current_month++;
	if( l.current_month > 12 )
	{
		l.current_month -= 12;
      l.current_year++;
	}
   UpdateCalendar( FALSE );
}

OnCreateMenuButton( "Calendar/Next Month" )( PMENU_BUTTON button )
{
   InterShell_SetButtonText( button, "Next_Month" );
   return (PTRSZVAL)button;
}

OnKeyPressEvent( "Calendar/Select Today" )( PTRSZVAL button )
{
   ResetDate();
   UpdateCalendar( FALSE );
}

OnCreateMenuButton( "Calendar/Select Today" )( PMENU_BUTTON button )
{
   InterShell_SetButtonText( button, "Select_Today" );
   return (PTRSZVAL)button;
}

