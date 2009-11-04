
#include <configscript.h>
#include <../../include/banner.h>

static struct {
	CDATA back_color;
	CDATA text_color;
	CTEXTSTR file;
	TEXTSTR text;
	PTRSZVAL text_size;
	int lines;
	int columns;
	int display;
	struct flags_tag
	{
		BIT_FIELD bYesNo : 1;
		BIT_FIELD bOkayCancel : 1;
		BIT_FIELD bTop : 1;
	} flags;
} l;

int HandleArgs( int argc, char **argv )
{

   PVARTEXT pvt = NULL;
	int arg = 1;
	if( argc == 1 )
	{
		MessageBox( NULL, "Available Arguments\n"
					  " -back $aarrggbb  : set background color in hex, alpha, red, green, blue [default black]\n"
					  " -text $aarrggbb  : set text color in hex, alpha, red, green, blue [default white]\n"
					  " -lines N  : Set the target number of lines (font height, bigger number is smaller) [default 20]\n"
					  " -cols N  : Set the target number of columns (font width, bigger number is smaller) [default 30]\n"
					  " -file <filename>  : Use contents fo the file specified for text\n"
					  " -display N : specify which display to show on\n"
					  " -notop : shows the banner as not topmost (default is top).\n"
					  " -yesno : shows the Yes/No buttons, and returns error level 0 for yes and 1 for no.\n"
					  " -okcancel : shows the Okay/Cancel buttons, and returns error level 0 for yes and 1 for no.\n"
					  " -yesno and -okaycancel : returns error level 0 for yes and 1 for no, 2 for cancel, 3 for ok.\n"
					  " \n"
					  " any other unknown argument or other word will show as text.\n"
					  " banner_command -back $20800010 -text $FF70A080 Show \"This Text On\" Banner\n"
                 "   - the prior command will show 3 lines 'show', 'this text on', 'banner'"
					  " banner_command Show \"\\\"This Text On\\\"\" Banner\n"
                 "   - the prior command shows how to include showing quotes"
					 , "Usage"
					 , MB_OK );
      return 0;
	}
   // default to topmost.
	l.flags.bTop = 1;

	while( arg < argc )
	{
		if( argv[arg][0]=='-' )
		{
			if( StrCaseCmp( argv[arg]+1, "back" ) == 0 )
			{
				// blah
				arg++;
				if( arg < argc )
				{
               PTEXT tmp2;
					PTEXT tmp = SegCreateFromText( argv[arg] );
               tmp2 = tmp;
					GetColorVar( &tmp, &l.back_color );
					LineRelease( tmp2 );
				}
			}
			else if( StrCaseCmp( argv[arg]+1, "text" ) == 0 )
			{
				arg++;
				if( arg < argc )
				{
               PTEXT tmp2;
					PTEXT tmp = SegCreateFromText( argv[arg] );
               tmp2 = tmp;
					GetColorVar( &tmp, &l.text_color );
					LineRelease( tmp2 );
				}

			}
			else if( StrCaseCmp( argv[arg]+1, "lines" ) == 0 )
			{
				arg++;
				if( arg < argc )
				{
               l.lines = atoi( argv[arg] );
				}
			}
			else if( StrCaseCmp( argv[arg]+1, "cols" ) == 0 )
			{
				arg++;
				if( arg < argc )
				{
               l.columns = atoi( argv[arg] );
				}
			}
			else if( StrCaseCmp( argv[arg]+1, "display" ) == 0 )
			{
				arg++;
				if( arg < argc )
				{
               l.display = atoi( argv[arg] );
				}
			}
			else if( StrCaseCmp( argv[arg]+1, "file" ) == 0 )
			{
				// blah
				arg++;
				if( arg < argc )
				{
               POINTER p;
					l.file = argv[arg];
					p = OpenSpace( NULL, l.file, &l.text_size );
					if( p )
					{
						l.text = NewArray( TEXTCHAR, l.text_size + 1 );
						MemCpy( l.text, p, l.text_size );
						l.text[l.text_size] = 0;
                  Release( p );
					}
				}
			}
			else if( StrCaseCmp( argv[arg]+1, "notop" ) == 0 )
			{
            l.flags.bTop = 0;
			}
			else if( StrCaseCmp( argv[arg]+1, "yesno" ) == 0 )
			{
            l.flags.bYesNo = 1;
			}
			else if( StrCaseCmp( argv[arg]+1, "okcancel" ) == 0 )
			{
            l.flags.bOkayCancel = 1;
			}
			else
			{
				if( !pvt )
				{
					pvt = VarTextCreate();
					vtprintf( pvt, "%s", argv[arg] );
				}
				else
					vtprintf( pvt, "\n%s", argv[arg] );
			}
		}
		else
		{
			if( !pvt )
			{
				pvt = VarTextCreate();
            vtprintf( pvt, "%s", argv[arg] );
			}
         else
            vtprintf( pvt, "\n%s", argv[arg] );
		}
      arg++;
	}
	if( pvt )
	{
      l.text = StrDup( GetText( VarTextPeek( pvt ) ) );
	}

	if( !l.text )
	{
      l.text = "INVALID COMMAND LINE\nARGUMENTS" ;
	}
	else
	{
		int n = 0;
      int o = 0;
		for( n = 0; l.text[n]; n++ )
		{
			if( l.text[n] < 32 )
			{
				if( l.text[n] == '\n' )
				{
					l.text[o++] = l.text[n];
				}
			}
			else
				l.text[o++] = l.text[n];
		}
		l.text[o++] = l.text[n];

	}
   return 1;
}


int main( int argc, char **argv )
{
   int result;
	PBANNER banner = NULL;
	if( !HandleArgs( argc, argv ) )
      return 0;

	if( !l.back_color )
      l.back_color = 0xFF000000;
	if( !l.text_color )
      l.text_color = 0xFFFFFFFF;

	result = CreateBannerExtended( NULL, &banner, l.text
										  , (l.flags.bYesNo?BANNER_OPTION_YESNO:0)
											|( l.flags.bOkayCancel?BANNER_OPTION_OKAYCANCEL:0)
											|( l.flags.bTop?BANNER_TOP:0)
											|((l.flags.bYesNo||l.flags.bOkayCancel)?0:BANNER_CLICK)
										  , 0
										  , l.text_color
										  , l.back_color
										  , l.lines
										  , l.columns
										  , l.display );

	{
		int result2 = WaitForBanner( banner );
      lprintf( "result is %d", result,result2 );
		return result;
	}

   return 0;
}
