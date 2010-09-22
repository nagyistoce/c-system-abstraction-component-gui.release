#ifndef _ACCOUNT_STRUCTURE_DEFINED
#define _ACCOUNT_STRUCTURE_DEFINED

#include <filemon.h>

#define ACCOUNT_STRUCTURES_DEFINED

#ifndef O_BINARY
#define O_BINARY 0
#endif

typedef struct filechange_tag
{
	_32 start;
	_32 size;
} FILECHANGE, *PFILECHANGE;

typedef struct netbuffer_tag {
    SOCKADDR *sa;
    char *buffer;
    int size;
    int valid;   // if we ever got a real game state....
    int present;
	 _32 LastMessage;
    DeclareLink( struct netbuffer_tag );
} NETBUFFER, *PNETBUFFER;

typedef struct address_tag {
   _32 start; // non net order (host order)
   _32 end;   // host order max (inclusive)
   struct address_tag *next;
} ADDRESS,*PADDRESS;

typedef struct directory_tag {
	struct {
		_32 bIncoming : 1;
	} flags;
	char *name;  //optional name (to be done)
	_32 ID;  // optional ID (to be done)
   CTEXTSTR mask;
   TEXTCHAR path[];
} *PDIRECTORY, DIRECTORY;

typedef struct accounts_tag {
   char unique_name[128];
   int logincount;
	PADDRESS allowed;
	PLIST Monitors; // one monitor for each in/out directory.
	PLIST Directories;
   _16 WinnerPort;
   PNETBUFFER netbuffer;
   // in case someone else has updated,
   // just assume that we do need to send also without
   // recomputing differential.
   int lastupdate;
   int file;
   char *buffer;
   _32 SendResponce;
   _32 NextResponce;
	_32 WhatResponce;
   char LastFile[320];
   PCLIENT pc;
   PLINKQUEUE segments;
   _32 DoScanTime;
	_32 timer;
   DeclareLink( struct accounts_tag );
} ACCOUNT, *PACCOUNT;

typedef struct monitored_directory_tag
{
	struct {
		_32 bIncoming : 1;
	} flags;
   INDEX PathID;
	PACCOUNT account;
   PCHANGEHANDLER pHandler;
	PMONITOR monitor;
   PDIRECTORY pDirectory;
} MONDIR, *PMONDIR;

#endif
