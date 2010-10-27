
#include <sharemem.h>
#include <filesys.h>
#include <pssql.h>
#include <sqlgetoption.h>

int main( int argc, char **argv )
{
   int saveas = 0;
	if( argc < 2 )
	{
		printf( "Usage: %s <config filename> [optional - set as config filename]\n", argv[0] );
      return 1;

	}
	if( argc > 2 )
		saveas = 1;

	{
      TEXTSTR tmp = (TEXTSTR)StrDup( argv[1] );
		TEXTSTR extend = (TEXTSTR)strrchr( tmp, '.' );
		TEXTSTR filename = (TEXTSTR)pathrchr( tmp );
      if( extend )
			extend[0] = 0;
		if( !filename )
         filename = tmp;


		{
			_32 size = 0;
			POINTER mem = OpenSpace( NULL, argv[1], &size );
			if( mem && size )
			{
#ifndef __NO_SQL__
				SACK_WriteProfileBlob( "intershell/configuration", saveas?argv[2]:filename, (char*)mem, size );
#endif
				CloseSpace( mem );
			}
		}

	}
   return 1;
}
