/*
 *  Crafted by Jim Buckeyne
 *   (c)1999-2006++ Freedom Collective
 * 
 *  Define operations on Images
 *    Image image = MakeImageFile(width,height);
 *    UnmakeImageFile( image );
 *    ... and draw on it inbetween I suppose...
 *  consult doc/image.html
 *
 */



// this library will be locked out by image.h
// and likewise image.h will be locked out by this.
// this is DESIRED - DO NOT FIX.

// if the library is to have it's own idea of what
// an image is - then it should have included
// the definition for 'Font', and 'Image' before 
// including this... otherwise, it is assumed to 
// be a client, and therefore does not need the information
// if a custom structure is used - then it MUST define 
// it's ACTUAL x,y,width,height as the first 4 S_32 bit values.

#ifndef IMAGE_H
#define IMAGE_H

#if defined( _MSC_VER ) && defined( SACK_BAG_EXPORTS ) && 0
#define HAS_ASSEMBLY
#endif

#include <sack_types.h>
#include <colordef.h>
#include <msgprotocol.h>
#include <fractions.h>
#include <procreg.h>

#ifndef SECOND_IMAGE_LEVEL
#define SECOND_IMAGE_LEVEL _2
#define PASTE(sym,name) name
#else
#define PASTE2(sym,name) sym##name
#define PASTE(sym,name) PASTE2(sym,name)
#endif
#define _PASTE2(sym,name) sym##name
#define _PASTE(sym,name) _PASTE2(sym,name)


#  ifdef BCC16
#     ifdef IMAGE_LIBRARY_SOURCE
#        define IMAGE_PROC(type,name) type STDPROC _export PASTE(SECOND_IMAGE_LEVEL,name)
#     else
#        define IMAGE_PROC(type,name) extern type STDPROC PASTE(SECOND_IMAGE_LEVEL,name)
#     endif
#  else
#     if !defined(__STATIC__) && !defined( GCC )
#        ifdef IMAGE_LIBRARY_SOURCE 
#           define IMAGE_PROC(type,name) LITERAL_LIB_EXPORT_METHOD type CPROC PASTE(SECOND_IMAGE_LEVEL,name)
#           ifdef IMAGE_MAIN
#               define IMAGE_PROC_D(type,name,args) LITERAL_LIB_EXPORT_METHOD type (CPROC*PASTE(SECOND_IMAGE_LEVEL,name))args
#           else
#             ifdef _MSC_VER
#                define IMAGE_PROC_D(type,name,args) extern type (CPROC*PASTE(SECOND_IMAGE_LEVEL,name))args
#             else
#                define IMAGE_PROC_D(type,name,args) extern __declspec(dllexport) type (CPROC*PASTE(SECOND_IMAGE_LEVEL,name))args
#             endif
#           endif
#        else
#           define IMAGE_PROC(type,name) LITERAL_LIB_IMPORT_METHOD type CPROC PASTE(SECOND_IMAGE_LEVEL,name)
#           define IMAGE_PROC_D(type,name,args) LITERAL_LIB_IMPORT_METHOD type (CPROC*PASTE(SECOND_IMAGE_LEVEL,name))args
#        endif
#     else
#        if defined( __CYGWIN__ ) && 0
#define STUPID_NO_DATA_EXPORTS
#           ifdef IMAGE_LIBRARY_SOURCE 
#              define IMAGE_PROC(type,name) LITERAL_LIB_EXPORT_METHOD type CPROC PASTE(SECOND_IMAGE_LEVEL,name)
#              ifdef IMAGE_MAIN
#                 define IMAGE_PROC_D(type,name,args) type CPROC PASTE(SECOND_IMAGE_LEVEL,name)args; \
                       type (CPROC*PASTE(SECOND_IMAGE_LEVEL,_PASTE(_,name)))args
#              else
#                 define IMAGE_PROC_D(type,name,args) extern type CPROC PASTE(SECOND_IMAGE_LEVEL,name)args; \
                                  extern type (CPROC*PASTE(SECOND_IMAGE_LEVEL,_PASTE(_,name)))args
#              endif
#           else
#              define IMAGE_PROC(type,name) extern type CPROC PASTE(SECOND_IMAGE_LEVEL,name)
#              define IMAGE_PROC_D(type,name,args) extern type CPROC PASTE(SECOND_IMAGE_LEVEL,name)args; \
                                    extern type (CPROC*PASTE(SECOND_IMAGE_LEVEL,_PASTE(_,name)))args
#           endif
#        else
#           ifdef IMAGE_LIBRARY_SOURCE 
#              define IMAGE_PROC(type,name) LITERAL_LIB_EXPORT_METHOD type CPROC PASTE(SECOND_IMAGE_LEVEL,name)
#              ifdef IMAGE_MAIN
#                 define IMAGE_PROC_D(type,name,args) type (CPROC*PASTE(SECOND_IMAGE_LEVEL,name))args
#              else
#                 define IMAGE_PROC_D(type,name,args) extern type (CPROC*PASTE(SECOND_IMAGE_LEVEL,name))args
#              endif
#           else
#              define IMAGE_PROC(type,name) extern type CPROC PASTE(SECOND_IMAGE_LEVEL,name)
#              define IMAGE_PROC_D(type,name,args) extern type (CPROC*PASTE(SECOND_IMAGE_LEVEL,name))args
#           endif
#        endif
#     endif
#  endif

#ifdef _WIN32 
#define _INVERT_IMAGE
#endif

#ifdef __cplusplus
#define IMAGE_NAMESPACE namespace sack { namespace image {
#define IMAGE_NAMESPACE_END }}
#define ASM_IMAGE_NAMESPACE extern "C" {
#define ASM_IMAGE_NAMESPACE_END }
#else
#define IMAGE_NAMESPACE 
#define IMAGE_NAMESPACE_END
#define ASM_IMAGE_NAMESPACE 
#define ASM_IMAGE_NAMESPACE_END
#endif

IMAGE_NAMESPACE

typedef S_32 fixed;
//#ifndef IMAGE_STRUCTURE_DEFINED
//#define IMAGE_STRUCTURE_DEFINED
// consider minimal size - +/- 32000 should be enough for display purposes.
// print... well that's another matter.
   typedef S_32 IMAGE_COORDINATE;
   typedef _32  IMAGE_SIZE_COORDINATE;

   typedef IMAGE_COORDINATE IMAGE_POINT[2];
   typedef IMAGE_SIZE_COORDINATE IMAGE_EXTENT[2];
   typedef IMAGE_COORDINATE *P_IMAGE_POINT;
   typedef IMAGE_SIZE_COORDINATE *P_IMAGE_EXTENT;

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
      IMAGE_EXTENT size;
      struct {
         IMAGE_SIZE_COORDINATE width, height;
      };
   };
} IMAGE_RECTANGLE, *P_IMAGE_RECTANGLE;
#else
typedef struct boundry_rectangle_tag
{  
   union {
      struct {
         IMAGE_COORDINATE x, y;
         IMAGE_SIZE_COORDINATE width, height;
      };
      struct {
         IMAGE_POINT position;
         IMAGE_EXTENT size;
      };
   };
} IMAGE_RECTANGLE, *P_IMAGE_RECTANGLE;
#endif
#define IMAGE_POINT_H(ImagePoint) ((ImagePoint)[0])
#define IMAGE_POINT_V(ImagePoint) ((ImagePoint)[1])

// the image at exactly this position and size 
// is the one being referenced, the actual size and position 
// may vary depending on use (a sub-image outside the
// boundry of its parent).
#define ImageData union {                           \
      struct {                                      \
         IMAGE_COORDINATE x, y;                     \
         IMAGE_SIZE_COORDINATE width, height;       \
      };                                            \
      struct {                                      \
         IMAGE_POINT position;                      \
         IMAGE_EXTENT size;                         \
      };                                            \
   }


typedef struct ImageFile_tag *Image;
#if defined( __cplusplus )
IMAGE_NAMESPACE_END
#endif
#include <imglib/imagestruct.h>
#if defined( __cplusplus )
IMAGE_NAMESPACE
#endif

typedef struct sprite_tag *PSPRITE;
//#endif
// at some point, it may be VERY useful
// to have this structure also have a public member.
//

#ifndef Font
typedef struct simple_font_tag {
   _16 height; // all characters same height
   _16 characters; // number of characters in the set
   _8 char_width[1]; // open ended array size characters...
} FontData;

typedef struct font_tag *Font;

//typedef EMPTY_STRUCT RealFont;

#endif

typedef struct data_transfer_state_tag {
   _32 size;
   _32 offset;
   CDATA buffer;
} *DataState;

//-----------------------------------------------------

enum string_behavior {
   STRING_PRINT_RAW   // every character assumed to have a glyph-including '\0'
   ,STRING_PRINT_CONTROL // control characters perform 'typical' actions - newline, tab, backspace...
   ,STRING_PRINT_C  // c style escape characters are handled \n \b \x## - literal text
   ,STRING_PRINT_MENU // & performs an underline, also does C style handling. \& == &
};

enum blot_methods {
    BLOT_C    
   , BLOT_ASM  
   , BLOT_MMX           
}; 

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
// 512-767 ; the low byte of this is subtracted from the alpha of the image
//         ; this allows images to be more transparent than they were originally
//          512 - no modification alpha imge normal
//          600 - mid range... more transparent
//          767 - totally transparent
// any value of transparent greater than the max will be clipped to max
// this will make very high values opaque totally...

// library global changes.
// string behavior cannot be tracked per image.
// string behavior should, for all strings, be the same
// usage for an application... so behavior is associated with
// the particular stream and/or image family.
// does not modify character handling behavior - only strings.
   IMAGE_PROC( void, SetStringBehavior)( Image pImage, _32 behavior );
   IMAGE_PROC( void, SetBlotMethod)    ( _32 method );
   IMAGE_PROC_D( CDATA, ColorAverage ,( CDATA c1, CDATA c2
                                 , int d, int max ) );
//-----------------------------------------------------

   IMAGE_PROC( Image,BuildImageFileEx) ( PCOLOR pc, _32 width, _32 height DBG_PASS); // already have the color plane....
   IMAGE_PROC( Image,MakeImageFileEx)  (_32 Width, _32 Height DBG_PASS);
   IMAGE_PROC( Image,MakeSubImageEx)   ( Image pImage, S_32 x, S_32 y, _32 width, _32 height DBG_PASS );
   IMAGE_PROC( void, AdoptSubImage )   ( Image pFoster, Image pOrphan );
   IMAGE_PROC( void, OrphanSubImage )  ( Image pImage );
   IMAGE_PROC( Image,RemakeImageEx)    ( Image pImage, PCOLOR pc, _32 width, _32 height DBG_PASS);
   IMAGE_PROC( Image,LoadImageFileEx)  ( CTEXTSTR name DBG_PASS );
   IMAGE_PROC( Image, DecodeMemoryToImage )( P_8 buf, _32 size );
   IMAGE_PROC( Image, ImageRawBMPFile )(_8* ptr, _32 filesize); // direct hack for processing clipboard data...

	IMAGE_PROC( void,UnmakeImageFileEx) ( Image pif DBG_PASS );
   IMAGE_PROC( void ,SetImageBound)    ( Image pImage, P_IMAGE_RECTANGLE bound );
// reset clip rectangle to the full image (subimage part )
// Some operations (move, resize) will also reset the bound rect, 
// this must be re-set afterwards.  
// ALSO - one SHOULD be nice and reset the rectangle when done, 
// otherwise other people may not have checked this.
   IMAGE_PROC( void ,FixImagePosition) ( Image pImage );

//-----------------------------------------------------
   // width, height < 0 are for image fixups...
   IMAGE_PROC( void,ResizeImageEx)     ( Image pImage, S_32 width, S_32 height DBG_PASS);
   IMAGE_PROC( void,MoveImage)         ( Image pImage, S_32 x, S_32 y );

//-----------------------------------------------------

   IMAGE_PROC( void,BlatColor)         ( Image pifDest, S_32 x, S_32 y, _32 w, _32 h, CDATA color );
   IMAGE_PROC( void,BlatColorAlpha)    ( Image pifDest, S_32 x, S_32 y, _32 w, _32 h, CDATA color );

   IMAGE_PROC( void,BlotImageEx)       ( Image pDest, Image pIF, S_32 x, S_32 y, _32 nTransparent, _32 method, ... ); // always to screen...
   IMAGE_PROC( void,BlotImageSizedEx)  ( Image pDest, Image pIF, S_32 x, S_32 y, S_32 xs, S_32 ys, _32 wd, _32 ht, _32 nTransparent, _32 method, ... );

   IMAGE_PROC( void,BlotScaledImageSizedEx)( Image pifDest, Image pifSrc
                                   , S_32 xd, S_32 yd
                                   , _32 wd, _32 hd
                                   , S_32 xs, S_32 ys
                                   , _32 ws, _32 hs
                                   , _32 nTransparent
                                   , _32 method, ... );


//-------------------------------
// Your basic PLOT functions  (Image.C, plotasm.asm)
//-------------------------------
   //void,*plotraw)   ( Image pi, S_32 x, S_32 y, CDATA c );
   IMAGE_PROC_D( void,plot,      ( Image pi, S_32 x, S_32 y, CDATA c ));
   IMAGE_PROC_D( void,plotalpha, ( Image pi, S_32 x, S_32 y, CDATA c ));
   IMAGE_PROC_D( CDATA,getpixel, ( Image pi, S_32 x, S_32 y ));
//-------------------------------
// Line functions  (lineasm.asm) // should include a line.c ... for now core was assembly...
//-------------------------------
   IMAGE_PROC_D( void,do_line,     ( Image pBuffer, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA color ));  // d is color data...
   IMAGE_PROC_D( void,do_lineAlpha,( Image pBuffer, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA color));  // d is color data...

   IMAGE_PROC_D( void,do_hline,     ( Image pImage, S_32 y, S_32 xfrom, S_32 xto, CDATA color ));
   IMAGE_PROC_D( void,do_vline,     ( Image pImage, S_32 x, S_32 yfrom, S_32 yto, CDATA color ));
   IMAGE_PROC_D( void,do_hlineAlpha,( Image pImage, S_32 y, S_32 xfrom, S_32 xto, CDATA color ));
   IMAGE_PROC_D( void,do_vlineAlpha,( Image pImage, S_32 x, S_32 yfrom, S_32 yto, CDATA color ));
	IMAGE_PROC_D( void, do_lineExV,( Image pImage, S_32 x, S_32 y
									, S_32 xto, S_32 yto, CDATA color
		                            , void (*func)( Image pif, S_32 x, S_32 y, int d ) ));
   IMAGE_PROC( Font,GetDefaultFont) ( void );
   IMAGE_PROC( _32 ,GetFontHeight)  ( Font );
   IMAGE_PROC( _32 ,GetStringSizeFontEx)( CTEXTSTR pString, _32 len, _32 *width, _32 *height, Font UseFont );
   IMAGE_PROC( _32, GetStringRenderSizeFontEx )( CTEXTSTR pString, _32 nLen, _32 *width, _32 *height, _32 *charheight, Font UseFont );

// background of color 0,0,0 is transparent - alpha component does not
// matter....
   IMAGE_PROC( void,PutCharacterFont)              ( Image pImage
                                                  , S_32 x, S_32 y
                                                  , CDATA color, CDATA background,
                                                   TEXTCHAR c, Font font );
   IMAGE_PROC( void,PutCharacterVerticalFont)      ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, TEXTCHAR c, Font font );
   IMAGE_PROC( void,PutCharacterInvertFont)        ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, TEXTCHAR c, Font font );
   IMAGE_PROC( void,PutCharacterVerticalInvertFont)( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, TEXTCHAR c, Font font );

   IMAGE_PROC( void,PutStringFontEx)              ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
   IMAGE_PROC( void,PutStringVerticalFontEx)      ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
   IMAGE_PROC( void,PutStringInvertFontEx)        ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
   IMAGE_PROC( void,PutStringInvertVerticalFontEx)( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );

   //_32 (*PutMenuStringFontEx)            ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
   //_32 (*PutCStringFontEx)               ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
   IMAGE_PROC( _32, GetMaxStringLengthFont ) ( _32 width, Font UseFont );

   IMAGE_PROC( void, GetImageSize)            ( Image pImage, _32 *width, _32 *height );
   IMAGE_PROC( PCDATA, GetImageSurface )       ( Image pImage );

   // would seem silly to load fonts - but for server implementations
   // the handle received is not the same as the font sent.
   IMAGE_PROC( Font, LoadFont )               ( Font font );
   IMAGE_PROC( void, UnloadFont )             ( Font font );
	IMAGE_PROC( void, SyncImage )                 ( void );
	// intersect rectangle, results with the overlapping portion of R1 and R2
   // into R ...
   IMAGE_PROC( int, IntersectRectangle )( IMAGE_RECTANGLE *r, IMAGE_RECTANGLE *r1, IMAGE_RECTANGLE *r2 );
   IMAGE_PROC( int, MergeRectangle )( IMAGE_RECTANGLE *r, IMAGE_RECTANGLE *r1, IMAGE_RECTANGLE *r2 );
   IMAGE_PROC( void, GetImageAuxRect )   ( Image pImage, P_IMAGE_RECTANGLE pRect );
   IMAGE_PROC( void, SetImageAuxRect )   ( Image pImage, P_IMAGE_RECTANGLE pRect );

	IMAGE_PROC( PSPRITE, MakeSpriteImageFileEx )( CTEXTSTR fname DBG_PASS );
	IMAGE_PROC( PSPRITE, MakeSpriteImageEx )( Image image DBG_PASS );
	IMAGE_PROC( void, UnmakeSprite )( PSPRITE sprite, int bForceImageAlso );
	// angle is a fixed scaled integer with 0x1 0000 0000 being the full circle.
	IMAGE_PROC( void, rotate_scaled_sprite )(Image bmp, PSPRITE sprite, fixed angle, fixed scale_width, fixed scale_height );
	IMAGE_PROC( void, rotate_sprite )(Image bmp, PSPRITE sprite, fixed angle);
	IMAGE_PROC( void, BlotSprite )( Image pdest, PSPRITE ps ); // hotspot bitmaps...
IMAGE_PROC( PSPRITE, SetSpriteHotspot )( PSPRITE sprite, S_32 x, S_32 y );
IMAGE_PROC( PSPRITE, SetSpritePosition )( PSPRITE sprite, S_32 x, S_32 y );

IMAGE_PROC( Font, InternalRenderFontFile )( CTEXTSTR file
										, S_16 nWidth
										, S_16 nHeight
										, _32 flags // whether to render 1(0/1), 2(2), 8(3) bits, ...
										);
IMAGE_PROC( Font, InternalRenderFont )( _32 nFamily
								  , _32 nStyle
								  , _32 nFile
								  , S_16 nWidth
								  , S_16 nHeight
                          , _32 flags
												  );
IMAGE_PROC( void, DestroyFont)( Font *font );
IMAGE_PROC( struct font_global_tag *, GetGlobalFonts)( void );
// types of data which may result...
typedef struct font_data_tag *PFONTDATA;
typedef struct render_font_data_tag *PRENDER_FONTDATA;

IMAGE_PROC( Font, RenderFont)( POINTER data, _32 flags ); // flags ? override?
IMAGE_PROC( Font, RenderScaledFontData)( PFONTDATA pfd, PFRACTION width_scale, PFRACTION height_scale );
#define RenderFontData(pfd) RenderScaledFontData( pfd,NULL,NULL )

IMAGE_PROC( Font, RenderFontFileEx )( CTEXTSTR file, _32 width, _32 height, _32 flags, P_32 pnFontDataSize, POINTER *pFontData );
#define RenderFontFile(file,w,h,flags) RenderFontFileEx(file,w,h,flags,NULL,NULL)
//typedef struct blah *PFONTDATA;
IMAGE_PROC( int, GetFontRenderData )( Font font, POINTER *fontdata, _32 *fontdatalen );
// exported for the PSI font chooser to set the data for the font
// to be retreived later when only the font handle remains.
IMAGE_PROC( void, SetFontRendererData )( Font font, POINTER pResult, _32 size );


#define IMAGE_PROC_PTR(type,name) type (CPROC*_##name)
#define DIMAGE_PROC_PTR(type,name) type (CPROC**_##name)
//-----------------------------------------------------
typedef struct image_interface_tag 
{
/*4*/ IMAGE_PROC_PTR( void, SetStringBehavior) ( Image pImage, _32 behavior );
/*5*/ IMAGE_PROC_PTR( void, SetBlotMethod)     ( _32 method );

//-----------------------------------------------------

/*6*/   IMAGE_PROC_PTR( Image,BuildImageFileEx) ( PCOLOR pc, _32 width, _32 height DBG_PASS); // already have the color plane....
/*7*/   IMAGE_PROC_PTR( Image,MakeImageFileEx)  (_32 Width, _32 Height DBG_PASS);
/*8*/   IMAGE_PROC_PTR( Image,MakeSubImageEx)   ( Image pImage, S_32 x, S_32 y, _32 width, _32 height DBG_PASS );
/*9*/   IMAGE_PROC_PTR( Image,RemakeImageEx)    ( Image pImage, PCOLOR pc, _32 width, _32 height DBG_PASS);
/*10*/  IMAGE_PROC_PTR( Image,LoadImageFileEx)  ( CTEXTSTR name DBG_PASS );
/*11*/  IMAGE_PROC_PTR( void,UnmakeImageFileEx) ( Image pif DBG_PASS );
#define UnmakeImageFile(pif) UnmakeImageFileEx( pif DBG_SRC )
/*12*/  IMAGE_PROC_PTR( void ,SetImageBound)    ( Image pImage, P_IMAGE_RECTANGLE bound );
// reset clip rectangle to the full image (subimage part )
// Some operations (move, resize) will also reset the bound rect, 
// this must be re-set afterwards.  
// ALSO - one SHOULD be nice and reset the rectangle when done, 
// otherwise other people may not have checked this.
/*13*/  IMAGE_PROC_PTR( void ,FixImagePosition) ( Image pImage );

//-----------------------------------------------------

/*14*/  IMAGE_PROC_PTR( void,ResizeImageEx)     ( Image pImage, S_32 width, S_32 height DBG_PASS);
/*15*/   IMAGE_PROC_PTR( void,MoveImage)         ( Image pImage, S_32 x, S_32 y );

//-----------------------------------------------------

/*16*/   IMAGE_PROC_PTR( void,BlatColor)     ( Image pifDest, S_32 x, S_32 y, _32 w, _32 h, CDATA color );
/*17*/   IMAGE_PROC_PTR( void,BlatColorAlpha)( Image pifDest, S_32 x, S_32 y, _32 w, _32 h, CDATA color );

/*18*/   IMAGE_PROC_PTR( void,BlotImageEx)     ( Image pDest, Image pIF, S_32 x, S_32 y, _32 nTransparent, _32 method, ... ); // always to screen...
/*19*/   IMAGE_PROC_PTR( void,BlotImageSizedEx)( Image pDest, Image pIF, S_32 x, S_32 y, S_32 xs, S_32 ys, _32 wd, _32 ht, _32 nTransparent, _32 method, ... );

/*20*/   IMAGE_PROC_PTR( void,BlotScaledImageSizedEx)( Image pifDest, Image pifSrc
                                   , S_32 xd, S_32 yd
                                   , _32 wd, _32 hd
                                   , S_32 xs, S_32 ys
                                   , _32 ws, _32 hs
                                   , _32 nTransparent
                                   , _32 method, ... );


//-------------------------------
// Your basic PLOT functions  (Image.C, plotasm.asm)
//-------------------------------
   //void,*plotraw)   ( Image pi, S_32 x, S_32 y, CDATA c );
/*21*/   DIMAGE_PROC_PTR( void,plot)      ( Image pi, S_32 x, S_32 y, CDATA c );
/*22*/   DIMAGE_PROC_PTR( void,plotalpha) ( Image pi, S_32 x, S_32 y, CDATA c );
/*23*/   DIMAGE_PROC_PTR( CDATA,getpixel) ( Image pi, S_32 x, S_32 y );
//-------------------------------
// Line functions  (lineasm.asm) // should include a line.c ... for now core was assembly...
//-------------------------------
/*24*/   DIMAGE_PROC_PTR( void,do_line)     ( Image pBuffer, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA color );  // d is color data...
/*25*/   DIMAGE_PROC_PTR( void,do_lineAlpha)( Image pBuffer, S_32 x, S_32 y, S_32 xto, S_32 yto, CDATA color);  // d is color data...

/*26*/   DIMAGE_PROC_PTR( void,do_hline)     ( Image pImage, S_32 y, S_32 xfrom, S_32 xto, CDATA color );
/*27*/   DIMAGE_PROC_PTR( void,do_vline)     ( Image pImage, S_32 x, S_32 yfrom, S_32 yto, CDATA color );
/*28*/   DIMAGE_PROC_PTR( void,do_hlineAlpha)( Image pImage, S_32 y, S_32 xfrom, S_32 xto, CDATA color );
/*29*/   DIMAGE_PROC_PTR( void,do_vlineAlpha)( Image pImage, S_32 x, S_32 yfrom, S_32 yto, CDATA color );

/*30*/   IMAGE_PROC_PTR( Font,GetDefaultFont) ( void );
/*31*/   IMAGE_PROC_PTR( _32 ,GetFontHeight)  ( Font );
/*32*/   IMAGE_PROC_PTR( _32 ,GetStringSizeFontEx)( CTEXTSTR pString, _32 len, _32 *width, _32 *height, Font UseFont );

// background of color 0,0,0 is transparent - alpha component does not
// matter....
/*33*/   IMAGE_PROC_PTR( void,PutCharacterFont)              ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, TEXTCHAR c, Font font );
/*34*/   IMAGE_PROC_PTR( void,PutCharacterVerticalFont)      ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, TEXTCHAR c, Font font );
/*35*/   IMAGE_PROC_PTR( void,PutCharacterInvertFont)        ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, TEXTCHAR c, Font font );
/*36*/   IMAGE_PROC_PTR( void,PutCharacterVerticalInvertFont)( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, TEXTCHAR c, Font font );

/*37*/   IMAGE_PROC_PTR( void,PutStringFontEx)              ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
/*38*/   IMAGE_PROC_PTR( void,PutStringVerticalFontEx)      ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
/*39*/   IMAGE_PROC_PTR( void,PutStringInvertFontEx)        ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
/*40*/   IMAGE_PROC_PTR( void,PutStringInvertVerticalFontEx)( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );

   //_32 (*PutMenuStringFontEx)            ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
   //_32 (*PutCStringFontEx)               ( Image pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, Font font );
/*41*/   IMAGE_PROC_PTR( _32, GetMaxStringLengthFont )( _32 width, Font UseFont );

/*42*/   IMAGE_PROC_PTR( void, GetImageSize)                ( Image pImage, _32 *width, _32 *height );
/*43*/   IMAGE_PROC_PTR( Font, LoadFont )                   ( Font font );
         IMAGE_PROC_PTR( void, UnloadFont )                 ( Font font );

/*44*/   IMAGE_PROC_PTR( DataState, BeginTransferData )    ( _32 total_size, _32 segsize, CDATA data );
/*45*/   IMAGE_PROC_PTR( void, ContinueTransferData )      ( DataState state, _32 segsize, CDATA data );
/*46*/   IMAGE_PROC_PTR( Image, DecodeTransferredImage )    ( DataState state );
/*47*/   IMAGE_PROC_PTR( Font, AcceptTransferredFont )     ( DataState state );
/*48*/   DIMAGE_PROC_PTR( CDATA, ColorAverage )( CDATA c1, CDATA c2
                                              , int d, int max );
/*49*/   IMAGE_PROC_PTR( void, SyncImage )                 ( void );
         IMAGE_PROC_PTR( PCDATA, GetImageSurface )       ( Image pImage );
         IMAGE_PROC_PTR( int, IntersectRectangle )      ( IMAGE_RECTANGLE *r, IMAGE_RECTANGLE *r1, IMAGE_RECTANGLE *r2 );
   IMAGE_PROC_PTR( int, MergeRectangle )( IMAGE_RECTANGLE *r, IMAGE_RECTANGLE *r1, IMAGE_RECTANGLE *r2 );
   IMAGE_PROC_PTR( void, GetImageAuxRect )   ( Image pImage, P_IMAGE_RECTANGLE pRect );
   IMAGE_PROC_PTR( void, SetImageAuxRect )   ( Image pImage, P_IMAGE_RECTANGLE pRect );
   IMAGE_PROC_PTR( void, OrphanSubImage )  ( Image pImage );
   IMAGE_PROC_PTR( void, AdoptSubImage )   ( Image pFoster, Image pOrphan );
	IMAGE_PROC_PTR( PSPRITE, MakeSpriteImageFileEx )( CTEXTSTR fname DBG_PASS );
	IMAGE_PROC_PTR( PSPRITE, MakeSpriteImageEx )( Image image DBG_PASS );
	IMAGE_PROC_PTR( void   , rotate_scaled_sprite )(Image bmp, PSPRITE sprite, fixed angle, fixed scale_width, fixed scale_height);
	IMAGE_PROC_PTR( void   , rotate_sprite )(Image bmp, PSPRITE sprite, fixed angle);
	IMAGE_PROC_PTR( void   , BlotSprite )( Image pdest, PSPRITE ps ); // hotspot bitmaps...
   IMAGE_PROC_PTR( Image, DecodeMemoryToImage )( P_8 buf, _32 size );

   // returns a Font
	IMAGE_PROC_PTR( Font, InternalRenderFontFile )( CTEXTSTR file
																 , S_16 nWidth
																 , S_16 nHeight
																 , _32 flags // whether to render 1(0/1), 2(2), 8(3) bits, ...
																 );
   // requires knowing the font cache....
	IMAGE_PROC_PTR( Font, InternalRenderFont )( _32 nFamily
															, _32 nStyle
															, _32 nFile
															, S_16 nWidth
															, S_16 nHeight
															, _32 flags
															);
IMAGE_PROC_PTR( Font, RenderScaledFontData)( PFONTDATA pfd, PFRACTION width_scale, PFRACTION height_scale );
//IMAGE_PROC_PTR( Font, RenderFontData )( PFONTDATA pfd );
IMAGE_PROC_PTR( Font, RenderFontFileEx )( CTEXTSTR file, _32 width, _32 height, _32 flags, P_32 size, POINTER *pFontData );
//typedef struct blah *PFONTDATA;
IMAGE_PROC_PTR( void, DestroyFont)( Font *font );
struct font_global_tag *_global_font_data;
IMAGE_PROC_PTR( int, GetFontRenderData )( Font font, POINTER *fontdata, _32 *fontdatalen );
IMAGE_PROC_PTR( void, SetFontRendererData )( Font font, POINTER pResult, _32 size );
IMAGE_PROC_PTR( PSPRITE, SetSpriteHotspot )( PSPRITE sprite, S_32 x, S_32 y );
IMAGE_PROC_PTR( PSPRITE, SetSpritePosition )( PSPRITE sprite, S_32 x, S_32 y );
	IMAGE_PROC_PTR( void, UnmakeSprite )( PSPRITE sprite, int bForceImageAlso );
IMAGE_PROC_PTR( struct font_global_tag *, GetGlobalFonts)( void );

IMAGE_PROC_PTR( _32, GetStringRenderSizeFontEx )( CTEXTSTR pString, _32 nLen, _32 *width, _32 *height, _32 *charheight, Font UseFont );

} IMAGE_INTERFACE, *PIMAGE_INTERFACE;


IMAGE_PROC( struct image_interface_tag*, GetImageInterface )( void );
IMAGE_PROC( void, DropImageInterface )( PIMAGE_INTERFACE );

#ifndef PSPRITE_METHOD
#define PSPRITE_METHOD PSPRITE_METHOD
IMAGE_NAMESPACE_END
//RENDER_NAMESPACE
	typedef struct sprite_method_tag *PSPRITE_METHOD;
//END_RENDER_NAMESPACE
IMAGE_NAMESPACE
#endif
	// provided for display rendering portion to define this method for sprites to use.
   // deliberately out of namespace... please do not move this up.
IMAGE_PROC( void, SetSavePortion )( void (CPROC*_SavePortion )( PSPRITE_METHOD psm, _32 x, _32 y, _32 w, _32 h ) );

#define GetImageInterface() (PIMAGE_INTERFACE)GetInterface( WIDE("image") )
#define DropImageInterface(x) DropInterface( WIDE("image"), NULL )

#define PROC_ALIAS(name) ((USE_IMAGE_INTERFACE)->_##name)
#define PPROC_ALIAS(name) (*(USE_IMAGE_INTERFACE)->_##name)

#ifdef DEFINE_DEFAULT_IMAGE_INTERFACE
//static PIMAGE_INTERFACE always_defined_interface_that_makes_this_efficient;
#define USE_IMAGE_INTERFACE GetImageInterface()
#endif

#ifdef FORCE_NO_INTERFACE
#undef USE_IMAGE_INTERFACE
#endif

#ifdef USE_IMAGE_INTERFACE
#define SetStringBehavior                  PROC_ALIAS(SetStringBehavior )
#define SetBlotMethod                      PROC_ALIAS(SetBlotMethod )
#define BuildImageFileEx                   PROC_ALIAS(BuildImageFileEx )
#define MakeImageFileEx                    PROC_ALIAS(MakeImageFileEx )
#define MakeSubImageEx                     PROC_ALIAS(MakeSubImageEx )
#define RemakeImageEx                      PROC_ALIAS(RemakeImageEx )
#define ResizeImageEx                      PROC_ALIAS(ResizeImageEx )
#define MoveImage                          PROC_ALIAS(MoveImage )
#define SetImageBound                      PROC_ALIAS(SetImageBound )
#define FixImagePosition                   PROC_ALIAS(FixImagePosition )
#define LoadImageFileEx                    PROC_ALIAS(LoadImageFileEx )
#define DecodeMemoryToImage                PROC_ALIAS(DecodeMemoryToImage )
#define UnmakeImageFileEx                  PROC_ALIAS(UnmakeImageFileEx )
#define BlatColor                          PROC_ALIAS(BlatColor )
#define BlatColorAlpha                     PROC_ALIAS(BlatColorAlpha )
#define BlotImageSizedEx                   PROC_ALIAS(BlotImageSizedEx )
#define BlotImageEx                        PROC_ALIAS(BlotImageEx )
#define BlotScaledImageSizedEx             PROC_ALIAS(BlotScaledImageSizedEx )
#define plot                               PPROC_ALIAS(plot )
#define plotalpha                          PPROC_ALIAS(plotalpha )
#define getpixel                           PPROC_ALIAS(getpixel )
#define do_line                            PPROC_ALIAS(do_line )
#define do_lineAlpha                       PPROC_ALIAS(do_lineAlpha )
#define do_hline                           PPROC_ALIAS(do_hline )
#define do_vline                           PPROC_ALIAS(do_vline )
#define do_hlineAlpha                      PPROC_ALIAS(do_hlineAlpha )
#define do_vlineAlpha                      PPROC_ALIAS(do_vlineAlpha )
#define GetDefaultFont                     PROC_ALIAS(GetDefaultFont )
#define GetFontHeight                      PROC_ALIAS(GetFontHeight )
#define GetStringSizeFontEx                PROC_ALIAS(GetStringSizeFontEx )
#define PutCharacterFont                   PROC_ALIAS(PutCharacterFont )
#define PutCharacterVerticalFont           PROC_ALIAS(PutCharacterVerticalFont )
#define PutCharacterInvertFont             PROC_ALIAS(PutCharacterInvertFont )
#define PutCharacterVerticalInvertFont     PROC_ALIAS(PutCharacterVerticalInvertFont )
#define PutStringFontEx                    PROC_ALIAS(PutStringFontEx )
#define PutStringVerticalFontEx            PROC_ALIAS(PutStringVerticalFontEx )
#define PutStringInvertFontEx              PROC_ALIAS(PutStringInvertFontEx )
#define PutStringInvertVerticalFontEx      PROC_ALIAS(PutStringInvertVerticalFontEx )
#define GetMaxStringLengthFont             PROC_ALIAS(GetMaxStringLengthFont )
#define GetImageSize                       PROC_ALIAS(GetImageSize )
#define LoadFont                           PROC_ALIAS(LoadFont )
#define UnloadFont                         PROC_ALIAS(UnloadFont )
#define ColorAverage                       PPROC_ALIAS(ColorAverage)
#define SyncImage                          PROC_ALIAS(SyncImage )
#define IntersectRectangle                 PROC_ALIAS(IntersectRectangle)
#define MergeRectangle                     PROC_ALIAS(MergeRectangle)
#define GetImageSurface                    PROC_ALIAS(GetImageSurface)
#define SetImageAuxRect                    PROC_ALIAS(SetImageAuxRect)
#define GetImageAuxRect                    PROC_ALIAS(GetImageAuxRect)
#define OrphanSubImage                     PROC_ALIAS(OrphanSubImage)
#define GetGlobalFonts                     PROC_ALIAS(GetGlobalFonts)
#define AdoptSubImage                      PROC_ALIAS(AdoptSubImage)

#define MakeSpriteImageFileEx   PROC_ALIAS(MakeSpriteImageFileEx)
#define MakeSpriteImageEx       PROC_ALIAS(MakeSpriteImageEx)
#define UnmakeSprite            PROC_ALIAS(UnmakeSprite )
#define rotate_scaled_sprite    PROC_ALIAS(rotate_scaled_sprite)
#define rotate_sprite           PROC_ALIAS(rotate_sprite)
#define BlotSprite              PROC_ALIAS(BlotSprite)
#define SetSpritePosition  PROC_ALIAS(  SetSpritePosition )
#define SetSpriteHotspot  PROC_ALIAS(  SetSpriteHotspot )

#define InternalRenderFont          PROC_ALIAS(InternalRenderFont)
#define InternalRenderFontFile      PROC_ALIAS(InternalRenderFontFile)
#define RenderScaledFontData              PROC_ALIAS(RenderScaledFontData)
#define RenderFontFileEx              PROC_ALIAS(RenderFontFileEx)
#define DestroyFont              PROC_ALIAS(DestroyFont)
#define GetFontRenderData              PROC_ALIAS(GetFontRenderData)
#define SetFontRendererData              PROC_ALIAS(SetFontRendererData)
//#define global_font_data         (*PROC_ALIAS(global_font_data))
#endif

#define GetMaxStringLength(w) GetMaxStringLengthFont(w, NULL )

#ifdef DEFINE_IMAGE_PROTOCOL
#include <stddef.h>
// need to define BASE_IMAGE_MESSAGE_ID before hand to determine what the base message is.
//#define MSG_ID(method)  ( ( offsetof( struct image_interface_tag, _##method ) / sizeof( void(*)(void) ) ) + BASE_IMAGE_MESSAGE_ID + MSG_EventUser )
#define MSG_SetStringBehavior                  MSG_ID( SetStringBehavior )
#define MSG_SetBlotMethod                      MSG_ID( SetBlotMethod )
#define MSG_BuildImageFileEx                   MSG_ID( BuildImageFileEx )
#define MSG_MakeImageFileEx                    MSG_ID( MakeImageFileEx )
#define MSG_MakeSubImageEx                     MSG_ID( MakeSubImageEx )
#define MSG_RemakeImageEx                      MSG_ID( RemakeImageEx )
#define MSG_UnmakeImageFileEx                  MSG_ID( UnmakeImageFileEx )
#define MSG_ResizeImageEx                      MSG_ID( ResizeImageEx )
#define DecodeMemoryToImage                    MSG_ID( DecodeMemoryToImage )
#define MSG_MoveImage                          MSG_ID( MoveImage )
#define MSG_SetImageBound                      MSG_ID( SetImageBound )
#define MSG_FixImagePosition                   MSG_ID( FixImagePosition )
#define MSG_BlatColor                          MSG_ID( BlatColor )
#define MSG_BlatColorAlpha                     MSG_ID( BlatColorAlpha )
#define MSG_BlotImageSizedEx                   MSG_ID( BlotImageSizedEx )
#define MSG_BlotImageEx                        MSG_ID( BlotImageEx )
#define MSG_BlotScaledImageSizedEx             MSG_ID( BlotScaledImageSizedEx )
#define MSG_plot                               MSG_ID( plot )
#define MSG_plotalpha                          MSG_ID( plotalpha )
#define MSG_getpixel                           MSG_ID( getpixel )
#define MSG_do_line                            MSG_ID( do_line )
#define MSG_do_lineAlpha                       MSG_ID( do_lineAlpha )
#define MSG_do_hline                           MSG_ID( do_hline )
#define MSG_do_vline                           MSG_ID( do_vline )
#define MSG_do_hlineAlpha                      MSG_ID( do_hlineAlpha )
#define MSG_do_vlineAlpha                      MSG_ID( do_vlineAlpha )
#define MSG_GetDefaultFont                     MSG_ID( GetDefaultFont )
#define MSG_GetFontHeight                      MSG_ID( GetFontHeight )
#define MSG_GetStringSizeFontEx                MSG_ID( GetStringSizeFontEx )
#define MSG_PutCharacterFont                   MSG_ID( PutCharacterFont )
#define MSG_PutCharacterVerticalFont           MSG_ID( PutCharacterVerticalFont )
#define MSG_PutCharacterInvertFont             MSG_ID( PutCharacterInvertFont )
#define MSG_PutCharacterVerticalInvertFont     MSG_ID( PutCharacterVerticalInvertFont )
#define MSG_PutStringFontEx                    MSG_ID( PutStringFontEx )
#define MSG_PutStringVerticalFontEx            MSG_ID( PutStringVerticalFontEx )
#define MSG_PutStringInvertFontEx              MSG_ID( PutStringInvertFontEx )
#define MSG_PutStringInvertVerticalFontEx      MSG_ID( PutStringInvertVerticalFontEx )
#define MSG_GetMaxStringLengthFont             MSG_ID( GetMaxStringLengthFont )
#define MSG_GetImageSize                       MSG_ID( GetImageSize )
#define MSG_ColorAverage                       MSG_IC( ColorAverage )
// these messages follow all others... and are present to handle
// LoadImageFile
// #define MSG_LoadImageFile (no message)
// #define MSG_LoadFont      (no message)
#define MSG_UnloadFont                         MSG_ID( UnloadFont )
#define MSG_BeginTransferData                  MSG_ID( BeginTransferData )
#define MSG_ContinueTransferData               MSG_ID( ContinueTransferData )
#define MSG_DecodeTransferredImage             MSG_ID( DecodeTransferredImage )
#define MSG_AcceptTransferredFont              MSG_ID( AcceptTransferredFont )
#define MSG_SyncImage                          MSG_ID( SyncImage )
#define MSG_IntersectRectangle                 MSG_ID( IntersectRectangle )
#define MSG_MergeRectangle                     MSG_ID( MergeRectangle)
#define MSG_GetImageSurface                    MSG_ID( GetImageSurface )
#define MSG_SetImageAuxRect                    MSG_ID(SetImageAuxRect)
#define MSG_GetImageAuxRect                    MSG_ID(GetImageAuxRect)
#define MSG_OrphanSubImage                     MSG_ID(OrphanSubImage)
#define MSG_GetGlobalFonts                     MSG_ID(GetGlobalFonts)
#define MSG_AdoptSubImage                      MSG_ID(AdoptSubImage)


#define MSG_MakeSpriteImageFileEx   MSG_ID(MakeSpriteImageFileEx)
#define MSG_MakeSpriteImageEx       MSG_ID(MakeSpriteImageEx)
#define MSG_UnmakeSprite            MSG_ID(UnmakeSprite )
#define MSG_rotate_scaled_sprite    MSG_ID(rotate_scaled_sprite)
#define MSG_rotate_sprite           MSG_ID(rotate_sprite)
#define MSG_BlotSprite              MSG_ID(BlotSprite)
#define MSG_SetSpritePosition  MSG_ID(  SetSpritePosition )
#define MSG_SetSpriteHotspot  MSG_ID(  SetSpriteHotspot )
#define MSG_InternalRenderFont          MSG_ID(InternalRenderFont)
#define MSG_InternalRenderFontFile      MSG_ID(InternalRenderFontFile)
#define MSG_RenderScaledFontData              MSG_ID(RenderScaledFontData)
#define MSG_RenderFontData              MSG_ID(RenderFontData)
#define MSG_RenderFontFileEx              MSG_ID(RenderFontFileEx)
#define MSG_DestroyFont              MSG_ID(DestroyFont)
#define MSG_GetFontRenderData              MSG_ID(GetFontRenderData)
#define MSG_SetFontRendererData              MSG_ID(SetFontRendererData)
#endif

#ifdef USE_IMAGE_LEVEL
#warning ...
#define PASTELEVEL(level,name) level##name
#define LEVEL_ALIAS(name)      PASTELEVEL(USE_IMAGE_LEVEL,name)
#  ifdef STUPID_NO_DATA_EXPORTS
#define PLEVEL_ALIAS(name)      (*PASTELEVEL(USE_IMAGE_LEVEL,_PASTE(_,name)))
#  else
#define PLEVEL_ALIAS(name)      (*PASTELEVEL(USE_IMAGE_LEVEL,name))
#  endif
#define SetStringBehavior                  LEVEL_ALIAS(SetStringBehavior )
#define SetBlotMethod                      LEVEL_ALIAS(SetBlotMethod )
#define BuildImageFileEx                   LEVEL_ALIAS(BuildImageFileEx )
#define MakeImageFileEx                    LEVEL_ALIAS(MakeImageFileEx )
#define MakeSubImageEx                     LEVEL_ALIAS(MakeSubImageEx )
#define RemakeImageEx                      LEVEL_ALIAS(RemakeImageEx )
#define ResizeImageEx                      LEVEL_ALIAS(ResizeImageEx )
#define MoveImage                          LEVEL_ALIAS(MoveImage )
#define LoadImageFileEx                    LEVEL_ALIAS(LoadImageFileEx )
#define DecodeMemoryToImage                LEVEL_ALIAS(DecodeMemoryToImage )
#define UnmakeImageFileEx                  LEVEL_ALIAS(UnmakeImageFileEx )
#define BlatColor                          LEVEL_ALIAS(BlatColor )
#define BlatColorAlpha                     LEVEL_ALIAS(BlatColorAlpha )
#define BlotImageSizedEx                   LEVEL_ALIAS(BlotImageSizedEx )
#define BlotImageEx                        LEVEL_ALIAS(BlotImageEx )
#define BlotScaledImageSizedEx             LEVEL_ALIAS(BlotScaledImageSizedEx )
#define plot                               PLEVEL_ALIAS(plot )
#define plotalpha                          PLEVEL_ALIAS(plotalpha )
#error 566
#define getpixel                           PLEVEL_ALIAS(getpixel )
#define do_line                            PLEVEL_ALIAS(do_line )
#define do_lineAlpha                       PLEVEL_ALIAS(do_lineAlpha )
#define do_hline                           PLEVEL_ALIAS(do_hline )
#define do_vline                           PLEVEL_ALIAS(do_vline )
#define do_hlineAlpha                      PLEVEL_ALIAS(do_hlineAlpha )
#define do_vlineAlpha                      PLEVEL_ALIAS(do_vlineAlpha )
#define GetDefaultFont                     LEVEL_ALIAS(GetDefaultFont )
#define GetFontHeight                      LEVEL_ALIAS(GetFontHeight )
#define GetStringSizeFontEx                LEVEL_ALIAS(GetStringSizeFontEx )
#define PutCharacterFont                   LEVEL_ALIAS(PutCharacterFont )
#define PutCharacterVerticalFont           LEVEL_ALIAS(PutCharacterVerticalFont )
#define PutCharacterInvertFont             LEVEL_ALIAS(PutCharacterInvertFont )
#define PutCharacterVerticalInvertFont     LEVEL_ALIAS(PutCharacterVerticalInvertFont )
#define PutStringFontEx                    LEVEL_ALIAS(PutStringFontEx )
#define PutStringVerticalFontEx            LEVEL_ALIAS(PutStringVerticalFontEx )
#define PutStringInvertFontEx              LEVEL_ALIAS(PutStringInvertFontEx )
#define PutStringInvertVerticalFontEx      LEVEL_ALIAS(PutStringInvertVerticalFontEx )
#define GetMaxStringLengthFont             LEVEL_ALIAS(GetMaxStringLengthFont )
#define GetImageSize                       LEVEL_ALIAS(GetImageSize )
#define LoadFont                           LEVEL_ALIAS(LoadFont )
#define UnloadFont                         LEVEL_ALIAS(UnloadFont )
#define ColorAverage                       LEVEL_ALIAS(ColorAverage)
#define SyncImage                          LEVEL_ALIAS(SyncImage )
#define IntersectRectangle                 LEVEL_ALIAS( IntersectRectangle )
#define MergeRectangle                     LEVEL_ALIAS(MergeRectangle)
#define GetImageSurface                    LEVEL_ALIAS(GetImageSurface)
#define SetImageAuxRect                    LEVEL_ALIAS(SetImageAuxRect)
#define GetImageAuxRect                    LEVEL_ALIAS(GetImageAuxRect)
#define OrphanSubImage                     LEVEL_ALIAS(OrphanSubImage)
#define GetGlobalFonts                     LEVEL_ALIAS(GetGlobalFonts)
#define AdoptSubImage                      LEVEL_ALIAS(AdoptSubImage)
#define InternalRenderFont          LEVEL_ALIAS(InternalRenderFont)
#define InternalRenderFontFile      LEVEL_ALIAS(InternalRenderFontFile)
#define RenderScaledFontData              LEVEL_ALIAS(RenderScaledFontData)
#define RenderFontData              LEVEL_ALIAS(RenderFontData)
#define RenderFontFileEx              LEVEL_ALIAS(RenderFontFileEx)
#endif

// these macros provide common extensions for 
// commonly used shorthands of the above routines.
// no worry - one way or another, the extra data is 
// created, and the base function called, it's a sad 
// truth of life, that one codebase is easier to maintain
// than a duplicate copy for each minor case.
// although - special forwards - such as DBG_SRC will just dissappear
// in certain compilation modes (NON_DEBUG)

#define BuildImageFile(p,w,h) BuildImageFileEx( p,w,h DBG_SRC )
#define MakeImageFile(w,h) MakeImageFileEx( w,h DBG_SRC )
#define MakeSubImage( image, x, y, w, h ) MakeSubImageEx( image, x, y, w, h DBG_SRC )
#define RemakeImage(p,pc,w,h) RemakeImageEx(p,pc,w,h DBG_SRC)
#define ResizeImage( p,w,h) ResizeImageEx( p,w,h DBG_SRC )
#define UnmakeImageFile(pif) UnmakeImageFileEx( pif DBG_SRC )
#define MakeSpriteImage(image) MakeSpriteImageEx(image DBG_SRC)
#define MakeSpriteImageFile(file) MakeSpriteImageFileEx( image DBG_SRC )

IMAGE_PROC( void, FlipImageEx )( Image pif DBG_PASS );
#define FlipImage(pif) FlipImageEx( pif DBG_SRC )

#define LoadImageFile(file) LoadImageFileEx( file DBG_SRC )
#define ClearImageTo(img,color) BlatColor(img,0,0,(img)->width,(img)->height, color )
#define ClearImage(img) BlatColor(img,0,0,(img)->width,(img)->height, 0 )

#define BlotImage( pd, ps, x, y ) BlotImageEx( pd, ps, x, y, TRUE, BLOT_COPY )
#define BlotImageAlpha( pd, ps, x, y, a ) BlotImageEx( pd, ps, x, y, a, BLOT_COPY )
#define BlotImageSized( pd, ps, x, y, w, h ) BlotImageSizedEx( pd, ps, x, y, 0, 0, w, h, TRUE, BLOT_COPY )
#define BlotImageSizedAlpha( pd, ps, x, y, w, h, a ) BlotImageSizedEx( pd, ps, x, y, 0, 0, w, h, a, BLOT_COPY )
#define BlotImageSizedTo( pd, ps, xd, yd, xs, ys, w, h )  BlotImageSizedEx( pd, ps, xd, yd, xs, ys, w, h, TRUE, BLOT_COPY )

#define BlotImageShaded( pd, ps, xd, yd, c ) BlotImageEx( pd, ps, xd, yd, TRUE, BLOT_SHADED, c )
#define BlotImageShadedSized( pd, ps, xd, yd, xs, ys, ws, hs, c ) BlotImageSizedEx( pd, ps, xd, yd, xs, ys, ws, hs, TRUE, BLOT_SHADED, c )

#define BlotImageMultiShaded( pd, ps, xd, yd, r, g, b ) BlotImageEx( pd, ps, xd, yd, ALPHA_TRANSPARENT, BLOT_MULTISHADE, r, g, b )
#define BlotImageMultiShadedSized( pd, ps, xd, yd, xs, ys, ws, hs, r, g, b ) BlotImageSizedEx( pd, ps, xd, yd, xs, ys, ws, hs, TRUE, BLOT_MULTISHADE, r, g, b )

#define BlotScaledImageSized( pd, ps, xd, yd, wd, hd, xs, ys, ws, hs ) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, xs, ys, ws, hs, 0, BLOT_COPY )
#define BlotScaledImageSizedMultiShaded( pd, ps, xd, yd, wd, hd, xs, ys, ws, hs,r,g,b ) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, xs, ys, ws, hs, 0, BLOT_MULTISHADE,r,g,b )

#define BlotScaledImageSizedTo( pd, ps, xd, yd, wd, hd) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, (ps)->width, (ps)->height, 0, BLOT_COPY )
#define BlotScaledImageSizedToAlpha( pd, ps, xd, yd, wd, hd, a) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, (ps)->width, (ps)->height, a, BLOT_COPY )

#define BlotScaledImageSizedToShaded( pd, ps, xd, yd, wd, hd,shade) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, (ps)->width, (ps)->height, 0,BLOT_SHADED, shade )
#define BlotScaledImageSizedToShadedAlpha( pd, ps, xd, yd, wd, hd,a,shade) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, (ps)->width, (ps)->height, a, BLOT_SHADED, shade )

#define BlotScaledImageSizedToMultiShaded( pd, ps, xd, yd, wd, hd,r,g,b) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, (ps)->width, (ps)->height, 0,BLOT_MULTISHADE, r,g,b )
#define BlotScaledImageSizedToMultiShadedAlpha( pd, ps, xd, yd, wd, hd,a,r,g,b) BlotScaledImageSizedEx( pd, ps, xd, yd, wd, hd, 0, 0, (ps)->width, (ps)->height, a,BLOT_MULTISHADE, r,g,b )

#define BlotScaledImageAlpha( pd, ps, t ) BlotScaledImageSizedEx( pd, ps, 0, 0, (pd)->width, (pd)->height, 0, 0, (ps)->width, (ps)->height, t, BLOT_COPY )
#define BlotScaledImageShadedAlpha( pd, ps, t, shade ) BlotScaledImageSizedEx( pd, ps, 0, 0, (pd)->width, (pd)->height, 0, 0, (ps)->width, (ps)->height, t, BLOT_SHADED, shade )
#define BlotScaledImageMultiShadedAlpha( pd, ps, t, r, g, b ) BlotScaledImageSizedEx( pd, ps, 0, 0, (pd)->width, (pd)->height, 0, 0, (ps)->width, (ps)->height, t, BLOT_MULTISHADE, r, g, b )

#define BlotScaledImage( pd, ps ) BlotScaledImageSizedEx( pd, ps, 0, 0, (pd)->width, (pd)->height, 0, 0, (ps)->width, (ps)->height, 0, BLOT_COPY )
#define BlotScaledImageShaded( pd, ps, shade ) BlotScaledImageSizedEx( pd, ps, 0, 0, (pd)->width, (pd)->height, 0, 0, (ps)->width, (ps)->height, 0, BLOT_SHADED, shade )
#define BlotScaledImageMultiShaded( pd, ps, r, g, b ) BlotScaledImageSizedEx( pd, ps, 0, 0, (pd)->width, (pd)->height, 0, 0, (ps)->width, (ps)->height, 0, BLOT_MULTISHADE, r, g, b )

#define BlotScaledImageTo( pd, ps )  BlotScaledImageToEx( pd, ps, FALSE, BLOT_COPY )

// now why would we need an inverse line? I don't get it....
#define do_inv_line(pb,x,y,xto,yto,d) do_line( pb,y,x,yto,xto,d)

#define PutCharacter(i,x,y,fore,back,c)               PutCharacterFont(i,x,y,fore,back,c,NULL )
#define PutCharacterVertical(i,x,y,fore,back,c)       PutCharacterVerticalFont(i,x,y,fore,back,c,NULL )
#define PutCharacterInvert(i,x,y,fore,back,c)         PutCharacterInvertFont(i,x,y,fore,back,c,NULL )
#define PutCharacterInvertVertical(i,x,y,fore,back,c) PutCharacterInvertVerticalFont(i,x,y,fore,back,c,NULL )
#define PutCharacterInvertVerticalFont(i,x,y,fore,back,c,f) PutCharacterVerticalInvertFont(i,x,y,fore,back,c,f )

#define PutString(pi,x,y,fore,back,pc) PutStringFontEx( pi, x, y, fore, back, pc, strlen(pc), NULL )
#define PutStringEx(pi,x,y,color,back,pc,len) PutStringFontEx( pi, x, y, color,back,pc,len,NULL )
#define PutStringFont(pi,x,y,fore,back,pc,font) PutStringFontEx(pi,x,y,fore,back,pc,strlen(pc), font )

#define PutStringVertical(pi,x,y,fore,back,pc) PutStringVerticalFontEx( pi, x, y, fore, back, pc, strlen(pc), NULL )
#define PutStringVerticalEx(pi,x,y,color,back,pc,len) PutStringVerticalFontEx( pi, x, y, color,back,pc,len,NULL )
#define PutStringVerticalFont(pi,x,y,fore,back,pc,font) PutStringVerticalFontEx(pi,x,y,fore,back,pc,strlen(pc), font )

#define PutStringInvert( pi, x, y, fore, back, pc ) PutStringInvertFontEx( pi, x, y, fore, back, pc,strlen(pc), NULL )
#define PutStringInvertEx( pi, x, y, fore, back, pc, nLen ) PutStringInvertFontEx( pi, x, y, fore, back, pc, nLen, NULL )
#define PutStringInvertFont( pi, x, y, fore, back, pc, nLen ) PutStringInvertFontEx( pi, x, y, fore, back, pc, strlen(pc), font )

#define PutStringInvertVertical( pi, x, y, fore, back, pc ) PutStringInvertVerticalFontEx( pi, x, y, fore, back, pc, strlen(pc), NULL )
#define PutStringInvertVerticalEx( pi, x, y, fore, back, pc, nLen ) PutStringInvertVerticalFontEx( pi, x, y, fore, back, pc, nLen, NULL )
#define PutStringInvertVerticalFont( pi, x, y, fore, back, pc, font ) PutStringInvertVerticalFontEx( pi, x, y, fore, back, pc, strlen(pc), font )

//IMG_PROC _32 PutMenuStringFontEx        ( ImageFile *pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, PFONT font );
//#define PutMenuStringFont(img,x,y,fore,back,string,font) PutMenuStringFontEx( img,x,y,fore,back,string,strlen(string),font)
//#define PutMenuString(img,x,y,fore,back,str)           PutMenuStringFont(img,x,y,fore,back,str,NULL)
//
//IMG_PROC _32 PutCStringFontEx           ( ImageFile *pImage, S_32 x, S_32 y, CDATA color, CDATA background, CTEXTSTR pc, _32 nLen, PFONT font );
//#define PutCStringFont(img,x,y,fore,back,string,font) PutCStringFontEx( img,x,y,fore,back,string,strlen(string),font)
//#define PutCString( img,x,y,fore,back,string) PutCStringFont(img,x,y,fore,back,string,NULL )

//IMG_PROC _32 GetMenuStringSizeFontEx( CTEXTSTR string, _32 len, _32 *width, _32 *height, PFONT font );
//IMG_PROC _32 GetMenuStringSizeFont( CTEXTSTR string, _32 *width, _32 *height, PFONT font );

#define GetStringSizeEx(s,len,pw,ph) GetStringSizeFontEx( (s),len,pw,ph,NULL)
#define GetStringSize(s,pw,ph)       GetStringSizeFontEx( (s),(s)?strlen(s):0,pw,ph,NULL)
#define GetStringSizeFont(s,pw,ph,f) GetStringSizeFontEx( (s),(s)?strlen(s):0,pw,ph,f )

#ifdef __cplusplus
IMAGE_NAMESPACE_END
using namespace sack::image;
#endif

#endif


//-------------------------------------------------------------------------
// $Log: image.h,v $
// Revision 1.70  2005/06/20 17:24:25  jim
// Sync with dev repository.
//
// Revision 1.69  2005/05/30 11:56:20  d3x0r
// various fixes... working on psilib update optimization... various stabilitizations... also extending msgsvr functionality.
//
// Revision 1.68  2005/05/25 16:50:09  d3x0r
// Synch with working repository.
//
// Revision 1.69  2005/04/13 18:29:19  jim
// Export DecodeMemoryToImage in interface.
//
// Revision 1.68  2005/04/05 11:56:04  panther
// Adding sprite support - might have added an extra draw callback...
//
// Revision 1.67  2005/01/18 10:48:19  panther
// Define image interface export so there's no conflict between image and display_image
//
// Revision 1.66  2004/08/17 02:30:44  d3x0r
// Fix reference macro for ColorAverage
//
// Revision 1.65  2004/06/21 07:47:36  d3x0r
// Checkpoint - make system rouding out nicly.
//
// Revision 1.64  2004/06/01 21:46:33  d3x0r
// Parenthise USE_IMAGE_INTERFACE usage
//
// Revision 1.63  2004/06/01 05:58:32  d3x0r
// Include procreg instead of interface.h
//
// Revision 1.62  2004/04/26 09:47:24  d3x0r
// Cleanup some C++ problems, and standard C issues even...
//
// Revision 1.61  2004/03/04 01:09:47  d3x0r
// Modifications to force slashes to wlib.  Updates to Interfaces to be gotten from common interface manager.
//
// Revision 1.60  2003/10/28 01:14:34  panther
// many changes to implement msgsvr on windows.  Even to get displaylib service to build, there's all sorts of errors in inconsistant definitions...
//
// Revision 1.59  2003/10/14 16:26:29  panther
// Remove redundant Log Messages.  Fix string function names.
//
// Revision 1.58  2003/10/07 20:30:13  panther
// Fix name mangling for PutStringInvertVertical
//
// Revision 1.57  2003/09/20 00:41:04  panther
// Fix order of interface def
//
// Revision 1.56  2003/09/19 16:40:35  panther
// Implement Adopt and Orphan sub image - for up coming Sheet Control
//
// Revision 1.55  2003/09/18 12:14:49  panther
// MergeRectangle Added.  Seems Control edit near done (fixing move/size errors)
//
// Revision 1.54  2003/09/18 08:35:19  panther
// Oops fudged the getauxrect
//
// Revision 1.53  2003/09/18 07:53:20  panther
// Added to idle - IdleFor - which sleeps for a time, calling idle procs
//
// Revision 1.52  2003/09/15 17:06:37  panther
// Fixed to image, display, controls, support user defined clipping , nearly clearing correct portions of frame when clearing hotspots...
//
// Revision 1.51  2003/09/08 12:59:54  panther
// Oops forgot a semi-colon
//
// Revision 1.50  2003/09/08 12:56:51  panther
// Define common image data x, y info (IMAGE_RECTANGLE) in an anonymous structure method
//
// Revision 1.49  2003/09/01 20:04:37  panther
// Added OpenGL Interface to windows video lib, Modified RCOORD comparison
//
// Revision 1.48  2003/08/27 16:05:09  panther
// Define image and renderer sync functions
//
// Revision 1.47  2003/07/24 16:56:41  panther
// Updates to expliclity define C procedure model for callbacks and assembly modules - incomplete
//
// Revision 1.46  2003/06/25 15:36:58  panther
// Make that two broken macros
//
// Revision 1.45  2003/06/25 15:33:38  panther
// Ooops broken one of the function macros
//
// Revision 1.44  2003/06/24 11:46:10  panther
// Fix definition of pointer second_level
//
// Revision 1.43  2003/05/01 21:32:08  panther
// Cleaned up from having moved several methods into frame/control common space
//
// Revision 1.42  2003/04/24 00:03:49  panther
// Added ColorAverage to image... Fixed a couple macros
//
// Revision 1.41  2003/03/30 21:38:54  panther
// Fix MSG_ definitions.  Fix lack of anonymous unions
//
// Revision 1.40  2003/03/29 22:51:59  panther
// New render/image layering ability.  Added support to Display for WIN32 usage (native not SDL)
//
// Revision 1.39  2003/03/27 15:36:38  panther
// Changes were done to limit client messages to server - but all SERVER-CLIENT messages were filtered too... Define LOWEST_BASE_MESSAGE
//
// Revision 1.38  2003/03/25 23:37:13  panther
// Pass signed parameter to ResizeImage
//
// Revision 1.37  2003/03/25 08:38:11  panther
// Add logging
//
// Revision 1.36  2003/03/24 00:22:03  panther
// Parenthise parameters to GetStringSize macros
//
// Revision 1.35  2003/03/21 09:00:44  panther
// Define exported data type functions (needed for BCC32/16)
//
// Revision 1.34  2003/03/19 02:03:45  panther
// BCC32 compatibilty updates
//
// Revision 1.33  2003/03/19 00:58:48  panther
// Compatibility fixes for BCC32
//
// Revision 1.32  2003/03/17 08:19:38  panther
// Fix PutCharacter macro
//
// Revision 1.31  2003/03/10 15:05:20  panther
// Master Checkpoint - Tweaks fixes, etc all go in now.
//
// Revision 1.30  2003/02/23 18:26:12  panther
// Exoprt handle message proc for message clients
//
// Revision 1.29  2003/02/20 02:34:58  panther
// Added LoadFont definition
//
// Revision 1.28  2003/02/19 01:44:53  panther
// Added image loader program - image load commpleted - table bias solved
//
// Revision 1.27  2003/02/18 18:01:37  panther
// Added LoadImageFile to services, test8 to test it
//
// Revision 1.26  2003/02/17 02:58:23  panther
// Changes include - better support for clipped images in displaylib.
// More events handled.
// Modifications to image structure and better unification of clipping
// ideology.
// Better definition of image and render interfaces.
// MUCH logging has been added and needs to be trimmed out.
//
// Revision 1.25  2003/02/13 13:26:09  panther
// Need to set the invert_image flag
//
// Revision 1.24  2003/02/13 12:54:35  panther
// Added Loadimagefileex back in
//
// Revision 1.23  2003/02/12 22:17:31  panther
// Cleanups for modified image.h, render.h - minor patches to psi_client/server
//
// Revision 1.22  2003/02/12 20:30:53  panther
// Image interface.h -> image.h
//
// Revision 1.12  2003/02/12 14:52:12  panther
// Migrate vidlib to render_interface compatibility, migrate makefiles to newer system
//
// Revision 1.11  2003/02/10 08:56:42  panther
// Fix OwnMouse
//
// Revision 1.10  2003/02/09 04:03:34  panther
// General tweaks to improve functionality
//
// Revision 1.9  2003/02/07 13:59:50  panther
// Minor typo fixes, minor additions to support added routines
//
// Revision 1.8  2003/02/05 23:39:29  panther
// Begin message inteface for control_interface
//
// Revision 1.7  2003/02/05 23:34:45  panther
// Landmark commit for all server/client message service things
//
// Revision 1.6  2003/02/05 23:22:01  panther
// Added literal data to Image definition
//
// Revision 1.5  2003/02/05 22:40:46  panther
// Fixup image library to be explicitly 32 bit - no questionable 'int's
//
// Revision 1.4  2003/02/05 11:00:44  panther
// Make render interface more similar to image_interface which is the more evolved methodology.
//
// Revision 1.3  2002/12/12 11:03:06  panther
// New image definitions - working on interface defs.
//
// Revision 1.2  2002/12/02 12:57:11  panther
// image interface changes all over, viewlib...
//
