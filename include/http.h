
#ifndef HTTP_PROCESSING_INCLUDED
#define HTTP_PROCESSING_INCLUDED

#include <sack_types.h>

#ifdef HTTP_SOURCE
#define HTTP_EXPORT EXPORT_METHOD
#else
#define HTTP_EXPORT IMPORT_METHOD
#endif
#define HTTPAPI CPROC

#ifdef __cplusplus
#define _HTTP_NAMESPACE namespace http {
#define _HTTP_NAMESPACE_END }
#else
#define _HTTP_NAMESPACE 
#define _HTTP_NAMESPACE_END
#endif
#define HTTP_NAMESPACE TEXT_NAMESPACE _HTTP_NAMESPACE
#define HTTP_NAMESPACE_END _HTTP_NAMESPACE_END TEXT_NAMESPACE_END

TEXT_NAMESPACE
	_HTTP_NAMESPACE


HTTP_EXPORT struct HttpState * HTTPAPI CreateHttpState( void );
HTTP_EXPORT void HTTPAPI DestroyHttpState( struct HttpState *pHttpState );
HTTP_EXPORT void HTTPAPI AddHttpData( struct HttpState *pHttpState, POINTER buffer, int size );
// returns TRUE if completed until content-length
// if content-length is not specified, data is still collected, but the status
// never results TRUE.
HTTP_EXPORT int HTTPAPI ProcessHttp( struct HttpState *pHttpState ); 
HTTP_EXPORT PTEXT HTTPAPI GetHttpResponce( struct HttpState *pHttpState );
HTTP_EXPORT PTEXT HTTPAPI GetHttpContent( struct HttpState *pHttpState );
HTTP_EXPORT void HTTPAPI ProcessHttpFields( struct HttpState *pHttpState, void (CPROC*f)( PTRSZVAL psv, PTEXT name, PTEXT value ), PTRSZVAL psv );
HTTP_EXPORT void HTTPAPI EndHttp( struct HttpState *pHttpState );

	_HTTP_NAMESPACE_END
TEXT_NAMESPACE_END

#ifdef __cplusplus
using namespace sack::containers::text::http;
#endif

#endif