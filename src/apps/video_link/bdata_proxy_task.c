#include <stdhdrs.h>
#include <system.h>
#include <sqlgetoption.h>


static struct local_task_info
{

	CTEXTSTR local_hall_bdata_service;
   int bdata_port;
	CTEXTSTR current_bdata_host;

   PTASK_INFO bdata_task;

} local_task;
#define l local_task

PRELOAD( LoadBdataTasks )
{
	{
		TEXTCHAR tmp[256];
      SACK_GetPrivateProfileInt( "config", "Bdata Local Server", "172.17.2.153", tmp, sizeof( tmp ), "vserver.ini" );
		l.local_hall_bdata_service = StrDup( tmp );
      l.bdata_port = SACK_GetPrivateProfileInt( "config", "Bdata Service Port", 6594, tmp, sizeof( tmp ), "vserver.ini" );
	}

}

static void WriteProxyConfig( CTEXTSTR type, CTEXTSTR host, int port )
{
	FILE *output;
	TEXTCHAR tmp_name[256];
   snprintf( tmp_name, sizeof( tmp_name ), "%s/proxy_%s.conf", GetProgramPath(), type );
	output = fopen( tmp_name, "wt" );
	fprintf( output, "Proxy Service: %d %s:%d\n", port, host, port );
   fclose( output );
}


static void CPROC BDataEnded( PTRSZVAL psv, PTASK_INFO task )
{
   l.bdata_task = NULL;
}

static void CPROC BDataOutput( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 size )
{
   lprintf( "%s", buffer );
}

static void RestartBData( void )
{
	if( l.bdata_task )
	{
		StopProgram( l.bdata_task );
	}
	if( l.bdata_task )
	{
		lprintf( "Did not stop bdata task...wait a second..." );
		while( l.bdata_task )
		{
			WakeableSleep( 1000 );
         lprintf( "Doing terminate..." );
         TerminateProgram( l.bdata_task );
		}
	}
   l.bdata_task = LaunchPeerProgram( "proxy_bdata.exe", GetProgramPath(), NULL, BDataOutput, BDataEnded, 0 );
}

void UpdateBdataHost( CTEXTSTR new_host )
{
	if( StrCmp( new_host, l.current_bdata_host ) )
	{
		l.current_bdata_host = new_host;
		if( StrCaseCmp( new_host, GetSystemName() ) == 0 )
		{
			WriteProxyConfig( "bdata", l.local_hall_bdata_service, l.bdata_port );
         RestartBData();
		}
	}
}

ATEXIT( closebdata )
{
   StopProgram( l.bdata_task );
}
