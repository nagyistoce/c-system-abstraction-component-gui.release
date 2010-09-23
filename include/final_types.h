//  these are rude defines overloading otherwise very practical types
// but - they have to be dispatched after all standard headers.

#ifndef FINAL_TYPES
#define FINAL_TYPES

# ifdef _MSC_VER

#include <stdio.h>
#include <baseTsd.h>
#include <windef.h>
#include <winbase.h>  // this redefines lprintf sprintf etc... and strsafe is preferred
#include <winuser.h> // more things that need override by strsafe.h
#include <tchar.h>
#include <strsafe.h>

#endif

// may consider changing this to P_16 for unicode...
#ifdef UNICODE
# ifndef NO_UNICODE_C
#define strrchr          wcsrchr  
#define strchr           wcschr
#define strncpy          wcsncpy
# ifdef strcpy
#  undef strcpy
# endif
#define strcpy           StrCpy
#define strcmp           wcscmp

#define strlen           wcslen
#define stricmp          wcsicmp
#define strnicmp         wcsnicmp
#define stat             _wstat
#  ifdef _MSC_VER
#    ifndef __cplusplus_cli
#define fprintf   fwprintf
#define atoi      _wtoi
#define printf    wprintf
// define sprintf here.
#       endif
#    endif
#    ifdef _ARM_
// len should be passed as character count. this was the wrongw ay to default this.
#define snprintf StringCbPrintf
//#define snprintf StringCbPrintf
#    endif
#  endif
#else // not unicode...
# ifdef _MSC_VER

#ifdef UNICDE
#define atoi      _wtoi
#define fprintf   fwprintf
#endif

# endif
#endif

#  ifdef _MSC_VER
#    define vsnprintf(buf,len,format,args) (SUCCEEDED(StringCbVPrintf( buf, len, format, args ))?StrLen(buf):-1)
#    define snprintf(buf,len,format,...)  (SUCCEEDED(StringCbPrintf( buf, len, format,##__VA_ARGS__ ))?StrLen(buf):-1)
#  endif

#endif
