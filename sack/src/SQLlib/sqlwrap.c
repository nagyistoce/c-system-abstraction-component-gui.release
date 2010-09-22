#include <sack_types.h>
#include <pssql.h>

SQL_NAMESPACE

int DoSQLCommandf( CTEXTSTR fmt, ... )
{
	int result;
	PTEXT cmd;
	PVARTEXT pvt = VarTextCreate();
	va_list args;
	va_start( args, fmt );
	vvtprintf( pvt, fmt, args );
	cmd = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	result = DoSQLCommand( GetText( cmd ) );
	LineRelease( cmd );
	return result;
}

int DoSQLQueryf( CTEXTSTR *result, CTEXTSTR fmt, ... )
{
	int result_code;
	PTEXT cmd;
	PVARTEXT pvt = VarTextCreate();
	va_list args;
	va_start( args, fmt );
	vvtprintf( pvt, fmt, args );
	cmd = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	result_code = DoSQLQuery( (CTEXTSTR)GetText( cmd ), result );
	LineRelease( cmd );
	return result_code;
}

int DoSQLRecordQueryf( int *nResults, CTEXTSTR **result, CTEXTSTR **fields, CTEXTSTR fmt, ... )
{
	int result_code;
	PTEXT cmd;
	PVARTEXT pvt = VarTextCreate();
	va_list args;
	va_start( args, fmt );
	vvtprintf( pvt, fmt, args );
	cmd = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	result_code = SQLRecordQuery( NULL, GetText( cmd ), nResults, result, fields );
	LineRelease( cmd );
	return result_code;
}

int SQLCommandf( PODBC odbc, CTEXTSTR fmt, ... )
{
	int result;
	PTEXT cmd;
	PVARTEXT pvt = VarTextCreate();
	va_list args;
	va_start( args, fmt );
	vvtprintf( pvt, fmt, args );
	cmd = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	result = SQLCommand( odbc, GetText( cmd ) );
	LineRelease( cmd );
	return result;
}

int SQLQueryf( PODBC odbc, CTEXTSTR *result, CTEXTSTR fmt, ... )
{
	int result_code;
	PTEXT cmd;
	PVARTEXT pvt = VarTextCreate();
	va_list args;
	va_start( args, fmt );
	vvtprintf( pvt, fmt, args );
	cmd = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	result_code = SQLQuery( odbc, (CTEXTSTR)GetText( cmd ), result );
	LineRelease( cmd );
	return result_code;
}

int SQLRecordQueryf( PODBC odbc, int *nResults, CTEXTSTR **result, CTEXTSTR **fields, CTEXTSTR fmt, ... )
{
	int result_code;
	PTEXT cmd;
	PVARTEXT pvt = VarTextCreate();
	va_list args;
	va_start( args, fmt );
	vvtprintf( pvt, fmt, args );
	cmd = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	result_code = SQLRecordQuery( odbc, GetText( cmd ), nResults, result,fields );
	LineRelease( cmd );
	return result_code;
}

SQL_NAMESPACE_END
