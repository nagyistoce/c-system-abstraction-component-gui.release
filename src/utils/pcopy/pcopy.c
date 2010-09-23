//#define DEBUG_DISCOVERY
//#define BYTE unsigned char
//#define WORD unsigned short
//#define LONG signed long
//#define DWORD unsigned long
#include <stdhdrs.h>
#include <stdio.h>
#include <sharemem.h>
#include <filesys.h>
#include "pcopy.h"

GLOBAL g;

#define MY_IMAGE_DOS_SIGNATURE 0x5A4D

typedef struct MY_IMAGE_DOS_HEADER {
	WORD e_magic;
	WORD e_cblp;
	WORD e_cp;
	WORD e_crlc;
	WORD e_cparhdr;
	WORD e_minalloc;
	WORD e_maxalloc;
	WORD e_ss;
	WORD e_sp;
	WORD e_csum;
	WORD e_ip;
	WORD e_cs;
	WORD e_lfarlc;
	WORD e_ovno;
	WORD e_res[4];
	WORD e_oemid;
	WORD e_oeminfo;
	WORD e_res2[10];
	LONG e_lfanew;
} MY_IMAGE_DOS_HEADER,*PMY_IMAGE_DOS_HEADER;

#define MY_IMAGE_NT_SIGNATURE 0x00004550

typedef struct MY_IMAGE_FILE_HEADER {
	WORD Machine;
	WORD NumberOfSections;
	DWORD TimeDateStamp;
	DWORD PointerToSymbolTable;
	DWORD NumberOfSymbols;
	WORD SizeOfOptionalHeader;
	WORD Characteristics;
} MY_IMAGE_FILE_HEADER, *PMY_IMAGE_FILE_HEADER;

#define MY_IMAGE_SIZEOF_SHORT_NAME 8

typedef struct MY_IMAGE_SECTION_HEADER {
	BYTE Name[MY_IMAGE_SIZEOF_SHORT_NAME];
	union {
		DWORD PhysicalAddress;
		DWORD VirtualSize;
	} Misc;
	DWORD VirtualAddress;
	DWORD SizeOfRawData;
	DWORD PointerToRawData;
	DWORD PointerToRelocations;
	DWORD PointerToLinenumbers;
	WORD NumberOfRelocations;
	WORD NumberOfLinenumbers;
	DWORD Characteristics;
} MY_IMAGE_SECTION_HEADER,*PMY_IMAGE_SECTION_HEADER;

typedef struct MY_IMAGE_IMPORT_BY_NAME {
	WORD Hint;
	BYTE Name[1];
} MY_IMAGE_IMPORT_BY_NAME,*PMY_IMAGE_IMPORT_BY_NAME;

#define DUMMYUNIONNAME
#define _ANONYMOUS_UNION
typedef struct MY_IMAGE_IMPORT_DESCRIPTOR {
	_ANONYMOUS_UNION union {
		DWORD Characteristics;
		DWORD OriginalFirstThunk;
	} DUMMYUNIONNAME;
	DWORD TimeDateStamp;
	DWORD ForwarderChain;
	DWORD Name;
	DWORD FirstThunk;
} MY_IMAGE_IMPORT_DESCRIPTOR,*PMY_IMAGE_IMPORT_DESCRIPTOR;

typedef struct MY_IMAGE_IMPORT_LOOKUP_TABLE {
	union {
		struct {
			DWORD NameOffset:31;
			DWORD NameIsString:1;
		};
		DWORD Name;
		WORD Id;
	};
} MY_IMAGE_IMPORT_LOOKUP_TABLE;

typedef struct MY_IMAGE_RESOURCE_DIRECTORY_ENTRY {
	union {
		struct {
			DWORD NameOffset:31;
			DWORD NameIsString:1;
		};
		DWORD Name;
		WORD Id;
	};
	union {
		DWORD OffsetToData;
		struct {
			DWORD OffsetToDirectory:31;
			DWORD DataIsDirectory:1;
		};
	};
} MY_IMAGE_RESOURCE_DIRECTORY_ENTRY,*PMY_IMAGE_RESOURCE_DIRECTORY_ENTRY;


typedef struct MY_IMAGE_RESOURCE_DIRECTORY {
	DWORD Characteristics;
	DWORD TimeDateStamp;
	WORD MajorVersion;
	WORD MinorVersion;
	WORD NumberOfNamedEntries;
	WORD NumberOfIdEntries;
   MY_IMAGE_RESOURCE_DIRECTORY_ENTRY entries[]; 
} MY_IMAGE_RESOURCE_DIRECTORY,*PMY_IMAGE_RESOURCE_DIRECTORY;


typedef struct MY_IMAGE_DATA_DIRECTORY {
	DWORD VirtualAddress;
	DWORD Size;
} MY_IMAGE_DATA_DIRECTORY,*PMY_IMAGE_DATA_DIRECTORY;

#define MY_IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct MY_IMAGE_OPTIONAL_HEADER {
	WORD Magic;
	BYTE MajorLinkerVersion;
	BYTE MinorLinkerVersion;
	DWORD SizeOfCode;
	DWORD SizeOfInitializedData;
	DWORD SizeOfUninitializedData;
	DWORD AddressOfEntryPoint;
	DWORD BaseOfCode;
	DWORD BaseOfData; /* not used in PE32+ */
	//------ preceed is 'standard fields'
	union {
		struct {
			DWORD ImageBase;
			DWORD SectionAlignment;
			DWORD FileAlignment;
			WORD MajorOperatingSystemVersion;
			WORD MinorOperatingSystemVersion;
			WORD MajorImageVersion;
			WORD MinorImageVersion;
			WORD MajorSubsystemVersion;
			WORD MinorSubsystemVersion;
			DWORD Reserved1;
			DWORD SizeOfImage;
			DWORD SizeOfHeaders;
			DWORD CheckSum;
			WORD Subsystem;
			WORD DllCharacteristics;
			DWORD SizeOfStackReserve;
			DWORD SizeOfStackCommit;
			DWORD SizeOfHeapReserve;
			DWORD SizeOfHeapCommit;
			DWORD LoaderFlags;
			DWORD NumberOfRvaAndSizes;
			MY_IMAGE_DATA_DIRECTORY DataDirectory[MY_IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
		} PE32;
      /*
		struct {
			_64 ImageBase;
			DWORD SectionAlignment;
			DWORD FileAlignment;
			WORD MajorOperatingSystemVersion;
			WORD MinorOperatingSystemVersion;
			WORD MajorImageVersion;
			WORD MinorImageVersion;
			WORD MajorSubsystemVersion;
			WORD MinorSubsystemVersion;
			DWORD Reserved1;
			DWORD SizeOfImage;
			DWORD SizeOfHeaders;
			DWORD CheckSum;
			// -- windows specific fieds preceed...
			WORD Subsystem;
			WORD DllCharacteristics;
			_64 SizeOfStackReserve;
			_64 SizeOfStackCommit;
			_64 SizeOfHeapReserve;
			_64 SizeOfHeapCommit;
			DWORD LoaderFlags;  // obsolete
			DWORD NumberOfRvaAndSizes;
			MY_IMAGE_DATA_DIRECTORY DataDirectory[MY_IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
		} PE32_plus;
      */
	};
} MY_IMAGE_OPTIONAL_HEADER,*PMY_IMAGE_OPTIONAL_HEADER;

typedef struct MY_IMAGE_NT_HEADERS {
	DWORD Signature;
	MY_IMAGE_FILE_HEADER FileHeader;
	//MY_IMAGE_OPTIONAL_HEADER OptionalHeader;
} MY_IMAGE_NT_HEADERS,*PMY_IMAGE_NT_HEADERS;

void FixResourceDirEntry( char *resources, PMY_IMAGE_RESOURCE_DIRECTORY pird )
{
	int x;
	pird->TimeDateStamp = 0;
	for( x = 0; x < pird->NumberOfNamedEntries + 
	                pird->NumberOfIdEntries; x++ )
	{
		//printf( WIDE("DirectoryEntry: %08x %08x\n"), pird->entries[x].Name, pird->entries[x].OffsetToData );
		if( pird->entries[x].DataIsDirectory )
		{
			FixResourceDirEntry( resources
									, (PMY_IMAGE_RESOURCE_DIRECTORY)(resources 
												+ pird->entries[x].OffsetToDirectory) );
		}
	}
}


int ScanFile( PFILESOURCE pfs )
{
	FILE *file;
//cpg27dec2006 c:\work\sack\src\utils\pcopy\pcopy.c(226): Warning! W202: Symbol 'n' has been defined, but not referenced
//cpg27dec2006 	int n;
	MY_IMAGE_DOS_HEADER dos_header;
	MY_IMAGE_NT_HEADERS nt_header;
	IMAGE_OPTIONAL_HEADER nt_optional_header;
	//printf("Attempt to scan: %s\n", pfs->name );
	{
		//INDEX idx;
		//CTEXTSTR exclude;
		//LIST_FORALL( g.excludes, idx, CTEXTSTR, exclude )
		{
			/*
			if( stristr( pfs->name, exclude ) )
			{
				return 0;
				}
            */
		}
	}
	{
		if( strncmp( pfs->name, g.SystemRoot, strlen( g.SystemRoot ) ) == 0 )
		{
         pfs->flags.bScanned = 1;
         pfs->flags.bSystem = 1;
			return 0;
		}
		file = sack_fopen( 0, pfs->name, WIDE("rb") );
		if( file )
		{
			pfs->flags.bScanned = 1;
			fread( &dos_header, 1, sizeof( dos_header ), file );
			if( dos_header.e_magic != MY_IMAGE_DOS_SIGNATURE )
			{
				//fprintf( stderr, WIDE("warning: \'%s\' is not a program.\n"), pfs->name );
            // copy it anyway.
				fclose( file );
				return 0;
			}
			fseek( file, dos_header.e_lfanew, SEEK_SET );
			fread( &nt_header, sizeof( nt_header ), 1, file );
			if( nt_header.Signature != MY_IMAGE_NT_SIGNATURE )
			{
				fprintf( stderr, WIDE("warning: \'%s\' is not a 32 bit windows program.\n"), pfs->name );
				fclose( file );
				return 1;
			}
			fread( &nt_optional_header, sizeof( nt_optional_header ), 1, file );

			//fseek( file, nt_optional_header.PE32.DataDirectory[1].VirtualAddress, SEEK_SET );
			{
            //fread( &
			}


			// track down and kill resources.
			{
				int n;
				long FPISections = dos_header.e_lfanew 
				                 + sizeof( nt_header ) 
				                 + nt_header.FileHeader.SizeOfOptionalHeader;
				MY_IMAGE_SECTION_HEADER section;
				//fseek( file, FPISections, SEEK_SET );
				for( n = 0; n < nt_header.FileHeader.NumberOfSections; n++ )
				{
               fseek( file, FPISections + n * sizeof( section ), SEEK_SET );
					fread( &section, 1, sizeof( section ), file );
					if( strcmp( (CTEXTSTR)section.Name, WIDE(".rsrc") ) == 0 )
					{
						//MY_IMAGE_RESOURCE_DIRECTORY *ird;
						// Resources begin here....
						//char *data;
						//data = malloc( section.SizeOfRawData );
						//fseek( file, section.PointerToRawData, SEEK_SET );
						//fread( data, 1, section.SizeOfRawData, file );
						//FixResourceDirEntry( data, (MY_IMAGE_RESOURCE_DIRECTORY *)data );
						//fseek( file, section.PointerToRawData, SEEK_SET );
						//fwrite( data, 1, section.SizeOfRawData, file );
						//free( data );
					}
					else if( strcmp( (CTEXTSTR)section.Name, WIDE(".idata") ) == 0 )
					{
                  char *data;
						MY_IMAGE_IMPORT_DESCRIPTOR *iid;
                  int m;
//cpg27dec 2006 c:\work\sack\src\utils\pcopy\pcopy.c(295): Warning! W202: Symbol 'iilt' has been defined, but not referenced
//cpg27dec 2006 c:\work\sack\src\utils\pcopy\pcopy.c(226): Warning! W202: Symbol 'n' has been defined, but not referenced
//cpg27dec 2006 						int n;
						data = (char*)malloc( section.SizeOfRawData );
						fseek( file, section.PointerToRawData, SEEK_SET );
						fread( data, 1, section.SizeOfRawData, file );

						//FixResourceDirEntry( data, (MY_IMAGE_RESOURCE_DIRECTORY *)data );
						//fseek( file, section.PointerToRawData, SEEK_SET );
						//fwrite( data, 1, section.SizeOfRawData, file );

						//printf( WIDE("Import data at %08x\n"), section.VirtualAddress );

						for( m = 0; ( iid = (MY_IMAGE_IMPORT_DESCRIPTOR*)(data + sizeof( *iid ) * m) )
							 , ( iid->Characteristics || iid->TimeDateStamp || iid->Name ||
										 iid->FirstThunk || iid->ForwarderChain )
							 ; m++ )
						{
							AddDependCopy( pfs, data+( iid->Name - section.VirtualAddress ) );
#ifdef DEBUG_DISCOVERY
							printf( WIDE("%s %08x %08x %08x\n")
									, data +( iid->Name - section.VirtualAddress )
									 , ( iid->Name - section.VirtualAddress )
									, iid->Characteristics
									, iid->FirstThunk );
#endif
						}

#if 0
						{
							MY_IMAGE_IMPORT_LOOKUP_TABLE iilt;
							do
							{
								printf( WIDE("Reading %d\n"), sizeof( iilt ) );
								fread( &iilt, sizeof( iilt ), 1, file );
								if( !iilt.NameIsString )
								{
									printf( WIDE("Name at %08x\n"), iilt.Name - section.VirtualAddress + section.PointerToRawData );
								}
								else
									printf( WIDE("Oridinal %d\n"), iilt.Id );
							} while( iilt.Name );

							do
							{
								printf( WIDE("Reading %d\n"), sizeof( iilt ) );
								fread( &iilt, sizeof( iilt ), 1, file );
								if( !iilt.NameIsString )
								{
									printf( WIDE("Name at %08x\n"), iilt.Name - section.VirtualAddress + section.PointerToRawData );
								}
								else
									printf( WIDE("Oridinal %d\n"), iilt.Id );
							} while( iilt.Name );
						}
#endif
						//free( data );

					}
               //else
					//	printf( WIDE("%*.*s\n")
					//			, MY_IMAGE_SIZEOF_SHORT_NAME
					//			, MY_IMAGE_SIZEOF_SHORT_NAME
					//			, section.Name );
					
				}
			}


			fclose( file );
		}
		else
		{
#ifdef __WINDOWS__
			HMODULE hModule;
			static TEXTCHAR name[256];

			hModule = LoadLibrary( pfs->name );
			if( hModule )
			{
				GetModuleFileName( hModule, name, sizeof( name ) );
				//printf( "Real name is %s\n", name );
            //fflush( stdout );
            AddDependCopy( pfs, name )->flags.bExternal = 1;
				FreeLibrary( hModule );
            return 0;
			}
			else
			{
				TEXTSTR path = (TEXTSTR)OSALOT_GetEnvironmentVariable( "PATH" );
				TEXTSTR tmp;
				static TEXTCHAR tmpfile[256 + 64];
				while( tmp = strchr( path, ';' ) )
				{
					tmp[0] = 0;
					snprintf( tmpfile, sizeof( tmpfile ), "%s/%s", path, pfs->name );
					//printf( "attmpt %s\n", tmpfile );

					hModule = LoadLibrary( tmpfile );
					if( hModule )
					{
						GetModuleFileName( hModule, name, sizeof( name ) );
						//printf( "*Real name is %s\n", name );
						//fflush( stdout );
						AddDependCopy( pfs, name )->flags.bExternal = 1;
						FreeLibrary( hModule );
						return 0;
					}
					else
					{
						FILE *file = sack_fopen( 0, tmpfile, "rb" );
						if( file )
						{
							AddDependCopy( pfs, tmpfile )->flags.bExternal = 1;
							fclose( file );
							return 0;
						}
					}
					path = tmp+1;
				}

			}
#endif
			pfs->flags.bInvalid = 1;
			if( pfs->flags.bInvalid )
				fprintf( stderr, WIDE("Failed to open %s\n"), pfs->name );
			return 1;
		}
	}
	return 0;
}


int main( int argc, CTEXTSTR *argv )
{
	if( argc < 3 || !IsPath( argv[argc-1] ) )
	{
		printf( WIDE("usage: pcopy <file...> <destination>\n")
				 "  file - .dll or .exe referenced, all referenced DLLs\n"
				 "         are also copied to the dstination\n"
				 "  dest - directory name to cpoy to, will fail otherwise.\n"
				);
		if( argc >= 3 )
			printf( WIDE("EROR: Final argument is not a directory\n") );
		return 1;
	}
	StrCpyEx( g.SystemRoot, getenv( WIDE("SystemRoot") ), sizeof( g.SystemRoot ) );
	{
		int c;
		for( c = 1; c < argc-1; c++ )
		{
			if( argv[c][0] == '-' )
			{
            int done = 0;
            int ch = 1;
				while( argv[c][ch] && !done )
				{
					switch(argv[c][ch])
					{
					case 'v':
					case 'V':
						g.flags.bVerbose = 1;
                  break;
					case 'x':
					case 'X':
						if( argv[c][ch+1] )
						{
							AddLink( &g.excludes, StrDup( argv[c] + ch + 1 ) );
							done = 1; // skip remaining characters in parameter
						}
						else
						{
							if( ( c + 1 ) < (argc-1) )
							{
								AddLink( &g.excludes, StrDup( argv[c+1] ) );
								c++; // skip one word...
								done = 1; // skip remining characters, go to next parameter (c++)
							}
							else
							{
								fprintf( stderr, "-x parameter specified without a name to exclude... invalid paramters..." );
								exit(1);
							}
						}
						break;
					}
					ch++;
				}
			}
			else
				AddFileCopy( argv[c ]);
		}
	}
	CopyFileCopyTree( argv[argc-1] );
	printf( WIDE("Copied %d file%s\n"), g.copied, g.copied==1?"":"s" );
	return 0;

}
