#include <stdio.h>
#include <futgetpr.h>
#include <getoption.h>

int main( int argc, char **argv )
{
	POINTER buffer;
	_32 buflen;
   const char *filename;
	fnSQLGetProfileBlob( WIDE("intershell/configuration"), filename = (argc<2?"issue_pos.config":argv[1] ), &buffer, &buflen );
	//fnSQLGetProfileBlob( WIDE("issue_pos/configuration"), "issue_pos.config", &buffer, &buflen );
	{
		FILE *out = fopen( filename, "wb" );
		fwrite( buffer, 1, buflen, out );
		fclose( out );
	}
   return 0;
}