

#include <sqlgetoption.h>

int main( int argc, char **argv )
{
	if( argc < 5 )
	{
		printf( "Usage: %s [root_name] [path] [option] [value]\n", argv[0] );
		printf( "   root_name = \"flashdrive.ini\" (if \"\" then option will go under DEFAULT \n" );
		printf( "   path = \"eltanin receiver\"\n" );
		printf( "   option = \"Bonanza Enable\"\n" );
		printf( "   value = 0/1\n" );
      printf( "  %s \"/flashdrive.ini/eltanin receiver\" \"Bonanza Enable\" 1\n", argv[0] );
      return 0;
	}
   if( argv[1][0] )
		SACK_WritePrivateProfileString( argv[2], argv[3], argv[4], argv[1] );
   else
		SACK_WriteProfileString( argv[2], argv[3], argv[4] );
   return 0;
}
