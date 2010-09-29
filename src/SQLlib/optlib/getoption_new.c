#define OPTION_MAIN_SOURCE
#ifndef GETOPTION_SOURCE
#define GETOPTION_SOURCE
#endif
#include <stdhdrs.h>
#include <sack_types.h>
#include <deadstart.h>
#include <sharemem.h>
#include <filesys.h>
#include <system.h>
#include <network.h>
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

SACK_OPTION_NAMESPACE

#include "optlib.h"

typedef struct sack_option_global_tag OPTION_GLOBAL;


#define og sack_global_option_data
extern OPTION_GLOBAL og;

//------------------------------------------------------------------------

#define MKSTR(n,...) #__VA_ARGS__

#include "makeopts.mysql"
;

//------------------------------------------------------------------------

//------------------------------------------------------------------------

//#define OPTION_ROOT_VALUE INVALID_INDEX
#define OPTION_ROOT_VALUE 0

INDEX NewGetOptionIndexExx( PODBC odbc, INDEX parent, const char *file, const char *pBranch, const char *pValue, int bCreate DBG_PASS )
//#define GetOptionIndex( f,b,v ) GetOptionIndexEx( OPTION_ROOT_VALUE, f, b, v, FALSE )
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
			StrCpyEx( namebuf, (*start), sizeof( namebuf )-1 );
         (*start) = NULL;
         start = NULL;
	  }

      // remove references of 'here' during parsing.
		if( strcmp( namebuf, "." ) == 0 )
         continue;

      {
          // double convert 'precistion loss 64bit gcc'
			INDEX node_id = ((INDEX)(PTRSZVAL)FamilyTreeFindChild( *GetOptionTree( odbc ), (PTRSZVAL)namebuf )) - 1;
			if( node_id != INVALID_INDEX )
			{
#ifdef DETAILED_LOGGING
				lprintf( WIDE("Which is found, and new parent ID result...%d"), node_id );
#endif
				parent = node_id;
				continue;
			}
		}

		{
         INDEX IDName = SQLReadNameTable(odbc,namebuf,OPTION_NAME,"name_id" );

			PushSQLQueryExEx(odbc DBG_RELAY );
         snprintf( query, sizeof( query )
                 , "select option_id from "OPTION_MAP" where parent_option_id=%ld and name_id=%d"
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
               snprintf( query, sizeof( query ), "Insert into "OPTION_MAP"(`parent_option_id`,`name_id`) values (%ld,%lu)", parent, IDName );
               if( SQLCommand( odbc, query ) )
               {
                  ID = FetchLastInsertID( odbc, OPTION_MAP, WIDE("option_id") );
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
            lprintf( WIDE("Option tree corrupt.  No option option_id=%ld"), ID );
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
   return parent;
}

//------------------------------------------------------------------------

_32 NewGetOptionStringValue( PODBC odbc, INDEX optval, char *buffer, _32 len DBG_PASS )
{
	char query[256];
	CTEXTSTR result = NULL;
	int last_was_session, last_was_system;
	INDEX _optval;
	_32 result_len = 0;
	len--;

	snprintf( query, sizeof( query ), "select override_value_id from "OPTION_EXCEPTION" "
            "where ( apply_from<=now() or apply_from=0 )"
            "and ( apply_until>now() or apply_until=0 )"
            "and ( system_id=%d or system_id=0 )"
            "and option_id=%d "
           , og.SystemID
           , optval );
	last_was_session = 0;
	last_was_system = 0;
	PushSQLQueryEx( odbc );
	for( SQLQuery( odbc, query, &result ); result; FetchSQLResult( odbc, &result ) )
	{
		_optval = optval;
		sscanf( result, WIDE("%") _32f, &optval );
		if( (!optval) || ( optval == INVALID_INDEX ) )
			optval = _optval;
	}
	snprintf( query, sizeof( query ), "select string from "OPTION_VALUES" where option_id=%ld", optval );
	// have to push here, the result of the prior is kept outstanding
	// if this was not pushed, the prior result would evaporate.
	PushSQLQueryEx( odbc );
	buffer[0] = 0;
	//lprintf( WIDE("do query for value string...") );
	if( SQLQuery( odbc, query, &result ) )
	{
		//lprintf( WIDE(" query succeeded....") );
		if( result )
		{
			result_len = StrLen( result );
			StrCpyEx( buffer, result, min(len,result_len+1)*sizeof(TEXTCHAR) );
			buffer[min(len,result_len)] = 0;
		}
		else
		{
			buffer[0] = 0;
		}
	}
	PopODBCEx( odbc );
	PopODBCEx( odbc );
	return result_len;
}


int NewGetOptionBlobValueOdbc( PODBC odbc, INDEX optval, char **buffer, _32 *len )
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
							  , "select `binary`,length(`binary`) from "OPTION_BLOBS" where option_id=%ld"
							  , optval ) )
	{
      int success = FALSE;
		//lprintf( WIDE(" query succeeded....") );
		if( buffer && result && result[0] && result[1] )
		{
         success = TRUE;
			(*buffer) = NewArray( TEXTCHAR, (*len)=atol( result[1] ));
			MemCpy( (*buffer), result[0], (*len) );
		}
		PopODBCEx( odbc );
		return success;
	}
	return FALSE;
}


//------------------------------------------------------------------------

INDEX NewCreateValue( PODBC odbc, INDEX value, CTEXTSTR pValue )
{
   TEXTCHAR insert[256];
	CTEXTSTR result=NULL;
   TEXTSTR newval = EscapeSQLBinary( odbc, pValue, StrLen( pValue ) );
   int IDValue;
	if( pValue == NULL )
		snprintf( insert, sizeof( insert ), "insert into "OPTION_BLOBS " (`option_id`,`blob` ) values (%lu,'')"
				  , value
				  );
	else
		snprintf( insert, sizeof( insert ), "insert into "OPTION_VALUES " (`option_id`,`string` ) values (%lu,%s%s%s)"
				  , value
				  ,pValue?"\'":""
				  , pValue?newval:"NULL"
				  ,pValue?"\'":"" );
	if( SQLCommand( odbc, insert ) )
   {
      //IDValue = FetchLastInsertID( odbc, WIDE(""OPTION_VALUES""), WIDE("option_id") );
   }
   else
   {
      FetchSQLError( odbc, &result );
      lprintf( WIDE("Insert value failed: %s"), result );
      IDValue = INVALID_INDEX;
	}
   Release( newval );
   return value;
}


SACK_OPTION_NAMESPACE_END

