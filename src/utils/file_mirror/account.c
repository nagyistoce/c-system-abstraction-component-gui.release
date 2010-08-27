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
extern int bGetWinners; // contact a dekware scan agent to parse winners.
extern int bReceiveLinkedPacket;

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

PTRSZVAL CPROC SetWinnerPort( PTRSZVAL psv, arg_list args )
{
    PARAM( args, S_64, port );
    PACCOUNT account;
    if( account = (PACCOUNT)psv )
    {
        account->WinnerPort = port;
    }
    else
    {
      Log( "Winner port specified without a user name." );
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

PTRSZVAL CPROC SetReceiveLinked( PTRSZVAL psv, arg_list args )
{
    PARAM( args, LOGICAL, bit );
   bReceiveLinkedPacket = bit;
   return psv;
}

//-------------------------------------------------------------------------

PTRSZVAL CPROC SetUseDekware( PTRSZVAL psv, arg_list args )
{
    PARAM( args, LOGICAL, bit );
   bGetWinners = bit;
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
    AddConfiguration( pch, "winners=%i", SetWinnerPort );
    //AddConfiguration( pch, "allow=%w", AddAllowedAddress );
    AddConfiguration( pch, "server=%w", SetServerAddress );
    AddConfiguration( pch, "login=%w", SetLoginName );
    AddConfiguration( pch, "watchdog=%b", SetUseWatchdog );
    AddConfiguration( pch, "receive=%b", SetReceiveLinked );
    AddConfiguration( pch, "dekware=%b", SetUseDekware );
    AddConfiguration( pch, "broadcast=%b", SetSendBroadcast );
    //AddConfiguration( pch, "forcecase=%b", SetForceCase );
    SetConfigurationEndProc( pch, FinishReading );
   ProcessConfigurationFile( pch, configname, 0 );
   DestroyConfigurationEvaluator( pch );
   return 0;
}
// $Revision: 1.65 $
// $Log: account.c,v $
// Revision 1.65  2003/11/10 15:42:19  jim
// Massive changes to port to newest filemon lib
//
// Revision 1.64  2003/07/25 15:57:44  jim
// Update to make watcom compile happy
//
// Revision 1.63  2003/06/02 22:48:22  jim
// Intense logging added - found problem code though
//
// Revision 1.62  2003/06/02 19:08:29  jim
// Added errno definition header
//
// Revision 1.61  2003/06/02 19:05:58  jim
// Restored some logging... Added som elogging, handled failures better
//
// Revision 1.60  2003/04/23 17:47:02  jim
// When logging in - send options.... Windows clients will tell linux clients to force lower case...
//
// Revision 1.59  2003/04/21 22:09:34  jim
// Add some logging and safeguards... for when things DO go bad.
//
// Revision 1.58  2003/04/21 17:45:52  jim
// And ask for the correct size of missing file.
//
// Revision 1.57  2003/04/21 17:34:32  jim
// When the file doesn't exist... build the right requests
//
// Revision 1.56  2003/04/21 15:46:22  jim
// Modify the scan loop - max out segment sends...
//
// Revision 1.55  2003/04/18 22:40:16  jim
// And remove the 'current' monitor at logout
//
// Revision 1.54  2003/04/18 22:24:03  jim
// Don't destory a file while it's queued... isntead mark it for later destruction
//
// Revision 1.53  2003/04/18 21:36:47  jim
// Delay deletions into change queue.
//
// Revision 1.52  2003/04/18 20:38:34  jim
// Cleaned up unused variables, and quietened logging
//
// Revision 1.51  2003/04/18 17:48:00  jim
// Add another layer of delay of forwarding changes.
//
// Revision 1.50  2003/04/18 15:42:59  jim
// Log when we get the data for a file we requested..
//
// Revision 1.49  2003/04/17 23:08:57  jim
// Cleanup better on Logout
//
// Revision 1.48  2003/04/17 22:09:37  jim
// Set monitors on the account more correctly.
//
// Revision 1.47  2003/04/17 22:08:21  jim
// ...
//
// Revision 1.46  2003/04/17 19:23:27  jim
// Windows build compatibility.  Should build stat requests to result in deletions.
//
// Revision 1.45  2003/04/17 18:38:59  jim
// CHeck directory even before lowering the case
//
// Revision 1.44  2003/04/17 18:38:10  jim
// Fix handling 'what' responce.  Validate directory before opening a file
//
// Revision 1.43  2003/04/17 17:21:28  jim
// Oops - variable fumble
//
// Revision 1.42  2003/04/17 17:02:51  jim
// Oops strncmp not strcmp
//
// Revision 1.41  2003/04/16 23:52:56  jim
// Added docs.  Added forcecase option
//
// Revision 1.40  2003/04/15 21:48:57  jim
// Standardize to non-standard linux/fcntl.h
//
// Revision 1.39  2003/04/08 16:55:06  jim
// Handle options better.
//
// Revision 1.38  2003/04/08 16:17:30  jim
// Add first command pararmeter as configname
//
// Revision 1.37  2003/04/07 22:52:50  jim
// Updates to linuxfiles, makefile typo fixed
//
// Revision 1.36  2003/04/07 18:50:48  jim
// Enabled more program level logging by default
//
// Revision 1.35  2003/04/07 17:35:05  jim
// Comment out noisy logging
//
// Revision 1.34  2003/04/04 23:27:13  jim
// Use correctly sized buffer for reading...
//
// Revision 1.33  2003/04/04 23:18:11  jim
// Cleaning up logging - but it looks like all is a go.
//
// Revision 1.32  2003/04/04 23:01:17  jim
// Fix when to queue the monitor to the account - otherwise we wait too long.
//
// Revision 1.31  2003/04/04 22:26:10  jim
// Dang typos..
//
// Revision 1.30  2003/04/04 22:25:07  jim
// Used wrong name on outgoing CRC construction.
//
// Revision 1.29  2003/04/04 19:20:30  jim
// Changes to monitor CRC blocks of the file.
//
// Revision 1.28  2003/04/04 00:18:57  jim
// Make sure logging statements after elses are wrapped in {}
//
// Revision 1.27  2003/04/03 23:57:08  jim
// Forced logging... cleaned up opening windows monitor.  fixed KILL message
//
// Revision 1.26  2003/04/03 22:03:07  jim
// Call logout better...
//
// Revision 1.25  2003/04/03 21:53:54  jim
// Supply a logout function to cleanup accounts
//
// Revision 1.24  2003/04/03 21:00:31  jim
// Do file monitor opens in login
//
// Revision 1.23  2003/04/03 20:06:11  jim
// Removed many loggings - shortened timeout - soon to be config param.
//
// Revision 1.22  2003/04/03 19:51:28  jim
// Move logging a bit - check the stat of files, yeah?
//
// Revision 1.21  2003/04/03 18:53:44  jim
// Added much loggings
//
// Revision 1.20  2003/04/03 16:56:00  jim
// Log pathname instead of path ID
//
// Revision 1.19  2003/04/03 16:49:34  jim
// Minor typos...
//
// Revision 1.18  2003/04/03 16:47:18  jim
// Go through account layer to get change updates.  Pass account when creating a file monitor
//
// Revision 1.17  2003/04/03 01:33:09  jim
// Add support option to ignore getting winners from local parser
//
// Revision 1.16  2003/04/03 01:19:38  jim
// Oops forgot to copy the name to the new buffer
//
// Revision 1.15  2003/04/02 23:30:23  jim
// Disable splashman, watchdog, broadcast interfaces optionally
//
// Revision 1.14  2003/04/02 23:21:48  jim
// Should now handle filemasks, and multiple incoming/outgiong directories
//
// Revision 1.13  2003/02/19 00:24:11  jim
// Think somewhere I lost some revisions on linux make... Compatibility with new make system portings
//
// Revision 1.12  2002/08/07 14:48:58  panther
// Modified logging in account.c
//
// Revision 1.11  2002/08/07 14:47:38  panther
// Adding locking - especially important with windows.
// Also release the allocated read buffer.
//
// Revision 1.10  2002/07/17 17:26:53  panther
// Fixing tracking of master caller statuses...
//
// Revision 1.9  2002/07/01 14:51:43  panther
// Added support for multiple 'common' directories
// added support for winner information port
//
// Revision 1.8  2002/06/25 18:23:30  panther
// Updated to switch to listening addresses per account, for purposes of
// multiple callers transmitting simultaneously.
//
// Revision 1.7  2002/06/19 15:54:09  panther
// Updates to listen to seperate callers for all clients connected.
//
// Revision 1.6  2002/04/15 22:12:51  panther
// Updated Accounts.Data
// remove Author tag... log is sufficent.
//
// Revision 1.5  2002/04/15 22:08:55  panther
// Added additional local-only mirror ability for accounts.
// This allows directed updates to be performed...
//
// Revision 1.4  2002/04/15 16:25:03  panther
// Added Revision Tags...
//
