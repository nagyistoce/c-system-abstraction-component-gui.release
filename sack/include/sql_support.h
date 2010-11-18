#ifndef ADO_SQL_SUPPORT_MACROS
#define ADO_SQL_SUPPORT_MACROS

#include "Logging.H"

HRESULT hr; // global instance... non-threadsafe?!
_com_error cer(0);

#define LOGERROR(hr)  ((hr)?((cer = hr), Log1( WIDE("%s\n"), cer.ErrorMessage() ) \
                                             ):0 ) 
#define SQL_BEGIN try {

#define DO_SQL(exp)  ( hr = (exp) )
#define SQL_EXPR(exp) ((hr=0),(exp))

#define SQL_END   } catch( _com_error &e )                            \
   {                                                                  \
      /* Crack _com_error */                                          \
      _bstr_t bstrSource(e.Source());                                 \
      _bstr_t bstrDescription(e.Description());                       \
                                                                      \
      Log( WIDE("Exception thrown for classes generated by #import") );     \
      Log1( WIDE("\tCode = %08lx\n"),      e.Error());                      \
      Log1( WIDE("\tCode meaning = %s\n"), e.ErrorMessage());               \
      Log1( WIDE("\tSource = %s\n"),       (LPCTSTR) bstrSource);           \
      Log1( WIDE("\tDescription = %s\n"),  (LPCTSTR) bstrDescription);      \
                                                                      \
      /* Errors Collection may not always be populated       */       \
      /*LOGERROR(hr); */ /*this should already be in info*/           \
   }                                                                  \
   catch(...)                                                         \
   {                                                                  \
      hr = -1;                                                        \
      Log( WIDE("*** Unhandled Exception ***") );                           \
   }

#define SQL(e) SQL_BEGIN DO_SQL(e); SQL_END
#define _SQL(e) SQL_BEGIN SQL_EXPR(e); SQL_END

#endif

// $Log: $