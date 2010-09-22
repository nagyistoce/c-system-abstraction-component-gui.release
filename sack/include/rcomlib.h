#ifndef RCOM_LIBRARY_INCLUDED
#define RCOM_LIBRARY_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int GetRegistryItem( HKEY hRoot, char *pPrefix, 
                     char *pProduct, char *pKey, 
                     DWORD dwType,  
                     char *nResult, int nSize );

int SetRegistryItem( HKEY hRoot, char *pPrefix,
                     char *pProduct, char *pKey, 
                     DWORD dwType,
                     char *pValue, int nSize );


int GetRegistryInt( char *pProduct, char *pKey, int *Value );
int GetLocalRegistryInt( char *pProduct, char *pKey, int *Value );

int GetRegistryString( char *pProduct, char *pKey, char *Value, int nMaxLen );
int GetLocalRegistryString( char *pProduct, char *pKey, char *Value, int nMaxLen );

int SetRegistryInt( char *pProduct, char *pKey, int Value );
int SetLocalRegistryInt( char *pProduct, char *pKey, int Value );

int SetRegistryString( char *pProduct, char *pKey, char *pValue );
int SetLocalRegistryString( char *pProduct, char *pKey, char *pValue );

//---------------------------------------------------------------
// rc_dialog.c
BOOL SimpleQuery( char *pText, char *pResult, int nResultLen );
void CenterDialog( HWND hWnd );

BOOL OpenRegister( void );
void ShowRegister( void ); // show dialog...

//---------------------------------------------------------------
// register.cpp
int IsRegistered( char *pProgramID, char *pProgramName, int bSilent );
// return 0 - registered
// return 1 - not regitered
// return 2 - expired
// return 3 - did not accept liscensing
// return 4 - tampered / invalid regcode
// return ... more to come ... 

int DoRegister( void );
int  GetRegisterResult( void );
void GetRegisterError( char *pResult );
int RegisterProduct( char *pResult, char *pProductID, char *pProductName );

//---------------------------------------------------------------
// file.c
char *GetFile( char *pMatch, void *pi );

#ifdef __cplusplus
}
#endif

#endif
// $Log: $
