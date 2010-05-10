#include <stdhdrs.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sack_types.h>
#include <sharemem.h>
#include <colordef.h>
#include <loadsock.h> // sockaddr, createaddress(str,defport)
#include <network.h>
#include <procreg.h>
#include <deadstart.h>
#include <system.h>
#include <filedotnet.h>
#include <sqlgetoption.h>
//#define FULL_TRACE
//#define LOG_LINES_READ
////#define DEBUG_SLOWNESS
//#define DEBUG_SAVE_CONFIG

//#if defined( GCC ) && !defined( __arm__ )
//#define NEED_ASSEMBLY_CALLER
//#endif

#define DO_LOGGING
#include <logging.h>
#include <configscript.h>
#include "fractions.h"

#ifdef __cplusplus
namespace sack { namespace config {
using namespace sack::math::fraction;
#endif
#ifdef __LINUX__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#define CompareText(l1,l2)    ( StrCaseCmp( GetText(l1), GetText(l2) ) )

// all matches to content are done case insensitive.

// consider vector/array declarations

enum config_types {
     CONFIG_UNKNOWN
   // must match case-insensative exact length.
   // literal text
   , CONFIG_TEXT
   // a yes/no field may be 0/1, y[es]/n[o], on/off
   //
   , CONFIG_YESNO
   , CONFIG_TRUEFALSE = CONFIG_YESNO
   , CONFIG_ONOFF = CONFIG_YESNO
   , CONFIG_OPENCLOSE = CONFIG_YESNO
   , CONFIG_BOOLEAN = CONFIG_YESNO

   // may not have a . point - therefore the . is a terminator and needs
   // to match the next word.
    , CONFIG_INTEGER
    // has to be a floating point number (perhaps integral ie. no decimal)
	 , CONFIG_FLOAT

	 // binary data storage - stored as base64 encoded passed as PDATA
    , CONFIG_BINARY

    // formated number [+/-][[ ]## ]##[/##]
    , CONFIG_FRACTION

   // matches any single word.
    , CONFIG_SINGLE_WORD

   // protocol://[user[:password]](ip/name)[:port][/filepath][?cgi]
   // by convention this will not contain spaces... but perhaps
   // &20; (?)
    , CONFIG_URL

   // matches several words in a row - the end word to match is supplied.
    , CONFIG_MULTI_WORD
   // file name - does not have any path part.
   // the following are all treated like multi_word since file names/paths
   // may contain spaces
    , CONFIG_FILE
   // ends in a / or \,
    , CONFIG_PATH
   // may have path and filename
    , CONFIG_FILEPATH

   // (IP/name)[:port]
    , CONFIG_ADDRESS

    // end of configuration line (match assumed)
    , CONFIG_PROCEDURE

    , CONFIG_COLOR
};

typedef struct config_element_tag CONFIG_ELEMENT, *PCONFIG_ELEMENT;
struct config_element_tag
{
    enum config_types type;
    struct config_test_tag *next;// if a match is found, follow this to next.
	 struct config_element_tag *prior;
	 struct config_element_tag **ppMe; // this is where we came from
    struct {
        _32 vector : 1;
		  _32 multiword_terminator : 1; // prior == actual segment...
		  _32 singleword_terminator : 1; // prior == actual segment...
		  // careful - assembly here requires absolute known
		  // posisitioning - -fpack-struct will short-change
        // this structure to the minimal number of bits.
		  _32 filler:29;
    } flags;
    _32 element_count; // used with vector fields.
    union {
        PTEXT pText;
        struct {
            LOGICAL bTrue;
        } truefalse;
        _64 integer_number;
		  double float_number;
		  TEXTSTR pWord; // also pFilename, pPath, pURL
		  struct {
			  TEXTSTR pWord; // also pFilename, pPath, pURL
           struct config_element_tag *pEnd; // also this ends single word...
		  } singleword;
        // maybe pURL should be burst into
        //   ( address, user, password, path, page, params )
        SOCKADDR *psaSockaddr;
        struct {
            TEXTSTR pWords;
     // next thing to match...
     // this is probably a constant text thing, but
     // may be say an integer, filename, or some known
     // format thing...
            struct config_element_tag *pEnd;
        } multiword;
        FRACTION fraction;
        USER_CONFIG_HANDLER Process;
		  CDATA Color;
		  struct {
			  _32 length;
			  POINTER data;
		  } binary;
    } data[1]; // these are value holders... if there is a vector field,
            // either the count will be specified, or this will have to
            // be auto expanded....
};

typedef struct config_test_tag
{
   // this constant list could be a more optimized structure like
   // a tree...
    PLIST pConstElementList;  // list of words which are constant to be checked.
    PLIST pVarElementList;     // list of fields which are variables.
} CONFIG_TEST, *PCONFIG_TEST;

#define MAXCONFIG_TESTSPERSET 128
DeclareSet( CONFIG_TEST );
#define MAXCONFIG_ELEMENTSPERSET 128
DeclareSet( CONFIG_ELEMENT );

typedef struct config_file_tag CONFIG_HANDLER;
struct config_file_tag
{
	// this needs to be the first element - 
	// address of this IS the address of main structure
	CONFIG_TEST ConfigTestRoot;
	PDATASTACK ConfigStateStack;

    FILE *file;
	//gcroot<System::IO::StreamReader^> sr;
	//gcroot<System::IO::FileStream^> fs;


	// this should be more than one...
	// and each will be called in order that it was
	// added, very importatn first in first process
	 PLIST filters;
    PLIST filter_data; // a scratch pointer addrss passd to each filter ... (per config handler)
    //PTRSZVAL (CPROC *filter)(PTRSZVAL,int *,char**);

	PTRSZVAL psvUser;
	PTRSZVAL (CPROC *EndProcess)( PTRSZVAL );
	PTRSZVAL (CPROC *Unhandled)( PTRSZVAL, CTEXTSTR );

	PCONFIG_ELEMENTSET elements;
	PCONFIG_TESTSET test_elements;
   LOGICAL config_recovered;
   CTEXTSTR save_config_as;
	PLIST states; // history of saved coniguration states...
};


typedef struct configuation_state *PCONFIG_STATE;
typedef struct configuation_state {
   LOGICAL recovered;
	CTEXTSTR name;
	CONFIG_TEST ConfigTestRoot;
	PTRSZVAL psvUser;
	PTRSZVAL (CPROC *EndProcess)( PTRSZVAL );
	PTRSZVAL (CPROC *Unhandled)( PTRSZVAL, CTEXTSTR );
} CONFIG_STATE;

typedef struct global_tag {
   //LOGICAL bSaveMemDebug;
	int _last_allocate_logging;
	int _disabled_allocate_logging;

	struct {
		BIT_FIELD bDisableMemoryLogging : 1;
		BIT_FIELD bLogUnhandled : 1;
	} flags;
} GLOBAL;

//#ifdef __STATIC__
static GLOBAL *global_config_data;
#define g (*global_config_data)
PRIORITY_PRELOAD( InitGlobal, CONFIG_SCRIPT_PRELOAD_PRIORITY )
{
	SimpleRegisterAndCreateGlobal( global_config_data );
}

PRELOAD( InitGlobalConfig2 )
{
#ifdef __NO_OPTIONS__
	g.flags.bDisableMemoryLogging = 1;
	g.flags.bLogUnhandled = 0;
#else
	g.flags.bDisableMemoryLogging = SACK_GetProfileIntEx( GetProgramName(), "SACK/Config Script/Disable Memory Logging", 1, TRUE );
	g.flags.bLogUnhandled = SACK_GetProfileIntEx( "SACK/Config Script", "Log Unhandled if no application handler", 0, TRUE );
#endif
}

void DoInit( void )
{
	if( !global_config_data )
		SimpleRegisterAndCreateGlobal( global_config_data );

}
//#else
//#define g global_config_data
//static GLOBAL g;
//#endif


#include <configscript.h>

//---------------------------------------------------------------------

void LogElementEx( TEXTCHAR *leader, PCONFIG_ELEMENT pce DBG_PASS)
#define LogElement(leader,pc) LogElementEx(leader,pc DBG_SRC )

{
    if( !pce )
    {
        _lprintf(DBG_RELAY)( WIDE("Nothing.") );
        return;
    }
    switch( pce->type )
    {
    case CONFIG_UNKNOWN:
        _lprintf(DBG_RELAY)( WIDE("This thing was never configured?") );
        break;
    case CONFIG_TEXT:
        _lprintf(DBG_RELAY)( WIDE("%s text constant: %s"), leader, GetText( pce->data[0].pText ) );
        break;
    case CONFIG_BOOLEAN:
        _lprintf(DBG_RELAY)( WIDE("%s a boolean"), leader );
        break;
    case CONFIG_INTEGER:
        _lprintf(DBG_RELAY)( WIDE("%s integer"), leader );
        break;
    case CONFIG_COLOR:
        _lprintf(DBG_RELAY)( WIDE("%s color"), leader );
        break;
    case CONFIG_BINARY:
        _lprintf(DBG_RELAY)( WIDE("%s binary"), leader );
        break;
    case CONFIG_FLOAT:
        _lprintf(DBG_RELAY)( WIDE("%s Floating"), leader );
        break;
    case CONFIG_FRACTION:
        _lprintf(DBG_RELAY)( WIDE("%s fraction"), leader );
        break;
    case CONFIG_SINGLE_WORD:
		 _lprintf(DBG_RELAY)( WIDE("%s a single word:%p"), leader, pce->data[0].pWord );
        break;
    case CONFIG_MULTI_WORD:
        _lprintf(DBG_RELAY)( WIDE("%s a multi word"), leader );
        break;
   case CONFIG_PROCEDURE:
    _lprintf(DBG_RELAY)( WIDE("%s a procedure to call."), leader );
    break;
   case CONFIG_URL:
    _lprintf(DBG_RELAY)( WIDE("%s a url?"), leader );
    break;
   case CONFIG_FILE:
    _lprintf(DBG_RELAY)( WIDE("%s a filename"), leader );
    break;
   case CONFIG_PATH:
    _lprintf(DBG_RELAY)( WIDE("%s a path name"), leader );
    break;
   case CONFIG_FILEPATH:
    _lprintf(DBG_RELAY)( WIDE("%s a full path and file name"), leader );
    break;
   case CONFIG_ADDRESS:
    _lprintf(DBG_RELAY)( WIDE("%s an address"), leader );
    break;
    default:
        _lprintf(DBG_RELAY)( WIDE("Do not know what this is.") );
        break;
    }
}

//---------------------------------------------------------------------

void DumpConfigurationEvaluator( PCONFIG_HANDLER pch )
{
	INDEX idx;
   PCONFIG_ELEMENT pce;
	LIST_FORALL( pch->ConfigTestRoot.pConstElementList, idx, PCONFIG_ELEMENT, pce )
	{
      LogElement( WIDE("const"), pce );
	}
	LIST_FORALL( pch->ConfigTestRoot.pVarElementList, idx, PCONFIG_ELEMENT, pce )
	{
      LogElement( WIDE( "var" ), pce );
	}
}

//---------------------------------------------------------------------

// man what a friggin mess just to deal with ANY
// kind of mangled input and spit out some kinda lines
// uhmm...
static PTEXT CPROC FilterLines( POINTER *scratch, PTEXT buffer )
{
	struct my_scratch_data {
		int skip;
      int lastread;
      PTEXT linebuf;
	} *data = (struct my_scratch_data*)scratch[0];
	int total_length;
	int n;
	int thisskip;
	if( !data )
	{
		scratch[0] = data = New( struct my_scratch_data );
		data[0].skip = 0;
		data[0].linebuf = 0;
      data[0].lastread = 0;
	}
	thisskip = data->skip; // skip N characters in first buffer.
	if( buffer )
	{
      data->lastread = FALSE;
		data->linebuf = SegAppend( data->linebuf, buffer );
	}
	else if( buffer && !GetTextSize( buffer ) )
	{
      PTEXT text = data->linebuf;
		data->lastread = FALSE;
		Release( scratch[0] );
		scratch[0] = NULL;
		if( text )
		{
			LineRelease( buffer );
         lprintf( WIDE( "Returning buffer [%s]" ), GetText( text ) );
         return text;
		}
		else
		{
         lprintf( WIDE( "Returning buffer [%s]" ), GetText( buffer ) );
			return buffer; // pass it on to others - end of stream..
		}
	}
	else if( !buffer )
	{
		if( data->lastread )
		{
			PTEXT final = NULL;
			if( data->linebuf )
				final = SegCreateFromText( GetText( data->linebuf ) + data->skip );
         LineRelease( data->linebuf );
			Release( scratch[0] );
			scratch[0] = NULL;
#ifdef LOG_LINES_READ
			lprintf( WIDE( "Returning buffer [%s]" ), GetText( final ) );
#endif
         return final;
		}
	}
	buffer = data->linebuf;
	total_length = 0;
	while( buffer )
	{
		// full new buffer, which may or may not add to prior segments...
		int end = 0;
		int length = GetTextSize( buffer );
		CTEXTSTR chardata = GetText( buffer );
      //lprintf( WIDE( "Considering buffer %s" ), GetText( buffer ) + data->skip );
		if( !length )
			LineRelease( SegGrab( buffer ) );
		for( n = thisskip; n < length; n++ )
		{
			if( chardata[n] == '\n' ||
				chardata[n] == '\r' )
			{
				if( chardata[n] == '\n' )
				{
               end = 1;
					n++; // include this character.
					//lprintf( WIDE("BLANK LINE - CONSUMED") );
					break;
				}
				if( end ) // \r\r is two lines too
				{
					break;
				}
				end = 1;
			}
			else if( end )
			{
				// any other character... after a \r.. don't include the character.
				break;
			}
		}
		total_length += n - thisskip;
		if( end )
		{
			// new character, trim at -1 from here...
			PTEXT result = SegCreate( total_length );
			int ofs;
			buffer = data->linebuf;
			thisskip = data->skip;
			n = thisskip;
			ofs = 0;
			while( ofs < total_length )
			{
				int len = GetTextSize( buffer );
				if( len > ( len - thisskip ) )
					len = len - thisskip;
				if( len > ( total_length - ofs ) )
					len = total_length - ofs;
				MemCpy( GetText( result ) + ofs
					, GetText( buffer ) + thisskip
					, sizeof( TEXTCHAR)*len );
				ofs += len;
				n += len;
				if( ofs < total_length )
				{
					n = 0;
					thisskip = 0;
					buffer = NEXTLINE( buffer );
				}
			}
			if( buffer )
			{
				data->skip = n;
				LineRelease( SegBreak( buffer ) );
				data->linebuf = buffer;
			}
			else
				data->skip = 0;
			GetText(result)[total_length] = 0;
			//lprintf( "Considering buffer %s", GetText( result ) );
#ifdef LOG_LINES_READ
			lprintf( WIDE( "Returning buffer [%s]" ), GetText( result ) );
#endif
			return result;
		}
		else
		{
			//lprintf( WIDE("Had no end within the buffer, waiting for another...") );
		}
		thisskip = 0; // no more skips.
		buffer = NEXTLINE( buffer );
	}
   data->lastread = TRUE;
	return NULL;
}

//---------------------------------------------------------------------

static PTEXT CPROC FilterTerminators( POINTER *scratch, PTEXT buffer )
{
	struct my_scratch_data {
      PTEXT newline;
	} *data = (struct my_scratch_data*)scratch[0];
	if( !data )
	{
		scratch[0] = data = New( struct my_scratch_data );
      data[0].newline = NULL;
	}
   if( !buffer )
	{
		if( data[0].newline )
		{
			PTEXT tmp = data[0].newline;
         data[0].newline = NULL;
			return tmp;
		}
		else
         return NULL;
	}
	{
		int modified;
		int end = TRUE;
		// filter \r\n\\ just cause...
		data[0].newline = SegAppend( data[0].newline, buffer );
		while( buffer )
		{
			TEXTSTR chardata = GetText( buffer );
			int length = GetTextSize( buffer );
         //LogBinary( chardata, length );
			do
			{
				modified = 0;
				if( length && chardata[length-1] == '\n' )
				{
					//Log( WIDE("Removing newline...") );
					end = TRUE;
					length--;
					modified = 1;
				}
				if( length && chardata[length-1] == '\r' )
				{
					//Log( WIDE("Removing cr...") );
					end = TRUE;
					length--;
					modified = 1;
				}
				if( length && chardata[length-1] == '\\' )
				{
					if( ( length > 1 ) && ( chardata[length-2] != '\\' ) )
					{
						//Log( WIDE("Removing continue slash...") );
						end = FALSE;
						length--;
						modified = 1;
					}
				}
			} while( modified );
			if( !length )
			{
				LineRelease( SegGrab( buffer ) );
				data[0].newline = NULL;
				return NULL;
			}
			chardata[length] = 0;
			//lprintf( WIDE("Resulting line: %s"), chardata );
			SetTextSize( buffer, length );
			buffer = NEXTLINE( buffer );
		}

		if( end )
		{
			PTEXT result = data[0].newline;
         data[0].newline = NULL;
			return result;
		}
	}
   return NULL;
}

//---------------------------------------------------------------------------

/* does not need scratch buffer... */
static PTEXT CPROC FilterEscapesAndComments( POINTER *scratch, PTEXT pText )
{
	CTEXTSTR text = GetText( pText );
	PTEXT pNewText;
	if( text /*&& strchr( text, '\\' )*/ )
	{
		PTEXT tmp;
		tmp = pNewText = TextDuplicate( pText, FALSE );
		while( tmp )
		{
			int dest = 0, src = 0;
			TEXTSTR text = GetText( tmp );
			while( text && text[src] )
			{
				if( text[src] == '\\' )
				{
					src++;
					switch( text[src] )
					{
					case 0:
                  lprintf( WIDE( "Continuation at end of line... save this and append next line please." ) );
                  break;
					default:
						text[dest++] = text[src];
						break;
					}
				}
				else
				{
					if( text[src] == '#' )
                  break;
					text[dest++] = text[src];
				}
				src++;
			}
			text[dest] = 0;
			SetTextSize( tmp, dest );
			tmp = NEXTLINE( tmp );
		}
		LineRelease( pText );
	}
	else
		pNewText = pText;
	return pNewText;
}

//---------------------------------------------------------------------

static PTEXT get_line(FILE *source /*FOLD00*/
							, PLIST *filters
							, PLIST *filter_data
							, int bReturnBlank )
{
#define WORKSPACE 1024  // character for workspace
	PTEXT newline = NULL;
	if( !source )
		return NULL;
	// create a workspace to read input from the file.
	// read a line of input from the file.
	// can't use fgets with encrypted data, but have to use something
	// more like read(), pass to decrypt, then decode lines.
	{
		INDEX idx;
		PTEXT (CPROC *filter)(POINTER*,PTEXT);
		size_t readlen;
		do
		{
			int didone;
			do
			{
				didone = FALSE;
				LIST_FORALL( filters[0], idx, PTEXT (CPROC *)(POINTER*,PTEXT), filter )
				{
				// request any existing data without adding more...
					newline = filter( GetLinkAddress( filter_data, idx ), newline );
					//lprintf( WIDE( "Process line: %s" ), GetText( newline ) );
               //lprintf( WIDE("after filter %d line = %s"), idx, GetText( newline ) );
					if( newline )
						didone = TRUE;
					else
					{
                  //lprintf( WIDE("no more newline... it's been deleted...") );
						if( didone ) // had one, lost it, try for another from the top of blank...
						{
							if( bReturnBlank )
								return SegCreate(0);
							break;
						}
					}
				}
			}
			while( !newline && didone );
			if( !newline )
			{
				newline = SegCreate( WORKSPACE );
				readlen = fread( GetText(newline), 1, WORKSPACE, source );
				if( !readlen )
				{
					LineRelease( newline );
					newline = NULL;
				}
				else
				{
					SetTextSize( newline, (_32)readlen );
					//lprintf( WIDE( "Process line: %s" ), GetText( newline ) );
				}

				LIST_FORALL( filters[0], idx, PTEXT (CPROC *)(POINTER*,PTEXT), filter )
				{
					newline = filter( GetLinkAddress( filter_data, idx ), newline );
					//lprintf( WIDE( "Process line: %s" ), GetText( newline ) );
					if( !newline )
						break; // get next bit of data ....
				}
			}
		}
		while( !newline && readlen );
	}
	return newline;      // return the line read from the file.
}

//---------------------------------------------------------------------

PTEXT GetConfigurationLine( PCONFIG_HANDLER pConfigHandler )
{
    PTEXT line, p;
   while( ( line = get_line( pConfigHandler->file
									, &pConfigHandler->filters
									, &pConfigHandler->filter_data
									, pConfigHandler->Unhandled?TRUE:FALSE ) ) )
   {
#if defined( LOG_LINES_READ )
		lprintf( WIDE( "Process line: %s" ), GetText( line ) );
#endif
      p = burst( line );
      LineRelease( line );
#if defined( FULL_TRACE ) || defined( LOG_LINES_READ )
		{
			PTEXT x = p;
			while( x )
			{
				lprintf( WIDE("Word is: %s (%d,%d)"), GetText( x ), GetTextSize( x ), x->format.position.spaces );
				x = NEXTLINE( x );
			}
		}
      {
         char byLogBuffer[256];
         PTEXT tmp = BuildLine( p );
			Log1( WIDE("input: %s"), GetText( tmp ) );
             LineRelease( tmp );
      }
#endif
		if( p &&
			GetTextSize( p ) )
		{
			return p;
		}
		else // drop and consume blank lines.
		{
         //lprintf( WIDE("Okay got a blank line... might handle it with 'unhandled'") );
			if( pConfigHandler->Unhandled )
				pConfigHandler->Unhandled( pConfigHandler->psvUser, NULL );
			LineRelease(p);
		}
	}
	return NULL;
}

//---------------------------------------------------------------------
// Is____Var results in true/false, plus the config element is filled
// with the current value.  start will also be updated to the current
// location to process.  Otherwise, false will be returned, the element
// will remain uninitialized, and start will not be updated.
//---------------------------------------------------------------------
int IsAnyVar( PCONFIG_ELEMENT pce, PTEXT *start );

//---------------------------------------------------------------------

int IsConstText( PCONFIG_ELEMENT pce, PTEXT *start )
{
    if( pce->type != CONFIG_TEXT )
    {
        return FALSE;
    }
#ifdef FULL_TRACE
    Log2( WIDE("Testing %s vs %s"), GetText( *start ), GetText( pce->data[0].pText ) );
#endif
    if( CompareText( *start, pce->data[0].pText ) == 0 )
    {
#ifdef FULL_TRACE
       Log2( WIDE("%s matched %s"), GetText( *start ), GetText( pce->data[0].pText ) );
#endif
        *start = NEXTLINE( *start );
        return TRUE;
    }
#ifdef FULL_TRACE
	 Log( WIDE("isn't constant...") );
#endif
    return FALSE;
}

//---------------------------------------------------------------------
static CTEXTSTR charset1 = WIDE("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-+");
static CTEXTSTR charset2 = WIDE("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY.-Z+");
typedef union bintobase{
	struct {
      _8 bytes[3];
	} bin;
	struct {
		_32 data1 : 6;
		_32 data2 : 6;
		_32 data3 : 6;
		_32 data4 : 6;
	} base;
} BINBASE;

void EncodeBinaryConfig( TEXTSTR *encode, POINTER data, _32 length )
{
	BINBASE convert;
	CTEXTSTR charset = charset2;
   unsigned char *p;
	TEXTCHAR *q;
	_32 l;
   int bExtraBytes;
	q = (TEXTCHAR*)((*encode) = NewArray( TEXTCHAR, ( ( ( 1 + (length + 2) / 3 ) * 4 ) + 3 ) * 2 ));
	(q++)[0]= '[';
	p = (unsigned char*)&length;

	convert.bin.bytes[0] = (p++)[0];
	convert.bin.bytes[1] = (p++)[0];
	convert.bin.bytes[2] = (p++)[0];
#define EXPLOIT_BURST_FEATURE() 	if( ((q[-1] == '+')?(q[0]='0'),1:0 ) || \
	((q[-1] == '.')?(q[0]='1'),1:0 ) ||                               \
	((q[-1] == '-')?(q[0]='2'),1:0 ) )                                 \
	{                                                 \
	q[-1] = '.';                                \
   q++;                                        \
	}
	(q++)[0] = charset[convert.base.data1];
   EXPLOIT_BURST_FEATURE();
	(q++)[0] = charset[convert.base.data2];
   EXPLOIT_BURST_FEATURE();
	(q++)[0] = charset[convert.base.data3];
   EXPLOIT_BURST_FEATURE();
	(q++)[0] = charset[convert.base.data4];
   EXPLOIT_BURST_FEATURE();
	p = (unsigned char*)data;
	for( l = 0; l < length - 2; l += 3 )
	{
      convert.bin.bytes[0] = (p++)[0];
      convert.bin.bytes[1] = (p++)[0];
		convert.bin.bytes[2] = (p++)[0];
      (q++)[0] = charset[convert.base.data1];
   EXPLOIT_BURST_FEATURE();
      (q++)[0] = charset[convert.base.data2];
   EXPLOIT_BURST_FEATURE();
      (q++)[0] = charset[convert.base.data3];
   EXPLOIT_BURST_FEATURE();
      (q++)[0] = charset[convert.base.data4];
   EXPLOIT_BURST_FEATURE();
	}
	bExtraBytes = 0;
	if( l < length )
	{
		bExtraBytes = 1;
		convert.bin.bytes[0] = (p++)[0];
      l++;
	}
	if( l < length )
	{
		bExtraBytes = 1;
		convert.bin.bytes[1] = (p++)[0];
		l++;
	}
   else if( bExtraBytes )
		convert.bin.bytes[1] = 0;
	if( l < length )
	{
		bExtraBytes = 1;
		convert.bin.bytes[2] = (p++)[0];
		l++;
	}
	else if( bExtraBytes )
		convert.bin.bytes[2] = 0;
	if( bExtraBytes )
	{
		(q++)[0] = charset[convert.base.data1];
   EXPLOIT_BURST_FEATURE();
		(q++)[0] = charset[convert.base.data2];
   EXPLOIT_BURST_FEATURE();
		(q++)[0] = charset[convert.base.data3];
   EXPLOIT_BURST_FEATURE();
		(q++)[0] = charset[convert.base.data4];
   EXPLOIT_BURST_FEATURE();
	}
	(q++)[0]= '}';
	(q++)[0] = 0;

}

//---------------------------------------------------------------------

int DoDecodeBinary( PTEXT *start, POINTER *binary_buffer, _32 *buflen )
{
	_32 failsafe_len;
	CTEXTSTR charset;
	POINTER failsafe_buffer;
	if( !buflen )
		buflen = &failsafe_len;
	if( !binary_buffer )
		binary_buffer = &failsafe_buffer;

	if( GetText( *start )[0] == '{' )
	{
		charset = charset1;
	}
	else if( GetText( *start )[0] == '[' )
	{
		charset = charset2;
	}
	else
		charset = NULL;
	if( charset )
	{
		BINBASE convert;
		int len;
		CTEXTSTR p = GetText( NEXTLINE( *start ) );
		// looks like a good chance this is a binary blob
		CTEXTSTR x;
		char *q;
		TEXTCHAR ch;
#define HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION()  \
	if( (ch=(p++)[0]) == '.' ) {\
	if( p[0] == '0' ) {p++;ch='+';}                     \
	else if ( p[0] == '1' ) {p++;ch = '.';}            \
	else if ( p[0] == '2' ) {p++;ch = '-';}            \
	}
      // if it is empty data... null, and 0
		if( NEXTLINE( *start ) &&
			( ( charset == charset1 && GetText( NEXTLINE( *start  ) )[0] == '}' )
         || ( charset == charset2 && GetText( NEXTLINE( *start  ) )[0] == ']' ) ) )

		{
			(*binary_buffer) = NULL;
			(*buflen) = 0;
			(*start) = NEXTLINE( *start ); // step from { onto }
			(*start) = NEXTLINE( *start ); // step from } onto next token
			return TRUE;
		}
		if( !GetText( NEXTLINE( NEXTLINE( *start  ) ) ) ||
			( GetText( NEXTLINE( NEXTLINE( *start  ) ) )[0] != '}'
			&& GetText( NEXTLINE( NEXTLINE( *start  ) ) )[0] != ']' )
		  )
		{
			(*binary_buffer) = NULL;
			(*buflen) = 0;
			return FALSE;
		}


		// HANLDE_BURST_EXPLOIT converts .0, .1, .2 into .-+ characters... and sets 'ch'
		// to the character to find.
		HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
		convert.base.data1 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
		HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
		convert.base.data2 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
		HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
		convert.base.data3 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
		HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
		convert.base.data4 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
		q = (char*)buflen;
		q[0] = convert.bin.bytes[0];
		q[1] = convert.bin.bytes[1];
		q[2] = convert.bin.bytes[2];
		q[3] = 0;
		// may be as much as 2 extra bytes (expressed as 3)
		(*binary_buffer) = Allocate( len = (*buflen) );
		q = (char*)(*binary_buffer);
		while( p[0] && len )
		{
			HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
			convert.base.data1 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
			HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
			convert.base.data2 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
			HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
			convert.base.data3 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
			HANDLE_BURST_PECULIARITY_WITH_DECIMALS_AND_NUMBER_COLLATION();
			convert.base.data4 = (x=strchr( charset, ch ))?(_32)(x-charset):0;
			if( len && len-- )
				(q++)[0] = convert.bin.bytes[0];
			if( len && len-- )
				(q++)[0] = convert.bin.bytes[1];
			if( len && len-- )
				(q++)[0] = convert.bin.bytes[2];
		}
		(*start) = NEXTLINE( *start ); // step from { onto 0235
		(*start) = NEXTLINE( *start ); // step from 25234 onto }
		(*start) = NEXTLINE( *start ); // step from } onto next token
		return TRUE;
	}
	else
	{
		(*binary_buffer) = NULL;
		(*buflen) = 0;
	}
	return FALSE;
}

int DecodeBinaryConfig( CTEXTSTR string, POINTER *binary_buffer, _32 *buflen )
{
	int status;
	PTEXT tmp1 = SegCreateFromText( string );
	PTEXT start = burst( tmp1 );
	PTEXT delete_string = start; // save this decode updates position...
	LineRelease( tmp1 );
	status = DoDecodeBinary( &start, binary_buffer, buflen );
	LineRelease( delete_string );
	return status;
}

int IsBinaryVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	if( pce->type != CONFIG_BINARY )
		return FALSE;
	if( pce->data[0].binary.data )
	{
		Release( pce->data[0].binary.data );
		//pce->data[0].binary.data = NULL; // decode binary should do this?!
	}
	return DoDecodeBinary( start, &pce->data[0].binary.data, &pce->data[0].binary.length );
}

//---------------------------------------------------------------------

int IsBooleanVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
    int len = GetTextSize( *start );
    int bOkay = FALSE;
    if( pce && pce->type != CONFIG_BOOLEAN )
        return FALSE;
    //Log1( WIDE("Is %s boolean?"), GetText( *start ) );
#define CmpMin(constlen)  ((len <= (constlen))?(len):0)
#define NearText(text,const)   ( CmpMin( sizeof( const ) - 1 ) &&      \
                 ( StrCaseCmpEx( GetText( text ), const, len ) == 0 ) )
    if( NearText( *start, WIDE("yes") ) ||
        NearText( *start, WIDE("1") ) ||
       NearText( *start, WIDE("on") ) ||
       NearText( *start, WIDE("true") ) ||
       NearText( *start, WIDE("open") )
      )
    {
        if(pce)pce->data[0].truefalse.bTrue = 1;
        bOkay = TRUE;
    }
    else if( NearText( *start, WIDE("no") ) ||
          NearText( *start, WIDE("0") ) ||
          NearText( *start, WIDE("off") ) ||
        NearText( *start, WIDE("false") ) ||
        NearText( *start, WIDE("close") )
        )
    {
        if(pce)pce->data[0].truefalse.bTrue = 0;
        bOkay = TRUE;
    }
    else if( NearText( *start, WIDE("are") ) ||
          NearText( *start, WIDE("is") ) )
    {
       PTEXT word;
       bOkay = TRUE;
       if(pce)pce->data[0].truefalse.bTrue = 1;
       if( ( word = NEXTLINE( *start ) ) )
       {
        if( NearText( *start, WIDE("not") ) )
        {
            if(pce)pce->data[0].truefalse.bTrue = 0;
            *start = word;
        }
       }
    }
    if( bOkay )
    {
        *start = NEXTLINE( *start );
        return bOkay;
    }
    return FALSE;
}

int GetBooleanVar( PTEXT *start, LOGICAL *data )
{
	CONFIG_ELEMENT element = { CONFIG_BOOLEAN };
	if( IsBooleanVar( &element, start ) )
	{
		if( data )
			(*data) = element.data[0].truefalse.bTrue;
      return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------

static TEXTCHAR maxbase1[] = WIDE("0123456789abcdefghijklmnopqrstuvwxyz");
static TEXTCHAR maxbase2[] = WIDE("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

int TextToInt( CTEXTSTR text, PS_64 out )
{
    struct {
        _32 neg : 1;
        _32 success : 1;
    } flags;
    _32 base;
    S_64 accum;
    flags.neg = 0;
    flags.success = 1;
    if( text[0] == '-' )
    {
       flags.neg = 1;
       text++;
    }
    else if( text[0] == '+' )
    {
        text++;
    }
    base = 10;
    if( text[0] == '0' )
    {
        base = 8;
        text++;
        if( text[0] == 'x' )
        {
            base = 16;
            text++;
        }
        else if( text[0] == 'b' )
        {
            base = 2;
            text++;
        }
    }
    accum = 0;
    while( text[0] )
    {
        TEXTCHAR *c;
        _32 val;
        if( ( c = strchr( maxbase1, text[0] ) ) ) val = (_32)(c - maxbase1);
        if( !c ) if( ( c = strchr( maxbase2, text[0] ) ) ) val = (_32)(c - maxbase2);
        if( !c ) { flags.success = 0; break; }
        if( val < base )
        {
            accum *= base;
            accum += val;
        }
        else
		{
			// yeah this works... there are times when badly behaved programs generate
			// output that should not match...
			// launch at screenX by ScreenY     "launch at %i by %i" fails
			// launch at 50 by 59                   "                works
			// another rule that took launchat %w by %w could also work... in case
			// there is some variadric thing .. but a different config proc will probably be
			// called for such a case.
            //Log3( WIDE("Base Error : [%c]%s is not within base %d"), text[0], text+1, base );
            flags.success = 0;
            break;
        }
        text++;
    }
    if( flags.success )
    {
        if( out )
        {
		     if( flags.neg )
      	     *out = -accum;
           else
              *out = accum;
        }
        return TRUE;
    }
    return FALSE;
}

int IsIntegerVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
    CTEXTSTR text;
    S_64 accum;
    if( pce->type != CONFIG_INTEGER )
        return FALSE;
    text = GetText( *start );
    if( TextToInt( text, &accum ) )
    {
        pce->data[0].integer_number = accum;
        *start = NEXTLINE( *start );
        return TRUE;
    }
    return FALSE;
}

//---------------------------------------------------------------------

int IsColorVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	if( pce->type != CONFIG_COLOR )
		return FALSE;
	// might consider color names also, (of course that table
	// would have to be configurable)

	//Log1( WIDE("COlor testing: %s"), GetText( *start ) );
	if( GetText( *start )[0] == '$' )
	{
		int ofs = 0;
		PTEXT val = NEXTLINE( *start );
		if( GetTextSize( *start ) == 1 )
			val = NEXTLINE( *start );
		else
		{
			val = *start;
			ofs = 1;
		}

		// potentially a hex variation...
		if( val )
		{
			_32 accum = 0;
			CTEXTSTR digit;
			digit = GetText( val ) + ofs;
			//Log1( WIDE("COlor testing: %s"), digit );
			while( digit && digit[0] )
			{
				int n;
				CTEXTSTR p;
				n = 16;
				p = strchr( maxbase1, digit[0] );
				if( p )
					n = (_32)(p-maxbase1);
				else
				{	
					p = strchr( maxbase2, digit[0] );
					if( p )
						n = (_32)(p-maxbase2);
				}	
				if( n < 16 )
				{
					accum *= 16;
					accum += n;
				}
				else
					break;
				digit++;	
			}
			if( ( digit - GetText( val ) ) < 6 )
			{
				Log( WIDE("Perhaps an error in color variable...") );
				pce->data[0].Color = accum | 0xFF000000;	
			}
			else
			{
				if( ( digit - GetText( val ) ) == 6 )
					pce->data[0].Color = accum | 0xFF000000;	
				else
					pce->data[0].Color = accum;	
			}
			//Log4( WIDE("Color is: $%08X/(%d,%d,%d)")
			//		, pce->data[0].Color
			//		, RedVal( pce->data[0].Color ), GreenVal(pce->data[0].Color), BlueVal(pce->data[0].Color) );
			*start = NEXTLINE( val );
		}
		return TRUE;
	}
	else if( GetText( *start )[0] == '(' )
	{
		// potentially a parenthetical variation.
		PTEXT val;
		int components = 0;
		_32 color = 0;
		for( val = *start;
		     val && GetText( val )[0] != ')';
		     val = NEXTLINE( val ) )
		{
			S_64 accum = 0;
			//Log1( WIDE("Test : %s"), GetText( val ) );
			if( GetText(val)[0] == ',' )
				continue;
			if( TextToInt( GetText( val ), &accum ) )
			{	
				components++;
				color <<= 8;
				color |= ( accum & 0xFF );
			}
		}
		if( val )
		{
			pce->data[0].Color = color;
			if( components < 4 )
				pce->data[0].Color |= 0xFF000000;	
			//Log4( WIDE("Color is: $%08X/(%d,%d,%d)")
			//		, pce->data[0].Color
			//		, RedVal( pce->data[0].Color ), GreenVal(pce->data[0].Color), BlueVal(pce->data[0].Color) );
			*start = NEXTLINE( val );
			return TRUE;
		}
	}
	return FALSE;
}

int GetColorVar( PTEXT *start, CDATA *data )
{
	CONFIG_ELEMENT element = { CONFIG_COLOR };
	if( IsColorVar( &element, start ) )
	{
		if( data )
			(*data) = element.data[0].Color;
      return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------

int IsFloatVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
    //char *text;
    if( pce->type != CONFIG_FLOAT )
        return FALSE;

    //text = GetText( *start );
    Log( WIDE("Floating values are not processed yet.") );

    return FALSE;
}

//---------------------------------------------------------------------

int IsFractionVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
    S_32 accum1, accum2, accum3;
    int neg1, neg2, neg3;
    PTEXT current;
    CTEXTSTR text;
    if( pce->type != CONFIG_FRACTION )
        return FALSE;
    // this may consume multiple segments (4?)  (+/-)##(1) ##(2)/(3)##(4)
    current = *start;
    text = GetText( current );
    accum1 = 0;
    neg1 = 1;

    if( text[0] == '-' )
    {
        neg1 = -1;
        text++;
    }
    else if( text[0] == '+' )
    {
        text++;
    }
    while( text[0] && text[0] >= '0' && text[0] <= '9' )
    {
        accum1 *= 10;
        accum1 += text[0] - '0';
      text++;
    }
    if( text[0] )
    {
        Log( WIDE("Error in first argument of format of fraction.") );
        return FALSE;
    }
// collect numerator after whole number.
   current = NEXTLINE( current );
   if( current )
   {
    text = GetText( current );
    accum2 = 0;
      neg2 = 1;
    if( text[0] == '/' )
    {
        //Log( WIDE("Promoting whole to numerator, getting denominaotr") );
        neg2   = neg1;
        accum2 = accum1;
        neg1   = 1;
        accum1 = 0;
        goto collect_denominator;
    }

    if( text[0] == '-' )
    {
        neg2 = -1;
        text++;
      }
      else if( text[0] == '+' )
      {
        text++;
      }
      while( text[0] && text[0] >= '0' && text[0] <= '9' )
      {
        accum2 *= 10;
        accum2 += text[0] - '0';
        text++;
      }
      if( text[0] )
        Log( WIDE("Error in format of numerator") );
   }
   else
   {
    //Log( WIDE("End of line before numerator") );
      pce->data[0].fraction.numerator = accum1;
      pce->data[0].fraction.denominator = neg1;
      *start = NEXTLINE( current );
    return TRUE;
   }
   current = NEXTLINE( current );
   if( current )
   {
    text = GetText( current );
    if( text[0] != '/' )
    {
        Log( WIDE("No denominator specified - error in fraction.") );
        return FALSE;
    }
   }
   else
   {
    Log( WIDE("End of line before '/'") );
   }
collect_denominator:
    current = NEXTLINE( current );
    if( current )
    {
        text = GetText( current );
        accum3 = 0;
        neg3 = 1;
      if( text[0] == '-' )
      {
        neg3 = -1;
        text++;
      }
      else if( text[0] == '+' )
      {
        text++;
      }
      while( text[0] && text[0] >= '0' && text[0] <= '9' )
      {
        accum3 *= 10;
        accum3 += text[0] - '0';
        text++;
      }
      if( text[0] )
      {
        Log( WIDE("Error in format of denominator.") );
        return FALSE;
      }
      //Log7( WIDE("%d*%d+%d / %d*%d*%d*%d"), accum1, accum3, accum2, neg1, neg2, neg3, accum3 );
      pce->data[0].fraction.numerator = accum1 * accum3 + accum2;
      pce->data[0].fraction.denominator = neg1 * neg2 * neg3 * accum3;
      *start = NEXTLINE( current );
    }
    else
    {
        Log( WIDE("End of line before denominator.") );
        return FALSE;
    }
    return TRUE;
}

//---------------------------------------------------------------------

int IsSingleWordVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	PTEXT pWords = NULL;
   //PTEXT __start = *start;
	if( pce->type != CONFIG_SINGLE_WORD )
		return FALSE;
	if( pce->data[0].singleword.pWord )
	{
		Release( pce->data[0].singleword.pWord );
		pce->data[0].singleword.pWord = NULL;
	}
	while( *start )
	{
		if( pWords )
		{
         PTEXT _start = *start;
			if( IsAnyVar( pce->data[0].singleword.pEnd, start ) )
			{
#ifdef FULL_TRACE
				lprintf( WIDE("Failed check for var check..") );
#endif
            *start = _start; // restore start..
				break;
			}
			if( (*start)->format.position.spaces )
			{
				//if( pWords )
				//	LineRelease( pWords );
#ifdef FULL_TRACE
            // so at a space, stop appending.
				lprintf( WIDE("next word has spaces... [%s](%d)"), GetText(*start), (*start)->format.position.spaces );
#endif
            break;
            //*start = _start;
				//return TRUE;
			}
		}
		pWords = SegAppend( pWords, SegDuplicate( *start ) );
#ifdef FULL_TRACE
		lprintf( WIDE("Appending more to single word...[%s]"), GetText( (*start) ) );
#endif
		*start = NEXTLINE( *start );
	}
	if( pWords )
	{
		PTEXT pText;
      pWords->format.position.spaces= 0;
		pText = BuildLine( pWords );
		LineRelease( pWords );

#ifdef FULL_TRACE
      lprintf( WIDE("Setting text word...") );
#endif
		pce->data[0].singleword.pWord = StrDup( GetText( pText ) );
		LineRelease( pText );
      return TRUE;
	}
    return FALSE;
}

//---------------------------------------------------------------------

int IsMultiWordVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
   int matched = 1;
    PTEXT pWords = NULL;
    if( pce->type != CONFIG_MULTI_WORD )
        return FALSE;
    if( pce->data[0].multiword.pWords )
    {
        Release( pce->data[0].multiword.pWords );
		  pce->data[0].multiword.pWords = NULL;
	 }
    while( *start &&
			 !(matched = IsAnyVar( pce->data[0].multiword.pEnd, start ) ) )
    {
        pWords = SegAppend( pWords, SegDuplicate( *start ) );
        *start = NEXTLINE( *start );
	 }
	 if( (!*start) )
	 {
		 if( !matched && pce->data[0].multiword.pEnd )
		 {
			 LineRelease( pWords );
			 pWords = NULL;
			 lprintf( WIDE( "Ended multi word badly." ) );
          return FALSE;
		 }
#ifdef FULL_TRACE
		 else
			 lprintf( WIDE( "is alright - gathered to end of line ok." ) );
#endif
	 }
	 //if( !pWords )
    //   pWords = SegCreate(0);
    if( pWords )
	 {
		 pWords->format.position.spaces = 0;
		 {
			 PTEXT out = BuildLine( pWords );
			 TEXTSTR buf = StrDup( GetText( out ) );
			 LineRelease( out );
			 LineRelease( pWords );
			 pce->data[0].multiword.pWords = buf;
		 }
		 return TRUE;
    }
    return FALSE;
}

//---------------------------------------------------------------------

int IsPathVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	PTEXT pWords = NULL;
	if( pce->type != CONFIG_PATH )
		return FALSE;
	if( pce->data[0].multiword.pWords )
	{
		Release( pce->data[0].multiword.pWords );
		pce->data[0].multiword.pWords = NULL;
	}
	while( *start &&
			!IsAnyVar( pce->data[0].multiword.pEnd, start ) )
	{
		pWords = SegAppend( pWords, SegDuplicate( *start ) );
		*start = NEXTLINE( *start );
	}
	if( pWords )
	{
		pWords->format.position.spaces = 0;
		{
			PTEXT out = BuildLine( pWords );
			TEXTSTR buf = NewArray( TEXTCHAR, GetTextSize( out ) + 1 );
			StrCpy( buf, GetText( out ) );
			LineRelease( out );
			LineRelease( pWords );
			pce->data[0].multiword.pWords = buf;
		}
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------

int IsFileVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	PTEXT pWords = NULL;
	if( pce->type != CONFIG_FILE )
		return FALSE;
	if( pce->data[0].multiword.pWords )
	{
		Release( pce->data[0].multiword.pWords );
		pce->data[0].multiword.pWords = NULL;
	}
	while( *start &&
			!IsAnyVar( pce->data[0].multiword.pEnd, start ) )
	{
		pWords = SegAppend( pWords, SegDuplicate( *start ) );
		*start = NEXTLINE( *start );
	}
	if( pWords )
	{
		pWords->format.position.spaces = 0;
		{
			PTEXT out = BuildLine( pWords );
			TEXTSTR buf = NewArray( TEXTCHAR, GetTextSize( out ) + 1 );
			StrCpy( buf, GetText( out ) );
			LineRelease( out );
			LineRelease( pWords );
			pce->data[0].multiword.pWords = buf;
		}
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------

int IsFilePathVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	PTEXT pWords = NULL;
    if( pce->type != CONFIG_FILEPATH )
		 return FALSE;
    if( pce->data[0].multiword.pWords )
    {
        Release( pce->data[0].multiword.pWords );
		  pce->data[0].multiword.pWords = NULL;
	 }
    while( *start &&
			 !IsAnyVar( pce->data[0].multiword.pEnd, start ) )
    {
        pWords = SegAppend( pWords, SegDuplicate( *start ) );
        *start = NEXTLINE( *start );
	 }
	 if( pWords )
	 {
		 pWords->format.position.spaces = 0;
		 {
			 PTEXT out = BuildLine( pWords );
			 TEXTSTR buf = NewArray( TEXTCHAR, GetTextSize( out ) + 1 );
			 StrCpy( buf, GetText( out ) );
			 LineRelease( out );
			 LineRelease( pWords );
			 pce->data[0].multiword.pWords = buf;
		 }
		 return TRUE;
	 }
	 return FALSE;
}

//---------------------------------------------------------------------

int IsAddressVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	PTEXT pWords = NULL;
	if( pce->type != CONFIG_ADDRESS )
		return FALSE;
	if( pce->data[0].psaSockaddr )
	{
		ReleaseAddress( pce->data[0].psaSockaddr );
		pce->data[0].psaSockaddr = NULL;
	}
	while( *start && !(*start)->format.position.spaces )
	{
		pWords = SegAppend( pWords, SegDuplicate( *start ) );
		*start = NEXTLINE( *start );
	}
   if( pWords )
	{
		pWords->format.position.spaces = 0;
		{
			PTEXT pText = BuildLine( pWords );
			LineRelease( pWords );
			pce->data[0].psaSockaddr = CreateSockAddress( (CTEXTSTR)GetText( pText ), 0 );
			LineRelease( pText );
		}
      return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------

int IsURLVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	if( pce->type != CONFIG_URL )
		return FALSE;
	return FALSE;
}

//---------------------------------------------------------------------

int IsAnyVar( PCONFIG_ELEMENT pce, PTEXT *start )
{
	if( !pce || !start )
	{
		//Log( WIDE("No pce or no start") );
		return FALSE;
	}
	return( ( IsConstText( pce, start ) ) ||
			 ( IsBooleanVar( pce, start ) ) ||
			 ( IsBinaryVar( pce, start ) ) ||
			 ( IsIntegerVar( pce, start ) ) ||
			 ( IsFloatVar( pce, start ) ) ||
			 ( IsFractionVar( pce, start ) ) ||
			 ( IsSingleWordVar( pce, start ) ) ||
			 ( IsMultiWordVar( pce, start ) ) ||
			 ( IsPathVar( pce, start ) ) ||
			 ( IsFileVar( pce, start ) ) ||
			 ( IsFilePathVar( pce, start ) ) ||
			 ( IsURLVar( pce, start ) ) ||
			 ( IsColorVar( pce, start ) ) );
}

//---------------------------------------------------------------------
//#define PushArgument( type, arg ) ( (parampack = Preallocate( parampack, argsize += sizeof( arg )) )?(*(type*)(parampack) = (arg)),0:0)
//#define PopArguments( sz ) Release( parampack )
//---------------------------------------------------------------------

void DoProcedure( PTRSZVAL *ppsvUser, PCONFIG_TEST Check )
{
	INDEX idx;
	PCONFIG_ELEMENT pce = NULL;
	va_args parampack;
#ifdef __WATCOMC__
	va_args save_parampack;
#endif
	init_args( parampack );
	LIST_FORALL( Check->pVarElementList, idx, PCONFIG_ELEMENT, pce )
	{
		if( pce->type == CONFIG_PROCEDURE )
		{
			if( pce->data[0].Process)
			{
#ifdef NEED_ASSEMBLY_CALLER
				CallProcedure( ppsvUser, pce );
#else
				PCONFIG_ELEMENT pcePush = pce->prior;
				// push arguments in reverse order...
				//Log( WIDE("Calling process... ") );
				while( pcePush )
				{
#ifdef FULL_TRACE
					LogElement( WIDE("pushing"), pcePush );
#endif
					switch( pcePush->type )
					{
					case CONFIG_TEXT:
						break;
					case CONFIG_BINARY:
						PushArgument( parampack, POINTER, pcePush->data[0].binary.data );
						PushArgument( parampack, _32, pcePush->data[0].binary.length );
						break;
					case CONFIG_BOOLEAN:
						{
							LOGICAL val = pcePush->data[0].truefalse.bTrue;
							PushArgument( parampack, LOGICAL, val );
						}
						break;
					case CONFIG_INTEGER:
						PushArgument( parampack, S_64, pcePush->data[0].integer_number );
						break;
					case CONFIG_FLOAT:
						PushArgument( parampack, float, (float)pcePush->data[0].float_number );
						break;
					case CONFIG_FRACTION:
						PushArgument( parampack, FRACTION, pcePush->data[0].fraction );
						break;
					case CONFIG_SINGLE_WORD:
						PushArgument( parampack, CTEXTSTR, pcePush->data[0].pWord );
						break;
					case CONFIG_COLOR:
						PushArgument( parampack, CDATA, pcePush->data[0].Color );
						break;
					case CONFIG_MULTI_WORD:
					case CONFIG_FILEPATH:
						PushArgument( parampack, CTEXTSTR, pcePush->data[0].multiword.pWords );
						break;
					default:
						break;
						  }
					//Log1( WIDE("Total args are now: %d"), argsize );
					pcePush = pcePush->prior;
				}
				// should really be #ifdef __IDIOTS_WROTE_THIS_COMPILER__
#ifdef __WATCOMC__
				save_parampack = parampack;
				(*ppsvUser) = pce->data[0].Process( *ppsvUser, pass_args(parampack) );
				parampack = save_parampack;
#else
				(*ppsvUser) = pce->data[0].Process( *ppsvUser, pass_args(parampack) );
#endif
				PopArguments( parampack );
#endif
				break; // done, end of list, please leave and do not iterate further!
			}
		}
		else
		{
			switch( pce->type )
			{
			case CONFIG_MULTI_WORD:
			case CONFIG_SINGLE_WORD:
				// null content ?
				break;
			default:
				// actually this probably means that there was no content to complete
				// the match, and NULL is not a valid responce to data...
				Log( WIDE("Multiple options here for what to do at end of line?") );
			}
		}
	}
}

//---------------------------------------------------------------------

CONFIGSCR_PROC( int, ProcessConfigurationFile )( PCONFIG_HANDLER pch, CTEXTSTR name, PTRSZVAL psv )
{
	PTEXT line;
	Fopen( pch->file, name, WIDE("rb") );
#ifndef UNDER_CE
	if( !pch->file )
	{
		CTEXTSTR workpath = OSALOT_GetEnvironmentVariable( WIDE( "MY_WORK_PATH" ) );
		TEXTCHAR pathname[255];
		snprintf( pathname, sizeof( pathname ), WIDE("%s/%s"), workpath, name );
		Fopen( pch->file, pathname, WIDE("rb") );
	}
	if( !pch->file )
	{
		CTEXTSTR workpath = OSALOT_GetEnvironmentVariable( WIDE( "MY_LOAD_PATH" ) );
		TEXTCHAR pathname[255];
		snprintf( pathname, sizeof( pathname ), WIDE("%s/%s"), workpath, name );
		Fopen( pch->file, pathname, WIDE("rb") );
	}
#endif
	if( !pch->file )
	{
		TEXTCHAR pathname[255];
		snprintf( pathname, sizeof( pathname ), WIDE("/etc/%s"), name );
		Fopen( pch->file, pathname, WIDE("rb") );
	}
	if( !pch->file )
	{
		TEXTCHAR pathname[255];
		snprintf( pathname, sizeof( pathname ), WIDE("\\ftn3000\\working\\%s"), name );
		Fopen( pch->file, pathname, WIDE("rb") );
	}
	if( !pch->file )
	{
		TEXTCHAR pathname[255];
		snprintf( pathname, sizeof( pathname ), WIDE("C:\\ftn3000\\working\\%s"), name );
		Fopen( pch->file, pathname, WIDE("rb") );
	}
	pch->psvUser = psv;
	if( pch->file )
	{
		while( ( line = GetConfigurationLine( pch ) ) )
		{
			PCONFIG_TEST Check = &pch->ConfigTestRoot;
			PTEXT word = line;
			while( word && Check )
			{
					INDEX idx;
					PCONFIG_ELEMENT pce = NULL;
#ifdef FULL_TRACE
					Log1( WIDE("Test word (%s) vs constant elements"), GetText( word ) );
#endif
					LIST_FORALL( Check->pConstElementList, idx, PCONFIG_ELEMENT, pce )
					{
#ifdef FULL_TRACE
						Log2( WIDE("Is %s == %s?"), GetText( word ), GetText( pce->data[0].pText ) );
#endif
						if( IsConstText( pce, &word ) )
						{
							Check = pce->next;
							break;
						}
					}
#ifdef FULL_TRACE
					Log1( WIDE("Test word (%s) vs variable elements"), GetText( word ) );
#endif
					{
						if( !pce )
						{
							int found = 0;
							LIST_FORALL( Check->pVarElementList, idx, PCONFIG_ELEMENT, pce )
							{
								// end of the line, match should be the process...
#ifdef FULL_TRACE
								Log1( WIDE("Is %s a Thing"), GetText( word ) );
								LogElement( WIDE("Thing is"), pce );
#endif
								if( IsAnyVar( pce, &word ) )
								{
#ifdef FULL_TRACE
                           lprintf( "Yes, it is any var." );
#endif

                           found = 1;
									Check = pce->next;
									break;
								}
#ifdef FULL_TRACE
								else
								{
                           lprintf( "But it's not anything I know." );
								}
#endif
							}
#ifdef FULL_TRACE
							lprintf( "is %s", found?"found":"Not found" );
#endif
							if( !found )
							{
								PTEXT pLine = BuildLine( line );
#ifdef FULL_TRACE
								lprintf( "Line not matched[%s]", GetText( pLine ) );
#endif
								if( pch->Unhandled )
									pch->Unhandled( pch->psvUser, GetText( pLine ) );
								else
								{
									if( g.flags.bLogUnhandled )
										xlprintf(LOG_NOISE)( WIDE("Unknown Configuration Line(No unhandled proc): %s"), GetText( pLine ) );
								}

								LineRelease( pLine );
								break;
							}
						}
					}
			}
			if( !Check )
			{
				lprintf( WIDE("Fell off the end the line processor, still have data...") );
				{
					PTEXT pLine = BuildLine( line );
					if( pch->Unhandled )
						pch->Unhandled( pch->psvUser, GetText( pLine ) );
					else
					{
						// I didn't want to get rid of this...
						// but ahh well, it's noisy and it works.
						//Log1( WIDE("Unknown Configuration Line: %s"), GetText( pLine ) );
					}
					LineRelease( pLine );
				}
			}
			else if( !word )    // otherwise we may have bailed early.
			{
				// check here for Procedure at end of line (word == NULL)
				DoProcedure( &pch->psvUser, Check );
			}
#if 0
			else
			{
#ifdef FULL_TRACE
				lprintf( WIDE("line partially matched... need to recover and re-evaluate..") );
#endif
				{
					PTEXT pLine = BuildLine( line );
					if( pch->Unhandled )
						pch->Unhandled( pch->psvUser, GetText( pLine ) );
					else
					{
						// I didn't want to get rid of this...
						// but ahh well, it's noisy and it works.
						//Log1( WIDE("Unknown Configuration Line: %s"), GetText( pLine ) );
					}
					LineRelease( pLine );
				}
			}
#endif
			LineRelease( line );
        }
        fclose( pch->file );
        if( pch->EndProcess )
			pch->EndProcess( pch->psvUser );
		pch->file = NULL;
		return TRUE;
    }
    else
    {
		 Log1( WIDE("Failed to open config file: %s"), name );
       return FALSE;
    }
}

//---------------------------------------------------------------------

static PCONFIG_ELEMENT NewConfigTestElement( PCONFIG_HANDLER pch )
{

    PCONFIG_ELEMENT pceNew = GetFromSet( CONFIG_ELEMENT, &pch->elements ); //&(PCONFIG_ELEMENT)Allocate( sizeof( CONFIG_ELEMENT ) );
    MemSet( pceNew, 0, sizeof( CONFIG_ELEMENT ) );
    return pceNew;
}

//---------------------------------------------------------------------

//---------------------------------------------------------------------

static PCONFIG_TEST NewConfigTest( PCONFIG_HANDLER pch )
{
	PCONFIG_TEST pctNew = GetFromSet( CONFIG_TEST, &pch->test_elements ); //(PCONFIG_TEST)AllocateEx( sizeof( CONFIG_TEST ) DBG_RELAY );
    MemSet( pctNew, 0, sizeof( CONFIG_TEST ) );
	// I don't actually have to create list...
   // they will be filled in and allocated on demand.
    pctNew->pConstElementList = NULL;//CreateListEx( DBG_VOIDRELAY );
    pctNew->pVarElementList = NULL;//CreateListEx( DBG_VOIDRELAY );
    return pctNew;
}

//---------------------------------------------------------------------

void DestroyConfigElement( PCONFIG_HANDLER pch, PCONFIG_ELEMENT pct );

//---------------------------------------------------------------------

void BeginConfiguration( PCONFIG_HANDLER pch )
{
	// pushes the config file handler state...
	if( pch )
	{
		CONFIG_STATE save_state;
		if( !pch->ConfigStateStack )
			pch->ConfigStateStack = CreateDataStack( sizeof( CONFIG_STATE ) );
#ifdef DEBUG_SAVE_CONFIG
		lprintf( "Saving config and psvUser is %08x", pch->psvUser );
      DumpConfigurationEvaluator( pch );
#endif
      save_state.recovered = pch->config_recovered;
		save_state.ConfigTestRoot = pch->ConfigTestRoot;
		save_state.psvUser = pch->psvUser;
		save_state.EndProcess = pch->EndProcess;
		save_state.Unhandled = pch->Unhandled;
      save_state.name = pch->save_config_as;
		PushData( &pch->ConfigStateStack, &save_state );
#ifdef DEBUG_SAVE_CONFIG
		lprintf( "<BEGIN>" );
#endif
		pch->ConfigTestRoot.pConstElementList = NULL;
		pch->ConfigTestRoot.pVarElementList = NULL;
		pch->EndProcess = NULL;
		pch->Unhandled = NULL;
      pch->config_recovered = FALSE;
#ifdef DEBUG_SAVE_CONFIG
		lprintf( "Setting config as not savable." );
#endif
		pch->save_config_as = NULL;
	}
}

LOGICAL BeginNamedConfiguration( PCONFIG_HANDLER pch, CTEXTSTR name )
{
	// returns true if the configuration previously exists...
	// returns false if it needs to be built.
	INDEX idx;
	PCONFIG_STATE state;
	LIST_FORALL( pch->states, idx, PCONFIG_STATE, state )
	{
		if( StrCmp( state->name, name ) == 0 )
		{
#ifdef DEBUG_SAVE_CONFIG
			lprintf( "found previous config, setting this up, and resulting." );
#endif
			BeginConfiguration( pch );
			pch->ConfigTestRoot = state->ConfigTestRoot;
			pch->EndProcess = state->EndProcess;
			pch->Unhandled = state->Unhandled;
			pch->config_recovered = TRUE;
#ifdef DEBUG_SAVE_CONFIG
			lprintf( "Beginning a named configuration..." );
#endif
         pch->save_config_as = state->name;
#ifdef DEBUG_SAVE_CONFIG
			DumpConfigurationEvaluator( pch );
#endif
			return TRUE;
		}
	}
#ifdef DEBUG_SAVE_CONFIG
	lprintf( "No previous found, setting this up to save at end config..." );
#endif
	BeginConfiguration( pch );
	pch->config_recovered = FALSE;
#ifdef DEBUG_SAVE_CONFIG
	lprintf( "Setting named configuration." );
#endif
   pch->save_config_as = StrDup( name );
   return FALSE;
}


void DestroyConfigTest( PCONFIG_HANDLER pch, PCONFIG_TEST pct, int dealloc );


// this is more like a pop configuration.
void EndConfiguration( PCONFIG_HANDLER pch )
{
	PCONFIG_STATE prior_state;
	//PCONFIG_TEST prior;
	if( pch->EndProcess )
		pch->EndProcess( pch->psvUser );
#ifdef DEBUG_SAVE_CONFIG
	lprintf( "<END>" );
#endif
	prior_state = (PCONFIG_STATE)PopData( &pch->ConfigStateStack );
	if( prior_state )
	{
		if( pch->save_config_as )
		{
			if( !pch->config_recovered )
			{
				CONFIG_STATE *state = New( CONFIG_STATE );
				state->ConfigTestRoot = pch->ConfigTestRoot;
				state->EndProcess = pch->EndProcess;
				state->Unhandled = pch->Unhandled;
				state->name = pch->save_config_as;
				AddLink( &pch->states, state );
			}
         // otherwise there's no action to do... already have it saved.
		}
		else
		{
			//lprintf( WIDE("Config was not saved, destroying.") );
			DestroyConfigTest( pch, &pch->ConfigTestRoot, FALSE );
		}
		// didn't recover from states list.
		pch->config_recovered = prior_state->recovered;
		pch->ConfigTestRoot = prior_state->ConfigTestRoot;
		pch->EndProcess = prior_state->EndProcess;
		pch->Unhandled = prior_state->Unhandled;
		pch->psvUser = prior_state->psvUser;
		pch->save_config_as = prior_state->name;
	}
}

void AddConfigurationEx( PCONFIG_HANDLER pch, CTEXTSTR format, USER_CONFIG_HANDLER Process DBG_PASS )
{
    PTEXT pTemp = SegCreateFromText( format );
    PTEXT pLine; 
    PTEXT pWord;
    struct {
        _32 vartag : 1;
        _32 vector : 1;
        _32 ignore_new : 1;
        _32 store_next_as_end : 1;
        _32 also_store_next_as_end : 1;
        _32 store_as_end : 1;
        _32 also_store_as_end : 1;
    }flags;
    PCONFIG_TEST pct;
    PCONFIG_ELEMENT pceNew, pcePrior;
	 ((_32*)&flags)[0] = 0;
	 //flags.dw = 0;

#if defined( FULL_TRACE ) || defined( DEBUG_SLOWNESS )
	 lprintf( WIDE( "Burst..." ) );
#endif
    pLine = burst( pTemp );
    LineRelease( pTemp );

    pct = &pch->ConfigTestRoot;
	 pceNew = NewConfigTestElement( pch );
	 pcePrior = NULL;
	 pWord = pLine;
    while( pWord )
	 {
#ifdef FULL_TRACE
		 Log1( WIDE("Evaluating %s ... "), GetText( pWord ) );
#endif
		 if( flags.vartag )
		 {
			 CTEXTSTR pWordText = GetText( pWord );
			 if( pWordText[0] == 'v' )
			 {
				 flags.vector = 1;
				 pWordText++;
				 if( !pWordText[0] )
				 {
					 Log1( WIDE("Format: %s"), format );
					 lprintf( WIDE("Configuration error %%v[no type]") );
				 }
				 LineRelease( pLine );
				 lprintf( WIDE( "Destroy config element %p" ), pceNew );
				 DestroyConfigElement( pch, pceNew );
				 return;
			 }
			 switch( pWordText[0] )
			 {
			 case 'b':
				 //lprintf( WIDE("is a boolean...") );
				 pceNew->type = CONFIG_BOOLEAN;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'B':
				 //lprintf( WIDE("is a binary...") );
				 pceNew->type = CONFIG_BINARY;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'i':
				 //lprintf( WIDE("Is an integer... ")) ;
				 pceNew->type = CONFIG_INTEGER;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'c':
				 pceNew->type = CONFIG_COLOR;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'f':
				 pceNew->type = CONFIG_FLOAT;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'q':
				 pceNew->type = CONFIG_FRACTION;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'w':
#ifdef FULL_TRACE
				 lprintf( WIDE("Setting new as type SINGLE_WORD") );
#endif
				 pceNew->type = CONFIG_SINGLE_WORD;
				 pceNew->flags.vector = flags.vector;
				 flags.also_store_next_as_end = 1;
				 break;
			 case 'm':
#ifdef FULL_TRACE
				 lprintf( WIDE("Setting new as type MULTI_WORD") );
#endif
				 pceNew->type = CONFIG_MULTI_WORD;
				 pceNew->flags.vector = flags.vector;
				 flags.store_next_as_end = 1;
				 break;
			 case 'u':
				 pceNew->type = CONFIG_URL;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'd':
				 pceNew->type = CONFIG_PATH;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'n':
				 pceNew->type = CONFIG_FILE;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'p':
				 pceNew->type = CONFIG_FILEPATH;
				 pceNew->flags.vector = flags.vector;
				 break;
			 case 'a':
				 pceNew->type = CONFIG_ADDRESS;
				 pceNew->flags.vector = flags.vector;
				 break;
			 default:
				 Log1( WIDE("Format: %s"), format );
				 Log1( WIDE("Unknown format character: %c"), pWordText[0] );
				 flags.ignore_new = 1;
				 break;
			 }
			 if( !flags.ignore_new )
			 {
#ifdef FULL_TRACE
				 lprintf( WIDE("Not ignoring the new thing...") );
#endif
				 if( flags.store_as_end )
				 {
#ifdef FULL_TRACE
					 lprintf( WIDE("Storing as end...") );
#endif
					 pcePrior->data[0].multiword.pEnd = pceNew;
					 pceNew->flags.multiword_terminator = 1;
					 pceNew->prior = pcePrior;
					 pcePrior = pceNew;
					 pceNew = NewConfigTestElement( pch );

					 flags.store_as_end = 0;
				 }
				 else
				 {
					 if( flags.store_next_as_end )
					 {
						 flags.store_as_end = 1; // flag to store next thing as end condition.
						 flags.store_next_as_end = 0;
					 }
					 if( flags.also_store_as_end )
					 {
#ifdef FULL_TRACE
						 lprintf( WIDE("Also Storing as end...") );
#endif
						 pcePrior->data[0].singleword.pEnd = pceNew;
						 pceNew->flags.singleword_terminator = 1;
						 flags.also_store_as_end = 0;
					 }
					 if( flags.also_store_next_as_end )
					 {
						 flags.also_store_as_end = 1; // flag to store next thing as end condition.
						 flags.also_store_next_as_end = 0;
					 }

					 {
						 INDEX idx;
						 PCONFIG_ELEMENT pceCheck;
						 LIST_FORALL( pct->pVarElementList, idx, PCONFIG_ELEMENT, pceCheck )
						 {
							 if( pceCheck->type == pceNew->type )
							 {
								 // any data set will be overwritten...
								 // uhmm should probably update to new links.
								 if( !pceCheck->next )
								 {
									 lprintf( WIDE("Something fishy here... second instance of same type...") );
								 }
								 pcePrior = pceCheck;
								 pct = pceCheck->next;
								 break;
							 }
						 }
						 if( !pceCheck )
						 {
#if defined( FULL_TRACE ) || defined( DEBUG_SLOWNESS )
							 lprintf( WIDE("Adding into a new config test") );
#endif
							 AddLink( &pct->pVarElementList, pceNew );
							 pct = pceNew->next = NewConfigTest( pch );
							 pceNew->prior = pcePrior;
							 pcePrior = pceNew;
							 pceNew = NewConfigTestElement( pch );
#if defined( FULL_TRACE ) || defined( DEBUG_SLOWNESS )
							 lprintf( WIDE( "Added." ) );
#endif
						 }
					 }
				 }
			 }
			 else
			 {
				 lprintf( WIDE("ignoreing NEW!?") );
			 }

			 flags.vartag = 0;
			 flags.vector = 0;
			 flags.ignore_new = 0;
		 }
		 else
		 {
			 if( TextIs( pWord, WIDE("%") ) )
			 {
#ifdef FULL_TRACE
				 lprintf( WIDE("next thing is a format character") );
#endif
				 flags.vartag = 1;
			 }
			 else // is static text - literal match.
			 {
				 INDEX idx;
				 PCONFIG_ELEMENT pConst = NULL;
#ifdef FULL_TRACE
				 Log1( WIDE("Storing %s as a constant text"), GetText( pWord ) );
#endif
				 if( flags.store_as_end )
				 {
					 pceNew->type = CONFIG_TEXT;
#ifdef FULL_TRACE
					 Log1( WIDE("Adding %s as the terminator"), GetText( pWord ) );
#endif
					 pceNew->data[0].pText = SegDuplicate( pWord );
					 pcePrior->data[0].multiword.pEnd = pceNew;
					 pceNew->flags.multiword_terminator = 1;
					 pceNew->prior = pcePrior;
					 pcePrior = pceNew;
					 pceNew = NewConfigTestElement( pch );
					 flags.store_as_end = 0;
				 }
				 else
				 {
					 if( flags.also_store_as_end )
					 {
#ifdef FULL_TRACE
						 lprintf( WIDE("Also Storing as end...") );
#endif
						 pcePrior->data[0].singleword.pEnd = pceNew;
						 pceNew->flags.singleword_terminator = 1;
						 flags.also_store_as_end = 0;
					 }
					 LIST_FORALL( pct->pConstElementList, idx, PCONFIG_ELEMENT, pConst )
					 {
						 if( IsConstText( pConst, &pWord ) )
						 {
							 pct = pConst->next;
							 break;
						 }
						 pConst = NULL; // this is not always cleared...
					 }
					 if( pConst ) // continue outer loop (while word)
					 {
#ifdef FULL_TRACE
						 lprintf( WIDE("Found constant already in tree") );
#endif
						 continue;
					 }
					 else
					 {
#ifdef FULL_TRACE
						 lprintf( WIDE("Adding new constant to tree") );
#endif

						 pceNew->type = CONFIG_TEXT;
						 pceNew->data[0].pText = SegDuplicate( pWord );
						 AddLink( &pct->pConstElementList, pceNew );
						 pct = pceNew->next = NewConfigTest( pch);
						 pceNew->prior = pcePrior;
						 pcePrior = pceNew;
						 pceNew = NewConfigTestElement(pch );
					 }
				 }
			 }
		 }
		 pWord = NEXTLINE( pWord );
	 }
	 LineRelease( pLine );
	 // end of the format line - add the procedure.
	 pceNew->prior = pcePrior;
	 // no need to update pcePrior - we're done.
	 pceNew->type = CONFIG_PROCEDURE;
	 pceNew->data[0].Process = Process;
	 AddLink( &pct->pVarElementList, pceNew );
}

//---------------------------------------------------------------------
#undef AddConfiguration
CONFIGSCR_PROC( void, AddConfiguration )( PCONFIG_HANDLER pch, CTEXTSTR format, USER_CONFIG_HANDLER Process )
{
   AddConfigurationEx( pch, format, Process DBG_SRC );
}

//---------------------------------------------------------------------

	CONFIGSCR_PROC( void, SetConfigurationEndProc )( PCONFIG_HANDLER pch
                                        , PTRSZVAL (CPROC *Process)( PTRSZVAL ) )
{
    pch->EndProcess = Process;
}

//---------------------------------------------------------------------

CONFIGSCR_PROC( void, SetConfigurationUnhandled )( PCONFIG_HANDLER pch
																, PTRSZVAL (CPROC *Process)( PTRSZVAL, CTEXTSTR ) )
{
    pch->Unhandled = Process;
}

//---------------------------------------------------------------------
CONFIGSCR_PROC( PCONFIG_HANDLER, CreateConfigurationEvaluator )( void )
{
	PCONFIG_HANDLER pch;
#ifdef __STATIC__
	DoInit();
#endif

	if( g.flags.bDisableMemoryLogging )
		if( !g._disabled_allocate_logging )
		{
			g._last_allocate_logging = SetAllocateLogging( FALSE );
			g._disabled_allocate_logging++;
		}
	pch = (PCONFIG_HANDLER)Allocate( sizeof( CONFIG_HANDLER ) );
	MemSet( pch, 0, sizeof( *pch ) );
	// break input chunks into lines....
	AddLink( &pch->filters, FilterLines );
	SetLink( &pch->filter_data, FindLink( &pch->filters, (POINTER)FilterLines ), 0 );
	// end the lines at # (also remove \r, \n)
	AddLink( &pch->filters, FilterTerminators );
	SetLink( &pch->filter_data, FindLink( &pch->filters, (POINTER)FilterTerminators ), 0 );
	// get rid of \\ and \# to replace with appropriate things
	// since \# might be escaped, this is the only place I can handle the '#' character
	// to terminate lines at comment points.
	AddLink( &pch->filters, FilterEscapesAndComments );
	SetLink( &pch->filter_data, FindLink( &pch->filters, (POINTER)FilterEscapesAndComments ), 0 );
	//pch->ConfigTestRoot.pConstElementList = NULL; //CreateList();
	//pch->ConfigTestRoot.pVarElementList = NULL;//CreateList();
	return pch;
}

//---------------------------------------------------------------------

CONFIGSCR_PROC( void, ClearDefaultFilters )( PCONFIG_HANDLER pch )
{
   EmptyList( &pch->filters );
}

CONFIGSCR_PROC( void, AddConfigurationFilter )( PCONFIG_HANDLER pch, USER_FILTER filter )
{
	AddLink( &pch->filters, filter );
}


//---------------------------------------------------------------------

void DestroyConfigElement( PCONFIG_HANDLER pch, PCONFIG_ELEMENT pce )
{
#ifdef DEBUG_SAVE_CONFIG
	lprintf( "Destroy element %p %p", pch, pce );
#endif
	DestroyConfigTest( pch, pce->next, TRUE );
	switch( pce->type )
	{
	case CONFIG_BINARY:
		if( pce->data[0].binary.data )
         Release( pce->data[0].binary.data );
      break;
	case CONFIG_TEXT:
		if( pce->data[0].pText )
         LineRelease( pce->data[0].pText );
      break;
	case CONFIG_SINGLE_WORD:
      if( pce->data[0].pWord )
			Release( pce->data[0].pWord );
		break;
	case CONFIG_MULTI_WORD:
      if( pce->data[0].multiword.pWords )
			Release( pce->data[0].multiword.pWords );
		if( pce->data[0].multiword.pEnd )
		{
#ifdef DEBUG_SAVE_CONFIG
			lprintf( "Destroy config element %p", pce->data[0].multiword.pEnd );
#endif
			DestroyConfigElement( pch, pce->data[0].multiword.pEnd );
		}
		break;
	case CONFIG_URL:
	case CONFIG_PATH:
	case CONFIG_FILE:
	case CONFIG_FILEPATH:
	case CONFIG_ADDRESS:
      break;
	case CONFIG_COLOR:
	case CONFIG_PROCEDURE:
	case CONFIG_UNKNOWN:
	case CONFIG_BOOLEAN:
	case CONFIG_INTEGER:
	case CONFIG_FRACTION:
	case CONFIG_FLOAT:
      break;
	}
	DeleteFromSet( CONFIG_ELEMENT, pch->elements, pce );
	//Release( pce );
}

void DestroyConfigTest( PCONFIG_HANDLER pch, PCONFIG_TEST pct, int deallocate )
{
	PCONFIG_ELEMENT pce;
	INDEX idx;
	if( !pct )
		return;
#ifdef DEBUG_SAVE_CONFIG
	lprintf( "Destroy Config %p %p", pch, pct );
#endif
	LIST_FORALL( pct->pConstElementList, idx, PCONFIG_ELEMENT, pce )
	{
#ifdef DEBUG_SAVE_CONFIG
		lprintf( "Delete const element" );
#endif
		DestroyConfigElement( pch, pce );
	}
	DeleteList( &pct->pConstElementList );
	LIST_FORALL( pct->pVarElementList, idx, PCONFIG_ELEMENT, pce )
	{
#ifdef DEBUG_SAVE_CONFIG
		lprintf( "Delete var element" );
#endif
		DestroyConfigElement( pch, pce );
	}
	DeleteList( &pct->pVarElementList );
	// a pconfig_handler has one static root - guaranteed root.
	// if this pct IS the root, don't attempt to delete from set
	if( deallocate )
		//if( pct != &pch->ConfigTestRoot )
		DeleteFromSet( CONFIG_TEST, pch->test_elements, pct );
	//Release( pct );
}

//---------------------------------------------------------------------

CONFIGSCR_PROC( void, DestroyConfigurationEvaluator )( PCONFIG_HANDLER pch )
{
	// since as noted in the structure
	// 	// address of this IS the address of main structure
	// the configtest Release() will quickly free this .
	PLIST save_list_pConstElementList = pch->ConfigTestRoot.pConstElementList;
	PLIST save_list_pVarElementList = pch->ConfigTestRoot.pVarElementList;
#ifdef DEBUG_SAVE_CONFIG
	lprintf( "Destroy evaluator %p", pch );
#endif
	DestroyConfigTest( pch, &pch->ConfigTestRoot, FALSE );
	{
		INDEX idx;
		PCONFIG_STATE state;
		LIST_FORALL( pch->states, idx, PCONFIG_STATE, state )
		{
			if( ( save_list_pConstElementList
				  && (state->ConfigTestRoot.pConstElementList == save_list_pConstElementList ))
				|| ( save_list_pVarElementList
					 && (state->ConfigTestRoot.pVarElementList == save_list_pVarElementList ) )
			  )
			{
				//probably this was the error I found in the field... there were
				// probably macros or something that caused excessive layering.
				// or - it was the old bug of not saving the state quite correctly.
				continue;
			}
			DestroyConfigTest( pch, &state->ConfigTestRoot, FALSE );
		}
		LIST_FORALL( pch->states, idx, PCONFIG_STATE, state )
		{
			Release( (POINTER)state->name );
			Release( state );
		}
	}
	if( pch->save_config_as )
		Release( (POINTER)pch->save_config_as );
	//lprintf( WIDE("Setting memory logging to %d"), _last_allocate_logging );
	if( g.flags.bDisableMemoryLogging )
		if( g._disabled_allocate_logging )
		{
			g._disabled_allocate_logging--;
			if( !g._disabled_allocate_logging )
			{
				SetAllocateLogging( g._last_allocate_logging );
			}
		}
	DeleteList( &pch->states );
	DeleteList( &pch->filters );
	DeleteList( &pch->filter_data );
	DeleteSet( (PGENERICSET*)&(pch->elements) );
	DeleteSet( (PGENERICSET*)&(pch->test_elements) );
	Release( pch );
}


void StripConfigString( TEXTSTR out, CTEXTSTR in )
{
	// convert \r\n to literal characters...
	if( !in )
	{
		out[0] = 0;
		return;
	}
	for( ; in[0]; in++,out++ )
	{
		if( in[0] == '\\' )
		{
			switch( in[1] )
			{
			case 'n':
				in++;
				out[0] = '\n';
				break;
			default:
				out[0] = in[1];
				in++;
				break;
			}
		}
		else
			out[0] = in[0];
	}
	out[0] = in[0];
}

void ExpandConfigString( TEXTSTR out, CTEXTSTR in )
{
	// convert \r\n to literal characters...
	if( !in )
	{
		out[0] = 0;
		return;
	}
	for( ; in[0]; in++,out++ )
	{
		if( in[0] == '\n' )
		{
			out[0] = '\\';
			out++;
			out[0] = 'n';
		}
		else if( in[0] == '\\' )
		{
			out[0] = '\\';
			out++;
			out[0] = '\\';
		}
		else
			out[0] = in[0];
	}
	out[0] = in[0];
}


#ifdef __cplusplus
}} //namespace sack { namespace config {
#endif

//AddConfiguration( WIDE("card pattern = %d"),  );
//AddConfiguration( WIDE("card directory = %p"), );
//AddConfiguration( "
// $Log: configscript.c,v $
// Revision 1.54  2005/04/08 11:24:29  panther
// If continuation \ is an escaped \\ do not continue, instead result in EOL.
//
// Revision 1.53  2005/02/15 06:44:20  panther
// Add a seperate filter to handle escapes - and the removal thereof... basically it handles \# and \\ \<anything> becomes <anything> ... no translation like \n=0x0a
//
// Revision 1.52  2005/01/27 07:39:23  panther
// Linux cleaned.
//
// Revision 1.51  2005/01/16 23:44:49  panther
// Minor spelling
//
// Revision 1.50  2004/12/13 11:08:17  panther
// Checkpoint, minor tweaks
//
// Revision 1.49  2004/12/05 15:32:06  panther
// Some minor cleanups fixed a couple memory leaks
//
// Revision 1.48  2004/10/21 16:45:51  d3x0r
// Updaes to dialog handling... still ahve  aproblem with caption resize
//
// Revision 1.47  2004/09/07 18:53:59  d3x0r
// Multi-word things at the end of the line caused some problems.
//
// Revision 1.46  2004/08/25 22:57:40  d3x0r
// Remove noisy message from days of filter development...
//
// Revision 1.45  2004/08/25 16:30:37  d3x0r
// Remove assembly for MSC_VER can do it with real code now.
//
// Revision 1.44  2004/08/24 11:15:15  d3x0r
// Checkpoint Visual studio mods.
//
// Revision 1.43  2004/08/13 07:25:59  d3x0r
// clean some warnings, move an exported routine.
//
// Revision 1.42  2004/08/13 07:12:22  d3x0r
// rip up the readline layer to apply two filters, to test filter philosophy.
//
// Revision 1.41  2004/07/26 17:51:23  jim
// Add set button color... merge test 1.
//
// Revision 1.41  2004/07/26 16:57:35  d3x0r
// Extend config script reading to check ftn3000 working kinda directory
//
// Revision 1.40  2004/07/07 15:33:55  d3x0r
// Cleaned c++ warnings, bad headers, fixed make system, fixed reallocate...
//
// Revision 1.39  2004/06/15 12:18:37  d3x0r
// Hex testing for capitals in color format was broke!
//
// Revision 1.38  2004/06/01 21:53:18  d3x0r
// Rename global section to avoid common name conflict
//
// Revision 1.37  2004/06/01 06:22:05  d3x0r
// Okay cannot disable debug unless the app desires...
//
// Revision 1.36  2004/05/02 05:06:22  d3x0r
// Sweeping changes to logging which by default release was very very noisy...
//
// Revision 1.35  2004/02/09 10:05:13  d3x0r
// Updates to new common arg methods... revert control changes
//
// Revision 1.34  2004/01/15 03:45:58  d3x0r
// Fixed some minor useless expression warnings from watcom
//
// Revision 1.33  2003/12/09 16:15:35  panther
// Pass unhandled stuff to application defined callback if one existss
//
// Revision 1.32  2003/12/09 14:07:22  panther
// Remove noisy message
//
// Revision 1.31  2003/12/09 14:01:12  panther
// Respect 'standard' inclusion \ at end of line
//
// Revision 1.30  2003/12/04 06:21:22  panther
// minor typo fix
//
// Revision 1.29  2003/12/03 17:16:44  panther
// Oops and disable tracing
//
// Revision 1.28  2003/12/03 17:16:13  panther
// Make is single word just a bit more smarter
//
// Revision 1.27  2003/11/28 17:07:40  panther
// Fix single word gathering.  Fix ENDFORALL
//
// Revision 1.26  2003/11/09 22:32:35  panther
// Modify single word to all tokens without a space, Implement %a
//
// Revision 1.25  2003/10/28 01:14:34  panther
// many changes to implement msgsvr on windows.  Even to get displaylib service to build, there's all sorts of errors in inconsistant definitions...
//
// Revision 1.24  2003/10/16 00:12:00  panther
// Fix configscript arg building.  Added Preallocate which like Reallocate resizes memmory, but puts the old memory at the end of the allcoated space.
//
// Revision 1.23  2003/10/14 16:31:43  panther
// Fix va_list passing for watcom
//
// Revision 1.22  2003/10/13 05:13:51  panther
// Include stdargs.h sooner
//
// Revision 1.21  2003/10/13 04:25:14  panther
// Fix configscript library... make sure types are consistant (watcom)
//
// Revision 1.20  2003/10/11 15:41:34  panther
// Oops didn't pass filepath in normal list - also use generic va_list to pass args (ARM)
//
// Revision 1.19  2003/10/11 12:08:18  panther
// Arm variable argument caller...
//
// Revision 1.18  2003/10/10 09:32:29  panther
// Pass 1 arm linux support
//
// Revision 1.17  2003/08/08 07:47:37  panther
// Clear pConst at end of loop - not cleared on loop exit
//
// Revision 1.16  2003/07/24 16:56:41  panther
// Updates to expliclity define C procedure model for callbacks and assembly modules - incomplete
//
// Revision 1.15  2003/04/24 16:32:50  panther
// Initial calibration support in display (incomplete)...
// mods to support more features in controls... (returns set interface)
// Added close service support to display_server and display_image_server
//
// Revision 1.14  2003/04/21 23:33:59  panther
// Add inline assembly for VC support
//
// Revision 1.13  2003/04/17 09:32:51  panther
// Added true/false result from processconfigfile.  Added default load from /etc to msgsvr and display
//
// Revision 1.12  2003/03/25 08:45:50  panther
// Added CVS logging tag
//
