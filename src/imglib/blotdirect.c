/*
 *  Crafted by Jim Buckeyne
 *   (c)1999-2006++ Freedom Collective
 *
 *  Support for putting one image on another without scaling.
 *
 *
 *
 *  consult doc/image.html
 *
 */

#define IMAGE_LIBRARY_SOURCE

#include <stdhdrs.h>
#include <sharemem.h>
#include <imglib/imagestruct.h>
#include <image.h>

#define NEED_ALPHA2
#include "blotproto.h"

#ifdef __cplusplus
namespace sack {
namespace image {
#endif


//---------------------------------------------------------------------------

#define StartLoop oo /= 4;    \
   oi /= 4;                   \
   {                          \
      int row= 0;             \
      while( row < hs )       \
      {                       \
         int col=0;           \
         while( col < ws )    \
         {                    \
            {

#define EndLoop   }           \
/*lprintf( "in %08x out %08x", ((CDATA*)pi)[0], ((CDATA*)po)[1] );*/ \
            po++;             \
            pi++;             \
            col++;            \
         }                    \
         pi += oi;            \
         po += oo;            \
         row++;               \
      }                       \
   }

 void CPROC cCopyPixelsT0( PCDATA po, PCDATA  pi
                          , int oo, int oi
                          , int ws, int hs
                           )
{
   StartLoop
            *po = *pi;
   EndLoop
}

//---------------------------------------------------------------------------

 void CPROC cCopyPixelsT1( PCDATA po, PCDATA  pi
                          , int oo, int oi
                          , int ws, int hs
                           )
{
   StartLoop
            CDATA cin;
            if( (cin = *pi) )
            {
               *po = cin;
            }
   EndLoop
}

//---------------------------------------------------------------------------

 void CPROC cCopyPixelsTA( PCDATA po, PCDATA  pi
                          , int oo, int oi
                          , int ws, int hs
                          , int nTransparent )
{
   StartLoop
            CDATA cin;
            if( (cin = *pi) )
            {
               *po = DOALPHA2( *po, cin, nTransparent );
               //lprintf( "cin=%08x", cin );
            }
   EndLoop
}

//---------------------------------------------------------------------------

 void CPROC cCopyPixelsTImgA( PCDATA po, PCDATA  pi
                          , int oo, int oi
                          , int ws, int hs
                          , int nTransparent )
{
   StartLoop
            int alpha;
            CDATA cin;
            if( (cin = *pi) )
            {
               alpha = ( cin & 0xFF000000 ) >> 24;
               alpha += nTransparent;
               *po = DOALPHA2( *po, cin, alpha );
            }
   EndLoop
}
//---------------------------------------------------------------------------

 void CPROC cCopyPixelsTImgAI( PCDATA po, PCDATA  pi
                          , int oo, int oi
                          , int ws, int hs
                          , int nTransparent )
{
   StartLoop
            int alpha;

            CDATA cin;
            if( (cin = *pi) )
            {
               alpha = ( cin & 0xFF000000 ) >> 24;
               alpha -= nTransparent;
               if( alpha > 1 )
               {
                  *po = DOALPHA2( *po, cin, alpha );
               }
            }
   EndLoop
}

//---------------------------------------------------------------------------

 void CPROC cCopyPixelsShadedT0( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , CDATA c )
{
   StartLoop
            _32 pixel;
            pixel = *pi;
            *po = SHADEPIXEL(pixel, c);
   EndLoop
}

//---------------------------------------------------------------------------
 void CPROC cCopyPixelsShadedT1( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , CDATA c )
{
   StartLoop
            _32 pixel;
            if( (pixel = *pi) )
            {
               *po = SHADEPIXEL(pixel, c);
            }
   EndLoop
}
//---------------------------------------------------------------------------

 void CPROC cCopyPixelsShadedTA( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , int nTransparent
                            , CDATA c )
{
   StartLoop
            _32 pixel, pixout;
            if( (pixel = *pi) )
            {
               pixout = SHADEPIXEL(pixel, c);
               *po = DOALPHA2( *po, pixout, nTransparent );
            }
   EndLoop
}

//---------------------------------------------------------------------------
 void CPROC cCopyPixelsShadedTImgA( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , int nTransparent
                            , CDATA c )
{
   StartLoop
            _32 pixel, pixout;
            if( (pixel = *pi) )
            {
               int alpha;
               pixout = SHADEPIXEL(pixel, c);
               alpha = ( pixel & 0xFF000000 ) >> 24;
               alpha += nTransparent;
               *po = DOALPHA2( *po, pixout, alpha );
            }
   EndLoop
}

//---------------------------------------------------------------------------
 void CPROC cCopyPixelsShadedTImgAI( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , int nTransparent
                            , CDATA c )
{
   StartLoop
            _32 pixel, pixout;
            if( (pixel = *pi) )
            {
               int alpha;
               alpha = ( pixel & 0xFF000000 ) >> 24;
               alpha -= nTransparent;
               if( alpha > 1 )
               {
                  pixout = SHADEPIXEL(pixel, c);
                  *po = DOALPHA2( *po, pixout, alpha );
               }
            }
   EndLoop
}


//---------------------------------------------------------------------------

 void CPROC cCopyPixelsMultiT0( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , CDATA r, CDATA g, CDATA b )
{
   StartLoop
            _32 pixel, pixout;
            {
               int rout, gout, bout;
               pixel = *pi;
               pixout = MULTISHADEPIXEL( pixel, r,g,b);
            }
            *po = pixout;
   EndLoop
}

//---------------------------------------------------------------------------
 void CPROC cCopyPixelsMultiT1( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , CDATA r, CDATA g, CDATA b )
{
   StartLoop
            _32 pixel, pixout;
            if( (pixel = *pi) )
            {
               int rout, gout, bout;
               pixout = MULTISHADEPIXEL( pixel, r,g,b);

               *po = pixout;
            }
   EndLoop
}
//---------------------------------------------------------------------------
 void CPROC cCopyPixelsMultiTA( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , int nTransparent
                            , CDATA r, CDATA g, CDATA b )
{
   StartLoop
            _32 pixel, pixout;
            if( (pixel = *pi) )
            {
               int rout, gout, bout;
               pixout = MULTISHADEPIXEL( pixel, r,g,b);
               *po = DOALPHA2( *po, pixout, nTransparent );
            }
   EndLoop
}
//---------------------------------------------------------------------------
 void CPROC cCopyPixelsMultiTImgA( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , int nTransparent
                            , CDATA r, CDATA g, CDATA b )
{
   StartLoop
            _32 pixel, pixout;
            if( (pixel = *pi) )
            {
               int rout, gout, bout;
               int alpha;
               alpha = ( pixel & 0xFF000000 ) >> 24;
               alpha += nTransparent;
               pixout = MULTISHADEPIXEL( pixel, r,g,b);
               //lprintf( "pixel %08x pixout %08x r %08x g %08x b %08x", pixel, pixout, r,g,b);
               *po = DOALPHA2( *po, pixout, alpha );
            }
   EndLoop
}
//---------------------------------------------------------------------------
 void CPROC cCopyPixelsMultiTImgAI( PCDATA po, PCDATA  pi
                            , int oo, int oi
                            , int ws, int hs
                            , int nTransparent
                            , CDATA r, CDATA g, CDATA b )
{
   StartLoop
            _32 pixel, pixout;
            if( (pixel = *pi) )
            {
               int rout, gout, bout;
               int alpha;
               alpha = ( pixel & 0xFF000000 ) >> 24;
               alpha -= nTransparent;
               if( alpha > 1 )
               {
                  pixout = MULTISHADEPIXEL( pixel, r,g,b);
                  *po = DOALPHA2( *po, pixout, alpha );
               }
            }
   EndLoop
}


//---------------------------------------------------------------------------
// x, y is position
// xs, ys is starting position on source bitmap (x, y is upper left) + xs, ys )
// w, h is height and width of the image to use.
// default behavior is to omit copying 0 pixels for transparency
// overlays....
 void  BlotImageSizedEx ( ImageFile *pifDest, ImageFile *pifSrc
                              , S_32 xd, S_32 yd
                              , S_32 xs, S_32 ys
                              , _32 ws, _32 hs
                              , _32 nTransparent
                              , _32 method
                              , ... )
{
#define BROKEN_CODE
   PCDATA po, pi;
   int  hd, wd;
   _32 oo, oi; // treated as an adder... it is unsigned by math, but still results correct offset?
   static _32 lock;
	va_list colors;
	va_start( colors, method );
   if( nTransparent > ALPHA_TRANSPARENT_MAX )
      return;

   if(  !pifSrc
     || !pifSrc->image
     || !pifDest
     || !pifDest->image )
      return;
   //lprintf( "BlotImageSized %d,%d to %d,%d by %d,%d", xs, ys, xd, yd, ws, hs );

   wd = pifDest->width + pifDest->x;
	hd = pifDest->height + pifDest->y;
	{
//cpg26dec2006 c:\work\sack\src\imglib\blotdirect.c(348): Warning! W202: Symbol 'r' has been defined, but not referenced
//cpg26dec2006  	IMAGE_RECTANGLE r;
		IMAGE_RECTANGLE r1;
		IMAGE_RECTANGLE r2;
      IMAGE_RECTANGLE rs;
      IMAGE_RECTANGLE rd;
      //IMAGE_RECTANGLE r3;
      r1.x = xd;
      r1.y = yd;
      r1.width = ws;
      r1.height = hs;
      r2.x = pifDest->eff_x;
      r2.y = pifDest->eff_y;
      r2.width = (pifDest->eff_maxx - pifDest->eff_x) + 1;
		r2.height = (pifDest->eff_maxy - pifDest->eff_y) + 1;
		if( !IntersectRectangle( &rd, &r1, &r2 ) )
		{
			//lprintf( "Images do not overlap. %d,%d %d,%d vs %d,%d %d,%d", r1.x,r1.y,r1.width,r1.height
			//		 , r2.x,r2.y,r2.width,r2.height);
         return;
		}

		//lprintf( "Correcting coordinates by %d,%d"
		//		 , rd.x - xd
		//		 , rd.y - yd
		//		 );

      xs += rd.x - xd;
      ys += rd.y - yd;
      ws -= rd.x - xd;
      hs -= rd.y - yd;
      //lprintf( "Resulting dest is %d,%d %d,%d", rd.x,rd.y,rd.width,rd.height );
		xd = rd.x;
      yd = rd.y;
      r1.x = xs;
      r1.y = ys;
      r1.width = ws;
      r1.height = hs;
      r2.x = pifSrc->eff_x;
      r2.y = pifSrc->eff_y;
      r2.width = (pifSrc->eff_maxx - pifSrc->eff_x) + 1;
		r2.height = (pifSrc->eff_maxy - pifSrc->eff_y) + 1;
		if( !IntersectRectangle( &rs, &r1, &r2 ) )
		{
         //lprintf( "Desired Output does not overlap..." );
         return;
		}
		//lprintf( "Resulting dest is %d,%d %d,%d", rs.x,rs.y,rs.width,rs.height );
		ws = rs.width<rd.width?rs.width:rd.width;
		hs = rs.height<rd.height?rs.height:rd.height;
		xs = rs.x;
		ys = rs.y;
		//lprintf( "Resulting rect is %d,%d to %d,%d dim: %d,%d", rs.x, rs.y, rd.x, rd.y, rs.width, rs.height );
		//lprintf( "Resulting rect is %d,%d to %d,%d dim: %d,%d", xs, ys, xd, yd, ws, hs );
	}
	//lprintf( WIDE("Doing image (%d,%d)-(%d,%d) (%d,%d)-(%d,%d)"), xs, ys, ws, hs, xd, yd, wd, hd );
#if 0
#ifdef BROKEN_CODE
   if( xs < 0 ) // source X before the start of image (0,0)
   {
      xd -= (xs - 0 ); // adds to the original position...
      ws += (xs - 0 ); // subtract this from width to display
      xs = 0;
   }
   if( ys < 0 )  // source Y above start of image( 0,0)
   {
      yd -= ys - 0; // add to destination to offset origin...
      hs += ys - 0; // subtract from height source to display
      ys = 0;
   }
   //
   //lprintf( WIDE("Doing image (%d,%d)-(%d,%d) (%d,%d)-(%d,%d)"), xs, ys, ws, hs, xd, yd, wd, hd );
   if( xd < pifDest->x )  // if destinatino before start of dest image(0,0)
   {
      xs -= xd - pifDest->x; // add to x source to place new xsource on edge
      ws += xd - pifDest->x; // subtract from width possible to display from source
      xd = pifDest->x;
   }
   if( yd < pifDest->y )  // destination before top of dest image(0,0)
   {
      ys -= yd - pifDest->y; // add to y source to place new ysource on edge
      hs += yd - pifDest->y; // subtract this amount from the height
      yd = pifDest->y;
   }
   //lprintf( WIDE("Doing image (%d,%d)-(%d,%d) (%d,%d)-(%d,%d)"), xs, ys, ws, hs, xd, yd, wd, hd );
#else
   if( xs < 0 ) // source X before the start of image (0,0)
   {
      xd -= (xs - 0 ); // adds to the original position...
      ws += (xs - 0 ); // subtract this from width to display
      xs = 0;
   }
   if( ys < 0 )  // source Y above start of image( 0,0)
   {
      yd -= ys - 0; // add to destination to offset origin...
      hs += ys - 0; // subtract from height source to display
      ys = 0;
   }
   //
   if( xd < 0 )  // if destinatino before start of dest image(0,0)
   {
      xs -= xd - 0; // add to x source to place new xsource on edge
      ws += xd - 0; // subtract from width possible to display from source
      xd = 0;
   }
   if( yd < 0 )  // destination before top of dest image(0,0)
   {
      ys -= yd - 0; // add to y source to place new ysource on edge
      hs += yd - 0; // subtract this amount from the height
      yd = 0;
   }
#endif
   if( ( ws + xd ) > ( pifDest->width ) )
   {
      ws = pifDest->width - xd;
   }
   if( ( hs + yd ) > ( pifDest->height ) )
   {
      hs = pifDest->height - yd;
   }
#ifdef BROKEN_CODE
   if( ( (S_32)hs + ys ) > ( pifSrc->real_height ) )
   {
      hs = pifSrc->real_height - ys;
   }
   if( ( (S_32)ws + xs ) > ( pifSrc->real_width ) )
   {
      ws = pifSrc->real_width - xs;
   }
   //lprintf( WIDE("Doing image (%d,%d)-(%d,%d) (%d,%d)-(%d,%d)"), xs, ys, ws, hs, xd, yd, wd, hd );
#endif
#endif
   if( (S_32)ws <= 0 ||
       (S_32)hs <= 0 ||
       (S_32)wd <= 0 ||
       (S_32)hd <= 0 )
      return;
#ifdef _INVERT_IMAGE
   // set pointer in to the starting x pixel
   // on the last line of the image to be copied
   //pi = IMG_ADDRESS( pifSrc, xs, ys );
   //po = IMG_ADDRESS( pifDest, xd, yd );
   pi = IMG_ADDRESS( pifSrc, xs, ys );
   po = IMG_ADDRESS( pifDest, xd, yd );
//cpg 19 Jan 2007 2>c:\work\sack\src\imglib\blotdirect.c(492) : warning C4146: unary minus operator applied to unsigned type, result still unsigned
//cpg 19 Jan 2007 2>c:\work\sack\src\imglib\blotdirect.c(493) : warning C4146: unary minus operator applied to unsigned type, result still unsigned
   oo = 4*-(int)(ws+pifDest->pwidth);     // w is how much we can copy...
   oi = 4*-(int)(ws+pifSrc->pwidth); // adding remaining width...
#else
   // set pointer in to the starting x pixel
   // on the first line of the image to be copied...
   pi = IMG_ADDRESS( pifSrc, xs, ys );
   po = IMG_ADDRESS( pifDest, xd, yd );
   oo = 4*(pifDest->pwidth - ws);     // w is how much we can copy...
   oi = 4*(pifSrc->pwidth - ws); // adding remaining width...
#endif
   //lprintf( WIDE("Doing image (%d,%d)-(%d,%d) (%d,%d)-(%d,%d)"), xs, ys, ws, hs, xd, yd, wd, hd );
   //oo = 4*(pifDest->pwidth - ws);     // w is how much we can copy...
   //oi = 4*(pifSrc->pwidth - ws); // adding remaining width...
	while( LockedExchange( &lock, 1 ) )
      Relinquish();
   {
      switch( method )
      {
      case BLOT_COPY:
         if( !nTransparent )
            CopyPixelsT0( po, pi, oo, oi, ws, hs );
         else if( nTransparent == 1 )
            CopyPixelsT1( po, pi, oo, oi, ws, hs );
         else if( nTransparent & ALPHA_TRANSPARENT )
            CopyPixelsTImgA( po, pi, oo, oi, ws, hs, nTransparent & 0xFF);
         else if( nTransparent & ALPHA_TRANSPARENT_INVERT )
            CopyPixelsTImgAI( po, pi, oo, oi, ws, hs, nTransparent & 0xFF );
         else
            CopyPixelsTA( po, pi, oo, oi, ws, hs, nTransparent );
         break;
      case BLOT_SHADED:
         if( !nTransparent )
            CopyPixelsShadedT0( po, pi, oo, oi, ws, hs
                           , va_arg( colors, CDATA ) );
         else if( nTransparent == 1 )
            CopyPixelsShadedT1( po, pi, oo, oi, ws, hs
                           , va_arg( colors, CDATA ) );
         else if( nTransparent & ALPHA_TRANSPARENT )
            CopyPixelsShadedTImgA( po, pi, oo, oi, ws, hs, nTransparent & 0xFF
                           , va_arg( colors, CDATA ) );
         else if( nTransparent & ALPHA_TRANSPARENT_INVERT )
            CopyPixelsShadedTImgAI( po, pi, oo, oi, ws, hs, nTransparent & 0xFF
                           , va_arg( colors, CDATA ) );
         else
            CopyPixelsShadedTA( po, pi, oo, oi, ws, hs, nTransparent
                           , va_arg( colors, CDATA ) );
         break;
		case BLOT_MULTISHADE:
			{
				CDATA r,g,b;
				r = va_arg( colors, CDATA );
				g = va_arg( colors, CDATA );
				b = va_arg( colors, CDATA );
				//lprintf( "r g b %08x %08x %08x", r,g, b );
				if( !nTransparent )
					CopyPixelsMultiT0( po, pi, oo, oi, ws, hs
										  , r, g, b );
				else if( nTransparent == 1 )
					CopyPixelsMultiT1( po, pi, oo, oi, ws, hs
										  , r, g, b );
				else if( nTransparent & ALPHA_TRANSPARENT )
					CopyPixelsMultiTImgA( po, pi, oo, oi, ws, hs, nTransparent & 0xFF
											  , r, g, b );
				else if( nTransparent & ALPHA_TRANSPARENT_INVERT )
					CopyPixelsMultiTImgAI( po, pi, oo, oi, ws, hs, nTransparent & 0xFF
												, r, g, b );
				else
					CopyPixelsMultiTA( po, pi, oo, oi, ws, hs, nTransparent
										  , r, g, b );
			}
			break;
      }
	}
   lock = 0;
   //lprintf( "Image done.." );
}
// copy all of pifSrc to the destination - placing the upper left
// corner of pifSrc on the point specified.
 void  BlotImageEx ( ImageFile *pifDest, ImageFile *pifSrc, S_32 xd, S_32 yd, _32 nTransparent, _32 method, ... )
{
	va_list colors;
	CDATA r;
	CDATA g;
	CDATA b;
	va_start( colors, method );
	r = va_arg( colors, CDATA );
	g = va_arg( colors, CDATA );
	b = va_arg( colors, CDATA );
	BlotImageSizedEx( pifDest, pifSrc, xd, yd, 0, 0
                   , pifSrc->real_width, pifSrc->real_height, nTransparent, method
                                      , r,g,b
                                    );
}
#ifdef __cplusplus
}; //namespace sack::image {
}; //namespace sack::image {
#endif


// $Log: blotdirect.c,v $
// Revision 1.21  2003/12/13 08:26:57  panther
// Fix blot direct for image bound having been set (break natural?)
//
// Revision 1.20  2003/09/21 20:47:26  panther
// Removed noisy logging messages.
//
// Revision 1.19  2003/09/21 16:25:28  panther
// Removed much noisy logging, all in the interest of sheet controls.
// Fixed some linking of services.
// Fixed service close on dead client.
//
// Revision 1.18  2003/09/12 14:37:55  panther
// Fix another overflow in drawing sources rects greater than source image
//
// Revision 1.17  2003/09/12 14:12:54  panther
// Fix apparently blotting image problem...
//
// Revision 1.16  2003/08/30 10:05:01  panther
// Fix clipping blotted images beyond dest boundries
//
// Revision 1.15  2003/08/14 11:57:48  panther
// Okay - so this blotdirect code definatly works....
//
// Revision 1.14  2003/08/13 16:24:22  panther
// Well - found what was broken...
//
// Revision 1.13  2003/08/13 16:14:11  panther
// Remove stupid timing...
//
// Revision 1.12  2003/07/31 08:55:30  panther
// Fix blotscaled boundry calculations - perhaps do same to blotdirect
//
// Revision 1.11  2003/07/25 00:08:31  panther
// Fixeup all copyies, scaled and direct for watcom
//
// Revision 1.10  2003/07/01 08:54:13  panther
// Fix seg fault when blotting soft cursor over bottom of screen
//
// Revision 1.9  2003/04/25 08:33:09  panther
// Okay move the -1's back out of IMG_ADDRESS
//
// Revision 1.8  2003/04/24 00:03:49  panther
// Added ColorAverage to image... Fixed a couple macros
//
// Revision 1.7  2003/03/30 18:39:03  panther
// Update image blotters to use IMG_ADDRESS
//
// Revision 1.6  2003/03/25 08:45:51  panther
// Added CVS logging tag
//
