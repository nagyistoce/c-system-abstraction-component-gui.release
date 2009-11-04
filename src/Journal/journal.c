#include <stdio.h>
#include <string.h>
#include <sack_types.h>
#include <sharemem.h>

/********************************************************************
  This library subsystem is intended to provide a general 
  registration joural for applications to maintain a common 
  persistant database.

  Journal will maintain date or update, type of information,
  A reference name, and a type of information.

  Journal entries will be added per page, which an application 
  may choose to retain... If no page is specified, calls will default
  to the last page referenced.

  Is there already an existing product? Perhaps... 
  Am I better than them? I have no idea.  
  Was this easy to develop? yes.

  Freedom Collective, and independant software group of one.  

  Journals can be used for databases of structures?


********************************************************************/

// Since the journal is maintained in a file, we shall apply this
// file as a memory mapped structure.  All members will be offsets
// which will require special access methods internally to access
// efficiently.  This will be handled by a set of macros, or perhaps
// functions.

enum JOURNALTYPES{ 
	JT_PAGE   = 0  // data points to a tree of pages
	, JT_FILE = 1  // internal file type...
	, JT_BINARY    // data is application specific
	, JT_STRING    // string - NUL terminated, may be interpreted as such
	, JT_SIGNEDBYTE      // signed 8 bit value
	, JT_BYTE            // usigned 8 bit value
	, JT_SIGNEDWORD      // signed 16 bit value
	, JT_WORD            // unsigned 16 bit value
	, JT_SIGNEDDOUBLEWORD// signed 32 bit value
	, JT_DOUBLEWORD      // unsigned 32 bit value
	, JT_SIGNEDQUADWORD  // signed 64 bit value
	, JT_QUADWORD        // unsigned 64 bit value

	, JT_FLOAT           // IEEE 32 bit float value
	, JT_DOUBLE          // IEEE 64 bit float value

	, JT_INTEGER = JT_SIGNEDDOUBLEWORD // alias for integer types...
	, JT_BLOB            // user defined value
} 

typedef _32 IJENTRY;
typedef struct journalentry_tag JENTRY, *PJENTRY;
typedef struct journalentry_tag {
	_32     type;
	_32     size;
	IJENTRY lesser, greater, me;
	_8      namesize;
	_8      data[0]; 
};
#define ENTRYNAME( pjentry ) pjentry->data 
#define ENTRYDATA( pjentry ) (pjentry->data + namesize + 1)

typedef struct journaltrack_tag {
	char *name;
	P_8  pointer;
	struct journaltrack_tag *next, **me;
} JOURNALTRACK, *PJOURNALTRACK;

PJOURNALTRACK journals; // current open journals...

#ifdef _WIN32
#define DEFAULT_JOURNAL_FILE "c:/journal"
#else
#define DEFAULT_JOURNAL_FILE "~/.journal"
#endif

//------------------------------------------------------------------------

// name formats are...
// "/page/page/page/value"
// "/filepath/filename:/page/subpage/value"

void TokenizeName( char *name, char **filename, char ***pages )
{
   char *start;
   if( !name )
   	return;
   start = name;
   while( start[0] && start[0] != ':' ) start++;
	if( start[0] ) 
	{
		// well leading part of name is a filename/path...
		if( filename )
			*filename = name;
		start[0] = 0;
		start++;
	}
	else
	{
		if( filename ) 
			*filename = NULL;
		start = name;
	}
	// start is aligned on the part which will be the page/value to get
	{
		char *savestart = start;
		int parts;
		parts = 1;  // will always be at least 1 
		while( start[0] )
		{
			if( start[0] == '/' || start[0] == '\\' )
				parts++;
			start++;
		}
		parts++; // will always terminate the list with a NULL string
		if( pages )
			*pages = (TEXTCHAR**)Allocate( parts * sizeof( TEXTCHAR* ) );
		parts = 0;
		start = savestart;
		(*pages)[0] = start;
		while( start[0] )
		{
			if( start[0] == '/' || start[0] == '\\' )
			{
				start[0] = 0;
				(*pages)[parts] = start + 1;
			}
			start++;
		}
		(*pages)[parts] = NULL;
	}
}

//------------------------------------------------------------------------

PJENTRY OpenJournalEntry( char *valuename )
{
	if( valuename )
	{
		char *file, **pages;
		char *name = Allocate( strlen( valuename ) + 1 );
		strcpy( name, valuename );
		TokenizeName( name, &file, &pages );
		// if file is set - it will be part of name which is to be released...
		if( file )
		{
			
		}
		Release( name );
	}
	return NULL;
}

//------------------------------------------------------------------------



//------------------------------------------------------------------------
// $Log: $
