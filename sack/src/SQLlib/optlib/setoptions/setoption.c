

#include <sqlgetoption.h>

int main( int argc, char **argv )
{
	if( argc < 4 )
	{
		printf( "Usage: %s [path] [option] [value]\n", argv[0] );
		printf( "   path = \"/flashdrive.ini/eltanin receiver\"\n" );
		printf( "   option = \"Bonanza Enable\"\n" );
		printf( "   value = 0/1\n" );
      printf( "  %s \"/flashdrive.ini/eltanin receiver\" \"Bonanza Enable\" 1\n", argv[0] );
      return 0;
	}
   SACK_WriteProfileString( argv[1], argv[2], argv[3] );
   return 0;
}
