#include <controls.h>
#include <deadstart.h>
#include <psi.h>

static CONTROL_REGISTRATION calendar;

typedef struct {
	struct {
		_32 bShowMonth : 1;
		_32 bStuff : 1;
		_32 bMonth: 1;
		_32 bWeek : 1;
		_32 bDays : 1;
		_32 bNow : 1;
	} flags;

} CALENDER, *PCALENDAR;




#if 0
int CPROC DrawCalender( void )
{
	// stuff...
	ValidatedControlData( PCALENDAR, calendar.nType, pCal, pc );
	if( pCal )
	{
		Image surface = GetCommonSurface( pc );
		if( pCal->flags.bMonth )
		{
		}
		else if( pCal->flags.bWeek )
		{
		}
		else if( pCal->flags.bDays )
		{
		}
		else if( pCal->flags.bNow )
		{
			if( pCal->flags.bTime )
			{
			}
		}
	}

}













CONTROL_REGISTRATION calendar = { "Calender Widget"
										  , { { 50, 50 }, BORDER_NONE, sizeof( CALENDER ) }
										  , NULL
};

PRELOAD(DoRegisterControl)
{
   DoRegisterControl( &calendar );
}


#endif
