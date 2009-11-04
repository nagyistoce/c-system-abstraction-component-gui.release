/* Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd
   See the file COPYING for copying permission.
*/

/* External API definitions */

#if defined(_MSC_EXTENSIONS) && !defined(__BEOS__) && !defined(__CYGWIN__)
#define XML_USE_MSC_EXTENSIONS 1
#endif

// also ifdef SACK!
#ifndef XMLIMPORT
# include <sack_types.h>
# ifndef __STATIC__
#  ifdef SEXPAT_SOURCE
#   define XMLIMPORT EXPORT_METHOD
#  else
#   define XMLIMPORT IMPORT_METHOD
#  endif
# else
#  ifdef SEXPAT_SOURCE
#   define XMLIMPORT
#  else
#   define XMLIMPORT extern
#  endif
# endif
# ifdef __WATCOMC__
#  define XMLCALL CPROC
# endif
# define XML_NS 1
# define XML_DTD 1
# define XML_CONTEXT_BYTES 1024

#ifdef __cplusplus
#define CONST
#else
#define CONST const
#endif

/* we will assume all Windows platforms are little endian */
# define BYTEORDER 1234

/* Windows has memmove() available. */
# define HAVE_MEMMOVE
#endif
/* Expat tries very hard to make the API boundary very specifically
   defined.  There are two macros defined to control this boundary;
   each of these can be defined before including this header to
   achieve some different behavior, but doing so it not recommended or
   tested frequently.

   XMLCALL    - The calling convention to use for all calls across the
                "library boundary."  This will default to cdecl, and
                try really hard to tell the compiler that's what we
                want.

   XMLIMPORT  - Whatever magic is needed to note that a function is
                to be imported from a dynamically loaded library
                (.dll, .so, or .sl, depending on your platform).

   The XMLCALL macro was added in Expat 1.95.7.  The only one which is
   expected to be directly useful in client code is XMLCALL.

   Note that on at least some Unix versions, the Expat library must be
   compiled with the cdecl calling convention as the default since
   system headers may assume the cdecl convention.
*/
#ifndef XMLCALL
#if defined(XML_USE_MSC_EXTENSIONS)
#if defined( __clrpure ) || defined( __cplusplus_cli )
#define XMLCALL 
#else
#define XMLCALL __cdecl
#endif
#elif defined(__GNUC__) && defined(__i386)
#define XMLCALL __attribute__((cdecl))
#else
/* For any platform which uses this definition and supports more than
   one calling convention, we need to extend this definition to
   declare the convention used on that platform, if it's possible to
   do so.

   If this is the case for your platform, please file a bug report
   with information on how to identify your platform via the C
   pre-processor and how to specify the same calling convention as the
   platform's malloc() implementation.
*/
#define XMLCALL
#endif
#endif  /* not defined XMLCALL */


#if !defined(XML_STATIC) && !defined(XMLIMPORT)
#ifndef XML_BUILDING_EXPAT
/* using Expat from an application */

#ifdef XML_USE_MSC_EXTENSIONS
#define XMLIMPORT __declspec(dllimport)
#endif

#endif
#endif  /* not defined XML_STATIC */

/* If we didn't define it above, define it away: */
#ifndef XMLIMPORT
#define XMLIMPORT
#endif


#define XMLPARSEAPI(type) XMLIMPORT type XMLCALL

#ifdef __cplusplus
#define SEXPAT_NAMESPACE namespace sack { namespace xml { namespace parse {
#define USE_SEXPAT_NAMESPACE using namespace sack::xml::parse;
#define SEXPAT_NAMESPACE_END }}}
#else
#define SEXPAT_NAMESPACE 
#define USE_SEXPAT_NAMESPACE 
#define SEXPAT_NAMESPACE_END 

#endif

SEXPAT_NAMESPACE

#ifdef __cplusplus_cli
#ifdef UNICODE
#define XML_UNICODE_WCHAR_T
#endif
#endif

#ifdef XML_UNICODE_WCHAR_T
#define XML_UNICODE
#endif

#ifdef XML_UNICODE     /* Information is UTF-16 encoded. */
#ifdef XML_UNICODE_WCHAR_T
typedef wchar_t XML_Char;
typedef wchar_t XML_LChar;
#else
typedef unsigned short XML_Char;
typedef char XML_LChar;
#endif /* XML_UNICODE_WCHAR_T */
#else                  /* Information is UTF-8 encoded. */
typedef char XML_Char;
typedef char XML_LChar;
#endif /* XML_UNICODE */
