#include <stdhdrs.h>
#include <sharemem.h>
#include <filesys.h>
#include <system.h>
#include <configscript.h>

//#define DEBUG_GLOBAL_REGISTRATION
#define REGISTRY_STRUCTURE_DEFINED
//#define PROCREG_SOURCE
// include this first so we can have the namespace...
#include <procreg.h>
#undef REGISTRY_STRUCTURE_DEFINED
#include "registry.h"

PROCREG_NAMESPACE

struct procreg_local_tag {
	struct {
		_32 bInterfacesLoaded : 1;
		_32 bIndexNameTable : 1;
	} flags;

	PTREEDEF Names;
	PTREEROOT NameIndex;
	PTREEDEFSET TreeNodes;
	PNAMESET NameSet;

	PNAMESPACE NameSpace;
	PLIST TransationSpaces;

   int translations; // open group ID

	TEXTCHAR *config_filename;
	FILE *file;
   CRITICALSECTION csName;
   //gcroot<System::IO::FileStream^> fs;
};

//#ifdef __STATIC__
//#ifdef __cplusplus
//#define procreg_local_data procreg_local_data_plusplus
//#define l (*procreg_local_data_plusplus)
//#define SAFE_INIT()      if( !procreg_local_data )	SimpleRegisterAndCreateGlobalWithInit( procreg_local_data_plusplus, InitGlobalSpace );
//
//#else
#define l (*procreg_local_data)
#define SAFE_INIT()      if( !procreg_local_data )	SimpleRegisterAndCreateGlobalWithInit( procreg_local_data, InitGlobalSpace );
//#endif
static struct procreg_local_tag *procreg_local_data;
//#else
//static struct procreg_local_tag l;
//#endif



static CTEXTSTR SaveName( CTEXTSTR name );
PTREEDEF GetClassTreeEx( PTREEDEF root
										, PTREEDEF name_class
										, PTREEDEF alias, LOGICAL bCreate );
#define GetClassTree( root, name_class ) GetClassTreeEx( root, name_class, NULL, TRUE )

//---------------------------------------------------------------------------


static int CPROC SavedNameCmpEx(CTEXTSTR dst, CTEXTSTR src, size_t srclen)
{
	// NUL does not nessecarily terminate strings
	// instead slave off the length...
    TEXTCHAR f,last;
	size_t l1 = srclen; // one for length, one for nul
	size_t l2 = dst[-1] - 2; // one for length, one for nul
	// case insensitive loop..
	//lprintf( WIDE("Compare %s(%d) vs %s(%d)"), src, l1, dst, l2 );
	// interesting... first sort by length
	// and then by content?
	//if( l1 != l2 )
    //  return l2-l1;
	do {
		if( (*src) == '\\' || (*src)=='/' )
		{
         l1 = 0; // no more length .. should have gotten a matched length on dst...
			break;
		}
		if ( ((f = (TEXTCHAR)(*(dst++))) >= 'A') && (f <= 'Z') )
			f -= ('A' - 'a');
		if ( ((last = (TEXTCHAR)(*(src++))) >= 'A') && (last <= 'Z') )
			last -= ('A' - 'a');
		--l2;
      --l1;
	} while ( l2 && l1 && (f == last) );
	//lprintf( WIDE("Results to compare...%d,%d  %c,%c"), l1, l2, f, last );
	// if up to the end of some portion of the strings matched...
	if( !f && !last )
	{
		return 0;
	}
	if( !l2 && !l1 )
	{
		return f-last;
	}
	if( f == last )
	{
		if( l2 && !l1 )
			return 1;
		if( l1 && !l2 )
			return -1;
	}
	return(f - last);
}
//---------------------------------------------------------------------------

static int CPROC SavedNameCmp(CTEXTSTR dst, CTEXTSTR src)
{
   //lprintf( WIDE("Compare names... (tree) %s,%s"), dst, src );
   return SavedNameCmpEx( dst, src, src[-1]-2 );
}
//---------------------------------------------------------------------------

static TEXTSTR StripName( TEXTSTR buf, CTEXTSTR name )
{
	TEXTSTR savebuf = buf;
	int escape = 0;
	if( !name )
	{
		buf[0] = 0;
		return buf;
	}
	while( name[0] )
	{
		if( name[0] == '\\' )
		{
			escape = 1;
		}
		else
		{
         // drop spaces...
			if( escape || ( name[0] > ' ' && name[0] < 127 ) )
			{
				*(buf++) = name[0];
			}
			escape = 0;
		}
		name++;
	}
   buf[0] = 0;
   return savebuf;
}

//---------------------------------------------------------------------------

static TEXTSTR GetFullName( CTEXTSTR name )
{
	int len;
	int out;
	int totlen = name[-1];
	TEXTSTR result;
	//for( len = 0; name[len] != 0 || name[len+1] != 0; len++ );
	result = NewArray( TEXTCHAR, totlen + 1);
	out = 0;
	for( len = 0; len < totlen; len++ )
		if( name[len] )
			result[out++] = name[len];
	result[out] = 0;
	return result;
	

}

//---------------------------------------------------------------------------


static CTEXTSTR DressName( TEXTSTR buf, CTEXTSTR name )
{
	TEXTSTR savebuf = buf;
	savebuf[0] = 2;
	buf++;
	if( !name )
	{
		savebuf[0] = 0;
		return buf;
	}
	while( name[0] )
	{
		if( name[0] == '/' || name[0] == '\\' )
			break;
		if( name[0] < ' ' || name[0] >= 127 )
		{
			savebuf[0]++;
			(*buf++) = '\\';
			savebuf[0]++;
			(*buf++) = name[0];
		}
		else
		{
			savebuf[0]++;
			(*buf++) = name[0];
		}
		name++;
	}
   buf[0] = 0;
   return savebuf + 1;
}

//---------------------------------------------------------------------------

static CTEXTSTR DoSaveName( CTEXTSTR stripped, size_t len )
{
	PNAMESPACE space = l.NameSpace;
	TEXTCHAR *p;
	// cannot save 0 length strings.
	if( !stripped || !stripped[0] || !len )
	{
		//lprintf( WIDE("zero length string passed") );
		return NULL;
	}
	EnterCriticalSec( &l.csName );
	if( l.flags.bIndexNameTable )
	{
		POINTER p;
		p = FindInBinaryTree( l.NameIndex, (PTRSZVAL)stripped );
		if( p )
		{
			LeaveCriticalSec( &l.csName );
			return ((CTEXTSTR)p);
		}
	}
	else
	{
		for( space = l.NameSpace; space; space = space->next )
		{
			p = space->buffer;
			while( p[0] && len )
			{
				//lprintf( WIDE("Compare %s(%d) vs %s(%d)"), p+1, p[0], stripped,len );
				if( SavedNameCmpEx( p+1, stripped, len ) == 0 )
				{
					LeaveCriticalSec( &l.csName );
					return (CTEXTSTR)p+1;
				}
				p +=
#if defined( __ARM__ ) || defined( UNDER_CE )
					(
#endif
					 p[0]
#if defined( __ARM__ ) || defined( UNDER_CE )
					 +3 ) & 0xFC;
#endif
				;
			}
		}
	}
	for( space = l.NameSpace; space; space = space->next )
	{
		if( ( space->nextname + len ) < ( NAMESPACE_SIZE - 3 ) )
		{
			p = NULL;
         break;
		}
	}
	if( !space || !p )
	{
		size_t alloclen;
		if( !space )
		{
			space = (PNAMESPACE)Allocate( sizeof( NAMESPACE ) );
			space->nextname = 0;
			LinkThing( l.NameSpace, space );
		}

		MemCpy( p = space->buffer + space->nextname + 1, stripped,(_32)(sizeof( TEXTCHAR)*(len + 1)) );
		p[len] = 0; // make sure we get a null terminator...
		alloclen = (len + 2);
					// +2 1 for byte of len, 1 for nul at end.

		space->buffer[space->nextname] = (TEXTCHAR)(alloclen);
#if defined( __ARM__ ) || defined( UNDER_CE )
		alloclen = ( alloclen + 3 ) & 0xFC;
					// +3&0xFC rounds to next full dword segment
					// arm requires this name be aligned on a dword boundry
               // because later code references this as a DWORD value.
#endif
		space->nextname += (_32)alloclen;
		space->buffer[space->nextname] = 0;

		if( l.flags.bIndexNameTable )
		{
			AddBinaryNode( l.NameIndex, p, (PTRSZVAL)p );
			BalanceBinaryTree( l.NameIndex );
		}
	}
	LeaveCriticalSec( &l.csName );
   return (CTEXTSTR)p;
}

//---------------------------------------------------------------------------

static CTEXTSTR SaveName( CTEXTSTR name )
{
	if( name )
	{
		size_t len = StrLen( name );
		size_t n;
		for( n = 0; n < len; n++ )
			if( name[n] == '\\' || name[n] == '/' )
			{
				len = n;
				break;
			}
		return DoSaveName( name, len );
	}
	return NULL;
}

//---------------------------------------------------------------------------
CTEXTSTR SaveNameConcatN( CTEXTSTR name1, ... )
#define SaveNameConcat(n1,n2) SaveNameConcatN( (n1),(n2),NULL )
{
	// space concat since that's eaten by strip...
	TEXTCHAR stripbuffer[256];
	size_t len = 0;
	CTEXTSTR namex;
	va_list args;
	va_start( args, name1 );

	for( namex = name1;
			 namex;
			 namex = va_arg( args, CTEXTSTR ) )
	{
		size_t newlen;
		// concat order for libraries is
		// args, return type, library, library_procname
		// this is appeneded to the key value FUNCTION
		//lprintf( WIDE("Concatting %s"), namex );
		newlen = StrLen( StripName( stripbuffer + len, namex ) );
		//if( newlen )
		newlen++;
		len += newlen;
	}
   // and add another - final part of string is \0\0
	//stripbuffer[len] = 0;
   //len++;
	return DoSaveName( stripbuffer, len );
}

//---------------------------------------------------------------------------
CTEXTSTR SaveText( CTEXTSTR text )
#define SaveNameConcat(n1,n2) SaveNameConcatN( (n1),(n2),NULL )
{
	return DoSaveName( text, StrLen( text ) );
}

//---------------------------------------------------------------------------

CTEXTSTR TranslateText( CTEXTSTR text )
{
	return NULL;
}

//---------------------------------------------------------------------------

void LoadTranslation( CTEXTSTR translation_name, CTEXTSTR filename )
{
   //FILE *input = sack_fopen( l.translations, filename, "rb" );
}

//---------------------------------------------------------------------------

void SaveNames( void )
{
}

//---------------------------------------------------------------------------

void RecoverNames( void )
{
}

//---------------------------------------------------------------------------

static void CPROC KillName( POINTER user, PTRSZVAL key )
{
	PNAME name = (PNAME)user;
	if( name->tree.Tree )
	{
	}
	else if( name->flags.bValue )
	{
	}
	else if( name->flags.bProc )
	{
	}
	else if( name->flags.bData )
	{
	}
   DeleteFromSet( NAME, &l.Names, user );
//	Release( user );
}

//---------------------------------------------------------------------------

// p would be the global space, but it's also already set in it's correct spot
static void CPROC InitGlobalSpace( POINTER p, PTRSZVAL size )
{
	(*(struct procreg_local_tag*)p).Names = (PTREEDEF)GetFromSet( TREEDEF, &(*(struct procreg_local_tag*)p).TreeNodes );
	(*(struct procreg_local_tag*)p).Names->Magic = MAGIC_TREE_NUMBER;
	(*(struct procreg_local_tag*)p).Names->Tree = CreateBinaryTreeExx( BT_OPT_NODUPLICATES, (int(CPROC *)(PTRSZVAL,PTRSZVAL))SavedNameCmp, KillName );

	// enable name indexing.
	// if we have 500 names, 9 searches is much less than 250 avg
	(*(struct procreg_local_tag*)p).flags.bIndexNameTable = 1;
	(*(struct procreg_local_tag*)p).NameIndex = CreateBinaryTreeExx( BT_OPT_NODUPLICATES, (int(CPROC *)(PTRSZVAL,PTRSZVAL))SavedNameCmp, KillName );

}

static void Init( void )
{
	SAFE_INIT();
}

PRIORITY_PRELOAD( InitProcreg, NAMESPACE_PRELOAD_PRIORITY )
{
	Init();
}

//---------------------------------------------------------------------------

static PTREEDEF AddClassTree( PTREEDEF class_root, TEXTCHAR *name, PTREEROOT root, int bAlias )
{
	if( root && class_root )
	{
		PNAME classname = GetFromSet( NAME, &l.NameSet ); //Allocate( sizeof( NAME ) );
		//MemSet( classname, 0, sizeof( NAME ) );
		classname->flags.bAlias = bAlias;
		classname->name = SaveName( name );

		classname->tree.Magic = MAGIC_TREE_NUMBER;
		classname->tree.Tree = root;
		classname->flags.bTree = TRUE;
		//lprintf( WIDE("Adding class tree thing %s to %s"), name, classname->name );
		if( !AddBinaryNode( class_root->Tree, classname, (PTRSZVAL)classname->name ) )
		{
			Log( WIDE("For some reason could not add new class tree to tree!") );
			DeleteFromSet( NAME, &l.NameSet, classname );
			return NULL;
		}
		return &classname->tree;
	}
   return NULL;
}

//---------------------------------------------------------------------------

// if name_class is NULL then root is returned.
// if name_class is not NULL then if name_class references
// PTREEDEF structure, then name_class is returned.
// if root is NULL then it is set to l.nmaes... if this library has
// never been initialized it will return NULL.
// if name_class does not previously exist, then it is created.
// There is no protection for someone to constantly create large trees just
// by asking for them.
PTREEDEF GetClassTreeEx( PTREEDEF root, PTREEDEF _name_class, PTREEDEF alias, LOGICAL bCreate )
{
	PTREEDEF class_root;

	//Init();
	if( !root )
	{
      SAFE_INIT();

		if( !l.Names )
			InitGlobalSpace( &l, 0);
		root = l.Names;
	}// fix root...
	class_root = root;

	if(
#if defined( __ARM__ ) || defined( UNDER_CE )
	  // if its odd, it comes from the name space
      // (savename)
		(((PTRSZVAL)class_root)&0x3) ||
#endif
		(class_root->Magic != MAGIC_TREE_NUMBER) )
	{
		// if root name is passed as a NAME, then resolve it
      // assuming the root of all names as the root...
      class_root = GetClassTreeEx( l.Names, class_root, NULL, bCreate );
	}

	if( _name_class )
	{
		if(
#if defined( __ARM__ ) || defined( UNDER_CE )
	  // if its odd, it comes from the name space
      // (savename)
		    !(((PTRSZVAL)_name_class)&0x3) &&
#endif
			(_name_class->Magic == MAGIC_TREE_NUMBER) )
		{
			return _name_class;
		}
		else
		{
			size_t buflen = 0;
			//TEXTCHAR *original;
			TEXTCHAR *end, *start;
			CTEXTSTR name_class = (CTEXTSTR)_name_class;
			size_t len = StrLen( name_class ) + 1;
			PNAME new_root;
			if( len > buflen )
			{
				//if( original )
				//   Release( original );
				//original = Allocate( len + 32 );
				buflen = len + 32;
			}
			//
			//MemCpy( original, name_class, len );
			start = (TEXTCHAR*)name_class;
			do
			{
				end = start;
				do
				{
					end = (TEXTCHAR*)pathchr( end );
					if( end == start )
					{
						end = start = start+1;
						continue;
					}
					if( !end || ((pathchr(end+1) - end) != 1) )
						break;
					{
						TEXTCHAR *p = end;
						while( p[0] )
						{
							p[0] = p[1];
							p++;
							//(p++)[0] = p[1];
						}
					}
					end++;
				}
				while( 1 );
				//if( end )
				//	end[0] = 0;
				{
					TEXTCHAR buf[256];
					new_root = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, start ) );
               //lprintf( WIDE("Finding %s(%d)=%s"), buf+1, buf[0], start );
				}
				if( !new_root )
				{
					if( !bCreate )
                  return NULL;
					if( alias && !end )
					{
                  //lprintf( WIDE("name not found, adding...!end && alias") );
						class_root = AddClassTree( class_root
														 , start
														 , alias->Tree
														 , TRUE );
					}
					else
					{
						// interesting note - while searching for
						// a member, branches are created.... should consider
						// perhaps offering an option to read for class root without creating
                  // however it gives one an idea of what methods might be avaialable...
						//lprintf( WIDE("name not found, adding.. %s %s"), start, class_root );
						class_root = AddClassTree( class_root
														 , start
														 , CreateBinaryTreeExx( BT_OPT_NODUPLICATES
																					 , (int(CPROC *)(PTRSZVAL,PTRSZVAL))SavedNameCmp
																					 , KillName )
														 , FALSE
														 );
					}
				}
				else
				{
					class_root = &new_root->tree;
				}
				if( end )
					start = end + 1;
				else
					break;
			}
			while( class_root && start[0] );
         //Release( original );
		}
	}
	return class_root;
}

//---------------------------------------------------------------------------
PROCREG_PROC( PCLASSROOT, CheckClassRoot )( CTEXTSTR name_class )
{
   return GetClassTreeEx( l.Names, (PCLASSROOT)name_class, NULL, FALSE );
}

//---------------------------------------------------------------------------

PROCREG_PROC( PTREEDEF, GetClassRootEx )( PCLASSROOT root, CTEXTSTR name_class )
{
	return GetClassTreeEx( root, (PCLASSROOT)name_class, NULL, TRUE );
}

PROCREG_PROC( PTREEDEF, GetClassRoot )( CTEXTSTR name_class )
{
	SAFE_INIT();
	return GetClassRootEx( l.Names, name_class );
}
#ifdef __cplusplus
PROCREG_PROC( PTREEDEF, GetClassRootEx )( PCLASSROOT root, PCLASSROOT name_class )
{
	return GetClassTreeEx( root, (PCLASSROOT)name_class, NULL, TRUE );
}

PROCREG_PROC( PTREEDEF, GetClassRoot )( PCLASSROOT name_class )
{
	SAFE_INIT();
	return GetClassRootEx( l.Names, (PCLASSROOT)name_class );
}
#endif
//---------------------------------------------------------------------------

int AddNode( PTREEDEF class_root, POINTER data, PTRSZVAL key )
{
	if( class_root )
	{
		TEXTCHAR buf[256];
		PNAME oldname = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, (CTEXTSTR)key ) );
		if( oldname )
		{
			Log( WIDE("Name already in the tree...") );
			return FALSE;
		}
		else
		{
         //lprintf( WIDE("addnode? a data ndoe - create data structure") );
			if( !AddBinaryNode( class_root->Tree, data, key ) )
			{
				Log( WIDE("For some reason could not add new name to tree!") );
				return FALSE;
			}
		}
		return TRUE;
	}
	Log( WIDE("Nowhere to add the node...") );
	return FALSE;
}

//---------------------------------------------------------------------------

static int CPROC MyStrCmp( PTRSZVAL s1, PTRSZVAL s2 )
{
	return StrCaseCmp( (TEXTCHAR*)s1, (TEXTCHAR*)s2 );
}
//---------------------------------------------------------------------------
#undef RegisterFunctionExx
PROCREG_PROC( LOGICAL, RegisterFunctionExx )( PCLASSROOT root
													 , PCLASSROOT name_class
													 , CTEXTSTR public_name
													 , CTEXTSTR returntype
													 , PROCEDURE proc
													 , CTEXTSTR args
													 , CTEXTSTR library
													 , CTEXTSTR real_name
													  DBG_PASS
													)
{
	if( root || name_class )
	{
		PNAME newname = GetFromSet( NAME, &l.NameSet );//Allocate( sizeof( NAME ) );
		TEXTCHAR strippedargs[256];
		CTEXTSTR func_name = real_name?real_name:public_name;
		CTEXTSTR root_func_name = func_name;
		PTREEDEF class_root = (PTREEDEF)GetClassTree( root, name_class );
		int tmp;
		MemSet( newname, 0, sizeof( NAME ) );
		newname->flags.bProc = 1;
		// this is kinda messed up...
		newname->name = SaveName( public_name );
		newname->data.proc.library = SaveName( library );
		newname->data.proc.procname = SaveName( real_name );
		//newname->data.proc.ret = SaveName( returntype );
		for( tmp = 0; func_name[tmp]; tmp++ )
			if( func_name[tmp] == '/' ||
				func_name[tmp] == '\\' )
			{
				func_name = func_name + tmp + 1;
				tmp = -1;
			}
		if( func_name != root_func_name )
		{
			int len;
			TEXTSTR new_root_func_name = NewArray( TEXTCHAR, len = ( func_name - root_func_name ) );
			StrCpyEx( new_root_func_name, root_func_name, len );
			new_root_func_name[len-1] = 0;
			//lprintf( "trimmed name would be %s  /   %s", new_root_func_name, func_name );
			class_root = GetClassTree( class_root, (PCLASSROOT)new_root_func_name );
			Release( new_root_func_name );
		}
		//newname->data.proc.args = SaveName( StripName( strippedargs, args ) );

		newname->data.proc.name = SaveNameConcatN( StripName( strippedargs, args )
															  , returntype
															  , library?library:WIDE("_")
															  , func_name
															  , NULL
															  );
		newname->data.proc.proc = proc;
		if( class_root )
		{
			PNAME oldname;
			oldname = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)newname->name);
			if( oldname )
			{
				if( !oldname->data.proc.proc )
				{
					// old branch location might have existed, but no value assigned...
					//lprintf( WIDE( "overloading prior %p with %p and %p with %p" )
					//		 , oldname->data.proc.proc, proc
					//		 , oldname->data.proc.name, newname->data.proc.name
					//		 );
               oldname->flags.bProc = 1;
					oldname->data.proc.proc = proc;
					oldname->data.proc.name = newname->data.proc.name;
					oldname->data.proc.library = newname->data.proc.library;
               oldname->data.proc.procname = newname->data.proc.procname;
					newname->data.proc.name = NULL;
				}
				else if( oldname->data.proc.proc == proc )
					Log( WIDE("And fortunatly it's the same address... all is well...") );
				else
				{
					TEXTSTR s1, s2;
					_xlprintf( 2 DBG_RELAY)( WIDE("proc %s/%s regisry by %s of %s(%s) conflicts with %s(%s)...")
												  , (CTEXTSTR)name_class?(CTEXTSTR)name_class:WIDE("@")
												  , public_name?public_name:WIDE("@")
												  , newname->name
												  , s1 = GetFullName( newname->data.proc.name )
													//,library
												  , newname->data.proc.procname
												  , s2 = GetFullName( oldname->data.proc.name )
												  //,library
													, oldname->data.proc.procname );
					Release( s1 );
					Release( s2 );
					// perhaps it's same in a different library...
					Log( WIDE("All is not well - found same funciton name in tree with different address. (ignoring second) ") );
					//DumpRegisteredNames();
					//DebugBreak();
				}
				DeleteFromSet( NAME, &l.NameSet, newname );
				return TRUE;
			}
			else
			{
				if( !AddBinaryNode( class_root->Tree, (PCLASSROOT)newname, (PTRSZVAL)newname->name ) )
				{
					Log( WIDE("For some reason could not add new name to tree!") );
					DeleteFromSet( NAME, &l.NameSet, newname );
				}
			}
			{
				//PTREEDEF root = GetClassRoot( newname );
				newname->tree.Magic = MAGIC_TREE_NUMBER;
				newname->tree.Tree = CreateBinaryTreeExx( 0 // dups okay BT_OPT_NODUPLICATES
																	 , (int(CPROC *)(PTRSZVAL,PTRSZVAL))MyStrCmp
																	 , KillName );
#ifdef _DEBUG
				{
					CTEXTSTR name = pathrchr( pFile );
               // chop the trailing filename, removing path of filename.
					if( name )
						name++;
					else
						name = pFile;
					RegisterValue( (CTEXTSTR)&newname->tree, WIDE( "Source File" ), name );
					RegisterIntValue( (CTEXTSTR)&newname->tree, WIDE( "Source Line" ), nLine );
				}
#endif
			}
		}
		else
		{
			lprintf( WIDE("I'm relasing this name!?") );
			DeleteFromSet( NAME, &l.NameSet, newname );
		}
		return 1;
	}
	return FALSE;
}
#ifdef __cplusplus
PROCREG_PROC( LOGICAL, RegisterFunctionExx )( CTEXTSTR root
													 , CTEXTSTR name_class
													 , CTEXTSTR public_name
													 , CTEXTSTR returntype
													 , PROCEDURE proc
													 , CTEXTSTR args
													 , CTEXTSTR library
													 , CTEXTSTR real_name
													  DBG_PASS
														  )
{
	return RegisterFunctionExx( (PCLASSROOT)root, (PCLASSROOT)name_class, public_name, returntype
                              , proc, args, library, real_name DBG_RELAY );
}
#endif

//---------------------------------------------------------------------------

int ReleaseRegisteredFunctionEx( PCLASSROOT root, CTEXTSTR name_class
							 , CTEXTSTR public_name
							 )
{
	PTREEDEF class_root = GetClassTree( root, (PCLASSROOT)name_class );
   TEXTCHAR buf[256];
	PNAME node = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, public_name ) );
	if( node )
	{
		if( node->flags.bProc )
		{
			UnloadFunction( &node->data.proc.proc );
         //node->data.proc.proc = NULL;
			node->flags.bProc = 0;
         return 1;
		}
	}
   return 0;
}

//---------------------------------------------------------------------------

PROCREG_PROC( int, RegisterProcedureExx )( PCLASSROOT root
														, CTEXTSTR name_class
														, CTEXTSTR public_name
														, CTEXTSTR returntype
														, CTEXTSTR library
														, CTEXTSTR name
														, CTEXTSTR args
														 DBG_PASS
														)
{
	//PROCEDURE proc = (PROCEDURE)LoadFunction( library, name );
	//if( proc )
	{
		return RegisterFunctionExx( root, (PCLASSROOT)name_class
										  , public_name
										  , returntype
										  , NULL
										  , args
										  , library
                                , name
											 DBG_RELAY );
	}
   //return 0;
}

#undef RegisterProcedureEx
PROCREG_PROC( int, RegisterProcedureEx )( CTEXTSTR name_class
                                        , CTEXTSTR public_name
													 , CTEXTSTR returntype
                                        , CTEXTSTR library
													 , CTEXTSTR name
													 , CTEXTSTR args
                                         DBG_PASS
													 )
{
   return RegisterProcedureExx( NULL, name_class, public_name, returntype, library, name, args DBG_RELAY );
}


PROCREG_PROC( PROCEDURE, ReadRegisteredProcedureEx )( PCLASSROOT root
                                                    , CTEXTSTR returntype
																	 , CTEXTSTR parms
																  )
{
	PTREEDEF class_root = GetClassTree( root, NULL );
	PNAME oldname = (PNAME)GetCurrentNode( class_root->Tree );
	if( oldname )
	{
		PROCEDURE proc = oldname->data.proc.proc;
		if( !proc && ( oldname->data.proc.library && oldname->data.proc.procname ) )
		{
			proc = (PROCEDURE)LoadFunction( oldname->data.proc.library
													, oldname->data.proc.procname );
			//lprintf( WIDE("Found a procedure %s=%p  (%p)"), name, oldname, proc );
			// should compare whether the types match...
			if( !proc )
			{
				Log( WIDE("Failed to load function when requested from tree...") );
			}
			oldname->data.proc.proc = proc;
		}
		return oldname->data.proc.proc;
	}
   return NULL;
}

//---------------------------------------------------------------------------
// can use the return type and args to validate the correct
// type of routine is called...
// name is not the function name, but rather the public/common name...
// this name may optionally include a # remark detailing more information
// about the name... the comparison of this name is done up to the #
// and data after a # is checked only if both values have a sub-comment.
// library name is not checked.

// this routine may find more than 1 routine which matches the given
// criteria.  return type and args may be NULL indicating a care-less
// approach.

void DumpRegisteredNamesWork( PTREEDEF tree, int level );

#undef GetRegisteredProcedureExx
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( PCLASSROOT root, PCLASSROOT name_class, CTEXTSTR returntype, CTEXTSTR name, CTEXTSTR args )
#define GetRegisteredProcedureExx GetRegisteredProcedureExxx
{
	PTREEDEF class_root = GetClassTree( root, name_class );
	if( class_root )
	{
		PNAME oldname;
      //TEXTCHAR buf[256];
		//lprintf( WIDE("Found class %s=%p for %s"), name_class, class_root, name );
		//DumpRegisteredNamesWork( class_root, 5 );
		oldname = (PNAME)LocateInBinaryTree( class_root->Tree, (PTRSZVAL)name, NULL );
		//oldname = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, name ) );
		if( oldname )
		{
			PROCEDURE proc = oldname->data.proc.proc;
			if( !proc && ( oldname->data.proc.library && oldname->data.proc.procname ) )
			{
				proc = (PROCEDURE)LoadFunction( oldname->data.proc.library
														, oldname->data.proc.procname );
				//lprintf( WIDE("Found a procedure %s=%p  (%p)"), name, oldname, proc );
				// should compare whether the types match...
				if( !proc )
				{
               Log( WIDE("Failed to load function when requested from tree...") );
				}
				oldname->data.proc.proc = proc;
			}
			return oldname->data.proc.proc;
		}
		//else
      //   lprintf( WIDE("Failed to find %s in the tree"), buf );
	}
   //lprintf( WIDE("Failed to find the class root...") );
	return NULL;
}

#ifdef __cplusplus
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( CTEXTSTR root, CTEXTSTR name_class, CTEXTSTR returntype, CTEXTSTR name, CTEXTSTR args )
{
   return GetRegisteredProcedureExxx( (PCLASSROOT)root, (PCLASSROOT)name_class, returntype, name, args );
}
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR returntype, CTEXTSTR name, CTEXTSTR args )
{
   return GetRegisteredProcedureExxx( (PCLASSROOT)root, (PCLASSROOT)name_class, returntype, name, args );
}
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( CTEXTSTR root, PCLASSROOT name_class, CTEXTSTR returntype, CTEXTSTR name, CTEXTSTR args )
{
   return GetRegisteredProcedureExxx( (PCLASSROOT)root, name_class, returntype, name, args );
}
#endif
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureEx )( PCLASSROOT name_class, CTEXTSTR returntype, CTEXTSTR name, CTEXTSTR args )
{
   return GetRegisteredProcedureExx( l.Names, name_class, returntype, name, args );
}
#ifdef __cplusplus
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureEx )( CTEXTSTR name_class, CTEXTSTR returntype, CTEXTSTR name, CTEXTSTR args )
{
   return GetRegisteredProcedureExx( l.Names, name_class, returntype, name, args );
}
#endif

//---------------------------------------------------------------------------

void DumpRegisteredNamesWork( PTREEDEF tree, int level )
{
	PNAME name;
	PVARTEXT pvt;
	PTEXT pText;
   int bLogging = SetAllocateLogging( FALSE );
   // at least save the create/destroy uselessness...
	if( !tree->Tree )
	{
      SetAllocateLogging( bLogging );
		return;
	}
	pvt = VarTextCreateExx( 512, 1024 );
	for( name = (PNAME)GetLeastNode( tree->Tree );
		  name;
		  name = (PNAME)GetGreaterNode( tree->Tree ) )
	{
		int n;
		for( n = 0; n < level; n++ )
			vtprintf( pvt, WIDE("   ") );
		vtprintf( pvt, WIDE("%s"), name->name );
		if( name->flags.bValue )
		{
			vtprintf( pvt, WIDE(" = ") );
			if( name->flags.bIntVal )
				vtprintf( pvt, WIDE("[%ld]"), name->data.name.iValue );
			if( name->flags.bStringVal )
				vtprintf( pvt, WIDE("\"%s\""), name->data.name.sValue );
			if( name->flags.bProc )
            vtprintf( pvt, WIDE("*%p"), name->data.proc.proc );
		}
		else if( name->flags.bProc )
		{
			CTEXTSTR p = name->data.proc.name;
			if( p )
			{
				size_t len = p[-1] - 2;
				vtprintf( pvt, WIDE(" = ") );
				while( len )
				{
					size_t tmp;
					vtprintf( pvt, WIDE("%s "), p );
					tmp = StrLen( p ) + 1;
					len-= tmp;
					p += tmp;
				}
				vtprintf( pvt, WIDE("*%p"), name->data.proc.proc );

			}
		}
		pText = VarTextGet( pvt );
		xlprintf(LOG_INFO)( WIDE("%s"), GetText( pText ) );
		LineRelease( pText );
		DumpRegisteredNamesWork( &name->tree, level + 1 );
	}
	VarTextDestroy( &pvt );
	SetAllocateLogging( bLogging );
}

//---------------------------------------------------------------------------

struct browse_index
{
	PCLASSROOT current_limbs;
   PCLASSROOT current_branch;
};

PROCREG_PROC( int, NewNameHasBranches )( PCLASSROOT *data )
{
   struct browse_index *class_root = (struct browse_index*)(*data);
	PNAME name;
	name = (PNAME)GetCurrentNode( class_root->current_branch->Tree );
	return name->flags.bTree; // may also have a value, but it was created as a path node in the tree
}

PROCREG_PROC( int, NameHasBranches )( PCLASSROOT *data )
{
   PTREEDEF class_root;
	PNAME name;
	class_root = (PTREEDEF)*data;
	name = (PNAME)GetCurrentNode( class_root->Tree );
	return name->flags.bTree; // may also have a value, but it was created as a path node in the tree
}


int NewNameIsAlias( PCLASSROOT *data )
{
   struct browse_index *class_root = (struct browse_index*)(*data);
	PNAME name;
	name = (PNAME)GetCurrentNode( class_root->current_branch->Tree );
	return name->flags.bAlias; // may also have a value, but it was created as a path node in the tree
}

int NameIsAlias( PCLASSROOT *data )
{
   PTREEDEF class_root;
	PNAME name;
	class_root = (PTREEDEF)*data;
	name = (PNAME)GetCurrentNode( class_root->Tree );
	return name->flags.bAlias; // may also have a value, but it was created as a path node in the tree
}

PROCREG_PROC( PCLASSROOT, GetCurrentRegisteredTree )( PCLASSROOT *data )
{
   PTREEDEF class_root;
	PNAME name;
	class_root = (PTREEDEF)*data;
	name = (PNAME)GetCurrentNode( class_root->Tree );
   if( name )
		return &name->tree;
   return NULL;
}



//---------------------------------------------------------------------------


PROCREG_PROC( CTEXTSTR, GetFirstRegisteredNameEx )( PCLASSROOT root, CTEXTSTR classname, PCLASSROOT *data )
#if 1
{
   PTREEDEF class_root;
	PNAME name;
   *data =
		class_root = GetClassTree( root, (PCLASSROOT)classname );
	if( class_root )
	{
		name = (PNAME)GetLeastNode( class_root->Tree );
		if( name )
		{
			//lprintf( WIDE("Resulting first name: %s"), name->name );
			return name->name;
		}
	}
   return NULL;
}
#else
{
   struct browse_index *class_root;
   //PTREEDEF class_root;
	PNAME name;
	*data = (PCLASSROOT)(class_root =New( struct browse_index ));
	class_root->current_branch = GetClassTree( root, (PCLASSROOT)classname );
	if( class_root )
	{
		name = (PNAME)GetLeastNode( class_root->current_branch->Tree );
      class_root->current_limbs = &name->tree;
		if( name )
		{
			//lprintf( WIDE("Resulting first name: %s"), name->name );
			return name->name;
		}
		else
		{
         Release( *data );
			(*data) = NULL;
		}
	}
   return NULL;
}                        
#endif

PROCREG_PROC( CTEXTSTR, GetFirstRegisteredName )( CTEXTSTR classname, PCLASSROOT *data )
{
   return GetFirstRegisteredNameEx( l.Names, classname, data );
}

//---------------------------------------------------------------------------

PROCREG_PROC( CTEXTSTR, GetNextRegisteredName )( PCLASSROOT *data )
#if 1
{
   PTREEDEF class_root;
	PNAME name;
   class_root = (PTREEDEF)*data;
	if( class_root )
	{
		name = (PNAME)GetGreaterNode( class_root->Tree );
		if( name )
		{
			//lprintf( WIDE("Resulting next name: %s"), name->name );
			return name->name;
		}
	}
	return NULL;

}
#else
{
   struct browse_index *class_root;
   //PTREEDEF class_root;
	PNAME name;
   class_root = (struct browse_index*)*data;
	if( class_root )
	{
		name = (PNAME)GetGreaterNode( class_root->current_branch->Tree );
      class_root->current_limbs = &name->tree;
		if( name )
		{
			//lprintf( WIDE("Resulting next name: %s"), name->name );
			return name->name;
		}
		else
		{
         Release( *data );
			*data = NULL;
		}
	}
   return NULL;

}
#endif

//---------------------------------------------------------------------------

PROCREG_PROC( void, DumpRegisteredNames )( void )
{
   if( l.Names )
		DumpRegisteredNamesWork( l.Names, 0 );
}

//---------------------------------------------------------------------------

PROCREG_PROC( void, DumpRegisteredNamesFrom )( PCLASSROOT root )
{
	DumpRegisteredNamesWork( GetClassRoot((CTEXTSTR)root), 0 );
}

//---------------------------------------------------------------------------

PROCREG_PROC( void, InvokeProcedure )( void )
{
}

//---------------------------------------------------------------------------

PROCREG_PROC( int, RegisterValueExx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name, int bIntVal, CTEXTSTR value )
{
	PTREEDEF class_root = GetClassTree( root, (PCLASSROOT)name_class );
	if( class_root )
	{
      TEXTCHAR buf[256];
		PNAME oldname = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, name ) );

		if( oldname )
		{
			oldname->flags.bValue = 1; // it is now a value, okay?
			if( bIntVal )
			{
				oldname->flags.bIntVal = 1;
				oldname->data.name.iValue = (PTRSZVAL)value;
			}
			else
			{
				oldname->flags.bStringVal = 1;
				oldname->data.name.sValue = SaveName( value );
			}
		}
		else
		{
			PNAME newname = GetFromSet( NAME, &l.NameSet ); //Allocate( sizeof( NAME ) );
			//MemSet( newname, 0, sizeof( NAME ) );
			if( name )
				newname->name = SaveName( name );
			newname->flags.bValue = 1;
			if( bIntVal )
			{
				newname->flags.bIntVal = 1;
				newname->data.name.iValue = (PTRSZVAL)value;
			}
			else
			{
				newname->flags.bStringVal = 1;
				newname->data.name.sValue = SaveName( value ); //StrDup( value );
			}
			if( !AddBinaryNode( class_root->Tree, newname, (PTRSZVAL)newname->name ) )
			{
				Log( WIDE("Failed to add name to tree...") );
			}
		}
		return TRUE;
	}
   return FALSE;
}

PROCREG_PROC( int, RegisterValueEx )( CTEXTSTR name_class, CTEXTSTR name, int bIntVal, CTEXTSTR value )
{
   return RegisterValueExx( l.Names, name_class, name, bIntVal, value );
}
//---------------------------------------------------------------------------

PROCREG_PROC( int, RegisterValue )( CTEXTSTR name_class, CTEXTSTR name, CTEXTSTR value )
{
   return RegisterValueEx( name_class, name, FALSE, value );
}
//---------------------------------------------------------------------------
PROCREG_PROC( int, RegisterIntValueEx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name, PTRSZVAL value )
{
   return RegisterValueExx( root, name_class, name, TRUE, (CTEXTSTR)value );
}

PROCREG_PROC( int, RegisterIntValue )( CTEXTSTR name_class, CTEXTSTR name, PTRSZVAL value )
{
   return RegisterValueEx( name_class, name, TRUE, (CTEXTSTR)value );
}

//---------------------------------------------------------------------------

int GetRegisteredStaticValue( PCLASSROOT root, CTEXTSTR name_class
									 , CTEXTSTR name
									 , CTEXTSTR *result
									 , int bIntVal )
{
	PTREEDEF class_root = GetClassTree( root, (PCLASSROOT)name_class );
   TEXTCHAR buf[256];
	PNAME oldname = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, name ));
	if( oldname )
	{
		if( bIntVal )
		{
         *((int*)result) = oldname->data.name.iValue;
			return TRUE;
		}
		else if( oldname->flags.bStringVal )
		{
         (*result) = oldname->data.name.sValue;
			return TRUE;
		}
	}
   return FALSE;
}

//---------------------------------------------------------------------------

PROCREG_PROC( CTEXTSTR, GetRegisteredValueExx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name, int bIntVal )
{
	PTREEDEF class_root;
	TEXTCHAR buf[256];
	PNAME oldname;
	class_root = GetClassTree( root, (PCLASSROOT)name_class );
	oldname = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, name ));
	if( oldname )
	{
		if( bIntVal )
			return (CTEXTSTR)oldname->data.name.iValue;
		else if( oldname->flags.bStringVal )
			return oldname->data.name.sValue;
	}
	return NULL;
}
#ifdef __cplusplus
PROCREG_PROC( CTEXTSTR, GetRegisteredValueExx )( CTEXTSTR root, CTEXTSTR name_class, CTEXTSTR name, int bIntVal )
{
	return GetRegisteredValueExx( (PCLASSROOT)root, name_class, name, bIntVal );
}
#endif

PROCREG_PROC( CTEXTSTR, GetRegisteredValueEx )( CTEXTSTR name_class, CTEXTSTR name, int bIntVal )
{
	return GetRegisteredValueExx( l.Names, name_class, name, bIntVal );
}
//---------------------------------------------------------------------------

PROCREG_PROC( CTEXTSTR, GetRegisteredValue )( CTEXTSTR name_class, CTEXTSTR name )
{
	return GetRegisteredValueEx( name_class, name, FALSE );
}

//---------------------------------------------------------------------------

PROCREG_PROC( int, GetRegisteredIntValue )( CTEXTSTR name_class, CTEXTSTR name )
{
/*
 * this has a warning - typecast to value of differnet size.
 * this is OK.  the value originates as an 'int' and is typecast to a
 * CTEXTSTR which this then down converts back to 'int'
 */
	return (int)(PTRSZVAL)GetRegisteredValueEx( (CTEXTSTR)name_class, name, TRUE );
}

#ifdef __cplusplus
PROCREG_PROC( int, GetRegisteredIntValue )( PCLASSROOT name_class, CTEXTSTR name )
{
/*
 * this has a warning - typecast to value of differnet size.
 * this is OK.  the value originates as an 'int' and is typecast to a
 * CTEXTSTR which this then down converts back to 'int'
 */
	return (int)(PTRSZVAL)GetRegisteredValueEx( (CTEXTSTR)name_class, name, TRUE );
}
#endif
//---------------------------------------------------------------------------

PROCREG_PROC( PCLASSROOT, RegisterClassAliasEx )( PCLASSROOT root, CTEXTSTR original, CTEXTSTR alias )
{
	PTREEDEF class_root = GetClassTreeEx( root, (PCLASSROOT)original, NULL, TRUE );
	//lprintf( WIDE("Registering alias %s=%s %p"), alias, original, class_root );
	return GetClassTreeEx( root, (PCLASSROOT)alias, class_root, TRUE );
}

//---------------------------------------------------------------------------

PROCREG_PROC( PCLASSROOT, RegisterClassAlias )( CTEXTSTR original, CTEXTSTR alias )
{
	SAFE_INIT();
	return RegisterClassAliasEx( l.Names, original, alias );
}

//---------------------------------------------------------------------------

PROCREG_PROC( PTRSZVAL, RegisterDataTypeEx )( PCLASSROOT root
												 , CTEXTSTR classname
												 , CTEXTSTR name
												 , PTRSZVAL size
												 , void (CPROC *Open)(POINTER,PTRSZVAL)
												 , void (CPROC *Close)(POINTER,PTRSZVAL) )
{
	PTREEDEF class_root = GetClassTreeEx( root, (PCLASSROOT)classname, NULL, TRUE );
	if( class_root )
	{
		PNAME pName = GetFromSet( NAME, &l.NameSet ); //(PNAME)Allocate( sizeof( NAME ) );
		//MemSet( pName, 0, sizeof( NAME ) );
		pName->flags.bData = 1;
		pName->name = SaveName( name );
		pName->data.data.Open = Open;
		pName->data.data.Close = Close;
		pName->data.data.size = size;
		pName->data.data.instances.Magic = MAGIC_TREE_NUMBER;
		pName->data.data.instances.Tree = CreateBinaryTreeExx( 0 // dups okay BT_OPT_NODUPLICATES
														, (int(CPROC *)(PTRSZVAL,PTRSZVAL))MyStrCmp
														, KillName );
		if( !AddNode( class_root, pName, (PTRSZVAL)pName->name ) )
		{
			DeleteFromSet( NAME, &l.NameSet, pName );
			return 0; // NULL
		}
		return (PTRSZVAL)pName;
	}
	return 0; // NULL
}

PROCREG_PROC( PTRSZVAL, RegisterDataType )( CTEXTSTR classname
												 , CTEXTSTR name
												 , PTRSZVAL size
												 , void (CPROC *Open)(POINTER,PTRSZVAL)
												 , void (CPROC *Close)(POINTER,PTRSZVAL) )
{
   return RegisterDataTypeEx( l.Names, classname, name, size, Open, Close );
}

//---------------------------------------------------------------------------

PROCREG_PROC( PTRSZVAL, MakeRegisteredDataTypeEx)( PCLASSROOT root
																 , CTEXTSTR classname
																 , CTEXTSTR name
																 , CTEXTSTR instancename
																 , POINTER data
																 , PTRSZVAL datasize
																 )
{
	PTREEDEF class_root = GetClassTree( root, (PCLASSROOT)classname );
	if( class_root )
	{
      	TEXTCHAR buf[256];
		PNAME pName = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, name ));
		if( !pName )
			pName = (PNAME)RegisterDataTypeEx( root, classname, name, datasize, NULL, NULL );

		if( pName && pName->flags.bData )
		{
			PDATADEF pDataDef = &pName->data.data;
			if( pDataDef )
			{
				if( !instancename )
				{
					TEXTCHAR buf[256];
					snprintf( buf, sizeof(buf), WIDE("%s_%d"), name, (int)pDataDef->unique++ );
					instancename = SaveName( buf );
				}
				else
					instancename = SaveName( instancename );
				{
					POINTER p;
               // look up prior instance...
					if( !( p = FindInBinaryTree( pDataDef->instances.Tree, (PTRSZVAL)instancename ) ) )
					{
						AddBinaryNode( pDataDef->instances.Tree
										 , data
										 , (PTRSZVAL)instancename );
					}
					else
					{
						lprintf( WIDE("Suck. We just created one externally, and want to use that data, but it already exists.") );
                  DumpRegisteredNames();
                  DebugBreak();
						// increment instances referenced so that close does not
						// destroy - fortunatly this is persistant data, and therefore
						// doesn't get destroyed yet.
					}
					return (PTRSZVAL)data;
				}
			}
		}
		else
		{
			lprintf( WIDE("No such struct defined: %s"), name );
		}
	}
	return 0;
}

//---------------------------------------------------------------------------

PROCREG_PROC( PTRSZVAL, CreateRegisteredDataTypeEx)( PCLASSROOT root
																	, CTEXTSTR classname
																	, CTEXTSTR name
																	, CTEXTSTR instancename )
{
	PTREEDEF class_root = GetClassTree( root, (PCLASSROOT)classname );
	if( class_root )
	{
      TEXTCHAR buf[256];
		PNAME pName = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)DressName( buf, name ));
		if( pName && pName->flags.bData )
		{
			PDATADEF pDataDef = &pName->data.data;
			if( pDataDef )
			{
				if( !instancename )
				{
					TEXTCHAR buf[256];
					snprintf( buf, sizeof(buf), WIDE("%s_%d"), name, (int)pDataDef->unique++ );
					instancename = SaveName( buf );
				}
				else
					instancename = SaveName( instancename );
				{
					POINTER p;
               // look up prior instance...
					if( !( p = FindInBinaryTree( pDataDef->instances.Tree, (PTRSZVAL)instancename ) ) )
					{
#ifdef DEBUG_GLOBAL_REGISTRATION
						lprintf( WIDE( "Allocating new struct data :%" )_32f, pDataDef->size );
#endif
						p = Allocate( pDataDef->size );
						MemSet( p, 0, pDataDef->size );
						if( pDataDef->Open )
							pDataDef->Open( p, pDataDef->size );
						AddBinaryNode( pDataDef->instances.Tree
										 , p
										 , (PTRSZVAL)instancename );
					}
#ifdef DEBUG_GLOBAL_REGISTRATION
					else
					{
                  lprintf( WIDE("Resulting with previuosly created instance.") );
					// increment instances referenced so that close does not
					// destroy - fortunatly this is persistant data, and therefore
					// doesn't get destroyed yet.
					}
#endif
					return (PTRSZVAL)p;
				}
			}
		}
#ifdef DEBUG_GLOBAL_REGISTRATION
		else
		{
			lprintf( WIDE("No such struct defined:[%s]%s"), classname, name );
		}
#endif
	}
	return 0;
}

//---------------------------------------------------------------------------

PROCREG_PROC( PTRSZVAL, CreateRegisteredDataType)( CTEXTSTR classname
																 , CTEXTSTR name
																 , CTEXTSTR instancename )
{
//#ifdef __STATIC__
//	return CreateRegisteredDataTypeEx( NULL, classname, name, instancename );
//#else
	SAFE_INIT();
	return CreateRegisteredDataTypeEx( l.Names, classname, name, instancename );
//#endif
}

//---------------------------------------------------------------------------

typedef POINTER (CPROC *LOADPROC)( void );
typedef void	 (CPROC *UNLOADPROC)( POINTER );

//-----------------------------------------------------------------------

LOGICAL RegisterInterface( CTEXTSTR servicename, POINTER(CPROC*load)(void), void(CPROC*unload)(POINTER))
{
	//PARAM( args, TEXTCHAR*, servicename );
	//PARAM( args, TEXTCHAR*, library );
	//PARAM( args, TEXTCHAR*, load_proc_name );
	//PARAM( args, TEXTCHAR*, unload_proc_name );
	PCLASSROOT pcr = GetClassRoot( WIDE("system/interfaces") );
	if( GetRegisteredProcedureExx( pcr, (PCLASSROOT)servicename, WIDE("POINTER"), WIDE("load"), WIDE("void") ) )
	{
		lprintf( WIDE("Service: %s has multiple definitions, will use last first.")
				 , servicename );
		return FALSE;
	}
	//lprintf( WIDE("Registering library l:%p ul:%p"), load, unload );
	{
		RegisterFunctionExx( pcr
								  , (PCLASSROOT)servicename
								  , WIDE("load")
								  , WIDE("POINTER")
								  , (PROCEDURE)load
								  , WIDE("(void)"), NULL, NULL DBG_SRC );
		RegisterFunctionExx( pcr
								  , (PCLASSROOT)servicename
								  , WIDE("unload")
								  , WIDE("void")
								  , (PROCEDURE)unload
								  , WIDE("(POINTER)"), NULL, NULL DBG_SRC );
	}
	return TRUE;
}

//-----------------------------------------------------------------------

static PTRSZVAL CPROC HandleLibrary( PTRSZVAL psv, arg_list args )
{
	PARAM( args, TEXTCHAR*, servicename );
	PARAM( args, TEXTCHAR*, library );
	PARAM( args, TEXTCHAR*, load_proc_name );
	PARAM( args, TEXTCHAR*, unload_proc_name );
	PCLASSROOT pcr = GetClassRoot( WIDE("system/interfaces") );
	if( GetRegisteredProcedureExx( pcr, (PCLASSROOT)servicename, WIDE("POINTER"), WIDE("load"), WIDE("void") ) )
	{
		lprintf( WIDE("Service: %s has multiple definitions, will use last first.")
				 , servicename );
      return psv;
	}
   //lprintf( WIDE("Registering library %s function %s"), library, load_proc_name );
	{
		RegisterProcedureExx( pcr
								  , servicename
								  , WIDE("load")
								  , WIDE("POINTER")
                          , library
								  , load_proc_name
								  , WIDE("void") DBG_SRC );
		RegisterProcedureExx( pcr
								  , servicename
								  , WIDE("unload")
								  , WIDE("void")
                          , library
								  , unload_proc_name, WIDE("POINTER") DBG_SRC );
	}
	return psv;
}

//-----------------------------------------------------------------------
static PTRSZVAL CPROC HandleAlias( PTRSZVAL psv, arg_list args )
{
	PARAM( args, TEXTCHAR*, servicename );
	PARAM( args, TEXTCHAR*, originalname );
	TEXTCHAR fullservicename[256];
	TEXTCHAR fulloriginalname[256];
    snprintf( fullservicename, sizeof( fullservicename), WIDE("system/interfaces/%s"), servicename );
	snprintf( fulloriginalname, sizeof( fulloriginalname), WIDE("system/interfaces/%s"), originalname );
	RegisterClassAlias( fulloriginalname, fullservicename );
	return psv;
}

//-----------------------------------------------------------------------

static PTRSZVAL CPROC HandleModule( PTRSZVAL psv, arg_list args )
{
	PARAM( args, TEXTCHAR*, module );
	LoadPrivateFunction( module, NULL );
	return psv;
}	

//-----------------------------------------------------------------------

static PTRSZVAL CPROC HandleModulePath( PTRSZVAL psv, arg_list args )
{
	PARAM( args, TEXTSTR, filepath );
#ifdef _WIN32
	{
		TEXTSTR e1, e2;
#ifndef UNDER_CE
		// no environment
		TEXTSTR oldpath, newpath;
#endif
		e1 = strrchr( filepath, '\\' );
		e2 = strrchr( filepath, '/' );
		if( e1 && e2 && ( e1 > e2 ) )
			e1[0] = 0;
		else if( e1 && e2 )
         e2[0] = 0;
		else if( e1 )
         e1[0] = 0;
		else if( e2 )
         e2[0] = 0;

# ifndef UNDER_CE
		oldpath = (TEXTCHAR*)getenv( WIDE("PATH") );
		if( !StrStr( oldpath, filepath ) )
		{
			int len;
			newpath = (TEXTCHAR*)Allocate( len=(_32)(StrLen( oldpath ) + 1 + StrLen(filepath)) );
			snprintf( newpath, sizeof(TEXTCHAR)*len, WIDE("%s;%s"), filepath, oldpath );
#   ifndef UNDER_CE
#     ifdef WIN32
			SetEnvironmentVariable( WIDE("PATH"), newpath );
#     else
			setenv( WIDE("PATH"), newpath );
#     endif
#   endif
			lprintf( WIDE("Updated path to %s from %s"), newpath, oldpath );
			Release( newpath );
		}
# endif
	}
#elif defined( __LINUX__ )
	{
      TEXTCHAR *e1, *e2, *oldpath, *newpath;
		e1 = strrchr( filepath, '\\' );
		e2 = strrchr( filepath, '/' );
		if( e1 && e2 && ( e1 > e2 ) )
         e1[0] = 0;
		else if( e1 && e2 )
         e2[0] = 0;
		else if( e1 )
         e1[0] = 0;
		else if( e2 )
         e2[0] = 0;

		oldpath = getenv( WIDE("LD_LIBRARY_PATH") );
		if( !strstr( oldpath, filepath ) )
		{
			newpath = (TEXTCHAR*)Allocate( StrLen( oldpath ) + 1 + StrLen(filepath) );
			sprintf( newpath, WIDE("%s;%s"), filepath, oldpath );
			setenv( WIDE("LD_LIBRARY_PATH"), newpath, 1 );
			lprintf( WIDE("Updated library path to %s from %s"), newpath, oldpath );
			Release( newpath );
		}
	}

#endif
   return psv;
}


PROCREG_PROC( void, SetInterfaceConfigFile )( TEXTCHAR *filename )
{
	if( l.config_filename )
      Release( l.config_filename );
   l.config_filename = StrDup( filename );
}
//-----------------------------------------------------------------------

static void ReadConfiguration( void )
{
	if( !l.flags.bInterfacesLoaded )
	{
		PCONFIG_HANDLER pch;
		pch = CreateConfigurationHandler();
		AddConfigurationMethod( pch, WIDE("service=%w library=%w load=%w unload=%w"), HandleLibrary );
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __CYGWIN__ )
		AddConfigurationMethod( pch, WIDE("win32 service=%w library=%w load=%w unload=%w"), HandleLibrary );
#elif defined( __LINUX__ )
		AddConfigurationMethod( pch, WIDE("linux service=%w library=%w load=%w unload=%w"), HandleLibrary );
#endif
      AddConfigurationMethod( pch, WIDE("alias service %w %w"), HandleAlias );
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __CYGWIN__ )
      AddConfigurationMethod( pch, WIDE("win32 alias service %w %w"), HandleAlias );
#elif defined( __LINUX__ )
		AddConfigurationMethod( pch, WIDE("linux alias service %w %w"), HandleAlias );
#endif
      AddConfigurationMethod( pch, WIDE("module %w"), HandleModule );
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __CYGWIN__ )
      AddConfigurationMethod( pch, WIDE("win32 module %w"), HandleModule );
#elif defined( __LINUX__ )
		AddConfigurationMethod( pch, WIDE("linux module %w"), HandleModule );
#endif

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __CYGWIN__ )
      AddConfigurationMethod( pch, WIDE("win32 module path %w"), HandleModulePath );
#elif defined( __LINUX__ )
		AddConfigurationMethod( pch, WIDE("linux module path %w"), HandleModulePath );
#endif
		AddConfigurationMethod( pch, WIDE("module path %m"), HandleModulePath );

		{
#ifdef HAVE_ENVIRONMENT
			CTEXTSTR filepath = getenv( WIDE("MY_LOAD_PATH") );
#else
			CTEXTSTR filepath = NULL;
#endif
			TEXTSTR loadname;
			int success = FALSE;
			if( !filepath )
				filepath = WIDE(".");
			if( l.config_filename )
			{
				success = ProcessConfigurationFile( pch, l.config_filename, 0 );
				if( !success )
				{
					if( filepath )
					{
						int len;
						loadname = NewArray( TEXTCHAR, len = (_32)StrLen( filepath ) + (_32)StrLen( l.config_filename ) + 2 );
#ifdef __STATIC__
#define STATIC WIDE(".static")
#else
#define STATIC
#endif
						snprintf( loadname, sizeof(TEXTCHAR)*len, WIDE("%s/%s"), filepath, l.config_filename?l.config_filename
									:WIDE("interface.conf") STATIC );
					}
					else
					{
						lprintf( WIDE("MY_LOAD_PATH wasn't set...") );
						loadname = StrDup( WIDE("interface.conf") STATIC );
					}
				}
				else
               loadname = NULL;
 			}
			else 
			{
				if( filepath )
				{
					size_t len;
					loadname = NewArray( TEXTCHAR, (_32)(len = StrLen( filepath ) + StrLen( WIDE("interface.conf") STATIC ) + 2) );
					snprintf( loadname, sizeof(TEXTCHAR)*len, WIDE("%s/%s"), filepath, WIDE("interface.conf") STATIC );
				}
				else
				{
               lprintf( WIDE("MY_LOAD_PATH wasn't set...") );
					loadname = StrDup( WIDE("interface.conf") STATIC );
				}
				//lprintf( WIDE("TO process %s"), loadname );
			}
			if( loadname )
			{
				success = ProcessConfigurationFile( pch, loadname, 0 );
				if( !success )
				{
					success = ProcessConfigurationFile( pch, "interface.conf", 0 );
					if( !success )
					{
						lprintf( WIDE("Failed to open interface configuration file:%s - assuming it will never exist, and aborting trying this again")
								 , l.config_filename?l.config_filename:WIDE("interface.conf") STATIC);
					}
				}
            Release( loadname );
			}
		}
		DestroyConfigurationHandler( pch );
		//at this point... we should probably NOT
      // dump this information, a vast amount of information may occur.
      // consider impelmenting enumerators and allowing browsing
		//DumpRegisteredNames();
		// if we failed, probably noone will notice, and nooone will
		// get the clue that we need to have an interface.conf
		// for this to preload extra libraries that the program may be
      // requesting.
      l.flags.bInterfacesLoaded = 1;
	}
}

//-----------------------------------------------------------------------

POINTER GetInterface( CTEXTSTR pServiceName )
{
	TEXTCHAR interface_name[256];
	POINTER (CPROC *load)( void );
	// this might be the first clean chance to run deadstarts
   // for ill behaved platforms that have forgotten to do this.
	if( !IsRootDeadstartStarted() )
		InvokeDeadstart();
	ReadConfiguration();
	if( pServiceName )
	{
		snprintf( interface_name, sizeof( interface_name ), WIDE("system/interfaces/%s"), pServiceName );
		load = GetRegisteredProcedure( (PCLASSROOT)interface_name, POINTER, load, (void) );
		//lprintf( WIDE("GetInterface for %s is %p"), pServiceName, load );
		if( load )
		{
			POINTER p = load();
			//lprintf( WIDE("And the laod proc resulted %p"), p );
			return p; //load();
		}
		else
		{
			if( !GetRegisteredValueExx( (PCLASSROOT)interface_name, NULL, WIDE( "Logged" ), 1 ) )
			{
				lprintf( WIDE("Did not find load procedure for:[%s] (dumping names from /system/interface/* so you can see what might be availalbe)"), interface_name );
				DumpRegisteredNamesFrom(GetClassRoot(WIDE( "system/interfaces" )));
				RegisterValueExx( (PCLASSROOT)interface_name, NULL, WIDE( "Logged" ), 1, (CTEXTSTR)1 );
			}
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------

PROCREG_PROC( void, DropInterface )( TEXTCHAR *pServiceName, POINTER interface_drop )
{
	TEXTCHAR interfacename[256];
	void (CPROC *unload)( POINTER );
	ReadConfiguration();
	snprintf( interfacename, sizeof(interfacename), WIDE("system/interfaces/%s"), pServiceName );
	unload = GetRegisteredProcedure( (PCLASSROOT)interfacename, void, unload, (POINTER) );
	if( unload )
		unload( interface_drop );
}


//-----------------------------------------------------------------------

static void DoDeleteRegistry( void )
{
   DoDeleteRegistry( );
}

//-----------------------------------------------------------------------

ATEXIT( DeleteRegistry )
{
   //DoDeleteRegistry( l.root );
}

void RegisterAndCreateGlobalWithInit( POINTER *ppGlobal, PTRSZVAL global_size, CTEXTSTR name, void (CPROC*Open)(POINTER,PTRSZVAL) )
{
	POINTER *ppGlobalMain;
	POINTER p;

	if( ppGlobal == (POINTER*)&procreg_local_data )
	{
		PTRSZVAL size = global_size;
		_32 created;
		TEXTCHAR spacename[32];
		if( procreg_local_data != NULL )
		{
			// if local already has something, just return.
			return;
		}
#ifdef DEBUG_GLOBAL_REGISTRATION
		lprintf( WIDE("Opening space...") );
#endif
#ifdef WIN32
		snprintf( spacename, sizeof( spacename ), WIDE("%s:%08LX"), name, (GetMyThreadID()) >> 32 );
#else
		snprintf( spacename, sizeof( spacename ), WIDE("%s:%08lX"), name, getpid() );
#endif
		// hmm application only shared space?
		// how do I get that to happen?
		(*ppGlobal) = OpenSpaceExx( spacename, NULL, 0, &size, &created );
		// I myself must have a global space, which is kept sepearte from named spaces
		// but then... blah
		if( created )
		{
#ifdef DEBUG_GLOBAL_REGISTRATION
			lprintf( WIDE("(specific procreg global)clearing memory:%s(%p)"), spacename, (*ppGlobal ) );
#endif
			MemSet( (*ppGlobal), 0, global_size );
			if( Open )
				Open( (*ppGlobal), global_size );
			p = (POINTER)MakeRegisteredDataTypeEx( NULL, WIDE("system/global data"), name, name, (*ppGlobal), global_size );
		}
#ifdef DEBUG_GLOBAL_REGISTRATION
		else
		{
			lprintf( WIDE("(specific procreg global)using memory untouched:%s(%p)"), spacename, (*ppGlobal ) );
		}
#endif
		// result is the same as the pointer input...
		return;
	}

	if( ppGlobal && !(*ppGlobal) )
	{
		// RTLD_DEFAULT
		ppGlobalMain = &p;
		p = (POINTER)CreateRegisteredDataType( WIDE("system/global data"), name, name );
		if( !p )
		{
			RegisterDataType( WIDE("system/global data"), name, global_size
								 , NULL
								 , NULL );
			p = (POINTER)CreateRegisteredDataType( WIDE("system/global data"), name, name );
			if( !p )
				ppGlobalMain = NULL;
#ifdef DEBUG_GLOBAL_REGISTRATION
			lprintf( WIDE("Recovered global by registered type. %p"), p );
#endif
		}
#ifdef DEBUG_GLOBAL_REGISTRATION
		else
		{
         lprintf( WIDE("Found our shared region by asking politely for it! *********************") );
		}
#endif
		if( !ppGlobalMain )
		{
			lprintf( WIDE("None found in main... no way to mark for a peer...") );
			exit(0);
		}
	}
	else
	{
      // thing is already apparently initizliaed.. don't do this.
		ppGlobalMain = NULL;
	}
	if(ppGlobalMain )
	{
		if( ppGlobalMain == ppGlobal )
		{
			lprintf( WIDE("This is the global space we need...") );
			//MemSet( (*ppGlobal) = Allocate( global_size )
			//						  , 0 , global_size );
			(*ppGlobal) = (POINTER)CreateRegisteredDataType( WIDE("system/global data"), name, name );
			if( !(*ppGlobal) )
			{
				RegisterDataType( WIDE("system/global data"), name, global_size
									 , NULL
									 , NULL );
				(*ppGlobal) = (POINTER)CreateRegisteredDataType( WIDE("system/global data"), name, name );
			}
		}
		else if( *ppGlobalMain )
		{
#ifdef DEBUG_GLOBAL_REGISTRATION
			lprintf( WIDE("Resulting with a global space to use... %p"), (*ppGlobalMain) );
#endif
			(*ppGlobal) = (*ppGlobalMain);
		}
		else
		{
			lprintf( WIDE("Failure to get global_procreg_data block.") );
			exit(0);
		}
	}
}

void RegisterAndCreateGlobal( POINTER *ppGlobal, PTRSZVAL global_size, CTEXTSTR name )
{
   RegisterAndCreateGlobalWithInit( ppGlobal, global_size, name, NULL );
}

#ifdef __cplusplus_cli

#include <vcclr.h>

using namespace System;

public ref class ProcReg
{
	static ProcReg()
	{
		InvokeDeadstart();
	}
	
	int Register( System::String^ name_class, String^ proc, STDPROCEDURE Delegate )
	{
		if( name_class )
		{
			PNAME newname = GetFromSet( NAME, &l.NameSet );//Allocate( sizeof( NAME ) );
			TEXTCHAR strippedargs[256];
			pin_ptr<const WCHAR> tmp2 = PtrToStringChars(name_class);
			CTEXTSTR __name_class = sack::containers::text::WcharConvert( tmp2 );
			pin_ptr<const WCHAR> tmp = PtrToStringChars(proc);
			CTEXTSTR real_name = sack::containers::text::WcharConvert( tmp );
	
			PTREEDEF class_root = (PTREEDEF)GetClassTree( NULL, (PTREEDEF)__name_class );
			MemSet( newname, 0, sizeof( NAME ) );
			newname->flags.bStdProc = 1;
			// this is kinda messed up...
			newname->name = SaveName( real_name );
			//newname->data.stdproc.library = SaveName( library );
	
	
			newname->data.stdproc.procname = SaveName( real_name );
			//newname->data.proc.ret = SaveName( returntype );
			//newname->data.proc.args = SaveName( StripName( strippedargs, args ) );
			newname->data.proc.name = SaveNameConcatN( StripName( strippedargs, "(*)" )
																  , WIDE("") //returntype
																  , WIDE("") // library 
																  , real_name
																  , NULL
																  );
			newname->data.stdproc.proc = Delegate;
			if( class_root )
			{
				PNAME oldname;
				oldname = (PNAME)FindInBinaryTree( class_root->Tree, (PTRSZVAL)newname->name);
				if( oldname )
				{
					if( oldname->data.stdproc.proc == Delegate )
						Log( WIDE("And fortunatly it's the same address... all is well...") );
					else
					{
						xlprintf( 2 )( WIDE("proc %s/%s regisry by %s of %s(%s) conflicts with %s(%s)...")
													  , (CTEXTSTR)__name_class?(CTEXTSTR)__name_class:WIDE("@")
													  , real_name
													  , newname->name
													  , newname->data.proc.name
														//,library
													  , newname->data.proc.procname
													  , oldname->data.proc.name
													  //,library
													  , oldname->data.proc.procname );
						// perhaps it's same in a different library...
						Log( WIDE("All is not well - found same funciton name in tree with different address. (ignoring second) ") );
						//DebugBreak();
						//DumpRegisteredNames();
					}
					return TRUE;
				}
				else
				{
					if( !AddBinaryNode( class_root->Tree, newname, (PTRSZVAL)newname->name ) )
					{
						Log( WIDE("For some reason could not add new name to tree!") );
						DeleteFromSet( NAME, &l.NameSet, newname );
					}
				}
			}
			else
			{
				lprintf( WIDE("I'm relasing this name!?") );
				DeleteFromSet( NAME, &l.NameSet, newname );
			}
			return 1;
		}
	}
};

#endif

PROCREG_NAMESPACE_END

//---------------------------------------------------------------------------
//
// $Log: names.c,v $
// Revision 1.71  2005/07/06 00:03:06  jim
// Typecasts to make getenv macro happy.  Implemented in OSALOT since watcom's getenv implementation SUCKS.
//
// Revision 1.70  2005/07/05 19:25:22  jim
// Blah defined the symbols wrong!
//
// Revision 1.69  2005/07/05 19:19:31  jim
// Restore extended method, required for controls to validate their methods
//
// Revision 1.68  2005/07/05 18:19:56  jim
// Added registration of MY_LOAD_PATH to load the interface.conf from - instead of relying on current working directory
//
// Revision 1.56  2005/06/30 13:18:56  d3x0r
// Improve loading of configuration file... if current working path fails, attempt to load from the original program path - requires chances to system lib to register the loaded path.
//
// Revision 1.55  2005/06/01 17:15:21  d3x0r
// remove leftover debug logging...
//
// Revision 1.54  2005/06/01 17:11:11  d3x0r
// Fix static buffer varible when getting class root.
//
// Revision 1.53  2005/05/30 11:56:35  d3x0r
// various fixes... working on psilib update optimization... various stabilitizations... also extending msgsvr functionality.
//
// Revision 1.52  2005/05/25 16:50:17  d3x0r
// Synch with working repository.
//
// Revision 1.64  2005/04/19 22:49:30  jim
// Looks like the display module technology nearly works... at least exits graceful are handled somewhat gracefully.
//
// Revision 1.63  2005/04/01 03:23:05  panther
// OOps typo
//
// Revision 1.62  2005/03/31 00:27:11  panther
// Define RegsiterInterface
//
// Revision 1.61  2005/03/15 18:42:52  panther
// Add support for application to specify the config file to use.
//
// Revision 1.60  2005/03/15 17:21:50  panther
// FIx for linux setenv
//
// Revision 1.59  2005/03/14 16:30:30  panther
// Add the ability to GetInterface(NULL); to cause proc_reg library to read its config.
//
// Revision 1.58  2005/02/09 21:22:18  panther
// Add ability to add paths to environment to speicfy where modules come from...
//
// Revision 1.57  2005/01/23 13:33:10  panther
// Fix lossing of conflicting address to display meaningful information
//
// Revision 1.56  2005/01/16 23:44:49  panther
// Minor spelling
//
// Revision 1.55  2004/12/20 19:41:00  panther
// Minor reformats and function protection changes (static)
//
// Revision 1.54  2004/12/19 15:45:06  panther
// Checkpoint
//
// Revision 1.53  2004/12/05 15:32:06  panther
// Some minor cleanups fixed a couple memory leaks
//
// Revision 1.52  2004/11/29 12:22:50  panther
// Modify default interface config.
//
// Revision 1.51  2004/10/05 00:59:01  d3x0r
// Checkpoint...
//
// Revision 1.50  2004/10/04 20:08:39  d3x0r
// Minor adjustments for static linking
//
// Revision 1.49  2004/10/03 01:10:03  d3x0r
// Remove dump names after load config
//
// Revision 1.48  2004/09/20 20:39:53  d3x0r
// Fix function registarrion defs
//
// Revision 1.47  2004/09/13 09:12:39  d3x0r
// Simplify procregsitration, standardize registration, cleanups...
//
//
