
#include <stdhdrs.h>
#include <filesys.h>

//#define DEBUG_FILEOPEN

#ifndef UNDER_CE
//#include <fcntl.h>
//#include <io.h>
#endif

FILESYS_NAMESPACE

struct file{
	TEXTSTR name;
	TEXTSTR fullname;
	int fullname_size;

	PLIST handles; // HANDLE 's
   PLIST files; // FILE *'s
};

struct Group {
	TEXTSTR name;
	TEXTSTR base_path;
};

PLIST files;
PLIST groups;


static struct Group *GetGroupFilePath( CTEXTSTR group )
{
	struct Group *filegroup;
   INDEX idx;
	LIST_FORALL( groups, idx, struct Group *, filegroup )
	{
		if( StrCaseCmp( filegroup->name, group ) == 0 )
		{
         break;
		}
	}
   return filegroup;
}

FILESYS_PROC( int, GetFileGroup )( CTEXTSTR groupname, CTEXTSTR default_path )
{
	struct Group *filegroup = GetGroupFilePath( groupname );
	if( !filegroup )
	{
		filegroup = New( struct Group );
		filegroup->name = StrDup( groupname );
		filegroup->base_path = StrDup( default_path );
		AddLink( &groups, filegroup );
	}
   return FindLink( &groups, filegroup );
}


FILESYS_PROC( int, SetGroupFilePath )( CTEXTSTR group, CTEXTSTR path )
{
	struct Group *filegroup = GetGroupFilePath( group );
	if( !filegroup )
	{
		filegroup = New( struct Group );
		filegroup->name = StrDup( group );
		filegroup->base_path = StrDup( path );
      AddLink( &groups, filegroup );
	}
	else
	{
		Release( (POINTER)filegroup->base_path );
		filegroup->base_path = StrDup( path );
	}
   return FindLink( &groups, filegroup );
}


void SetDefaultFilePath( CTEXTSTR path )
{
	struct Group *filegroup;
	filegroup = (struct Group *)GetLink( &groups, 0 );
	if( groups && filegroup )
	{
		Release( (POINTER)filegroup->base_path );
		filegroup->base_path = StrDup( path );
	}
	else
	{
		SetGroupFilePath( WIDE( "Default" ), path );
	}
}

static TEXTSTR PrependBasePath( int groupid, struct Group *group, CTEXTSTR filename )
{
	TEXTSTR fullname;
	if( !groups )
	{
#ifdef UNDER_CE
		SetDefaultFilePath( GetProgramPath() );
#else
		SetDefaultFilePath( OSALOT_GetEnvironmentVariable( "MY_WORK_PATH" ) );
#endif

		if( !groupid )
		{
         	group = (struct Group *)GetLink( &groups, groupid );
		}
	}
	if( !group || filename[0] == '/' || filename[0] == '\\' || filename[1] == ':' )
      return StrDup( filename );
	{
		int len;
		fullname = NewArray( TEXTCHAR, len = StrLen( filename ) + StrLen(group->base_path) + 2 );
		snprintf( fullname, len * sizeof( TEXTCHAR ), WIDE("%s/%s"), group->base_path, filename );
	}
	return fullname;
}

TEXTSTR sack_prepend_path( int group, CTEXTSTR filename )
{
	struct Group *filegroup = (struct Group *)GetLink( &groups, group );
	TEXTSTR result = PrependBasePath( group, filegroup, filename );
	return result;
}

int sack_open( int group, CTEXTSTR filename, int opts, ... )
{
	HANDLE handle;
	struct file *file;
   INDEX idx;
	LIST_FORALL( files, idx, struct file *, file )
	{
		if( StrCmp( file->name, filename ) == 0 )
		{
         break;
		}
	}
	if( !file )
	{
		struct Group *filegroup = (struct Group *)GetLink( &groups, group );
		file = New( struct file );
		file->name = StrDup( filename );
      file->fullname = PrependBasePath( group, filegroup, filename );
		file->handles = NULL;
      file->files = NULL;
		AddLink( &files,file );
	}
	switch( opts & 3 )
	{
	case 0:

	handle = CreateFile( file->fullname
							 , GENERIC_READ
							 , FILE_SHARE_READ|FILE_SHARE_WRITE
							 , NULL
							 ,OPEN_EXISTING | ((opts&O_CREAT)?CREATE_ALWAYS:0)
							 , FILE_ATTRIBUTE_NORMAL
							 , NULL );
	break;
	case 1:
	handle = CreateFile( file->fullname
							 , GENERIC_WRITE
							 , FILE_SHARE_READ|FILE_SHARE_WRITE
							 , NULL
							 ,OPEN_EXISTING | ((opts&O_CREAT)?CREATE_ALWAYS:0)
							 , FILE_ATTRIBUTE_NORMAL
							 , NULL );
		break;
	case 2:
	handle = CreateFile( file->fullname
							 ,(GENERIC_READ|GENERIC_WRITE)
							 , FILE_SHARE_READ|FILE_SHARE_WRITE
							 , NULL
							 ,OPEN_EXISTING | ((opts&O_CREAT)?CREATE_ALWAYS:0)
							 , FILE_ATTRIBUTE_NORMAL
							 , NULL );
		break;
	}
#ifdef DEBUG_FILEOPEN
	lprintf( WIDE( "open %s %p %d" ), file->fullname, handle, opts );
#endif
	if( handle == INVALID_HANDLE_VALUE )
	{
#ifdef DEBUG_FILEOPEN
		lprintf( WIDE( "Failed to open file [%s]=[%s]" ), file->name, file->fullname );
#endif
		return -1;
	}
	AddLink( &file->handles, handle );
	return (int)handle;
}


struct file *FindFileByHandle( HANDLE file_file )
{
	struct file *file;
   INDEX idx;
	LIST_FORALL( files, idx, struct file *, file )
	{
		INDEX idx2;
      HANDLE check;
		LIST_FORALL( file->handles, idx2, HANDLE, check )
		{
			if( check == file_file )
				break;
		}
		if( check )
         break;
	}
   return file;
}


int sack_creat( int group, CTEXTSTR file, int opts, ... )
{
   return sack_open( group, file, opts | O_CREAT );
}

int sack_close( HANDLE file_handle )
{

	struct file *file = FindFileByHandle( (HANDLE)file_handle );
	DeleteLink( &file->handles, (POINTER)file_handle );
#ifdef DEBUG_FILEOPEN
	lprintf( WIDE("Close %s"), file->fullname );
#endif
   /*
	Release( file->name );
	Release( file->fullname );
	Release( file );
	DeleteLink( &files, file );
   */
	return CloseHandle((HANDLE)file_handle);
}



int sack_lseek( HANDLE file_handle, int pos, int whence )
{
	return SetFilePointer((HANDLE)file_handle,pos,NULL,whence);
}

int sack_read( HANDLE file_handle, POINTER buffer, int size )
{
   DWORD dwLastReadResult;
   return (ReadFile( (HANDLE)file_handle, buffer, size, &dwLastReadResult, NULL )?dwLastReadResult:-1 );
}

int sack_write( HANDLE file_handle, POINTER buffer, int size )
{
   DWORD dwLastWrittenResult;
	return (WriteFile( (HANDLE)file_handle, buffer, size, &dwLastWrittenResult, NULL )?dwLastWrittenResult:-1 );
}

int sack_unlink( CTEXTSTR filename )
{
   int okay;
   struct Group *filegroup = (struct Group *)GetLink( &groups, 0 );
   TEXTSTR tmp = PrependBasePath( 0, filegroup, filename );
	okay = DeleteFile(tmp);
	Release( tmp );
   return !okay; // unlink returns TRUE is 0, else error...
}

struct file *FindFileByFILE( FILE *file_file )
{
	struct file *file;
   INDEX idx;
	LIST_FORALL( files, idx, struct file *, file )
	{
		INDEX idx2;
      FILE *check;
		LIST_FORALL( file->files, idx2, FILE *, check )
		{
			if( check == file_file )
				break;
		}
		if( check )
         break;
	}
   return file;
}

FILESYS_PROC( FILE*, sack_fopen )( int group, CTEXTSTR filename, CTEXTSTR opts )
{
	FILE *handle;
	struct file *file;
   INDEX idx;
	LIST_FORALL( files, idx, struct file *, file )
	{
		if( StrCmp( file->name, filename ) == 0 )
		{
         break;
		}
	}
	if( !file )
	{
		struct Group *filegroup = (struct Group *)GetLink( &groups, group );
		file = New( struct file );
		file->handles = NULL;
      	file->files = NULL;
		file->name = StrDup( filename );
		file->fullname = PrependBasePath( group, filegroup, filename );
		AddLink( &files,file );
	}

#ifdef UNICODE
	handle = _wfopen( file->fullname, opts );
#else
#ifdef _STRSAFE_H_INCLUDED_
	fopen_s( &handle, file->fullname, opts );
#else
	handle = fopen( file->fullname, opts );
#endif
#endif
	if( !handle )
	{
#ifdef DEBUG_FILEOPEN
		lprintf( WIDE( "Failed to open file [%s]=[%s]" ), file->name, file->fullname );
#endif
		return NULL;
	}
#ifdef DEBUG_FILEOPEN
	lprintf( WIDE( "Fopen %s" ), file->fullname );
#endif
	AddLink( &file->files, handle );
	return handle;
}
FILESYS_PROC( int, sack_fseek )( FILE *file_file, int pos, int whence )
{
	//struct file *file = FindFileByFILE( file_file );
   return fseek( file_file, pos, whence );
}
FILESYS_PROC( int, sack_fclose )( FILE *file_file )
{
	struct file *file;
	file = FindFileByFILE( file_file );
#ifdef DEBUG_FILEOPEN
	lprintf( WIDE("Closing %s"), file->fullname );
#endif
	DeleteLink( &file->files, file_file );
   /*
	Release( file->name );
   Release( file->fullname );
	Release( file );
	DeleteLink( &files, file );
   */
   return fclose( file_file );
}
FILESYS_PROC( int, sack_fread )( POINTER buffer, int size, int count,FILE *file_file )
{
   return fread( buffer, size, count, file_file );
}
FILESYS_PROC( int, sack_fwrite )( POINTER buffer, int size, int count,FILE *file_file )
{
   return fwrite( buffer, size, count, file_file );
}


FILESYS_PROC( int, sack_rename )( CTEXTSTR file_source, CTEXTSTR new_name )
{
	return MoveFile( file_source, new_name );
}


_32 GetSizeofFile( TEXTCHAR *name, P_32 unused )
{
	_32 size;
#ifdef __LINUX__
	int hFile = open( name,           // open MYFILE.TXT
							  O_RDONLY );              // open for reading
	if( hFile >= 0 )
	{
		size = lseek( hFile, 0, SEEK_END );
		close( hFile );
		return size;
	}
	else
		return 0;
#else
   HANDLE hFile = CreateFile( name, 0, 0, NULL, OPEN_EXISTING, 0, NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		GetFileSize( hFile, &size );
		CloseHandle( hFile );
		return size;
	}
	else
		return 0;
#endif
}

//-------------------------------------------------------------------------

_32 GetFileTimeAndSize( CTEXTSTR name
							 , LPFILETIME lpCreationTime
							 ,  LPFILETIME lpLastAccessTime
							 ,  LPFILETIME lpLastWriteTime
							 , int *IsDirectory
							 )
{
	_32 size;
#ifdef __LINUX__
	int hFile = open( name,           // open MYFILE.TXT
							  O_RDONLY );              // open for reading
	if( hFile >= 0 )
	{
		size = lseek( hFile, 0, SEEK_END );
		close( hFile );
		return size;
	}
	else
		return (_32)-1;
#else
   HANDLE hFile = CreateFile( name, 0, 0, NULL, OPEN_EXISTING, 0, NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		GetFileSize( hFile, &size );
		GetFileTime( hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime );
      if( IsDirectory )
		{
			_32 dwAttr = GetFileAttributes( name );
			if( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
				(*IsDirectory) = 1;
         else
				(*IsDirectory) = 0;
		}
		CloseHandle( hFile );
		return size;
	}
	else
		return (_32)-1;
#endif
}



FILESYS_NAMESPACE_END
