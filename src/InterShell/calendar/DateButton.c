//comment
#include <controls.h>

#define USES_INTERSHELL_INTERFACE
#define DEFINES_INTERSHELL_INTERFACE
#include "../intershell_registry.h"
#include "../intershell_export.h"

static struct local_calendar_info
{
	int current_index;

	int current_month;
	int current_year;
	int current_day;
	int first_date;
   int days;
	PVARIABLE pCurrentDate;
   PLIST variables;
   CTEXTSTR current_date;
} l;

static int day_map[] = {31,28,31,30,31,30,31,31,30,31,30,31};

static int get_month_days( int month, int year )
{
	if( month == 2 )
		return day_map[1] + ((year %4)==0)?((year%100)==0)?((year%400)==0)?1:0:1;
   return day_map[month-1];
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
	l.pCurrentDate = CreateLabelVariable( "<Current Date>", &l.date );
   AddLink( &l.variables, l.pCurrentDate );
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

static void UpdateCalendar( void )
{
   ShellSetCurrentPage( "here" );
   //UpdateLabelVariables( l.variables );
}

OnPageChange( "Calendar" )
{
	l.current_index = 0;
	l.first_date = GetStartDow( l.current_month, l.current_year );
   l.days = get_month_days( l.current_month, l.current_year );
}

OnQueryShowControl( "Calendar/Date Selector" )( PTRSZVAL psv )
{
	if( l.current_index < 0 )
	{
      return FALSE;
	}
   if( l.current_index > l.first_date + l.days;
   return TRUE;
}

OnCreateButton( "Calendar/Date Selector" )( PMENU_BUTTON button )
{
   return button;
}

OnKeyPressEvent( "Calendar/Previous Month" )( PTRSZVAL button )
{
	l.month--;
	if( l.month < 1 )
		l.month += 12;
   UpdateCalendar();
}

OnCreateButton( "Calendar/Previous Month" )( PMENU_BUTTON button )
{
   InterShell_SetButtonText( button, "Previous_Month" );
   return button;
}

OnKeyPressEvent( "Calendar/Next Month" )( PTRSZVAL button )
{
	l.month++;
	if( l.month > 12 )
      l.month -= 12;
   UpdateCalendar();
}

OnCreateButton( "Calendar/Next Month" )( PMENU_BUTTON button )
{
   InterShell_SetButtonText( button, "Next_Month" );
   return button;
}

OnKeyPressEvent( "Calendar/Select Today" )( PTRSZVAL button )
{
   ResetDate();
   UpdateCalendar();
}

OnCreateButton( "Calendar/Select Today" )( PMENU_BUTTON button )
{
   InterShell_SetButtonText( button, "Select_Today" );
   return button;
}

