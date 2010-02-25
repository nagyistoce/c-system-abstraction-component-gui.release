#ifndef COLOR_STRUCTURE_DEFINED
#define COLOR_STRUCTURE_DEFINED

#include <sack_types.h>

#ifdef __cplusplus
SACK_NAMESPACE
	namespace image {
#endif

// byte index values for colors on the video buffer...
#define I_BLUE 0
#define I_GREEN 1
#define I_RED 2
#define I_ALPHA 3

#ifdef _OPENGL_DRIVER
#define Color( r,g,b ) (((_32)( ((_8)(r))|((_16)((_8)(g))<<8))|(((_32)((_8)(b))<<16)))|0xFF000000)
#define AColor( r,g,b,a ) (((_32)( ((_8)(r))|((_16)((_8)(g))<<8))|(((_32)((_8)(b))<<16)))|((a)<<24))
#define SetAlpha( rgb, a ) ( ((rgb)&0xFFFFFF) | ( (a)<<24 ) )
#define SetGreen( rgb, g ) ( ((rgb)&0xFFFF00FF) | ( ((g)&0xFF)<<8 ) )
#define GLColor( c )  (c)
#define AlphaVal(color) (((color) >> 24) & 0xFF)
#define RedVal(color)   (((color)) & 0xFF)
#define GreenVal(color) (((color) >> 8) & 0xFF)
#define BlueVal(color)  (((color) >> 16) & 0xFF)
#else
#ifdef _WIN64
#define AND_FF 0xFF
#else
#define AND_FF
#endif
#define Color( r,g,b ) (((_32)( ((_8)((b)AND_FF))|((_16)((_8)((g)AND_FF))<<8))|(((_32)((_8)((r)AND_FF))<<16)))|0xFF000000)
#define AColor( r,g,b,a ) (((_32)( ((_8)((b)AND_FF))|((_16)((_8)((g)AND_FF))<<8))|(((_32)((_8)((r)AND_FF))<<16)))|(((a)AND_FF)<<24))
#define SetAlpha( rgb, a ) ( ((rgb)&0x00FFFFFF) | ( (a)<<24 ) )
#define SetGreen( rgb, g ) ( ((rgb)&0xFFFF00FF) | ( ((g)0x0000FF)<<8 ) )
#define GLColor( c )  (((c)&0xFF00FF00)|(((c)&0xFF0000)>>16)|(((c)&0x0000FF)<<16))
#define AlphaVal(color) (((color) >> 24) & 0xFF)
#define RedVal(color)   (((color) >> 16) & 0xFF)
#define GreenVal(color) (((color) >> 8) & 0xFF)
#define BlueVal(color)  (((color)) & 0xFF)
#endif
typedef char COLOR[4];
typedef _32 CDATA, *PCDATA;  // color data raw...
typedef COLOR *PCOLOR;
//typedef unsigned long COLOR, *PCOLOR;  // 32 bit color (hopefully)
//typedef unsigned short COLOR, *PCOLOR;

//-----------------------------------------------
// common color definitions....
//-----------------------------------------------
// both yellows need to be fixed.
#define BASE_COLOR_BLACK         Color( 0,0,0 )
#define BASE_COLOR_BLUE          Color( 0, 0, 128 )
#define BASE_COLOR_GREEN         Color( 0, 128, 0 )
#define BASE_COLOR_CYAN          Color( 0, 128, 128 )
#define BASE_COLOR_RED           Color( 192, 32, 32 )
#define BASE_COLOR_BROWN         Color( 140, 140, 0 )
#define BASE_COLOR_MAGENTA       Color( 160, 0, 160 )
#define BASE_COLOR_LIGHTGREY     Color( 192, 192, 192 )
#define BASE_COLOR_DARKGREY      Color( 128, 128, 128 )
#define BASE_COLOR_LIGHTBLUE     Color( 0, 0, 255 )
#define BASE_COLOR_LIGHTGREEN    Color( 0, 255, 0 )
#define BASE_COLOR_LIGHTCYAN     Color( 0, 255, 255 )
#define BASE_COLOR_LIGHTRED      Color( 255, 0, 0 )
#define BASE_COLOR_LIGHTMAGENTA  Color( 255, 0, 255 )
#define BASE_COLOR_YELLOW        Color( 255, 255, 0 )
#define BASE_COLOR_WHITE         Color( 255, 255, 255 )
#define BASE_COLOR_ORANGE        Color( 204,96,7 )
#define BASE_COLOR_PURPLE        0xFF7A117C
#ifdef __cplusplus

}; // 	namespace image {
SACK_NAMESPACE_END
using namespace sack::image;
#endif
#endif



// $Log: colordef.h,v $
// Revision 1.4  2003/04/24 00:03:49  panther
// Added ColorAverage to image... Fixed a couple macros
//
// Revision 1.3  2003/03/25 08:38:11  panther
// Add logging
//
