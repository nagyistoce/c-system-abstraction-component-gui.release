#define DO_LOGGING
#include <logging.h>
#include <sqlstub.h>
#include <stdio.h>
#include <string.h>

int fnCheckData( void )
{
    char szCmd[512];
    CTEXTSTR error = NULL, result = NULL;
    char y[] = {"NONE"};
    int x;

    lprintf( "Test Sting = %s", y);
    snprintf( szCmd, sizeof(szCmd), "SELECT announcement FROM link_hall_state" );
    lprintf( " szCmd = %s", szCmd );
    if( !DoSQLQuery( szCmd, &result ) )
    {
	GetSQLError( &error );
	lprintf( "SQL Error: %s", error );
	return -1;
    }
    lprintf("Result = %s, Test = %s", result, y );
    x = strncmp( y, result, 4 );
    lprintf( "X = %d", x );
    return x;
}

int fnWriteSoundFileName( char * buf )
{
    char szSqlCmd[512];
    
    snprintf(szSqlCmd, sizeof(szSqlCmd), "UPDATE link_hall_state SET announcement =\"%s\" WHERE master_ready = 1 OR delegate_ready = 1 OR participating = 1", buf );
#if 0
    if( !fnCheckData() )
    {
    	DoSQLCommand( szSqlCmd );
        return 1;
    }
    return 0;
#endif
    DoSQLCommand( szSqlCmd );
    return 1;
}


int main( int argc, char **argv )
{
    if( argc < 1)
    {
	lprintf( WIDE( "No sound ...ERROR"));
	return 0;
    }
    if( !fnWriteSoundFileName( argv[1] ) )
    {
	lprintf( "could not write sound file name to database" );
	return 0;
    }
    lprintf( "%s has been wriiten to the datatbase", argv[1] );

    return 1; 

}
