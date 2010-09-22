#include <types.h>

#ifndef ACCOUNT_STRUCTURES_DEFINED
#include "accstruc.h"
#endif

	PACCOUNT LoginEx( PCLIENT pc, char *user, _32 dwIP DBG_PASS );
#define Login( pc, user, ip ) LoginEx( pc, user, ip DBG_SRC )
void Logout( PACCOUNT current );
void ReadAccounts( char *configname );
int NextChange( PACCOUNT account );
int SendChange( PACCOUNT account, _32 start, _32 size );
char *GetAccountBuffer( PACCOUNT account );
void UpdateAccountFile( PACCOUNT account, int start, int size );
int OpenFileOnAccount( PACCOUNT account
                       , _32 PathID
                       , char *filename
                       , _32 size
                       , time_t time 
                       , _32 *crc
                       , _32 crclen );

void CloseCurrentFile( PACCOUNT account );

int CheckDirectoryOnAccount( PACCOUNT account
                           , _32 PathID
									, char *filename );

void CloseAllAccounts( void );

int SendFileChange( PCLIENT pc, char *file, _32 start, _32 length );

