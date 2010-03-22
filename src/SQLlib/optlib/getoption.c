#ifndef GETOPTION_SOURCE
#define GETOPTION_SOURCE
#endif
// we want access to GLOBAL from sqltub
#define SQLLIB_SOURCE
#include <stdhdrs.h>
#include <sack_types.h>
#include <deadstart.h>
#include <sharemem.h>
#include <filesys.h>
#include <system.h>
#include <network.h>
#include <controls.H> // INI prompt
#ifdef __WATCOMC__
#include <io.h> // unlink
#endif

#include <pssql.h>
#include <sqlgetoption.h>

#include "../sqlstruc.h"
// define this to show very verbose logging during creation and
// referencing of option tree...
//#define DETAILED_LOGGING

/*
 Dump Option table...
 SELECT oname2.name,oname.name,optionvalues.string,omap.*
 FROM `optionmap` as omap
 join optionname as oname on omap.name_id=oname.name_id
 left join optionvalues on omap.value_id=optionvalues.value_id
 left join optionmap as omap2 on omap2.node_id=omap.parent_node_id
 left join optionname as oname2 on omap2.name_id=oname2.name_id
*/
SQL_NAMESPACE
extern GLOBAL *global_sqlstub_data;
SQL_NAMESPACE_END


SACK_OPTION_NAMESPACE

#include "optlib.h"

typedef struct sack_option_global_tag OPTION_GLOBAL;

#ifdef _cut_sql_statments_

SQL table create is in 'mkopttabs.sql'

#endif
#define og sack_global_option_data
	OPTION_GLOBAL og;


//------------------------------------------------------------------------

//int mystrcmp( char *x, char *y )
//{
//   lprintf( WIDE("Is %s==%s?"), x, y );
//   return strcmp( x, y );
//}

//------------------------------------------------------------------------

#define MKSTR(n,...) #__VA_ARGS__
//char *tablestatements =
#include "makeopts.mysql"
;

void SetOptionDatabase( PODBC odbc )
{
   // maybe, if previously open with private database, close that connection
	og.Option = odbc;
	og.flags.bInited = FALSE;
   CreateOptionDatabase(); // make sure the option tables exist.
}


SQLGETOPTION_PROC( void, CreateOptionDatabaseEx )( PODBC odbc );

POPTION_TREE GetOptionTreeEx( PODBC odbc )
{
	POPTION_TREE node;
   INDEX idx;
#ifdef DETAILED_LOGGING
	lprintf( "Finding tree for %p", odbc );
#endif
	LIST_FORALL( og.trees, idx, struct sack_option_tree_family*, node )
	{
		if( node->odbc == odbc )
			return node;
	}
	node = New( struct sack_option_tree_family );
   // if it's a new optiontree, pass it to create...
	node->option_tree = CreateFamilyTree( (int(CPROC*)(PTRSZVAL,PTRSZVAL))StrCaseCmp, NULL );
	node->odbc = odbc;
   // default to the old version... allow other code to select new version.
   node->flags.bNewVersion = 0;
   node->flags.bCreated = 0;
	AddLink( &og.trees, node );
   CreateOptionDatabaseEx( odbc );
	return node;
}

PFAMILYTREE* GetOptionTree( PODBC odbc )
{
	POPTION_TREE node = GetOptionTreeEx( odbc );
   if( node )
		return &node->option_tree;
   return NULL;
}



SQLGETOPTION_PROC( void, CreateOptionDatabaseEx )( PODBC odbc )
{
	SetSQLLoggingDisable( odbc, TRUE );
	{
		PTABLE table;
		POPTION_TREE tree = GetOptionTreeEx( odbc );
		if( !tree->flags.bCreated )
		{
			if( !tree->flags.bNewVersion )
			{
				table = GetFieldsInSQLEx( option_exception, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( option_map, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( option_name, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( option_values, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MATCH );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( systems, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
			}
			else
			{
				table = GetFieldsInSQLEx( option2_exception, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( option2_map, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( option2_name, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( option2_values, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MATCH );
				DestroySQLTable( table );
				table = GetFieldsInSQLEx( option2_blobs, FALSE DBG_SRC );
				CheckODBCTable( odbc, table, CTO_MERGE );
				DestroySQLTable( table );
			}
         //SQLCommit( odbc );
         //SQLCommand( odbc, "COMMIT" );
			tree->flags.bCreated = 1;
		}
	}
}

void SetOptionDatabaseOption( PODBC odbc, int bNewVersion )
{
   POPTION_TREE node = GetOptionTreeEx( odbc );
	if( node )
	{
		node->flags.bCreated = FALSE;
		node->flags.bNewVersion = bNewVersion;
      CreateOptionDatabaseEx( odbc );
	}
}

SQLGETOPTION_PROC( void, CreateOptionDatabase )( void )
{
   if( !og.Option )
	{
#ifdef ROMBUILD
		TEXTCHAR buffer[256];
		CTEXTSTR loadpath = GetCurrentPath( buffer, sizeof( buffer ) );
#else
		CTEXTSTR loadpath = OSALOT_GetEnvironmentVariable( "MY_LOAD_PATH" );
      //lprintf( "Loadpath is %p", loadpath );
		if( !loadpath )
         loadpath = ".";
#endif
		if( strlen( global_sqlstub_data->Option.info.pDSN ) == 0 )
			global_sqlstub_data->Option.info.pDSN = "option.db";
		//lprintf( "connect to %s[%s]", global_sqlstub_data->Option.info.pDSN, loadpath );

		//if( !pathchr( global_sqlstub_data->Option.info.pDSN )
		//	&& strchr( global_sqlstub_data->Option.info.pDSN, '.' ))
		{
			size_t len;
			TEXTSTR out = NewArray( TEXTCHAR, len = ( strlen( loadpath ) + strlen( global_sqlstub_data->Option.info.pDSN ) + 2 ) );
			if( ( pathchr( global_sqlstub_data->Option.info.pDSN ) == global_sqlstub_data->Option.info.pDSN )
				|| global_sqlstub_data->Option.info.pDSN[1] == ':'
				|| !strchr( global_sqlstub_data->Option.info.pDSN, '.' ) )
			{
				snprintf( out, len, "%s", global_sqlstub_data->Option.info.pDSN );
			}
         else
				snprintf( out, len, "%s/%s", loadpath, global_sqlstub_data->Option.info.pDSN );
#ifdef DETAILED_LOGGING
			lprintf( "Connecting to default SQLite dabase?" );
#endif
			if( !og.Option )
			{
            //lprintf( "connect to %s", out );
				og.Option = ConnectToDatabase( out );
				og.Option->flags.bAutoTransact = 1;
				og.Option->last_command_tick = 0; // just to make sure

			}
		}
      //else
		//	og.Option = ConnectToDatabase( global_sqlstub_data->Option.info.pDSN );
		if( og.Option )
		{
         // make sure it's created as a tracked option thing...
			GetOptionTree( og.Option );
         // set to use new option tables.
			SetOptionDatabaseOption( og.Option, TRUE );
		}
#ifdef DETAILED_LOGGING
		lprintf( "Connected to default SQLite dabase?" );
#endif
		CreateOptionDatabaseEx( og.Option );
	}

}

void InitMachine( void )
{
   if( !og.flags.bInited )
	{
      _32 timeout;
      CreateOptionDatabase();
		// acutlaly init should be called always ....
      timeout = GetTickCount() + 1000;
		while( !IsSQLOpen( og.Option ) && ( timeout > GetTickCount() ) )
		{
			Sleep( 100 );
		}
		if( !IsSQLOpen(og.Option) )
		{
			lprintf( WIDE("Get Option init failed... no database...") );
         return;
		}
      // og.system = GetSYstemID( WIDE("SYSTEMNAME") );
      og.SystemID = 0;  // default - any system...
      og.flags.bInited = 1;
      {
#ifdef _WIN32
         WSADATA ws;
         WSAStartup( MAKEWORD(1,1), &ws );
#endif
         gethostname( og.SystemName, sizeof( og.SystemName ) );
			og.SystemID = SQLReadNameTable( og.Option, og.SystemName, WIDE("systems"), WIDE("system_id")  );
      }
   }
}

//------------------------------------------------------------------------

#define CreateName(o,n) SQLReadNameTable(o,n,OPTION_MAP,"name_id")

//------------------------------------------------------------------------

//#define OPTION_ROOT_VALUE INVALID_INDEX
#define OPTION_ROOT_VALUE 0

static INDEX GetOptionIndexExx( PODBC odbc, INDEX parent, const char *file, const char *pBranch, const char *pValue, int bCreate DBG_PASS )
//#define GetOptionIndex( f,b,v ) GetOptionIndexEx( OPTION_ROOT_VALUE, f, b, v, FALSE )
{
	POPTION_TREE tree = GetOptionTreeEx( odbc );
	if( tree->flags.bNewVersion )
		return NewGetOptionIndexExx( odbc, parent, file, pBranch, pValue, bCreate DBG_RELAY );
	{
		const char **start = NULL;
		char namebuf[256];
		char query[256];
		const char *p;
		static const char *_program = NULL;
		const char *program = NULL;
		static const char *_system = NULL;
		const char *system = NULL;
		CTEXTSTR *result = NULL;
		INDEX ID;
		//, IDName; // Name to lookup
		if( og.flags.bUseProgramDefault )
		{
			if( !_program )
				_program = GetProgramName();
			program = _program;
		}
		if( og.flags.bUseSystemDefault )
		{
			if( !_system )
				_system = GetSystemName();
			system = _system;
		}
		InitMachine();
		// resets the search/browse cursor... not empty...
		FamilyTreeReset( GetOptionTree( odbc ) );
		while( system || program || file || pBranch || pValue || start )
		{
#ifdef DETAILED_LOGGING
			lprintf( WIDE("Top of option loop") );
#endif
			if( !start || !(*start) )
			{
				if( program )
					start = &program;
				if( !start && system )
					start = &system;

				if( !start && file )
				{
#ifdef DETAILED_LOGGING
					lprintf( WIDE("Token parsing at FILE") );
#endif
					start = &file;
				}
				if( !start && pBranch )
				{
#ifdef DETAILED_LOGGING
					lprintf( WIDE("Token parsing at branch") );
#endif
					start = &pBranch;
				}
				if( !start && pValue )
				{
#ifdef DETAILED_LOGGING
					lprintf( WIDE("Token parsing at value") );
#endif
					start = &pValue;
				}
				if( !start || !(*start) ) continue;
			}
			p = pathchr( *start );
			if( p )
			{
				if( p-(*start) > 0 )
				{
					MemCpy( namebuf, (*start), p - (*start) );
					namebuf[p-(*start)] = 0;
				}
				else
				{
					(*start) = p + 1;
					continue;
				}
				(*start) = p + 1;
			}
			else
			{
				strncpy( namebuf, (*start), sizeof( namebuf )-1 );
				(*start) = NULL;
				start = NULL;
			}

			// remove references of 'here' during parsing.
			if( strcmp( namebuf, "." ) == 0 )
				continue;
#ifdef DETAILED_LOGGING
			lprintf( WIDE("First - check local cache for %s"), namebuf );
#endif
			{
				// return is UserData, assume I DO store this as an index.
				INDEX IDName = ((INDEX)(PTRSZVAL)FamilyTreeFindChild( *GetOptionTree( odbc ), (PTRSZVAL)namebuf )) - 1;
				if( IDName != INVALID_INDEX )
				{
#ifdef DETAILED_LOGGING
					lprintf( WIDE("Which is found, and new parent ID result...%d"), IDName );
#endif
					parent = IDName;
					continue;
				}
			}

			{
				INDEX IDName = SQLReadNameTable(odbc,namebuf,"option_name","name_id");

				PushSQLQueryEx(odbc);
				snprintf( query, sizeof( query )
						  , WIDE("select node_id,value_id from option_map where parent_node_id=%ld and name_id=%d")
						  , parent
						  , IDName );
				//lprintf( "doing %s", query );
				if( !SQLRecordQuery( odbc, query, NULL, &result, NULL ) || !result )
				{
					if( bCreate )
					{
						// this is the only place where ID must be set explicit...
						// otherwise our root node creation failes if said root is gone.
						//lprintf( "New entry... create it..." );
						snprintf( query, sizeof( query ), WIDE("Insert into option_map(`parent_node_id`,`name_id`) values (%ld,%lu)"), parent, IDName );
						if( SQLCommand( odbc, query ) )
						{
							ID = FetchLastInsertID( odbc, WIDE("option_map"), WIDE("node_id") );
						}
						else
						{
							CTEXTSTR error;
							FetchSQLError( odbc, &error );
#ifdef DETAILED_LOGGING
							lprintf( WIDE("Error inserting option: %s"), error );
#endif
						}
#ifdef DETAILED_LOGGING
						lprintf( WIDE("Created option root...") );
#endif
						parent = ID;
						//lprintf( WIDE("Adding new option to family tree... ") );
						FamilyTreeAddChild( GetOptionTree( odbc ), (POINTER)(ID+1), (PTRSZVAL)SaveText( namebuf ) );
						PopODBCEx( odbc );
						continue; // get out of this loop, continue outer.
					}
#ifdef DETAILED_LOGGING
					lprintf( WIDE("Option tree corrupt.  No option node_id=%ld"), ID );
#endif
					PopODBCEx( odbc );
					return INVALID_INDEX;
				}
				else
				{
#ifdef DETAILED_LOGGING
					lprintf( WIDE("found the node which has the name specified...") );
#endif
					parent = atol( result[0] );
					// might as well fetch the value ID associated here alsos.
					//if( result[1] )
					//	value = atoi( result[1] );
					//else
					//   value = INVALID_INDEX;
					//sscanf( result, WIDE("%lu"), &parent );
					FamilyTreeAddChild( GetOptionTree( odbc ), (POINTER)(parent+1), (PTRSZVAL)SaveText( namebuf ) );
				}
				PopODBCEx( odbc );
			}
		}
	}
	return parent;
}

INDEX GetOptionIndexEx( INDEX parent, const char *file, const char *pBranch, const char *pValue, int bCreate DBG_PASS )
{
	InitMachine();
   return GetOptionIndexExx( og.Option, parent, file, pBranch, pValue, bCreate DBG_RELAY );
}
//------------------------------------------------------------------------

INDEX GetSystemIndex( CTEXTSTR pSystemName )
{
   if( pSystemName )
      return ReadNameTable( pSystemName, WIDE("systems"), WIDE("system_id") );
   else
      return og.SystemID;
}

//------------------------------------------------------------------------

INDEX GetOptionValueIndexEx( PODBC odbc, INDEX ID )
{
	POPTION_TREE tree = GetOptionTreeEx( odbc );
	if( tree->flags.bNewVersion )
      return ID;
	{
		char query[256];
		CTEXTSTR result = NULL;
		INDEX IDValue = INVALID_INDEX;
		if( ID && ID!= INVALID_INDEX )
		{
			snprintf( query, sizeof( query ), WIDE("select value_id from option_map where node_id=%ld"), ID );
			//lprintf( WIDE("push get value index.") );
			PushSQLQueryEx( odbc );
			if( !SQLQuery( odbc, query, &result )
				|| !result )
			{
				lprintf( WIDE("Option tree corrupt.  No option node_id=%ld") );
				return INVALID_INDEX;
			}
			//lprintf( WIDE("okay and then we pop!?") );
			IDValue = INVALID_INDEX;
			sscanf( result, WIDE("%lu"), &IDValue );
			PopODBCEx( odbc);
			//lprintf( WIDE("and then by the time done...") );
		}
		return IDValue;
	}
}


INDEX GetOptionValueIndex( INDEX ID )
{
   return GetOptionValueIndexEx( og.Option, ID );
}

INDEX NewDuplicateValue( PODBC odbc, INDEX iOriginalOption, INDEX iNewOption )
{
	char query[256];
	CTEXTSTR *results;
   TEXTSTR tmp;
	PushSQLQueryEx( odbc );
   // my nested parent may have a select state in a condition that I think it's mine.
	SQLRecordQueryf( odbc, NULL, &results, NULL, "select `string` from "OPTION_VALUES" where option_id=%ld", iOriginalOption );

	if( results && results[0] )
	{
		snprintf( query, sizeof( query )
			  , "replace into "OPTION_VALUES" (option_id,`string`) values (%ld,'%s')"
				  , iNewOption, tmp = EscapeSQLBinary( odbc, results[0], strlen( results[0] ) ) );
		Release( tmp );
      SQLEndQuery( odbc );
		SQLCommand( odbc, query );
	}

	SQLRecordQueryf( odbc, NULL, &results, NULL, "select `binary` from "OPTION_BLOBS" where option_id=%ld", iOriginalOption );

	if( results && results[0] )
	{
		snprintf( query, sizeof( query )
				  , "replace into "OPTION_BLOBS" (option_id,`binary`) values (%ld,'%s')"
				  , iNewOption, tmp = EscapeSQLBinary( odbc, results[0], strlen( results[0] ) ) );
		Release( tmp );
      SQLEndQuery( odbc );
		SQLCommand( odbc, query );
	}
	PopODBCEx( odbc );
	return iNewOption;
}

// this changes in the new code...
INDEX DuplicateValue( INDEX iOriginalValue, INDEX iNewValue )
{
	POPTION_TREE tree = GetOptionTreeEx( og.Option );
	if( tree->flags.bNewVersion )
	{
      return NewDuplicateValue( og.Option, iOriginalValue, iNewValue );
	}
	else
	{
		char query[256];
		INDEX iNewValue;
		snprintf( query, sizeof( query )
				  , "insert into option_values select 0,`string`,`binary` from option_values where value_id=%ld"
				  , iOriginalValue );
		SQLCommand( og.Option, query );
		iNewValue = FetchLastInsertID(og.Option,NULL,NULL);
		return iNewValue;
	}
}

//------------------------------------------------------------------------

_32 GetOptionStringValueEx( PODBC odbc, INDEX optval, char *buffer, _32 len DBG_PASS )
{
	POPTION_TREE tree = GetOptionTreeEx( odbc );
	if( tree->flags.bNewVersion )
	{
      return NewGetOptionStringValue( odbc, optval, buffer, len DBG_RELAY );
	}
	else
	{
		char query[256];
   CTEXTSTR result = NULL;
   int last_was_session, last_was_system;
	INDEX _optval;
   PTRSZVAL result_len = 0;
   len--;

   snprintf( query, sizeof( query ), "select override_value_id from option_exception "
            "where ( apply_from<=now() or apply_from=0 )"
            "and ( apply_until>now() or apply_until=0 )"
            "and ( system_id=%d or system_id=0 )"
            "and value_id=%d "
           , og.SystemID
           , optval );
   last_was_session = 0;
   last_was_system = 0;
   PushSQLQueryEx( og.Option );
	for( SQLQuery( og.Option, query, &result ); result; FetchSQLResult( og.Option, &result ) )
	{
		_optval = optval;
		sscanf( result, WIDE("%") _32f, &optval );
		if( (!optval) || ( optval == INVALID_INDEX ) )
			optval = _optval;
	}
	snprintf( query, sizeof( query ), WIDE("select string from option_values where value_id=%ld"), optval );
	// have to push here, the result of the prior is kept outstanding
   // if this was not pushed, the prior result would evaporate.
	PushSQLQueryEx( og.Option );
	buffer[0] = 0;
	//lprintf( WIDE("do query for value string...") );
	if( SQLQuery( og.Option, query, &result ) )
	{
		//lprintf( WIDE(" query succeeded....") );
		if( result )
		{
			result_len = StrLen( result );
			strncpy( buffer, result, min(len,result_len) );
			buffer[min(len,result_len)] = 0;
		}
		else
		{
			buffer[0] = 0;
		}
	}
	PopODBCEx( og.Option );
   PopODBCEx( og.Option );
	return result_len;
	}
}

_32 GetOptionStringValue( INDEX optval, char *buffer, _32 len )
{
	return GetOptionStringValueEx( og.Option, optval, buffer, len DBG_SRC );
}

int GetOptionBlobValueOdbc( PODBC odbc, INDEX optval, char **buffer, _32 *len )
{
   CTEXTSTR *result = NULL;
	_32 tmplen;
	if( !len )
      len = &tmplen;
	PushSQLQueryEx( odbc );
#ifdef DETAILED_LOGGING
	lprintf( WIDE("do query for value string...") );
#endif
	if( SQLRecordQueryf( odbc, NULL, &result, NULL
							  , WIDE("select `binary`,length(`binary`) from option_values where value_id=%ld")
							  , optval ) )
	{
      int success = FALSE;
		//lprintf( WIDE(" query succeeded....") );
		if( buffer && result && result[0] && result[1] )
		{
         	success = TRUE;
#ifdef _WIN64
			(*buffer) = NewArray( TEXTCHAR, (*len)=atol( result[1] ));
#else
			(*buffer) = NewArray( TEXTCHAR, (*len)=atoi( result[1] ));
#endif
			MemCpy( (*buffer), result[0], (PTRSZVAL)(*len) );
		}
		PopODBCEx( odbc );
		return success;
	}
	return FALSE;
}


int GetOptionBlobValue( INDEX optval, char **buffer, _32 *len )
{
   return GetOptionBlobValueOdbc( og.Option, optval, buffer, len );
}

//------------------------------------------------------------------------

LOGICAL GetOptionIntValue( INDEX optval, int *result_value DBG_PASS )
{
   char value[3];
   if( GetOptionStringValueEx( og.Option, optval, value, sizeof( value ) DBG_RELAY ) != INVALID_INDEX )
   {
      if( value[0] == 'y' || value[0] == 'Y' )
         *result_value = 1;
      else
         *result_value = atoi( value );
      return TRUE;
   }
   return FALSE;
}

//------------------------------------------------------------------------

INDEX CreateValue( PODBC odbc, INDEX iOption, CTEXTSTR pValue )
{
	POPTION_TREE tree = GetOptionTreeEx( odbc );
	if( tree->flags.bNewVersion )
	{
		return NewCreateValue( odbc, iOption, pValue );
	}
	else
	{
		char insert[256];
		CTEXTSTR result=NULL;
		TEXTSTR newval = EscapeSQLBinary( odbc, pValue, StrLen( pValue ) );
		int IDValue;
		if( pValue == NULL )
			snprintf( insert, sizeof( insert ), WIDE("insert into option_blobs (`blob` ) values ('')")
					  );
		else
			snprintf( insert, sizeof( insert ), WIDE("insert into option_values (`string` ) values (%s%s%s)")
					  ,pValue?"\'":""
					  , pValue?newval:"NULL"
					  ,pValue?"\'":"" );
		if( SQLCommand( odbc, insert ) )
		{
			IDValue = FetchLastInsertID( odbc, WIDE("option_values"), WIDE("value_id") );
		}
		else
		{
			FetchSQLError( odbc, &result );
			lprintf( WIDE("Insert value failed: %s"), result );
			IDValue = INVALID_INDEX;
		}
		Release( newval );
		return IDValue;
	}
}


//------------------------------------------------------------------------
// result with option value ID
INDEX SetOptionValueEx( PODBC odbc, INDEX optval, INDEX iValue )
{
	POPTION_TREE tree = GetOptionTreeEx( odbc );
	if( tree->flags.bNewVersion )
	{
      return optval;
	}
	else
	{
	char update[128];
	CTEXTSTR result = NULL;
	// should escape quotes passed in....
	snprintf( update, sizeof( update ), WIDE("update option_map set value_id=%ld where node_id=%ld"), iValue, optval );
	if( !SQLCommand( odbc, update ) )
	{
		GetSQLResult( &result );
		lprintf( WIDE("Update value failed: %s"), result );
		iValue = INVALID_INDEX;
	}
	// should do some sort of pop temp... it's a commit of sorts.
	PopODBCEx( odbc );
	return iValue;
	}
}

INDEX SetOptionValue( INDEX optval, INDEX iValue )
{
   return SetOptionValueEx( og.Option, optval, iValue );
}

//------------------------------------------------------------------------
// result with option value ID
INDEX SetOptionStringValue( INDEX optval, CTEXTSTR pValue )
{
   // update value.
   char update[256];
   char value[256]; // SQL friendly string...
	CTEXTSTR result = NULL;
   TEXTSTR newval;
	INDEX IDValue;
	POPTION_TREE tree = GetOptionTreeEx( og.Option );
	IDValue = GetOptionValueIndexEx( og.Option, optval );
	// should escape quotes passed in....
   if( IDValue && IDValue != INVALID_INDEX )
		snprintf( update, sizeof( update ), WIDE("select string from %s where %s=%lu")
				  , tree->flags.bNewVersion?OPTION_VALUES:"option_values"
				  , tree->flags.bNewVersion?"option_id":"value_id"
				  , IDValue );
   strncpy( value, pValue, sizeof( value )-1 );
   newval = EscapeSQLBinary( og.Option, pValue, strlen( pValue ) );
   if( IDValue && SQLQuery( og.Option, update, &result ) && result )
   {
		snprintf( update, sizeof( update ), WIDE("update %s set string='%s' where %s=%ld")
				  , tree->flags.bNewVersion?OPTION_VALUES:"option_values"
				  , newval
				  , tree->flags.bNewVersion?"option_id":"value_id"
				  , IDValue );
      SQLEndQuery( og.Option );
      if( !SQLCommand( og.Option, update ) )
      {
         FetchSQLError( og.Option, &result );
         lprintf( WIDE("Update value failed: %s"), result );
         IDValue = INVALID_INDEX;
      }
   }
   else
   {
      IDValue = CreateValue( og.Option, optval, value );
      if( IDValue != INVALID_INDEX )
      {
		  // setoption might fail, resulting in an invalid index ID
		  IDValue = SetOptionValueEx( og.Option, optval, IDValue );
      }
   }
   Release( newval );
   return IDValue;
}

//------------------------------------------------------------------------
// result with option value ID
INDEX SetOptionBlobValueEx( PODBC odbc, INDEX optval, POINTER buffer, _32 length )
{
   // update value.
   CTEXTSTR result = NULL;
   INDEX IDValue = GetOptionValueIndexEx( odbc, optval );
	// should escape quotes passed in....
	if( !IDValue )
	{
		IDValue = CreateValue( odbc, optval, NULL );
		IDValue = SetOptionValueEx( odbc, optval, IDValue );
	}

   if( IDValue && IDValue != INVALID_INDEX )
	{
      char *tmp = EscapeBinary( (CTEXTSTR)buffer, length );
      if( !SQLCommandf( odbc, WIDE("update option_values set `binary`='%s' where value_id=%ld"), tmp, IDValue ) )
      {
         FetchSQLError( odbc, &result );
         lprintf( WIDE("Update value failed: %s"), result );
         IDValue = INVALID_INDEX;
		}
      Release( tmp );
		PopODBCEx( odbc );
   }
   return IDValue;
}

#define DIA_X(x) x * 2
#define DIA_Y(y) y * 2
#define DIA_W(w) w * 2
#define DIA_H(h) h * 2

typedef int (CPROC *_F)(
									  LPCSTR lpszSection,
									  LPCSTR lpszEntry,
									  LPCSTR lpszDefault,
									  LPSTR lpszReturnBuffer,
									  int cbReturnBuffer,
									  LPCSTR filename
											  );


int SQLPromptINIValue(
													 LPCSTR lpszSection,
													 LPCSTR lpszEntry,
													 LPCSTR lpszDefault,
													 LPSTR lpszReturnBuffer,
													 int cbReturnBuffer,
													 LPCSTR filename
													)
{
#ifndef __NO_GUI__
#ifndef __STATIC__
	static _F _SQLPromptINIValue;
	if( !_SQLPromptINIValue )
		_SQLPromptINIValue = (_F)LoadFunction( "bag.psi.dll", "_SQLPromptINIValue" );
	if( !_SQLPromptINIValue )
		_SQLPromptINIValue =  (_F)LoadFunction( "libbag.psi.so", "_SQLPromptINIValue" );
	if( !_SQLPromptINIValue )
		_SQLPromptINIValue =  (_F)LoadFunction( "sack_bag.dll", "_SQLPromptINIValue" );
	if( _SQLPromptINIValue )
		return _SQLPromptINIValue(lpszSection, lpszEntry, lpszDefault, lpszReturnBuffer, cbReturnBuffer, filename );
#else
	return _SQLPromptINIValue(lpszSection, lpszEntry, lpszDefault, lpszReturnBuffer, cbReturnBuffer, filename );
#endif
#endif
#if prompt_stdout
	fprintf( stdout, "[%s]%s=%s?\nor enter new value:", lpszSection, lpszEntry, lpszDefault );
	fflush( stdout );
	if( fgets( lpszReturnBuffer, cbReturnBuffer, stdin ) && lpszReturnBuffer[0] != '\n' && lpszReturnBuffer[0] )
	{
      return strlen( lpszReturnBuffer );
	}
#endif
	strncpy( lpszReturnBuffer, lpszDefault, cbReturnBuffer );
	lpszReturnBuffer[cbReturnBuffer-1] = 0;
	return strlen( lpszReturnBuffer );
}

//------------------------------------------------------------------------

SQLGETOPTION_PROC( int, SACK_GetPrivateProfileStringExx )( CTEXTSTR pSection
																		  , CTEXTSTR pOptname
																		  , CTEXTSTR pDefaultbuf
																		  , TEXTCHAR *pBuffer
																		  , PTRSZVAL nBuffer
																		  , CTEXTSTR pININame
																		  , LOGICAL bQuiet
																			DBG_PASS
																		  )
{
	EnterCriticalSec( &og.cs_option );
	{
		INDEX optval = GetOptionIndex( pININame, pSection, pOptname );
      // maybe do an if( l.flags.bLogOptionsRead )
      //lprintf( "Getting option [%s] %s %s", pSection, pOptname, pININame );
		if( optval == INVALID_INDEX )
		{
			// issue dialog
			if( !bQuiet )
			{
				if( og.flags.bPromptDefault )
				{
					SQLPromptINIValue( pSection, pOptname, pDefaultbuf, pBuffer, nBuffer, pININame );
				}
				else
				{
					strncpy( pBuffer, pDefaultbuf, nBuffer );			
				}
			}
			else
				strncpy( pBuffer, pDefaultbuf, nBuffer );
			// create the option branch since it doesn't exist...
			optval = GetOptionIndexEx( OPTION_ROOT_VALUE, pININame, pSection, pOptname, TRUE DBG_RELAY );
			{
				int x = SetOptionStringValue( optval, pBuffer ) != INVALID_INDEX;
				LeaveCriticalSec( &og.cs_option );
				return x;
			}
			//strcpy( pBuffer, pDefaultbuf );
		}
		else
		{
			int x = GetOptionStringValueEx( og.Option, GetOptionValueIndexEx( og.Option, optval ), pBuffer, nBuffer DBG_RELAY ) != INVALID_INDEX;
			LeaveCriticalSec( &og.cs_option );
			return x;
		}
	}
	LeaveCriticalSec( &og.cs_option );

	return FALSE;
}

SQLGETOPTION_PROC( int, SACK_GetPrivateProfileStringEx )( CTEXTSTR pSection
																		  , CTEXTSTR pOptname
																		  , CTEXTSTR pDefaultbuf
																		  , char *pBuffer
																		  , _32 nBuffer
																		  , CTEXTSTR pININame
																		  , LOGICAL bQuiet
																		  )
{
   return SACK_GetPrivateProfileStringExx( pSection, pOptname, pDefaultbuf, pBuffer, nBuffer, pININame, bQuiet DBG_SRC );
}

SQLGETOPTION_PROC( int, SACK_GetPrivateProfileString )( CTEXTSTR pSection
                                                 , CTEXTSTR pOptname
                                                 , CTEXTSTR pDefaultbuf
                                                 , char *pBuffer
                                                 , _32 nBuffer
                                                 , CTEXTSTR pININame )
{
   return SACK_GetPrivateProfileStringEx( pSection, pOptname, pDefaultbuf, pBuffer, nBuffer, pININame, FALSE );
}
//------------------------------------------------------------------------

SQLGETOPTION_PROC( S_32, SACK_GetPrivateProfileIntEx )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 nDefault, CTEXTSTR pININame, LOGICAL bQuiet )
{
   char buffer[32];
   char defaultbuf[32];
   snprintf( defaultbuf, sizeof( defaultbuf ), WIDE("%ld"), nDefault );
   if( SACK_GetPrivateProfileStringEx( pSection, pOptname, defaultbuf, buffer, sizeof( buffer ), pININame, bQuiet ) )
	{
		if( buffer[0] == 'Y' || buffer[0] == 'y' )
         return 1;
      return atoi( buffer );
   }
   return nDefault;
}

SQLGETOPTION_PROC( S_32, SACK_GetPrivateProfileInt )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 nDefault, CTEXTSTR pININame )
{
   return SACK_GetPrivateProfileIntEx( pSection, pOptname, nDefault, pININame, FALSE );
}

//------------------------------------------------------------------------

#define DEFAULT_PUBLIC_KEY "DEFAULT"
//#define DEFAULT_PUBLIC_KEY "system"

SQLGETOPTION_PROC( int, SACK_GetProfileStringEx )( CTEXTSTR pSection, CTEXTSTR pOptname, CTEXTSTR pDefaultbuf, TEXTCHAR *pBuffer, _32 nBuffer, LOGICAL bQuiet )
{
   return SACK_GetPrivateProfileStringEx( pSection, pOptname, pDefaultbuf, pBuffer, nBuffer, DEFAULT_PUBLIC_KEY, bQuiet );
}

SQLGETOPTION_PROC( int, SACK_GetProfileString )( CTEXTSTR pSection, CTEXTSTR pOptname, CTEXTSTR pDefaultbuf, TEXTCHAR *pBuffer, _32 nBuffer )
{
   return SACK_GetPrivateProfileString( pSection, pOptname, pDefaultbuf, pBuffer, nBuffer, DEFAULT_PUBLIC_KEY );
}

//------------------------------------------------------------------------


SQLGETOPTION_PROC( int, SACK_GetProfileBlobOdbc )( PODBC odbc, CTEXTSTR pSection, CTEXTSTR pOptname, TEXTCHAR **pBuffer, _32 *pnBuffer )
{
   INDEX optval = GetOptionIndexExx( odbc, OPTION_ROOT_VALUE, NULL, pSection, pOptname, FALSE DBG_SRC );
   if( optval == INVALID_INDEX )
   {
      return FALSE;
   }
   else
	{
      return GetOptionBlobValueOdbc( odbc, GetOptionValueIndexEx( odbc, optval ), pBuffer, pnBuffer );
   }
   return FALSE;
//   int status = SACK_GetProfileString( );
}

SQLGETOPTION_PROC( int, SACK_GetProfileBlob )( CTEXTSTR pSection, CTEXTSTR pOptname, TEXTCHAR **pBuffer, _32 *pnBuffer )
{
   return SACK_GetProfileBlobOdbc( og.Option, pSection, pOptname, pBuffer, pnBuffer );
}

//------------------------------------------------------------------------

SQLGETOPTION_PROC( S_32, SACK_GetProfileIntEx )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 defaultval, LOGICAL bQuiet )
{
   return SACK_GetPrivateProfileIntEx( pSection, pOptname, defaultval, DEFAULT_PUBLIC_KEY, bQuiet );
}

//------------------------------------------------------------------------

SQLGETOPTION_PROC( S_32, SACK_GetProfileInt )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 defaultval )
{
   return SACK_GetPrivateProfileInt( pSection, pOptname, defaultval, DEFAULT_PUBLIC_KEY );
}

//------------------------------------------------------------------------

SQLGETOPTION_PROC( int, SACK_WritePrivateProfileString )( CTEXTSTR pSection, CTEXTSTR pName, CTEXTSTR pValue, CTEXTSTR pINIFile )
{
   INDEX optval;
   optval = GetOptionIndexEx( OPTION_ROOT_VALUE, pINIFile, pSection, pName, TRUE DBG_SRC );
   if( optval == INVALID_INDEX )
   {
      lprintf( WIDE("Creation of path failed!") );
      return FALSE;
   }
   else
   {
      return SetOptionStringValue( optval, pValue ) != INVALID_INDEX;
   }
}

//------------------------------------------------------------------------

SQLGETOPTION_PROC( S_32, SACK_WritePrivateProfileInt )( CTEXTSTR pSection, CTEXTSTR pName, S_32 value, CTEXTSTR pINIFile )
{
   char valbuf[32];
   snprintf( valbuf, sizeof( valbuf ), WIDE("%ld"), value );
   return SACK_WritePrivateProfileString( pSection, pName, valbuf, pINIFile );
}


//------------------------------------------------------------------------

SQLGETOPTION_PROC( int, SACK_WriteProfileString )( CTEXTSTR pSection, CTEXTSTR pName, CTEXTSTR pValue )
{
   return SACK_WritePrivateProfileString( pSection, pName, pValue, DEFAULT_PUBLIC_KEY );
}

//------------------------------------------------------------------------

SQLGETOPTION_PROC( S_32, SACK_WriteProfileInt )( CTEXTSTR pSection, CTEXTSTR pName, S_32 value )
{
   return SACK_WritePrivateProfileInt( pSection, pName, value, DEFAULT_PUBLIC_KEY );
}

SQLGETOPTION_PROC( int, SACK_WriteProfileBlobOdbc )( PODBC odbc, CTEXTSTR pSection, CTEXTSTR pOptname, char *pBuffer, _32 nBuffer )
{
   INDEX optval;
   optval = GetOptionIndexExx( odbc, OPTION_ROOT_VALUE, NULL, pSection, pOptname, TRUE DBG_SRC );
   if( optval == INVALID_INDEX )
   {
      lprintf( WIDE("Creation of path failed!") );
      return FALSE;
   }
   else
   {
      return SetOptionBlobValueEx( odbc, optval, pBuffer, nBuffer ) != INVALID_INDEX;
	}
   return 0;
}


SQLGETOPTION_PROC( int, SACK_WriteProfileBlob )( CTEXTSTR pSection, CTEXTSTR pOptname, char *pBuffer, _32 nBuffer )
{
   return SACK_WriteProfileBlobOdbc( og.Option, pSection, pOptname, pBuffer, nBuffer );
}

//------------------------------------------------------------------------

#if 0
/// this still needs a way to communicate the time from and time until.
SQLGETOPTION_PROC( int, SACK_WritePrivateProfileExceptionString )( CTEXTSTR pSection
                                                            , CTEXTSTR pName
                                                            , CTEXTSTR pValue
                                                            , CTEXTSTR pINIFile
                                                            , _32 from // SQLTIME
                                                            , _32 to   // SQLTIME
                                                            , CTEXTSTR pSystemName )
{
   INDEX optval = GetOptionIndexEx( OPTION_ROOT_VALUE, pINIFile, pSection, pName, TRUE DBG_SRC );
   if( optval == INVALID_INDEX )
   {
      lprintf( WIDE("Creating of path failed!") );
      return FALSE;
   }
   else
   {
      //CTEXTSTR result = NULL;
      char exception[256];
      INDEX system;
      INDEX IDValue = CreateValue( og.Option, optval,pValue );
      system = GetSystemIndex( pSystemName );

		snprintf( exception, sizeof( exception ), WIDE("insert into option_exception (`apply_from`,`apply_to`,`value_id`,`override_value_idvalue_id`,`system`) ")
																	  WIDE( "values( \'%04d%02d%02d%02d%02d\', \'%04d%02d%02d%02d%02d\', %ld, %ld,%d")
             , wYrFrom, wMoFrom, wDyFrom
             , wHrFrom, wMnFrom,wScFrom
             , wYrTo, wMoTo, wDyTo
             , wHrTo, wMnTo,wScTo
             , optval
              , IDValue
              , system // system
              );
      if( !SQLCommand( og.Option, exception ) )
      {
         CTEXTSTR result = NULL;
         GetSQLResult( &result );
         lprintf( WIDE("Insert exception failed: %s"), result );
      }
      else
      {
         if( system || session )
         {
            INDEX IDTime = FetchLastInsertID( og.Option, WIDE("option_exception"), WIDE("exception_id") );
            // lookup system name... provide detail record
         }
      }
   }
   return 1;
}
#endif

struct option_interface_tag DefaultInterface =
{
   SACK_GetPrivateProfileString
   , SACK_GetPrivateProfileInt
   , SACK_GetProfileString
   , SACK_GetProfileInt
   , SACK_WritePrivateProfileString
   , SACK_WritePrivateProfileInt
   , SACK_WriteProfileString
   , SACK_WriteProfileInt
   , SACK_GetPrivateProfileStringEx
   , SACK_GetPrivateProfileIntEx
   , SACK_GetProfileStringEx
   , SACK_GetProfileIntEx
};

#undef GetOptionInterface
SQLGETOPTION_PROC( POPTION_INTERFACE, GetOptionInterface )( void )
{
   return &DefaultInterface;
}

SQLGETOPTION_PROC( void, DropOptionInterface )( POPTION_INTERFACE interface_drop )
{

}

PRELOAD(RegisterSQLOptionInterface)
{
	RegisterInterface( WIDE("SACK_SQL_Options"), (POINTER(CPROC *)(void))GetOptionInterface, (void(CPROC *)(POINTER))DropOptionInterface );
   og.flags.bUseProgramDefault = SACK_GetProfileIntEx( GetProgramName(), "SACK/SQL/Options/Options Use Program Name Default", 0, TRUE );
   og.flags.bUseSystemDefault = SACK_GetProfileIntEx( GetProgramName(), "SACK/SQL/Options/Options Use System Name Default", 0, TRUE );
}

SQLGETOPTION_PROC( INDEX, GetSystemID )( void )
{
   InitMachine();
   return og.SystemID;
}

SQLGETOPTION_PROC( void, BeginBatchUpdate )( void )
{
	//   SQLCommand(
   //SQLCommand( og.Option, "BEGIN TRANSACTION" );
}

SQLGETOPTION_PROC( void, EndBatchUpdate )( void )
{
   //SQLCommand( og.Option, "COMMIT" );
}
_OPTION_NAMESPACE_END SACK_NAMESPACE_END

