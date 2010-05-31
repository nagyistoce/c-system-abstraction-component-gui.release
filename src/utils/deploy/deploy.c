
#include <stdio.h>
#include <string.h>

int main( int argc, char **argv )
{
	char *path = strdup( argv[0] );
	FILE *out;
   char tmp[256];
	char *last1 = strrchr( path, '/' );
	char *last2 = strrchr( path, '\\' );
   char *last;
	if( last1 )
		if( last2 )
			if( last1 > last2 )
				last = last1;
			else
				last = last2;
		else
			last = last1;
	else
		if( last2 )
			last = last2;
		else
			last = NULL;
	if( last )
		last[0] = 0;
	else
      path = ".";
   snprintf( tmp, sizeof( tmp ), "%s/CMakePackage", path );
	out = fopen( "CMakePackage", "wt" );
	if( out )
	{
		int c;
		for( c = 0; path[c]; c++ )
         if( path[c] == '\\' ) path[c] = '/';
      fprintf( out, "set( SACK_BASE %s )\n", path );
      fprintf( out, "set( SACK_INCLUDE_DIR ${SACK_BASE}/include/sack )\n" );
      fprintf( out, "set( SACK_LIBRARIES sack_bag sack_bag++ )\n" );
      fprintf( out, "set( SACK_LIBRARY_DIR ${SACK_BASE}/lib )\n" );
      fprintf( out, "\n" );
      fprintf( out, "if( NOT C++ )\n" );
      fprintf( out, "  if( ${CMAKE_COMPILER_IS_GNUCC} )\n" );
      fprintf( out, "  SET( FIRST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/sack/deadstart_list.c )\n" );
      fprintf( out, "  SET( FIRST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/sack/deadstart_list.c )\n" );
      fprintf( out, "  SET( LAST_GCC_LIBRARY_SOURCE ${SACK_BASE}/src/sack/deadstart_lib.c ${SACK_BASE}/src/sack/deadstart_end.c )\n" );
      fprintf( out, "  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/sack/deadstart_lib.c ${SACK_BASE}/src/sack/deadstart_prog.c ${SACK_BASE}/src/sack/deadstart_end.c )\n" );
      fprintf( out, "  endif()\n" );
      fprintf( out, "  if( ${MSVC}${WATCOM} )\n" );
      fprintf( out, "  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/sack/deadstart_prog.c )\n" );
      fprintf( out, "  endif()\n" );
      fprintf( out, "else()\n" );
      fprintf( out, "  if( ${CMAKE_COMPILER_IS_GNUCC} )\n" );
      fprintf( out, "  # gcc I still need this, otherwise a program needs to InvokeDeadstarts()\n" );
      fprintf( out, "  SET( LAST_GCC_PROGRAM_SOURCE ${SACK_BASE}/src/sack/deadstart_prog.c )\n" );
      fprintf( out, "  endif()\n" );
      fprintf( out, "endif()\n" );
      fclose( out );
	}
   return 0;
}
