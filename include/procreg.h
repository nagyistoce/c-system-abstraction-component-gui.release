/*
 * Create: James Buckeyne
 *
 * Purpose: Provide a general structure to register names of
 *   routines and data structures which may be consulted
 *   for runtime linking.  Aliases and other features make this
 *   a useful library for tracking interface registration...
 *
 *  The namespace may be enumerated.
 */



#ifndef PROCEDURE_REGISTRY_LIBRARY_DEFINED
#define PROCEDURE_REGISTRY_LIBRARY_DEFINED
#include <sack_types.h>
#include <deadstart.h>
#ifdef BCC16
#ifdef PROCREG_SOURCE
#define PROCREG_PROC(type,name) type STDPROC _export name
#else
#define PROCREG_PROC(type,name) type STDPROC name
#endif
#else
#ifdef PROCREG_SOURCE
#define PROCREG_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define PROCREG_PROC(type,name) IMPORT_METHOD type CPROC name
#endif
#endif



#ifdef __cplusplus
#ifdef __cplusplus_cli
#include <vcclr.h>
using namespace System;
#endif
#define PROCREG_NAMESPACE namespace sack { namespace system { namespace registry {
#define PROCREG_NAMESPACE_END }}}
//extern "C"  {
#else
#define PROCREG_NAMESPACE 
#define PROCREG_NAMESPACE_END
#endif

PROCREG_NAMESPACE
// POINTER in these two are equal to (void(*)(void))
// but - that's rarely the most useful thing... so


// name class is a tree of keys...
//   <libname>/<...>
//
//   psi/control/## might contain procs
//       Init
//       Destroy
//       Move
//
//   RegAlias( WIDE("psi/control/3"), WIDE("psi/control/button") );
//     psi/control/button and
//     psi/control/3   might reference the same routines
//
//   psi/frame
//       Init
//       Destroy
//       Move
//  memlib
//     Alloc
//     Free
//
//   network/tcp
//
//
// I guess name class trees are somewhat shallow at the moment
//  not going beyond 1-3 layers
//
//  names may eventually be registered and
//  reference out of body services, even out of box...
//
//

// the values passed as returntype and parms/args need not be
// real genuine types, but do need to be consistant between the
// registrant and the requestor... this provides for full name
// dressing, return type and paramter type may both cause
// overridden functions to occur...


#ifndef REGISTRY_STRUCTURE_DEFINED
	// make these a CTEXTSTR to be compatible with name_class...
#ifdef __cplusplus
	// because of name mangling and stronger type casting
	// it becomes difficult to pass a tree_def_tag * as a CTEXTSTR classname
	// as valid as this is.
	typedef struct tree_def_tag *PCLASSROOT;
#else
	typedef CTEXTSTR PCLASSROOT;
#endif

	typedef void (CPROC *PROCEDURE)(void);
#ifdef __cplusplus_cli
	typedef void (__stdcall *STDPROCEDURE)(array<Object^>^);
#endif
#else
   typedef struct tree_def_tag *PCLASSROOT;
	typedef void (CPROC *PROCEDURE)(void);
#ifdef __cplusplus_cli
	typedef void (__stdcall *STDPROCEDURE)(array<Object^>^);
#endif
#endif

///
///what is this
///Triple Comment...
	//void GetClassRoot( CTEXTSTR class_name );


// Process name registry
	// document
	// it's a tree of names.
	// there are paths, and entries
	// paths are represented as class_name
	// PCLASSROOT is also a suitable class name
	// PCLASSROOT is defined as a valid CTEXTSTR.
	// there is (apparently) a name that is not valid as a path namne
	// that is TREE


PROCREG_PROC( PCLASSROOT, CheckClassRoot )( CTEXTSTR class_name );
PROCREG_PROC( PCLASSROOT, GetClassRoot )( CTEXTSTR class_name );
PROCREG_PROC( PCLASSROOT, GetClassRootEx )( PCLASSROOT root, CTEXTSTR name_class );
#ifdef __cplusplus
PROCREG_PROC( PCLASSROOT, GetClassRoot )( PCLASSROOT class_name );
PROCREG_PROC( PCLASSROOT, GetClassRootEx )( PCLASSROOT root, PCLASSROOT name_class );
#endif

PROCREG_PROC( void, SetInterfaceConfigFile )( char *filename );


// data is a &(POINTER data = NULL);
// POINTER data = NULL; .. Get[First/Next]RegisteredName( WIDE("classname"), &data );
// these operations are not threadsafe and multiple thread accesses will cause mis-stepping
//
// Usage: 
/*  CTEXTSTR result;
 *  POINTER data = NULL;
 *  for( result = GetFirstRegisteredName( "some/class/path", &data );
 *       result;
 *       result = GetNextRegisteredName( &data ) )
 *  {
 *     if( NameHasBranches( &data ) )  // for consitancy in syntax
 *     {
 *        // consider recursing through tree, name becomes a valid classname for GetFirstRegisteredName()
 *     } 
 */
// these functions as passed the address of a POINTER.
// this POINTER is for the use of the browse routines and should 
// is meaningless to he calling application.
// 
PROCREG_PROC( CTEXTSTR, GetFirstRegisteredNameEx )( PCLASSROOT root, CTEXTSTR classname, PCLASSROOT *data );
PROCREG_PROC( CTEXTSTR, GetFirstRegisteredName )( CTEXTSTR classname, PCLASSROOT *data );
PROCREG_PROC( CTEXTSTR, GetNextRegisteredName )( PCLASSROOT *data );
// result with the current node ( useful for pulling registered subvalues like description....
PROCREG_PROC( PCLASSROOT, GetCurrentRegisteredTree )( PCLASSROOT *data );
#ifdef __cplusplus
//PROCREG_PROC( CTEXTSTR, GetFirstRegisteredName )( CTEXTSTR classname, POINTER *data );
//PROCREG_PROC( CTEXTSTR, GetNextRegisteredName )( POINTER *data );
#endif
// while doing a scan for registered procedures, allow applications to check for branches
//PROCREG_PROC( int, NameHasBranches )( POINTER *data );
PROCREG_PROC( int, NameHasBranches )( PCLASSROOT *data );
// while doing a scan for registered procedures, allow applications to ignore aliases...
//PROCREG_PROC( int, NameIsAlias )( POINTER *data );
PROCREG_PROC( int, NameIsAlias )( PCLASSROOT *data );

/*
 * RegisterProcedureExx( 
 *
 */
PROCREG_PROC( int, RegisterProcedureExx )( PCLASSROOT root // root name or PCLASSROOT of base path
													  , CTEXTSTR name_class // an additional path on root
													  , CTEXTSTR public_name // the name of the value entry saved in the tree
													  , CTEXTSTR returntype // the text return type of this function - may be checked to validate during GetRegisteredProcedure
													  , CTEXTSTR library // name of the library this symbol is in - may be checked to validate during GetRegisteredProcedure
													  , CTEXTSTR name // actual C function name in library - may be checked to validate during GetRegisteredProcedure
													  , CTEXTSTR args // preferably the raw argument string of types and no variable references "([type][,type]...)"
													  DBG_PASS // file and line of the calling application.  May be no parameter in release mode.
													  );


/*
 * RegisterProcedureEx( root       // root path
 *                    , name_class // additional name
 *                    , nice_name  // nice name
 *                    , return type not in quotes  'void'
 *                    , function_name in quotes '"Function"'
 *                    , args not in quotes '(int,char,float,UserType*)'
 */
#define RegisterProcedureEx(root,nc,n,rtype,proc,args)  RegisterProcedureExx( (root),(nc),(n),#rtype,TARGETNAME,(proc), #args DBG_SRC)
/*
 * RegisterProcedure( name_class // additional name
 *                    , nice_name  // nice name
 *                    , return type not in quotes  'void'
 *                    , function_name in quotes '"Function"'
 *                    , args not in quotes '(int,char,float,UserType*)'
 */
#define RegisterProcedure(nc,n,rtype,proc,args)  RegisterProcedureExx( NULL, (nc),(n),#rtype,TARGETNAME,(proc), #args DBG_SRC)


/* 
 * Branches on the tree may be aliased together to form a single branch
 * 
 */
				// RegisterClassAlias( WIDE("psi/control/button"), WIDE("psi/control/3") );
				// then the same set of values can be referenced both ways with
				// really only a single modified value.
/* parameters to RegisterClassAliasEx are the original name, and the new alias name for the origianl branch*/
PROCREG_PROC( PCLASSROOT, RegisterClassAliasEx )( PCLASSROOT root, CTEXTSTR original, CTEXTSTR alias );
PROCREG_PROC( PCLASSROOT, RegisterClassAlias )( CTEXTSTR original, CTEXTSTR newalias );



// root, return, public, args, address

PROCREG_PROC( PROCEDURE, ReadRegisteredProcedureEx )( PCLASSROOT root
                                                    , CTEXTSTR returntype
																	 , CTEXTSTR parms
																  );
#define ReadRegisteredProcedure( root,rt,a) ((rt(CPROC*)a)ReadRegisteredProcedureEx(root,#rt,#a))
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( PCLASSROOT root
																	 , PCLASSROOT name_class
                                                    , CTEXTSTR returntype
																	 , CTEXTSTR name
																	 , CTEXTSTR parms
																	 );
#define GetRegisteredProcedureExx(root,nc,rt,n,a) ((rt (CPROC*)a)GetRegisteredProcedureExxx(root,nc,_WIDE(#rt),n,_WIDE(#a)))
#define GetRegisteredProcedure2(nc,rtype,name,args) (rtype (CPROC*)args)GetRegisteredProcedureEx((nc),#rtype, name, #args )
#define GetRegisteredProcedureNonCPROC(nc,rtype,name,args) (rtype (*)args)GetRegisteredProcedureEx((nc),#rtype, name, #args )

PROCREG_PROC( PROCEDURE, GetRegisteredProcedureEx )( PCLASSROOT name_class
																	, CTEXTSTR returntype
																	, CTEXTSTR name
																	, CTEXTSTR parms 
																	);

PROCREG_PROC( LOGICAL, RegisterFunctionExx )( PCLASSROOT root
													, PCLASSROOT name_class
													, CTEXTSTR public_name
													, CTEXTSTR returntype
													, PROCEDURE proc
													 , CTEXTSTR args
													 , CTEXTSTR library
													 , CTEXTSTR real_name
													  DBG_PASS
														  );
#ifdef __cplusplus
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( CTEXTSTR root
																	 , CTEXTSTR name_class
                                                    , CTEXTSTR returntype
																	 , CTEXTSTR name
																	 , CTEXTSTR parms
																	 );
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( CTEXTSTR root
																	 , PCLASSROOT name_class
                                                    , CTEXTSTR returntype
																	 , CTEXTSTR name
																	 , CTEXTSTR parms
																	 );
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureExxx )( PCLASSROOT root
																	 , CTEXTSTR name_class
                                                    , CTEXTSTR returntype
																	 , CTEXTSTR name
																	 , CTEXTSTR parms
																	 );
PROCREG_PROC( PROCEDURE, GetRegisteredProcedureEx )( CTEXTSTR name_class
																	, CTEXTSTR returntype
																	, CTEXTSTR name
																	, CTEXTSTR parms 
																	);
PROCREG_PROC( LOGICAL, RegisterFunctionExx )( CTEXTSTR root
													, CTEXTSTR name_class
													, CTEXTSTR public_name
													, CTEXTSTR returntype
                                       , PROCEDURE proc
													 , CTEXTSTR args
													 , CTEXTSTR library
													 , CTEXTSTR real_name
													  DBG_PASS
														  );
#endif
//#define RegisterFunctionExx( r,nc,p,rt,proc,ar ) RegisterFunctionExx( r,nc,p,rt,proc,ar,TARGETNAME,NULL DBG_SRC )
#define RegisterFunctionExx(r,nc,pn,rt,proc,args,lib,rn) RegisterFunctionExx(r,nc,pn,rt,proc,args,lib,rn DBG_SRC)
#define RegisterFunctionEx( root,proc,rt,pn,a) RegisterFunctionExx( root,NULL,pn,rt,(PROCEDURE)(proc),a,NULL,NULL )
#ifndef TARGETNAME 
#define TARGETNAME "sack"
#endif
#define RegisterFunction( nc,proc,rt,pn,a) RegisterFunctionExx( (PCLASSROOT)NULL,nc,pn,rt,(PROCEDURE)(proc),a,TARGETNAME,NULL )

#define SimpleRegisterMethod(r,proc,rt,name,args) RegisterFunctionExx(r,NULL,name,rt,(PROCEDURE)proc,args,NULL,NULL )

#define GetRegisteredProcedure(nc,rtype,name,args) (rtype (CPROC*)args)GetRegisteredProcedureEx((nc),_WIDE(#rtype), _WIDE(#name), _WIDE(#args) )
PROCREG_PROC( int, RegisterIntValueEx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name, PTRSZVAL value );
PROCREG_PROC( int, RegisterIntValue )( CTEXTSTR name_class, CTEXTSTR name, PTRSZVAL value );

PROCREG_PROC( int, RegisterValueExx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name, int bIntVal, CTEXTSTR value );
PROCREG_PROC( int, RegisterValueEx )( CTEXTSTR name_class, CTEXTSTR name, int bIntVal, CTEXTSTR value );
PROCREG_PROC( int, RegisterValue )( CTEXTSTR name_class, CTEXTSTR name, CTEXTSTR value );

PROCREG_PROC( CTEXTSTR, GetRegisteredValueExx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name, int bIntVal );
PROCREG_PROC( CTEXTSTR, GetRegisteredValueEx )( CTEXTSTR name_class, CTEXTSTR name, int bIntVal );
PROCREG_PROC( CTEXTSTR, GetRegisteredValue )( CTEXTSTR name_class, CTEXTSTR name );
#ifdef __cplusplus
PROCREG_PROC( CTEXTSTR, GetRegisteredValueExx )( CTEXTSTR root, CTEXTSTR name_class, CTEXTSTR name, int bIntVal );
PROCREG_PROC( int, RegisterIntValueEx )( CTEXTSTR root, CTEXTSTR name_class, CTEXTSTR name, PTRSZVAL value );
#endif

// if bIntVale, result should be passed as an (&int)
PROCREG_PROC( int, GetRegisteredStaticValue )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name
															, CTEXTSTR *result
															, int bIntVal );
#define GetRegisteredStaticIntValue(r,nc,name,result) GetRegisteredStaticValue(r,nc,name,(CTEXTSTR*)result,TRUE )

// these should be avoided...
// the above
PROCREG_PROC( int, GetRegisteredIntValueEx )( PCLASSROOT root, CTEXTSTR name_class, CTEXTSTR name );
PROCREG_PROC( int, GetRegisteredIntValue )( CTEXTSTR name_class, CTEXTSTR name );
#ifdef __cplusplus
PROCREG_PROC( int, GetRegisteredIntValue )( PCLASSROOT name_class, CTEXTSTR name );
#endif

typedef void (CPROC*OpenCloseNotification)( POINTER, PTRSZVAL );
#define PUBLIC_DATA( public, struct, open, close )    \
	PRELOAD( Data_##open##_##close ) { \
	RegisterDataType( WIDE("system/data/structs")  \
      	, public, sizeof(struct)    \
	, (OpenCloseNotification)open, (OpenCloseNotification)close ); }

#define PUBLIC_DATA_EX( public, struct, open, update, close )    \
	PRELOAD( Data_##open##_##close ) { \
	RegisterDataTypeEx( WIDE("system/data/structs")  \
      	, public, sizeof(struct)    \
	, (OpenCloseNotification)open, (OpenCloseNotification)update, (OpenCloseNotification)close ); }

#define GET_PUBLIC_DATA( public, type, instname ) \
   (type*)CreateRegisteredDataType( WIDE("system/data/structs"), public, instname )
PROCREG_PROC( PTRSZVAL, RegisterDataType )( CTEXTSTR classname
												 , CTEXTSTR name
												 , PTRSZVAL size
												 , OpenCloseNotification open
												 , OpenCloseNotification close );
PROCREG_PROC( PTRSZVAL, CreateRegisteredDataType)( CTEXTSTR classname
																 , CTEXTSTR name
																 , CTEXTSTR instancename );

PROCREG_PROC( PTRSZVAL, RegisterDataTypeEx )( PCLASSROOT root
													, CTEXTSTR classname
													, CTEXTSTR name
													, PTRSZVAL size
													, OpenCloseNotification Open
													, OpenCloseNotification Close );

PROCREG_PROC( PTRSZVAL, CreateRegisteredDataTypeEx)( PCLASSROOT root
																	, CTEXTSTR classname
																	, CTEXTSTR name
																	, CTEXTSTR instancename );

PROCREG_PROC( void, DumpRegisteredNames )( void );
PROCREG_PROC( void, DumpRegisteredNamesFrom )( PCLASSROOT root );

PROCREG_PROC( int, SaveTree )( void );
PROCREG_PROC( int, LoadTree )( void );

#define METHOD_PTR(type,name) type (CPROC *_##name)
#define DMETHOD_PTR(type,name) type (CPROC **_##name)

#define METHOD_ALIAS(i,name) ((i)->_##name)
#define PDMETHOD_ALIAS(i,name) (*(i)->_##name)

PROCREG_PROC( void, DropInterface )( TEXTCHAR *pServiceName, POINTER interface_x );
PROCREG_PROC( POINTER, GetInterface )( CTEXTSTR pServiceName );

#define GetRegisteredInterface(name) GetInterface(name)
PROCREG_PROC( LOGICAL, RegisterInterface )( TEXTCHAR *name, POINTER(CPROC*load)(void), void(CPROC*unload)(POINTER));

// unregister a function, should be smart and do full return type
// and parameters..... but for now this only references name, this indicates
// that this has not been properly(fully) extended, and should be layered
// in such a way as to allow this function work in it's minimal form.
PROCREG_PROC( int, ReleaseRegisteredFunctionEx )( PCLASSROOT root
													, CTEXTSTR name_class
													, CTEXTSTR public_name
													  );
#define ReleaseRegisteredFunction(nc,pn) ReleaseRegisteredFunctionEx(NULL,nc,pn)

#define paste(a,b) _WIDE(a##b)


#define ___DefineRegistryMethod2(task,name,classtype,methodname,desc,returntype,argtypes,line)   \
	CPROC paste(name,line)argtypes;       \
	PRELOAD( paste(Register##name##Button,line) ) {  \
	SimpleRegisterMethod( task WIDE("/") classtype, paste(name,line)  \
	, _WIDE(#returntype), methodname, _WIDE(#argtypes) ); \
   RegisterValue( task "/" classtype "/" methodname, "Description", desc ); \
}                                                                          \
	static returntype CPROC paste(name,line)

#define __DefineRegistryMethod2(task,name,classtype,methodname,desc,returntype,argtypes,line)   \
	___DefineRegistryMethod2(task,name,classtype,methodname,desc,returntype,argtypes,line)

#define ___DefineRegistryMethod(task,name,classtype,classbase,methodname,returntype,argtypes,line)   \
	CPROC paste(name,line)argtypes;       \
	PRELOAD( paste(Register##name##Button,line) ) {  \
	SimpleRegisterMethod( task WIDE("/") classtype WIDE("/") classbase, paste(name,line)  \
	, _WIDE(#returntype), methodname, _WIDE(#argtypes) ); \
}                                                                          \
	static returntype CPROC paste(name,line)

#define __DefineRegistryMethod(task,name,classtype,classbase,methodname,returntype,argtypes,line)   \
	___DefineRegistryMethod(task,name,classtype,classbase,methodname,returntype,argtypes,line)


#define _DefineRegistryMethod(task,name,classtype,classbase,methodname,returntype,argtypes,line)   \
	static returntype __DefineRegistryMethod(task,name,classtype,classbase,methodname,returntype,argtypes,line)

#define DefineRegistryMethod(task,name,classtype,classbase,methodname,returntype,argtypes)  \
	_DefineRegistryMethod(task,name,classtype,classbase,methodname,returntype,argtypes,__LINE__)

// this fails. Because this is used with complex names
// an extra alias of priority_preload must be used to fully resolve paramters.
#define PRIOR_PRELOAD(a,p) PRIORITY_PRELOAD(a,p)
#define ___DefineRegistryMethodP(priority,task,name,classtype,classbase,methodname,returntype,argtypes,line)   \
	CPROC paste(name,line)argtypes;       \
	PRIOR_PRELOAD( paste(Register##name##Button,line), priority ) {  \
	SimpleRegisterMethod( task WIDE("/") classtype WIDE("/") classbase, paste(name,line)  \
	, _WIDE(#returntype), methodname, _WIDE(#argtypes) ); \
}                                                                          \
	static returntype CPROC paste(name,line)

#define __DefineRegistryMethodP(priority,task,name,classtype,classbase,methodname,returntype,argtypes,line)   \
	___DefineRegistryMethodP(priority,task,name,classtype,classbase,methodname,returntype,argtypes,line)


#define _DefineRegistryMethodP(priority,task,name,classtype,classbase,methodname,returntype,argtypes,line)   \
	static returntype __DefineRegistryMethodP(priority,task,name,classtype,classbase,methodname,returntype,argtypes,line)

#define DefineRegistryMethodP(priority,task,name,classtype,classbase,methodname,returntype,argtypes)  \
	_DefineRegistryMethodP(priority,task,name,classtype,classbase,methodname,returntype,argtypes,__LINE__)

#define _DefineRegistrySubMethod(task,name,classtype,classbase,methodname,subname,returntype,argtypes,line)   \
	static returntype CPROC paste(name,line)argtypes;       \
	PRELOAD( paste(Register##name##Button,line) ) {  \
	SimpleRegisterMethod( task WIDE("/") classtype WIDE("/") classbase WIDE("/") methodname, paste(name,line)  \
	, _WIDE(#returntype), subname, _WIDE(#argtypes) ); \
}                                                                          \
	static returntype CPROC paste(name,line)

#define DefineRegistrySubMethod(task,name,classtype,classbase,methodname,subname,returntype,argtypes)  \
	_DefineRegistrySubMethod(task,name,classtype,classbase,methodname,subname,returntype,argtypes,__LINE__)

// attempts to use dynamic linking functions to resolve passed global name
				// if that fails, then a type is registered for this global, and an instance
				// created, so that that instance may be reloaded again, otherwise the
				// data in the main application is used... actually we should depreicate the dynamic
// loading part, and just register the type.
PROCREG_PROC( void, RegisterAndCreateGlobal )( POINTER *ppGlobal, PTRSZVAL global_size, CTEXTSTR name );
#define SimpleRegisterAndCreateGlobal( name ) 	RegisterAndCreateGlobal( (POINTER*)&name, sizeof( *name ), #name )
// Init routine is called, otherwise a 0 filled space is returned.
// Init routine is passed the pointer to the global and the size of the global block
// the global data block is zero initialized.
PROCREG_PROC( void, RegisterAndCreateGlobalWithInit )( POINTER *ppGlobal, PTRSZVAL global_size, CTEXTSTR name, void (CPROC*Init)(POINTER,PTRSZVAL) );
#define SimpleRegisterAndCreateGlobalWithInit( name,init ) 	RegisterAndCreateGlobalWithInit( (POINTER*)&name, sizeof( *name ), #name, init )

/* a tree dump will result with dictionary names that may translate automatically. */
/* This has been exported as a courtesy for StrDup.
 * this routine MAY result with a translated string.
 * this routine MAY result with the same pointer.
 * this routine MAY need to be improved if MANY more strdups are replaced
 * Add a binary tree search index when large.
 * Add a transaltion tree index at the same time.
 */
PROCREG_PROC( CTEXTSTR, SaveNameConcatN )( CTEXTSTR name1, ... );
// no space stripping.
PROCREG_PROC( CTEXTSTR, SaveText )( CTEXTSTR text );


PROCREG_NAMESPACE_END
#ifdef __cplusplus

//PROCREG_PROC( PCLASSROOT, GetClassRootEx )( CTEXTSTR root, CTEXTSTR name_class );
//PROCREG_PROC( PCLASSROOT, GetClassRootEx )( CTEXTSTR root, PCLASSROOT name_class );
//PROCREG_PROC( PCLASSROOT, GetClassRootEx )( PCLASSROOT root, PCLASSROOT name_class );

	using namespace sack::system::registry;
#endif

#endif
