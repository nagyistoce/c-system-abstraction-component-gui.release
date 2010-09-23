;
#include <sack_types.h>

typedef struct global_tag
{
	struct {
		_32 bVerbose : 1;
	} flags;
	char SystemRoot[256];
	_32 copied;
   PLIST excludes;
} GLOBAL;



typedef struct file_source
{
	struct
	{
		_32 bScanned : 1;
		_32 bInvalid : 1;
		_32 bSystem : 1;
		_32 bExternal : 1;
	} flags;
	char *name;
   struct file_source *children, *next;
} FILESOURCE, *PFILESOURCE;

void AddFileCopy( CTEXTSTR name );

int ScanFile( PFILESOURCE pfs );
PFILESOURCE AddDependCopy( PFILESOURCE pfs, CTEXTSTR name );

void ScanFileCopyTree( void );
void CopyFileCopyTree( CTEXTSTR dest );

