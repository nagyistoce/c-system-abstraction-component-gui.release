#ifndef InterShellFONTHEADER
#define InterShellFONTHEADER

#include "intershell_local.h"

#if 0
typedef struct font_preset_tag *PFONT_PRESET;


InterShell_PROC( Font *, SelectAFont )( PSI_CONTROL parent, CTEXTSTR *default_name );
InterShell_PROC( Font *, UseAFont )( CTEXTSTR name );

// depricated - used for forward migration...
InterShell_PROC( Font *, CreateAFont )( CTEXTSTR name, Font font, POINTER data, _32 datalen );
#endif
#endif
