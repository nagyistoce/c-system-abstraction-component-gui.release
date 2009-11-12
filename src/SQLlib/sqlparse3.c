
#include <stdhdrs.h>
#include <sack_types.h>
#include <sharemem.h>
#include <pssql.h>

SQL_NAMESPACE

//----------------------------------------------------------------------

int ValidateCreateTable( PTEXT *word )
{

	if( !TextLike( (*word), "create" ) )
		return FALSE;

	(*word) = NEXTLINE( (*word) );

   if( TextLike( (*word), "temporary" ) )
		(*word) = NEXTLINE( (*word) );
   else if( TextLike( (*word), "temp" ) )
		(*word) = NEXTLINE( (*word) );

	if( !TextLike( (*word), "table" ) )
		return FALSE;

	(*word) = NEXTLINE( (*word) );
	if( TextLike( (*word), "if" ) )
	{
		(*word) = NEXTLINE( (*word) );
		if( TextLike( (*word), "not" ) )
		{
			(*word) = NEXTLINE( (*word) );
			if( TextLike( (*word), "exists" ) )
				(*word) = NEXTLINE( (*word) );
			else
				return FALSE;
		}
		else
			return FALSE;
	}
   return TRUE;
}

//----------------------------------------------------------------------

int GrabName( PTEXT *word, TEXTSTR *result, int *bQuoted DBG_PASS )
{
	TEXTSTR name = NULL;
	//PTEXT start = (*word);
   //printf( "word is %s", GetText( *word ) );
	if( TextLike( (*word), "`" ) )
	{
		PTEXT phrase = NULL;
		PTEXT line;
		if( bQuoted )
			(*bQuoted) = 1;
		(*word) = NEXTLINE( *word );
		while( (*word) && ( GetText( *word )[0] != '`' ) )
		{
			phrase = SegAppend( phrase, SegDuplicateEx(*word DBG_RELAY ) );
			(*word) = NEXTLINE( *word );
		}
      // skip one more - end after the last `
		(*word) = NEXTLINE( *word );
		if( GetText( *word )[0] == '.' )
		{
			(*word) = NEXTLINE( *word );
			LineRelease( phrase );
         phrase = NULL;
			if( TextLike( (*word), "`" ) )
			{
				(*word) = NEXTLINE( *word );
				while( (*word) && ( GetText( *word )[0] != '`' ) )
				{
					phrase = SegAppend( phrase, SegDuplicateEx(*word DBG_RELAY ) );
					(*word) = NEXTLINE( *word );
				}
				(*word) = NEXTLINE( *word );
			}
			else
			{
				phrase = SegAppend( phrase, SegDuplicateEx( *word DBG_RELAY ));
				(*word) = NEXTLINE( *word );
			}
		}
		line = BuildLine( phrase );
		LineRelease( phrase );
		name = StrDupEx( GetText( line ) DBG_RELAY );
		LineRelease( line );
	}
	else
	{
		// don't know...
		char *next;
		if( bQuoted )
			(*bQuoted) = 0;
		next = GetText( NEXTLINE( *word ) );
		if( next && next[0] == '.' )
		{
			// database and table name...
			(*word) = NEXTLINE( *word );
			(*word) = NEXTLINE( *word );
			name = StrDup( GetText(*word ) );
			(*word) = NEXTLINE( *word );
		}
		else
		{
			name = StrDupEx( GetText(*word ) DBG_RELAY );
			(*word) = NEXTLINE( *word );
		}
	}
	if( result )
		(*result) = name;
	return name?TRUE:FALSE;
}

//----------------------------------------------------------------------

static int GrabType( PTEXT *word, TEXTSTR *result DBG_PASS )
{
	if( (*word ) )
	{
		//int quote = 0;
		//int escape = 0;
		PTEXT type = SegDuplicate(*word);
		type->format.position.spaces = 0;
		type->format.position.tabs = 0;
		(*word) = NEXTLINE( *word );
		if( (*word) && GetText( *word )[0] == '(' )
		{
			while( (*word) && GetText( *word )[0] != ')' )
			{
				type = SegAppend( type, SegDuplicate( *word ) );
				(*word) = NEXTLINE( *word );
			}
			type = SegAppend( type, SegDuplicate( *word ) );
			(*word) = NEXTLINE( *word );
		}
		{
			PTEXT tmp = BuildLine( type );
			LineRelease( type );
			if( result )
				(*result) = StrDupEx( GetText( tmp ) DBG_RELAY );
			LineRelease( tmp );
		}
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------

static int GrabExtra( PTEXT *word, TEXTSTR *result )
{
	if( (*word ) )
	{
		PTEXT type = NULL;
		{
			char *tmp;
			while( (*word) && ( ( tmp = GetText( *word ) )[0] != ',' ) && (tmp[0] != ')') )
			{
				if( tmp[0] == ')' )
					break;
				type = SegAppend( type, SegDuplicate( *word ) );
				(*word) = NEXTLINE( *word );
			}
		}
		if( type )
		{
			type->format.position.spaces = 0;
			type->format.position.tabs = 0;
			{
				PTEXT tmp = BuildLine( type );
				LineRelease( type );
				if( result )
					(*result) = StrDup( GetText( tmp ) );
				LineRelease( tmp );
			}
		}
		else
			if( result )
            (*result) = NULL;
	}
   return TRUE;
}

void GrabKeyColumns( PTEXT *word, CTEXTSTR *columns )
{
	int cols = 0;
	if( (*word) && GetText( *word )[0] == '(' )
	{
		do
		{
			(*word) = NEXTLINE( *word );
			if( cols >= MAX_KEY_COLUMNS )
			{
				lprintf( "Too many key columns specified in key for current structure limits." );
				DebugBreak();
			}
			GrabName( word, (TEXTSTR*)columns + cols, NULL DBG_SRC );
			cols++;
			columns[cols] = NULL;
		} 
		while( (*word) && GetText( *word )[0] != ')' );
		(*word) = NEXTLINE( *word );
	}
}

//----------------------------------------------------------------------

void AddIndexKey( PTABLE table, PTEXT *word, int has_name, int primary, int unique )
{
	table->keys.count++;
	table->keys.key = Renew( DB_KEY_DEF
	                       , table->keys.key
	                       , table->keys.count + 1 );
	table->keys.key[table->keys.count-1].null = NULL;
	table->keys.key[table->keys.count-1].flags.bPrimary = primary;
	table->keys.key[table->keys.count-1].flags.bUnique = unique;
	if( has_name )
		GrabName( word, (TEXTSTR*)&table->keys.key[table->keys.count-1].name, NULL  DBG_SRC);
	else
		table->keys.key[table->keys.count-1].name = NULL;
	//table->keys.key[table->keys.count-1].colnames = New( CTEXTSTR );
	table->keys.key[table->keys.count-1].colnames[0] = NULL;
	if( StrCaseCmp( GetText(*word), "USING" ) == 0 )
	{
		(*word) = NEXTLINE( *word );
		// next word is the type, skip that word too....
		(*word) = NEXTLINE( *word );
	}
	GrabKeyColumns( word, table->keys.key[table->keys.count-1].colnames );
}

//----------------------------------------------------------------------

int GetTableColumns( PTABLE table, PTEXT *word )
{
	if( !*word)
		return FALSE;
	//DebugBreak();
	if( GetText( *word )[0] != '(' )
	{
		PTEXT line;
		lprintf( "Failed to find columns... extra data between table name and columns...." );
		lprintf( "Failed at %s", GetText( line = BuildLine( *word ) ) );
		LineRelease( line );
		return FALSE;
	}
	while( (*word) && GetText( *word )[0] != ')' )
	{
		TEXTSTR name = NULL;
		TEXTSTR type = NULL;
		TEXTSTR extra = NULL;
		int bQuoted;
		(*word) = NEXTLINE( *word );
		while( !GetTextSize( *word ) )
			(*word) = NEXTLINE( *word );

		//if( (*word) && GetText( *word )[0] == ',' )
		//	(*word) = NEXTLINE( *word );
		if( !GrabName( word, &name, &bQuoted  DBG_SRC) )
		{
			lprintf( "Failed column parsing..." );
		}
		else
		{
			if( !bQuoted )
			{
				if( StrCaseCmp( name, "PRIMARY" ) == 0 )
				{
					if( StrCaseCmp( GetText(*word), "KEY" ) == 0 )
					{
						(*word) = NEXTLINE( *word );
						if( StrCaseCmp( GetText(*word), "USING" ) == 0 )
						{
							(*word) = NEXTLINE( *word );
                     // next word is the type, skip that word too....
							(*word) = NEXTLINE( *word );
						}
						AddIndexKey( table, word, 0, 1, 0 );
					}
					else
					{
						lprintf( "PRIMARY keyword without KEY keyword is invalid." );
						DebugBreak();
					}
				}
				else if( StrCaseCmp( name, "UNIQUE" ) == 0 )
				{
					if( ( StrCaseCmp( GetText(*word), "KEY" ) == 0 )
						|| ( StrCaseCmp( GetText(*word), "INDEX" ) == 0 ) )
					{
						// skip this word.
						(*word) = NEXTLINE( *word );
					}
					AddIndexKey( table, word, 1, 0, 1 );
				}
				else if( ( StrCaseCmp( name, "INDEX" ) == 0 )
					   || ( StrCaseCmp( name, "KEY" ) == 0 ) )
				{
					AddIndexKey( table, word, 1, 0, 0 );
				}
				else
				{
					GrabType( word, &type DBG_SRC );
					GrabExtra( word, &extra );
					table->fields.count++;
					table->fields.field = Renew( FIELD, table->fields.field, table->fields.count + 1 );
					table->fields.field[table->fields.count-1].name = name;
					table->fields.field[table->fields.count-1].type = type;
					table->fields.field[table->fields.count-1].extra = extra;
					table->fields.field[table->fields.count-1].previous_names[0] = NULL;

				}
			}
			else
			{
				GrabType( word, &type DBG_SRC );
				GrabExtra( word, &extra );
				table->fields.count++;
				table->fields.field = Renew( FIELD, table->fields.field, table->fields.count + 1 );
				table->fields.field[table->fields.count-1].name = name;
				table->fields.field[table->fields.count-1].type = type;
				table->fields.field[table->fields.count-1].extra = extra;
				table->fields.field[table->fields.count-1].previous_names[0] = NULL;
			}
		}
	}

	return TRUE;
}

//----------------------------------------------------------------------

int GetTableExtra( PTABLE table, PTEXT *word )
{
   return TRUE;
}

void LogTable( PTABLE table )
{
	FILE *out;
	out = fopen( "sparse.txt", "at" );
	if( out )
	{
		if( table )
		{
			int n;
			fprintf( out, "\n" );
			fprintf( out, "//--------------------------------------------------------------------------\n" );
			fprintf( out, "// %s \n", table->name );
			fprintf( out, "// Total number of fields = %d\n", table->fields.count );
			fprintf( out, "// Total number of keys = %d\n", table->keys.count );
			fprintf( out, "//--------------------------------------------------------------------------\n" );
			fprintf( out, "\n" );
			fprintf( out, "FIELD %s_fields[] = {\n", table->name );
			for( n = 0; n < table->fields.count; n++ )
				fprintf( out, "\t%s{%s%s%s, %s%s%s, %s%s%s }\n"
					, n?", ":""
					, table->fields.field[n].name?"\"":""
					, table->fields.field[n].name?table->fields.field[n].name:"NULL"
					, table->fields.field[n].name?"\"":""
					, table->fields.field[n].type?"\"":""
					, table->fields.field[n].type?table->fields.field[n].type:"NULL"
					, table->fields.field[n].type?"\"":""
					, table->fields.field[n].extra?"\"":""
					, table->fields.field[n].extra?table->fields.field[n].extra:"NULL"
					, table->fields.field[n].extra?"\"":""
				);
			fprintf( out, "};\n" );
			fprintf( out, "\n" );
			if( table->keys.count )
			{
				fprintf( out, "DB_KEY_DEF %s_keys[] = { \n", table->name );
				for( n = 0; n < table->keys.count; n++ )
				{
					int m;
					fprintf( out, "#ifdef __cplusplus\n" );
					fprintf( out, "\t%srequired_key_def( %d, %d, %s%s%s, \"%s\" )\n"
							 , n?", ":""
							 , table->keys.key[n].flags.bPrimary
							 , table->keys.key[n].flags.bUnique
							 , table->keys.key[n].name?"\"":""
							 , table->keys.key[n].name?table->keys.key[n].name:"NULL"
							 , table->keys.key[n].name?"\"":""
							 , table->keys.key[n].colnames[0] );
					if( table->keys.key[n].colnames[1] )
						fprintf( out, ", ... columns are short this is an error.\n" );
					fprintf( out, "#else\n" );
					fprintf( out, "\t%s{ {%d,%d}, %s%s%s, { "
							 , n?", ":""
							 , table->keys.key[n].flags.bPrimary
							 , table->keys.key[n].flags.bUnique
							 , table->keys.key[n].name?"\"":""
							 , table->keys.key[n].name?table->keys.key[n].name:"NULL"
							 , table->keys.key[n].name?"\"":""
							 );
					for( m = 0; table->keys.key[n].colnames[m]; m++ )
						fprintf( out, "%s\"%s\""
								 , m?", ":""
								 , table->keys.key[n].colnames[m] );
					fprintf( out, " } }\n" );
					fprintf( out, "#endif\n" );
				}
				fprintf( out, "};\n" );
				fprintf( out, "\n" );
			}
			fprintf( out, "\n" );
			fprintf( out, "TABLE %s = { \"%s\" \n", table->name, table->name );
			fprintf( out, "	 , FIELDS( %s_fields )\n", table->name );
         if( table->keys.count )
				fprintf( out, "	 , TABLE_KEYS( %s_keys )\n", table->name );
         else
				fprintf( out, "	 , { 0, NULL }\n" );
			fprintf( out, "	, { 0 }\n" );
			fprintf( out, "	, NULL\n" );
			fprintf( out, "	, NULL\n" );
			fprintf( out, "	,NULL\n" );
			fprintf( out, "};\n" );
			fprintf( out, "\n" );
		}
		else
		{
			fprintf( out, "//--------------------------------------------------------------------------\n" );
			fprintf( out, "// No Table\n" );
			fprintf( out, "//--------------------------------------------------------------------------\n" );
		}
		fclose( out );
	}
}

//----------------------------------------------------------------------

PTABLE GetFieldsInSQLEx( CTEXTSTR cmd, int writestate DBG_PASS )
{
	PTEXT tmp;
	PTEXT pParsed;
	PTEXT pWord;
	PTABLE pTable = New( TABLE );
	MemSet( pTable, 0, sizeof( TABLE ) );
	pTable->fields.field = New( FIELD );
	pTable->flags.bDynamic = TRUE;
	pTable->keys.key = New( DB_KEY_DEF );
	tmp = SegCreateFromText( cmd);
	// tmp will become parsed... the first segment is
	// not released, it is merely truncated.
	{
		// but first, go through, and remove carriage returns... which even
		// if a delimieter need to be considered more like spaces...
		int n, m;
		TEXTCHAR *str = GetText( tmp );
		for( n = 0, m = GetTextSize( tmp ); n < m; n++ )
			if( str[n] == '\n' )
            str[n] = ' ';
	}
	pParsed = TextParse( tmp, WIDE("\'\"\\({[<>]}):@%/,;!?=*&$^~#`"), WIDE(" \t\n\r" ), 1, 1 DBG_RELAY );
	//{
	//   PTEXT outline = DumpText( pParsed );
	//	fprintf( stderr, "%s", GetText( outline ) );
	//}

	// pparsed is tmp...

	pWord = pParsed;

	if( pWord )
	{
		if( ValidateCreateTable( &pWord ) )
		{
			if( !GrabName( &pWord, (TEXTSTR*)&pTable->name, NULL  DBG_SRC) )
			{
				DestroySQLTable( pTable );
				pTable = NULL;
			}
			else
			{
				if( !GetTableColumns( pTable, &pWord ) )
				{
					DestroySQLTable( pTable );
					pTable = NULL;
				}
				else
					GetTableExtra( pTable, &pWord );
			}
		}
		else
		{
			DestroySQLTable( pTable );
			pTable = NULL;
		}
	}
	LineRelease( pParsed );
 	if( writestate )
		LogTable( pTable );
	return pTable;
}

SQL_NAMESPACE_END
