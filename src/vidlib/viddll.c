#include <windows.h>
#ifndef __STATIC__
BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
   return TRUE;
}

BOOL APIENTRY LibMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
   return TRUE;
}
#endif
// $Log: $
