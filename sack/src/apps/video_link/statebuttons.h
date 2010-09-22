#ifndef STATE_BUTTONS_HEADER
#define STATE_BUTTONS_HEADER


typedef struct hall_info
{
	char *name;
	_32 hall_id;
	struct {
		// prior states are tracked as _bFlagName so the transitions
		// can be tracked ... bFlagName is set from the database, and only if
      // ( bFlagName != _bFlagName ) then cause an update
		_32 _bEnabled : 1;	   //_32 _bProhibited : 1; // this is bDisabled
		_32 bEnabled : 1; 		//_32 bProhibited : 1; // this is bDisabled
		_32 _bDisabled : 1;
		_32 bDisabled : 1;
		_32 _bMaster : 1;
		_32 bMaster : 1;
		_32 _bDelegate : 1;
		_32 bDelegate : 1;
		_32 _bParticipating : 1;
      _32 bParticipating : 1;
		_32 _bLaunching : 1;
		_32 bLaunching : 1;
		_32 bStateChanged : 1; // set when one of the above has changed during a sql state read.
		_32 bStateRead : 1;
		_32 bDVDActive : 1;
		_32 _bDVDActive : 1;
		_32 bMediaActive : 1;
		_32 _bMediaActive : 1;
	} flags;
	struct {
		_32 host : 1;
	} type;
   PLIST buttons; // (PBUTTON_INFO) buttons which this is represented by
	PVARIABLE label_var;
	_32 seconds_ago_alive;
   TEXTCHAR second_alive_buffer[64];
   PVARIABLE label_alive_var;
} HALL_INFO, *PHALL_INFO;


typedef struct global_statebuttons_tag{
	PLIST halls; // all known halls. PHALL_INFO
#ifdef USE_KEYPRESSGUARD
	struct{
		_64 pressed;
		_64 period;
		PVARIABLE label_var;
		CTEXTSTR label_str;
		struct{
			_32 remind:1;
			_32 reminding:1;
			_8 ucReminded; // number of times reminded.
		}flags;
	} keypressguard;
#endif

#ifdef USE_RESETGUARD
	struct{
		_64 pressed;
      PMENU_BUTTON btn; //sorry.
	}resetguard;
#endif
        PODBC odbc;
}GLOBAL;

#define g global_statebutton_data
#ifndef DEFINE_STATEBUTTONS_GLOBAL
extern
#endif
GLOBAL g;

void DVDActiveStateChangedEach(_32 which, _32 how);
void DVDActiveStateChanged(void);
LOGICAL AllowLinkStateKeypress(void);
void MediaActiveStateChangedEach(_32 which, _32 how );

#endif
