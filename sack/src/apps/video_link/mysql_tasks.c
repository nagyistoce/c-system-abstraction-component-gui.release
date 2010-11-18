#include <stdhdrs.h>
#include <system.h>

struct task_tracker {
	PTASK_INFO task;
	CTEXTSTR program;
	CTEXTSTR path;
	CTEXTSTR args;
	CTEXTSTR run_on;
   CTEXTSTR run_against;
};

static struct local_task_info
{
	PLIST sql_tasks;
} local_task;
#define l local_task;

PRELOAD( LoadTasks )
{
	PCONFIGHANDLER pch;

	pch = CreateConfigurationHandler();

	AddConfigurationMethod( pch, "Site %m", begin_site );
	AddConfigurationMethod( pch, "Serves MySQL %b", is_mysql_server );
	AddConfigurationMethod( pch, "MySQL Remote %m", begin_site );


	for( SQLRecordQueryf( NULL, NULL, &results, NULL, "select program,path,args,run_on,run_against from link_task_configuration where task_type='SQL'" );
		 results;
		  FetchSQLRecord( NULL, &results ) )
	{
		new_task = New( struct task_tracker );
		new_task->task = NULL;
      new_task->program = StrDup( results[0] );
      new_task->path = StrDup( results[1] );
		new_task->args = StrDup( results[2] );
      new_task->run_on = StrDup( results[3] );
		new_task->run_against = StrDup( results[4] );
      AddLink( &l.sql_tasks, new_task );
	}

	{
		TEXTCHAR tmp[256];
      FutGetPrivateProfileInt( "config", "Bdata Local Server", "172.17.2.153", tmp, sizeof( tmp ), "vserver.ini" );
		l.local_hall_bdata_service = StrDup( tmp );
      l.bdata_port = FutGetPrivateProfileInt( "config", "Bdata Service Port", 6594, tmp, sizeof( tmp ), "vserver.ini" );
	}

}

void WriteProxyConfig( CTEXTSTR type, CTEXTSTR host, int port )
{
	FILE *output;
	TEXTCHAR tmp_name[256];
   snprintf( tmp_name, "%s/proxy_%s.conf", GetProgramPath(), type );
	output = fopen( tmp_name, "wt" );
	fprintf( "Proxy Service: %d %s:%d\n", port, host, port );
   fclose( output );
}


void SetSQLConfig( void )
{
	CTEXTSTR me = GetSystemName();
	struct task_tracker *task;
   INDEX idx;
	LIST_FORALL( l.tasks, idx, struct task_tracker *, task )
	{
		if( StrCaseCmp( task->run_on, me ) == 0 )
		{
			if( task->task )
			{

			}
		}
	}
}

void CPROC BDataEnded( PTRSZVAL psv, PTASK_INFO task )
{
   l.bdata_task = NULL;
}

void CPROC BDataOutput( PTRSZVAL psv, PTASK_INFO task, CTEXTSTR buffer, _32 size )
{
   lprintf( "%s", buffer );
}

void RestartBData( void )
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
   l.bdata_task = LaunchPeerProgram( "proxy_bdata.exe", GetProgramPath(), NULL, BdataOutput, BdataEnded, 0 );
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
   StopProgram( bdata_task );
}
