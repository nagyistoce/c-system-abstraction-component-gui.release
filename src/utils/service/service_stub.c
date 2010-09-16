#include <stdhdrs.h>

static {
   CTEXTSTR next_service_name;
} local_service_info;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

int InitService()
{
   return 1;
}

void ControlHandler( DWORD request )
{
   switch(request) 
   { 
      case SERVICE_CONTROL_STOP: 
         //WriteToLog("Monitoring stopped.");

         ServiceStatus.dwWin32ExitCode = 0; 
         ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
         SetServiceStatus (hStatus, &ServiceStatus);
         return; 
 
      case SERVICE_CONTROL_SHUTDOWN: 
         //WriteToLog("Monitoring stopped.");

         ServiceStatus.dwWin32ExitCode = 0; 
         ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
         SetServiceStatus (hStatus, &ServiceStatus);
         return; 

	case SERVICE_CONTROL_PAUSE:
		break;
	//case SERVICE_CONTROL_RESUME:
      default:
         break;
    } 
 
    // Report current status
    SetServiceStatus (hStatus, &ServiceStatus);
 
    return; 
}


void APIENTRY ServiceMain( _32 argc, char **argv )
{

   int error; 
 
   ServiceStatus.dwServiceType = 
      SERVICE_WIN32; 
   ServiceStatus.dwCurrentState = 
      SERVICE_START_PENDING; 
   ServiceStatus.dwControlsAccepted   =  
      SERVICE_ACCEPT_STOP | 
		SERVICE_ACCEPT_SHUTDOWN;

   ServiceStatus.dwWin32ExitCode = 0; 
   ServiceStatus.dwServiceSpecificExitCode = 0; 
   ServiceStatus.dwCheckPoint = 0; 
   ServiceStatus.dwWaitHint = 0; 
 
   hStatus = RegisterServiceCtrlHandler(
      next_service_name,
      (LPHANDLER_FUNCTION)ControlHandler); 
   if (hStatus == (SERVICE_STATUS_HANDLE)0) 
   { 
      // Registering Control Handler failed
      return; 
	}

   // Initialize Service 
   error = 0;//InitService();
   if (error) 
   {
      // Initialization failed
      ServiceStatus.dwCurrentState = 
         SERVICE_STOPPED; 
      ServiceStatus.dwWin32ExitCode = -1; 
      SetServiceStatus(hStatus, &ServiceStatus); 
      return; 
   } 
   // We report the running status to SCM. 
   ServiceStatus.dwCurrentState = SERVICE_RUNNING;
   SetServiceStatus (hStatus, &ServiceStatus);
 
   //MEMORYSTATUS memory;
   // The worker loop of a service
   while (ServiceStatus.dwCurrentState == 
          SERVICE_RUNNING)
	{
		WakeableSleep( 100000 );
   }
   return; 
}


void SetupService( CTEXTSTR name )
{
	SERVICE_TABLE_ENTRY ServiceTable[2];
   next_service_name = name;
	ServiceTable[0].lpServiceName = name;
	ServiceTable[0].lpServiceProc = ServiceMain;
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
   StartServiceCtrlDispatcher( ServiceTable );
}
