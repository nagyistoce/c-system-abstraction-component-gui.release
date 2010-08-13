
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
	PVARIABLE pCurrentDate;
   CTEXTSTR current_date;
} l;


PRELOAD( InitCalendar )
{
   pCurrentDate = CreateLabelVariable( "<Current Date>", &l.date );
}

int GetStartDow( int mo, int year )
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


OnCreateButton( "Calendar/Date Selector" )( PMENU_BUTTON button )
{
}

OnCreateButton( "Calendar/Previous Month" )( PMENU_BUTTON button )
{
}

OnCreateButton( "Calendar/Next Month" )( PMENU_BUTTON button )
{
}

OnCreateButton( "Calendar/Select Today" )( PMENU_BUTTON button )
{
}

