#include <stdhdrs.h>
#include <configscript.h>
#include <image.h>


IMAGE_NAMESPACE
#ifdef __cplusplus
namespace loader {
#endif
extern LOGICAL PngImageFile ( Image pImage, _8 ** buf, int *size);
#ifdef __cplusplus
};
#endif
IMAGE_NAMESPACE_END
#ifdef __cplusplus
using namespace sack::image::loader;
#endif
int main( int argc,  char **argv )
{
	//printf( "content-type:plain/text\r\n\r\n" );
	{
		const char *file = argv[1];
      PTEXT red = SegCreateFromText( argv[2] );
      PTEXT green = SegCreateFromText( argv[3] );
		PTEXT blue = SegCreateFromText( argv[4] );
      CDATA cred, cblue, cgreen;
		if( file && red && blue && green )
		{
			Image image = LoadImageFile( file );
			if( image )
			{
				Image out = MakeImageFile( image->width, image->height );
            lprintf("%s %s %s", GetText( red ), GetText( green) , GetText( blue ) );
				if( !GetColorVar( &red, &cred ) )
               lprintf( "FAIL RED" );
				if( !GetColorVar( &blue, &cblue ) )
					lprintf( "FAIL BLUE" );

				if( !GetColorVar( &green, &cgreen ) )
					lprintf( "FAIL gREEN" );

				ClearImage( out );
				//lprintf( "uhmm... %08x %08x %08x", cred, cgreen, cblue );
				BlotImageEx( out, image, 0, 0, ALPHA_TRANSPARENT, BLOT_MULTISHADE, cred, cgreen, cblue );

				//BlotImageMultiShaded( out, image, 0, 0, cred, cgreen, cblue );
				{
					int size;
					_8 *buf;
					if( PngImageFile( out, &buf, &size ) )
					{
						FILE *output = fopen( argv[5], "wb" );
						fwrite( buf, 1, size, output );
					}
				}
			}
		}
	}





	return 0;
}
