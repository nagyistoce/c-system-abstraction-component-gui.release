#include <stdhdrs.h>
#include <sack_types.h>
#include <sharemem.h>
#include <procreg.h>
#include <filesys.h>
#include <system.h>
#include "sqlstruc.h"

SQL_NAMESPACE

static GLOBAL *global_sqlstub_data;
#define g (*global_sqlstub_data)
PRIORITY_PRELOAD( InitGlobal, GLOBAL_INIT_PRELOAD_PRIORITY )
{
   SimpleRegisterAndCreateGlobal( global_sqlstub_data );
}

#ifdef __cplusplus
using namespace sack::containers::BinaryTree;
using namespace sack::memory;
#endif

struct params
{
	CTEXTSTR name;
   PODBC odbc;
};

static int CPROC MyParmCmp( PTRSZVAL s1, PTRSZVAL s2 )
{
   struct params *p1 = (struct params*)s1;
	struct params *p2 = (struct params*)s2;
   if( p1->odbc == p2->odbc )
		return stricmp( p1->name, p2->name );
	else
      return 1;
}
static int CPROC MyStrCmp( PTRSZVAL s1, PTRSZVAL s2 )
{
	return stricmp( (TEXTCHAR*)s1, (TEXTCHAR*)s2 );
}

static PTREEROOT GetTableCache( PODBC odbc, CTEXTSTR tablename )
{
	static PTREEROOT tables;
	PTREEROOT newcache;
	struct params parameters;
	parameters.odbc = odbc;
   parameters.name = tablename;
   //lprintf( WIDE("Looking for name cache of table %s"), tablename );
	if( !tables )
	{
      //lprintf( WIDE("Creating initial tree.") );
		tables = CreateBinaryTreeExx( BT_OPT_NODUPLICATES
										 , MyParmCmp
										 , NULL );
	}
	if( !( newcache = (PTREEROOT)FindInBinaryTree( tables, (PTRSZVAL)&parameters ) ) )
	{
		struct params *saveparams = New( struct params );
		saveparams->name = StrDup( tablename );
      saveparams->odbc = odbc;
      //lprintf( WIDE("Failed to find entry, create new tree for cache") );
		AddBinaryNode( tables
						 , newcache = CreateBinaryTreeExx( BT_OPT_NODUPLICATES
																	, MyStrCmp
																	, NULL )
						 , (PTRSZVAL)saveparams );
	}
	//else
   //   lprintf( WIDE("Found tree cache...") );
   return newcache;
}


INDEX GetNameIndex(PODBC odbc, CTEXTSTR table,CTEXTSTR name)
{
/* this resulting truncation warning is OK. */
	return (INDEX)(((PTRSZVAL)FindInBinaryTree( GetTableCache( odbc, table ), (PTRSZVAL)name ))-1);

}

//-----------------------------------------------------------------------
SQLSTUB_PROC( INDEX, FetchLastInsertIDEx)( PODBC odbc, CTEXTSTR table, CTEXTSTR col DBG_PASS )
{
	INDEX RecordID = INVALID_INDEX;
	CTEXTSTR result = NULL;
	if( !odbc )
		odbc = g.odbc;
	if( !odbc )
		return INVALID_INDEX;
   //lprintf( "getting last insert ID?" );
#ifdef POSGRES_BACKEND
	{
		char query[256];
		sprintf( query, WIDE("select currval('%s_%s_seq')"), table, col );
		if( SQLQueryEx( odbc, query, &result ) && result DBG_RELAY )
		{
			RecordID=atoi( result );
			Release( result );
			while( GetSQLResult( &result ) );
		}
	}
#endif
#ifdef USE_SQLITE
	// extended sqlite functions with LAST_INSERT_ID() so the following code would work alos.
	if( odbc->flags.bSQLite_native )
	{
      // can also be done with 'select last_insert_rowid()'
		RecordID = sqlite3_last_insert_rowid( odbc->db );
	}
#endif
#ifdef USE_ODBC
	PushSQLQueryEx( odbc );
	if( odbc->flags.bAccess )
	{
		if( SQLQueryEx( odbc, "select @@IDENTITY", &result DBG_RELAY ) && result )
		//snprintf(sql,256,WIDE("SELECT %s FROM %s ORDER BY %s DESC"), col, table, col);
		//if( SQLQueryEx( odbc, sql, &result DBG_RELAY ) && result )
			RecordID=IntCreateFromText( result );
	}
	else if( odbc->flags.bODBC )
	{
		if( SQLQueryEx( odbc, WIDE("select LAST_INSERT_ID()"), &result DBG_RELAY ) && result )
		{
			//lprintf( WIDE("Result is %s"), result );
			RecordID=IntCreateFromText( result );
			//while( GetSQLResult( &result ) );
		}
	}
	PopODBCEx( odbc );
#endif
//
	return RecordID;
}



//-----------------------------------------------------------------------
SQLSTUB_PROC( INDEX, GetLastInsertIDEx)( CTEXTSTR table, CTEXTSTR col DBG_PASS )
{
   return FetchLastInsertIDEx( NULL, table, col DBG_RELAY );
}

//---------------------------------------------------------------------------

#undef EscapeBinary
#undef EscapeString

SQLSTUB_PROC( TEXTSTR,EscapeSQLBinaryEx )( PODBC odbc, CTEXTSTR blob, _32 bloblen DBG_PASS )
{
	int type_mysql = 1;

	TEXTCHAR *tmpnamebuf, *result;
	unsigned int n;
	int targetlen;

	if( odbc && ( odbc->flags.bSQLite || odbc->flags.bSQLite_native ) )
      type_mysql = 0;

	if( type_mysql )
	{
	n = 0;

	targetlen = 0;
	while( n < bloblen )
	{
		if( blob[n] == '\'' ||
			blob[n] == '\\' ||
			blob[n] == '\0' ||
			blob[n] == '\"' )
			targetlen++;
		n++;
	}

	n = 0;

	result = tmpnamebuf = (TEXTSTR)AllocateEx( targetlen + bloblen + 1 DBG_RELAY );

	while( n < bloblen )
	{
		if( blob[n] == '\'' ||
			blob[n] == '\\' ||
			blob[n] == '\0' ||
			blob[n] == '\"' )
			(*tmpnamebuf++) = '\\';
		if( blob[n] )
			(*tmpnamebuf++) = blob[n];
		else
			(*tmpnamebuf++) = '0';
		n++;
	}
	(*tmpnamebuf) = 0; // best terminate this thing.
	}
	else
	{
		n = 0;

		targetlen = 0;
		while( n < bloblen )
		{
			if( blob[n] == '\'' )
				targetlen++;
			n++;
		}

		n = 0;

		result = tmpnamebuf = (TEXTSTR)AllocateEx( targetlen + bloblen + 1 DBG_RELAY );

		while( n < bloblen )
		{
			if( blob[n] == '\'' )
				(*tmpnamebuf++) = '\'';
			(*tmpnamebuf++) = blob[n];
			n++;
		}
		(*tmpnamebuf) = 0; // best terminate this thing.
	}
	return result;
}

SQLSTUB_PROC( TEXTSTR,EscapeBinaryEx )( CTEXTSTR blob, _32 bloblen DBG_PASS )
{
   return EscapeSQLBinaryEx( NULL, blob, bloblen DBG_RELAY );
}


SQLSTUB_PROC( TEXTCHAR *,EscapeBinary )( CTEXTSTR blob, _32 bloblen )
{
	return EscapeBinaryEx( blob, bloblen DBG_SRC );
}

//---------------------------------------------------------------------------

SQLSTUB_PROC( TEXTCHAR *,EscapeStringEx )( CTEXTSTR name DBG_PASS )
{
// cpg29dec2006 c:\work\sack\src\sqllib\sqlutil.c(160) : warning C4267: 'argument' : conversion from 'size_t' to 'sack::_32', possible loss of data
// cpg29dec2006    return EscapeBinaryEx( name, strlen( name ) DBG_RELAY );

   return EscapeBinaryEx( name, (_32)strlen( name ) DBG_RELAY );
}

SQLSTUB_PROC( TEXTCHAR *,EscapeSQLStringEx )( PODBC odbc, CTEXTSTR name DBG_PASS )
{
   return EscapeSQLBinaryEx( odbc, name, (_32)strlen( name ) DBG_RELAY );
}

SQLSTUB_PROC( TEXTCHAR *,EscapeString )( CTEXTSTR name )
{
// cpg29dec2006  c:\work\sack\src\sqllib\sqlutil.c(160) : warning C4267: 'argument' : conversion from 'size_t' to 'sack::_32', possible loss of data
// cpg29dec2006    return EscapeBinaryEx( name, strlen( name ) DBG_SRC );

   return EscapeBinaryEx( name, (_32)strlen( name ) DBG_SRC );
}

_8 hexbyte( TEXTCHAR *string )
{
	static TEXTCHAR hex[17] = WIDE("0123456789abcdef");
	static TEXTCHAR HEX[17] = WIDE("0123456789ABCDEF");
	TEXTCHAR *digit;
	_8 value = 0;

	digit = strchr( hex, string[0] );
	if( !digit )
	{
		digit = strchr( HEX, string[0] );
		if( digit )
		{
//cpg 19 Jan 2007 1>c:\work\sack\src\sqllib\sqlutil.c(187) : warning C4244: '=' : conversion from '__w64 int' to 'sack::_8', possible loss of data
			value = digit - HEX;
		}
		else
         return 0;
	}
	else
	{
//cpg 19 Jan 2007 1>c:\work\sack\src\sqllib\sqlutil.c(194) : warning C4244: '=' : conversion from '__w64 int' to 'sack::_8', possible loss of data
		value = digit - hex;
	}

	value *= 16;
	digit = strchr( hex, string[1] );
	if( !digit )
	{
		digit = strchr( HEX, string[1] );
		if( digit )
		{
//cpg 19 Jan 2007 1>c:\work\sack\src\sqllib\sqlutil.c(204) : warning C4244: '+=' : conversion from '__w64 int' to 'sack::_8', possible loss of data
			value += digit - HEX;
		}
		else
			return 0;
	}
	else
	{
//cpg 19 Jan 2007 1>c:\work\sack\src\sqllib\sqlutil.c(211) : warning C4244: '+=' : conversion from '__w64 int' to 'sack::_8', possible loss of data
		value += digit - hex;
	}
	return value;
}

TEXTSTR DeblobifyString( CTEXTSTR blob, TEXTSTR outbuf, int outbuflen  )
{

	TEXTCHAR *result;
	TEXTCHAR *x, *y;

	if( blob )
	{
		//lprintf("got blob %s", blob);
		if( !outbuf )
			outbuf = (TEXTSTR)Allocate( ( strlen( blob ) / 2 ) + 1 );
		result = outbuf;
		for( x=(TEXTSTR)blob, y = result;
			  x[0] && ((y-outbuf) < outbuflen);
			  y++, x+=2 )
		{
			y[0] = hexbyte( x );
			y[1] = 0;
         //lprintf("y is %s", y);
		}
      //lprintf("returning %s", result );
		return result;
	}
	else
		lprintf( WIDE("Duh.  No Blob.") );
	return NULL;
}

//---------------------------------------------------------------------------

TEXTSTR RevertEscapeBinary( CTEXTSTR blob, _32 *bloblen )
{
	TEXTCHAR *tmpnamebuf, *result;
	int n;
	int escape;
	int targetlen;
	n = 0;

	escape = 0;
	targetlen = 0;
	for( n = 0; blob[n]; n++ )
	{
		if( !escape && ( blob[n] == '\\' ) )
			escape = 1;
		else if( escape )
		{
			if( blob[n] == '\\' ||
				blob[n] == '0' ||
				blob[n] =='\'' ||
				blob[n] == '\"' )
			{
            // targetlen is a subtraction for missing charactercount
            targetlen++;
			}
         escape = 0;
		}
      n++;
	}
	if( bloblen )
	{
		(*bloblen) = n - targetlen;
	}

	escape = 0;
	result = tmpnamebuf = (TEXTCHAR*)Allocate( (*bloblen) );
	for( n = 0; blob[n]; n++ )
	{
		if( !escape && ( blob[n] == '\\' ) )
			escape = 1;
		else if( escape )
		{
			if( blob[n] == '\\' ||
				blob[n] =='\'' ||
				blob[n] == '\"' )
			{
			// targetlen is a subtraction for missing charactercount
				(*tmpnamebuf++) = blob[n];
			}
			else if( blob[n] == '0' )
				(*tmpnamebuf++) = 0;
			else
			{
				(*tmpnamebuf++) = '\\';
				(*tmpnamebuf++) = blob[n];
			}
			escape = 0;
		}
		else
			(*tmpnamebuf++) = blob[n];
	}

   (*tmpnamebuf) = 0; // best terminate this thing.
   return result;
}

//---------------------------------------------------------------------------

TEXTSTR RevertEscapeString( CTEXTSTR name )
{
   return RevertEscapeBinary( name, NULL );
}

//---------------------------------------------------------------------------

SQLSTUB_PROC( INDEX, SQLReadNameTableExEx)( PODBC odbc, CTEXTSTR name, CTEXTSTR table, CTEXTSTR col, CTEXTSTR namecol, int bCreate DBG_PASS )
{
			TEXTCHAR query[256];
			TEXTCHAR *tmp;
			CTEXTSTR result = NULL;
			INDEX IDName = INVALID_INDEX;
			if( !table || !name )
				return INVALID_INDEX;

			// look in internal cache first...
			IDName = GetNameIndex(odbc,table,name);
			if( IDName != INVALID_INDEX )
				return IDName;

			PushSQLQueryEx( odbc );
			tmp = EscapeSQLStringEx( odbc, name DBG_RELAY );
			snprintf( query, sizeof( query ), WIDE("select %s from %s where %s like \'%s\'"), col?col:WIDE("id"), table, namecol, tmp );
			Release( tmp );
			if( SQLQueryEx( odbc, query, &result DBG_RELAY) && result )
			{
				IDName = IntCreateFromText( result );
			}
			else if( bCreate )
			{
            TEXTSTR newval = EscapeSQLString( odbc, name );
				snprintf( query, sizeof( query ), WIDE("insert into %s (%s) values( \'%s\' )"), table, namecol, newval );
				if( !SQLCommandEx( odbc, query DBG_RELAY ) )
				{
					lprintf( WIDE("insert failed, how can we define name %s?"), name );
					// inser failed...
				}
				else
				{
					// all is well.
					IDName = FetchLastInsertIDEx( odbc, table, col?col:WIDE("id") DBG_RELAY );
				}
            Release( newval );
			}
			else
				IDName = INVALID_INDEX;

			PopODBCEx(odbc);

			if( IDName != INVALID_INDEX )
			{
				TEXTCHAR *tmp;
            // instead of strdup, consider here using SaveName from procreg?
				AddBinaryNode( GetTableCache(odbc,table), (POINTER)((PTRSZVAL)(IDName+1))
								 , (PTRSZVAL)StrDup(tmp = EscapeString( name )) );
				Release( tmp );
			}
			return IDName;
}

SQLSTUB_PROC( INDEX, ReadNameTableExEx)( CTEXTSTR name, CTEXTSTR table, CTEXTSTR col, CTEXTSTR namecol, int bCreate DBG_PASS )
{
	return SQLReadNameTableExEx( NULL, name, table,col,namecol,bCreate DBG_RELAY );
}

//---------------------------------------------------------------------------

SQLSTUB_PROC( INDEX, ReadNameTableEx)( CTEXTSTR name, CTEXTSTR table, CTEXTSTR col DBG_PASS )
{
   return ReadNameTableExEx( name,table,col,WIDE("name"),TRUE DBG_RELAY);
}

//---------------------------------------------------------------------------

SQLSTUB_PROC( int, ReadFromNameTableEx )( INDEX id, CTEXTSTR table, CTEXTSTR id_colname, CTEXTSTR name_colname, CTEXTSTR *result DBG_PASS)
{
	TEXTCHAR query[256];
	if( !result || !table || id == INVALID_INDEX )
		return FALSE;
	// the tree locally cached is in NAME order, but the data is
	// the key, so we would have to scan the tree otherwise both directions
   // keyed so that we could get the name key from the ID data..
	snprintf( query, sizeof( query ), WIDE("select %s from %s where %s=%") _32f
			  , name_colname?name_colname:WIDE("name")
			  , table
			  , id_colname?id_colname:WIDE("id")
			  , id );
	if( !DoSQLQueryEx( query, result DBG_RELAY ) || !(*result) )
	{
		lprintf( WIDE("name ID(%") _32f WIDE(" as %s) was not found in %s.%s"), id, table, id_colname?id_colname:WIDE("id") );
		return FALSE;
	}
	else
	{
		//PopODBC();
	}
	return TRUE;
}

//---------------------------------------------------------------------------

SQLSTUB_PROC( int, ReadFromNameTableExEx )( INDEX id, CTEXTSTR table, CTEXTSTR id_col, CTEXTSTR colname, CTEXTSTR *result DBG_PASS)
{
	TEXTCHAR query[256];
	if( !result || !table || id == INVALID_INDEX )
		return FALSE;
	// the tree locally cached is in NAME order, but the data is
	// the key, so we would have to scan the tree otherwise both directions
   // keyed so that we could get the name key from the ID data..
	snprintf( query, sizeof( query ), WIDE("select %s from %s where %s=%") _32f
			, colname
			  , table
			 , id_col?id_col:WIDE("id")
			  , id );
	if( !DoSQLQueryEx( query, result DBG_RELAY ) || !(*result) )
	{
		lprintf( WIDE("name ID(%") _32f WIDE(") was not found in %s.%s"), id, table, colname?colname:WIDE("id") );
		return FALSE;
	}
	else
	{
		//PopODBC();
	}
	return TRUE;
}

//---------------------------------------------------------------------------

SQLSTUB_PROC( int, SQLCreateTableEx )( PODBC odbc, CTEXTSTR filename, CTEXTSTR templatename, CTEXTSTR tablename, _32 options )
{
	//CTEXTSTR result;
	//TEXTCHAR query[256];
	if( !tablename )
		tablename = templatename;
	//if( odbc->flags.bSQLite_native )
	//	;
	//else if( odbc->flags.bAccess )
	//	snprintf( query, 256, WIDE("DESCRIBE [%s]"), tablename );
	//else
	//	snprintf( query, 256, WIDE("DESCRIBE `%s`"), tablename );
	//if( !SQLQuery( odbc, query, &result ) || !result || (options & (CTO_DROP|CTO_MATCH|CTO_MERGE)) )
	{
		TEXTCHAR sec_file[284];
		FILE *file;
      sec_file[0] = 0;
		Fopen( file, filename, WIDE("rt") );
		if( !file )
		{
			if( !pathchr( filename ) )
			{
            CTEXTSTR path = OSALOT_GetEnvironmentVariable( "MY_LOAD_PATH" );
				snprintf( sec_file, sizeof( sec_file ), "%s/%s", path, filename );
            Fopen( file, sec_file, WIDE("rt") );
			}
		}
		if( file )
		{
			int done = FALSE;
			int gathering = FALSE;
			TEXTCHAR *buf;
			char fgets_buf[1024];
			TEXTCHAR cmd[1024];
			int nOfs = 0;
         if( !odbc->flags.bNoLogging )
				lprintf( WIDE("Opened %s to read for table %s(%s)"), sec_file[0]?sec_file:filename, tablename,templatename );
			while( fgets( fgets_buf, sizeof( fgets_buf ), file ) )
			{
				TEXTCHAR *p;
#ifdef __cplusplus_cli
				buf = DupCStr( fgets_buf );
#else
				buf = fgets_buf;
#endif
				p = buf + strlen( buf ) - 1;
				while( p[0] == ' ' || p[0] == '\t' || p[0] == '\n' || p[0] == '\r')
				{
					p[0] = 0;
					p--;
				}
				p = buf;
				while( p[0] )
				{
					if ( p[0] == '#' )
					{
						p[0] = 0;
						break;
					}
					p++;
				}
				p = buf;
				while( p[0] == ' ' || p[0] == '\t' )
					p++;
				if( p[0] )
				{
					// have content on the line...
					TEXTCHAR *end = p + strlen( p ) - 1;
					if( gathering && end[0] == ';' )
					{
						done = TRUE;
						end[0] = 0;
					}
					if( !gathering )
					{
						if( strnicmp( p, WIDE("CREATE"), 6 ) == 0 )
						{
							CTEXTSTR tabname;
							// cpg29dec2006  c:\work\sack\src\sqllib\sqlutil.c(498) : warning C4267: 'initializing' : conversion from 'size_t' to 'int', possible loss of data
							// cpg29dec2006                             int len = strlen( tablename );
							int len = (int)strlen( tablename );
							if( (tabname = StrStr( p, templatename )) &&
								(tabname[len] == '`' || tabname[len] ==' ' || tabname[len] =='\t' ) &&
								(tabname[-1] == '`' || tabname[-1] ==' ' || tabname[-1] =='\t'))
							{
								// need to gather, repace...
								TEXTCHAR line[1024];
								CTEXTSTR trailer;
								trailer = tabname;
								while( trailer[0] != '\'' &&
										trailer[0] != '`' &&
										trailer[0] != ' ' &&
										trailer[0] != '\t' )
									trailer++;
								sprintf( line, WIDE("%*.*s%s%s")
										 , (int)(tabname - p), (int)(tabname - p), p
										 , templatename
										 , trailer
										 );
								strcpy( buf, line );
								gathering = TRUE;
							}
							else
							{

							}
						}
					}
					if( gathering )
					{
						nOfs += sprintf( cmd + nOfs, WIDE("%s "), p );
						if( done )
						{
							if( options & CTO_DROP )
							{
								TEXTCHAR buf[1024];
								snprintf( buf, 1024, WIDE("Drop table %s"), templatename );
								if( !SQLCommand( odbc, buf ) )
								{
									CTEXTSTR result;
									GetSQLError( &result );
									lprintf( WIDE("Failed to do drop: %s"), result );
								}
							}
							// result is set with the first describe result
							// the matching done in CheckMySQLTable should
							// be done here, after parsing the line from the file (cmd)
							// into a TABLE structure.
							//DebugBreak();
							{
								PTABLE table;
								table = GetFieldsInSQL( cmd , 0 );
								CheckODBCTable( odbc, table, options );
								DestroySQLTable( table );
							}
							break;
						}
					}
				}
#ifdef __cplusplus_cli
				Release( buf );
#endif
			}
			//lprintf( WIDE("Done with create...") );
			fclose( file );
		}
		else
		{
			lprintf( WIDE("Unable to open templatefile: %s or %s/%s"), filename
					 , OSALOT_GetEnvironmentVariable( "MY_LOAD_PATH" )
                 , filename );

		}
	}
	return TRUE;
}

//---------------------------------------------------------------------------

void DestroySQLTable( PTABLE table )
{
	int n;
	int m;
// don't release tables created statically in C files...
	if( !table->flags.bDynamic )
      return;
	for( n = 0; n < table->fields.count; n++ )
	{
		Release( (POINTER)table->fields.field[n].name );
		Release( (POINTER)table->fields.field[n].type );
		Release( (POINTER)table->fields.field[n].extra );
		for( m = 0; table->fields.field[n].previous_names[m] && m < MAX_PREVIOUS_FIELD_NAMES; m++ )
		{
         Release( (POINTER)table->fields.field[n].previous_names[m] );
		}
	}
	for( n = 0; n < table->keys.count; n++ )
	{
		for( m = 0; table->keys.key[n].colnames[m] && m < MAX_KEY_COLUMNS; m++ )
		{
			Release( (POINTER)table->keys.key[n].colnames[m] );
		}
		Release( (POINTER)table->keys.key[n].name );
	}
	Release( (POINTER)table->fields.field );
	Release( (POINTER)table->keys.key );
	Release( table );
}

void DumpSQLTable( PTABLE table )
{
	int n;
	int m;
	//if( 1 )
	//   return;
	// don't release tables created statically in C files...
	lprintf( "Table name: %s", table->name );
	for( n = 0; n < table->fields.count; n++ )
	{
		lprintf( "Column %d '%s' [%s] [%s]"
              , n
				 ,( (POINTER)table->fields.field[n].name )
				 ,( (POINTER)table->fields.field[n].type )
				 ,( (POINTER)table->fields.field[n].extra )
				 );
		for( m = 0; table->fields.field[n].previous_names[m] && m < MAX_PREVIOUS_FIELD_NAMES; m++ )
		{
         //Release( (POINTER)table->fields.field[n].previous_names[m] );
		}
	}
	for( n = 0; n < table->keys.count; n++ )
	{
		lprintf( "Key %s", table->keys.key[n].name?table->keys.key[n].name:"<NONAME>" );
		for( m = 0; table->keys.key[n].colnames[m] && m < MAX_KEY_COLUMNS; m++ )
		{
			lprintf( "Key part = %s"
					 , ( (POINTER)table->keys.key[n].colnames[m] )
					 );
		}
	}
}


SQLSTUB_PROC( int, CreateTableEx )( CTEXTSTR filename, CTEXTSTR templatename, CTEXTSTR tablename, _32 options )
{
   OpenSQL();
   if( g.odbc )
		return SQLCreateTableEx( g.odbc, filename, templatename, tablename, options );
   return FALSE;
}

#define FAILPARSE() do { if( ( start[0] < '0' ) || ( start[0] > '9' ) ) {  \
lprintf( WIDE("string failes date parsing... %s"), timestring );                  \
return 0; } } while (0);

SQLSTUB_PROC( int, ConvertDBTimeString )( CTEXTSTR timestring
                                        , CTEXTSTR *endtimestring
													 , int *pyr, int *pmo, int *pdy
													 , int *phr, int *pmn, int *psc )
{
	int mo,dy,yr;
   int hr = 0,mn = 0,sc = 0;
	CTEXTSTR start;
	start = timestring;
	if( !start )
	{
      if( pyr ) (*pyr) = 0;
      if( pmo ) (*pmo) = 0;
      if( pdy ) (*pdy) = 0;
      if( phr ) (*phr) = 0;
      if( pmn ) (*pmn) = 0;
      if( psc ) (*psc) = 0;
		return 0;
	}
	yr = 0;
   FAILPARSE();
	yr = (yr * 10) + (*start++) - '0';
   FAILPARSE();
	yr = (yr * 10) + (*start++) - '0';
   FAILPARSE();
	yr = (yr * 10) + (*start++) - '0';
   FAILPARSE();
	yr = (yr * 10) + (*start++) - '0';
	if( (*start) == '-' ) start++;
	mo = 0;
   FAILPARSE();
	mo = (mo * 10) + (*start++) - '0';
   FAILPARSE();
	mo = (mo * 10) + (*start++) - '0';
	if( (*start) == '-' ) start++;
	dy = 0;
   FAILPARSE();
	dy = (dy * 10) + (*start++) - '0';
   FAILPARSE();
	dy = (dy * 10) + (*start++) - '0';
	if( (*start) == ' ' )
	{
		start++;
		FAILPARSE();
		hr = (hr * 10) + (*start++) - '0';
		FAILPARSE();
		hr = (hr * 10) + (*start++) - '0';
		if( (*start) == ':' ) start++;
		FAILPARSE();
		mn = (mn * 10) + (*start++) - '0';
		FAILPARSE();
		mn = (mn * 10) + (*start++) - '0';
		if( (*start) == ':' ) start++;
		FAILPARSE();
		sc = (sc * 10) + (*start++) - '0';
		FAILPARSE();
		sc = (sc * 10) + (*start++) - '0';
	}
	if( endtimestring )
		*endtimestring = start;
	if( pyr )
		*pyr = yr;
	if( pmo )
		*pmo = mo;
	if( pdy )
		*pdy = dy;
	if( phr )
		*phr = hr;
	if( pmn )
		*pmn = mn;
	if( psc )
		*psc = sc;
   return 1;
}

LOGICAL CheckAccessODBCTable( PODBC odbc, PTABLE table, _32 options )
{
	CTEXTSTR *_fields = NULL;
	CTEXTSTR *fields;
	int columns;
	PVARTEXT pvtCreate = NULL;
	TEXTCHAR *cmd = WIDE("select top 1 * from [%s]");
	int retry = 0;
retry:
	if( SQLRecordQueryf( odbc, &columns, NULL, &_fields, cmd, table->name ) )
	{
		int n;
		fields = NewArray( CTEXTSTR, columns );
		for( n = 0; n < columns; n++ )
			fields[n] = StrDup( _fields[n] );
		for( n = 0; n < columns; n++ )
		{
			int m;
			for( m = 0; m < table->fields.count; m++ )
			{
				if( stricmp( fields[n], table->fields.field[m].name ) == 0 )
					break;
			}
			if( m == table->fields.count )
			{
				// did not find this column in the definition drop it.
				{
					for( m = 0; m < table->fields.count; m++ )
					{
						int prev;
						for( prev = 0; table->fields.field[m].previous_names[prev]; prev++ )
						{
							if( strcmp( table->fields.field[m].previous_names[prev]
										 , fields[n] ) == 0 )
							{
                        break;
							}
						}
						if( table->fields.field[m].previous_names[prev] )
                     break;
					}
					if( m < table->fields.count )
					{
						// this column was known to be named something else
						// and we should do some extra fun stuff to preserve the data
						// In access rename is done with DROP and ADD column
						ReleaseODBC( odbc ); // release so that the alter statement may be done.
						SQLCommandf( odbc
									  , WIDE("alter table [%s] add column [%s] %s%s%s")
									  , table->name
									  , table->fields.field[m].name
									  , table->fields.field[m].type
									  , table->fields.field[m].extra?WIDE(" "):WIDE("")
									  , table->fields.field[m].extra?table->fields.field[m].extra:WIDE("")
									  );
						SQLCommandf( odbc, WIDE("update [%s] set [%s]=[%s]")
									  , table->name
									  , table->fields.field[m].name
									  , fields[n] );
					}
					ReleaseODBC( odbc ); // release all prior locks on the table...
					SQLCommandf( odbc
								  , WIDE("alter table [%s] drop column [%s]")
								  , table->name
								  , fields[n] );

					if( m < table->fields.count )
					{
						/* this field is now handled and done, forget it.*/
						Release( (void*)fields[n] );
						// okay we already added this one, so make it
						// match our definition... otherwise following code will
						// also attempt to add this...
						// but in the process of dropping we may NOT
                  // drop a renamed column- but must instead preserve it's data
						fields[n] = StrDup( table->fields.field[m].name );
					}
				}
			}
		}

		for( n = 0; n < table->fields.count; n++ )
		{
			int m;
			for( m = 0; m < columns; m++ )
			{
            if( fields[m] )
					if( stricmp( fields[m], table->fields.field[n].name ) == 0 )
						break;
			}
			if( m == columns )
			{
				// did not find this defined column in the table, add it.
				PTEXT cmd;
				if( !pvtCreate )
					pvtCreate = VarTextCreate();
				vtprintf( pvtCreate, WIDE("alter table [%s] add column [%s] %s%s%s")
						  , table->name
						  , table->fields.field[n].name
						  , table->fields.field[n].type
						  , table->fields.field[n].extra?WIDE(" "):WIDE("")
						  , table->fields.field[n].extra?table->fields.field[n].extra:WIDE("")
						  );
				cmd = VarTextGet( pvtCreate );
				// close all prior statement handles so it's not locked
				// especially my own.
	            PopODBCEx( odbc ); // release so that the alter statement may be done.
				SQLCommand( odbc, GetText( cmd ) );
				LineRelease( cmd );
			}
		}
		// release the duplicated fields...
		for( n = 0; n < columns; n++ )
			Release( (POINTER)fields[n] );
		Release( fields );
	}
	else
	{
		// table doesn't exist?
		CTEXTSTR error = NULL;
		FetchSQLError( odbc, &error );
		if( StrCmpEx( error, WIDE("(37000)"), 7 ) == 0 )
		{
			// ODBC driver is old and does not support
			// 'TOP' command... please try again, using a less fancy
			// select... since it's file based, probably the data is not
         // all read, but one row at a time is read from the database.
			if( !retry )
			{
				cmd = WIDE("select * from [%s]");
				retry++;
				goto retry;
			}
		}
		if( StrCmpEx( error, WIDE("(S0002)"), 7 ) == 0 )
		{
         PTEXT cmd;
			int n;
         int first = 1;
			if( !pvtCreate )
				pvtCreate = VarTextCreate();
			vtprintf( pvtCreate, WIDE("create table [%s] ("), table->name );
			for( n = 0; n < table->fields.count; n++ )
			{
				CTEXTSTR type;
				if( StrCaseCmpEx( table->fields.field[n].type, "varchar", 7 ) == 0 )
					type = "TEXT";
				else if( StrCaseCmpEx( table->fields.field[n].type, "tinyint", 7 ) == 0 )
					type = "INT";
				else if( StrCaseCmpEx( table->fields.field[n].type, "int(", 4 ) == 0 )
					type = "INT";
				else
				{
					if( table->fields.field[n].extra && strstr( table->fields.field[n].extra, "auto_increment" ) )
						type = "COUNTER";
					else
						type = table->fields.field[n].type;
				}
				if( strchr( table->fields.field[n].name, ' ' ) )
				{
					vtprintf( pvtCreate, WIDE("%s[%s] %s%s%s")
							  , first?"":","
							  , table->fields.field[n].name
							  , type
							  , ""//table->fields.field[n].extra?" ":""
							  , ""//table->fields.field[n].extra?table->fields.field[n].extra:""
							  );
				}
            else
				{
					vtprintf( pvtCreate, WIDE("%s[%s] %s%s%s")
							  , first?"":","
							  , table->fields.field[n].name
							  , type
							  , ""//(strstr( table->fields.field[n].extra, "auto_increment" ))?"COUNTER":""
							  , ""//table->fields.field[n].extra?table->fields.field[n].extra:""
							  );
				}
				first = 0;
			}
			// not even sure where in the syntax key fields go...
			// does access actually have key fields?  or just things
         // called key fields
			//for( n = 0; n < table->keys.count; n++ )
			//{
            // for implementation see Check MYSQL
			//}
			vtprintf( pvtCreate, WIDE(")") );
			cmd = VarTextGet( pvtCreate );
			SQLCommand( odbc, GetText( cmd ) );
			LineRelease( cmd );
		}
		else
		{
			lprintf( WIDE("error is : %s"), error );
		}
	}
	if( pvtCreate )
		VarTextDestroy( &pvtCreate );
   return 1;
}

LOGICAL CPROC CheckMySQLODBCTable( PODBC odbc, PTABLE table, _32 options )
{
// when this gets to be implemented...
// the type "counter" needs to be interpreted as auto increment.
// also, the behavior for mysql auto increment (in extra fields )
// needs to be interpreted counter-intuitively for access databases..

	CTEXTSTR *fields = NULL;
	CTEXTSTR *result = NULL;
   FILE *f_odbc = NULL;
	int columns;
	int retry;
   int success;
	int buflen;
	PVARTEXT pvtCreate = NULL;
							 //char *cmd = "select * from %s limit 1";
	TEXTCHAR *cmd;
	if( options & CTO_LOG_CHANGES )
	{
		f_odbc = fopen( "changes.sql", "at+" );
      if( !f_odbc )
			f_odbc = fopen( "changes.sql", "wt" );
	}
	cmd = NewArray( TEXTCHAR, 1024);
	buflen = 0;
#ifdef USE_SQLITE
	if( odbc->flags.bSQLite_native )
		buflen += snprintf( cmd+buflen , 1024-buflen ,WIDE("select tbl_name,sql from sqlite_master where type='table' and name='%s'")
								, table->name );
	else
#endif
		buflen += snprintf( cmd+buflen , 1024-buflen ,WIDE("show create table `%s`") ,table->name);
	retry = 0;
retry:
   PushSQLQueryEx( odbc );
	if( ( success = SQLRecordQueryf( odbc, &columns, &result, &fields, cmd, table->name ) )
		&& result )
			//    if( DoSQLQuery( cmd, &result ) && result )
	{
		int n;
		PTABLE pTestTable;
		//lprintf("Does this work or not?");
		pTestTable = GetFieldsInSQL( result[1] , 0 );
		//lprintf(" ---------------Table to test-----------------------------------------" );
		//DumpSQLTable( pTestTable );
		//lprintf(" ---------------original table -----------------------------------------" );
		//DumpSQLTable( table );
		//lprintf(" -----------------end tables ---------------------------------------" );
		//lprintf(" . . . I guess so");
		for( n = 0; n < pTestTable->fields.count; n++ )
		{
			int m;
			for( m = 0; m < table->fields.count; m++ )
			{
				if( stricmp( pTestTable->fields.field[n].name
							  , table->fields.field[m].name ) == 0 )
					break;
			}
			if( m == table->fields.count )
			{
			// did not find this column in the definition drop it.
				{
					int prev = 0;
					for( m = 0; m < table->fields.count; m++ )
					{
						for( prev = 0; table->fields.field[m].previous_names[prev]; prev++ )
						{
							if( strcmp( table->fields.field[m].previous_names[prev]
										 , pTestTable->fields.field[n].name ) == 0 )
							{
								break;
							}
						}
						if( table->fields.field[m].previous_names[prev] )
							break;
					}
					if( m < table->fields.count )
					{
					// this column was known to be named something else
					// and we should do some extra fun stuff to preserve the data
					// In access rename is done with DROP and ADD column
						if( options & CTO_DROP )
						{
							if( f_odbc )
								fprintf( f_odbc, WIDE("drop table `%s`;\n"), table->name );
							else
								SQLCommandf( odbc, WIDE("drop table `%s`"), table->name );
							goto do_create_table;
						}
						ReleaseODBC( odbc ); // release so that the alter statement may be done.
						if( f_odbc )
						{
							fprintf( f_odbc
									 , WIDE("alter table `%s` add column `%s` %s%s%s;\n")
									 , table->name
									 , table->fields.field[m].name
									 , table->fields.field[m].type
									 , table->fields.field[m].extra?WIDE(" "):WIDE("")
									 , table->fields.field[m].extra?table->fields.field[m].extra:WIDE("")
									 );
							fprintf( f_odbc, WIDE("update `%s` set `%s`=`%s`;\n")
									 , table->name
									 , table->fields.field[m].name
									 , table->fields.field[m].previous_names[prev] );
						}
						else
						{
							SQLCommandf( odbc
										  , WIDE("alter table `%s` add column `%s` %s%s%s")
										  , table->name
										  , table->fields.field[m].name
										  , table->fields.field[m].type
										  , table->fields.field[m].extra?WIDE(" "):WIDE("")
										  , table->fields.field[m].extra?table->fields.field[m].extra:WIDE("")
										  );
							SQLCommandf( odbc, WIDE("update `%s` set `%s`=`%s`")
										  , table->name
										  , table->fields.field[m].name
										  , table->fields.field[m].previous_names[prev] );
						}
					}
					ReleaseODBC( odbc ); // release all prior locks on the table...
					if( !( options & CTO_MERGE ) )
					{
						if( options & CTO_DROP )
						{
							if( f_odbc )
								fprintf( f_odbc, WIDE("drop table `%s`"), table->name );
							else
								SQLCommandf( odbc, WIDE("drop table `%s`"), table->name );
							goto do_create_table;
						}
						if( f_odbc )
							fprintf( f_odbc
										  , WIDE("alter table `%s` drop column `%s`;\n")
										  , table->name
										  , pTestTable->fields.field[n].name );
						else
							SQLCommandf( odbc
										  , WIDE("alter table `%s` drop column `%s`")
										  , table->name
										  , pTestTable->fields.field[n].name );

						if( m < table->fields.count )
						{
						/* this field is now handled and done, forget it.*/
							Release( (void*)pTestTable->fields.field[n].name );
						// okay we already added this one, so make it
						// match our definition... otherwise following code will
						// also attempt to add this...
						// but in the process of dropping we may NOT
						// drop a renamed column- but must instead preserve it's data
							pTestTable->fields.field[n].name = StrDup( table->fields.field[m].name );
						}
					}
				}
			}
		}

		for( n = 0; n < table->fields.count; n++ )
		{
			int m;
			for( m = 0; m < pTestTable->fields.count; m++ )
			{
			//                if( fields[m] )
				if( stricmp( pTestTable->fields.field[m].name, table->fields.field[n].name ) == 0 )
					break;
			}
			if( m == pTestTable->fields.count )
			{
			// did not find this defined column in the table, add it.
				PTEXT txt_cmd;
				if( options & CTO_DROP )
				{
					if( f_odbc )
						fprintf( f_odbc, WIDE("drop table `%s`"), table->name );
					else
						SQLCommandf( odbc, WIDE("drop table `%s`"), table->name );
					goto do_create_table;
				}
				if( !pvtCreate )
					pvtCreate = VarTextCreate();
				vtprintf( pvtCreate, WIDE("alter table `%s` add column `%s` %s%s%s")
						  , table->name
						  , table->fields.field[n].name
						  , table->fields.field[n].type
						  , table->fields.field[n].extra?WIDE(" "):WIDE("")
						  , table->fields.field[n].extra?table->fields.field[n].extra:WIDE("")
						  );
				txt_cmd = VarTextGet( pvtCreate );
				ReleaseODBC( odbc ); // release so that the alter statement may be done.
				if( f_odbc )
					fprintf( f_odbc, "%s;\n", GetText( txt_cmd ) );
				else
					SQLCommand( odbc, GetText( txt_cmd ) );
				LineRelease( txt_cmd );
			}
		}
						  /* here check the pTable->keys.count vs pTestTable.keys.count
							* ... to figure out whether to add or drop keys...
							*/
						  /*        for( n = 0;n<table->keys.count; n++ )
							{
							int m;
							for( m=0;m<pTestTable->keys.count; m++ )
							{
							if( fields[m]
							*/
		PopODBCEx(odbc);
		DestroySQLTable( pTestTable );
	}
	else
	{
		// table doesn't exist?
		if( success && !result )
         goto do_create_table;
		if( !success )
		{
			CTEXTSTR error;
			error = NULL;
			FetchSQLError( odbc, &error );
			if( StrCmpEx( error, WIDE("(37000)"), 7 ) == 0 )
			{
				// ODBC driver is old and does not support
				// 'TOP' command... please try again, using a less fancy
				// select... since it's file based, probably the data is not
				// all read, but one row at a time is read from the database.
				if( !retry )
				{
					strcpy( cmd, WIDE("select * from `%s`") );
					retry++;
					goto retry;
				}
			}
			if( StrCmpEx( error, WIDE("(S0002)"), 7 ) == 0 )
			{
				PTEXT txt_cmd;
				int n;
				int first;
				CTEXTSTR auto_increment_column;
			do_create_table:
				auto_increment_column = NULL;
				first = 1;
				if( !pvtCreate )
					pvtCreate = VarTextCreate();
				vtprintf( pvtCreate, WIDE("create table `%s` ("), table->name );
				for( n = 0; n < table->fields.count; n++ )
				{
#ifdef USE_SQLITE
					if( odbc->flags.bSQLite_native )
					{
						if( table->fields.field[n].extra
							&&  strstr( table->fields.field[n].extra, "auto_increment" ) )
						{
							if( auto_increment_column )
								lprintf( "SQLITE ERROR: Failure will happen - more than one auto_increment" );
							auto_increment_column = table->fields.field[n].name;
							vtprintf( pvtCreate, WIDE("%s`%s` %s%s")
									  , first?WIDE(""):WIDE(",")
									  , table->fields.field[n].name
									  , "INTEGER" //table->fields.field[n].type
									  , " PRIMARY KEY"
									  );
						}
						else
						{
							CTEXTSTR unsigned_word;
							if(  table->fields.field[n].extra
								&& (unsigned_word=StrStr( table->fields.field[n].extra
								                        , "unsigned" )) )
							{
								TEXTSTR extra = StrDup( table->fields.field[n].extra );
								int len = StrLen( unsigned_word + 8 ); 
								// use same buffer allocated to write into...
								snprintf( extra, strlen( table->fields.field[n].extra ), "%*.*s%*.*s"
								       , (int)(unsigned_word-table->fields.field[n].extra)
								       , (int)(unsigned_word-table->fields.field[n].extra)
									   , table->fields.field[n].extra
									   , len 
									   , len
									   , unsigned_word + 8
									   );
								vtprintf( pvtCreate, WIDE("%s`%s` %s %s")
										  , first?WIDE(""):WIDE(",")
										  , table->fields.field[n].name
										  , table->fields.field[n].type
										  , extra
										  );
								Release( extra );
							}
							else
								vtprintf( pvtCreate, WIDE("%s`%s` %s%s%s")
										  , first?WIDE(""):WIDE(",")
										  , table->fields.field[n].name
										  , table->fields.field[n].type
										  , table->fields.field[n].extra?WIDE(" "):WIDE("")
										  , table->fields.field[n].extra?table->fields.field[n].extra:WIDE("")
										  );
						}
					}
					else
#endif
						vtprintf( pvtCreate, WIDE("%s`%s` %s%s%s")
								  , first?WIDE(""):WIDE(",")
								  , table->fields.field[n].name
								  , table->fields.field[n].type
								  , table->fields.field[n].extra?WIDE(" "):WIDE("")
								  , table->fields.field[n].extra?table->fields.field[n].extra:WIDE("")
								  );
					first = 0;
				}
				for( n = 0; n < table->keys.count; n++ )
				{
					int col;
					int colfirst = 1;
					if( table->keys.key[n].flags.bPrimary )
					{
#ifdef USE_SQLITE
						if( odbc->flags.bSQLite_native )
						{
							if( auto_increment_column )
							{
								if( table->keys.key[n].colnames[1] )
								{
									lprintf( "SQLITE ERROR: Complex PRIMARY KEY promoting to UNIQUE" );
									vtprintf( pvtCreate, WIDE("%sUNIQUE `primary` (")
											  , first?WIDE(""):WIDE(",") );
								}
								if( strcmp( auto_increment_column, table->keys.key[n].colnames[0] ) )
									lprintf( "SQLITE ERROR: auto_increment column was not the PRMIARY KEY" );
								else
								{
									// ignore key
									continue;
								}
							}
							else
							{
								//vtprintf( pvtCreate, WIDE("%sPRIMARY KEY (")
								//		  , first?WIDE(""):WIDE(",") );
							}
						}
						else
#endif
						{
							vtprintf( pvtCreate, WIDE("%sPRIMARY KEY (")
									  , first?WIDE(""):WIDE(",") );
							for( col = 0; table->keys.key[n].colnames[col]; col++ )
							{
								if( !table->keys.key[n].colnames[col] )
									break;
								vtprintf( pvtCreate, WIDE("%s`%s`")
										  , colfirst?WIDE(""):WIDE(",")
										  , table->keys.key[n].colnames[col]
										  );
								colfirst = 0;
							}
							vtprintf( pvtCreate, WIDE(")") );
						}
					}
					else
					{

#ifdef USE_SQLITE
						if( odbc->flags.bSQLite_native )
						{
							if( table->keys.key[n].flags.bUnique && 0 )
							{
								vtprintf( pvtCreate, WIDE("%s`%s`%s(")
										  , first?WIDE(""):WIDE(",")
										  , table->keys.key[n].name
										  , table->keys.key[n].flags.bUnique?WIDE("UNIQUE "):WIDE("KEY")
										  );
								for( col = 0; table->keys.key[n].colnames[col]; col++ )
								{
									if( !table->keys.key[n].colnames[col] )
										break;
									vtprintf( pvtCreate, WIDE("%s`%s`")
											  , colfirst?WIDE(""):WIDE(",")
											  , table->keys.key[n].colnames[col]
											  );
									colfirst = 0;
								}
								vtprintf( pvtCreate, WIDE(")") );
								first = 0;

							}

						}
						else
#endif
						{
							vtprintf( pvtCreate, WIDE("%s%sKEY `%s` (")
									  , first?WIDE(""):WIDE(",")
									  , table->keys.key[n].flags.bUnique?WIDE("UNIQUE "):WIDE("")
									  , table->keys.key[n].name );
							for( col = 0; table->keys.key[n].colnames[col]; col++ )
							{
								if( !table->keys.key[n].colnames[col] )
									break;
								vtprintf( pvtCreate, WIDE("%s`%s`")
										  , colfirst?WIDE(""):WIDE(",")
										  , table->keys.key[n].colnames[col]
										  );
								colfirst = 0;
							}
							vtprintf( pvtCreate, WIDE(")") );
							first = 0;

						}
					}
				}
				vtprintf( pvtCreate, WIDE(")") ) ; // closing paren of all columns...
#ifdef USE_SQLITE
				if( !odbc->flags.bSQLite_native )
#endif
				{
               /* these are not supported under sqlite backend*/
					if( table->type )
						vtprintf( pvtCreate, WIDE("TYPE=%s"),table->type ) ;
					//else
					//	vtprintf( pvtCreate, WIDE("TYPE=MyISAM") ); // cpg 15 dec 2006
					if( table->comment )
						vtprintf( pvtCreate, WIDE(" COMMENT=\'%s\'" ), table->comment );
				}
				txt_cmd = VarTextGet( pvtCreate );
				if( f_odbc )
					fprintf( f_odbc, "%s;\n", GetText( txt_cmd ) );
				else
					SQLCommand( odbc, GetText( txt_cmd ) );
				LineRelease( txt_cmd );
			}
			else
			{
				lprintf( WIDE("error is : %s"), error );
			}
		}
	}
	Release(cmd);
	if( pvtCreate )
		VarTextDestroy( &pvtCreate );
	if( f_odbc )
      fclose( f_odbc );
	return 1;
}


LOGICAL CheckODBCTable( PODBC odbc, PTABLE table, _32 options )
{
	if( !odbc )
	{
		OpenSQL();
		odbc = g.odbc;

	}
			  //    DebugBreak();

	if( !odbc )
		return FALSE;


			  // should check some kinda flag on ODBC to see if it's MySQL or Access
			  //    DebugBreak();
	if( odbc->flags.bAccess )
		return CheckAccessODBCTable( odbc, table, options );
	else
		return CheckMySQLODBCTable( odbc, table, options );

}

static void CreateNameTable( PODBC odbc, CTEXTSTR table_name )
{
	TEXTCHAR field1[256];
	TEXTCHAR field2[256];
	TABLE table;
	FIELD fields[2];
#ifdef __cplusplus
#else
	DB_KEY_DEF keys[1];
#endif
	snprintf( field1, sizeof( field1 ), WIDE("%s_id"), table_name );
#ifdef __cplusplus
	DB_KEY_DEF keys[1] = { required_key_def( TRUE, FALSE, NULL, field1 ) };
#endif
	table.name = table_name;
	table.fields.count = 2;
	table.fields.field = fields;
	table.keys.count = 1;
	table.keys.key = keys;
	table.type = NULL;
	table.comment = WIDE( "Auto Created table." );
	fields[0].name = field1;
	fields[0].type = WIDE("int");
	fields[0].extra = WIDE("auto_increment");
	fields[0].previous_names[0] = NULL;
	snprintf( field2, sizeof( field2 ), WIDE("%s_name"), table_name );
	fields[1].name = field2;
	fields[1].type = WIDE("varchar(100)");
	fields[1].extra = NULL;
	fields[1].previous_names[0] = NULL;

#ifndef __cplusplus
	keys[0].name = NULL; // primary key needs no name
	keys[0].flags.bPrimary = 1;
	keys[0].flags.bUnique = 0;
	keys[0].colnames[0] = field1;
	keys[0].colnames[1] = NULL;
	keys[0].null = NULL;
#endif
	CheckODBCTable( odbc, &table, CTO_MERGE );
}


INDEX FetchSQLNameID( PODBC odbc, CTEXTSTR table_name, CTEXTSTR name )
{
	{
		CTEXTSTR result;
		int bTried = 0;
	retry:
		if( !SQLQueryf( odbc
						  , &result
						  , WIDE("select %s_id from %s where %s_name=\'%s\'")
						  , table_name
						  , table_name
						  , table_name
						  , name ) )
		{
			FetchSQLError( odbc, &result );
			if( ( StrCmpEx( result, WIDE("(S0022)"), 7 ) == 0 ) ||
				( StrCmpEx( result, WIDE("(S0002)"), 7 ) == 0 ) )
			{
				if( !bTried )
				{
					bTried = 1;
					CreateNameTable( odbc, table_name );
					goto retry;
				}
			}
		}
		else
		{
			if( !result )
			{
				if( !SQLCommandf( odbc, WIDE("insert into %s (%s_name)values(\'%s\')"), table_name, table_name, name ) )
				{
					lprintf( WIDE("blah!") );
				}
				else
				{
					TEXTCHAR table_name_id[256];
					snprintf( table_name_id, sizeof( table_name_id ), WIDE("%s_id"), table_name );
					return FetchLastInsertID( odbc, table_name, table_name_id );
				}
			}
			else
			{
				return IntCreateFromText( result );
			}
		}
	}
   return INVALID_INDEX;
}

CTEXTSTR FetchSQLName( PODBC odbc, CTEXTSTR table_name, INDEX iName )
{
	{
		CTEXTSTR result;
		int bTried = 0;
	retry:
		if( !SQLQueryf( odbc
						  , &result
						  , WIDE("select %s_name from %s where %s_id=%lu")
						  , table_name
						  , table_name
						  , table_name
						  , iName ) )
		{
			FetchSQLError( odbc, &result );
			if( ( StrCmpEx( result, WIDE("(S0022)"), 7 ) == 0 ) ||
				( StrCmpEx( result, WIDE("(S0002)"), 7 ) == 0 ) )
			{
				if( !bTried )
				{
					bTried = 1;
					CreateNameTable( odbc, table_name );
					goto retry;
				}
			}
		}
		else
		{
			if( !result )
			{
            return NULL;
			}
			else
			{
            return StrDup( result );
			}
		}
	}
   return NULL;

}

INDEX GetSQLNameID( CTEXTSTR table_name, CTEXTSTR name )
{
	if( !g.odbc )
		OpenSQL();
	if( !g.odbc )
      return INVALID_INDEX;
   return FetchSQLNameID( g.odbc, table_name, name );
}
CTEXTSTR GetSQLName( CTEXTSTR table_name, INDEX iName )
{
	if( !g.odbc )
		OpenSQL();
	if( !g.odbc )
      return NULL;
   return FetchSQLName( g.odbc, table_name, iName );
}



SQL_NAMESPACE_END

