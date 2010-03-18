#ifndef BANNER_WIDGET_DEFINED
#define BANNER_WIDGET_DEFINED

#include <controls.h>


#if !defined(__STATIC__) && !defined(__UNIX__)
#ifdef BANNER_SOURCE
#define BANNER_PROC(type,name) __declspec(dllexport) type CPROC name
#else
#define BANNER_PROC(type,name) __declspec(dllimport) type CPROC name
#endif
#else
#ifdef BANNER_SOURCE
#define BANNER_PROC(type,name) EXPORT_METHOD type name
#else
#define BANNER_PROC(type,name) extern type name
#endif
#endif


typedef struct banner_tag *PBANNER;

#define BANNER_CLICK    0x01
#define BANNER_TIMEOUT  0x02
#define BANNER_CLOSED   0x04
#define BANNER_WAITING  0x08
#define BANNER_OKAY     0x10
#define BANNER_DEAD     0x20 // banner does not click, banner does not timeout...
#define BANNER_TOP      0x40 // banner does not click, banner does not timeout...
#define BANNER_NOWAIT   0x80 // don't wait in banner create...
#define BANNER_EXPLORER  0x100 // don't wait in banner create...
#define BANNER_ABSOLUTE_TOP      (0x200|BANNER_TOP) // banner does not click, banner does not timeout...
#define BANNER_EXTENDED_RESULT     0x400  //OKAY/CANCEL on YesNo dialog 

#define BANNER_OPTION_YESNO          0x010000
#define BANNER_OPTION_OKAYCANCEL     0x020000

#define BANNER_OPTION_LEFT_JUSTIFY   0x100000

	// banner will have a keyboard to enter a value...
   // banner text will show above the keyboard?
#include <render.h>

#define BANNER_OPTION_KEYBOARD       0x01000000

BANNER_PROC( int, CreateBannerExtended )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor, int lines, int cols, int display );
BANNER_PROC( int, CreateBannerExx )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout, CDATA textcolor, CDATA basecolor );
BANNER_PROC( int, CreateBannerEx )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text, int options, int timeout );
BANNER_PROC( int, CreateBanner )( PRENDERER parent, PBANNER *ppBanner, CTEXTSTR text );
BANNER_PROC( void, SetBannerOptions )( PBANNER, _32 flags, _32 extra );
BANNER_PROC( void, SetBannerOptionsEx )( PBANNER*, _32 flags, _32 extra );
#define SetBannerOptions( banner,flags,extra ) SetBannerOptionsEx( &(banner), flags,extra )

// click, closed, etc...
// if it's in wait, it will not be destroyed until after
// wait results...
// if wait is not active, .. then the resulting kill banner
// will clear the ppBanner originally passed... so that becomes
// the wait state variable...
// results false if no banner to wait for.
BANNER_PROC( void, SetBannerText )( PBANNER banner, TEXTCHAR *text );
BANNER_PROC( int, WaitForBanner )( PBANNER banner );
BANNER_PROC( int, WaitForBannerEx )( PBANNER *banner );
#define WaitForBanner(b) WaitForBannerEx( &(b) )
BANNER_PROC( void, RemoveBannerEx )( PBANNER *banner DBG_PASS );
BANNER_PROC( void, RemoveBanner )( PBANNER banner );
#define RemoveBanner(b) RemoveBannerEx( &(b) DBG_SRC )
BANNER_PROC( Font, GetBannerFont )( void );
BANNER_PROC( _32, GetBannerFontHeight )( void );

BANNER_PROC( PRENDERER, GetBannerRenderer )( PBANNER banner );
BANNER_PROC( PSI_CONTROL, GetBannerControl )( PBANNER banner );


#define BannerNoWait( text ) CreateBannerEx( NULL, NULL, text, BANNER_NOWAIT|BANNER_DEAD, 0 )
#define BannerTopNoWait( text ) CreateBannerEx( NULL, NULL, text, BANNER_NOWAIT|BANNER_DEAD|BANNER_TOP, 0 )
#define BannerTop( text ) CreateBannerEx( NULL, NULL, text, BANNER_DEAD|BANNER_TOP|BANNER_TIMEOUT, 5000 )
#define BannerYesNo( renderparent, text ) CreateBannerEx( renderparent, NULL, text, BANNER_OPTION_YESNO, 0 )
#define BannerTopYesNo( renderparent, text ) CreateBannerEx( renderparent, NULL, text, BANNER_TOP|BANNER_OPTION_YESNO, 0 )
#define BannerYesNoEx( renderparent, pb, text ) CreateBannerEx( renderparent, pb, text, BANNER_OPTION_YESNO, 0 )
#define BannerMessage( text ) CreateBanner( NULL, NULL, text )
#define BannerMessageEx( renderparent,text ) CreateBanner( renderparent, NULL, text )
#define RemoveBannerMessage() RemoveBanner( NULL )

#endif
