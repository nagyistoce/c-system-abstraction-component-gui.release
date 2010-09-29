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

#  endif

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
#define SUFFER_WITH_NO_SNPRINTF
#    ifndef SUFFER_WITH_NO_SNPRINTF
#      define vnsprintf protable_vsnprintf
//   this one gives deprication warnings
//   #    define vsnprintf _vsnprintf

//   this one doesn't work to measure strings
//   #    define vsnprintf(buf,len,format,args) _vsnprintf_s(buf,len,(len)/sizeof(TEXTCHAR),format,args)
//   this one doesn't macro well, and doesnt' measure strings
//  (SUCCEEDED(StringCbVPrintf( buf, len, format, args ))?StrLen(buf):-1)

#      define snprintf portable_snprintf

//   this one gives deprication warnings
//   #    define snprintf _snprintf

//   this one doesn't work to measure strings
//   #    define snprintf(buf,len,format,...) _snprintf_s(buf,len,(len)/sizeof(TEXTCHAR),format,##__VA_ARGS__)
//   this one doesn't macro well, and doesnt' measure strings
//   (SUCCEEDED(StringCbPrintf( buf, len, format,##__VA_ARGS__ ))?StrLen(buf):-1)

// make sure this is off, cause we really don't, and have to include the following
#      undef HAVE_SNPRINTF
#      define PREFER_PORTABLE_SNPRINTF // define this anyhow so we can avoid name collisions
#      ifdef SACK_CORE_BUILD
#        include <../src/snprintf_2.2/snprintf.h>
#      else
#        include <snprintf-2.2/snprintf.h>
#      endif // SACK_CORE_BUILD
#    else // SUFFER_WITH_WARNININGS
#      define snprintf _snprintf
#      define vsnprintf _vsnprintf
#    endif// suffer_with_warnings

#    define sscanf sscanf_s

#  endif // _MSC_VER

#endif
