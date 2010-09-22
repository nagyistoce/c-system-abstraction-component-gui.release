#include <stdhdrs.h>
#include <filesys.h>

char *curdir=".";

typedef struct path_mask_tag
{
	struct  {
		_32 ignore : 1;
	}flags;
	char *mask;
} MASK, *PMASK;

PDATALIST masks;

char *base;

FILE *in, *out;

enum {
	TYPE_UNKNOWN
	,TYPE_C
	,TYPE_H
	,TYPE_CPP
	,TYPE_HPP
	,TYPE_MAKEFILE
};

int tab	 // set to collapse spaces into N spaces per tab
  , detab // set to decompress tabs to N spaces per tab
  , carriage // set to write \r\n instead of just \n
  , fix_double // set to write \n instead of \r\r
  , linefeed // set to write \r\n isntead of just \r
  , type		 // set to know how to comment
  , bDoLog	 // do minor logging
  , subcurse // go into subdirectories...
  , fbsdmake // mangle makefiles (all files) into bsd command syntax
  , gnumake	 // unmangle makefiles from fbsd to GNU make syntax
  , strip_linefeed // remove \n and replace with ' '
;


int FBSDMangle( char *line, char length )
{
#define INSERT_DOT() {	memmove( line+1, line, length ); line[0] = '.'; return 1; }
#ifdef __LINUX__
#define strnicmp strncasecmp
#endif
	// line incoming points at the first non whitespace character.
	// the linebuffer should be sufficiently long to handle inserting a .
	// if nessescary.
	// length is the length of the line from here to the end of the line
	if( strnicmp( line, WIDE("include"), 7 ) == 0 )
		INSERT_DOT()
	else if( strnicmp( line, WIDE("ifeq"), 4 ) == 0 )
	{
		// ug - gotta find the expressions, and resort them...
		// and worse - gotta do the same in reverse on the other side
		//INSERT_DOT()
		{
			char *expr1, *expr2;
			char outline[1024];
			int len;
			int expr1len, expr2len;
			expr1 = line;
			while( expr1[0] != '\'' )
				expr1++;
			expr1len = 1;
			while( expr1[expr1len] != '\'' )
				expr1len++;
			expr1len++;
			expr2 = expr1 + expr1len;
			while( expr2[0] != '\'' )
			{
				expr2++;
			}
			expr2len = 1;
			while( expr2[expr2len] != '\'' )
			{
				expr2len++;
			}
			expr2len++;
			len = sprintf( outline, WIDE(".if %*.*s = %*.*s")
					, expr1len, expr1len, expr1
					, expr2len, expr2len, expr2 );
			len = len - strlen( line );
			strcpy( line, outline );
			return len;
		}
	}
	else if( strnicmp( line, WIDE("ifneq"), 5 ) == 0 )
	{
		char *expr1, *expr2;
		int expr1len, expr2len;
		int len;
			char outline[1024];
		expr1 = line;
		while( expr1[0] != '\'' )
			expr1++;
		expr1len = 1;
		while( expr1[expr1len] != '\'' )
			expr1len++;
		expr1len++;
		expr2 = expr1 + expr1len;
		while( expr2[0] != '\'' )
			expr2++;
		expr2len = 1;
		while( expr2[expr2len] != '\'' )
			expr2len++;
		expr2len++;
		sprintf( outline, WIDE(".if %*.*s != %*.*s")
				, expr1len, expr1len, expr1
				, expr2len, expr2len, expr2 );
		len = strlen( line ) - len;
		strcpy( line, outline );
		return len;
	}
	else if( strnicmp( line, WIDE("ifdef"), 5 ) == 0 )
		INSERT_DOT()
	else if( strnicmp( line, WIDE("ifndef"), 6 ) == 0 )
		INSERT_DOT()
	else if( strnicmp( line, WIDE("if"), 2 ) == 0 )
		INSERT_DOT()
	else if( strnicmp( line, WIDE("else"), 4 ) == 0 )
		INSERT_DOT()
	else if( strnicmp( line, WIDE("endif"), 5 ) == 0 )
		INSERT_DOT()
	return 0;
}

int GNUMangle( char *line, char length )
{
#define DELETE_DOT()	 { memmove( line, line+1, length - 1 ); return -1; }	// line incoming points at the first non whitespace character.
	// the linebuffer should be sufficiently long to handle deleting a .
	// if nessescary.
	// length is the length of the line from here to the end of the line
	if( strnicmp( line, WIDE(".include"), 8 ) == 0 )
		DELETE_DOT()
	else if( strnicmp( line, WIDE(".ifdef"), 6 ) == 0 )
		DELETE_DOT()
	else if( strnicmp( line, WIDE(".ifndef"), 7 ) == 0 )
		DELETE_DOT()
	else if( strnicmp( line, WIDE(".if"), 3 ) == 0 )
	{

		DELETE_DOT()
	}
	else if( strnicmp( line, WIDE(".else"), 5 ) == 0 )
		DELETE_DOT()
	else if( strnicmp( line, WIDE(".endif"), 6 ) == 0 )
		DELETE_DOT()

	return 0;
}

unsigned char linebuffer[1024 * 64];

#ifdef _WIN32
#define KEEP_CASE FALSE
#else
#define KEEP_CASE TRUE
#endif

void CPROC process( PTRSZVAL psv, CTEXTSTR file, int flags )
{
	static int count; // used for fix_double processing.
	char outfile[256];
	int collect_idx, at_start = 1;
	int idx, nonewline = 0;
	{
		int matched = FALSE;
		INDEX idx;
		PMASK pMask;
		DATA_FORALL( masks, idx, PMASK, pMask )
		{
			CTEXTSTR name = pathrchr( file );
			if( !name )
				name = file;
			else
				name = name+1;
			if( !matched || pMask->flags.ignore )
			{
				if( CompareMask( pMask->mask, name, KEEP_CASE ) )
				{
					if( pMask->flags.ignore )
					{
						if( bDoLog )
							printf( WIDE("Ignoring file: %s\n"), file );
						return;
					}
					matched = TRUE;
				}
			}
		}
		if( !matched )
		{
			printf( WIDE("Skipping file: %s\n"), file );
			return;
		}
	}
	if( bDoLog )
		printf( WIDE("Processing file: %s\n"), file );
	in = fopen( file, WIDE("rb") );
	if( in )
	{
		int c;
		sprintf( outfile, WIDE("%s.new"), file );
		out = fopen( outfile, WIDE("wb") );
		if( !out )
		{
			fprintf( stderr, WIDE("Could not create %s\n"), outfile );
			return;
		}
		if( type )
		{
			// This code is UNUSED... was going to auto mark what I did
			// to a file, but have decided that might not be a good idea.
			// so since 'type' is never set, this is never done.

			switch( type )
			{
			case TYPE_C:
			case TYPE_H:
				fprintf( out, WIDE("/* Expanding tabs %d tabbing to %d %s '\r's */%s")
								, detab, tab
								, (carriage)?"with":"without"
								, (carriage)?"\r\n":"\n" );
				break;
			case TYPE_CPP:
			case TYPE_HPP:
				fprintf( out, WIDE("// Expanding tabs %d tabbing to %d %s '\r's%s")
								, detab, tab
								, (carriage)?"with":"without"
								, (carriage)?"\r\n":"\n" );
				break;
			case TYPE_MAKEFILE:
				fprintf( out, WIDE("# Expanding tabs %d tabbing to %d %s '\r's%s")
								, detab, tab
								, (carriage)?"with":"without"
								, (carriage)?"\r\n":"\n" );
				break;
			case TYPE_UNKNOWN:
			default:
				break;
			}
		}
		idx = 0;
		while( !nonewline &&
				 ( c = fgetc( in ) ) > 0 )
		{
			if( c == '\r' )
			{
				if( linefeed )
				{
					if( carriage )
						linebuffer[idx++] = '\r';
					linebuffer[idx++] = '\n';
				}
				if( fix_double )
				{
					count++;
					if( count == 2 )
					{
						count = 0;
						if( carriage )
							linebuffer[idx++] = '\r';
						linebuffer[idx++] = '\n';
					}
				}
				continue;
			}
			else { count = 0; if( detab && c == '\t' )
			{
				int end = ( ( idx + (detab) ) / detab ) * detab;
				do
				{
					linebuffer[idx++] = ' ';
				} while( idx < end );
				continue;
			}
			else if( c == '\n' )
			{
				if( strip_linefeed )
				{
					linebuffer[idx++] = ' ';
					continue;
				}
			storeline:
				collect_idx = 0; // flush current line.
				at_start = 1;
				if( tab )
				{
					int n, startblank, endblank, ofs;
					startblank = -1;
					ofs = 0;
					for( n = 0; n < idx; n++ )
					{
						if( linebuffer[n] == ' ' )
						{
							if( startblank < 0 )
							{
								startblank = n;
							}
						}
						else
						{
							int col;
							endblank = n;
							if( startblank >= 0
							  && ( endblank - startblank > 1 ) )
							{
								do
								{
									col = ( ( startblank + (tab) ) / tab ) * tab;
									if( col <= endblank )
									{
										linebuffer[ofs++] = '\t';
										startblank = col;
									}
								} while( col < endblank );
							}
							if( startblank >= 0 )
							{
								for( col = startblank; col < endblank; col++ )
									linebuffer[ofs++] = ' ';
								startblank = -1;
							}
							linebuffer[ofs++] = linebuffer[n];
						}
					}
					idx = ofs;
				}
				// well - since gcc often complains - no newline
				// we should add this to make it happy.
				//if( !nonewline )
				{
					unsigned char *line_start = linebuffer;
					int len = idx;
					while( line_start[0] &&							 ( line_start[0] == '\t' || line_start[0] == ' ' ) )
					{
						line_start++;
						len--;
					}
					/*
					// these won't work anyhow - the other
					// differences in the make system cause it to fail anyhow...
					if( line_start[0] )
					{
						if( fbsdmake )
						{
							idx += FBSDMangle( line_start, len );
						}
						else if( gnumake )
						{
							idx += GNUMangle( line_start, len );						}
					}
					*/
				}

				{
					if( carriage )
						linebuffer[idx++] = '\r';
					linebuffer[idx++] = '\n';
				}
				fwrite( linebuffer, 1, idx, out );
				idx = 0;
				continue;
			}
			linebuffer[idx++] = c;
			}
		} // added brace after '\r' processing.
		if( idx )
		{
			nonewline = 1;
			goto storeline;
		}
		fclose( out );
		fclose( in );
		remove( file );
		rename( outfile, file );
	}
	else
		fprintf( stderr, WIDE("%s is not a file.\n"), file );
}


int main( int argc, char **argv )
{
	int argstart = 1;
	int all_ignored = 1;
	masks = CreateDataList( sizeof( MASK ) );
	if( argc == 1 )
	{
		fprintf( stderr, WIDE("usage: %s [-T#D#RSV] <path> <files...>\n"), argv[0] );
		fprintf( stderr, WIDE(" all parameter options are case-insensative\n") );
		fprintf( stderr, WIDE(" -[Tt]# - specifies the # of spaces to make a tab\n") );
		fprintf( stderr, WIDE(" -[Dd]# - specifies the # of spaces to make from a tab\n") );
		fprintf( stderr, WIDE(" -[Nn] - convert \\r to \\n(if -R also make \\r\\n\n") );
		fprintf( stderr, WIDE(" -[Aa] - Strip \\r and \\n into a single ' '\n") );
		fprintf( stderr, WIDE(" -[Ff] - Strip double \\r\\r into a single '\\n'\n") );
		fprintf( stderr, WIDE(" -[Ss] - subcurse (recurse) through all sub-directories\n") );
		fprintf( stderr, WIDE(" -[Rr] - add carriage returns (\\r) to output\n") );
		fprintf( stderr, WIDE(" -[Vv] - verbose operation - list files processed\n") );
		//fprintf( stderr, WIDE(" -B - mangles makefiles into FreeBSD ugliness\n") );
		//fprintf( stderr, WIDE(" -G - unmalged makefiles from FreeBSD to GNU\n") );
		fprintf( stderr, WIDE(" -D, -T specified together the file is first de-tabbed and then tabbed\n") );
		fprintf( stderr, WIDE(" <path> is the starting path...\n") );
		fprintf( stderr, WIDE(" !<file> specifies NOT that mask...\n") );
		fprintf( stderr, WIDE(" no options - Remove \\r, check current directory only\n") );
		fprintf( stderr, WIDE("Minor effect - adds a line terminator to ALL lines\n") );
		fprintf( stderr, WIDE("GNUish compilers often complain 'no end of line at end of file'.\n") );
		return -1;
	}
	for( ;argv[argstart]; argstart++ )
	{
		int n;
		if( argv[argstart][0] == '-' )
		{
			for( n = 1; argv[argstart][n]; n++ )
			{
			switch( argv[argstart][n] )
			{
			case 'B':
			case 'b':
				fbsdmake = 1;
				break;
			case 'G':
			case 'g':
				gnumake = 1;
				break;
			case 'a':
			case 'A':
				strip_linefeed = 1;
				break;
			case 'T':
			case 't':
				if( argv[argstart][2] >= '0' &&
					 argv[argstart][2] <= '9' )
					tab = argv[argstart][2] - '0';
				else
					fprintf( stderr, WIDE("invalid length to Tab option\n") );
				break;
			case 'D':
			case 'd':
				if( argv[argstart][2] >= '0' &&
					 argv[argstart][2] <= '9' )
					detab = argv[argstart][2] - '0';
				else
					fprintf( stderr, WIDE("invalid length to Tab option\n") );
				break;
			case 'R':
			case 'r':
				carriage = 1;
				break;
			case 'F':
			case 'f':
				fix_double = 1;
				break;
			case 'N':
			case 'n':
				linefeed = 1;
				break;
			case 'V':
			case 'v':
				bDoLog = 1;
				break;
			case 's':
			case 'S':
				subcurse = 1;
			}
			}
		}
		else
		{
			if( !base )
			{
				if( strchr( argv[argstart], '*' )
				  ||strchr( argv[argstart], '?' ) )
				{
					base = ".";
				}
				else
				{
					base = argv[argstart];
					continue;
				}
			}
			{
				MASK mask;
				mask.mask = argv[argstart];
				if( mask.mask[0] == '!' )
				{
					mask.flags.ignore = 1;
					mask.mask++;
				}
				else
				{
					all_ignored = FALSE;
					mask.flags.ignore = 0;
				}
				AddDataItem( &masks, &mask );
			}
		}
	}
	if( !masks->Cnt || all_ignored )
	{
		MASK mask;
		mask.mask = "*";
		mask.flags.ignore = 0;
		AddDataItem( &masks, &mask );
	}
	if( base )
	{
		void *data = NULL;
		while( ScanFiles( base
							 , WIDE("*")
							 , &data
							 , process
							 , subcurse?SFF_SUBCURSE:0
							 , 0 ) );
	}
	return argstart;
}

//-------------------------------------------------------------------
// $Log: strip.c,v $
// Revision 1.11	2004/12/17 23:23:30	d3x0r
// Cleanup warnings
//
// Revision 1.10	2003/04/27 01:20:36	panther
// Oops operator typo
//
// Revision 1.9  2003/04/27 01:20:09  panther
// Added CVS logging.  Added if all are ignored, add implicit *, and do all but what's ignored
//
//
