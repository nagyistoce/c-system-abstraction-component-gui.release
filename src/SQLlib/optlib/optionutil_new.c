#ifndef GETOPTION_SOURCE
#define GETOPTION_SOURCE
#endif

#include <stdhdrs.h>
#include <sharemem.h>
#include <pssql.h>
#include <sqlgetoption.h>


#include "../sqlstruc.h"
#include "makeopts.mysql"
SACK_OPTION_NAMESPACE

#include "optlib.h"

#define og sack_global_option_data
extern struct sack_option_global_tag og;

#define ENUMOPT_FLAG_HAS_VALUE 1
#define ENUMOPT_FLAG_HAS_CHILDREN 2


void NewEnumOptions( PODBC odbc
												  , INDEX parent
												  , int (CPROC *Process)(PTRSZVAL psv
																				, CTEXTSTR name
																				, _32 ID
																				, int flags )
												  , PTRSZVAL psvUser )
{
	char query[256];
   static PODBC pending;
	int first_result, popodbc;
	CTEXTSTR *results = NULL;
	if( !odbc )
		odbc = pending;
	if( !odbc )
		return;
   pending = odbc;
	// any existing query needs to be saved...
	InitMachine();
   SetSQLLoggingDisable( odbc, TRUE );
	PushSQLQueryEx( odbc ); // any subqueries will of course clean themselves up.
	snprintf( query
			  , sizeof( query )
			  , "select option_id,n.name "
				"from "OPTION_MAP" as m "
				"join "OPTION_NAME" as n on n.name_id=m.name_id "
				"where parent_option_id=%ld "
            "order by n.name"
			  , parent );
	popodbc = 0;
	for( first_result = SQLRecordQuery( odbc, query, NULL, &results, NULL );
		 results;
		  FetchSQLRecord( odbc, &results ) )
	{
		CTEXTSTR optname;
		INDEX node = atol( results[0] );
		popodbc = 1;
		optname = results[1];

      // psv is a pointer to args in some cases...
      //lprintf( "Enum %s %ld", optname, node );
		//ReadFromNameTable( name, WIDE(""OPTION_NAME""), WIDE("name_id"), &result);
		if( !Process( psvUser, optname, node
						, ((node!=INVALID_INDEX)?1:0)
						) )
		{
			break;
		}
		//lprintf( WIDE("reget: %s"), query );
	}
	PopODBCEx( odbc );
   SetSQLLoggingDisable( odbc, FALSE );
	pending = NULL;

}


struct complex_args
{
	INDEX iNewRoot;
   INDEX iOldRoot;
   PODBC odbc;
};

static int CPROC CopyRoot( PTRSZVAL psvArgs, CTEXTSTR name, _32 ID, int flags )
{
   struct complex_args *args = (struct complex_args*)psvArgs;
	INDEX iCopy = GetOptionIndexEx( args->iNewRoot, NULL, name, NULL, TRUE DBG_SRC );
	NewDuplicateValue( args->odbc, ID, iCopy );

	{
		struct complex_args c_args;
		c_args.iNewRoot = iCopy;
		c_args.odbc = args->odbc;
		EnumOptions( ID, CopyRoot, (PTRSZVAL)&c_args );
	}
   return TRUE;
}

void NewDuplicateOption( PODBC odbc, INDEX iRoot, CTEXTSTR pNewName )
{
	INDEX iParent;
	CTEXTSTR result = NULL;
	INDEX iNewName;
	SetSQLLoggingDisable( odbc, TRUE );
	if( SQLQueryf( odbc, &result, "select parent_option_id from " OPTION_MAP " where option_id=%ld", iRoot ) && result )
	{
		struct complex_args args;
		iParent = atoi( result );
		SQLEndQuery( odbc );
		iNewName = GetOptionIndexEx( iParent, NULL, pNewName, NULL, TRUE DBG_SRC );
		args.iNewRoot = iNewName;
		args.odbc = odbc;
		NewEnumOptions( args.odbc, iRoot, CopyRoot, (PTRSZVAL)&args );
	}
	SetSQLLoggingDisable( odbc, FALSE );
}


static void NewFixOrphanedBranches( void )
{
	PLIST options = CreateList();
	CTEXTSTR *result = NULL;
	CTEXTSTR result2 = NULL;
	SQLQuery( og.Option, "select count(*) from " OPTION_MAP, &result2 );
   // expand the options list to max extent real quickk....
	SetLink( &options, atoi( result2 ) + 1, 0 );
	for( SQLRecordQuery( og.Option, "select option_id,parent_option_id from "OPTION_MAP, NULL, &result, NULL );
		  result;
		  FetchSQLRecord( og.Option, &result ) )
	{
		INDEX node_id, parent_option_id;
		node_id = atol( result[0] );
		parent_option_id = atol( result[1] );
		//sscanf( result, WIDE("%ld,%ld"), &node_id, &parent_option_id );
      SetLink( &options, node_id, (POINTER)(parent_option_id+1) );
	}
	{
		INDEX idx;
      int deleted;
		_32 parent;
		do
		{
			deleted = 0;
			LIST_FORALL( options, idx, _32, parent )
			{
				//lprintf( WIDE("parent node is...%ld"), parent );
			// node ID parent of 0 or -1 is a parent node...
         // so nodeID+1 of 0 is 1
				if( (parent > 1) && !GetLink( &options, parent-1 ) )
				{
					deleted = 1;
					//lprintf( WIDE("node %ld has parent id %ld which does not exist."), idx, parent-1 );
					SetLink( &options, idx, NULL );
					SQLCommandf( og.Option, "delete from "OPTION_MAP " where option_id=%ld", idx );
				}
			}
		}while( deleted );
	}
	DeleteList( &options );
}


void NewDeleteOption( PODBC odbc, INDEX iRoot )
{
	SetSQLLoggingDisable( odbc, TRUE );
	SQLCommandf( odbc, "delete from "OPTION_MAP " where option_id=%ld", iRoot );
	SQLCommandf( odbc, "delete from "OPTION_VALUES " where option_id=%ld", iRoot );
	SQLCommandf( odbc, "delete from "OPTION_BLOBS " where option_id=%ld", iRoot );
   NewFixOrphanedBranches();
	SetSQLLoggingDisable( odbc, FALSE );
}

SACK_OPTION_NAMESPACE_END

//--------------------------------------------------------------------------
//
// $Log: optionutil.c,v $
// Revision 1.3  2005/03/21 19:37:08  jim
// Update to reflect newest database standardized changes
//
// Revision 1.2  2004/07/08 18:23:26  jim
// roughly works now.
//
// Revision 1.1  2004/04/15 00:12:56  jim
// Checkpoint
//
//

