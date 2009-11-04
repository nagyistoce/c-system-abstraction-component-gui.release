#include <stdhdrs.h>
#include <stdio.h>
#include <pssql.h>
#include <sharemem.h>

void Usage( int bFull )
{
	if( bFull )
		fprintf( stderr, "usage: %s [DSN or Sqlite db file]\n", GetProgramName() );

	fprintf( stderr, WIDE("Command must start with '?', '!' or '='.\n") );
	fprintf( stderr, WIDE("?<query> results go to the current output, or the screen if non specified before\n") );
	fprintf( stderr, WIDE("!<command> - issues command to \n") );
	fprintf( stderr, WIDE("=<table>@<DSN><,DSN>,...> setup output to these DSN's for next command or query result\n") );
}

void CPROC ShowSQLStates( CTEXTSTR message )
{
   printf( "%s\n", message );
}

int main( int argc, char **argv )
{
   PVARTEXT pvt_cmd;
	TEXTCHAR readbuf[4096];
	TEXTCHAR *buf;
	int offset = 0;
   PODBC default_odbc = NULL;
	CTEXTSTR select_into;
	PLIST output = NULL;
	PLIST outputs = NULL;
   SQLSetFeedbackHandler( ShowSQLStates );
	//SetAllocateDebug( TRUE );
	//SetAllocateLogging( TRUE );
	if( argc < 2 )
		Usage( 1 );
	else
	{
		default_odbc = ConnectToDatabase( argv[1] );
	}
	SetHeapUnit( 4096 * 1024 ); // 4 megs expansion if needed...
	pvt_cmd = VarTextCreateExx( 10000, 50000 );


	while( (buf = readbuf), fgets( readbuf + offset
					, sizeof( readbuf ) - offset
					, stdin ) )
	{
		CTEXTSTR *result = NULL;
		int len;
		while( buf[0] == ' ' || buf[0] == '\t' )
         buf++;
		len = strlen( buf );
		if( buf[0] == '#' )
			continue;
		if( buf[len-1] == '\n' )
		{
			len--;
			buf[len] = 0;
		}

		if( !buf[0] )
			continue;

		if( buf[len-1] == '\\' )
		{
			vtprintf( pvt_cmd, "%s", buf );
         offset = 0;
			//offset = (len - 1); // read over the slash
			continue;
		}
		else
		{
			vtprintf( pvt_cmd, "%s", buf );
			offset = 0;
		}

      buf = GetText( VarTextPeek( pvt_cmd ) );

		if( buf[0] == '?' )
		{
			int fields;
			CTEXTSTR *columns;
			TEXTSTR *_columns;
			PVARTEXT pvt = NULL;
			if( output )
			{
				if( !select_into || !select_into[0] )
				{
					printf( "Table name was invalid to insert into on the destination side...\'%s\'", select_into );
               VarTextEmpty( pvt_cmd );
					continue;
				}
				pvt = VarTextCreateExx( 10000, 50000 );
			}
			if( SQLRecordQuery( default_odbc, buf +1, &fields, &result, &columns ) )
			{
				int count = 0;
				int first = 1;
				_columns = NewArray( TEXTSTR, fields );
				{
					int n;
					for( n = 0; n < fields; n++ )
					{
						_columns[n] = StrDup( columns[n] );
						if( !pvt )
							fprintf( stdout, "%s%s", n?",":"", columns[n] );
					}
				}
				if( !pvt )
               fprintf( stdout, "\n" );
				for( ; result; FetchSQLRecord( default_odbc, &result ) )
				{
					if( pvt && first )
					{
						vtprintf( pvt, "insert ignore into `%s` (", select_into );
						{
							int first = 1;
							int n;
							for( n = 0; n < fields; n++ )
							{
								vtprintf( pvt, "%s`%s`", first?"":",", _columns[n] );
								first = 0;
							}
						}
						vtprintf( pvt, ") values " );
					}
					if( pvt )
					{
						vtprintf( pvt, "%s(", first?"":"," );
						{
							int first = 1; // private first, sorry :) parse that, Visual studio can.
							int n;
							for( n = 0; n < fields; n++ )
							{
								TEXTSTR tmp;
								vtprintf( pvt, "%s%s%s%s"
									, first?"":","
									, result[n]?"'":""
									, result[n]?(tmp=EscapeString( result[n])):((tmp=NULL),"NULL")
									, result[n]?"'":"" );
								Release( tmp );
								first = 0;
							}
						}
						vtprintf( pvt, ")" );
					}
					else
					{
						int n;
						int first = 1;
						for( n = 0; n < fields; n++ )
						{

							fprintf( stdout, WIDE("%s%s"), first?"":",",result[n]?result[n]:"NULL" );
							first = 0;
						}
						fprintf( stdout, WIDE("\n") );
					}
					first = 0;
					count++;
					if( ( VarTextLength( pvt ) ) > 100000 )
					{
						PTEXT cmd;
						first = 1; // reset first to rebuild the beginning of the insert.
						printf( "Flushing at 100k characters...%d records\n", count );
						if( pvt )
						{
							cmd = VarTextGet( pvt );
							if( cmd )
							{
								INDEX idx;
								PODBC odbc;
								LIST_FORALL( output, idx, PODBC, odbc )
								{
									if( !SQLCommand( odbc, GetText( cmd ) ) )
										printf( "Failed command to:%s\n", (CTEXTSTR)GetLink( &outputs, idx ) );
								}
								LineRelease( cmd );
							}
						}
					}
				}
				{
					int n;
					for( n = 0; n < fields; n++ )
					{
						Release( _columns[n] );
					}
				}
				Release( _columns );
			}
			else
			{
				CTEXTSTR result;
				FetchSQLError( default_odbc, &result );
				if( result )
					fprintf( stderr, WIDE("%s\n"), result );
				else
               fprintf( stderr, "Failed, no error result.\n" );
			}
			if( pvt )
			{
				PTEXT cmd;
				printf( "Flushing command\n" );
				cmd = VarTextGet( pvt );
				if( cmd )
				{
					INDEX idx;
					PODBC odbc;
					LIST_FORALL( output, idx, PODBC, odbc )
					{
						printf( "Flushing command to %s\n", (CTEXTSTR)GetLink( &outputs, idx ) );
						if( !SQLCommand( odbc, GetText( cmd ) ) )
						{
							CTEXTSTR error;
                     FetchSQLError( odbc, &error );
							printf( "%s\n", error );
						}
					}
					LineRelease( cmd );
				}
			}

			if( pvt )
			{
				VarTextDestroy( &pvt );
			}
			if( output )
			{
				INDEX idx;
				PODBC odbc;
				LIST_FORALL( output, idx, PODBC, odbc )
				{
					TEXTSTR output_name = (TEXTSTR)GetLink( &outputs, idx );
					Release( output_name );
					CloseDatabase( odbc );
				}
				DeleteList( &outputs );
				DeleteList( &output );
			}
		}
		else if( buf[0] == '!' )
		{
			if( output )
			{
				PODBC odbc;
				INDEX idx;
				LIST_FORALL( output, idx, PODBC, odbc )
				{
					printf( "Issue command to: %s\n", (CTEXTSTR)GetLink( &outputs, idx ) );
					if( !SQLCommand( odbc, buf +1 ) )
					{
						CTEXTSTR result;
						FetchSQLError( odbc, &result );
						fprintf( stderr, WIDE("%s\n"), result );
					}
				}
				if( output )
				{
					INDEX idx;
					PODBC odbc;
					LIST_FORALL( output, idx, PODBC, odbc )
					{
						TEXTSTR output_name = (TEXTSTR)GetLink( &outputs, idx );
						Release( output_name );
						CloseDatabase( odbc );
					}
					DeleteList( &outputs );
					DeleteList( &output );
				}
				printf( WIDE("Ok.\n") );
			}
			else
			{
				if( !SQLCommand( default_odbc, buf +1 ) )
				{
					CTEXTSTR result;
					FetchSQLError( default_odbc, &result );
					fprintf( stderr, WIDE("%s\n"), result );
				}
				else
					fprintf( stderr, WIDE("Ok.\n") );
			}
		}
		else if( buf[0] == '=' )
		{
			TEXTCHAR *start = buf + 1;
			TEXTCHAR *end;
			end = strchr( start, '@' );
			if( !end )
			{
				printf( "Must specify table@dsn,dsn,dsn... no @ found on :%s\n", buf );
			}
			else
			{
				end[0] = 0;
				end++;
				select_into = StrDup( start );
				while( ( start = end ) != 0 )
				{
					PODBC odbc;
					end = strchr( start, ',' );
					if( end )
					{
						end[0] = 0;
						end++;
					}

					AddLink( &output, odbc = ConnectToDatabase( start ) );
					if( IsSQLOpen( odbc ) )
					{
						printf( "Connected to: %s\n", start );
						AddLink( &outputs, StrDup( start ) );
					}
					else
					{
                  CTEXTSTR error;
                  FetchSQLError( odbc, &error );
						printf( "Failed connect to:%s [%s]\n", start, error );
					}
				}

			}
		}
		else
		{
			fprintf( stderr, WIDE("%s"), buf );
         Usage( 0 );
		}
		VarTextEmpty( pvt_cmd );
	}
	{
		//_32 a,b,c,d;
		//DebugDumpMem();
	}
	return 0;
}


