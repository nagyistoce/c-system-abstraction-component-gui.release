#define DO_LOGGING
#include <sack_types.h>
#include <stdhdrs.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#define ZLIB_DLL
#include <../src/zlib-1.2.5/zlib.h>
#ifdef _WIN32
#include <direct.h> // directory junk
#include <io.h>
#else
#include <unistd.h>
#endif
#ifdef __LINUX__
#include <ctype.h>
//#include <linux/fcntl.h>
#else
#include <fcntl.h>
#endif

#include <string.h>

//#define DO_LOGGING

#include <types.h>
#include <configscript.h>
#include <sharemem.h>
#include <logging.h>
#include <network.h>
#include <timers.h>
#include <filesys.h>
#include <futcrc.h>
#include "relay.h"
#include "accstruc.h"

PACCOUNT AccountList;

static FILE *logfile;
SOCKADDR *server;
PNETBUFFER NetworkBuffers;
char defaultlogin[128];
extern int bUseWatchdog;
extern int bEnableBroadcast;
extern int bForceLowerCase;

#define FILEPERMS

//-------------------------------------------------------------------------

int SendFileChange( PCLIENT pc, char *file, _32 start, _32 length )
{
  // char *data;
   int thisread, hFile;
   if( length )
   {
       hFile = open( file, O_RDONLY|O_BINARY );              // open for reading
       if( hFile != -1 )
       {
          _32 *msg;
          msg = Allocate( length + sizeof( _32[3]) );
          msg[0] = *(_32*)"FDAT";
          msg[1] = start;
          msg[2] = length;
          lseek( hFile, start, SEEK_SET );
          thisread = read( hFile, msg+3, length );
          close( hFile );
          SendTCPLong( pc, msg, thisread + sizeof( _32[3]) );
       }
   }
   else
   {
       _32 msg[3];
       msg[0] = *(_32*)"FDAT";
       msg[1] = start;
       msg[2] = 0;
       SendTCP( pc, msg, 12 );
   }
   return TRUE;
}

//-------------------------------------------------------------------------

int CheckDirectoryOnAccount( PACCOUNT account
                             , PDIRECTORY pDir
                             , char *filename )
{
   char filepath[280];
   struct stat statbuf;
    sprintf( filepath, "%s/%s"
             , pDir->path
             , filename );
   if( stat( filepath, &statbuf ) < 0 )
   {
#ifdef _WIN32
       if( mkdir( filepath ) < 0 ) // all permissions?
#else
       if( mkdir( filepath, -1 ) < 0 ) // all permissions?
#endif
         Log1( "Failed to create directory: %s", filepath );
   }
   else
   {
      if( statbuf.st_mode & S_IFDIR )
      {
         Log( "Directory Existed" );
      }
      else
      {
         Log( "File existed in its place - THIS IS BAD!!!!!" );
      }
   }
   SendTCP( account->pc, &account->NextResponce, 4 );
   return 0;
}

//-------------------------------------------------------------------------

int ReadValidateCRCs( PACCOUNT account, _32 *crc, _32 crclen
                     , char *name, _32 finalsize )
{
	_32 size = 0;
	P_8 mem;
	PFILECHANGE pfc = NULL;
	mem = OpenSpace( NULL, name, &size );
	Log4( "Validating CRCs %p %s %ld(%ld)", mem, name, size, finalsize );
	if( mem )
	{
		_32 n;
		for( n = 0; n < crclen; n++ )
		{
			int blocklen = 4096;
			Log2( "Checking block: %ld of %ld", n, crclen );
			if( ( finalsize - ( n * 4096 ) ) < blocklen )
				blocklen = finalsize - ( n * 4096 );
			if( ( n * 4096 ) > finalsize )
			{
				Log( "fatality - more done than was accounted for in CRCLEN!" );
				return 2;
			}
			if( (( n*4096) + blocklen ) > size )
			{
				// more in real file than our file, so
				// please request all left, and get out.
				if( !pfc )
				{
					pfc = Allocate( sizeof( FILECHANGE ) );
					pfc->start = n*4096;
					Log1( "pfc start: %ld", pfc->start );
				}
				else
					Log( "Invalid CRC continued (length short)" );
				pfc->size = finalsize - pfc->start;
				Log3( "Enque: %ld %ld (%ld)", pfc->start, pfc->size, finalsize );
				EnqueLink( &account->segments, pfc );
				pfc = NULL;
				break;
			}

			// need to put a block limit on this also....
			if( crc[n] != crc32( crc32(0L, NULL, 0), mem + n * 4096, blocklen ) )
			{
				if( !pfc )
				{
					pfc = Allocate( sizeof( FILECHANGE ) );
					pfc->start = n * 4096; // start of this block is bad.
					Log1( "pfc start: %ld", pfc->start );
				}
				// else - we already marked the start of invalid data...
			}
			else
			{
				if( pfc )
				{
					// up to the beginning of this block...
					pfc->size = (n * 4096) - pfc->start;
					Log3( "Enque: %ld %ld (%ld)", pfc->start, pfc->size, finalsize );
					EnqueLink( &account->segments, pfc );
					pfc = NULL;
				}
			}
		}
		if( pfc )
		{
			pfc->size = finalsize - pfc->start;
			Log3( "Enque: %ld %ld (%ld)", pfc->start, pfc->size, finalsize );
			EnqueLink( &account->segments, pfc );
			pfc = NULL;
		}
		CloseSpace( mem );
		return 1;
	}
	else
	{
		// build list of requests which are nicely sized...
		PFILECHANGE pfc;
		int n;
      Log( "Requesting whole file since we know nothing... ");
		for( n = 0; n < (finalsize + ((10*4096) - 1)) / (10*4096); n++ )
		{
			pfc = Allocate( sizeof( FILECHANGE ) );
			pfc->start = n * 4096*10; // start of this block is bad.
			pfc->size = 4096 * 10;
			if( pfc->start + pfc->size > finalsize )
				pfc->size = finalsize - pfc->start;
			Log3( "Finalsize: %ld start: %ld size: %ld", finalsize, pfc->start, pfc->size );
			EnqueLink( &account->segments, pfc );
		}
		return 0;
	}
}

//-------------------------------------------------------------------------

int OpenFileOnAccount( PACCOUNT account
                       , _32 PathID
							, char *filename
							, _32 size
							, time_t time
							, _32 *crc
							, _32 crclen )
{
	char filepath[280];
  // struct stat statbuf;
	PDIRECTORY pDir = GetLink( &account->Directories, PathID );
	if( !pDir )
	{
		lprintf( "Could not find the directory referenced...%d", PathID );
		SendTCP( account->pc, &account->WhatResponce, 4 );
		return 0;
	}
	if( bForceLowerCase )
	{
		char *fname = filename;
		for( fname = filename; fname[0]; fname++ )
			fname[0] = tolower( fname[0] );
		Log1( "Lowered case..: %s", filename );
	}
	if( size == 0xFFFFFFFF )
	{
		Log( "Checking directory ( size -1 )" );
		return CheckDirectoryOnAccount( account, pDir, filename );
	}
	sprintf( filepath, "%s/%s", pDir->path, filename );
	Log3( "File: %s size: %d time: %d"
		 , filepath
		 , size, time );

	if( account->file >= 0 )
	{
		Log( "Closing existing file before opening a new one... ");
		close( account->file );
		account->file = -1;
	}

	if( account->buffer )
	{
      Release( account->buffer );
		account->buffer = NULL;
	}

	// hmm - with this check I don't care about time/date
	// I don't care about existance....

	if( !ReadValidateCRCs(account, crc, crclen
								, filepath, size ) )
	{
#ifdef _WIN32
		account->file = open( filepath, O_RDWR|O_CREAT|O_BINARY );
		SetFileAttributes( filepath, FILE_ATTRIBUTE_NORMAL   );
#else
		account->file = open( filepath, O_RDWR|O_CREAT, 0666 );
#endif
		if( account->file < 0 )
		{
			Log1( "Failed to open... %s", filepath );
			SendTCP( account->pc, &account->WhatResponce, 4 );
			return 0;
		}
		// need to open the file....
	}
	else
	{
#ifdef _WIN32
		account->file = open( filepath, O_RDWR|O_BINARY );
		SetFileAttributes( filepath, FILE_ATTRIBUTE_NORMAL   );
#else
		account->file = open( filepath, O_RDWR, 0666 );
#endif
		if( account->file < 0 )
		{
			Log1( "Failed to open... %s", filepath );
			SendTCP( account->pc, &account->WhatResponce, 4 );
			return 0;
		}
		// need to open the file....
	}
	Log1( "And now the file is open: %d", account->file );
	{
		PFILECHANGE pfc = DequeLink( &account->segments );
		if( pfc )
		{
			_32 msg[3];
			account->buffer = Allocate( pfc->size );

			msg[0] = account->SendResponce;
			msg[1] = pfc->start;  // file position
			msg[2] = pfc->size;   // length...
			Log3( "%s Sent message: (first) SEND fpi:%d len:%d"
				 , account->unique_name, pfc->start, pfc->size );
			SendTCP( account->pc, msg, 12 );
         Release( pfc );
		}
		else
		{
			// no changes... get next file.
			SendTCP( account->pc, &account->NextResponce, 4 );
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

void CloseCurrentFile( PACCOUNT account )
{
    if( account->buffer )
    {
        Release( account->buffer );
        account->buffer = NULL;
    }
    if( account->file >= 0 )
    {
        Log1( "Done with file: %d", account->file );
        close( account->file );
        account->file = -1;
    }
}

//-------------------------------------------------------------------------

void UpdateAccountFile( PACCOUNT account, int start, int size )
{
   //Log3( "Storing Data : file:%d %d %d", account->file, start, size );

    if( size )
     {
          Log3( "Storing Data : file:%d %d %d", account->file, start, size );
          if( lseek( account->file, start, SEEK_SET ) < 0 )
              Log1( "Error in seek: %d", errno );
          if( write( account->file, account->buffer, size ) < 0 )
              Log1( "Error in write: %d", errno );
    }
    {
       // dequeue the segment block, if any left, ask for that data...
       // if non left, close the file, request next.
       PFILECHANGE pfc = DequeLink( &account->segments );
       if( pfc )
       {
        _32 msg[4];
        // going to need a secondary buffer here...
         Release( account->buffer );
         account->buffer = Allocate( pfc->size );
         msg[0] = account->SendResponce; 
         msg[1] = pfc->start;
         msg[2] = pfc->size;
           Log2( "asking for more data... %d %d"
                   , pfc->start, pfc->size );
         SendTCP( account->pc, msg, 12 );
       }
       else
         {
             Log1( "Closing file: %d", account->file );
         CloseCurrentFile( account );
         //Log( "Asking for next file..." );
         SendTCP( account->pc, &account->NextResponce, 4 );
       }
    }
}

//-------------------------------------------------------------------------

char *GetAccountBuffer( PACCOUNT account )
{
   return account->buffer;
}

//-------------------------------------------------------------------------

char *GetAccountDirectory( PACCOUNT account, _32 PathID )
{
   if( account )
      return ((PDIRECTORY)GetLink( &account->Directories, PathID ))->path;
   return NULL;
}

//-------------------------------------------------------------------------

int NextChange( PACCOUNT account )
{
	// _32 changes = 0;
    INDEX idx;
    PMONDIR pDir;
    LIST_FORALL( account->Monitors, idx, PMONDIR, pDir )
		 if( DispatchChanges( pDir->monitor ) )
			 return 1;
    return 0;
}

//-------------------------------------------------------------------------

void CPROC ScanForChanges( PTRSZVAL psv )
{
   PACCOUNT current = (PACCOUNT)psv;
   if( current->DoScanTime && (current->DoScanTime < GetTickCount()) )
   {
        NextChange(current);
        current->DoScanTime = 0;
   }
}


//-------------------------------------------------------------------------

void SendCRCs( PCLIENT pc, CTEXTSTR name, _32 insize )
{
    _32 *crc;
    _32 crclen;
    _32 size = 0;
    P_8 mem;
    mem = OpenSpace( NULL, name, &size );
    crclen = ( size + 4095 ) / 4096;
    if( mem )
    {
        _32 n;
        if( size != insize )
         Log( "Mismatched sizes in send crcs - will cause problems..." );
        crclen = ( size + 4095 ) / 4096;
        crc = Allocate( sizeof( _32 ) * crclen );
        for( n = 0; n < crclen; n++ )
        {
            int blocklen = 4096;
            if( ( size - ( n * 4096 ) ) < blocklen )
                blocklen = size - ( n * 4096 );
            crc[n] = crc32( crc32(0L, NULL, 0), mem + n * 4096, blocklen );
        } 
    }
    else if( size )
      crclen = 0;
    CloseSpace( mem );
   if( crclen )
        SendTCP( pc, crc, crclen * 4 );
}

//-------------------------------------------------------------------------

int CPROC NextFileChange( PTRSZVAL psv
                        , CTEXTSTR filepath
                        , _64 size
								, _64 timestamp
                        , LOGICAL bCreated
                        , LOGICAL bDirectory
                        , LOGICAL bDeleted )
{
	PMONDIR pDir = (PMONDIR)psv;
	if( filepath )
	{
		if( !bDeleted )
		{
			int len ;
			_32 msg[4+64]; // 4 for 4 words of message header
			// +64 for 256 bytes of name
			CTEXTSTR name;
			char *charmsg;
			if( pDir->flags.bIncoming )
				msg[0] = *(_32*)"STAT";
			else
			{
				msg[0] = *(_32*)"FILE";
				strcpy( pDir->account->LastFile, filepath );
			}
			msg[1] = size;
			msg[2] = timestamp;
			msg[3] = pDir->PathID;
			len = 16;
			name = pathrchr( filepath );
			if( !name )
				name = filepath;
			else
				name++;
			charmsg = (char*)msg;
			len += sprintf( charmsg+len, "%c%s"
							  , strlen( name )
							  , name );
			Log5( "Announced %4.4s %s (%d,%d) %d", msg
				 , filepath, msg[1], msg[2], len );
			SendTCP( pDir->account->pc, msg, len );
			if( !pDir->flags.bIncoming )
				SendCRCs( pDir->account->pc, filepath, size );
			return 0;
		}
		else
		{
			CTEXTSTR name;
			_8 msg[256];
			*(_32*)msg = *(_32*)"KILL";
			*(_32*)(msg+4) = pDir->PathID;
			name = pathrchr( filepath );
			if( !name )
				name = filepath;
			else
				name++;
			if( !pDir->flags.bIncoming )
				pDir->account->LastFile[0] = 0;
			msg[8] = sprintf( msg + 9, "%s", name );
			SendTCP( pDir->account->pc, msg, 9 + msg[8] );
			Log2( "Announced %4.4s %s", msg
				 , filepath );
			return 0;
		}
	}
	else
	{
		pDir->account->LastFile[0] = 0;
		if( pDir->flags.bIncoming )
		{
			Log( "All files on incoming directories have reported for stat." );
			EndMonitor( pDir->monitor ); // no longer need THIS...
			DeleteLink( &pDir->account->Monitors, pDir );
			Release( pDir );
		}
		else
		{
         Log( "Reported NULL filepath on outgoing..." );
		}
		return 1;
	}
}

//-------------------------------------------------------------------------

PACCOUNT LoginEx( PCLIENT pc, char *user, _32 dwIP DBG_PASS )
{
	PACCOUNT current;
	if( !dwIP ) // localhost login
	{

	}
	for( current = AccountList; current; current = current->next )
	{
		if( !strcmp( current->unique_name, user ) )
		{
			int start = GetTickCount() + 2000;

			if( current->pc )
			{
				fprintf( logfile, "Client was open here... closing the client...\n" );
				fflush( logfile );
			}

			while( current->pc && ( start > GetTickCount() ) )
				Sleep(0);

			if( current->pc )
			{
				fprintf( logfile, "%s(%d): Hmm connection is hanging around peristant...\n", __FILE__, __LINE__ );
				fflush( logfile );
				fprintf( logfile, "%s(%d): We have forced aquisition of the account\n", __FILE__, __LINE__ );
				fflush( logfile );
				RemoveClient( current->pc ); // it's dead.
			}

			current->logincount++;
			current->pc = pc;
			CloseCurrentFile( current );

#ifdef _DEBUG
			Log5( "%s(%d):" "Login Success:(%d) %s at %s", pFile, nLine
				 , current->logincount
				 , user
				 , (char*)inet_ntoa( *(struct in_addr*)&dwIP ) );
#endif
			fprintf( logfile, "Login Success:(%d) %s at %s\n"
					 , current->logincount
					 , user
					 , (char*)inet_ntoa( *(struct in_addr*)&dwIP ) );
			fflush( logfile );

			{
				INDEX PathID = 0;
				INDEX idx;
				PDIRECTORY pDirectory;
				PMONDIR pDir;
				LIST_FORALL( current->Monitors, idx, PMONDIR, pDir )
				{
					Log( "Closing current account monitors... " );
					// otherwise we probably just opened these!!!!
					if( !pDir->flags.bIncoming )
					{
						EndMonitor( pDir->monitor );
						Release( pDir );
						SetLink( &current->Monitors, idx, NULL );
					}
				}
				LIST_FORALL( current->Directories, idx, PDIRECTORY, pDirectory )
				{
					PMONDIR pDir = Allocate( sizeof( MONDIR ) );
					pDir->PathID = PathID++;
					pDir->account = current;
					pDir->flags.bIncoming = pDirectory->flags.bIncoming;
					pDir->pDirectory = pDirectory;
					pDir->monitor = MonitorFiles( pDirectory->path, 0 );
					pDir->pHandler = AddExtendedFileChangeCallback( pDir->monitor
																				 , pDirectory->mask
																				 , NextFileChange
																				 , (PTRSZVAL)pDir );
					Log3( "%s %s monitor pathname is : %s"
						 , user
						 , pDir->flags.bIncoming?"incoming":"outgoing"
						 , pDirectory->path );
					SetLink( &current->Monitors, pDir->PathID, pDir );
				}
			}
			return current;
		}
	}
	if( logfile && user )
	{
		fprintf( logfile, "Login Error  : %s at %s\n",user, inet_ntoa( *(struct in_addr*)&dwIP ) );
		fflush( logfile );
	}
	return NULL;
}

//-------------------------------------------------------------------------

void Logout( PACCOUNT current )
{
    INDEX idx;
    PMONDIR pDir;
    if( !current )
        return;
	 SetNetworkLong( current->pc, NL_ACCOUNT, 0 );
    Log( "Logout - clear current monitor..." );
    LIST_FORALL( current->Monitors, idx, PMONDIR, pDir )
    {
        Log( "Logout..." );
        EndMonitor( pDir->monitor );
        Release( pDir );
        SetLink( &current->Monitors, idx, NULL );
	 }
	 {
       PFILECHANGE pfc;
		 while ( pfc = DequeLink( &current->segments ) )
			 Release( pfc );
	 }
    DeleteLinkQueue( &current->segments );
    DeleteList( &current->Monitors );
    RemoveTimer( current->timer );
    current->logincount--;
    current->pc = NULL;
    CloseCurrentFile( current );
    fprintf( logfile, "Logout Success:(%d) %s\n"
             , current->logincount
             , current->unique_name );
    fflush( logfile );
}

//-------------------------------------------------------------------------

void CloseAllAccounts( void )
{
	PACCOUNT current;
	while( current = AccountList )
	{
		PDIRECTORY pDirectory;
		INDEX idx;
		if( current->pc )
         Logout( current );
		LIST_FORALL( current->Directories, idx, PDIRECTORY, pDirectory )
		{
			Release( pDirectory );
		}
		DeleteList( &current->Directories );
		UnlinkThing( current );
		Release( current );
	}
	{
		PNETBUFFER pNetBuf;
		while( pNetBuf = NetworkBuffers )
		{
			UnlinkThing( pNetBuf );
			ReleaseAddress( pNetBuf->sa );
			Release( pNetBuf->buffer );
			Release( pNetBuf );
		}
	}
}

//-------------------------------------------------------------------------


PNETBUFFER FindNetBuffer( char *address )
{
    PNETBUFFER pNetBuffer;
    SOCKADDR *sa = CreateSockAddress( address, 3000 );
    pNetBuffer = NetworkBuffers;
    while( pNetBuffer )
    {
        if( *(_64*)pNetBuffer->sa == *(_64*)sa )
        {
            ReleaseAddress( sa );
            return pNetBuffer;
        }
        pNetBuffer = pNetBuffer->next;
    }
    pNetBuffer = Allocate( sizeof( NETBUFFER ) );
    pNetBuffer->sa = sa;
    pNetBuffer->buffer = Allocate( 1024 );
    pNetBuffer->size = 0;
    pNetBuffer->valid = 0;
    LinkThing( NetworkBuffers, pNetBuffer );
    return pNetBuffer;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetCommon( PTRSZVAL psv, arg_list args )
{
    //PARAM( args, char*, name );
    PACCOUNT account;
    if( account = (PACCOUNT)psv )
    {
     //  strcpy( account->CommonDirectory, name );
    }
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC CreateUser( PTRSZVAL psv, arg_list args )
{
    PARAM( args, char*, name );
    PACCOUNT account;
    if( account = (PACCOUNT)psv )
    {
      LinkThing( AccountList, account );
    }
    account = Allocate( sizeof( ACCOUNT ) );
    MemSet( account, 0, sizeof( ACCOUNT ) );
    account->file = -1;
    account->SendResponce = *(_32*)"SEND";
    account->NextResponce = *(_32*)"NEXT";
    account->WhatResponce = *(_32*)"WHAT";
	 strcpy( account->unique_name, name );
	 lprintf( "Create user account: %s", name );
	 return (PTRSZVAL)account;
}

//-------------------------------------------------------------------------

void TrimSlashes( char *path )
{
    while( ( path[strlen(path)-1] == '/' )
            || ( path[strlen(path)-1] == '\\' )
            || ( path[strlen(path)-1] == ' ' )
            || ( path[strlen(path)-1] == '\t' )
          )
            path[strlen(path)-1] = 0;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC AddIncomingPath( PTRSZVAL psv, arg_list args )
{
    PARAM( args, char*, path );
    PACCOUNT account;
    if( account = (PACCOUNT)psv )
    {
        TrimSlashes( path );
        {
			  PDIRECTORY NewPath = Allocate( strlen( path ) + 1 + sizeof( DIRECTORY ) );
			  NewPath->ID = 0;
           NewPath->flags.bIncoming = 1;
           NewPath->name = 0;
			  strcpy( NewPath->path, path );
			  NewPath->mask = pathrchr( NewPath->path );
			  if( NewPath->mask && ( strchr( NewPath->mask, '*' ) ||
										strchr( NewPath->mask, '?' ) ) )
			  {
				  ((TEXTSTR)NewPath->mask)[0] = 0;
			  }
			  else
				  NewPath->mask = NULL;
			  AddLink( &account->Directories, NewPath );
		  }
		  lprintf( "add incoming %s to %s", path, account->unique_name );
    }
    else
    {
      Log( "Incoming specified without a user name." );
    }
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC AddOutgoingPath( PTRSZVAL psv, arg_list args )
{
    PARAM( args, char*, path );
    PACCOUNT account;
    if( account = (PACCOUNT)psv )
    {
        TrimSlashes( path );
        {
			  PDIRECTORY NewPath = Allocate( strlen( path ) + 1 + sizeof( DIRECTORY ) );
			  NewPath->ID = 0;
           NewPath->flags.bIncoming = 0;
           NewPath->name = 0;
			  strcpy( NewPath->path, path );
			  NewPath->mask = pathrchr( NewPath->path );
			  if( NewPath->mask && ( strchr( NewPath->mask, '*' ) ||
										strchr( NewPath->mask, '?' ) ) )
			  {
				  ((TEXTSTR)NewPath->mask)[0] = 0;
			  }
			  else
				  NewPath->mask = NULL;
			  AddLink( &account->Directories, NewPath );
		  }
		  lprintf( "add outgoing %s to %s", path, account->unique_name );
    }
    else
    {
      Log( "Outgoing specified without a user name." );
    }
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetAccountAddress( PTRSZVAL psv, arg_list args )
{
    PARAM( args, char*, address );
    PACCOUNT account;
    if( account = (PACCOUNT)psv )
    {
         account->netbuffer = FindNetBuffer( address );
         lprintf( "Set account %s listento at: %s (3000?)", account->unique_name, address );
    }
    else
    {
      Log( "Listen specified without a user name." );
    }
   return psv;

}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetServerAddress( PTRSZVAL psv, arg_list args )
{
    PARAM( args, char *, address );
    if( server )
    {
        Log( "Multiple servers specified, replacing first..." );
      ReleaseAddress( server );
	 }
#ifdef _WIN32
	 {
		 WSADATA ws;  // used to start up the socket services...
       WSAStartup( MAKEWORD(1,1), &ws );
	 }
#endif
    server = CreateSockAddress( address, 3000 );
     lprintf( "connect to: %s (3000?)", address );
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetLoginName( PTRSZVAL psv, arg_list args )
{
   PARAM( args, char*, name );
    strcpy( defaultlogin, name );
    lprintf( "Login as: %s", defaultlogin );
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetUseWatchdog( PTRSZVAL psv, arg_list args )
{
    PARAM( args, LOGICAL, bit );
   bUseWatchdog = bit;
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetSendBroadcast( PTRSZVAL psv, arg_list args )
{
    PARAM( args, LOGICAL, bit );
   bEnableBroadcast = bit;
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetForceCase( PTRSZVAL psv, arg_list args )
{
    PARAM( args, char *, opt );
    if( opt[0] == 'l' || opt[0] == 'L' )
      bForceLowerCase = 1;
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC FinishReading( PTRSZVAL psv )
{
   PACCOUNT account;
    if( account = (PACCOUNT)psv )
    {
      LinkThing( AccountList, account );
    }
   return 0;
}


//-------------------------------------------------------------------------

int ReadAccounts( char *configname )
{
    PCONFIG_HANDLER pch;
   logfile = fopen( "login.log", "at+" );
   if( !logfile )
      logfile = fopen( "login.log", "wt" );
    pch = CreateConfigurationEvaluator();
  // AddConfiguration( pch, "common=%m", SetCommon );
   AddConfiguration( pch, "user=%m", CreateUser );
   //AddConfiguration( pch, "path %i incoming=%m", AddIncomingPathID );
   //AddConfiguration( pch, "path %i outgoing=%m", AddOutgoingPathID );
   //AddConfiguration( pch, "path named %m incoming=%m", AddIncomingPathName );
   //AddConfiguration( pch, "path named %m outgoing=%m", AddOutgoingPathName );
   AddConfiguration( pch, "incoming=%m", AddIncomingPath );
   AddConfiguration( pch, "outgoing=%m", AddOutgoingPath );
    AddConfiguration( pch, "listen=%w", SetAccountAddress );
    //AddConfiguration( pch, "allow=%w", AddAllowedAddress );
    AddConfiguration( pch, "server=%w", SetServerAddress );
    AddConfiguration( pch, "login=%w", SetLoginName );
    AddConfiguration( pch, "watchdog=%b", SetUseWatchdog );
    AddConfiguration( pch, "broadcast=%b", SetSendBroadcast );
    //AddConfiguration( pch, "forcecase=%b", SetForceCase );
    SetConfigurationEndProc( pch, FinishReading );
   ProcessConfigurationFile( pch, configname, 0 );
   DestroyConfigurationEvaluator( pch );
   return 0;
}
