/* more documentation at end */

/*
 *
 *   Creator: Panther   #implemented in Dekware
 *   Modified by: Jim Buckeyne #ported to service SQL via proxy.
 *   Returned to sack by: Jim Buckeyne
 *                  # stripped application specific
 *                  # features, returned to SACK.
 *
 *  Provides a simple, intuitive interface to SQL.  Used sensibly,
 *  provides garbage collection of resources.
 *
 *  Commands without an ODBC specifier are the perferred method to
 *  use this interface.  This allows the internal system to maintain
 *  a primary and a redundant backup connection to provide transparent
 *  reliability to the application.
 *
 *  Provides some slick table creation routines
 *     - check for existance, and drop  (CTO_DROP)
 *     - check for existance, and match (CTO_MATCH)
 *     - check for existance, and merge (CTO_MERGE)
 *     - create table if not exist.
 *
 *  Latest additions provide ...RecordQuery... functions which
 *  result with a const CTEXTSTR * of results;  (ie, result[0] = (CTEXTSTR)result1 )
 *  also available are the column names from the query.
 *  I strongly recommend passing NULL always to the field names, and
 *  using sensible enumerators that follow the query definition.
 *
 *  (c)Freedom Collective (Jim Buckeyne <2000-2006)
 *
 */


#ifndef PSSQL_STUB_DEFINED
#define PSSQL_STUB_DEFINED
#include <sack_types.h>

#ifdef BCC16
#if defined( SQLSTUB_SOURCE ) || defined( SQLPROXY_LIBRARY_SOURCE )
#define SQLSTUB_PROC(type,name) type STDPROC _export name
#else
#define SQLSTUB_PROC(type,name) type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#if defined( SQLSTUB_SOURCE ) || defined( SQLPROXY_LIBRARY_SOURCE )
#define SQLSTUB_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define SQLSTUB_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#else
#if defined( SQLSTUB_SOURCE ) || defined( SQLPROXY_LIBRARY_SOURCE )
#define SQLSTUB_PROC(type,name) type CPROC name
#else
#define SQLSTUB_PROC(type,name) extern type CPROC name
#endif
#endif
#endif

#ifdef __cplusplus
#define SQL_NAMESPACE   namespace sack { namespace sql {
#define SQL_NAMESPACE_END } }
#else
#define SQL_NAMESPACE   
#define SQL_NAMESPACE_END 
#endif

SQL_NAMESPACE

#ifdef _MSC_VER
#ifndef __NO_SQL__
#ifndef USE_SQLITE
#define USE_SQLITE
#endif
#ifndef USE_ODBC
#define USE_ODBC
#endif
#endif
#endif


#define SQLPROXY_PROC SQLSTUB_PROC
typedef struct odbc_handle_tag *PODBC, ODBC;

// recently added {} container braces for structure element
#define FIELDS(n) {( sizeof( n ) / sizeof( FIELD ) ), n}

#define MAX_PREVIOUS_FIELD_NAMES 4
typedef struct required_field_tag
{
	CTEXTSTR name;
	CTEXTSTR type;
	CTEXTSTR extra;
	// if you have renamed this column more than 4
	// times - you really need to stop messing around
	// and get a life.
	CTEXTSTR previous_names[MAX_PREVIOUS_FIELD_NAMES];
} FIELD, *PFIELD;

#ifndef _MSC_VER
#define KEY_COLUMNS(...) { __VA_ARGS__, NULL }
#endif
#define TABLE_KEYS(n) {( sizeof( n ) / sizeof( DB_KEY_DEF ) ), n}
// DB_KEY_DEF keys[] = { { "lockey", KEY_COLUMNS("hall_id","charity_id") } };
// FIELD fields[] = { { "ID", WIDE("int") }
																																																																																													//                  ,
#define MAX_KEY_COLUMNS 8


typedef struct required_key_def  DB_KEY_DEF, *PDB_KEY_DEF;
struct required_key_def
{
	struct {
		_32 bPrimary : 1;
		_32 bUnique : 1;
	} flags;
	CTEXTSTR name;
   CTEXTSTR colnames[MAX_KEY_COLUMNS]; // uhm up to 5 colnames...
   CTEXTSTR null; // if not null, broken structure...
#ifdef __cplusplus
   required_key_def( int bPrimary, int bUnique, CTEXTSTR _name, CTEXTSTR colname1 ) { flags.bPrimary = bPrimary; flags.bUnique = bUnique; name = _name; colnames[0] = colname1; colnames[1] = NULL; }
   required_key_def( int bPrimary, int bUnique, CTEXTSTR _name, CTEXTSTR colname1, CTEXTSTR colname2 ) { flags.bPrimary = bPrimary; flags.bUnique = bUnique; name = _name; colnames[0] = colname1; colnames[1] = colname2; colnames[2] = 0; }
#endif
}; // PKEY and KEY are too simple... too easy to collide in namespace

typedef struct required_table_tag
{
	CTEXTSTR name;
	struct {
		int count;
		PFIELD field;
	} fields;
	struct {
		int count;
      PDB_KEY_DEF key;
	} keys;
	struct {
		BIT_FIELD bDynamic : 1; // set this if defined dynamically (from getfields in SQL)
		BIT_FIELD bTemporary : 1;
		BIT_FIELD bIfNotExist : 1;
	} flags;
   CTEXTSTR create_like_table_name;
   CTEXTSTR database;
   CTEXTSTR type;
   CTEXTSTR comment;
} TABLE, *PTABLE;

SQLSTUB_PROC( LOGICAL, CheckODBCTable)( PODBC odbc, PTABLE table, _32 options );
SQLSTUB_PROC( void, SetSQLLoggingDisable )( PODBC odbc, LOGICAL bDisable );



// result is FALSE on error
// result is TRUE on success
SQLSTUB_PROC( int, DoSQLCommandEx )( CTEXTSTR command DBG_PASS);
#define DoSQLCommand(c) DoSQLCommandEx(c DBG_SRC )


// parameters to this are pairs of "name", type, WIDE("value")
//  type == 0 - value is text, do not quote
//  type == 1 - value is text, add quotes appropriate for database
//  type == 2 - value is an integer, do not quote
// the last pair's name is NULL, and value does not matter.
// insert values into said table.
SQLSTUB_PROC( int, DoSQLInsert )( CTEXTSTR table, ... );
// attempt to open some sort of odbc handler, the primary or backup
// available through this interface
SQLSTUB_PROC( int, OpenSQL )( void );
SQLSTUB_PROC( int, OpenSQLConnection )( PODBC );


// should pass to this a &(CTEXTSTR) which starts as NULL for result.
// result is FALSE on error
// result is TRUE on success, and **result is updated to 
// contain the resulting data.
SQLSTUB_PROC( int, DoSQLQueryEx )( CTEXTSTR query, CTEXTSTR *result DBG_PASS);
#define DoSQLQuery(q,r) DoSQLQueryEx( q,r DBG_SRC )
#define DoSQLRecordQuery(q,r,c,f) SQLRecordQueryEx( NULL,q,r,c,f DBG_SRC )
#define DoSQLQueryRecord(q,r,c)   DoSQLRecordQuery(q,r,c,NULL)
#define SQLQueryRecord(o,q,r,c)   SQLRecordQuery(o,q,r,c,NULL)
#define GetSQLResultRecord(r,c)   GetSQLRecord(c)

// gets another result from a query else only one 
// comes back.  One does not HAVE to get the results if more
// exist... and *result will be NULL when there are no more.
SQLSTUB_PROC( int, GetSQLResult )( CTEXTSTR *result );
// column count and fields will not change when getting subsequent results...
// so don't require the params...
SQLSTUB_PROC( int, GetSQLRecord )( CTEXTSTR **result );
SQLSTUB_PROC( int, GetSQLError )( CTEXTSTR *result );
SQLSTUB_PROC( int, IsSQLReady )( void );

SQLSTUB_PROC( int, PushSQLQuery )( void );
SQLSTUB_PROC( void, PopODBC )( void );
SQLSTUB_PROC( void, PopODBCEx )( PODBC );
SQLSTUB_PROC( void, SQLEndQuery )( PODBC odbc );
// release any open queries on the database... all result
// sets are now invalid... uhmm what about things like fields?
// could be messy...
SQLSTUB_PROC( void, ReleaseODBC )( PODBC odbc );

// does a query responce kinda thing returning types.
// if( GetSQLTypes() ) while( GetSQLResult( &result ) && result )
SQLSTUB_PROC( int, GetSQLTypes )( void );

SQLSTUB_PROC( void, ConvertSQLDateEx )( CTEXTSTR date
												  , int *year, int *month, int *day
												  , int *hour, int *minute, int *second
												  , int *msec, S_32 *nsec );
#define ConvertSQLDate( date, y,m,d) ConvertSQLDateEx( date,y,m,d,NULL,NULL,NULL,NULL,NULL)
#define ConvertSQLDateTime( date, y,mo,d,h,mn,s) ConvertSQLDateEx( date,y,mo,d,h,mn,s,NULL,NULL)


//------------------------------
// this set of functions will auto create a suitable name table
// providing table_name_id and table_name_name as the columns to query by standard
// previous defaults where "id" and "name" which results in inability to use natural join
//
SQLSTUB_PROC( INDEX, FetchSQLNameID )( PODBC odbc, CTEXTSTR table_name, CTEXTSTR name );
SQLSTUB_PROC( INDEX, GetSQLNameID )( CTEXTSTR table_name, CTEXTSTR name );

SQLSTUB_PROC( CTEXTSTR, FetchSQLName )( PODBC odbc, CTEXTSTR table_name, INDEX iName );
SQLSTUB_PROC( CTEXTSTR, GetSQLName )( CTEXTSTR table_name, INDEX iName );
//------------------------------


//------------------------------
// these functions provide general name lookup utility
SQLSTUB_PROC( INDEX, ReadNameTableExEx)( CTEXTSTR name, CTEXTSTR table, CTEXTSTR col, CTEXTSTR namecol, int bCreate DBG_PASS );
#define ReadNameTableExx( name,table,col,namecol,bCreate) ReadNameTableExEx( name,table,col,namecol,bCreate DBG_SRC )
//column name if NOT specified will be 'ID'
SQLSTUB_PROC( INDEX, ReadNameTableEx)( CTEXTSTR name, CTEXTSTR table, CTEXTSTR col DBG_PASS );
#define ReadNameTable(n,t,c) ReadNameTableExEx( n,t,c,"name",TRUE DBG_SRC )
// TRUE if name in result...
// again if !colname colname = 'ID'
SQLSTUB_PROC( int, ReadFromNameTableEx )( INDEX id, CTEXTSTR table, CTEXTSTR id_colname, CTEXTSTR name_colname, CTEXTSTR *result DBG_PASS);
SQLSTUB_PROC( int, ReadFromNameTableExEx )( INDEX id, CTEXTSTR table, CTEXTSTR id_column, CTEXTSTR colname, CTEXTSTR *result DBG_PASS);
#define ReadFromNameTableExx(id,t,ic,nc,r) ReadFromNameTableExEx(id,t,ic,nc,r DBG_SRC )
#define ReadFromNameTable(id,t,c,r) ReadFromNameTableEx(id,t,c,"name",r DBG_SRC )

SQLSTUB_PROC( INDEX, SQLReadNameTableExEx)( PODBC odbc, CTEXTSTR name, CTEXTSTR table, CTEXTSTR col, CTEXTSTR namecol, int bCreate DBG_PASS );
#define SQLReadNameTableExx( odbc,name,table,col,namecol,bCreate) SQLReadNameTableExEx( odbc,name,table,col,namecol,bCreate DBG_SRC )
#define SQLReadNameTable(o,n,t,c) SQLReadNameTableExEx( o,n,t,c,"name",TRUE DBG_SRC )



// table and col are not used if a MySQL backend is used...
// they are needed to get the last ID from a postgresql backend.
SQLSTUB_PROC( INDEX, GetLastInsertIDEx)( CTEXTSTR table, CTEXTSTR col DBG_PASS );
#define GetLastInsertID(t,c) GetLastInsertIDEx(t,c DBG_SRC )
SQLSTUB_PROC( INDEX, FetchLastInsertIDEx)( PODBC odbc, CTEXTSTR table, CTEXTSTR col DBG_PASS );
#define FetchLastInsertID(o,t,c) FetchLastInsertIDEx(o,t,c DBG_SRC )

// CreateTable Options (CTO_)
#define CTO_DROP 1   // drop old table before create.
#define CTO_MATCH 4  // attempt to figure out alter statements to drop or add columns to exact match definition
#define CTO_MERGE 8  // attempt to figure out alter statements to add missing columns, do not drop.  Rename?
#define CTO_LOG_CHANGES 16 // log changes to "changes.sql"

SQLSTUB_PROC( int, SQLCreateTableEx )(PODBC odbc, CTEXTSTR filename, CTEXTSTR templatename, CTEXTSTR tablename, _32 options );
#define SQLCreateTable( odbc, file, table ) SQLCreateTableEx(odbc,file,table,table,0)
SQLSTUB_PROC( int, CreateTableEx )( CTEXTSTR filename, CTEXTSTR templatename, CTEXTSTR tablename, _32 options );
#define CreateTable( file, table ) CreateTableEx(file,table,table,0)

// results in a static buffer with escapes filled in for characterws
// which would otherwise conflict with string punctuators.
SQLSTUB_PROC( TEXTSTR ,EscapeStringEx )( CTEXTSTR name DBG_PASS );
#define EscapeString(s) EscapeStringEx( s DBG_SRC )
SQLSTUB_PROC( TEXTCHAR *,EscapeSQLStringEx )( PODBC odbc, CTEXTSTR name DBG_PASS );
#define EscapeSQLString(odbc, s) EscapeSQLStringEx( odbc, s DBG_SRC )
//SQLSTUB_PROC( CTEXTSTR ,EscapeString )( CTEXTSTR name );
// the following functions return an allcoated buffer which the application must Release()
SQLSTUB_PROC( TEXTSTR ,EscapeBinaryEx )( CTEXTSTR blob, _32 bloblen DBG_PASS );
#define EscapeBinary(b,bl) EscapeBinaryEx(b,bl DBG_SRC )
//SQLSTUB_PROC( CTEXTSTR ,EscapeBinary )( CTEXTSTR blob, _32 bloblen );

SQLSTUB_PROC( TEXTSTR,EscapeSQLBinaryEx )( PODBC odbc, CTEXTSTR blob, _32 bloblen DBG_PASS );
#define EscapeSQLBinary(odbc,blob,len) EscapeSQLBinaryEx( odbc,blob,len DBG_SRC )

SQLSTUB_PROC( TEXTSTR ,RevertEscapeString )( CTEXTSTR name );
SQLSTUB_PROC( TEXTSTR ,RevertEscapeBinary )( CTEXTSTR blob, _32 *bloblen );
SQLSTUB_PROC( TEXTSTR , DeblobifyString )( CTEXTSTR blob, TEXTSTR buffer, int buflen );

SQLSTUB_PROC( int, ConvertDBTimeString )( CTEXTSTR timestring
                                        , CTEXTSTR *endtimestring
													 , int *pyr, int *pmo, int *pdy
													 , int *phr, int *pmn, int *psc );


SQLSTUB_PROC( int, SQLCommandEx )( PODBC odbc, CTEXTSTR command DBG_PASS);
#define SQLCommand(o,c) SQLCommandEx(o,c DBG_SRC )
SQLSTUB_PROC( int, SQLInsertBegin )( PODBC odbc );
SQLSTUB_PROC( int, vSQLInsert )( PODBC odbc, CTEXTSTR table, va_list args );
SQLSTUB_PROC( int, SQLInsert )( PODBC odbc, CTEXTSTR table, ... );
SQLSTUB_PROC( int, DoSQLInsert )( CTEXTSTR table, ... );
SQLSTUB_PROC( int, SQLInsertFlush )( PODBC odbc );

SQLSTUB_PROC( int, SQLQueryEx )( PODBC odbc, CTEXTSTR query, CTEXTSTR *result DBG_PASS);
#define SQLQuery(o,q,r) SQLQueryEx( o,q,r DBG_SRC )
SQLSTUB_PROC( int, SQLRecordQueryEx )( PODBC odbc
												 , CTEXTSTR query
												 , int *pnResult
												 , CTEXTSTR **result
												 , CTEXTSTR **fields DBG_PASS);
#define SQLRecordQuery(o,q,prn,r,f) SQLRecordQueryEx( o,q,prn,r,f DBG_SRC )
SQLSTUB_PROC( int, FetchSQLResult )( PODBC, CTEXTSTR *result );
SQLSTUB_PROC( int, FetchSQLRecord )( PODBC, CTEXTSTR **result ); // count and fields are constant from query.
SQLSTUB_PROC( int, FetchSQLError )( PODBC, CTEXTSTR *result );
SQLSTUB_PROC( int, IsSQLOpen )( PODBC );

SQLSTUB_PROC( int, PushSQLQueryExEx )(PODBC DBG_PASS);
SQLSTUB_PROC( int, PushSQLQueryEx )(PODBC);
#define PushSQLQueryEx(odbc) PushSQLQueryExEx(odbc DBG_SRC )

// no application support for username/password, sorry, trust thy odbc layer, please
SQLSTUB_PROC( PODBC, ConnectToDatabase )( CTEXTSTR dsn );
// default parameter to require is the global flag RequireConnection from sql.config....
SQLSTUB_PROC( PODBC, ConnectToDatabaseEx )( CTEXTSTR DSN, LOGICAL bRequireConnection );
SQLSTUB_PROC( void, CloseDatabase)(PODBC odbc );

// does a query responce kinda thing returning types.
// if( GetSQLTypes() ) while( GetSQLResult( &result ) && result )
SQLSTUB_PROC( int, GetSQLTypes )( void );
SQLSTUB_PROC( int, FetchSQLTypes )( PODBC );

SQLSTUB_PROC( int, DoSQLRecordQueryf )( int *columns, CTEXTSTR **result, CTEXTSTR **fields, CTEXTSTR fmt, ... );
SQLSTUB_PROC( int, DoSQLQueryf )( CTEXTSTR *result, CTEXTSTR fmt, ... );
SQLSTUB_PROC( int, DoSQLCommandf )( CTEXTSTR fmt, ... );
SQLSTUB_PROC( int, SQLRecordQueryf )( PODBC odbc, int *columns, CTEXTSTR **result, CTEXTSTR **fields, CTEXTSTR fmt, ... );
SQLSTUB_PROC( int, SQLQueryf )( PODBC odbc, CTEXTSTR *result, CTEXTSTR fmt, ... );
SQLSTUB_PROC( int, SQLCommandf )( PODBC odbc, CTEXTSTR fmt, ... );


// register a feedback message for startup messages
//  allows external bannering of status... perhaps this can handle failures
//  and disconnects also...
SQLSTUB_PROC( void, SQLSetFeedbackHandler )( void (CPROC *HandleSQLFeedback)(CTEXTSTR message) );

SQLSTUB_PROC( PTABLE, GetFieldsInSQLEx )( CTEXTSTR cmd, int writestate DBG_PASS );
#define GetFieldsInSQL(c,w) GetFieldsInSQLEx( c, w DBG_SRC )
//SQLSTUB_PROC( PTABLE, GetFieldsInSQL )( CTEXTSTR cmd, int writestate);
// this is used to destroy the table returned by GetFieldsInSQL
SQLSTUB_PROC( void, DestroySQLTable )( PTABLE table );

// allow setting and getting of a bit of user data associated with the PODBC...
// though this can result in memory losses at the moment, cause there is no notification
// that the PODBC has gone away, and that the user needs to remove his data...
SQLSTUB_PROC( PTRSZVAL, SQLGetUserData )( PODBC odbc );
SQLSTUB_PROC( void, SQLSetUserData )( PODBC odbc, PTRSZVAL );

//--------------
// some internal stub-proxy linkage for generating
// remote responders..
typedef struct responce_tag
{
	struct {
		_32 bSingleLine : 1;
		_32 bMultiLine : 1;
		_32 bFields : 1;
	} flags;
	PVARTEXT result_single_line;
   int nLines;
	CTEXTSTR *pLines;
   CTEXTSTR *pFields;
} SQL_RESPONCE, *PSQL_RESPONCE;

typedef void (CPROC *result_responder)( int responce
									  , PSQL_RESPONCE result );


SQLSTUB_PROC( void, RegisterResponceHandler )( result_responder );

SQL_NAMESPACE_END
#ifdef __cplusplus
	using namespace sack::sql;
#endif
#endif



#if 0
/*

By default, CreateTable( CTEXTSTR tablename, CTEXTSTR filename ) which reads a 'create table' statement from a file to create a table, this now parses the create table structure into an internal structure TABLE which has FIELDs and DB_KEY_DEFs.  This structure is now passed to CheckODBCTable which is able to compare the structure with the table definition available from the database via DESCRIBE TABLE, and then update the table in the database to match the TABLE definition.

One can use the table structure to define tables instead of maintaining external files... and without having to create a temporary external file which could then contain a create table statement to create the table.

// declare some fields...
FIELD some_table_field_array_name[] = { { "field one", "int", NULL }
, { "field two", "varchar(100)", NULL }
, { "ID field", "int", "auto_increment" }
, { "some other field", "int", "NOT NULL default '8'" }
};

// define some keys...
DB_KEY_DEF some_table_key_array_name[] = { { .flags = { .bPrimary = 1 }, NULL, {"ID Field"} }
, { {0}, "namekey", { "field two", NULL } }
};

// the structure for DB_KEY_DEF takes an array of column names used to define the key, there should be a NULL to end the list.  The value after the array of field names is called 'null' which should always be set to NULL.  If these are declared in global data space, then any unset value will be initialized to zero.

TABLE some_table_var_name = { "table name", FIELDS( some_table_field_array_name ), TABLE_KEYS( some_table_key_array_name ), 1 );

 LOGICAL CheckODBCTable( PODBC odbc, PTABLE table, _32 options )
     PODBC odbc - may be left NULL to use the default database connection.
     PTABLE table - a pointer to a TABLE structure which has been initialized.
     _32 options - zero or more of  the following symbols or'ed together.
                #define CTO_MATCH 4  // attempt to figure out alter statements to drop or add columns to exact match definition
                #define CTO_MERGE 8  // attempt to figure out alter statements to add missing columns, do not drop.  Rename?



// Then some routine later
{
   ...
   CheckODBCTable( NULL, &some_table_var_name, CTO_MERGE );
   ..
}



------------------------------------------------------------


alternatively tables may be checked and updated using the following code, given an internal constant text string that is the create table statement, this may be parsed into a PTABLE structure which the resulting table can be used in CheckODBCTable();

static CTEXTSTR create_player_info = "CREATE TABLE `players_info` ("
"  `player_id` int(11) NOT NULL auto_increment,           "
"  PRIMARY KEY  (`player_id`),                            "
") TYPE=MyISAM;                                           ";

        PTABLE table = GetFieldsInSQL( create_player_info, FALSE );
      CheckODBCTable( NULL, table, CTO_MERGE );
    DestroySQLTable( table );

	 */
#endif
