// this library will be locked out by image_interface.h
// and likewise image_interface.h will be locked out by this.
// this is DESIRED - DO NOT FIX.
#ifndef IMAGE_H
#define IMAGE_H

#include <sack_types.h>
                  
#include <colordef.h>

#ifdef BCC16
#ifdef IMAGE_LIBRARY_SOURCE 
#define IMAGE_PROC(type,name) type STDPROC _export name
#else
#define IMAGE_PROC(type,name) extern type STDPROC name
#endif
#else
#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef IMAGE_LIBRARY_SOURCE 
#define IMAGE_PROC(type,name) EXPORT_METHOD type name
#else
#define IMAGE_PROC(type,name) __declspec(dllimport) type name
#endif
#else
#ifdef IMAGE_LIBRARY_SOURCE 
#define IMAGE_PROC(type,name) type name
#else
#define IMAGE_PROC(type,name) extern type name
#endif
#endif
#endif

#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef IMAGE_LIBRARY_SOURCE 
#define IMG_PROC __declspec(dllexport)
#else
#define IMG_PROC __declspec(dllimport)
#endif
#else
#ifdef IMAGE_LIBRARY_SOURCE
#define IMG_PROC
#else
#define IMG_PROC extern
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32 
#define _INVERT_IMAGE
#endif

//-----------------------------------------------------
#define BLOT_C    0
#define BLOT_ASM  1
#define BLOT_MMX  2         

IMG_PROC void SetBlotMethod( int method );
//-----------------------------------------------------

#ifndef IMAGE_STRUCTURE_DEFINED
#define IMAGE_STRUCTURE_DEFINED
// consider minimal size - +/- 32000 should be enough for display purposes.
// print... well that's another matter.
typedef int IMAGE_COORDINATE;

typedef IMAGE_COORDINATE IMAGE_POINT[2];
typedef IMAGE_COORDINATE *P_IMAGE_POINT;
#ifdef HAVE_ANONYMOUS_STRUCTURES
typedef struct boundry_rectangle_tag
{  
	union {
		IMAGE_POINT position;
		struct {
			IMAGE_COORDINATE x, y;
		};
	};
	union {
		IMAGE_POINT size;
		struct {
			IMAGE_COORDINATE width, height;
		};
	};
} IMAGE_RECTANGLE, *P_IMAGE_RECTANGLE;
#else
typedef struct boundry_rectangle_tag
{  
 	IMAGE_COORDINATE x, y;
       	IMAGE_COORDINATE width, height;
} IMAGE_RECTANGLE, *P_IMAGE_RECTANGLE;

#endif

#define IMAGE_POINT_H(ImagePoint) ((ImagePoint)[0])
#define IMAGE_POINT_V(ImagePoint) ((ImagePoint)[1])

// the image at exactly this position and size 
// is the one being referenced, the actual size and position 
// may vary depending on use (a sub-image outside the
// boundry of its parent).

typedef struct image_tag {
	union {
		IMAGE_RECTANGLE;
		IMAGE_RECTANGLE rect;
	};
	EMPTY_STRUCT extra; // data beyond this is private.  
} *Image, ImageFile;

// at some point, it may be VERY useful
// to have this structure also have a public member.
//
typedef struct font_tag {
   _16 height; // all characters same height
   _16 characters;
   _8 char_width[1]; // open ended array size characters...
} *Font;

#endif

enum string_behavior {
	STRING_PRINT_RAW   // every character assumed to have a glyph-including '\0'
	,STRING_PRINT_CONTROL // control characters perform 'typical' actions - newline, tab, backspace...
	,STRING_PRINT_C  // c style escape characters are handled \n \b \x## - literal text
	,STRING_PRINT_MENU // & performs an underline, also does C style handling. \& == &
};
IMAGE_PROC( void, SetStringBehavior )(ImageFile *pImage, _32 behavior );

IMG_PROC Image BuildImageFileEx( PCOLOR pc, int width, int height DBG_PASS); // already have the color plane....
#define BuildImageFile(p,w,h) BuildImageFileEx( p,w,h DBG_SRC )
IMG_PROC Image MakeImageFileEx(int Width, int Height DBG_PASS);
#define MakeImageFile(w,h) MakeImageFileEx( w,h DBG_SRC )
IMG_PROC Image MakeSubImageEx( Image pImage, int x, int y, int width, int height DBG_PASS );
#define MakeSubImage( image, x, y, w, h ) MakeSubImageEx( image, x, y, w, h DBG_SRC )
// may return a new image file if pImage was NULL
IMG_PROC Image RemakeImageEx( Image pImage, PCOLOR pc, int width, int height DBG_PASS);
#define RemakeImage(p,pc,w,h) RemakeImageEx(p,pc,w,h DBG_SRC)
IMG_PROC void ResizeImageEx( Image pImage, int width, int height DBG_PASS);
#define ResizeImage( p,w,h) ResizeImageEx( p,w,h DBG_SRC )
IMG_PROC void MoveImage( Image pImage, int x, int y );

IMG_PROC void GetImageSize( Image image, int *width, int *height );

IMG_PROC void UnmakeImageFileEx( Image pif DBG_PASS );
#define UnmakeImageFile(pif) UnmakeImageFileEx( pif DBG_SRC )

// at this piont flip image will not work on a externally allocated image.
// would require copying back if is either not owned, or if is a child image
IMG_PROC void FlipImageEx( Image pif DBG_PASS );
#define FlipImage(pif) FlipImageEx( pif DBG_SRC )
//Image BitmapToImageFile( PBITMAPINFO pbm );

// temporarily modify the x, y offset and width/height of the image.
// Fortunetlaty the image itself maintains it's absolute position, and
// from this we can apply a bound rect (a bound rect is clipped to the
// absolute image boundry.
IMG_PROC void SetImageBound( Image pImage, P_IMAGE_RECTANGLE bound );
// reset clip rectangle to the full image (subimage part )
// Some operations (move, resize) will also reset the bound rect, 
// this must be re-set afterwards.  
// ALSO - one SHOULD be nice and reset the rectangle when done, 
// otherwise other people may not have checked this.
IMG_PROC void FixImagePosition( Image pImage );


IMG_PROC Image ImageLoadImageFileEx( char *filename DBG_PASS );
IMG_PROC Image LoadImageFileEx( char *filename DBG_PASS );
#define LoadImageFile(file) LoadImageFileEx( file DBG_SRC )
IMG_PROC Image DecodeMemoryToImage( char *buf, int size );

IMG_PROC void BlatColorAlpha( Image pifDest, int x, int y, int w, int h, int color );

IMG_PROC void BlatColor( Image pifDest, int x, int y, int w, int h, int color );
#define ClearImageTo(img,color) BlatColor(img,0,0,-1,-1, color )
#define ClearImage(img) BlatColor(img,0,0,-1,-1, 0 )
//IMG_PROC void ClearImageTo( Image pImage, CDATA color );
//IMG_PROC void ClearImage( Image pImage );


#define BLOT_COPY 0
#define BLOT_SHADED 1
#define BLOT_MULTISHADE 2

#define ALPHA_TRANSPARENT 0x100
#define ALPHA_TRANSPARENT_INVERT 0x200
#define ALPHA_TRANSPARENT_MAX 0x2FF // more than this clips to total transparency
                                    // for line, plot more than 255 will 
                                    // be total opaque... this max only 
                                    // applies to blotted images
//Transparency parameter definition
// 0       : no transparency - completely opaque
// 1 (TRUE): 0 colors (absolute transparency) only
// 2-255   : 0 color transparent, plus transparency factor applied to all 
//           2 - mostly almost completely transparent
//           255 not transparent (opaque)
// 257-511 : alpha transparency in pixel plus transparency value - 256 
//           0 pixels will be transparent
//           257 - slightly more opaquen than the original
//           511 - image totally opaque - alpha will be totally overriden
//           no addition 511 nearly completely transparent
// 512-769 ; the low byte of this is subtracted from the alpha of the image
//         ; this allows images to be more transparent than they were originally
//          512 - no modification alpha imge normal
//          600 - mid range... more transparent
//          769 - totally transparent
// any value of transparent greater than the max will be clipped to max
// this will make very high values opaque totally...

IMG_PROC void BlotImageSizedEx( Image pDest, Image pIF, int x, int y, int xs, int ys, int wd, int ht, int nTransparent, int method, ...  );
// x and y are destination coordinates....
// source image assumed to start from 0,0 on x,y and go for the full
// width of the image...
IMG_PROC void BlotImageEx( Image pDest, Image pIF, int x, int y, int nTransparent, int method, ...  ); // always to screen...

#define BlotImage( pd, ps, x, y ) BlotImageEx( pd, ps, x, y, TRUE, BLOT_COPY )
#define BlotImageSized( pd, ps, x, y, w, h ) BlotImageSizedEx( pd, ps, x, y, 0, 0, w, h, TRUE, BLOT_COPY )
#define BlotImageSizedTo( pd, ps, xd, yd, xs, ys, w, h )  BlotImageSizedEx( pd, ps, xd, yd, xs, ys, w, h, TRUE, BLOT_COPY )

#define BlotImageShaded( pd, ps, xd, yd, c ) BlotImageEx( pd, ps, xd, yd, TRUE, BLOT_SHADED, c )
#define BlotImageShadedSized( pd, ps, xd, yd, xs, ys, ws, hs, c ) BlotImageSizedEx( pd, ps, xd, yd, xs, ys, ws, hs, TRUE, BLOT_SHADED, c )

#define BlotImageMultiShaded( pd, ps, xd, yd, r, g, b ) BlotImageEx( pd, ps, xd, yd, TRUE, BLOT_MULTISHADE, r, g, b )
#define BlotImageMultiShadedSized( pd, ps, xd, yd, xs, ys, ws, hs, r, g, b ) BlotImageEx( pd, ps, xd, yd, xs, ys, ws, hs, TRUE, BLOT_MULTISHADE, r, g, b )

IMG_PROC void BlotScaledImageSizedEx( Image pifDest, Image pifSrc
                                    , int xd, int yd
                                    , int wd, int hd
                                    , int xs, int ys
                                    , int ws, int hs
                                    , int nTransparent
                                    , int method, ... );

#define BlotScaledImageSized( pd, ps, xd, yd, wd, hd, xs, ys, ws, hs ) BlotScaledImageSized( pd, ps, xd, yd, wd, hd, xs, ys, ws, hs, 0, BLOT_COPY )

#define BlotScaledImageSizedTo( pd, ps, xd, yd, wd, hd) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, ps->width, ps->height, 0, BLOT_COPY )
#define BlotScaledImageSizedToAlpha( pd, ps, xd, yd, wd, hd, a) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, ps->width, ps->height, a, BLOT_COPY )

#define BlotScaledImageSizedToShaded( pd, ps, xd, yd, wd, hd,shade) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, ps->width, ps->height, 0,BLOT_SHADED, shade )
#define BlotScaledImageSizedToShadedAlpha( pd, ps, xd, yd, wd, hd,a,shade) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, ps->width, ps->height, a, BLOT_SHADED, shade )

#define BlotScaledImageSizedToMultiShaded( pd, ps, xd, yd, wd, hd,t,r,g,b) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, ps->width, ps->height, 0,BLOT_MULTISHADE, r,g,b )
#define BlotScaledImageSizedToMultiShadedAlpha( pd, ps, xd, yd, wd, hd,a,r,g,b) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, ps->width, ps->height, a,BLOT_MULTISHADE, r,g,b )

#define BlotScaledImageAlpha( pd, ps, t ) BlotScaledImageSizedEx( pd, ps, 0, 0, pd->width, pd->height, 0, 0, ps->width, ps->height, t, BLOT_COPY )
#define BlotScaledImageShadedAlpha( pd, ps, t, shade ) BlotScaledImageSizedEx( pd, ps, 0, 0, pd->width, pd->height, 0, 0, ps->width, ps->height, t, BLOT_SHADED, shade )
#define BlotScaledImageMultiShadedAlpha( pd, ps, t, r, g, b ) BlotScaledImageSizedEx( pd, ps, 0, 0, pd->width, pd->height, 0, 0, ps->width, ps->height, t, BLOT_MULTISHADE, r, g, b )

#define BlotScaledImage( pd, ps ) BlotScaledImageSizedEx( pd, ps, 0, 0, pd->width, pd->height, 0, 0, ps->width, ps->height, 0, BLOT_COPY )
#define BlotScaledImageShaded( pd, ps, shade ) BlotScaledImageSizedEx( pd, ps, 0, 0, pd->width, pd->height, 0, 0, ps->width, ps->height, 0, BLOT_SHADED, shade )
#define BlotScaledImageMultiShaded( pd, ps, r, g, b ) BlotScaledImageSizedEx( pd, ps, 0, 0, pd->width, pd->height, 0, 0, ps->width, ps->height, 0, BLOT_MULTISHADE, r, g, b )

#define BlotScaledImageTo( pd, ps )  BlotScaledImageToEx( pd, ps, FALSE, BLOT_COPY )

//-------------------------------
// Your basic PLOT functions  (Image.C, plotasm.asm)
//-------------------------------

IMG_PROC void (*plotraw)( Image pi, int x, int y, CDATA c );
IMG_PROC void (*plot)( Image pi, int x, int y, CDATA c );
IMG_PROC CDATA (*getpixel)( Image pi, int x, int y );
IMG_PROC void (*plotalpha)( Image pi, int x, int y, CDATA c );

//-------------------------------
// Line functions  (lineasm.asm) // should include a line.c ... for now core was assembly...
//-------------------------------
IMG_PROC void (*do_line)( Image pBuffer, int x, int y, int xto, int yto, CDATA color );  // d is color data...
// now why would we need an inverse line? I don't get it....
#define do_inv_line(pb,x,y,xto,yto,d) do_line( pb,y,x,yto,xto,d)

// callback expects ImageFile*, x, y, D (dword user)
// note - vertical increment always!
IMG_PROC void (*do_lineExV)( Image pBuf, int X1, int Y1, int X2, int Y2, CDATA color, void(*proc)(Image, int x, int y, int d ) );
// now why would we need an inverse line? I don't get it....
#define do_inv_lineExV(pb,x1,y1,x2,y2,d,proc) do_lineExV(pb,y1,x1,y2,x2,d,proc)

IMG_PROC void (*do_lineAlpha)( Image pBuffer, int x, int y, int xto, int yto, CDATA color);  // d is color data...


IMG_PROC void (*do_hline)( Image pImage, int y
                  , int xfrom, int xto, CDATA color );
IMG_PROC void (*do_vline)( Image pImage, int x
                  , int yfrom, int yto, CDATA color );
IMG_PROC void (*do_hlineAlpha)( Image pImage, int y
                  , int xfrom, int xto, CDATA color );
IMG_PROC void (*do_vlineAlpha)( Image pImage, int x
                  , int yfrom, int yto, CDATA color );

//-------------------------------
// Font functions
//-------------------------------
#ifndef FONT_TYPES_DEFINED
typedef unsigned char FONT; // data attached I suppose .. void FONT;
typedef FONT *PFONT;  		 // naturally a pointer to a font...
#endif


IMG_PROC PFONT GetDefaultFont(void);
//IMG_PROC void SetDefaultFont( PFONT );

// background of color 0,0,0 is transparent - alpha component does not
// matter....
IMG_PROC int PutCharacterFont			( Image pImage, int x, int y, CDATA color, CDATA background, unsigned char c, PFONT font );
IMG_PROC int PutCharacterVerticalFont	( Image pImage, int x, int y, CDATA color, CDATA background, unsigned char c, PFONT font );
IMG_PROC int PutCharacterInvertFont			( Image pImage, int x, int y, CDATA color, CDATA background, unsigned char c, PFONT font );
IMG_PROC int PutCharacterVerticalInvertFont	( Image pImage, int x, int y, CDATA color, CDATA background, unsigned char c, PFONT font );

#define PutCharacter(i,x,y,fore,back,c)               PutCharacterFont(i,x,y,fore,back,c,NULL,NULL )
#define PutCharacterVertical(i,x,y,fore,back,c)       PutCharacterVerticalFont(i,x,y,fore,back,c,NULL,NULL )
#define PutCharacterInvert(i,x,y,fore,back,c)         PutCharacterInvertFont(i,x,y,fore,back,c,NULL,NULL )
#define PutCharacterVerticalInvert(i,x,y,fore,back,c) PutCharacterVerticalInvertFont(i,x,y,fore,back,c,NULL,NULL )

IMG_PROC int PutStringFontEx				( Image pImage, int x, int y, CDATA color, CDATA background, char *pc, int nLen, PFONT font );
IMG_PROC int PutStringVerticalFontEx	( Image pImage, int x, int y, CDATA color, CDATA background, char *pc, int nLen, PFONT font );
IMG_PROC int PutStringInvertFontEx				( Image pImage, int x, int y, CDATA color, CDATA background, char *pc, int nLen, PFONT font );
IMG_PROC int PutStringInvertVerticalFontEx	( Image pImage, int x, int y, CDATA color, CDATA background, char *pc, int nLen, PFONT font );

#define PutString(pi,x,y,fore,back,pc) PutStringFontEx( pi, x, y, fore, back, pc, strlen(pc), NULL )
#define PutStringEx(pi,x,y,color,back,pc,len) PutStringFontEx( pi, x, y, color,back,pc,len,NULL )
#define PutStringFont(pi,x,y,fore,back,pc,font) PutStringFontEx(pi,x,y,fore,back,pc,strlen(pc), font )

#define PutStringVertical(pi,x,y,fore,back,pc) PutStringVerticalFontEx( pi, x, y, fore, back, pc, strlen(pc), NULL )
#define PutStringVerticalEx(pi,x,y,color,back,pc,len) PutStringVerticalFontEx( pi, x, y, color,back,pc,len,NULL )
#define PutStringVerticalFont(pi,x,y,fore,back,pc,font) PutStringVerticalFontEx(pi,x,y,fore,back,pc,strlen(pc), font )

#define PutStringInvert( pi, x, y, fore, back, pc ) PutStringInvertFontEx( pi, x, y, fore, back, pc,strlen(pc), NULL )
#define PutStringInvertEx( pi, x, y, fore, back, pc, nLen ) PutStringInvertFontEx( pi, x, y, fore, back, pc, nLen, NULL )
#define PutStringInvertFont( pi, x, y, fore, back, pc, nLen ) PutStringInvertFontVerticalFontEx( pi, x, y, fore, back, pc, strlen(pc), font )

#define PutStringInvertVertical( pi, x, y, fore, back, pc ) PutStringInvertVerticalFontEx( pi, x, y, fore, back, pc, strlen(pc), NULL )
#define PutStringInvertVerticalEx( pi, x, y, fore, back, pc, nLen ) PutStringInvertFontVerticalFontEx( pi, x, y, fore, back, pc, nLen, NULL )
#define PutStringInvertVerticalFont( pi, x, y, fore, back, pc, font ) PutStringInvertVerticalFontEx( pi, x, y, fore, back, pc, strlen(pc), font )

IMG_PROC int PutMenuStringFontEx			( Image pImage, int x, int y, CDATA color, CDATA background, char *pc, int nLen, PFONT font );
#define PutMenuStringFont(img,x,y,fore,back,string,font) PutMenuStringFontEx( img,x,y,fore,back,string,strlen(string),font)
#define PutMenuString(img,x,y,fore,back,str)	         PutMenuStringFont(img,x,y,fore,back,str,NULL)

IMG_PROC int PutCStringFontEx			   ( Image pImage, int x, int y, CDATA color, CDATA background, char *pc, int nLen, PFONT font );
#define PutCStringFont(img,x,y,fore,back,string,font) PutCStringFontEx( img,x,y,fore,back,string,strlen(string),font)
#define PutCString( img,x,y,fore,back,string) PutCStringFont(img,x,y,fore,back,string,NULL )

IMG_PROC int GetMenuStringSizeFontEx( char *string, int len, int *width, int *height, PFONT font );
IMG_PROC int GetMenuStringSizeFont( char *string, int *width, int *height, PFONT font );

#define GetStringSizeEx(s,len,pw,ph) GetStringSizeFontEx( s,len,pw,ph,NULL)
#define GetStringSize(s,pw,ph)       GetStringSizeFontEx( s,strlen(s),pw,ph,NULL)
IMG_PROC int GetStringSizeFontEx			( char *pString, int nLen, int *width, int *height, PFONT font );
#define GetStringSizeFont(s,pw,ph,f) GetStringSizeFontEx(s,strlen(s),pw,ph,f )
//IMG_PROC int GetStringSizeFont			( char *pString, int *width, int *height, PFONT font );
IMG_PROC int GetMaxStringLengthFont			( int width, PFONT font ); // returns characters within the width...(fixed font only)

IMG_PROC int GetFontHeight             ( PFONT font );

///------ these shoudl all be defines.

IMG_PROC int GetMenuStringSizeEx( char *string, int len, int *width, int *height );
IMG_PROC int GetMenuStringSize( char *string, int *width, int *height );

//IMG_PROC int GetStringSizeEx( char *pString, int nLen, int *width, int *height );
//IMG_PROC int GetStringSize( char *pString, int *width, int *height );
IMG_PROC int GetMaxStringLength( int width ); // returns characters within the width...

//-------------------------------
// Sprite functions
//-------------------------------

typedef struct sprite_tag
{
   Image image;
   // curx,y are kept for moving the sprite independantly
   int curx, cury;  // current x and current y for placement on image.
   int hotx, hoty;  // int of bitmap hotspot... centers cur on hot
   // should consider keeping the angle of rotation
   // and also should cosider keeping velocity/acceleration
   // but then limits would have to be kept also... so perhaps
   // the game module should keep such silly factors... but then couldn't
   // it also keep curx, cury ?  though hotx hoty is the actual
   // origin to rotate this image about, and to draw ON curx 0 cury 0
  // int orgx, orgy;  // rotated origin of bitmap. 
} SPRITE, *PSPRITE;

typedef int fixed;

/******************************************************/
/************ Graphics and sprite routines ************/
/******************************************************/

IMG_PROC PSPRITE MakeSpriteEx( DBG_VOIDPASS );
#define MakeSprite() MakeSpriteEx(DBG_VOIDSRC)
IMG_PROC PSPRITE MakeSpriteImageEx( Image pImage DBG_PASS);
#define MakeSpriteImage( pif) MakeSpriteImageEx(pif DBG_SRC )
IMG_PROC PSPRITE MakeSpriteImageFileEx( char *fname DBG_PASS );
#define MakeSpriteImageFile(file) MakeSpriteImageFileEx(file DBG_SRC )

IMG_PROC void rotate_sprite(Image dest, SPRITE *sprite, fixed angle);
IMG_PROC void rotate_scaled_sprite(Image dest, SPRITE *sprite, fixed angle, fixed scale);

IMG_PROC void BlotSprite( Image dest, SPRITE *ps );

#define SpriteImage(ps) (ps->image)


#ifdef __cplusplus
}
#endif

#endif


// $Log: $
