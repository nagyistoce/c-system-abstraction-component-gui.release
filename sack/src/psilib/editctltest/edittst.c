#define USE_IMAGE_INTERFACE GetImageInterface()
#define USE_RENDER_INTERFACE GetRenderInterface()


#include <controls.h>

int main( void )
{
   int okay = 0;
	PCOMMON frame = MakeCaptionedControl( NULL, CONTROL_FRAME, 50, 50, 420, 250, 0, WIDE("Edit Control Test") );
	SetCommonFont( frame, RenderFontFile( WIDE("arialbd.ttf")
								  , 40, 40
								  , 3 ) );

	MakeCaptionedControl( frame, EDIT_FIELD, 5, 5, 410, 18, 0, WIDE("Edit Box 1") );
	MakeCaptionedControl( frame, EDIT_FIELD, 5, 28, 410, 18, 0, WIDE("Edit Box 2") );
	MakeCaptionedControl( frame, EDIT_FIELD, 5, 51, 410, 18, 0, WIDE("Edit Box 3") );
	AddCommonButtons( frame, NULL, &okay );
   DisplayFrame( frame );
	CommonWait( frame );
   return 0;

}

