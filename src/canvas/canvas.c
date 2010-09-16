
#include <sack_types.h>
#include <image.h>
#include <sharemem.h>
#include <fractions.h>

enum {
   border_left, border_right, border_top, border_bottom
} BORDERS;

typedef struct canvas_tag
{
	struct {
		_32 bSomething : 1;
	} flags;
   Image image;
	FRACTION dpi, pel, width, height, border[4];
   // calculated.. may be useful later
	_32 img_width, img_height;
} CANVAS, *PCANVAS;


PCANVAS CreateCanvas( PFRACTION dpi
						  , PFRACTION pel
						  , PFRACTION width
						  , PFRACTION height
						  , PFRACTION border[4] )
{
	PCANVAS canvas = Allocate( sizeof( CANVAS ) );
   canvas->dpi = *dpi;
   canvas->pel = *pel;
   canvas->width = *width;
   canvas->height = *height;
   canvas->border[0] = *border[0];
   canvas->border[1] = *border[1];
   canvas->border[2] = *border[2];
	canvas->border[3] = *border[3];
	{
		FRACTION tmp;
		tmp = canvas->dpi;
      canvas->img_width = ReduceFraction( MulFractions( tmp, &canvas->width ) );
		tmp = canvas->dpi;
      canvas->img_height = ReduceFraction( MulFractions( tmp, &canvas->height ) );
		canvas->Image = MakeImageFile( img_width, img_height );
	}
   return canvas;
}

PCANVAS GetCanvas( void )
{
   FRACTION dpi, pel, width, height, border[4];
	SetFraction( dpi, 300, 1 );
	SetFraction( pel, 1, 72 );
	SetFractionV( width, 8, 1, 2 ); // 8 and 1 / 2
	SetFractionV( height, 11, 0, 1 ); // 11 and nothing / 1
   SetFractionV(  border[0], 0, 1, 4 );
   SetFractionV(  border[1], 0, 1, 4 );
   SetFractionV(  border[2], 0, 1, 4 );
   SetFractionV(  border[3], 0, 1, 4 );
	return CreateCanvas( &dpi, &pel, &width, &height, border );
}

