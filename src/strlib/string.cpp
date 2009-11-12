
#include "strlib.hpp"

bool STRING_CONVERSION::Reduce( char *out, WCHAR *in )
{
   char *pc, *pend = NULL;
   WCHAR *pw;

   if( !out || !in )
      return false;

   pw = in;
   pc = out;
   while( *(pw) )
   {
      (*pc) = (CHAR)(*pw);
      if( pend ) 
      {
         if( *pc != ' ' )
            pend = NULL;
      }
      else
         if( *pc == ' ' )
            pend = pc;
      pc++;
      pw++;
   }
   if( pend ) 
      pc = pend;
   *pc= 0; // terminate gateway address string...
   return true;
}

bool STRING_CONVERSION::Reduce( char *out, _bstr_t in )
{
   char *pc, *pend = NULL;
   WCHAR *pw;
   if( !out || !in )
      return false;
   pw = in;
   pc = out;
   while( *(pw) )
   {
      (*pc) = (CHAR)(*pw);
      if( pend ) 
      {
         if( *pc != ' ' )
            pend = NULL;
      }
      else
         if( *pc == ' ' )
            pend = pc;
      pc++;
      pw++;
   }
   if( pend ) 
      pc = pend;
   *pc = 0; // terminate gateway address string...
   return true;
}

bool STRING_CONVERSION::Reduce( char *out, char *in )
{
   char *pc, *pend = NULL;
   char *pw;

   if( !out || !in )
      return false;
   pc = out;
   pw = in;
   while( *(pw) )
   {
      (*pc) = (*pw);
      if( pend ) 
      {
         if( *pc != ' ' )
            pend = NULL;
      }
      else
         if( *pc == ' ' )
            pend = pc;
      pc++;
      pw++;
   }
   if( pend ) 
      pc = pend;
   *pc = 0; // terminate gateway address string...
   return true;
}

