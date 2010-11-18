
#include <stdhdrs.h>
#include <sharemem.h>
#include <../src/sexpat/expat.h>
#include <configscript.h>
#include <filesys.h>
#include "controlstruc.h"

#include <image.h>
// XML_SetUserData
// XML_GetUserData

//#define DEBUG_RESOURCE_NAME_LOOKUP

PSI_XML_NAMESPACE
// need to protect this ...
// ptu it in a structure...
static struct {
	CRITICALSECTION cs;
	void (CPROC*InitProc)(PTRSZVAL,PSI_CONTROL);
	PTRSZVAL psv; // psvInitProc
   PSI_CONTROL frame;
}l;

struct xml_userdata {
	XML_Parser xp;
	PSI_CONTROL pc;
};

void XMLCALL start_tags( void *UserData
							  , const XML_Char *name
							  , const XML_Char **atts )
{
	struct xml_userdata *userdata = (struct xml_userdata *)UserData;
	_32 ID = -1;
	_32 x, y;
	_32 edit_set = 0;
	_32 disable_edit = 0;
	_32 width, height;
	CTEXTSTR caption = NULL;
	_32 border;
	TEXTSTR font = NULL;
	TEXTSTR control_data = NULL;
	TEXTSTR IDName = NULL;
	TEXTSTR type = NULL;
	PSI_CONTROL pc;
	CTEXTSTR *p = atts;
	//lprintf( WIDE("begin a tag %s with..."), name );
	while( p && *p )
	{
		//lprintf( WIDE("begin a attrib %s=%s with..."), p[0], p[1] );
		if( strcmp( p[0], WIDE("ID") ) == 0 )
		{
			ID = (int)IntCreateFromText( p[1] );
		}
		else if( strcmp( p[0], WIDE("IDName") ) == 0 )
		{
			IDName = StrDup( p[1] );
		}
		else if( strcmp( p[0], WIDE("border") ) == 0 )
		{
			border = (int)IntCreateFromText( p[1] );
		}
		else if( strcmp( p[0], WIDE("size") ) == 0 )
		{
#ifdef __cplusplus_cli
			char *mybuf = CStrDup( p[1] );
#define SCANBUF mybuf
#else
#define SCANBUF p[1]
#endif
#ifdef UNICODE
#define sscanf swscanf
#endif

			sscanf( SCANBUF, WIDE("%") _32f WIDE(",") WIDE("%") _32f, &width, &height );
#ifdef __cplusplus_cli
			Release( mybuf );
#endif
		}
		else if( StrCmp( p[0], WIDE("position") ) == 0 )
		{
#ifdef __cplusplus_cli
			char *mybuf = CStrDup( p[1] );
#define SCANBUF mybuf
#else
#define SCANBUF p[1]
#endif
			sscanf( SCANBUF, WIDE("%") _32f WIDE(",") WIDE("%") _32f, &x, &y );
#ifdef __cplusplus_cli
			Release( mybuf );
#endif
		}
		else if( strcmp( p[0], WIDE("caption") ) == 0 )
		{
			caption = p[1];
		}
		else if( strcmp( p[0], WIDE("font") ) == 0 )
		{
			font = StrDup( p[1] );
		}
		else if( strcmp( p[0], WIDE("PrivateData") ) == 0 )
		{
			control_data = StrDup( p[1] );
		}
		else if( strcmp( p[0], WIDE("type") ) == 0 )
		{
			type = StrDup( p[1] );
		}
		else if( strcmp( p[0], WIDE("edit") ) == 0 )
		{
			edit_set = 1;
			disable_edit = (int)IntCreateFromText( p[1] );
		}
		else
		{
			lprintf( WIDE("Unknown Att Pair = (%s=%s)"), p[0], p[1] );
		}

      p += 2;
	}
	if( IDName )
	{
		//lprintf( WIDE( "Making a control... %s %s %s" ), type?type:WIDE("notype"), caption?caption:WIDE("nocatpion"), IDName );
		pc = MakeNamedCaptionedControlByName( userdata->pc
														, type
														, x, y
														, width, height
														, IDName
														, ID
														, caption );
		Release( IDName );
	}
	else
	{
		//lprintf( WIDE( "Making a control... %s %s" ), type?type:WIDE("notype"), caption?caption:WIDE("nocatpion") );
		pc = MakeNamedCaptionedControl( userdata->pc
														, type
														, x, y
														, width, height
														, ID
												, caption );
	}
	//lprintf( WIDE( "control done..." ) );
	if( pc )
	{
		if( edit_set )
			pc->flags.bEditLoaded = 1; // mark that edit was loaded from the XML file.  the function of bEditSet can be used to fix setting edit before frame display.
		pc->flags.bEditSet = edit_set;
		pc->flags.bNoEdit = disable_edit;
	}
	if( type )
		Release( type );
	if( !l.frame )
		l.frame = pc; // mark this as the frame to return
	if( font )
	{
		POINTER fontbuf = NULL;
		_32 fontlen;
		if( DecodeBinaryConfig( font, &fontbuf, &fontlen ) )
			SetCommonFont( pc, RenderFontData( (PFONTDATA)fontbuf ) );
		Release( font );
	}
	if( control_data )
		Release( control_data );
	// SetCommonFont()
	//
	userdata->pc = pc;
}

void XMLCALL end_tags( void *UserData
							, const XML_Char *name )
{
	struct xml_userdata *userdata = (struct xml_userdata *)UserData;
	if( userdata->pc )
		userdata->pc = userdata->pc->parent;
}

//-------------------------------------------------------------------------
static struct {
	CTEXTSTR pFile;
   _32 nLine;
} current_loading;
#ifdef _DEBUG
void * MyAllocate( size_t s ) { return AllocateEx( s, current_loading.pFile, current_loading.nLine ); }
#else
void * MyAllocate( size_t s ) { return AllocateEx( s ); }
#endif
void *MyReallocate( void *p, size_t s ) { return Reallocate( p, s ); }
void MyRelease( void *p ) { Release( p ); }

static XML_Memory_Handling_Suite XML_memhandler;// = { MyAllocate, MyReallocate, MyRelease };
//-------------------------------------------------------------------------

// expected character buffer of appropriate size.
PSI_CONTROL ParseXMLFrameEx( POINTER buffer, _32 size DBG_PASS )
{
	POINTER xml_buffer;
	struct xml_userdata userdata;
	l.frame = NULL;
#  ifdef USE_INTERFACES
	if( !g.MyImageInterface )
		return NULL;
#endif
	//lprintf( WIDE("Beginning parse frame...") );
#if DBG_AVAILABLE
	current_loading.pFile = pFile;
	current_loading.nLine = nLine;
#endif
	XML_memhandler.malloc_fcn = MyAllocate;
	XML_memhandler.realloc_fcn = MyReallocate;
	XML_memhandler.free_fcn = MyRelease;
	userdata.xp = XML_ParserCreate_MM( NULL, &XML_memhandler, NULL );
	userdata.pc = NULL;
	XML_SetElementHandler( userdata.xp, start_tags, end_tags );
	XML_SetUserData( userdata.xp, &userdata );
	xml_buffer = XML_GetBuffer( userdata.xp, size );
	MemCpy( xml_buffer, buffer, size );
	XML_ParseBuffer( userdata.xp, size, TRUE );
	XML_ParserFree( userdata.xp );
	userdata.xp = 0;
	//lprintf( WIDE("Parse done...") );
	return l.frame;
}

PSI_CONTROL LoadXMLFrameOverEx( PSI_CONTROL parent, CTEXTSTR file DBG_PASS )
//PSI_CONTROL  LoadXMLFrame( char *file )
{
	POINTER buffer;
	TEXTSTR tmp = NULL;
	PTRSZVAL size;
#ifdef UNDER_CE
	_32 zz;
#endif
	PSI_CONTROL frame;
#  ifdef USE_INTERFACES
	if( !g.MyImageInterface )
		return NULL;
#endif
	EnterCriticalSec( &l.cs );
	// enter critical section!
	l.frame = NULL;
#if DBG_AVAILABLE
	current_loading.pFile = pFile;
	current_loading.nLine = nLine;
#endif
	size = 0;
#ifdef UNDER_CE
	{
		FILE *file_read = sack_fopen( 0, file, "rt" );
		if( file_read )
		{
			sack_fseek( file_read, 0, SEEK_END );
			zz = ftell( file_read );
			sack_fseek( file_read, 0, SEEK_SET );

			size = zz;
			buffer = Allocate( zz );
			fread( buffer, 1, zz, file );
			sack_fclose( file_read );
			lprintf( "loaded font blob %s %d %p", file, zz, buffer );
		}
	}
#else
	buffer = OpenSpace( NULL, file, &size );
#endif
	if( !buffer || !size )
	{
		// attempt secondary open within frames/*
#ifdef UNDER_CE
		int len;
#endif
		size = 0;
#ifdef UNDER_CE
		{
			FILE *file_read = sack_fopen( GetFileGroup( "PSI Frames", "./frames" ), file, "rt" );
			if( file_read )
			{
				sack_fseek( file_read, 0, SEEK_END );
				zz = ftell( file_read );
				sack_fseek( file_read, 0, SEEK_SET );

				size = zz;
				buffer = Allocate( zz );
				sack_fread( buffer, 1, zz, file_read );
				sack_fclose( file_read );
				//lprintf( "loaded font blob %s %d %p", file, zz, buffer );
			}
		}
#else
		{
			TEXTSTR tmp;
			buffer = OpenSpace( NULL, tmp = sack_prepend_path( GetFileGroup( "PSI Frames", "./frames" ), file ), &size );
			Release( tmp );
		}
#endif
      //if( !buffer || !size )
		//	file = _file; // restore filenaem to mark on the dialog, else use new filename cuase we loaded success
	}
	if( buffer && size )
	{
		ParseXMLFrame( buffer, size );
		Release( buffer );
	}

	if( !l.frame )
	{
		//create_editable_dialog:
		{
			PSI_CONTROL frame;
         TEXTSTR tmp;
			frame = CreateFrame( tmp = sack_prepend_path( GetFileGroup( "PSI Frames", "./frames" )
																	  , file )
									 , 0, 0
									 , 420, 250, 0, NULL );
			frame->save_name = StrDup( tmp );
			if( tmp )
			{
				Release( tmp );
				tmp = NULL;
			}
			DisplayFrameOver( frame, parent );
			EditFrame( frame, TRUE );
			CommonWaitEndEdit( &frame ); // edit may result in destroying (cancelling) the whole thing
			if( frame )
			{
				// save it, and result with it...
				SaveXMLFrame( frame, tmp );
				LeaveCriticalSec( &l.cs );
				return frame;
			}
			LeaveCriticalSec( &l.cs );
			return NULL;
		}
	}
	frame = l.frame;
	l.frame->save_name = StrDup( file );
	if( tmp )
		Release( tmp );
	LeaveCriticalSec( &l.cs );
	return frame;
}

PSI_CONTROL LoadXMLFrameEx( CTEXTSTR file DBG_PASS )
//PSI_CONTROL  LoadXMLFrame( char *file )
{
   return LoadXMLFrameOverEx( NULL, file DBG_RELAY );
}
PSI_XML_NAMESPACE_END


