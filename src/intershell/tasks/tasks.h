
#ifndef TASK_STRUCTURES_DEFINED
#define TASK_STRUCTURES_DEFINED

typedef struct task_tag
{
   DeclareLink( struct task_tag );
	struct {
		_32 bConfirm : 1;
		_32 bPassword : 1;
		//_32 bImage : 1;
		//_32 bUpdated : 1; // key modified...
		_32 bLaunchAt : 1;
		_32 bLaunchWhenCallerUp : 1;
		_32 bAutoLaunch : 1;
		_32 bNonExclusive : 1;
		_32 bOneLaunch : 1;  // not same as exclusive, can launch in parallel with other things.
		_32 bRestart : 1; // if it stops, respawn it.
		_32 bDestroy : 1; // set if destroyed, but launched...
		_32 bButton : 1; // if it's created as a control instead of via common load
		_32 bCaptureOutput : 1;
		_32 bDisallowedRun : 1; // system mismatched
		_32 bAllowedRun : 1;
		_32 bStarting : 1; // still working on thinking about starting this...
		_32 bBackground : 1;
		_32 bHideCanvas : 1;
	} flags;
	_32 last_lauch_time;
   _32 launch_count;
   _32 launch_width, launch_height;

	char pName[256], pTask[256], pPath[256];
	char **pArgs;
	//CDATA color, textcolor;
	//char pImage[256];
	PLIST spawns;
	struct menu_button_tag *button;
	CDATA highlight_normal_color;  // kept when button is loaded from button normal background color
	CDATA highlight_color;
	PLIST allowed_run_on;
   PTHREAD waiting_thread;
} LOAD_TASK, *PLOAD_TASK;


PLOAD_TASK CPROC CreateTask( struct menu_button_tag * );
void DestroyTask( PLOAD_TASK *ppTask );

//void CPROC RunATask( PTRSZVAL psv );
//void KillSpawnedPrograms( void );
int LaunchAutoTasks( int bCaller );



#endif
