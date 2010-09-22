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
#define vsnprintf StringCbVPrintf
#define snprintf  StringCbPrintf
#define printf    wprintf
// define sprintf here.
#undef sprintf
#define sprintf(buf,format,...)  StringCchPrintf( buf, sizeof( buf )/sizeof(TEXTCHAR), format,##__VA_ARGS__ )
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

// len should be passed as character count.
#define vsnprintf(buf,len,format,args) StringCchVPrintf( buf, len, format, args )
#define snprintf(buf,len,format,...)  StringCchPrintf( buf, len, format,##__VA_ARGS__ )

// this is a success condition to redefine this here (for now)
#define sprintf(buf,format,...)  StringCchPrintf( buf, sizeof( buf )/sizeof(TEXTCHAR), format,##__VA_ARGS__ )

# endif
#endif


#endif
