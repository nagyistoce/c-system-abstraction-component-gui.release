#include <pssql.h>
#include <sqloptint.h>


#ifdef __cplusplus
#define _OPTION_NAMESPACE namespace options {
#define _OPTION_NAMESPACE_END };
#define USE_OPTION_NAMESPACE 	using namespace sack::options;

#else
#define _OPTION_NAMESPACE
#define _OPTION_NAMESPACE_END
#define USE_OPTION_NAMESPACE
#endif

#define SACK_OPTION_NAMESPACE SACK_NAMESPACE _OPTION_NAMESPACE
#define SACK_OPTION_NAMESPACE_END _OPTION_NAMESPACE_END SACK_NAMESPACE_END

SACK_OPTION_NAMESPACE

#define SQLGetProfileInt SACK_GetProfileInt
//#define SQLGetProfileBlob SACK_GetProfileBlob
//#define SQLWriteProfileBlob SACK_WriteProfileBlob
#define SQLGetPrivateProfileInt SACK_GetPrivateProfileInt
#define SQLGetProfileIntEx SACK_GetProfileIntEx
#define SQLGetProfileStringEx SACK_GetProfileStringEx


SQLGETOPTION_PROC( int, SACK_GetPrivateProfileString )( CTEXTSTR pSection, CTEXTSTR pOptname, CTEXTSTR pDefaultbuf, char *pBuffer, _32 nBuffer, CTEXTSTR pININame );
SQLGETOPTION_PROC( S_32, SACK_GetPrivateProfileInt )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 nDefault, CTEXTSTR pININame );
SQLGETOPTION_PROC( int, SACK_GetProfileString )( CTEXTSTR pSection, CTEXTSTR pOptname, CTEXTSTR pDefaultbuf, char *pBuffer, _32 nBuffer );
SQLGETOPTION_PROC( int, SACK_GetProfileBlob )( CTEXTSTR pSection, CTEXTSTR pOptname, char **pBuffer, _32 *pnBuffer );
SQLGETOPTION_PROC( int, SACK_GetProfileBlobOdbc )( PODBC odbc, CTEXTSTR pSection, CTEXTSTR pOptname, char **pBuffer, _32 *pnBuffer );
SQLGETOPTION_PROC( S_32, SACK_GetProfileInt )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 defaultval );
SQLGETOPTION_PROC( int, SACK_WritePrivateProfileString )( CTEXTSTR pSection, CTEXTSTR pName, CTEXTSTR pValue, CTEXTSTR pINIFile );
SQLGETOPTION_PROC( S_32, SACK_WritePrivateProfileInt )( CTEXTSTR pSection, CTEXTSTR pName, S_32 value, CTEXTSTR pINIFile );
SQLGETOPTION_PROC( int, SACK_WriteProfileString )( CTEXTSTR pSection, CTEXTSTR pName, CTEXTSTR pValue );
SQLGETOPTION_PROC( int, SACK_WriteProfileBlob )( CTEXTSTR pSection, CTEXTSTR pOptname, char *pBuffer, _32 nBuffer );
SQLGETOPTION_PROC( int, SACK_WriteProfileBlobOdbc )( PODBC odbc, CTEXTSTR pSection, CTEXTSTR pOptname, char *pBuffer, _32 nBuffer );
SQLGETOPTION_PROC( S_32, SACK_WriteProfileInt )( CTEXTSTR pSection, CTEXTSTR pName, S_32 value );

SQLGETOPTION_PROC( int, SACK_GetPrivateProfileStringEx )( CTEXTSTR pSection, CTEXTSTR pOptname, CTEXTSTR pDefaultbuf, char *pBuffer, _32 nBuffer, CTEXTSTR pININame, LOGICAL bQuiet );
SQLGETOPTION_PROC( S_32, SACK_GetPrivateProfileIntEx )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 nDefault, CTEXTSTR pININame, LOGICAL bQuiet );
SQLGETOPTION_PROC( int, SACK_GetProfileStringEx )( CTEXTSTR pSection, CTEXTSTR pOptname, CTEXTSTR pDefaultbuf, char *pBuffer, _32 nBuffer, LOGICAL bQuiet );
//SQLGETOPTION_PROC( int, SACK_GetProfileBlobEx )( CTEXTSTR pSection, CTEXTSTR pOptname, char **pBuffer, _32 *pnBuffer, LOGICAL bQuiet );
SQLGETOPTION_PROC( S_32, SACK_GetProfileIntEx )( CTEXTSTR pSection, CTEXTSTR pOptname, S_32 defaultval, LOGICAL bQuiet );
SQLGETOPTION_PROC( int, SACK_WritePrivateProfileStringEx )( CTEXTSTR pSection, CTEXTSTR pName, CTEXTSTR pValue, CTEXTSTR pINIFile, LOGICAL bQuiet );
SQLGETOPTION_PROC( S_32, SACK_WritePrivateProfileIntEx )( CTEXTSTR pSection, CTEXTSTR pName, S_32 value, CTEXTSTR pINIFile, LOGICAL bQuiet );
SQLGETOPTION_PROC( int, SACK_WriteProfileStringEx )( CTEXTSTR pSection, CTEXTSTR pName, CTEXTSTR pValue, LOGICAL bQuiet );
//SQLGETOPTION_PROC( int, SACK_WriteProfileBlobEx )( CTEXTSTR pSection, CTEXTSTR pOptname, char *pBuffer, _32 nBuffer, LOGICAL bQuiet );
SQLGETOPTION_PROC( S_32, SACK_WriteProfileIntEx )( CTEXTSTR pSection, CTEXTSTR pName, S_32 value, LOGICAL bQuiet );




SQLGETOPTION_PROC( void, CreateOptionDatabase )( void );

SQLGETOPTION_PROC( INDEX, GetSystemID )( void );

SQLGETOPTION_PROC( void, EnumOptions )( INDEX parent
					 , int (CPROC *Process)(PTRSZVAL psv, CTEXTSTR name, _32 ID, int flags )
                , PTRSZVAL psvUser );
SQLGETOPTION_PROC( void, EnumOptionsEx )( PODBC odbc, INDEX parent
					 , int (CPROC *Process)(PTRSZVAL psv, CTEXTSTR name, _32 ID, int flags )
                , PTRSZVAL psvUser );

SQLGETOPTION_PROC( _32, GetOptionStringValueEx )( PODBC odbc, INDEX optval, char *buffer, _32 len DBG_PASS );
SQLGETOPTION_PROC( _32, GetOptionStringValue )( INDEX optval, char *buffer, _32 len );
SQLGETOPTION_PROC( INDEX, GetOptionValueIndex )( INDEX ID );
SQLGETOPTION_PROC( INDEX, GetOptionValueIndexEx )( PODBC odbc, INDEX ID );
SQLGETOPTION_PROC( void, DuplicateOption )( INDEX iRoot, CTEXTSTR pNewName );

//-- private --
SQLGETOPTION_PROC( INDEX, SetOptionStringValue )( INDEX optval, CTEXTSTR pValue );
SQLGETOPTION_PROC( void, DeleteOption )( INDEX iRoot );

SQLGETOPTION_PROC( void, SetOptionDatabase )( PODBC odbc );
SQLGETOPTION_PROC( void, SetOptionDatabaseOption )( PODBC odbc, int bNewVersion );


SQLGETOPTION_PROC( void, NewDuplicateOption )( PODBC odbc, INDEX iRoot, CTEXTSTR pNewName );

SQLGETOPTION_PROC( void, BeginBatchUpdate )( void );
SQLGETOPTION_PROC( void, EndBatchUpdate )( void );

_OPTION_NAMESPACE_END SACK_NAMESPACE_END

USE_OPTION_NAMESPACE
