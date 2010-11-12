#ifndef GETOPTION_SOURCE
#define GETOPTION_SOURCE
#endif

#include <stdhdrs.h>
#include <sharemem.h>
#include <pssql.h>
#include <sqlgetoption.h>


#include "../sqlstruc.h"

SACK_OPTION_NAMESPACE

#include "optlib.h"

#define og sack_global_option_data
extern struct sack_option_global_tag og;

#define ENUMOPT_FLAG_HAS_VALUE 1
#define ENUMOPT_FLAG_HAS_CHILDREN 2


SQLGETOPTION_PROC( void, EnumOptionsEx )( PODBC odbc, INDEX parent
					 , int (CPROC *Process)(PTRSZVAL psv, CTEXTSTR name, _32 ID, int flags )
											  , PTRSZVAL psvUser )
{
	POPTION_TREE tree = GetOptionTreeEx( odbc );
	if( tree->flags.bNewVersion )
	{
		NewEnumOptions( og.Option, parent, Process, psvUser );
	}
	else
	{
		char query[256];
		int first_result, popodbc;
		CTEXTSTR result = NULL;
		// any existing query needs to be saved...
		InitMachine();
		PushSQLQueryEx( og.Option ); // any subqueries will of course clean themselves up.
		snprintf( query
				  , sizeof( query )
				  , "select node_id,m.name_id,value_id,n.name"
					" from option_map as m"
					" join option_name as n on n.name_id=m.name_id"
					" where parent_node_id=%ld"
               " order by option_name"
				  , parent );
		popodbc = 0;
		for( first_result = SQLQuery( og.Option, query, &result );
			 result;
			  FetchSQLResult( og.Option, &result ) )
		{
			CTEXTSTR optname;
			INDEX node, name, value;
			popodbc = 1;
			sscanf( result, WIDE("%lu,%lu,%lu"), &node, &name, &value );
			optname = strrchr( result, ',' );
			if( optname )
				optname++;
			//ReadFromNameTable( name, WIDE("option_name"), WIDE("name_id"), &result);
			if( !Process( psvUser, (char*)optname, node
							, ((value!=INVALID_INDEX)?1:0)
							) )
			{
				break;
			}
			//lprintf( WIDE("reget: %s"), query );
		}
		PopODBCEx( og.Option );
	}
}

SQLGETOPTION_PROC( void, EnumOptions )( INDEX parent
					 , int (CPROC *Process)(PTRSZVAL psv, CTEXTSTR name, _32 ID, int flags )
											  , PTRSZVAL psvUser )
{
   EnumOptionsEx( og.Option, parent, Process, psvUser );
}
struct copy_data {
	INDEX iNewName;
   POPTION_TREE tree;
};

static int CPROC CopyRoot( PTRSZVAL iNewRoot, CTEXTSTR name, _32 ID, int flags )
{
	struct copy_data *copydata = (struct copy_data *)iNewRoot;
   struct copy_data newcopy;
	// iNewRoot is at its source an INDEX
	INDEX iCopy = GetOptionIndexEx( copydata->iNewName, NULL, name, NULL, TRUE DBG_SRC );
	INDEX iValue = GetOptionValueIndex( ID );
	newcopy.tree = copydata->tree;
   newcopy.iNewName = iCopy;
   if( iValue != INVALID_INDEX )
		SetOptionValueEx( copydata->tree, iCopy, DuplicateValue( iValue, iCopy ) );

	EnumOptions( ID, CopyRoot, (PTRSZVAL)&newcopy );
   return TRUE;
}

SQLGETOPTION_PROC( void, DuplicateOptionEx )( PODBC odbc, INDEX iRoot, CTEXTSTR pNewName )
{
	INDEX iParent;
   CTEXTSTR result = NULL;
	INDEX iNewName;
	struct copy_data copydata;
	POPTION_TREE tree = GetOptionTreeEx( odbc );
	copydata.tree = tree;
	if( tree->flags.bNewVersion )
	{
		NewDuplicateOption( og.Option, iRoot, pNewName );
		return;
	}
	if( SQLQueryf( og.Option, &result, WIDE("select parent_node_id from option_map where node_id=%ld"), iRoot ) && result )
	{
		iParent = atoi( result );
		iNewName = GetOptionIndexEx( iParent, NULL, pNewName, NULL, TRUE DBG_SRC );
		copydata.iNewName = iNewName;
		EnumOptions( iRoot, CopyRoot, (PTRSZVAL)&copydata );
	}
}

SQLGETOPTION_PROC( void, DuplicateOption )( INDEX iRoot, CTEXTSTR pNewName )
{
   DuplicateOptionEx( og.Option, iRoot, pNewName );
}

static void FixOrphanedBranches( void )
{
	PLIST options = CreateList();
	CTEXTSTR *result = NULL;
	CTEXTSTR result2 = NULL;
	SQLQuery( og.Option, WIDE("select count(*) from option_map"), &result2 );
   // expand the options list to max extent real quickk....
	SetLink( &options, atoi( result2 ) + 1, 0 );
	for( SQLRecordQuery( og.Option, WIDE("select node_id,parent_node_id from option_map"), NULL, &result, NULL );
		  result;
		  GetSQLRecord( &result ) )
	{
		INDEX node_id, parent_node_id;
		node_id = atol( result[0] );
		parent_node_id = atol( result[1] );
		//sscanf( result, WIDE("%ld,%ld"), &node_id, &parent_node_id );
      SetLink( &options, node_id, (POINTER)(parent_node_id+1) );
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
				lprintf( WIDE("parent node is...%ld"), parent );
				// node ID parent of 0 or -1 is a parent node...
				// so nodeID+1 of 0 is 1
				if( (parent > 1) && !GetLink( &options, parent-1 ) )
				{
					deleted = 1;
					lprintf( WIDE("node %ld has parent id %ld which does not exist."), idx, parent-1 );
					SetLink( &options, idx, NULL );
					SQLCommandf( og.Option, WIDE("delete from option_map where node_id=%ld"), idx );
				}
			}
		}while( deleted );
	}
	DeleteList( &options );
}


SQLGETOPTION_PROC( void, DeleteOption )( INDEX iRoot )
{
	POPTION_TREE tree = GetOptionTreeEx( og.Option );
	if( tree->flags.bNewVersion )
	{
		NewDeleteOption( og.Option, iRoot );
		return;
	}
	SQLCommandf( og.Option, WIDE("delete from option_map where node_id=%ld"), iRoot );
   FixOrphanedBranches();
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

